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

#include "ParamItemProxyModel.h"

ParamItemProxyModel::ParamItemProxyModel(QObject* pParent)
	: QSortFilterProxyModel(pParent)
	, _padSettingsType(0)
{
}

bool ParamItemProxyModel::filterAcceptsRow(int sourceRow, const QModelIndex & sourceParent) const
{
	switch (_padSettingsType)
	{
	case 0:
		{
			if (!sourceParent.isValid())
			{
				if (sourceRow>=0 && sourceRow <=6)
				{
					return true;
				}
			}
			else
			{
				return true;
			}
			break;
		}
	case 1:
		{
			if (!sourceParent.isValid())
			{
				if (sourceRow>=7 && sourceRow <=9)
				{
					return true;
				}
			}
			else
			{
				return true;
			}
			break;
		}
	case 2:
		{
			if (!sourceParent.isValid())
			{
				int rows = sourceModel()->rowCount(sourceParent);
				if (sourceRow>=10 && sourceRow <rows-1)
				{
					return true;
				}
			}
			else
			{
				return true;
			}
			break;
		}
	case 3:
		{
			if (!sourceParent.isValid())
			{
				int rows = sourceModel()->rowCount(sourceParent);
				if (sourceRow==rows-1)
				{
					return true;
				}
			}
			else
			{
				return true;
			}
			break;
		}
	default:
		{
			return true;
		}
	}
	return false;
}
