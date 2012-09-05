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

#include "UserSettings.h"

UserSettings::CurveVisibility UserSettings::CURVE_HIHAT_CONTROL			= 1<<0;
UserSettings::CurveVisibility UserSettings::CURVE_HIHAT_ACCELERATION	= 1<<1;
UserSettings::CurveVisibility UserSettings::CURVE_HIHAT					= 1<<2;
UserSettings::CurveVisibility UserSettings::CURVE_HIHAT_PEDAL			= 1<<3;
UserSettings::CurveVisibility UserSettings::CURVE_CRASH					= 1<<4;
UserSettings::CurveVisibility UserSettings::CURVE_YELLOW_CRASH			= 1<<5;
UserSettings::CurveVisibility UserSettings::CURVE_RIDE					= 1<<6;
UserSettings::CurveVisibility UserSettings::CURVE_TOM1					= 1<<7;
UserSettings::CurveVisibility UserSettings::CURVE_TOM2					= 1<<8;
UserSettings::CurveVisibility UserSettings::CURVE_TOM3					= 1<<9;
UserSettings::CurveVisibility UserSettings::CURVE_SNARE					= 1<<10;
UserSettings::CurveVisibility UserSettings::CURVE_BASS_PEDAL			= 1<<11;

UserSettings::LogActivation UserSettings::LOG_ACTIVATED				= 1<<0;
UserSettings::LogActivation UserSettings::LOG_RAW_DATA				= 1<<1;
UserSettings::LogActivation UserSettings::LOG_FILTERED_DATA			= 1<<2;
UserSettings::LogActivation UserSettings::LOG_HIHAT_CONTROL			= 1<<3;
UserSettings::LogActivation UserSettings::LOG_OTHERS				= 1<<4;
