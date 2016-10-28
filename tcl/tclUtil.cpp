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
#include "tclUtil.h"
#include "../datamodel/camodel.h"
#include "../datamodel/model.h"

TclUtil::TclUtil() {
}

TclUtil::~TclUtil() {

}

// static
QString TclUtil::constructCAGroup (    QString aAboutComboBoxText ,
                                       QString aConcernsComboBoxText ,
                                       QString aInComboBoxText,
                                       const Model& aModel ) {
    QString retval ;
    // check out if pre-defined string from combobox or
    // something that user wrote himself to combobox.
    // it seems like digging that information from combobox itself
    // is difficult to do .. user would need to hit enter
    // to set the indexes correctly and she won't
    if ( aModel.classifiedAdsModel()
            .aboutComboBoxTexts()
            .indexOf(aAboutComboBoxText) != -1 ) {
        retval = aModel
                 .classifiedAdsModel()
                 .purposeOfAdString((ClassifiedAdsModel::PurposeOfAd)aModel
                                    .classifiedAdsModel()
                                    .aboutComboBoxTexts()
                                    .indexOf(aAboutComboBoxText)) ;
    } else {
        retval = aAboutComboBoxText ;
    }
    retval.append(".") ;
    if ( aModel
            .classifiedAdsModel()
            .regardingComboBoxTexts()
            .indexOf(aConcernsComboBoxText) != -1 ) {
        retval.append( aModel
                       .classifiedAdsModel()
                       .concernOfAdString((ClassifiedAdsModel::ConcernOfAd)aModel
                                          .classifiedAdsModel()
                                          .regardingComboBoxTexts()
                                          .indexOf( aConcernsComboBoxText))) ;
    } else {
        retval.append( aConcernsComboBoxText ) ;
    }
    retval.append(".") ;

    if ( aModel
            .classifiedAdsModel()
            .whereComboBoxTexts()
            .indexOf( aInComboBoxText ) == 0 ) {
        // explanation: the country names in "where" are not localized.
        // except for the first entry, "Any country" and if we get that,
        // lets put un-localized "Any country" in there -> note that this
        // same string appears inside tr() in datamodel/camodel.cpp
        retval.append( "Any country" ) ;
    } else {
        // if not the "Any country", then just take the literal
        // string value, regardless of index
        retval.append( aInComboBoxText ) ;
    }
    return retval ;
}
