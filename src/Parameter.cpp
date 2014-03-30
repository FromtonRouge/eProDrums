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

#include "Parameter.h"

Parameter::Parameter(	const QString& szName,
						const QString& szDescription,
						const AnyProperty::Ptr& pProperty,
						AnyProperty::Value minimum,
						AnyProperty::Value maximum,
						const DictEnums& dictEnums)
	: _szName(szName)
	, _szDescription(szDescription)
	, _bEnabled(true)
	, _pProperty(pProperty)
	, _minimum(minimum)
	, _maximum(maximum)
	, _dictEnums(dictEnums)
	, _infiniteExtremities(InfiniteExtremities(false, false))
{
	if (_pProperty)
	{
		_value = _pProperty->getValue();
	}
}

Parameter::Parameter(	const QString& szName,
						const QString& szDescription,
						const AnyProperty::Ptr& pProperty,
						AnyProperty::Value minimum,
						AnyProperty::Value maximum,
						bool bInfiniteMin,
						bool bInfiniteMax)
	: _szName(szName)
	, _szDescription(szDescription)
	, _bEnabled(true)
	, _pProperty(pProperty)
	, _minimum(minimum)
	, _maximum(maximum)
	, _infiniteExtremities(InfiniteExtremities(bInfiniteMin, bInfiniteMax))
{
	if (_pProperty)
	{
		_value = _pProperty->getValue();
	}
}

Parameter::Parameter(	const QString& szName,
						const QString& szDescription,
						const AnyProperty::Ptr& pProperty,
						const LinearFunction::Description::Ptr&	pFunctionDescription)
	: _szName(szName)
	, _szDescription(szDescription)
	, _bEnabled(true)
	, _pProperty(pProperty)
	, _infiniteExtremities(InfiniteExtremities(false, false))
	, _pFunctionDescription(pFunctionDescription)
{
	if (_pProperty)
	{
		_value = _pProperty->getValue();
	}
}

void Parameter::setValue(const AnyProperty::Value& value)
{
	_value = value;
	if (_pProperty)
	{
		_pProperty->setValue(_value);
	}
}
