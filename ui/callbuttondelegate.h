/*  -*-C++-*- -*-coding: utf-8-unix;-*-
  Classified Ads is Copyright (c) Antti Järvinen 2015.

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

#ifndef CALLBUTTON_DELEGATE_H
#define CALLBUTTON_DELEGATE_H

#include <QItemDelegate>
#include <QApplication>
#include <QMouseEvent>
#include <QDialog>

class VoiceCallEngine ;

/**
 * @brief class for displaying button inside table view
 */
class CallButtonDelegate : public QItemDelegate {
    Q_OBJECT

public:
    /**
     * Constructor
     * @param aCallEngine is datamodel where call data comes from.
     * @param aParent Just parent object.
     */
    CallButtonDelegate(VoiceCallEngine& aCallEngine,
                       QObject* aParent = NULL ) ;
    /** destructor */
    ~CallButtonDelegate() ;
public:
    /**
     * Painter, overridden from QItemDelegate
     */
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;
    bool editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index);
private:
    VoiceCallEngine& iCallEngine ;
};


#endif
