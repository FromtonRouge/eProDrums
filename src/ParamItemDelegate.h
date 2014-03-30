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

#include <QtWidgets/QStyledItemDelegate>

class ParamItemDelegate : public QStyledItemDelegate
{
	Q_OBJECT

public:
	ParamItemDelegate(QObject* pParent=NULL);
	~ParamItemDelegate();

	QSize sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const;
	virtual void initStyleOption(QStyleOptionViewItem* option, const QModelIndex & index ) const;
	virtual void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const;
	virtual QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem &, const QModelIndex&) const;
	virtual void setEditorData(QWidget* p, const QModelIndex& index) const;
	virtual void setModelData(QWidget* p1, QAbstractItemModel* p2, const QModelIndex& index) const;
};
