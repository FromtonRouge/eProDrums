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

#include <QtWidgets/QStyledItemDelegate>
#include <QtWidgets/QMenu>
#include <QtGui/QContextMenuEvent>

SlotListView::SlotListView(QWidget* pParent)
	: QListView(pParent)
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
		/* TODO
		QAction* pActionCopy = new QAction(QIcon(":/icons/blue-documents.png"), tr("Copy"), &menu);
		connect(pActionCopy, SIGNAL(triggered(bool)), this, SLOT(onCopySlot()));
		menu.addAction(pActionCopy);
		*/

		QAction* pActionRemove = new QAction(QIcon(":/icons/blue-document--minus.png"), tr("Remove"), &menu);
		connect(pActionRemove, SIGNAL(triggered(bool)), this, SLOT(onRemoveSlot()));
		menu.addAction(pActionRemove);
	}

	menu.exec(pEvent->globalPos());
}

void SlotListView::onAddSlot()
{
	SlotItemModel* pSlotItemModel = static_cast<SlotItemModel*>(model());
	pSlotItemModel->addSlot();
}

void SlotListView::onRemoveSlot()
{
	SlotItemModel* pSlotItemModel = static_cast<SlotItemModel*>(model());
	const QModelIndexList& selected = selectedIndexes();
	if (!selected.isEmpty())
	{
		pSlotItemModel->removeRow(selected.front().row(), selected.front().parent());
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
		const Slot::Ptr& pSelectedSlot = firstSelected.data(SlotItemModel::SLOT_ROLE).value<Slot::Ptr>();
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
