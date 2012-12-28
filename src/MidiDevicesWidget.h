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

#include "MidiDevice.h"

#include <QtGui/QWidget>

class QComboBox;
class QPushButton;

class MidiDevicesWidget : public QWidget
{
	Q_OBJECT

signals:
	void signalStart(int, int);
	void signalStop();

public:

	MidiDevicesWidget(QWidget* pParent = NULL);
	virtual ~MidiDevicesWidget();

	void setMidiInDevices(const MidiDevice::List& devices);
	void setMidiOutDevices(const MidiDevice::List& devices);

	bool setMidiIn(const std::string& szMidiIn);
	bool setMidiOut(const std::string& szMidiOut);

	std::string getMidiInString() const;
	std::string getMidiOutString() const;

public slots:
	void onMidiStart();
	void onMidiStop();

private:
	int getMidiInId() const;
	int getMidiOutId() const;

private:
	QComboBox*		_pComboBoxMidiIn;
	QComboBox*		_pComboBoxMidiOut;
	QPushButton*	_pPushButtonStart;
	QPushButton*	_pPushButtonStop;
};
