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

#include "EProPlotCurve.h"

class QwtPlotHistogram;

class HiHatPedalCurve : public EProPlotCurve
{
public:
    HiHatPedalCurve(QwtPlot* pPlot);
    virtual ~HiHatPedalCurve();

	virtual void setVisible(bool state);
	virtual void clear();

	void add(const QPointF& point, int mt2, int vel2, int mt3, int vel3);

	void showFootCancelStrategy2Info(bool state);

	void changeFootCancelStrategy2MaskTime2(int maskLength);
	void changeFootCancelStrategy2Velocity2(int value);
	void changeFootCancelStrategy2MaskTime3(int maskLength);
	void changeFootCancelStrategy2Velocity3(int value);

private:
	QwtPlotHistogram* _pLayerFootCancelMaskTime2;
	QwtPlotHistogram* _pLayerFootCancelMaskTime3;
	bool _bShowFootCancelStragegy2Info;

};
