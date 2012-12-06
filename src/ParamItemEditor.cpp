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
#include "Parameter.h"
#include "DialogFunction.h"

#include <QtGui/QStackedWidget>
#include <QtGui/QHBoxLayout>
#include <QtGui/QCheckBox>
#include <QtGui/QLineEdit>
#include <QtGui/QSpinBox>
#include <QtGui/QDoubleSpinBox>
#include <QtGui/QComboBox>
#include <QtGui/QPushButton>

#include <limits>

const float SLIDER_FACTOR(1000);

ParamItemEditor::ParamItemEditor(QWidget* pParent):QWidget(pParent),
	_pData(NULL),
	_pStackedWidget(NULL),
	_pCheckBox(NULL),
	_pSpinBox(NULL),
	_pDoubleSpinBox(NULL),
	_pSlider(NULL),
	_pDoubleSlider(NULL),
	_pLineEdit(NULL),
	_pComboBox(NULL),
	_pPushButton(NULL)
{
	_pStackedWidget = new QStackedWidget(this);

	// For bool values
	_pCheckBox = new QCheckBox(this);
	connect(_pCheckBox, SIGNAL(stateChanged(int)), this, SLOT(onCheckBoxValueChanged(int)));
	_pStackedWidget->addWidget(_pCheckBox);

	// For int values
	{
		QWidget* pSubWidget = new QWidget(this);
		QHBoxLayout* pSubLayout = new QHBoxLayout;
		pSubLayout->setContentsMargins(0,0,0,0);	// Note: without this setting, widgets are not visible
		_pSlider = new Slider(Qt::Horizontal, this);
		connect(_pSlider, SIGNAL(valueChanged(int)), this, SLOT(onSliderValueChanged(int)));
		_pSlider->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
		pSubLayout->addWidget(_pSlider);
		_pSpinBox = new QSpinBox(this);
		connect(_pSpinBox, SIGNAL(valueChanged(int)), this, SLOT(onSpinBoxValueChanged(int)));
		pSubLayout->addWidget(_pSpinBox);
		pSubWidget->setLayout(pSubLayout);
		_pStackedWidget->addWidget(pSubWidget);
	}

	// For float values
	{
		QWidget* pSubWidget = new QWidget(this);
		QHBoxLayout* pSubLayout = new QHBoxLayout;
		pSubLayout->setContentsMargins(0,0,0,0);	// Note: without this setting, widgets are not visible
		_pDoubleSlider = new Slider(Qt::Horizontal, this);
		connect(_pDoubleSlider, SIGNAL(valueChanged(int)), this, SLOT(onDoubleSliderValueChanged(int)));
		_pDoubleSlider->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
		pSubLayout->addWidget(_pDoubleSlider);
		_pDoubleSpinBox = new QDoubleSpinBox(this);
		_pDoubleSpinBox->setSingleStep(0.01);
		connect(_pDoubleSpinBox, SIGNAL(valueChanged(double)), this, SLOT(onDoubleSpinBoxValueChanged(double)));
		pSubLayout->addWidget(_pDoubleSpinBox);
		pSubWidget->setLayout(pSubLayout);
		_pStackedWidget->addWidget(pSubWidget);
	}

	// For string values
	_pLineEdit = new QLineEdit(this);
	connect(_pLineEdit, SIGNAL(textChanged(const QString&)), this, SLOT(onLineEditValueChanged(const QString&)));
	_pStackedWidget->addWidget(_pLineEdit);

	// For enums
	_pComboBox = new QComboBox(this);
	connect(_pComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(onComboBoxIndexChanged(int)));
	_pStackedWidget->addWidget(_pComboBox);

	// For LinearFunction::List
	_pPushButton = new QPushButton(this);
	_pPushButton->setIcon(QIcon(":/FunctionEdit.png"));
	_pPushButton->setText(tr("Edit parameters..."));
	connect(_pPushButton, SIGNAL(clicked(bool)), this, SLOT(onPushButtonClicked()));
	_pStackedWidget->addWidget(_pPushButton);

	QHBoxLayout* pLayout = new QHBoxLayout;
	pLayout->setContentsMargins(0,0,0,0);
	pLayout->addWidget(_pStackedWidget);
	setLayout(pLayout);
}

void ParamItemEditor::onCheckBoxValueChanged(int value)
{
	bool bChecked = value==Qt::Checked;
	_pData->setValue(bChecked);
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

	const Parameter::InfiniteExtremities& extremities = _pData->getInfiniteExtremities();
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

	_pData->setValue(value);

	emit editFinished(this);
}

void ParamItemEditor::onDoubleSpinBoxValueChanged(double value)
{
	_pDoubleSlider->blockSignals(true);
	_pDoubleSlider->setValue(value*SLIDER_FACTOR);
	_pDoubleSlider->blockSignals(false);
	
	const Parameter::InfiniteExtremities& extremities = _pData->getInfiniteExtremities();
	if (extremities.first && value==_pDoubleSpinBox->minimum())
	{
		_pDoubleSpinBox->setEnabled(false);
		value = std::numeric_limits<float>::min();
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

	_pData->setValue(float(value));

	emit editFinished(this);
}

void ParamItemEditor::onLineEditValueChanged(const QString& value)
{
	_pData->setValue(value.toStdString());
	emit editFinished(this);
}

void ParamItemEditor::onComboBoxIndexChanged(int index)
{
	const QVariant& variant = _pComboBox->itemData(index);
	if (!variant.isNull())
	{
		_pData->setValue(variant.toInt());
	}
	emit editFinished(this);
}

ParamItemEditor::~ParamItemEditor()
{
}

void ParamItemEditor::setData(Parameter* pData)
{
	_pData = pData;

	if (_pData->isConnected())
	{
		setEnabled(_pData->isEnabled());
	}
	else
	{
		setEnabled(false);
	}

	const Parameter::Value& value = _pData->getValue();

	if (boost::get<bool>(&value))
	{
		_pStackedWidget->setCurrentIndex(0);
		_pCheckBox->setChecked(boost::get<bool>(value));
	}
	else if (boost::get<int>(&value))
	{
		if (_pData->hasEnums())
		{
			_pStackedWidget->setCurrentIndex(4);

			// Fill the combo box if empty
			_pComboBox->blockSignals(true);
			if (_pComboBox->count()==0)
			{
				const Parameter::DictEnums& dictEnums = _pData->getEnums();
				Parameter::DictEnums::const_iterator it = dictEnums.begin();
				while (it!=dictEnums.end())
				{
					const Parameter::DictEnums::value_type& v = *(it++);
					_pComboBox->addItem(v.second.c_str(), v.first);
				}
			}

			// Select the item that match the value
			int index = _pComboBox->findData(boost::get<int>(value));
			if (index>=0)
			{
				_pComboBox->setCurrentIndex(index);
			}
			_pComboBox->blockSignals(false);
		}
		else
		{
			_pStackedWidget->setCurrentIndex(1);
			int minimum = boost::get<int>(_pData->minimum);
			int maximum = boost::get<int>(_pData->maximum);
			int v = boost::get<int>(value);
			_pSlider->setMinimum(minimum);
			_pSlider->setMaximum(maximum);
			_pSpinBox->setMinimum(minimum);
			_pSpinBox->setMaximum(maximum);
			_pSpinBox->setValue(v);
		}
	}
	else if (boost::get<float>(&value))
	{
		_pStackedWidget->setCurrentIndex(2);
		float minimum = boost::get<float>(_pData->minimum);
		float maximum = boost::get<float>(_pData->maximum);
		float v = boost::get<float>(value);
		_pDoubleSlider->setMinimum(minimum*SLIDER_FACTOR);
		_pDoubleSlider->setMaximum(maximum*SLIDER_FACTOR);
		_pDoubleSpinBox->setMinimum(minimum);
		_pDoubleSpinBox->setMaximum(maximum);
		_pDoubleSpinBox->setValue(v);
	}
	else if (boost::get<std::string>(&value))
	{
		_pStackedWidget->setCurrentIndex(3);
		_pLineEdit->setText(boost::get<std::string>(value).c_str());
	}
	else if (boost::get<LinearFunction::List>(&value))
	{
		_pStackedWidget->setCurrentIndex(5);
	}
}

void ParamItemEditor::onPushButtonClicked()
{
	const LinearFunction::List& functions = boost::get<LinearFunction::List>(_pData->getValue());
	DialogFunction dlg(_pData->getFunctionDescription(), functions, this);
	if (dlg.exec())
	{
		_pData->setValue(dlg.getFunctions());
		emit editFinished(this);
	}
}
