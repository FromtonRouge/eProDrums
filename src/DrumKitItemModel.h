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

#include "DrumKitMidiMap.h"
#include <QtCore/QAbstractTableModel>

#ifndef Q_DECLARE_METATYPE_FOR_PAD_DESCRIPTION_DONE
#define Q_DECLARE_METATYPE_FOR_PAD_DESCRIPTION_DONE
Q_DECLARE_METATYPE(Pad::MidiDescription)
#endif

/**
 * Drum item model for QListView.
 */
class DrumKitItemModel : public QAbstractTableModel
{
	Q_OBJECT

public:
	DrumKitItemModel(QObject* pParent=NULL):
		QAbstractTableModel(pParent),
		_pDrumKit(NULL)
   	{
   	}

	virtual ~DrumKitItemModel() {}

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
				return QString("Pad name");
			case 1:
				return QString("Midi notes");
			}
		}
		return QVariant();
	}

	virtual bool setData( const QModelIndex& index, const QVariant& value, int role = Qt::EditRole )
	{
		if (!_pDrumKit)
		{
			return false;
		}

		if (role==Qt::EditRole)
		{
			DrumKitMidiMap::Description& rDescription = _pDrumKit->getDescription();
			if (!value.isNull() && qVariantCanConvert<Pad::MidiDescription>(value))
			{
				const Pad::MidiDescription& description = value.value<Pad::MidiDescription>();
				rDescription[index.row()] = description;
				emit dataChanged(index, this->index(index.row(), 1));
				return true;
			}
			else
			{
				return false;
			}
		}
		return false;
	}

	virtual int rowCount(const QModelIndex& parent=QModelIndex()) const
	{
		if (!_pDrumKit)
		{
			return 0;
		}
		return _pDrumKit->getDescription().size();
	}

	virtual Qt::ItemFlags flags( const QModelIndex& index ) const
	{
		switch(index.column())
		{
		case 0:
			return Qt::ItemIsEnabled|Qt::ItemIsSelectable;
		case 1:
			return Qt::ItemIsEnabled|Qt::ItemIsSelectable|Qt::ItemIsEditable;
		}
		return Qt::ItemIsEnabled|Qt::ItemIsSelectable;
	}

	virtual int columnCount(const QModelIndex& parent=QModelIndex()) const
	{
	   	return 2;
	}

	virtual QVariant data(const QModelIndex& index, int role=Qt::DisplayRole) const
	{
		if (!_pDrumKit)
		{
			return QVariant();
		}

		const DrumKitMidiMap::Description& description = _pDrumKit->getDescription();
		switch (role)
		{
		case Qt::DisplayRole:
			{
				if (index.column()==0)
				{
					return QString(description[index.row()].getTypeLabel().c_str());
				}
				break;
			}
		case Qt::EditRole:
			{
				if (index.column()==1)
				{
					QVariant variant;
					variant.setValue(description[index.row()]);
					return variant;
				}
				break;
			}
		default:
			{
				break;
			}
		}
		return QVariant();
	}

	void setDrumKit(DrumKitMidiMap* pDrumKit)
	{
		if (rowCount())
		{
			beginRemoveRows(QModelIndex(), 0, rowCount()-1);
			endRemoveRows();
		}

		_pDrumKit = pDrumKit;

		const DrumKitMidiMap::Description& description = _pDrumKit->getDescription();
		if (!description.empty())
		{
			beginInsertRows(QModelIndex(), 0, description.size()-1);
			for (size_t i=0; i<description.size(); ++i)
			{
				// Index for label
				createIndex(i, 0);

				// Index for the widget
				const QModelIndex& index = createIndex(i, 1);

				QVariant variant;
				variant.setValue(description[i]);
				setData(index, variant);
			}
			endInsertRows();
		}
	}

private:
	DrumKitMidiMap* _pDrumKit;
};
