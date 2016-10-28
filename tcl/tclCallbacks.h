/*     -*-C++-*- -*-coding: utf-8-unix;-*-
  Classified Ads is Copyright (c) Antti Järvinen 2013-2016.

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


#ifndef TCLCALLBACKPROCEDURES_H
#define TCLCALLBACKPROCEDURES_H
#include <QObject>
#include "../mcontroller.h"
#include <tcl.h>

class Model ;
class MController ;

/**
 * @brief Class implementing TCL interpreter callbacks
 *
 * This class is closely coupled with @ref TclWrapper class
 * and the TCL interpreter instantiated by TclWrapper will
 * use command-callbacks implemented by this class.
 */
class TclCallbacks : public QObject {
    Q_OBJECT
public:
    TclCallbacks(Model& aModel,
                 MController& aController);

    /**
     * Destructor
     */
    ~TclCallbacks() ;

signals:
    void error(MController::CAErrorSituation aError,
               const QString& aExplanation) ;


public: // methods, called from tclWrapper.h static methods
    /**
     * non-static method for getting binary file
     */
    int getBinaryFileCmdImpl(ClientData aCData, Tcl_Interp *aInterp, int aObjc, Tcl_Obj *const aObjv[]) ;

    /**
     * non-static method for getting profile comment
     */
    int getProfileCommentCmdImpl(ClientData aCData, Tcl_Interp *aInterp, int aObjc, Tcl_Obj *const aObjv[]) ;

    /**
     * non-static method for getting classified ad
     */
    int getClassifiedAdCmdImpl(ClientData aCData, Tcl_Interp *aInterp, int aObjc, Tcl_Obj *const aObjv[]) ;

    /**
     * non-static method for getting profile details
     */
    int getProfileCmdImpl(ClientData aCData, Tcl_Interp *aInterp, int aObjc, Tcl_Obj *const aObjv[]) ;

    /**
     * Non-static Tcl extension method: when TCL scripts asks for list of
     * profiles, it invokes this method. See @initExtensions
     * where this method is added into TCL interpreters repertoire.
     * Called from @ref listItemsCmd method.
     */
    int listItemsCmdImpl(ClientData aCData, Tcl_Interp *aInterp, int aObjc, Tcl_Obj *const aObjv[]) ;

    /**
     * Tcl extension method for publishing an item. This will be
     * called by several in-TCL commands, for instance "publishFile",
     * "publishComment" or "publishProfile". The TCL command
     * will be in aObjv[0] and aObjv[1] will contain the actual
     * object to be published.
     *
     * This is the non-static-version, see also @ref TclWrapper::publishItemCmd.
     * @param aCData clientdata from tcl interpreter. Not used.
     * @param aInterp pointer to calling TCL interpreter
     * @param aObjc number of items in array aObjv
     * @param aObjv command arguments. First is name of the command
     *        itself, like "publishFile", 2nd is actual object
     * @return TCL_OK on success.
     */
    int publishItemCmdImpl(ClientData aCData, Tcl_Interp *aInterp, int aObjc, Tcl_Obj* const aObjv[]) ;

private: // methods
    /**
     * work-horse method for publishing a profile comment. See method
     * @ref publishItemCmdImpl that calls this.
     */
    int publishProfileCommentCmdImpl(ClientData aCData, Tcl_Interp *aInterp, int aObjc, Tcl_Obj *const aObjv[]) ;
    /**
     * work-horse method for publishing a binary file. See method
     * @ref publishItemCmdImpl that calls this.
     */
    int publishFileCmdImpl(ClientData aCData, Tcl_Interp *aInterp, int aObjc, Tcl_Obj *const aObjv[]) ;

    /**
     * non-static method for publishing a profile
     */
    int publishProfileCmdImpl(ClientData aCData, Tcl_Interp *aInterp, int aObjc, Tcl_Obj *const aObjv[]) ;

    /**
     * non-static method for publishing a classified ad
     */
    int publishClassifiedAdCmdImpl(ClientData aCData, Tcl_Interp *aInterp, int aObjc, Tcl_Obj *const aObjv[]) ;

private: // variables
    Model& iModel ; /**< datamodel reference */
    MController& iController ;
} ;
#endif
