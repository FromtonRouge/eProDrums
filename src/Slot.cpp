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

#include "Slot.h"
#include "HiHatPedalElement.h"
#include "DrumKitMidiMap.h"

Slot::MutableCrashSettings Slot::CRASH_CRASH	= 1<<0;
Slot::MutableCrashSettings Slot::CRASH_RIDE		= 1<<1;
Slot::MutableCrashSettings Slot::CRASH_SNARE	= 1<<2;
Slot::MutableCrashSettings Slot::CRASH_TOM2		= 1<<3;
Slot::MutableCrashSettings Slot::CRASH_TOM3		= 1<<4;

Slot::Slot(const Slot& rOther)
{
	Mutex::scoped_lock lock(_mutex);
	this->operator=(rOther);
}

Slot& Slot::operator=(const Slot& rOther)
{
	Mutex::scoped_lock lock(_mutex);
	if (this != &rOther)
	{
		_szSlotName = rOther._szSlotName;

		// Copying pads
		_pads.clear();
		Pad::List::const_iterator it = rOther._pads.begin();
		while (it!=rOther._pads.end())
		{
			const Pad::Ptr& pPad = *(it++);
			const HiHatPedalElement::Ptr& pHiHat = boost::dynamic_pointer_cast<HiHatPedalElement>(pPad);
			if (pHiHat.get())
			{
				_pads.push_back(HiHatPedalElement::Ptr(new HiHatPedalElement(*pHiHat)));
			}
			else
			{
				_pads.push_back(Pad::Ptr(new Pad(*pPad)));
			}
		}

		_cymbalSimHitWindow = rOther._cymbalSimHitWindow;
		_mutableCrashSettings = rOther._mutableCrashSettings;
	}
	return *this;
}

void Slot::onDrumKitLoaded(DrumKitMidiMap* pDrumKit, const boost::filesystem::path&)
{
	Mutex::scoped_lock lock(_mutex);
	const DrumKitMidiMap::Description& desc = pDrumKit->getDescription();

	Pad::List::iterator it = _pads.begin();
	while (it!=_pads.end())
	{
		const Pad::Ptr& pPad = *(it++);
		DrumKitMidiMap::Description::const_iterator it = std::find_if(desc.begin(), desc.end(), boost::bind(&Pad::MidiDescription::type, _1)==pPad->getType());
		if (it!=desc.end())
		{
			pPad->setMidiNotes(it->midiNotes);
		}
	}
}
const std::string& Slot::getName() const
{
	Mutex::scoped_lock lock(_mutex);
	return _szSlotName;
}

void Slot::setName(const std::string& szName)
{
	Mutex::scoped_lock lock(_mutex);
	_szSlotName = szName;
}

const Pad::List& Slot::getPads() const
{
	Mutex::scoped_lock lock(_mutex);
	return _pads;
}

Pad::List& Slot::getPads()
{
	Mutex::scoped_lock lock(_mutex);
	return _pads;
}

bool Slot::isMutableCrash(const MutableCrashSettings& bit) const
{
	Mutex::scoped_lock lock(_mutex);
	return (_mutableCrashSettings & bit) == bit; 
}

void Slot::setMutableCrash(const MutableCrashSettings& bit, const Parameter::Value& state)
{
	Mutex::scoped_lock lock(_mutex);
   	_mutableCrashSettings = boost::get<bool>(state)?_mutableCrashSettings|bit:(_mutableCrashSettings|bit)^bit;
}

int Slot::getCymbalSimHitWindow() const
{
	Mutex::scoped_lock lock(_mutex);
	return boost::get<int>(_cymbalSimHitWindow);
}

void Slot::setCymbalSimHitWindow(const Parameter::Value& simHit)
{
	Mutex::scoped_lock lock(_mutex);
	_cymbalSimHitWindow = simHit;
}
