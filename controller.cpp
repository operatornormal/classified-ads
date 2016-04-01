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

#include <QtGui>
#include <QMenuBar>
#include <QMainWindow>
#include <QMenu>
#include <QMessageBox>
#include <QProgressDialog>
#include <QErrorMessage>
#include <QSharedMemory>
#include "controller.h"
#include "FrontWidget.h"
#include "log.h"
#include "net/node.h"
#include "datamodel/model.h"
#include "datamodel/contentencryptionmodel.h"
#include "net/networklistener.h"
#include "net/networkconnectorengine.h"
#include "net/publishingengine.h"
#include "net/retrievalengine.h"
#ifndef WIN32
#include <unistd.h> // for getpid() 
#endif
#include <signal.h>
#include "ui/passwd_dialog.h"
#include <assert.h>
#include "datamodel/profile.h"
#include "datamodel/profilemodel.h"
#include "datamodel/profilecommentmodel.h"
#include "ui/settings.h"
#include "ui/status.h"
#include "ui/aboutdialog.h"
#include "ui/searchdisplay.h"
#include "datamodel/searchmodel.h"
#include "datamodel/profile.h"
#include "datamodel/profilecomment.h"
#include "datamodel/trusttreemodel.h"
#include "datamodel/binaryfile.h"
#include "net/voicecallengine.h"
#include "datamodel/voicecall.h"

static const char *KPrivateDataContactsSection = "contacts" ;
static const char *KPrivateDataContactsCache = "contactsCache" ;
static const char *KPrivateDataTrustTree = "trustTree" ;

Controller::Controller(QApplication &app) : iWin(NULL),
    iCurrentWidget(NULL),
    iApp ( app ) ,
    iLayout (NULL),
    iFileMenu(NULL),
    iExitAct(NULL),
    iAboutAct(NULL),
    iPwdChangeAct(NULL),
    iProfileDeleteAct(NULL),
    iProfileCreateAct(NULL),
    iProfileSelectAct(NULL),
    iDisplaySettingsAct(NULL),
    iDisplayStatusAct(NULL),
    iDisplaySearchAct(NULL),
    iNode(NULL),
    iModel(NULL),
    iListener(NULL),
    iNetEngine(NULL),
    iPubEngine(NULL),
    iRetrievalEngine(NULL),
    iVoiceCallEngine (NULL),
    iInsideDestructor(false),
    iSharedMemory(NULL) {
    LOG_STR("Controller::Controller constructor out") ;
}

bool Controller::init() {
    QLOG_STR("Controller::init in") ; 
#ifndef WIN32
    int pid ;
    if ( ( pid =  createPidFile() ) != -1 ) {
        // there is old instance, if we have url to open, put it into shared memory 
        // segment before signaling the previously opened instance:
        if (  ( iSharedMemory = new QSharedMemory("classified-ads-"
                                                  + QString::number(pid))) != NULL ) {

            if( iSharedMemory->attach() == true &&
                iSharedMemory->constData() != NULL &&
                iSharedMemory->size() >= 1024 &&
                iObjectToOpen.scheme().length() > 0 ) {
                strcpy((char *)(iSharedMemory->data()),
                       iObjectToOpen.toString().toUtf8().constData()) ;
            } else {
                if (iSharedMemory->constData() != NULL &&
                    iSharedMemory->size() >= 1024 &&
                    iObjectToOpen.scheme().length() == 0 ) {
                    // there is shared memory segment but nothing in iObjectToOpen
                    // so null-terminate the shared memory segment contents
                    char* d ( (char *)(iSharedMemory->data())) ; 
                    *d = '\0' ; 
                }
            }
        } else {
            return false ; 
        }
        LOG_STR2("Signaling old instance %d", pid) ;
        if ( kill(pid, SIGUSR2) != -1 ) {
            // was success
            kill(getpid(), SIGINT) ; // then signal this instance to go away..
            LOG_STR("Instance signaled ") ;
            return  false   ;
        } else {
            // the process did not exist..
            LOG_STR("But process was not there?") ;
            deletePidFile() ;
            createPidFile() ;
        }
    } else {
        // createPidFile() returned -1 meaning that the
        // process was not there: create a shared memory segment:
        if ( iSharedMemory == NULL ) {
            iSharedMemory = new QSharedMemory("classified-ads-"
                                              + QString::number(getpid())) ; 
        }
        if ( iSharedMemory ) {
            if( iSharedMemory->attach() == false ) {
            // 1 kb and some extra
                iSharedMemory->create(1024,QSharedMemory::ReadWrite) ;
            }
        }
    }
#endif
    qRegisterMetaType<MController::CAErrorSituation>("MController::CAErrorSituation");
    qRegisterMetaType<Hash>("Hash");
    qRegisterMetaType<ProtocolItemType>("ProtocolItemType");
    qRegisterMetaType<QHostAddress>("QHostAddress");
    qRegisterMetaType<VoiceCallEngine::CallState>("VoiceCallEngine::CallState");
    qRegisterMetaType<QVector<int> >("QVector<int>"); // used by dataChanged
    iWin = new QMainWindow(NULL) ;
    iWin->setWindowTitle(tr("Classified ads")) ;
    // somehow Qt should calculate size from widgets. it gets too small vertical..
    iWin->setMinimumSize(450,600) ;
    createMenus() ;
    iLayout = new QBoxLayout( QBoxLayout::TopToBottom, NULL ) ;
    QWidget *centralWidget = new QWidget() ;
    iWin->setCentralWidget(centralWidget) ;
    centralWidget->setLayout(iLayout) ;
    iModel = new Model(this);
    iDisplaySearchAct->setEnabled(true) ;
    displayFront() ;  // put UI in place only after model has been instantiated
    iWin->show() ;
    iModel->getNetReqExecutor()->start() ;
    iNode = new Node(iModel->nodeModel().nodeFingerPrint(),
                     iModel->nodeModel().listenPortOfThisNode()) ;
    iNode->setDNSAddr(iModel->nodeModel().getDnsName()) ;
    iListener = new  NetworkListener (this, iModel) ;
    // network listener enumerates network interfaces and sets
    // possible ipv6 addr into iNode() ->
    if (!Connection::Ipv6AddressesEqual(iNode->ipv6Addr(),
                                        KNullIpv6Addr)) {
        LOG_STR("We have IPv6") ;
        iListener->startListen(true) ;
    } else {
        iListener->startListen(false) ; // ipv4 only
    }
    // lets pass iListner to net engine because it is also observer
    // for new connections so lets have it observering the
    // newly-connecting outgoing connections too, it doesn't
    // need to know who originated the connection..
    iNetEngine = new NetworkConnectorEngine(this, iModel,*iListener) ;
    iNetEngine->start() ;
    PasswdDialog *pwd_dialog = new PasswdDialog(iWin, *this,tr("Enter password for protection of your messages:")) ;
    connect(pwd_dialog,
            SIGNAL(  error(MController::CAErrorSituation,
                           const QString&) ),
            this,
            SLOT(handleError(MController::CAErrorSituation,
                             const QString&)),
            Qt::QueuedConnection ) ;
    pwd_dialog->show() ;

    iPubEngine = new PublishingEngine(this, *iModel) ;
    iPubEngine->setInterval(5000); // every 5 seconds

    iRetrievalEngine = new RetrievalEngine (this, *iModel) ;
    iRetrievalEngine->setInterval(5000); // every 5 seconds

    // network listener is also connection status observer so lets
    // have it to signal the pub-engine when connections are opened/closed
    // (also the failed ones that are usually of no interest)
    assert(
        connect(iListener,
                SIGNAL(  nodeConnectionAttemptStatus(Connection::ConnectionState ,
                         const Hash  ) ),
                iPubEngine,
                SLOT( nodeConnectionAttemptStatus(Connection::ConnectionState ,
                        const Hash  )),
                Qt::QueuedConnection )  ) ;
    assert(
        connect(iListener,
                SIGNAL(  nodeConnectionAttemptStatus(Connection::ConnectionState ,
                         const Hash  ) ),
                iNetEngine,
                SLOT( nodeConnectionAttemptStatus(Connection::ConnectionState ,
                        const Hash  )),
                Qt::QueuedConnection )  ) ;
    assert(
        connect(iListener,
                SIGNAL(  nodeConnectionAttemptStatus(Connection::ConnectionState ,
                         const Hash  ) ),
                iRetrievalEngine,
                SLOT( nodeConnectionAttemptStatus(Connection::ConnectionState ,
                        const Hash  )),
                Qt::QueuedConnection )  ) ;
    assert(
        connect(iListener,
                SIGNAL(  nodeConnectionAttemptStatus(Connection::ConnectionState ,
                         const Hash  ) ),
                iModel->getNetReqExecutor(),
                SLOT( nodeConnectionAttemptStatus(Connection::ConnectionState ,
                        const Hash  )),
                Qt::QueuedConnection )  ) ;
    assert(
        connect(iListener,
                SIGNAL(  nodeConnectionAttemptStatus(Connection::ConnectionState ,
                         const Hash  ) ),
                iCurrentWidget,
                SLOT( nodeConnectionAttemptStatus(Connection::ConnectionState ,
                        const Hash  )),
                Qt::QueuedConnection )  ) ;
    assert(
        connect(iRetrievalEngine,
                SIGNAL(   notifyOfContentNotReceived(const Hash& ,
                          const ProtocolItemType  ) ),
                this,
                SLOT(    notifyOfContentNotReceived(const Hash& ,
                         const ProtocolItemType  )),
                Qt::QueuedConnection )  ) ;
    assert(
        connect(this,
                SIGNAL(userProfileSelected(const Hash&)),
                this,
                SLOT(    checkForObjectToOpen(const Hash&)),
                Qt::QueuedConnection));
    // after signals are connected, start publishing and retrieval
    iPubEngine->start() ;
    iRetrievalEngine->start() ;
    // instantiate voice call engine from UI thread, otherwise
    // some transient connection-related thread will do it and 
    // problems start appearing after the thread finishes..
    this->voiceCallEngine() ; 
    /*
    // debug thing:
    iModel->classifiedAdsModel().reIndexAllAdsIntoFTS() ;
    iModel->profileModel().reIndexAllProfilesIntoFTS() ;
    iModel->profileCommentModel().reIndexAllCommentsIntoFTS() ;
    */
    QLOG_STR("Controller::init out") ; 
    return true ; 
}

Controller::~Controller() {
    LOG_STR("Controller::~Controller") ;
    iInsideDestructor = true ; 
    if ( iVoiceCallEngine ) {
        delete iVoiceCallEngine ; 
        iVoiceCallEngine = NULL ; // call status dialog, if open, will ask..
    }
    if ( iListener ) {
        iListener->stopAccepting() ;
    }
    if ( iNetEngine ) {
        iNetEngine->iNeedsToRun = false ;
    }
    if ( iPubEngine ) {
        iPubEngine->iNeedsToRun = false ;
        iPubEngine->stop() ;
        delete iPubEngine ;
        iPubEngine = NULL ;
    }
    LOG_STR("Controller::~Controller 1 pubengine gone") ;
    if ( iRetrievalEngine ) {
        iRetrievalEngine->iNeedsToRun = false ;
        iRetrievalEngine->stop() ;
        delete iRetrievalEngine ;
        iRetrievalEngine = NULL ;
    }
    if ( iNetEngine ) {
#ifndef WIN32
        deletePidFile() ; // if we did not instantiate net engine, we
        // did not create pidfile either
        QLOG_STR("NetEngine->terminate") ;
        iNetEngine->terminate() ;
        iNetEngine->wait(1000) ; // 1 sec max, then just delete..
        QLOG_STR("NetEngine->terminate out") ;
#endif
    }
    LOG_STR("Controller::~Controller 2 netengine gone") ;
    // .. connections reference iListener.
    // so in order to prevent random crash at closing, lets first get rid
    // of connections, only after that delete iListener ;
    if ( iModel ) {
        iModel->closeAllConnections(false) ;
        // after all connections have been instructed to close themselves,
        // wait for some time to give them time to do so..
        unsigned char waitCounter ( 0 ) ;
        while (  ( waitCounter < 60 && iModel->getConnections().count() > 0 ) ||
                 ( waitCounter < 2 ) ) {
            ++waitCounter ;
            QLOG_STR("Waitcounter " + QString::number(waitCounter) +
                     " conn count " + QString::number( iModel->getConnections().count() ) ) ;
            QWaitCondition waitCondition;
            QMutex mutex;
            mutex.lock() ; // waitCondition needs a mutex initially locked
            waitCondition.wait(&mutex, 500);// give other threads a chance..
            mutex.unlock() ;
            QThread::yieldCurrentThread ();
            QCoreApplication::processEvents(QEventLoop::ExcludeUserInputEvents) ;
        }
        iModel->closeAllConnections(true) ; // forcefully delete the remaining
        // now safe to delete listener (and net engine)
        LOG_STR("Controller::~Controller connections closed") ;
        delete iNetEngine ; // will delete also connections opened by net engine
        iNetEngine = NULL ; 
        LOG_STR("Controller::~Controller netengine deleted") ;
        delete iListener ; // will delete also connections received by listener
        iListener = NULL ; 
        LOG_STR("Controller::~Controller listener deleted") ;
        delete iModel ;
    }
    iModel = NULL ; 
    LOG_STR("Controller::~Controller datamodel") ;
    delete iNode ;
    if ( iFileMenu ) {
        delete iFileMenu;
    }
    if ( iExitAct ) {
        delete iExitAct ;
    }
    if ( iAboutAct ) {
        delete iAboutAct ;
    }
    if ( iPwdChangeAct ) {
        delete iPwdChangeAct;
    }
    if ( iProfileDeleteAct) {
        delete iProfileDeleteAct  ;
    }
    if (  iProfileCreateAct ) {
        delete iProfileCreateAct  ;
    }
    if (  iProfileSelectAct ) {
        delete iProfileSelectAct  ;
    }
    if ( iDisplaySettingsAct ) {
        delete iDisplaySettingsAct ;
    }
    if ( iDisplayStatusAct ) {
        delete iDisplayStatusAct ;
    }
    if ( iDisplaySearchAct ) {
        delete iDisplaySearchAct ;
    }
    if ( iWin ) {
        delete iWin ;
        iWin = NULL ;
    }
    if ( iSharedMemory ) {
        iSharedMemory->detach() ; 
        delete iSharedMemory ; 
    }
}

void Controller::userInterfaceAction ( CAUserInterfaceRequest aRequest,
                                       const Hash& aHashConcerned ,
                                       const Hash& aFetchFromNode ,
                                       const QString* aAdditionalInformation ) {
    LOG_STR2("Controller::userInterfaceAction %d", aRequest) ;

    if ( aRequest == ViewProfileDetails ) {

        iModel->lock() ;
        Profile *p = iModel->profileModel().profileByFingerPrint((aHashConcerned) ) ;
        iModel->unlock() ;
        if (p) {
            delete (p) ;
            iCurrentWidget->showDetailsOfProfile(aHashConcerned) ;
        } else {
            // seems like we do not have the profile. lets put a wait-dialog
            // in place and profile fetch request to network..
            NetworkRequestExecutor::NetworkRequestQueueItem req ;
            req.iRequestType =RequestForUserProfile ;
            req.iRequestedItem = aHashConcerned ;
            req.iState = NetworkRequestExecutor::NewRequest ;
            req.iDestinationNode = aFetchFromNode ; // may be KNullHash
            startRetrievingContent(req,false,UserProfile) ;
            userInterfaceAction ( DisplayProgressDialog,
                                  KNullHash ,
                                  KNullHash ) ;
        }
    } else if ( aRequest == ViewCa ) {
        iModel->lock() ;
        CA ca = iModel->classifiedAdsModel().caByHash(aHashConcerned)  ;
        iModel->unlock() ;
        if ( ca.iFingerPrint != KNullHash ) {
            // classified ad was found from local storage
            iCurrentWidget->showClassifiedAd(ca) ;
        } else {
            // classified ad was not found from local storage, begin fetch
            NetworkRequestExecutor::NetworkRequestQueueItem req ;
            req.iRequestType =RequestForClassifiedAd ;
            req.iRequestedItem = aHashConcerned ;
            req.iState = NetworkRequestExecutor::NewRequest ;
            req.iDestinationNode = aFetchFromNode ; // may be KNullHash
            startRetrievingContent(req,false,ClassifiedAd) ;
            userInterfaceAction ( DisplayProgressDialog,
                                  KNullHash ,
                                  KNullHash ) ;
        }
    } else if ( aRequest == ViewProfileComment ) {
        // ok, profile comment is tricky to display because it
        // is two-stage process. first state is finding the
        // profile, that may or may not be locally stored.
        // after profile is found and displayed, we may
        // continue with the comment itself, that again
        // may or may not be found.
        //
        // in order to know the profile, the comment is required
        // first.
        iModel->lock() ;
        ProfileComment* c ( iModel->profileCommentModel().profileCommentByFingerPrint(aHashConcerned) ) ;
        iModel->unlock() ;
        if ( c ) {
            // see if we have the profile too:
            iModel->lock() ;
            Profile *p ( iModel->profileModel().profileByFingerPrint(c->iProfileFingerPrint,
                         true, /* emit */
                         true  /* no image */  ) ) ;
            iModel->unlock() ;
            if ( p ) {
                delete p ;
                p = NULL ;
                iCurrentWidget->showDetailsOfProfile(c->iProfileFingerPrint) ;
                iCurrentWidget->showSingleCommentOfProfile(aHashConcerned) ;
            } else {
                // seems like we do not have the profile. lets put a wait-dialog
                // in place and profile fetch request to network..
                NetworkRequestExecutor::NetworkRequestQueueItem req ;
                req.iRequestType = RequestForUserProfile ;
                req.iRequestedItem = c->iProfileFingerPrint ;
                req.iState = NetworkRequestExecutor::NewRequest ;
                req.iDestinationNode = aFetchFromNode ; // may be KNullHash
                startRetrievingContent(req,false,UserProfile) ;
                iHashOfProfileCommentBeingWaitedFor = aHashConcerned ;
                iNodeForCommentBeingWaitedFor =  aFetchFromNode ;
                userInterfaceAction ( DisplayProgressDialog,
                                      KNullHash ,
                                      KNullHash ) ;
            }
            delete c ;
        } else {
            // comment was not found, begin fetch for that:
            NetworkRequestExecutor::NetworkRequestQueueItem req ;
            req.iRequestType = RequestForProfileComment ;
            req.iRequestedItem = aHashConcerned ;
            req.iState = NetworkRequestExecutor::NewRequest ;
            req.iDestinationNode = aFetchFromNode ; // may be KNullHash
            startRetrievingContent(req,false,UserProfileComment) ;
            userInterfaceAction ( DisplayProgressDialog,
                                  KNullHash ,
                                  KNullHash ) ;
        }

    } else if ( aRequest == DisplayProgressDialog ) {
        QProgressDialog* waitDialog = new QProgressDialog(iCurrentWidget) ;
        waitDialog->setLabelText ( tr("Fetching item from network..") );
        waitDialog->setMaximum(0) ;
        waitDialog->setMinimum(0) ;
        connect(this,SIGNAL(waitDialogToBeDismissed()),
                waitDialog,SLOT(reject())) ;
        waitDialog->exec() ;
    } else if ( aRequest == VoiceCallToNode && iNode != NULL ) {
        QLOG_STR("Voice call request to node " +
                 aHashConcerned.toString() ) ;
        // here let voice call engine handle all the logic:
        if ( !iVoiceCallEngine ) {
            this->voiceCallEngine() ; // has side effect of instantiating one
        }
        VoiceCall callData;
        callData.iCallId = rand() ; 
        callData.iTimeOfCallAttempt = QDateTime::currentDateTimeUtc().toTime_t() ;
        callData.iOkToProceed = true ; 
        quint32 dummyTimeStamp ; 
        iModel->lock() ; QLOG_STR("unlock " + QString(__FILE__) + " "+ QString::number(__LINE__));
        iModel->contentEncryptionModel().PublicKey(iProfileHash,
                                                   callData.iOriginatingOperatorKey,
                                                   &dummyTimeStamp) ; 

        callData.iOriginatingNode = iNode->nodeFingerPrint() ;
        callData.iDestinationNode = iModel->nodeModel().nodeByHash(aHashConcerned)->nodeFingerPrint(); 
        if ( aAdditionalInformation ) {
            callData.iPeerOperatorNick = *aAdditionalInformation ; 
            QLOG_STR("Peer nick was set to " + callData.iPeerOperatorNick ) ; 
        }
        iVoiceCallEngine->insertCallStatusData(callData,
                                               iNode->nodeFingerPrint() ) ; 
        iModel->unlock() ; QLOG_STR("unlock " + QString(__FILE__) + " "+ QString::number(__LINE__));
        callData.iOriginatingNode = KNullHash ; // or it gets deleted
        // calldata destructor will delete callData.iDestinationNode
    } else {
        LOG_STR2("Unhandled user interface request %d", aRequest) ;
    }
    LOG_STR2("Controller::userInterfaceAction out %d", aRequest) ;
}

void Controller::hideUI() {
    if ( iWin ) {
        iWin->hide() ;
    }
}

void Controller::showUI() {
    if ( iWin ) {
        iWin->show() ;
    }
}

void Controller::createMenus() {
    iExitAct = new QAction(tr("E&xit"), this);
    iExitAct->setShortcuts(QKeySequence::Quit);
    iExitAct->setStatusTip(tr("Exit the application"));

    iAboutAct = new QAction(tr("&About"), this);
    iAboutAct->setStatusTip(tr("Show the application's About box"));

    iPwdChangeAct = new QAction(tr("&Change password"), this);
    iPwdChangeAct->setStatusTip(tr("Change password of current profile"));

    iProfileCreateAct = new QAction(tr("Create &new profile"), this);
    iProfileCreateAct->setStatusTip(tr("Makes a brand new user profile"));

    iProfileDeleteAct = new QAction(tr("&Delete current profile"), this);
    iProfileDeleteAct->setStatusTip(tr("Deletes currently open profile"));

    iProfileSelectAct = new QAction(tr("&Select another profile"), this);
    iProfileSelectAct->setStatusTip(tr("If you have multitude of profiles"));

    iDisplaySettingsAct = new QAction(tr("Settings.."), this);
    iDisplaySettingsAct->setStatusTip(tr("Node-wide settings.."));

    iDisplayStatusAct = new QAction(tr("Network status.."), this);

    iDisplaySearchAct = new QAction(tr("Search.."), this);
    connect(iAboutAct, SIGNAL(triggered()), this, SLOT(displayAboutBox()));
    connect(iExitAct, SIGNAL(triggered()), this, SLOT(exitApp()));
    connect(iPwdChangeAct, SIGNAL(triggered()), this, SLOT(changeProfilePasswd()));
    connect(iProfileCreateAct, SIGNAL(triggered()),
            this, SLOT(createProfile()));
    connect(iProfileDeleteAct, SIGNAL(triggered()),
            this, SLOT(deleteProfile()));
    connect(iProfileSelectAct, SIGNAL(triggered()),
            this, SLOT(selectProfile()));
    connect(iDisplaySettingsAct, SIGNAL(triggered()),
            this, SLOT(displaySettings()));
    connect(iDisplayStatusAct, SIGNAL(triggered()),
            this, SLOT(displayStatus()));
    connect(iDisplaySearchAct, SIGNAL(triggered()),
            this, SLOT(displaySearch()));

    iFileMenu = iWin->menuBar()->addMenu(tr("&File"));
    iFileMenu->addAction(iAboutAct);
    iFileMenu->addAction(iPwdChangeAct);
    iFileMenu->addAction(iProfileCreateAct);
    iFileMenu->addAction(iProfileDeleteAct);
    iFileMenu->addAction(iProfileSelectAct);
    iFileMenu->addAction(iDisplaySettingsAct) ;
    iFileMenu->addAction(iDisplayStatusAct) ;
    iFileMenu->addAction(iDisplaySearchAct) ;
    iPwdChangeAct->setDisabled(true) ; // enable when profile is open
    iProfileDeleteAct->setDisabled(true) ; // enable when profile is open
    iFileMenu->addAction(iExitAct);
}

void Controller::exitApp() {
    LOG_STR("Controller::exitApp") ;
    if ( iCurrentWidget && iLayout ) {
        iLayout->removeWidget(iCurrentWidget) ;
        delete iCurrentWidget ;
    }
    // iLayout is owned by iCurrentWidget so it should be
    // deleted there.
    if ( iWin ) {
        iWin->close() ;
    }
}

void Controller::displayAboutBox() {
    if ( iWin ) {
        const QString dlgName ("classified_ads_about_dialog") ;
        AboutDialog *dialog = iWin->findChild<AboutDialog *>(dlgName) ;
        if ( dialog == NULL ) {
            dialog = new AboutDialog(iWin, *this) ;
            dialog->setObjectName(dlgName) ;
            dialog->show() ;
        } else {
            dialog->setFocus(Qt::MenuBarFocusReason) ;
        }
    }
}

void Controller::changeProfilePasswd() {
    if ( iWin) {
        // the last false tells dialog to behave in change-mode instead
        // of query-mode
        PasswdDialog *pwd_dialog = new PasswdDialog(iWin, *this,tr("Enter new password:"),false) ;
        connect(pwd_dialog,
                SIGNAL(  error(MController::CAErrorSituation,
                               const QString&) ),
                this,
                SLOT(handleError(MController::CAErrorSituation,
                                 const QString&)),
                Qt::QueuedConnection ) ;
        pwd_dialog->show() ;
    }
    LOG_STR("Controller::changeProfilePasswd out") ;
}

//  Initiates UI sequence for new profile
void Controller::createProfile() {
    LOG_STR("createProfile in") ;
    iModel->lock() ;
    setProfileInUse(iModel->contentEncryptionModel().generateKeyPair()) ;
    iModel->unlock() ;
    if ( iProfileHash != KNullHash ) {
        changeProfilePasswd() ;
    }
    LOG_STR("createProfile in") ;
}

//  Initiates UI sequence for selecting a profile
void Controller::selectProfile() {
    LOG_STR("selectProfile in") ;
    PasswdDialog *pwd_dialog = new PasswdDialog(iWin, *this,tr("Activate another profile with password")) ;
    connect(pwd_dialog,
            SIGNAL(  error(MController::CAErrorSituation,
                           const QString&) ),
            this,
            SLOT(handleError(MController::CAErrorSituation,
                             const QString&)),
            Qt::QueuedConnection ) ;
    pwd_dialog->show() ;
    LOG_STR("selectProfile in") ;
}

void Controller::displaySettings() {
    LOG_STR("Controller::displaySettings") ;
    const QString dlgName ("classified_ads_settings_dialog") ;
    SettingsDialog *dialog = iWin->findChild<SettingsDialog *>(dlgName) ;
    if ( dialog == NULL ) {
        dialog = new SettingsDialog(iWin, *this) ;
        dialog->setObjectName(dlgName) ;
        connect(dialog,
                SIGNAL(  error(MController::CAErrorSituation,
                               const QString&) ),
                this,
                SLOT(handleError(MController::CAErrorSituation,
                                 const QString&)),
                Qt::QueuedConnection ) ;
        dialog->show() ;
    } else {
        dialog->setFocus(Qt::MenuBarFocusReason) ;
    }
}

void Controller::displayStatus() {
    LOG_STR("Controller::displayStatus") ;
    const QString dlgName ("classified_ads_status_dialog") ;
    StatusDialog *dialog = iWin->findChild<StatusDialog *>(dlgName) ;
    if ( dialog == NULL ) {
        dialog = new StatusDialog(iWin, *this) ;
        dialog->setObjectName(dlgName) ;
        connect(dialog,
                SIGNAL(  error(MController::CAErrorSituation,
                               const QString&) ),
                this,
                SLOT(handleError(MController::CAErrorSituation,
                                 const QString&)),
                Qt::QueuedConnection ) ;
        dialog->show() ;
    } else {
        dialog->setFocus(Qt::MenuBarFocusReason) ;
    }
}

void Controller::displaySearch() {
    LOG_STR("Controller::displaySearch") ;
    const QString dlgName ("classified_ads_search_dialog") ;
    SearchDisplay *dialog = iWin->findChild<SearchDisplay *>(dlgName) ;
    if ( dialog == NULL && iCurrentWidget->selectedProfile()) {
        dialog = new SearchDisplay(iCurrentWidget,
                                   this,
                                   iModel->searchModel(),
                                   *(iCurrentWidget->selectedProfile())) ;
        dialog->setObjectName(dlgName) ;
        connect(dialog,
                SIGNAL(  error(MController::CAErrorSituation,
                               const QString&) ),
                this,
                SLOT(handleError(MController::CAErrorSituation,
                                 const QString&)),
                Qt::QueuedConnection ) ;

        // pointer of current profile is inside dialog -> if that
        // changes, close the dialog ; user needs to re-open search
        // dialog if she switches profile in use
        assert(connect(this,
                       SIGNAL(userProfileSelected(const Hash&)),
                       dialog,
                       SLOT(deleteLater()),
                       Qt::QueuedConnection)  ) ;

        dialog->show() ;
    } else {
        dialog->setFocus(Qt::MenuBarFocusReason) ;
    }
}


// Initiates UI sequence for deleting profile
void Controller::deleteProfile() {
    LOG_STR("deleteProfile in") ;
    QMessageBox msgBox;
    iModel->lock() ;
    int numberOfPrivateKeys = iModel->contentEncryptionModel().listKeys(true,NULL).size() ;
    iModel->unlock() ;
    if ( numberOfPrivateKeys < 2 ) {
        msgBox.setText(tr("Can't delete only profile."));
        msgBox.setStandardButtons( QMessageBox::Ok );
        msgBox.exec();
    } else {
        msgBox.setText(tr("Permanently discard profile?"));
        msgBox.setInformativeText(tr("There will be NO way to access content of this profile later"));
        msgBox.setStandardButtons( QMessageBox::Ok | QMessageBox::Cancel);
        msgBox.setDefaultButton(QMessageBox::Cancel);
        int ret = msgBox.exec();
        if ( ret == QMessageBox::Ok  ) {
            LOG_STR("delete profile..") ;
            iModel->lock() ;
            bool deletiaResult = iModel->contentEncryptionModel().deleteKeyPair(iProfileHash)  ;
            iModel->unlock() ;
            if ( deletiaResult ) {
                PasswdDialog *pwd_dialog = new PasswdDialog(iWin, *this,tr("Activate another profile with password")) ;
                connect(pwd_dialog,
                        SIGNAL(  error(MController::CAErrorSituation,
                                       const QString&) ),
                        this,
                        SLOT(handleError(MController::CAErrorSituation,
                                         const QString&)),
                        Qt::QueuedConnection ) ;
                pwd_dialog->show() ;
            }
        } else {
            LOG_STR("user is unsure") ;
        }
    }
}

void Controller::displayFront() {
    LOG_STR("displayFront") ;
    if ( iCurrentWidget ) {
        iLayout->removeWidget(iCurrentWidget) ;
        delete iCurrentWidget ;
    }
    iLayout->addWidget(iCurrentWidget = new FrontWidget(this,*iWin)) ;
    connect(iCurrentWidget,
            SIGNAL(  error(MController::CAErrorSituation,
                           const QString&) ),
            this,
            SLOT(handleError(MController::CAErrorSituation,
                             const QString&)),
            Qt::QueuedConnection ) ;
    //
    // important:
    // connection is queued. in practice this signal is emitted at
    // situation where datamodel is locked.
    // UI in question will immediately try to obtain lock to datamodel
    // so we would end up in deadlock situation. Use queued signal
    // here so call stack of the caller is left to collapse
    // until this signal is delivered
    //
    assert(connect(this,
                   SIGNAL(userProfileSelected(const Hash&)),
                   iCurrentWidget,
                   SLOT(userProfileSelected(const Hash&)),
                   Qt::QueuedConnection)  ) ;
    LOG_STR("displayFront out") ;
}

void Controller::handleError(MController::CAErrorSituation aError,
                             const QString& aExplanation) {
    LOG_STR2("Error enum value %d", aError) ;
    QLOG_STR(aExplanation) ;
    switch ( aError ) {
    case OwnCertNotFound:
        QMessageBox::about(iWin,tr("Cant load node cert or key cert"),
                           aExplanation);
        LOG_STR("Cant load node cert or key cert") ;
        break ;
    case FileOperationError:  // type of non-fatal error
        QMessageBox::about(iWin,tr("File error"),
                           aExplanation);
        LOG_STR("File error " + aExplanation ) ;
        break ;
    case DataBaseNotMountable:
        QMessageBox::about(iWin,tr("Database error"),
                           aExplanation);
        LOG_STR("Can't open DB") ;
        QApplication::quit() ;
        break ;
    case ContentEncryptionError :
        QMessageBox::about(iWin,tr("Cryptographic module"),
                           aExplanation);
        QLOG_STR("ContentEncryptionError " + aExplanation ) ;
        break ;
    case DbTransactionError :
        QMessageBox::about(iWin,tr("Database module"),
                           aExplanation);
        QLOG_STR("DbTransactionError " + aExplanation ) ;
        break ;
    case BadPassword :
        QMessageBox::about(iWin,tr("Cryptographic module"),
                           tr("Bad password"));
        QLOG_STR("DbTransactionError " + aExplanation ) ;
        break ;
    }
}

Node& Controller::getNode() const {
    return *iNode ;
}

NetworkListener *Controller::networkListener() const {
    return iListener ;
}

Model& Controller::model() const {
    return *iModel ;
}

int Controller::createPidFile() {
    int retval = -1 ;
#ifndef WIN32
    QString path(QDir::home().path());
    path.append(QDir::separator()).append(".classified_ads");
    path.append(QDir::separator()).append("instance.pid") ;
    bool ok = false ;
    if (QFile(path).exists()) {
        QFile f(path) ;
        f.open(QIODevice::ReadOnly) ;
        QByteArray arr = f.readAll() ;
        QLatin1String content8bit(arr);
        QString content (content8bit) ;
        retval = content.toInt(&ok, 10) ;
    }
    if (!ok ) {
        QFile f(path) ;
        f.open(QIODevice::WriteOnly) ;
        QString content = QString::number(getpid());
        QByteArray contentBytes(content.toLatin1()) ;
        f.write(contentBytes) ;
        f.close() ;
        retval = -1 ;
    }
#endif
    return retval ;
}

void Controller::deletePidFile() {
#ifndef WIN32
    QString path(QDir::home().path());
    path.append(QDir::separator()).append(".classified_ads");
    path.append(QDir::separator()).append("instance.pid") ;
    QFile(path).remove() ;
#endif
}

void Controller::setContentKeyPasswd(QString aPasswd) {
    iContentKeyPasswd = aPasswd ;
}

QString Controller::contentKeyPasswd()  const {
    return iContentKeyPasswd ;
}

void Controller::setProfileInUse(const Hash& aProfileHash) {
    if ( iProfileHash != KNullHash ) {
        storePrivateDataOfSelectedProfile() ;
    }
    iProfileHash = aProfileHash ;
    iPwdChangeAct->setDisabled(false) ;
    int numberOfPrivateKeys = iModel->contentEncryptionModel().listKeys(true,NULL).size() ;
    if ( numberOfPrivateKeys > 1 ) {
        iProfileDeleteAct->setDisabled(false) ;
        iProfileSelectAct->setDisabled(false) ;
    } else {
        iProfileDeleteAct->setDisabled(true) ;
        iProfileSelectAct->setDisabled(true) ;
    }
    reStorePrivateDataOfSelectedProfile() ;
    emit userProfileSelected(iProfileHash) ; // must be Qt::QueuedConnection
}


const Hash& Controller::profileInUse() {
    return iProfileHash ;
}



// signal of received content
void Controller::notifyOfContentReceived(const Hash& aHashOfContent,
        const ProtocolItemType aTypeOfReceivedContent) {
    LOG_STR2("Controller::notifyOfContentReceived type : %d", aTypeOfReceivedContent);
    if ( iCurrentWidget ) {
        iCurrentWidget->receiveNotifyOfContentReceived( aHashOfContent,
                aTypeOfReceivedContent) ;
        if ( aHashOfContent == iHashOfObjectBeingWaitedFor &&
                aTypeOfReceivedContent == iTypeOfObjectBeingWaitedFor ) {
            emit waitDialogToBeDismissed() ;
            iHashOfObjectBeingWaitedFor= KNullHash ; // do not wait any more
            if ( aTypeOfReceivedContent == UserProfile ) {
                iCurrentWidget->showDetailsOfProfile(aHashOfContent) ;
                if ( iHashOfProfileCommentBeingWaitedFor != KNullHash ) {
                    // stage 2: we were waiting for profile but actually
                    // user wanted to see an ad: as we now have the profile,
                    // lets ask for comment too
                    //
                    // lets check if we have the comment:
                    ProfileComment* c ( iModel->profileCommentModel().profileCommentByFingerPrint(iHashOfProfileCommentBeingWaitedFor) ) ;
                    if ( c == NULL ) {
                        NetworkRequestExecutor::NetworkRequestQueueItem req ;
                        req.iRequestType = RequestForProfileComment ;
                        req.iRequestedItem = iHashOfProfileCommentBeingWaitedFor ;
                        iHashOfProfileCommentBeingWaitedFor = KNullHash ;
                        req.iState = NetworkRequestExecutor::NewRequest ;
                        req.iDestinationNode = iNodeForCommentBeingWaitedFor ;
                        iNodeForCommentBeingWaitedFor = KNullHash ;
                        startRetrievingContent(req,false,UserProfileComment) ;
                        userInterfaceAction ( DisplayProgressDialog,
                                              KNullHash ,
                                              KNullHash ) ;
                    } else {
                        iCurrentWidget->showSingleCommentOfProfile(iHashOfProfileCommentBeingWaitedFor) ;
                        delete c ;
                        iHashOfProfileCommentBeingWaitedFor = KNullHash ;
                    }
                }
            } else if ( aTypeOfReceivedContent == ClassifiedAd ) {
                iModel->lock() ;
                CA ca = iModel->classifiedAdsModel().caByHash(aHashOfContent)  ;
                iModel->unlock() ;
                if ( ca.iFingerPrint != KNullHash ) {
                    iCurrentWidget->showClassifiedAd(ca) ;
                }
            } else if ( aTypeOfReceivedContent == BinaryBlob ) {
                iCurrentWidget->openBinaryFile(aHashOfContent,false) ;
            }
        }
    }
    iRetrievalEngine->notifyOfContentReceived(aHashOfContent,
            aTypeOfReceivedContent) ;
    // offer profiles to trust list
    if ( aTypeOfReceivedContent == UserProfile ) {
        iModel->trustTreeModel()->contentReceived(aHashOfContent,
                aTypeOfReceivedContent) ;
    }
}

void Controller::notifyOfContentReceived(const Hash& aHashOfContent,
        const Hash& aHashOfClassification,
        const ProtocolItemType aTypeOfReceivedContent) {
    LOG_STR2("Controller::notifyOfContentReceived v2 type : %d", aTypeOfReceivedContent);
    if ( iCurrentWidget ) {
        iCurrentWidget->receiveNotifyOfContentReceived( aHashOfContent,
                aHashOfClassification,
                aTypeOfReceivedContent) ;
        if ( aHashOfContent == iHashOfObjectBeingWaitedFor &&
                aTypeOfReceivedContent == iTypeOfObjectBeingWaitedFor ) {
            emit waitDialogToBeDismissed() ;
            iHashOfObjectBeingWaitedFor= KNullHash ; // do not wait any more
            if ( aTypeOfReceivedContent == UserProfile ) {
                iCurrentWidget->showDetailsOfProfile(aHashOfContent) ;
                if ( iHashOfProfileCommentBeingWaitedFor != KNullHash ) {
                    ProfileComment* c ( iModel->profileCommentModel().profileCommentByFingerPrint(iHashOfProfileCommentBeingWaitedFor) ) ;
                    if ( c ) {
                        // we were waiting for profile but actually want to
                        // to display one comment from that profile. we now
                        // have the profile+comment
                        iCurrentWidget->showSingleCommentOfProfile(iHashOfProfileCommentBeingWaitedFor) ;
                        iHashOfProfileCommentBeingWaitedFor = KNullHash ;
                        delete c ;
                    } else {

                        // stage 2: we were waiting for profile but actually
                        // user wanted to see an ad: as we now have the profile,
                        // lets ask for comment too
                        NetworkRequestExecutor::NetworkRequestQueueItem req ;
                        req.iRequestType = RequestForProfileComment ;
                        req.iRequestedItem = iHashOfProfileCommentBeingWaitedFor ;
                        iHashOfProfileCommentBeingWaitedFor = KNullHash ;
                        req.iState = NetworkRequestExecutor::NewRequest ;
                        req.iDestinationNode = iNodeForCommentBeingWaitedFor ;
                        iNodeForCommentBeingWaitedFor = KNullHash ;
                        startRetrievingContent(req,false,UserProfileComment) ;
                        userInterfaceAction ( DisplayProgressDialog,
                                              KNullHash ,
                                              KNullHash ) ;
                    }
                }
            } else if ( aTypeOfReceivedContent == ClassifiedAd ) {
                iModel->lock() ;
                CA ca = iModel->classifiedAdsModel().caByHash(aHashOfContent)  ;
                iModel->unlock() ;
                if ( ca.iFingerPrint != KNullHash ) {
                    iCurrentWidget->showClassifiedAd(ca) ;
                }
            }
        }
    }
    iRetrievalEngine->notifyOfContentReceived(aHashOfContent,
            aTypeOfReceivedContent) ;
}
void Controller::notifyOfContentNotReceived(const Hash& aHashOfContent,
        const ProtocolItemType
#ifdef DEBUG
        aTypeOfNotReceivdContent  // needed only in debug build
#else
        /* aTypeOfNotReceivdContent  */
#endif
                                           ) {
    LOG_STR2("Controller::notifyOfContentNotReceived type : %d", aTypeOfNotReceivdContent);
    if ( iHashOfObjectBeingWaitedFor == aHashOfContent ) {
        iHashOfObjectBeingWaitedFor= KNullHash ;
        iHashOfProfileCommentBeingWaitedFor = KNullHash ;
        emit waitDialogToBeDismissed() ;
        QErrorMessage* errorDialog = new QErrorMessage(iCurrentWidget) ;
        errorDialog->showMessage ( tr("Could not find item from network..") );
    }
}

void Controller::startRetrievingContent(NetworkRequestExecutor::NetworkRequestQueueItem aReq,
                                        bool aIsBackgroundDl,
                                        ProtocolItemType aTypeOfExpectedObject) {
    // here queue the item into retrieval-engine
    iRetrievalEngine->startRetrieving(aReq,
                                      aIsBackgroundDl) ;
    iHashOfObjectBeingWaitedFor = aReq.iRequestedItem ;
    iTypeOfObjectBeingWaitedFor = aTypeOfExpectedObject ;
}

void Controller::storePrivateDataOfSelectedProfile(bool aPublishTrustListToo) {

    if ( iProfileHash!= KNullHash ) {
        QMap<QString,QVariant> m ;
        m.insert(KPrivateDataContactsSection,
                 iCurrentWidget->contactDataOfSelectedProfile()) ;
        if ( iHashDisplaynameMapping.size() > 0 ) {
            QMap<QString,QVariant> cache ;
            QMapIterator<Hash, QString> i(iHashDisplaynameMapping);
            while (i.hasNext()) {
                i.next();
                cache.insert(i.value(), i.key().toQVariant()) ;
            }
            m.insert(KPrivateDataContactsCache,
                     cache) ;
        }
        m.insert(KPrivateDataTrustTree,
                 iModel->trustTreeModel()->trustTreeSettings()) ;

        iModel->profileModel().setPrivateDataForProfile(iProfileHash,
                QVariant(m)) ;

        if ( iCurrentWidget->selectedProfile() ) {
            iCurrentWidget->selectedProfile()->iTrustList.clear() ;
            iCurrentWidget->selectedProfile()->iTrustList.append(iCurrentWidget->trustListOfSelectedProfile()) ;
            if ( aPublishTrustListToo ) {
                iCurrentWidget->selectedProfile()->iTimeOfPublish = QDateTime::currentDateTimeUtc().toTime_t() ;
                iModel->profileModel().publishProfile(*(iCurrentWidget->selectedProfile()));
            }
        }
    } else {
        LOG_STR("No selected profile while storePrivateDataOfSelectedProfile") ;
    }
}

void Controller::reStorePrivateDataOfSelectedProfile() {
    if ( iProfileHash!= KNullHash ) {
        QMap<QString,QVariant> privateData = iModel->profileModel().privateDataOfProfile(iProfileHash).toMap() ;
        if (privateData.contains(KPrivateDataContactsSection)) {
            QVariantList listOfContacts (privateData[KPrivateDataContactsSection].toList()) ;
            iCurrentWidget->setContactDataOfSelectedProfile(listOfContacts) ;
        }
        if (privateData.contains(KPrivateDataTrustTree)) {
            iModel->trustTreeModel()->initModel(privateData[KPrivateDataTrustTree]) ;
        } else {
            iModel->trustTreeModel()->initModel(QVariant()) ;
        }

        iHashDisplaynameMapping.clear() ;
        if (privateData.contains(KPrivateDataContactsCache)) {
            QVariantMap contactsCache (privateData[KPrivateDataContactsCache].toMap()) ;

            QMapIterator<QString,QVariant> i(contactsCache);
            while (i.hasNext()) {
                i.next();
                Hash profileKey ;
                profileKey.fromQVariant(i.value()) ;
                QLOG_STR("Inserting into iHashDisplaynameMapping key = " + profileKey.toString() + " name " + i.key()) ;
                if ( profileKey.toString() != i.key() ) {
                    iHashDisplaynameMapping.insert(profileKey,i.key() ) ; // not how i.key() is profile name. this is
                    // because in QVariantMap the key is hard-coded to be a QString
                } else {
                    QLOG_STR("..Or not, because name was profile hash") ;
                }
            }
        }
    }
}

bool Controller::isContactInContactList(const Hash& aFingerPrint) const {
    return iCurrentWidget->isContactInContactList(aFingerPrint) ;
}

QString Controller::displayableNameForProfile(const Hash& aProfileFingerPrint) const {
    return iHashDisplaynameMapping.value(aProfileFingerPrint, aProfileFingerPrint.toString()) ;
}

void Controller::offerDisplayNameForProfile(const Hash& aProfileFingerPrint,
        const QString& aDisplayName,
        const bool aStoreInPersistenStorage) {
    if ( aProfileFingerPrint.toString() != aDisplayName ) {
        if (aDisplayName.length() > 0 && ( ! iHashDisplaynameMapping.contains(aProfileFingerPrint )) ) {
            iHashDisplaynameMapping.insert(aProfileFingerPrint, aDisplayName) ;
            if ( aStoreInPersistenStorage ) {
                storePrivateDataOfSelectedProfile() ;
            }
        } else if ( iHashDisplaynameMapping.contains(aProfileFingerPrint ) &&
                    aDisplayName.length() > 0 ) {
            QString& contentAsDescriptor ( iHashDisplaynameMapping[aProfileFingerPrint] ) ;
            contentAsDescriptor = aDisplayName ;
        }
    } else {
        QLOG_STR("Offered display name was profile hash -> discarding") ;
    }
}

void Controller::displayFileInfoOnUi(const BinaryFile& aFileMetadata) {
    QMessageBox infoMessage ;
    QStringList info ;
    info.append(tr("SHA1: ")) ;
    info.append(aFileMetadata.iFingerPrint.toString()) ;
    info.append("\n") ;

    if ( aFileMetadata.iMimeType.length() > 0 ) {
        info.append(tr("Mime-Type: ")) ;
        info.append(aFileMetadata.iMimeType) ;
        info.append("\n") ;
    }

    if ( aFileMetadata.iDescription.length() > 0 ) {
        info.append(tr("Description: ")) ;
        info.append(aFileMetadata.iDescription) ;
        info.append("\n") ;
    }

    if ( aFileMetadata.iOwner.length() > 0 ) {
        info.append(tr("Publisher: ")) ;
        info.append(aFileMetadata.iOwner) ;
        info.append("\n") ;
    }

    if ( aFileMetadata.iContentOwner.length() > 0 ) {
        info.append(tr("Content owner: ")) ;
        info.append(aFileMetadata.iContentOwner) ;
        info.append("\n") ;
    }

    if ( aFileMetadata.iLicense.length() > 0 ) {
        info.append(tr("License: ")) ;
        info.append(aFileMetadata.iLicense) ;
        info.append("\n") ;
    }

    if ( aFileMetadata.iFileName.length() > 0 ) {
        info.append(tr("Name: ")) ;
        info.append(aFileMetadata.iFileName) ;
        info.append("\n") ;
    }

    QDateTime d ;
    d.setTime_t(aFileMetadata.iTimeOfPublish) ;
    info.append(tr("Date: ")) ;
    info.append(d.toString(Qt::SystemLocaleShortDate)) ;
    info.append("\n") ;

    infoMessage.setText(info.join("")) ;
    infoMessage.setStandardButtons( QMessageBox::Ok );
    infoMessage.exec() ;
}

void Controller::sendProfileUpdateQuery(const Hash& aProfileFingerPrint,
                                        const Hash& aProfileNodeFingerPrint ) {
    NetworkRequestExecutor::NetworkRequestQueueItem req ;
    req.iRequestType = RequestForProfilePoll ;
    req.iRequestedItem = aProfileFingerPrint ;
    req.iState = NetworkRequestExecutor::NewRequest ;
    req.iDestinationNode = aProfileNodeFingerPrint ;
    // try to get time of last poll..
    iModel->lock() ;
    req.iTimeStampOfItem = iModel->profileModel().getLastProfileUpdateTime (aProfileFingerPrint) ;
    iModel->addNetworkRequest(req) ;
    iModel->unlock() ;
}

VoiceCallEngine* Controller::voiceCallEngine() {
    if ( !iVoiceCallEngine && iInsideDestructor == false ) {
        iVoiceCallEngine = new VoiceCallEngine(*this,
                                               *iModel) ; 
        assert(
            connect(iVoiceCallEngine,
                    SIGNAL(   callStateChanged(quint32,
                                               VoiceCallEngine::CallState) ),
                    iCurrentWidget,
                    SLOT(    callStateChanged(quint32,
                                              VoiceCallEngine::CallState) ),
                    Qt::QueuedConnection )  ) ;
        assert(
            connect(iListener,
                    SIGNAL(  nodeConnectionAttemptStatus(Connection::ConnectionState ,
                                                         const Hash  ) ),
                    iVoiceCallEngine,
                    SLOT( nodeConnectionAttemptStatus(Connection::ConnectionState ,
                                                      const Hash  )),
                    Qt::QueuedConnection )  ) ;
    }
    return iVoiceCallEngine ;
} 

MVoiceCallEngine* Controller::voiceCallEngineInterface() {
    return this->voiceCallEngine() ; 
}

void Controller::addObjectToOpen(QUrl aClassifiedAdsObject) {
    iObjectToOpen = aClassifiedAdsObject ; 
}

void Controller::checkForObjectToOpen(const Hash& /* aIgnored */) {
    if ( iObjectToOpen.scheme().length() > 0 && iCurrentWidget ) {
        iCurrentWidget->linkActivated(iObjectToOpen.toString()) ; 
    }
    iObjectToOpen.setScheme("") ; 
}

void Controller::checkForSharedMemoryContents() {
    if (iSharedMemory &&
        iSharedMemory->constData() != NULL &&
        iSharedMemory->size() >= 1024 ) {
        QString urlStr ( QString::fromUtf8 ( reinterpret_cast<const char *>(iSharedMemory->constData()) ) ) ; 
        QLOG_STR("checkForSharedMemoryContents content = " + 
                 urlStr) ; 
        QUrl url (urlStr) ;
        if ( url.scheme() == "caprofile" ||
                url.scheme() == "caad" ||
                url.scheme() == "cacomment" ||
                url.scheme() == "cablob"  ) {
            if ( iProfileHash == KNullHash ) {
                // there is no profile in use
                addObjectToOpen(url) ; // <- goes into wait list
            } else {
                iObjectToOpen = url ; 
                emit userProfileSelected(iProfileHash) ; 
            }
        }
    }
}
