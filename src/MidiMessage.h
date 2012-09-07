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

#include <boost/chrono.hpp>

#include <list>
#include <vector>
#include <deque>
#include <set>

#include <windows.h>

class MidiMessage
{
public:
	typedef boost::chrono::high_resolution_clock Clock;

	enum IgnoreReason
	{
		NOT_IGNORED=0,
		IGNORED_BECAUSE_GHOST,
		IGNORED_BECAUSE_FOOT_CANCEL,
		IGNORED
	};

	typedef std::list<MidiMessage> List;
	typedef std::deque<MidiMessage> History;
	typedef std::vector<History> DictHistory;

public:
	MidiMessage(const Clock::time_point& time = Clock::time_point(), DWORD_PTR dwParam1=0, DWORD_PTR dwParam2=0): _tReceiveTime(time), _dwParam1(dwParam1), _dwParam2(dwParam2), _ignore(NOT_IGNORED), _alreadyModified(false), hiHatSpeed(0), hiHatAcceleration(0), _outputNote(0)
   	{
		_outputNote = getOriginalNote();
	}

	bool isNoteOnMsg() const {return getMsgType()==9;}
	bool isControllerMsg() const {return getMsgType()==11;}

	int getMsgType() const { return (_dwParam1 & 0x000000F0) >> 4;}
	int getChannel() const { return ((_dwParam1 & 0x0000000F) >> 0)+1;}
	int getOriginalNote() const { return (_dwParam1 & 0x0000FF00) >> 8;}
	int getOutputNote() const {return _outputNote;}
	void changeOutputNote(int note, bool bChangeModifiedState=true);
	int getValue() const { return (_dwParam1 & 0x00FF0000) >> 16;}
	void setValue(char value);
	int getTimestamp() const {return _dwParam2;}
	void setTimestamp(int timestamp) {_dwParam2 = timestamp;}

	bool isIgnored() const {return _ignore!=NOT_IGNORED;}
	IgnoreReason getIgnoreReason() const {return _ignore;}
	void ignore(IgnoreReason reason) {_ignore = reason;}

	bool isAlreadyModified() const {return _alreadyModified;}

	const Clock::time_point& getReceiveTime() const { return _tReceiveTime;}

	void print() const;
	std::string str() const;
	DWORD computeOutputMessage() const;
	bool isInTimeWindow(const MidiMessage& otherMessage, int timeWindow) const;
	int getAbsTimeDiff(const MidiMessage& otherMessage) const;

public:
	float hiHatSpeed;
	float hiHatAcceleration;

private:
	DWORD_PTR			_dwParam1;
	DWORD_PTR			_dwParam2;
	IgnoreReason		_ignore;
	bool				_alreadyModified;
	int					_outputNote; ///< By default the output note is the original note.
	Clock::time_point	_tReceiveTime; ///< receive time set at construction time.
};

typedef std::set<int> MidiNotes;
