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
	enum HiHatState
	{
		HHS_SECURED,
		HHS_CLOSED,
		HHS_HALF_OPEN,
		HHS_OPEN
	};

public:
    HiHatPositionCurve(QwtPlot* pPlot);
    virtual ~HiHatPositionCurve();

	void addOpenInfo(const QPointF& point);
	void addCloseInfo(const QPointF& point);

	virtual void setVisible(bool state);
	virtual void clear();
	virtual void add(const QPointF& point, const boost::any& userData = boost::any());

	void setHiHatState(HiHatState state) {_hiHatState = state;}

	void activateHiHatStates(bool state);
	void activateFootCancel(bool state);
	void showHiHatLayers(bool state);
	void showFootCancelLayers(bool state);

	void addFootCancelInfo(int startTime, int maskLength, int velocity);
	void setFootCancelMaskTime(int maskLength);
	void setFootCancelMaskVelocity(int velocity);

private:
	void updateHiHatLayers();
	void updateFootCancelLayers();

private:
	EProPlotCurve* _pLayerOpenInfo;
	EProPlotCurve* _pLayerCloseInfo;
	QwtPlotHistogram* _pLayerFootCanceMaskTime;

	QwtPlotIntervalCurve* _pLayerStateSecured;
	QwtPlotIntervalCurve* _pLayerStateClosed;
	QwtPlotIntervalCurve* _pLayerStateHalfOpen;
	QwtPlotIntervalCurve* _pLayerStateOpen;
	HiHatState _hiHatState;
	HiHatState _previousHiHatState;

	bool	_isFootCancelLayersShown;
	bool	_isFootCancelActivated;

	bool	_isHiHatLayersShown;
	bool	_isHiHatStatesActivated;
};
