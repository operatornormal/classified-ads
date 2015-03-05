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

#ifndef CONTACTSEARCHMODEL_H
#define CONTACTSEARCHMODEL_H

#include <QAbstractListModel>
#include "../util/hash.h"
#include "../mcontroller.h"
#include "contact.h"

class Model ; 

/**
 * @brief Model-class for performing search on contact-list contacts
 * This is supposed to act as an underlying data-container for 
 * an user-interface view (QTableView etc.)
 */
class ContactListingModel: public QAbstractTableModel {
 Q_OBJECT
public: 
 ContactListingModel(Model& aModel,MController& aController) ;
  ~ContactListingModel() ; 

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

  void newContactAdded(const Contact& aContact) ;  
  void removeContact(const Hash& aContact) ;
  /** method for retrieving contact. 
   * @param aFingerPrint tells what to seek
   * @param aResultingContact will have its field filled if match is made
   * @return true if contact was found
   */
  bool contactByFingerPrint(const Hash& aFingerPrint, Contact* aResultingContact) const ;
  /**
   * whoever calls this method, must set Model.lock() first 
   */
  QVariant contactsAsQVariant() const ; 
  void setContactsFromQVariant(const QVariantList& aContacts) ; 
  bool isContactContained(const Hash& aFingerPrint) const ; 
  void clearContents() ; 
  /**
   * method that returns subset of contacts that are marked as
   * trusted 
   */
  QList<Hash> trustList() const ; 
signals:
  void error(MController::CAErrorSituation aError,
	     const QString& aExplanation) ;

private: // data
  Model& iModel ; 
  MController& iController ;
  QList<Contact> iContacts ; 
} ; 
#endif
