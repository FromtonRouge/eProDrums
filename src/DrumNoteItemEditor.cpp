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

#include "DrumNoteItemEditor.h"
#include "DrumNote.h"

#include <QtGui/QStackedWidget>
#include <QtGui/QHBoxLayout>
#include <QtGui/QComboBox>
#include <QtGui/QSpinBox>

DrumNoteItemEditor::DrumNoteItemEditor(Pad::Type type, QWidget* pParent):QWidget(pParent),
	_type(type)
{
	_pStackedWidget = new QStackedWidget(this);

	_pSpinBox = new QSpinBox(this);
	_pSpinBox->setMinimum(0);
	_pSpinBox->setMaximum(127);
	_pSpinBox->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	connect(_pSpinBox, SIGNAL(valueChanged(int)), this, SLOT(onValueChanged(int)));
	_pStackedWidget->addWidget(_pSpinBox);

	_pComboBox = new QComboBox(this);
	_pComboBox->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	connect(_pComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(onCurrentIndexChanged(int)));

	_pComboBox->addItem(tr("None"), static_cast<int>(DrumNote::NONE));

	switch (_type)
	{
	case Pad::SNARE:
	case Pad::TOM1:
	case Pad::TOM2:
	case Pad::TOM3:
		{
			_pComboBox->addItem(tr("Head"), static_cast<int>(DrumNote::HEAD));
			_pComboBox->addItem(tr("Rim"), static_cast<int>(DrumNote::RIM));
			break;
		}

	case Pad::HIHAT:
		{
			_pComboBox->addItem(tr("Bow"), static_cast<int>(DrumNote::BOW));
			_pComboBox->addItem(tr("Edge"),  static_cast<int>(DrumNote::EDGE));
			break;
		}
	case Pad::CRASH1:
	case Pad::CRASH2:
	case Pad::CRASH3:
	case Pad::RIDE:
		{
			_pComboBox->addItem(tr("Bow"), static_cast<int>(DrumNote::BOW));
			_pComboBox->addItem(tr("Edge"),  static_cast<int>(DrumNote::EDGE));
			_pComboBox->addItem(tr("Bell"), static_cast<int>(DrumNote::BELL));
			break;
		}

	case Pad::HIHAT_PEDAL:
	case Pad::BASS_DRUM:
		{
			break;
		}
	}
	_pStackedWidget->addWidget(_pComboBox);

	QHBoxLayout* pLayout = new QHBoxLayout;
	pLayout->setContentsMargins(0,0,0,0);
	pLayout->addWidget(_pStackedWidget);
	setLayout(pLayout);
}

DrumNoteItemEditor::~DrumNoteItemEditor()
{
}

void DrumNoteItemEditor::onCurrentIndexChanged(int)
{
	emit editFinished(this);
}

void DrumNoteItemEditor::onValueChanged(int)
{
	emit editFinished(this);
}

void DrumNoteItemEditor::setData(const QModelIndex& index)
{
	const QVariant& variant = index.data(Qt::EditRole);
	_pStackedWidget->setCurrentIndex(index.column());
	switch (index.column())
	{
	case 0:
		{
			_pSpinBox->setValue(variant.toInt());
			break;
		}
	case 1:
		{
			int idx = _pComboBox->findData(variant);
			if (idx>=0)
			{
				_pComboBox->setCurrentIndex(idx);
			}
			break;
		}
	}
}

QVariant DrumNoteItemEditor::getData(const QModelIndex& index) const
{
	switch (index.column())
	{
	case 0:
		{
			return _pSpinBox->value();
		}
	case 1:
		{
			return _pComboBox->itemData(_pComboBox->currentIndex());
		}
	}
	return QVariant();
}
