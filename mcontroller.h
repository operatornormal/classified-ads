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

#ifndef M_CONTROLLER_H
#define M_CONTROLLER_H
#include <QObject>
#include "util/hash.h" // Hash
#include "datamodel/netrequestexecutor.h"

class Node ;
class Model ;
class NetworkListener ;
class NetworkConnectorEngine ;
class NetworkRequestExecutor;
class BinaryFile ;
class VoiceCallEngine ; 
class MVoiceCallEngine ; 

/**
 * @brief Pure-virtual interface class for controller.
 * This is spammed
 * across parties needing access to application controller.
 * Reason for this interface is that for testing purposes we can
 * replace real controller with a dummy mock-up
 */
class MController : public QObject {
    Q_OBJECT

public:
    /**
     * Enumeration for different errors that may occur within this
     * app. Class controller is supposed to handle these and may
     * be signaled an error
     */
    enum CAErrorSituation {
        OwnCertNotFound, /**< Does not have own cert and can't generate */
        DataBaseNotMountable, /**< corrupt database or permission? */
        BadPassword, /**< Could not open encryption keys with given pwd */
        DbTransactionError, /**< something went foul with db */
        ContentEncryptionError, /**< something went foul with content encryption interface */
        FileOperationError /**< Error related to binary files */
    } ;

    /**
     * Enumeration for different user-interface actions that
     * controller tries to route
     */
    enum CAUserInterfaceRequest {
        ViewProfileDetails, /**< User wants to view details of profile */
        ViewCa, /**< User wants to view classified ad */
        ViewProfileComment, /**< User wants to view profile comment */
        DisplayProgressDialog, /**< puts wait dialog on screen */
        VoiceCallToNode /**< User wants voice call to remote node */
    } ;

    /**
     * method that starts actions regarding content fetch from
     * network
     * @param aReq specifies the content,at least iRequestType and
     *             iRequestedItem need to be there
     * @param aIsBackgroundDl is true if the retrieval may be
     *        queued into background as a low-priority item
     */
    virtual void startRetrievingContent(NetworkRequestExecutor::NetworkRequestQueueItem aReq,
                                        bool aIsBackgroundDl,
                                        ProtocolItemType aTypeOfExpectedObject ) = 0 ;

    /**
     * Method for requesting different things to take place in UI.
     * controller mostly routes these to FrontWidget but other actions
     * may be in order too..
     * @param aRequest users orders
     * @param aHashConcerned possible hash parameter ; can be
     *        null hash if action is not about specific hash
     * @param aFetchFromNode possible node hash parameter ; if
     *        concerning item is not found from local storage,
     *        try to fetch it from given node ; is KNullHash,
     *        then just do fetch using normal algorithm.
     * @param aAdditionalInformation possible explanation or other info
     * @return none
     */
    virtual void userInterfaceAction ( CAUserInterfaceRequest aRequest,
                                       const Hash& aHashConcerned = KNullHash,
                                       const Hash& aFetchFromNode = KNullHash,
                                       const QString* aAdditionalInformation = NULL ) = 0 ;

    virtual void hideUI() = 0 ;
    /**
     * method for showing UI
     */
    virtual void showUI() = 0 ;
    /**
     * method selecting user profile in use.
     */
    virtual void setProfileInUse(const Hash& aProfileHash) = 0 ;
    /**
     * method getting user profile in use.
     */
    virtual const Hash& profileInUse() = 0 ;
    /**
     * method for setting passwd used to open private content encryption
     * rsa key. this password is stored in controller
     * and is then used by content-open/sign-operations when crypto lib
     * asks for password.
     */
    virtual void setContentKeyPasswd(QString aPasswd) = 0 ;
    /**
     * method for getting passwd of private content keys previously set, see method
     * @ref Controller::setContentKeyPasswd
     */
    virtual QString contentKeyPasswd()  const = 0;

public slots:
    virtual void exitApp() = 0 ; /**< quitting */
    virtual void displayAboutBox() = 0  ; /**< bragging */
    virtual void displayFront() = 0 ; /**< this initializes the "normal" display */
    /**
     * Method for handling errors inside application.
     * @param aError Reason for error call, from error enum above
     * @param aExplanation NULL or human-readable description about what went
     *                     wrong.
     */
    virtual void handleError(MController::CAErrorSituation aError,
                             const QString& aExplanation) = 0 ;
    /**
     * Method for node ; this may be changed during startup-phase
     * but not after that
     */
    virtual Node& getNode()  const = 0 ;
    /**
     * method for network listener ; it is parent of all connections,
     * also the outgoing
     */
    virtual NetworkListener *networkListener() const = 0 ;
    /**
     * method for getting datamodel
     */
    virtual Model &model() const = 0 ;
    /**
     * method for storing private data of profile currently in use
     *
     * @param aPublishTrustListToo if set to true, has selected profiles
     *        trust list to be updated profile data and profile published
     *        with the new trust list.
     */
    virtual void storePrivateDataOfSelectedProfile(bool aPublishTrustListToo = false ) = 0 ;
    /**
     * method for restoring private data of profile currently in use.
     * shall be called after new profile is selected in frontwidget.
     */
    virtual void reStorePrivateDataOfSelectedProfile() = 0 ;
    /**
     * method for checking if contact is in contact list
     */
    virtual bool isContactInContactList(const Hash& aFingerPrint) const = 0;
    virtual QString displayableNameForProfile(const Hash& aProfileFingerPrint) const = 0 ;
    virtual void offerDisplayNameForProfile(const Hash& aProfileFingerPrint,
                                            const QString& aDisplayName,
                                            const bool iUpdatePersistenStorage=false) = 0 ;
    /**
     * method that puts dialog or similar on display, about a published file
     */
    virtual void displayFileInfoOnUi(const BinaryFile& aFileMetadata) = 0 ;
    /**
     * Method for getting voice call engine. If there is no engine
     * prior to call, one will be constructed
     * @return engine instance
     */
    virtual VoiceCallEngine* voiceCallEngine() = 0 ; 

    /**
     * Method for getting voice call engine interface. If there is no engine
     * prior to call, one will be constructed. The instance is the same
     * that is returned via @ref voiceCallEngine method. 
     * 
     * @return engine instance, possibly a mock-up for testing purposes
     */
    virtual MVoiceCallEngine* voiceCallEngineInterface() = 0 ; 
} ;
#endif

