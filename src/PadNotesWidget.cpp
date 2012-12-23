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

#include "PadNotesWidget.h"
#include "AddMidiNote.h"

#include <boost/lexical_cast.hpp>

PadNotesWidget::PadNotesWidget(QWidget* pParent): QWidget(pParent)
{
	setupUi(this);
}

PadNotesWidget::~PadNotesWidget()
{
}

void PadNotesWidget::on_pushButtonAdd_clicked(bool)
{
	AddMidiNote dlg(_padDescription);
	dlg.setWindowTitle(Pad::getName(_padDescription.type).c_str());
	connect(this, SIGNAL(onMidiIn(const MidiMessage&)), &dlg, SLOT(onMidiNoteOn(const MidiMessage&)));

	if (dlg.exec())
	{
		_padDescription.inputNotes = dlg.getNotes();
		setDrumNotes(_padDescription.inputNotes);
		emit editFinished(this);
	}
}

void PadNotesWidget::addMidiNote(int note)
{
	if (_padDescription.inputNotes.findMidiNote(note)==_padDescription.inputNotes.endMidiNote())
	{
		_padDescription.inputNotes.insert(DrumNote(note));
		listWidgetNotes->addItem(boost::lexical_cast<std::string>(note).c_str());
	}
}

void PadNotesWidget::setDrumNotes(const DrumNotes& notes)
{
	listWidgetNotes->clear();
	DrumNotes::IteratorMidiNote it = notes.beginMidiNote();
	while (it!=notes.endMidiNote())
	{
		const DrumNote& drumNote = *(it++);
		listWidgetNotes->addItem(boost::lexical_cast<std::string>(drumNote.midiNote).c_str());
	}
}

void PadNotesWidget::setPadDescription(const Pad::MidiDescription& description)
{
	_padDescription = description;
	setDrumNotes(_padDescription.inputNotes);
}

void PadNotesWidget::on_pushButtonRemove_clicked(bool)
{
	if (listWidgetNotes->count())
	{
		QListWidgetItem* pCurrent = listWidgetNotes->takeItem(listWidgetNotes->currentRow());
		if (!pCurrent)
		{
			pCurrent = listWidgetNotes->takeItem(0);
		}

		if (pCurrent)
		{
			int note = boost::lexical_cast<int>(pCurrent->text().toStdString());
			_padDescription.inputNotes.eraseMidiNote(note);

			emit editFinished(this);
		}
	}
}
