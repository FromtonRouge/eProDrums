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

#include "GhostKitControl.h"
#include "GhostControl.h"

#include <QtGui/QGridLayout>

GhostKitControl::GhostKitControl(const Pad::List& drumKit)
{
    setDrumKit(drumKit);
}

GhostKitControl::~GhostKitControl()
{
}

void GhostKitControl::setDrumKit(const Pad::List& drumKit)
{
    // Create/Get layout
    QGridLayout* pLayoutControls = dynamic_cast<QGridLayout*>(layout());
    if (!pLayoutControls)
    {
        pLayoutControls = new QGridLayout();
        setLayout(pLayoutControls);
    }

    // Creating/Updating drumkit
    int column = 0;
    Pad::List::const_iterator it = drumKit.begin();
    while (it!=drumKit.end())
    {
        const Pad::List::value_type& pDrumKitElement = *(it++);
        if (pDrumKitElement->getType() == Pad::HIHAT_PEDAL)
        {
            continue;
        }

        Map::iterator itEl = _map.find(pDrumKitElement->getType());
        if (itEl==_map.end())
        {
            // Create
            GhostControl* p = new GhostControl(pDrumKitElement);
            _map[pDrumKitElement->getType()] = p;
            pLayoutControls->addWidget(p, 0, column++);
        }
        else
        {
            // Update
            itEl->second->setPad(pDrumKitElement);
        }
    }
}
