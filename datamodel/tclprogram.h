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

#ifndef CA_TCLPROGRAM_H
#define CA_TCLPROGRAM_H
#include <QString>
#include "../util/hash.h" // for class Hash  

class MController ;

/**
 * @brief Carrier-class for TCL-program and related settings
 *
 * Instances of this class are stored and retrieved using
 * @ref TclModel.
 */
class TclProgram {
public:
    TclProgram() ; /**< constructor */
    ~TclProgram() ; /**< destructor */
    QString displayName() const ;
    Hash iFingerPrint ;
    quint32 iTimeOfPublish ; /**< seconds since 1-jan-1970 */
    const QString& programText() const {
        return iProgramText ;
    } ;
    const QString& programName() const {
        return iProgramName ;
    } ;
    void setProgramText(const QString& aText) ;
    void setProgramName(const QString& aName) ;
private:
    QString iProgramText ; /**< used with setter and getter */
    QString iProgramName ; /**< used with setter and getter */
} ;
#endif
