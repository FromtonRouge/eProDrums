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
#include "GhostControl.h"
#include "GhostKitControl.h"
#include "FlamKitControl.h"
#include "ValueControl.h"
#include "DialogAbout.h"
#include "Settings.h"
#include "SettingsDlg.h"

#include "Pad.h"
#include "HiHatPedalElement.h"

#include "EProPlot.h"
#include "EProPlotZoomer.h"
#include "EProPlotCurve.h"
#include "HiHatPositionCurve.h"
#include "HiHatPedalCurve.h"

#include "qwt_symbol.h"
#include "qwt_scale_div.h"
#include "qwt_legend_item.h"

#include <QtCore/QTimer>
#include <QtGui/QMessageBox>
#include <QtGui/QFileDialog>
#include <QtGui/QGridLayout>

#include <boost/filesystem.hpp>
#include <boost/bind.hpp>
#include <boost/format.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string/trim.hpp>
namespace fs = boost::filesystem;

#include <iostream>
#include <algorithm>
#include <fstream>
#include <exception>
#include <cassert>

BOOST_CLASS_EXPORT(HiHatPedalElement); 

std::string MainWindow::APPLICATION_NAME("eProDrums");
std::string MainWindow::APPLICATION_VERSION("0.8.0");

Q_DECLARE_METATYPE(Slot::Ptr)

void MainWindow::toLog(const std::string& szText)
{
	emit sLog(boost::algorithm::trim_copy_if(szText, boost::is_any_of("\n")).c_str());
}

MainWindow::MainWindow():
	_pMutex(new Mutex),
	_pSettings(new Settings()),
    _midiThreadExit(false),
    _midiInHandle(NULL),
    _midiOutHandle(NULL),
    _bConnected(false),
	_currentSlot(_userSettings.configSlots.end()),
    _pHiHatBlueThreshold(new ValueControl("Blue Threshold")),
	_lastHiHatMsgControl(_clock.now(),0x000004B0, 0),
	_bRedrawState(true)
{
	setupUi(this);
	setWindowTitle((boost::format("%s")%APPLICATION_NAME).str().c_str());

	// Setup std::cout redirection
	_streamBuffer.open(StreamSink(boost::bind(&MainWindow::toLog, this, _1)));
	std::cout.rdbuf(&_streamBuffer);
	connect(this, SIGNAL(sLog(const QString&)), textEditLog, SLOT(append(const QString&)));

	// Timer used to redraw plots
	_pRedrawTimer = new QTimer(this);
	connect(_pRedrawTimer, SIGNAL(timeout()), this, SLOT(onRedrawCurves()));

	// Plot
    _pPlot = new EProPlot(this);
    gridLayoutPlot->addWidget(_pPlot, 0,0);

    // Plot zoomer
    _pPlotZoomer = new EProPlotZoomer(_pPlot->canvas());
	connect(_pPlotZoomer, SIGNAL(inRectSelection(bool)), this, SLOT(onRectSelection(bool)));
	connect(_pPlotZoomer, SIGNAL(leftMouseClicked(const QPoint&)), this, SLOT(onLeftMouseClicked(const QPoint&)));
	_pPlotZoomer->setEnabled(true);

    // Curve CC#4
    QColor qBlue(100, 150, 255);
	QColor qHiHatControl(245, 255, 166);
	QColor qHiHatAccel(245, 0, 150);
	_curveHiHatAcceleration = new EProPlotCurve("Hi Hat Acceleration", qHiHatAccel, 1, _pPlot);
	_curveHiHatAcceleration->setMarkerInformationOutlineColor(QColor(Qt::white));
	_curveHiHatAcceleration->setStyle(EProPlotCurve::Lines);
	_curveHiHatAcceleration->setSymbol(new QwtSymbol(QwtSymbol::Ellipse, qHiHatAccel, qHiHatAccel, QSize(2,2)));
	_curveHiHatPosition = new HiHatPositionCurve(_pPlot);
	_curveHiHatPedal = new HiHatPedalCurve(_pPlot);
	_curveHiHat = new EProPlotCurve("Hi Hat", QColor(255, 255, 0), 2, _pPlot);
	_curveHiHat->setStyle(EProPlotCurve::Sticks);
	_curveTom1 = new EProPlotCurve("Tom 1", QColor(255, 255, 0), 2, _pPlot);
	_curveTom1->setStyle(EProPlotCurve::Sticks);
	_curveTom2 = new EProPlotCurve("Tom 2", qBlue, 2, _pPlot);
	_curveTom2->setStyle(EProPlotCurve::Sticks);
	_curveTom3 = new EProPlotCurve("Tom 3", QColor(0, 255, 0), 2, _pPlot);
	_curveTom3->setStyle(EProPlotCurve::Sticks);
	_curveRide = new EProPlotCurve("Ride", qBlue, 2, _pPlot);
	_curveRide->setStyle(EProPlotCurve::Sticks);
	_curveYellowCrash = new EProPlotCurve("Yellow Crash", QColor(255, 255, 0), 2, _pPlot);
	_curveYellowCrash->setStyle(EProPlotCurve::Sticks);
	_curveCrash = new EProPlotCurve("Green Crash", QColor(0, 255, 0), 2, _pPlot);
	_curveCrash->setStyle(EProPlotCurve::Sticks);
	_curveSnare = new EProPlotCurve("Snare", QColor(255, 0, 0), 2, _pPlot);
	_curveSnare->setStyle(EProPlotCurve::Sticks);
	_curveBassPedal = new EProPlotCurve("Bass Drum", QColor(255, 200, 100), 2, _pPlot);
	_curveBassPedal->setStyle(EProPlotCurve::Sticks);

    _pPlot->showAll();

	// By default we hide the hh accel
	setCurveVisibility(_curveHiHatAcceleration, _userSettings.isCurveVisible(UserSettings::CURVE_HIHAT_ACCELERATION));

    // Position Hi Hat pedal control
    _pHiHatBlueThreshold->setToolTip("Hi-hat hits are converted to Ride when the Hi-hat control is greater to this threshold");
    _pHiHatBlueThreshold->showThermo();
    _pHiHatBlueThreshold->setThermoValue(127);
    _pHiHatBlueThreshold->setThermoAlarmLevel(127);
    _pHiHatBlueThreshold->setThermoFillColor(QColor(255,255,0));
    _pHiHatBlueThreshold->setThermoAlarmColor(QColor(0,0,255));
    connect(this, SIGNAL(hiHatPedalControl(int)), _pHiHatBlueThreshold, SLOT(setThermoValue(int)));
    connect(this, SIGNAL(hiHatStartMoving(int, int, int)), this, SLOT(onHiHatStartMoving(int, int, int)));
    connect(this, SIGNAL(hiHatBlueState(bool)), this, SLOT(onHiHatBlueState(bool)));
    connect(this, SIGNAL(footCancelStrategy1Started(int, int, int)), this, SLOT(onFootCancelStrategy1Started(int, int, int)));
    connect(_pHiHatBlueThreshold, SIGNAL(valueChanged(int)), this, SLOT(onHiHatPedalBlueNoteThreshold(int)));
    gridLayoutHiHatControlPos->addWidget(_pHiHatBlueThreshold,0,0);

    // Speed Hi Hat pedal control
    ValueControl* p = new ValueControl("Open Accel", 0, 5000);
	p->setToolTip("Max allowed acceleration (unit/s²) : To be definitively converted to blue cymbal, the current acceleration must be under or equal to this value");
    connect(p, SIGNAL(valueChanged(int)), this, SLOT(onHiHatBlueOpenAccelMax(int)));
    gridLayoutHiHatControlSpeed->addWidget(p,0,0);

    p = new ValueControl("Open Speed", 0, 5000);
	p->setToolTip("Hi-hat open speed threshold (unit/s) : If the current hi-hat speed is greater or equal to this value the hi-hat is converted to a blue cymbal");
    connect(p, SIGNAL(valueChanged(int)), this, SLOT(onHiHatBlueOnSpeedThreshold(int)));
    gridLayoutHiHatControlSpeed->addWidget(p,0,1);

    p = new ValueControl("Open Pos");
	p->setToolTip("If the current hi-hat position is strictly greater [Open Pos] the hi-hat is in open state");
    connect(p, SIGNAL(valueChanged(int)), this, SLOT(onHiHatOpenThreshold(int)));
	gridLayoutHiHatControlSpeed->addWidget(p, 0, 2);

    p = new ValueControl("Close Speed", -5000, 0);
	p->setToolTip("Hi-hat close speed threshold (unit/s) : If the current hi-hat speed is under or equal to this value the hi-hat is converted to a yellow cymbal");
    connect(p, SIGNAL(valueChanged(int)), this, SLOT(onHiHatBlueOffSpeedThreshold(int)));
    gridLayoutHiHatControlSpeed->addWidget(p, 0, 3);

    p = new ValueControl("Close Pos");
	p->setToolTip("If the current hi-hat position is strictly under [Close Pos] the hi-hat is in close state");
    connect(p, SIGNAL(valueChanged(int)), this, SLOT(onHiHatCloseThreshold(int)));
	gridLayoutHiHatControlSpeed->addWidget(p, 0, 4);


    connect(this, SIGNAL(updatePlot(int, int, int, int, int, float, float)), this, SLOT(onUpdatePlot(int, int, int, int, int, float, float)));

	// Buffer and Calibration offset
	lineEditCalibrationOffset->setToolTip("Rock Band calibration offset to apply on both video and audio settings.\nIt's an offset, you have to add this value to your existing settings");
    lineEditCalibrationOffset->setText("0");
	sliderBuffer->setToolTip("Modifying the midi buffer length (ms) affect your Rock Band calibration setting.\nSome features works better with a larger buffer length, a typical value is 35 ms");

	groupBoxMutableCrashSettings->setToolTip("To use this feature it's recommended to set properly the [Simultaneous Hit Time Window] and the global [Buffer Length]");

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

    // Building Cymbals tab
    QWidget* pTabCymbals = tabWidget->widget(0);
    if (pTabCymbals)
    {
        QGridLayout* pLayoutSimHit = dynamic_cast<QGridLayout*>(groupBoxSimHit->layout());
        ValueControl* pSimHitTime = new ValueControl("Sim Hit");
		pSimHitTime->setToolTip("Timing window used to detect a simultaneous hit between cymbals");
		connect(pSimHitTime, SIGNAL(valueChanged(int)), this, SLOT(onSimHitValueChanged(int)));
        pLayoutSimHit->addWidget(pSimHitTime, 0,0);
    }

    // Building Cymbals tab
    QWidget* pFootSplashCancel = tabWidget->widget(1);
    if (pFootSplashCancel)
    {
		groupBoxFootCancel1->setToolTip("This foot cancel strategy doesn't need a buffer length. The cancelling is done by interpreting position, speed and acceleration of the hi-hat");
        ValueControl* pFootCancelAccelLimit = new ValueControl("Accel", -20000, 0);
		pFootCancelAccelLimit->setToolTip("[Optional] Hi-hat close acceleration limit (unit/s²) : The mask time can only begin if the current acceleration is greater or equal to this value");
		connect(pFootCancelAccelLimit, SIGNAL(valueChanged(int)), this, SLOT(onFootCancelAccelLimit(int)));
        gridLayoutFootCancel1->addWidget(pFootCancelAccelLimit, 0,0);

        ValueControl* pFootCancelClosingSpeed = new ValueControl("Speed", -8000, 0);
		pFootCancelClosingSpeed->setToolTip("Hi-hat close speed (unit/s) : The mask time can only begin if the current speed is under or equal this value");
		connect(pFootCancelClosingSpeed, SIGNAL(valueChanged(int)), this, SLOT(onFootCancelClosingSpeed(int)));
        gridLayoutFootCancel1->addWidget(pFootCancelClosingSpeed, 0,1);

        ValueControl* pFootCancelPos = new ValueControl("Pos");
		pFootCancelPos->setToolTip("Hi-Hat position (unit) : The mask time can only begin if the current position is under or equal this value");
		connect(pFootCancelPos, SIGNAL(valueChanged(int)), this, SLOT(onFootCancelPos(int)));
        gridLayoutFootCancel1->addWidget(pFootCancelPos, 0,2);

        ValueControl* pFootCancelPosDiff = new ValueControl("Pos Delta");
		pFootCancelPosDiff->setToolTip("Position delta between the very beginning of the closing movement and the current position (unit) : \nThe mask time can only begin if the position delta is greater or equal this value");
		connect(pFootCancelPosDiff, SIGNAL(valueChanged(int)), this, SLOT(onFootCancelPosDiff(int)));
        gridLayoutFootCancel1->addWidget(pFootCancelPosDiff, 0,3);

        ValueControl* pFootCancelMaskTime = new ValueControl("Mt1", 0, 50);
		pFootCancelMaskTime->setToolTip("Mask time [Mt1] (ms) : During this time if the velocity of a hi-hat hit is under [Vel1] the hi-hat is ignored");
		connect(pFootCancelMaskTime, SIGNAL(valueChanged(int)), this, SLOT(onFootCancelMaskTime(int)));
        gridLayoutFootCancel1->addWidget(pFootCancelMaskTime, 0,4);

        ValueControl* pFootCancelVelocity = new ValueControl("Vel1");
		pFootCancelVelocity->setToolTip("Velocity [Vel1] (unit) : During [Mt1] all hi-hat hits under [Vel1] are ignored");
		connect(pFootCancelVelocity, SIGNAL(valueChanged(int)), this, SLOT(onFootCancelVelocity(int)));
        gridLayoutFootCancel1->addWidget(pFootCancelVelocity, 0,5);

		groupBoxFootCancel2->setToolTip("This foot cancel strategy needs a buffer length but it's easier to tune");
        ValueControl* pBeforeHitMaskTime = new ValueControl("Mt2");
		pBeforeHitMaskTime->setToolTip("Mask time after a Hi-Hat hit. During this time, if a Hi-Hat Pedal is hit and Hi-Hat velocity is under [Vel2], the Hi Hat hit is ignored");
		connect(pBeforeHitMaskTime, SIGNAL(valueChanged(int)), this, SLOT(onBeforeHitMaskTime(int)));
        gridLayoutFootCancel2->addWidget(pBeforeHitMaskTime, 0,0);

        ValueControl* pBeforeHitMaskVelocity = new ValueControl("Vel2");
		pBeforeHitMaskVelocity->setToolTip("During [Mt2] all Hi-Hat hits under [Vel2] are ignored");
		connect(pBeforeHitMaskVelocity, SIGNAL(valueChanged(int)), this, SLOT(onBeforeHitMaskVelocity(int)));
        gridLayoutFootCancel2->addWidget(pBeforeHitMaskVelocity, 0,1);

        ValueControl* pAfterHitMaskTime = new ValueControl("Mt3");
		pAfterHitMaskTime->setToolTip("Mask time after a Hi-Hat Pedal hit. During this time, all Hi-Hat hits are ignored if they are under [Vel3]");
		connect(pAfterHitMaskTime, SIGNAL(valueChanged(int)), this, SLOT(onAfterHitMaskTime(int)));
        gridLayoutFootCancel2->addWidget(pAfterHitMaskTime, 0,2);

        ValueControl* pAfterHitMaskVelocity = new ValueControl("Vel3");
		pAfterHitMaskVelocity->setToolTip("During [Mt3] all Hi-Hat hits under [Vel3] are ignored");
		connect(pAfterHitMaskVelocity, SIGNAL(valueChanged(int)), this, SLOT(onAfterHitMaskVelocity(int)));
        gridLayoutFootCancel2->addWidget(pAfterHitMaskVelocity, 0,3);

		// Cancel while open
        ValueControl* pOpenThreshold = new ValueControl("Thres");
		pOpenThreshold->setToolTip("If the current hi-hat position is greater than [Thres], hits under [Vel4] are ignored");
		connect(pOpenThreshold, SIGNAL(valueChanged(int)), this, SLOT(onOpenCancelHitThreshold(int)));
        gridLayoutCancelOpenHit->addWidget(pOpenThreshold, 0,0);

        ValueControl* pOpenHitVelocity = new ValueControl("Vel4");
		pOpenHitVelocity->setToolTip("If the current hi-hat position is greater than [Thres], hits under [Vel4] are ignored");
		connect(pOpenHitVelocity, SIGNAL(valueChanged(int)), this, SLOT(onOpenCancelHitVelocity(int)));
        gridLayoutCancelOpenHit->addWidget(pOpenHitVelocity, 0,1);
	}

    // Building Flam tab 
    QWidget* pWidgetFlams = tabWidget->widget(2);
    if (pWidgetFlams)
    {
        QGridLayout* pLayoutControls = dynamic_cast<QGridLayout*>(pWidgetFlams->layout());
        if (!pLayoutControls)
        {
            pLayoutControls = new QGridLayout();
            pLayoutControls->setMargin(3);
        }

        // FlamKitControl
        FlamKitControl* pControl = new FlamKitControl(getCurrentSlot()->getPads());

        pLayoutControls->addWidget(pControl, 0,0);
        pWidgetFlams->setLayout(pLayoutControls);
    }

    // Building Ghost tab 
    QWidget* pWidgetGhosts = tabWidget->widget(3);
    if (pWidgetGhosts)
    {
        QGridLayout* pLayoutControls = dynamic_cast<QGridLayout*>(pWidgetGhosts->layout());
        if (!pLayoutControls)
        {
            pLayoutControls = new QGridLayout();
            pLayoutControls->setMargin(3);
        }

        // GhostKitControl Widget
        GhostKitControl* pControl = new GhostKitControl(getCurrentSlot()->getPads());

        pLayoutControls->addWidget(pControl, 0,0);
        pWidgetGhosts->setLayout(pLayoutControls);
    }

    // Get last settings
    const std::string& szMidiIn = _pSettings->getMidiIn();
    const std::string& szMidiOut = _pSettings->getMidiOut();

    int currentMidiIn = -1;
    UINT numMidiIn = midiInGetNumDevs();
    for (UINT i=0;i<numMidiIn;i++)
    {
        MIDIINCAPS midiInCaps;
        MMRESULT res = midiInGetDevCaps(i, &midiInCaps, sizeof(midiInCaps));
		std::string name(midiInCaps.szPname);
        comboBoxMidiIn->blockSignals(true);
        comboBoxMidiIn->addItem(name.c_str(), i);
        comboBoxMidiIn->blockSignals(false);
        if (name == szMidiIn)
        {
            currentMidiIn = i;
        }
    }

    if (currentMidiIn>=0)
    {
        comboBoxMidiIn->setCurrentIndex(currentMidiIn);
    }

    int currentMidiOut = -1;
    UINT numMidiOut = midiOutGetNumDevs();
    for (UINT i=0;i<numMidiOut;i++)
    {
        MIDIOUTCAPS midiOutCaps;
        MMRESULT res = midiOutGetDevCaps(i, &midiOutCaps, sizeof(midiOutCaps));
		std::string name(midiOutCaps.szPname);
        comboBoxMidiOut->blockSignals(true);
        comboBoxMidiOut->addItem(name.c_str(), i);
        comboBoxMidiOut->blockSignals(false);
        if (name == szMidiOut)
        {
            currentMidiOut = i;
        }
    }

    if (currentMidiOut>=0)
    {
        comboBoxMidiOut->setCurrentIndex(currentMidiOut);
    }

    pushButtonStop->setEnabled(false);
    pushButtonStart->setEnabled(false);
    if (numMidiIn && numMidiOut)
    {
        pushButtonStart->setEnabled(true);
    }

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

	// Logs
	groupBoxLogs->setChecked(true);
	checkBoxLogsRawData->setChecked(false);
	checkBoxLogsFilteredData->setChecked(false);
	checkBoxLogsHiHatControl->setChecked(false);
	checkBoxLogsFlams->setChecked(false);
	checkBoxLogsGhostNotes->setChecked(false);
	checkBoxLogsOthers->setChecked(true);
	
    // Run the midi thread
    if (currentMidiIn >=0 && currentMidiOut >=0)
    {
        on_pushButtonStart_clicked();
    }
}

MainWindow::~MainWindow()
{
    stop();
}

void MainWindow::sendMidiMessage(const MidiMessage& midiMessage, bool bForce)
{
	const int DEFAULT_NOTE_MSG_CTRL(4);
    if (!midiMessage.isIgnored() || bForce)
    {
        MMRESULT result = midiOutShortMsg(_midiOutHandle, midiMessage.computeOutputMessage());
		if (result==MMSYSERR_NOERROR)
		{
			if (midiMessage.isControllerMsg() && midiMessage.getOutputNote() == DEFAULT_NOTE_MSG_CTRL)
			{
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
					emit updatePlot(m.getMsgType(), m.getChannel(), m.getOutputNote(), m.getValue(), m.getTimestamp(), m.hiHatSpeed, m.hiHatAcceleration);
				}
			}

			emit updatePlot(midiMessage.getMsgType(), midiMessage.getChannel(), midiMessage.getOutputNote(), midiMessage.getValue(), midiMessage.getTimestamp(), midiMessage.hiHatSpeed, midiMessage.hiHatAcceleration);
		}
		else
		{
			if (_userSettings.isLogs() && _userSettings.isLog(UserSettings::LOG_OTHERS))
			{
				std::cout << "Error can't send midi message : " << midiMessage.str() << " Reason=" << result <<  std::endl;
			}
		}
    }
}

void MainWindow::sendMidiMessages(const MidiMessage::List& midiMessages, bool bForce)
{
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
				midiMessage.changeOutputNote(pPad->getDefaultOutputNote(), false);
				bAddThisMidiMessage = true;
				break;
			}
		}
	}
	else if (midiMessage.isControllerMsg() && midiMessage.getOriginalNote() == _pSettings->getDrumKitMidiMap()->getHiHatControlCC())
	{
		midiMessage.changeOutputNote(4, false);
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

void CALLBACK MidiInProc(
        HMIDIIN hMidiIn,  
        UINT wMsg,        
        DWORD_PTR dwInstance, 
        DWORD_PTR dwParam1,   
        DWORD_PTR dwParam2)
{
	MainWindow* pMainWindow = (MainWindow*)dwInstance;

    switch (wMsg)
    {
    case MIM_DATA:
        {
			MainWindow::Mutex::scoped_lock lock(*(pMainWindow->_pMutex));
			const UserSettings& config = pMainWindow->getConfig();
            MidiMessage midiMessage(pMainWindow->_clock.now(), dwParam1, dwParam2);

			// First filtering, we get only note_on and controller messages
			if (midiMessage.isNoteOnMsg() || midiMessage.isControllerMsg())
			{
				if (midiMessage.isNoteOnMsg() && midiMessage.getValue()==0)
				{
					// Ignore note_on with velocity 0 (it happens...)
					// But we print the msg if logs are on
					if (config.isLogs() && config.isLog(UserSettings::LOG_RAW_DATA))
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
			else if (config.isLogs() && config.isLog(UserSettings::LOG_RAW_DATA))
			{
				midiMessage.print();
			}

            break;
        }
    default:
        {
            break;
        }
    }
}

void MainWindow::midiThread()
{
	MidiMessage::DictHistory lastMsgSent(Pad::TYPE_COUNT);

    while (!_midiThreadExit)
    {
        Mutex::scoped_lock lock(*_pMutex);
        _condition.wait(lock);

        while (!_midiMessages.empty() && !_midiThreadExit)
        {
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

					// If there is an existing delta in position and if the last message got the same
					// timestamp as the current, we need a default deltaTimeInS of 1 ms
					if (deltaPosition!=0 && deltaTimeInS==0.0f)
					{
						deltaTimeInS = 1.0f;
					}

					if (deltaTimeInS!=0.0f)
					{
						currentMsg.hiHatSpeed = float(deltaPosition)/deltaTimeInS;

						// Acceleration computation
						int deltaSpeed = currentMsg.hiHatSpeed - p->getCurrentControlSpeed();
						currentMsg.hiHatAcceleration = float(deltaSpeed)/deltaTimeInS;
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
	// Clearing plots
	clearPlots();
	_pPlot->replot();

    _bConnected = false;
    UINT midiInId = comboBoxMidiIn->itemData(comboBoxMidiIn->currentIndex()).toInt();
    MMRESULT res = midiInOpen(&_midiInHandle, midiInId, (DWORD_PTR)MidiInProc, (DWORD_PTR)this, CALLBACK_FUNCTION);
    if (res!=MMSYSERR_NOERROR)
    {
        QMessageBox::critical(this, "Error", "Cannot open MIDI IN");
    }
    else
    {
        UINT midiOutId = comboBoxMidiOut->itemData(comboBoxMidiOut->currentIndex()).toInt();
        res = midiOutOpen(&_midiOutHandle, midiOutId, NULL, NULL, CALLBACK_NULL);
        if (res!=MMSYSERR_NOERROR)
        {
            QMessageBox::critical(this, "Error", "Cannot open MIDI OUT");
        }
        else
        {
            res = midiInStart(_midiInHandle);
            if (res!=MMSYSERR_NOERROR)
            {
                QMessageBox::critical(this, "Error", "Cannot start MIDI IN");
            }
            else
            {
				_pSettings->setMidiIn(comboBoxMidiIn->currentText().toStdString());
				_pSettings->setMidiOut(comboBoxMidiOut->currentText().toStdString());

				// Thread already started, stopping it?
				if (_midiThread)
				{
					if (_userSettings.isLogs() && _userSettings.isLog(UserSettings::LOG_OTHERS)) { std::cout << "Closing previous Midi thread ..." << std::endl; }
					{
						Mutex::scoped_lock lock(*_pMutex);
						_midiThreadExit = true;
						notify();
					}

					_midiThread->join();
					if (_userSettings.isLogs() && _userSettings.isLog(UserSettings::LOG_OTHERS)) { std::cout << "Previous Midi thread closed" << std::endl; }
				}

                // Initializing Midi in and out
                _bConnected = true;
                _midiThreadExit = false;
                _midiThread = boost::shared_ptr<boost::thread>(new boost::thread(boost::bind(&MainWindow::midiThread, this)));
				if (_userSettings.isLogs() && _userSettings.isLog(UserSettings::LOG_OTHERS))
				{
					std::cout << "New Midi thread started" << std::endl;
				}

                comboBoxMidiIn->setEnabled(false);
                comboBoxMidiOut->setEnabled(false);
                pushButtonStart->setEnabled(false);
                pushButtonStop->setEnabled(true);
			}
        }
    }
}

void MainWindow::stop()
{
	// Exiting midi thread first
	if (_userSettings.isLogs() && _userSettings.isLog(UserSettings::LOG_OTHERS)) { std::cout << "Closing Midi thread ..." << std::endl; }

    {
        Mutex::scoped_lock lock(*_pMutex);
        _midiThreadExit = true;
        notify();
    }

    if (_midiThread)
    {
        _midiThread->join();
		if (_userSettings.isLogs() && _userSettings.isLog(UserSettings::LOG_OTHERS)) { std::cout << "Midi thread closed" << std::endl; }
		_midiThread.reset();
    }

	// Closing midi connection
    if (_bConnected)
    {
        midiInStop(_midiInHandle);
        _bConnected = false;
    }

    if (_midiInHandle)
    {
        midiInClose(_midiInHandle);
    }

    if (_midiOutHandle)
    {
        midiOutClose(_midiOutHandle);
    }
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

    QString filePath = QFileDialog::getSaveFileName(this, "Save configuration file", qszDefaultDir, "Rock Band Pro Drums (*.epd)");
    if (filePath.isNull())
    {
        return;
    }

    saveUserSettings(filePath.toStdString());
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
}

void MainWindow::loadUserSettings(const std::string& szFilePath)
{
	if (!fs::exists(szFilePath))
	{
		return;
	}

    Mutex::scoped_lock lock(*(_pMutex.get()));

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

				// Connections for mutex protection
				pSlot->connectMutex(boost::bind(&MainWindow::lock, this), boost::bind(&MainWindow::unlock, this));

				// Connection between global settings and the slot
				_pSettings->connectDrumKitMidiMapLoaded(boost::bind(&Slot::onDrumKitLoaded, pSlot, _1, _2));

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
			setCurveVisibility(_curveHiHatPosition, _userSettings.isCurveVisible(UserSettings::CURVE_HIHAT_CONTROL));
			setCurveVisibility(_curveHiHatAcceleration, _userSettings.isCurveVisible(UserSettings::CURVE_HIHAT_ACCELERATION));
			setCurveVisibility(_curveHiHat, _userSettings.isCurveVisible(UserSettings::CURVE_HIHAT));
			setCurveVisibility(_curveHiHatPedal, _userSettings.isCurveVisible(UserSettings::CURVE_HIHAT_PEDAL));
			setCurveVisibility(_curveCrash, _userSettings.isCurveVisible(UserSettings::CURVE_CRASH));
			setCurveVisibility(_curveYellowCrash, _userSettings.isCurveVisible(UserSettings::CURVE_YELLOW_CRASH));
			setCurveVisibility(_curveRide, _userSettings.isCurveVisible(UserSettings::CURVE_RIDE));
			setCurveVisibility(_curveTom1, _userSettings.isCurveVisible(UserSettings::CURVE_TOM1));
			setCurveVisibility(_curveTom2, _userSettings.isCurveVisible(UserSettings::CURVE_TOM2));
			setCurveVisibility(_curveTom3, _userSettings.isCurveVisible(UserSettings::CURVE_TOM3));
			setCurveVisibility(_curveSnare, _userSettings.isCurveVisible(UserSettings::CURVE_SNARE));
			setCurveVisibility(_curveBassPedal, _userSettings.isCurveVisible(UserSettings::CURVE_BASS_PEDAL));
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
		// On save we udpate all curve visibility
		_userSettings.setCurveVisibility(UserSettings::CURVE_HIHAT_CONTROL, _curveHiHatPosition->isVisible());
		_userSettings.setCurveVisibility(UserSettings::CURVE_HIHAT_ACCELERATION, _curveHiHatAcceleration->isVisible());
		_userSettings.setCurveVisibility(UserSettings::CURVE_HIHAT, _curveHiHat->isVisible());
		_userSettings.setCurveVisibility(UserSettings::CURVE_HIHAT_PEDAL, _curveHiHatPedal->isVisible());
		_userSettings.setCurveVisibility(UserSettings::CURVE_CRASH, _curveCrash->isVisible());
		_userSettings.setCurveVisibility(UserSettings::CURVE_YELLOW_CRASH, _curveYellowCrash->isVisible());
		_userSettings.setCurveVisibility(UserSettings::CURVE_RIDE, _curveRide->isVisible());
		_userSettings.setCurveVisibility(UserSettings::CURVE_TOM1, _curveTom1->isVisible());
		_userSettings.setCurveVisibility(UserSettings::CURVE_TOM2, _curveTom2->isVisible());
		_userSettings.setCurveVisibility(UserSettings::CURVE_TOM3, _curveTom3->isVisible());
		_userSettings.setCurveVisibility(UserSettings::CURVE_SNARE, _curveSnare->isVisible());
		_userSettings.setCurveVisibility(UserSettings::CURVE_BASS_PEDAL, _curveBassPedal->isVisible());

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

    Mutex::scoped_lock lock(*_pMutex);
    _userSettings.bufferLength = value;
    _calibrationOffset = _userSettings.bufferLength;
    lineEditCalibrationOffset->setText((boost::format("%d")%_calibrationOffset).str().c_str());
}

void MainWindow::on_spinBoxCC_valueChanged(int value)
{
    Mutex::scoped_lock lock(*_pMutex);
	_pSettings->getDrumKitMidiMap()->setHiHatControlCC(value);
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
    Mutex::scoped_lock lock(*_pMutex);
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
    Mutex::scoped_lock lock(*_pMutex);
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
			pDuplicated->connectMutex(boost::bind(&MainWindow::lock, this), boost::bind(&MainWindow::unlock, this));

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
    Mutex::scoped_lock lock(*_pMutex);
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
    pElHihat->setFlamTimeWindow2(0);
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

	pDefaultSlot->connectMutex(boost::bind(&MainWindow::lock, this), boost::bind(&MainWindow::unlock, this));
	_pSettings->connectDrumKitMidiMapLoaded(boost::bind(&Slot::onDrumKitLoaded, pDefaultSlot, _1, _2));
	_pSettings->reloadDrumKitMidiMap();

    return pDefaultSlot;
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
			const Slot::Ptr& pCurrentSlot = getCurrentSlot();

            checkBoxMutableWithCrash->setChecked(pCurrentSlot->isMutableCrash(Slot::CRASH_CRASH));
            checkBoxMutableWithRide->setChecked(pCurrentSlot->isMutableCrash(Slot::CRASH_RIDE));
            checkBoxMutableWithSnare->setChecked(pCurrentSlot->isMutableCrash(Slot::CRASH_SNARE));
            checkBoxMutableWithTom2->setChecked(pCurrentSlot->isMutableCrash(Slot::CRASH_TOM2));
            checkBoxMutableWithTom3->setChecked(pCurrentSlot->isMutableCrash(Slot::CRASH_TOM3));

			const Pad::List& pads = pCurrentSlot->getPads();

			const HiHatPedalElement::Ptr& pElHihatPedal = boost::dynamic_pointer_cast<HiHatPedalElement>(pads[Pad::HIHAT_PEDAL]);

            QWidget* pTabCymbals = tabWidget->widget(0);
            if (pTabCymbals)
            {
                QGridLayout* pLayoutSimHit = dynamic_cast<QGridLayout*>(groupBoxSimHit->layout());
				ValueControl* pSimHitTime = dynamic_cast<ValueControl*>(pLayoutSimHit->itemAt(1)->widget());
				pSimHitTime->setValue(pCurrentSlot->getCymbalSimHitWindow());

                _pHiHatBlueThreshold->setValue(pElHihatPedal->getControlPosThreshold());

				ValueControl* p = dynamic_cast<ValueControl*>(gridLayoutHiHatControlSpeed->itemAt(0)->widget());
				p->setValue(pElHihatPedal->getOpenAccelMax());

				p = dynamic_cast<ValueControl*>(gridLayoutHiHatControlSpeed->itemAt(1)->widget());
				p->setValue(pElHihatPedal->getControlSpeedOn());

				p = dynamic_cast<ValueControl*>(gridLayoutHiHatControlSpeed->itemAt(2)->widget());
				p->setValue(pElHihatPedal->getOpenPositionThresold());

				p = dynamic_cast<ValueControl*>(gridLayoutHiHatControlSpeed->itemAt(3)->widget());
				p->setValue(pElHihatPedal->getControlSpeedOff());

				p = dynamic_cast<ValueControl*>(gridLayoutHiHatControlSpeed->itemAt(4)->widget());
				p->setValue(pElHihatPedal->getClosePositionThresold());

				groupBoxHiHatControlPos->setChecked(pElHihatPedal->isControlPosActivated());
				groupBoxHiHatControlSpeed->setChecked(pElHihatPedal->isControlSpeedActivated());
			}

            QWidget* pFootSplashCancel = tabWidget->widget(1);
            if (pFootSplashCancel)
            {
				ValueControl* pFootCancelAccelLimit = dynamic_cast<ValueControl*>(gridLayoutFootCancel1->itemAt(0)->widget());
				ValueControl* pFootCancelClosingSpeed = dynamic_cast<ValueControl*>(gridLayoutFootCancel1->itemAt(1)->widget());
				ValueControl* pFootCancelPos = dynamic_cast<ValueControl*>(gridLayoutFootCancel1->itemAt(2)->widget());
				ValueControl* pFootCancelPosDiff = dynamic_cast<ValueControl*>(gridLayoutFootCancel1->itemAt(3)->widget());
				ValueControl* pFootCancelMaskTime = dynamic_cast<ValueControl*>(gridLayoutFootCancel1->itemAt(4)->widget());
				ValueControl* pFootCancelVelocity = dynamic_cast<ValueControl*>(gridLayoutFootCancel1->itemAt(5)->widget());
				pFootCancelAccelLimit->setValue(pElHihatPedal->getFootCancelAccelLimit());
				pFootCancelClosingSpeed->setValue(pElHihatPedal->getFootCancelClosingSpeed());
				pFootCancelPos->setValue(pElHihatPedal->getFootCancelPos());
				pFootCancelPosDiff->setValue(pElHihatPedal->getFootCancelPosDiff());
				pFootCancelMaskTime->setValue(pElHihatPedal->getFootCancelMaskTime());
				pFootCancelVelocity->setValue(pElHihatPedal->getFootCancelVelocity());
				groupBoxFootCancel1->setChecked(pElHihatPedal->isFootCancelStrategy1Activated());
				groupBoxFootCancel2->setChecked(pElHihatPedal->isFootCancelStrategy2Activated());
				groupBoxCancelOpenHit->setChecked(pElHihatPedal->isCancelOpenHitActivated());

				ValueControl* pBeforeHitMaskTime = dynamic_cast<ValueControl*>(gridLayoutFootCancel2->itemAt(0)->widget());
				ValueControl* pBeforeHitMaskVelocity = dynamic_cast<ValueControl*>(gridLayoutFootCancel2->itemAt(1)->widget());
				ValueControl* pAfterHitMaskTime = dynamic_cast<ValueControl*>(gridLayoutFootCancel2->itemAt(2)->widget());
				ValueControl* pAfterHitMaskVelocity = dynamic_cast<ValueControl*>(gridLayoutFootCancel2->itemAt(3)->widget());
				pBeforeHitMaskTime->setValue(pElHihatPedal->getBeforeHitMaskTime());
				pBeforeHitMaskVelocity->setValue(pElHihatPedal->getBeforeHitMaskVelocity());
				pAfterHitMaskTime->setValue(pElHihatPedal->getAfterHitMaskTime());
				pAfterHitMaskVelocity->setValue(pElHihatPedal->getAfterHitMaskVelocity());

				ValueControl* pOpenThreshold = dynamic_cast<ValueControl*>(gridLayoutCancelOpenHit->itemAt(0)->widget());
				ValueControl* pOpenHitVelocity = dynamic_cast<ValueControl*>(gridLayoutCancelOpenHit->itemAt(1)->widget());
				pOpenThreshold->setValue(pElHihatPedal->getCancelOpenHitThreshold());
				pOpenHitVelocity->setValue(pElHihatPedal->getCancelOpenHitVelocity());
			}

            QWidget* pTabFlams = tabWidget->widget(2);
            if (pTabFlams)
            {
                QGridLayout* pLayoutControls = dynamic_cast<QGridLayout*>(pTabFlams->layout());
                QLayoutItem* p = pLayoutControls->itemAt(0);
                FlamKitControl* pControl = dynamic_cast<FlamKitControl*>(p->widget());
                pControl->setDrumKit(pCurrentSlot->getPads());
            }

            QWidget* pTabGhosts = tabWidget->widget(3);
            if (pTabGhosts)
            {
                QGridLayout* pLayoutControls = dynamic_cast<QGridLayout*>(pTabGhosts->layout());
                QLayoutItem* p = pLayoutControls->itemAt(0);
                GhostKitControl* pControl = dynamic_cast<GhostKitControl*>(p->widget());
                pControl->setDrumKit(pCurrentSlot->getPads());
            }

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

void MainWindow::on_checkBoxMutableWithCrash_toggled(bool checked)
{
    Mutex::scoped_lock lock(*_pMutex);
    getCurrentSlot()->setMutableCrash(Slot::CRASH_CRASH, checked);
}

void MainWindow::on_checkBoxMutableWithRide_toggled(bool checked)
{
    Mutex::scoped_lock lock(*_pMutex);
    getCurrentSlot()->setMutableCrash(Slot::CRASH_RIDE, checked);
}

void MainWindow::on_checkBoxMutableWithSnare_toggled(bool checked)
{
    Mutex::scoped_lock lock(*_pMutex);
    getCurrentSlot()->setMutableCrash(Slot::CRASH_SNARE, checked);
}

void MainWindow::on_checkBoxMutableWithTom2_toggled(bool checked)
{
    Mutex::scoped_lock lock(*_pMutex);
    getCurrentSlot()->setMutableCrash(Slot::CRASH_TOM2, checked);
}

void MainWindow::on_checkBoxMutableWithTom3_toggled(bool checked)
{
    Mutex::scoped_lock lock(*_pMutex);
    getCurrentSlot()->setMutableCrash(Slot::CRASH_TOM3, checked);
}

void MainWindow::onSimHitValueChanged(int value)
{
    Mutex::scoped_lock lock(*_pMutex);
    getCurrentSlot()->setCymbalSimHitWindow(value);
}

void MainWindow::onBeforeHitMaskTime(int value)
{
    Mutex::scoped_lock lock(*_pMutex);
	const HiHatPedalElement::Ptr& pElHihatPedal = boost::dynamic_pointer_cast<HiHatPedalElement>(getCurrentSlot()->getPads()[Pad::HIHAT_PEDAL]);
	pElHihatPedal->setBeforeHitMaskTime(value);
	_curveHiHatPedal->changeFootCancelStrategy2MaskTime2(value);
}

void MainWindow::onBeforeHitMaskVelocity(int value)
{
    Mutex::scoped_lock lock(*_pMutex);
	const HiHatPedalElement::Ptr& pElHihatPedal = boost::dynamic_pointer_cast<HiHatPedalElement>(getCurrentSlot()->getPads()[Pad::HIHAT_PEDAL]);
	pElHihatPedal->setBeforeHitMaskVelocity(value);
	_curveHiHatPedal->changeFootCancelStrategy2Velocity2(value);
}

void MainWindow::onAfterHitMaskTime(int value)
{
    Mutex::scoped_lock lock(*_pMutex);
	const HiHatPedalElement::Ptr& pElHihatPedal = boost::dynamic_pointer_cast<HiHatPedalElement>(getCurrentSlot()->getPads()[Pad::HIHAT_PEDAL]);
	pElHihatPedal->setAfterHitMaskTime(value);
	_curveHiHatPedal->changeFootCancelStrategy2MaskTime3(value);
}

void MainWindow::onAfterHitMaskVelocity(int value)
{
    Mutex::scoped_lock lock(*_pMutex);
	const HiHatPedalElement::Ptr& pElHihatPedal = boost::dynamic_pointer_cast<HiHatPedalElement>(getCurrentSlot()->getPads()[Pad::HIHAT_PEDAL]);
	pElHihatPedal->setAfterHitMaskVelocity(value);
	_curveHiHatPedal->changeFootCancelStrategy2Velocity3(value);
}

void MainWindow::on_listWidgetSlots_itemChanged(QListWidgetItem* pItem)
{
    Mutex::scoped_lock lock(*_pMutex);
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

void MainWindow::on_actionAbout_triggered()
{
	boost::format fmtMsg("%s (beta) v%s");
	fmtMsg%APPLICATION_NAME%APPLICATION_VERSION;
	DialogAbout dlgAbout(this, fmtMsg.str(), "FromtonRouge");
	dlgAbout.exec();
}

void MainWindow::onHiHatPedalBlueNoteThreshold(int threshold)
{
    Mutex::scoped_lock lock(*_pMutex);
	const HiHatPedalElement::Ptr& pElHihatPedal = boost::dynamic_pointer_cast<HiHatPedalElement>(getCurrentSlot()->getPads()[Pad::HIHAT_PEDAL]);
	pElHihatPedal->setControlPosThreshold(threshold);
    _pHiHatBlueThreshold->setThermoAlarmLevel(threshold);
}

void MainWindow::onHiHatBlueOpenAccelMax(int accel)
{
    Mutex::scoped_lock lock(*_pMutex);
	const HiHatPedalElement::Ptr& pElHihatPedal = boost::dynamic_pointer_cast<HiHatPedalElement>(getCurrentSlot()->getPads()[Pad::HIHAT_PEDAL]);
	pElHihatPedal->setOpenAccelMax(accel);
}

void MainWindow::onHiHatBlueOnSpeedThreshold(int threshold)
{
    Mutex::scoped_lock lock(*_pMutex);
	const HiHatPedalElement::Ptr& pElHihatPedal = boost::dynamic_pointer_cast<HiHatPedalElement>(getCurrentSlot()->getPads()[Pad::HIHAT_PEDAL]);
	pElHihatPedal->setControlSpeedOn(threshold);
}

void MainWindow::onHiHatBlueOffSpeedThreshold(int threshold)
{
    Mutex::scoped_lock lock(*_pMutex);
	const HiHatPedalElement::Ptr& pElHihatPedal = boost::dynamic_pointer_cast<HiHatPedalElement>(getCurrentSlot()->getPads()[Pad::HIHAT_PEDAL]);
	pElHihatPedal->setControlSpeedOff(threshold);
}

void MainWindow::clearPlots()
{
	_pPlotZoomer->moveWindow(0, 5*1000);

	_curveHiHatPosition->showMarkers(false);
	_curveHiHatAcceleration->showMarkers(false);

	_pPlot->clear();
}

void MainWindow::onUpdatePlot(int msgType, int msgChannel, int msgNote, int msgVelocity, int timestamp, float hiHatControlSpeed, float hiHatAcceleration)
{
	int plotTimeWindow = 5*1000;
	if (timestamp>plotTimeWindow)
	{
		_pPlotZoomer->moveWindow(timestamp-plotTimeWindow, plotTimeWindow);
	}
	else
	{
		_pPlotZoomer->moveWindow(0, plotTimeWindow);
	}

    // CC#4
    if (msgType == 11)
    {
		int hiHatLevel(127-msgVelocity);
		boost::any userData(std::make_pair(hiHatControlSpeed, hiHatAcceleration));
		_curveHiHatPosition->add(QPointF(timestamp, hiHatLevel), userData);
		_curveHiHatAcceleration->add(QPointF(timestamp, hiHatAcceleration));
		if (_userSettings.isLogs() && _userSettings.isLog(UserSettings::LOG_HIHAT_CONTROL))
		{
			std::cout << (boost::format("%d (unit), %.3f (unit/s), %.3f (unit/ss)")%hiHatLevel%hiHatControlSpeed%hiHatAcceleration) << std::endl;
		}
    }
    else if (msgType == 9)
    {
		QPointF point(timestamp, msgVelocity);
	
        switch (msgNote)
        {
		case Pad::NOTE_SNARE:
            {
				_curveSnare->add(point);
                break;
            }

		case Pad::NOTE_TOM1:
            {
				_curveTom1->add(point);
                break;
            }

		case Pad::NOTE_TOM2:
            {
				_curveTom2->add(point);
                break;
            }

		case Pad::NOTE_TOM3:
            {
				_curveTom3->add(point);
                break;
            }

		case Pad::NOTE_HIHAT:
            {
				_curveHiHat->add(point);
                break;
            }

		case Pad::NOTE_HIHAT_PEDAL:
            {
				ValueControl* pMt2 = dynamic_cast<ValueControl*>(gridLayoutFootCancel2->itemAt(0)->widget());
				ValueControl* pVel2 = dynamic_cast<ValueControl*>(gridLayoutFootCancel2->itemAt(1)->widget());
				ValueControl* pMt3 = dynamic_cast<ValueControl*>(gridLayoutFootCancel2->itemAt(2)->widget());
				ValueControl* pVel3 = dynamic_cast<ValueControl*>(gridLayoutFootCancel2->itemAt(3)->widget());

				_curveHiHatPedal->add(point, pMt2->getValue(), pVel2->getValue(), pMt3->getValue(), pVel3->getValue());
                break;
            }

		case Pad::NOTE_RIDE:
            {
				_curveRide->add(point);
                break;
            }

		case Pad::NOTE_CRASH3:
			{
				_curveYellowCrash->add(point);
				break;
			}

		case Pad::NOTE_CRASH1:
		case Pad::NOTE_CRASH2:
            {
				_curveCrash->add(point);
                break;
            }

		case Pad::NOTE_BASS_DRUM:
            {
				_curveBassPedal->add(point);
                break;
            }

        default:
            {
                break;
            }
        }
    }

	// Redraw order, plots are redrawn after an idle time
	if (_bRedrawState)
	{
		_pRedrawTimer->start(25);
	}
}

void MainWindow::onRectSelection(bool bState)
{
	if (bState)
	{
		statusBar()->showMessage("[Ctrl + Left Click] to zoom");
	}
	else
	{
		statusBar()->showMessage("");
	}
}

void MainWindow::on_groupBoxLogs_toggled(bool checked)
{
	Mutex::scoped_lock lock(*_pMutex);
	_userSettings.setLog(UserSettings::LOG_ACTIVATED, checked);
}

void MainWindow::on_checkBoxLogsRawData_toggled(bool checked)
{
	Mutex::scoped_lock lock(*_pMutex);
	_userSettings.setLog(UserSettings::LOG_RAW_DATA, checked);
}

void MainWindow::on_checkBoxLogsFilteredData_toggled(bool checked)
{
	Mutex::scoped_lock lock(*_pMutex);
	_userSettings.setLog(UserSettings::LOG_FILTERED_DATA, checked);
}

void MainWindow::on_checkBoxLogsHiHatControl_toggled(bool checked)
{
	Mutex::scoped_lock lock(*_pMutex);
	_userSettings.setLog(UserSettings::LOG_HIHAT_CONTROL, checked);
}

void MainWindow::on_checkBoxLogsFlams_toggled(bool checked)
{
	Mutex::scoped_lock lock(*_pMutex);
	_userSettings.setLog(UserSettings::LOG_FLAMS, checked);
}

void MainWindow::on_checkBoxLogsGhostNotes_toggled(bool checked)
{
	Mutex::scoped_lock lock(*_pMutex);
	_userSettings.setLog(UserSettings::LOG_GHOST_NOTES, checked);
}

void MainWindow::on_checkBoxLogsOthers_toggled(bool checked)
{
	Mutex::scoped_lock lock(*_pMutex);
	_userSettings.setLog(UserSettings::LOG_OTHERS, checked);
}

void MainWindow::on_pushButtonClearLogs_clicked(bool)
{
	textEditLog->clear();
}

void MainWindow::computeMessage(MidiMessage& currentMsg, MidiMessage::DictHistory& lastMsgSent)
{
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
	bool mutableCrashWithCrash = getCurrentSlot()->isMutableCrash(Slot::CRASH_CRASH);
	bool mutableCrashWithRide = getCurrentSlot()->isMutableCrash(Slot::CRASH_RIDE);
	bool mutableCrashWithSnare = getCurrentSlot()->isMutableCrash(Slot::CRASH_SNARE);
	bool mutableCrashWithTom2 = getCurrentSlot()->isMutableCrash(Slot::CRASH_TOM2);
	bool mutableCrashWithTom3 = getCurrentSlot()->isMutableCrash(Slot::CRASH_TOM3);
	bool bHasNextMidiMessage = !_midiMessages.empty();

	const int DEFAULT_NOTE_MSG_CTRL(4);
	if (currentMsg.isControllerMsg() && currentMsg.getOutputNote() == DEFAULT_NOTE_MSG_CTRL)
	{
		int currentControlPos = pElHihatPedal->getCurrentControlPos();
		if (pElHihatPedal->isControlPosActivated())
		{
			if (currentControlPos > pElHihatPedal->getControlPosThreshold())
			{
				pElHihatPedal->setBlue(true);
			}
			else if (!pElHihatPedal->isControlSpeedActivated())
			{
				pElHihatPedal->setBlue(false);
			}
		}

		float currentAccel = pElHihatPedal->getCurrentControlAcceleration();
		if (pElHihatPedal->isControlSpeedActivated())
		{
			if (pElHihatPedal->getCurrentControlSpeed() > 0)
			{
				// Hi-Hat opening
				if (
						pElHihatPedal->getCurrentControlSpeed() >= pElHihatPedal->getControlSpeedOn() &&
						currentAccel <= pElHihatPedal->getOpenAccelMax()
				   )
				{
					pElHihatPedal->setBlue(true);
				}
			}
			else if (pElHihatPedal->getCurrentControlSpeed() < 0)
			{
				// Hi-Hat closing
				if (pElHihatPedal->getCurrentControlSpeed() <= pElHihatPedal->getControlSpeedOff())
				{
					// Hi Hat closing and the close speed is reached
					pElHihatPedal->setBlue(false);
				}
				else if (pElHihatPedal->isControlPosActivated() && currentControlPos <= pElHihatPedal->getControlPosThreshold())
				{
					// Closing speed not reached but the position off threshold is reach
					pElHihatPedal->setBlue(false);
				}
			}
			else
			{
				// current speed == 0 => nothing we keep the last blue state (on or off)
			}

			// Forcing open/close state
			if (currentControlPos < pElHihatPedal->getClosePositionThresold())
			{
				// Force close state from position info
				pElHihatPedal->setBlue(false);
			}
			else if (currentControlPos > pElHihatPedal->getOpenPositionThresold())
			{
				// Force open state from position info
				pElHihatPedal->setBlue(true);
			}
		}

		emit hiHatBlueState(pElHihatPedal->isBlue());

		if (pElHihatPedal->isFootCancelStrategy1Activated())
		{
			int currentTime = currentMsg.getTimestamp();
			int posDiff = pElHihatPedal->getPositionOnCloseBegin()-currentControlPos;
			float speed = pElHihatPedal->getCurrentControlSpeed();

			if ( 
					currentTime > pElHihatPedal->getFootCancelTimeLimit() && 
					currentAccel >= pElHihatPedal->getFootCancelAccelLimit() &&
					speed <= pElHihatPedal->getFootCancelClosingSpeed() &&
					currentControlPos <= pElHihatPedal->getFootCancelPos() &&
					posDiff >= pElHihatPedal->getFootCancelPosDiff()
			   )
			{
				int cancelMaskTime = pElHihatPedal->getFootCancelMaskTime();
				pElHihatPedal->setFootCancelTimeLimit(currentTime+cancelMaskTime);
				
				emit footCancelStrategy1Started(currentTime, cancelMaskTime, pElHihatPedal->getFootCancelVelocity());

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
		switch (currentMsg.getOutputNote())
		{
		case Pad::NOTE_HIHAT_PEDAL:
			{
				if (bHasNextMidiMessage && pElHihatPedal->isFootCancelStrategy2Activated())
				{
					// VH-11 filtering (do Do Hast for validation)
					// For Roland VH-11 and VH-12 Hi hat pedal
					MidiMessage* pNextHiHat = getNextMessage(pElHihat);
					if (pNextHiHat && currentMsg.isInTimeWindow(*pNextHiHat, pElHihatPedal->getAfterHitMaskTime()) &&
							pNextHiHat->getValue() < pElHihatPedal->getAfterHitMaskVelocity())
					{
						pNextHiHat->ignore(MidiMessage::IGNORED_BECAUSE_FOOT_CANCEL);
					}
				}
				break;
			}

		case Pad::NOTE_HIHAT:
			{
				int currentControlPos = pElHihatPedal->getCurrentControlPos();
				if ( pElHihatPedal->isCancelOpenHitActivated()
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
					currentMsg.changeOutputNote(pElRide->getDefaultOutputNote());
				}

				if (pElHihatPedal->isFootCancelStrategy2Activated())
				{
					if (bHasNextMidiMessage)
					{
						// VH-11 filtering. Check the next hh pedal hit
						MidiMessage* pNextHiHatPedal = getNextMessage(pElHihatPedal);
						if (pNextHiHatPedal && currentMsg.isInTimeWindow(*pNextHiHatPedal, pElHihatPedal->getBeforeHitMaskTime()) &&
								currentMsg.getValue() < pElHihatPedal->getBeforeHitMaskVelocity() )
						{
							currentMsg.ignore(MidiMessage::IGNORED_BECAUSE_FOOT_CANCEL);
						}
					}
					else
					{
						if (!lastMsgSent[Pad::HIHAT_PEDAL].empty())
						{
							const MidiMessage& rLastHiHatPedal = lastMsgSent[Pad::HIHAT_PEDAL].front();
							if (pElHihatPedal->isA(rLastHiHatPedal.getOriginalNote()))
							{
								if (currentMsg.isInTimeWindow(rLastHiHatPedal, pElHihatPedal->getAfterHitMaskTime()) &&
										currentMsg.getValue() < pElHihatPedal->getAfterHitMaskVelocity())
								{
									currentMsg.ignore(MidiMessage::IGNORED_BECAUSE_FOOT_CANCEL);
								}
							}
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

		case Pad::NOTE_CRASH1:
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
					currentMsg.changeOutputNote(pElHihat->getDefaultOutputNote());
				}
				else if (mutableCrashWithRide && pNextOrPreviousRIDE && currentMsg.isInTimeWindow(*pNextOrPreviousRIDE, cymbalsSimHitWindow))
				{
					currentMsg.changeOutputNote(pElHihat->getDefaultOutputNote());
				}
				else if (mutableCrashWithSnare && pNextOrPreviousSNARE && currentMsg.isInTimeWindow(*pNextOrPreviousSNARE, cymbalsSimHitWindow))
				{
					currentMsg.changeOutputNote(pElHihat->getDefaultOutputNote());
				}
				else if (mutableCrashWithTom2 && pNextOrPreviousTOM2 && currentMsg.isInTimeWindow(*pNextOrPreviousTOM2, cymbalsSimHitWindow))
				{
					currentMsg.changeOutputNote(pElHihat->getDefaultOutputNote());
				}
				else if (mutableCrashWithTom3 && pNextOrPreviousTOM3 && currentMsg.isInTimeWindow(*pNextOrPreviousTOM3, cymbalsSimHitWindow))
				{
					currentMsg.changeOutputNote(pElHihat->getDefaultOutputNote());
				}
				else
				{
					// Flam and ghost
					sendMidiMessages(pElCrash1->applyFlamAndGhost(getCurrentSlot()->getPads(), lastMsgSent, &currentMsg, getNextMessage(pElCrash1)), true);
				}
				break;
			}

		case Pad::NOTE_CRASH3:
			{
				// Flam and ghost
				sendMidiMessages(pElCrash3->applyFlamAndGhost(getCurrentSlot()->getPads(), lastMsgSent, &currentMsg, getNextMessage(pElCrash3)), true);
				break;
			}

		case Pad::NOTE_CRASH2:
			{
				MidiMessage* pPreviousMutableCrash = NULL;
				if (!lastMsgSent[Pad::CRASH1].empty())
				{
					pPreviousMutableCrash = getCurrentSlot()->getPads()[Pad::CRASH1]->isA(lastMsgSent[Pad::CRASH1].front().getOriginalNote())?&(lastMsgSent[Pad::CRASH1].front()):NULL;
				}

				if (mutableCrashWithCrash && pPreviousMutableCrash && !pPreviousMutableCrash->isAlreadyModified() && currentMsg.isInTimeWindow(*pPreviousMutableCrash, cymbalsSimHitWindow))
				{
					// Previous was a mutable crash, if the mutable was not changed we have to change the CRASH2 to yellow
					currentMsg.changeOutputNote(pElHihat->getDefaultOutputNote());
				}
				else
				{
					// Flam and ghost
					sendMidiMessages(pElCrash2->applyFlamAndGhost(getCurrentSlot()->getPads(), lastMsgSent, &currentMsg, getNextMessage(pElCrash2)), true);
				}
				break;
			}

		case Pad::NOTE_RIDE:
			{
				MidiMessage* pNextMutableCrash = getNextMessage(pElCrash1);
				if (mutableCrashWithRide && pNextMutableCrash && currentMsg.isInTimeWindow(*pNextMutableCrash, cymbalsSimHitWindow))
				{
					// Yellow Crash
					pNextMutableCrash->changeOutputNote(pElHihat->getDefaultOutputNote());
				}
				else
				{
					// Flam and ghost
					sendMidiMessages(pElRide->applyFlamAndGhost(getCurrentSlot()->getPads(), lastMsgSent, &currentMsg, getNextMessage(pElRide)), true);
				}
				break;
			}

		case Pad::NOTE_SNARE:
			{
				MidiMessage* pNextMutableCrash = getNextMessage(pElCrash1);
				if (mutableCrashWithSnare && pNextMutableCrash && currentMsg.isInTimeWindow(*pNextMutableCrash, cymbalsSimHitWindow))
				{
					// Yellow Crash
					pNextMutableCrash->changeOutputNote(pElHihat->getDefaultOutputNote());
				}
				else
				{
					// Flam and ghost
					sendMidiMessages(pElSnare->applyFlamAndGhost(getCurrentSlot()->getPads(), lastMsgSent, &currentMsg, getNextMessage(pElSnare)), true);
				}
				break;
			}

		case Pad::NOTE_TOM1:
			{
				// Flam and ghost
				sendMidiMessages(pElTom1->applyFlamAndGhost(getCurrentSlot()->getPads(), lastMsgSent, &currentMsg, getNextMessage(pElTom1)), true);
				break;
			}

		case Pad::NOTE_TOM2:
			{
				MidiMessage* pNextMutableCrash = getNextMessage(pElCrash1);
				if (mutableCrashWithTom2 && pNextMutableCrash && currentMsg.isInTimeWindow(*pNextMutableCrash, cymbalsSimHitWindow))
				{
					// Yellow crash
					pNextMutableCrash->changeOutputNote(pElHihat->getDefaultOutputNote());
				}
				else
				{
					// Flam and ghost
					sendMidiMessages(pElTom2->applyFlamAndGhost(getCurrentSlot()->getPads(), lastMsgSent, &currentMsg, getNextMessage(pElTom2)), true);
				}
				break;
			}

		case Pad::NOTE_TOM3:
			{
				MidiMessage* pNextMutableCrash = getNextMessage(pElCrash1);
				if (mutableCrashWithTom3 && pNextMutableCrash && currentMsg.isInTimeWindow(*pNextMutableCrash, cymbalsSimHitWindow))
				{
					// Yellow crash
					pNextMutableCrash->changeOutputNote(pElHihat->getDefaultOutputNote());
				}
				else
				{
					// Flam and ghost
					sendMidiMessages(pElTom3->applyFlamAndGhost(getCurrentSlot()->getPads(), lastMsgSent, &currentMsg, getNextMessage(pElTom3)), true);
				}
				break;
			}

		case Pad::NOTE_BASS_DRUM:
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

void MainWindow::onLeftMouseClicked(const QPoint& pos)
{
	int index = _curveHiHatPosition->updateMarkers(pos);
	if (index>=0)
	{
		int position(_curveHiHatPosition->y(index));
		std::pair<float, float> speedAndAccel = boost::any_cast< std::pair<float, float> >(_curveHiHatPosition->getUserData(index));

		boost::format fmtControl("%d (unit), %.3f (unit/s), %.3f (unit/s²)");
		_curveHiHatPosition->setMarkerInformationLabel((fmtControl%position%speedAndAccel.first%speedAndAccel.second).str());
	}

	index = _curveHiHatAcceleration->updateMarkers(pos);
	if (index>=0)
	{
		float accel = _curveHiHatAcceleration->y(index);

		boost::format fmtAccel("%.3f (unit/s²)");
		_curveHiHatAcceleration->setMarkerInformationLabel((fmtAccel%accel).str());
	}

	_pPlot->replot();
}

void MainWindow::on_groupBoxHiHatControlPos_toggled(bool checked)
{
    Mutex::scoped_lock lock(*_pMutex);
	const HiHatPedalElement::Ptr& pElHihatPedal = boost::dynamic_pointer_cast<HiHatPedalElement>(getCurrentSlot()->getPads()[Pad::HIHAT_PEDAL]);
	pElHihatPedal->setControlPosActivation(checked);
}

void MainWindow::on_groupBoxHiHatControlSpeed_toggled(bool checked)
{
    Mutex::scoped_lock lock(*_pMutex);
	const HiHatPedalElement::Ptr& pElHihatPedal = boost::dynamic_pointer_cast<HiHatPedalElement>(getCurrentSlot()->getPads()[Pad::HIHAT_PEDAL]);
	pElHihatPedal->setControlSpeedActivation(checked);
}

void MainWindow::onFootCancelAccelLimit(int value)
{
    Mutex::scoped_lock lock(*_pMutex);
	const HiHatPedalElement::Ptr& pElHihatPedal = boost::dynamic_pointer_cast<HiHatPedalElement>(getCurrentSlot()->getPads()[Pad::HIHAT_PEDAL]);
	pElHihatPedal->setFootCancelAccelLimit(value);
}

void MainWindow::onFootCancelClosingSpeed(int value)
{
    Mutex::scoped_lock lock(*_pMutex);
	const HiHatPedalElement::Ptr& pElHihatPedal = boost::dynamic_pointer_cast<HiHatPedalElement>(getCurrentSlot()->getPads()[Pad::HIHAT_PEDAL]);
	pElHihatPedal->setFootCancelClosingSpeed(value);
}

void MainWindow::onFootCancelPos(int value)
{
    Mutex::scoped_lock lock(*_pMutex);
	const HiHatPedalElement::Ptr& pElHihatPedal = boost::dynamic_pointer_cast<HiHatPedalElement>(getCurrentSlot()->getPads()[Pad::HIHAT_PEDAL]);
	pElHihatPedal->setFootCancelPos(value);
}

void MainWindow::onFootCancelPosDiff(int value)
{
    Mutex::scoped_lock lock(*_pMutex);
	const HiHatPedalElement::Ptr& pElHihatPedal = boost::dynamic_pointer_cast<HiHatPedalElement>(getCurrentSlot()->getPads()[Pad::HIHAT_PEDAL]);
	pElHihatPedal->setFootCancelPosDiff(value);
}

void MainWindow::onFootCancelMaskTime(int value)
{
    Mutex::scoped_lock lock(*_pMutex);
	const HiHatPedalElement::Ptr& pElHihatPedal = boost::dynamic_pointer_cast<HiHatPedalElement>(getCurrentSlot()->getPads()[Pad::HIHAT_PEDAL]);
	pElHihatPedal->setFootCancelMaskTime(value);
	_curveHiHatPosition->changeFootCancelStrategy1MaskLength(value);
}

void MainWindow::onFootCancelVelocity(int value)
{
    Mutex::scoped_lock lock(*_pMutex);
	const HiHatPedalElement::Ptr& pElHihatPedal = boost::dynamic_pointer_cast<HiHatPedalElement>(getCurrentSlot()->getPads()[Pad::HIHAT_PEDAL]);
	pElHihatPedal->setFootCancelVelocity(value);
	_curveHiHatPosition->changeFootCancelStrategy1MaskVelocity(value);
}

void MainWindow::on_groupBoxFootCancel1_toggled(bool checked)
{
    Mutex::scoped_lock lock(*_pMutex);
	const HiHatPedalElement::Ptr& pElHihatPedal = boost::dynamic_pointer_cast<HiHatPedalElement>(getCurrentSlot()->getPads()[Pad::HIHAT_PEDAL]);
	pElHihatPedal->setFootCancelStrategy1Activation(checked);
	_curveHiHatPosition->showFootCancelStrategy1Info(checked);
}

void MainWindow::on_groupBoxFootCancel2_toggled(bool checked)
{
    Mutex::scoped_lock lock(*_pMutex);
	const HiHatPedalElement::Ptr& pElHihatPedal = boost::dynamic_pointer_cast<HiHatPedalElement>(getCurrentSlot()->getPads()[Pad::HIHAT_PEDAL]);
	pElHihatPedal->setFootCancelStrategy2Activation(checked);
	_curveHiHatPedal->showFootCancelStrategy2Info(checked);
}

void MainWindow::on_groupBoxCancelOpenHit_toggled(bool checked)
{
    Mutex::scoped_lock lock(*_pMutex);
	const HiHatPedalElement::Ptr& pElHihatPedal = boost::dynamic_pointer_cast<HiHatPedalElement>(getCurrentSlot()->getPads()[Pad::HIHAT_PEDAL]);
	pElHihatPedal->setCancelOpenHit(checked);
}

void MainWindow::setCurveVisibility(EProPlotCurve* pCurve, bool state)
{
	QwtLegend* pLegend = _pPlot->legend();
	QwtLegendItem* pLegendItem = dynamic_cast<QwtLegendItem*>(pLegend->find(pCurve));
	pCurve->setVisible(state);
	pLegendItem->setChecked(state);
}

void MainWindow::onRedrawCurves()
{
	_pRedrawTimer->stop();
	_pPlot->replot();
}

void MainWindow::onHiHatStartMoving(int movingState, int pos, int timestamp)
{
	switch (movingState)
	{
	case HiHatPedalElement::MS_START_OPEN:
		{
			_curveHiHatPosition->addOpenInfo(QPointF(timestamp, pos));
			break;
		}
	case HiHatPedalElement::MS_START_CLOSE:
		{
			_curveHiHatPosition->addCloseInfo(QPointF(timestamp, pos));
			break;
		}
	default:
		{
			break;
		}
	}
}

void MainWindow::onHiHatBlueState(bool state)
{
	_curveHiHatPosition->setBlueState(state);
}

void MainWindow::onFootCancelStrategy1Started(int startTime, int maskLength, int velocity)
{
	_curveHiHatPosition->addFootCancelStrategy1Info(startTime, maskLength, velocity);
}

void MainWindow::on_tabWidget_currentChanged(int index)
{
	switch(index)
	{
	case 0:
		{
			_curveHiHatPosition->showBlueState(true);
			_curveHiHatPosition->showFootCancelStrategy1Info(false);
			_curveHiHatPedal->showFootCancelStrategy2Info(false);
			break;
		}
	case 1:
		{
			_curveHiHatPosition->showBlueState(false);
			if (groupBoxFootCancel1->isChecked())
			{
				_curveHiHatPosition->showFootCancelStrategy1Info(true);
			}
			if (groupBoxFootCancel2->isChecked())
			{
				_curveHiHatPedal->showFootCancelStrategy2Info(true);
			}
			break;
		}
	default:
		{
			_curveHiHatPosition->showBlueState(false);
			_curveHiHatPosition->showFootCancelStrategy1Info(false);
			_curveHiHatPedal->showFootCancelStrategy2Info(false);
			break;
		}
	}
}

void MainWindow::showEvent(QShowEvent* pEvent)
{
	_pRedrawTimer->start(25);
	_bRedrawState = true;
	QMainWindow::showEvent(pEvent);
}

void MainWindow::hideEvent(QHideEvent* pEvent)
{
	_bRedrawState = false;
	QMainWindow::hideEvent(pEvent);
}

void MainWindow::on_spinBoxPlotWindowSize_valueChanged(int value)
{
	QwtScaleDiv* pScaleDiv = _pPlot->axisScaleDiv(QwtPlot::xBottom);
	int timeWindowInMs = value*1000;
	int maxValue = pScaleDiv->interval().maxValue();
	_pPlotZoomer->moveWindow(maxValue-timeWindowInMs,timeWindowInMs, true, false);
}

void MainWindow::onOpenCancelHitThreshold(int value)
{
    Mutex::scoped_lock lock(*_pMutex);
	const HiHatPedalElement::Ptr& pElHihatPedal = boost::dynamic_pointer_cast<HiHatPedalElement>(getCurrentSlot()->getPads()[Pad::HIHAT_PEDAL]);
	pElHihatPedal->setCancelOpenHitThreshold(value);
}
void MainWindow::onOpenCancelHitVelocity(int value)
{
    Mutex::scoped_lock lock(*_pMutex);
	const HiHatPedalElement::Ptr& pElHihatPedal = boost::dynamic_pointer_cast<HiHatPedalElement>(getCurrentSlot()->getPads()[Pad::HIHAT_PEDAL]);
	pElHihatPedal->setCancelOpenHitVelocity(value);
}

void MainWindow::onHiHatCloseThreshold(int value)
{
	Mutex::scoped_lock lock(*_pMutex);
	const HiHatPedalElement::Ptr& pElHihatPedal = boost::dynamic_pointer_cast<HiHatPedalElement>(getCurrentSlot()->getPads()[Pad::HIHAT_PEDAL]);
	pElHihatPedal->setClosePositionThresold(value);
}
void MainWindow::onHiHatOpenThreshold(int value)
{
	Mutex::scoped_lock lock(*_pMutex);
	const HiHatPedalElement::Ptr& pElHihatPedal = boost::dynamic_pointer_cast<HiHatPedalElement>(getCurrentSlot()->getPads()[Pad::HIHAT_PEDAL]);
	pElHihatPedal->setOpenPositionThresold(value);
}

void MainWindow::on_actionSettings_triggered()
{
	SettingsDlg settings(_pSettings.get());
	connect(this, SIGNAL(midiNoteOn(int,int)), &settings, SIGNAL(midiNoteOn(int,int)));
	settings.exec();
}
