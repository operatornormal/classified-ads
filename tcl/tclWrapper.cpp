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
#include "tclWrapper.h"
#include "../ui/tclConsole.h"
#include "../log.h"
#include <QCoreApplication>
#ifdef WIN32
#include <tk.h>
#else
#include <tk/tk.h>
#endif
#include "tclCallbacks.h"

extern MController* controllerInstanceEx ;
Tcl_ChannelType stdOutChannel ;
Tcl_ChannelType stdErrChannel ;
const char *KStdOutChannelName = "stdout" ;
const char *KStdErrChannelName = "stderr" ;
const char *KMainWindowTitle = "tcl/tk" ;
const char *KTCLCommandPublishFile = "publishFile" ;
const char *KTCLCommandPublishProfile = "publishProfile" ;
const char *KTCLCommandPublishProfileComment = "publishProfileComment" ;
const char *KTCLCommandPublishClassifiedAd = "publishClassifiedAd" ;
const char *KTCLCommandPublishDbRecord = "publishDbRecord" ;
const char *KTCLCommandSHA1 = "calculateSHA1" ;
const char *KTCLCommandStore = "storeLocalData" ; 
const char *KTCLCommandRetrieve = "retrieveLocalData" ; 
const char *KTCLCommandOpen = "openFileSystemFile" ; 
const char *KTCLCommandSave = "saveFileSystemFile" ; 

TclWrapper::TclWrapper(Model& aModel,
                       MController& aController)
    : iModel(aModel),
      iController(aController),
      iInterp(NULL),
      iDeleteLater(false),
      iConsole (NULL),
      iTclCallbacks(NULL) {
    iMainWindowTitle = KMainWindowTitle ;
    iTclCallbacks = new TclCallbacks(aModel,aController) ;
}

TclWrapper::~TclWrapper() {
    QLOG_STR("TclWrapper::~TclWrapper in") ;
    if ( iInterp ) {
        Tcl_DeleteInterp(iInterp) ;
        iInterp = NULL ;
    }
    if ( iConsole ) {
        delete iConsole ;
    }
    delete iTclCallbacks ;
}

// note that all memory allocation and freeing inside this method
// as TCL interpreter is picky about objects being allocated and
// free'd inside same thread (==heap)
void TclWrapper::run() {
    LOG_STR("TclWrapper::run in") ;
    iNeedsToRun = true ;
    if ( iInterp == NULL ) {
        iInterp = initInterpreter() ;
    }
    if ( iInterp != NULL ) {
        if(initTk(iInterp) &&
                initExtensions(iInterp) &&
                initProgram(iInterp) ) {
            bool mainWindowExists ( true ) ;
            Tcl_Time timeOut;
            timeOut.sec = 0;
            timeOut.usec = 100000; // 10/s
            while ( mainWindowExists ) {
                if ( Tcl_EvalEx(iInterp, "set caTopLevelExistence [ $safeInterp eval \"winfo exists .\"]",
                                -1,TCL_EVAL_GLOBAL) == TCL_OK ) {
                    const char* tclResultCStr (Tcl_GetVar2(iInterp,"caTopLevelExistence",NULL,0) ) ;
                    if ( !( tclResultCStr &&
                            *tclResultCStr == '1') )  {
                        mainWindowExists = false ;
                    }
                } else {
                    mainWindowExists = false ;
                }
                if ( mainWindowExists == true &&
                        iNeedsToRun == false &&
                        iInterp ) {
                    Tcl_EvalEx(iInterp, "$safeInterp eval exit",
                               -1,TCL_EVAL_GLOBAL) ;
                    iNeedsToRun =true ; // set flag back on, one "exit" should suffice
                } else {
                    // running continues, check if we have anything
                    // in queue
                    bool qEmpty(false) ;
                    bool notifyQEmpty(false) ;
                    iModel.lock() ;
                    qEmpty = iScriptQueue.isEmpty() ;
                    notifyQEmpty = iAddedDataModelItems.isEmpty() ;
                    iModel.unlock() ;
                    if ( qEmpty == false || notifyQEmpty == false ) {

                        const char* safeInterpCStr ( Tcl_GetVar2(iInterp,"safeInterp",NULL,0 ) ) ;
                        Tcl_Interp* safeInterp ( Tcl_GetSlave(iInterp, safeInterpCStr ) ) ;
                        if ( safeInterp ) {
                            if ( qEmpty == false ) {
                                iModel.lock() ;
                                QString script ( iScriptQueue.dequeue() ) ;
                                iModel.unlock() ;
                                Tcl_Preserve(safeInterp);
                                int evalResult =
                                    Tcl_EvalEx(safeInterp, script.toUtf8().constData(),
                                               -1,TCL_EVAL_GLOBAL) ;
                                if (Tcl_InterpDeleted(safeInterp)) {
                                    iNeedsToRun = false ;
                                }
                                Tcl_Release(safeInterp);
                                if ( evalResult == TCL_OK ) {
                                    emit consoleOutput("OK") ;
                                } else {
                                    QString errorMessage ( QString::fromUtf8( Tcl_GetStringResult(safeInterp))  ) ;
                                    QLOG_STR(errorMessage) ;
                                    emit consoleOutput(errorMessage) ;
                                }
                            }

                            if ( notifyQEmpty == false && iNeedsToRun ) {
                                notifyInterpreterOfContentReceived ( safeInterp ) ;
                            }
                        }
                    }
                }


                QCoreApplication::processEvents() ;
                Tcl_WaitForEvent(&timeOut) ;
                int workDone = 1 ;
                do {
                    workDone = Tcl_DoOneEvent(TCL_DONT_WAIT);
                    // QLOG_STR("Tcl_DoOneEvent done " + QString::number(workDone)) ;
                } while ( workDone != 0 ) ;
            } // end of while ( mainWindow )
            if ( iInterp ) {
                int evalRet = Tcl_EvalEx(iInterp, "destroy [ winfo children . ]",-1, TCL_EVAL_GLOBAL) ;
                if ( evalRet != TCL_OK ) {
                    QString errorMessage ( Tcl_GetStringResult(iInterp)  ) ;
                    QLOG_STR(errorMessage) ;
                } else {
                    int workDone = 1 ;
                    do {
                        workDone = Tcl_DoOneEvent(TCL_DONT_WAIT);
                        QLOG_STR("Tcl_DoOneEvent at end " + QString::number(workDone)) ;
                    } while ( workDone != 0 ) ;
                }
            }
        }
    }
    if ( iInterp != NULL ) {
        Tcl_DeleteInterp(iInterp) ;
        iInterp = NULL ;
    }
    if ( iDeleteLater ) {
        QLOG_STR("TclWrapper::run calls deleteLater()") ;
        deleteLater() ;
    }
    iMainWindowTitle = KMainWindowTitle ;
    LOG_STR("TclWrapper::run out") ;
    QCoreApplication::processEvents() ;
}

void TclWrapper::stopScript(bool aDeleteLater) {
    iNeedsToRun = false ;
    if ( ( iInterp == NULL || isRunning() == false ) && aDeleteLater ) {
        QLOG_STR("TclWrapper::stopScript calls deleteLater()") ;
        deleteLater() ;
    } else {
        if ( aDeleteLater ) {
            iDeleteLater = true ;
            msleep(1000) ;
        }
    }
}

void TclWrapper::showConsole() {
    QLOG_STR("TclWrapper::showConsole") ;
    if ( !iConsole ) {
        iConsole = new TclConsoleDialog(iController.frontWidget(),
                                        iController) ;
        if ( iConsole ) {
            connect(iConsole, SIGNAL(rejected()),
                    this, SLOT(consoleClosed()), Qt::QueuedConnection) ;
            connect(iConsole, SIGNAL(evalScript(QString,QString*)),
                    this, SLOT(evalScript(QString,QString*)),
                    Qt::QueuedConnection) ;
            connect(this, SIGNAL(consoleOutput(QString)),
                    iConsole, SLOT(consoleOutput(QString)),
                    Qt::QueuedConnection) ;
            iConsole->show() ;
        }
    } else {
        iConsole->setFocus(Qt::MenuBarFocusReason) ;
    }
}

// see also method notifyInterpreterOfContentReceived
void TclWrapper::notifyOfContentReceived(const Hash& aHashOfContent,
        const ProtocolItemType aTypeOfReceivedContent ) {
    iModel.lock() ;
    QPair<Hash, ProtocolItemType> addedItem ( aHashOfContent,
            aTypeOfReceivedContent ) ;
    if ( isRunning() ) {
        iAddedDataModelItems.enqueue(addedItem) ;
    }
    iModel.unlock() ;
}


// see also method  notifyOfContentReceived.
void TclWrapper::notifyInterpreterOfContentReceived(Tcl_Interp* aInterp) {
    bool isQueueEmpty ( false ) ;
    do {
        iModel.lock() ;
        QPair<Hash, ProtocolItemType> item ( iAddedDataModelItems.dequeue() ) ;
        iModel.unlock() ;
        QString protocolItemType ;
        switch ( item.second ) {
        case ClassifiedAd:
        case ClassifiedAdPublish:
        case ClassifiedAd2NdAddr:
            protocolItemType = "classified-ad" ;
            break ;
        case PrivateMessage:
        case PrivateMessagePublish:
            protocolItemType = "private-message" ;
            break ;
        case BinaryBlob:
        case BinaryFilePublish:
            protocolItemType = "binary-file" ;
            break ;
        case UserProfile:
        case UserProfilePublish:
            protocolItemType = "operator-profile" ;
            break ;
        case UserProfileComment:
        case ProfileCommentPublish:
        case UserProfileCommentsForProfile:
            protocolItemType = "profile-comment" ;
            break ;
        case DbRecord:
        case DbRecordPublish:
            protocolItemType = "dbrecord" ;
            break ;
        default:
            QLOG_STR("Unhandled protocol item type in TclWrapper::notifyInterpreterOfContentReceived " + QString::number(item.second)) ;
        }
        if ( protocolItemType.isEmpty() == false ) {
            QString notifyCommand = QString("dataItemChanged %1 %2")
                                    .arg(item.first.toString())
                                    .arg(protocolItemType) ;
            Tcl_Preserve(aInterp);
            Tcl_EvalEx(aInterp,
                       notifyCommand.toUtf8().constData()
                       ,-1, TCL_EVAL_GLOBAL) ;
            if (Tcl_InterpDeleted(aInterp)) {
                iNeedsToRun = false ;
            }
            Tcl_Release(aInterp);
        }
        iModel.lock() ;
        isQueueEmpty = iAddedDataModelItems.isEmpty() ;
        iModel.unlock() ;
    } while ( isQueueEmpty == false ) ;
}

void TclWrapper::consoleClosed() {
    QLOG_STR("TclWrapper::consoleClosed") ;
    iConsole = NULL ;
}

void TclWrapper::setScript(const QString& aScript) {
    iTCLScript = aScript ;
}

void TclWrapper::evalScript(QString aScript,
                            QString* aWindowTitle ) {
    QLOG_STR("TclWrapper::evalScript") ;
    if ( aWindowTitle && aWindowTitle->length() > 0 ) {
        iMainWindowTitle = *aWindowTitle ;
    }
    if ( aScript.length() > 0 ) {
        if ( !isRunning() ) {
            setScript(aScript) ;
            start() ;
        } else {
            iModel.lock() ;
            iScriptQueue.enqueue(aScript) ;
            iModel.unlock() ;
        }
    }
}

Tcl_Interp* TclWrapper::initInterpreter() {
    Tcl_Interp* retval (Tcl_CreateInterp()) ;
    if ( retval ) {
        QString argv0 = QCoreApplication::arguments()[0] ;
        QLOG_STR("TclWrapper::initInterpreter argv0 " + argv0) ;
        Tcl_FindExecutable(argv0.toLatin1().constData()) ;
        int errorCode ( 0 ) ;
        if((errorCode = Tcl_Init(retval) ) == TCL_ERROR) {
            QLOG_STR("Tcl_init returns TCL_ERROR?") ;
            emit error(MController::TCLEvalError,
                       "Tcl_Init:" + QString::number(errorCode)) ;
            Tcl_DeleteInterp(retval) ;
            retval = NULL ;
        } else {
            Tcl_SetSystemEncoding(retval,"utf-8") ;
            Tcl_SetVar(retval,"windowTitleStr",iMainWindowTitle.toUtf8().constData(), 0) ;
            QString initialization("package require Tk\n"
                                   "wm title . $windowTitleStr\n"
                                   "set w .safeTkFrame\n"
                                   "frame $w -container 1;\n"
                                   "pack .safeTkFrame\n"
                                   "set topLevelWindowId [winfo id $w]\n") ;
            errorCode = Tcl_EvalEx(retval,initialization.toUtf8().constData() ,-1, TCL_EVAL_GLOBAL) ;
            QLOG_STR("Tk init " + QString::number(errorCode)) ;
            if ( errorCode != TCL_OK ) {
                QString errorMessage ( Tcl_GetStringResult(retval)  ) ;
                QLOG_STR(errorMessage) ;
                emit error(MController::TCLEvalError,
                           errorMessage) ;
                Tcl_DeleteInterp(retval) ;
                retval = NULL ;
            }
        }
    }
    return retval ;
}

bool TclWrapper::initTk(Tcl_Interp* aInterp) {
    if ( iTCLScript.length() > 0 ) {
        Tk_GetNumMainWindows() ;
        Tcl_SetVar(aInterp,"unTrustedScript",iTCLScript.toUtf8().constData(), 0) ;
        int evalRet = Tcl_EvalEx(aInterp, "set safeInterp [safe::interpCreate]\nsafe::loadTk $safeInterp -use $topLevelWindowId\n",-1, TCL_EVAL_GLOBAL) ;
        if ( evalRet == TCL_OK ) {
            return true ; // script should be running..
        } else {
            QString errorMessage ( Tcl_GetStringResult(aInterp)  ) ;
            QLOG_STR(errorMessage) ;
            emit error(MController::TCLEvalError,
                       errorMessage) ;

            return false ;
        }
    } else {
        return false ; // no script to run
    }
}

bool TclWrapper::initExtensions(Tcl_Interp* aInterp) {
    if (Tcl_PkgProvide(aInterp,
                       "classified-ads",
                       CLASSIFIED_ADS_VERSION) == TCL_ERROR) {
        return false;
    }
    const char* safeInterpCStr ( Tcl_GetVar2(iInterp,"safeInterp",NULL,0 ) ) ;
    Tcl_Interp* safeInterp ( Tcl_GetSlave(aInterp, safeInterpCStr ) ) ;
    if ( safeInterp ) {
        Tcl_SetVar(safeInterp,"profileInUse",
                   iController.profileInUse().toString().toUtf8().constData(), -1) ;
        Tcl_SetVar(safeInterp,"caVersion",
                   CLASSIFIED_ADS_VERSION, -1) ;
        if (( Tcl_CreateObjCommand(safeInterp, "listProfiles",
                                   listItemsCmd, NULL, NULL) == NULL ) ||
            (Tcl_CreateObjCommand(safeInterp, "listAds",
                                  listItemsCmd, NULL, NULL) == NULL ) ||
            (Tcl_CreateObjCommand(safeInterp, "listComments",
                                  listItemsCmd, NULL, NULL) == NULL ) ||
            (Tcl_CreateObjCommand(safeInterp, "getProfile",
                                  getProfileCmd, NULL, NULL) == NULL ) ||
            (Tcl_CreateObjCommand(safeInterp, "getClassifiedAd",
                                  getClassifiedAdCmd, NULL, NULL) == NULL ) ||
            (Tcl_CreateObjCommand(safeInterp, "getProfileComment",
                                  getProfileCommentCmd, NULL, NULL) == NULL ) ||
            (Tcl_CreateObjCommand(safeInterp, "getBinaryFile",
                                  getBinaryFileCmd, NULL, NULL) == NULL ) ||
            (Tcl_CreateObjCommand(safeInterp, "getDbRecord",
                                  getDbRecordCmd, NULL, NULL) == NULL ) ||
            (Tcl_CreateObjCommand(safeInterp, KTCLCommandPublishFile,
                                  publishItemCmd, NULL, NULL) == NULL )||
            (Tcl_CreateObjCommand(safeInterp, KTCLCommandPublishProfile,
                                      publishItemCmd, NULL, NULL) == NULL )||
            (Tcl_CreateObjCommand(safeInterp, KTCLCommandPublishProfileComment,
                                  publishItemCmd, NULL, NULL) == NULL )||
            (Tcl_CreateObjCommand(safeInterp, KTCLCommandPublishClassifiedAd,
                                  publishItemCmd, NULL, NULL) == NULL )||
            (Tcl_CreateObjCommand(safeInterp, KTCLCommandSHA1,
                                  sha1Cmd, NULL, NULL) == NULL )||
            (Tcl_CreateObjCommand(safeInterp, KTCLCommandStore,
                                  storeTCLProgLocalDataCmd, NULL, NULL) == NULL )||
            (Tcl_CreateObjCommand(safeInterp, KTCLCommandRetrieve,
                                  retrieveTCLProgLocalDataCmd, NULL, NULL) == NULL )||
            (Tcl_CreateObjCommand(safeInterp, KTCLCommandOpen,
                                  openFileCmd, NULL, NULL) == NULL )||
            (Tcl_CreateObjCommand(safeInterp, KTCLCommandSave,
                                  saveFileCmd, NULL, NULL) == NULL )||
            (Tcl_CreateObjCommand(safeInterp, KTCLCommandPublishDbRecord,
                                  publishItemCmd, NULL, NULL) == NULL )) {            
            QLOG_STR("Could not create command listProfiles") ;
            return false ;
        } else {
            QLOG_STR("listProfilesImpl created") ;
            // then set up standard output from the safe interpreter.
            // while not strictly necessary, it might be great help
            // in debugging tcl scripts.
            stdOutChannel.typeName = KStdOutChannelName ;
            stdOutChannel.version = TCL_CHANNEL_VERSION_2;
            stdOutChannel.closeProc = closeProc ;
            stdOutChannel.inputProc = inputProc ;
            stdOutChannel.outputProc = outputProc ;
            stdOutChannel.seekProc  = NULL;
            stdOutChannel.setOptionProc  = NULL;
            stdOutChannel.getOptionProc  = NULL;
            stdOutChannel.watchProc = watchProc ;
            stdOutChannel.getHandleProc = getHandleProc ;
            stdOutChannel.close2Proc  = NULL;
            stdOutChannel.blockModeProc   = NULL;
            stdOutChannel.flushProc  = NULL;
            stdOutChannel.handlerProc  = NULL;
            stdOutChannel.wideSeekProc  = NULL;
            stdOutChannel.threadActionProc  = NULL;
            stdOutChannel.truncateProc  = NULL;
            Tcl_Channel stdOutTclChannel =
                Tcl_CreateChannel(&stdOutChannel,
                                  KStdOutChannelName,
                                  (ClientData *)KStdOutChannelName,
                                  TCL_WRITABLE) ;
            Tcl_SetStdChannel(stdOutTclChannel, TCL_STDOUT) ;
            Tcl_RegisterChannel(safeInterp,stdOutTclChannel ) ;
            Tcl_SetChannelBufferSize(stdOutTclChannel, 0); // no buffer
            Tcl_EvalEx(aInterp, "fconfigure stdout -buffering none\ninterp share {} stdout $safeInterp",-1, TCL_EVAL_GLOBAL) ;
            // same for stderr:
            stdErrChannel.typeName = KStdErrChannelName ;
            stdErrChannel.version = TCL_CHANNEL_VERSION_2;
            stdErrChannel.closeProc = closeProc ;
            stdErrChannel.inputProc = inputProc ;
            stdErrChannel.outputProc = outputProc ;
            stdErrChannel.seekProc  = NULL;
            stdErrChannel.setOptionProc  = NULL;
            stdErrChannel.getOptionProc  = NULL;
            stdErrChannel.watchProc = watchProc ;
            stdErrChannel.getHandleProc = getHandleProc ;
            stdErrChannel.close2Proc  = NULL;
            stdErrChannel.blockModeProc   = NULL;
            stdErrChannel.flushProc  = NULL;
            stdErrChannel.handlerProc  = NULL;
            stdErrChannel.wideSeekProc  = NULL;
            stdErrChannel.threadActionProc  = NULL;
            stdErrChannel.truncateProc  = NULL;
            Tcl_Channel stdErrTclChannel =
                Tcl_CreateChannel(&stdErrChannel,
                                  KStdErrChannelName,
                                  (ClientData *)KStdErrChannelName,
                                  TCL_WRITABLE) ;
            Tcl_SetStdChannel(stdErrTclChannel, TCL_STDERR) ;
            Tcl_RegisterChannel(safeInterp,stdErrTclChannel ) ;
            Tcl_SetChannelBufferSize(stdErrTclChannel, 0); // no buffer
            Tcl_EvalEx(aInterp, "fconfigure stderr -buffering none\ninterp share {} stderr $safeInterp",-1, TCL_EVAL_GLOBAL) ;
            // if there are errors in event loop, forward them to stderr,
            // in a way suggested by hypnotoad at #tcl:
            Tcl_EvalEx(safeInterp, "proc bgerror message { puts stderr $message ; puts  $::errorInfo }",-1, TCL_EVAL_GLOBAL) ;
            // create empty procedure for receiving notifications
            // about datamodel content changes
            Tcl_EvalEx(safeInterp,
                       "proc dataItemChanged { itemHash itemType } {\n"
                       "}\n",-1, TCL_EVAL_GLOBAL) ;
            // set (useful) variables
            Tcl_SetVar(safeInterp, 
                       "::systemLocale",
                       QLocale::system().name().toUtf8().constData(),
                       0) ; 
            Tcl_SetVar(safeInterp, 
                       "::classifiedAdsVersion",
                       CLASSIFIED_ADS_VERSION,
                       0) ; 
        }
    } else {
        QLOG_STR("Could not find slave interpreter") ;
        return false ;
    }
    return true;
}

bool TclWrapper::initProgram(Tcl_Interp* aInterp) {
    if ( iTCLScript.length() > 0 ) {
        Tcl_SetVar(iInterp,"unTrustedScript",iTCLScript.toUtf8().constData(), 0) ;
        int evalRet = Tcl_EvalEx(aInterp, "$safeInterp eval $unTrustedScript",-1, TCL_EVAL_GLOBAL) ;
        if ( evalRet == TCL_OK ) {
            return true ; // script should be running..
        } else {
            QString errorMessage ( Tcl_GetStringResult(aInterp)  ) ;
            QLOG_STR(errorMessage) ;
            emit error(MController::TCLEvalError,
                       errorMessage) ;

            return false ;
        }
    } else {
        return false ; // no script to run
    }
}

// static method
int TclWrapper::listItemsCmd(ClientData aCData, Tcl_Interp *aInterp, int aObjc, Tcl_Obj *const aObjv[]) {
    return controllerInstanceEx->tclWrapper()
        .iTclCallbacks->listItemsCmdImpl(aCData,
                                         aInterp,
                                         aObjc,
                                         aObjv) ;
}


// static method
int TclWrapper::getProfileCmd(ClientData aCData, Tcl_Interp *aInterp, int aObjc, Tcl_Obj *const aObjv[]) {
    return controllerInstanceEx->tclWrapper()
        .iTclCallbacks->getProfileCmdImpl(aCData,
                                          aInterp,
                                          aObjc,
                                          aObjv) ;
}

// static method for getting an ad. called by interpreter that does not
// know about object instances, just forwards the call to non-static
// method via global controller instance
int TclWrapper::getClassifiedAdCmd(ClientData aCData, Tcl_Interp *aInterp, int aObjc, Tcl_Obj *const aObjv[]) {
    return controllerInstanceEx->tclWrapper()
        .iTclCallbacks->getClassifiedAdCmdImpl(aCData,
                                               aInterp,
                                               aObjc,
                                               aObjv) ;
}



//     * static method for getting one profile comment

int TclWrapper::getProfileCommentCmd(ClientData aCData, Tcl_Interp *aInterp, int aObjc, Tcl_Obj *const aObjv[]) {
    return controllerInstanceEx->tclWrapper()
        .iTclCallbacks->getProfileCommentCmdImpl(aCData,
                                                 aInterp,
                                                 aObjc,
                                                 aObjv) ;
}


int TclWrapper::getBinaryFileCmd(ClientData aCData, Tcl_Interp *aInterp, int aObjc, Tcl_Obj *const aObjv[]) {
    return controllerInstanceEx->tclWrapper()
        .iTclCallbacks->getBinaryFileCmdImpl(aCData,
                                             aInterp,
                                             aObjc,
                                             aObjv) ;
}

// static method
int TclWrapper::publishItemCmd(ClientData aCData, Tcl_Interp *aInterp, int aObjc, Tcl_Obj *const aObjv[]) {
    return controllerInstanceEx->tclWrapper()
        .iTclCallbacks->publishItemCmdImpl(aCData,
                                           aInterp,
                                           aObjc,
                                           aObjv) ;
}

// static method
int TclWrapper::sha1Cmd(ClientData /* aCData */, Tcl_Interp *aInterp, int aObjc, Tcl_Obj *const aObjv[]) {
    if ( aObjc != 2 ) {
        Tcl_AppendResult(aInterp, "Usage: calculateSHA1 string", NULL);
        return TCL_ERROR ; 
    }
    unsigned char* bytes (NULL);
    QByteArray b ; 
    int length ;
    if ( ( bytes = Tcl_GetByteArrayFromObj(aObjv[1], &length) ) != NULL &&
         length > 0 ) {
        b.append(reinterpret_cast<const char *>(bytes), length) ;
    } else {
        Tcl_AppendResult(aInterp, "Could not get string content?", NULL);
        return TCL_ERROR ;
    }    
    Hash h ; 
    h.calculate(b) ; 
    Tcl_AppendResult(aInterp, h.toString().toUtf8().constData(), NULL);
    return TCL_OK ;
}
// static method
int TclWrapper::storeTCLProgLocalDataCmd(ClientData aCData, Tcl_Interp *aInterp, int aObjc, Tcl_Obj *const aObjv[]) {
    return controllerInstanceEx->tclWrapper()
        .iTclCallbacks->storeTCLProgLocalDataImpl(aCData,
                                                  aInterp,
                                                  aObjc,
                                                  aObjv) ;
}

// static method
int TclWrapper::retrieveTCLProgLocalDataCmd(ClientData aCData, Tcl_Interp *aInterp, int aObjc, Tcl_Obj *const aObjv[]) {
    QLOG_STR("TclWrapper::retrieveTCLProgLocalDataCmd in") ; 
    return controllerInstanceEx->tclWrapper()
        .iTclCallbacks->retrieveTCLProgLocalDataImpl(aCData,
                                                     aInterp,
                                                     aObjc,
                                                     aObjv) ;
}

// static method
int TclWrapper::openFileCmd(ClientData aCData, Tcl_Interp *aInterp, int aObjc, Tcl_Obj *const aObjv[]) {
    QLOG_STR("TclWrapper::openFileCmd in") ; 
    return controllerInstanceEx->tclWrapper()
        .iTclCallbacks->openFileImpl(aCData,
                                     aInterp,
                                     aObjc,
                                     aObjv) ;
}
// static method
int TclWrapper::saveFileCmd(ClientData aCData, Tcl_Interp *aInterp, int aObjc, Tcl_Obj *const aObjv[]) {
    QLOG_STR("TclWrapper::saveFileCmd in") ; 
    return controllerInstanceEx->tclWrapper()
        .iTclCallbacks->saveFileImpl(aCData,
                                     aInterp,
                                     aObjc,
                                     aObjv) ;
}


int TclWrapper::closeProc(
    ClientData /* aInstanceData */,
    Tcl_Interp * /* aInterp */ ) {
    // If the close operation is successful, the procedure should return zero
    QLOG_STR("TclWrapper::closeProc") ;
    return 0 ;
}

/**
 * this method may be used to write data to tcl interpreter.
 */
int TclWrapper::inputProc(
    ClientData /* aInstanceData */,
    char * /* aBuf */,
    int /* aBufSize */,
    int * /* aErrorCodePtr */) {
    // we don't actualy write anything to interpreter, not via this channel
    QLOG_STR("TclWrapper::inputProc") ;
    return 0 ;
}
/**
 * output from tcl interpreter comes to this method. Note this
 * method is not static, but called from static method outputProc()
 */
int TclWrapper::outputProcImpl(
    ClientData /* aInstanceData */,
    const char *aBuf,
    int aToWrite,
    int * /* aErrorCodePtr */) {
    if ( aToWrite > 0 ) {
        if ( *aBuf == '\n' ) {
            QString output ( iStdOutBuffer ) ;
            QLOG_STR("TclWrapper::stdout " + output) ;
            emit consoleOutput(output) ; // signal usually connected to console dlg
            iStdOutBuffer.clear() ;
        } else {
            iStdOutBuffer.append(aBuf, aToWrite) ;
        }
    }
    return aToWrite ;
}

int TclWrapper::outputProc(
    ClientData  aInstanceData ,
    const char *aBuf,
    int aToWrite,
    int *  aErrorCodePtr ) {
    return controllerInstanceEx->tclWrapper().outputProcImpl(aInstanceData,
                                                             aBuf,
                                                             aToWrite,
                                                             aErrorCodePtr) ;
}

void TclWrapper::watchProc(
    ClientData /* instanceData */,
    int /* mask */ ) {
    QLOG_STR("TclWrapper::watchProc") ;
}

int TclWrapper::getHandleProc(
    ClientData /*aInstanceData*/,
    int /*aDirection*/,
    ClientData * /* aHandlePtr */) {
    QLOG_STR("TclWrapper::getHandleProc") ;
    return TCL_ERROR ; // no meaningful "handle" concept here
}

int TclWrapper::getDbRecordCmd(ClientData aCData, Tcl_Interp *aInterp, int aObjc, Tcl_Obj *const aObjv[]) {
    return controllerInstanceEx->tclWrapper()
        .iTclCallbacks->getDbRecordCmdImpl(aCData,
                                           aInterp,
                                           aObjc,
                                           aObjv); 
}
