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
		QAbstractTableModel(pParent)
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
				return QString("Color");
			case 2:
				return QString("Output note");
			case 3:
				return QString("Input notes");
			}
		}
		return QVariant();
	}

	virtual bool setData( const QModelIndex& idx, const QVariant& value, int role = Qt::EditRole )
	{
		bool bResult = false;
		if (role==Qt::EditRole)
		{
			DrumKitMidiMap::Description& rDescription = _drumKit.getDescription();
			const Pad::MidiDescription& description = value.value<Pad::MidiDescription>();
			switch (idx.column())
			{
			case 1:
				{
					rDescription.pads[idx.row()].color = description.color;
					bResult = true;
					break;
				}
			case 2:
				{
					rDescription.pads[idx.row()].outputNote = description.outputNote;
					bResult = true;
					break;
				}
			case 3:
				{
					rDescription.pads[idx.row()].inputNotes = description.inputNotes;
					bResult = true;
					break;
				}
			}
		}

		if (bResult)
		{
			emit dataChanged(index(idx.row(), 0), index(idx.row(), columnCount()-1));
		}

		return bResult;
	}

	virtual int rowCount(const QModelIndex& = QModelIndex()) const
	{
		return _drumKit.getDescription().pads.size();
	}

	virtual Qt::ItemFlags flags( const QModelIndex& index ) const
	{
		switch(index.column())
		{
		case 0:
			return Qt::ItemIsEnabled|Qt::ItemIsSelectable;
		case 1:
		case 2:
		case 3:
			return Qt::ItemIsEnabled|Qt::ItemIsSelectable|Qt::ItemIsEditable;
		}
		return Qt::ItemIsEnabled|Qt::ItemIsSelectable;
	}

	virtual int columnCount(const QModelIndex& parent= QModelIndex()) const
	{
		Q_UNUSED(parent);
	   	return 4;
	}

	virtual QVariant data(const QModelIndex& index, int role=Qt::DisplayRole) const
	{
		const DrumKitMidiMap::Description& description = _drumKit.getDescription();
		switch (role)
		{
		case Qt::DisplayRole:
			{
				QVariant variant;
				switch (index.column())
				{
				case 2:
					{
						variant = description.pads[index.row()].outputNote;
						break;
					}
				default:
					{
						variant.setValue(description.pads[index.row()]);
						break;
					}
				}
				return variant;
			}
		case Qt::EditRole:
			{
				QVariant variant;
				variant.setValue(description.pads[index.row()]);
				return variant;
			}
		default:
			{
				break;
			}
		}
		return QVariant();
	}

	const DrumKitMidiMap& getDrumKit() const {return _drumKit;}

	void setDrumKit(const DrumKitMidiMap& drumKit)
	{
		if (rowCount())
		{
			beginRemoveRows(QModelIndex(), 0, rowCount()-1);
			endRemoveRows();
		}

		_drumKit = drumKit;

		const DrumKitMidiMap::Description& description = _drumKit.getDescription();
		if (!description.pads.empty())
		{
			beginInsertRows(QModelIndex(), 0, description.pads.size()-1);
			for (size_t i=0; i<description.pads.size(); ++i)
			{
				QVariant variant;
				variant.setValue(description.pads[i]);
				setData(createIndex(i, 1), variant);
				setData(createIndex(i, 2), variant);
				setData(createIndex(i, 3), variant);
			}
			endInsertRows();
		}
	}

private:
	DrumKitMidiMap _drumKit;
};
