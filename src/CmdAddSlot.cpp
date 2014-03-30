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

#include "CmdAddSlot.h"
#include "SlotItemModel.h"

#include <QtCore/QAbstractItemModel>

CmdAddSlot::CmdAddSlot(	QAbstractItemModel* pModel,
						const Slot::Ptr& pSlot,
						QUndoCommand* pParent)
	: UndoCommand("Add slot", pParent)
	, _pModel(pModel)
	, _pSlot(pSlot)
{
}

CmdAddSlot::~CmdAddSlot()
{
}

void CmdAddSlot::undo()
{
	SlotItemModel* pModel = static_cast<SlotItemModel*>(_pModel);
	pModel->removeRow(pModel->rowCount()-1);
}

void CmdAddSlot::redo()
{
	SlotItemModel* pModel = static_cast<SlotItemModel*>(_pModel);
	pModel->addSlot(_pSlot);
}

int CmdAddSlot::id() const
{
	return static_cast<int>(qHash("CmdAddSlot"));
}
