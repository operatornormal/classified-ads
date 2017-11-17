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


#ifndef TCLUTIL_H
#define TCLUTIL_H
#include <QObject>
#include "../mcontroller.h"

/**
 * @brief Misc utilities needed in TCL-related code
 *
 * Functionality not really TCL-dependent but called from
 * TCL-wrapper.
 */
class TclUtil : public QObject {
    Q_OBJECT
public:
    TclUtil();
    ~TclUtil() ;

signals:
    void error(MController::CAErrorSituation aError,
               const QString& aExplanation) ;

public: // methods
    /**
     * Method for constructing the group string of
     * classified ad group string. String is of form about.concerns.where.
     * See method in @ref FrontWidget::selectedClassification that has same
     * purpose but has different input
     *
     * @param aAboutComboBoxText "about" part of group string. This is checked
     *            against localized strings in datamodel and user types
     *            "Veneet" (boats in finnish) and happens to have finnish
     *            locale, then datamodel will catch the situation and
     *            return "Boats" - idea is that any boat ad will go to
     *            same group regardless of locale.
     * @param aConcernsComboBoxText "concerns" e.g. the verb part of
     *            group string.
     * @param aInComboBoxText "where" part of the group string.
     * @param aModel datamodel reference
     *
     * @return something alike "ToBeAnnounced.Software.Any country"
     */
    static QString constructCAGroup (   QString aAboutComboBoxText ,
                                        QString aConcernsComboBoxText ,
                                        QString aInComboBoxText ,
                                        const Model& aModel ) ;
private: // variables

} ;
#endif
