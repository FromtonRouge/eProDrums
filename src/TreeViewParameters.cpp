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

#include "TreeViewParameters.h"
#include "ParamItemModel.h"
#include "ParamItemDelegate.h"

TreeViewParameters::TreeViewParameters(	QWidget* pParent)
	: QTreeView(pParent)
{
	setItemDelegate(new ParamItemDelegate(this));
	//setIndentation(0);
	//setRootIsDecorated(false);
}

void TreeViewParameters::openPersistentEditors()
{
	QAbstractItemModel* pModel = model();
	if (pModel)
	{
		// Opening editors
		for (int i=0; i<pModel->rowCount(); ++i)
		{
			const QModelIndex& parentIndex = pModel->index(i, 1);
			for (int j=0; j<pModel->rowCount(parentIndex); ++j)
			{
				openPersistentEditor(pModel->index(j, 1, parentIndex));
			}
			openPersistentEditor(parentIndex);
		}

		expandAll();
		resizeColumnToContents(0);
	}
}

void TreeViewParameters::closePersistentEditors()
{
	QAbstractItemModel* pModel = model();
	if (pModel)
	{
		// Opening editors
		for (int i=0; i<pModel->rowCount(); ++i)
		{
			const QModelIndex& parentIndex = pModel->index(i, 1);
			for (int j=0; j<pModel->rowCount(parentIndex); ++j)
			{
				closePersistentEditor(pModel->index(j, 1, parentIndex));
			}
			closePersistentEditor(parentIndex);
		}
	}
}
