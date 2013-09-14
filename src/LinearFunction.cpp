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

#include "LinearFunction.h"

bool LinearFunction::apply(const QPolygonF& curveSamples, float x, float& y)
{
	const int nbSamples = curveSamples.size();
	for (int i=0; i<nbSamples; ++i)
	{
		const QPointF& point1 = curveSamples[i];
		if (i+1 < nbSamples)
		{
			const QPointF& point2 = curveSamples[i+1];
			if (point1.x()!=point2.x() && x>=point1.x() && x<=point2.x())
			{
				LinearFunction f(point1, point2);
				y = f(x);
				return true;
			}
		}
	}
	return false;
}
