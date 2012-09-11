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
#include "LinearFunction.h"

/**
 * Function item delegate.
 */
class FunctionItemDelegate : public QStyledItemDelegate
{
	Q_OBJECT

public:
	FunctionItemDelegate(const LinearFunction::Description::Ptr& pDescription, QObject* pParent=NULL):QStyledItemDelegate(pParent), _pDescription(pDescription) { }
	virtual ~FunctionItemDelegate() {}

	virtual QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem&, const QModelIndex&) const
	{
		FunctionItemEditor* pEditor = new FunctionItemEditor(parent);
		connect(pEditor, SIGNAL(editFinished(QWidget*)), this, SIGNAL(commitData(QWidget*)));
		return pEditor;
	}

	virtual void setEditorData(QWidget* p, const QModelIndex& index) const
	{
		FunctionItemEditor* pEditor = dynamic_cast<FunctionItemEditor*>(p);
		const QVariant& variant = index.data(Qt::EditRole);
		if (pEditor && !variant.isNull())
		{
			switch (index.column())
			{
			case 1:
			case 2:
				{
					pEditor->setSingleStep(_pDescription->xStep);
					break;
				}
			case 3:
			case 4:
				{
					pEditor->setSingleStep(_pDescription->yStep);
					break;
				}
			case 5:
				{
					pEditor->setSingleStep(_pDescription->aStep);
					pEditor->setDecimals(_pDescription->aDecimals);
					pEditor->setMaximum(5000);
					pEditor->setMinimum(-5000);
					break;
				}
			case 6:
				{
					pEditor->setSingleStep(_pDescription->bStep);
					pEditor->setMaximum(500);
					pEditor->setMinimum(0);
					break;
				}
			default:
				{
					pEditor->setSingleStep(1);
					break;
				}
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

private:
	LinearFunction::Description::Ptr	_pDescription;
};
