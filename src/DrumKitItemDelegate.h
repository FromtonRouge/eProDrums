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
#include "DrumKitItemModel.h"
#include "DrumKitItemEditor.h"
#include <QtWidgets/QStyledItemDelegate>
#include <QtGui/QPainter>
#include <QtWidgets/QSpinBox>

#ifndef Q_DECLARE_METATYPE_FOR_PAD_DESCRIPTION_DONE
#define Q_DECLARE_METATYPE_FOR_PAD_DESCRIPTION_DONE
Q_DECLARE_METATYPE(Pad::MidiDescription)
#endif

class MidiMessage;

/**
 * Drum item delegate for QListView.
 */
class DrumKitItemDelegate : public QStyledItemDelegate
{
	Q_OBJECT

signals:
	void onMidiIn(const MidiMessage&);

public:
	DrumKitItemDelegate(QObject* pParent=NULL):QStyledItemDelegate(pParent) { }
	virtual ~DrumKitItemDelegate() {}

	QSize sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const
	{
		QSize result = QStyledItemDelegate::sizeHint(option, index);
		if (index.column()==0)
		{
			int width = option.fontMetrics.width(Pad::getName(index.data().value<Pad::MidiDescription>().type));
			result.rwidth() += width + INDENT;
		}
		return result;
	}

	virtual void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
	{
		switch (index.column())
		{
		case 0:
			{
				const Pad::MidiDescription& description = index.data().value<Pad::MidiDescription>();

				painter->save();
				QRectF rect = option.rect;
				QColor color(description.color);
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

				painter->drawText(rect, Qt::AlignLeft, Pad::getName(description.type));
				painter->restore();
				break;
			}

		case 1:
			{
				painter->save();
				const Pad::MidiDescription& description = index.data().value<Pad::MidiDescription>();
				painter->fillRect(option.rect, QColor(description.color));
				painter->restore();
				break;
			}

		default:
			{
				QStyledItemDelegate::paint(painter, option, index);
				break;
			}
		}
	}

	virtual QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const
	{
		switch (index.column())
		{
		case 1:
		case 2:
			{
				DrumKitItemEditor* pEditor = new DrumKitItemEditor(parent, index);
				connect(pEditor, SIGNAL(editFinished(QWidget*)), this, SIGNAL(commitData(QWidget*)));
				return pEditor;
			}
		case 3:
			{
				PadNotesWidget* pEditor = new PadNotesWidget(parent);
				connect(this, SIGNAL(onMidiIn(const MidiMessage&)), pEditor, SIGNAL(onMidiIn(const MidiMessage&)));
				connect(pEditor, SIGNAL(editFinished(QWidget*)), this, SIGNAL(commitData(QWidget*)));
				return pEditor;
			}
		default:
			{
				break;
			}
		}

		return QStyledItemDelegate::createEditor(parent, option, index);
	}

	virtual void setEditorData(QWidget* pEditor, const QModelIndex& index) const
	{
		switch (index.column())
		{
		case 1:
		case 2:
			{
				DrumKitItemEditor* p = static_cast<DrumKitItemEditor*>(pEditor);
				p->setPadDescription(index.data(Qt::EditRole).value<Pad::MidiDescription>());
				break;
			}

		case 3:
			{
				PadNotesWidget* p = static_cast<PadNotesWidget*>(pEditor);
				p->setPadDescription(index.data(Qt::EditRole).value<Pad::MidiDescription>());
				break;
			}
		default:
			{
				QStyledItemDelegate::setEditorData(pEditor, index);
				break;
			}
		}
	}

	virtual void setModelData(QWidget* p1, QAbstractItemModel* p2, const QModelIndex& index ) const
	{
		DrumKitItemModel* pModel = dynamic_cast<DrumKitItemModel*>(p2);
		switch (index.column())
		{
		case 1:
		case 2:
			{
				DrumKitItemEditor* pEditor = static_cast<DrumKitItemEditor*>(p1);
				QVariant variant;
				variant.setValue(pEditor->getPadDescription());
				pModel->setData(index, variant);
				break;
			}
		case 3:
			{
				PadNotesWidget* pEditor = static_cast<PadNotesWidget*>(p1);
				QVariant variant;
				variant.setValue(pEditor->getPadDescription());
				pModel->setData(index, variant);
				break;
			}
		default:
			{
				QStyledItemDelegate::setModelData(p1, p2, index);
				break;
			}
		}
	}

private:
	static const int INDENT = 8;
};
