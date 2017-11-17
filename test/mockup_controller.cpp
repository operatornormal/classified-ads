/*   -*-C++-*- -*-coding: utf-8-unix;-*-
  Classified Ads is Copyright (c) Antti Järvinen 2013-2017.

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

#include "mockup_controller.h"
#include "../log.h"
#include "../net/node.h"
#include "../datamodel/model.h"
#include "../net/networklistener.h"
#include "../net/networkconnectorengine.h"
#include <unistd.h> // for getpid() 
#include <signal.h>
#include "../datamodel/binaryfile.h"
#include "mockup_voicecallengine.h"
#include "../tcl/tclWrapper.h"

MockUpController::MockUpController() :
    iNode(NULL),
    iModel(NULL),
    iListener(NULL),
    iCallEngine(NULL),
    iTclWrapper(NULL) {
    LOG_STR("MockUpController::Controller in\n") ;
    qRegisterMetaType<MController::CAErrorSituation>("MController::CAErrorSituation");
    iModel = new Model(this);
    iNode = new Node(iModel->nodeModel().nodeFingerPrint(),
                     iModel->nodeModel().listenPortOfThisNode()) ;
    iListener = new  NetworkListener (this, iModel) ;
    // network listener enumerates network interfaces and sets
    // possible ipv6 addr into iNode() ->
    iCallEngine = new MockUpVoiceCallEngine ; 
    iTclWrapper = new TclWrapper(*iModel, *this) ; 
    LOG_STR("MockUpController::Controller out\n") ;
}

MockUpController::~MockUpController() {
    LOG_STR("MockUpController::~Controller\n") ;
    // .. connections reference iListener.
    // so in order to prevent random crash at closing, lets first get rid
    // of connections, only after that delete iListener ;
    iModel->closeAllConnections(true) ;
    // now safe to delete listener (and net engine)
    delete iListener ; // will delete also connections received by listener
    delete iModel ;
    delete iNode ;
    delete iCallEngine ; 
    delete iTclWrapper ; 
}

void MockUpController::userInterfaceAction ( CAUserInterfaceRequest aRequest,
                                             const Hash& aHashConcerned,
                                             const Hash& aFetchFromNode,
                                             const QString* /*aAdditionalInformation*/ ) {
    LOG_STR2("MockUpController::userInterfaceAction %d\n", (int) aRequest) ;
}

void MockUpController::hideUI() {

}
void MockUpController::showUI() {

}


void MockUpController::exitApp() {

}

void MockUpController::displayAboutBox() {

}

void MockUpController::displayFront() {
    LOG_STR("displayFront\n") ;

}

void MockUpController::handleError(MController::CAErrorSituation aError,
                                   const QString& aExplanation) {
    LOG_STR2("Error enum value %d\n", aError) ;
    QLOG_STR(aExplanation) ;
    switch ( aError ) {
    case DataBaseNotMountable:
        LOG_STR("Can't open DB\n") ;
        break ;
    }
    return ;
}

Node& MockUpController::getNode() const {
    return *iNode ;
}

NetworkListener *MockUpController::networkListener() const {
    return iListener ;
}

Model& MockUpController::model() const {
    return *iModel ;
}


void MockUpController::setContentKeyPasswd(QString aPasswd) {
    iContentPasswd = aPasswd ;
}

QString MockUpController::contentKeyPasswd()  const {
    return iContentPasswd ;
}

void MockUpController::setProfileInUse(const Hash& aProfileHash) {
    iProfileHash = aProfileHash ;
}

const Hash& MockUpController::profileInUse() {
    return iProfileHash;
}


void MockUpController::startRetrievingContent(NetworkRequestExecutor::NetworkRequestQueueItem aReq,bool aIsBackgroundDl, ProtocolItemType aTypeOfExpectedObject) {
    return ;
}

void MockUpController::startRetrievingContent(CaDbRecord::SearchTerms /*aSearchTerms*/) {
    return ;
}

void MockUpController::storePrivateDataOfSelectedProfile(bool /*aPublishTrustListToo*/) {
    return ;
}

void MockUpController::reStorePrivateDataOfSelectedProfile() {
    return ;
}


bool MockUpController::isContactInContactList(const Hash& aFingerPrint) const {
    return true ;
}


QString MockUpController::displayableNameForProfile(const Hash& aProfileFingerPrint) const {
    return "eino leino reino" ;
}

void MockUpController::offerDisplayNameForProfile(const Hash& aProfileFingerPrint,
        const QString& aDisplayName,
        const bool iUpdatePersistenStorage) {
    return ;
}

void MockUpController::displayFileInfoOnUi(const BinaryFile& aFileMetadata) {
    QLOG_STR("displayFileInfoOnUi file = " + aFileMetadata.iFileName) ;
}

MVoiceCallEngine* MockUpController::voiceCallEngineInterface() {
    return iCallEngine ; 
}

TclWrapper& MockUpController::tclWrapper() {
    return *iTclWrapper ; 
}

QString MockUpController::getFileName(bool& aSuccess,
                                      bool /*aIsSaveFile*/ , 
                                      QString /*aSuggestedFileName*/) {
    aSuccess = true ; 
    char fileNameBuffer[L_tmpnam+1] ; // buffer for file name, macro
                                      // L_tmpnam should be in stdio.h telling
                                      // max len of returned name. 
    return QString(tmpnam_r(fileNameBuffer)) ; 
}
