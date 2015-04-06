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
#include "../mcontroller.h"
#include "../log.h"
#include "../datamodel/model.h"
#include "../datamodel/mnodemodelprotocolinterface.h"
#include "manualconnection.h"
#include <QHostInfo>

ManualConnectionDialog::ManualConnectionDialog(QWidget *aParent,
        MController& aController)
    : QDialog(aParent),
      iController(aController) {
    ui.setupUi(this) ;

    connect(ui.connectionDlgButtonBox, SIGNAL(accepted()),
            this, SLOT(addButtonClicked()));
    connect(this, SIGNAL(rejected()),
            this, SLOT(deleteLater())) ;
    adjustSize() ;
}

ManualConnectionDialog::~ManualConnectionDialog() {
    LOG_STR("ManualConnectionDialog::~ManualConnectionDialog\n") ;
}

void ManualConnectionDialog::addButtonClicked() {
    LOG_STR("ManualConnectionDialog::addButtonClicked()\n") ;
    const bool hasIpv6 =
        !Connection::Ipv6AddressesEqual(iController.getNode().ipv6Addr(),
                                        KNullIpv6Addr) ;
    QMessageBox msgBox;
    msgBox.setText(tr("DNS lookup failure")) ;
    msgBox.setStandardButtons( QMessageBox::Ok );
    bool isAddressSet (false) ;
    if ( ui.addressEdit->text().length() > 0 ) {
        QLOG_STR("Entered network addr " + ui.addressEdit->text()) ;
        QHostAddress addrToConnect ;
        QHostInfo info = QHostInfo::fromName(ui.addressEdit->text()) ;
        if ( info.error() == QHostInfo::NoError ) {
            // check for ipv6 addr if we have one
            if ( hasIpv6 ) {
                foreach ( const QHostAddress& result,
                          info.addresses() ) {
                    if ( result.protocol() == QAbstractSocket::IPv6Protocol ) {
                        isAddressSet = true ;
                        addrToConnect.setAddress(result.toIPv6Address()) ;
                        QLOG_STR("Found valid IPv6 addr " + addrToConnect.toString()) ;
                        break  ;
                    }
                }
            }
            if ( isAddressSet == false ) {
                foreach ( const QHostAddress& result,
                          info.addresses() ) {
                    if ( result.protocol() == QAbstractSocket::IPv4Protocol ) {
                        isAddressSet = true ;
                        addrToConnect.setAddress(result.toIPv4Address()) ;
                        QLOG_STR("Found valid IPv4 addr " + addrToConnect.toString()) ;
                        break  ;
                    }
                }
            }
            if ( isAddressSet ) {
                Node* n = new Node(KNullHash,
                                   ui.portEdit->value()) ;
                if ( addrToConnect.protocol() == QAbstractSocket::IPv4Protocol ) {
                    n->setIpv4Addr(addrToConnect.toIPv4Address()) ;
                } else {
                    n->setIpv6Addr(addrToConnect.toIPv6Address()) ;
                }
                iController.model().nodeModel().addNodeToConnectionWishList(n) ;
                // nodemodel will delete n
                close() ;
                deleteLater() ;
            } else {
                msgBox.exec();
            }
        } else {
            msgBox.exec();
        }
    }
}




