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

#include <QtCore/QAbstractItemModel>

class QUndoStack;
class UndoCommand;

class AbstractItemModel : public QAbstractItemModel
{
	Q_OBJECT

public:
	AbstractItemModel(QObject* pParent = NULL);
	virtual ~AbstractItemModel();

	void			setUndoStack(QUndoStack* pUndoStack)	{_pUndoStack = pUndoStack;}
	QUndoStack*		getUndoStack() const					{return _pUndoStack;}
	void			beginUndoMacro(const QString& szText);
	void			endUndoMacro();

	virtual bool			setData(const QModelIndex & index, const QVariant & value, int role = Qt::EditRole);
	virtual void			setDataNoUndo(const QModelIndex&, const QVariant& , int) {}
	virtual UndoCommand*	createUndoSetData(const QModelIndex & index, const QVariant & value, int role = Qt::EditRole);

	virtual bool			insertRows(int row, int count, const QModelIndex& parent = QModelIndex());
	virtual void			insertRowsNoUndo(int, int, const QModelIndex&) {}
	virtual UndoCommand*	createUndoInsertRows(int row, int count, const QModelIndex& parent = QModelIndex());

	virtual bool			removeRows(int row, int count, const QModelIndex & parent = QModelIndex());
	virtual void			removeRowsNoUndo(int, int, const QModelIndex&) {}
	virtual UndoCommand*	createUndoRemoveRows(int row, int count, const QModelIndex& parent = QModelIndex());

	// TODO moveRows

private:
	QUndoStack*		_pUndoStack;
};
