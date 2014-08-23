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

#include <QtCore/QPersistentModelIndex>

/**
 * Model index that can be rebuilt automatically when
 * the internal persistent model index is invalid.
 */
class ModelIndex
{
public:
	ModelIndex(const QModelIndex& index)
		: _index(index)
		, _pModel(index.model())
	{
		// Build full path from root to index
		QModelIndex current = index;
		while (current.isValid())
		{
			_path.push_front(current);
			current = current.parent();
		}
	}

	operator QModelIndex() const {return index();}
	bool operator==(const ModelIndex& rObj) const {return index()==rObj.index();}

private:
	QModelIndex index() const
	{
		QModelIndex result = _index;
		if (!result.isValid())
		{
			// rebuild index
			for (int i=0; i<_path.size(); ++i)
			{
				const QModelIndex& current = _path[i];
				result = _pModel->index(current.row(), current.column(), result);
			}
		}
		return result;
	}

private:
	const QAbstractItemModel*	_pModel;
	QPersistentModelIndex		_index;
	QVector<QModelIndex>		_path;
};
