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

#include "ParamItemModel.h"
#include "ParameterData.h"
#include "TreeNode.h"
#include "Parameter.h"

ParamItemModel::ParamItemModel(QObject* pParent)
	: TreeNodeItemModel(pParent)
{
}

QVariant ParamItemModel::headerData(	int				section,
										Qt::Orientation	orientation,
										int				role) const
{
	if (role!=Qt::DisplayRole)
	{
		return QVariant();
	}

	if (orientation==Qt::Horizontal)
	{
		switch (section)
		{
		case 0:
			return QString("Name");
		case 1:
			return QString("Value");
		}
	}
	return QVariant();
}

void ParamItemModel::setDataNoUndo(	const QModelIndex&	index,
									const QVariant&		newData,
									int					role)
{
	TreeNode* pNode = static_cast<TreeNode*>(index.internalPointer());
	ParameterBaseData* pData = static_cast<ParameterBaseData*>(pNode->getData());

	// Data given by the editor
	switch (role)
	{
	case Qt::EditRole:
		{
			switch (pData->getType())
			{
			case ParameterBaseData::PARAMETER:
				{
					ParameterData* pParameterData = static_cast<ParameterData*>(pData);
					const Parameter& newParameter = newData.value<Parameter>();
					pParameterData->getParameter()->setValue(newParameter.getValue());

					// Disable stack to avoid new undo commands
					QUndoStack* pStack = getUndoStack();
					setUndoStack(NULL);
					emit dataChanged(index, index);
					setUndoStack(pStack);

					break;
				}
			default:
				{
					break;
				}
			}
			break;
		}
	}
}

Qt::ItemFlags ParamItemModel::flags(const QModelIndex& index) const
{
	switch(index.column())
	{
	case 0:
		return Qt::ItemIsEnabled|Qt::ItemIsSelectable;
	case 1:
		return Qt::ItemIsEnabled|Qt::ItemIsSelectable|Qt::ItemIsEditable;
	}
	return Qt::ItemIsEnabled|Qt::ItemIsSelectable;
}

int ParamItemModel::columnCount(const QModelIndex&) const
{
	return 2;
}

QVariant ParamItemModel::data(const QModelIndex& index, int role) const
{
	if (!index.isValid())
	{
		return QVariant();
	}

	TreeNode* pNode = static_cast<TreeNode*>(index.internalPointer());
	ParameterBaseData* pData = static_cast<ParameterBaseData*>(pNode->getData());
	int type = pData->getType();

	switch (role)
	{
	case Qt::ToolTipRole:
		{
			//return pParameter->getDescription().c_str();
			break;
		}
	case Qt::DisplayRole:
		{
			if (index.column()==0)
			{
				return pData->getName();
			}
			break;
		}
	case Qt::EditRole:
		{
			if (index.column() == 1)
			{
				switch (type)
				{ 
				case ParameterBaseData::PARAMETER:
					{
						ParameterData* pParameterData = static_cast<ParameterData*>(pData);
						QVariant variant;
						variant.setValue(*pParameterData->getParameter());
						return variant;
					}
				case ParameterBaseData::GROUP:
					{
						ParameterData* pParameterData = static_cast<ParameterData*>(pData);
						QVariant variant;
						variant.setValue(*pParameterData->getParameter());
						return variant;
					}
				}
			}
			break;
		}
	default:
		{
			break;
		}
	}
	return QVariant();
}
