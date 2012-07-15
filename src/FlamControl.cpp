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

#include "FlamControl.h"
#include "ValueControl.h"
#include "DoubleValueControl.h"

#include <QtGui/QGridLayout>
#include <QtGui/QGroupBox>
#include <QtGui/QSizePolicy>
#include <QtGui/QComboBox>

#include <boost/bind.hpp>
#include <boost/lambda/lambda.hpp>
#include <algorithm>

FlamControl::FlamControl(const Pad::List& drumKit, const boost::shared_ptr<Pad>& pDrumKitElement):PadHandler(pDrumKitElement)
{
    setupUi(this);
	comboBoxDrumKitElement->setToolTip("Define the remapping to do on the 2nd hit");

	_pSmallWindow = new ValueControl("Time Window", 0, 150);
	_pSmallWindow->setToolTip("[FTW1] Timing window where double hits are converted to a flam, the velocity of the 2nd hit does not matter");
	gridLayoutTW1->addWidget(_pSmallWindow, 0,0);
	connect(_pSmallWindow, SIGNAL(valueChanged(int)), this, SLOT(onSmallWindowChanged(int)));

	_pLargeWindow = new ValueControl("Time Window", 0, 150);
	_pLargeWindow->setToolTip("[FTW2] Another timing window where double hits are converted to a flam and where the velocity \nof the 2nd hit must be greater or equal to [2nd Hit Velocity Factor] of the 1st hit");
	gridLayoutTW2->addWidget(_pLargeWindow, 0,0);
	connect(_pLargeWindow, SIGNAL(valueChanged(int)), this, SLOT(onLargeWindowChanged(int)));

	_pVelocityFactor = new DoubleValueControl("2nd Hit Velocity Factor", 1.0, 2.0);
	_pVelocityFactor->setToolTip("Define how times the 2nd hit must be greater to the 1st hit");
	gridLayoutTW2->addWidget(_pVelocityFactor, 0, 1);
	connect(_pVelocityFactor, SIGNAL(valueChanged(double)), this, SLOT(onVelocityChanged(double)));

	_pFlamCancelDuringRoll = new ValueControl("Flam cancel (ms)", 0, 250);
	_pFlamCancelDuringRoll->setToolTip("If the previous hit before the flam is under the specified time, the flam is canceled");
	gridLayoutFlamCancel->addWidget(_pFlamCancelDuringRoll, 0, 1);
	connect(_pFlamCancelDuringRoll, SIGNAL(valueChanged(int)), this, SLOT(onFlamCancelDuringRoll(int)));

    setDrumKit(drumKit);
}

FlamControl::~FlamControl()
{
}

void FlamControl::onSmallWindowChanged(int value)
{
    getPad()->setFlamTimeWindow1(value);
}

void FlamControl::onLargeWindowChanged(int value)
{
    getPad()->setFlamTimeWindow2(value);
}

void FlamControl::onVelocityChanged(double value)
{
    getPad()->setFlamVelocityFactor(value);
}

void FlamControl::updateControl()
{
    Pad* pDrumKitElement = getPad();

    _pSmallWindow->blockSignals(true);
    _pSmallWindow->setValue(pDrumKitElement->getFlamTimeWindow1());
    _pSmallWindow->blockSignals(false);

    _pLargeWindow->blockSignals(true);
    _pLargeWindow->setValue(pDrumKitElement->getFlamTimeWindow2());
    _pLargeWindow->blockSignals(false);

    _pVelocityFactor->blockSignals(true);
    _pVelocityFactor->setValue(pDrumKitElement->getFlamVelocityFactor());
    _pVelocityFactor->blockSignals(false);

    _pFlamCancelDuringRoll->blockSignals(true);
    _pFlamCancelDuringRoll->setValue(pDrumKitElement->getFlamCancelDuringRoll());
    _pFlamCancelDuringRoll->blockSignals(false);

    Pad::Type typeFlam = pDrumKitElement->getTypeFlam();
    Pad::List::iterator it = std::find_if(myDrumKit.begin(), myDrumKit.end(), boost::bind(&Pad::getType, boost::lambda::_1) == typeFlam);
    if (it!=myDrumKit.end())
    {
        int index = comboBoxDrumKitElement->findText((*it)->getName().c_str());
        if (index>=0)
        {
            comboBoxDrumKitElement->blockSignals(true);
            comboBoxDrumKitElement->setCurrentIndex(index);
            comboBoxDrumKitElement->blockSignals(false);
        }
    }
}

void FlamControl::setDrumKit(const Pad::List& drumKit)
{
    comboBoxDrumKitElement->blockSignals(true);
    comboBoxDrumKitElement->clear();

    Pad::List::const_iterator it = drumKit.begin();
    while (it!=drumKit.end())
    {
        const Pad::List::value_type& pDrumKitElement = *(it++);
        comboBoxDrumKitElement->addItem(pDrumKitElement->getName().c_str());
    }
    myDrumKit = drumKit;
    comboBoxDrumKitElement->blockSignals(false);

    updateControl();
}

void FlamControl::on_comboBoxDrumKitElement_currentIndexChanged(const QString& text)
{
    Pad::List::iterator it = std::find_if(myDrumKit.begin(), myDrumKit.end(), boost::bind(&Pad::getName, boost::lambda::_1) == text.toStdString());
    if (it!=myDrumKit.end())
    {
        getPad()->setTypeFlam((*it)->getType());
    }
}

void FlamControl::onFlamCancelDuringRoll(int value)
{
    getPad()->setFlamCancelDuringRoll(value);
}
