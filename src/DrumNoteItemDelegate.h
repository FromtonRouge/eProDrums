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

#include "DrumNoteItemEditor.h"
#include "Pad.h"

#include <QtWidgets/QStyledItemDelegate>

/**
 * Drum note item delegate.
 */
class DrumNoteItemDelegate : public QStyledItemDelegate
{
	Q_OBJECT

public:
	DrumNoteItemDelegate(Pad::Type type, QObject* pParent=NULL):QStyledItemDelegate(pParent), _type(type) { }
	virtual ~DrumNoteItemDelegate() {}

	virtual QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem&, const QModelIndex&) const
	{
		DrumNoteItemEditor* pEditor = new DrumNoteItemEditor(_type, parent);
		connect(pEditor, SIGNAL(editFinished(QWidget*)), this, SIGNAL(commitData(QWidget*)));
		return pEditor;
	}

	virtual void setEditorData(QWidget* p, const QModelIndex& index) const
	{
		DrumNoteItemEditor* pEditor = dynamic_cast<DrumNoteItemEditor*>(p);
		pEditor->setData(index);
	}

	virtual void setModelData(QWidget* p, QAbstractItemModel* pModel, const QModelIndex& index) const
	{
		DrumNoteItemEditor* pEditor = dynamic_cast<DrumNoteItemEditor*>(p);
		pModel->setData(index, pEditor->getData(index));
	}

	void setType(Pad::Type type) {_type = type;}

private:
	Pad::Type _type;
};

