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

#include "TimeBar.h"
#include "TimeSlider.h"
#include "TimeSpinBox.h"
#include "MidiMessage.h"

#include <QtWidgets/QHBoxLayout>

TimeBar::TimeBar(QWidget* pParent):QWidget(pParent)
{
	QHBoxLayout* pLayout = new QHBoxLayout;
	pLayout->setContentsMargins(0,0,0,0);

	_pTimeSlider = new TimeSlider(this);
	connect(_pTimeSlider, SIGNAL(valueChanged(int)), this, SLOT(onTimeSliderChanged(int)));
	pLayout->addWidget(_pTimeSlider);

	_pTimeSpinBox = new TimeSpinBox(this);
	connect(_pTimeSpinBox, SIGNAL(signalValueChanged(int)), this, SLOT(onTimeSpinBoxChanged(int)));
	pLayout->addWidget(_pTimeSpinBox);

	setLayout(pLayout);
}

TimeBar::~TimeBar()
{
}

void TimeBar::onMidiOut(const MidiMessage& midiMessage)
{
	_pTimeSlider->blockSignals(true);
	_pTimeSlider->setMaximum(midiMessage.getSentTimestamp());
	_pTimeSlider->setValue(_pTimeSlider->maximum());
	_pTimeSlider->blockSignals(false);

	_pTimeSpinBox->blockSignals(true);
	_pTimeSpinBox->setTime(_pTimeSlider->maximum());
	_pTimeSpinBox->blockSignals(false);
}

void TimeBar::onTimeOffset(int offset)
{
	_pTimeSlider->setValue(_pTimeSlider->value()+offset);
}

void TimeBar::onTimeSliderChanged(int t)
{
	_pTimeSpinBox->blockSignals(true);
	_pTimeSpinBox->setTime(t);
	_pTimeSpinBox->blockSignals(false);

	emit signalTimeChanged(t);
}

void TimeBar::onTimeSpinBoxChanged(int t)
{
	_pTimeSlider->blockSignals(true);
	_pTimeSlider->setValue(t);
	_pTimeSlider->blockSignals(false);

	emit signalTimeChanged(t);
}
