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

#include "ValueControl.h"
#include "qwt_thermo.h"

ValueControl::ValueControl(const std::string& szLabel, int minValue, int maxValue)
{
	setupUi(this);

	// Thermo creation
	_pThermo = new QwtThermo();
	_pThermo->setMinValue(minValue);
	_pThermo->setMaxValue(maxValue);
	_pThermo->setScalePosition(QwtThermo::RightScale);
	_pThermo->setPipeWidth(12);
	gridLayout->addWidget(_pThermo, 0, 1);

    setThermoValue(minValue);
    setThermoAlarmLevel(maxValue);
    hideThermo();
    _pThermo->setScaleMaxMajor(10);

	label->setText(szLabel.c_str());

	spinBox->setMinimum(minValue);
	slider->setMinimum(minValue);
	spinBox->setMaximum(maxValue);
	slider->setMaximum(maxValue);
	spinBox->setSingleStep(1);
	slider->setSingleStep(1);
}

ValueControl::~ValueControl()
{
	delete _pThermo;
}

void ValueControl::on_slider_valueChanged(int value)
{
	spinBox->setValue(value);
}

void ValueControl::on_spinBox_valueChanged(int value)
{
	slider->blockSignals(true);
	slider->setValue(value);
	slider->blockSignals(false);
	
	emit valueChanged(value);
}

int ValueControl::getValue() const
{
	return spinBox->value();
}
void ValueControl::setValue(int value)
{
    spinBox->setValue(value);
}

void ValueControl::showThermo()
{
    _pThermo->show();
}

void ValueControl::hideThermo()
{
    _pThermo->hide();
}

void ValueControl::setThermoValue(int value)
{
    _pThermo->setValue(value);
}

void ValueControl::setThermoAlarmLevel(int value)
{
    _pThermo->setAlarmLevel(value);
}

void ValueControl::setThermoFillColor(const QColor& color)
{
    _pThermo->setFillBrush(color);
}

void ValueControl::setThermoAlarmColor(const QColor& color)
{
    _pThermo->setAlarmBrush(color);
}
