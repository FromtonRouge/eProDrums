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

#include <QtCore/QAbstractItemModel>

#ifndef Q_DECLARE_METATYPE_FOR_PARAMETER_VALUE
#define Q_DECLARE_METATYPE_FOR_PARAMETER_VALUE
Q_DECLARE_METATYPE(Parameter*)
#endif

class ParamItemModel : public QAbstractItemModel
{
	Q_OBJECT

public:
	ParamItemModel(const Parameter::Ptr& pRoot, QObject* pParent=NULL):
	   	QAbstractItemModel(pParent),
		_pRoot(pRoot)
	{
	}

	virtual ~ParamItemModel() {}

	/**
	 * \return the index of the item identified by row within the parent
	 */
	virtual QModelIndex index( int row, int column, const QModelIndex & parent = QModelIndex()) const
	{
		QModelIndex result;
		if (!_pRoot.get())
		{
			return result;
		}

		if (!parent.isValid())
		{
			if (_pRoot->hasChildren())
			{
				result = createIndex(row, column, (void*)_pRoot->getChildAt(row).get());
			}
		}
		else
		{
			const Parameter* pParent = static_cast<Parameter*>(parent.internalPointer());
			result = createIndex(row, column, (void*)pParent->getChildAt(row).get());
		}
		return result;
	}

	/**
	 * \return the parent of item identified by index
	 */
	virtual QModelIndex parent(const QModelIndex& index) const
	{
		QModelIndex result;
		if (!index.isValid())
		{
			return result;
		}
		else
		{
			const Parameter* pCurrent = static_cast<Parameter*>(index.internalPointer());
			const Parameter::Ptr& pParent = pCurrent->getParent();
			if (_pRoot!=pParent && pParent.get())
			{
				result = createIndex(pParent->getIndex(), 0, (void*)pParent.get());
			}
		}
		return result;
	}

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
				return QString("Value");
			}
		}
		return QVariant();
	}

	virtual bool setData( const QModelIndex& idx, const QVariant&, int role = Qt::EditRole )
	{
		// Data given by the editor
		if (role==Qt::EditRole)
		{
			// If idx is a group parameter we update children activation states
			if (hasChildren(idx))
			{
				Parameter* pGroup = static_cast<Parameter*>(idx.internalPointer());
				bool bActivated = boost::get<bool>(pGroup->getValue());
				int count = rowCount(idx);
				for (int i=0; i<count; ++i)
				{
					const QModelIndex& labelIdx = index(i, 0, idx);
					const QModelIndex& editorIdx = index(i, 1, idx);
					Parameter* pChild = static_cast<Parameter*>(editorIdx.internalPointer());
					pChild->setEnabled(bActivated);
					emit dataChanged(labelIdx, editorIdx);
				}
			}
			return true;
		}
		return false;
	}

	/**
	 * \return Child count of parent
	 */
	virtual int rowCount(const QModelIndex& parent=QModelIndex()) const
	{
		int result = 0;
		if (!_pRoot.get())
		{
			return result;
		}

		if (!parent.isValid())
		{
			// Root item
			result = _pRoot->getChildrenCount();
		}
		else
		{
			const Parameter* pParent = static_cast<Parameter*>(parent.internalPointer());
			result = pParent->getChildrenCount();
		}
		return result;
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

	virtual int columnCount(const QModelIndex&) const
	{
		return 2;
	}

	virtual QVariant data(const QModelIndex& index, int role=Qt::DisplayRole) const
	{
		if (!index.isValid())
		{
			return QVariant();
		}

		Parameter* p = static_cast<Parameter*>(index.internalPointer());
		switch (role)
		{
		case Qt::DisplayRole:
			{
				QVariant variant;
				variant.setValue(p);
				return variant;
			}
		case Qt::EditRole:
			{
				if (index.column()==1)
				{
					QVariant variant;
					variant.setValue(p);
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

	/**
	 * Update the view just by emitting signals for each cell.
	 */
	void update()
	{
		// Emit signals for each cell on the editor column only
		for (int i=0; i<rowCount(); ++i)
		{
			const QModelIndex& parentIndex = index(i, 1);
			emit dataChanged(parentIndex, parentIndex);
			for (int j=0; j<rowCount(parentIndex); ++j)
			{
				const QModelIndex& idx = index(j, 1, parentIndex);
				emit dataChanged(idx, idx);
			}
		}
	}

private:
	Parameter::Ptr	_pRoot;
};
