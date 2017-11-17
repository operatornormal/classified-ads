/* -*-C++-*- -*-coding: utf-8-unix;-*-
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

#include "tclprogram.h"
#include "../util/hash.h"
#include "../log.h"
#include "../util/jsonwrapper.h"
#include <QVariantMap>
#include "../mcontroller.h"
#include "model.h"


TclProgram::TclProgram() :
    iFingerPrint(KNullHash),
    iTimeOfPublish(0) {
    LOG_STR("TclProgram::TclProgram()") ;
}

TclProgram::~TclProgram() {
    LOG_STR("TclProgram::~TclProgram()") ;
}


QString TclProgram::displayName() const {
    QString retval ;
    if ( iProgramName.length() > 0 ) {
        retval = iProgramName ;
    } else {
        retval = iFingerPrint.toString() ;
    }
    return retval ;
}

void TclProgram::setProgramText(const QString& aText) {
    iProgramText = aText ;
    if ( iProgramText.length() == 0 ) {
        iFingerPrint = KNullHash ;
    } else {
        iFingerPrint.calculate(iProgramText.toUtf8()) ;
    }
}
void TclProgram::setProgramName(const QString& aName) {
    iProgramName = aName ;
}
