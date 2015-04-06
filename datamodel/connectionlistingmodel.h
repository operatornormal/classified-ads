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

#ifndef CONNECTIONLISTINGMODEL_H
#define CONNECTIONLISTINGMODEL_H

#include "../mcontroller.h"
#include "../net/connection.h"
#include <QAbstractTableModel>

class Model ;

/**
 * @brief Model-class for displaying open network connections
 * This is the underlying data-container of the "network status"
 * dialog
 */
class ConnectionListingModel: public QAbstractTableModel {
    Q_OBJECT
public:
    /**
     * instead of relaying the connections from Model::iConnections
     * we keep our local copy of the same stuff here, updating
     * it as necessary
     */
    typedef struct ConnectionDisplayItemStructure {
        QHostAddress iAddr ;
        bool iIsInBound ;
        unsigned long iBytesIn ;
        unsigned long iBytesOut ;
        time_t iOpenTime ;
        Hash iNodeFingerPrint ;
    } ConnectionDisplayItem ;

    ConnectionListingModel(Model& aModel,MController& aController) ;
    ~ConnectionListingModel() ;

    virtual int rowCount(const QModelIndex & parent = QModelIndex())  const  ;
    /**
     * re-implemented from QAbstractTableModel
     * @return number of columns in view
     */
    virtual int columnCount(const QModelIndex & parent = QModelIndex())  const  ;
    /**
     * re-implemented from QAbstractListModel
     * @return data to display in list
     */
    virtual QVariant data(const QModelIndex & index, int role = Qt::DisplayRole) const ;
    virtual QVariant headerData ( int section, Qt::Orientation orientation, int role = Qt::DisplayRole ) const ;

signals:
    void error(MController::CAErrorSituation aError,
               const QString& aExplanation) ;

protected: // methods
    /**
     * for periodical stuff inside datamodel
     */
    void timerEvent(QTimerEvent *event);
private: // methods
    void updateModelContents() ;
private: // data
    Model& iModel ;
    MController& iController ;
    QList<ConnectionDisplayItem> iConnections ;
    int iTimerId ;
} ;
#endif
