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

#include <QtGui/QWidget>

struct Parameter;
class QStackedWidget;
class QCheckBox;
class QSlider;
class QSpinBox;
class QDoubleSpinBox;
class QLineEdit;
class QComboBox;

/**
 * Parameter Editor used in ParamItemDelegate.
 */
class ParamItemEditor : public QWidget
{
	Q_OBJECT

signals:
	void editFinished(QWidget*);

public:
	ParamItemEditor(QWidget* pParent=NULL);
	virtual ~ParamItemEditor();
	void setData(Parameter* pData);
	Parameter* getData() const {return _pData;}

private slots:
	void onCheckBoxValueChanged(int);
	void onSliderValueChanged(int);
	void onSpinBoxValueChanged(int);
	void onDoubleSliderValueChanged(int);
	void onDoubleSpinBoxValueChanged(double);
	void onLineEditValueChanged(const QString&);
	void onComboBoxIndexChanged(int);

private:
	Parameter*			_pData;
	QStackedWidget*		_pStackedWidget;
	QCheckBox*			_pCheckBox;
	QSpinBox*			_pSpinBox;
	QDoubleSpinBox*		_pDoubleSpinBox;
	QSlider*			_pSlider;
	QSlider*			_pDoubleSlider;
	QLineEdit*			_pLineEdit;
	QComboBox*			_pComboBox;
};
