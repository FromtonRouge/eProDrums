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

#include "ParamItemEditor.h"
#include "DialogFunction.h"

#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QSpinBox>
#include <QtWidgets/QDoubleSpinBox>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QPushButton>

#include <limits>

const float SLIDER_FACTOR(1000);

ParamItemEditor::ParamItemEditor(QWidget* pParent):QWidget(pParent),
	_pCheckBox(NULL),
	_pSpinBox(NULL),
	_pDoubleSpinBox(NULL),
	_pSlider(NULL),
	_pDoubleSlider(NULL),
	_pLineEdit(NULL),
	_pComboBox(NULL),
	_pPushButton(NULL)
{
	_pLayout = new QHBoxLayout;
	_pLayout->setContentsMargins(0,0,0,0);	// Note: without this setting, widgets are not visible
	setLayout(_pLayout);
}

ParamItemEditor::~ParamItemEditor()
{
}

void ParamItemEditor::onCheckBoxValueChanged(int value)
{
	bool bChecked = value==Qt::Checked;
	_data.setValue(bChecked);
	emit editFinished(this);
}

void ParamItemEditor::onSliderValueChanged(int value)
{
	_pSpinBox->setValue(value);
}

void ParamItemEditor::onDoubleSliderValueChanged(int value)
{
	_pDoubleSpinBox->setValue(float(value)/SLIDER_FACTOR);
}

void ParamItemEditor::onSpinBoxValueChanged(int value)
{
	_pSlider->blockSignals(true);
	_pSlider->setValue(value);
	_pSlider->blockSignals(false);

	const Parameter::InfiniteExtremities& extremities = _data.getInfiniteExtremities();
	if (extremities.first && value==_pSpinBox->minimum())
	{
		_pSpinBox->setEnabled(false);
		value = std::numeric_limits<int>::min();
	}
	else if (extremities.second && value==_pSpinBox->maximum())
	{
		_pSpinBox->setEnabled(false);
		value = std::numeric_limits<int>::max();
	}
	else
	{
		_pSpinBox->setEnabled(true);
	}

	_data.setValue(value);

	emit editFinished(this);
}

void ParamItemEditor::onDoubleSpinBoxValueChanged(double value)
{
	_pDoubleSlider->blockSignals(true);
	_pDoubleSlider->setValue(value*SLIDER_FACTOR);
	_pDoubleSlider->blockSignals(false);
	
	const Parameter::InfiniteExtremities& extremities = _data.getInfiniteExtremities();
	if (extremities.first && value==_pDoubleSpinBox->minimum())
	{
		_pDoubleSpinBox->setEnabled(false);
		value = -std::numeric_limits<float>::max();
	}
	else if (extremities.second && value==_pDoubleSpinBox->maximum())
	{
		_pDoubleSpinBox->setEnabled(false);
		value = std::numeric_limits<float>::max();
	}
	else
	{
		_pDoubleSpinBox->setEnabled(true);
	}

	_data.setValue(float(value));

	emit editFinished(this);
}

void ParamItemEditor::onLineEditValueChanged(const QString& value)
{
	_data.setValue(value);
	emit editFinished(this);
}

void ParamItemEditor::onComboBoxIndexChanged(int index)
{
	const QVariant& variant = _pComboBox->itemData(index);
	if (!variant.isNull())
	{
		_data.setValue(variant.toInt());
	}
	emit editFinished(this);
}

void ParamItemEditor::setData(const Parameter& data)
{
	_data = data;

	setEnabled(_data.isEnabled());

	const AnyProperty::Value& value = _data.getValue();

	if (value.type()==typeid(bool))
	{
		// For bool values
		if (!_pCheckBox)
		{
			_pCheckBox = new QCheckBox(this);
			_pLayout->addWidget(_pCheckBox);
			connect(_pCheckBox, SIGNAL(stateChanged(int)), this, SLOT(onCheckBoxValueChanged(int)));
		}

		_pCheckBox->setChecked(boost::any_cast<bool>(value));
	}
	else if (value.type()==typeid(int))
	{
		if (_data.hasEnums())
		{
			// For enums
			if (!_pComboBox)
			{
				_pComboBox = new QComboBox(this);
				_pLayout->addWidget(_pComboBox);
				connect(_pComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(onComboBoxIndexChanged(int)));
			}

			// Fill the combo box if empty
			_pComboBox->blockSignals(true);
			if (_pComboBox->count()==0)
			{
				const Parameter::DictEnums& dictEnums = _data.getEnums();
				Parameter::DictEnums::const_iterator it = dictEnums.begin();
				while (it!=dictEnums.end())
				{
					const Parameter::DictEnums::value_type& v = *(it++);
					_pComboBox->addItem(v.second, v.first);
				}
			}

			// Select the item that match the value
			int index = _pComboBox->findData(boost::any_cast<int>(value));
			if (index>=0)
			{
				_pComboBox->setCurrentIndex(index);
			}
			_pComboBox->blockSignals(false);
		}
		else
		{
			// For int values
			if (!_pSlider && !_pSpinBox)
			{
				_pSlider = new Slider(Qt::Horizontal, this);
				connect(_pSlider, SIGNAL(valueChanged(int)), this, SLOT(onSliderValueChanged(int)));
				_pSlider->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
				_pLayout->addWidget(_pSlider);
				_pSpinBox = new QSpinBox(this);
				connect(_pSpinBox, SIGNAL(valueChanged(int)), this, SLOT(onSpinBoxValueChanged(int)));
				_pLayout->addWidget(_pSpinBox);
			}

			int minimum = boost::any_cast<int>(_data.getMinimum());
			int maximum = boost::any_cast<int>(_data.getMaximum());
			int v = boost::any_cast<int>(value);
			_pSlider->setMinimum(minimum);
			_pSlider->setMaximum(maximum);
			_pSpinBox->setMinimum(minimum);
			_pSpinBox->setMaximum(maximum);
			_pSpinBox->setValue(v);
		}
	}
	else if (value.type() == typeid(float))
	{
		if (!_pDoubleSlider && !_pDoubleSpinBox)
		{
			_pDoubleSlider = new Slider(Qt::Horizontal, this);
			connect(_pDoubleSlider, SIGNAL(valueChanged(int)), this, SLOT(onDoubleSliderValueChanged(int)));
			_pDoubleSlider->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
			_pLayout->addWidget(_pDoubleSlider);
			_pDoubleSpinBox = new QDoubleSpinBox(this);
			_pDoubleSpinBox->setSingleStep(0.01);
			connect(_pDoubleSpinBox, SIGNAL(valueChanged(double)), this, SLOT(onDoubleSpinBoxValueChanged(double)));
			_pLayout->addWidget(_pDoubleSpinBox);
		}

		float minimum = boost::any_cast<float>(_data.getMinimum());
		float maximum = boost::any_cast<float>(_data.getMaximum());
		float v = boost::any_cast<float>(value);
		_pDoubleSlider->setMinimum(minimum*SLIDER_FACTOR);
		_pDoubleSlider->setMaximum(maximum*SLIDER_FACTOR);
		_pDoubleSpinBox->setMinimum(minimum);
		_pDoubleSpinBox->setMaximum(maximum);
		_pDoubleSpinBox->setValue(v);
	}
	else if (value.type() == typeid(QString))
	{
		if (!_pLineEdit)
		{
			_pLineEdit = new QLineEdit(this);
			_pLayout->addWidget(_pLineEdit);
			connect(_pLineEdit, SIGNAL(textChanged(const QString&)), this, SLOT(onLineEditValueChanged(const QString&)));
		}
		_pLineEdit->setText(boost::any_cast<QString>(value));
	}
	else if (value.type() == typeid(QPolygonF))
	{
		// For QPolygonF
		if (!_pPushButton)
		{
			_pPushButton = new QPushButton(this);
			_pPushButton->setIcon(QIcon(":/icons/application-wave.png"));
			_pPushButton->setText(tr("Edit parameters..."));
			_pLayout->addWidget(_pPushButton);
			connect(_pPushButton, SIGNAL(clicked(bool)), this, SLOT(onPushButtonClicked()));
		}
	}
}

void ParamItemEditor::onPushButtonClicked()
{
	const QPolygonF& curveSamples = boost::any_cast<QPolygonF>(_data.getValue());
	DialogFunction dlg(_data.getFunctionDescription(), curveSamples, this);
	if (dlg.exec())
	{
		_data.setValue(dlg.getCurveSamples());
		emit editFinished(this);
	}
}
