// ============================================================
// 
// This file is a part of the eProDrums rock band project
// 
// Copyright (C) 2012 by Vissale Neang <fromtonrouge at gmail dot com>
// 
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 2 of
// the License or (at your option) version 3 or any later version
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
// 
// ============================================================ 

#include "MainWindow.h"
#include "DialogAbout.h"
#include "Settings.h"
#include "SettingsDlg.h"
#include "TreeViewParameters.h"

#include "Pad.h"
#include "HiHatPedalElement.h"
#include "HiHatPositionCurve.h"

#include <QtGui/QMessageBox>
#include <QtGui/QFileDialog>
#include <QtCore/QFileInfo>

#include <boost/shared_ptr.hpp>
#include <boost/filesystem.hpp>
#include <boost/bind.hpp>
#include <boost/format.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string/trim.hpp>
namespace fs = boost::filesystem;

#include <fstream>
#include <exception>
#include <algorithm>
#include <limits>
#include <iostream>

BOOST_CLASS_EXPORT(HiHatPedalElement); 

std::string MainWindow::APPLICATION_NAME("eProDrums");
std::string MainWindow::APPLICATION_VERSION("dev");

Q_DECLARE_METATYPE(Slot::Ptr)
Q_DECLARE_METATYPE(MidiMessage)

/**
 * processMidi is called every milliseconds \see on_pushButtonStart_clicked() and Pt_Start()
 */
void processMidi(PtTimestamp timestamp, void* pUserData)
{
	MainWindow* pMainWindow = static_cast<MainWindow*>(pUserData);
	PmStream* pMidiIn = pMainWindow->getMidiInStream();
	PmStream* pMidiOut = pMainWindow->getMidiOutStream();
	if (!pMidiIn || !pMidiOut)
	{
		return;
	}

	PmEvent buffer; // just one message at a time
	MidiMessage::Status status = 0;
	MidiMessage::Data data1 = 0;
	MidiMessage::Data data2 = 0;
	const UserSettings& userSettings = pMainWindow->getConfig();
	while (Pm_Poll(pMidiIn))
	{
		if (Pm_Read(pMidiIn, &buffer, 1) == pmBufferOverflow) 
		{
			continue;
		}

		// Unless there was overflow, we should have a message now
		status = Pm_MessageStatus(buffer.message);
		data1 = Pm_MessageData1(buffer.message);
		data2 = Pm_MessageData2(buffer.message);

		MainWindow::Mutex::scoped_lock lock(pMainWindow->_mutex);
		MidiMessage midiMessage(pMainWindow->_clock.now(), timestamp, status, data1, data2);

		// First filtering, we get only note-on and controller messages
		if (midiMessage.isNoteOnMsg() || midiMessage.isControllerMsg())
		{
			if (midiMessage.isNoteOnMsg() && midiMessage.getValue()==0)
			{
				// Ignore note-on with velocity 0 (it happens...)
				// But we print the msg if logs are on
				if (userSettings.isLogs() && userSettings.isLog(UserSettings::LOG_RAW_DATA))
				{
					midiMessage.print();
				}
			}
			else
			{
				pMainWindow->addIncomingMidiMessage(midiMessage);
				lock.unlock();
				pMainWindow->notify();
			}
		}
		else if (userSettings.isLogs() && userSettings.isLog(UserSettings::LOG_RAW_DATA))
		{
			midiMessage.print();
		}
	}
}

void MainWindow::toLog(const std::string& szText)
{
	emit sLog(boost::algorithm::trim_copy_if(szText, boost::is_any_of("\n")).c_str());
}

MainWindow::MainWindow():
	_pOldStreambuf(std::cout.rdbuf()),
	_pSettings(new Settings()),
	_currentSlot(_userSettings.configSlots.end()),
	_pMidiIn(NULL),
	_pMidiOut(NULL),
	_bConnected(false),
	_lastHiHatMsgControl(_clock.now(),0x000004B0, 0)
{
	qRegisterMetaType<MidiMessage>();

	setupUi(this);
	setWindowTitle((boost::format("%s")%APPLICATION_NAME).str().c_str());

	_pProcessAssistant = new QProcess(this);

	// Setup std::cout redirection
	_streamBuffer.open(StreamSink(boost::bind(&MainWindow::toLog, this, _1)));
	std::cout.rdbuf(&_streamBuffer);
	connect(this, SIGNAL(sLog(const QString&)), textEditLog, SLOT(append(const QString&)));

	_pGrapSubWindow = new GraphSubWindow(&_userSettings, this);
	_pSettings->signalKitDefined.connect(boost::bind(&GraphSubWindow::onDrumKitLoaded, _pGrapSubWindow, _1, _2));
	connect(this, SIGNAL(hiHatStartMoving(int, int, int)), _pGrapSubWindow, SLOT(onHiHatStartMoving(int, int, int)));
	connect(this, SIGNAL(hiHatState(int)), _pGrapSubWindow, SLOT(onHiHatState(int)));
	connect(this, SIGNAL(footCancelStarted(int, int, int)), _pGrapSubWindow, SLOT(onFootCancelStarted(int, int, int)));
	connect(this, SIGNAL(updatePlot(const MidiMessage&)), _pGrapSubWindow, SLOT(onUpdatePlot(const MidiMessage&)));
	mdiArea->addSubWindow(_pGrapSubWindow);
	_pGrapSubWindow->showMaximized();

	// Buffer and Calibration offset
	lineEditCalibrationOffset->setToolTip("Rock Band calibration offset to apply on both video and audio settings.\nIt's an offset, you have to add this value to your existing settings");
	lineEditCalibrationOffset->setText("0");
	sliderBuffer->setToolTip("Modifying the midi buffer length (ms) affect your Rock Band calibration setting.\nSome features works better with a larger buffer length, a typical value is 35 ms");

	listWidgetSlots->setContextMenuPolicy(Qt::CustomContextMenu);

	// Loading last user settings
	loadUserSettings(_pSettings->getUserSettingsFile().generic_string());

	_calibrationOffset = _userSettings.bufferLength;

	// Building the default config if empty
	if (_userSettings.configSlots.empty())
	{
		const Slot::Ptr& pSlot = createDefaultSlot();
		_userSettings.configSlots.push_back(pSlot);
		QListWidgetItem* pNewItem = new QListWidgetItem(pSlot->getName().c_str());
		QVariant variant;
		variant.setValue(pSlot);
		pNewItem->setData(Qt::UserRole, variant);
		listWidgetSlots->addItem(pNewItem);
	}

	_currentSlot = _userSettings.configSlots.begin();

	selectLastSlot();

	const Slot::Ptr& pCurrentSlot = getCurrentSlot();
	const Pad::List& pads = pCurrentSlot->getPads();
	const HiHatPedalElement::Ptr& pElHihatPedal = boost::dynamic_pointer_cast<HiHatPedalElement>(pads[Pad::HIHAT_PEDAL]);

	std::vector<QColor> groupColors;
	groupColors.push_back(QColor(150, 150, 250));
	groupColors.push_back(QColor(150, 250, 150));
	groupColors.push_back(QColor(250, 150, 150));
	groupColors.push_back(QColor(250, 250, 150));
	groupColors.push_back(QColor(150, 250, 250));
	groupColors.push_back(QColor(250, 150, 250));
	groupColors.push_back(QColor(180, 180, 220));

	// Hi-hat
	{
		Parameter::Ptr pRoot(new Parameter());

		Parameter::Ptr pGroup1(new Parameter("Hi-hat secured positions", groupColors[0]));
		{
			pGroup1->addChild(Parameter::Ptr(new Parameter("Secured close position (unit)", 0, 127,
							pElHihatPedal->getSecurityPosition(),
							boost::bind(&HiHatPedalElement::setSecurityPosition, pElHihatPedal, _1),
							tr("Under this position the hi-hat is in close color").toStdString())));
			pGroup1->addChild(Parameter::Ptr(new Parameter("Bow hits always in close color",
							pElHihatPedal->isBowAlwaysYellow(),
							boost::bind(&HiHatPedalElement::setBowAlwaysYellow, pElHihatPedal, _1),
							tr("If checked, bow hits are always in close color").toStdString())));
		}

		Parameter::Ptr pGroup2(new Parameter("Hi-hat open color on edge accent", groupColors[1],
					pElHihatPedal->isBlueDetectionByAccent(),
					boost::bind(&HiHatPedalElement::setBlueDetectionByAccent, pElHihatPedal, _1)));
		{
			LinearFunction::Description::Ptr pDescription(new LinearFunction::Description());
			pDescription->szLabelX = tr("Hi-hat position (unit)").toStdString();
			pDescription->szLabelY = tr("Velocity (unit)").toStdString();
			pGroup2->addChild(Parameter::Ptr(new Parameter("Parameters",
							pDescription,
							pElHihatPedal->getBlueAccentFunctions(),
							boost::bind(&HiHatPedalElement::setBlueAccentFunctions, pElHihatPedal, _1),
							tr("List of linear functions used to determine when to convert an accented hi-hat note to open color").toStdString())));

			pGroup2->addChild(Parameter::Ptr(new Parameter("Override secured position",
							pElHihatPedal->isBlueAccentOverride(),
							boost::bind(&HiHatPedalElement::setBlueAccentOverride, pElHihatPedal, _1),
							tr("If checked, edge accented hits are converted to open color even under the secured close position").toStdString())));
		}

		Parameter::Ptr pGroup3(new Parameter("Hi-hat open color detection by position", groupColors[2],
					pElHihatPedal->isBlueDetectionByPosition(),
					boost::bind(&HiHatPedalElement::setBlueDetectionByPosition, pElHihatPedal, _1)));
		{
			pGroup3->addChild(Parameter::Ptr(new Parameter("Control Position (unit)", 0, 127,
							pElHihatPedal->getControlPosThreshold(),
							boost::bind(&HiHatPedalElement::setControlPosThreshold, pElHihatPedal, _1),
							tr("Above this position the hi-hat is converted to open color").toStdString())));
			pGroup3->addChild(Parameter::Ptr(new Parameter("Delay time (ms)", 0, 5000,
							pElHihatPedal->getControlPosDelayTime(),
							boost::bind(&HiHatPedalElement::setControlPosDelayTime, pElHihatPedal, _1),
							tr("Delay time before switching to open color if the control position is above the threshold").toStdString())));
		}

		Parameter::Ptr pGroup4(new Parameter("Hi-hat open color detection by speed", groupColors[3],
					pElHihatPedal->isBlueDetectionBySpeed(),
					boost::bind(&HiHatPedalElement::setBlueDetectionBySpeed, pElHihatPedal, _1)));
		{
			Parameter::Ptr pOpenSpeed(new Parameter("Open speed (unit/s)", 0, 5000,
						pElHihatPedal->getOpenSpeed(),
						boost::bind(&HiHatPedalElement::setOpenSpeed, pElHihatPedal, _1),
						tr("Above this speed the hi-hat is converted to open color").toStdString()));
			pOpenSpeed->setInfiniteExtremities(Parameter::InfiniteExtremities(false, true));

			pGroup4->addChild(pOpenSpeed);

			pGroup4->addChild(Parameter::Ptr(new Parameter("Close speed (unit/s)", -5000, 0,
							pElHihatPedal->getCloseSpeed(),
							boost::bind(&HiHatPedalElement::setCloseSpeed, pElHihatPedal, _1),
							tr("Under this speed the hi-hat is converted to close color").toStdString())));
		}

		Parameter::Ptr pGroup5(new Parameter("Hi-hat half-open mode", groupColors[4],
					pElHihatPedal->isHalfOpenModeEnabled(),
					boost::bind(&HiHatPedalElement::setHalfOpenModeEnabled, pElHihatPedal, _1)));
		{

			pGroup5->addChild(Parameter::Ptr(new Parameter("Half open maximum position (unit)", 0, 127,
							pElHihatPedal->getHalfOpenMaximumPosition(),
							boost::bind(&HiHatPedalElement::setHalfOpenMaximumPosition, pElHihatPedal, _1),
							tr("Half open detection algorithm starts between [Security close position] and this position").toStdString())));

			pGroup5->addChild(Parameter::Ptr(new Parameter("Half open detection time (ms)", 0, 5000,
							pElHihatPedal->getHalfOpenActivationTime(),
							boost::bind(&HiHatPedalElement::setHalfOpenActivationTime, pElHihatPedal, _1),
							tr("Afters the specified time (ms), if the hi-hat is still in close color, it goes in half open mode. It will leave this mode if the control position go back under [Security close position]").toStdString())));
		}

		Parameter::Ptr pGroup6(new Parameter("Chameleon Crash conversion", groupColors[5]));
		{
			const std::string& szDescription = tr("A Chameleon Crash change its crash color to hi-hat close color if one of these pads is hit at the same time").toStdString();
			pGroup6->setDescription(szDescription);

			pGroup6->addChild(Parameter::Ptr(new Parameter("Time window (ms)", 0, 100,
							pCurrentSlot->getCymbalSimHitWindow(),
							boost::bind(&Slot::setCymbalSimHitWindow, pCurrentSlot, _1),
							tr("Timing window used to detect simultaneous hits between cymbals").toStdString())));

			pGroup6->addChild(Parameter::Ptr(new Parameter("Crash",
							pCurrentSlot->isAutoConvertCrash(Slot::CRASH_CRASH),
							boost::bind(&Slot::setAutoConvertCrash, pCurrentSlot, Slot::CRASH_CRASH, _1), szDescription)));

			pGroup6->addChild(Parameter::Ptr(new Parameter("Ride",
							pCurrentSlot->isAutoConvertCrash(Slot::CRASH_RIDE),
							boost::bind(&Slot::setAutoConvertCrash, pCurrentSlot, Slot::CRASH_RIDE, _1), szDescription)));

			pGroup6->addChild(Parameter::Ptr(new Parameter("Snare",
							pCurrentSlot->isAutoConvertCrash(Slot::CRASH_SNARE),
							boost::bind(&Slot::setAutoConvertCrash, pCurrentSlot, Slot::CRASH_SNARE, _1), szDescription)));

			pGroup6->addChild(Parameter::Ptr(new Parameter("Tom 2",
							pCurrentSlot->isAutoConvertCrash(Slot::CRASH_TOM2),
							boost::bind(&Slot::setAutoConvertCrash, pCurrentSlot, Slot::CRASH_TOM2, _1), szDescription)));

			pGroup6->addChild(Parameter::Ptr(new Parameter("Tom 3",
							pCurrentSlot->isAutoConvertCrash(Slot::CRASH_TOM3),
							boost::bind(&Slot::setAutoConvertCrash, pCurrentSlot, Slot::CRASH_TOM3, _1), szDescription)));
		}

		pRoot->addChild(pGroup1);
		pRoot->addChild(pGroup2);
		pRoot->addChild(pGroup3);
		pRoot->addChild(pGroup4);
		pRoot->addChild(pGroup5);
		pRoot->addChild(pGroup6);
		gridLayoutHiHat->addWidget(new TreeViewParameters(this, pRoot), 0,0);
	}

	// Foot splash cancel
	{
		Parameter::Ptr pRoot(new Parameter());
		Parameter::Ptr pGroup1(new Parameter("Foot splash cancel from control speed", groupColors[0],
					pElHihatPedal->isFootCancel(),
					boost::bind(&HiHatPedalElement::setFootCancel, pElHihatPedal, _1),
					tr("A hi-hat mask window is created if conditions below are met").toStdString()));
		pElHihatPedal->connectFootCancelActivated(boost::bind(&GraphSubWindow::onFootCancelActivated, _pGrapSubWindow, _1));
		{
			pGroup1->addChild(Parameter::Ptr(new Parameter("Control speed (unit/s)", HiHatPedalElement::MIN_FOOT_SPEED, 0,
							pElHihatPedal->getFootCancelClosingSpeed(),
							boost::bind(&HiHatPedalElement::setFootCancelClosingSpeed, pElHihatPedal, _1),
							tr("Under this speed the mask window can start if other conditions are met").toStdString())));
			pGroup1->addChild(Parameter::Ptr(new Parameter("Maximum hi-hat position (unit)", 0, 127,
							pElHihatPedal->getFootCancelPos(),
							boost::bind(&HiHatPedalElement::setFootCancelPos, pElHihatPedal, _1),
							tr("Under this position the mask window can start if other conditions are met").toStdString())));
			pGroup1->addChild(Parameter::Ptr(new Parameter("Control Position delta (unit)", 0, 127,
							pElHihatPedal->getFootCancelPosDiff(),
							boost::bind(&HiHatPedalElement::setFootCancelPosDiff, pElHihatPedal, _1),
							tr("The delta must be greater than the entered value to activate the mask window.\nThe delta position is between the beginning of the close movement (the green point on the curve) and the current position.").toStdString())));
			pGroup1->addChild(Parameter::Ptr(new Parameter("Mask Time (ms)", 0, 200,
							pElHihatPedal->getFootCancelMaskTime(),
							boost::bind(&HiHatPedalElement::setFootCancelMaskTime, pElHihatPedal, _1),
							tr("Time length of the mask window (ms)").toStdString())));
			pElHihatPedal->connectFootCancelMaskTime(boost::bind(&GraphSubWindow::onFootCancelMaskTime, _pGrapSubWindow, _1));

			pGroup1->addChild(Parameter::Ptr(new Parameter("Velocity to ignore (unit)", 0, 127,
							pElHihatPedal->getFootCancelVelocity(),
							boost::bind(&HiHatPedalElement::setFootCancelVelocity, pElHihatPedal, _1),
							tr("Height of the mask window. All hi-hat hits under this velocity are ignored").toStdString())));
			pElHihatPedal->connectFootCancelVelocity(boost::bind(&GraphSubWindow::onFootCancelVelocity, _pGrapSubWindow, _1));
		}

		Parameter::Ptr pGroup2(new Parameter("Foot splash cancel after a pedal hit", groupColors[1],
					pElHihatPedal->isFootCancelAfterPedalHit(),
					boost::bind(&HiHatPedalElement::setFootCancelAfterPedalHit, pElHihatPedal, _1),
					tr("A hi-hat mask window is created when a hi-hat pedal hit is detected").toStdString()));

		pElHihatPedal->connectFootCancelAfterPedalHitActivated(boost::bind(&GraphSubWindow::onFootCancelAfterPedalHitActivated, _pGrapSubWindow, _1));
		{
			pGroup2->addChild(Parameter::Ptr(new Parameter("Mask time (ms)", 0, 200,
							pElHihatPedal->getFootCancelAfterPedalHitMaskTime(),
							boost::bind(&HiHatPedalElement::setFootCancelAfterPedalHitMaskTime, pElHihatPedal, _1),
							tr("Time length of the mask window (ms)").toStdString())));
			pElHihatPedal->connectFootCancelAfterPedalHitMaskTime(boost::bind(&GraphSubWindow::onFootCancelAfterPedalHitMaskTime, _pGrapSubWindow, _1));

			pGroup2->addChild(Parameter::Ptr(new Parameter("Velocity to ignore (unit)", 0, 127,
							pElHihatPedal->getFootCancelAfterPedalHitVelocity(),
							boost::bind(&HiHatPedalElement::setFootCancelAfterPedalHitVelocity, pElHihatPedal, _1),
							tr("Height of the mask window. All hi-hat hits under this velocity are ignored").toStdString())));
			pElHihatPedal->connectFootCancelAfterPedalHitVelocity(boost::bind(&GraphSubWindow::onFootCancelAfterPedalHitVelocity, _pGrapSubWindow, _1));
		}

		Parameter::Ptr pGroup3(new Parameter("Cancel while open", groupColors[2],
					pElHihatPedal->isCancelHitWhileOpen(),
					boost::bind(&HiHatPedalElement::setCancelHitWhileOpen, pElHihatPedal, _1)));
		{
			pGroup3->addChild(Parameter::Ptr(new Parameter("Control Position (unit)", 0, 127,
							pElHihatPedal->getCancelOpenHitThreshold(),
							boost::bind(&HiHatPedalElement::setCancelOpenHitThreshold, pElHihatPedal, _1),
							tr("If the current control pos is >= [Position] and hi-hat hit is < [Velocity] the hi-hat hit is ignored").toStdString())));
			pGroup3->addChild(Parameter::Ptr(new Parameter("Velocity (unit)", 0, 127,
							pElHihatPedal->getCancelOpenHitVelocity(),
							boost::bind(&HiHatPedalElement::setCancelOpenHitVelocity, pElHihatPedal, _1),
							tr("If the current control pos is >= [Position] and hi-hat hit is < [Velocity] the hi-hat hit is ignored").toStdString())));
		}
		pRoot->addChild(pGroup1);
		pRoot->addChild(pGroup2);
		pRoot->addChild(pGroup3);
		gridLayoutFootSplashCancel->addWidget(new TreeViewParameters(this, pRoot), 0,0);
	}

	// Flams
	{
		Parameter::Ptr pRoot(new Parameter());
		{
			Pad::List::const_iterator it = pads.begin();
			while (it!=pads.end())
			{
				const Pad::Ptr& pPad = *(it++);
				if (pPad->getType()==Pad::HIHAT_PEDAL)
				{
					continue;
				}
				Parameter::Ptr pGroup1(new Parameter(pPad->getName(), QColor(pPad->getColor().c_str()),
							pPad->isFlamActivated(),
							boost::bind(&Pad::setFlamActivated, pPad, _1)));

				pGroup1->addChild(Parameter::Ptr(new Parameter("Pad Type of the second hit", 0, Pad::TYPE_COUNT,
								pPad->getTypeFlam(),
								boost::bind(&Pad::setTypeFlam, pPad, _1),
								tr("Specify the Pad type of the second hit of the flam").toStdString(),
								Pad::DICT_NAMES)));

				LinearFunction::Description::Ptr pDescription(new LinearFunction::Description());
				pDescription->szLabelX = tr("Time between 2 hits (ms)").toStdString();
				pDescription->szLabelY = tr("Velocity factor").toStdString();
				pDescription->xMin = 0;
				pDescription->xMax = 150;
				pDescription->yMin = 0;
				pDescription->yMax = 2;
				pDescription->x1Default = 0;
				pDescription->x2Default = 30;
				pDescription->y1Default = 1.0f;
				pDescription->y2Default = 1.0f;
				pDescription->yStep = 0.01f;
				pDescription->aStep = 0.001f;
				pDescription->bStep = 0.01f;
				pDescription->aDecimals = 3;
				pGroup1->addChild(Parameter::Ptr(new Parameter("Parameters",
								pDescription,
								pPad->getFlamFunctions(),
								boost::bind(&Pad::setFlamFunctions, pPad, _1),
								tr("List of linear functions used to determine when to convert a flam in 2 different colors").toStdString())));

				pGroup1->addChild(Parameter::Ptr(new Parameter("Flam cancel detection (ms)", 0, 250,
								pPad->getFlamCancelDuringRoll(),
								boost::bind(&Pad::setFlamCancelDuringRoll, pPad, _1),
								tr("If the previous hit before the flam is under this value the flam is cancelled").toStdString())));

				pRoot->addChild(pGroup1);
			}
		}
		gridLayoutFlams->addWidget(new TreeViewParameters(this, pRoot), 0,0);
	}


	// Ghost notes
	{
		Parameter::Ptr pRoot(new Parameter());
		Parameter::Ptr pGroup1(new Parameter("Ghost notes", groupColors[0]));
		{
			Pad::List::const_iterator it = pads.begin();
			while (it!=pads.end())
			{
				const Pad::Ptr& pPad = *(it++);
				Parameter::Ptr pParameter(new Parameter(pPad->getName(), 0, 127,
							pPad->getGhostVelocityLimit(),
							boost::bind(&Pad::setGhostVelocityLimit, pPad, _1),
							tr("Under this velocity the note is ignored").toStdString()));
				pParameter->setColor(QColor(pPad->getColor().c_str()));
				pGroup1->addChild(pParameter);
			}
		}
		pRoot->addChild(pGroup1);
		gridLayoutGhostNotes->addWidget(new TreeViewParameters(this, pRoot), 0,0);
	}

	// Filling midi devices
	Pm_Initialize();
	int comboBoxIndexMidiIn = -1;
	int comboBoxIndexMidiOut = -1;
	const std::string& szMidiIn = _pSettings->getMidiIn();
	const std::string& szMidiOut = _pSettings->getMidiOut();
	int nbDevices = Pm_CountDevices();
	for (int deviceId=0; deviceId<nbDevices; ++deviceId)
	{
		const PmDeviceInfo* pDeviceInfo = Pm_GetDeviceInfo(deviceId);
		if (pDeviceInfo)
		{
			if (pDeviceInfo->input)
			{
				comboBoxMidiIn->blockSignals(true);
				comboBoxMidiIn->addItem(pDeviceInfo->name, deviceId);
				comboBoxMidiIn->blockSignals(false);

				if (szMidiIn == pDeviceInfo->name)
				{
					comboBoxIndexMidiIn = comboBoxMidiIn->count()-1;
				}
			}

			if (pDeviceInfo->output)
			{
				comboBoxMidiOut->blockSignals(true);
				comboBoxMidiOut->addItem(pDeviceInfo->name, deviceId);
				comboBoxMidiOut->blockSignals(false);
				if (szMidiOut == pDeviceInfo->name)
				{
					comboBoxIndexMidiOut = comboBoxMidiOut->count()-1;
				}
			}
		}
	}
	Pm_Terminate();

	// Select midi in
	if (comboBoxIndexMidiIn>=0)
	{
		comboBoxMidiIn->setCurrentIndex(comboBoxIndexMidiIn);
	}

	// Select midi out
	if (comboBoxIndexMidiOut>=0)
	{
		comboBoxMidiOut->setCurrentIndex(comboBoxIndexMidiOut);
	}

	// Enable/Disable Stop/Start buttons
	pushButtonStop->setEnabled(false);
	pushButtonStart->setEnabled(false);
	if (comboBoxMidiIn->count() && comboBoxMidiOut->count())
	{
		pushButtonStart->setEnabled(true);
	}

	// Logs
	groupBoxLogs->setChecked(true);
	checkBoxLogsRawData->setChecked(false);
	checkBoxLogsFilteredData->setChecked(false);
	checkBoxLogsHiHatControl->setChecked(false);
	checkBoxLogsOthers->setChecked(true);

	// Run the midi thread
	if (comboBoxIndexMidiIn >=0 && comboBoxIndexMidiOut >=0)
	{
		on_pushButtonStart_clicked();
	}

	// Settings connections
	_pSettings->signalRedrawPeriodChanged.connect(boost::bind(&GraphSubWindow::onRedrawPeriodChanged, _pGrapSubWindow, _1));
	_pSettings->signalCurveWindowLengthChanged.connect(boost::bind(&GraphSubWindow::onCurveWindowLengthChanged, _pGrapSubWindow, _1));
	_pGrapSubWindow->onRedrawPeriodChanged(_pSettings->getRedrawPeriod());
	_pGrapSubWindow->onCurveWindowLengthChanged(_pSettings->getCurveWindowLength());
}

MainWindow::~MainWindow()
{
	stop();

	// Restoring the std::cout streambuf
	std::cout.rdbuf(_pOldStreambuf);
}

void MainWindow::sendMidiMessage(const MidiMessage& midiMessage, bool bForce)
{
	Mutex::scoped_lock lock(_mutex);

	if (!midiMessage.isIgnored() || bForce)
	{
		const int DEFAULT_NOTE_MSG_CTRL(4);
		PmError error = Pm_WriteShort(_pMidiOut, midiMessage.getTimestamp(), midiMessage.computeOutputMessage());
		if (error==pmNoError)
		{
			if (midiMessage.isControllerMsg() && midiMessage.getOutputNote() == DEFAULT_NOTE_MSG_CTRL)
			{
				// TODO
				emit hiHatPedalControl(127-midiMessage.getValue());
			}
			else
			{
				// Note on message = 9
				// Sending a fake hh control to the plotter
				if (_lastHiHatMsgControl.isControllerMsg() && _lastHiHatMsgControl.getOutputNote() == DEFAULT_NOTE_MSG_CTRL)
				{
					// Update the timestamp of the last CC#4 to the same timestamp of the current note
					_lastHiHatMsgControl.setTimestamp(midiMessage.getTimestamp());
					const MidiMessage& m = _lastHiHatMsgControl;
					emit updatePlot(m);
				}
			}

			emit updatePlot(midiMessage);
		}
		else
		{
			if (_userSettings.isLogs() && _userSettings.isLog(UserSettings::LOG_OTHERS))
			{
				std::cout << "Error can't send midi message : " << midiMessage.str() << " Reason=" << Pm_GetErrorText(error) <<  std::endl;
			}
		}
	}
}

void MainWindow::sendMidiMessages(const MidiMessage::List& midiMessages, bool bForce)
{
	Mutex::scoped_lock lock(_mutex);

	MidiMessage::List::const_iterator it = midiMessages.begin();
	while (it!=midiMessages.end())
	{
		const MidiMessage& msg = *(it++);
		sendMidiMessage(msg, bForce);
	}
}

void MainWindow::addIncomingMidiMessage(const MidiMessage& midiMsg)
{
	MidiMessage midiMessage(midiMsg);

	// Additionnal filtering
	bool bAddThisMidiMessage = false;
	if (midiMessage.isNoteOnMsg())
	{
		emit midiNoteOn(midiMessage.getOriginalNote(), midiMessage.getValue());

		// Filtering, we only process notes defined in the drumkit, we also change the default output note
		const Slot::Ptr& pCurrentSlot = getCurrentSlot();
		const Pad::List& pads = pCurrentSlot->getPads();
		Pad::List::const_iterator it = pads.begin();
		while (it!=pads.end())
		{
			const Pad::Ptr& pPad = *(it++);
			if (pPad->isA(midiMessage.getOriginalNote()))
			{
				midiMessage.changeNoteTo(pPad.get(), false);
				midiMessage.padType = pPad->getType();
				bAddThisMidiMessage = true;
				break;
			}
		}
	}
	else if (midiMessage.isControllerMsg() && midiMessage.getOriginalNote() == _pSettings->getDrumKitMidiMap()->getHiHatControlCC())
	{
		midiMessage.changeNoteTo(4, false);
		bAddThisMidiMessage = true;
	}

	if (bAddThisMidiMessage)
	{
		if (_userSettings.isLogs() && (_userSettings.isLog(UserSettings::LOG_FILTERED_DATA) || _userSettings.isLog(UserSettings::LOG_RAW_DATA)))
		{
			midiMessage.print();
		}
		_midiMessages.push_back(midiMessage);
	}
	else
	{
		// Note ignored but we display it in the logs
		if (_userSettings.isLogs() && _userSettings.isLog(UserSettings::LOG_RAW_DATA))
		{
			midiMessage.print();
		}
	}
}

void MainWindow::midiThread()
{
	MidiMessage::DictHistory lastMsgSent(Pad::TYPE_COUNT);

	while (true)
	{
		// Data to protect between midiInProc, midiThread and Qt:
		//	-	_userSettings
		//	-	_midiMessages
		//	-	_clock
		//	-	_currentSlot
		//	-	_lastHiHatMsgControl
		//	-	Slot (thread safe)
		//	-	Pad (thread safe)
		Mutex::scoped_lock lock(_mutex);
		_condition.wait(lock);

		while (!_midiMessages.empty())
		{
			boost::this_thread::interruption_point();

			// Get the current message
			MidiMessage currentMsg(_midiMessages.front());

			// Buffering here
			boost::chrono::milliseconds elapsed(boost::chrono::duration_cast<boost::chrono::milliseconds>(_clock.now() - currentMsg.getReceiveTime()));
			int waiting(_userSettings.bufferLength - elapsed.count());
			if (waiting>0)
			{
				lock.unlock();
				boost::this_thread::sleep(boost::posix_time::milliseconds(waiting));
				lock.lock();
			}

			// and removing it immediatly from the stack
			_midiMessages.pop_front();

			// We have to check at least if a slot is selected
			if (_currentSlot == _userSettings.configSlots.end())
			{
				// No slot selected we just forward the message
				sendMidiMessage(currentMsg);
				continue;
			}

			// Hi hat pedal controller msg
			const int DEFAULT_NOTE_MSG_CTRL(4);
			if (currentMsg.isControllerMsg() && currentMsg.getOutputNote() == DEFAULT_NOTE_MSG_CTRL && !currentMsg.isAlreadyModified())
			{
				currentMsg.hiHatSpeed = 0.0f;
				const HiHatPedalElement::Ptr& p = boost::dynamic_pointer_cast<HiHatPedalElement>(getCurrentSlot()->getPads()[Pad::HIHAT_PEDAL]);
				p->setCurrentControlPos(127-currentMsg.getValue());

				// Compute the instant speed in unit/s
				if (_lastHiHatMsgControl.isControllerMsg() && _lastHiHatMsgControl.getOutputNote()==DEFAULT_NOTE_MSG_CTRL)
				{
					int deltaPosition = _lastHiHatMsgControl.getValue() - currentMsg.getValue();
					float deltaTimeInS = float(currentMsg.getTimestamp()-_lastHiHatMsgControl.getTimestamp())/1000;

					// Speed computation
					const float MAX_FLOAT(std::numeric_limits<float>::max());
					const float MIN_FLOAT(std::numeric_limits<float>::min());
					if (deltaPosition!=0 && deltaTimeInS==0.0f)
					{
						currentMsg.hiHatSpeed = (deltaPosition>0)?MAX_FLOAT:MIN_FLOAT;
					}
					else
					{
						currentMsg.hiHatSpeed = float(deltaPosition)/deltaTimeInS;
					}

					// Acceleration computation
					int deltaSpeed = currentMsg.hiHatSpeed - p->getCurrentControlSpeed();
					if (deltaSpeed!=0 && deltaTimeInS==0.0f)
					{
						currentMsg.hiHatAcceleration = (deltaSpeed>0)?MAX_FLOAT:MIN_FLOAT;
					}
					else
					{
						currentMsg.hiHatAcceleration = float(deltaSpeed)/deltaTimeInS;
					}
					p->setCurrentControlAcceleration(currentMsg.hiHatAcceleration);

					// Current speed
					HiHatPedalElement::MovingState movingState = p->setCurrentControlSpeed(currentMsg.hiHatSpeed);
					switch (movingState)
					{
					case HiHatPedalElement::MS_START_CLOSE:
						{
							emit hiHatStartMoving(movingState, p->getPositionOnCloseBegin(), _lastHiHatMsgControl.getTimestamp());
							break;
						}
					case HiHatPedalElement::MS_START_OPEN:
						{
							emit hiHatStartMoving(movingState, p->getPositionOnOpenBegin(), _lastHiHatMsgControl.getTimestamp());
							break;
						}
					default:
						{
							break;
						}
					}
				}
			}

			// Compute midi message
			computeMessage(currentMsg, lastMsgSent);

			// Send the message
			sendMidiMessage(currentMsg);

			// Saving the sended message
			if (currentMsg.isNoteOnMsg())
			{
				Pad::List::iterator it = getCurrentSlot()->getPads().begin();
				while (it!=getCurrentSlot()->getPads().end())
				{
					Pad::List::value_type& p = *(it++);
					if (p->isA(currentMsg.getOriginalNote()))
					{
						MidiMessage::History& rHistory = lastMsgSent[p->getType()];
						rHistory.push_front(currentMsg);
						// We save the last 5 notes for this element
						rHistory.resize(5);
						break;
					}
				}
			}
			else if (currentMsg.isControllerMsg() && currentMsg.getOutputNote() == DEFAULT_NOTE_MSG_CTRL)
			{
				_lastHiHatMsgControl = currentMsg;
				_lastHiHatMsgControl.hiHatSpeed = 0;
				_lastHiHatMsgControl.hiHatAcceleration = 0;
			}
		}
	}
}

void MainWindow::on_pushButtonStart_clicked(bool)
{
	Pt_Start(1, &processMidi, this); // start a timer with millisecond accuracy

	Pm_Initialize();

#define INPUT_BUFFER_SIZE 0 // if INPUT_BUFFER_SIZE is 0, PortMidi uses a default value
#define OUTPUT_BUFFER_SIZE 100
#define DRIVER_INFO NULL
#define TIME_PROC NULL
#define TIME_INFO NULL
#define LATENCY 0 // use zero latency because we want output to be immediate

	_bConnected = false;
	PmError error = pmNoError;

	// Clearing plots
	_pGrapSubWindow->clearPlots();
	_pGrapSubWindow->replot();

	// Open Midi in device
	int midiInId = comboBoxMidiIn->itemData(comboBoxMidiIn->currentIndex()).toInt();
	const PmDeviceInfo* pDeviceInInfo = Pm_GetDeviceInfo(midiInId);
	error = Pm_OpenInput(	&_pMidiIn, 
			midiInId, 
			DRIVER_INFO,
			INPUT_BUFFER_SIZE,
			TIME_PROC,
			TIME_INFO);
	if (error!=pmNoError)
	{
		QMessageBox::critical(this, tr("Midi Error"), tr("Cannot open midi in %1, reason : %2").arg(pDeviceInInfo->name).arg(Pm_GetErrorText(error)));
		return;
	}
	else
	{
		// Ignoring sysex, tick, song position etc...
		Pm_SetFilter(_pMidiIn, PM_FILT_REALTIME | PM_FILT_SYSTEMCOMMON);
		_pSettings->setMidiIn(comboBoxMidiIn->currentText().toStdString());
	}

	// Open Midi out device
	int midiOutId = comboBoxMidiOut->itemData(comboBoxMidiOut->currentIndex()).toInt();
	const PmDeviceInfo* pDeviceOutInfo = Pm_GetDeviceInfo(midiOutId);
	error = Pm_OpenOutput(	&_pMidiOut, 
			midiOutId, 
			DRIVER_INFO,
			OUTPUT_BUFFER_SIZE,
			TIME_PROC,
			TIME_INFO,
			LATENCY);

	if (error!=pmNoError)
	{
		QMessageBox::critical(this, tr("Midi Error"), tr("Cannot open midi out %1, reason : %2").arg(pDeviceOutInfo->name).arg(Pm_GetErrorText(error)));
		return;
	}
	else
	{
		_pSettings->setMidiOut(comboBoxMidiOut->currentText().toStdString());
	}

	// Thread already started, stopping it?
	if (_pMidiThread.get())
	{
		if (_userSettings.isLogs() && _userSettings.isLog(UserSettings::LOG_OTHERS))
		{
			std::cout << "Closing previous Midi thread ..." << std::endl; 
		}

		_pMidiThread->interrupt();

		if (_userSettings.isLogs() && _userSettings.isLog(UserSettings::LOG_OTHERS))
		{
			std::cout << "Previous Midi thread closed" << std::endl;
		}
	}

	_bConnected = true;
	_pMidiThread.reset(new boost::thread(boost::bind(&MainWindow::midiThread, this)));
	if (_userSettings.isLogs() && _userSettings.isLog(UserSettings::LOG_OTHERS))
	{
		std::cout << "New Midi thread started" << std::endl;
	}

	comboBoxMidiIn->setEnabled(false);
	comboBoxMidiOut->setEnabled(false);
	pushButtonStart->setEnabled(false);
	pushButtonStop->setEnabled(true);
}

void MainWindow::stop()
{
	// Exiting midi thread first
	if (_userSettings.isLogs() && _userSettings.isLog(UserSettings::LOG_OTHERS))
	{
		std::cout << "Closing Midi thread ..." << std::endl;
	}

	if (_pMidiThread)
	{
		_pMidiThread->interrupt();
		if (_userSettings.isLogs() && _userSettings.isLog(UserSettings::LOG_OTHERS))
		{
			std::cout << "Midi thread closed" << std::endl;
		}
	}

	Pt_Stop();

	if (_pMidiOut)
	{
		Pm_Close(_pMidiOut);
		_pMidiOut = NULL;
	}

	if (_pMidiIn)
	{
		Pm_Close(_pMidiIn);
		_pMidiIn = NULL;
	}

	_bConnected = false;

	Pm_Terminate();
}

void MainWindow::on_pushButtonStop_clicked(bool)
{
	stop();

	comboBoxMidiIn->setEnabled(true);
	comboBoxMidiOut->setEnabled(true);
	pushButtonStart->setEnabled(true);
	pushButtonStop->setEnabled(false);
}

void MainWindow::on_comboBoxMidiIn_currentIndexChanged(const QString& text)
{
	_pSettings->setMidiIn(text.toStdString());
}

void MainWindow::on_comboBoxMidiOut_currentIndexChanged(const QString& text)
{
	_pSettings->setMidiOut(text.toStdString());
}

void MainWindow::on_actionSave_As_triggered()
{
	fs::path pathDefaultConfig(_pSettings->getUserSettingsFile());
	QString qszDefaultDir(".\\default.epd");
	if (pathDefaultConfig.has_parent_path())
	{
		qszDefaultDir = pathDefaultConfig.parent_path().generic_string().c_str();
	}

	QFileDialog fileDlg(this, tr("Save configuration file"), qszDefaultDir, "Rock Band Pro Drums (*.epd)");
	fileDlg.setDefaultSuffix("epd");
	fileDlg.setAcceptMode(QFileDialog::AcceptSave);
	if (fileDlg.exec())
	{
		const QStringList& files = fileDlg.selectedFiles();
		saveUserSettings(files[0].toStdString());
	}
}

void MainWindow::selectLastSlot()
{
	// Select the slot
	if (listWidgetSlots->count())
	{
		int index = _pSettings->getLastSelectedSlotIndex();
		if (index<listWidgetSlots->count())
		{
			listWidgetSlots->item(index)->setSelected(true);
		}
		else
		{
			listWidgetSlots->item(0)->setSelected(true);
		}
	}
}

void MainWindow::on_actionOpen_triggered()
{
	fs::path pathDefaultConfig(_pSettings->getUserSettingsFile());
	QString qszDefaultDir(".\\");
	if (pathDefaultConfig.has_parent_path() && fs::exists(pathDefaultConfig.parent_path()))
	{
		qszDefaultDir = pathDefaultConfig.parent_path().generic_string().c_str();
	}

	QString filePath = QFileDialog::getOpenFileName(this, "Open configuration file", qszDefaultDir, "eProDrums Config File (*.epd)");
	if (filePath.isNull())
	{
		return;
	}

	loadUserSettings(filePath.toStdString());
	selectLastSlot();
}

void MainWindow::loadUserSettings(const std::string& szFilePath)
{
	if (!fs::exists(szFilePath))
	{
		return;
	}

	Mutex::scoped_lock lock(_mutex);

	try
	{
		fs::path pathConfig(szFilePath);
		std::ifstream ifs(pathConfig.generic_string().c_str());
		if (ifs.good())
		{
			boost::archive::xml_iarchive ia(ifs);
			ia >> BOOST_SERIALIZATION_NVP(_userSettings);
			_userSettings.filePath = pathConfig.generic_string();

			listWidgetSlots->clear();

			// Setting the mutex
			Slot::List::iterator it  = _userSettings.configSlots.begin();
			while (it!=_userSettings.configSlots.end())
			{
				const Slot::Ptr& pSlot = *(it++);

				// Connection between global settings and the slot
				_pSettings->signalKitDefined.connect(boost::bind(&Slot::onDrumKitLoaded, pSlot, _1, _2));

				// Add the item in the list widget
				QListWidgetItem* pNewItem = new QListWidgetItem(pSlot->getName().c_str());
				QVariant variant;
				variant.setValue(pSlot);
				pNewItem->setData(Qt::UserRole, variant);
				listWidgetSlots->addItem(pNewItem);

				// The "default" slot name can't be changed
				if (pSlot->getName() != "default")
				{
					pNewItem->setFlags(pNewItem->flags() | Qt::ItemIsEditable);
				}
			}

			// Global settings and all slots are connected, we can reload/load the drum kit map
			_pSettings->reloadDrumKitMidiMap();

			_pSettings->setUserSettingsFile(pathConfig.generic_string());
			setWindowTitle((boost::format("%s - [%s]")%APPLICATION_NAME%pathConfig.filename()).str().c_str());
			spinBoxBuffer->setValue(_userSettings.bufferLength);

			// Set curve visibility
			_pGrapSubWindow->loadCurveVisibility();
		}
	}
	catch (const std::exception& e)
	{
		boost::format fmtMsg("Error while loading configuration. Reason : %s");
		fmtMsg%e.what();
		QMessageBox::critical(this,"Error while loading configuration", fmtMsg.str().c_str());
		_userSettings.configSlots.clear();
		_pSettings->setUserSettingsFile("./");
	}
}

void MainWindow::saveUserSettings(const std::string& szFilePath)
{
	try
	{
		_pGrapSubWindow->saveCurveVisibility();

		fs::path pathConfig(szFilePath);
		std::ofstream ofs(pathConfig.generic_string().c_str());
		boost::archive::xml_oarchive oa(ofs);
		oa << BOOST_SERIALIZATION_NVP(_userSettings);
		_userSettings.filePath = pathConfig.generic_string();
		_pSettings->setUserSettingsFile(pathConfig.generic_string());
		setWindowTitle((boost::format("%s - [%s]")%APPLICATION_NAME%pathConfig.filename()).str().c_str());
	}
	catch (const std::exception& e)
	{
		boost::format fmtMsg("Error while saving configuration. Reason : %s");
		fmtMsg%e.what();
		QMessageBox::critical(this,"Error while loading configuration", fmtMsg.str().c_str());
	}
}

void MainWindow::on_actionSave_triggered()
{
	if (fs::exists(_userSettings.filePath) && !fs::is_directory(_userSettings.filePath))
	{
		saveUserSettings(_userSettings.filePath);
	}
	else
	{
		on_actionSave_As_triggered();
	}
}

void MainWindow::on_actionQuit_triggered()
{
	QApplication::exit();
}

void MainWindow::on_sliderBuffer_valueChanged(int value)
{
	spinBoxBuffer->setValue(value);
}

void MainWindow::on_spinBoxBuffer_valueChanged(int value)
{
	sliderBuffer->blockSignals(true);
	sliderBuffer->setValue(value);
	sliderBuffer->blockSignals(false);

	Mutex::scoped_lock lock(_mutex);
	_userSettings.bufferLength = value;
	_calibrationOffset = _userSettings.bufferLength;
	lineEditCalibrationOffset->setText((boost::format("%d")%_calibrationOffset).str().c_str());
}

void MainWindow::on_listWidgetSlots_customContextMenuRequested(const QPoint& point)
{
	QListWidgetItem* p = listWidgetSlots->itemAt(point);
	actionAdd_Slot->setEnabled(true);
	if (p && p->isSelected())
	{
		actionRemove_Slot->setEnabled(p->text() != "default");
	}
	else
	{
		actionRemove_Slot->setEnabled(false);
	}
	menuEdit->exec(QCursor::pos());
}

void MainWindow::on_actionAdd_Slot_triggered()
{
	Mutex::scoped_lock lock(_mutex);
	const Slot::Ptr& pSlot = createDefaultSlot();
	pSlot->setName(createNewSlotName());
	_userSettings.configSlots.push_back(pSlot);
	QListWidgetItem* pNewItem = new QListWidgetItem(pSlot->getName().c_str());
	QVariant variant;
	variant.setValue(pSlot);
	pNewItem->setData(Qt::UserRole, variant);
	listWidgetSlots->addItem(pNewItem);
	pNewItem->setFlags(pNewItem->flags() | Qt::ItemIsEditable);
	pNewItem->setSelected(true);
}

std::string MainWindow::createNewSlotName(const std::string& szBaseName) const
{
	Slot::List::const_iterator it = std::find_if(_userSettings.configSlots.begin(), _userSettings.configSlots.end(), boost::bind(&Slot::getName, _1) == szBaseName);
	if (it==_userSettings.configSlots.end())
	{
		return szBaseName;
	}

	boost::format fmtSlot("%s_%d");
	std::string szNewName("error");
	// Find new name
	for (size_t i=0; i<_userSettings.configSlots.size(); i++)
	{
		const std::string& szName = (fmtSlot%szBaseName%(i+1)).str();
		Slot::List::const_iterator it = std::find_if(_userSettings.configSlots.begin(), _userSettings.configSlots.end(), boost::bind(&Slot::getName, _1) == szName);
		if (it==_userSettings.configSlots.end())
		{
			szNewName = szName;
			break;
		}
	}
	return szNewName;
}

void MainWindow::on_actionDuplicate_Slot_triggered()
{
	Mutex::scoped_lock lock(_mutex);
	const QList<QListWidgetItem*>& selected = listWidgetSlots->selectedItems();
	if (!selected.empty())
	{
		QListWidgetItem* pSelected = selected[0];
		const std::string& szSlotName = pSelected->text().toStdString();
		Slot::List::iterator itSlot = std::find_if(_userSettings.configSlots.begin(), _userSettings.configSlots.end(), boost::bind(&Slot::getName, _1) == szSlotName);
		if (itSlot!=_userSettings.configSlots.end())
		{
			const Slot::Ptr& pSlotToDuplicate = *(itSlot);
			Slot::Ptr pDuplicated(new Slot(*pSlotToDuplicate));
			pDuplicated->setName(createNewSlotName(pSlotToDuplicate->getName()));

			// Add the new slot internally and on the GUI
			_userSettings.configSlots.push_back(pDuplicated);

			QListWidgetItem* pNewItem = new QListWidgetItem(pDuplicated->getName().c_str());
			QVariant variant;
			variant.setValue(pDuplicated);
			pNewItem->setData(Qt::UserRole, variant);
			listWidgetSlots->addItem(pNewItem);
			pNewItem->setFlags(pNewItem->flags() | Qt::ItemIsEditable);
			pNewItem->setSelected(true);
		}
	}
}

void MainWindow::on_actionRemove_Slot_triggered()
{
	Mutex::scoped_lock lock(_mutex);
	const QList<QListWidgetItem*>& selected = listWidgetSlots->selectedItems();
	if (!selected.empty())
	{
		QListWidgetItem* pSelected = selected[0];
		if (pSelected->text() != "default")
		{
			int row = listWidgetSlots->row(pSelected);
			listWidgetSlots->takeItem(row);
			_userSettings.configSlots.erase(std::find_if(_userSettings.configSlots.begin(), _userSettings.configSlots.end(), boost::bind(&Slot::getName, _1) == pSelected->text().toStdString()));

			// Select the next one
			if (row < listWidgetSlots->count())
			{
				listWidgetSlots->item(row)->setSelected(true);
			}
			else
			{
				listWidgetSlots->item(listWidgetSlots->count()-1)->setSelected(true);
			}
		}
	}
}

Slot::Ptr MainWindow::createDefaultSlot()
{
	Slot::Ptr pDefaultSlot(new Slot());
	pDefaultSlot->setName("default");

	// Snare
	const Pad::Ptr& pElSnare = Pad::Ptr(new Pad(Pad::SNARE, Pad::NOTE_SNARE));
	pElSnare->setTypeFlam(Pad::TOM1);
	pDefaultSlot->getPads().push_back(pElSnare);

	// Hi Hat
	const Pad::Ptr& pElHihat = Pad::Ptr(new Pad(Pad::HIHAT, Pad::NOTE_HIHAT)); 
	pElHihat->setTypeFlam(Pad::SNARE);
	pDefaultSlot->getPads().push_back(pElHihat);

	// Hi Hat Pedal
	const HiHatPedalElement::Ptr& pElHihatPedal = HiHatPedalElement::Ptr(new HiHatPedalElement());
	pDefaultSlot->getPads().push_back(pElHihatPedal);

	const Pad::Ptr& pElTom1 = Pad::Ptr(new Pad(Pad::TOM1, Pad::NOTE_TOM1)); 
	pElTom1->setTypeFlam(Pad::TOM2);
	pDefaultSlot->getPads().push_back(pElTom1);

	const Pad::Ptr& pElTom2 = Pad::Ptr(new Pad(Pad::TOM2, Pad::NOTE_TOM2)); 
	pElTom2->setTypeFlam(Pad::TOM3);
	pDefaultSlot->getPads().push_back(pElTom2);

	const Pad::Ptr& pElTom3 = Pad::Ptr(new Pad(Pad::TOM3, Pad::NOTE_TOM3)); 
	pElTom3->setTypeFlam(Pad::TOM2);
	pDefaultSlot->getPads().push_back(pElTom3);

	const Pad::Ptr& pElCrash1 = Pad::Ptr(new Pad(Pad::CRASH1, Pad::NOTE_CRASH1)); 
	pElCrash1->setTypeFlam(Pad::RIDE);
	pDefaultSlot->getPads().push_back(pElCrash1);

	const Pad::Ptr& pElCrash2 = Pad::Ptr(new Pad(Pad::CRASH2, Pad::NOTE_CRASH2)); 
	pElCrash2->setTypeFlam(Pad::RIDE);
	pDefaultSlot->getPads().push_back(pElCrash2);

	const Pad::Ptr& pElCrash3 = Pad::Ptr(new Pad(Pad::CRASH3, Pad::NOTE_CRASH3)); 
	pDefaultSlot->getPads().push_back(pElCrash3);

	const Pad::Ptr& pElRide = Pad::Ptr(new Pad(Pad::RIDE, Pad::NOTE_RIDE)); 
	pElRide->setTypeFlam(Pad::CRASH2);
	pDefaultSlot->getPads().push_back(pElRide);

	const Pad::Ptr& pElBassDrum = Pad::Ptr(new Pad(Pad::BASS_DRUM, Pad::NOTE_BASS_DRUM)); 
	pDefaultSlot->getPads().push_back(pElBassDrum);

	_pSettings->signalKitDefined.connect(boost::bind(&Slot::onDrumKitLoaded, pDefaultSlot, _1, _2));
	_pSettings->reloadDrumKitMidiMap();

	return pDefaultSlot;
}

void MainWindow::updateCurrentSlot()
{
	const Slot::Ptr& pCurrentSlot = getCurrentSlot();
	if (!pCurrentSlot.get())
	{
		return;
	}

	const Pad::List& pads = pCurrentSlot->getPads();
	const HiHatPedalElement::Ptr& pElHihatPedal = boost::dynamic_pointer_cast<HiHatPedalElement>(pads[Pad::HIHAT_PEDAL]);

	{
		QLayoutItem* pLayoutItem = gridLayoutHiHat->itemAtPosition(0,0);
		if (pLayoutItem)
		{
			TreeViewParameters* pTreeView = dynamic_cast<TreeViewParameters*>(pLayoutItem->widget());

			// Modifying data
			const Parameter::Ptr& pRoot = pTreeView->getRoot();
			const Parameter::Ptr& pGroup1 = pRoot->getChildAt(0);
			{
				pGroup1->getChildAt(0)->update(	pElHihatPedal->getSecurityPosition(),
						boost::bind(&HiHatPedalElement::setSecurityPosition, pElHihatPedal, _1));
				pGroup1->getChildAt(1)->update(	pElHihatPedal->isBowAlwaysYellow(),
						boost::bind(&HiHatPedalElement::setBowAlwaysYellow, pElHihatPedal, _1));
			}

			const Parameter::Ptr& pGroup2 = pRoot->getChildAt(1);
			pGroup2->update(	pElHihatPedal->isBlueDetectionByAccent(),
					boost::bind(&HiHatPedalElement::setBlueDetectionByAccent, pElHihatPedal, _1));
			{
				pGroup2->getChildAt(0)->update(	pElHihatPedal->getBlueAccentFunctions(),
						boost::bind(&HiHatPedalElement::setBlueAccentFunctions, pElHihatPedal, _1));
				pGroup2->getChildAt(1)->update(	pElHihatPedal->isBlueAccentOverride(),
						boost::bind(&HiHatPedalElement::setBlueAccentOverride, pElHihatPedal, _1));
			}

			const Parameter::Ptr& pGroup3 = pRoot->getChildAt(2);
			pGroup3->update(	pElHihatPedal->isBlueDetectionByPosition(),
					boost::bind(&HiHatPedalElement::setBlueDetectionByPosition, pElHihatPedal, _1));
			{
				pGroup3->getChildAt(0)->update(	pElHihatPedal->getControlPosThreshold(),
						boost::bind(&HiHatPedalElement::setControlPosThreshold, pElHihatPedal, _1));

				pGroup3->getChildAt(1)->update(	pElHihatPedal->getControlPosDelayTime(),
						boost::bind(&HiHatPedalElement::setControlPosDelayTime, pElHihatPedal, _1));
			}

			const Parameter::Ptr& pGroup4 = pRoot->getChildAt(3);
			pGroup4->update(	pElHihatPedal->isBlueDetectionBySpeed(),
					boost::bind(&HiHatPedalElement::setBlueDetectionBySpeed, pElHihatPedal, _1));
			{
				pGroup4->getChildAt(0)->update(	pElHihatPedal->getOpenSpeed(),
						boost::bind(&HiHatPedalElement::setOpenSpeed, pElHihatPedal, _1));

				pGroup4->getChildAt(1)->update(	pElHihatPedal->getCloseSpeed(),
						boost::bind(&HiHatPedalElement::setCloseSpeed, pElHihatPedal, _1));
			}

			const Parameter::Ptr pGroup5 = pRoot->getChildAt(4);
			pGroup5->update(pElHihatPedal->isHalfOpenModeEnabled(),
					boost::bind(&HiHatPedalElement::setHalfOpenModeEnabled, pElHihatPedal, _1));
			{

				pGroup5->getChildAt(0)->update(	pElHihatPedal->getHalfOpenMaximumPosition(),
						boost::bind(&HiHatPedalElement::setHalfOpenMaximumPosition, pElHihatPedal, _1));

				pGroup5->getChildAt(1)->update(	pElHihatPedal->getHalfOpenActivationTime(),
						boost::bind(&HiHatPedalElement::setHalfOpenActivationTime, pElHihatPedal, _1));
			}

			const Parameter::Ptr& pGroup6 = pRoot->getChildAt(5);
			{
				pGroup6->getChildAt(0)->update(	pCurrentSlot->getCymbalSimHitWindow(),
						boost::bind(&Slot::setCymbalSimHitWindow, pCurrentSlot, _1));

				pGroup6->getChildAt(1)->update(	pCurrentSlot->isAutoConvertCrash(Slot::CRASH_CRASH),
						boost::bind(&Slot::setAutoConvertCrash, pCurrentSlot, Slot::CRASH_CRASH, _1));

				pGroup6->getChildAt(2)->update(	pCurrentSlot->isAutoConvertCrash(Slot::CRASH_RIDE),
						boost::bind(&Slot::setAutoConvertCrash, pCurrentSlot, Slot::CRASH_RIDE, _1));

				pGroup6->getChildAt(3)->update(	pCurrentSlot->isAutoConvertCrash(Slot::CRASH_SNARE),
						boost::bind(&Slot::setAutoConvertCrash, pCurrentSlot, Slot::CRASH_SNARE, _1));

				pGroup6->getChildAt(4)->update(	pCurrentSlot->isAutoConvertCrash(Slot::CRASH_TOM2),
						boost::bind(&Slot::setAutoConvertCrash, pCurrentSlot, Slot::CRASH_TOM2, _1));

				pGroup6->getChildAt(5)->update(	pCurrentSlot->isAutoConvertCrash(Slot::CRASH_TOM3),
						boost::bind(&Slot::setAutoConvertCrash, pCurrentSlot, Slot::CRASH_TOM3, _1));
			}

			// Update view
			pTreeView->update();
		}
	}

	{
		QLayoutItem* pLayoutItem = gridLayoutFootSplashCancel->itemAtPosition(0,0);
		if (pLayoutItem)
		{
			TreeViewParameters* pTreeView = dynamic_cast<TreeViewParameters*>(pLayoutItem->widget());
			const Parameter::Ptr& pRoot = pTreeView->getRoot();
			const Parameter::Ptr& pGroup1 = pRoot->getChildAt(0);

			pElHihatPedal->connectFootCancelActivated(boost::bind(&GraphSubWindow::onFootCancelActivated, _pGrapSubWindow, _1));
			pGroup1->update(	pElHihatPedal->isFootCancel(),
					boost::bind(&HiHatPedalElement::setFootCancel, pElHihatPedal, _1));
			{
				pGroup1->getChildAt(0)->update(	pElHihatPedal->getFootCancelClosingSpeed(),
						boost::bind(&HiHatPedalElement::setFootCancelClosingSpeed, pElHihatPedal, _1));
				pGroup1->getChildAt(1)->update(	pElHihatPedal->getFootCancelPos(),
						boost::bind(&HiHatPedalElement::setFootCancelPos, pElHihatPedal, _1));
				pGroup1->getChildAt(2)->update(	pElHihatPedal->getFootCancelPosDiff(),
						boost::bind(&HiHatPedalElement::setFootCancelPosDiff, pElHihatPedal, _1));
				pGroup1->getChildAt(3)->update(	pElHihatPedal->getFootCancelMaskTime(),
						boost::bind(&HiHatPedalElement::setFootCancelMaskTime, pElHihatPedal, _1));
				pElHihatPedal->connectFootCancelMaskTime(boost::bind(&GraphSubWindow::onFootCancelMaskTime, _pGrapSubWindow, _1));

				pGroup1->getChildAt(4)->update(	pElHihatPedal->getFootCancelVelocity(),
						boost::bind(&HiHatPedalElement::setFootCancelVelocity, pElHihatPedal, _1));
				pElHihatPedal->connectFootCancelVelocity(boost::bind(&GraphSubWindow::onFootCancelVelocity, _pGrapSubWindow, _1));
			}

			const Parameter::Ptr& pGroup2 = pRoot->getChildAt(1);
			pElHihatPedal->connectFootCancelAfterPedalHitActivated(boost::bind(&GraphSubWindow::onFootCancelAfterPedalHitActivated, _pGrapSubWindow, _1));
			pGroup2->update(	pElHihatPedal->isFootCancelAfterPedalHit(),
					boost::bind(&HiHatPedalElement::setFootCancelAfterPedalHit, pElHihatPedal, _1));
			{
				pGroup2->getChildAt(0)->update(	pElHihatPedal->getFootCancelAfterPedalHitMaskTime(),
						boost::bind(&HiHatPedalElement::setFootCancelAfterPedalHitMaskTime, pElHihatPedal, _1));
				pElHihatPedal->connectFootCancelAfterPedalHitMaskTime(boost::bind(&GraphSubWindow::onFootCancelAfterPedalHitMaskTime, _pGrapSubWindow, _1));

				pGroup2->getChildAt(1)->update(	pElHihatPedal->getFootCancelAfterPedalHitVelocity(),
						boost::bind(&HiHatPedalElement::setFootCancelAfterPedalHitVelocity, pElHihatPedal, _1));
				pElHihatPedal->connectFootCancelAfterPedalHitVelocity(boost::bind(&GraphSubWindow::onFootCancelAfterPedalHitVelocity, _pGrapSubWindow, _1));
			}

			const Parameter::Ptr& pGroup3 = pRoot->getChildAt(2);
			pGroup3->update(	pElHihatPedal->isCancelHitWhileOpen(),
					boost::bind(&HiHatPedalElement::setCancelHitWhileOpen, pElHihatPedal, _1));
			{
				pGroup3->getChildAt(0)->update(	pElHihatPedal->getCancelOpenHitThreshold(),
						boost::bind(&HiHatPedalElement::setCancelOpenHitThreshold, pElHihatPedal, _1));
				pGroup3->getChildAt(1)->update(	pElHihatPedal->getCancelOpenHitVelocity(),
						boost::bind(&HiHatPedalElement::setCancelOpenHitVelocity, pElHihatPedal, _1));
			}

			pTreeView->update();
		}
	}

	{
		QLayoutItem* pLayoutItem = gridLayoutFlams->itemAtPosition(0,0);
		if (pLayoutItem)
		{
			TreeViewParameters* pTreeView = dynamic_cast<TreeViewParameters*>(pLayoutItem->widget());
			const Parameter::Ptr& pRoot = pTreeView->getRoot();
			size_t count = pRoot->getChildrenCount();
			size_t indexPad = 0;
			for (size_t i=0; i<count; ++i, ++indexPad)
			{
				Parameter::Ptr pGroup1 = pRoot->getChildAt(i);
				Pad::Ptr pPad = pads[indexPad];
				if (pPad->getType()==Pad::HIHAT_PEDAL)
				{
					pPad = pads[++indexPad];
				}

				//Group update
				pGroup1->setColor(QColor(pPad->getColor().c_str()));
				pGroup1->update(pPad->isFlamActivated(), boost::bind(&Pad::setFlamActivated, pPad, _1));

				pGroup1->getChildAt(0)->update(	pPad->getTypeFlam(), boost::bind(&Pad::setTypeFlam, pPad, _1));
				pGroup1->getChildAt(1)->update(	pPad->getFlamFunctions(), boost::bind(&Pad::setFlamFunctions, pPad, _1));
				pGroup1->getChildAt(2)->update(	pPad->getFlamCancelDuringRoll(), boost::bind(&Pad::setFlamCancelDuringRoll, pPad, _1));
			}

			pTreeView->update();
		}
	}

	{
		QLayoutItem* pLayoutItem = gridLayoutGhostNotes->itemAtPosition(0,0);
		if (pLayoutItem)
		{
			TreeViewParameters* pTreeView = dynamic_cast<TreeViewParameters*>(pLayoutItem->widget());
			const Parameter::Ptr& pRoot = pTreeView->getRoot();
			const Parameter::Ptr& pGroup1 = pRoot->getChildAt(0);
			size_t id = 0;
			Pad::List::const_iterator it = pads.begin();
			while (it!=pads.end())
			{
				const Pad::Ptr& pPad = *(it++);

				const Parameter::Ptr& pParameter = pGroup1->getChildAt(id++);
				pParameter->setColor(QColor(pPad->getColor().c_str()));
				pParameter->update(	pPad->getGhostVelocityLimit(), boost::bind(&Pad::setGhostVelocityLimit, pPad, _1));
			}
			pTreeView->update();
		}
	}
}

void MainWindow::on_listWidgetSlots_itemSelectionChanged()
{
	const QList<QListWidgetItem*>& selected = listWidgetSlots->selectedItems();
	if (!selected.empty())
	{
		QListWidgetItem* pSelected = selected[0];
		_pSettings->setLastSelectedSlotIndex(listWidgetSlots->row(pSelected));
		_currentSlot = std::find_if(_userSettings.configSlots.begin(), _userSettings.configSlots.end(), boost::bind(&Slot::getName, _1) == pSelected->text().toStdString());
		if (_currentSlot != _userSettings.configSlots.end())
		{
			updateCurrentSlot();

			// Hi Hat Curve default states
			on_tabWidget_currentChanged(tabWidget->currentIndex());
		}

		actionDuplicate_Slot->setEnabled(true);
		actionRemove_Slot->setEnabled(true);
	}

	actionAdd_Slot->setEnabled(true);
}

void MainWindow::on_menuEdit_aboutToShow()
{
	const QList<QListWidgetItem*>& selected = listWidgetSlots->selectedItems();

	actionAdd_Slot->setEnabled(true);
	if (!selected.empty())
	{
		actionRemove_Slot->setEnabled(selected[0]->text() != "default");
		actionDuplicate_Slot->setEnabled(true);
	}
	else
	{
		actionRemove_Slot->setEnabled(false);
		actionDuplicate_Slot->setEnabled(false);
	}
}

MidiMessage* MainWindow::getNextMessage(const boost::shared_ptr<Pad>& pElement, int msgType)
{
	MidiMessage* pResult = NULL;

	MidiMessage::List::iterator it = _midiMessages.begin();
	while (it!=_midiMessages.end())
	{
		MidiMessage& r = *(it++);
		if (r.getMsgType() == msgType && pElement->isA(r.getOriginalNote()))
		{
			pResult = &r;
			break;
		}
	}

	return pResult;
}

void MainWindow::on_listWidgetSlots_itemChanged(QListWidgetItem* pItem)
{
	Mutex::scoped_lock lock(_mutex);
	const QVariant& variant = pItem->data(Qt::UserRole);
	if (!variant.isNull())
	{
		const Slot::Ptr& pSlot = variant.value<Slot::Ptr>();

		const std::string& szNewName = pItem->text().toStdString();
		const std::string& szOldName = pSlot->getName();
		if (szNewName != szOldName)
		{
			// Find the orignal slot with szOldName
			Slot::List::iterator it = std::find_if(_userSettings.configSlots.begin(), _userSettings.configSlots.end(), boost::bind(&Slot::getName, _1) == szOldName);
			if (it!=_userSettings.configSlots.end())
			{
				// Found, we rename the slot
				const Slot::Ptr& pSlotToRename = *(it);
				pSlotToRename->setName(createNewSlotName(szNewName));

				listWidgetSlots->blockSignals(true);
				pItem->setText(pSlotToRename->getName().c_str());
				QVariant variant;
				variant.setValue(pSlotToRename);
				pItem->setData(Qt::UserRole, variant);
				listWidgetSlots->blockSignals(false);
			}
		}
	}
}

void MainWindow::on_actionAssistant_triggered()
{
	QString applicationPath(QApplication::applicationDirPath());
	std::vector<QFileInfo> collections;
	collections.push_back(QFileInfo(applicationPath + "/doc/collection.qhc"));
	collections.push_back(QFileInfo(applicationPath + "/../doc/collection.qhc"));
	QString collectionPath;
	for (size_t i=0; i<collections.size(); ++i)
	{
		if (collections[i].exists())
		{
			collectionPath = collections[i].filePath();
			break;
		}
	}

	if (!collectionPath.isEmpty())
	{
#ifdef _WIN32
		QFileInfo program(applicationPath + "/assistant.exe");
#else
		QFileInfo program(applicationPath + "/assistant");
#endif
		if (program.exists())
		{
			QStringList arguments;
			arguments << "-collectionFile" << collectionPath;
			_pProcessAssistant->start(program.filePath(), arguments);
		}
		else
		{
			QMessageBox::critical(this, tr("Error can't find assistant"), tr("Can't find the binary \"%1\"").arg(program.filePath()));
		}
	}
}

void MainWindow::on_actionAbout_triggered()
{
	boost::format fmtMsg("%s (beta) %s");
	fmtMsg%APPLICATION_NAME%APPLICATION_VERSION;
	DialogAbout dlgAbout(this, fmtMsg.str(), "FromtonRouge");
	dlgAbout.exec();
}

void MainWindow::on_groupBoxLogs_toggled(bool checked)
{
	Mutex::scoped_lock lock(_mutex);
	_userSettings.setLog(UserSettings::LOG_ACTIVATED, checked);
}

void MainWindow::on_checkBoxLogsRawData_toggled(bool checked)
{
	Mutex::scoped_lock lock(_mutex);
	_userSettings.setLog(UserSettings::LOG_RAW_DATA, checked);
}

void MainWindow::on_checkBoxLogsFilteredData_toggled(bool checked)
{
	Mutex::scoped_lock lock(_mutex);
	_userSettings.setLog(UserSettings::LOG_FILTERED_DATA, checked);
}

void MainWindow::on_checkBoxLogsHiHatControl_toggled(bool checked)
{
	Mutex::scoped_lock lock(_mutex);
	_userSettings.setLog(UserSettings::LOG_HIHAT_CONTROL, checked);
}

void MainWindow::on_checkBoxLogsOthers_toggled(bool checked)
{
	Mutex::scoped_lock lock(_mutex);
	_userSettings.setLog(UserSettings::LOG_OTHERS, checked);
}

void MainWindow::on_pushButtonClearLogs_clicked(bool)
{
	textEditLog->clear();
}

void MainWindow::computeMessage(MidiMessage& currentMsg, MidiMessage::DictHistory& lastMsgSent)
{
	Mutex::scoped_lock lock(_mutex);

	const Pad::Ptr& pElSnare =		getCurrentSlot()->getPads()[Pad::SNARE];
	const Pad::Ptr& pElHihat =		getCurrentSlot()->getPads()[Pad::HIHAT];
	const HiHatPedalElement::Ptr& pElHihatPedal =	boost::dynamic_pointer_cast<HiHatPedalElement>(getCurrentSlot()->getPads()[Pad::HIHAT_PEDAL]);
	const Pad::Ptr& pElTom1 =		getCurrentSlot()->getPads()[Pad::TOM1];
	const Pad::Ptr& pElTom2 =		getCurrentSlot()->getPads()[Pad::TOM2];
	const Pad::Ptr& pElTom3 =		getCurrentSlot()->getPads()[Pad::TOM3];
	const Pad::Ptr& pElCrash1 =		getCurrentSlot()->getPads()[Pad::CRASH1];
	const Pad::Ptr& pElCrash2 =		getCurrentSlot()->getPads()[Pad::CRASH2];
	const Pad::Ptr& pElCrash3 =		getCurrentSlot()->getPads()[Pad::CRASH3];
	const Pad::Ptr& pElRide =		getCurrentSlot()->getPads()[Pad::RIDE];
	const Pad::Ptr& pElBassDrum =	getCurrentSlot()->getPads()[Pad::BASS_DRUM];

	int cymbalsSimHitWindow = getCurrentSlot()->getCymbalSimHitWindow();
	bool mutableCrashWithCrash = getCurrentSlot()->isAutoConvertCrash(Slot::CRASH_CRASH);
	bool mutableCrashWithRide = getCurrentSlot()->isAutoConvertCrash(Slot::CRASH_RIDE);
	bool mutableCrashWithSnare = getCurrentSlot()->isAutoConvertCrash(Slot::CRASH_SNARE);
	bool mutableCrashWithTom2 = getCurrentSlot()->isAutoConvertCrash(Slot::CRASH_TOM2);
	bool mutableCrashWithTom3 = getCurrentSlot()->isAutoConvertCrash(Slot::CRASH_TOM3);
	bool bHasNextMidiMessage = !_midiMessages.empty();

	const int DEFAULT_NOTE_MSG_CTRL(4);
	int currentTime = currentMsg.getTimestamp();
	if (currentMsg.isControllerMsg() && currentMsg.getOutputNote() == DEFAULT_NOTE_MSG_CTRL)
	{
		int currentControlPos = pElHihatPedal->getCurrentControlPos();
		float currentAccel = pElHihatPedal->getCurrentControlAcceleration();
		bool bSecured = currentControlPos<=pElHihatPedal->getSecurityPosition();
		if (bSecured)
		{
			pElHihatPedal->setBlue(false, HiHatPedalElement::IN_SECURED_ZONE);
		}

		// Half-open mode
		if (pElHihatPedal->isHalfOpenModeEnabled())
		{
			if (bSecured)
			{
				pElHihatPedal->setHalfOpenEnteringTime(0);
				pElHihatPedal->setHalfOpen(false);
			}
			else if (currentControlPos<=pElHihatPedal->getHalfOpenMaximumPosition() && pElHihatPedal->getHalfOpenEnteringTime()==0)
			{
				pElHihatPedal->setHalfOpenEnteringTime(currentTime);
			}
			else if (currentControlPos<=pElHihatPedal->getHalfOpenMaximumPosition() && pElHihatPedal->getHalfOpenEnteringTime()>0)
			{
				int enteringTime = pElHihatPedal->getHalfOpenEnteringTime();
				int activationTime = pElHihatPedal->getHalfOpenActivationTime();
				if (currentTime-enteringTime > activationTime && !pElHihatPedal->isBlue())
				{
					// Still yellow after activationTime, we can switch to half-open mode
					pElHihatPedal->setHalfOpen(true);
				}
			}
			else
			{
				pElHihatPedal->setHalfOpenEnteringTime(0);
			}
		}
		else
		{
			pElHihatPedal->setHalfOpen(false);
		}

		// Blue state by pedal speed
		if (pElHihatPedal->isBlueDetectionBySpeed())
		{
			if (!pElHihatPedal->isHalfOpen() && !bSecured)
			{
				// Are we opening the Hi-Hat
				if (pElHihatPedal->getCurrentControlSpeed() > 0)
				{
					if (pElHihatPedal->getCurrentControlSpeed() >= pElHihatPedal->getOpenSpeed())
					{
						pElHihatPedal->setBlue(true, HiHatPedalElement::OPENING_MOVEMENT);
					}
					else
					{
						// Only change the reason to OPENING_MOVEMENT
						pElHihatPedal->setBlueStateChangeReason(HiHatPedalElement::OPENING_MOVEMENT);
					}
				}
				else if (pElHihatPedal->getCurrentControlSpeed() < 0)
				{
					// Hi-Hat closing
					if (pElHihatPedal->getCurrentControlSpeed() <= pElHihatPedal->getCloseSpeed())
					{
						// Hi Hat closing and the close speed is reached
						pElHihatPedal->setBlue(false, HiHatPedalElement::CLOSING_MOVEMENT);
					}
				}
				else
				{
					// current speed == 0 => nothing we keep the last blue state (on or off)
				}
			}
		}

		// Blue state by pedal position
		if (!pElHihatPedal->isHalfOpen() && pElHihatPedal->isBlueDetectionByPosition())
		{
			if (currentControlPos > pElHihatPedal->getControlPosThreshold())
			{
				if (pElHihatPedal->getBlueStateEnteringTime()==0)
				{
					pElHihatPedal->setBlueStateEnteringTime(currentTime); }

				if (currentTime-pElHihatPedal->getBlueStateEnteringTime() >= pElHihatPedal->getControlPosDelayTime())
				{
					pElHihatPedal->setBlue(true, HiHatPedalElement::POSITION_THRESHOLD);
				}
			}
			else if (!pElHihatPedal->isBlueDetectionBySpeed())
			{
				pElHihatPedal->setBlueStateEnteringTime(0);
				pElHihatPedal->setBlue(false, HiHatPedalElement::POSITION_THRESHOLD);
			}
		}

		// Send the hi-hat open state to the GUI
		HiHatPositionCurve::HiHatState state = HiHatPositionCurve::HHS_CLOSED;
		if (pElHihatPedal->isBlue())
		{
			state = HiHatPositionCurve::HHS_OPEN;
		}
		else if (pElHihatPedal->isHalfOpen())
		{
			state = HiHatPositionCurve::HHS_HALF_OPEN;
		}
		else if (bSecured)
		{
			state = HiHatPositionCurve::HHS_SECURED;
		}
		else
		{
			state = HiHatPositionCurve::HHS_CLOSED;
		}
		emit hiHatState(state);

		if (pElHihatPedal->isFootCancel())
		{
			int posDiff = pElHihatPedal->getPositionOnCloseBegin()-currentControlPos;
			float speed = pElHihatPedal->getCurrentControlSpeed();

			if ( 
					currentTime > pElHihatPedal->getFootCancelTimeLimit() && 
					speed <= pElHihatPedal->getFootCancelClosingSpeed() &&
					currentControlPos <= pElHihatPedal->getFootCancelPos() &&
					posDiff >= pElHihatPedal->getFootCancelPosDiff()
			   )
			{
				int cancelMaskTime = pElHihatPedal->getFootCancelMaskTime();
				pElHihatPedal->setFootCancelTimeLimit(currentTime+cancelMaskTime);

				emit footCancelStarted(currentTime, cancelMaskTime, pElHihatPedal->getFootCancelVelocity());

				if (_userSettings.isLogs() && _userSettings.isLog(UserSettings::LOG_HIHAT_CONTROL))
				{
					boost::format fmt("Foot Splash cancel timer started : speed=%.3f (unit/s), accel=%.3f (unit/ss), pos=%d (unit), posClose=%d (unit), posDiff=%d (unit)");
					std::cout << (fmt%speed%currentAccel%currentControlPos%pElHihatPedal->getPositionOnCloseBegin()%posDiff) << std::endl;
				}
			}
		}
	}

	if (currentMsg.isNoteOnMsg() && !currentMsg.isAlreadyModified())
	{
		switch (currentMsg.padType)
		{
		case Pad::HIHAT_PEDAL:
			{
				if (bHasNextMidiMessage && pElHihatPedal->isFootCancelAfterPedalHit())
				{
					MidiMessage* pNextHiHat = getNextMessage(pElHihat);
					if (	pNextHiHat && currentMsg.isInTimeWindow(*pNextHiHat, pElHihatPedal->getFootCancelAfterPedalHitMaskTime()) &&
							pNextHiHat->getValue() < pElHihatPedal->getFootCancelAfterPedalHitVelocity())
					{
						pNextHiHat->ignore(MidiMessage::IGNORED_BECAUSE_FOOT_CANCEL);
					}
				}
				break;
			}

		case Pad::HIHAT:
			{
				int currentControlPos = pElHihatPedal->getCurrentControlPos();
				if ( pElHihatPedal->isCancelHitWhileOpen()
						&& currentControlPos >= pElHihatPedal->getCancelOpenHitThreshold() 
						&& currentMsg.getValue()<pElHihatPedal->getCancelOpenHitVelocity())
				{
					currentMsg.ignore(MidiMessage::IGNORED_BECAUSE_FOOT_CANCEL);
				}
				else if (currentMsg.getTimestamp() <= pElHihatPedal->getFootCancelTimeLimit() && currentMsg.getValue() < pElHihatPedal->getFootCancelVelocity())
				{
					currentMsg.ignore(MidiMessage::IGNORED_BECAUSE_FOOT_CANCEL);
				}

				if (pElHihatPedal->isBlue())
				{
					if (!pElHihatPedal->isBowAlwaysYellow() || !pElHihat->isA(currentMsg.getOriginalNote(), DrumNote::BOW))
					{
						// Change the yellow hi-hat to blue if the pedal is blue
						currentMsg.changeNoteTo(pElRide.get());
					}
				}
				else if (pElHihatPedal->isBlueDetectionByAccent() && !pElHihatPedal->isHalfOpen())
				{
					// Above security position ?
					if (currentControlPos > pElHihatPedal->getSecurityPosition() || pElHihatPedal->isBlueAccentOverride())
					{
						// Accented notes only detected on hi-hat edge (bow are alwyas yellow)
						if (pElHihat->isA(currentMsg.getOriginalNote(), DrumNote::EDGE))
						{
							// When blue detection by speed is activated 
							// and if the blue state is false because of a closing movement we don't apply linear functions
							if (!pElHihatPedal->isBlueDetectionBySpeed() || pElHihatPedal->getBlueStateChangeReason()!=HiHatPedalElement::CLOSING_MOVEMENT)
							{
								float y = 0.f;
								if (LinearFunction::apply(pElHihatPedal->getBlueAccentFunctions(), currentControlPos, y) && currentMsg.getValue() > y)
								{
									currentMsg.changeNoteTo(pElRide.get());
								}
							}
						}
					}
				}

				if (pElHihatPedal->isFootCancelAfterPedalHit() && !lastMsgSent[Pad::HIHAT_PEDAL].empty())
				{
					const MidiMessage& rLastHiHatPedal = lastMsgSent[Pad::HIHAT_PEDAL].front();
					if (pElHihatPedal->isA(rLastHiHatPedal.getOriginalNote()))
					{
						if (	currentMsg.isInTimeWindow(rLastHiHatPedal, pElHihatPedal->getFootCancelAfterPedalHitMaskTime()) &&
								currentMsg.getValue() < pElHihatPedal->getFootCancelAfterPedalHitVelocity())
						{
							currentMsg.ignore(MidiMessage::IGNORED_BECAUSE_FOOT_CANCEL);
						}
					}
				}

				// Flam and ghost
				if (!currentMsg.isIgnored())
				{
					sendMidiMessages(pElHihat->applyFlamAndGhost(getCurrentSlot()->getPads(), lastMsgSent, &currentMsg, getNextMessage(pElHihat)), true);
				}
				break;
			}

		case Pad::CRASH1:
			{
				// For the crash 2 we only get the previous
				MidiMessage* pPreviousCRASH2 = NULL;
				if (!lastMsgSent[Pad::CRASH2].empty())
				{
					pPreviousCRASH2 = getCurrentSlot()->getPads()[Pad::CRASH2]->isA(lastMsgSent[Pad::CRASH2].front().getOriginalNote())?&(lastMsgSent[Pad::CRASH2].front()):NULL;
				}

				MidiMessage* pNextOrPreviousRIDE = NULL;
				MidiMessage* pNextOrPreviousSNARE = NULL;
				MidiMessage* pNextOrPreviousTOM2 = NULL;
				MidiMessage* pNextOrPreviousTOM3 = NULL;
				if (bHasNextMidiMessage)
				{
					// Get next messages
					pNextOrPreviousRIDE = getNextMessage(pElRide);
					pNextOrPreviousSNARE = getNextMessage(pElSnare);
					pNextOrPreviousTOM2 = getNextMessage(pElTom2);
					pNextOrPreviousTOM3 = getNextMessage(pElTom3);
				}
				else
				{
					// Get last messages
					if (!lastMsgSent[Pad::RIDE].empty())
					{
						pNextOrPreviousRIDE = getCurrentSlot()->getPads()[Pad::RIDE]->isA(lastMsgSent[Pad::RIDE].front().getOriginalNote())?&(lastMsgSent[Pad::RIDE].front()):NULL;
					}
					if (!lastMsgSent[Pad::SNARE].empty())
					{
						pNextOrPreviousSNARE = getCurrentSlot()->getPads()[Pad::SNARE]->isA(lastMsgSent[Pad::SNARE].front().getOriginalNote())?&(lastMsgSent[Pad::SNARE].front()):NULL;
					}
					if (!lastMsgSent[Pad::TOM2].empty())
					{
						pNextOrPreviousTOM2 = getCurrentSlot()->getPads()[Pad::TOM2]->isA(lastMsgSent[Pad::TOM2].front().getOriginalNote())?&(lastMsgSent[Pad::TOM2].front()):NULL;
					}
					if (!lastMsgSent[Pad::TOM3].empty())
					{
						pNextOrPreviousTOM3 = getCurrentSlot()->getPads()[Pad::TOM3]->isA(lastMsgSent[Pad::TOM3].front().getOriginalNote())?&(lastMsgSent[Pad::TOM3].front()):NULL;
					}
				}

				if (mutableCrashWithCrash && pPreviousCRASH2 && !pPreviousCRASH2->isAlreadyModified() && currentMsg.isInTimeWindow(*pPreviousCRASH2, cymbalsSimHitWindow))
				{
					currentMsg.changeNoteTo(pElHihat.get());
				}
				else if (mutableCrashWithRide && pNextOrPreviousRIDE && currentMsg.isInTimeWindow(*pNextOrPreviousRIDE, cymbalsSimHitWindow))
				{
					currentMsg.changeNoteTo(pElHihat.get());
				}
				else if (mutableCrashWithSnare && pNextOrPreviousSNARE && currentMsg.isInTimeWindow(*pNextOrPreviousSNARE, cymbalsSimHitWindow))
				{
					currentMsg.changeNoteTo(pElHihat.get());
				}
				else if (mutableCrashWithTom2 && pNextOrPreviousTOM2 && currentMsg.isInTimeWindow(*pNextOrPreviousTOM2, cymbalsSimHitWindow))
				{
					currentMsg.changeNoteTo(pElHihat.get());
				}
				else if (mutableCrashWithTom3 && pNextOrPreviousTOM3 && currentMsg.isInTimeWindow(*pNextOrPreviousTOM3, cymbalsSimHitWindow))
				{
					currentMsg.changeNoteTo(pElHihat.get());
				}
				else
				{
					// Flam and ghost
					sendMidiMessages(pElCrash1->applyFlamAndGhost(getCurrentSlot()->getPads(), lastMsgSent, &currentMsg, getNextMessage(pElCrash1)), true);
				}
				break;
			}

		case Pad::CRASH3:
			{
				// Flam and ghost
				sendMidiMessages(pElCrash3->applyFlamAndGhost(getCurrentSlot()->getPads(), lastMsgSent, &currentMsg, getNextMessage(pElCrash3)), true);
				break;
			}

		case Pad::CRASH2:
			{
				MidiMessage* pPreviousAutoConvertCrash = NULL;
				if (!lastMsgSent[Pad::CRASH1].empty())
				{
					pPreviousAutoConvertCrash = getCurrentSlot()->getPads()[Pad::CRASH1]->isA(lastMsgSent[Pad::CRASH1].front().getOriginalNote())?&(lastMsgSent[Pad::CRASH1].front()):NULL;
				}

				if (mutableCrashWithCrash && pPreviousAutoConvertCrash && !pPreviousAutoConvertCrash->isAlreadyModified() && currentMsg.isInTimeWindow(*pPreviousAutoConvertCrash, cymbalsSimHitWindow))
				{
					// Previous was a mutable crash, if the mutable was not changed we have to change the CRASH2 to yellow
					currentMsg.changeNoteTo(pElHihat.get());
				}
				else
				{
					// Flam and ghost
					sendMidiMessages(pElCrash2->applyFlamAndGhost(getCurrentSlot()->getPads(), lastMsgSent, &currentMsg, getNextMessage(pElCrash2)), true);
				}
				break;
			}

		case Pad::RIDE:
			{
				MidiMessage* pNextAutoConvertCrash = getNextMessage(pElCrash1);
				if (mutableCrashWithRide && pNextAutoConvertCrash && currentMsg.isInTimeWindow(*pNextAutoConvertCrash, cymbalsSimHitWindow))
				{
					// Yellow Crash
					pNextAutoConvertCrash->changeNoteTo(pElHihat.get());
				}
				else
				{
					// Flam and ghost
					sendMidiMessages(pElRide->applyFlamAndGhost(getCurrentSlot()->getPads(), lastMsgSent, &currentMsg, getNextMessage(pElRide)), true);
				}
				break;
			}

		case Pad::SNARE:
			{
				MidiMessage* pNextAutoConvertCrash = getNextMessage(pElCrash1);
				if (mutableCrashWithSnare && pNextAutoConvertCrash && currentMsg.isInTimeWindow(*pNextAutoConvertCrash, cymbalsSimHitWindow))
				{
					// Yellow Crash
					pNextAutoConvertCrash->changeNoteTo(pElHihat.get());
				}
				else
				{
					// Flam and ghost
					sendMidiMessages(pElSnare->applyFlamAndGhost(getCurrentSlot()->getPads(), lastMsgSent, &currentMsg, getNextMessage(pElSnare)), true);
				}
				break;
			}

		case Pad::TOM1:
			{
				// Flam and ghost
				sendMidiMessages(pElTom1->applyFlamAndGhost(getCurrentSlot()->getPads(), lastMsgSent, &currentMsg, getNextMessage(pElTom1)), true);
				break;
			}

		case Pad::TOM2:
			{
				MidiMessage* pNextAutoConvertCrash = getNextMessage(pElCrash1);
				if (mutableCrashWithTom2 && pNextAutoConvertCrash && currentMsg.isInTimeWindow(*pNextAutoConvertCrash, cymbalsSimHitWindow))
				{
					// Yellow crash
					pNextAutoConvertCrash->changeNoteTo(pElHihat.get());
				}
				else
				{
					// Flam and ghost
					sendMidiMessages(pElTom2->applyFlamAndGhost(getCurrentSlot()->getPads(), lastMsgSent, &currentMsg, getNextMessage(pElTom2)), true);
				}
				break;
			}

		case Pad::TOM3:
			{
				MidiMessage* pNextAutoConvertCrash = getNextMessage(pElCrash1);
				if (mutableCrashWithTom3 && pNextAutoConvertCrash && currentMsg.isInTimeWindow(*pNextAutoConvertCrash, cymbalsSimHitWindow))
				{
					// Yellow crash
					pNextAutoConvertCrash->changeNoteTo(pElHihat.get());
				}
				else
				{
					// Flam and ghost
					sendMidiMessages(pElTom3->applyFlamAndGhost(getCurrentSlot()->getPads(), lastMsgSent, &currentMsg, getNextMessage(pElTom3)), true);
				}
				break;
			}

		case Pad::BASS_DRUM:
			{
				// Flam and ghost
				sendMidiMessages(pElBassDrum->applyFlamAndGhost(getCurrentSlot()->getPads(), lastMsgSent, &currentMsg, getNextMessage(pElBassDrum)), true);
				break;
			}

		default:
			{
				break;
			}
		}
	}
}

void MainWindow::on_tabWidget_currentChanged(int index)
{
	switch(index)
	{
	case 0:
		{
			_pGrapSubWindow->showHiHatLayers(true);
			_pGrapSubWindow->showFootCancelLayers(false);
			_pGrapSubWindow->showHiHatPedalMaskLayer(false);
			break;
		}
	case 1:
		{
			_pGrapSubWindow->showHiHatLayers(false);
			_pGrapSubWindow->showFootCancelLayers(true);
			_pGrapSubWindow->showHiHatPedalMaskLayer(true);
			break;
		}
	default:
		{
			break;
		}
	}
}

void MainWindow::on_actionSettings_triggered()
{
	SettingsDlg settings(_pSettings.get());
	connect(this, SIGNAL(midiNoteOn(int,int)), &settings, SIGNAL(midiNoteOn(int,int)));
	if (settings.exec())
	{
		updateCurrentSlot();
	}
}

PmStream* MainWindow::getMidiInStream() const
{
	Mutex::scoped_lock lock(_mutex);
	return _pMidiIn;
}

PmStream* MainWindow::getMidiOutStream() const
{
	Mutex::scoped_lock lock(_mutex);
	return _pMidiOut;
}
const UserSettings& MainWindow::getConfig() const
{
	Mutex::scoped_lock lock(_mutex);
	return _userSettings;
}
