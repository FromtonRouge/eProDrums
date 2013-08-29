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

#include "EProPlotZoomer.h"
#include "qwt_plot_panner.h"
#include "qwt_plot_magnifier.h"
#include "qwt_plot_canvas.h"
#include "qwt_plot.h"
#include <QMouseEvent>
#include <boost/format.hpp>
#include <cmath>
#include <iostream>

EProPlotZoomer::EProPlotZoomer(QwtPlotCanvas* pCanvas)
	: QwtPlotZoomer( QwtPlot::xBottom, QwtPlot::yLeft, pCanvas)
	, _bModifierPressed(false)
{
	_plotPanner = new QwtPlotPanner(pCanvas);
	_plotPanner->setAxisEnabled(QwtPlot::xBottom, true);
	_plotPanner->setAxisEnabled(QwtPlot::yLeft, true);
	_plotPanner->setMouseButton(Qt::MidButton);
	_plotPanner->setEnabled(true);

	// Plot magnifier, allow zooming by wheel
	_pPlotMagnifier = new QwtPlotMagnifier(pCanvas);
	_pPlotMagnifier->setMouseButton(Qt::NoButton); // Zoom when clicking Qt::RightButton disabled

	setRubberBandPen(QColor(Qt::white));
	setTrackerPen(QColor(Qt::white));
	setTrackerMode(AlwaysOn);

	connect(this, SIGNAL(appended(const QPoint&)), this, SLOT(onPlotSelectionAppended(const QPoint&)));
	connect(this, SIGNAL(moved(const QPoint&)), this, SLOT(onPlotSelectionMoved(const QPoint&)));
}

EProPlotZoomer::~EProPlotZoomer()
{
}

void EProPlotZoomer::widgetMousePressEvent(QMouseEvent*	pMouseEvent)
{
	QMouseEvent* pNewMouseEvent = pMouseEvent;

	switch(pMouseEvent->button())
	{
	case Qt::LeftButton:
		{
			_bModifierPressed = false;
			setRubberBandPen(QColor(Qt::white));
			setTrackerPen(QColor(Qt::white));

			Qt::KeyboardModifiers kbModifiers = pMouseEvent->modifiers();
			if (kbModifiers & Qt::ControlModifier)
			{
				setRubberBandPen(QColor(Qt::green));
				setTrackerPen(QColor(Qt::green));
				_bModifierPressed = true;
				pNewMouseEvent = new QMouseEvent(pMouseEvent->type(), pMouseEvent->pos(), pMouseEvent->button(), pMouseEvent->buttons(), Qt::NoModifier);
			}
			else
			{
				emit leftMouseClicked(pMouseEvent->pos());
			}
			QwtPlotZoomer::widgetMousePressEvent(pNewMouseEvent);
			break;
		}

	case Qt::RightButton:
		{
			break;
		}

	default:
		{
			break;
		}
	}
}

void EProPlotZoomer::widgetMouseReleaseEvent(QMouseEvent* pMouseEvent)
{
	QMouseEvent* pNewMouseEvent = pMouseEvent;

	switch(pMouseEvent->button())
	{
	case Qt::LeftButton:
		{

			setRubberBandPen(QColor(Qt::white));
			setTrackerPen(QColor(Qt::white));
			_rect.first = 0;
			_rect.second = 0;

			Qt::KeyboardModifiers kbModifiers = pMouseEvent->modifiers();
			if (kbModifiers & Qt::ControlModifier)
			{
				_bModifierPressed = true;
				pNewMouseEvent = new QMouseEvent(pMouseEvent->type(), pMouseEvent->pos(), pMouseEvent->button(), pMouseEvent->buttons(), Qt::NoModifier);
			}
			emit inRectSelection(false);
			QwtPlotZoomer::widgetMouseReleaseEvent(pNewMouseEvent);
			break;
		}
	case Qt::RightButton:
		{
			break;
		}
	case Qt::MidButton:
		{
			break;
		}
	default:
		{
			break;
		}
	}
}

bool EProPlotZoomer::accept(QPolygon&	polygon) const
{
	if (_bModifierPressed)
	{
		return QwtPlotZoomer::accept(polygon);
	}

	_bModifierPressed = false;
	return false;
}

void EProPlotZoomer::widgetKeyPressEvent(QKeyEvent*	pKeyEvent)
{
	Qt::KeyboardModifiers kbModifiers = pKeyEvent->modifiers();
	if (kbModifiers & Qt::ControlModifier)
	{
		setRubberBandPen(QColor(Qt::green));
		setTrackerPen(QColor(Qt::green));
		_bModifierPressed = true;
	}
}

void EProPlotZoomer::widgetKeyReleaseEvent(QKeyEvent*	pKeyEvent)
{
	_bModifierPressed = false;
	setRubberBandPen(QColor(Qt::white));
	setTrackerPen(QColor(Qt::white));
	QwtPlotZoomer::widgetKeyReleaseEvent(pKeyEvent);
}

QwtText EProPlotZoomer::trackerText(const QPoint& pointInPixel) const
{
	boost::format fmtMsg("%.1f (ms), %.1f (unit)");
	const QPointF& point = invTransform(pointInPixel);
	if (_bModifierPressed)
	{
		QString result((fmtMsg%point.x()%point.y()).str().c_str());
		return result;
	}
	else
	{
		if (_rect.first==0 && _rect.second==0)
		{
			QString result((fmtMsg%point.x()%point.y()).str().c_str());
			return result;
		}
		else
		{
			QString result((fmtMsg%_rect.first%_rect.second).str().c_str());
			return result;
		}
	}
}

void EProPlotZoomer::onPlotSelectionAppended(const QPoint& pointInPixel)
{
	if (_plotSelectionPoints.size()>=2)
	{
		_plotSelectionPoints.clear();
	}
	_plotSelectionPoints.push_back(invTransform(pointInPixel));
}

void EProPlotZoomer::onPlotSelectionMoved(const QPoint& pointInPixel)
{
	if (_plotSelectionPoints.size()==2)
	{
		const QPointF& p1 = _plotSelectionPoints[0];
		const QPointF& p2 = invTransform(pointInPixel);

		_rect.first = int(std::abs(p2.x()-p1.x()));
		_rect.second = int(std::abs(p2.y()-p1.y()));
	}
	else
	{
		_rect.first = 0;
		_rect.second = 0;
	}
	emit inRectSelection(true);
}
