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
#if QT_VERSION >= 0x050000
// qt5 has its own mime-type-handling so lets depend on that, easier..
#include <QMimeDatabase>
#else
// with qt4 use libmagic that does the same job
#include <magic.h> // from libmagic
#endif
#include <QFile>
#include "../mcontroller.h"
#include "../log.h"
#include "../datamodel/model.h"
#include "metadataQuery.h"

MetadataQueryDialog::MetadataQueryDialog(QWidget *aParent,
					 MController& aController,
					 MetadataResultSet& aResultsSet)
  : QDialog(aParent),
    iController(aController),
    iResultsSet ( aResultsSet )
{
  ui.setupUi(this) ; 
  ui.fileNameEdit->setText(aResultsSet.iFileName) ; 
  if ( aResultsSet.iMimeType.length() == 0 ) {
    ui.mimetypeEdit->setText(findMimeTypeForFile(aResultsSet.iFileName) ) ; 
  }
  connect (ui.bottomButtonsBox, SIGNAL(rejected()), this, SLOT(deleteLater())) ;
  connect (ui.bottomButtonsBox, SIGNAL(accepted()), this, SLOT(okButtonClicked())) ;
}

MetadataQueryDialog::~MetadataQueryDialog() {
  LOG_STR("MetadataQueryDialog::~MetadataQueryDialog\n") ;
}

void MetadataQueryDialog::okButtonClicked() {
  LOG_STR("MetadataQueryDialog::okButtonClicked\n") ;
  iResultsSet.iMimeType = ui.mimetypeEdit->text() ; 
  iResultsSet.iDescription = ui.descriptionEdit->text() ; 
  iResultsSet.iOwner = ui.ownerEdit->text() ; 
  iResultsSet.iLicense = ui.licenseComboBox->currentText() ; 
  done(QDialog::Accepted) ; 
  close() ; 
  deleteLater() ; 
}


QString MetadataQueryDialog::findMimeTypeForFile(const QString& aFileName) {
  QString retval ; 
#if QT_VERSION >= 0x050000
  if ( QFile(aFileName).exists() ) {
    QMimeDatabase db ; 
    QMimeType detectedType ( db.mimeTypeForFile(aFileName) ) ; 
    if ( detectedType.isValid() ) {
      retval = detectedType.name() ; 
    }
  }
#else
  if ( QFile(aFileName).exists() ) {
    // what is going to happen if the filesystem is not UTF-8 fs ???
    const char *fileNameCStyleHeap (aFileName.toUtf8().constData()) ; 
    // serious magick here. if we offer to libmagick the heap buffer
    // from aFileName.toUtf8().constData() then it will not open the
    // file .. so lets allocate a stack buffer, as it seems to work.
    char fileNameCStyle[1024]  ;
    if ( strlen(fileNameCStyleHeap) < 1024 ) {
      strncpy(fileNameCStyle, fileNameCStyleHeap, 1023) ; 
      // code stolen from http://vivithemage.co.uk/blog/?p=105 so thanks Vivi. 

      magic_t magic_cookie;
      /*MAGIC_MIME tells magic to return a mime of the file, but you can specify different things*/
      magic_cookie = magic_open(MAGIC_MIME);
      if (magic_cookie == NULL) {
	QLOG_STR("unable to initialize magic library");
	return retval ; 
      }
      if (magic_load(magic_cookie, NULL) != 0) {
	LOG_STR2("cannot load magic database - %s", magic_error(magic_cookie));
	magic_close(magic_cookie);
	return retval ; 
      }
      LOG_STR2("Filename for magic = >%s<", fileNameCStyle);
      const char *magic_full( magic_file(magic_cookie, fileNameCStyle) ) ;
      if ( magic_full == NULL ) {
	LOG_STR2("Magic_file error - %s", magic_error(magic_cookie));
      } else {
	retval = QString::fromUtf8(magic_full) ; 
      }
      magic_close(magic_cookie);
    }
  }
#endif
  return retval ; 
}
