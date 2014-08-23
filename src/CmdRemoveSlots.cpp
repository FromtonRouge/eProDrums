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

#include "CmdRemoveSlots.h"
#include "AbstractItemModel.h"
#include "SlotItemModel.h"

CmdRemoveSlots::CmdRemoveSlots(	AbstractItemModel* pModel,
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
	if (_count==1)
	{
		setText(QObject::tr("1 slot removed"));
	}
	else
	{
		setText(QObject::tr("%1 slots removed").arg(_count));
	}

	// Keep all slots pointers (with data(SLOT_ROLE))
	for (int i=_row; i<_row+_count; ++i)
	{
		const QModelIndex& index = _pModel->index(i, 0, parent);
		const QVariant& variant = _pModel->data(index, SlotItemModel::SLOT_ROLE);
		const Slot::Ptr& pSlot = variant.value<Slot::Ptr>();
		_slots.push_back(pSlot);
	}
}

CmdRemoveSlots::~CmdRemoveSlots()
{
}

void CmdRemoveSlots::undo()
{
	_pModel->insertRowsNoUndo(_row, _count, _parent);
	for (int i=0; i<_count; ++i)
	{
		const QModelIndex& index = _pModel->index(_row+i, 0, _parent);
		QVariant variant;
		variant.setValue(_slots[i]);
		_pModel->setDataNoUndo(index, variant, SlotItemModel::SLOT_ROLE);
	}
}

void CmdRemoveSlots::redo()
{
	_pModel->removeRowsNoUndo(_row, _count, _parent);
}

int CmdRemoveSlots::id() const
{
	return static_cast<int>(qHash("CmdRemoveSlots"));
}
