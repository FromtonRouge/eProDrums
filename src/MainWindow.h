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

#include "MidiEngine.h"
#include "Pad.h"
#include "StreamSink.h"
#include "GraphSubWindow.h"
#include "UserSettings.h"
#include "SlotItemModel.h"

#include "ui_MainWindow.h"

#include <QtWidgets/QMainWindow>
#include <QtCore/QProcess>

#include <boost/iostreams/stream_buffer.hpp> 

class ParamItemProxyModel;
class Settings;
class MidiDevicesWidget;
class QSpinBox;
class QDoubleSpinBox;
class QUndoStack;

class MainWindow : public QMainWindow, private Ui::MainWindow
{
    Q_OBJECT

public:
	typedef boost::recursive_mutex Mutex;

public:
	mutable Mutex _mutex;

public:
    MainWindow();
    ~MainWindow();

signals:
	void signalLog(const QString&);

private slots:
	void on_actionOpen_triggered();
	void on_actionSave_triggered();
	void on_actionSave_As_triggered();
	void on_actionQuit_triggered();
	void on_actionAssistant_triggered();
	void on_actionAbout_triggered();
	void on_actionSettings_triggered();
	void on_comboBoxPadSettingsType_currentIndexChanged(int index);
	void on_pushButtonClearLogs_clicked(bool checked=false);

	void onInputBufferChanged(int);
	void onMidiEngineStarted();
	void onMidiEngineStopped();
	void onSlotChanged(const Slot::Ptr&);

protected:
	void closeEvent(QCloseEvent*);

private:
	void toLog(const std::string&);

	void saveUserSettings(const std::string& szFilePath);
	void loadUserSettings(const std::string& szFilePath);

	Slot::Ptr	createDefaultSlot(const QString& szSlotName = QString("default"));
	Slot::Ptr	getCurrentSlot() const;
	void		selectLastSlot();

private:
	boost::iostreams::stream_buffer<StreamSink> _streamBuffer;
	std::streambuf*			_pOldStreambuf;

	MidiEngine				_midiEngine;

	std::auto_ptr<Settings> _pSettings;
	UserSettings			_userSettings;
	Slot::List::iterator	_currentSlot;

	SlotItemModel*			_pSlotItemModel;
	GraphSubWindow*			_pGrapSubWindow;
	QProcess*				_pProcessAssistant;
	MidiDevicesWidget*		_pMidiDevicesWidget;
	QSpinBox*				_pSpinBoxInputBuffer;
	QDoubleSpinBox*			_pAverageLatency;
	QUndoStack*				_pUndoStack;
	ParamItemProxyModel*	_pProxyModel;
};
