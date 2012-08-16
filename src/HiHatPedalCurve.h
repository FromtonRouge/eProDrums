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

class HiHatPedalCurve : public EProPlotCurve
{
public:
    HiHatPedalCurve(QwtPlot* pPlot);
    virtual ~HiHatPedalCurve();

	virtual void setVisible(bool state);
	virtual void clear();
	virtual void add(const QPointF& point, const boost::any& userData = boost::any());

	void setFootCancelMaskTime(int maskLength);
	void setFootCancelMaskVelocity(int velocity);

	void activateMask(bool state);
	void showMaskLayer(bool state);

protected:
	void updateMaskLayer();

private:
	QwtPlotHistogram*	_pLayerFootCancelMask;
	int					_maskVelocity;
	int					_maskTime;
	bool				_isMaskActivated;
	bool				_isMaskLayerShown;
};

