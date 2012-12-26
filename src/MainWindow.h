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

#include "ui_MainWindow.h"

#include <QtGui/QMainWindow>
#include <QtCore/QProcess>

#include <boost/iostreams/stream_buffer.hpp> 

class Settings;
class MidiDevicesWidget;
class QSpinBox;
class QDoubleSpinBox;

class MainWindow : public QMainWindow, private Ui::MainWindow
{
    Q_OBJECT

public:
	static std::string APPLICATION_NAME;
	static std::string APPLICATION_VERSION;
	typedef boost::recursive_mutex Mutex;

public:
	mutable Mutex _mutex;

public:
    MainWindow();
    ~MainWindow();

signals:
	void signalLog(const QString&);
	void signalSlotChanged(const Slot::Ptr&);

private slots:
	void on_pushButtonClearLogs_clicked(bool checked=false);

    void on_pushButtonStart_clicked(bool checked=false);
    void on_pushButtonStop_clicked(bool checked=false);

	void on_actionOpen_triggered();
	void on_actionSave_triggered();
	void on_actionSave_As_triggered();
	void on_actionQuit_triggered();
	void on_actionAdd_Slot_triggered();
	void on_actionDuplicate_Slot_triggered();
	void on_actionRemove_Slot_triggered();
	void on_actionAssistant_triggered();
	void on_actionAbout_triggered();
	void on_actionSettings_triggered();

	void on_listWidgetSlots_customContextMenuRequested(const QPoint&);
	void on_listWidgetSlots_itemSelectionChanged();
	void on_listWidgetSlots_itemChanged(QListWidgetItem* pItem);

    void on_menuEdit_aboutToShow();
	void on_tabWidget_currentChanged(int index);

	void onBufferChanged(int);

private:
	void toLog(const std::string&);

	void saveUserSettings(const std::string& szFilePath);
	void loadUserSettings(const std::string& szFilePath);

	Slot::Ptr createDefaultSlot();
	std::string createNewSlotName(const std::string& szBaseName = std::string("slot")) const;
	Slot::Ptr getCurrentSlot() const;
	void selectLastSlot();
	void updateCurrentSlot();

private:
	boost::iostreams::stream_buffer<StreamSink> _streamBuffer;
	std::streambuf*			_pOldStreambuf;

	MidiEngine				_midiEngine;

	std::auto_ptr<Settings> _pSettings;
	UserSettings			_userSettings;
	Slot::List::iterator	_currentSlot;

	int						_calibrationOffset;

	GraphSubWindow*			_pGrapSubWindow;
	QProcess*				_pProcessAssistant;
	MidiDevicesWidget*		_pMidiDevicesWidget;
	QSpinBox*				_pSpinBoxBuffer;
	QDoubleSpinBox*			_pAverageLatency;
};
