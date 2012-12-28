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

#include <portmidi.h>
#include <porttime.h>

#include <list>
#include <vector>
#include <deque>
#include <string>

class Pad;

class MidiMessage
{
public:
	typedef unsigned char Status;		///< 1 byte for midi status (status is located at the first byte of Midi message 0x000000FF)
	typedef int Data;			///< 1 byte for midi data1 or data2 (data1 is located at the second byte of a Midi message 0x0000FF00, data2 at the third 0x00FF0000)

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
	MidiMessage(PtTimestamp timestamp=0, Status status = 0, Data data1=0, Data data2=0):
	   	hiHatSpeed(0),
	   	hiHatAcceleration(0),
	   	hiHatJerk(0),
		padType(-1),
		_timestamp(timestamp),
		_sentTimestamp(_timestamp),
		_status(status),
	   	_data1(data1),
	   	_data2(data2),
	   	_outputNote(_data1),
	   	_ignore(NOT_IGNORED),
	   	_alreadyModified(false)
   	{
	}

	bool isNoteOnMsg() const {return (_status & 0xF0) == 0x90;}
	bool isNoteOffMsg() const {return (_status & 0xF0) == 0x80;}
	bool isControllerMsg() const {return (_status & 0xF0) == 0xB0;}

	PtTimestamp getTimestamp() const {return _timestamp;}
	void setTimestamp(PtTimestamp t) {_timestamp = t;}
	PtTimestamp getSentTimestamp() const {return _sentTimestamp;}
	void setSentTimestamp(PtTimestamp t) {_sentTimestamp = t;}
	int getLatency() const {return _sentTimestamp-_timestamp;}
	
	Status getStatus() const {return _status;}
	int getMsgType() const { return (_status & 0xF0) >> 4;}
	int getChannel() const { return (_status & 0x0F) + 1;}
	Data getOriginalNote() const { return _data1;}
	Data getOutputNote() const {return _outputNote;}
	Data getValue() const { return _data2;}
	void setValue(Data value) {_data2 = value;}
	void changeNoteTo(Pad* pPad, bool bChangeModifiedState=true);
	void changeNoteTo(Data note, bool bChangeModifiedState=true);

	bool isIgnored() const {return _ignore!=NOT_IGNORED;}
	IgnoreReason getIgnoreReason() const {return _ignore;}
	void ignore(IgnoreReason reason) {_ignore = reason;}
	bool isAlreadyModified() const {return _alreadyModified;}

	std::string str() const;
	PmMessage computeOutputMessage() const { return Pm_Message(getStatus(), getOutputNote(), getValue()); }
	bool isInTimeWindow(const MidiMessage& otherMessage, int timeWindow) const;
	int getAbsTimeDiff(const MidiMessage& otherMessage) const;

public:
	float	hiHatSpeed;
	float	hiHatAcceleration;
	float	hiHatJerk;
	int		padType;

private:

	PtTimestamp			_timestamp;			///< timestamp from driver
	PtTimestamp			_sentTimestamp;		///< sent time
	Status				_status;			///< Note on or off
	Data				_data1;				///< aka original note
	Data				_data2;				///< aka note value

	Data				_outputNote;	///< By default the output note is the original note.
	IgnoreReason		_ignore;
	bool				_alreadyModified;
};
