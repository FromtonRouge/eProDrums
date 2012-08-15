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

#include "Pad.h"
#include "PadNotesWidget.h"
#include "DrumKitItemModel.h"
#include <QtGui/QStyledItemDelegate>
#include <QtGui/QPainter>

#ifndef Q_DECLARE_METATYPE_FOR_PAD_DESCRIPTION_DONE
#define Q_DECLARE_METATYPE_FOR_PAD_DESCRIPTION_DONE
Q_DECLARE_METATYPE(Pad::MidiDescription)
#endif

/**
 * Drum item delegate for QListView.
 */
class DrumKitItemDelegate : public QStyledItemDelegate
{
	Q_OBJECT
signals:
	void midiNoteOn(int, int);

public:
	DrumKitItemDelegate(QObject* pParent=NULL):QStyledItemDelegate(pParent) { }
	virtual ~DrumKitItemDelegate() {}

	QSize sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const
	{
		QSize result = QStyledItemDelegate::sizeHint(option, index);
		if (index.column()==0)
		{
			int width = option.fontMetrics.width(Pad::getName(index.data().value<Pad::MidiDescription>().type).c_str());
			result.rwidth() += width + INDENT;
		}
		return result;
	}

	virtual void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
	{
		if (index.column()==0)
		{
			const Pad::MidiDescription& description = index.data().value<Pad::MidiDescription>();

			painter->save();
			QRectF rect = option.rect;
			QColor color(Pad::getColor(description.type).c_str());
			const QColor& lighterColor = color.lighter(130);
			painter->fillRect(rect, lighterColor);

			// Centering vertically
			QSize sizeData = sizeHint(option, index);
			int offset((rect.height()-sizeData.height())/2);
			QPointF point = rect.bottomLeft();
			rect.moveBottom(point.y()+offset);	// Relative to up-left corner of the treeview

			// Horizontal indent
			rect.moveLeft(point.x()+INDENT);			// Relative to up-left corner of the treeview
			rect.setWidth(rect.width()-INDENT);

			painter->drawText(rect, Qt::AlignLeft, Pad::getName(description.type).c_str());
			painter->restore();
		}
		else
		{
			QStyledItemDelegate::paint(painter, option, index);
		}
	}

	virtual QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem &, const QModelIndex&) const
	{
		PadNotesWidget* pEditor = new PadNotesWidget(parent);
		connect(this, SIGNAL(midiNoteOn(int, int)), pEditor, SIGNAL(midiNoteOn(int, int)));
		connect(pEditor, SIGNAL(editFinished(QWidget*)), this, SIGNAL(commitData(QWidget*)));
		return pEditor;
	}

	virtual void updateEditorGeometry( QWidget * editor, const QStyleOptionViewItem & option, const QModelIndex & index ) const
	{
		QStyledItemDelegate::updateEditorGeometry(editor, option, index);
	}

	virtual void setEditorData(QWidget* pEditor, const QModelIndex& index) const
	{
		PadNotesWidget* p = dynamic_cast<PadNotesWidget*>(pEditor);
		const QVariant& variant = index.data(Qt::EditRole);
		if (!variant.isNull() && qVariantCanConvert<Pad::MidiDescription>(variant))
		{
			const Pad::MidiDescription& description = variant.value<Pad::MidiDescription>();
			p->setPadDescription(description);
		}
	}

	virtual void setModelData(QWidget* p1, QAbstractItemModel* p2, const QModelIndex& index ) const
	{
		PadNotesWidget* pEditor = dynamic_cast<PadNotesWidget*>(p1);
		const Pad::MidiDescription& padDescription = pEditor->getPadDescription();
		DrumKitItemModel* pModel = dynamic_cast<DrumKitItemModel*>(p2);
		QVariant variant;
		variant.setValue(padDescription);
		pModel->setData(index, variant);
	}

private:
	static const int INDENT = 8;
};
