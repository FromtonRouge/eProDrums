// ============================================================ // 
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

#include "CmdInsertRows.h"
#include "AbstractItemModel.h"

CmdInsertRows::CmdInsertRows(	AbstractItemModel* pModel,
								int row,
								int count,
								const QModelIndex& parent,
								QUndoCommand* pParent)
	: UndoCommand(pParent)
	, _pModel(pModel)
	, _row(row)
	, _count(count)
	, _parent(parent)
{
}

CmdInsertRows::~CmdInsertRows()
{
}

void CmdInsertRows::undo()
{
	_pModel->removeRowsNoUndo(_row, _count, _parent);
}

void CmdInsertRows::redo()
{
	_pModel->insertRowsNoUndo(_row, _count, _parent);
}

int CmdInsertRows::id() const
{
	return static_cast<int>(qHash("CmdInsertRows"));
}
