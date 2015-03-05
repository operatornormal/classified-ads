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
};
#endif
