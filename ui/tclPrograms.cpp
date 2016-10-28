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

#include <QPushButton>
#include <QInputDialog>
#include <QMessageBox>
#include "../mcontroller.h"
#include "../log.h"
#include "../datamodel/model.h"
#include "../datamodel/tclmodel.h"
#include "../datamodel/tclprogram.h"
#include "tclPrograms.h"
#include "../tcl/tclWrapper.h"

TclProgramsDialog::TclProgramsDialog(QWidget *aParent,
                                     MController& aController)
    : QDialog(aParent),
      iController(aController),
      iProgramListingModel(0,1), // 0 rows, 1 column
      iEvalButton(NULL) {
    ui.setupUi(this) ;

    connect (this, SIGNAL(rejected()), this, SLOT(deleteLater())) ;

    iEvalButtonStartText = tr("Evaluate") ;
    iEvalButtonStopText = tr("Stop program");

    iEvalButton = new QPushButton(iEvalButtonStartText, this) ;
    ui.buttonBox->addButton(iEvalButton, QDialogButtonBox::ActionRole);
    connect (iEvalButton, SIGNAL(clicked()), this, SLOT(evalButtonPressed())) ;

    iDeleteButton = new QPushButton("Delete program", this) ;
    ui.buttonBox->addButton(iDeleteButton, QDialogButtonBox::ActionRole);
    connect (iDeleteButton, SIGNAL(clicked()), this, SLOT(discardButtonPressed())) ;

    connect (ui.buttonBox, SIGNAL(clicked(QAbstractButton *)),
             this, SLOT(dialogButtonClicked(QAbstractButton *))) ;

    QMap<QString, Hash> programsInStorage ( aController.model().tclModel().getListOfTclPrograms());
    QLOG_STR("Programs in TCL prog storage = " +
             QString::number(programsInStorage.count())) ;
    int i (0);
    foreach ( const QString& key, programsInStorage.keys() ) {
        QLOG_STR("Setting to listing model edit role hash " +
                 programsInStorage.value(key).toString() +
                 " for program " +
                 key ) ;
        QStandardItem* modelItem  = new QStandardItem ( key );
        modelItem->setData(programsInStorage.value(key).toQVariant()) ;
        iProgramListingModel.insertRow(i++, modelItem) ;
    }
    ui.listView->setModel(&iProgramListingModel) ;
    connect( ui.listView,SIGNAL( activated(const QModelIndex &) ),
             this, SLOT( programInListActivated(const QModelIndex &) ) ) ;
}

TclProgramsDialog::~TclProgramsDialog() {
    LOG_STR("TclProgramsDialog::~TclProgramsDialog") ;
    if ( ui.listView ) {
        ui.listView->setModel(NULL) ;
    }
    iEvalButton = NULL ; // will be automatically deleted, no need here
}

void TclProgramsDialog::evalButtonPressed() {
    LOG_STR("TclProgramsDialog::evalButtonPressed") ;
    if ( iController.tclWrapper().isRunning() == false ) {
        if ( ui.commandInputEdit->toPlainText().length() > 0 ) {
            emit evalScript(ui.commandInputEdit->toPlainText(),
                            iNameOfCurrentProgram.length() > 0 ?
                            &iNameOfCurrentProgram :
                            NULL ) ;
        }
    } else {
        iController.tclWrapper().stopScript() ;
    }
}

void TclProgramsDialog::saveButtonPressed() {
    LOG_STR("TclProgramsDialog::saveButtonPressed") ;
    bool ok;
    QString name = QInputDialog::getText(this, tr("TCL Program name"),
                                         tr("Name:"), QLineEdit::Normal,
                                         iNameOfCurrentProgram, &ok);
    if (ok && !name.isEmpty()) {
        TclProgram prog ;
        prog.setProgramName(name) ;
        prog.setProgramText(ui.commandInputEdit->toPlainText()) ;
        prog.iTimeOfPublish = QDateTime::currentDateTimeUtc().toTime_t() ;
        Hash savedProgramFingerPrint ;
        savedProgramFingerPrint =
            iController
            .model()
            .tclModel()
            .locallyStoreTclProgram(prog,
                                    name == iNameOfCurrentProgram ? iFingerPrintOfCurrentProgram : KNullHash)  ;
        QLOG_STR("At save, fingerprint of new prog = " + savedProgramFingerPrint.toString()) ;
        if ( name == iNameOfCurrentProgram ) {
            // same name, replace in index
            QModelIndex currentIndex ( ui.listView->currentIndex() ) ;
            if ( currentIndex.isValid() ) {
                QStandardItem* currentItem ( iProgramListingModel.itemFromIndex(currentIndex) );
                if ( currentItem ) {
                    currentItem->setData(savedProgramFingerPrint.toQVariant()) ;
                }
            }
        } else {
            // same name, add to listing
            QStandardItem* modelItem  = new QStandardItem ( name );
            modelItem->setData(savedProgramFingerPrint.toQVariant()) ;
            iProgramListingModel.insertRow(iProgramListingModel.rowCount(),
                                           modelItem) ;
        }
        iFingerPrintOfCurrentProgram = savedProgramFingerPrint ;
        iNameOfCurrentProgram = name ;
    }
}

void TclProgramsDialog::discardButtonPressed() {
    LOG_STR("TclProgramsDialog::discardButtonPressed") ;
    if ( iFingerPrintOfCurrentProgram != KNullHash ) {
        QMessageBox::StandardButton reply;
        reply = QMessageBox::question(this, tr("Deletion confirmation"),
                                      tr("Permanently delete program %1?").arg(iNameOfCurrentProgram),
                                      QMessageBox::Yes|QMessageBox::No);
        if (reply == QMessageBox::Yes) {
            if ( iController
                    .model()
                    .tclModel()
                    .discardTclProgram(iFingerPrintOfCurrentProgram) ) {
                QModelIndex currentIndex ( ui.listView->currentIndex() ) ;
                if ( currentIndex.isValid() ) {
                    iProgramListingModel.removeRows(currentIndex.row(),1) ;
                }
            }
        }
    }
}

void TclProgramsDialog::dialogButtonClicked(QAbstractButton *aButton) {
    QDialogButtonBox::StandardButton standardButton = ui.buttonBox->standardButton(aButton);
    switch(standardButton) {
    // Standard buttons:
    case QDialogButtonBox::Save:
        saveButtonPressed() ;
        break;

    // Non-standard buttons:
    case QDialogButtonBox::NoButton:
        // do no thing: close is handled already in dialog
        // and eval-button has its own signal
        break;

    default:
        QLOG_STR("TclProgramsDialog::dialogButtonClicked but was not handled");
        break ;
    }
}

void TclProgramsDialog::tclProgramStarted() {
    QLOG_STR("TclProgramsDialog::tclProgramStarted") ;
    if ( iEvalButton ) {
        iEvalButton->setText(iEvalButtonStopText) ;
    }
}

void TclProgramsDialog::tclProgramStopped() {
    QLOG_STR("TclProgramsDialog::tclProgramStopped") ;
    if ( iEvalButton ) {
        iEvalButton->setText(iEvalButtonStartText) ;
    }
}

void TclProgramsDialog::programInListActivated(const QModelIndex &aIndex) {

    Hash originallyActivatedProgram ( iFingerPrintOfCurrentProgram ) ;
    if ( aIndex.isValid() ) {
        QStandardItem *activatedItem ( iProgramListingModel.itemFromIndex ( aIndex ) ) ;
        if ( activatedItem ) {
            iFingerPrintOfCurrentProgram.fromQVariant(activatedItem->data()) ;
            iNameOfCurrentProgram = activatedItem->data(Qt::DisplayRole).toString() ;
        }
    }
    QLOG_STR("programInListActivated " + iFingerPrintOfCurrentProgram.toString()) ;
    if ( originallyActivatedProgram != iFingerPrintOfCurrentProgram ) {
        // program changed, get program text from storage:
        const TclProgram p ( iController
                             .model()
                             .tclModel()
                             .tclProgramByFingerPrint(iFingerPrintOfCurrentProgram));
        if ( p.iFingerPrint != KNullHash ) {
            ui.commandInputEdit->document()->setPlainText(p.programText()) ;
        }
    }
}
