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

#include "DoubleValueControl.h"

DoubleValueControl::DoubleValueControl(const std::string& szLabel, double minValue, double maxValue)
{
	setupUi(this);

	label->setText(szLabel.c_str());

	doubleSpinBox->setMinimum(minValue);
	slider->setMinimum(minValue*100);
	doubleSpinBox->setMaximum(maxValue);
	slider->setMaximum(maxValue*100);
	doubleSpinBox->setSingleStep(0.01);
	slider->setSingleStep(1);
}

DoubleValueControl::~DoubleValueControl()
{
}

void DoubleValueControl::on_slider_valueChanged(int value)
{
	doubleSpinBox->setValue(double(value)/100);
}

void DoubleValueControl::on_doubleSpinBox_valueChanged(double value)
{
	slider->blockSignals(true);
	slider->setValue(int(value*100));
	slider->blockSignals(false);

	emit valueChanged(value);
}

double DoubleValueControl::getValue() const
{
	return doubleSpinBox->value();
}
void DoubleValueControl::setValue(double value)
{
	doubleSpinBox->setValue(value);
}
