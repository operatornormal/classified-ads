/*     -*-C++-*- -*-coding: utf-8-unix;-*-
       Classified Ads is Copyright (c) Antti Järvinen 2013. 

       This file is part of Classified Ads.

       Classified Ads is free software: you can redistribute it and/or modify
       it under the terms of the GNU General Public License as published by
       the Free Software Foundation, either version 3 of the License, or
       (at your option) any later version.

       Classified Ads is distributed in the hope that it will be useful,
       but WITHOUT ANY WARRANTY; without even the implied warranty of
       MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
       GNU General Public License for more details.

       You should have received a copy of the GNU General Public License
       along with Classified Ads.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef NEW_CLASSIFIED_AD_DIALOG_H
#define NEW_CLASSIFIED_AD_DIALOG_H

#include <QDialog>
#include "../mcontroller.h"
#include "../ui_newClassifiedAd.h"
#include "../textedit/textedit.h"

class ProfileSearchModel ;
class ProfileReadersListingModel ; 
class Profile ;
class CAListingModel ; 
/**
 * @brief class for allowing posting of new classified ad
 *
 * Inherits @ref DialogBase via @ref TextEdit where most editing
 * functionality comes from. 
 */
class NewClassifiedAdDialog : public TextEdit
{
  Q_OBJECT

public:
  /**
   * Constructor.
   *
   * @param aParent is owner-window of this dialog
   * @param aController application controller reference
   * @param aAboutComboxIndex index of combobox of "about" in classification-selection
   * @param aRegardingComboxIndex index of combobox of "regarding" in classification-selection
   * @param aWhereComboxIndex index of combobox of "in" in classification-selection
   * @param aAboutComboText text of combobox in of "about" in classification-selection ; this may be text
   *                        typed by user, in which case the index (aAboutComboxIndex) is ignored
   * @param aRegardingComboText  text of combobox in of "regarding" in classification-selection 
   * @param aWhereComboxText  text of combobox in of "where" in classification-selection 
   * @param aSelectedProfile profile doing the sending
   * @param aReferences if CA is reply to another CA, this is article referenced. NULL 
   *                    if article is start of a new thread. 
   * @param aSubject if CA is reply to another CA, this is subject of 
   *                 the original posting 
   */
  NewClassifiedAdDialog(QWidget *aParent,
			MController* aController,
			int aAboutComboxIndex ,
			int aRegardingComboxIndex ,
			int aWhereComboxIndex ,
			const QString& aAboutComboText,
			const QString& aRegardingComboText,
			const QString& aWhereComboxText,
			Profile& aSelectedProfile,
			CAListingModel& aCaListingModel,
			const Hash* aReferences = NULL,
			const QString* aSubject = NULL );
  /** destructor */
  ~NewClassifiedAdDialog();

private slots:
  void okButtonClicked() ;
  void cancelButtonClicked() ;
signals:
  void error(MController::CAErrorSituation aError,
	     const QString& aExplanation) ;
private:
  Ui_newCaDialog ui ; 
  Hash iReferences ; /**< if we're referencing another CA, this is the FP */
  CAListingModel& iCAListingModel ;
};

#endif
