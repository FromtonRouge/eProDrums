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

#include "Pad.h"
#include "ui_PadNotesWidget.h"

#include <QtGui/QWidget>

class MidiMessage;

class PadNotesWidget : public QWidget, private Ui::PadNotesWidget
{
	Q_OBJECT

public:
	PadNotesWidget(QWidget* pParent);
	virtual ~PadNotesWidget();

	void setPadDescription(const Pad::MidiDescription& description);
	const Pad::MidiDescription& getPadDescription() const {return _padDescription;}

signals:
	void onMidiIn(const MidiMessage&);
	void editFinished(QWidget*);

private slots:
	void on_pushButtonAdd_clicked(bool checked=false);
	void on_pushButtonRemove_clicked(bool checked=false);

private:
	void addMidiNote(int note);
	void setDrumNotes(const DrumNotes& notes);

private:
	Pad::MidiDescription _padDescription;
};
