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
//

#include "ParamItemDelegate.h"
#include "ParamItemEditor.h"

#include <QtGui/QPainter>
#include <QtGui/QFontMetrics>
#include <QtWidgets/QStyleOptionButton>
#include <QtWidgets/QStyleOptionViewItem>

ParamItemDelegate::ParamItemDelegate(QObject* pParent)
	: QStyledItemDelegate(pParent)
{
}

ParamItemDelegate::~ParamItemDelegate()
{
}

QSize ParamItemDelegate::sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const
{
	QSize result = QStyledItemDelegate::sizeHint(option, index);
	return result;
}

void ParamItemDelegate::initStyleOption(QStyleOptionViewItem* pOption, const QModelIndex& index) const
{
	QStyledItemDelegate::initStyleOption(pOption, index);

	const QVariant& variant = index.data(Qt::EditRole);
	if (!variant.isNull())
	{
		const Parameter& parameter = variant.value<Parameter>();
		const AnyProperty::Value& value = parameter.getValue();
		if (value.type()==typeid(int))
		{
			if (parameter.hasEnums())
			{
				const Parameter::DictEnums& dictEnums = parameter.getEnums();
				Parameter::DictEnums::const_iterator it = dictEnums.find(boost::any_cast<int>(value));
				if (it!=dictEnums.end())
				{
					pOption->text = it->second;
				}
			}
			else
			{
				pOption->text = QString("%1").arg(boost::any_cast<int>(value));
			}
		}
		else if (value.type() == typeid(float))
		{
				pOption->text = QString("%1").arg(boost::any_cast<float>(value));
		}
		else if (value.type() == typeid(QString))
		{
			pOption->text = boost::any_cast<QString>(value);
		}
		else if (value.type() == typeid(QPolygonF))
		{
			pOption->text = "...";
		}
	}
}

void ParamItemDelegate::paint(QPainter* pPainter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
	QStyleOptionViewItem viewItemOption;
	initStyleOption(&viewItemOption, index);

	// Paint with EditRole data
	if (index.column()==1)
	{
		const QVariant& variant = index.data(Qt::EditRole);
		if (!variant.isNull())
		{
			const Parameter& parameter = variant.value<Parameter>();
			const AnyProperty::Value& value = parameter.getValue();
			if (value.type()==typeid(bool))
			{
				bool bState = boost::any_cast<bool>(value);
				QStyle* pStyle = option.widget->style();
				QStyleOptionButton optCheckBox;
				optCheckBox.rect = option.rect;
				optCheckBox.state = QStyle::State_Enabled | (bState?QStyle::State_On:QStyle::State_Off);
				pStyle->drawControl(QStyle::CE_CheckBox, &optCheckBox, pPainter);
			}
		}
	}

	QStyledItemDelegate::paint(pPainter, option, index);

	/* TODO
	   if (index.parent().isValid() && index.column()==1)
	   {
	   return;
	   }
	   pPainter->save();

	   pPainter->setRenderHint(QPainter::Antialiasing);
	   QStyleOptionViewItemV4 opt = option;
	   QStyledItemDelegate::initStyleOption(&opt, index);
	   QRectF rect = opt.rect;

	   if (!index.parent().isValid())
	   {
	   const Parameter* pParameter = index.data().value<Parameter*>();

	   if (opt.state & QStyle::State_MouseOver)
	   {
	   pPainter->fillRect(rect, pParameter->getColor().darker(110));
	   }
	   else
	   {
	   pPainter->fillRect(rect, pParameter->getColor());
	   }

	   if (index.column()==0)
	   {
	   QFont font = pPainter->font();
	   font.setBold(true);
	   pPainter->setFont(font);

	// Centering vertically
	QSize sizeData = sizeHint(opt, index);
	int offset((rect.height()-sizeData.height())/2);
	QPointF point = rect.bottomLeft();
	rect.moveBottom(point.y()+offset);	// Relative to up-left corner of the treeview

	if (opt.state & QStyle::State_Open)
	{
	pPainter->drawText(rect, Qt::AlignLeft, QString("- ") + pParameter->label.c_str());
	}
	else
	{
	pPainter->drawText(rect, Qt::AlignLeft, QString("+ ") + pParameter->label.c_str());
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
	pPainter->fillRect(rect, color.lighter(120));
	}
	else
	{
	pPainter->fillRect(rect, color.lighter(130));
	}
	}

	// Centering vertically
	QSize sizeData = sizeHint(opt, index);
	int offset((rect.height()-sizeData.height())/2);
	QPointF point = rect.bottomLeft();
	rect.moveBottom(point.y()+offset);	// Relative to up-left corner of the treeview

	// Horizontal indent
	const int INDENT(pPainter->fontMetrics().width("- "));
	rect.moveLeft(point.x()+INDENT);	// Relative to up-left corner of the treeview
	rect.setWidth(rect.width()-INDENT);

	pPainter->drawText(rect, Qt::AlignLeft, pParameter->label.c_str());
}

pPainter->restore();
*/
}

QWidget* ParamItemDelegate::createEditor(QWidget* parent, const QStyleOptionViewItem &, const QModelIndex& index) const
{
	if (index.flags() & Qt::ItemIsEditable)
	{
		ParamItemEditor* pEditor = new ParamItemEditor(parent);
		connect(pEditor, SIGNAL(editFinished(QWidget*)), this, SIGNAL(commitData(QWidget*)));
		return pEditor;
	}

	return NULL;
}

void ParamItemDelegate::setEditorData(QWidget* pWidget, const QModelIndex& index) const
{
	ParamItemEditor* pEditor = dynamic_cast<ParamItemEditor*>(pWidget);
	QVariant variant = index.data(Qt::EditRole);
	if (!variant.isNull())
	{
		pEditor->setData(variant.value<Parameter>());
	}
}

void ParamItemDelegate::setModelData(QWidget* pWidget, QAbstractItemModel* pModel, const QModelIndex& index) const
{
	ParamItemEditor* pEditor = dynamic_cast<ParamItemEditor*>(pWidget);
	if (pEditor->hasDataChanged())
	{
		QVariant variant;
		variant.setValue(pEditor->getData());
		pModel->setData(index, variant);
	}
}
