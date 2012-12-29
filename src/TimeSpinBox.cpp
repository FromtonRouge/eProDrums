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

#include "TimeSpinBox.h"

#include <QtGui/QHBoxLayout>
#include <QtGui/QSpinBox>
#include <QtGui/QLabel>

#include <boost/chrono.hpp>

TimeSpinBox::TimeSpinBox(QWidget* pParent):QWidget(pParent)
{
	QHBoxLayout* pLayout = new QHBoxLayout;
	pLayout->setSpacing(0);
	pLayout->setContentsMargins(0,0,0,0);
	setLayout(pLayout);

	_pHours = new QSpinBox(this);
	connect(_pHours, SIGNAL(valueChanged(int)), this, SLOT(onHoursChanged(int)));
	_pHours->setMaximum(99);
	_pHours->setAlignment(Qt::AlignRight);
	pLayout->addWidget(_pHours);
	pLayout->addWidget(new QLabel(":"));

	_pMinutes = new QSpinBox(this);
	connect(_pMinutes, SIGNAL(valueChanged(int)), this, SLOT(onMinutesChanged(int)));
	_pMinutes->setMinimum(-1);
	_pMinutes->setMaximum(60);
	_pMinutes->setAlignment(Qt::AlignRight);
	pLayout->addWidget(_pMinutes);
	pLayout->addWidget(new QLabel(":"));

	_pSeconds = new QSpinBox(this);
	connect(_pSeconds, SIGNAL(valueChanged(int)), this, SLOT(onSecondsChanged(int)));
	_pSeconds->setMinimum(-1);
	_pSeconds->setMaximum(60);
	_pSeconds->setAlignment(Qt::AlignRight);
	pLayout->addWidget(_pSeconds);
	pLayout->addWidget(new QLabel("."));

	_pMilliseconds = new QSpinBox(this);
	connect(_pMilliseconds, SIGNAL(valueChanged(int)), this, SLOT(onMillisecondsChanged(int)));
	_pMilliseconds->setSingleStep(100);
	_pMilliseconds->setMinimum(-1);
	_pMilliseconds->setMaximum(1000);
	_pMilliseconds->setAlignment(Qt::AlignRight);
	pLayout->addWidget(_pMilliseconds);
}

TimeSpinBox::~TimeSpinBox()
{
}

void TimeSpinBox::onSliderChange(int tInMs)
{
	using namespace boost::chrono;
	milliseconds ms(tInMs);
	hours h = duration_cast<hours>(ms);
	minutes m = duration_cast<minutes>(ms);
	seconds s = duration_cast<seconds>(ms);

	_pHours->blockSignals(true);
	_pMinutes->blockSignals(true);
	_pSeconds->blockSignals(true);
	_pMilliseconds->blockSignals(true);

	_pHours->setValue((h).count());
	_pMinutes->setValue((m-h).count());
	_pSeconds->setValue((s-m).count());
	_pMilliseconds->setValue((ms-s).count());

	_pHours->blockSignals(false);
	_pMinutes->blockSignals(false);
	_pSeconds->blockSignals(false);
	_pMilliseconds->blockSignals(false);

	emit signalTimeChanged(tInMs);
}

void TimeSpinBox::updateMinMax(QSpinBox* p1, QSpinBox* p2)
{
	if (p1->value()==p1->minimum())
	{
		p1->blockSignals(true);
		p1->setValue(p1->maximum()-p1->singleStep());
		p1->blockSignals(false);
		p2->setValue(p2->value()-p2->singleStep());
	}
	else if (p1->value()==p1->maximum())
	{
		p1->blockSignals(true);
		p1->setValue(0);
		p1->blockSignals(false);
		p2->setValue(p2->value()+1);
	}
	else
	{
		computeTime();
	}
}

void TimeSpinBox::onHoursChanged(int)
{
	if (_pHours->value()==_pHours->minimum())
	{
		_pHours->blockSignals(true);
		_pHours->setValue(0);
		_pHours->blockSignals(false);
	}
	computeTime();
}

void TimeSpinBox::onMinutesChanged(int)
{
	if (_pHours->value()==0 && _pMinutes->value()==_pMinutes->minimum())
	{
		_pMinutes->blockSignals(true);
		_pMinutes->setValue(0);
		_pMinutes->blockSignals(false);
	}
	updateMinMax(_pMinutes, _pHours);
}

void TimeSpinBox::onSecondsChanged(int)
{
	if (_pHours->value()==0 && _pMinutes->value()==0 && _pSeconds->value()==_pSeconds->minimum())
	{
		_pSeconds->blockSignals(true);
		_pSeconds->setValue(0);
		_pSeconds->blockSignals(false);
	}
	updateMinMax(_pSeconds, _pMinutes);
}

void TimeSpinBox::onMillisecondsChanged(int)
{
	if (_pHours->value()==0 && _pMinutes->value()==0 && _pSeconds->value()==0 && _pMilliseconds->value()==_pMilliseconds->minimum())
	{
		_pMilliseconds->blockSignals(true);
		_pMilliseconds->setValue(0);
		_pMilliseconds->blockSignals(false);
	}
	updateMinMax(_pMilliseconds, _pSeconds);
}

void TimeSpinBox::computeTime()
{
	using namespace boost::chrono;
	hours h(_pHours->value());
	minutes m(_pMinutes->value());
	seconds s(_pSeconds->value());
	milliseconds ms(_pMilliseconds->value());

	milliseconds total = duration_cast<milliseconds>(h) + duration_cast<milliseconds>(m) + duration_cast<milliseconds>(s) + ms;
	emit signalTimeEdited(total.count());
	emit signalTimeChanged(total.count());
}
