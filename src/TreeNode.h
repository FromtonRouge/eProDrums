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

class TreeNodeData;

#include <QtCore/QVector>
#include <QtCore/QString>

class TreeNode
{
	friend class TreeNodeItemModel;

public:
	typedef QVector<TreeNode*> Children;

public:
	~TreeNode();

	QString getName() const;

	TreeNodeData*		getData() const {return _pData;}
	void				setData(TreeNodeData* pData) {_pData = pData;}

	TreeNode*			insertChild(int row, TreeNode* pNode);
	TreeNode*			insertChild(int row, TreeNodeData* pData);
	TreeNode*			addChild(TreeNode* pNode);
	TreeNode*			addChild(TreeNodeData* pData);

	TreeNode*			getParent() const {return _pParent;}
	const Children&		getChildren() const {return _children;}
	TreeNode*			getChildAt(int i) const {return _children[i];}
	bool				hasParent() const {return _pParent!=NULL;}
	bool				hasChildren() const {return !_children.empty();}
	int					getChildrenCount() const {return _children.size();}
	int					getRow(TreeNode* pChild) const;

protected:
	TreeNode(TreeNodeData* pData = NULL);

private:
	TreeNode*			_pParent;
	Children			_children;
	TreeNodeData*	_pData;
};
