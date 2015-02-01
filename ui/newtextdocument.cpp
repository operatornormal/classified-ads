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
#include "newtextdocument.h"
#include "../log.h"
#include "../datamodel/model.h"
#include "../FrontWidget.h"
#include "../datamodel/binaryfilemodel.h"
#include "../datamodel/binaryfilelistingmodel.h"
#include "../datamodel/profilemodel.h"
#include "../datamodel/profile.h"

NewTextDocument::NewTextDocument(QWidget *aParent,
				 MController* aController,
				 Profile& aSelectedProfile,
				 BinaryFileListingModel& aProfileFileListingModel)
  : TextEdit(aParent,
	     aController,
	     aSelectedProfile) ,
    iProfileFileListingModel(aProfileFileListingModel)
{
  ui.setupUi(this) ; 
  initializeTextEditor(ui.documentEdit,
		       ui.gridLayout,
		       ui.toolBoxLayoutUpper,
		       ui.toolBoxLayoutLower) ; 

  iAttachmentListLabel = ui.attahcmentsListLabel ;
  // lets not have attachments in here, this thing itself
  // is more or less an attachment ; user can create links and 
  // that is ok
  ui.attahcmentsListLabel->hide() ; 
  ui.attachButton->hide() ; 
  ui.attachmentsLabel->hide() ; 

  connect(ui.bottomButtonsBox, SIGNAL(accepted()), this, SLOT(okButtonClicked()));
  connect(ui.bottomButtonsBox, SIGNAL(rejected()), this, SLOT(cancelButtonClicked()));
  ui.titleEdit->setFocus(Qt::PopupFocusReason) ; 
}

NewTextDocument::~NewTextDocument()
{
  LOG_STR("NewTextDocument::~NewTextDocument") ;
}


void NewTextDocument::okButtonClicked()
{
  LOG_STR("NewTextDocument::okButtonClicked") ;

  QString fileName ;
  if ( ui.titleEdit->text().remove(QChar(' ')).length() < 1 ) {
    fileName = "doc.html" ; 
  } else {
    fileName = ui.titleEdit->text().remove(QChar(' '))+".html" ; 
  }
  QByteArray content ( qCompress( ui.documentEdit->toHtml().toUtf8() ) ) ; 

  iController->model().lock() ; 
  Hash publishedFileHash = 
    iController->model().binaryFileModel().publishBinaryFile(iSelectedProfile,
							     fileName,
							     ui.titleEdit->text(),
							     "application/classified_ads_text",
							     content,
							     true) ;
  iController->model().unlock() ; 
  if ( publishedFileHash != KNullHash ) {
    iProfileFileListingModel.addFile(publishedFileHash) ;
    iSelectedProfile.iTimeOfPublish = QDateTime::currentDateTimeUtc().toTime_t() ;
    iController->model().profileModel().publishProfile(iSelectedProfile) ; 
    close() ; 
    this->deleteLater() ;
  }
}


void NewTextDocument::cancelButtonClicked()
{
  LOG_STR("NewTextDocument::cancelButtonClicked") ;
  close() ; 
  this->deleteLater() ;
}

