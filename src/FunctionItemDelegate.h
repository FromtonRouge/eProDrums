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

#include <QtGui/QStyledItemDelegate>
#include "FunctionItemEditor.h"

/**
 * Function item delegate.
 */
class FunctionItemDelegate : public QStyledItemDelegate
{
	Q_OBJECT

public:
	FunctionItemDelegate(QObject* pParent=NULL):QStyledItemDelegate(pParent) { }
	virtual ~FunctionItemDelegate() {}

	virtual QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem&, const QModelIndex&) const
	{
		FunctionItemEditor* pEditor = new FunctionItemEditor(parent);
		//connect(this, SIGNAL(midiNoteOn(int, int)), pEditor, SIGNAL(midiNoteOn(int, int)));
		connect(pEditor, SIGNAL(editFinished(QWidget*)), this, SIGNAL(commitData(QWidget*)));
		return pEditor;
	}

	virtual void setEditorData(QWidget* p, const QModelIndex& index) const
	{
		FunctionItemEditor* pEditor = dynamic_cast<FunctionItemEditor*>(p);
		const QVariant& variant = index.data(Qt::EditRole);
		if (pEditor && !variant.isNull())
		{
			if (index.column()==5)
			{
				pEditor->setSingleStep(0.01);
				pEditor->setMaximum(5000);
				pEditor->setMinimum(-5000);
			}
			else if (index.column()==6)
			{
				pEditor->setSingleStep(1);
				pEditor->setMaximum(500);
				pEditor->setMinimum(0);
			}
			else
			{
				pEditor->setSingleStep(1);
			}
			pEditor->setData(variant.toFloat());
		}
	}

	virtual void setModelData(QWidget* p, QAbstractItemModel* pModel, const QModelIndex& index ) const
	{
		FunctionItemEditor* pEditor = dynamic_cast<FunctionItemEditor*>(p);
		QVariant variant(pEditor->getData());
		pModel->setData(index, variant);
	}
};
