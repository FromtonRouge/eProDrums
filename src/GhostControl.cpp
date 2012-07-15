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

#include "GhostControl.h"
#include "ValueControl.h"
#include "Pad.h"

#include <QtGui/QGridLayout>
#include <QtGui/QGroupBox>

GhostControl::GhostControl(const boost::shared_ptr<Pad>& pDrumKitElement):PadHandler(pDrumKitElement),
    _pGroupBox(new QGroupBox)
{
	QGridLayout* pLayoutControls = new QGridLayout;
	pLayoutControls->setMargin(0);
	_pGhostLimit = new ValueControl("Velocity");
	_pGhostLimit->setToolTip("Under the specified [Velocity] the midi note is ignored allowing you \nto play ghost notes without breaking the combo");
	pLayoutControls->addWidget(_pGhostLimit, 0,0);
	_pGroupBox->setLayout(pLayoutControls);

	QGridLayout* pWidgetLayout = new QGridLayout;
	pWidgetLayout->setMargin(0);
	pWidgetLayout->addWidget(_pGroupBox, 0,0);
	setLayout(pWidgetLayout);

	connect(_pGhostLimit, SIGNAL(valueChanged(int)), this, SLOT(onValueChanged(int)));
    updateControl();
}

GhostControl::~GhostControl()
{
}

void GhostControl::onValueChanged(int value)
{
    getPad()->setGhostVelocityLimit(value);
}

void GhostControl::updateControl()
{
	_pGroupBox->setTitle(getPad()->getName().c_str());
    _pGhostLimit->blockSignals(true);
    _pGhostLimit->setValue(getPad()->getGhostVelocityLimit());
    _pGhostLimit->blockSignals(false);
}
