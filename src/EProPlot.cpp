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

#include "EProPlot.h"
#include "EProPlotCurve.h"

#include <qwt_plot_grid.h>
#include <qwt_plot_marker.h>

#include <QPen>
#include <QMenu>
#include <QMouseEvent>

#include <boost/format.hpp>

Q_DECLARE_METATYPE(EProPlotCurve*);

EProPlot::EProPlot(QWidget* pParent, int plotTimeWindow)
	: QwtPlot(pParent)
	, _defaultScaleLength(5000)
	, _lastTime(0)
{
	setMinimumSize(1,1);
	setAutoReplot(false);
	
    setAxisScale(QwtPlot::yLeft, 0, 127, 30);
    setAxisMaxMinor(QwtPlot::yLeft, 2);
    setAxisScale(QwtPlot::xBottom, 0, plotTimeWindow, 1000);

    setCanvasBackground(QColor(Qt::black));

    QwtPlotGrid* pGrid = new QwtPlotGrid;
	pGrid->setMajorPen(QPen(Qt::gray, 0, Qt::DotLine));
    pGrid->attach(this);
}

EProPlot::~EProPlot()
{
}

void EProPlot::showCurve(const QVariant& itemInfo, bool bChecked, int)
{
	if (itemInfo.canConvert<QwtPlotItem *>())
	{
		EProPlotCurve* p = dynamic_cast<EProPlotCurve*>(qvariant_cast<QwtPlotItem *>(itemInfo));
		if (p)
		{
			p->setVisible(bChecked);
			replot();
		}
	}
}

void EProPlot::showAll()
{
	const QwtPlotItemList& list = itemList();
	QwtPlotItemList::const_iterator it = list.begin();
	while (it!=list.end())
	{
		QwtPlotItem* p = *(it++);
		EProPlotCurve* pCurve = dynamic_cast<EProPlotCurve*>(p);
		if (pCurve)
		{
			pCurve->setVisible(true);
		}
	}
}

void EProPlot::clear()
{
	const QwtPlotItemList& list = itemList();
	QwtPlotItemList::const_iterator it = list.begin();
	while (it!=list.end())
	{
		QwtPlotItem* p = *(it++);
		EProPlotCurve* pCurve = dynamic_cast<EProPlotCurve*>(p);
		if (pCurve)
		{
			pCurve->clear();
		}
	}
}

void EProPlot::onResetZoom()
{
	setAxisScale(QwtPlot::xBottom, _lastTime, _lastTime+_defaultScaleLength);
	setAxisScale(QwtPlot::yLeft, 0, 127);
	replot();
}

void EProPlot::onCurveVisibilityTriggered()
{
	QAction* pAction = qobject_cast<QAction*>(sender());
	EProPlotCurve* p = pAction->data().value<EProPlotCurve*>();
	p->setVisible(!p->isVisible());
	replot();
}

void EProPlot::mousePressEvent(QMouseEvent* pEvent)
{
	Qt::MouseButtons buttons = pEvent->buttons();
	if (buttons & Qt::RightButton)
	{
		QMenu menu(this);
		menu.addAction(QIcon(":/icons/magnifier-zoom-fit.png"), tr("Reset Zoom"), this, SLOT(onResetZoom()));
		menu.addSeparator();
		
		// Build the menu used to control curve visibility
		QPixmap pixmap(12,12);
		const QwtPlotItemList& list = itemList();
		QwtPlotItemList::const_iterator it = list.begin();
		while (it!=list.end())
		{
			QwtPlotItem* p = *(it++);
			EProPlotCurve* pCurve = dynamic_cast<EProPlotCurve*>(p);
			if (pCurve)
			{
				QAction* p = new QAction(pCurve->title().text(), &menu);
				connect(p, SIGNAL(triggered(bool)), this, SLOT(onCurveVisibilityTriggered()));
				pixmap.fill(pCurve->pen().color());
				p->setIcon(QIcon(pixmap));
				p->setIconVisibleInMenu(true);
				p->setCheckable(true);
				p->setChecked(pCurve->isVisible());
				QVariant v;
				v.setValue(pCurve);
				p->setData(v);
				menu.addAction(p);
			}
		}

		menu.exec(QCursor::pos());
	}
	QwtPlot::mousePressEvent(pEvent);
}
