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


#ifndef TCLWRAPPER_H
#define TCLWRAPPER_H
#include <QThread>
#include <QQueue>
#include "../mcontroller.h"
#include <tcl.h>

class Model ;
class MController ;
class TclConsoleDialog ;
class TclCallbacks ;

/**
 * @brief Class providing TCL interpreter services
 *
 * This class wraps TCL interpreter in safe mode. This class
 * is intended to be used so that, this is instantiated once
 * and deleted at application shutdown. One TCL program may be
 * running at same time - restriction of Tk is that one
 * instance per process only.
 *
 * This is a thread and it is supposed to be used so that
 * class is instantiated, setScript() method is called,
 * followed by start(). Start may be called multiple times,
 * calling start() when thread is already executing has
 * no effect.
 *
 * Additionally this implements commands for accessing profiles
 * and classified ads from within TCL interpreter, see for
 * example @ref listItemsCmd or @ref getProfileCmd that appear
 * as commands "listProfiles" and "getProfile" inside interpreter.
 */
class TclWrapper : public QThread {
    Q_OBJECT
public:
    TclWrapper(Model& aModel,
               MController& aController);

    /**
     * Method for tearing down the interpreter
     */
    ~TclWrapper() ;
    /**
     * stop possible running script.
     *
     * @aDeleteLater if set to true, will cause the instance to
     * delete itself after interpreter is deleted
     */
    void stopScript(bool aDeleteLater = false ) ;
    void setScript(const QString& aScript) ; /**< TCL to evaluate, called before start */
    /**
     * Returns the script currently being evaluated
     */
    const QString& currentProgram() const { return iTCLScript ; } ;
    void showConsole() ; /**< Displays TCL interpreter console dialog */
    /**
     * method for receiving notifications about data item
     * additions to data model. this is called for example when
     * new classified as are added. See also
     * @ref notifyInterpreterOfContentReceived.
     */
    void notifyOfContentReceived(const Hash& aHashOfContent,
                                 const ProtocolItemType aTypeOfReceivedContent ) ;
public slots:
    void run() ;

signals:
    /**
     * this is not method but signal ; if in error, get emit()ted
     */
    void error(MController::CAErrorSituation aError,
               const QString& aExplanation) ;
    /**
     * This slot may be connected to display of TCL interpreter
     * textual output
     */
    void consoleOutput(QString aOutput) ;
public slots:
    void consoleClosed() ;
    /**
     * this slot may be used to inject more TCL to be
     * interpreted
     * @param aScript is the TCL script to be evaluated
     * @param aMainWindowTitle contains optional main window title.
     *        This is used only in case interpreter is not yet
     *        running and this may then be used to set the title
     *        of the tk main window during interpreter initialization.
     */
    void evalScript(QString aScript,
                    QString* aMainWindowTitle = NULL ) ;
private: // methods
    /**
     * instantiates TCL interpreter and loads tk
     * @return interpreter or NULL
     */
    Tcl_Interp* initInterpreter() ;
    /**
     * Tcl/Tk toolkit init
     * @return true on success
     */
    bool  initTk(Tcl_Interp* aInterp) ;
    /**
     * Initialize extensions to TCL tk provided by classified ads.
     * This includes accessor commands to CA data like articles,
     * profiles and messages.
     *
     * @return true on success
     */
    bool  initExtensions(Tcl_Interp* aInterp) ;

    /**
     * Evaluates in safe interpreter the program given by method
     * @ref setScript.
     *
     * @return true on success
     */
    bool  initProgram(Tcl_Interp* aInterp) ;

    /**
     * Tcl extension method: when TCL scripts asks for list of
     * profiles or classified ads or profile comments, it invokes
     * this method. See @initExtensions
     * where this method is added into TCL interpreters repertoire
     * @param aCData clientdata from tcl interpreter
     * @param aInterp pointer to calling TCL interpreter
     * @param aObjc number of items in array aObjv
     * @param aObjv command arguments. First is name of the command
     *        itself, like "listProfiles" or "listAds", 2nd is search
     *        string.
     * @return TCL_OK on success. As side effect it calls Tcl_SetObjResult that
     *         is a list of dictionaries, each dictionary has fingerprint as the
     *         key and displayName as the value. If no object matches, empty list
     *         is returned.
     */
    static int listItemsCmd(ClientData aCData, Tcl_Interp *aInterp, int aObjc, Tcl_Obj *const aObjv[]) ;

    /**
     * static method for getting profile details
     * @param aObjv should contain 40-byte long hash-string in aObjv[1]
     *        that tells which profile to fetch.
     * @param return TCL_OK if ok, the actual TCL object returned is a TCL dictionary.
     */
    static int getProfileCmd(ClientData aCData, Tcl_Interp *aInterp, int aObjc, Tcl_Obj *const aObjv[]) ;


    /**
     * static method for getting one classified ad
     * @param aObjv should contain 40-byte long hash-string in aObjv[1]
     *        that tells which ad to fetch.
     * @param return TCL_OK if ok, the actual TCL object returned is a TCL dictionary.
     */
    static int getClassifiedAdCmd(ClientData aCData, Tcl_Interp *aInterp, int aObjc, Tcl_Obj *const aObjv[]) ;


    /**
     * static method for getting one profile comment
     * @param aObjv should contain 40-byte long hash-string in aObjv[1]
     *        that tells which comment to fetch.
     * @param return TCL_OK if ok, the actual TCL object returned is a TCL dictionary.
     */
    static int getProfileCommentCmd(ClientData aCData, Tcl_Interp *aInterp, int aObjc, Tcl_Obj *const aObjv[]) ;


    /**
     * static method for getting binary file
     * @param aObjv should contain 40-byte long hash-string in aObjv[1]
     *        that tells which file to fetch.
     * @param return TCL_OK if ok, the actual TCL object returned is a TCL dictionary.
     */
    static int getBinaryFileCmd(ClientData aCData, Tcl_Interp *aInterp, int aObjc, Tcl_Obj *const aObjv[]) ;

    /**
     * static method for getting db record
     * @param aObjv should contain dictionary expressing the search
     *              conditions. Same dictionary keys are used that are 
     *              in use when db record is published from TCL app.
     * @return If db records satisfying the conditions were found from
     *         local storage, they're returned synchronously as return
     *         value to this call, using Tcl_SetObjResult mechanism. 
     *         Search is sent to remote nodes also and as remote nodes
     *         return search results, new db records may be added to 
     *         database and TCL app running may receive notifications
     *         later concerning db records originally queries but
     *         arriving later. 
     * @param return TCL_OK if ok, the actual TCL object returned is a TCL dictionary.
     */
    static int getDbRecordCmd(ClientData aCData, Tcl_Interp *aInterp, int aObjc, Tcl_Obj *const aObjv[]) ;


    /**
     * Tcl extension method for publishing an item. This will be
     * called by several in-TCL commands, for instance "publishFile",
     * "publishComment" or "publishProfile". The TCL command
     * will be in aObjv[0] and aObjv[1] will contain the actual
     * object to be published.
     * @param aCData clientdata from tcl interpreter. Not used.
     * @param aInterp pointer to calling TCL interpreter
     * @param aObjc number of items in array aObjv
     * @param aObjv command arguments. First is name of the command
     *        itself, like "publishFile", 2nd is actual object
     * @return TCL_OK on success.
     */
    static int publishItemCmd(ClientData aCData, Tcl_Interp *aInterp, int aObjc, Tcl_Obj *const aObjv[]) ;
    /**
     * method for calculating SHA1 over a string, callable from TCL
     */
    static int sha1Cmd(ClientData aCData, Tcl_Interp *aInterp, int aObjc, Tcl_Obj *const aObjv[]) ;
    /**
     * method for storing locally a bytearray from TCL
     */
    static int storeTCLProgLocalDataCmd(ClientData aCData, Tcl_Interp *aInterp, int aObjc, Tcl_Obj *const aObjv[]) ;
    /**
     * method for retrieving a data-blob previously stored using @ref storeTCLProgLocalData.
     */
    static int retrieveTCLProgLocalDataCmd(ClientData aCData, Tcl_Interp *aInterp, int aObjc, Tcl_Obj *const aObjv[]) ;
    /**
     * callback-method for saving a file to local filesystem from 
     * tcl program. Method will open file selection dialog so user can always
     * cancel operation. 
     */
    static int saveFileCmd(ClientData aCData, Tcl_Interp *aInterp, int aObjc, Tcl_Obj *const aObjv[]) ;
    /**
     * Callback-method for reading a file from local filesystem.
     * Method will open file selection dialog so user can always
     * cancel operation. 
     */
    static int openFileCmd(ClientData aCData, Tcl_Interp *aInterp, int aObjc, Tcl_Obj *const aObjv[]) ;
    /**
     * Callback-method for checking profile in trust-tree. 
     */
    static int isProfileTrustedCmd(ClientData aCData, Tcl_Interp *aInterp, int aObjc, Tcl_Obj *const aObjv[]) ;
    /**
     * tcl channel procedures
     */
    static int closeProc(
        ClientData aInstanceData,
        Tcl_Interp *aInterp);
    /**
     * this method may be used to write data to tcl interpreter.
     */
    static int inputProc(
        ClientData aInstanceData,
        char *aBuf,
        int aBufSize,
        int *aErrorCodePtr);
    /**
     * output from tcl interpreter comes to this method. See
     * @ref outputProcImpl non-static version of this method
     */
    static int outputProc(
        ClientData aInstanceData,
        const char *aBuf,
        int aToWrite,
        int *aErrorCodePtr);

    static void watchProc(
        ClientData instanceData,
        int mask);
    static int getHandleProc(
        ClientData aInstanceData,
        int aDirection,
        ClientData *aHandlePtr);

    /**
     * output from tcl interpreter comes to this method
     */
    int outputProcImpl(
        ClientData aInstanceData,
        const char *aBuf,
        int aToWrite,
        int *aErrorCodePtr);

    /**
     * method that checks if there is anything in
     * iAddedDataModelItems and if yes, calls notify procedure
     * inside interpreter. Interpreter must have notify
     * procedure installed. See also @ref notifyOfContentReceived.
     * When this is called, there must be at least one item in
     * iAddedDataModelItems
     */
    void notifyInterpreterOfContentReceived( Tcl_Interp* aInterp ) ;
private: // these are not public
    Model& iModel ; /**< datamodel reference */
    MController& iController ;
    Tcl_Interp *iInterp  ; /**< actual TCL interpreter */
    QString iTCLScript ; /**< TCL script to interpret */
    bool iNeedsToRun ; /**< when set to false, will terminate TCL script */
    QByteArray iStdOutBuffer ;
    bool iDeleteLater ; /**< When set to true, will call deleteLater() */
    TclConsoleDialog* iConsole ;
    QQueue<QString> iScriptQueue ;
    QString iMainWindowTitle ;
    QQueue<QPair<Hash, ProtocolItemType> > iAddedDataModelItems ;
    TclCallbacks* iTclCallbacks ;
} ;
#endif
