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
#include <QMessageBox>
#include "profilecommentdisplay.h"
#include "../mcontroller.h"
#include "../log.h"
#include "../datamodel/model.h"
#include "../datamodel/profilecomment.h"
#include "../datamodel/profilecommentmodel.h"
#include "../datamodel/profilecommentlistingmodel.h"
#include "../datamodel/profile.h"
#include "profilecommentitemdelegate.h"
#include "newprofilecommentdialog.h"
#include "attachmentlistdialog.h"

ProfileCommentDisplay::ProfileCommentDisplay(QWidget *aParent,
					     MController* aController,
					     ProfileCommentListingModel* aListingModel,
					     ProfileCommentModel& aCommentModel,
					     const Hash& aFirstCommentToDisplay,
					     const Hash& aViewedProfile ,
					     const Profile& aSelectedProfile )
  : DialogBase(aParent,
	       aController,
	       aSelectedProfile),
    iListingModel(aListingModel),
    iCommentModel(aCommentModel),
    iCommentToDisplay ( aFirstCommentToDisplay ),
    iItemDelegate(NULL),
    iViewedProfile ( aViewedProfile ),
    iExportSharedFileAction(NULL)
{
  ui.setupUi(this) ; 
  // button "close" emits "rejected" signal?
  connect(ui.bottomButtonsBox, SIGNAL(rejected()), this, SLOT(closeButtonClicked()));
  QPushButton* newCommentButton = new QPushButton(tr("&Add comment..."));
  connect(newCommentButton,
	  SIGNAL(clicked()),
	  this,
	  SLOT(newCommentButtonClicked())) ;
  // here buttonbox takes ownership of the newCommentButton so it
  // may not be deleted from this class any more
  ui.bottomButtonsBox->addButton(newCommentButton, 
				 QDialogButtonBox::ActionRole);
  ui.commentListView->setModel(aListingModel) ;
  ui.commentListView->setModelColumn(0) ; 
  iExportSharedFileAction = new QAction(tr("Save attachment to disk.."),this) ; 
  ui.commentListView->setContextMenuPolicy(Qt::ActionsContextMenu);
  ui.commentListView->addAction(iExportSharedFileAction) ;
  connect(iExportSharedFileAction, SIGNAL(triggered()),
	  this, SLOT(exportSharedFile())) ;
  ProfileCommentItemDelegate* iItemDelegate = new ProfileCommentItemDelegate(*iListingModel,
									     *(ui.commentListView));
  ui.commentListView->setItemDelegate(iItemDelegate) ; 
  connect(ui.commentListView->selectionModel(),
	  SIGNAL(currentChanged ( const QModelIndex & , const QModelIndex & ) ),
	  this, 
	  SLOT(currentItemChanged(const QModelIndex & , const QModelIndex & ))) ; 
  iExportSharedFileAction->setEnabled(false) ; 
  // try to set focus on selected comment, if any
  if (aFirstCommentToDisplay != KNullHash ) {
    QModelIndexList matchingArticles = 
      aListingModel->match (aListingModel->index(0,0) ,
			    Qt::UserRole, 
			    aFirstCommentToDisplay.toQVariant(),
			    1, 
			    Qt::MatchExactly  ) ;
    if ( matchingArticles.size() > 0 ) {
      ui.commentListView->selectionModel()->select(matchingArticles[0],QItemSelectionModel::Select) ; 
      ui.commentListView->scrollTo(matchingArticles[0]) ; 
    } else {
      QMessageBox::about(this,tr("Error"),
			 tr("Article not found from local storage"));
    }
  }
}


ProfileCommentDisplay::~ProfileCommentDisplay()
{
  LOG_STR("ProfileCommentDisplay::~ProfileCommentDisplay") ;
  ui.commentListView->setItemDelegate(NULL) ; 
  delete iItemDelegate ; 
  delete iExportSharedFileAction ; 
}


void ProfileCommentDisplay::exportSharedFile()
{
  LOG_STR("ProfileCommentDisplay::exportSharedFile " + iFingerPrintOfCommentOnFocus.toString() ) ;
  ProfileComment* p ( NULL ); 
  if ( iFingerPrintOfCommentOnFocus != KNullHash &&
       ( p = iCommentModel.profileCommentByFingerPrint(iFingerPrintOfCommentOnFocus) ) != NULL ) {
    QLOG_STR("Nr of attachments = " + QString::number(p->iAttachedFiles.count())) ; 
    iController->model().lock() ; 
    AttachmentListDialog *listing_dialog = 
      new AttachmentListDialog(this, 
			       iController,
			       iSelectedProfile,
			       p->iAttachedFiles,
			       AttachmentListDialog::tryFindNodeByProfile(p->iCommentorHash, *iController)) ;
    iController->model().unlock() ; 
    connect(listing_dialog,
	    SIGNAL(  error(MController::CAErrorSituation,
			   const QString&) ),
	    iController,
	    SLOT(handleError(MController::CAErrorSituation,
			     const QString&)),
	    Qt::QueuedConnection ) ;
    connect (this,
	     SIGNAL(finished(int)),
	     listing_dialog,
	     SLOT(reject()),
	     Qt::QueuedConnection) ; 
    listing_dialog->show() ; // the dialog will delete self    
    delete p ; 
  }
}

void ProfileCommentDisplay::closeButtonClicked()
{
  LOG_STR("ProfileCommentDisplay::closeButtonClicked") ;
  close() ; 
  this->deleteLater() ;
}

void ProfileCommentDisplay::currentItemChanged(const QModelIndex & aCurrent, const QModelIndex & /*aPrevious*/ )
{
  LOG_STR("ProfileCommentDisplay::currentItemChanged") ;
  unsigned numberOfAttachments = 0 ; 

  numberOfAttachments = iListingModel->data(aCurrent,Qt::UserRole+3).toUInt();
  iFingerPrintOfCommentOnFocus.fromString((const unsigned char *)(qPrintable(iListingModel->data(aCurrent,Qt::ToolTipRole).toString()))) ; 

  if ( numberOfAttachments == 0 ) {
    iExportSharedFileAction->setEnabled(false) ; 
  } else {
    iExportSharedFileAction->setEnabled(true) ; 
  }
}

void ProfileCommentDisplay::newCommentButtonClicked() {
  LOG_STR("ProfileCommentDisplay::newCommentButtonClicked ") ; 

    NewProfileCommentDialog *posting_dialog = 
      new NewProfileCommentDialog(this, iController,
				  iViewedProfile.toString(),
				  "",
				  iSelectedProfile,
				  *iListingModel ) ;
    connect(posting_dialog,
	    SIGNAL(  error(MController::CAErrorSituation,
			   const QString&) ),
	    iController,
	    SLOT(handleError(MController::CAErrorSituation,
			     const QString&)),
	    Qt::QueuedConnection ) ;
    posting_dialog->show() ; // the dialog will delete self.
    // if this dialog is closed, cascade to possible posting dialog too
    connect ( this,
	      SIGNAL(rejected()),
	      posting_dialog,
	      SLOT(cancelButtonClicked()),
	      Qt::QueuedConnection  ) ;
}
