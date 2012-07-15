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

class QwtPlotIntervalCurve;
class QwtPlotHistogram;

class HiHatPositionCurve : public EProPlotCurve
{
public:
    HiHatPositionCurve(QwtPlot* pPlot);
    virtual ~HiHatPositionCurve();

	void addOpenInfo(const QPointF& point);
	void addCloseInfo(const QPointF& point);

	virtual void setVisible(bool state);
	virtual void clear();
	virtual void add(const QPointF& point, const boost::any& userData = boost::any());

	void setBlueState(bool state) {_blueState = state;}

	void showBlueState(bool state);
	void showFootCancelStrategy1Info(bool state);

	void addFootCancelStrategy1Info(int startTime, int maskLength, int velocity);
	void changeFootCancelStrategy1MaskLength(int maskLength);
	void changeFootCancelStrategy1MaskVelocity(int velocity);

private:
	EProPlotCurve* _pLayerOpenInfo;
	EProPlotCurve* _pLayerCloseInfo;
	QwtPlotHistogram* _pLayerFootCancelMaskTime1;
	bool _bShowFootCancelStragegy1Info;
	bool _bShowBlueState;

	QwtPlotIntervalCurve* _pLayerBlueOnInfo;
	QwtPlotIntervalCurve* _pLayerBlueOffInfo;
	bool _blueState;
	bool _previousBlueState;

};
