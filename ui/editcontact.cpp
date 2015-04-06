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
#include "newclassifiedaddialog.h"
#include "../mcontroller.h"
#include "../log.h"
#include "../datamodel/model.h"
#include "../FrontWidget.h"
#include "../datamodel/contactlistingmodel.h"
#include "../datamodel/trusttreemodel.h"
#include "editcontact.h"

EditContactDialog::EditContactDialog(QWidget *aParent,
                                     MController* aController,
                                     const Hash& aProfileFingerPrint,
                                     const QString& aNickName,
                                     bool aIsTrusted,
                                     ContactListingModel& aContactsModel)
    : QDialog(aParent),
      iController(aController),
      iContactsModel(aContactsModel) {
    ui.setupUi(this) ;

    connect(ui.bottomButtonsBox, SIGNAL(accepted()), this, SLOT(okButtonClicked()));
    connect(ui.bottomButtonsBox, SIGNAL(rejected()), this, SLOT(cancelButtonClicked()));

    // if we have the contact already, setup the UI from that:
    Contact c ;
    if (  aContactsModel.contactByFingerPrint(aProfileFingerPrint, &c) ) {
        ui.hashEdit->setText(aProfileFingerPrint.toString()) ;
        ui.nickNameEdit->setText(c.iNickName) ;
        ui.trustedCheckBox->setChecked(c.iIsTrusted) ;
    } else {
        if (aProfileFingerPrint != KNullHash ) {
            ui.hashEdit->setText(aProfileFingerPrint.toString()) ;
        }
        ui.nickNameEdit->setText(aNickName) ;
        ui.trustedCheckBox->setChecked(aIsTrusted) ;
    }
}

EditContactDialog::~EditContactDialog() {
    LOG_STR("EditContactDialog::~EditContactDialog\n") ;
}


void EditContactDialog::okButtonClicked() {
    LOG_STR("EditContactDialog::okButtonClicked\n") ;

    Contact c ;
    c.iFingerPrint.fromString(reinterpret_cast<const unsigned char *>(ui.hashEdit->text().toLatin1().constData())) ;
    if ( c.iFingerPrint == KNullHash ) {
        QMessageBox::about(this, tr("Error"),
                           tr("Operator addr is not valid")) ;
    } else {
        c.iNickName = ui.nickNameEdit->text() ;
        c.iIsTrusted = ui.trustedCheckBox->isChecked() ;
        iController->model().lock() ;
        iContactsModel.newContactAdded(c) ;
        iController->offerDisplayNameForProfile(c.iFingerPrint,
                                                c.iNickName,
                                                true) ;
        iController->storePrivateDataOfSelectedProfile(true) ; // true == publish trust list too
        // trust tree model will obtain the "self" profile from
        // profilemodel so this notify here must be given after
        // the profile has been persisted into permanent storage
        iController->model().trustTreeModel()->offerTrustList(iController->profileInUse(),
                QString(),
                iContactsModel.trustList()) ;
        iController->model().unlock() ;
        close() ;
        this->deleteLater() ;
    }
}


void EditContactDialog::cancelButtonClicked() {
    LOG_STR("EditContactDialog::cancelButtonClicked\n") ;
    close() ;
    this->deleteLater() ;
}

