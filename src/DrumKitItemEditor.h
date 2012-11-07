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

#include "PadNotesWidget.h"

#include <QtGui/QWidget>
#include <QtGui/QPushButton>
#include <QtGui/QColorDialog>
#include <QtGui/QHBoxLayout>
#include <QtGui/QSpinBox>
#include <QtCore/QModelIndex>

class DrumKitItemEditor : public QWidget
{
	Q_OBJECT

Q_SIGNALS:
	void editFinished(QWidget*);

public:
	DrumKitItemEditor(QWidget* pParent, const QModelIndex& index):
		QWidget(pParent),
		_pPushButtonEditColor(NULL),
		_pSpinBoxOutputNote(NULL)
	{
		QHBoxLayout* pLayout = new QHBoxLayout;
		pLayout->setContentsMargins(0,0,0,0);

		switch (index.column())
		{
		case 1:
			{
				_pPushButtonEditColor = new QPushButton("...", this);
				_pPushButtonEditColor->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
				connect(_pPushButtonEditColor, SIGNAL(clicked(bool)), this, SLOT(onEditColor()));
				pLayout->addWidget(_pPushButtonEditColor);
				break;
			}
		case 2:
			{
				_pSpinBoxOutputNote = new QSpinBox(this);
				_pSpinBoxOutputNote->setMaximum(127);
				_pSpinBoxOutputNote->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
				connect(_pSpinBoxOutputNote, SIGNAL(valueChanged(int)), this, SLOT(onEditOutputNote(int)));
				pLayout->addWidget(_pSpinBoxOutputNote);
				break;
			}
		default:
			{
				break;
			}
		}

		setLayout(pLayout);
	}

	virtual ~DrumKitItemEditor() {}

	void setPadDescription(const Pad::MidiDescription& description)
   	{
		_padDescription = description;
		if (_pSpinBoxOutputNote)
		{
			_pSpinBoxOutputNote->setValue(_padDescription.outputNote);
		}
	}

	const Pad::MidiDescription& getPadDescription() const {return _padDescription;}

private Q_SLOTS:

	void onEditColor()
	{
		QColorDialog dlg(QColor(_padDescription.color.c_str()), this);
		if (dlg.exec())
		{
			_padDescription.color = dlg.selectedColor().name().toStdString();
			emit editFinished(this);
		}
	}

	void onEditOutputNote(int value)
	{
		_padDescription.outputNote = value;
		emit editFinished(this);
	}

private:
	Pad::MidiDescription	_padDescription;

	QPushButton*			_pPushButtonEditColor;
	QSpinBox*				_pSpinBoxOutputNote;
};
