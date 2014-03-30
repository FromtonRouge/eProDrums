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

#include "CmdSetData.h"
#include "AbstractItemModel.h"

CmdSetData::CmdSetData( AbstractItemModel* pModel,
						const QModelIndex& index,
						const QVariant& value,
						int role,
						QUndoCommand* pParent)
	: UndoCommand(pParent)
	, _pModel(pModel)
	, _index(index)
	, _oldValue(index.data(role))
	, _newValue(value)
	, _role(role)
{
	setText(QObject::tr("\"%1\" has changed").arg(index.sibling(index.row(), 0).data().toString()));
}

CmdSetData::~CmdSetData()
{
}

void CmdSetData::undo()
{
	_pModel->setDataNoUndo(_index, _oldValue, _role);
}

void CmdSetData::redo()
{
	_pModel->setDataNoUndo(_index, _newValue, _role);
}

int CmdSetData::id() const
{
	return static_cast<int>(qHash("CmdSetData"));
}

bool CmdSetData::mergeWith(const QUndoCommand* pCommand)
{
	if (pCommand->id()!=id())
	{
		return false;
	}

	const CmdSetData* pUndoCommand = static_cast<const CmdSetData*>(pCommand);
	if (_index==pUndoCommand->_index)
	{
		_newValue = pUndoCommand->_newValue;
		return true;
	}

	return false;
}
