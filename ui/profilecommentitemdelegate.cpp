/*     -*-C++-*- -*-coding: utf-8-unix;-*-
       Classified Ads is Copyright (c) Antti Järvinen 2013. 

       This file is part of Classified Ads.

    Classified Ads is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    Classified Ads is distributed in the hope that it will be useful,
       but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with Classified Ads; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
*/

#include "profilecommentitemdelegate.h"
#include <QTextDocument>
#include <QApplication> // for styles
#include <QPainter>
#include <QAbstractTextDocumentLayout>
#include "../datamodel/profilecommentlistingmodel.h"
#include "../log.h"

// ideas mostly stolen from 
// http://stackoverflow.com/questions/1956542/how-to-make-item-view-render-rich-html-text-in-qt
// so thanks Serge :)

ProfileCommentItemDelegate::ProfileCommentItemDelegate(ProfileCommentListingModel& aListingModel,
						       const QWidget& aDrawableWidget) :
  iListingModel(aListingModel) ,
  iDrawableWidget(aDrawableWidget)
{
  // empty, just initialize the listing model 
}

ProfileCommentItemDelegate::~ProfileCommentItemDelegate()
{

}

void ProfileCommentItemDelegate::paint(QPainter *painter, 
				       const QStyleOptionViewItem &option, 
				       const QModelIndex &index) const
{
  QStyleOptionViewItemV4 optionV4 = option;
  initStyleOption(&optionV4, index);

  QStyle *style = optionV4.widget? optionV4.widget->style() : QApplication::style();

  QTextDocument doc;
  doc.setHtml(iListingModel.data(index, Qt::UserRole+1).toString());
  QString textToDisplayOnTop (iListingModel.data(index, Qt::UserRole+2).toString()) ; 
    
  // Painting item without text
  optionV4.text = QString(); 
  style->drawControl(QStyle::CE_ItemViewItem, &optionV4, painter);

  QAbstractTextDocumentLayout::PaintContext ctx;

  QPalette::ColorGroup cg = option.state & QStyle::State_Enabled ? QPalette::Normal : QPalette::Disabled;
  if (cg == QPalette::Normal && !(option.state & QStyle::State_Active)) {
    cg = QPalette::Inactive;
  }

  // Highlighting text if item is selected
  if (optionV4.state & QStyle::State_Selected) {
    ctx.palette.setColor(QPalette::Text, 
			 optionV4.palette.color(QPalette::Active, 
						QPalette::HighlightedText));
  }
  // textRect is whole item rect
  QRect textRect = option.rect ;
  doc.setTextWidth(iDrawableWidget.width()-10);
  painter->save();
  // paint "title text" on top of the rect
  painter->drawText(QRect(textRect.left(), textRect.top(), iDrawableWidget.width()-10, QFontMetrics(painter->font()).height()),
		    option.displayAlignment, textToDisplayOnTop);
  const unsigned numberOfAttachments = iListingModel.data(index, Qt::UserRole+3).toUInt() ;
  if ( numberOfAttachments == 0 ) {
    // then figure out where the html should begin: after the "title text"
    QPoint drawingOffsetForHtml ( 0 , QFontMetrics(painter->font()).height()+textRect.top() )  ;
    painter->translate(drawingOffsetForHtml) ; 
  } else {
    // print also number of attachments
    painter->drawText(QRect(textRect.left(), 
			    textRect.top()+QFontMetrics(painter->font()).height(), 
			    iDrawableWidget.width()-10, 
			    QFontMetrics(painter->font()).height()),
		      option.displayAlignment, tr("Attachments: ") + QString::number(numberOfAttachments));
    // then set offset to be top+2 lines of text
    QPoint drawingOffsetForHtml ( 0 , (2*QFontMetrics(painter->font()).height())+textRect.top() )  ;
    painter->translate(drawingOffsetForHtml) ; 
  }
  // and draw the actual comment text:
  doc.documentLayout()->draw(painter, ctx);
  painter->restore();
}

QSize ProfileCommentItemDelegate::sizeHint ( const QStyleOptionViewItem & option, 
					     const QModelIndex & index ) const
{
  QStyleOptionViewItemV4 options = option;
  initStyleOption(&options, index);

  QTextDocument doc;
  doc.setHtml(iListingModel.data(index, Qt::UserRole+1).toString()); 
  doc.setTextWidth(iDrawableWidget.width()-10);
  const QFont font = QApplication::font();
  // why making this a member variable results in crash at dialog close??
  const QFontMetrics fm(font);  // so, now automatics and works
  const unsigned numberOfAttachments = iListingModel.data(index, Qt::UserRole+3).toUInt() ;
  if ( numberOfAttachments == 0 ) {
    QSize retval( iDrawableWidget.width()-10, doc.size().height()+fm.height());
    return retval ;
  } else {
    QSize retval( iDrawableWidget.width()-10, doc.size().height()+(fm.height()*2));
    return retval ; 
  }
}
