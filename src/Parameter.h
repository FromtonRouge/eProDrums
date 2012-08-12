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

#include <QtGui/QColor>

#include <boost/variant.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/signals2.hpp>

#include <vector>
#include <map>
#include <string>

/**
 * Parameter used in pad settings.
 */
struct Parameter : public boost::enable_shared_from_this<Parameter>
{
	typedef boost::shared_ptr<Parameter> Ptr;
	typedef std::vector<Ptr> List;
	typedef boost::variant<bool, int, float, std::string> Value;
	typedef boost::signals2::signal<void (const Value&)> OnValueChanged;

	Parameter(	const std::string& szLabel = std::string(),
		   		const QColor& color = QColor(150, 150, 250),
				bool bEnabled = true,
				const OnValueChanged::slot_function_type& slot = OnValueChanged::slot_function_type()):
		label(szLabel),
		_bEnabled(bEnabled),
		_value(_bEnabled),
		_color(color)
   	{
		connect(slot);
	}

	Parameter(	const std::string& szLabel,
		   		int minimum,
		   		int maximum,
		   		int value,
				const OnValueChanged::slot_function_type& slot = OnValueChanged::slot_function_type()):
		label(szLabel),
		_bEnabled(true),
		minimum(minimum),
		maximum(maximum),
		_value(value)
   	{
		connect(slot);
   	}

	Parameter(	const std::string& szLabel,
		   		float minimum,
		   		float maximum,
		   		float value,
				const OnValueChanged::slot_function_type& slot = OnValueChanged::slot_function_type()):
		label(szLabel),
		_bEnabled(true),
		minimum(minimum),
		maximum(maximum),
		_value(value)
   	{
		connect(slot);
   	}

	Parameter(	const std::string& szLabel,
		   		bool value,
				const OnValueChanged::slot_function_type& slot = OnValueChanged::slot_function_type()):
		label(szLabel),
		_bEnabled(true),
		_value(value)
   	{
		connect(slot);
   	}

	Parameter(	const std::string& szLabel,
		   		const std::string& value,
				const OnValueChanged::slot_function_type& slot = OnValueChanged::slot_function_type()):
		label(szLabel),
		_bEnabled(true),
		_value(value)
   	{
		connect(slot);
	}

	size_t getIndex() const
	{
		size_t result = 0;
		if (_pParent.get())
		{
			for (size_t i=0;i<_pParent->_children.size();++i)
			{
				if (_pParent->_children[i].get()==this)
				{
					result = i;
					break;
				}
			}
		}
		return result;
	}

	void addChild(const Ptr& pParameter)
	{
		pParameter->_pParent = shared_from_this();
		pParameter->setEnabled(isEnabled());
		_children.push_back(pParameter);
	}

	void connect(const OnValueChanged::slot_function_type& slot)
   	{
		_onValueChanged.disconnect_all_slots();
		if (!slot.empty())
		{
			_onValueChanged.connect(slot);
		}
	}

	const Parameter::List& getChildren() const {return _children;}
	bool hasParent() const {return _pParent.get()!=NULL;}
	bool hasChildren() const {return !_children.empty();}
	size_t getChildrenCount() const {return _children.size();}
	const Ptr& getChildAt(size_t i) const {return _children[i];}
	const Ptr& getParent() const {return _pParent;}
	const QColor& getColor() const {return _color;}
	void setColor(const QColor& color) {_color=color;}
	const Value& getValue() const {return _value;}
	void setValue(const Value& value) {_value = value; if (!_onValueChanged.empty())_onValueChanged(_value);}
	bool isConnected() const {return !_onValueChanged.empty();}
	void update(const Value& value, const OnValueChanged::slot_function_type& slot) {connect(slot); setValue(value);}
	void setEnabled(bool state) {_bEnabled = state;}
	bool isEnabled() const {return _bEnabled;}
	void setDescription(const std::string& sz)  {_description = sz;}
	const std::string& getDescription() const {return _description;}

public:
	std::string label;
	Value minimum;
	Value maximum;

private:
	std::string					_description;

	bool						_bEnabled;
	Parameter::Ptr				_pParent;
	Parameter::List				_children;
	QColor						_color;
	Value						_value;
	OnValueChanged				_onValueChanged;
};

