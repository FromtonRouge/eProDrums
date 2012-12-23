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

#include "AddMidiNote.h"
#include "DrumNoteItemModel.h"
#include "DrumNoteItemDelegate.h"
#include "MidiMessage.h"
#include <QtGui/QGridLayout>
#include <QtGui/QDialogButtonBox>
#include <QtGui/QSpinBox>
#include <QtGui/QTableWidgetItem>
#include <QtGui/QMenu>
#include <boost/lexical_cast.hpp>

AddMidiNote::AddMidiNote(const Pad::MidiDescription& padDescription):
	_pDrumNoteItemModel(new DrumNoteItemModel()),
	_pDrumNoteItemDelegate(new DrumNoteItemDelegate(padDescription.type)),
	_prevNextState(0)
{
	setupUi(this);

	_pMenu = new QMenu(this);
	_pActionAdd = _pMenu->addAction(tr("Add"));
	connect(_pActionAdd, SIGNAL(triggered(bool)), this, SLOT(onActionAddTriggered(bool)));
	_pActionRemove = _pMenu->addAction(tr("Remove"));
	connect(_pActionRemove, SIGNAL(triggered(bool)), this, SLOT(onActionRemoveTriggered(bool)));

	connect(tableViewNotes, SIGNAL(customContextMenuRequested(const QPoint&)), this, SLOT(onCustomContextMenuRequested(const QPoint&)));
	tableViewNotes->setContextMenuPolicy(Qt::CustomContextMenu);

	tableViewNotes->verticalHeader()->hide();
	tableViewNotes->horizontalHeader()->setStretchLastSection(true);
	tableViewNotes->setModel(_pDrumNoteItemModel.get());
	tableViewNotes->setItemDelegate(_pDrumNoteItemDelegate.get());

	setPadDescription(padDescription);

	pushButtonPrev->hide();
	pushButtonNext->hide();
}

AddMidiNote::~AddMidiNote()
{
}

void AddMidiNote::onCustomContextMenuRequested(const QPoint&)
{
	QItemSelectionModel* pItemSelectionModel = tableViewNotes->selectionModel();
	if (pItemSelectionModel)
	{
		QModelIndexList selected = pItemSelectionModel->selectedIndexes();
		_pActionAdd->setEnabled(selected.empty());
		_pActionRemove->setEnabled(!selected.empty());
	}

	_pMenu->exec(QCursor::pos());
}

void AddMidiNote::onActionAddTriggered(bool)
{
	int row = _pDrumNoteItemModel->add();
	tableViewNotes->openPersistentEditor(_pDrumNoteItemModel->index(row, 1));
}

void AddMidiNote::onActionRemoveTriggered(bool)
{
	QItemSelectionModel* pItemSelectionModel = tableViewNotes->selectionModel();
	if (pItemSelectionModel)
	{
		QModelIndexList selected = pItemSelectionModel->selectedIndexes();
		while (!selected.empty())
		{
			_pDrumNoteItemModel->removeRow(selected[0].row());
			selected = pItemSelectionModel->selectedIndexes();
		}
	}
}

DrumNotes AddMidiNote::getNotes() const
{
	return _pDrumNoteItemModel->getDrumNotes();
}

void AddMidiNote::onMidiNoteOn(const MidiMessage& midiMessage)
{
	if (midiMessage.getValue())
	{
		int row = _pDrumNoteItemModel->add(midiMessage.getOriginalNote());
		tableViewNotes->openPersistentEditor(_pDrumNoteItemModel->index(row, 1));
	}
}

void AddMidiNote::clear()
{
	_prevNextState = 0;
	_pDrumNoteItemModel->removeRows(0, _pDrumNoteItemModel->rowCount());
	buttonBox->setFocus(Qt::OtherFocusReason);
}

void AddMidiNote::showPrevNextButtons()
{
	pushButtonPrev->show();
	pushButtonNext->show();
}

void AddMidiNote::on_pushButtonPrev_clicked(bool)
{
	_prevNextState = -1;
	accept();
}

void AddMidiNote::on_pushButtonNext_clicked(bool)
{
	_prevNextState = 1;
	accept();
}

void AddMidiNote::setPadDescription(const Pad::MidiDescription& padDescription)
{
	_pDrumNoteItemDelegate->setType(padDescription.type);
	_pDrumNoteItemModel->setDrumNotes(padDescription.inputNotes);
	for(int i=0;i<_pDrumNoteItemModel->rowCount();++i)
	{
		tableViewNotes->openPersistentEditor(_pDrumNoteItemModel->index(i, 1));
	}
	tableViewNotes->resizeColumnToContents(0);
}
