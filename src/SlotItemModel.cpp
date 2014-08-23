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
#include "HiHatPedalElement.h"
#include "CmdRemoveSlots.h"

#include <algorithm>

SlotItemModel::SlotItemModel(Settings* pSettings, Slot::List* pSlots, QObject* pParent)
	: AbstractItemModel(pParent)
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
	_pSettings->reloadDrumKitMidiMap();
}

Qt::ItemFlags SlotItemModel::flags(const QModelIndex& index) const
{
	if (index.isValid())
	{
		return Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable | Qt::ItemNeverHasChildren;
	}
	return Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable;
}

int SlotItemModel::rowCount(const QModelIndex&) const
{
	if (_pSlots)
	{
		return _pSlots->size();
	}
	return 0;
}

QModelIndex SlotItemModel::index(int row, int column, const QModelIndex& ) const
{
	if (_pSlots && row < static_cast<int>(_pSlots->size()))
	{
		return createIndex(row, column, row);
	}
	return QModelIndex();
}

QModelIndex SlotItemModel::parent(const QModelIndex&) const
{
	return QModelIndex();
}

QVariant SlotItemModel::data(const QModelIndex& index, int role) const
{
	QVariant result;
	if (!_pSlots)
	{
		return result;
	}

	const Slot::Ptr& pSlot = _pSlots->at(index.row());
	if (!pSlot)
	{
		return result;
	}

	switch (role)
	{
	case Qt::EditRole:
	case Qt::DisplayRole:
		{
			result = pSlot->getName();
			break;
		}
	case SLOT_ROLE:
		{
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

void SlotItemModel::setDataNoUndo(const QModelIndex& index, const QVariant& variant, int role)
{
	Slot::Ptr& pSlot = _pSlots->at(index.row());

	switch (role)
	{
	case Qt::EditRole: // Edit only the name
		{
			pSlot->setName(variant.toString());
			emit dataChanged(index, index, QVector<int>(1, role));
			break;
		}
	case SLOT_ROLE:
		{
			pSlot = variant.value<Slot::Ptr>();
			emit dataChanged(index, index, QVector<int>(1, role));
			break;
		}
	default:
		{
			break;
		}
	}
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

void SlotItemModel::insertRowsNoUndo(int row, int count, const QModelIndex& parent)
{
	if (!_pSlots)
	{
		return;
	}

	beginInsertRows(parent, row, row+count-1);

	for (int i=0; i<count; ++i)
	{
		_pSlots->insert(_pSlots->begin()+row+i, Slot::Ptr());
	}

	endInsertRows();
}

void SlotItemModel::addSlot()
{
	if (_pSlots) 
	{
		// Get a new slot name first
		QString szSlotName("default");
		if (rowCount())
		{
			szSlotName = createNewSlotName();
		}

		beginUndoMacro(tr("1 slot added"));
		if (insertRow(rowCount()))
		{
			const QModelIndex& newIndex = index(rowCount()-1, 0);

			QVariant variant;
			variant.setValue(createSlot(szSlotName));
			setData(newIndex, variant, SLOT_ROLE);

			_pSettings->reloadDrumKitMidiMap();
		}
		endUndoMacro();
	}
}

void SlotItemModel::removeRowsNoUndo(int row, int count, const QModelIndex& parent)
{
	if (!_pSlots)
	{
		return;
	}

	beginRemoveRows(parent, row, row+count-1);

	_pSlots->erase(_pSlots->begin()+row, _pSlots->begin()+row+count);

	endRemoveRows();
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

QString SlotItemModel::createNewSlotName(const QString& szBaseName) const
{
	Slot::List::const_iterator it = std::find_if(_pSlots->begin(), _pSlots->end(), boost::bind(&Slot::getName, _1) == szBaseName);
	if (it==_pSlots->end())
	{
		return szBaseName;
	}

	// Find new name
	QString szNewName("error");
	for (size_t i=0; i<_pSlots->size(); i++)
	{
		QString szName = QString("%1_%2").arg(szBaseName).arg(i+1);
		Slot::List::const_iterator it = std::find_if(_pSlots->begin(), _pSlots->end(), boost::bind(&Slot::getName, _1) == szName);
		if (it==_pSlots->end())
		{
			szNewName = szName;
			break;
		}
	}
	return szNewName;
}

Slot::Ptr SlotItemModel::createSlot(const QString& szSlotName)
{
	Slot::Ptr pDefaultSlot(new Slot());
	pDefaultSlot->setName(szSlotName);

	// Snare
	const Pad::Ptr& pElSnare = Pad::Ptr(new Pad(Pad::SNARE, Pad::NOTE_SNARE));
	pElSnare->typeFlam->set(Pad::TOM1);
	pDefaultSlot->getPads().push_back(pElSnare);

	// Hi Hat
	const Pad::Ptr& pElHihat = Pad::Ptr(new Pad(Pad::HIHAT, Pad::NOTE_HIHAT)); 
	pElHihat->typeFlam->set(Pad::SNARE);
	pDefaultSlot->getPads().push_back(pElHihat);

	// Hi Hat Pedal
	const HiHatPedalElement::Ptr& pElHihatPedal = HiHatPedalElement::Ptr(new HiHatPedalElement());
	pDefaultSlot->getPads().push_back(pElHihatPedal);

	const Pad::Ptr& pElTom1 = Pad::Ptr(new Pad(Pad::TOM1, Pad::NOTE_TOM1)); 
	pElTom1->typeFlam->set(Pad::TOM2);
	pDefaultSlot->getPads().push_back(pElTom1);

	const Pad::Ptr& pElTom2 = Pad::Ptr(new Pad(Pad::TOM2, Pad::NOTE_TOM2)); 
	pElTom2->typeFlam->set(Pad::TOM3);
	pDefaultSlot->getPads().push_back(pElTom2);

	const Pad::Ptr& pElTom3 = Pad::Ptr(new Pad(Pad::TOM3, Pad::NOTE_TOM3)); 
	pElTom3->typeFlam->set(Pad::TOM2);
	pDefaultSlot->getPads().push_back(pElTom3);

	const Pad::Ptr& pElCrash1 = Pad::Ptr(new Pad(Pad::CRASH1, Pad::NOTE_CRASH1)); 
	pElCrash1->typeFlam->set(Pad::RIDE);
	pDefaultSlot->getPads().push_back(pElCrash1);

	const Pad::Ptr& pElCrash2 = Pad::Ptr(new Pad(Pad::CRASH2, Pad::NOTE_CRASH2)); 
	pElCrash2->typeFlam->set(Pad::RIDE);
	pDefaultSlot->getPads().push_back(pElCrash2);

	const Pad::Ptr& pElCrash3 = Pad::Ptr(new Pad(Pad::CRASH3, Pad::NOTE_CRASH3)); 
	pDefaultSlot->getPads().push_back(pElCrash3);

	const Pad::Ptr& pElRide = Pad::Ptr(new Pad(Pad::RIDE, Pad::NOTE_RIDE)); 
	pElRide->typeFlam->set(Pad::CRASH2);
	pDefaultSlot->getPads().push_back(pElRide);

	const Pad::Ptr& pElBassDrum = Pad::Ptr(new Pad(Pad::BASS_DRUM, Pad::NOTE_BASS_DRUM)); 
	pDefaultSlot->getPads().push_back(pElBassDrum);

	return pDefaultSlot;
}

UndoCommand* SlotItemModel::createUndoRemoveRows(int row, int count, const QModelIndex& parent)
{
	return new CmdRemoveSlots(this, row, count, parent);
}
