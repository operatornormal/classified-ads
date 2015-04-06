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
#include "passwd_dialog.h"
#include "../mcontroller.h"
#include "../log.h"
#include "../datamodel/model.h"
#include "../datamodel/contentencryptionmodel.h"

PasswdDialog::PasswdDialog(QWidget *aParent,
                           MController& aController,
                           const QString& aPrompt,
                           bool aIsPwdQueryDialog)
    : QDialog(aParent),
      iController(aController),
      iPrompt(aPrompt),
      iIsPwdQueryDialog(aIsPwdQueryDialog) {
    ui.setupUi(this) ;

    connect(ui.okButton, SIGNAL(clicked()),
            this, SLOT(okClicked()));

    ui.passwordPromptLabel->setText( aPrompt );
    ui.passwordEdit->setFocus() ;
    LOG_STR("### PasswdDialog constructor out") ;
}
PasswdDialog::~PasswdDialog() {
    LOG_STR("PasswdDialog::~PasswdDialog") ;
}

void PasswdDialog::okClicked() {
    QString text = ui.passwordEdit->text();
    bool goodPasswordFound = false ;
    // 5 characters is damn lousy passwd.
    // this implementation does not restrict user to
    // any particular passwd max len, informed users are
    // still free to use long passphrases.
    if ( text.size() > 4 ) {
        if ( iIsPwdQueryDialog ) {
            iController.model().lock() ;
            iController.setContentKeyPasswd(text) ;
            QList<Hash> privateKeys = iController.model().contentEncryptionModel().listKeys(true,NULL)  ;
            if ( privateKeys.size() == 0 ) {
                // because we had no secret key of any kind lets generate one
                iController.setProfileInUse(iController.model().contentEncryptionModel().generateKeyPair()) ;
                close() ;
                this->deleteLater() ;
            } else {
                // try each private key we have, if we have one
                // where the password matches then use that
                for ( int i = 0 ; i  < privateKeys.size() ; i++ ) {
                    Hash keyFingerPrint = privateKeys[i] ;
                    QLOG_STR("Trying key fp " + keyFingerPrint.toString()) ;
                    QByteArray PEMBytes  ;
                    if ( iController.model().contentEncryptionModel().PrivateKey(keyFingerPrint,PEMBytes) == true ) {
                        EVP_PKEY* privateKey = iController.model().contentEncryptionModel().PrivateKeyFromPem(PEMBytes,false) ;
                        if ( privateKey != NULL ) {
                            // if we came here e.g. got the key, it is a sign
                            // that we also had the correct password to open the key
                            iController.setProfileInUse(keyFingerPrint) ;
                            EVP_PKEY_free(privateKey) ;
                            close() ;
                            QLOG_STR("Using profile " + keyFingerPrint.toString()) ;
                            this->deleteLater() ;
                            goodPasswordFound = true ;
                        }
                    }
                }
                if ( !goodPasswordFound )   {
                    QString errmsg("Bad password") ;
                    emit error(MController::BadPassword, errmsg) ;
                }
            }
            iController.model().unlock() ;
        } else {
            // this is password change dialog
            iController.model().lock() ;
            if ( iController.model().contentEncryptionModel().changeKeyPassword(iController.profileInUse(),text) == 0 ) {
                iController.setContentKeyPasswd(text) ;
            }
            iController.model().unlock() ;
            close() ;
            this->deleteLater() ;
        }
    } else {
        QString errmsg(tr("Min length 5 (use 10+)")) ;
        emit error(MController::ContentEncryptionError, errmsg) ;
    }
}
