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
#include "ParamItemModel.h"

Slot::Slot()
	: cymbalSimHitWindow(new Property<int>(35))
	, isChameleonCrashWithCrash(new Property<bool>(true))
	, isChameleonCrashWithRide(new Property<bool>(true))
	, isChameleonCrashWithSnare(new Property<bool>(false))
	, isChameleonCrashWithTom2(new Property<bool>(true))
	, isChameleonCrashWithTom3(new Property<bool>(true))
{
}

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

		cymbalSimHitWindow = rOther.cymbalSimHitWindow;
		isChameleonCrashWithCrash = rOther.isChameleonCrashWithCrash;
		isChameleonCrashWithRide = rOther.isChameleonCrashWithRide;
		isChameleonCrashWithSnare = rOther.isChameleonCrashWithSnare;
		isChameleonCrashWithTom2 = rOther.isChameleonCrashWithTom2;
		isChameleonCrashWithTom3 = rOther.isChameleonCrashWithTom3;
	}
	return *this;
}

void Slot::onDrumKitLoaded(DrumKitMidiMap* pDrumKit, const boost::filesystem::path&)
{
	Mutex::scoped_lock lock(_mutex);
	const DrumKitMidiMap::Description& desc = pDrumKit->getDescription();
	const DrumKitMidiMap::Description::Pads& pads = desc.pads;

	Pad::List::iterator itPad = _pads.begin();
	while (itPad!=_pads.end())
	{
		const Pad::Ptr& pPad = *(itPad++);
		DrumKitMidiMap::Description::Pads::const_iterator it = std::find_if(pads.begin(), pads.end(), boost::bind(&Pad::MidiDescription::type, _1)==pPad->type->get());
		if (it!=pads.end())
		{
			pPad->color->set(it->color);
			pPad->setInputNotes(it->inputNotes);
			pPad->defaultOutputNote->set(it->outputNote);
		}
	}
}

const QString& Slot::getName() const
{
	Mutex::scoped_lock lock(_mutex);
	return _szSlotName;
}

void Slot::setName(const QString& szName)
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
