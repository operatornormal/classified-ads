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

#ifndef PROFILECOMMENTITEM_DELEGATE_H
#define PROFILECOMMENTITEM_DELEGATE_H

#include <QStyledItemDelegate>

class ProfileCommentListingModel ;

/**
 * @brief class for displaying single profile comment in a list view
 */
class ProfileCommentItemDelegate : public QStyledItemDelegate {
    Q_OBJECT

public:
    /**
     * Constructor
     * @param aListingModel is the datamodel part where content comes from
     * @param aDrawableWidget is the container where these items will
     *        be drawn. It is passed here because we want to query
     *        its dimensions at runtime.
     */
    ProfileCommentItemDelegate(ProfileCommentListingModel& aListingModel,
                               const QWidget& aDrawableWidget) ;
    /** destructor */
    ~ProfileCommentItemDelegate() ;
protected:
    /**
     * implemented from class QStyledItemDelegate
     */
    void paint ( QPainter * painter, const QStyleOptionViewItem & option, const QModelIndex & index ) const;
    /**
     * implemented from class QStyledItemDelegate
     */
    QSize sizeHint ( const QStyleOptionViewItem & option, const QModelIndex & index ) const;
private:
    ProfileCommentListingModel& iListingModel ;
    const QWidget& iDrawableWidget ;
};


#endif /* PROFILECOMMENTITEM_DELEGATE_H */
