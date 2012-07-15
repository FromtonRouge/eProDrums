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

#include "qwt_plot_grid.h"
#include "qwt_plot_marker.h"
#include "qwt_legend.h"
#include "qwt_legend_item.h"

#include <QPen>

#include <boost/format.hpp>

EProPlot::EProPlot(QWidget* pParent, int plotTimeWindow):
    QwtPlot(pParent),
    _plotTimeWindow(plotTimeWindow)
{
	setAutoReplot(false);

    setAxisScale(QwtPlot::yLeft, 0, 127, 30);
    setAxisMaxMinor(QwtPlot::yLeft, 2);
    setAxisScale(QwtPlot::xBottom, 0, _plotTimeWindow, 1000);
    setCanvasBackground(QColor(Qt::black));

    QwtPlotGrid* pGrid = new QwtPlotGrid;
    pGrid->setMajPen(QPen(Qt::gray, 0, Qt::DotLine));
    pGrid->attach(this);

    QwtLegend *pLegend = new QwtLegend;
    pLegend->setItemMode(QwtLegend::CheckableItem);
    insertLegend(pLegend, QwtPlot::RightLegend);

    connect(this, SIGNAL(legendChecked(QwtPlotItem*, bool)), SLOT(showCurve(QwtPlotItem*, bool)));
}

EProPlot::~EProPlot()
{
}

void EProPlot::showCurve(QwtPlotItem* pPlotItem, bool bState)
{
	EProPlotCurve* p = dynamic_cast<EProPlotCurve*>(pPlotItem);
	if (p)
	{
		p->setVisible(bState);
		replot();
	}
}

void EProPlot::showAll()
{
    const QList<QWidget*>& legendItems = legend()->legendItems();
    QList<QWidget*>::const_iterator it = legendItems.begin();
    while (it!=legendItems.end())
    {
        QwtLegendItem* p = dynamic_cast<QwtLegendItem*>(*(it++));
        if (p)
        {
            p->setChecked(true);
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
