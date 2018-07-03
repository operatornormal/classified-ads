/*  -*-C++-*- -*-coding: utf-8-unix;-*-
  Classified Ads is Copyright (c) Antti Järvinen 2013-2018.

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
#include <QAction>
#include "profilereadersdialog.h"
#include "../mcontroller.h"
#include "../log.h"
#include "../datamodel/model.h"
#include "../datamodel/profilesearchmodel.h"
#include "../datamodel/profilereaderslistingmodel.h"
#include "../datamodel/profile.h"

ProfileReadersDialog::ProfileReadersDialog(QWidget *aParent,
        MController* aController,
        Profile &aProfile)
    : QDialog(aParent),
      iController(aController),
      iSearchModel(NULL),
      iProfile(aProfile),
      iListingModel(NULL),
      iViewProfileAction(NULL) {
    ui.setupUi(this) ;
    connect(ui.closeButton, SIGNAL(clicked()),
            this, SLOT(closeButtonClicked()));
    connect(ui.addSelectedButton, SIGNAL(clicked()),
            this, SLOT(addButtonClicked()));
    connect(ui.removeSelectedItemsButton, SIGNAL(clicked()),
            this, SLOT(removeButtonClicked()));
    iSearchModel = new ProfileSearchModel(iController->model()) ;
    connect(iSearchModel,
            SIGNAL(  error(MController::CAErrorSituation,
                           const QString&) ),
            iController,
            SLOT(handleError(MController::CAErrorSituation,
                             const QString&)),
            Qt::QueuedConnection) ;
    ui.listView->setModel(iSearchModel) ;
    iTimerId = startTimer(1000);   // 1-second timer
    iPreviousSearchFieldContent = ui.searchEdit->text() ;
    iListingModel = new ProfileReadersListingModel(iProfile,*aController) ;
    connect(iListingModel,
            SIGNAL(  error(MController::CAErrorSituation,
                           const QString&) ),
            iController,
            SLOT(handleError(MController::CAErrorSituation,
                             const QString&)),
            Qt::QueuedConnection) ;
    ui.currentListOfReadersView->setModel(iListingModel) ;
#if QT_VERSION >= 0x050000
    // qt5
    ui.currentListOfReadersView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
#else
    ui.currentListOfReadersView->horizontalHeader()->setResizeMode(QHeaderView::Stretch);
#endif
    ui.searchEdit->setFocus() ;
    iViewProfileAction = new QAction(tr("View profile"), this) ;
    ui.currentListOfReadersView->setContextMenuPolicy(Qt::ActionsContextMenu);
    ui.currentListOfReadersView->addAction(iViewProfileAction) ;

    connect(iViewProfileAction, SIGNAL(triggered()),
            this, SLOT(viewProfileSelected())) ;
}

ProfileReadersDialog::~ProfileReadersDialog() {
    LOG_STR("ProfileReadersDialog::~ProfileReadersDialog") ;
    killTimer(iTimerId) ;
    ui.listView->setModel(NULL) ;
    ui.currentListOfReadersView->setModel(NULL) ;
    delete iListingModel ;
    delete iSearchModel ;
    delete iViewProfileAction ;
}


void ProfileReadersDialog::closeButtonClicked() {
    LOG_STR("ProfileReadersDialog::closeButtonClicked") ;
    close() ;
    this->deleteLater() ;
}

void ProfileReadersDialog::viewProfileSelected() {
    LOG_STR("ProfileReadersDialog::viewProfileSelected") ;

    iController->model().lock() ;
    Hash fingerPrint (KNullHash);
    foreach(const QModelIndex &index,
            ui.currentListOfReadersView->selectionModel()->selectedIndexes()) {
        fingerPrint.fromString((const unsigned char *)(qPrintable(iListingModel->data(index,Qt::ToolTipRole).toString())));
        break ;
    }
    iController->model().unlock() ;
    if (  fingerPrint != KNullHash ) {
        iController->userInterfaceAction(MController::ViewProfileDetails,
                                         fingerPrint) ;
        close() ;
        this->deleteLater() ;
    }
}

void ProfileReadersDialog::removeButtonClicked() {
    LOG_STR("ProfileReadersDialog::removeButtonClicked") ;

    iController->model().lock() ;
    foreach(const QModelIndex &index,
            ui.currentListOfReadersView->selectionModel()->selectedIndexes()) {
        Hash fingerPrint ;
        fingerPrint.fromString((const unsigned char *)(qPrintable(iListingModel->data(index,Qt::ToolTipRole).toString())));
        if ( fingerPrint != iProfile.iFingerPrint ) {
            // can't remove self from
            // list of readers
            iListingModel->removeReader(fingerPrint);
        }
    }
    iController->model().unlock() ;
}

void ProfileReadersDialog::addButtonClicked() {
    LOG_STR("addButtonClicked") ;
    iController->model().lock() ;
    foreach(const QModelIndex &index,
            ui.listView->selectionModel()->selectedIndexes()) {
        Hash fingerPrint ;
        fingerPrint.fromString((const unsigned char *)(qPrintable(iSearchModel->data(index,Qt::ToolTipRole).toString())));
        iListingModel->addReader(fingerPrint);
    }
    iController->model().unlock() ;
}

void ProfileReadersDialog::timerEvent(QTimerEvent * /*event*/) {
    if (   iPreviousSearchFieldContent != ui.searchEdit->text()  ) {
        LOG_STR("Change in string..") ;
        iController->model().lock() ;
        iSearchModel->setSearchString(ui.searchEdit->text() ) ;
        iController->model().unlock() ;
    }
    iPreviousSearchFieldContent = ui.searchEdit->text() ;
}
