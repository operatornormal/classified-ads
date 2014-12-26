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
#include "../mcontroller.h"
#include "../log.h"
#include "../datamodel/model.h"
#include "../datamodel/connectionlistingmodel.h"
#include "status.h"
#include "manualconnection.h"

StatusDialog::StatusDialog(QWidget *aParent,
			       MController& aController)
  : QDialog(aParent),
    iController(aController),
    iListingModel(NULL) 
{
  ui.setupUi(this) ; 

  connect(ui.addConnectionButton, SIGNAL(clicked()), this, SLOT(addButtonClicked()));

  ui.portDisplay->setText ( QString::number(iController.model().nodeModel().listenPortOfThisNode()) );
  if ( iController.getNode().ipv4Addr() ) {
    QHostAddress ipv4 ( iController.getNode().ipv4Addr() ) ;
    ui.ipv4display->setText ( ipv4.toString()) ;
  }
  if ( Connection::Ipv6AddressesEqual(iController.getNode().ipv6Addr(),
				      KNullIpv6Addr ) == false ) {
    QHostAddress ipv6 ( iController.getNode().ipv6Addr() ) ;
    ui.ipv6display->setText ( ipv6.toString()) ;
  }
  adjustSize() ;
  connect (this, SIGNAL(rejected()), this, SLOT(deleteLater())) ;
  iListingModel = new   ConnectionListingModel(iController.model(),
					       iController) ; 
  ui.connectionsView->setModel(iListingModel) ; 
}

StatusDialog::~StatusDialog()
{
  LOG_STR("StatusDialog::~StatusDialog\n") ;
  ui.connectionsView->setModel(NULL) ; 
  delete iListingModel ; 
}

void StatusDialog::addButtonClicked()
{
  LOG_STR("StatusDialog::addButtonClicked()\n") ;
  const QString dlgName ("classified_ads_manual_connection_dialog") ; 
  ManualConnectionDialog *dialog = this->findChild<ManualConnectionDialog *>(dlgName) ;
  if ( dialog == NULL ) {
    dialog = new ManualConnectionDialog(this, iController) ;
    dialog->setObjectName(dlgName) ; 
    dialog->show() ;
    connect(this, SIGNAL(destroyed()),
	    dialog, SLOT(reject()));
  } else {
    dialog->setFocus(Qt::MenuBarFocusReason) ;
  }
}




