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
#include <QFileDialog>
#include <QLabel>
#include "dialogbase.h"
#include "../mcontroller.h"
#include "../log.h"
#include "../datamodel/model.h"
#include "../datamodel/profile.h"
#include "../datamodel/binaryfilemodel.h"

DialogBase::DialogBase (QWidget* aParent,
			MController* aController,
			Profile& aSelectedProfile) 
  : QDialog(aParent),
    iController(aController),
    iSelectedProfile(aSelectedProfile)
{

}

Hash DialogBase::publishBinaryAttachment(const MetadataQueryDialog::MetadataResultSet& aFileMetadata,
					 bool aForceNoEncryption,
					 const QList<Hash>* aBinaryRecipientList) 
{
  Hash retval ( KNullHash ) ; 
  bool isCompressed (false ) ; 

  QFile f ( aFileMetadata.iFileName ) ;
  if ( f.open(QIODevice::ReadOnly) ) {
    if ( f.size() > ( KMaxProtocolItemSize * 10 ) ) {
      emit error ( MController::FileOperationError, tr("File way too big")) ; 
    } else {
      QByteArray content ( f.read(f.size() ) ) ; 
      QByteArray compressedContent ( qCompress(content) ) ; 
      LOG_STR2("Size of content = %d", (int) (content.size())) ; 
      LOG_STR2("Size of compressed = %d", (int) (compressedContent.size())) ; 
      if (content.size() > (qint64)(( KMaxProtocolItemSize - ( 50*1024 ) )) &&
	  compressedContent.size() > (qint64)(( KMaxProtocolItemSize - ( 50*1024 ) )) ) {
	emit error ( MController::FileOperationError, tr("File too big")) ; 
      } else {
	if ( compressedContent.size() < content.size () ) {
	  content.clear() ; 
	  content.append(compressedContent) ; 
	  compressedContent.clear() ; 
	  isCompressed = true ; 
	} else {
	  compressedContent.clear() ; 
	}
	// ok, here we have content in content and it is 
	// either compressed or not. size is checked. 
	iController->model().lock() ; 
	retval= 
	  iController->model().
	  binaryFileModel().publishBinaryFile(iSelectedProfile,
					      QFileInfo(f).fileName(),
					      aFileMetadata.iDescription,
					      aFileMetadata.iMimeType,
					      aFileMetadata.iOwner,
					      aFileMetadata.iLicense,
					      content,
					      isCompressed,
					      aForceNoEncryption,
					      aBinaryRecipientList)  ;
	
	iController->model().unlock() ; 
      }
    }
  } else {
    emit error ( MController::FileOperationError, tr("File open error")) ; 
  }
  return retval ; 
}


void DialogBase::attachButtonClicked() {
  LOG_STR("DialogBase::attachButtonClicked\n") ;

  QString fileName = QFileDialog::getOpenFileName(this, tr("Select file to be published"),
						  "",
						  tr("Files (*.*)"));
  if ( fileName.length() > 0 ) {
    QFile f ( fileName ) ;
    if ( f.open(QIODevice::ReadOnly) ) {
      if ( f.size() > ( KMaxProtocolItemSize * 10 ) ) {
	emit error ( MController::FileOperationError, tr("File way too big")) ; 
      } else {
	QByteArray content ( f.read(f.size() ) ) ; 
	QByteArray compressedContent ( qCompress(content) ) ; 
	LOG_STR2("Size of content = %d", (int) (content.size())) ; 
	LOG_STR2("Size of compressed = %d", (int) (compressedContent.size())) ; 
	if (content.size() > (qint64)(( KMaxProtocolItemSize - ( 50*1024 ) )) &&
	    compressedContent.size() > (qint64)(( KMaxProtocolItemSize - ( 50*1024 ) )) ) {
	  emit error ( MController::FileOperationError, tr("File too big")) ; 
	} else {
	  // file was not too big. here now just mark the filename 
	  // for later use.
	  if ( iFilesAboutToBeAttached.count() != 0 ) {
	    iFilesAboutToBeAttached.clear() ;
	  }
	  MetadataQueryDialog::MetadataResultSet metadata ;
	  metadata.iFileName = fileName ; 
	  MetadataQueryDialog metadataDialog(this,
					     *iController,
					     metadata ) ;
	  if ( metadataDialog.exec() == QDialog::Accepted ) {
	    iFilesAboutToBeAttached.append(metadata) ;
	  
	    iAttachmentListLabel->setText(metadata.iFileName) ;  	     
	  } else {
	    iAttachmentListLabel->setText(QString()) ;  	     
	  }
	}
      }
    } else {
      emit error ( MController::FileOperationError, tr("File open error")) ; 
    }
  }
}

