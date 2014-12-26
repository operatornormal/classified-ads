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

#ifndef NEW_PROFILE_COMMENT_DIALOG_H
#define NEW_PROFILE_COMMENT_DIALOG_H

#include <QDialog>
#include "../mcontroller.h"
#include "../ui_newProfileComment.h"
#include "../datamodel/profile.h"
#include "../textedit/textedit.h"

class ProfileCommentListingModel ; 

/**
 * @brief class for allowing posting of a comment about user profile
 *
 */
class NewProfileCommentDialog : public TextEdit
{
  Q_OBJECT

public:
  /**
   * Constructor.
   *
   * @param aParent is owner-window of this dialog
   * @param aController application controller reference
   * @param aCommentedProfile is fingerprint of the profile
   *        that is about to be commented. If KNullHash, then
   *        dialog will open with empty recipient
   * @param aSubject if comment is reply to another comment, this is subject of 
   *                 the original posting  
   * @param aSelectedProfile profile doing the sending
   * @param aReferencesMsg if msg is reply to another msg, this is article referenced. NULL 
   *                    if article is start of a new thread. 
   */
  NewProfileCommentDialog (QWidget *aParent,
			   MController* aController,
			   const QString& aCommentedProfile,
			   const QString& aSubject,
			   const Profile& aSelectedProfile,
			   ProfileCommentListingModel& aCommentListingModel,
			   const Hash& aReferencesMsg = KNullHash,
			   const Hash& aReferencesCa = KNullHash,
			   const Hash& aRecipientsNode = KNullHash);
  /** destructor */
  ~NewProfileCommentDialog ();

private slots:
  void okButtonClicked() ;
  void cancelButtonClicked() ;
signals:
  void error(MController::CAErrorSituation aError,
	     const QString& aExplanation) ;
private: // methods
  // if recipients node is not known, here try finding out one
  Hash tryFindRecipientNode(const Hash& aRecipientFingerPrint) ;
private:
  Ui_newProfileCommentDialog ui ; 
  Hash iReferencesMsg ; /**< if we're referencing another msg, this is the FP */
  Hash iReferencesCa ; /**< if we're referencing ca, this is the FP */
  Hash iRecipientsNode ; 
  ProfileCommentListingModel& iCommentListingModel ; 
};

#endif
