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

#include "Parameter.h"
#include "LinearFunction.h"
#include <vector>

#include <QtCore/QAbstractTableModel>

class FunctionItemModel : public QAbstractTableModel
{
	Q_OBJECT

public:
	FunctionItemModel(const LinearFunction::Description::Ptr& pDescription, const LinearFunction::List& functions, QObject* pParent=NULL):
		QAbstractTableModel(pParent),
		_pDescription(pDescription),
		_functions(functions)
	{
	}

	virtual ~FunctionItemModel() {}

	const LinearFunction::List& getFunctions() const {return _functions;}

	virtual QVariant headerData( int section, Qt::Orientation orientation, int role = Qt::DisplayRole ) const
	{
		if (role!=Qt::DisplayRole)
		{
			return QVariant();
		}

		if (orientation==Qt::Horizontal)
		{
			switch (section)
			{
			case 0:
				return QString("Name");
			case 1:
				return QString("x1");
			case 2:
				return QString("x2");
			case 3:
				return QString("y1");
			case 4:
				return QString("y2");
			case 5:
				return QString("a");
			case 6:
				return QString("b");
			}
		}
		return QVariant();
	}

	virtual bool setData(const QModelIndex& idx, const QVariant& value, int role = Qt::EditRole)
	{
		// Data given by the editor
		if (role==Qt::EditRole)
		{
			size_t prevRow = idx.row()-1;
			size_t nextRow = idx.row()+1;
			switch (idx.column())
			{
			case 1:
				{
					if (prevRow<_functions.size())
					{
						_functions[prevRow].setX2(value.toFloat());
					}
					_functions[idx.row()].setX1(value.toFloat());
					break;
				}
			case 2:
				{
					_functions[idx.row()].setX2(value.toFloat());
					if (nextRow<_functions.size())
					{
						_functions[nextRow].setX1(value.toFloat());
					}
					break;
				}
			case 3:
				{
					_functions[idx.row()].setY1(value.toFloat());
					break;
				}
			case 4:
				{
					_functions[idx.row()].setY2(value.toFloat());
					break;
				}
			case 5:
				{
					_functions[idx.row()].setA(value.toFloat());
					break;
				}
			case 6:
				{
					_functions[idx.row()].setB(value.toFloat());
					break;
				}
			}
			emit dataChanged(index(0, 1), index(rowCount(), 6));
			return true;
		}
		return false;
	}

	/**
	 * \return Child count of parent
	 */
	virtual int rowCount(const QModelIndex& parent=QModelIndex()) const
	{
		Q_UNUSED(parent);
		return _functions.size();
	}

	virtual Qt::ItemFlags flags( const QModelIndex& index ) const
	{
		switch(index.column())
		{
		case 0:
			return Qt::ItemIsEnabled|Qt::ItemIsSelectable;
		case 1:
			return Qt::ItemIsEnabled|Qt::ItemIsEditable;
		case 2:
			return Qt::ItemIsEnabled|Qt::ItemIsEditable;
		case 3:
			return Qt::ItemIsEnabled|Qt::ItemIsEditable;
		case 4:
			return Qt::ItemIsEnabled|Qt::ItemIsEditable;
		case 5:
			return Qt::ItemIsEnabled|Qt::ItemIsEditable;
		case 6:
			return Qt::ItemIsEnabled|Qt::ItemIsEditable;
		}
		return Qt::ItemIsEnabled;
	}

	virtual int columnCount(const QModelIndex&) const
	{
		return 7;
	}

	virtual QVariant data(const QModelIndex& index, int role=Qt::DisplayRole) const
	{
		if (!index.isValid())
		{
			return QVariant();
		}

		const LinearFunction& f = _functions[index.row()];

		switch (role)
		{
		case Qt::ToolTipRole:
			{
				return QVariant();
			}
		case Qt::DisplayRole:
			{
				QVariant variant;
				switch (index.column())
				{
				case 0:
					{

						variant = tr("Function ") + QString::number(index.row());
						break;
					}
				}
				return variant;
			}
		case Qt::EditRole:
			{
				QVariant variant;
				switch (index.column())
				{
				case 0:
					{
						variant = tr("Function ") + QString::number(index.row());
						break;
					}
				case 1:
					{
						variant = f.getX1();
						break;
					}
				case 2:
					{
						variant = f.getX2();
						break;
					}
				case 3:
					{
						variant = f.getY1();
						break;
					}
				case 4:
					{
						variant = f.getY2();
						break;
					}
				case 5:
					{
						variant = f.getA();
						break;
					}
				case 6:
					{
						variant = f.getB();
						break;
					}
				}
				return variant;
			}
		default:
			{
				break;
			}
		}
		return QVariant();
	}

	int addFunction()
	{
		beginInsertRows(QModelIndex(), _functions.size(), _functions.size());
		LinearFunction newFunc;
		if (!_functions.empty())
		{
			const LinearFunction& prevFunc = _functions.back();
			float xDiff = _pDescription->x2Default-_pDescription->x1Default;
			newFunc.setPoints(prevFunc.getX2(), prevFunc.getY2(), prevFunc.getX2() + xDiff, prevFunc.getY2());
		}
		else
		{
			newFunc.setPoints(_pDescription->x1Default, _pDescription->y1Default, _pDescription->x2Default, _pDescription->y2Default);
		}
		_functions.push_back(newFunc);
		endInsertRows();

		emit dataChanged(index(rowCount()-1, 1), index(rowCount()-1, 6));
		return rowCount()-1;
	}

	virtual bool removeRows(int row, int count, const QModelIndex & parent = QModelIndex())
	{
		Q_UNUSED(parent);
		Q_UNUSED(count);
		beginRemoveRows(parent, row, row);
		_functions.erase(_functions.begin()+row);
		endRemoveRows();

		emit dataChanged(index(0, 1), index(rowCount()-1, 6));
		return true;
	}

private:

	LinearFunction::Description::Ptr	_pDescription;
	LinearFunction::List				_functions;
};
