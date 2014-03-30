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

#include "SlotListView.h"
#include "SlotItemModel.h"
#include "CmdAddSlot.h"
#include "CmdRemoveSlot.h"
#include "HiHatPedalElement.h"

#include <QtWidgets/QStyledItemDelegate>
#include <QtWidgets/QMenu>
#include <QtGui/QContextMenuEvent>
#include <QtWidgets/QUndoStack>

SlotListView::SlotListView(QWidget* pParent)
	: QListView(pParent)
	, _pUndoStack(NULL)
{
	setItemDelegate(new QStyledItemDelegate(this));
}

SlotListView::~SlotListView()
{
}

void SlotListView::contextMenuEvent(QContextMenuEvent* pEvent)
{
	QMenu menu(this);

	const QModelIndexList& selected = selectedIndexes();

	QAction* pActionAdd = new QAction(QIcon(":/icons/blue-document--plus.png"), tr("Add"), &menu);
	connect(pActionAdd, SIGNAL(triggered(bool)), this, SLOT(onAddSlot()));
	menu.addAction(pActionAdd);

	if (!selected.isEmpty())
	{
		QAction* pActionCopy = new QAction(QIcon(":/icons/blue-documents.png"), tr("Copy"), &menu);
		connect(pActionCopy, SIGNAL(triggered(bool)), this, SLOT(onCopySlot()));
		menu.addAction(pActionCopy);

		QAction* pActionRemove = new QAction(QIcon(":/icons/blue-document--minus.png"), tr("Remove"), &menu);
		connect(pActionRemove, SIGNAL(triggered(bool)), this, SLOT(onRemoveSlot()));
		menu.addAction(pActionRemove);
	}

	menu.exec(pEvent->globalPos());
}

Slot::Ptr SlotListView::createDefaultSlot(const QString& szSlotName)
{
	Slot::Ptr pDefaultSlot(new Slot());
	pDefaultSlot->setName(szSlotName.toStdString());

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

void SlotListView::onAddSlot()
{
	if (_pUndoStack)
	{
		_pUndoStack->push(new CmdAddSlot(model(), createDefaultSlot()));
	}
}

void SlotListView::onRemoveSlot()
{
	if (_pUndoStack)
	{
		const QModelIndexList& selected = selectedIndexes();
		for (int i=0; i<selected.size(); ++i)
		{
			_pUndoStack->push(new CmdRemoveSlot(selected[i]));
		}
	}
}

void SlotListView::onCopySlot()
{
	/* TODO
	Mutex::scoped_lock lock(_mutex);
	const QList<QListWidgetItem*>& selected = listWidgetSlots->selectedItems();
	if (!selected.empty())
	{
		QListWidgetItem* pSelected = selected[0];
		const std::string& szSlotName = pSelected->text().toStdString();
		Slot::List::iterator itSlot = std::find_if(_userSettings.configSlots.begin(), _userSettings.configSlots.end(), boost::bind(&Slot::getName, _1) == szSlotName);
		if (itSlot!=_userSettings.configSlots.end())
		{
			const Slot::Ptr& pSlotToDuplicate = *(itSlot);
			Slot::Ptr pDuplicated(new Slot(*pSlotToDuplicate));
			pDuplicated->setName(createNewSlotName(pSlotToDuplicate->getName().c_str()).toStdString());

			// Add the new slot internally and on the GUI
			_userSettings.configSlots.push_back(pDuplicated);

			QListWidgetItem* pNewItem = new QListWidgetItem(pDuplicated->getName().c_str());
			QVariant variant;
			variant.setValue(pDuplicated);
			pNewItem->setData(Qt::UserRole, variant);
			listWidgetSlots->addItem(pNewItem);
			pNewItem->setFlags(pNewItem->flags() | Qt::ItemIsEditable);
			pNewItem->setSelected(true);
		}
	}
	*/
}

void SlotListView::selectionChanged(const QItemSelection& selected, const QItemSelection& deselected)
{
	const QModelIndexList& selectedIndexes = selected.indexes();
	if (!selectedIndexes.isEmpty())
	{
		const QModelIndex& firstSelected = selectedIndexes.front();
		const Slot::Ptr& pSelectedSlot = firstSelected.data(Qt::UserRole).value<Slot::Ptr>();
		emit signalSlotChanged(pSelectedSlot);
	}

	QListView::selectionChanged(selected, deselected);
}

/* TODO
// Rename
void MainWindow::on_listWidgetSlots_itemChanged(QListWidgetItem* pItem)
{
	Mutex::scoped_lock lock(_mutex);
	const QVariant& variant = pItem->data(Qt::UserRole);
	if (!variant.isNull())
	{
		const Slot::Ptr& pSlot = variant.value<Slot::Ptr>();

		const QString& szNewName = pItem->text();
		QString szOldName = pSlot->getName().c_str();
		if (szNewName != szOldName)
		{
			// Find the orignal slot with szOldName
			Slot::List::iterator it = std::find_if(_userSettings.configSlots.begin(), _userSettings.configSlots.end(), boost::bind(&Slot::getName, _1) == szOldName.toStdString());
			if (it!=_userSettings.configSlots.end())
			{
				// Found, we rename the slot
				const Slot::Ptr& pSlotToRename = *(it);
				pSlotToRename->setName(createNewSlotName(szNewName).toStdString());

				listWidgetSlots->blockSignals(true);
				pItem->setText(pSlotToRename->getName().c_str());
				QVariant variant;
				variant.setValue(pSlotToRename);
				pItem->setData(Qt::UserRole, variant);
				listWidgetSlots->blockSignals(false);
			}
		}
	}
}
*/
