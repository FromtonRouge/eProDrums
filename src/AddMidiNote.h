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

#include "ui_AddMidiNote.h"
#include <set>

class AddMidiNote : public QDialog, private Ui::AddMidiNote
{
	Q_OBJECT
public:
	typedef std::set<int> Notes;

public:
	AddMidiNote(const Notes& notes = Notes());
	virtual ~AddMidiNote();

	void setNotes(const Notes& notes);
	Notes getNotes() const;
	void clear();
	void showPrevNextButtons();
	int getPrevNextState() const {return _prevNextState;}

public slots:
	void onMidiNoteOn(int, int);

protected slots:
	void on_pushButtonAdd_clicked(bool checked=false);
	void on_pushButtonDel_clicked(bool checked=false);
	void on_pushButtonPrev_clicked(bool checked=false);
	void on_pushButtonNext_clicked(bool checked=false);

private:
	Notes _notes;
	int _prevNextState;
};
