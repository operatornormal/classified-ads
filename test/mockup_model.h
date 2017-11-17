/*     -*-C++-*- -*-coding: utf-8-unix;-*-
  Classified Ads is Copyright (c) Antti Järvinen 2013-2016.

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

#ifndef MOCKUP_MODEL_H
#define MOCKUP_MODEL_H
#include "../mcontroller.h"
#include "../datamodel/mmodelprotocolinterface.h"
#include "../datamodel/mnodemodelprotocolinterface.h"
#include <QMutex>
class MockUpNodeModel ;
class ProfileModel ;
class BinaryFileModel ;
class ClassifiedAdsModel ;
class PrivMessageModel ;
class ContentEncryptionModel ;
class ProfileCommentModel ;
class SearchModel;
class CaDbRecordModel; 

/**
 * @brief not a real datamodel. debugging aid.
 */
class MockUpModel : public MModelProtocolInterface {
    Q_OBJECT

public:
    /**
     * constructor
     */
    MockUpModel(MController *aMController) ;
    /**
     * Destructor
     */
    ~MockUpModel() ;
    /**
     * method for adding a network request
     * @param aRequest is the request to add
     * @return none
     */
    virtual void addNetworkRequest(NetworkRequestExecutor::NetworkRequestQueueItem&
                                   aRequest) const  ;
    /**
     * thread sync: this claims access to datamodel
     */
    virtual bool lock()  ;
    /**
     * thread sync: releases data model to other threads
     */
    virtual void unlock()  ;
    /**
     * method for getting node-specific datamodel
     */
    virtual MNodeModelProtocolInterface& nodeModel() const  ;
    virtual ProfileModel& profileModel() const ;

    /**
     * method for getting blob-specific datamodel
     */
    virtual BinaryFileModel& binaryFileModel() const  ;
    /** method for getting the ads datamodel */
    virtual ClassifiedAdsModel& classifiedAdsModel() const  ;
    virtual PrivMessageModel& privateMessageModel() const ; /**< method for getting the priv msg datamodel */

    virtual ContentEncryptionModel& contentEncryptionModel() const  ;
    virtual ProfileCommentModel& profileCommentModel() const  ; /**< method for getting the comment datamodel */
    virtual SearchModel* searchModel() const  ; /**< method for getting the full text search datamodel */
    virtual CaDbRecordModel* caDbRecordModel() const ; /**< method for getting distributed database model part */
    virtual TclModel& tclModel() const ;
public:
    MockUpNodeModel* iNodeModel ;
    QList<NetworkRequestExecutor::NetworkRequestQueueItem>* iNetworkRequests ;
private: // member data
    MController *iController  ;
    QMutex iMutex  ;
    ProfileModel* iProfileModel ;
    BinaryFileModel* iBinaryFileModel  ;
    ClassifiedAdsModel* iCAModel ;
    PrivMessageModel* iPrivMsgModel ;
    ContentEncryptionModel* iContentEncryptionModel ;
    ProfileCommentModel* iProfileCommentModel ;
    SearchModel* iSearchModel ;
    CaDbRecordModel* iCaDbRecordModel ;
    TclModel* iTclModel ;
} ;

#endif /* #define MOCKUP_CONTROLLER_H */
