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

#ifndef PROFILESEARCHMODEL_H
#define PROFILESEARCHMODEL_H

#include <QAbstractListModel>
#include "../util/hash.h"
#include "../mcontroller.h"

class Model ; 
/**
 * @brief Model-class for performing a name-based search of profiles.
 * This is supposed to act as an underlying data-container for 
 * an user-interface view (QListView etc.)
 */
class ProfileSearchModel: public QAbstractListModel {
 Q_OBJECT
public: 
   ProfileSearchModel(Model& aModel) ;
   ~ProfileSearchModel() ; 

   void setSearchString(const QString& aSearch) ; 
   /**
    * re-implemented from QAbstractListModel
    * @return number of rows in list 
    */
   virtual int rowCount(const QModelIndex & parent = QModelIndex())  const  ; 
   /**
    * re-implemented from QAbstractListModel
    * @return data to display in list
    */
   virtual QVariant data(const QModelIndex & index, int role = Qt::DisplayRole) const ;
signals:
  void error(MController::CAErrorSituation aError,
             const QString& aExplanation) ;
private:// methods
   void performSearch() ; 
private: // data
   Model& iModel ; 
   QList<QPair<Hash,QString> > iProfiles ; 
   QString iSearchString ;
} ; 
#endif
