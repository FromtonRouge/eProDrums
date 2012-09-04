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

#include "FunctionItemEditor.h"
#include <QtGui/QHBoxLayout>
#include <QtGui/QDoubleSpinBox>

FunctionItemEditor::FunctionItemEditor(QWidget* pParent):QWidget(pParent)
{
	_pDoubleSpinBox = new QDoubleSpinBox(this);
	_pDoubleSpinBox->setSingleStep(1);
	_pDoubleSpinBox->setMaximum(127);
	_pDoubleSpinBox->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	connect(_pDoubleSpinBox, SIGNAL(valueChanged(double)), this, SLOT(onValueChanged(double)));
	QHBoxLayout* pLayout = new QHBoxLayout;
	pLayout->setContentsMargins(0,0,0,0);
	pLayout->addWidget(_pDoubleSpinBox);
	setLayout(pLayout);
}

void FunctionItemEditor::setSingleStep(double singleStep)
{
	_pDoubleSpinBox->setSingleStep(singleStep);
}

void FunctionItemEditor::setDecimals(int decimals)
{
	_pDoubleSpinBox->setDecimals(decimals);
}

void FunctionItemEditor::setMinimum(double value)
{
	_pDoubleSpinBox->setMinimum(value);
}

void FunctionItemEditor::setMaximum(double value)
{
	_pDoubleSpinBox->setMaximum(value);
}

FunctionItemEditor::~FunctionItemEditor()
{
}

void FunctionItemEditor::setData(double value)
{
	_pDoubleSpinBox->setValue(value);
}

double FunctionItemEditor::getData() const
{
	return _pDoubleSpinBox->value();
}

void FunctionItemEditor::onValueChanged(double)
{
	emit editFinished(this);
}
