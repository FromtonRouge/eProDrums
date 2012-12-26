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

#include "qwt_plot_zoomer.h"
#include <QtGui/QPolygon>
#include <vector>

class QwtPlotCanvas;
class QwtPlotPanner;
class QwtPlotMagnifier;

class EProPlotZoomer : public QwtPlotZoomer
{
    Q_OBJECT

public:
    EProPlotZoomer(QwtPlotCanvas* pCanvas);
    virtual ~EProPlotZoomer();

	void moveWindow(double x, double width, bool bSaveWindow = true);

signals:
	void inRectSelection(bool);
	void leftMouseClicked(const QPoint&);

public slots:
	void onTimeChange(int ms);

protected slots:
	void onPlotSelectionMoved(const QPoint& point);
	void onPlotSelectionAppended(const QPoint& point);

protected:
    virtual void widgetKeyPressEvent(QKeyEvent*	pKeyEvent);
    virtual void widgetKeyReleaseEvent(QKeyEvent*	pKeyEvent);
    virtual void widgetMousePressEvent(QMouseEvent* pMouseEvent);
    virtual void widgetMouseReleaseEvent(QMouseEvent* pMouseEvent);
    virtual bool accept(QPolygon&	polygon) const;
	virtual QwtText trackerText(const QPoint &) const;

private:
    mutable bool _bModifierPressed;
    std::vector<QPointF> _plotSelectionPoints;
	std::pair<int, int> _rect;
	QwtPlotPanner* _plotPanner;
	QwtPlotMagnifier* _pPlotMagnifier;
	double _savedWindowX;
	double _savedWindowWidth;
};
