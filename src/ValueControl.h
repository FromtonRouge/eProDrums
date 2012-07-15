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

#include "ui_ValueControl.h"

class QColor;
class QwtThermo;
class ValueControl : public QWidget, private Ui::ValueControl
{
    Q_OBJECT

public:
	ValueControl(const std::string& szLabel, int minValue=0, int maxValue=127);
	virtual ~ValueControl();

	int getValue() const;
    void setValue(int value);

    void showThermo();
    void hideThermo();
    void setThermoAlarmLevel(int value);

    void setThermoFillColor(const QColor& color);
    void setThermoAlarmColor(const QColor& color);

signals:
	void valueChanged(int);

private slots:
	void on_slider_valueChanged(int);
	void on_spinBox_valueChanged(int);

public slots:
    void setThermoValue(int value);

private:
	QwtThermo* _pThermo;
};
