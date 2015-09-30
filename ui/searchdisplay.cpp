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
#include "searchdisplay.h"
#include "../log.h"
#include "../datamodel/model.h"
#include "../datamodel/searchmodel.h"
#include "../datamodel/profile.h"
#include "../mcontroller.h"
#include <QShortcut>

SearchDisplay::SearchDisplay(QWidget *aParent,
                             MController* aController,
                             SearchModel* aSearchModel,
                             Profile& aSelectedProfile )
    : DialogBase(aParent,
                 aController,
                 aSelectedProfile),
    iSearchModel(aSearchModel),
    iOpenAction(NULL),
    iSearchDisplayKeyboardGrabber(NULL) {
    ui.setupUi(this) ;
    ui.resultsView->setModel(aSearchModel) ;
    ui.resultsView->setModelColumn(0) ;
    connect (this, SIGNAL(rejected()), this, SLOT(deleteLater())) ;
    connect (ui.searchButton,
             SIGNAL(clicked()),
             this,
             SLOT(searchButtonClicked())) ;
    connect (ui.bottomButtonsBox,
             SIGNAL(accepted()),
             this,
             SLOT(openButtonClicked())) ;
    ui.wordsEdit->setFocus() ;
    connect(ui.resultsView->selectionModel(),
            SIGNAL(currentChanged ( const QModelIndex & , const QModelIndex & ) ),
            this,
            SLOT(currentItemChanged(const QModelIndex & , const QModelIndex & ))) ;
    connect(ui.resultsView,
            SIGNAL(doubleClicked(const QModelIndex&)),
            this,
            SLOT(resultListDoubleClicked(const QModelIndex&))) ;

    iOpenAction = new QAction(tr("Open.."),this) ;
    ui.resultsView->setContextMenuPolicy(Qt::ActionsContextMenu);
    ui.resultsView->addAction(iOpenAction) ;
    connect(iOpenAction, SIGNAL(triggered()),
            this, SLOT(openSelectedContent())) ;
    // grab enter key
    iSearchDisplayKeyboardGrabber = new QShortcut(QKeySequence(QKeySequence::InsertParagraphSeparator),this) ;
    // and make entery key perform search in this dialog
    connect(iSearchDisplayKeyboardGrabber,
            SIGNAL(activated()),
            this,
            SLOT(keyEnterClicked())) ;
    if ( iSearchModel->isFTSSupported() == false ) {
        // if we can't do search ourselves, have only network search
        // available
        ui.networkSearchCheckBox->setCheckState(Qt::Checked) ;
        ui.networkSearchCheckBox->setEnabled(false) ;
    }
}


SearchDisplay::~SearchDisplay() {
    LOG_STR("SearchDisplay::~SearchDisplay") ;
    delete iOpenAction ;
    delete iSearchDisplayKeyboardGrabber ;
}


void SearchDisplay::closeButtonClicked() {
    LOG_STR("SearchDisplay::closeButtonClicked") ;
    close() ;
    this->deleteLater() ;
}

void SearchDisplay::searchButtonClicked() {
    LOG_STR("SearchDisplay::searchButtonClicked") ;

    iController->model().lock() ;
    iSearchModel->setSearchString(ui.wordsEdit->text(),
                                  ui.searchCAsCheckBox->isChecked(),
                                  ui.searchProfilesCheckBox->isChecked(),
                                  ui.searchCommentsCheckBox->isChecked(),
                                  ui.networkSearchCheckBox->isChecked()) ;
    iController->model().unlock() ;
}

void SearchDisplay::keyEnterClicked() {
    LOG_STR("SearchDisplay::keyEnterClicked") ;
    if ( ui.wordsEdit->hasFocus() ) {
        searchButtonClicked() ;
    }
}

void SearchDisplay::openButtonClicked() {
    LOG_STR("SearchDisplay::okButtonClicked") ;
    if ( iHashOfFocusedResult != KNullHash ) {
        openSelectedContent() ;
    }

}

void SearchDisplay::currentItemChanged(const QModelIndex & aCurrent, const QModelIndex & /*aPrevious*/ ) {
    iHashOfFocusedResult.fromQVariant(iSearchModel->data(aCurrent,Qt::UserRole)) ;
    iNodeOfFocusedResult.fromQVariant(iSearchModel->data(aCurrent,Qt::UserRole+2)) ;

    iTypeOfFocusedResult = (ProtocolItemType)(iSearchModel->data(aCurrent,Qt::UserRole+1).toInt()) ;

    QLOG_STR("SearchDisplay::currentItemChanged "+ QString::number(iTypeOfFocusedResult) + " " + iHashOfFocusedResult.toString() ) ;
}

void SearchDisplay::resultListDoubleClicked(const QModelIndex& aIndex) {
    LOG_STR("resultListDoubleClicked\n") ;
    iHashOfFocusedResult.fromQVariant(iSearchModel->data(aIndex,Qt::UserRole)) ;
    iNodeOfFocusedResult.fromQVariant(iSearchModel->data(aIndex,Qt::UserRole+2)) ;
    iTypeOfFocusedResult = (ProtocolItemType)(iSearchModel->data(aIndex,Qt::UserRole+1).toInt()) ;
    openSelectedContent() ;
}

void SearchDisplay::openSelectedContent() {
    LOG_STR("openSelectedContent\n") ;
    if ( iHashOfFocusedResult != KNullHash ) {
        switch ( iTypeOfFocusedResult ) {
        case ClassifiedAd:
            iController->userInterfaceAction(MController::ViewCa,
                                             iHashOfFocusedResult,
                                             iNodeOfFocusedResult) ;
            close() ;
            break ;
        case UserProfile:
            iController->userInterfaceAction(MController::ViewProfileDetails,
                                             iHashOfFocusedResult,
                                             iNodeOfFocusedResult) ;
            close() ;
            break ;
        case UserProfileComment:
            iController->userInterfaceAction(MController::ViewProfileComment,
                                             iHashOfFocusedResult,
                                             iNodeOfFocusedResult) ;
            close() ;
            break ;
        default:
            QLOG_STR("Unhandled UI item type " + QString::number(iTypeOfFocusedResult) + " in open") ;
            break ;
        }
    }
}
