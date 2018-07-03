/*     -*-C++-*- -*-coding: utf-8-unix;-*-
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

#include "callbuttondelegate.h"
#include <QPainter>
#include "../net/voicecallengine.h"
#include "../log.h"

const int KCallButtonHeight ( 30 ) ; 

CallButtonDelegate::CallButtonDelegate(VoiceCallEngine& aCallEngine,
                                       QObject* aParent) :
    QItemDelegate(aParent),
    iCallEngine(aCallEngine) {
}

CallButtonDelegate::~CallButtonDelegate() {

}

// idea stolen from 
// http://stackoverflow.com/questions/11777637/adding-button-to-qtableview
// so thanks SingerOfTheFall and Kim Bowles Sørhus
void CallButtonDelegate::paint(QPainter *painter, 
                               const QStyleOptionViewItem &option, 
                               const QModelIndex &index) const
{
    QStyleOptionButton button;
    QRect r = option.rect;//getting the rect of the cell
    int x,y,w,h;
    x = r.left();//the X coordinate
    y = r.top();//the Y coordinate
    w = r.width();// fill whole rectangle
    h = KCallButtonHeight;//button height
    button.rect = QRect(x,y,w,h);
    button.text = iCallEngine.data(index,Qt::DisplayRole).toString();
    button.state = QStyle::State_Enabled;

    QApplication::style()->drawControl( QStyle::CE_PushButton, &button, painter);
}

bool CallButtonDelegate::editorEvent(QEvent *event, 
                                     QAbstractItemModel * /* model */ , 
                                     const QStyleOptionViewItem &option, 
                                     const QModelIndex &index)
{
    if( event->type() == QEvent::MouseButtonRelease )
    {
        QMouseEvent * e = (QMouseEvent *)event;
        int clickX = e->x();
        int clickY = e->y();

        QRect r = option.rect;//getting the rect of the cell
        int x,y,w,h;
        x = r.left() ;//the X coordinate
        y = r.top();//the Y coordinate
        w = r.width();//button width should fill whole rectangle
        h = KCallButtonHeight ;// height constant

        if( clickX > x && clickX < x + w )
            if( clickY > y && clickY < y + h )
            {
                const int callId ( iCallEngine.data(index, Qt::UserRole).toInt() );
                switch ( index.column() ) {
                case 3:
                    // accept call
                    iCallEngine.acceptCall(callId) ; 
                    break ;
                case 4: 
                    // end/reject call
                    iCallEngine.closeCall(callId) ; 
                    break ;
                default:
                    QLOG_STR("Huh, editor-event in colum with no editor?") ; 
                    break ;
                }
            }
    }
    return true ; 
}
