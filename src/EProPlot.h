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

#include "qwt_plot.h"

class QWidget;

class EProPlot : public QwtPlot
{
    Q_OBJECT

public:
    EProPlot(QWidget* pParent, int plotTimeWindow = 5000);
    virtual ~EProPlot();
    void showAll();
	void clear();

	void setDefaultScaleLength(int length) {_defaultScaleLength = length;}
	int getDefaultScaleLength() const {return _defaultScaleLength;}
	void setLastTime(int t) {_lastTime = t;}

protected:
	virtual void mousePressEvent(QMouseEvent* pEvent);

private slots:
    void showCurve(const QVariant &, bool, int);
	void onResetZoom();
	void onCurveVisibilityTriggered();

private:
	int	_defaultScaleLength;
	int	_lastTime;
};
