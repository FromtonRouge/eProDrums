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

#include "EProPlotMarker.h"
#include <qwt_plot.h>

EProPlotMarker::EProPlotMarker(QwtPlot* pPlot, QwtSymbol::Style style, const QColor& inColor, const QColor& outColor, const QSize& size)
{
	setLineStyle(QwtPlotMarker::NoLine);
	setLabelAlignment(Qt::AlignHCenter|Qt::AlignTop);
	setLinePen(QPen(inColor, 0, Qt::DashDotLine));
    setSymbol(new QwtSymbol(style, inColor, outColor, size));
	attach(pPlot);
	setVisible(false);
}

EProPlotMarker::~EProPlotMarker()
{
}

void EProPlotMarker::setColor(const QColor& color)
{
	setLinePen(QPen(color, 0, Qt::DashDotLine));
	const_cast<QwtSymbol*>(symbol())->setColor(color);
}

void EProPlotMarker::move(double x, double y)
{
	setValue(x, y);
}

void EProPlotMarker::setOutlineColor(const QColor& qColorOutline)
{
	const_cast<QwtSymbol*>(symbol())->setPen(qColorOutline);
}
