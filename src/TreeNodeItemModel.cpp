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

#include "TreeNodeItemModel.h"
#include "TreeNode.h"

TreeNodeItemModel::TreeNodeItemModel(QObject* pParent)
	: AbstractItemModel(pParent)
	, _pRoot(new TreeNode())
{
}

TreeNodeItemModel::~TreeNodeItemModel()
{
}

int TreeNodeItemModel::rowCount(const QModelIndex& parent) const
{
	TreeNode* pParent = parent.isValid()?static_cast<TreeNode*>(parent.internalPointer()):_pRoot.get();
	if (!pParent)
	{
		return 0;
	}
	return pParent->getChildrenCount();
}

QModelIndex TreeNodeItemModel::index(int row, int column, const QModelIndex& parent) const
{
	TreeNode* pParent = _pRoot.get();
	if (parent.isValid())
	{
		pParent = static_cast<TreeNode*>(parent.internalPointer());
	}

	if (!pParent)
	{
		return QModelIndex();
	}

	if (row >= pParent->getChildrenCount())
	{
		return QModelIndex();
	}
	TreeNode* p = pParent->getChildAt(row);
	return createIndex(row, column, p);
}

QModelIndex TreeNodeItemModel::parent(const QModelIndex& index) const
{
	if (!index.isValid())
	{
		return QModelIndex();
	}

	TreeNode* pParent = static_cast<TreeNode*>(index.internalPointer())->getParent();
	if (!pParent)
	{
		return QModelIndex();
	}

	TreeNode* pGrandParent = pParent->getParent();
	if (!pGrandParent)
	{
		return QModelIndex();
	}
	return createIndex(pGrandParent->getRow(pParent), 0, pParent);
}

void TreeNodeItemModel::update()
{
	// Emit signals for each cell on the editor column only
	for (int i=0; i<rowCount(); ++i)
	{
		const QModelIndex& parentIndex = index(i, 0);
		emit dataChanged(index(i, 0), index(i, columnCount()-1));
		for (int j=0; j<rowCount(parentIndex); ++j)
		{
			emit dataChanged(index(j, 1, parentIndex), index(j, columnCount()-1, parentIndex));
		}
	}
}
