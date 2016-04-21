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

#ifndef MOCKUP_CONTROLLER_H
#define MOCKUP_CONTROLLER_H
#include <QtGui>
#include <QBoxLayout>
#include "../mcontroller.h"
#include "mockup_model.h"
class MockUpVoiceCallEngine ; 

/**
 * @brief Controller for testing purposes only. Not included in real binary.
 */
class MockUpController : public MController {
    Q_OBJECT

public:

    /**
     * constructor
     */
    MockUpController() ;
    /**
     * Destructor
     */
    ~MockUpController() ;
    /**
     * Method for requesting different things to take place in UI.
     * controller mostly routes these to FrontWidget but other actions
     * may be in order too..
     * @param aRequest users orders
     * @param aHashConcerned possible hash parameter ; can be
     *        null hash if action is not about specific hash
     * @return none
     */
    virtual void userInterfaceAction ( CAUserInterfaceRequest aRequest,
                                       const Hash& aHashConcerned = KNullHash,
                                       const Hash& aFetchFromNode = KNullHash,
                                       const QString* aAdditionalInformation = NULL ) ;
    /**
     * method for hiding UI
     */
    virtual void hideUI() ;
    /**
     * method for showing UI
     */
    virtual void showUI() ;
public slots:
    virtual void exitApp() ; /**< quitting */
    virtual void displayAboutBox() ; /**< bragging */
    virtual void displayFront() ; /**< this initializes the "normal" display */
    /**
     * Method for handling errors inside application.
     * @param aError Reason for error call, from error enum above
     * @param aExplanation NULL or human-readable description about what went
     *                     wrong.
     */
    virtual void handleError(MController::CAErrorSituation aError,
                             const QString& aExplanation) ;
    /**
     * Method for node ; this may be changed during startup-phase
     * but not after that
     */
    virtual Node& getNode() const ;
    /**
     * method for network listener ; it is parent of all connections,
     * also the outgoing
     */
    virtual NetworkListener *networkListener() const ;
    virtual Model& model() const ;
    virtual void setProfileInUse(const Hash& aProfileHash) ;
    const Hash& profileInUse() ;
    virtual void setContentKeyPasswd(QString aPasswd)  ;
    /**
     * method for getting gpg passwd previously set
     */
    virtual QString contentKeyPasswd()  const  ;


    virtual void startRetrievingContent(NetworkRequestExecutor::NetworkRequestQueueItem aReq,bool aIsBackgroundDl, ProtocolItemType aTypeOfExpectedObject) ;


    virtual void storePrivateDataOfSelectedProfile(bool aPublishTrustListToo = false)  ;


    virtual void reStorePrivateDataOfSelectedProfile() ;


    virtual bool isContactInContactList(const Hash& aFingerPrint) const;


    virtual QString displayableNameForProfile(const Hash& aProfileFingerPrint) const ;

    virtual void offerDisplayNameForProfile(const Hash& aProfileFingerPrint,
                                            const QString& aDisplayName,
                                            const bool iUpdatePersistenStorage=false)  ;
    /**
     * method that puts dialog or similar on display, about a published file
     */
    virtual void displayFileInfoOnUi(const BinaryFile& aFileMetadata)  ;
    /**
     * Method for getting voice call engine. This particular implementation
     * will return NULL always
     * @return NULL
     */
    virtual VoiceCallEngine* voiceCallEngine() { return NULL ; } 

    /**
     * Method that returns a mock-up of the voice call engine. 
     * Used for testing the call protocol parts
     */
    virtual MVoiceCallEngine* voiceCallEngineInterface() ; 
    /**
     * Getter-method for real voice call engine mock-up. After call data
     * methods have been called, state of this mocku-up is checked so
     * we can determine the success of test cases
     */
    MockUpVoiceCallEngine* voiceCallEngineMockUp() { return iCallEngine; } 
private:
    Node *iNode ; /**< our network presence object, there is single instance */
    Model *iModel ; /**< data storage animal */
    NetworkListener *iListener ; /**< Incoming connections handler, for ipv4 */
    QString iContentPasswd ;
    Hash iProfileHash ;
    MockUpVoiceCallEngine *iCallEngine ; 
} ;
#endif

