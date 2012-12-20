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

#include "EProPlotData.h"
#include <limits>

EProPlotData::EProPlotData():
	_yMin(std::numeric_limits<double>::max()),
	_yMax(-std::numeric_limits<double>::max())
{
}

EProPlotData::~EProPlotData()
{
}

QRectF EProPlotData::boundingRect() const
{
	QRectF result;
	if (!_data.empty())
	{
		result.setX(_data[0].x());
		result.setWidth(_data[_data.size()-1].x());
		result.setY(_yMin);
		result.setHeight(_yMax-_yMin);
	}
	return result;
}

void EProPlotData::add(const QPointF& point, const boost::any& userData)
{
	_yMin = point.y()<_yMin?point.y():_yMin;
	_yMax = point.y()>_yMax?point.y():_yMax;
	_data.push_back(point);
	_userData.push_back(userData);
}

void EProPlotData::clear()
{
	_userData.clear();
	_data.clear();
	_yMin = std::numeric_limits<double>::max();
	_yMax = -std::numeric_limits<double>::max();
}
