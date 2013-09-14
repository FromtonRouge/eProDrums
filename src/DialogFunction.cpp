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

#include "DialogFunction.h"

#include <qwt_symbol.h>
#include <qwt_plot_marker.h>
#include <qwt_plot_grid.h>
#include <qwt_plot_canvas.h>
#include <qwt_plot_curve.h>
#include <qwt_plot_intervalcurve.h>
#include <qwt_series_data.h>
#include <QtGui/QPen>
#include <QtGui/QMenu>
#include <QtGui/QCursor>
#include <QtGui/QMouseEvent>
#include <QtGui/QVector2D>

#include <boost/bind.hpp>

DialogFunction::DialogFunction( const LinearFunction::Description::Ptr& pDescription,
								const QPolygonF& curveSamples,
								QWidget* pParent)
	: QDialog(pParent)
	, _pDescription(pDescription)
	, _indexSelectedPoint(-1)
	, _relativeIndex(0)
{
	setupUi(this);

	widgetPlot->setMinimumSize(1,1);
	widgetPlot->setAxisScale(QwtPlot::xBottom, _pDescription->xMin, _pDescription->xMax);
	widgetPlot->setAxisScale(QwtPlot::yLeft, _pDescription->yMin, _pDescription->yMax);
	widgetPlot->setAxisMaxMinor(QwtPlot::yLeft, 2);
	widgetPlot->setAxisMaxMinor(QwtPlot::xBottom, 2);
	widgetPlot->setCanvasBackground(QColor(Qt::black));
	widgetPlot->setAxisTitle(QwtPlot::xBottom, _pDescription->szLabelX.c_str());
	widgetPlot->setAxisTitle(QwtPlot::yLeft, _pDescription->szLabelY.c_str());

	_pCurve.reset(new QwtPlotCurve());
	_pCurve->setStyle(QwtPlotCurve::Lines);
	_pCurve->setPen(Qt::white);
	_pCurve->setSymbol(new QwtSymbol(QwtSymbol::Diamond, Qt::lightGray, _pCurve->pen(), QSize(8,8)));
	QwtArraySeriesData<QPointF>* pSeries = static_cast<QwtArraySeriesData<QPointF>*>(_pCurve->data());
	if (pSeries)
	{
		_pCurve->setSamples(curveSamples);
	}
	_pCurve->attach(widgetPlot);

	QwtPlotGrid* pGrid = new QwtPlotGrid;
	pGrid->setMajorPen(QPen(Qt::gray, 0, Qt::DotLine));
	pGrid->attach(widgetPlot);

	QwtPlotCanvas* pPlotCanvas = static_cast<QwtPlotCanvas*>(widgetPlot->canvas());
	pPlotCanvas->installEventFilter(this);
	pPlotCanvas->setMouseTracking(true);
}

DialogFunction::~DialogFunction()
{
}

void DialogFunction::onInsertPoint()
{
	QAction* pActionInsertPoint = static_cast<QAction*>(sender());
	const QPoint& point = pActionInsertPoint->data().toPoint();
	QwtArraySeriesData<QPointF>* pSeries = static_cast<QwtArraySeriesData<QPointF>*>(_pCurve->data());
	if (pSeries)
	{
		QPolygonF samples = pSeries->samples();
		QPointF newPoint;
		newPoint.setX(widgetPlot->invTransform(_pCurve->xAxis(), point.x()));
		newPoint.setY(widgetPlot->invTransform(_pCurve->yAxis(), point.y()));
		QPolygonF::iterator it = qLowerBound(samples.begin(), samples.end(), newPoint, boost::bind(&QPointF::x, _1) < boost::bind(&QPointF::x, _2));
		samples.insert(it, newPoint);
		_pCurve->setSamples(samples);
		widgetPlot->replot();
	}
}

void DialogFunction::onRemovePoint()
{
	QwtArraySeriesData<QPointF>* pSeries = static_cast<QwtArraySeriesData<QPointF>*>(_pCurve->data());
	if (pSeries)
	{
		QPolygonF samples = pSeries->samples();
		if (_indexSelectedPoint>=0 && _indexSelectedPoint<samples.size())
		{
			samples.remove(_indexSelectedPoint);
			_pCurve->setSamples(samples);
			widgetPlot->replot();
		}
	}
}

bool DialogFunction::eventFilter(QObject* pWatched, QEvent* pEvent)
{
	QwtPlotCanvas* pPlotCanvas = static_cast<QwtPlotCanvas*>(pWatched);
	switch (pEvent->type())
	{
	case QEvent::MouseButtonPress:
		{
			QMouseEvent* pMouseEvent = static_cast<QMouseEvent*>(pEvent);
			Qt::MouseButtons buttons = pMouseEvent->buttons();
			if (buttons & Qt::RightButton)
			{
				QMenu menu;

				// Insert point action
				QAction* pActionInsertPoint = new QAction(tr("Insert point"), &menu);
				pActionInsertPoint->setData(pMouseEvent->pos());
				connect(pActionInsertPoint, SIGNAL(triggered(bool)), this, SLOT(onInsertPoint()));
				menu.addAction(pActionInsertPoint);

				// Remove point action
				if (_indexSelectedPoint>=0)
				{
					QAction* pActionRemovePoint = new QAction(tr("Remove point"), &menu);
					pActionRemovePoint->setData(pMouseEvent->pos());
					connect(pActionRemovePoint, SIGNAL(triggered(bool)), this, SLOT(onRemovePoint()));
					menu.addAction(pActionRemovePoint);
				}

				menu.exec(QCursor::pos());
			}
			else if (buttons & Qt::LeftButton && _indexSelectedPoint >=0)
			{
				pPlotCanvas->setCursor(Qt::ClosedHandCursor);
				_lastPosition = pMouseEvent->pos();
			}
			break;
		}
	case QEvent::MouseButtonRelease:
		{
			if (_indexSelectedPoint>=0)
			{
				pPlotCanvas->setCursor(Qt::OpenHandCursor);
			}
			break;
		}
	case QEvent::MouseMove:
		{
			QMouseEvent* pMouseEvent = static_cast<QMouseEvent*>(pEvent);
			Qt::MouseButtons buttons = pMouseEvent->buttons();
			const QPoint& point =  pMouseEvent->pos();
			double dist = std::numeric_limits<double>::max();
			
			if (buttons == 0)
			{
				_relativeIndex = 0;
				_indexSelectedPoint = _pCurve->closestPoint(point, &dist);
				if (dist > 10)
				{
					QwtArraySeriesData<QPointF>* pSeries = static_cast<QwtArraySeriesData<QPointF>*>(_pCurve->data());
					if (pSeries && _indexSelectedPoint >= 0)
					{
						const QPolygonF& samples = pSeries->samples();
						double x = widgetPlot->transform(_pCurve->xAxis(), samples[_indexSelectedPoint].x());
						double y = widgetPlot->transform(_pCurve->yAxis(), samples[_indexSelectedPoint].y());
						QVector2D vClosest(x, y);
						QVector2D vClosestToCursor(QVector2D(point)-vClosest);
						if (vClosestToCursor.x()>=0)
						{
							if (_indexSelectedPoint<samples.size()-1)
							{
								_relativeIndex = 1;
							}
						}
						else
						{
							if (_indexSelectedPoint>0)
							{
								_relativeIndex = -1;
							}
						}

						if (_relativeIndex!=0)
						{
							x = widgetPlot->transform(_pCurve->xAxis(), samples[_indexSelectedPoint+_relativeIndex].x());
							y = widgetPlot->transform(_pCurve->yAxis(), samples[_indexSelectedPoint+_relativeIndex].y());

							QVector2D vOrtho(QVector2D(x, y)-vClosest);
							vOrtho.normalize();
							QVector2D vTmp(vOrtho);
							vOrtho.setX(-vTmp.y());
							vOrtho.setY(vTmp.x());

							double dot = std::abs(QVector2D::dotProduct(vClosestToCursor, vOrtho));
							if (dot < 5)
							{
								pPlotCanvas->setCursor(Qt::OpenHandCursor);
								return false;
							}
						}
					}

					pPlotCanvas->setCursor(Qt::CrossCursor);
					_indexSelectedPoint = -1;
				}
			}

			if (_indexSelectedPoint >= 0)
			{
				if (buttons & Qt::LeftButton)
				{
					pPlotCanvas->setCursor(Qt::ClosedHandCursor);
					QwtArraySeriesData<QPointF>* pSeries = static_cast<QwtArraySeriesData<QPointF>*>(_pCurve->data());
					if (pSeries)
					{
						QPolygonF samples = pSeries->samples();

						switch (_relativeIndex)
						{
						case 0:
							{
								updatePoint(point, samples, _indexSelectedPoint);
								break;
							}
						case -1:
							{
								updateBranch(point, samples, _indexSelectedPoint-1, _indexSelectedPoint);
								break;
							}
						case 1:
							{
								updateBranch(point, samples, _indexSelectedPoint, _indexSelectedPoint+1);
								break;
							}
						default:
							{
								break;
							}
						}

						_pCurve->setSamples(samples);
						widgetPlot->replot();
					}
				}
				else
				{
					pPlotCanvas->setCursor(Qt::OpenHandCursor);
				}

				_lastPosition = pMouseEvent->pos();
			}
			else
			{
				pPlotCanvas->setCursor(Qt::CrossCursor);
			}
			break;
		}
	default:
		{
			return false;
		}
	}
	return false;
}

void DialogFunction::updatePoint(const QPointF& point, QPolygonF& rSamples, int indexPoint)
{
	QPointF& rPointToMove = rSamples[indexPoint];

	double x = widgetPlot->invTransform(_pCurve->xAxis(), point.x());
	double y = widgetPlot->invTransform(_pCurve->yAxis(), point.y());

	if (indexPoint>0)
	{
		const QPointF& prevPoint = rSamples[indexPoint-1];
		x = std::max<double>(x, prevPoint.x());
	}

	if (indexPoint<rSamples.size()-1)
	{
		const QPointF& nextPoint = rSamples[indexPoint+1];
		x = std::min<double>(x, nextPoint.x());
	}

	// Clamp x and y
	const QwtScaleDiv& scaleDivX = widgetPlot->axisScaleDiv(_pCurve->xAxis());
	const QwtScaleDiv& scaleDivY = widgetPlot->axisScaleDiv(_pCurve->yAxis());
	const int minValueX = scaleDivX.interval().minValue();
	const int maxValueX = scaleDivX.interval().maxValue();
	const int minValueY = scaleDivY.interval().minValue();
	const int maxValueY = scaleDivY.interval().maxValue();
	x = std::max<double>(x, minValueX);
	x = std::min<double>(x, maxValueX);
	y = std::max<double>(y, minValueY);
	y = std::min<double>(y, maxValueY);

	rPointToMove.rx() = x;
	rPointToMove.ry() = y;
}

void DialogFunction::updateBranch(const QPointF& point, QPolygonF& rSamples, int indexPoint1, int indexPoint2)
{
	const QwtScaleDiv& scaleDivX = widgetPlot->axisScaleDiv(_pCurve->xAxis());
	const QwtScaleDiv& scaleDivY = widgetPlot->axisScaleDiv(_pCurve->yAxis());
	const int minValueX = scaleDivX.interval().minValue();
	const int maxValueX = scaleDivX.interval().maxValue();
	const int minValueY = scaleDivY.interval().minValue();
	const int maxValueY = scaleDivY.interval().maxValue();

	QPointF& rPointToMove1 = rSamples[indexPoint1];
	QPointF& rPointToMove2 = rSamples[indexPoint2];
	QPointF backupPoint1(rPointToMove1);

	double x1 = widgetPlot->invTransform(_pCurve->xAxis(), _lastPosition.x());
	double y1 = widgetPlot->invTransform(_pCurve->yAxis(), _lastPosition.y());
	double x2 = widgetPlot->invTransform(_pCurve->xAxis(), point.x());
	double y2 = widgetPlot->invTransform(_pCurve->yAxis(), point.y());

	bool bUpdatePoint2X = true;
	bool bUpdatePoint2Y = true;
	rPointToMove1.rx() += (x2-x1);
	rPointToMove1.ry() += (y2-y1);
	if (indexPoint1>0)
	{
		const QPointF& prevPoint = rSamples[indexPoint1-1];

		// Clamp on x
		if (rPointToMove1.x() < prevPoint.x())
		{
			bUpdatePoint2X = false;
			rPointToMove1.rx() = prevPoint.x();
		}
	}
	else if (rPointToMove1.x() < minValueX)
	{
		bUpdatePoint2X = false;
		rPointToMove1.rx() = minValueX;
	}

	// Clamp rPointToMove1 on y
	if (rPointToMove1.y() < minValueY)
	{
		bUpdatePoint2Y = false;
		rPointToMove1.ry() = backupPoint1.y();
	}
	else if (rPointToMove1.y() > maxValueY)
	{
		bUpdatePoint2Y = false;
		rPointToMove1.ry() = backupPoint1.y();
	}

	// Update point2 x
	if (bUpdatePoint2X)
	{
		rPointToMove2.rx() += (x2-x1);
		if (indexPoint2<rSamples.size()-1)
		{
			const QPointF& nextPoint = rSamples[indexPoint2+1];
			if (rPointToMove2.x()>nextPoint.x())
			{
				rPointToMove1.rx() = backupPoint1.x();
				rPointToMove2.rx() = nextPoint.x();
			}
		}
		else if (rPointToMove2.x() > maxValueX)
		{
			rPointToMove1.rx() = backupPoint1.x();
			rPointToMove2.rx() = maxValueX;
		}
	}

	// Update point2 y
	if (bUpdatePoint2Y)
	{
		rPointToMove2.ry() += (y2-y1);

		// Clamp rPointToMove2 on y
		if (rPointToMove2.y() < minValueY)
		{
			rPointToMove1.ry() = backupPoint1.y();
			rPointToMove2.ry() = minValueY;
		}
		else if (rPointToMove2.y() > maxValueY)
		{
			rPointToMove1.ry() = backupPoint1.y();
			rPointToMove2.ry() = maxValueY;
		}
	}
}

QPolygonF DialogFunction::getCurveSamples() const
{
	QPolygonF samples;
	QwtArraySeriesData<QPointF>* pSeries = static_cast<QwtArraySeriesData<QPointF>*>(_pCurve->data());
	if (pSeries)
	{
		samples = pSeries->samples();
	}
	return samples;
}
