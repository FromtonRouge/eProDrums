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
#include "AbstractItemModel.h"

#include <boost/filesystem.hpp>

class Settings;

class SlotItemModel : public AbstractItemModel
{
	Q_OBJECT

public:
	enum CustomRole
	{
		SLOT_ROLE = Qt::UserRole
	};

public:
	SlotItemModel(Settings* pSettings, Slot::List* pSlots = NULL, QObject* pParent = NULL);
	virtual ~SlotItemModel();

	void setSlots(Slot::List* pSlots);
	void clear();
	void addSlot();
	void removeSlot(const Slot::Ptr& pSlot);

	virtual Qt::ItemFlags	flags(const QModelIndex& index) const;
	virtual int				rowCount(const QModelIndex& parent = QModelIndex()) const;
	virtual int				columnCount(const QModelIndex&) const {return 1;}
	virtual QModelIndex		index(int row, int column, const QModelIndex & parent = QModelIndex()) const;
	virtual QModelIndex		parent(const QModelIndex&) const;
	virtual QVariant		data(const QModelIndex& index, int role = Qt::DisplayRole) const;

	virtual void			insertRowsNoUndo(int row, int count, const QModelIndex& parent = QModelIndex());
	virtual void			removeRowsNoUndo(int row, int count, const QModelIndex & parent = QModelIndex());
	virtual void			setDataNoUndo(const QModelIndex&, const QVariant& , int);

	virtual UndoCommand*	createUndoRemoveRows(int row, int count, const QModelIndex& parent = QModelIndex());

	void onDrumKitChanged(DrumKitMidiMap*, const boost::filesystem::path&);

private:
	Slot::Ptr	createSlot(const QString& szSlotName);
	QString		createNewSlotName(const QString& szBaseName = QString("slot")) const;

private:
	Settings*	_pSettings;
	Slot::List* _pSlots;
};

