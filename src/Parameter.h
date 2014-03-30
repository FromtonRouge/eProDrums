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

#include "LinearFunction.h"
#include "AnyProperty.h"

#include <QtGui/QColor>
#include <QtCore/QMetaType>

#include <utility>
#include <vector>
#include <map>

/**
 * Parameter used in pad settings.
 */
struct Parameter
{
	typedef std::pair<bool, bool> InfiniteExtremities;
	typedef std::map<int, QString> DictEnums;

	Parameter(	const QString& szName = QString(),
				const QString& szDescription = QString(),
				const AnyProperty::Ptr& pProperty = AnyProperty::Ptr(),
				AnyProperty::Value minimum = AnyProperty::Value(),
				AnyProperty::Value maximum = AnyProperty::Value(),
				const DictEnums& dictEnums = DictEnums());

	Parameter(	const QString& szName,
				const QString& szDescription,
				const AnyProperty::Ptr& pProperty,
				AnyProperty::Value minimum,
				AnyProperty::Value maximum,
				bool bInfiniteMin,
				bool bInfiniteMax);

	Parameter(	const QString& szName,
				const QString& szDescription,
				const AnyProperty::Ptr& pProperty,
				const LinearFunction::Description::Ptr&	pFunctionDescription);

	const QString&	getName() const {return _szName;}
	void			setName(const QString& szName) {_szName = szName;}
	void			setDescription(const QString& sz)  {_szDescription = sz;}
	const QString&	getDescription() const {return _szDescription;}
	const QColor&	getColor() const {return _color;}
	void			setColor(const QColor& color) {_color=color;}

	const AnyProperty::Value&	getValue() const {return _value;}
	void						setValue(const AnyProperty::Value& value);
	const AnyProperty::Value&	getMinimum() const {return _minimum;}
	const AnyProperty::Value&	getMaximum() const {return _maximum;}

	void			setEnabled(bool state) {_bEnabled = state;}
	bool			isEnabled() const {return _bEnabled;}

	void				setEnums(const DictEnums& dictEnums) {_dictEnums = dictEnums;}
	const DictEnums&	getEnums() const {return _dictEnums;}
	bool				hasEnums() const {return !_dictEnums.empty();}

	const LinearFunction::Description::Ptr& getFunctionDescription() const {return _pFunctionDescription;}

	/**
	 * Only for int and float.
	 */
	void setInfiniteExtremities(const InfiniteExtremities& extremities) {_infiniteExtremities = extremities;}
	const InfiniteExtremities& getInfiniteExtremities() const {return _infiniteExtremities;}

private:
	QString								_szName;
	QString								_szDescription;
	bool								_bEnabled;
	QColor								_color;
	AnyProperty::Ptr					_pProperty;
	AnyProperty::Value					_value;
	AnyProperty::Value					_minimum;
	AnyProperty::Value					_maximum;
	DictEnums							_dictEnums;
	InfiniteExtremities					_infiniteExtremities;

	LinearFunction::Description::Ptr	_pFunctionDescription;		// Only used on functions
};

Q_DECLARE_METATYPE(Parameter)
