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

#include "mockup_model.h"
#include "mockup_nodemodel.h"
#include "../log.h"
#include "../net/node.h"
#include "../datamodel/profilemodel.h"
#include <openssl/ssl.h> // library init 
#include <openssl/rand.h> // random seed things
#include "../datamodel/binaryfilemodel.h"
#include "../datamodel/camodel.h"
#include "../datamodel/privmsgmodel.h"
#include "../datamodel/profilecommentmodel.h"
#include "../datamodel/searchmodel.h"
#include "../datamodel/contentencryptionmodel.h"
#include <QFile>

MockUpModel::MockUpModel( MController *aController ) : 
  iNetworkRequests(NULL),
  iController(aController),
  iProfileModel(NULL),
  iBinaryFileModel(NULL),
  iCAModel(NULL),
  iPrivMsgModel(NULL),
  iContentEncryptionModel(NULL),
  iProfileCommentModel(NULL),
  iSearchModel(NULL)
{
  LOG_STR("MockUpModel::MockUpModel in\n") ; 

  SSL_load_error_strings() ;
  SSL_library_init() ; 
  QFile randomFile("/dev/urandom") ;
  QByteArray randomBytes = randomFile.read(1024) ;
  char *randomBytesPointer = randomBytes.data() ;
  RAND_seed(randomBytesPointer, 1024);
  iNodeModel = new MockUpNodeModel(iController) ; 
  iProfileModel = new ProfileModel(aController, *this)  ; 
  iBinaryFileModel = new BinaryFileModel(aController, *this) ;
  iCAModel = new ClassifiedAdsModel(aController, *this) ;
  iPrivMsgModel = new PrivMessageModel(aController, *this) ;
  iProfileCommentModel = new ProfileCommentModel(aController, *this) ;
  iContentEncryptionModel = new ContentEncryptionModel(aController, *this) ;
  iSearchModel = new SearchModel(*this,*iController) ;
  iSearchModel->setObjectName("CA SearchModel test") ; 
  LOG_STR("MockUpModel::MockUpModel out\n") ; 
}

MockUpModel::~MockUpModel()
{
  delete iNetworkRequests ;
  delete iNodeModel ; 
  delete iProfileModel ;
  delete iBinaryFileModel ; 
  delete iCAModel ; 
  delete iPrivMsgModel ; 
  delete iProfileCommentModel ;
  delete iContentEncryptionModel ;
  delete iSearchModel;
  LOG_STR("MockUpModel::~MockUpModel\n") ; 
}
void MockUpModel::addNetworkRequest(NetworkRequestExecutor::NetworkRequestQueueItem&
                                 aRequest) const  
{
  LOG_STR2(" MockUpModel::addNetworkRequest type %d\n", aRequest.iRequestType) ; 
  if ( iNetworkRequests!= NULL ) {
  iNetworkRequests->append(aRequest) ;
  }
}

bool MockUpModel::lock()  
{
  return iMutex.tryLock(100*1000); 
}

void MockUpModel::unlock()  
{
  iMutex.unlock() ; 
}

MNodeModelProtocolInterface& MockUpModel::nodeModel() const  
{
  return *iNodeModel ; 
} 

ProfileModel& MockUpModel::profileModel() const 
{
  return *iProfileModel ; 
}

BinaryFileModel& MockUpModel::binaryFileModel() const {
  return *iBinaryFileModel ;
}

ClassifiedAdsModel& MockUpModel::classifiedAdsModel() const {
  return *iCAModel ;
}

PrivMessageModel& MockUpModel::privateMessageModel() const {
  return *iPrivMsgModel ;
}

ContentEncryptionModel& MockUpModel::contentEncryptionModel() const {
  return *iContentEncryptionModel ;
}

ProfileCommentModel& MockUpModel::profileCommentModel() const {
  return *iProfileCommentModel ; 
}

SearchModel* MockUpModel::searchModel() const {
  return iSearchModel ; 
}
