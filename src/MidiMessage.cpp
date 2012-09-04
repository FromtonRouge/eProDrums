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

#include "MidiMessage.h"
#include <boost/format.hpp>

std::string MidiMessage::str() const
{
    boost::format fmtMsg("hex=0x%X type=%d chan=%d note=%d vel=%d timestamp=%d");
	return (fmtMsg%_dwParam1%getMsgType()%getChannel()%getOriginalNote()%getValue()%getTimestamp()).str();
}
void MidiMessage::print() const
{
    std::cout << str() << std::endl;
}

DWORD MidiMessage::computeOutputMessage() const
{
    DWORD result = 0;
    DWORD byte1 = 0;
    byte1 |= static_cast<char>(getMsgType()) << 4;    
    byte1 |= static_cast<char>(getChannel()-1);

    DWORD byte2 = 0;
    byte2 = static_cast<char>(getOutputNote()) << 8;

    DWORD byte3 = 0;
    byte3 = static_cast<char>(getValue()) << 16;
    
    result = byte1 | byte2 | byte3;
    return result;
}

int MidiMessage::getAbsTimeDiff(const MidiMessage& otherMessage) const
{
	return std::abs(otherMessage.getTimestamp() - getTimestamp());
}

bool MidiMessage::isInTimeWindow(const MidiMessage& otherMessage, int timeWindow) const
{
	return timeWindow!=0 && (getAbsTimeDiff(otherMessage) <= timeWindow);
}

void MidiMessage::changeOutputNote(int note, bool bChangeModifiedState)
{
	_outputNote = note;
	_alreadyModified = bChangeModifiedState;
}
