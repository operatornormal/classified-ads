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
#include <QMessageBox>
#include <QUrl>
#include <QPushButton>
#include "insertlinkdialog.h"
#include "../mcontroller.h"
#include "../log.h"
#include "../datamodel/model.h"

InsertLinkDialog::InsertLinkDialog(QWidget *aParent,
                                   MController* aController)
    : QDialog(aParent),
      iController(aController) {
    ui.setupUi(this) ;
    connect(ui.urlEdit,
            SIGNAL(textEdited(const QString&)),
            this,
            SLOT(urlTextEdited(const QString&)),
            Qt::QueuedConnection) ;
    ui.bottomButtonsBox->button( QDialogButtonBox::Ok )->setEnabled( false );
    connect(ui.bottomButtonsBox, SIGNAL(accepted()), this, SLOT(okButtonClicked()));
    connect(ui.bottomButtonsBox, SIGNAL(rejected()), this, SLOT(deleteLater()));
    ui.urlEdit->setFocus(Qt::PopupFocusReason) ;
}

InsertLinkDialog::~InsertLinkDialog() {
    LOG_STR("InsertLinkDialog::~InsertLinkDialog\n") ;
}


void InsertLinkDialog::okButtonClicked() {
    LOG_STR("InsertLinkDialog::okButtonClicked\n") ;
    QString url ;
    // first try parse the text in editor without scheme
    // from listbox.
    if ( ui.urlEdit->text().indexOf("://") > -1 ) {
        // user did give scheme
        url =  ui.urlEdit->text()  ;
    } else {
        // take scheme from combobox
        url =  ui.schemeComboxBox->currentText() +
               "://" +
               ui.urlEdit->text()  ;
    }
    QUrl parsedUrl = QUrl::fromUserInput(url) ;
    url = QString(parsedUrl.toEncoded()) ;
    if ( !parsedUrl.isValid() ) {
        QMessageBox errorMessage ;
        errorMessage.setText(tr("Invalid URL"));
        errorMessage.setStandardButtons( QMessageBox::Ok );
        errorMessage.exec();
        return ;
    }

    QString label ( ui.linkLabelEdit->text() );
    if ( label.length() == 0 ) {
        label = url ;
    }
    // then check that if it was classified ads URL, that the
    // hash must parse.
    Hash parsedHash ;
    parsedHash.fromString(reinterpret_cast<const unsigned char *>(parsedUrl.host().toLatin1().constData())) ;
    if ( parsedHash == KNullHash &&
            ( parsedUrl.scheme() == "caprofile" ||
              parsedUrl.scheme() == "caad" ||
              parsedUrl.scheme() == "cacomment" ||
              parsedUrl.scheme() == "cablob" ) ) {
        QLOG_STR("Hash was not valid") ;
        QMessageBox errorMessage ;
        errorMessage.setText("Invalid SHA1 with scheme " + parsedUrl.scheme());
        errorMessage.setStandardButtons( QMessageBox::Ok );
        errorMessage.exec();
        return ;
    } else {
        QLOG_STR("Hash is " + parsedHash.toString()) ;
        QLOG_STR("Url " + url ) ;
        QLOG_STR("Label " + label ) ;
        emit linkReady(url,
                       label ) ;
        close() ;
        deleteLater();
    }

}

void InsertLinkDialog::urlTextEdited(const QString &aText) {
    if ( aText.length() == 0 ) {
        ui.bottomButtonsBox->button( QDialogButtonBox::Ok )->setEnabled( false );
    } else {
        ui.bottomButtonsBox->button( QDialogButtonBox::Ok )->setEnabled( true );
    }
}
