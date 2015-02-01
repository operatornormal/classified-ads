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

#include <QtGui>
#include <QMessageBox>
#include "newprofilecommentdialog.h"
#include "../log.h"
#include "../datamodel/model.h"
#include "../datamodel/profilecomment.h"
#include "../datamodel/profile.h" 
#include "../datamodel/profilecommentmodel.h"
#include "../datamodel/profilecommentlistingmodel.h"
#include "../datamodel/profilemodel.h"
#include "../FrontWidget.h"
#include "../datamodel/contentencryptionmodel.h"
#include "attachmentlistdialog.h" // for tryFindNodeByProfile()

NewProfileCommentDialog::NewProfileCommentDialog(QWidget *aParent,
						 MController* aController,
						 const QString& aCommentedProfile,
						 const QString& aSubject,
						 Profile& aSelectedProfile,
						 ProfileCommentListingModel& aCommentListingModel,
						 const Hash& aReferencesMsg,
						 const Hash& aReferencesCa,
						 const Hash& aRecipientsNode)
  : TextEdit(aParent,
	       aController,
	       aSelectedProfile),
    iRecipientsNode(aRecipientsNode),
    iCommentListingModel(aCommentListingModel) 
{
  ui.setupUi(this) ; 
  initializeTextEditor(ui.messageEdit,
		       ui.gridLayout,
		       ui.toolBoxLayoutUpper,
		       ui.toolBoxLayoutLower) ; 
  iReferencesMsg = aReferencesMsg ; 
  iReferencesCa = aReferencesCa ; 

  ui.subjectEdit->setText(aSubject) ; 
  ui.commentedProfileEdit->setText(aCommentedProfile) ; 
  iAttachmentListLabel = ui.attahcmentsListLabel ;
  connect(ui.attachButton, SIGNAL(clicked()),
	  this, SLOT(attachButtonClicked()));
  connect(ui.bottomButtonsBox, SIGNAL(accepted()), this, SLOT(okButtonClicked()));
  connect(ui.bottomButtonsBox, SIGNAL(rejected()), this, SLOT(cancelButtonClicked()));
  ui.messageEdit->setFocus(Qt::PopupFocusReason) ; 
}

NewProfileCommentDialog::~NewProfileCommentDialog()
{
  LOG_STR("NewProfileCommentDialog::~NewProfileCommentDialog") ;
}


void NewProfileCommentDialog::okButtonClicked()
{
  LOG_STR("NewProfileCommentDialog::okButtonClicked") ;

  Hash selectedUserProfileHash ( iController->profileInUse() ) ; 
  ProfileComment comment ; 

  if ( iSelectedProfile.iIsPrivate == false ) {
    comment.iCommentorNickName = iSelectedProfile.displayName() ; 
  }
  comment.iSubject = ui.subjectEdit->text() ; 
  comment.iCommentText = ui.messageEdit->toHtml() ; 

  comment.iCommentorHash = iSelectedProfile.iFingerPrint ; 
  QLOG_STR("New profile comment comment.iCommentorHash = " + comment.iCommentorHash.toString() ) ; 
  comment.iTimeOfPublish = QDateTime::currentDateTimeUtc().toTime_t() ; 
  comment.iProfileFingerPrint.fromString(reinterpret_cast<const unsigned char *>(ui.commentedProfileEdit->text().toLatin1().constData())) ;
  if ( comment.iProfileFingerPrint == KNullHash ) {
    QMessageBox::about(this, tr("Error"),
		       tr("Commented profile addr is not valid")) ;
  } else {
    QLOG_STR("Profilecomment comment.iProfileFingerPrint = " + comment.iProfileFingerPrint.toString() ) ; 
    if ( iReferencesMsg != KNullHash ) {
      comment.iReferences = iReferencesMsg  ; 
      comment.iTypeOfObjectReferenced = PrivateMessage ; 
    } else if ( iReferencesCa != KNullHash ) {
      comment.iReferences = iReferencesCa  ; 
      comment.iTypeOfObjectReferenced = ClassifiedAd ; 
    } else {
      comment.iReferences = KNullHash ; 
    }
    foreach (const QString& attachmentFile , iFilesAboutToBeAttached ) {
      Hash attachmentHash = publishBinaryAttachment(attachmentFile,
						    false) ;
      if ( attachmentHash != KNullHash ) {
	comment.iAttachedFiles.append(attachmentHash) ; 
      }
    }
    // TODO:
    // set iReferences to some meaningful value
    iController->model().lock() ;

    if ( iRecipientsNode == KNullHash ) {
      iRecipientsNode =  AttachmentListDialog::tryFindNodeByProfile(comment.iProfileFingerPrint, *iController) ;
    }

    quint32 dummy  ; 
    if ( 
	iController->model().contentEncryptionModel().PublicKey(iSelectedProfile.iFingerPrint,
								comment.iKeyOfCommentor,
								&dummy )) {
      if ( iController->model().profileCommentModel().publishProfileComment(comment) ) {
	iCommentListingModel.newCommentReceived(comment) ; 
      }
    } else {
      QMessageBox::about(this, tr("Error"),
			 tr("Recipient encryption key not found from storage")) ;
    }
    iController->model().unlock() ; 

    close() ; 
    this->deleteLater() ;
  }
}


void NewProfileCommentDialog::cancelButtonClicked()
{
  LOG_STR("NewProfileCommentDialog::cancelButtonClicked") ;
  close() ; 
  this->deleteLater() ;
}

