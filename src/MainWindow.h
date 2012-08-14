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

#pragma once

#include "MidiMessage.h"
#include "Pad.h"
#include "StreamSink.h"

#include "ui_MainWindow.h"

#include "UserSettings.h"

#include <QtGui/QMainWindow>

#include <windows.h>

#include <boost/shared_ptr.hpp>
#include <boost/thread.hpp>
#include <boost/thread/condition.hpp>
#include <boost/iostreams/stream_buffer.hpp> 

#include <list>

class EProPlot;
class EProPlotCurve;
class HiHatPositionCurve;
class EProPlotZoomer;
class QwtPlotItem;
class QTimer;
class Settings;

class MainWindow : public QMainWindow, private Ui::MainWindow
{
    Q_OBJECT

public:
	static std::string APPLICATION_NAME;
	static std::string APPLICATION_VERSION;
	typedef boost::recursive_mutex Mutex;

public:
	MidiMessage::Clock _clock;
	mutable Mutex _mutex;

public:
    MainWindow();
    ~MainWindow();

    void notify() {_condition.notify_all();}
    void addIncomingMidiMessage(const MidiMessage& midiMessage);
	const UserSettings& getConfig() const {return _userSettings;}

signals:
	void hiHatState(int state);
    void hiHatPedalControl(int value);
	void hiHatStartMoving(int movingState, int pos, int timestamp);
	void footCancelStrategy1Started(int timestamp, int maskLength, int velocity);
    void updatePlot(int msgType, int msgChannel, int msgNote, int msgVelocity, int timestamp, float , float);
	void midiNoteOn(int msgNote, int msgVelocity);
	void sLog(const QString&);

private slots:
	void on_pushButtonClearLogs_clicked(bool checked=false);
	void on_groupBoxLogs_toggled(bool checked);
	void on_checkBoxLogsRawData_toggled(bool checked);
	void on_checkBoxLogsFilteredData_toggled(bool checked);
	void on_checkBoxLogsHiHatControl_toggled(bool checked);
	void on_checkBoxLogsFlams_toggled(bool checked);
	void on_checkBoxLogsGhostNotes_toggled(bool checked);
	void on_checkBoxLogsOthers_toggled(bool checked);

	// Midi settings
    void on_pushButtonStart_clicked(bool checked=false);
    void on_pushButtonStop_clicked(bool checked=false);
	void on_comboBoxMidiIn_currentIndexChanged(const QString&);
	void on_comboBoxMidiOut_currentIndexChanged(const QString&);

	void on_actionOpen_triggered();
	void on_actionSave_triggered();
	void on_actionSave_As_triggered();
	void on_actionQuit_triggered();
	void on_actionAdd_Slot_triggered();
	void on_actionDuplicate_Slot_triggered();
	void on_actionRemove_Slot_triggered();
	void on_actionAbout_triggered();
	void on_actionSettings_triggered();

	void on_sliderBuffer_valueChanged(int);
	void on_spinBoxBuffer_valueChanged(int);

	void on_listWidgetSlots_customContextMenuRequested(const QPoint&);
	void on_listWidgetSlots_itemSelectionChanged();
	void on_listWidgetSlots_itemChanged(QListWidgetItem* pItem);

    void on_menuEdit_aboutToShow();
	void on_tabWidget_currentChanged(int index);

	void onFootCancelStrategy1Started(int timestamp, int maskLength, int velocity);
	void onHiHatState(int state);
	void onHiHatStartMoving(int movingState, int pos, int timestamp);
	void onRedrawCurves();
	void onRectSelection(bool);
	void onLeftMouseClicked(const QPoint&);
    void onUpdatePlot(int, int, int, int, int, float, float);

private:
	virtual void showEvent(QShowEvent* pEvent);
	virtual void hideEvent(QHideEvent* pEvent);

    void midiThread();
    void stop();
	void computeMessage(MidiMessage& currentMsg, MidiMessage::DictHistory& lastMsgSent);

    void sendMidiMessage(const MidiMessage& midiMessage, bool bForce = false);
    void sendMidiMessages(const MidiMessage::List& midiMessages, bool bForce = false);

	void saveUserSettings(const std::string& szFilePath);
	void loadUserSettings(const std::string& szFilePath);

	Slot::Ptr createDefaultSlot();

    MidiMessage* getNextMessage(const boost::shared_ptr<Pad>& pElement, int msgType = 9);

	std::string createNewSlotName(const std::string& szBaseName = std::string("slot")) const;

	void clearPlots();
	void setCurveVisibility(EProPlotCurve* pCurve, bool state);

	Slot::Ptr getCurrentSlot() const
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

	void toLog(const std::string&);

private:
	boost::iostreams::stream_buffer<StreamSink> _streamBuffer;

	std::auto_ptr<Settings> _pSettings;

	UserSettings _userSettings;
	Slot::List::iterator _currentSlot;

	// Midi related...
    HMIDIIN  _midiInHandle;
    HMIDIOUT _midiOutHandle;
    bool _bConnected;
    MidiMessage::List _midiMessages;
	MidiMessage _lastHiHatMsgControl;

	// Thread etc...
    boost::condition _condition;
    boost::shared_ptr<boost::thread> _midiThread;

	int _calibrationOffset;

    EProPlot* _pPlot;
    HiHatPositionCurve* _curveHiHatPosition;
    EProPlotCurve* _curveHiHatAcceleration;
    EProPlotCurve* _curveHiHatPedal;
    EProPlotCurve* _curveHiHat;
    EProPlotCurve* _curveCrash;
    EProPlotCurve* _curveYellowCrash;
    EProPlotCurve* _curveRide;
    EProPlotCurve* _curveTom1;
    EProPlotCurve* _curveTom2;
    EProPlotCurve* _curveTom3;
    EProPlotCurve* _curveSnare;
    EProPlotCurve* _curveBassPedal;

    // Plot pickers
    EProPlotZoomer* _pPlotZoomer;
	QTimer* _pRedrawTimer;
	bool _bRedrawState;
};
