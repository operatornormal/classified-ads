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
#include "newprivmsgdialog.h"
#include "../mcontroller.h"
#include "../log.h"
#include "../datamodel/model.h"
#include "../datamodel/profile.h"
#include "../FrontWidget.h"
#include "../datamodel/privmsgmodel.h"
#include "../datamodel/privmsg.h"
#include "../datamodel/contentencryptionmodel.h"
#include "../datamodel/profilemodel.h"

NewPrivMessageDialog::NewPrivMessageDialog(QWidget *aParent,
					   MController* aController,
					   const QString& aRecipient,
					   const QString& aSubject,
					   const Profile& aSelectedProfile,
					   PrivateMessageSearchModel& aSearchModel,
					   const Hash& aReferencesMsg,
					   const Hash& aReferencesCa,
					   const Hash& aRecipientsNode )
  : TextEdit(aParent,aController,aSelectedProfile),
    iSearchModel(aSearchModel),
    iRecipientsNode(aRecipientsNode)
{
  ui.setupUi(this) ; 
  initializeTextEditor(ui.messageEdit,
		       ui.gridLayout,
		       ui.toolBoxLayoutUpper,
		       ui.toolBoxLayoutLower) ; 
  iReferencesMsg = aReferencesMsg ; 
  iReferencesCa = aReferencesCa ; 

  ui.subjectEdit->setText(aSubject) ; 
  ui.recipientEdit->setText(aRecipient) ; 
  iAttachmentListLabel = ui.attachmentsListLabel ;
  connect(ui.attachButton, SIGNAL(clicked()),
	  this, SLOT(attachButtonClicked()));
  connect(ui.bottomButtonsBox, SIGNAL(accepted()), this, SLOT(okButtonClicked()));
  connect(ui.bottomButtonsBox, SIGNAL(rejected()), this, SLOT(cancelButtonClicked()));
  ui.messageEdit->setFocus(Qt::PopupFocusReason) ; 
}

NewPrivMessageDialog::~NewPrivMessageDialog()
{
  LOG_STR("NewPrivMessageDialog::~NewPrivMessageDialog") ;
}


void NewPrivMessageDialog::okButtonClicked()
{
  LOG_STR("NewPrivMessageDialog::okButtonClicked") ;

  Hash selectedUserProfileHash ( iController->profileInUse() ) ; 
  PrivMessage msg ; 

  msg.iSenderName = iSelectedProfile.displayName() ; 

  msg.iSubject = ui.subjectEdit->text() ; 
  msg.iMessageText = ui.messageEdit->toHtml() ; 

  msg.iSenderHash = iSelectedProfile.iFingerPrint ; 
  if ( msg.iSenderName == msg.iSenderHash.toString() ) {
    msg.iSenderName.clear() ; 
  }
  QLOG_STR("Privmsg msg.iSenderHash = " + msg.iSenderHash.toString() ) ; 
  msg.iTimeOfPublish = QDateTime::currentDateTimeUtc().toTime_t() ; 
  msg.iRecipient.fromString(reinterpret_cast<const unsigned char *>(ui.recipientEdit->text().toLatin1().constData())) ;
  if ( msg.iRecipient == KNullHash ) {
    QMessageBox::about(this, tr("Error"),
		       tr("Recipient addr is not valid")) ;
  } else {
    QLOG_STR("Privmsg msg.iRecipient = " + msg.iRecipient.toString() ) ; 
    if ( iReferencesMsg != KNullHash ) {
      msg.iReplyToMsg =  iReferencesMsg  ; 
    } else {
      msg.iReplyToMsg = KNullHash ; 
    }
    if ( iReferencesCa != KNullHash ) {
      msg.iReplyToCa =  iReferencesCa  ; 
    } else {
      msg.iReplyToCa = KNullHash ; 
    }
    foreach (const QString& attachmentFile , iFilesAboutToBeAttached ) {
      Hash attachmentHash = publishBinaryAttachment(attachmentFile,
						    false) ;
      if ( attachmentHash != KNullHash ) {
	msg.iAttachedFiles.append(attachmentHash) ; 
      }
    }
    iController->model().lock() ;

    if ( iRecipientsNode == KNullHash ) {
      iRecipientsNode =  tryFindRecipientNode(msg.iRecipient) ;
    }

    quint32 dummy  ; 
    if ( 
	iController->model().contentEncryptionModel().PublicKey(iSelectedProfile.iFingerPrint,
								msg.iProfileKey,
								&dummy )) {
      iController->model().privateMessageModel().publishPrivMessage(msg,
								    iRecipientsNode) ; 
      iSearchModel.newMsgReceived(msg) ; 
    } else {
      QMessageBox::about(this, tr("Error"),
			 tr("Recipient encryption key not found from storage")) ;
    }
    iController->model().unlock() ; 

    close() ; 
    this->deleteLater() ;
  }
}


void NewPrivMessageDialog::cancelButtonClicked()
{
  LOG_STR("NewPrivMessageDialog::cancelButtonClicked") ;
  close() ; 
  this->deleteLater() ;
}

Hash NewPrivMessageDialog::tryFindRecipientNode(const Hash& aRecipientFingerPrint) 
{
  Hash retval ;
  Profile *recipientProfile (NULL) ;
  if ( ( recipientProfile = iController->model().profileModel().profileByFingerPrint(aRecipientFingerPrint) ) != NULL ) {
    if ( recipientProfile->iNodeOfProfile ) {
      retval = recipientProfile->iNodeOfProfile->nodeFingerPrint() ; 
    }
    delete recipientProfile ; 
  }
  return retval ; 
}