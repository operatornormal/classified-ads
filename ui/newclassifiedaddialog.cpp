/*     -*-C++-*- -*-coding: utf-8-unix;-*-
       Classified Ads is Copyright (c) Antti Järvinen 2013. 

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

#include <QtGui>
#include "newclassifiedaddialog.h"
#include "../mcontroller.h"
#include "../log.h"
#include "../datamodel/model.h"
#include "../datamodel/profile.h"
#include "../FrontWidget.h"
#include "../datamodel/camodel.h"
#include "../datamodel/ca.h"
#include "../datamodel/contentencryptionmodel.h"
#include "../datamodel/calistingmodel.h"

NewClassifiedAdDialog::NewClassifiedAdDialog(QWidget *aParent,
					     MController* aController,
					     int aAboutComboxIndex ,
					     int aRegardingComboxIndex ,
					     int aWhereComboxIndex ,
					     const QString& aAboutComboText,
					     const QString& aRegardingComboText,
					     const QString& aWhereComboxText,
					     Profile& aSelectedProfile,
					     CAListingModel& aCaListingModel,
					     const Hash* aReferences,
					     const QString* aSubject )
  : TextEdit(aParent,
	     aController,
	     aSelectedProfile),
    iCAListingModel(aCaListingModel)
{
  ui.setupUi(this) ; 
  initializeTextEditor(ui.adEdit,
		       ui.gridLayout,
		       ui.toolBoxForActionsUpper,
		       ui.toolBoxForActionsLower) ; 
  iAttachmentListLabel = ui.attachmentListLabel ;
  FrontWidget::fillCaSelectionCombobox(*ui.caAboutComboBox,true,*aController) ; 
  FrontWidget::fillCaSelectionCombobox(*ui.caRegardingCombobox,false,*aController) ; 
  if ( aReferences ) {
    iReferences = *aReferences ; 
  }
  ui.caWhereComboBox->addItems(aController->model().classifiedAdsModel().whereComboBoxTexts()) ; 

  if ( aSubject ) {
    ui.subjectEdit->setText(*aSubject) ; 
  }
  ui.caAboutComboBox->setEditable(true) ;
  ui.caAboutComboBox->setInsertPolicy(QComboBox::InsertAtBottom) ; 
  ui.caRegardingCombobox->setEditable(true) ;
  ui.caRegardingCombobox->setInsertPolicy(QComboBox::InsertAtBottom) ; 
  ui.caWhereComboBox->setEditable(true) ;
  ui.caWhereComboBox->setInsertPolicy(QComboBox::InsertAtBottom) ; 

  LOG_STR2("aAboutComboxIndex = %d\n", aAboutComboxIndex) ; 
  LOG_STR2("ClassifiedAdsModel::ToBeAnnounced = %d\n", (int)(ClassifiedAdsModel::ToBeAnnounced)) ; 
  if ( aAboutComboxIndex == -1 ) {
    // custom text
    LOG_STR2("aAboutComboText = %s\n", qPrintable(aAboutComboText) ); 
    QLOG_STR("aAboutComboIndex = %s\n" + QString::number(aAboutComboxIndex)  ); 
    ui.caAboutComboBox->addItem(aAboutComboText) ; 
    ui.caAboutComboBox->setCurrentIndex(ui.caAboutComboBox->count()-1) ;
  } else {
    QLOG_STR("aAboutComboIndex = %s\n" + QString::number(aAboutComboxIndex)  ); 
    ui.caAboutComboBox->setCurrentIndex(aAboutComboxIndex) ;
  }


  if ( aRegardingComboxIndex == -1 ) {
    // custom text
    LOG_STR2( "aRegardingComboText= %s\n", qPrintable(aRegardingComboText) ); 
    ui.caRegardingCombobox->addItem(aRegardingComboText) ; 
    ui.caRegardingCombobox->setCurrentIndex(ui.caRegardingCombobox->count()-1) ;
  } else {
    ui.caRegardingCombobox->setCurrentIndex(aRegardingComboxIndex) ;
  }

  if ( aWhereComboxIndex == -1 ) {
    // custom text
    LOG_STR2( "aWhereComboxText= %s\n", qPrintable(aWhereComboxText) ); 
    ui.caWhereComboBox->addItem(aWhereComboxText) ; 
    ui.caWhereComboBox->setCurrentIndex(ui.caWhereComboBox->count()-1) ;
  } else {
    ui.caWhereComboBox->setCurrentIndex(aWhereComboxIndex) ;
  }


  connect(ui.bottomButtonsBox, SIGNAL(accepted()), this, SLOT(okButtonClicked()));
  connect(ui.bottomButtonsBox, SIGNAL(rejected()), this, SLOT(cancelButtonClicked()));

  connect(ui.addAttachmentBtn, SIGNAL(clicked()),
	  this, SLOT(attachButtonClicked()));
}

NewClassifiedAdDialog::~NewClassifiedAdDialog()
{
  LOG_STR("NewClassifiedAdDialog::~NewClassifiedAdDialog\n") ;
}


void NewClassifiedAdDialog::okButtonClicked()
{
  LOG_STR("NewClassifiedAdDialog::okButtonClicked\n") ;

  Hash selectedUserProfileHash ( iController->profileInUse() ) ; 
  CA ad ; 
  if ( selectedUserProfileHash != KNullHash &&
       ui.subjectEdit->text().length() > 0 &&
       ui.adEdit->toHtml().length() > 0 ) {
    if ( iSelectedProfile.iIsPrivate == false ) {
      ad.iSenderName = iSelectedProfile.displayName() ; 
    }
    foreach (const QString& attachmentFile , iFilesAboutToBeAttached ) {
      Hash attachmentHash = publishBinaryAttachment(attachmentFile,
						    true) ;
      if ( attachmentHash != KNullHash ) {
	ad.iAttachedFiles.append(attachmentHash) ; 
      }
    }
    ad.iSubject = ui.subjectEdit->text() ; 
    ad.iMessageText = ui.adEdit->toHtml() ; 
    ad.iGroup = FrontWidget::selectedClassification(*ui.caAboutComboBox,
						    *ui.caRegardingCombobox,
						    *ui.caWhereComboBox,
						    *iController) ; 
    ad.iSenderHash = iSelectedProfile.iFingerPrint ; 
    ad.iTimeOfPublish = QDateTime::currentDateTimeUtc().toTime_t() ; 
    ad.iReplyTo =  iReferences  ; 

    int indexOfAboutCombo ( iController->model().classifiedAdsModel().aboutComboBoxTexts().indexOf(ui.caAboutComboBox->currentText()) ) ;
    int indexOfRegardingCombo ( iController->model().classifiedAdsModel().regardingComboBoxTexts().indexOf(ui.caRegardingCombobox->currentText()) ) ;
    int indexOfWhereCombo ( iController->model().classifiedAdsModel().whereComboBoxTexts().indexOf(ui.caWhereComboBox->currentText()) ) ;

    if ( indexOfAboutCombo != -1 ) {
      ad.iAboutComboBoxIndex = indexOfAboutCombo ; 
    } else {
      ad.iAboutComboBoxText = ui.caAboutComboBox->currentText() ; 
    }

    if ( indexOfRegardingCombo != -1 ) {
      ad.iConcernsComboBoxIndex = indexOfRegardingCombo ; 
    } else {
      ad.iConcernsComboBoxText = ui.caRegardingCombobox->currentText() ; 
    }

    // hopefully the indexing of countries won't change inside Qt??
    // if it does, we'll need to start incorporating the old list
    // inside classified ads or groupings will just .. not work
    if ( indexOfWhereCombo != -1 ) {
      ad.iInComboBoxIndex = indexOfWhereCombo ; 
    } else {
      ad.iInComboBoxText = ui.caWhereComboBox->currentText() ; 
    }

    iController->model().lock() ;
    quint32 dummy  ; 
    if ( 
    iController->model().contentEncryptionModel().PublicKey(iSelectedProfile.iFingerPrint,
							    ad.iProfileKey,
							    &dummy )) {
      ad.iFingerPrint = iController->model().classifiedAdsModel().publishClassifiedAd(iSelectedProfile, ad) ; 
    }
    iController->model().unlock() ; 
  }
  Hash hashOfClassification ; 
  hashOfClassification.fromString(reinterpret_cast<const unsigned char*>(ad.iGroup.toLatin1().constData())) ; 
  iController->model().lock() ;
  iCAListingModel.newCaReceived(ad) ; 
  iController->model().unlock() ; 
  close() ; 
  this->deleteLater() ;
}


void NewClassifiedAdDialog::cancelButtonClicked()
{
  LOG_STR("NewClassifiedAdDialog::cancelButtonClicked\n") ;
  close() ; 
  this->deleteLater() ;
}
