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
#include "Pad.h"
#include "DrumNote.h"
#include <boost/scoped_ptr.hpp>

class DrumNoteItemModel;
class DrumNoteItemDelegate;
class QMenu;
class QAction;
class MidiMessage;

class AddMidiNote : public QDialog, private Ui::AddMidiNote
{
	Q_OBJECT
public:
	AddMidiNote(const Pad::MidiDescription& padDescription = Pad::MidiDescription());
	virtual ~AddMidiNote();

	void setPadDescription(const Pad::MidiDescription& padDescription);
	DrumNotes getNotes() const;
	void clear();
	void showPrevNextButtons();
	int getPrevNextState() const {return _prevNextState;}

public slots:
	void onMidiNoteOn(const MidiMessage&);

protected slots:
	void on_pushButtonPrev_clicked(bool checked=false);
	void on_pushButtonNext_clicked(bool checked=false);
	void onCustomContextMenuRequested(const QPoint&);
	void onActionAddTriggered(bool);
	void onActionRemoveTriggered(bool);

private:
	boost::scoped_ptr<DrumNoteItemModel>	_pDrumNoteItemModel;
	boost::scoped_ptr<DrumNoteItemDelegate> _pDrumNoteItemDelegate;
	QMenu*									_pMenu;
	QAction*								_pActionAdd;
	QAction*								_pActionRemove;
	int _prevNextState;
};
