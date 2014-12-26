/*     -*-C++-*- -*-coding: utf-8-unix;-*-
       Classified Ads is Copyright (c) Antti Järvinen 2013. 

       This file is part of Classified Ads.

       Classified Ads is free software: you can redistribute it and/or modify
       it under the terms of the GNU General Public License as published by
       the Free Software Foundation, either version 3 of the License, or
       (at your option) any later version.

       Classified Ads is distributed in the hope that it will be useful,
       but WITHOUT ANY WARRANTY; without even the implied warranty of
       MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
       GNU General Public License for more details.

       You should have received a copy of the GNU General Public License
       along with Classified Ads.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef PROFILECOMMENTITEM_DELEGATE_H
#define PROFILECOMMENTITEM_DELEGATE_H

#include <QStyledItemDelegate>

class ProfileCommentListingModel ;

/**
 * @brief class for displaying single profile comment in a list view
 */
class ProfileCommentItemDelegate : public QStyledItemDelegate
{
public:
  /**
   * Constructor 
   */
  ProfileCommentItemDelegate(ProfileCommentListingModel& aListingModel) ; 
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
};


#endif /* PROFILECOMMENTITEM_DELEGATE_H */
