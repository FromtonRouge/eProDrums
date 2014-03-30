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

#include "Slot.h"

#include <QtCore/QAbstractListModel>

#include <boost/filesystem.hpp>

class Settings;

class SlotItemModel : public QAbstractListModel
{
	Q_OBJECT

public:
	SlotItemModel(Settings* pSettings, Slot::List* pSlots = NULL, QObject* pParent = NULL);
	virtual ~SlotItemModel();

	void setSlots(Slot::List* pSlots);
	void clear();
	void insertSlot(int row, const Slot::Ptr& pSlot);
	void addSlot(const Slot::Ptr& pSlot);
	void removeSlot(const Slot::Ptr& pSlot);

	virtual int rowCount(const QModelIndex& parent = QModelIndex()) const;
	virtual QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const;
	virtual bool insertRows(int row, int count, const QModelIndex& parent = QModelIndex());
	virtual bool removeRows(int row, int count, const QModelIndex & parent = QModelIndex());

	void onDrumKitChanged(DrumKitMidiMap*, const boost::filesystem::path&);

private:
	Settings*	_pSettings;
	Slot::List* _pSlots;
};

