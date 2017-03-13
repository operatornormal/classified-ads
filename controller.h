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

/**
 * @mainpage Classified ads
 *
 * Classified ads is internet communications program that works
 * in distributed manner, requiring and supporting no concept of
 * "server". Classified ads supports
 *  - Concept of operator, that is identified by encryption key and
 *    can contain also other data like real name that human being
 *    operating the profile wants to publish.
 *  - Making some data available to listed operators only, this is
 *    done by marking a operator profile private.
 *  - Sending and receiving
 *   -# Public messages, that are called "classified ads" due to
 *      classification that they carry,
 *   -# Private messages between operators,
 *   -# Comments regarding operators, readable by group that see
 *      profile of the commented operator.
 *  - Basic word-based search functions.
 *  - Voice calls between operator nodes.
 *
 * For more information see http://katiska.org/classified-ads/. In
 * order to use classified ads in meaningful manner, user needs to
 * have the operated instance of this program to connect to some other
 * program instance over some network. Classified ads tries to do
 * that automatically but it may be necessary to ask for network address
 * of node operated by a friend to get connected and started.
 *
 * Classified ads is released under LGPL license.
 *
 * \copyright Antti Järvinen and others 2013-2016.
 */

#ifndef CONTROLLER_H
#define CONTROLLER_H
#include <QtGui>
#include <QBoxLayout>
#include <QUrl>
#include "mcontroller.h"
#include "net/protocol.h" // for ProtocolItemType
#include "datamodel/netrequestexecutor.h"

class FrontWidget ;
class PublishingEngine ;
class RetrievalEngine ;
class DbRecordRetrievalEngine ; 
class QMainWindow ;
class QMenu ;
class VoiceCallEngine ;
class QSharedMemory ;
#ifdef WIN32
class QLocalServer ;
#endif
class QFileDialog ; 

/**
 * @brief Class for keeping app state.
 *
 * C of MVC-pattern is considered here.
 * UI events are routed
 * via this class and this implements scheduling of events
 * so that things happen in correct order.
 */
class Controller : public MController {
    Q_OBJECT

public:

    /**
     * constructor. see also method @init.
     */
    Controller(QApplication& app) ;
    /**
     * Destructor
     */
    ~Controller() ;

    /**
     * Constructor extras. Constructor may fail but there is no way to
     * communicate that. Design is now so that constructor only allocates
     * memory and initializes member variables, this method here, @init
     * contains constructor logic and it may fail.
     *
     * @return true if initialization is ok
     */
    bool init() ;

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
                                        ProtocolItemType aTypeOfExpectedObject) ;
    /**
     * Variant of "start fetch" method that starts fetch of db records.
     * @param aSearchTerms Database query that fetched record should
     *        satisfy
     */
    virtual void startRetrievingContent( CaDbRecord::SearchTerms aSearchTerms ) ;
    /**
     * From MController.
     *
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
                                       const QString* aAdditionalInformation = NULL)  ;
    /**
     * method for hiding UI
     */
    virtual void hideUI() ;
    /**
     * method for showing UI
     */
    virtual void showUI() ;
    /**
     * method selecting user profile in use.
     */
    virtual void setProfileInUse(const Hash& aProfileHash) ;
    /**
     * method getting user profile in use.
     */
    virtual const Hash& profileInUse()  ;
    /**
     * method for setting passwd used to open private content encryption
     * rsa key. this password is stored in controller
     * and is then used by content-open/sign-operations when crypto lib
     * asks for password.
     */
    virtual void setContentKeyPasswd(QString aPasswd)  ;
    /**
     * method for getting passwd of private content keys previously set, see method
     * @ref Controller::setContentKeyPasswd
     */
    virtual QString contentKeyPasswd()  const ;

public: // methods
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
    /**
     * method for getting datamodel
     */
    virtual Model &model() const  ;
    /**
     * method for setting an URL to open. Url is opened only
     * if it is of classified-ads URL scheme, currently
     * supported protocols are caprofile, caad, cacomment and cablob
     * and if URL scheme is not among those, this method does
     * no thing. Host part contains hash of object, other parts
     * are ignored.
     *
     * @param aClassifiedAdsObject object to bring visible to user
     */
    void addObjectToOpen(QUrl aClassifiedAdsObject) ;
    /**
     * method called if old instance of this program is signaled
     * from new instace, calling for this instance to bring
     * itself to front, and, in this method, to check if
     * there is object mentioned in shared memory segment
     * that needs to be displayed
     */
    void checkForSharedMemoryContents() ;
signals:
    void userProfileSelected(const Hash& aProfile) ;
    /** used for signalling possible wait dialog about dismissal */
    void waitDialogToBeDismissed() ;
    void startGettingFileName(QString aSuggestedFileName,bool aIsSaveFile) ; /**< signal for launching file selection dlg*/
public slots:
    virtual void exitApp() ; /**< quitting */
    virtual void displayAboutBox() ; /**< bragging */
    virtual void displayFront() ; /**< this initializes the "normal" display */
    virtual void changeProfilePasswd() ; /**< name says it all. initiates UI sequence */
    virtual void createProfile() ; /**< Initiates UI sequence for new profile*/
    virtual void deleteProfile() ; /**< Initiates UI sequence for deleting profile*/
    virtual void selectProfile() ; /**< Initiates UI sequence for selecting profile*/
    virtual void displaySettings() ; /**< Slot for displaying node settings */
    virtual void displayStatus() ; /**< Slot for displaying network status */
    virtual void displaySearch() ; /**< Slot for displaying search dialog */

    /* tcl-related UI slots */
    virtual void displayTclProgs() ; /**< Slot for displaying TCL library */
    virtual void displayTclConsole() ; /**< Slot for displaying TCL console */

    /**
     * Method for handling errors inside application.
     * @param aError Reason for error call, from error enum above
     * @param aExplanation NULL or human-readable description about what went
     *                     wrong.
     */
    virtual void handleError(MController::CAErrorSituation aError,
                             const QString& aExplanation) ;
    /**
     * This is receiving slot of signals sent from actual content handlers ;
     * when we receive new content, this method is hit, reason for this
     * is that we may be waiting for specific content somewhere
     * @param aHashOfContent item that was requested
     * @param aTypeOfReceivedContent item type requested
     */
    void notifyOfContentReceived(const Hash& aHashOfContent,
                                 const ProtocolItemType aTypeOfReceivedContent );
    /**
     * This is receiving slot of signals sent from actual content handlers ;
     * when we receive new content, this method is hit, reason for this
     * is that we may be waiting for specific content somewhere.
     * This overload is mostly hit by classified ads.
     *
     * @param aHashOfContent item that was requested
     * @param aHashOfClassification of item that was requested
     * @param aTypeOfReceivedContent item type requested
     */
    void notifyOfContentReceived(const Hash& aHashOfContent,
                                 const Hash& aHashOfClassification,
                                 const ProtocolItemType aTypeOfReceivedContent );
    /**
     * This is receiving slot of signals sent from
     * retrieval engine;
     * when we try to receive content and we do not get any,
     * this notifies user that time-out is due
     * @param aHashOfContent item that was requested
     * @param aTypeOfNotReceivdContent item type requested
     */
    void notifyOfContentNotReceived(const Hash& aHashOfContent,
                                    const ProtocolItemType aTypeOfNotReceivdContent );

    /**
     * Method for persisting profile private data inside datamodel.
     * Tnis is supposed to be called every time after private data
     * changes.
     *
     * lock the datamodel before calling this method
     *
     * @param aPublishTrustListToo if set to true, has selected profiles
     *        trust list to be updated profile data and profile published
     *        with the new trust list.
     */
    virtual void storePrivateDataOfSelectedProfile(bool aPublishTrustListToo = false) ;
    /**
     * method for restoring private data of profile currently in use.
     * shall be called after new profile is selected in frontwidget.
     */
    virtual void reStorePrivateDataOfSelectedProfile() ;

    /**
     * method for checking if a profile is found from contact
     * list of selected user
     */
    virtual bool isContactInContactList(const Hash& aFingerPrint) const ;
    /**
     * method for producing a displayable version of a profile.
     * in practice this utilitizes the contacts of the selected profile
     * and a cache that is collected from private messages,
     * ads, profile comments and profiles
     */
    virtual QString displayableNameForProfile(const Hash& aProfileFingerPrint) const ;
    /**
     * method for keeping profile hash<->displayname relation up to date.
     * this is called when display names are seen in profiles, ads,
     * private messages etc.
     */
    virtual void offerDisplayNameForProfile(const Hash& aProfileFingerPrint,
                                            const QString& aDisplayName,
                                            const bool iUpdatePersistenStorage=false) ;
    /**
     * method that puts dialog or similar on display, about a published file
     */
    virtual void displayFileInfoOnUi(const BinaryFile& aFileMetadata) ;
    /**
     * Method for getting voice call engine, if there is any.
     * From MController interface.
     * @return engine or null
     */
    virtual VoiceCallEngine* voiceCallEngine()  ;
    /**
     * Method for getting voice call engine, if there is any.
     * From MController interface.
     * @return engine or mockup. In normal runtime this just calls @ref voiceCallEngine.
     */
    virtual MVoiceCallEngine* voiceCallEngineInterface()  ;
    /**
     * Method for getting tcl wrapper instance. If there is no instance
     * one will be created.
     * From @ref MController interface. 
     */
    virtual TclWrapper &tclWrapper() ;

    /**
     * Method for getting front-widget, to be used as parent of dialogs
     * spawned from non-ui threads. From MController interface. 
     */
    virtual QWidget *frontWidget() ; 
    /**
     * method for sending a poll around network regarding possible
     * update for a profile and possible addition of comments about
     * given profile.
     *
     * datamodel should not be locked when this is called.
     *
     * in practice this is called after user selects a profile to
     * be viewed ; it could be called periodically too for selected
     * profiles..
     *
     * @param aProfileFingerPrint is fingerprint of the profile concerned.
     * @param aProfileNodeFingerPrint fingerprint of node that is suspected to
     *                                be the node where profile is published from.
     *                                this is naturally good candidate for
     *                                sending the query.
     */
    void sendProfileUpdateQuery(const Hash& aProfileFingerPrint,
                                const Hash& aProfileNodeFingerPrint = KNullHash ) ;
#ifdef WIN32
    void newInstanceConnected() ; /**< WIN32 IPC callback */
#endif
    /**
     * Method for getting file name. Method displays file selection
     * dialog and returns the selected file.  Idea of this method is 
     * that it may be called from background threads and it will
     * display the dialog in UI thread, then report results back.
     * This is a blocking method that will suspend execution of the
     * calling thread for the duration while user is doing the selection. 
     *
     * @param aSuccess is set to true if operation ends all right. 
     * @param aIsSaveFile if set to true, "file save" dialog is
     *        shown, otherwise "file open" dialog.
     * @param aSuggestedFileName file name (pattern). If given empty,
     *        any file is suggested in dialog, if "*.jpg" is given, then
     *        dialog shall suggest only files with .jpg ending and
     *        if "foobar.txt" is given, then dialog will suggest literal
     *        file name "foobar.txt". 
     * @return file system file name or empty if aSuccess is set to false. 
     */
    virtual QString getFileName(bool& aSuccess,
                                bool aIsSaveFile = false , 
                                QString aSuggestedFileName = QString()) ;
private:
    void createMenus(); /**< menus here */
    int createPidFile(); /**< leave a mark to filesystem about instance */
    void deletePidFile(); /**< remove mark from filesystem about instance */
    /**
     * creates and possibly populates a shared memory segment
     * for IPC needs
     */
    bool createSharedMemSegment(QString& aSegmentName);
private slots:
    void checkForObjectToOpen(const Hash& aIgnored) ; /** processing of method addObjectToOpen */
    /**
     * worker-slot for @ref getFileName method ; this slot will be called
     * in queued manner and have its code run in UI thread.
     */
    void getFileNameSlot(QString aSuggestedFileName,bool aIsSaveFile) ;
private:
    QMainWindow* iWin ;
    FrontWidget* iCurrentWidget ; /**< normally points to "frontwidget" instance */
    QApplication& iApp ;
    QBoxLayout* iLayout ;
    QMenu *iFileMenu;
    QAction *iExitAct;
    QAction *iAboutAct;
    QAction *iPwdChangeAct;
    QAction *iProfileDeleteAct;
    QAction *iProfileCreateAct;
    QAction *iProfileSelectAct;
    QAction *iDisplaySettingsAct;
    QAction *iDisplayStatusAct;
    QAction *iDisplaySearchAct;
    QMenu *iTclMenu;
    QAction *iTclLibraryAct; /**< Menu item for opening tcl lib dialog */
    QAction *iTclConsoleAct; /**< Menu item for opening tcl console dialog */
    Node *iNode ; /**< our network presence object, there is single instance */
    Model *iModel ; /**< data storage animal */
    NetworkListener *iListener ; /**< Incoming connections handler, for ipv4 */
    NetworkConnectorEngine *iNetEngine ; /**< Outgoing connections handler */
    QString iContentKeyPasswd ; /** passwd used to protect profile private RSA key */
    Hash iProfileHash ; /**< fingerprint of profile currently in use */
    PublishingEngine *iPubEngine ; /**< Logic for handling content publish */
    RetrievalEngine* iRetrievalEngine ;/**< Logic for fetcing stuff from other nodes */
    DbRecordRetrievalEngine* iDbRetrievalEngine ;/**< Logic for fetcing db records from other nodes */
    /**
     * if user requests for item that we do not have, lets put a
     * wait dialog in place and start wait for the object to appear
     * from network. in order to properly dismiss the dialog,
     * have here type (and later hash) of the objects that we're waiting
     * for */
    ProtocolItemType iTypeOfObjectBeingWaitedFor ;
    /**
     * hash of object that user needs to wait.
     */
    Hash iHashOfObjectBeingWaitedFor ;
    /**
     * hash of profile comment that user needs to wait. This is
     * used in two-stage fetch process of profile comment where
     * the profile may need to be retrieved first, then the
     * comment: we store there the profile comment hash for
     * duration of profile fetch
     */
    Hash iHashOfProfileCommentBeingWaitedFor ;
    /**
     * Node where to ask for profile comment once profile
     * has been fetched
     */
    Hash iNodeForCommentBeingWaitedFor ;
    /**
     * profile hash<->display_name mapping
     */
    QMap<Hash,QString> iHashDisplaynameMapping ;
    /**
     * Currently there is support for one voice call at time
     */
    VoiceCallEngine* iVoiceCallEngine ;
    /**
     * Flag for destructor. If this is on, don't allocate more objects
     */
    bool iInsideDestructor ;
    /**
     * pending object to open
     */
    QUrl iObjectToOpen ;
    /**
     * Shared memory block for receiving iObjectToOpen from
     * external process.
     */
    QSharedMemory* iSharedMemory ;
#ifdef WIN32
    QLocalServer* iLocalServer ; /**< In WIN32 use named pipe for IPC */
#endif
    TclWrapper* iTclWrapper ; /**< wraps TCL interpreter */
    /** variable used in @ref getFileName wrapper: holds result string */
    QString iGetFileNameResult ; 
    /** variable used in @ref getFileName wrapper: points to dialog */
    QFileDialog* iGetFileNameDialog ; 
    /** variable used in @ref getFileName wrapper: mutex that is locked
     for duration while iGetFileNameDialog is on display */
    QSemaphore iGetFileNameSemaphore ; 
    /** variable used in @ref getFileName wrapper: result from file
     selection dialog */
    bool iGetFileNameSuccess ; 
} ;
#endif

