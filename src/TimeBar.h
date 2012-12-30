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

#include <QtGui/QWidget>

class MidiMessage;
class TimeSlider;
class TimeSpinBox;

class TimeBar : public QWidget
{
	Q_OBJECT

signals:
	void signalTimeChanged(int);

public:
	TimeBar(QWidget* pParent);
	virtual ~TimeBar();

public slots:
	void onTimeSliderChanged(int);
	void onTimeSpinBoxChanged(int);
	void onMidiOut(const MidiMessage&);
	void onTimeOffset(int);

private:
	TimeSlider*		_pTimeSlider;
	TimeSpinBox*	_pTimeSpinBox;
};

