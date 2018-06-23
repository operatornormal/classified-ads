/*     -*-C++-*- -*-coding: utf-8-unix;-*-
  Classified Ads is Copyright (c) Antti Järvinen 2013-2018.

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

#ifndef PROFILEREADERLISTINGMODEL_H
#define PROFILEREADERLISTINGMODEL_H

#include <QAbstractTableModel>
#include "../util/hash.h"
#include "../mcontroller.h"

class Profile ;
class Model ;
class MController ; 

/**
 * @brief Model-class for helping display of readers of a profile
 */
class ProfileReadersListingModel: public QAbstractTableModel {
    Q_OBJECT
public:
    ProfileReadersListingModel(Profile& aProfile,
                               MController& aController ) ;/**< Constructor */
    ~ProfileReadersListingModel() ; /**< Destructor */

    /**
     * method for adding a reader. this change will be made to iProfile
     * too.
     * @param aFingerPrint is fingerprint of the reader to add
     */
    void addReader(const Hash& aFingerPrint) ;
    /**
     * method for removing a reader. this change will be made to iProfile
     * too.
     * @param aFingerPrint is fingerprint of the reader to remove
     */
    void removeReader(const Hash& aFingerPrint) ;

    /**
     * method for retrieving display-name of a profile
     */
    QString profileDisplayNameByFingerPrint(const Hash& aFingerPrint)  ;

    /**
     * re-implemented from QAbstractTableModel
     * @return number of rows in list
     */
    virtual int rowCount(const QModelIndex & parent = QModelIndex())  const  ;
    /**
     * re-implemented from QAbstractTableModel
     * @return number of columns in model
     */
    virtual int columnCount ( const QModelIndex & parent = QModelIndex() ) const ;
    /**
     * re-implemented from QAbstractTableModel
     * @return data to display in list
     */
    virtual QVariant data(const QModelIndex & index, int role = Qt::DisplayRole) const ;
    virtual QVariant headerData(int aSection, Qt::Orientation orientation, int role) const ;
signals:
    void error(MController::CAErrorSituation aError,
               const QString& aExplanation) ;
private: // methods
    void updateReaderDataInArray( const Hash& aFingerPrint,bool aEmit ) ;
private: // data
    Profile& iProfile ;
    QList<QPair<Hash,QString> > iNamesAndFingerPrints ;
    MController& iController ;
} ;
#endif
