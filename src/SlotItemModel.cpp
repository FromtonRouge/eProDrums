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

#include "SlotItemModel.h"
#include "Settings.h"
#include <algorithm>

SlotItemModel::SlotItemModel(Settings* pSettings, Slot::List* pSlots, QObject* pParent)
	: QAbstractListModel(pParent)
	, _pSettings(pSettings)
	, _pSlots(pSlots)
{
	_pSettings->signalKitDefined.connect(boost::bind(&SlotItemModel::onDrumKitChanged, this, _1, _2));
}

SlotItemModel::~SlotItemModel()
{
}

void SlotItemModel::setSlots(Slot::List* pSlots)
{
	beginResetModel();
	_pSlots = pSlots;
	endResetModel();
}

int SlotItemModel::rowCount(const QModelIndex&) const
{
	if (_pSlots)
	{
		return _pSlots->size();
	}
	return 0;
}

QVariant SlotItemModel::data(const QModelIndex& index, int role) const
{
	QVariant result;
	if (!_pSlots)
	{
		return result;
	}

	switch (role)
	{
	case Qt::EditRole:
	case Qt::DisplayRole:
		{
			const Slot::Ptr& pSlot = _pSlots->at(index.row());
			result = pSlot->getName().c_str();
			break;
		}
	case Qt::UserRole:
		{
			const Slot::Ptr& pSlot = _pSlots->at(index.row());
			result.setValue(pSlot);
			break;
		}
	default:
		{
			break;
		}
	}

	return result;
}

void SlotItemModel::clear()
{
	beginResetModel();
	if (_pSlots)
	{
		_pSlots->clear();
	}
	endResetModel();
}

bool SlotItemModel::insertRows(int row, int count, const QModelIndex& parent)
{
	if (!_pSlots)
	{
		return false;
	}

	beginInsertRows(parent, row, row+count-1);

	for (int i=0; i<count; ++i)
	{
		_pSlots->insert(_pSlots->begin()+row+i, Slot::Ptr());
	}

	endInsertRows();
	return true;
}

void SlotItemModel::insertSlot(int row, const Slot::Ptr& pSlot)
{
	if (_pSlots && insertRow(row))
	{
		_pSlots->at(row) = pSlot;
		_pSettings->reloadDrumKitMidiMap();
	}
}

void SlotItemModel::addSlot(const Slot::Ptr& pSlot)
{
	insertSlot(rowCount(), pSlot);
}

bool SlotItemModel::removeRows(int row, int count, const QModelIndex& parent)
{
	if (!_pSlots)
	{
		return false;
	}

	beginRemoveRows(parent, row, row+count-1);

	_pSlots->erase(_pSlots->begin()+row, _pSlots->begin()+row+count);

	endRemoveRows();

	return true;
}

void SlotItemModel::removeSlot(const Slot::Ptr& pSlot)
{
	if (_pSlots)
	{
		Slot::List::const_iterator it = std::find(_pSlots->begin(), _pSlots->end(), pSlot);
		if (it!=_pSlots->end())
		{
			int row = it-_pSlots->begin();
			removeRow(row);
		}
	}
}

void SlotItemModel::onDrumKitChanged(DrumKitMidiMap* pDrumKit, const boost::filesystem::path& path)
{
	if (_pSlots)
	{
		std::for_each(_pSlots->begin(), _pSlots->end(), boost::bind(&Slot::onDrumKitLoaded, _1, pDrumKit, path));
	}
}
