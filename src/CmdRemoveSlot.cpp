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

#include "CmdRemoveSlot.h"
#include "SlotItemModel.h"

#include <QtCore/QAbstractItemModel>
#include <QtCore/QDebug>

CmdRemoveSlot::CmdRemoveSlot(	const QModelIndex& index,
								QUndoCommand* pParent)
	: UndoCommand("Remove slot", pParent)
	, _removedRow(index.row())
{
	_pModel = static_cast<SlotItemModel*>(const_cast<QAbstractItemModel*>(index.model()));

	// Backup the slot for undo
	const QVariant& variant = _pModel->data(_pModel->index(_removedRow, 0), Qt::UserRole);
	_slot = *(variant.value<Slot::Ptr>());
}

CmdRemoveSlot::~CmdRemoveSlot()
{
}

void CmdRemoveSlot::undo()
{
	Slot::Ptr pSlot(new Slot());
	*pSlot = _slot;
	_pModel->insertSlot(_removedRow, pSlot);
}

void CmdRemoveSlot::redo()
{
	_pModel->removeRow(_removedRow);
}

int CmdRemoveSlot::id() const
{
	return static_cast<int>(qHash("CmdRemoveSlot"));
}
