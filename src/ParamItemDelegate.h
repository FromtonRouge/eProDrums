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
#include "ParamItemEditor.h"
#include "ParamItemModel.h"

#include <QtGui/QStyledItemDelegate>
#include <QtGui/QPainter>
#include <QtGui/QFontMetrics>

class ParamItemDelegate : public QStyledItemDelegate
{
	Q_OBJECT

public:
	ParamItemDelegate(QObject* pParent=NULL): QStyledItemDelegate(pParent) { }
	virtual ~ParamItemDelegate() {}

	QSize sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const
	{
		QSize result = QStyledItemDelegate::sizeHint(option, index);
		if (index.column()==0)
		{
			Parameter* pParameter = index.data().value<Parameter*>();
			int width = option.fontMetrics.width(pParameter->label.c_str());
			const int INDENT(option.fontMetrics.width("- "));
			const int UNKNOW_OFFSET(10); // Hack...
			result.rwidth() += width + INDENT + UNKNOW_OFFSET;
		}
		return result;
	}

	virtual void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
	{
		if (index.parent().isValid() && index.column()==1)
		{
			return;
		}
		painter->save();

		painter->setRenderHint(QPainter::Antialiasing);
		QStyleOptionViewItemV4 opt = option;
		QStyledItemDelegate::initStyleOption(&opt, index);
		QRectF rect = opt.rect;

		if (!index.parent().isValid())
		{
			const Parameter* pParameter = index.data().value<Parameter*>();

			if (opt.state & QStyle::State_MouseOver)
			{
				painter->fillRect(rect, pParameter->getColor().darker(110));
			}
			else
			{
				painter->fillRect(rect, pParameter->getColor());
			}

			if (index.column()==0)
			{
				QFont font = painter->font();
				font.setBold(true);
				painter->setFont(font);

				// Centering vertically
				QSize sizeData = sizeHint(opt, index);
				int offset((rect.height()-sizeData.height())/2);
				QPointF point = rect.bottomLeft();
				rect.moveBottom(point.y()+offset);	// Relative to up-left corner of the treeview

				if (opt.state & QStyle::State_Open)
				{
					painter->drawText(rect, Qt::AlignLeft, QString("- ") + pParameter->label.c_str());
				}
				else
				{
					painter->drawText(rect, Qt::AlignLeft, QString("+ ") + pParameter->label.c_str());
				}
			}
		}
		else if (index.column()==0)
		{
			const QModelIndex& parent = index.parent();
			Parameter* pParentParameter = parent.data().value<Parameter*>();
			Parameter* pParameter = index.data().value<Parameter*>();
			QColor color = pParentParameter->getColor();

			if (pParameter->isEnabled())
			{
				const QColor& colorParameter = pParameter->getColor();
				if (colorParameter.isValid())
				{
					color = colorParameter;
				}

				if (opt.state & QStyle::State_MouseOver)
				{
					painter->fillRect(rect, color.lighter(120));
				}
				else
				{
					painter->fillRect(rect, color.lighter(130));
				}
			}

			// Centering vertically
			QSize sizeData = sizeHint(opt, index);
			int offset((rect.height()-sizeData.height())/2);
			QPointF point = rect.bottomLeft();
			rect.moveBottom(point.y()+offset);	// Relative to up-left corner of the treeview

			// Horizontal indent
			const int INDENT(painter->fontMetrics().width("- "));
			rect.moveLeft(point.x()+INDENT);	// Relative to up-left corner of the treeview
			rect.setWidth(rect.width()-INDENT);

			painter->drawText(rect, Qt::AlignLeft, pParameter->label.c_str());
		}

		painter->restore();
	}

	virtual QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem &, const QModelIndex&) const
	{
		ParamItemEditor* pEditor = new ParamItemEditor(parent);
		connect(pEditor, SIGNAL(editFinished(QWidget*)), this, SIGNAL(commitData(QWidget*)));
		return pEditor;
	}

	virtual void updateEditorGeometry( QWidget * editor, const QStyleOptionViewItem & option, const QModelIndex & index ) const
	{
		QStyledItemDelegate::updateEditorGeometry(editor, option, index);
	}

	virtual void setEditorData(QWidget* p, const QModelIndex& index) const
	{
		ParamItemEditor* pEditor = dynamic_cast<ParamItemEditor*>(p);
		const QVariant& variant = index.data(Qt::EditRole);
		if (!variant.isNull() && qVariantCanConvert<Parameter*>(variant))
		{
			Parameter* pParameter = variant.value<Parameter*>();
			pEditor->setData(pParameter);
		}
	}

	virtual void setModelData(QWidget* p1, QAbstractItemModel* p2, const QModelIndex& index) const
	{
		ParamItemEditor* pEditor = dynamic_cast<ParamItemEditor*>(p1);
		Parameter* pParameter= pEditor->getData();
		QVariant variant;
		variant.setValue(pParameter);
		p2->setData(index, variant);
	}
};
