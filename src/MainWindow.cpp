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

void MainWindow::toLog(const std::string& szText)
{
	emit signalLog(boost::algorithm::trim_copy_if(szText, boost::is_any_of("\n")).c_str());
}

MainWindow::MainWindow():
	_pOldStreambuf(std::cout.rdbuf()),
	_pSettings(new Settings()),
	_currentSlot(_userSettings.configSlots.end())
{
	qRegisterMetaType<Slot::Ptr>();
	qRegisterMetaType<MidiMessage>();

	setupUi(this);
	setWindowTitle((boost::format("%s")%APPLICATION_NAME).str().c_str());

	_pProcessAssistant = new QProcess(this);

	// Setup std::cout redirection
	_streamBuffer.open(StreamSink(boost::bind(&MainWindow::toLog, this, _1)));
	std::cout.rdbuf(&_streamBuffer);
	connect(this, SIGNAL(signalLog(const QString&)), textEditLog, SLOT(append(const QString&)));

	_pGrapSubWindow = new GraphSubWindow(&_userSettings, this);
	_pSettings->signalKitDefined.connect(boost::bind(&GraphSubWindow::onDrumKitLoaded, _pGrapSubWindow, _1, _2));
	_pSettings->signalKitDefined.connect(boost::bind(&MidiEngine::onDrumKitLoaded, &_midiEngine, _1, _2));
	connect(&_midiEngine, SIGNAL(signalHiHatStartMoving(int, int, int)), _pGrapSubWindow, SLOT(onHiHatStartMoving(int, int, int)));
	connect(&_midiEngine, SIGNAL(signalHiHatState(int)), _pGrapSubWindow, SLOT(onHiHatState(int)));
	connect(&_midiEngine, SIGNAL(signalFootCancelStarted(int, int, int)), _pGrapSubWindow, SLOT(onFootCancelStarted(int, int, int)));
	connect(&_midiEngine, SIGNAL(signalMidiOut(const MidiMessage&)), _pGrapSubWindow, SLOT(onUpdatePlot(const MidiMessage&)));
	connect(this, SIGNAL(signalSlotChanged(const Slot::Ptr&)), &_midiEngine, SLOT(onSlotChanged(const Slot::Ptr&)));
	connect(sliderBuffer, SIGNAL(valueChanged(int)), spinBoxBuffer, SLOT(setValue(int)));
	connect(spinBoxBuffer, SIGNAL(valueChanged(int)), &_midiEngine, SLOT(onBufferLengthChanged(int)));

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

		Parameter::Ptr pGroup5(new Parameter("Hi-hat open color detection by acceleration", groupColors[4],
					pElHihatPedal->isBlueDetectionByAcceleration(),
					boost::bind(&HiHatPedalElement::setBlueDetectionByAcceleration, pElHihatPedal, _1)));
		{
			Parameter::Ptr pOpenAcceleration(new Parameter("Open acceleration (unit/s²)", 0, 50000,
						pElHihatPedal->getOpenAcceleration(),
						boost::bind(&HiHatPedalElement::setOpenAcceleration, pElHihatPedal, _1),
						tr("Above this acceleration the hi-hat is converted to open color").toStdString()));
			pOpenAcceleration->setInfiniteExtremities(Parameter::InfiniteExtremities(false, true));
			pGroup5->addChild(pOpenAcceleration);
			pGroup5->addChild(Parameter::Ptr(new Parameter("Open position delta (unit)", 0, 127,
							pElHihatPedal->getOpenPositionDelta(),
							boost::bind(&HiHatPedalElement::setOpenPositionDelta, pElHihatPedal, _1),
							tr("Minimum open delta position").toStdString())));

			Parameter::Ptr pCloseAcceleration(new Parameter("Close acceleration (unit/s²)", -50000, 0,
						pElHihatPedal->getCloseAcceleration(),
						boost::bind(&HiHatPedalElement::setCloseAcceleration, pElHihatPedal, _1),
						tr("Under this acceleration the hi-hat is converted to close color").toStdString()));
			pCloseAcceleration->setInfiniteExtremities(Parameter::InfiniteExtremities(true, false));
			pGroup5->addChild(pCloseAcceleration);
			pGroup5->addChild(Parameter::Ptr(new Parameter("Close position delta (unit)", -127, 0,
							pElHihatPedal->getClosePositionDelta(),
							boost::bind(&HiHatPedalElement::setClosePositionDelta, pElHihatPedal, _1),
							tr("Minimum close delta position").toStdString())));
		}

		Parameter::Ptr pGroup6(new Parameter("Hi-hat half-open mode", groupColors[5],
					pElHihatPedal->isHalfOpenModeEnabled(),
					boost::bind(&HiHatPedalElement::setHalfOpenModeEnabled, pElHihatPedal, _1)));
		{

			pGroup6->addChild(Parameter::Ptr(new Parameter("Half open maximum position (unit)", 0, 127,
							pElHihatPedal->getHalfOpenMaximumPosition(),
							boost::bind(&HiHatPedalElement::setHalfOpenMaximumPosition, pElHihatPedal, _1),
							tr("Half open detection algorithm starts between [Security close position] and this position").toStdString())));

			pGroup6->addChild(Parameter::Ptr(new Parameter("Half open detection time (ms)", 0, 5000,
							pElHihatPedal->getHalfOpenActivationTime(),
							boost::bind(&HiHatPedalElement::setHalfOpenActivationTime, pElHihatPedal, _1),
							tr("Afters the specified time (ms), if the hi-hat is still in close color, it goes in half open mode. It will leave this mode if the control position go back under [Security close position]").toStdString())));
		}

		Parameter::Ptr pGroup7(new Parameter("Chameleon Crash conversion", groupColors[6]));
		{
			const std::string& szDescription = tr("A Chameleon Crash change its crash color to hi-hat close color if one of these pads is hit at the same time").toStdString();
			pGroup7->setDescription(szDescription);

			pGroup7->addChild(Parameter::Ptr(new Parameter("Time window (ms)", 0, 100,
							pCurrentSlot->getCymbalSimHitWindow(),
							boost::bind(&Slot::setCymbalSimHitWindow, pCurrentSlot, _1),
							tr("Timing window used to detect simultaneous hits between cymbals").toStdString())));

			pGroup7->addChild(Parameter::Ptr(new Parameter("Crash",
							pCurrentSlot->isAutoConvertCrash(Slot::CRASH_CRASH),
							boost::bind(&Slot::setAutoConvertCrash, pCurrentSlot, Slot::CRASH_CRASH, _1), szDescription)));

			pGroup7->addChild(Parameter::Ptr(new Parameter("Ride",
							pCurrentSlot->isAutoConvertCrash(Slot::CRASH_RIDE),
							boost::bind(&Slot::setAutoConvertCrash, pCurrentSlot, Slot::CRASH_RIDE, _1), szDescription)));

			pGroup7->addChild(Parameter::Ptr(new Parameter("Snare",
							pCurrentSlot->isAutoConvertCrash(Slot::CRASH_SNARE),
							boost::bind(&Slot::setAutoConvertCrash, pCurrentSlot, Slot::CRASH_SNARE, _1), szDescription)));

			pGroup7->addChild(Parameter::Ptr(new Parameter("Tom 2",
							pCurrentSlot->isAutoConvertCrash(Slot::CRASH_TOM2),
							boost::bind(&Slot::setAutoConvertCrash, pCurrentSlot, Slot::CRASH_TOM2, _1), szDescription)));

			pGroup7->addChild(Parameter::Ptr(new Parameter("Tom 3",
							pCurrentSlot->isAutoConvertCrash(Slot::CRASH_TOM3),
							boost::bind(&Slot::setAutoConvertCrash, pCurrentSlot, Slot::CRASH_TOM3, _1), szDescription)));
		}

		pRoot->addChild(pGroup1);
		pRoot->addChild(pGroup2);
		pRoot->addChild(pGroup3);
		pRoot->addChild(pGroup4);
		pRoot->addChild(pGroup5);
		pRoot->addChild(pGroup6);
		pRoot->addChild(pGroup7);
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

void MainWindow::on_pushButtonStart_clicked(bool)
{
	int midiInId = comboBoxMidiIn->itemData(comboBoxMidiIn->currentIndex()).toInt();
	int midiOutId = comboBoxMidiOut->itemData(comboBoxMidiOut->currentIndex()).toInt();
	if (_midiEngine.start(midiInId, midiOutId))
	{
		_pSettings->setMidiIn(comboBoxMidiIn->currentText().toStdString());
		_pSettings->setMidiOut(comboBoxMidiOut->currentText().toStdString());

		_pGrapSubWindow->clearPlots();
		_pGrapSubWindow->replot();

		comboBoxMidiIn->setEnabled(false);
		comboBoxMidiOut->setEnabled(false);
		pushButtonStart->setEnabled(false);
		pushButtonStop->setEnabled(true);
	}
}

void MainWindow::stop()
{
	_midiEngine.stop();
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

void MainWindow::on_spinBoxBuffer_valueChanged(int value)
{
	sliderBuffer->blockSignals(true);
	sliderBuffer->setValue(value);
	sliderBuffer->blockSignals(false);

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

			const Parameter::Ptr& pGroup5 = pRoot->getChildAt(4);
			pGroup5->update(	pElHihatPedal->isBlueDetectionByAcceleration(),
					boost::bind(&HiHatPedalElement::setBlueDetectionByAcceleration, pElHihatPedal, _1));
			{
				pGroup5->getChildAt(0)->update(	pElHihatPedal->getOpenAcceleration(),
						boost::bind(&HiHatPedalElement::setOpenAcceleration, pElHihatPedal, _1));
				pGroup5->getChildAt(1)->update(	pElHihatPedal->getOpenPositionDelta(),
						boost::bind(&HiHatPedalElement::setOpenPositionDelta, pElHihatPedal, _1));
				pGroup5->getChildAt(2)->update(	pElHihatPedal->getCloseAcceleration(),
						boost::bind(&HiHatPedalElement::setCloseAcceleration, pElHihatPedal, _1));
				pGroup5->getChildAt(3)->update(	pElHihatPedal->getClosePositionDelta(),
						boost::bind(&HiHatPedalElement::setClosePositionDelta, pElHihatPedal, _1));
			}

			const Parameter::Ptr pGroup6 = pRoot->getChildAt(5);
			pGroup6->update(pElHihatPedal->isHalfOpenModeEnabled(),
					boost::bind(&HiHatPedalElement::setHalfOpenModeEnabled, pElHihatPedal, _1));
			{

				pGroup6->getChildAt(0)->update(	pElHihatPedal->getHalfOpenMaximumPosition(),
						boost::bind(&HiHatPedalElement::setHalfOpenMaximumPosition, pElHihatPedal, _1));

				pGroup6->getChildAt(1)->update(	pElHihatPedal->getHalfOpenActivationTime(),
						boost::bind(&HiHatPedalElement::setHalfOpenActivationTime, pElHihatPedal, _1));
			}

			const Parameter::Ptr& pGroup7 = pRoot->getChildAt(6);
			{
				pGroup7->getChildAt(0)->update(	pCurrentSlot->getCymbalSimHitWindow(),
						boost::bind(&Slot::setCymbalSimHitWindow, pCurrentSlot, _1));

				pGroup7->getChildAt(1)->update(	pCurrentSlot->isAutoConvertCrash(Slot::CRASH_CRASH),
						boost::bind(&Slot::setAutoConvertCrash, pCurrentSlot, Slot::CRASH_CRASH, _1));

				pGroup7->getChildAt(2)->update(	pCurrentSlot->isAutoConvertCrash(Slot::CRASH_RIDE),
						boost::bind(&Slot::setAutoConvertCrash, pCurrentSlot, Slot::CRASH_RIDE, _1));

				pGroup7->getChildAt(3)->update(	pCurrentSlot->isAutoConvertCrash(Slot::CRASH_SNARE),
						boost::bind(&Slot::setAutoConvertCrash, pCurrentSlot, Slot::CRASH_SNARE, _1));

				pGroup7->getChildAt(4)->update(	pCurrentSlot->isAutoConvertCrash(Slot::CRASH_TOM2),
						boost::bind(&Slot::setAutoConvertCrash, pCurrentSlot, Slot::CRASH_TOM2, _1));

				pGroup7->getChildAt(5)->update(	pCurrentSlot->isAutoConvertCrash(Slot::CRASH_TOM3),
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

			emit signalSlotChanged(*_currentSlot);

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

void MainWindow::on_pushButtonClearLogs_clicked(bool)
{
	textEditLog->clear();
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
	connect(&_midiEngine, SIGNAL(signalMidiIn(const MidiMessage&)), &settings, SIGNAL(onMidiIn(const MidiMessage&)));
	if (settings.exec())
	{
		updateCurrentSlot();
	}
}

Slot::Ptr MainWindow::getCurrentSlot() const
{
	Mutex::scoped_lock lock(_mutex);
	if (_currentSlot!=_userSettings.configSlots.end())
	{
		return *_currentSlot;
	}
	else
	{
		return Slot::Ptr();
	}
}
