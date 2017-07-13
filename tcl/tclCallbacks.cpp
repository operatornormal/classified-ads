/*     -*-C++-*- -*-coding: utf-8-unix;-*-
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
#include "tclCallbacks.h"
#include "../controller.h"
#include "../datamodel/model.h"
#include "../datamodel/searchmodel.h"
#include "../datamodel/profilemodel.h"
#include "../datamodel/profile.h"
#include "../datamodel/camodel.h"
#include "../datamodel/ca.h"
#include "../datamodel/profilecommentmodel.h"
#include "../datamodel/profilecomment.h"
#include "../datamodel/binaryfilemodel.h"
#include "../datamodel/binaryfile.h"
#include "../datamodel/contentencryptionmodel.h"
#include "../datamodel/cadbrecord.h"
#include "../datamodel/cadbrecordmodel.h"
#include "../datamodel/trusttreemodel.h"
#include "../datamodel/tclprogram.h"
#include "../datamodel/tclmodel.h"
#include "../tcl/tclWrapper.h"
#include "../ui/dialogbase.h"
#include "../log.h"
#include "tclUtil.h"
#include <QCoreApplication>
#include <QTextDocument>
#include "../FrontWidget.h"

const int KHashStringLen ( 40 ) ;
const char * KTCLFingerPrintProperty = "fingerPrint" ;
const char * KTCLFileNameProperty = "fileName" ;
const char * KTCLFingerPrintOfCommented = "fingerPrintOfCommented" ;
const char * KTCLFingerPrintOfSender = "fingerPrintOfSender" ;
const char * KTCLTimeOfPublish = "timeOfPublish" ;
const char * KTCLSenderName = "senderName" ;
const char * KTCLSubject = "subject" ;
const char * KTCLMessageText = "messageText";
const char * KTCLPlainMessageText = "plainMessageText" ;
const char * KTCLIsPrivate = "isPrivate" ;
const char * KTCLReplyTo = "replyTo" ;
const char * KTCLAttachedFiles = "attachedFiles" ;
const char * KTCLGroup = "group" ;
const char * KTCLSenderHash = "senderHash" ;
const char * KTCLAboutComboboxText = "aboutComboboxText" ;
const char * KTCLConcernsComboxboxText = "concernsComboboxText" ;
const char * KTCLInComboboxText = "inComboboxText" ;
const char * KTCLAboutComboBoxIndex = "aboutComboBoxIndex" ;
const char * KTCLInComboBoxIndex = "inComboBoxIndex" ;
const char * KTCLConcernsComboBoxIndex = "concernsComboBoxIndex" ;
const char * KTCLMimeType = "mimeType" ;
const char * KTCLDescription = "description" ;
const char * KTCLOwner = "owner" ;
const char * KTCLContentOwner = "contentOwner" ;
const char * KTCLLicense = "license" ;
const char * KTCLFileData = "fileData" ;
const char * KTCLDisplayName = "displayName" ;
const char * KTCLGreetingText = "greetingText" ;
const char * KTCLFirstName = "firstName" ;
const char * KTCLFamilyName = "familyName" ;
const char * KTCLCityCountry = "cityCountry" ;
const char * KTCLBTCAdress = "BTCAddress" ;
const char * KTCLStateOfTheWorld = "stateOfTheWorld" ;
const char * KTCLImagePNG = "imagePNG" ;
const char * KTCLProfileReaders = "profileReaders" ;
const char * KTCLSharedFiles = "sharedFiles" ;
const char * KTCLTrustList = "trustList" ;
const char * KTCLForceNoEncryption = "forceNoEncryption" ;
const char * KTCLFileRecipients = "fileRecipients" ;

const char * KTCLDbRecordId = "recordId" ;
const char * KTCLDbRecordCollectionId = "collectionId" ;
const char * KTCLDbRecordSenderId = "senderId" ;
const char * KTCLDbRecordSearchPhrase = "searchPhrase" ;
const char * KTCLDbRecordSearchNumber = "searchNumber" ;
const char * KTCLDbRecordSearchNumberLessThan = "searchNumberLessThan" ;
const char * KTCLDbRecordSearchNumberMoreThan = "searchNumberMoreThan" ;
const char * KTCLDbRecordData = "data" ;
const char * KTCLDbRecordIsSigVerified = "isSignatureVerified" ;
const char * KTCLDbRecordRecipients = "recordRecipients" ;
const char * KTCLDbRecordEncrypted = "encrypted" ;

extern const char *KTCLCommandPublishFile ;
extern const char *KTCLCommandPublishProfile ;
extern const char *KTCLCommandPublishProfileComment ;
extern const char *KTCLCommandPublishClassifiedAd ;
extern const char *KTCLCommandPublishDbRecord ;

TclCallbacks::TclCallbacks(Model& aModel,
                           MController& aController)
    : iModel(aModel),
      iController(aController) {
}

TclCallbacks::~TclCallbacks() {

}

// non-static method
int TclCallbacks::listItemsCmdImpl(ClientData /*aCData*/, Tcl_Interp *aInterp, int aObjc, Tcl_Obj *const aObjv[]) {
    Tcl_Obj* resultList ( Tcl_NewListObj(0, NULL) );
    int argumentStrLen (0) ;
    bool listAds ( false ) ;
    bool listProfiles ( false ) ;
    bool listComments ( false ) ;
    if(strncmp(Tcl_GetStringFromObj(aObjv[0], &argumentStrLen),
               "listProfiles",
               12) == 0 ) {
        listProfiles = true ;
    } else if(strncmp(Tcl_GetStringFromObj(aObjv[0], &argumentStrLen),
                      "listAds",
                      7) == 0 ) {
        listAds = true ;
    } else if(strncmp(Tcl_GetStringFromObj(aObjv[0], &argumentStrLen),
                      "listComments",
                      12) == 0 ) {
        listComments = true ;
    }
    // normally aObjc = 2, as first argument is command itself,
    // and 2nd argument is the search term
    if ( aObjc > 1 ) {

        char *argumentStr(Tcl_GetStringFromObj(aObjv[1], &argumentStrLen)) ;
        QLOG_STR("TclCallbacks::listItemsCmd in " + QString::number(aObjc)) ;

        if ( resultList && argumentStrLen > 0 ) {
            QString searchTerm ( QString::fromUtf8( argumentStr,argumentStrLen ) ) ;
            QLOG_STR("Search phrase >" + searchTerm + "<") ;
            iModel.lock() ;
            const QList<SearchModel::SearchResultItem>
            resultSet ( iModel.searchModel()->
                        performSearch(searchTerm,
                                      listAds,
                                      listProfiles,
                                      listComments) ) ;
            iModel.unlock() ;
            QLOG_STR("Search result count + " + QString::number(resultSet.size())) ;
            foreach ( const SearchModel::SearchResultItem& resultItem,
                      resultSet) {
                Tcl_Obj* resultAsTclObj =  Tcl_NewDictObj() ;
                int resultStrLen = strlen(resultItem.iDisplayName.toUtf8().constData()) ;
                int hashLen = strlen(resultItem.iItemHash.toString().toUtf8().constData()) ;
                Tcl_Obj* key = Tcl_NewStringObj(resultItem.iItemHash.toString().toUtf8().constData(), hashLen) ;
                Tcl_Obj* value = Tcl_NewStringObj(resultItem.iDisplayName.toUtf8().constData(), resultStrLen) ;
                Tcl_DictObjPut(aInterp,
                               resultAsTclObj,
                               key,
                               value) ;
                Tcl_ListObjAppendElement(aInterp,
                                         resultList,
                                         resultAsTclObj) ;
            }
        }
    }
    Tcl_SetObjResult(aInterp, resultList);
    return TCL_OK;
}

// non-static version of profile getter. profile is returned as tcl dictionary.
int TclCallbacks::getProfileCmdImpl(ClientData /* aCData */, Tcl_Interp *aInterp, int aObjc, Tcl_Obj *const aObjv[]) {
    int argumentStrLen (0) ;
    // normally aObjc = 2, as first argument is command itself,
    // and 2nd argument is the search term
    if ( aObjc > 1 ) {
        const unsigned char *argumentStr(
            reinterpret_cast<const unsigned char *>( Tcl_GetStringFromObj(aObjv[1], &argumentStrLen))) ;
        QLOG_STR("TclCallbacks::getProfileCmdImpl in " + QString::number(aObjc)) ;
        if ( argumentStrLen == KHashStringLen ) { // search term is hash and that is 40
            Hash searchTerm ;
            searchTerm.fromString( argumentStr ) ;
            if ( searchTerm != KNullHash ) {
                iModel.lock() ;
                Profile *result ( iModel.profileModel().
                                  profileByFingerPrint(searchTerm) ) ;
                iModel.unlock() ;
                if ( result ) {
                    Tcl_Obj* resultAsTclObj =  Tcl_NewDictObj() ;

                    Tcl_Obj* key = Tcl_NewStringObj(KTCLDisplayName,-1) ;
                    Tcl_Obj* value = Tcl_NewStringObj(result->displayName().toUtf8().constData(), -1) ;
                    Tcl_DictObjPut(aInterp,
                                   resultAsTclObj,
                                   key,
                                   value) ;

                    key = Tcl_NewStringObj(KTCLFingerPrintProperty,-1) ;
                    value = Tcl_NewStringObj(result->iFingerPrint.toString().toUtf8().constData(), -1) ;
                    Tcl_DictObjPut(aInterp,
                                   resultAsTclObj,
                                   key,
                                   value) ;

                    if ( result->iGreetingText.length() > 0 ) {
                        key = Tcl_NewStringObj(KTCLGreetingText,-1) ;
                        value = Tcl_NewStringObj(result->iGreetingText.toUtf8().constData(), -1) ;
                        Tcl_DictObjPut(aInterp,
                                       resultAsTclObj,
                                       key,
                                       value) ;
                    }
                    if ( result->iFirstName.length() > 0 ) {
                        key = Tcl_NewStringObj(KTCLFirstName,-1) ;
                        value = Tcl_NewStringObj(result->iFirstName.toUtf8().constData(), -1) ;
                        Tcl_DictObjPut(aInterp,
                                       resultAsTclObj,
                                       key,
                                       value) ;
                    }
                    if ( result->iFamilyName .length() > 0 ) {
                        key = Tcl_NewStringObj(KTCLFamilyName,-1) ;
                        value = Tcl_NewStringObj(result->iFamilyName .toUtf8().constData(), -1) ;
                        Tcl_DictObjPut(aInterp,
                                       resultAsTclObj,
                                       key,
                                       value) ;
                    }
                    if ( result->iCityCountry.length() > 0 ) {
                        key = Tcl_NewStringObj(KTCLCityCountry,-1) ;
                        value = Tcl_NewStringObj(result->iCityCountry.toUtf8().constData(), -1) ;
                        Tcl_DictObjPut(aInterp,
                                       resultAsTclObj,
                                       key,
                                       value) ;
                    }
                    if ( result->iBTCAddress.length() > 0 ) {
                        key = Tcl_NewStringObj(KTCLBTCAdress,-1) ;
                        value = Tcl_NewStringObj(result->iBTCAddress.toUtf8().constData(), -1) ;
                        Tcl_DictObjPut(aInterp,
                                       resultAsTclObj,
                                       key,
                                       value) ;
                    }
                    if ( result->iStateOfTheWorld.length() > 0 ) {
                        key = Tcl_NewStringObj(KTCLStateOfTheWorld,-1) ;
                        value = Tcl_NewStringObj(result->iStateOfTheWorld.toUtf8().constData(), -1) ;
                        Tcl_DictObjPut(aInterp,
                                       resultAsTclObj,
                                       key,
                                       value) ;
                    }

                    key = Tcl_NewStringObj(KTCLTimeOfPublish,-1) ;
                    value = Tcl_NewLongObj(result->iTimeOfPublish) ;
                    Tcl_DictObjPut(aInterp,
                                   resultAsTclObj,
                                   key,
                                   value) ;

                    key = Tcl_NewStringObj(KTCLIsPrivate,-1) ;
                    value = Tcl_NewBooleanObj(result->iIsPrivate == true ? 1 : 0 ) ;
                    Tcl_DictObjPut(aInterp,
                                   resultAsTclObj,
                                   key,
                                   value) ;
                    if ( result->iProfilePicture.isNull() == false ) {
                        // there is image data
                        QByteArray imageData ;
                        QBuffer memoryBuffer ( &imageData );
                        memoryBuffer.open(QBuffer::WriteOnly) ;
                        result->iProfilePicture.save(&memoryBuffer, "PNG") ;
                        if ( imageData.size() > 0 ) {
                            key = Tcl_NewStringObj(KTCLImagePNG,-1) ;
                            value = Tcl_NewByteArrayObj(
                                        reinterpret_cast<const unsigned char *>(imageData.constData()),
                                        imageData.size() ) ;
                            Tcl_DictObjPut(aInterp,
                                           resultAsTclObj,
                                           key,
                                           value) ;
                        }
                    }

                    if ( result->iProfileReaders.size() > 0 ) {
                        Tcl_Obj* profileReaders ( Tcl_NewListObj(0, NULL) );
                        foreach ( const Hash& reader,
                                  result->iProfileReaders ) {
                            Tcl_ListObjAppendElement(aInterp,
                                                     profileReaders,
                                                     Tcl_NewStringObj(reader.toString().toUtf8().constData(), -1)) ;
                        }
                        key = Tcl_NewStringObj(KTCLProfileReaders,-1) ;
                        Tcl_DictObjPut(aInterp,
                                       resultAsTclObj,
                                       key,
                                       profileReaders) ;
                    }
                    if ( result->iSharedFiles.size() > 0 ) {
                        Tcl_Obj* sharedFiles ( Tcl_NewListObj(0, NULL) );
                        foreach ( const Hash& sharedFile,
                                  result->iSharedFiles ) {
                            Tcl_ListObjAppendElement(aInterp,
                                                     sharedFiles,
                                                     Tcl_NewStringObj(sharedFile.toString().toUtf8().constData(), -1)) ;
                        }
                        key = Tcl_NewStringObj(KTCLSharedFiles,-1) ;
                        Tcl_DictObjPut(aInterp,
                                       resultAsTclObj,
                                       key,
                                       sharedFiles) ;
                    }
                    if ( result->iTrustList.size() > 0 ) {
                        Tcl_Obj* trustList ( Tcl_NewListObj(0, NULL) );
                        foreach ( const Hash& trustee,
                                  result->iTrustList ) {
                            Tcl_ListObjAppendElement(aInterp,
                                                     trustList,
                                                     Tcl_NewStringObj(trustee.toString().toUtf8().constData(), -1)) ;
                        }
                        key = Tcl_NewStringObj(KTCLTrustList,-1) ;
                        Tcl_DictObjPut(aInterp,
                                       resultAsTclObj,
                                       key,
                                       trustList) ;
                    }
                    Tcl_SetObjResult(aInterp, resultAsTclObj);
                    delete result ;
                    return TCL_OK ;
                } else {
                    Tcl_AppendResult(aInterp, "Profile not found", NULL);
                }
            } else {
                Tcl_AppendResult(aInterp, "Invalid 40-character input, not parseable as hash", NULL);
            }
        } else {
            Tcl_AppendResult(aInterp, "Argument string length not 40", NULL);
        }
    } else {
        Tcl_AppendResult(aInterp, "Usage: getProfile fingerprint", NULL);
    }
    return TCL_ERROR ;
}

// non-static version of ad getter
int TclCallbacks::getClassifiedAdCmdImpl(ClientData /* aCData */, Tcl_Interp *aInterp, int aObjc, Tcl_Obj *const aObjv[]) {
    // normally aObjc = 2, as first argument is command itself,
    // and 2nd argument is the search term
    if ( aObjc > 1 ) {
        int argumentStrLen(0) ;
        const unsigned char *argumentStr(
            reinterpret_cast<const unsigned char *>( Tcl_GetStringFromObj(aObjv[1], &argumentStrLen))) ;

        QLOG_STR("TclCallbacks::getClassifiedAdCmdImpl in " + QString::number(aObjc)) ;
        if ( argumentStrLen == KHashStringLen ) { // search term is hash and that is 40
            Hash searchTerm ;
            searchTerm.fromString( argumentStr ) ;
            if ( searchTerm != KNullHash ) {
                iModel.lock() ;
                CA result ( iModel.classifiedAdsModel().
                            caByHash(searchTerm) ) ;
                iModel.unlock() ;
                if ( result.iFingerPrint != KNullHash ) {
                    Tcl_Obj* resultAsTclObj =  Tcl_NewDictObj() ;

                    Tcl_Obj* key = Tcl_NewStringObj(KTCLDisplayName,-1) ;
                    Tcl_Obj* value = Tcl_NewStringObj(result.displayName().toUtf8().constData(), -1) ;
                    Tcl_DictObjPut(aInterp,
                                   resultAsTclObj,
                                   key,
                                   value) ;

                    key = Tcl_NewStringObj(KTCLFingerPrintProperty,-1) ;
                    value = Tcl_NewStringObj(result.iFingerPrint.toString().toUtf8().constData(), -1) ;
                    Tcl_DictObjPut(aInterp,
                                   resultAsTclObj,
                                   key,
                                   value) ;

                    key = Tcl_NewStringObj(KTCLTimeOfPublish,-1) ;
                    value = Tcl_NewLongObj(result.iTimeOfPublish) ;
                    Tcl_DictObjPut(aInterp,
                                   resultAsTclObj,
                                   key,
                                   value) ;

                    if ( result.iSenderName.length() > 0 ) {
                        key = Tcl_NewStringObj(KTCLSenderName,-1) ;
                        value = Tcl_NewStringObj(result.iSenderName.toUtf8().constData(), -1) ;
                        Tcl_DictObjPut(aInterp,
                                       resultAsTclObj,
                                       key,
                                       value) ;
                    }
                    if ( result.iSubject.length() > 0 ) {
                        key = Tcl_NewStringObj(KTCLSubject,-1) ;
                        value = Tcl_NewStringObj(result.iSubject.toUtf8().constData(), -1) ;
                        Tcl_DictObjPut(aInterp,
                                       resultAsTclObj,
                                       key,
                                       value) ;
                    }
                    if ( result.iGroup.length() > 0 ) {
                        key = Tcl_NewStringObj(KTCLGroup,-1) ;
                        value = Tcl_NewStringObj(result.iGroup.toUtf8().constData(), -1) ;
                        Tcl_DictObjPut(aInterp,
                                       resultAsTclObj,
                                       key,
                                       value) ;
                    }
                    if ( result.iMessageText.length() > 0 ) {
                        key = Tcl_NewStringObj(KTCLMessageText,-1) ;
                        value = Tcl_NewStringObj(result.iMessageText.toUtf8().constData(), -1) ;
                        Tcl_DictObjPut(aInterp,
                                       resultAsTclObj,
                                       key,
                                       value) ;
                        QTextDocument formatter ;
                        formatter.setHtml(result.iMessageText) ;
                        if ( formatter.isEmpty() == false ) {
                            key = Tcl_NewStringObj(KTCLPlainMessageText,-1) ;
                            value = Tcl_NewStringObj(formatter.toPlainText().toUtf8().constData(), -1) ;
                            Tcl_DictObjPut(aInterp,
                                           resultAsTclObj,
                                           key,
                                           value) ;
                        }
                    }
                    if ( result.iAboutComboBoxText.length() > 0 ) {
                        key = Tcl_NewStringObj(KTCLAboutComboboxText,-1) ;
                        value = Tcl_NewStringObj(result.iAboutComboBoxText.toUtf8().constData(), -1) ;
                        Tcl_DictObjPut(aInterp,
                                       resultAsTclObj,
                                       key,
                                       value) ;
                    }
                    if ( result.iConcernsComboBoxText.length() > 0 ) {
                        key = Tcl_NewStringObj(KTCLConcernsComboxboxText,-1) ;
                        value = Tcl_NewStringObj(result.iConcernsComboBoxText.toUtf8().constData(), -1) ;
                        Tcl_DictObjPut(aInterp,
                                       resultAsTclObj,
                                       key,
                                       value) ;
                    }
                    if ( result.iInComboBoxText.length() > 0 ) {
                        key = Tcl_NewStringObj(KTCLInComboboxText,-1) ;
                        value = Tcl_NewStringObj(result.iInComboBoxText.toUtf8().constData(), -1) ;
                        Tcl_DictObjPut(aInterp,
                                       resultAsTclObj,
                                       key,
                                       value) ;
                    }

                    key = Tcl_NewStringObj(KTCLAboutComboBoxIndex,-1) ;
                    value = Tcl_NewIntObj(result.iAboutComboBoxIndex) ;
                    Tcl_DictObjPut(aInterp,
                                   resultAsTclObj,
                                   key,
                                   value) ;

                    key = Tcl_NewStringObj(KTCLInComboBoxIndex,-1) ;
                    value = Tcl_NewIntObj(result.iInComboBoxIndex) ;
                    Tcl_DictObjPut(aInterp,
                                   resultAsTclObj,
                                   key,
                                   value) ;

                    key = Tcl_NewStringObj(KTCLConcernsComboBoxIndex,-1) ;
                    value = Tcl_NewIntObj(result.iConcernsComboBoxIndex) ;
                    Tcl_DictObjPut(aInterp,
                                   resultAsTclObj,
                                   key,
                                   value) ;

                    if ( result.iSenderHash != KNullHash ) {
                        key = Tcl_NewStringObj(KTCLSenderHash,-1) ;
                        value = Tcl_NewStringObj(result.iSenderHash.toString().toUtf8().constData(), -1) ;
                        Tcl_DictObjPut(aInterp,
                                       resultAsTclObj,
                                       key,
                                       value) ;
                    }

                    if ( result.iReplyTo != KNullHash ) {
                        key = Tcl_NewStringObj(KTCLReplyTo,-1) ;
                        value = Tcl_NewStringObj(result.iReplyTo.toString().toUtf8().constData(), -1) ;
                        Tcl_DictObjPut(aInterp,
                                       resultAsTclObj,
                                       key,
                                       value) ;
                    }
                    if ( result.iAttachedFiles.size() > 0 ) {
                        Tcl_Obj* attachedFiles (NULL) ;
                        if ( ( attachedFiles = Tcl_NewListObj(0, NULL) )  != NULL ) {
                            foreach ( const Hash& attachedFile,
                                      result.iAttachedFiles ) {
                                Tcl_ListObjAppendElement(aInterp,
                                                         attachedFiles,
                                                         Tcl_NewStringObj(attachedFile.toString().toUtf8().constData(), -1)) ;
                            }
                            key = Tcl_NewStringObj(KTCLAttachedFiles,-1) ;
                            Tcl_DictObjPut(aInterp,
                                           resultAsTclObj,
                                           key,
                                           attachedFiles) ;
                        }
                    }

                    Tcl_SetObjResult(aInterp, resultAsTclObj);
                    return TCL_OK ;
                } else {
                    Tcl_AppendResult(aInterp, "Classified ad not found", NULL);
                }
            } else {
                Tcl_AppendResult(aInterp, "Invalid 40-character input, not parseable as hash", NULL);
            }
        } else {
            Tcl_AppendResult(aInterp, "Argument string length not 40", NULL);
        }
    } else {
        Tcl_AppendResult(aInterp, "Usage: getClassifiedAd fingerprint", NULL);
    }
    return TCL_ERROR ;
}


// non-static profile-getter
int TclCallbacks::getProfileCommentCmdImpl(ClientData /* aCData */, Tcl_Interp *aInterp, int aObjc, Tcl_Obj *const aObjv[]) {
    int argumentStrLen (0) ;
    // normally aObjc = 2, as first argument is command itself,
    // and 2nd argument is the search term
    if ( aObjc > 1 ) {
        const unsigned char *argumentStr(
            reinterpret_cast<const unsigned char *>( Tcl_GetStringFromObj(aObjv[1], &argumentStrLen))) ;
        QLOG_STR("TclCallbacks::getProfileCommentCmdImpl in " + QString::number(aObjc)) ;
        if ( argumentStrLen == KHashStringLen ) { // search term is hash and that is 40
            Hash searchTerm ;
            searchTerm.fromString( argumentStr ) ;
            if ( searchTerm != KNullHash ) {
                iModel.lock() ;
                ProfileComment* result ( iModel.profileCommentModel().
                                         profileCommentByFingerPrint(searchTerm) ) ;
                iModel.unlock() ;
                if ( result ) {
                    Tcl_Obj* resultAsTclObj =  Tcl_NewDictObj() ;

                    Tcl_Obj* key = Tcl_NewStringObj(KTCLFingerPrintProperty,-1) ;
                    Tcl_Obj* value = Tcl_NewStringObj(result->iFingerPrint.toString().toUtf8().constData(), -1) ;
                    Tcl_DictObjPut(aInterp,
                                   resultAsTclObj,
                                   key,
                                   value) ;

                    key = Tcl_NewStringObj(KTCLFingerPrintOfCommented,-1) ;
                    value = Tcl_NewStringObj(result->iProfileFingerPrint.toString().toUtf8().constData(), -1) ;
                    Tcl_DictObjPut(aInterp,
                                   resultAsTclObj,
                                   key,
                                   value) ;
                    key = Tcl_NewStringObj(KTCLFingerPrintOfSender,-1) ;
                    value = Tcl_NewStringObj(result->iCommentorHash.toString().toUtf8().constData(), -1) ;
                    Tcl_DictObjPut(aInterp,
                                   resultAsTclObj,
                                   key,
                                   value) ;

                    key = Tcl_NewStringObj(KTCLTimeOfPublish,-1) ;
                    value = Tcl_NewLongObj(result->iTimeOfPublish) ;
                    Tcl_DictObjPut(aInterp,
                                   resultAsTclObj,
                                   key,
                                   value) ;

                    if ( result->iCommentorNickName.length() > 0 ) {
                        key = Tcl_NewStringObj(KTCLSenderName,-1) ;
                        value = Tcl_NewStringObj(result->iCommentorNickName.toUtf8().constData(), -1) ;
                        Tcl_DictObjPut(aInterp,
                                       resultAsTclObj,
                                       key,
                                       value) ;
                    }
                    if ( result->iSubject.length() > 0 ) {
                        key = Tcl_NewStringObj(KTCLSubject,-1) ;
                        value = Tcl_NewStringObj(result->iSubject.toUtf8().constData(), -1) ;
                        Tcl_DictObjPut(aInterp,
                                       resultAsTclObj,
                                       key,
                                       value) ;
                    }

                    if ( result->iCommentText.length() > 0 ) {
                        key = Tcl_NewStringObj(KTCLMessageText,-1) ;
                        value = Tcl_NewStringObj(result->iCommentText.toUtf8().constData(), -1) ;
                        Tcl_DictObjPut(aInterp,
                                       resultAsTclObj,
                                       key,
                                       value) ;
                        QTextDocument formatter ;
                        formatter.setHtml(result->iCommentText) ;
                        if ( formatter.isEmpty() == false ) {
                            key = Tcl_NewStringObj(KTCLPlainMessageText,-1) ;
                            value = Tcl_NewStringObj(formatter.toPlainText().toUtf8().constData(), -1) ;
                            Tcl_DictObjPut(aInterp,
                                           resultAsTclObj,
                                           key,
                                           value) ;
                        }
                    }

                    key = Tcl_NewStringObj(KTCLIsPrivate,-1) ;
                    value = Tcl_NewBooleanObj(result->iIsPrivate ? 1 : 0 ) ;
                    Tcl_DictObjPut(aInterp,
                                   resultAsTclObj,
                                   key,
                                   value) ;


                    if ( result->iReferences != KNullHash ) {
                        key = Tcl_NewStringObj(KTCLReplyTo,-1) ;
                        value = Tcl_NewStringObj(result->iReferences.toString().toUtf8().constData(), -1) ;
                        Tcl_DictObjPut(aInterp,
                                       resultAsTclObj,
                                       key,
                                       value) ;
                    }
                    if ( result->iAttachedFiles.size() > 0 ) {
                        Tcl_Obj* attachedFiles (NULL) ;
                        if ( ( attachedFiles = Tcl_NewListObj(0, NULL) )  != NULL ) {
                            foreach ( const Hash& attachedFile,
                                      result->iAttachedFiles ) {
                                Tcl_ListObjAppendElement(aInterp,
                                                         attachedFiles,
                                                         Tcl_NewStringObj(attachedFile.toString().toUtf8().constData(), -1)) ;
                            }
                            key = Tcl_NewStringObj(KTCLAttachedFiles,-1) ;
                            Tcl_DictObjPut(aInterp,
                                           resultAsTclObj,
                                           key,
                                           attachedFiles) ;
                        }
                    }

                    Tcl_SetObjResult(aInterp, resultAsTclObj);
                    return TCL_OK ;
                } else {
                    Tcl_AppendResult(aInterp, "Profile comment not found", NULL);
                }
            } else {
                Tcl_AppendResult(aInterp, "Invalid 40-character input, not parseable as hash", NULL);
            }
        } else {
            Tcl_AppendResult(aInterp, "Argument string length not 40", NULL);
        }
    } else {
        Tcl_AppendResult(aInterp, "Usage: getProfileComment fingerprint", NULL);
    }
    return TCL_ERROR ;
}


// non-static blob-getter
int TclCallbacks::getBinaryFileCmdImpl(ClientData /* aCData */, Tcl_Interp *aInterp, int aObjc, Tcl_Obj *const aObjv[]) {
    int argumentStrLen (0) ;
    // normally aObjc = 2, as first argument is command itself,
    // and 2nd argument is the search term
    if ( aObjc > 1 ) {
        const unsigned char *argumentStr(
            reinterpret_cast<const unsigned char *>( Tcl_GetStringFromObj(aObjv[1], &argumentStrLen))) ;
        QLOG_STR("TclCallbacks::getBinaryFileCmdImpl in " + QString::number(aObjc)) ;
        if ( argumentStrLen == KHashStringLen ) { // search term is hash and that is 40
            Hash searchTerm ;
            searchTerm.fromString( argumentStr ) ;
            if ( searchTerm != KNullHash ) {
                iModel.lock() ;
                BinaryFile* result ( iModel.binaryFileModel().
                                     binaryFileByFingerPrint(searchTerm) ) ;
                iModel.unlock() ;
                if ( result ) {
                    QByteArray fileData ;
                    QByteArray fileSignature ;
                    bool isEncrypted ;
                    Hash fileOwnerFingerPrint ;
                    fileOwnerFingerPrint.fromString((const unsigned char *)(result->iOwner.toUtf8().constData())) ;
                    iModel.lock() ;
                    bool fileContentFound = iModel.binaryFileModel().
                                            binaryFileDataByFingerPrint(searchTerm,
                                                    fileOwnerFingerPrint,
                                                    fileData,
                                                    fileSignature,
                                                    &isEncrypted ) ;
                    iModel.unlock() ;
                    if ( fileContentFound ) {

                        Tcl_Obj* resultAsTclObj = Tcl_NewDictObj() ;

                        Tcl_Obj* key = Tcl_NewStringObj(KTCLFingerPrintProperty,-1) ;
                        Tcl_Obj* value = Tcl_NewStringObj(result->iFingerPrint.toString().toUtf8().constData(), -1) ;
                        Tcl_DictObjPut(aInterp,
                                       resultAsTclObj,
                                       key,
                                       value) ;

                        key = Tcl_NewStringObj(KTCLTimeOfPublish,-1) ;
                        value = Tcl_NewLongObj(result->iTimeOfPublish) ;
                        Tcl_DictObjPut(aInterp,
                                       resultAsTclObj,
                                       key,
                                       value) ;

                        if ( result->iMimeType.length() > 0 ) {
                            key = Tcl_NewStringObj(KTCLMimeType,-1) ;
                            value = Tcl_NewStringObj(result->iMimeType.toUtf8().constData(), -1) ;
                            Tcl_DictObjPut(aInterp,
                                           resultAsTclObj,
                                           key,
                                           value) ;
                        }
                        if ( result->iDescription.length() > 0 ) {
                            key = Tcl_NewStringObj(KTCLDescription,-1) ;
                            value = Tcl_NewStringObj(result->iDescription.toUtf8().constData(), -1) ;
                            Tcl_DictObjPut(aInterp,
                                           resultAsTclObj,
                                           key,
                                           value) ;
                        }
                        if ( result->iOwner.length() > 0 ) {
                            key = Tcl_NewStringObj(KTCLOwner,-1) ;
                            value = Tcl_NewStringObj(result->iOwner.toUtf8().constData(), -1) ;
                            Tcl_DictObjPut(aInterp,
                                           resultAsTclObj,
                                           key,
                                           value) ;
                        }
                        if ( result->iContentOwner.length() > 0 ) {
                            key = Tcl_NewStringObj(KTCLContentOwner,-1) ;
                            value = Tcl_NewStringObj(result->iContentOwner.toUtf8().constData(), -1) ;
                            Tcl_DictObjPut(aInterp,
                                           resultAsTclObj,
                                           key,
                                           value) ;
                        }
                        if ( result->iLicense.length() > 0 ) {
                            key = Tcl_NewStringObj(KTCLLicense,-1) ;
                            value = Tcl_NewStringObj(result->iLicense.toUtf8().constData(), -1) ;
                            Tcl_DictObjPut(aInterp,
                                           resultAsTclObj,
                                           key,
                                           value) ;
                        }
                        if ( result->iFileName.length() > 0 ) {
                            key = Tcl_NewStringObj(KTCLFileNameProperty,-1) ;
                            value = Tcl_NewStringObj(result->iFileName.toUtf8().constData(), -1) ;
                            Tcl_DictObjPut(aInterp,
                                           resultAsTclObj,
                                           key,
                                           value) ;
                        }

                        key = Tcl_NewStringObj(KTCLFileData,-1) ;
                        value = Tcl_NewByteArrayObj(
                                    reinterpret_cast<const unsigned char *>(fileData.constData()),
                                    fileData.size() ) ;
                        Tcl_DictObjPut(aInterp,
                                       resultAsTclObj,
                                       key,
                                       value) ;

                        Tcl_SetObjResult(aInterp, resultAsTclObj);
                        delete result ;

                        return TCL_OK ;

                    } else {
                        Tcl_AppendResult(aInterp, "Binary file data not found", NULL);
                        delete result ;
                    }


                } else {
                    Tcl_AppendResult(aInterp, "Binary file not found", NULL);
                }
            } else {
                Tcl_AppendResult(aInterp, "Invalid 40-character input, not parseable as hash", NULL);
            }
        } else {
            Tcl_AppendResult(aInterp, "Argument string length not 40", NULL);
        }
    } else {
        Tcl_AppendResult(aInterp, "Usage: getBinaryFile fingerprint", NULL);
    }
    return TCL_ERROR ;
}


int TclCallbacks::publishItemCmdImpl(ClientData  aCData , Tcl_Interp *aInterp, int aObjc, Tcl_Obj* const aObjv[]) {
    if ( aObjc > 0 ) {
        if ( iController.profileInUse() == KNullHash ) {
            Tcl_AppendResult(aInterp, "No current profile, can't publish", NULL);
            return TCL_ERROR ;
        }
        int cmdLen ( 0 ) ;
        const char *commandName = Tcl_GetStringFromObj(aObjv[0], &cmdLen) ;
        if ( cmdLen > 0 && strncmp(commandName,
                                   KTCLCommandPublishFile,
                                   strlen(KTCLCommandPublishFile)) == 0 ) {
            return publishFileCmdImpl( aCData , aInterp, aObjc, aObjv) ;
        } else if ( cmdLen > 0 &&
                    strncmp(commandName,
                            KTCLCommandPublishProfileComment,
                            strlen(KTCLCommandPublishProfileComment)) == 0 )  {
            return publishProfileCommentCmdImpl( aCData , aInterp, aObjc, aObjv) ;
        } else if ( cmdLen > 0 &&
                    strncmp(commandName,
                            KTCLCommandPublishProfile,
                            strlen(KTCLCommandPublishProfile)) == 0 )  {
            return publishProfileCmdImpl( aCData , aInterp, aObjc, aObjv) ;
        } else if ( cmdLen > 0 &&
                    strncmp(commandName,
                            KTCLCommandPublishClassifiedAd,
                            strlen(KTCLCommandPublishClassifiedAd)) == 0 )  {
            return publishClassifiedAdCmdImpl( aCData , aInterp, aObjc, aObjv) ;
        } else if ( cmdLen > 0 &&
                    strncmp(commandName,
                            KTCLCommandPublishDbRecord,
                            strlen(KTCLCommandPublishDbRecord)) == 0 )  {
            return publishDbRecordCmdImpl( aCData , aInterp, aObjc, aObjv) ;
        } else {
            Tcl_AppendResult(aInterp, "unknown command", NULL);
            return TCL_ERROR ;
        }
    }
    return TCL_ERROR ;
}


int TclCallbacks::publishProfileCommentCmdImpl(ClientData /* aCData */, Tcl_Interp *aInterp, int aObjc, Tcl_Obj *const aObjv[]) {
    if ( aObjc > 1 ) {

        Tcl_DictSearch searchPtr  ;

        int done(false) ;
        Tcl_Obj* key (NULL) ;
        Tcl_Obj* value (NULL) ;

        if (Tcl_DictObjFirst(aInterp, aObjv[1], &searchPtr,
                             &key, &value, &done) != TCL_OK) {
            return TCL_ERROR;
        } else {
            ProfileComment pc ;
            do {
                // both key and value have been set
                if ( key && value ) {
                    int argumentStrLen (0) ;
                    QString keyStr ( Tcl_GetStringFromObj(key, &argumentStrLen) ) ;
                    QLOG_STR("Found key at publishProfileCommentCmdImpl " + keyStr) ;
                    if ( keyStr == KTCLFingerPrintOfCommented ) {
                        pc.iProfileFingerPrint.fromString (
                            reinterpret_cast<const unsigned char *>(Tcl_GetStringFromObj(value,&argumentStrLen)) ) ;
                    } else if ( keyStr == KTCLSenderName ) {
                        pc.iCommentorNickName = QString::fromUtf8 (
                                                    reinterpret_cast<const char *>(Tcl_GetStringFromObj(value,&argumentStrLen)) ) ;
                    } else if ( keyStr == KTCLSubject ) {
                        pc.iSubject = QString::fromUtf8 (
                                          reinterpret_cast<const char *>(Tcl_GetStringFromObj(value,&argumentStrLen)) ) ;
                    } else if ( keyStr == KTCLMessageText ) {
                        pc.iCommentText = QString::fromUtf8 (
                                              reinterpret_cast<const char *>(Tcl_GetStringFromObj(value,&argumentStrLen)) ) ;
                    } else if ( keyStr == KTCLPlainMessageText ) {
                        pc.iCommentText = "<html><body>" + QString::fromUtf8 (
                                              reinterpret_cast<const char *>(Tcl_GetStringFromObj(value,&argumentStrLen)) ) + "</body></html>" ;
                    } else if ( keyStr == KTCLIsPrivate ) {
                        int intFromObj ( 0 ) ;
                        if ( Tcl_GetIntFromObj(aInterp, value, &intFromObj) == TCL_OK ) {
                            pc.iIsPrivate = intFromObj == 0 ? false : true ;
                        }
                    } else if ( keyStr == KTCLReplyTo) {
                        pc.iReferences.fromString (
                            reinterpret_cast<const unsigned char *>(Tcl_GetStringFromObj(value,&argumentStrLen)) ) ;
                    } else if ( keyStr == KTCLAttachedFiles ) {
                        // in case of attached files the object in the dictionary
                        // is a list
                        int listLen ( 0 ) ;
                        if ( Tcl_ListObjLength(aInterp,value,&listLen) == TCL_OK ) {
                            for ( int i = 0 ; i < listLen ; i++ ) {
                                Tcl_Obj *objPtrPtr ;
                                if ( Tcl_ListObjIndex(aInterp, value, i, &objPtrPtr ) == TCL_OK ) {
                                    Hash attachedFileHash ;
                                    attachedFileHash.fromString (
                                        reinterpret_cast<const unsigned char *>(Tcl_GetStringFromObj(objPtrPtr,&argumentStrLen)) ) ;
                                    if ( attachedFileHash != KNullHash ) {
                                        pc.iAttachedFiles.append ( attachedFileHash ) ;
                                    }
                                } else {
                                    return TCL_ERROR ;
                                }
                            }
                        } else {
                            return TCL_ERROR ;
                        }
                    }
                }
                Tcl_DictObjNext(&searchPtr, &key, &value, &done) ;
            } while ( !done ) ;

            if ( pc.iCommentText.length() == 0 &&
                    pc.iSubject.length() == 0 ) {
                Tcl_AppendResult(aInterp, "Either comment text or subject must be given", NULL);
                return TCL_ERROR ;
            }
            if ( pc.iProfileFingerPrint == KNullHash ) {
                Tcl_AppendResult(aInterp, "Hash of commented profile must be given", NULL);
                return TCL_ERROR ;
            }
            pc.iTimeOfPublish = QDateTime::currentDateTimeUtc().toTime_t() ;
            pc.iCommentorHash = iController.profileInUse() ;

            quint32 dummy ;
            iController.model().lock() ;
            if (pc.iCommentorHash != KNullHash &&
                    iController.model()
                    .contentEncryptionModel()
                    .PublicKey(pc.iCommentorHash,
                               pc.iKeyOfCommentor,
                               &dummy ) &&
                    iController.model().profileCommentModel()
                    .publishProfileComment(pc) ) {
                iController.model().unlock() ;
                Tcl_AppendResult(aInterp, pc.iFingerPrint.toString().toUtf8().constData(), NULL);
                return TCL_OK ;
            } else {
                iController.model().unlock() ;
                Tcl_AppendResult(aInterp, "Publish failed", NULL);
                return TCL_ERROR ;
            }
        }
        return TCL_ERROR ;
    } else {
        Tcl_AppendResult(aInterp, "Usage: publishProfileComment profile ( as dictionary )", NULL);
    }
    return TCL_ERROR ;
}

int TclCallbacks::publishProfileCmdImpl(ClientData /* aCData */, Tcl_Interp *aInterp, int aObjc, Tcl_Obj *const aObjv[]) {
    if ( aObjc > 1 ) {

        Tcl_DictSearch searchPtr  ;

        int done(false) ;
        Tcl_Obj* key (NULL) ;
        Tcl_Obj* value (NULL) ;

        if (Tcl_DictObjFirst(aInterp, aObjv[1], &searchPtr,
                             &key, &value, &done) != TCL_OK) {
            return TCL_ERROR;
        } else {
            // both key and value have been set
            if ( key && value ) {

            }
        }
        for (; !done ; Tcl_DictObjNext(&searchPtr, &key, &value, &done)) {
            QLOG_STR("Found key at publishProfileCmdImpl " + QString(Tcl_GetString(key))) ;
        }
        return TCL_OK ;
    } else {
        Tcl_AppendResult(aInterp, "Usage: publishProfile profile ( as dictionary )", NULL);
    }
    return TCL_ERROR ;
}

int TclCallbacks::publishFileCmdImpl(ClientData /* aCData */, Tcl_Interp *aInterp, int aObjc, Tcl_Obj *const aObjv[]) {

    if ( aObjc > 1 ) {
        Tcl_DictSearch searchPtr  ;
        int done(false) ;
        Tcl_Obj* key (NULL) ;
        Tcl_Obj* value (NULL) ;

        if (Tcl_DictObjFirst(aInterp, aObjv[1], &searchPtr,
                             &key, &value, &done) != TCL_OK) {
            return TCL_ERROR;
        } else {
            QByteArray fileContent ;
            QString mimeType ;  /**< what kind of data */
            QString description ; /**< what is inside */
            QString owner ; /**< fingerprint of publisher */
            QString contentOwner ; /**< if someone owns the content, name or SHA1 fp */
            QString license ; /**< restriction in usage;PD or GPL or C-C or anything? */
            QString fileName ; /**< name of the file-system file */
            QList<Hash> fileRecipientList ;
            bool isEncrypted (false);
            do {
                // both key and value have been set
                if ( key && value ) {
                    int argumentStrLen (0) ;
                    QString keyStr ( Tcl_GetStringFromObj(key, &argumentStrLen) ) ;
                    QLOG_STR("Found key at publishFileCmdImpl " + keyStr) ;
                    if ( keyStr == KTCLFileNameProperty ) {
                        fileName = QString::fromUtf8 (
                                       reinterpret_cast<const char *>(Tcl_GetStringFromObj(value,&argumentStrLen)) ) ;
                    } else if ( keyStr == KTCLDescription ) {
                        description = QString::fromUtf8 (
                                          reinterpret_cast<const char *>(Tcl_GetStringFromObj(value,&argumentStrLen)) ) ;
                    } else if ( keyStr == KTCLMimeType ) {
                        mimeType = QString::fromUtf8 (
                                       reinterpret_cast<const char *>(Tcl_GetStringFromObj(value,&argumentStrLen)) ) ;
                    }  else if ( keyStr == KTCLContentOwner ) {
                        contentOwner =  QString::fromUtf8 (
                                            reinterpret_cast<const char *>(Tcl_GetStringFromObj(value,&argumentStrLen)) ) ;
                    } else if ( keyStr == KTCLLicense ) {
                        license =  QString::fromUtf8 (
                                       reinterpret_cast<const char *>(Tcl_GetStringFromObj(value,&argumentStrLen)) ) ;
                    } else if ( keyStr == KTCLForceNoEncryption ) {
                        int intFromObj ( 0 ) ;
                        if ( Tcl_GetIntFromObj(aInterp, value, &intFromObj) == TCL_OK ) {
                            isEncrypted = intFromObj == 0 ? false : true ;
                        }
                    } else if ( keyStr == KTCLFileRecipients ) {
                        // in case of recipient list the object in the dictionary
                        // is a list
                        int listLen ( 0 ) ;
                        if ( Tcl_ListObjLength(aInterp,value,&listLen) == TCL_OK ) {
                            for ( int i = 0 ; i < listLen ; i++ ) {
                                Tcl_Obj *objPtrPtr ;
                                if ( Tcl_ListObjIndex(aInterp, value, i, &objPtrPtr ) == TCL_OK ) {
                                    Hash recipientHash ;
                                    recipientHash.fromString (
                                        reinterpret_cast<const unsigned char *>(Tcl_GetStringFromObj(objPtrPtr,&argumentStrLen)) ) ;
                                    if ( recipientHash != KNullHash ) {
                                        fileRecipientList.append ( recipientHash ) ;
                                        QLOG_STR("File recipient " + recipientHash.toString()) ;
                                    }
                                } else {
                                    return TCL_ERROR ;
                                }
                            }
                        } else {
                            return TCL_ERROR ;
                        }
                    } else if ( keyStr == KTCLFileData ) {
                        unsigned char* bytes (NULL);
                        int length ;
                        if ( ( bytes = Tcl_GetByteArrayFromObj(value, &length) ) != NULL &&
                                length > 0 ) {
                            fileContent.append(reinterpret_cast<const char *>(bytes), length) ;
                        } else {
                            Tcl_AppendResult(aInterp, "Could not get file content?", NULL);
                            return TCL_ERROR ;
                        }
                    }
                }
                Tcl_DictObjNext(&searchPtr, &key, &value, &done) ;
            } while ( !done ) ;

            if ( fileContent.length() == 0  ) {
                Tcl_AppendResult(aInterp, "Empty file not permitted", NULL);
                return TCL_ERROR ;
            }

#if QT_VERSION >= 0x050000
            if ( mimeType.length() == 0 ) {
                QMimeDatabase db ;
                QMimeType detectedType ( db.mimeTypeForData(fileContent) ) ;
                if ( detectedType.isValid() ) {
                    mimeType = detectedType.name() ;
                    QLOG_STR("Detected file mime-type: " + mimeType) ;
                }
            }
#endif

            owner = iController.profileInUse().toString() ;

            iController.model().lock() ;
            const FrontWidget* frontWidget = reinterpret_cast<const FrontWidget*>
                                             ( iController.frontWidget() ) ;
            if ( frontWidget ) {
                const Profile *currentProfile ( frontWidget->selectedProfile() ) ;
                Hash fingerPrint ;
                if (currentProfile &&
                        ( fingerPrint =
                              iController.model().binaryFileModel()
                              .publishBinaryFile(*currentProfile,
                                                 fileName,
                                                 description,
                                                 mimeType,
                                                 owner,
                                                 license,
                                                 fileContent,
                                                 false,
                                                 isEncrypted,
                                                 fileRecipientList.length() == 0 ?
                                                 NULL :
                                                 &fileRecipientList ) ) != KNullHash ) {
                    iController.model().unlock() ;
                    Tcl_AppendResult(aInterp, fingerPrint.toString().toUtf8().constData(), NULL);

                    return TCL_OK ;
                } else {
                    iController.model().unlock() ;
                    Tcl_AppendResult(aInterp, "Publish failed", NULL);
                    return TCL_ERROR ;
                }
            }
            iController.model().unlock() ;
        }
        return TCL_ERROR ;
    } else {
        Tcl_AppendResult(aInterp, "Usage: publishFile file ( as dictionary )", NULL);
    }
    return TCL_ERROR ;
}

int TclCallbacks::publishClassifiedAdCmdImpl(ClientData /* aCData */, Tcl_Interp *aInterp, int aObjc, Tcl_Obj *const aObjv[]) {
    int retval = TCL_ERROR ;

    if ( aObjc > 1 ) {
        Tcl_DictSearch searchPtr  ;
        int done(false) ;
        Tcl_Obj* key (NULL) ;
        Tcl_Obj* value (NULL) ;

        if (Tcl_DictObjFirst(aInterp, aObjv[1], &searchPtr,
                             &key, &value, &done) != TCL_OK) {
            return TCL_ERROR;
        } else {
            CA ca ; // the ad that we're about to post
            ca.iAboutComboBoxIndex = -1 ;
            ca.iConcernsComboBoxIndex = -1 ;
            ca.iInComboBoxIndex = -1 ;
            do {
                // both key and value have been set
                if ( key && value ) {
                    int argumentStrLen (0) ;
                    QString keyStr ( Tcl_GetStringFromObj(key, &argumentStrLen) ) ;
                    QLOG_STR("Found key at publishClassifiedAdCmdImpl " + keyStr) ;
                    if ( keyStr == KTCLMessageText ) {
                        ca.iMessageText = QString::fromUtf8 (
                                              reinterpret_cast<const char *>(Tcl_GetStringFromObj(value,&argumentStrLen)) ) ;
                    }
                    if ( keyStr == KTCLSenderName ) {
                        ca.iSenderName = QString::fromUtf8 (
                                             reinterpret_cast<const char *>(Tcl_GetStringFromObj(value,&argumentStrLen)) ) ;
                    }
                    if ( keyStr == KTCLSubject ) {
                        ca.iSubject = QString::fromUtf8 (
                                          reinterpret_cast<const char *>(Tcl_GetStringFromObj(value,&argumentStrLen)) ) ;
                    }
                    if ( keyStr == KTCLPlainMessageText ) {
                        ca.iMessageText = "<html><body>"+
                                          QString::fromUtf8 (
                                              reinterpret_cast<const char *>(Tcl_GetStringFromObj(value,&argumentStrLen)) ) +
                                          "</body></html>";
                    }
                    if ( keyStr == KTCLAboutComboboxText ) {
                        ca.iAboutComboBoxText = QString::fromUtf8 (
                                                    reinterpret_cast<const char *>(Tcl_GetStringFromObj(value,&argumentStrLen)) ) ;
                    }
                    if ( keyStr == KTCLConcernsComboxboxText ) {
                        ca.iConcernsComboBoxText = QString::fromUtf8 (
                                                       reinterpret_cast<const char *>(Tcl_GetStringFromObj(value,&argumentStrLen)) ) ;
                    }
                    if ( keyStr == KTCLInComboboxText ) {
                        ca.iInComboBoxText = QString::fromUtf8 (
                                                 reinterpret_cast<const char *>(Tcl_GetStringFromObj(value,&argumentStrLen)) ) ;
                    }
                    if ( keyStr == KTCLAboutComboBoxIndex ) {
                        int intFromObj ( 0 ) ;
                        if ( Tcl_GetIntFromObj(aInterp, value, &intFromObj) == TCL_OK ) {
                            ca.iAboutComboBoxIndex = intFromObj ;
                        }
                    }
                    if ( keyStr == KTCLInComboBoxIndex ) {
                        int intFromObj ( 0 ) ;
                        if ( Tcl_GetIntFromObj(aInterp, value, &intFromObj) == TCL_OK ) {
                            ca.iInComboBoxIndex = intFromObj ;
                        }
                    }
                    if ( keyStr == KTCLConcernsComboBoxIndex ) {
                        int intFromObj ( 0 ) ;
                        if ( Tcl_GetIntFromObj(aInterp, value, &intFromObj) == TCL_OK ) {
                            ca.iConcernsComboBoxIndex = intFromObj ;
                        }
                    }
                    if ( keyStr == KTCLReplyTo ) {
                        ca.iReplyTo.fromString (
                            reinterpret_cast<const unsigned char *>(Tcl_GetStringFromObj(value,&argumentStrLen)) ) ;
                    }
                    if ( keyStr == KTCLAttachedFiles ) {
                        // in case of attached files the object in the dictionary
                        // is a list
                        int listLen ( 0 ) ;
                        if ( Tcl_ListObjLength(aInterp,value,&listLen) == TCL_OK ) {
                            for ( int i = 0 ; i < listLen ; i++ ) {
                                Tcl_Obj *objPtrPtr ;
                                if ( Tcl_ListObjIndex(aInterp, value, i, &objPtrPtr ) == TCL_OK ) {
                                    Hash attachedFileHash ;
                                    attachedFileHash.fromString (
                                        reinterpret_cast<const unsigned char *>(Tcl_GetStringFromObj(objPtrPtr,&argumentStrLen)) ) ;
                                    if ( attachedFileHash != KNullHash ) {
                                        ca.iAttachedFiles.append ( attachedFileHash ) ;
                                    }
                                } else {
                                    return TCL_ERROR ;
                                }
                            }
                        } else {
                            return TCL_ERROR ;
                        }
                    }
                }
                Tcl_DictObjNext(&searchPtr, &key, &value, &done) ;
            } while ( !done ) ;
            if ( ca.iMessageText.length() == 0  ) {
                Tcl_AppendResult(aInterp, "Empty ad not permitted", NULL);
                return TCL_ERROR ;
            }
            ca.iTimeOfPublish = QDateTime::currentDateTimeUtc().toTime_t() ;
            ca.iSenderHash = iController.profileInUse() ;
            if ( ca.iAboutComboBoxIndex >= ClassifiedAdsModel::ToBeBought &&
                    ca.iAboutComboBoxIndex <= ClassifiedAdsModel::ToBeAnnounced ) {
                ca.iAboutComboBoxText = iModel
                                        .classifiedAdsModel()
                                        .purposeOfAdString(static_cast<ClassifiedAdsModel::PurposeOfAd>(ca.iAboutComboBoxIndex)) ;
            } else {
                ca.iAboutComboBoxIndex = -1 ; // not in range
            }
            if ( ca.iConcernsComboBoxIndex >= ClassifiedAdsModel::ConcerningCars &&
                    ca.iConcernsComboBoxIndex <= ClassifiedAdsModel::ConcerningPhilosophy ) {
                ca.iConcernsComboBoxText = iModel
                                           .classifiedAdsModel()
                                           .concernOfAdString(static_cast<ClassifiedAdsModel::ConcernOfAd>(ca.iConcernsComboBoxIndex)) ;
            } else {
                ca.iConcernsComboBoxIndex = -1 ; // not in range
            }
            if ( ( ca.iInComboBoxIndex == 0 ||
                    ca.iInComboBoxIndex == -1 ) &&
                    ca.iInComboBoxText.length() < 1 ) {
                ca.iInComboBoxText = "Any country" ;
            } else {
                ca.iInComboBoxIndex = -1 ;
            }
            ca.iGroup = TclUtil::constructCAGroup ( ca.iAboutComboBoxText ,
                                                    ca.iConcernsComboBoxText ,
                                                    ca.iInComboBoxText ,
                                                    iModel ) ;
            QLOG_STR("TCL ad publish constructed group string " +
                     ca.iGroup ) ;
            iModel.lock() ;
            quint32 dummy  ;
            if (
                iModel.contentEncryptionModel().PublicKey( iController.profileInUse(),
                        ca.iProfileKey,
                        &dummy )) {
                Profile *p (iModel.profileModel().profileByFingerPrint(iController.profileInUse() ) ) ;
                if ( p != NULL ) {
                    ca.iFingerPrint = iModel.classifiedAdsModel().publishClassifiedAd(*p, ca) ;
                    delete p ;
                    if ( ca.iFingerPrint != KNullHash ) {
                        Tcl_AppendResult(aInterp, ca.iFingerPrint.toString().toUtf8().constData(), NULL);
                        retval = TCL_OK ;
                    } else {
                        Tcl_AppendResult(aInterp, "Publish failed", NULL);
                    }
                }
            }
            iModel.unlock() ;
        }
    } else {
        Tcl_AppendResult(aInterp, "Usage: publishClassifiedAd ad ( as dictionary )", NULL);
    }
    return retval ;
}


int TclCallbacks::publishDbRecordCmdImpl(ClientData /* aCData */, Tcl_Interp *aInterp, int aObjc, Tcl_Obj *const aObjv[]) {

    if ( aObjc > 1 ) {
        Tcl_DictSearch searchPtr  ;
        int done(false) ;
        Tcl_Obj* key (NULL) ;
        Tcl_Obj* value (NULL) ;

        if (Tcl_DictObjFirst(aInterp, aObjv[1], &searchPtr,
                             &key, &value, &done) != TCL_OK) {
            return TCL_ERROR;
        } else { // aObjv[1] was a dictionary
            CaDbRecord r ; 
            QList<Hash> recordRecipientList ;
            do {
                // both key and value have been set
                if ( key && value ) {
                    int argumentStrLen (0) ;
                    QString keyStr ( Tcl_GetStringFromObj(key, &argumentStrLen) ) ;
                    QLOG_STR("Found key at publishDbRecordCmdImpl " + keyStr) ;
                    if ( keyStr == KTCLDbRecordId ) {
                        r.iRecordId.fromString(reinterpret_cast<const unsigned char *>(Tcl_GetStringFromObj(value,&argumentStrLen))) ; 
                        if ( r.iRecordId == KNullHash ) {
                            QLOG_STR("Got null record id - record will be published as new") ;
                        }
                            
                    } else if ( keyStr == KTCLDbRecordCollectionId ) {
                        r.iCollectionId.fromString(reinterpret_cast<const unsigned char *>(Tcl_GetStringFromObj(value,&argumentStrLen))) ; 
                        if ( r.iCollectionId == KNullHash ) {
                            QLOG_STR("Got null collection id - can not continue publish") ;
                        }
                            
                    } else if ( keyStr == KTCLDbRecordSearchPhrase ) {
                        r.iSearchPhrase = QString::fromUtf8 (
                            reinterpret_cast<const char *>(Tcl_GetStringFromObj(value,&argumentStrLen)) ) ;
                    } else if ( keyStr == KTCLDbRecordSearchNumber ) {

                        Tcl_WideInt intFromObj ( 0 ) ;
                        if ( Tcl_GetWideIntFromObj(aInterp, value, &intFromObj) == TCL_OK ) {
                            r.iSearchNumber = intFromObj ;
                        } else {
                            Tcl_AppendResult(aInterp, "Invalid search number", NULL);
                            return TCL_ERROR ;
                        }
                    } else if ( keyStr == KTCLDbRecordEncrypted ) {
                        int intFromObj ( 0 ) ;
                        if ( Tcl_GetIntFromObj(aInterp, value, &intFromObj) == TCL_OK ) {
                            r.iIsEncrypted = intFromObj == 0 ? false : true ;
                        }
                    } else if ( keyStr == KTCLDbRecordRecipients ) {
                        // in case of recipient list the object in the dictionary
                        // is a list
                        int listLen ( 0 ) ;
                        if ( Tcl_ListObjLength(aInterp,value,&listLen) == TCL_OK ) {
                            for ( int i = 0 ; i < listLen ; i++ ) {
                                Tcl_Obj *objPtrPtr ;
                                if ( Tcl_ListObjIndex(aInterp, value, i, &objPtrPtr ) == TCL_OK ) {
                                    Hash recipientHash ;
                                    recipientHash.fromString (
                                        reinterpret_cast<const unsigned char *>(Tcl_GetStringFromObj(objPtrPtr,&argumentStrLen)) ) ;
                                    if ( recipientHash != KNullHash ) {
                                        recordRecipientList.append ( recipientHash ) ;
                                        QLOG_STR("Record recipient " + recipientHash.toString()) ;
                                    }
                                } else {
                                    return TCL_ERROR ;
                                }
                            }
                        } else {
                            return TCL_ERROR ;
                        }
                    } else if ( keyStr == KTCLDbRecordData ) {
                        unsigned char* bytes (NULL);
                        int length ;
                        if ( ( bytes = Tcl_GetByteArrayFromObj(value, &length) ) != NULL &&
                             length > 0 ) {
                            r.iData.append(reinterpret_cast<const char *>(bytes), length) ;
                        } 
                    }
                }
                Tcl_DictObjNext(&searchPtr, &key, &value, &done) ;
            } while ( !done ) ;


            iController.model().lock() ;

            QString errorString ;
            errorString =
                iController.model().caDbRecordModel()
                ->publishDbRecord(r,
                                  &recordRecipientList ) ; 
            if ( errorString.isEmpty() ) {
                iController.model().unlock() ;
                // on success db record get assigned an id. 
                // return it to calling TCL program. Note that
                // aObjv[1] can't be modified, TCL considers
                // it a shared object. 
                Tcl_AppendResult(aInterp, r.iRecordId.toString().toUtf8().constData(), NULL);
                return TCL_OK ;
            } else {
                Tcl_AppendResult(aInterp, errorString.toUtf8().constData(), NULL);
                iController.model().unlock() ;
                return TCL_ERROR ;
            }
        }
    } else {
        Tcl_AppendResult(aInterp, "Usage: publishFile file ( as dictionary )", NULL);
    }
    return TCL_ERROR ;
}
// here expect a dictionary. if there is key that belongs to 
// db record, treat it as "and" search term and return records
// that satisfy the conditions. Also send search to remote nodes
// that will then respond in async way.
int TclCallbacks::getDbRecordCmdImpl(ClientData /* aCData */, Tcl_Interp *aInterp, int aObjc, Tcl_Obj *const aObjv[]) {
    if ( aObjc > 1 ) {
        Tcl_DictSearch searchPtr  ;
        int done(false) ;
        Tcl_Obj* key (NULL) ;
        Tcl_Obj* value (NULL) ;
        CaDbRecord searchObj ; // object whose members will be used in
                               // search if they differ from detaults
        // if it is less than "max" then everything will be included.
        // user can limit the search below
        qint64 searchNumberLessThan (std::numeric_limits<qint64>::max());
        // if it is more than "min" then everything will be included
        qint64 searchNumberMoreThan (std::numeric_limits<qint64>::min());
        if (Tcl_DictObjFirst(aInterp, aObjv[1], &searchPtr,
                             &key, &value, &done) != TCL_OK) {
            return TCL_ERROR;
        } else { // aObjv[1] was a dictionary
            QList<Hash> recordRecipientList ;
            do {
                // both key and value have been set
                if ( key && value ) {
                    int argumentStrLen (0) ;
                    QString keyStr ( Tcl_GetStringFromObj(key, &argumentStrLen) ) ;
                    QLOG_STR("Found key at getDbRecordCmdImpl " + keyStr) ;
                    if ( keyStr == KTCLDbRecordId ) {
                        searchObj.iRecordId.fromString(reinterpret_cast<const unsigned char *>(Tcl_GetStringFromObj(value,&argumentStrLen))) ; 
                        if ( searchObj.iRecordId == KNullHash ) {
                            QLOG_STR("Got null record id - not included in search") ;
                        }
                            
                    } else if ( keyStr == KTCLDbRecordCollectionId ) {
                        searchObj.iCollectionId.fromString(reinterpret_cast<const unsigned char *>(Tcl_GetStringFromObj(value,&argumentStrLen))) ; 
                        if ( searchObj.iCollectionId == KNullHash ) {
                            QLOG_STR("Got null collection hash - can't retrieve") ;
                        }
                            
                    } else if ( keyStr == KTCLDbRecordSearchPhrase ) {
                        searchObj.iSearchPhrase = QString::fromUtf8 (
                            reinterpret_cast<const char *>(Tcl_GetStringFromObj(value,&argumentStrLen)) ) ;
                    } else if ( keyStr == KTCLDbRecordSearchNumber ) {

                        Tcl_WideInt intFromObj ( 0 ) ;
                        if ( Tcl_GetWideIntFromObj(aInterp, value, &intFromObj) == TCL_OK ) {
                            searchNumberMoreThan = searchNumberLessThan = intFromObj ;
                        } else {
                            Tcl_AppendResult(aInterp, "Invalid search number", NULL);
                            return TCL_ERROR ;
                        }
                    } else { 
                        if ( keyStr == KTCLDbRecordSearchNumberLessThan ) {

                            Tcl_WideInt intFromObj ( 0 ) ;
                            if ( Tcl_GetWideIntFromObj(aInterp, value, &intFromObj) == TCL_OK ) {
                                searchNumberLessThan = intFromObj ;
                            } else {
                                Tcl_AppendResult(aInterp, "Invalid less-than search number", NULL);
                                return TCL_ERROR ;
                            }

                        }
                        if ( keyStr == KTCLDbRecordSearchNumberMoreThan ) {
                            Tcl_WideInt intFromObj ( 0 ) ;
                            if ( Tcl_GetWideIntFromObj(aInterp, value, &intFromObj) == TCL_OK ) {
                                searchNumberMoreThan = intFromObj ;
                            } else {
                                Tcl_AppendResult(aInterp, "Invalid more-than search number", NULL);
                                return TCL_ERROR ;
                            }
                        }
                        
                    }
                }
                Tcl_DictObjNext(&searchPtr, &key, &value, &done) ;
            } while ( !done ) ;


            iController.model().lock() ;

            QList<CaDbRecord *> resultSet =
                iController.model().caDbRecordModel()
                ->searchRecords(searchObj.iCollectionId,
                                searchObj.iRecordId,
                                0, // modified after
                                std::numeric_limits<quint32>::max(),// modified before
                                searchNumberMoreThan ,
                                searchNumberLessThan,
                                searchObj.iSearchPhrase, 
                                searchObj.iSenderHash ) ;
            iController.model().unlock() ;
            Tcl_Obj* resultList ( Tcl_NewListObj(0, NULL) );

            while ( resultSet.size() > 0 ) {
                CaDbRecord * resultItem (resultSet.takeAt(0)) ; 
                Tcl_Obj* resultAsTclObj =  Tcl_NewDictObj() ;

                // record id
                Tcl_Obj* key = Tcl_NewStringObj(KTCLDbRecordId, -1) ;
                Tcl_Obj* value = Tcl_NewStringObj(resultItem->iRecordId.toString().toUtf8().constData(), KHashStringLen) ;
                Tcl_DictObjPut(aInterp,
                               resultAsTclObj,
                               key,
                               value) ;
                // collection id
                key = Tcl_NewStringObj(KTCLDbRecordCollectionId, -1) ;
                value = Tcl_NewStringObj(resultItem->iCollectionId.toString().toUtf8().constData(), KHashStringLen) ;
                Tcl_DictObjPut(aInterp,
                               resultAsTclObj,
                               key,
                               value) ;
                // record sender id, the profile SHA1 in practice
                key = Tcl_NewStringObj(KTCLDbRecordSenderId, -1) ;
                value = Tcl_NewStringObj(resultItem->iSenderHash.toString().toUtf8().constData(), KHashStringLen) ;
                Tcl_DictObjPut(aInterp,
                               resultAsTclObj,
                               key,
                               value) ;

                // search number
                if ( resultItem->iSearchNumber != std::numeric_limits<qint64>::min() ) {
                    key = Tcl_NewStringObj(KTCLDbRecordSearchNumber, -1) ;
                    value = Tcl_NewWideIntObj(resultItem->iSearchNumber) ;
                    Tcl_DictObjPut(aInterp,
                                   resultAsTclObj,
                                   key,
                                   value) ;
                }
                // search phrase
                if ( resultItem->iSearchPhrase.isEmpty() == false ) {
                    key = Tcl_NewStringObj(KTCLDbRecordSearchPhrase, -1) ;
                    value = Tcl_NewStringObj(resultItem->iSearchPhrase.toUtf8().constData(),-1) ;
                    Tcl_DictObjPut(aInterp,
                                   resultAsTclObj,
                                   key,
                                   value) ;
                }
                // time of publish
                key = Tcl_NewStringObj(KTCLTimeOfPublish, -1) ;
                value = Tcl_NewLongObj(resultItem->iTimeOfPublish) ;
                Tcl_DictObjPut(aInterp,
                               resultAsTclObj,
                               key,
                               value) ;
                // is encrypted
                key = Tcl_NewStringObj(KTCLDbRecordEncrypted, -1) ;
                value = Tcl_NewIntObj(resultItem->iIsEncrypted ? 1 : 0 ) ;
                Tcl_DictObjPut(aInterp,
                               resultAsTclObj,
                               key,
                               value) ;
                // is verified
                key = Tcl_NewStringObj(KTCLDbRecordIsSigVerified, -1) ;
                value = Tcl_NewIntObj(resultItem->iIsSignatureVerified ? 1 : 0 ) ;
                Tcl_DictObjPut(aInterp,
                               resultAsTclObj,
                               key,
                               value) ;

                // actual record data
                key = Tcl_NewStringObj(KTCLDbRecordData,-1) ;
                value = Tcl_NewByteArrayObj(
                    reinterpret_cast<const unsigned char *>(resultItem->iData.constData()),
                    resultItem->iData.size() ) ;
                Tcl_DictObjPut(aInterp,
                               resultAsTclObj,
                               key,
                               value) ;

                Tcl_ListObjAppendElement(aInterp,
                                         resultList,
                                         resultAsTclObj) ;
                delete resultItem ; 
            }
            Tcl_SetObjResult(aInterp, resultList);
            return TCL_OK ; 
        }
    } else {
        Tcl_AppendResult(aInterp, "Usage: getDbRecord ...", NULL);
    }
    return TCL_ERROR ;
}

int TclCallbacks::storeTCLProgLocalDataImpl(ClientData /*aCData*/, Tcl_Interp *aInterp, int aObjc, Tcl_Obj* const aObjv[]) {
    if ( aObjc > 1 ) {
        QByteArray fileContent ; 
        unsigned char* bytes (NULL);
        int length ;
        if ( ( bytes = Tcl_GetByteArrayFromObj(aObjv[1], &length) ) != NULL ) {
            fileContent.append(reinterpret_cast<const char *>(bytes), length) ;
        } else {
            Tcl_AppendResult(aInterp, "Could not get file content?", NULL);
            return TCL_ERROR ;
        }
        iController.model().lock() ;
        TclProgram p ;
        const QString programText (iController.tclWrapper()
                                   .currentProgram( )) ; 
        p.setProgramText(programText) ; 
        QString errorMessage (
            iController.model()
            .tclModel()
            .storeTCLProgLocalData(p.iFingerPrint,
                                   fileContent) );
        iController.model().unlock() ;
        if ( errorMessage.isEmpty() ) {
            return TCL_OK ; 
        } else {
            Tcl_AppendResult(aInterp, errorMessage.toUtf8().constData(), NULL);
            return TCL_ERROR ; 
        }
    } else {
        Tcl_AppendResult(aInterp, "Usage: storeLocalData data", NULL);
    } 
    return TCL_ERROR ; 
}

int TclCallbacks::retrieveTCLProgLocalDataImpl(ClientData /*aCData*/, Tcl_Interp *aInterp, int /*aObjc*/, Tcl_Obj* const /*aObjv*/[]) {
    iController.model().lock() ;
    TclProgram p ;
    p.setProgramText(iController.tclWrapper()
                     .currentProgram( )) ; 
    const QByteArray fileContent =
        iController.model()
        .tclModel()
        .retrieveTCLProgLocalData(p.iFingerPrint) ; 
    iController.model().unlock() ;
    Tcl_Obj* result = Tcl_NewByteArrayObj(
        reinterpret_cast<const unsigned char *>(fileContent.constData()),
        fileContent.size() ) ;
    Tcl_SetObjResult(aInterp, result);
    return TCL_OK ; 
}

int TclCallbacks::saveFileImpl(ClientData /*aCData*/, Tcl_Interp *aInterp, int aObjc, Tcl_Obj* const aObjv[]) {
    QByteArray* fileContent = new QByteArray() ; 
    QString errorMessage ; 
    if ( aObjc > 1 && fileContent != NULL ) {
        QString fileName ; 
        unsigned char* bytes (NULL);
        int length ;
        if ( ( bytes = Tcl_GetByteArrayFromObj(aObjv[1], &length) ) != NULL ) {
            fileContent->append(reinterpret_cast<const char *>(bytes), length) ;
        } else {
            Tcl_AppendResult(aInterp, "Could not get file content?", NULL);
            delete fileContent ; 
            return TCL_ERROR ;
        }
        if ( aObjc > 2 ) {
            int argumentStrLen (0) ;
            fileName = QString::fromUtf8 (
                reinterpret_cast<const char *>(Tcl_GetStringFromObj(aObjv[2],&argumentStrLen)) ) ;
        }
        QLOG_STR("Starting to save file fro tcl prog") ; 
        bool successInGettingFileName ; 
        fileName = 
            iController.getFileName(successInGettingFileName,
                                    true,
                                    fileName.length() > 0 ?
                                    fileName : 
                                    QString::null ) ;
        if ( successInGettingFileName && fileName.length() > 0 ) {
            QFile f ( fileName ) ;
            if ( f.open(QIODevice::WriteOnly) ) {
                f.write(*fileContent) ; 
                f.close() ; 
                QLOG_STR("File saved") ; 
            } else {
                errorMessage = f.errorString() ; 
            }
        } else {
            Tcl_AppendResult(aInterp, "Cancelled", NULL);
        }
        delete fileContent ; 
    }
    if ( errorMessage.isEmpty() ) {
        return TCL_OK ; 
    } else {
        Tcl_AppendResult(aInterp, errorMessage.toUtf8().constData(), NULL);
        return TCL_ERROR ; 
    }
}

int TclCallbacks::openFileImpl(ClientData /*aCData*/, Tcl_Interp *aInterp, int aObjc, Tcl_Obj* const aObjv[]) {
    QByteArray* fileContent = new QByteArray() ; 
    QString errorMessage ; 
    QString fileName ; 

    if ( aObjc > 1 ) {
        int argumentStrLen (0) ;
        fileName = QString::fromUtf8 (
            reinterpret_cast<const char *>(Tcl_GetStringFromObj(aObjv[1],&argumentStrLen)) ) ;
    }
    QLOG_STR("Starting to open file from tcl prog") ; 
    bool successInGettingFileName ; 
    fileName = 
        iController.getFileName(successInGettingFileName,
                                false,
                                fileName.length() > 0 ?
                                fileName : 
                                QString::null ) ;
    if ( successInGettingFileName && fileName.length() > 0 ) {
        QFile f ( fileName ) ;
        if ( f.open(QIODevice::ReadOnly) ) {
            fileContent->append(f.read(f.size())) ;
            f.close() ; 
            QLOG_STR("File opened") ; 
        } else {
            errorMessage = f.errorString() ; 
        }
    } else {
        Tcl_AppendResult(aInterp, "Cancelled", NULL);
        delete fileContent ; 
        return TCL_ERROR ; 
    }

    Tcl_Obj* result = Tcl_NewByteArrayObj(
        reinterpret_cast<const unsigned char *>(fileContent->constData()),
        fileContent->size() ) ;
    Tcl_SetObjResult(aInterp, result);
    delete fileContent ; 
    return TCL_OK ; 
}

// non-static version of profile trust status getter.
int TclCallbacks::isProfileTrustedImpl(ClientData /* aCData */, Tcl_Interp *aInterp, int aObjc, Tcl_Obj *const aObjv[]) {
    int argumentStrLen (0) ;
    // normally aObjc = 2, as first argument is command itself,
    // and 2nd argument is the search term
    if ( aObjc > 1 ) {
        const unsigned char *argumentStr(
            reinterpret_cast<const unsigned char *>( Tcl_GetStringFromObj(aObjv[1], &argumentStrLen))) ;
        QLOG_STR("TclCallbacks::isProfileTrustedImpl in " + QString::number(aObjc)) ;
        if ( argumentStrLen == KHashStringLen ) { // search term is hash and that is 40
            Hash searchTerm ;
            searchTerm.fromString( argumentStr ) ;
            if ( searchTerm != KNullHash ) {
                iModel.lock() ;
                bool result ( iModel.trustTreeModel()->
                              isProfileTrusted(searchTerm,NULL,NULL) ) ;
                iModel.unlock() ;

                Tcl_Obj* resultAsTclObj =  Tcl_NewBooleanObj(result) ;
                Tcl_SetObjResult(aInterp, resultAsTclObj);
                return TCL_OK ;

            } else {
                Tcl_AppendResult(aInterp, "Invalid 40-character input, not parseable as hash", NULL);
            }
        } else {
            Tcl_AppendResult(aInterp, "Argument string length not 40", NULL);
        }
    } else {
        Tcl_AppendResult(aInterp, "Usage: isProfileTrusted fingerprint", NULL);
    }
    return TCL_ERROR ;
}
