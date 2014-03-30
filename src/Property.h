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

#include "AnyProperty.h"

#include <QtGui/QPolygonF>
#include <QtCore/QString>

#include <boost/archive/xml_oarchive.hpp>
#include <boost/archive/xml_iarchive.hpp>
#include <boost/serialization/variant.hpp>
#include <boost/serialization/version.hpp>
#include <boost/serialization/split_member.hpp>
#include <boost/variant.hpp>

/**
 * Thread safe property.
 */
template <typename T>
class Property : public AnyProperty
{
public:
	//typedef boost::variant<bool, int, float, QString, QPolygonF> Value;
	typedef boost::shared_ptr< Property<T> > Ptr;

public:
	Property(const T& value = T()): AnyProperty(value) {}

	void set(const T& value)
   	{
		Mutex::scoped_lock lock(_mutex);
		_any = value;
	}

	T get() const
	{
		Mutex::scoped_lock lock(_mutex);
		return boost::any_cast<T>(_any);
	}

private:
	friend class boost::serialization::access;

    template<class Archive> void save(Archive & ar, const unsigned int) const
    {
		T value = get();
		ar << boost::serialization::make_nvp("value", value);
    }

    template<class Archive> void load(Archive & ar, const unsigned int)
    {
		T value;
		ar >> boost::serialization::make_nvp("value", value);
		set(value);
	}

    BOOST_SERIALIZATION_SPLIT_MEMBER()
};

namespace boost
{ 
	namespace serialization
	{
		template <typename T>
		struct version<Property<T> >
		{
			BOOST_STATIC_CONSTANT(unsigned int, value = 0);
		};
	}
}
