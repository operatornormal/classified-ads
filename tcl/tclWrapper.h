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
 * This class wraps [TCL](http://www.tcl.tk) interpreter in 
 * [safe mode](https://www.tcl.tk/man/tcl8.6/TclCmd/safe.htm). 
 * This class is intended to be used so that, this 
 * is instantiated once and deleted at application shutdown. 
 * One TCL program may be running at same time - restriction of 
 * Tk is that one instance per process only.
 *
 * This is a thread and it is supposed to be used so that
 * class is instantiated, setScript() method is called,
 * followed by start(). Start may be called multiple times,
 * calling start() when thread is already executing has
 * no effect. In user interface user may open the TCL
 * console dialog from menu, commands typed are evaluated
 * using an interpreter. In addition to "ad-hoc" commands
 * to interpreter, a program text may be stored into 
 * TCL program library. 
 *
 * Classified-ads implements a local data storage for TCL
 * programs but a precondition for its successful usage is 
 * that program being run has been previously stored into
 * TCL program library of classified ads. Reason for this 
 * the underlying database implementation: each program may
 * have its own local data but for being able to associate 
 * the program with the data, the program must be stored
 * first. Longer TCL program may be typed directly into
 * "TCL Programs" dialog but for really long programs it
 * is naturally easier to use a proper code-editor and then
 * just copy-paste the code in classified-ads as it does not
 * implement a proper code-editor, just a "normal" text editor.
 * TCL interpreter output for debugging purposes may be seen
 * at "TCL Interpreter" dialog in the UI. Stdout+stderr of
 * the interpreter are directed there. At time of writing there
 * is known issue with very long output lines and output lines
 * containing non-ASCII7 -characters like accented characters
 * of finnish language. Using those will not stop the interpreter
 * or program but will break interpreter output. 
 *
 * In addition to standard safe-mode-TCL/TK this wrapper
 * introduces several TCL extension commands for accessing
 * features implemented uniquely by classified-ads software. 
 * These features include for example search, fetch and post
 * of an classified ad. List of additional commands is
 *
 *   TCL command                              | Purpose
 *   ---------------------------------------  | -------------
 *   [listProfiles](@ref listProfilesLabel)   | Returns list of profile id's matching given conditions
 *   [listComments](@ref listCommentsLabel)   | Returns list of profile comment id's matching given conditions
 *   [listAds](@ref listAdsLabel)             | Returns list of classified ad id's matching given conditions
 *   [getProfile](@ref getProfileLabel)       | Returns profile details by id
 *   [getProfileComment](@ref getCommentLabel)| Returns profile comment by id
 *   [getClassifiedAd](@ref getAdLabel)       | Returns classified ad by id
 *   [getBinaryFile](@ref getFileLabel)       | Returns binary file by id
 *
 * Longer documentation of each command follows. 
 * 
 * listProfiles {#listProfilesLabel}
 * ============
 * Basic usage of listProfiles is 
 * ```
 * set profileList [ listProfiles foobar ]
 * ```
 * after which profileList contans a (possibly empty) array of
 * profile idenfiers. The search condition here is the word
 * "foobar" that is used to match profiles from search index. 
 * Returned list is list of dictionaries. Each dictionary
 * has as its key profile address (SHA1) and in its value the
 * "display name" of the profile, that is human-readable combination
 * of profile address, nickname, given and family names. 
 *
 * The search engine in use is the powerful full-text-search (FTS) 
 * feature of the sqlite database. It is well documented in 
 * [SQLite documentation](https://www.sqlite.org/fts3.html). 
 * For a profile to be indexed for search purposes, it must be
 * a public profile. The fields that are put into index are
 *  * profile address (SHA1 identifier), field name = profilehash
 *  * nickname, field name = nickname
 *  * greeting text, field name = greetingtext
 *  * first name, field name = firstname
 *  * family name, field name = familyname
 *  * city, field name = city
 *  * btc wallet address, field name = btc
 *  * state, field name = state
 *
 * Example queries include
 * ```
 * set profileList [ listProfiles {a*} ]
 * ```
 * that set value of `profileList` to contain every profile from local
 * storage that in some indexed field contains a word that begins 
 * with letter "a". 
 * ```
 * set profileList [ listProfiles {Suomi Finland} ]
 * ```
 * that returns every profile that in some indexed field
 * contain word "Suomi" and also word "Finland". If only 
 * "Suomi" or "Finland" is present, profile is not returned
 * ```
 * set profileList [ listProfiles {Suomi OR Finland} ]
 * ```
 * that returns every profile that in some indexed field
 * contain word "Suomi" or word "Finland".
 * ```
 * set profileList [ listProfiles {nickname:s*} ]
 * ```
 * Returns profiles that in nickname field have a word that
 * begins with letter "s". Example content of $profileList from such a 
 * query might be for example
 * 
 * ```
 * {1D2CB451BD348B12BA39316499F5AF6D53ACEECF Simo} 
 * {22C13DCF54C8D9E6148C834E601683DD894FCBF5 {Elmeri Sukeltaja}} 
 * {52F79319C034BA9939E930A5BD812B6CBE5DF2DD {Susanna K}}
 * ```
 *
 * Search is performed on local data only
 * 
 * listComments {#listCommentsLabel}
 * ============
 * listComments is similar in function as command
 * [listProfiles](@ref listProfilesLabel) but it returns profile 
 * comments instead.
 * ```
 * set commentList [ listComments foobar ]
 * ```
 * Would return every profile comment with word foobar in text. 
 * Indexed fields are
 *  * Commented profile identifier (SHA1), field name = profilehash
 *  * Author profile identifier (SHA1), field name = commentorhash
 *  * Comment text , field name = comment
 *  * Comment subject line, field name = commentsubject
 *  * Nickname of autoher, field name = commentornickname
 *
 * Results are returned in same manner, as a list of dictionaries
 * where key is the comment SHA1 identifier and value is
 * comment subject. 
 *
 * listAds {#listAdsLabel}
 * ============
 * listAds is similar in function as command
 * [listProfiles](@ref listProfilesLabel) but it returns identifiers
 * of classified ads instead.
 * ```
 * set adList [ listAds computer ]
 * ```
 * Would return every classified ad with word foobar in any indexed field. 
 * Indexed fields are
 *  * Author profile identifier (SHA1), field name = senderhash
 *  * Author profile display name, field name = sendername
 *  * Subject, field name = subject
 *  * Text of the ad, field name = text
 *
 * Results are returned in same manner, as a list of dictionaries
 * where key is the SHA1 identifier of the classified ad and value is
 * ad subject. 
 *
 * getProfile {#getProfileLabel}
 * ============
 * getProfile returns details of operator profile. Argument is the 
 * SHA1 identifier of the profile. Profile is returned as a dictionary
 * where keys are mapped to their values. 
 * ```
 * set p [ getProfile $profileIdentifier ]
 * ```
 * The keys in the dictionary are the following
 * 
 *  
 *   Key                | Description of the value
 *   ------------------ | ----------------------------------
 *   displayName        | Human-readable name of profile
 *   fingerPrint        | Profile SHA1 address
 *   greetingText       | Operator greeting text
 *   firstName          | Operator given name
 *   familyName         | Operator family name
 *   cityCountry        | Operator city/country
 *   timeOfPublish      | Time of last publish, as seconds since 1-Jan-1970
 *   isPrivate          | Boolean indicating if profile is private
 *   imagePNG           | Image data of profile picture
 *   BTCAddress         | BTC wallet address of the operator
 *   stateOfTheWorld    | Situation with the world, as perceived by the operator
 *   profileReaders     | List of profiles readers, value is list of operator profiles
 *   sharedFiles        | List of files shared by profile, value is list of file SHA1 identifiers
 *   trustList          | List of profiles publicly trusted by this operator
 *
 * Every field may be omitted from dictionary if there is no value. 
 * Value is always present in fields fingerPrint, isPrivate, timeOfPublish 
 * and displayName. Note that operators own profile may not have been 
 * published at all and for this reason getting operators own profile 
 * may fail. Inside TCL programs the current profile SHA1 is stored in 
 * global variable `::profileInUse` so command `getProfile $::profileInUse` 
 * should always return details of operators own profile, if published. 
 *
 * getProfileComment {#getCommentLabel}
 * ============
 * getProfileComment returns single comment to some operators profile. 
 * Argument is the  SHA1 identifier of the comment as returned by
 * [listComments](@ref listCommentsLabel) command. Comment is returned 
 * as a dictionary where keys are mapped to their values. 
 * ```
 * set c [ getProfileComment $commentIdentifier ]
 * ```
 * The keys in the dictionary are the following
 *  
 *   Key                    | Description of the value
 *   ---------------------- | ----------------------------------
 *   fingerPrint            | Comment SHA1 identifier
 *   fingerPrintOfCommented | SHA1 identifier of the operator that comment concerns
 *   fingerPrintOfSender    | SHA1 identifier of the author of the comment
 *   timeOfPublish          | Time of comment publish, as seconds since 1-Jan-1970
 *   isPrivate              | Boolean indicating if comment is private
 *   senderName             | Display name of profile of author of comment
 *   subject                | Comment subject
 *   messageText            | Comment text in html format
 *   plainMessageText       | Comment text in plain text
 *   replyTo                | SHA1 id of comment that this comments is a reply to
 *   attachedFiles          | List of files attached to this comment, value is list of file SHA1 identifiers
 *
 * getClassifiedAd {#getAdLabel}
 * ============
 * getClassifiedAd is similar in function to [getProfileComment](@ref getCommentLabel) 
 * command but it returns an classified ad. 
 * ```
 * set a [ getClassifiedAd $SHA1Identifier ]
 * ```
 * The keys in returned dictionary are the following
 *  
 *   Key                    | Description of the value
 *   ---------------------- | ----------------------------------
 *   fingerPrint            | Ad SHA1 identifier
 *   displayName            | Human-readable subject of the ad
 *   timeOfPublish          | Time of comment publish, as seconds since 1-Jan-1970
 *   senderName             | Display name of profile of author of ad
 *   senderHash             | SHA1 identifier of author of the ad
 *   subject                | Comment subject
 *   messageText            | Comment text in html format
 *   plainMessageText       | Comment text in plain text
 *   replyTo                | SHA1 id of ad that this ad is a reply to
 *   attachedFiles          | List of files attached to this ad, value is list of file SHA1 identifiers
 *   aboutComboboxText      | In "ads" view, value of left-side classification selection combobox
 *   concernsComboboxText   | In "ads" view, value of middle classification selection combobox
 *   inComboboxText         | In "ads" view, value of right-side classification selection combobox
 *   aboutComboBoxIndex     | In "ads" view, selection index of left-side classification selection combobox
 *   inComboBoxIndex        | In "ads" view, selection index of middle classification selection combobox
 *   concernsComboBoxIndex  | In "ads" view, selection index of right-side classification selection combobox
 *   group                  | Combination of "concerning.where",f.ex. "ToBeSold.Cars.UnitedStates"
 * 
 *
 * getBinaryFile {#getFileLabel}
 * ============
 * getBinaryFile is similar in function to [getProfileComment](@ref getCommentLabel) 
 * command but it returns a binary file. 
 * ```
 * set f [ getBinaryFile $fileSHA1 ]
 * ```
 * The keys in returned dictionary are the following
 *  
 *   Key                    | Description of the value
 *   ---------------------- | ----------------------------------
 *   fingerPrint            | Ad SHA1 identifier
 *   timeOfPublish          | Time of comment publish, as seconds since 1-Jan-1970
 *   mimeType               | Mime-type of the file, like 'application/tcl'
 *   description            | Description of the content
 *   owner                  | SHA1 of profile who published the file
 *   contentOwner           | Name (string) of possible (copyright) owner of content
 *   license                | License under which the file may be further shared
 *   fileData               | Actual binary data of the file
 *
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
