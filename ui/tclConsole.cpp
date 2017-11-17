/*     -*-C++-*- -*-coding: utf-8-unix;-*-
  Classified Ads is Copyright (c) Antti Järvinen 2015-2016.

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
#include "tclConsole.h"

TclConsoleDialog::TclConsoleDialog(QWidget *aParent,
                                   MController& aController)
    : QDialog(aParent),
      iController(aController) {
    ui.setupUi(this) ;

    connect (this, SIGNAL(rejected()), this, SLOT(deleteLater())) ;
    connect (ui.evalButton, SIGNAL(clicked()), this, SLOT(evalButtonPressed())) ;
    ui.listView->setModel(&iConsoleOutputText) ;
}

TclConsoleDialog::~TclConsoleDialog() {
    LOG_STR("TclConsoleDialog::~TclConsoleDialog") ;
    if ( ui.listView ) {
        ui.listView->setModel(NULL) ;
    }
}

void TclConsoleDialog::consoleOutput(QString aMessage) {
    LOG_STR("TclConsoleDialog::consoleOutput " + aMessage) ;

    iConsoleOutputText.insertRow(iConsoleOutputText.rowCount());
    QModelIndex index =
        iConsoleOutputText.index(iConsoleOutputText.rowCount()-1) ;
    iConsoleOutputText.setData(index, aMessage);
    ui.listView->scrollToBottom() ;
}

void TclConsoleDialog::evalButtonPressed() {
    LOG_STR("TclConsoleDialog::evalButtonPressed") ;
    if ( ui.commandInputEdit->toPlainText().length() > 0 ) {
        emit evalScript(ui.commandInputEdit->toPlainText(),NULL) ;
    }
}
