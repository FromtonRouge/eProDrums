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

#include <boost/iostreams/concepts.hpp> 
#include <boost/signals2.hpp>
#include <boost/shared_ptr.hpp>

#include <string>

/**
 * iostream sink.
 */
class StreamSink : public boost::iostreams::sink
{
public:
	typedef boost::signals2::signal<void (const std::string&)> SignalWrite;

public:
	StreamSink(const SignalWrite::slot_function_type& slot):_pSignalWrite(new SignalWrite) {_pSignalWrite->connect(slot);}

	/**
	 * Write the incoming string.
	 * We just send the signal.
	 */
	std::streamsize write(const char* s, std::streamsize n)
	{
		// The signal is thread safe
		_pSignalWrite->operator()(std::string(s, static_cast<unsigned int>(n)));
		return n;
	}

private:
	boost::shared_ptr<SignalWrite> _pSignalWrite; // signals are noncopyable, that's why we use a shared_ptr here.
};

