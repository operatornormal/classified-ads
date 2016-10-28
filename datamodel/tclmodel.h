/*    -*-C++-*- -*-coding: utf-8-unix;-*-
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

#ifndef CLASSIFIED_TCLPROGRAMMODEL_H
#define CLASSIFIED_TCLPROGRAMMODEL_H
#include <QMap>
#include "../mcontroller.h" // because enum from there is needed
#include "mmodelprotocolinterface.h"
#include "datamodelbase.h"

class Hash ;
class Profile ;
class TclProgram ;

/**
 * @brief Datamodel-part of storage related to TCL-programs
 *
 * This class provides with storage of TCL-scripts themselves,
 * possible settings related to TCL-scripts. See class @ref TclProgram.
 */
class TclModel : public QObject {
    Q_OBJECT

public:
    TclModel(MController *aMController,
             const MModelProtocolInterface &aModel) ;
    ~TclModel() ;

    /**
     * Method for putting tcl program into local storage.
     * Method performs insert or update. Logick is this: if there is
     * program in db with @ref aPreviousFingerPrint then update is
     * performed. In this case previous fingerprint won't be valid
     * any more. Otherwise insert is done and method returns fingerprint
     * of the program stored.
     *
     * @param aProgram is the program to store.
     * @param aPreviousFingerPrint if persisting program from UI, fingerprint of
     *        the program
     * @return fingerprint of program saved.
     */
    Hash locallyStoreTclProgram(const TclProgram& aProgram,
                                const Hash& aPreviousFingerPrint = KNullHash) ;

    /**
     * method for getting tcl progrom from local storage
     */
    TclProgram tclProgramByFingerPrint(const Hash& aFingerPrint) ;

    /**
     * method for removing tcl script from local storage
     *
     * @return true if program was deleted
     */
    bool discardTclProgram(const Hash& aFingerPrint) ;

    /**
     * method for getting list of tcl programs in local storage
     * @return Listing as qmap where key is the program name,
     *         and value is fingerprint of the program
     */
    QMap<QString, Hash> getListOfTclPrograms() ;

signals:
    void error(MController::CAErrorSituation aError,
               const QString& aExplanation) ;

private: // member variables:
    MController& iController  ;
    const MModelProtocolInterface& iModel ;
} ;
#endif
