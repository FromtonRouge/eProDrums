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
#include <QtGui/QGridLayout>
#include <QtGui/QDialogButtonBox>
#include <QtGui/QSpinBox>
#include <boost/lexical_cast.hpp>

AddMidiNote::AddMidiNote(const Notes& notes):_prevNextState(0)
{
	setupUi(this);
	pushButtonPrev->hide();
	pushButtonNext->hide();
	setNotes(notes);
}

AddMidiNote::~AddMidiNote()
{
}

AddMidiNote::Notes AddMidiNote::getNotes() const
{
	return _notes;
}

void AddMidiNote::onMidiNoteOn(int note, int velocity)
{
	if (velocity)
	{
		if (_notes.find(note) == _notes.end())
		{
			listWidgetNotes->addItem(boost::lexical_cast<std::string>(note).c_str());
			_notes.insert(note);
		}
	}
}

void AddMidiNote::on_pushButtonAdd_clicked(bool)
{
	QDialog dlg(this);
	dlg.setWindowTitle("Add note manually");

	QGridLayout layout(&dlg);
	QSpinBox spinBox(&dlg);
	spinBox.setMaximum(127);
	QLabel label(&dlg);
	label.setText("Midi note");
	layout.addWidget(&label, 0, 0);
	layout.addWidget(&spinBox, 0, 1);
	QDialogButtonBox buttonBox(QDialogButtonBox::Ok|QDialogButtonBox::Cancel);
	layout.addWidget(&buttonBox, 1, 1);

	connect(&buttonBox, SIGNAL(accepted()), &dlg, SLOT(accept()));
	connect(&buttonBox, SIGNAL(rejected()), &dlg, SLOT(reject()));

	if (dlg.exec())
	{
		int note = spinBox.value();
		if (_notes.find(note) == _notes.end())
		{
			listWidgetNotes->addItem(boost::lexical_cast<std::string>(note).c_str());
		}
		_notes.insert(note);
	}
}

void AddMidiNote::on_pushButtonDel_clicked(bool)
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
			_notes.erase(note);
		}
	}
}

void AddMidiNote::clear()
{
	_prevNextState = 0;
	listWidgetNotes->clear();
	_notes.clear();
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

void AddMidiNote::setNotes(const Notes& notes)
{
	_notes = notes;
	Notes::const_iterator it = _notes.begin();
	while (it!=_notes.end())
	{
		int note = *(it++);
		listWidgetNotes->addItem(boost::lexical_cast<std::string>(note).c_str());
	}
}
