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

#include "DrumNote.h"
#include <QtCore/QAbstractTableModel>
#include <vector>

/**
 * Drum item model for QListView.
 */
class DrumNoteItemModel : public QAbstractTableModel
{
	Q_OBJECT

public:
	DrumNoteItemModel(QObject* pParent=NULL): QAbstractTableModel(pParent) { }
	virtual ~DrumNoteItemModel() {}

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
				return QString("Midi note");
			case 1:
				return QString("Hit zone");
			}
		}
		return QVariant();
	}

	virtual bool setData( const QModelIndex& idx, const QVariant& value, int role = Qt::EditRole )
	{
		if (role==Qt::EditRole)
		{
			const DrumNote& drumNote = _drumNotes._container[idx.row()];
			DrumNotes::Container::iterator it = _drumNotes._container.iterator_to(drumNote);
			switch (idx.column())
			{
			case 0:
				{
					_drumNotes._container.replace(it, DrumNote(value.toInt(), drumNote.hitZone));
					break;
				}
			case 1:
				{
					_drumNotes._container.replace(it, DrumNote(drumNote.midiNote, static_cast<DrumNote::HitZone>(value.toInt())));
					break;
				}
			}
			emit dataChanged(index(idx.row(), 0), index(idx.row(), 1));
			return true;
		}
		return false;
	}

	virtual int rowCount(const QModelIndex& = QModelIndex()) const
	{
		return _drumNotes._container.size();
	}

	virtual Qt::ItemFlags flags( const QModelIndex& index ) const
	{
		switch(index.column())
		{
		case 0:
		case 1:
			return Qt::ItemIsEnabled|Qt::ItemIsSelectable|Qt::ItemIsEditable;
		}
		return Qt::ItemIsEnabled|Qt::ItemIsSelectable;
	}

	virtual int columnCount(const QModelIndex&) const
	{
	   	return 2;
	}

	virtual QVariant data(const QModelIndex& index, int role=Qt::DisplayRole) const
	{
		switch (role)
		{
		case Qt::DisplayRole:
			{
				switch (index.column())
				{
				case 0:
					{
						QVariant variant;
						variant.setValue(_drumNotes._container[index.row()].midiNote);
						return variant;
					}
				case 1:
					{
						QVariant variant;
						variant.setValue(static_cast<int>(_drumNotes._container[index.row()].hitZone));
						return variant;
					}
				}
				break;
			}
		case Qt::EditRole:
			{
				switch (index.column())
				{
				case 0:
					{
						QVariant variant;
						variant.setValue(_drumNotes._container[index.row()].midiNote);
						return variant;
					}
				case 1:
					{
						QVariant variant;
						variant.setValue(static_cast<int>(_drumNotes._container[index.row()].hitZone));
						return variant;
					}
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

	int add(int midiNote = -1)
	{
		if (midiNote<0)
		{
			beginInsertRows(QModelIndex(), _drumNotes._container.size(), _drumNotes._container.size());
			midiNote = 0;
			if (!_drumNotes._container.empty())
			{
				midiNote = _drumNotes._container.back().midiNote;
			}

			while (_drumNotes.findMidiNote(midiNote)!=_drumNotes.endMidiNote())
			{
				midiNote++;
			}
			_drumNotes._container.push_back(DrumNote(midiNote));
			endInsertRows();
			emit dataChanged(index(rowCount()-1, 0), index(rowCount()-1, 1));
		}
		else
		{
			if (_drumNotes.findMidiNote(midiNote)==_drumNotes.endMidiNote())
			{
				beginInsertRows(QModelIndex(), _drumNotes._container.size(), _drumNotes._container.size());
				_drumNotes._container.push_back(DrumNote(midiNote));
				endInsertRows();
				emit dataChanged(index(rowCount()-1, 0), index(rowCount()-1, 1));
			}
		}
		return rowCount()-1;
	}

	virtual bool removeRows(int row, int count, const QModelIndex & parent = QModelIndex())
	{
		beginRemoveRows(parent, row, row+count-1);
		for (int i=row;i<row+count;++i)
		{
			_drumNotes.eraseMidiNote(_drumNotes._container[i].midiNote);
		}
		endRemoveRows();
		emit dataChanged(index(0, 0), index(rowCount()-1, 1));
		return true;
	}

	const DrumNotes& getDrumNotes() const {return _drumNotes;}

	void setDrumNotes(const DrumNotes& drumNotes)
	{
		if (rowCount())
		{
			removeRows(0, rowCount());
		}

		if (drumNotes._container.size())
		{
			beginInsertRows(QModelIndex(), 0, drumNotes._container.size()-1);
			_drumNotes = drumNotes;
			endInsertRows();
			emit dataChanged(index(0, 0), index(rowCount()-1, 1));
		}
	}

private:
	DrumNotes _drumNotes;
};

