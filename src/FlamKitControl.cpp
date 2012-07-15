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

#include "FlamKitControl.h"
#include "FlamControl.h"

Q_DECLARE_METATYPE(FlamControl*)

FlamKitControl::FlamKitControl(const Pad::List& drumKit)
{
    setupUi(this);

    setDrumKit(drumKit);
}

FlamKitControl::~FlamKitControl()
{
}

void FlamKitControl::setDrumKit(const Pad::List& drumKit)
{
    QGridLayout* pLayoutControls = dynamic_cast<QGridLayout*>(layout());
    if (!pLayoutControls)
    {
        return;
    }

    // Clear
    listWidgetDrumKit->blockSignals(true);
    listWidgetDrumKit->clear();
    listWidgetDrumKit->blockSignals(false);

    Pad::List::const_iterator it = drumKit.begin();
    while (it!=drumKit.end())
    {
        const Pad::List::value_type& pDrumKitElement = *(it++);
        if (pDrumKitElement->getType() == Pad::HIHAT_PEDAL)
        {
            continue;
        }

        // Create
        QListWidgetItem* pNewItem = new QListWidgetItem(pDrumKitElement->getName().c_str());

        FlamControl* pFlamControl = NULL;
        Map::iterator itEl = _map.find(pDrumKitElement->getType());
        if (itEl==_map.end())
        {
            pFlamControl = new FlamControl(drumKit, pDrumKitElement);
            _map[pDrumKitElement->getType()] = pFlamControl;
            pLayoutControls->addWidget(pFlamControl, 0,1);
        }
        else
        {
            pFlamControl = _map[pDrumKitElement->getType()];
            pFlamControl->setDrumKit(drumKit);
            pFlamControl->setPad(pDrumKitElement);
        }

        pFlamControl->hide();

        QVariant variant;
        variant.setValue(pFlamControl);
        pNewItem->setData(Qt::UserRole, variant);
        listWidgetDrumKit->addItem(pNewItem);
    }

    // Display the first element by Default
    if (listWidgetDrumKit->count())
    {
        listWidgetDrumKit->item(0)->setSelected(true);
    }
}

void FlamKitControl::on_listWidgetDrumKit_itemSelectionChanged()
{
    // Hide all first
    for (int i=0;i<listWidgetDrumKit->count(); i++)
    {
        QListWidgetItem* p = listWidgetDrumKit->item(i);

        QVariant v = p->data(Qt::UserRole);
        if (!v.isNull())
        {
            FlamControl* pFlamControl = v.value<FlamControl*>();
            if (pFlamControl)
            {
                pFlamControl->hide();
            }
        }
    }

    // Display the selected one
    const QList<QListWidgetItem*>& selected = listWidgetDrumKit->selectedItems();
    if (!selected.empty())
    {
        QVariant v = selected[0]->data(Qt::UserRole);
        if (!v.isNull())
        {
            FlamControl* pFlamControl = v.value<FlamControl*>();
            if (pFlamControl)
            {
                pFlamControl->show();
            }
        }
    }
}
