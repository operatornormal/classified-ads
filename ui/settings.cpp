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
#include "../mcontroller.h"
#include "../log.h"
#include "../datamodel/model.h"
#include "../datamodel/profilemodel.h"
#include "../datamodel/binaryfilemodel.h"
#include "../datamodel/camodel.h"
#include "../datamodel/privmsgmodel.h"
#include "../datamodel/profilecommentmodel.h"
#include "settings.h"

SettingsDialog::SettingsDialog(QWidget *aParent,
                               MController& aController)
    : QDialog(aParent),
      iController(aController) {
    ui.setupUi(this) ;

    connect(ui.bottomButtonsBox, SIGNAL(accepted()), this, SLOT(okButtonClicked()));
    connect(ui.bottomButtonsBox, SIGNAL(rejected()), this, SLOT(cancelButtonClicked()));

    ui.portEdit->setValue ( iController.model().nodeModel().listenPortOfThisNode() );
    ui.dnsNameEdit->setText ( iController.model().nodeModel().getDnsName() );

    ui.nrProfilesEdit->setValue ( iController.model().profileModel().getMaxRowsToKeep() );
    ui.nrMessagesEdit->setValue ( iController.model().privateMessageModel().getMaxRowsToKeep() );
    ui.nrBinaryFilesEdit->setValue ( iController.model().binaryFileModel().getMaxRowsToKeep() );
    ui.nrAdsEdit->setValue ( iController.model().classifiedAdsModel().getMaxRowsToKeep() );
    ui.nrCommenstEdit->setValue ( iController.model().profileCommentModel().getMaxRowsToKeep() );
    ui.dnsNameEdit->setText( iController.getNode().DNSAddr()) ;
    adjustSize() ;
}

SettingsDialog::~SettingsDialog() {
    LOG_STR("SettingsDialog::~SettingsDialog\n") ;
}


void SettingsDialog::okButtonClicked() {
    LOG_STR("SettingsDialog::okButtonClicked\n") ;

    iController.model().nodeModel().setListenPortOfThisNode(ui.portEdit->value());
    iController.model().nodeModel().setDnsName(ui.dnsNameEdit->text()) ;
    iController.model().profileModel().setMaxRowsToKeep(ui.nrProfilesEdit->value()) ;
    iController.model().privateMessageModel().setMaxRowsToKeep(ui.nrMessagesEdit->value (  ) ) ;
    iController.model().binaryFileModel().setMaxRowsToKeep(ui.nrBinaryFilesEdit->value (  ));
    iController.model().classifiedAdsModel().setMaxRowsToKeep(ui.nrAdsEdit->value (  ));
    iController.model().profileCommentModel().setMaxRowsToKeep(ui.nrCommenstEdit->value (  )) ;
    iController.getNode().setDNSAddr(ui.dnsNameEdit->text( ) ) ;
    close() ;
    this->deleteLater() ;
}


void SettingsDialog::cancelButtonClicked() {
    LOG_STR("SettingsDialog::cancelButtonClicked\n") ;
    close() ;
    this->deleteLater() ;
}

