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

#include "AbstractItemModel.h"
#include "CmdSetData.h"
#include "CmdInsertRows.h"
#include "CmdRemoveRows.h"

AbstractItemModel::AbstractItemModel(QObject* pParent)
	: QAbstractItemModel(pParent)
	, _pUndoStack(NULL)
{
}

AbstractItemModel::~AbstractItemModel()
{
}

void AbstractItemModel::beginUndoMacro(const QString& szText)
{
	if (_pUndoStack)
	{
		_pUndoStack->beginMacro(szText);
	}
}

void AbstractItemModel::endUndoMacro()
{
	if (_pUndoStack)
	{
		_pUndoStack->endMacro();
	}
}

bool AbstractItemModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
	if (value==data(index, role))
	{
		return true;
	}

	if (_pUndoStack)
	{
		_pUndoStack->push(createUndoSetData(index, value, role));
	}
	else
	{
		setDataNoUndo(index, value, role);
	}
	return true;
}

bool AbstractItemModel::insertRows(int row, int count, const QModelIndex& parent)
{
	if (_pUndoStack)
	{
		_pUndoStack->push(createUndoInsertRows(row, count, parent));
	}
	else
	{
		insertRowsNoUndo(row, count, parent);
	}
	return true;
}

bool AbstractItemModel::removeRows(int row, int count, const QModelIndex & parent)
{
	if (_pUndoStack)
	{
		_pUndoStack->push(createUndoRemoveRows(row, count, parent));
	}
	else
	{
		removeRowsNoUndo(row, count, parent);
	}
	return true;
}

UndoCommand* AbstractItemModel::createUndoSetData(const QModelIndex & index, const QVariant & value, int role)
{
	return new CmdSetData(this, index, value, role);
}

UndoCommand* AbstractItemModel::createUndoInsertRows(int row, int count, const QModelIndex& parent)
{
	return new CmdInsertRows(this, row, count, parent);
}

UndoCommand* AbstractItemModel::createUndoRemoveRows(int row, int count, const QModelIndex& parent)
{
	return new CmdRemoveRows(this, row, count, parent);
}
