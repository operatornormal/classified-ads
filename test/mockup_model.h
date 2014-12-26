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

/**
 * @brief not a real datamodel. debugging aid. 
 */
class MockUpModel : public MModelProtocolInterface
{
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
} ; 

#endif /* #define MOCKUP_CONTROLLER_H */
