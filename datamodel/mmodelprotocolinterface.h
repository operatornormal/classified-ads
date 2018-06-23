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

#ifndef M_MODEL_PROTOCOL_INTERFACE_H
#define M_MODEL_PROTOCOL_INTERFACE_H
#include <QObject>
#include "../mcontroller.h"
#include "netrequestexecutor.h"
#include "mmodelprotocolinterface.h"

class MNodeModelProtocolInterface ;
class ProfileModel ;
class BinaryFileModel ;
class ClassifiedAdsModel ;
class PrivMessageModel ;
class ContentEncryptionModel ;
class ProfileCommentModel ;
class SearchModel ;
class CaDbRecordModel ; 
class TclModel ; 
class QSqlDatabase ; 
/**
 * @brief Pure-virtual interface of datamodel for message parser to use
 *
 * This interface is given to incoming-message -parsing part of
 * networking code. Main reason for existence of this interface
 * is testability; in unit-test situation we will have mock-up model
 * behind this interface, in normal situation the normal datamodel.
 */
class MModelProtocolInterface : public QObject {
    Q_OBJECT
public:
    /**
     * method for adding a network request
     * @param aRequest is the request to add
     * @return none
     */
    virtual void addNetworkRequest(NetworkRequestExecutor::NetworkRequestQueueItem&
                                   aRequest) const = 0 ;
    /**
     * thread sync: this claims access to datamodel
     */
    virtual bool lock() = 0 ;
    /**
     * thread sync: releases data model to other threads
     */
    virtual void unlock() = 0 ;
    /**
     * method for getting node-specific datamodel
     */
    virtual MNodeModelProtocolInterface& nodeModel() const = 0 ;
    /**
     * method for getting profile-specific datamodel
     */
    virtual ProfileModel& profileModel() const = 0 ;
    /**
     * method for getting blob-specific datamodel
     */
    virtual BinaryFileModel& binaryFileModel() const = 0 ;
    /** method for getting the ads datamodel */
    virtual ClassifiedAdsModel& classifiedAdsModel() const = 0 ;
    /** method for getting the private message datamodel */
    virtual PrivMessageModel& privateMessageModel() const = 0 ;
    /** method for getting the en/de-cryption part of the datamodel */
    virtual ContentEncryptionModel& contentEncryptionModel() const = 0 ;
    virtual ProfileCommentModel& profileCommentModel() const = 0 ; /**< method for getting the comment datamodel */
    virtual SearchModel* searchModel() const = 0 ; /**< method for getting the full text search datamodel */
    virtual CaDbRecordModel* caDbRecordModel() const = 0 ; /**< method for getting distributed database model part */
    virtual TclModel& tclModel() const = 0 ;
    /** 
     * Method for opening database connection. Since Qt5.11 database
     * class can't be shared between threads.
     * @param aIsFirstTime Optional parameter that, when set to non-NULL
     *        will have its value set to true, if the database did not
     *        exist prior to this call. 
     *
     * @return Instance of database connection. Caller is responsible
     *         to properly ->close() and call ::removeDatabase() in
     *         correct way. 
     */
    virtual QSqlDatabase dataBaseConnection(bool* aIsFirstTime = NULL) = 0 ; 
    /**
     * Currently open connections. Caller does not own the returned list
     * and should not try adding/removing items from it. 
     */
    virtual const QList <Connection *>& getConnections() const = 0 ;
    /**
     * Currently pending network requests.
     * Even as this returns a pointer, not a reference,
     * ownership of the list is not transferred ; caller
     * may modiify content but is not supposed to delete
     */
    virtual QList <NetworkRequestExecutor::NetworkRequestQueueItem>& getNetRequests() const = 0 ;
};
#endif
