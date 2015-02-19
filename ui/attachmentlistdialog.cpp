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
#include <QFileDialog>
#include "attachmentlistdialog.h"
#include "../mcontroller.h"
#include "../log.h"
#include "../datamodel/model.h"
#include "../FrontWidget.h"
#include "../datamodel/binaryfilelistingmodel.h"
#include "editcontact.h"
#include "../datamodel/binaryfilemodel.h"
#include "../datamodel/binaryfile.h"
#include "../datamodel/profile.h"
#include "../datamodel/profilemodel.h"

AttachmentListDialog::AttachmentListDialog(QWidget *aParent,
					   MController* aController,
					   Profile& aSelectedProfile,
					   QList<Hash>& aFilesToDisplay,
					   const Hash& aNodeToTryForRetrieval)
  : DialogBase(aParent,
	       aController,
	       aSelectedProfile),
    iListingModel(NULL),
    iExportSharedFileAction(NULL),
    iNodeToTryForRetrieval(aNodeToTryForRetrieval)
{
  ui.setupUi(this) ; 
  iListingModel = new BinaryFileListingModel(aFilesToDisplay) ; 
  ui.fileListView->setModel(iListingModel) ; 
  connect(ui.bottomButtonsBox, SIGNAL(accepted()), this, SLOT(okButtonClicked()));
  connect(ui.bottomButtonsBox, SIGNAL(rejected()), this, SLOT(cancelButtonClicked()));
  connect(ui.fileListView,
	  SIGNAL(doubleClicked(const QModelIndex&)),
	  this,
	  SLOT(fileListDoubleClicked(const QModelIndex&))) ; 
  iExportSharedFileAction = new QAction(tr("Save file to disk.."),this) ; 
  ui.fileListView->setContextMenuPolicy(Qt::ActionsContextMenu);
  ui.fileListView->addAction(iExportSharedFileAction) ;
  connect(iExportSharedFileAction, SIGNAL(triggered()),
	  this, SLOT(exportSharedFile())) ;
}

AttachmentListDialog::~AttachmentListDialog()
{
  LOG_STR("AttachmentListDialog::~AttachmentListDialog\n") ;
  ui.fileListView->setModel(NULL) ; 
  delete iListingModel; 
  delete iExportSharedFileAction ; 
}


void AttachmentListDialog::okButtonClicked()
{
  LOG_STR("AttachmentListDialog::okButtonClicked\n") ;
  // display or otherwise perform some file action here..
  exportSharedFile() ; 
}


void AttachmentListDialog::cancelButtonClicked()
{
  LOG_STR("AttachmentListDialog::cancelButtonClicked\n") ;
  close() ; 
  this->deleteLater() ;
}


void AttachmentListDialog::fileListDoubleClicked(const QModelIndex& /*aIndex*/)
{
  LOG_STR("fileListDoubleClicked\n") ;
  exportSharedFile() ; 
}

void AttachmentListDialog::exportSharedFile() {
  LOG_STR("exportSharedFile") ;
  // ok, see what user had selected ; one file only:
  bool netRequestStarted(false) ; 
  QByteArray fileData ; 
  BinaryFile* metadata (NULL); 
  iController->model().lock() ; 
  if ( iListingModel ) {
    Hash fingerPrint ;

    foreach(const QModelIndex &index, 
	    ui.fileListView->selectionModel()->selectedIndexes()) {
      fingerPrint.fromString((const unsigned char *)(qPrintable(iListingModel->data(index,Qt::ToolTipRole).toString())));
      break ; 
    }

    if ( fingerPrint != KNullHash ) {
      // aye, found a selected fingerprint; first check if we have 
      //  the file or just know the fingerprint. both are possible. 
      if ( ( metadata = iController->model().binaryFileModel().binaryFileByFingerPrint(fingerPrint) ) == NULL ) {
	// got no file, ask it to be retrieved:
	NetworkRequestExecutor::NetworkRequestQueueItem req ;
	req.iRequestType = RequestForBinaryBlob ;
	req.iRequestedItem = fingerPrint ;
	req.iState = NetworkRequestExecutor::NewRequest ; 
	req.iMaxNumberOfItems = 1 ; 
	// if the file was shared by some other operator, 
	// ask node of that operator first .. who was the operator?
	if (iNodeToTryForRetrieval != KNullHash ) {
	  req.iDestinationNode = iNodeToTryForRetrieval;
	}
	iController->startRetrievingContent(req,true,BinaryBlob) ; 
	netRequestStarted = true ; 
      } else {
	QByteArray fileSignature ; 
	Hash fileOwnerFingerPrint ; 
	fileOwnerFingerPrint.fromString((const unsigned char *)(metadata->iOwner.toLatin1().constData())) ;
	bool dummy ; 
	iController->model().binaryFileModel().setTimeLastReference(fingerPrint, QDateTime::currentDateTimeUtc().toTime_t()) ; 
	if ( !iController->model().binaryFileModel().binaryFileDataByFingerPrint(fingerPrint,
										 fileOwnerFingerPrint,
										 fileData,
										 fileSignature,
										 &dummy) ) {
	  LOG_STR("Got no file?") ;
	}
      }
    }
  }
  iController->model().unlock() ; 
  if ( fileData.size() > 0 && metadata ) {
    int periodPosition = metadata->iFileName.lastIndexOf(".") ;
    QString suffix ; 
    if ( periodPosition > 0 ) {
      QString filenameSuffix = metadata->iFileName.mid(periodPosition+1) ;
      suffix = filenameSuffix + " "+tr("files")+" (*."+filenameSuffix+")" ;
    } else {
      suffix = tr("files")+" (*.*)" ;
    }
    QString fileName = QFileDialog::getSaveFileName(this, tr("Choose file name for saving"),
						    metadata->iFileName,
						    suffix);
    if ( fileName.length() > 0 ) {
      QFile f ( fileName ) ; 
      if ( f.open(QIODevice::WriteOnly) ) {
	f.write(fileData) ;
	f.close() ;
	close() ;  // close this dialog after successful save
	this->deleteLater() ;
      } else {
	QMessageBox::about(this,tr("Error"),
			   tr("File open error"));
      }
    }
  }
  delete metadata ; 
  if ( netRequestStarted ) {
    iController-> userInterfaceAction ( MController::DisplayProgressDialog,
					KNullHash ,
					KNullHash ) ; 
  }
}
	       
Hash AttachmentListDialog::tryFindNodeByProfile(const Hash& aProfileFingerPrint, MController& aController)
{
  Hash retval ;
  Profile *recipientProfile (NULL) ;
  if ( ( recipientProfile = 
	 aController.model().profileModel().profileByFingerPrint(aProfileFingerPrint,
								 false/* do not emit encryption errors*/,
								 true /* omit image */) ) != NULL ) {
    if ( recipientProfile->iNodeOfProfile ) {
      retval = recipientProfile->iNodeOfProfile->nodeFingerPrint() ; 
    }
    delete recipientProfile ; 
  }
  return retval ; 
} 
