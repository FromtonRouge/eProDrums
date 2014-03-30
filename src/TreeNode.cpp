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

#include "TreeNode.h"
#include "TreeNodeData.h"

TreeNode::TreeNode(TreeNodeData* pData)
	: _pParent(NULL)
	, _pData(pData)
{
}

TreeNode::~TreeNode()
{
	delete _pData;
	for (int i=0; i<_children.size(); ++i)
	{
		delete _children[i];
	}
}

QString TreeNode::getName() const
{
	return _pData?_pData->getName():"";
}

int	TreeNode::getRow(TreeNode* pChild) const
{
	for (int i=0; i<_children.size(); ++i)
	{
		if (_children[i]==pChild)
		{
			return i;
		}
	}
	return -1;
}

TreeNode* TreeNode::insertChild(int row, TreeNode* pNode)
{
	pNode->_pParent = this;
	_children.insert(row, pNode);
	return pNode;
}

TreeNode* TreeNode::insertChild(int row, TreeNodeData* pData)
{
	return insertChild(row, new TreeNode(pData));
}

TreeNode* TreeNode::addChild(TreeNode* pNode)
{
	pNode->_pParent = this;
	_children.push_back(pNode);
	return pNode;
}

TreeNode* TreeNode::addChild(TreeNodeData* pData)
{
	return addChild(new TreeNode(pData));
}
