/*     -*-C++-*- -*-coding: utf-8-unix;-*-
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


#include "FrontWidget.h"
#include "log.h"
#include "datamodel/model.h"
#include "mcontroller.h"
#include "datamodel/profile.h"
#include "datamodel/profilemodel.h"
#include "ui/profilereadersdialog.h"
#include "datamodel/binaryfile.h"
#include "datamodel/binaryfilemodel.h"
#include "datamodel/binaryfilelistingmodel.h"
#include "datamodel/camodel.h"
#include "datamodel/ca.h"
#include "ui/newclassifiedaddialog.h"
#include "ui/newprivmsgdialog.h"
#include "datamodel/privmsgmodel.h"
#include "ui/editcontact.h"
#include "datamodel/contentencryptionmodel.h"
#include "ui/newprofilecommentdialog.h"
#include <assert.h>
#include "ui/profilecommentdisplay.h"
#include "ui/attachmentlistdialog.h"
#include <QFileDialog>
#include <QMessageBox>
#include <QScrollBar>
#include <QMainWindow>

static const QString internalNameOfProfileTab("profileTabNamed") ; 
static const QString internalNameOfCommentDialog("caCommentDlgNamed") ; 

FrontWidget::FrontWidget(Controller* aController,
                         QMainWindow& aParent):
  iController(aController), 
  iParent(aParent),
  iSelectedProfile(NULL),
  iViewedProfile(NULL),
  iAddSharedFileAction(NULL),
  iRemoveSharedFileAction(NULL),
  iExportSharedFileAction(NULL),
  iSelectedProfileFileListingModel(NULL),
  iViewedProfileFileListingModel(NULL),
  iReplyToCaButton(NULL),
  iReplyToCaToForumButton(NULL),
  iNewCaButton(NULL),
  iCommentCaPosterButton(NULL),
  iViewCaPosterProfileButton(NULL),
  iViewCaAttachmentsButton(NULL),
  iAddToContactsFromCaList(NULL),
  iCaListingModel(KNullHash,iController->model(),iController),
  iArticleTopLeft(0,40),
  iPrivMsgSearchModel(aController->model(),*aController),
  iPrivMsgTopLeft(0,40),
  iReplyToPrivMsgButton(NULL),
  iNewPrivMsgButton (NULL),
  iCommentPrivMsgPosterButton(NULL),
  iViewPrivMsgPosterProfileButton(NULL),
  iViewPrivMsgAttachmentsButton(NULL),
  iAddToContactsFromMsgList(NULL),
  iContactsModel(aController->model(),*aController),
  iEditContactAction(NULL),
  iSelectedProfileCommentListingModel(aController->model(),*aController),
  iViewedProfileCommentListingModel(aController->model(),*aController),
  iProfileListingKeyboardGrabber(NULL),
  iWindowSizeAdjusted(false)
{
  ui.setupUi(this) ;
  ui.tabWidget->setTabText(0, tr("Classified ads")) ;
  ui.tabWidget->setTabText(1, tr("My profile")) ;
  ui.tabWidget->setTabText(2, tr("Contacts")) ;
  ui.tabWidget->setTabText(3, tr("Private messages")) ;

  // connect signals of profile details tab, that is to be removed
  connect(ui.profileDetailsSendMsgButton,
          SIGNAL(clicked()),
          this,
          SLOT(profileSendMsgButtonClicked())) ;
  connect(ui.profileDetailsReadersButton,
          SIGNAL(clicked()),
          this,
          SLOT(profileShowReadersButtonClicked())) ;
  connect(ui.profileDetailsCommentButton,
          SIGNAL(clicked()),
          this,
          SLOT(profileSendCommentButtonClicked())) ;
  connect(ui.searchAdsButton,
          SIGNAL(clicked()),
          this,
          SLOT(performAdsSearchClicked())) ;

  // add profile text editors to slot that enables "publish" button
  // when user edits the profile:
  connect(ui.profileNickNameEdit,
          SIGNAL(textEdited(const QString&)),
          this,
          SLOT(profileEdited(const QString&)),
          Qt::QueuedConnection) ;
  connect(ui.greetingTextEdit,
          SIGNAL(textEdited(const QString&)),
          this,
          SLOT(profileEdited(const QString&)),
          Qt::QueuedConnection) ;
  connect(ui.firstNameEdit,
          SIGNAL(textEdited(const QString&)),
          this,
          SLOT(profileEdited(const QString&)),
          Qt::QueuedConnection) ;
  connect(ui.familyNameEdit,
          SIGNAL(textEdited(const QString&)),
          this,
          SLOT(profileEdited(const QString&)),
          Qt::QueuedConnection) ;
  connect(ui.cityCountryEdit,
          SIGNAL(textEdited(const QString&)),
          this,
          SLOT(profileEdited(const QString&)),
          Qt::QueuedConnection) ;
  connect(ui.btcAddressEdit,
          SIGNAL(textEdited(const QString&)),
          this,
          SLOT(profileEdited(const QString&)),
          Qt::QueuedConnection) ;
  connect(ui.stateOfTheWorldEdit,
          SIGNAL(textEdited(const QString&)),
          this,
          SLOT(profileEdited(const QString&)),
          Qt::QueuedConnection) ;

  // have context-menu for "shared files:"
  iAddSharedFileAction = new QAction(tr("Add shared file.."),this) ; 
  iRemoveSharedFileAction = new QAction(tr("Stop advertising selected shared file"),this) ; 
  iExportSharedFileAction = new QAction(tr("Save file to disk.."),this) ; 
  ui.sharedFilesView->setContextMenuPolicy(Qt::ActionsContextMenu);
  ui.sharedFilesView->addAction(iAddSharedFileAction) ;
  ui.sharedFilesView->addAction(iRemoveSharedFileAction) ;
  ui.sharedFilesView->addAction(iExportSharedFileAction) ;

  ui.profileDetailsSharedFilesView->setContextMenuPolicy(Qt::ActionsContextMenu);
  ui.profileDetailsSharedFilesView->addAction(iExportSharedFileAction) ;

  connect(iAddSharedFileAction, SIGNAL(triggered()),
	  this, SLOT(fileToBeSharedAdded())) ;
  connect(iRemoveSharedFileAction, SIGNAL(triggered()),
	  this, SLOT(fileToBeSharedRemoved())) ;
  connect(iExportSharedFileAction, SIGNAL(triggered()),
	  this, SLOT(exportSharedFile())) ;

  // and initially remove tab 4 "profile details"
  ui.tabWidget->removeTab(4);
  
  connect(ui.publisChangesButton,
          SIGNAL(clicked()),
          this,
          SLOT(publishProfileButtonClicked())) ;
  connect(ui.revertChangesButton,
          SIGNAL(clicked()),
          this,
          SLOT(revertProfileButtonClicked())) ;
  connect(ui.imageButton,
          SIGNAL(clicked()),
          this,
          SLOT(profileImageClicked())) ;
  connect(ui.profileReadersButton,
          SIGNAL(clicked()),
          this,
          SLOT(profileReadersClicked())) ;
  connect(ui.isPrivateCheckbox,
          SIGNAL(stateChanged(int)),
          this,
          SLOT(isPrivateSettingChanged(int))) ;
  
  setupPrivateMessagesTab() ;  
  setupClassifiedAdsTab() ;
  setupContactsTab() ;

  ui.contactsView->setModel(&iContactsModel) ;   

  connect(& iSelectedProfileCommentListingModel,
	  SIGNAL(  error(MController::CAErrorSituation,
			 const QString&) ),
	  iController,
	  SLOT(handleError(MController::CAErrorSituation,
			   const QString&)),
	  Qt::QueuedConnection ) ;
  connect(& iViewedProfileCommentListingModel,
	  SIGNAL(  error(MController::CAErrorSituation,
			 const QString&) ),
	  iController,
	  SLOT(handleError(MController::CAErrorSituation,
			   const QString&)),
	  Qt::QueuedConnection ) ;

  ui.profileCommentsView->setModel(& iSelectedProfileCommentListingModel) ; 
  ui.profileDetailsCommentsView->setModel(& iViewedProfileCommentListingModel) ; 
  connect(ui.profileCommentsView->selectionModel(), SIGNAL(selectionChanged (const QItemSelection &, const QItemSelection &)),
	  this, SLOT(ownProfileCommentselectionChangedSlot(const QItemSelection &, const QItemSelection &)));
  connect(ui.profileDetailsCommentsView->selectionModel(), SIGNAL(selectionChanged (const QItemSelection &, const QItemSelection &)),
	  this, SLOT(viewedProfileCommentselectionChangedSlot(const QItemSelection &, const QItemSelection &)));

  connect(ui.profileCommentsView, SIGNAL(doubleClicked (const QModelIndex &)),
	  this, SLOT(ownProfileCommentDoubleClicked(const QModelIndex &)));
  connect(ui.profileDetailsCommentsView, SIGNAL(doubleClicked (const QModelIndex &)),
	  this, SLOT(viewedProfileCommentDoubleClicked(const QModelIndex &)));

  // interestingly "enter" key is mapped into "InsertParagraphSeparator"
  // but that seems to be true for all desktop configurations of Qt
  iProfileListingKeyboardGrabber = new QShortcut(QKeySequence(QKeySequence::InsertParagraphSeparator),ui.profileDetailsCommentsView) ;
  assert(
	 connect(iProfileListingKeyboardGrabber,
		 SIGNAL(activated()),
		 this,
		 SLOT(viewedProfileCommentKeyPressed()))) ; 

  ui.tabWidget->setCurrentWidget(ui.myProfileTab) ; 
  assert(
  connect ( iController, SIGNAL(userProfileSelected(const Hash&)),
	    &iSelectedProfileCommentListingModel, SLOT(profileSelected(const Hash&)),
	    Qt::QueuedConnection)) ; 
}

FrontWidget::~FrontWidget() {
  disconnect(this, 0, 0, 0);
  delete iProfileListingKeyboardGrabber ; 
  iCaScene.removeItem(&iCaText);
  iCaScene.removeItem(&iFromField);
  iCaScene.removeItem(&iSubjectField ) ; 
  ui.caMessageView->setScene(NULL) ; 
  delete iAddSharedFileAction ; 
  delete iRemoveSharedFileAction ; 
  delete iExportSharedFileAction ; 
  delete iEditContactAction ; 
  if ( iSelectedProfileFileListingModel ) {
    delete  iSelectedProfileFileListingModel ; 
    ui.sharedFilesView->setModel(NULL) ; 
  }
  if ( iViewedProfileFileListingModel ) {
    delete  iViewedProfileFileListingModel  ; 
    ui.profileDetailsSharedFilesView->setModel(NULL) ; 
  }
  if ( ui.privateMessageListView ) {
    ui.privateMessageListView->setModel(NULL) ; 
  }
  if ( iSelectedProfile ) {
    delete iSelectedProfile;
  }
  if ( iViewedProfile) {
    delete iViewedProfile; 
  }
  if ( ui.profileCommentsView ) {
    ui.profileCommentsView->setModel(NULL) ; 
  }
  if ( ui.profileDetailsCommentsView ) {
    ui.profileDetailsCommentsView->setModel(NULL) ; 
  }
  delete iReplyToCaButton ;
  delete iReplyToCaToForumButton ;
  delete iNewCaButton ; 
  delete iCommentCaPosterButton ; 
  delete iViewCaPosterProfileButton;
  delete iViewCaAttachmentsButton ; 
  delete iReplyToPrivMsgButton ;
  delete iNewPrivMsgButton ;
  delete iCommentPrivMsgPosterButton;
  delete iViewPrivMsgPosterProfileButton;
  delete iViewPrivMsgAttachmentsButton ;
  delete iAddToContactsFromMsgList ;
  delete iAddToContactsFromCaList ; 
  LOG_STR("FrontWidget::~FrontWidget()") ;
}

void FrontWidget::publishProfileButtonClicked() {
  LOG_STR("publishProfileButtonClicked() ") ;
  if ( iSelectedProfile ) {
    iSelectedProfile->iNickName = ui.profileNickNameEdit->text() ;
    iSelectedProfile->iGreetingText = ui.greetingTextEdit->text() ;  
    iSelectedProfile->iFirstName  = ui.firstNameEdit->text() ; 
    iSelectedProfile->iFamilyName = ui.familyNameEdit->text(); 
    iSelectedProfile->iCityCountry  = ui.cityCountryEdit->text() ; 
    iSelectedProfile->iBTCAddress = ui.btcAddressEdit->text() ; 
    iSelectedProfile->iStateOfTheWorld = ui.stateOfTheWorldEdit->text() ; 
    iSelectedProfile->iTimeOfPublish = QDateTime::currentDateTimeUtc().toTime_t() ;
    iController->model().lock() ;
    iController->model().profileModel().publishProfile(*iSelectedProfile);
    iController->model().unlock() ;
    updateUiFromSelectedProfile() ;
  }
}
void FrontWidget::revertProfileButtonClicked() {
  LOG_STR("revertProfileButtonClicked() ") ;
  if ( iSelectedProfile ) {
    userProfileSelected(iController->profileInUse()) ; // re-load
  }
  ui.publisChangesButton->setEnabled(false) ; 
  ui.revertChangesButton->setEnabled(false) ; 
}
void FrontWidget::profileImageClicked() {
  LOG_STR("profileImageClicked() ") ;

  if ( iSelectedProfile ) {
    QString fileName = QFileDialog::getOpenFileName(this, tr("Open File"),
						    "",
						    tr("Files (*.*)"));
    if ( fileName.length() > 0 ) {
      QPixmap profilePic ; 
      if ( ! profilePic.load(fileName) ) {
	QMessageBox::about(this,tr("Error"),
			   tr("Can't load image"));
	return ; 
      } else {
	// see if there is need to scale:
	if ( profilePic.height() > ui.imageButton->height() ) {
	  QPixmap scaledImage ( profilePic.scaledToHeight(ui.imageButton->height()) ) ;
	  profilePic = scaledImage ; 
	  LOG_STR("Scaling image to height") ; 
	}
	if ( profilePic.width() > ui.imageButton->width() ) {
	  QPixmap scaledImage ( profilePic.scaledToWidth(ui.imageButton->width()) ) ;
	  profilePic = scaledImage ; 
	  LOG_STR("Scaling image to width") ; 
	}
	ui.imageButton->setIcon(QIcon ( profilePic )) ; 
	iSelectedProfile->iProfilePicture = profilePic ; 
	ui.publisChangesButton->setEnabled(true) ; 
	ui.revertChangesButton->setEnabled(true) ; 
      }
    }
  }
}

void FrontWidget::profileEdited(const QString& /*aNewText*/) {
  // user touched her profile ; lets give her a clue that "publish"
  // might be in order
  ui.publisChangesButton->setEnabled(true) ; 
  ui.revertChangesButton->setEnabled(true) ; 
}

void FrontWidget::profileReadersClicked() {
  LOG_STR("profileReadersClicked() ") ;
  if ( iSelectedProfile ) {
    ProfileReadersDialog *profile_dialog = 
      new ProfileReadersDialog(this, iController,*iSelectedProfile) ;
    connect(profile_dialog,
	    SIGNAL(  error(MController::CAErrorSituation,
			   const QString&) ),
	    iController,
	    SLOT(handleError(MController::CAErrorSituation,
			     const QString&)),
	    Qt::QueuedConnection ) ;
    profile_dialog->show() ;
    ui.publisChangesButton->setEnabled(true) ; 
    ui.revertChangesButton->setEnabled(true) ; 
  }
}

void FrontWidget::profileSendMsgButtonClicked() {
  LOG_STR("profileSendMsgButtonClicked") ;
  if ( iViewedProfile && iViewedProfile->iFingerPrint != KNullHash ) {
    NewPrivMessageDialog *posting_dialog = 
      new NewPrivMessageDialog(this, iController,
			       iViewedProfile->iFingerPrint.toString(),
			       "",
			       *iSelectedProfile,
			       iPrivMsgSearchModel,
			       KNullHash,
			       KNullHash,
			       ( iViewedProfile->iNodeOfProfile == NULL ?
				 KNullHash : 
				 iViewedProfile->iNodeOfProfile->nodeFingerPrint() ) ) ;
    connect(posting_dialog,
	    SIGNAL(  error(MController::CAErrorSituation,
			   const QString&) ),
	    iController,
	    SLOT(handleError(MController::CAErrorSituation,
			     const QString&)),
	    Qt::QueuedConnection ) ;
    posting_dialog->show() ; // the dialog will delete self
  }
}
void FrontWidget::profileShowReadersButtonClicked() {
  LOG_STR("profileShowReadersButtonClicked") ;
}
void FrontWidget::profileSendCommentButtonClicked() { 
  LOG_STR("profileSendCommentButtonClicked") ;
  if ( iViewedProfile && iViewedProfile->iFingerPrint != KNullHash ) {
    NewProfileCommentDialog *posting_dialog = 
      new NewProfileCommentDialog(this, iController,
				  iViewedProfile->iFingerPrint.toString(),
				  "",
				  *iSelectedProfile,
				  iViewedProfileCommentListingModel,
				  KNullHash,
				  KNullHash,
				  ( iViewedProfile->iNodeOfProfile == NULL ?
				    KNullHash : 
				    iViewedProfile->iNodeOfProfile->nodeFingerPrint() ) ) ;
    connect(posting_dialog,
	    SIGNAL(  error(MController::CAErrorSituation,
			   const QString&) ),
	    iController,
	    SLOT(handleError(MController::CAErrorSituation,
			     const QString&)),
	    Qt::QueuedConnection ) ;
    posting_dialog->show() ; // the dialog will delete self
  }
}

void FrontWidget::performAdsSearchClicked() {
  LOG_STR("performAdsSearchClicked") ;
  selectClassification(selectedClassification(*ui.caAboutComboBox,*ui.caRegardingCombobox,*ui.caWhereComboBox,*iController)) ;
  iCaText.setHtml("<html></html>") ; 
  iSubjectField.setText("") ; 
  iFromField.setText("") ; 
  iCaOnDisplay.iSenderHash = KNullHash ; 
  emit displayedCaChanged() ; 
  ui.caAboutComboBox->setEnabled(true) ; 
  ui.caHeadersView->setEnabled(true) ; 
}

void FrontWidget::caTabAboutComboChanged(int /*aNewIndex*/) {
  // parameter not needed as selectedClassification() will ask it by itself */
  LOG_STR2("caTabAboutComboChanged selection: %s", qPrintable(selectedClassification(*ui.caAboutComboBox,*ui.caRegardingCombobox,*ui.caWhereComboBox,*iController)  )) ;
  iCaText.setHtml("<html></html>") ; 
  iSubjectField.setText("") ; 
  iFromField.setText("") ; 
  iCaOnDisplay.iSenderHash = KNullHash ; 
  emit displayedCaChanged() ; 
  ui.caHeadersView->setEnabled(false) ; 
  iReplyToCaButton->setEnabled(false) ; 
  iReplyToCaToForumButton->setEnabled(false) ; 
  iCommentCaPosterButton->setEnabled(false) ; 
  iViewCaPosterProfileButton->setEnabled(false) ; 
  iViewCaAttachmentsButton->setEnabled(false) ; 
}

void FrontWidget::caTabConcernsComboChanged(int /*aNewIndex*/) {
  // parameter not needed as selectedClassification() will ask it by itself */
  LOG_STR2("caTabConcernsComboChanged selection: %s", qPrintable(selectedClassification(*ui.caAboutComboBox,*ui.caRegardingCombobox,*ui.caWhereComboBox,*iController)  )) ;
  iCaText.setHtml("<html></html>") ; 
  iSubjectField.setText("") ; 
  iFromField.setText("") ;
  iCaOnDisplay.iSenderHash = KNullHash ;  
  emit displayedCaChanged() ; 
  ui.caHeadersView->setEnabled(false) ;
  iReplyToCaButton->setEnabled(false) ; 
  iReplyToCaToForumButton->setEnabled(false) ; 
  iCommentCaPosterButton->setEnabled(false) ; 
  iViewCaPosterProfileButton->setEnabled(false) ;  
  iViewCaAttachmentsButton->setEnabled(false) ; 
}

void FrontWidget::caTabWhereComboChanged(int /*aNewIndex*/) {
  // parameter not needed as selectedClassification() will ask it by itself */
  LOG_STR2("caTabWhereComboChanged selection: %s", qPrintable(selectedClassification(*ui.caAboutComboBox,*ui.caRegardingCombobox,*ui.caWhereComboBox,*iController)  )) ;
  iCaText.setHtml("<html></html>") ; 
  iSubjectField.setText("") ; 
  iFromField.setText("") ;
  iCaOnDisplay.iSenderHash = KNullHash ;  
  emit displayedCaChanged() ; 
  ui.caHeadersView->setEnabled(false) ; 
  iReplyToCaButton->setEnabled(false) ; 
  iReplyToCaToForumButton->setEnabled(false) ; 
  iCommentCaPosterButton->setEnabled(false) ; 
  iViewCaPosterProfileButton->setEnabled(false) ; 
  iViewCaAttachmentsButton->setEnabled(false) ; 
}

void FrontWidget::replyToCaButtonClicked() {
  LOG_STR("replyToCaButtonClicked") ;
  if ( iCaOnDisplay.iSenderHash != KNullHash ) {
    /*
     * before continuing check this: we have the CA but
     * it is possible that CA poster pubkey has already
     * been expired in db as they're kept in separate
     * tables -> check that key is in place before 
     * continuing or message encryption will fail
     */
    quint32 dummy ; 
    QByteArray dummyKey ; 
    if ( iController->model().contentEncryptionModel().PublicKey(iCaOnDisplay.iSenderHash,
								 dummyKey,
								 &dummy) == false ) {
      // key not found, insert first:
      QLOG_STR("CA sender key not in db, inserting first. " + iCaOnDisplay.iSenderHash.toString()) ; 
      iController->model().contentEncryptionModel().insertOrUpdatePublicKey (iCaOnDisplay.iProfileKey,
									     iCaOnDisplay.iSenderHash,
									     &iCaOnDisplay.iSenderName ) ; 
    }
    NewPrivMessageDialog *posting_dialog = 
      new NewPrivMessageDialog(this, iController,
			       iCaOnDisplay.iSenderHash.toString(),
			       iCaOnDisplay.iSubject,
			       *iSelectedProfile,
			       iPrivMsgSearchModel,
			       KNullHash,
			       iCaOnDisplay.iFingerPrint,
			       KNullHash) ;
    connect(posting_dialog,
	    SIGNAL(  error(MController::CAErrorSituation,
			   const QString&) ),
	    iController,
	    SLOT(handleError(MController::CAErrorSituation,
			     const QString&)),
	    Qt::QueuedConnection ) ;
    posting_dialog->show() ; // the dialog will delete self
  }
}
void FrontWidget::replyToCaToForumButtonClicked() {
  LOG_STR("replyToCaToForumButtonClicked") ;
  // check which article we're displaying:

  Hash hashOnDisplay ;
  foreach(const QModelIndex &index, 
	  ui.caHeadersView->selectionModel()->selectedIndexes()) {
    hashOnDisplay.fromQVariant( iCaListingModel.theCaModel()->data(index,Qt::UserRole) ) ;    
    break ; 
  }      

  if ( iSelectedProfile && hashOnDisplay != KNullHash ) {

    NewClassifiedAdDialog *posting_dialog = 
      new NewClassifiedAdDialog(this, iController,
				iController->model().classifiedAdsModel().aboutComboBoxTexts().indexOf(ui.caAboutComboBox->currentText()),
				iController->model().classifiedAdsModel().regardingComboBoxTexts().indexOf(ui.caRegardingCombobox->currentText()),
				iController->model().classifiedAdsModel().whereComboBoxTexts().indexOf(ui.caWhereComboBox->currentText()),
				ui.caAboutComboBox->currentText(),
				ui.caRegardingCombobox->currentText(),
				ui.caWhereComboBox->currentText(),
				*iSelectedProfile,
				iCaListingModel,
				&hashOnDisplay,
				&(iCaOnDisplay.iSubject)) ;
    connect(posting_dialog,
	    SIGNAL(  error(MController::CAErrorSituation,
			   const QString&) ),
	    iController,
	    SLOT(handleError(MController::CAErrorSituation,
			     const QString&)),
	    Qt::QueuedConnection ) ;
    posting_dialog->show() ; // the dialog will delete self
  }
}

void FrontWidget::postNewClassifiedAd() {
  LOG_STR("postNewClassifiedAd") ;
  if ( iSelectedProfile ) {
    NewClassifiedAdDialog *posting_dialog = 
      new NewClassifiedAdDialog(this, iController,
				iController->model().classifiedAdsModel().aboutComboBoxTexts().indexOf(ui.caAboutComboBox->currentText()),
				iController->model().classifiedAdsModel().regardingComboBoxTexts().indexOf(ui.caRegardingCombobox->currentText()),
				iController->model().classifiedAdsModel().whereComboBoxTexts().indexOf(ui.caWhereComboBox->currentText()),
				ui.caAboutComboBox->currentText(),
				ui.caRegardingCombobox->currentText(),
				ui.caWhereComboBox->currentText(),
				*iSelectedProfile,
				iCaListingModel) ;
    connect(posting_dialog,
	    SIGNAL(  error(MController::CAErrorSituation,
			   const QString&) ),
	    iController,
	    SLOT(handleError(MController::CAErrorSituation,
			     const QString&)),
	    Qt::QueuedConnection ) ;
    posting_dialog->show() ; // the dialog will delete self
  }
  LOG_STR("postNewClassifiedAd out") ;
}

void FrontWidget::commentCaPosterProfile() {
  LOG_STR("commentCaPosterProfile"); 
  Profile* posterProfile (NULL);
  if ( iSelectedProfile && iCaOnDisplay.iSenderHash != KNullHash ) {
    if((posterProfile = iController->model().profileModel().profileByFingerPrint(iCaOnDisplay.iSenderHash) ) == NULL ) {
      QMessageBox::about(this,tr("Profile not in database"),
			 tr("Try viewing profile first to obtain data"));
    } else {
      NewProfileCommentDialog *posting_dialog = 
	new NewProfileCommentDialog(this, iController,
				    iCaOnDisplay.iSenderHash.toString(),
				    iCaOnDisplay.iSubject,
				    *iSelectedProfile,
				    iViewedProfileCommentListingModel,
				    KNullHash,
				    KNullHash,
				    ( posterProfile->iNodeOfProfile == NULL ?
				      KNullHash : 
				      posterProfile->iNodeOfProfile->nodeFingerPrint() ) ) ;
      connect(posting_dialog,
	      SIGNAL(  error(MController::CAErrorSituation,
			     const QString&) ),
	      iController,
	      SLOT(handleError(MController::CAErrorSituation,
			       const QString&)),
	      Qt::QueuedConnection ) ;
      posting_dialog->show() ; // the dialog will delete self
      delete posterProfile ;
    }
  }
}

void FrontWidget::viewCaPosterProfile() {
  LOG_STR("viewCaPosterProfile"); 
  if ( iCaOnDisplay.iSenderHash != KNullHash ) {
    iController->userInterfaceAction(MController::ViewProfileDetails, 
				     iCaOnDisplay.iSenderHash ) ; 
  }
}

void FrontWidget::viewCaAttachments() {
  LOG_STR("viewCaAttachments"); 
  if ( iCaOnDisplay.iFingerPrint != KNullHash &&
       iCaOnDisplay.iSenderHash != KNullHash &&
       iCaOnDisplay.iAttachedFiles.count() > 0 ) {
    AttachmentListDialog *listing_dialog = 
      new AttachmentListDialog(this, 
			       iController,
			       *iSelectedProfile,
			       iCaOnDisplay.iAttachedFiles,
			       AttachmentListDialog::tryFindNodeByProfile(iCaOnDisplay.iSenderHash, *iController)) ;
    connect(listing_dialog,
	    SIGNAL(  error(MController::CAErrorSituation,
			   const QString&) ),
	    iController,
	    SLOT(handleError(MController::CAErrorSituation,
			     const QString&)),
	    Qt::QueuedConnection ) ;
    connect (this,
	     SIGNAL(displayedCaChanged()),
	     listing_dialog,
	     SLOT(reject())) ; 
    listing_dialog->show() ; // the dialog will delete self    
  }
}

void FrontWidget::viewPrivMsgAttachments() {
  LOG_STR("viewPrivMsgAttachments"); 
  if ( iPrivMsgOnDisplay.iFingerPrint != KNullHash &&
       iPrivMsgOnDisplay.iSenderHash != KNullHash &&
       iPrivMsgOnDisplay.iAttachedFiles.count() > 0 ) {
    Hash nodeOfMessageSender ;
    if ( iPrivMsgOnDisplay.iNodeOfSender ) {
      nodeOfMessageSender = iPrivMsgOnDisplay.iNodeOfSender->nodeFingerPrint();
    }
    AttachmentListDialog *listing_dialog = 
      new AttachmentListDialog(this, 
			       iController,
			       *iSelectedProfile,
			       iPrivMsgOnDisplay.iAttachedFiles,
			       nodeOfMessageSender) ;
    connect(listing_dialog,
	    SIGNAL(  error(MController::CAErrorSituation,
			   const QString&) ),
	    iController,
	    SLOT(handleError(MController::CAErrorSituation,
			     const QString&)),
	    Qt::QueuedConnection ) ;
    connect (this,
	     SIGNAL(displayedPrivMsgChanged()),
	     listing_dialog,
	     SLOT(reject())) ; 
    listing_dialog->show() ; // the dialog will delete self    
  }
}

void FrontWidget::replyToPrivMsgButtonClicked()
{
  LOG_STR("replyToPrivMsgButtonClicked"); 
  /*
   * before continuing check this: we have the msg but
   * it is possible that msg poster pubkey has already
   * been expired in db as they're kept in separate
   * tables -> check that key is in place before 
   * continuing or message encryption will fail
   */
  quint32 dummy ; 
  QByteArray dummyKey ; 
  if ( iController->model().contentEncryptionModel().PublicKey(iPrivMsgOnDisplay.iSenderHash,
							       dummyKey,
							       &dummy) == false ) {
    // key not found, insert first:
    QLOG_STR("msg sender key not in db, inserting first. " + iPrivMsgOnDisplay.iSenderHash.toString()) ; 
    iController->model().contentEncryptionModel().insertOrUpdatePublicKey (iPrivMsgOnDisplay.iProfileKey,
									   iPrivMsgOnDisplay.iSenderHash,
									   &iPrivMsgOnDisplay.iSenderName ) ; 
  }
  if ( iPrivMsgOnDisplay.iSenderHash != KNullHash ) {
    NewPrivMessageDialog *posting_dialog = 
      new NewPrivMessageDialog(this, iController,
			       iSelectedProfile->iFingerPrint == iPrivMsgOnDisplay.iSenderHash ? 
			       iPrivMsgOnDisplay.iRecipient.toString() : 
			       iPrivMsgOnDisplay.iSenderHash.toString(),
			       iPrivMsgOnDisplay.iSubject,
			       *iSelectedProfile,
			       iPrivMsgSearchModel, 
			       iPrivMsgOnDisplay.iFingerPrint,
			       KNullHash,
			       KNullHash) ;
    connect(posting_dialog,
	    SIGNAL(  error(MController::CAErrorSituation,
			   const QString&) ),
	    iController,
	    SLOT(handleError(MController::CAErrorSituation,
			     const QString&)),
	    Qt::QueuedConnection ) ;
    posting_dialog->show() ; // the dialog will delete self
  }
}

void FrontWidget::commentPrivMsgPosterProfile()
{
  LOG_STR("commentPrivMsgPosterProfile"); 
  if ( iPrivMsgOnDisplay.iSenderHash != KNullHash ) {
    Profile* posterProfile (NULL);
    if ( iSelectedProfile && iPrivMsgOnDisplay.iSenderHash != KNullHash ) {
      if((posterProfile = iController->model().profileModel().profileByFingerPrint(iPrivMsgOnDisplay.iSenderHash) ) == NULL ) {
	QMessageBox::about(this,tr("Profile not in database"),
			   tr("Try viewing profile first to obtain data"));
      } else {
	NewProfileCommentDialog *posting_dialog = 
	  new NewProfileCommentDialog(this, iController,
				      iPrivMsgOnDisplay.iSenderHash.toString(),
				      iPrivMsgOnDisplay.iSubject,
				      *iSelectedProfile,
				      iViewedProfileCommentListingModel,
				      KNullHash,
				      KNullHash,
				      ( posterProfile->iNodeOfProfile == NULL ?
					KNullHash : 
					posterProfile->iNodeOfProfile->nodeFingerPrint() ) ) ;
	connect(posting_dialog,
		SIGNAL(  error(MController::CAErrorSituation,
			       const QString&) ),
		iController,
		SLOT(handleError(MController::CAErrorSituation,
				 const QString&)),
		Qt::QueuedConnection ) ;
	posting_dialog->show() ; // the dialog will delete self
	delete posterProfile ;
      }
    }
  }
}

void FrontWidget::viewPrivMsgPosterProfile() 
{
  LOG_STR("viewPrivMsgPosterProfile"); 
  if ( iPrivMsgOnDisplay.iSenderHash != KNullHash ) {
    iController->userInterfaceAction(MController::ViewProfileDetails, 
				     iPrivMsgOnDisplay.iSenderHash ) ; 
  }
}
void FrontWidget::postNewPrivateMessage() 
{
  NewPrivMessageDialog *posting_dialog = 
    new NewPrivMessageDialog(this, iController,
			     "",
			     "",
			     *iSelectedProfile,
			     iPrivMsgSearchModel) ;
  connect(posting_dialog,
	  SIGNAL(  error(MController::CAErrorSituation,
			 const QString&) ),
	  iController,
	  SLOT(handleError(MController::CAErrorSituation,
			   const QString&)),
	  Qt::QueuedConnection ) ;
  posting_dialog->show() ; // the dialog will delete self

}

void FrontWidget::isPrivateSettingChanged(int aState) {
  LOG_STR2("isPrivateSettingChanged() %d ", aState) ;
  if ( iSelectedProfile ) {
    if ( aState == Qt::Unchecked ) {
      // public profile
      iSelectedProfile->iIsPrivate = false ; 
    } else {
      iSelectedProfile->iIsPrivate = true ; 
      // add self to readers
      if ( ! ( iSelectedProfile->iProfileReaders.contains(iSelectedProfile->iFingerPrint ) ) ) {
	iSelectedProfile->iProfileReaders.append ( iSelectedProfile->iFingerPrint ) ;
      }
    }
    if ( iSelectedProfile->iIsPrivate ) {
      ui.profileReadersButton->setEnabled(true) ;
    } else {
      ui.profileReadersButton->setEnabled(false) ;
    }
    ui.publisChangesButton->setEnabled(true) ; 
    ui.revertChangesButton->setEnabled(true) ; 
  }
}

void FrontWidget::userProfileSelected(const Hash& aProfile) {
  QLOG_STR("userProfileSelected " + aProfile.toString()) ;
  iController->model().lock() ;
  if ( iSelectedProfile != NULL ) {
    delete iSelectedProfile ;
    iSelectedProfile = NULL ;
    setUpSelectedProfileFileListingModel() ; 
  }
  if ( ( iSelectedProfile = iController->model().profileModel().profileByFingerPrint(aProfile) ) == NULL  ) {
    iSelectedProfile = new Profile(aProfile) ;
  }
  if ( iSelectedProfile ) {
    iController->model().profileModel().setTimeLastReference(iSelectedProfile->iFingerPrint, QDateTime::currentDateTimeUtc().toTime_t()) ; 
  }
  if ( iSelectedProfile ) {
    iPrivMsgSearchModel.setSearchHash(iSelectedProfile->iFingerPrint) ; 
  }
  iController->model().unlock() ;
  ui.privateMessageListView->resizeColumnToContents(2) ; 
  updateUiFromSelectedProfile() ;
  iContactsModel.clearContents() ; 
  iController->reStorePrivateDataOfSelectedProfile() ; 
  ui.contactsView->resizeColumnToContents(0) ; 
  ui.contactsView->setColumnWidth(2,30) ; 
  LOG_STR("userProfileSelected out") ;
}

void FrontWidget::fileToBeSharedAdded() {
  LOG_STR("fileToBeSharedAdded") ;
  if ( iSelectedProfile ) {
    bool isCompressed ( false ) ; 
    QString fileName = QFileDialog::getOpenFileName(this, tr("Select file to be published"),
						    "",
						    tr("Files (*.*)"));
    if ( fileName.length() > 0 ) {
      QFile f ( fileName ) ;
      if ( f.open(QIODevice::ReadOnly) ) {
	if ( f.size() > ( KMaxProtocolItemSize * 10 ) ) {
	  emit error ( MController::FileOperationError, tr("File way too big")) ; 
	} else {
	  QByteArray content ( f.read(f.size() ) ) ; 
	  QByteArray compressedContent ( qCompress(content) ) ; 
	  LOG_STR2("Size of content = %d", (int) (content.size())) ; 
	  LOG_STR2("Size of compressed = %d", (int) (compressedContent.size())) ; 
	  if (content.size() > (qint64)(( KMaxProtocolItemSize - ( 50*1024 ) )) &&
	      compressedContent.size() > (qint64)(( KMaxProtocolItemSize - ( 50*1024 ) )) ) {
	    emit error ( MController::FileOperationError, tr("File too big")) ; 
	  } else {
	    if ( compressedContent.size() < content.size () ) {
	      content.clear() ; 
	      content.append(compressedContent) ; 
	      compressedContent.clear() ; 
	      isCompressed = true ; 
	    } else {
	      compressedContent.clear() ; 
	    }
	    // ok, here we have content in content and it is 
	    // either compressed or not. size is checked. 
	    QString description ; // TODO: query user 
	    QString mimetype ; 
	    iController->model().lock() ; 
	    Hash fileFingerPrint ( 
				  iController->model().binaryFileModel().publishBinaryFile(*iSelectedProfile,
											   QFileInfo(f).fileName(),
											   description,
											   mimetype,
											   content,
											   isCompressed) ) ;
	    if ( fileFingerPrint != KNullHash ) {
	      if ( ! iSelectedProfile->iSharedFiles.contains(fileFingerPrint ) ) {
		// addFile will also modify iSelectedProfile->iSharedFiles
		iSelectedProfileFileListingModel->addFile(fileFingerPrint) ;
		iSelectedProfile->iTimeOfPublish = QDateTime::currentDateTimeUtc().toTime_t() ;
		iController->model().profileModel().publishProfile(*iSelectedProfile) ; 
	      }
	    }
	    iController->model().unlock() ; 
	  }
	}
      } else {
	emit error ( MController::FileOperationError, tr("File open error")) ; 
      }
    }
  }
}

void FrontWidget::fileToBeSharedRemoved() {
  LOG_STR("fileToBeSharedRemoved") ;

  // ok, see what user had selected ; one file only:
  iController->model().lock() ; 
  if ( iSelectedProfileFileListingModel ) {
    Hash fingerPrint ;
    foreach(const QModelIndex &index, 
	    ui.sharedFilesView->selectionModel()->selectedIndexes()) {
      fingerPrint.fromString((const unsigned char *)(qPrintable(iSelectedProfileFileListingModel->data(index,Qt::ToolTipRole).toString())));
      break ; 
    }
    if ( fingerPrint != KNullHash ) {
      if ( iSelectedProfile->iSharedFiles.contains(fingerPrint) ) {
	// addFile will also modify iSelectedProfile->iSharedFiles
	iSelectedProfileFileListingModel->removeFile(fingerPrint) ;
	iSelectedProfile->iTimeOfPublish = QDateTime::currentDateTimeUtc().toTime_t() ;
	iController->model().profileModel().publishProfile(*iSelectedProfile) ; 
      }
    }
  }
  iController->model().unlock() ; 
}

void FrontWidget::exportSharedFile() {
  LOG_STR("exportSharedFile") ;
  // ok, see what user had selected ; one file only:
  bool netRequestStarted(false) ; 
  QByteArray fileData ; 
  BinaryFile* metadata (NULL); 
  iController->model().lock() ; 
  if ( iSelectedProfileFileListingModel ) {
    Hash fingerPrint ;

    if ( ui.tabWidget->currentIndex() == 1 ) { // 1 == "my profile" so this is export of my own file
      foreach(const QModelIndex &index, 
	      ui.sharedFilesView->selectionModel()->selectedIndexes()) {
	fingerPrint.fromString((const unsigned char *)(qPrintable(iSelectedProfileFileListingModel->data(index,Qt::ToolTipRole).toString())));
	break ; 
      }
    } else {
      if ( iViewedProfileFileListingModel ) {
	LOG_STR("Trying to find shared file from other profiles listing..") ; 
	foreach(const QModelIndex &index, 
		ui.profileDetailsSharedFilesView->selectionModel()->selectedIndexes()) {
	  fingerPrint.fromString((const unsigned char *)(qPrintable(iViewedProfileFileListingModel->data(index,Qt::ToolTipRole).toString())));
	  break ; 
	}      
      }
    }
    if ( fingerPrint != KNullHash ) {
      // aye, found a selected fingerprint; first check if we have 
      //  the file or just know the fingerprint. both are possible. 
      if ( ( metadata = iController->model().binaryFileModel().binaryFileByFingerPrint(fingerPrint) ) == NULL ) {
	// got no file, ask it to be retrieved:
	NetworkRequestExecutor::NetworkRequestQueueItem req ;
	req.iRequestType = RequestForBinaryBlob ;
	req.iRequestedItem = fingerPrint ;
	req.iState = NetworkRequestExecutor::NewRequest ; 
	req.iMaxNumberOfItems = 1 ; 
	// if the file was shared by some other operator, 
	// ask node of that operator first
	if ( ui.tabWidget->currentIndex() != 1 ) { // was not our own profile
	  if ( iViewedProfile && iViewedProfile->iNodeOfProfile ) {
	    req.iDestinationNode = iViewedProfile->iNodeOfProfile->nodeFingerPrint();
	  }
	}
	iController->startRetrievingContent(req,true) ; 
	netRequestStarted = true ; 
      } else {
	QByteArray fileSignature ; 
	Hash fileOwnerFingerPrint ; 
	fileOwnerFingerPrint.fromString((const unsigned char *)(metadata->iOwner.toLatin1().constData())) ;
	bool dummy ; 
	iController->model().binaryFileModel().setTimeLastReference(fingerPrint, QDateTime::currentDateTimeUtc().toTime_t()) ; 
	if ( !iController->model().binaryFileModel().binaryFileDataByFingerPrint(fingerPrint,
										 fileOwnerFingerPrint,
										 fileData,
										 fileSignature,
										 &dummy) ) {
	  LOG_STR("Got no file?") ;
	}
      }
    }

  }
  iController->model().unlock() ; 
  if ( fileData.size() > 0 && metadata ) {
    QString fileName = QFileDialog::getSaveFileName(this, tr("Choose file name for saving"),
						    metadata->iFileName,
						    tr("Files (*.*)"));
    if ( fileName.length() > 0 ) {
      QFile f ( fileName ) ; 
      if ( f.open(QIODevice::WriteOnly) ) {
	f.write(fileData) ;
	f.close() ;
      } else {
	QMessageBox::about(this,tr("Error"),
			   tr("File open error"));
      }
    }
  }
  delete metadata ; 
  if ( netRequestStarted ) {
    QMessageBox::about(this,tr("Please wait"),
		       tr("File is being fetched from network"));
  }
}

void FrontWidget::updateUiFromSelectedProfile() {
  if ( iSelectedProfile ) {
    ui.profileAddressValue->setText(iSelectedProfile->iFingerPrint.toString()) ;
    if ( iWindowSizeAdjusted == false ) {
      ui.profileAddressValue->setMinimumWidth(ui.profileAddressValue->fontMetrics().width(iSelectedProfile->iFingerPrint.toString()));
      iParent.setMinimumWidth ( 
			       ui.profileAddressValue->fontMetrics().width(iSelectedProfile->iFingerPrint.toString()) +
			       ui.imageButton->width() + 
			       ui.profileAddressValue->width() -55 ) ;
      iWindowSizeAdjusted = true ; 
    }
    ui.profileNickNameEdit->setText(iSelectedProfile->iNickName) ;
    ui.greetingTextEdit->setText(iSelectedProfile->iGreetingText  ) ;  
    ui.firstNameEdit->setText(iSelectedProfile->iFirstName   ) ; 
    ui.familyNameEdit->setText(iSelectedProfile->iFamilyName  ); 
    ui.cityCountryEdit->setText(iSelectedProfile->iCityCountry   ) ; 
    ui.btcAddressEdit->setText(iSelectedProfile->iBTCAddress  ) ; 
    ui.stateOfTheWorldEdit->setText(iSelectedProfile->iStateOfTheWorld  ) ; 

    if ( iSelectedProfile->iTimeOfPublish ) {
      QDateTime d ;
      d.setTime_t(iSelectedProfile->iTimeOfPublish) ;
      ui.timeOfLastUpdateValue->setText(d.toString(Qt::SystemLocaleShortDate)) ;
    } else {
      ui.timeOfLastUpdateValue->setText("") ;
    }
    if ( iSelectedProfile->iIsPrivate ) {
      ui.profileReadersButton->setEnabled(true) ;
      ui.isPrivateCheckbox->setChecked(true) ; 
    } else {
      ui.profileReadersButton->setEnabled(false) ;
      ui.isPrivateCheckbox->setChecked(false) ; 
    }
    if ( !( iSelectedProfile->iProfilePicture.isNull()) ) {
      ui.imageButton->setIcon(QIcon(iSelectedProfile->iProfilePicture)) ; 
    } else {
      QPixmap p ( ":/ui/ui/Lenin_reading_Pravda.png" ) ; 
      ui.imageButton->setIcon(QIcon(p)) ;
    }
    setUpSelectedProfileFileListingModel() ; 
    ui.publisChangesButton->setEnabled(false) ; 
    ui.revertChangesButton->setEnabled(false) ; 
  } 
}
void FrontWidget::showDetailsOfProfile(const Hash& aFingerPrint) {
  for ( int i = ui.tabWidget->count()-1 ; i >= 0 ; i-- ) {
    if (     ui.tabWidget->widget(i)->objectName() == internalNameOfProfileTab ) {
      ui.tabWidget->removeTab(i);
      break ;
    }
  }

  if  ( iViewedProfile ) {
    delete iViewedProfile ;
    iViewedProfile = NULL ; 
    iViewedProfileCommentListingModel.setSearchHash(KNullHash) ; // empty model
  }
  iController->model().lock() ;
  iViewedProfile = iController->model().profileModel().profileByFingerPrint(aFingerPrint) ;
  if ( iViewedProfile ) {
    iController->model().profileModel().setTimeLastReference(iViewedProfile->iFingerPrint, QDateTime::currentDateTimeUtc().toTime_t()) ; 
    iController->offerDisplayNameForProfile(iViewedProfile->iFingerPrint,
					    iViewedProfile->displayName(),
					    true) ; 
  }
  iController->model().unlock() ;
  if ( iViewedProfile ) {
    iController-> sendProfileUpdateQuery(iViewedProfile->iFingerPrint,
					 iViewedProfile->iNodeOfProfile == NULL ? 
					 KNullHash :
					 iViewedProfile->iNodeOfProfile->nodeFingerPrint()) ;
    
    ui.tabWidget->insertTab(ui.tabWidget->count()+1,ui.profileViewTab, iViewedProfile->displayName());
    ui.tabWidget->setCurrentWidget(ui.profileViewTab) ; 
    ui.tabWidget->currentWidget()->setObjectName(internalNameOfProfileTab) ; 

    updateUiFromViewedProfile() ; 
    iController->model().lock() ;
    iController->model().profileModel().setTimeLastReference(iViewedProfile->iFingerPrint, QDateTime::currentDateTimeUtc().toTime_t()) ; 
    iViewedProfileCommentListingModel.setSearchHash(iViewedProfile->iFingerPrint) ; // empty model
    iController->model().unlock() ;
  } else {
    ui.profileDetailsCommentButton->setEnabled(false) ; 
    ui.profileDetailsSendMsgButton->setEnabled(false) ; 
  }

}

void   FrontWidget::showClassifiedAd(const CA& ca) {
  QLOG_STR("Show classified ad " + ca.iFingerPrint.toString() ) ;
  if ( ca.iFingerPrint != KNullHash && ui.tabWidget ) {
    ui.tabWidget->setCurrentIndex(0) ; // 0 is the ca tab
    if ( ca.iAboutComboBoxIndex > 0 ) {
      ui.caAboutComboBox->setCurrentIndex(ca.iAboutComboBoxIndex) ;
    } else {
      ui.caAboutComboBox->setEditText(ca.iAboutComboBoxText) ;
    }
    if ( ca.iConcernsComboBoxIndex > 0 ) {
      ui.caRegardingCombobox->setCurrentIndex(ca.iConcernsComboBoxIndex) ;
    } else {
      ui.caRegardingCombobox->setEditText(ca.iConcernsComboBoxText) ;
    }
    if ( ca.iInComboBoxIndex > 0 ) {
      ui.caWhereComboBox->setCurrentIndex(ca.iInComboBoxIndex) ;
    } else {
      ui.caWhereComboBox->setEditText(ca.iInComboBoxText) ;
    }
    Hash groupFingerPrint ; 
    groupFingerPrint.calculate(ca.iGroup.toUtf8()) ; 
    iCaListingModel.setClassification(groupFingerPrint) ;
    // then perform (perfectly linear?) search ; no hopping or jumping at all:
    QModelIndexList matchingArticles = 
      iCaListingModel.theCaModel()->match (iCaListingModel.theCaModel()->index(0,0) ,
					   Qt::UserRole, 
					   ca.iFingerPrint.toQVariant(),
					   2, 
					   Qt::MatchRecursive|Qt::MatchExactly  ) ;
    
    if ( matchingArticles.size() > 0 ) {
      ui.caHeadersView->selectionModel()->select(matchingArticles[0],QItemSelectionModel::Select) ; 
      ui.caHeadersView->scrollTo(matchingArticles[0]) ; 
    } else {
      QMessageBox::about(this,tr("Error"),
			 tr("Article not found from local storage"));
    }
  }
}

void FrontWidget::showSingleCommentOfProfile(const Hash aCommentFingerPrint) {
  if (  iSelectedProfile )  {
    activateProfileCommentDisplay(&iViewedProfileCommentListingModel,
				  iController->model().profileCommentModel(),
				  aCommentFingerPrint,
				  iSelectedProfile->iFingerPrint ) ;
  }
}
void FrontWidget::updateUiFromViewedProfile() {
  if ( iViewedProfile ) {
    ui.profileDetailsCommentButton->setEnabled(true) ; 
    ui.profileDetailsSendMsgButton->setEnabled(true) ; 

    ui.profileDetailsNickNameValue->setText(iViewedProfile->iNickName) ;

    if ( isContactInContactList(iViewedProfile->iFingerPrint) ) {
      ui.profileDetailsAddrValue->setTextColor(QColor((Qt::blue))) ; 
      LOG_STR("Profile was in contacts list, setting blue color") ; 
    } else {
      // default color then:
      ui.profileDetailsAddrValue->setTextColor(QApplication::palette().text().color()) ;
      LOG_STR("Profile was not in contacts list, setting default color") ; 
    }
    ui.profileDetailsAddrValue->setText(iViewedProfile->iFingerPrint.toString()) ;

    if ( iViewedProfile->iIsPrivate ) {
      ui.profileDetailsReadersButton->setEnabled(true) ;
    } else {
      ui.profileDetailsReadersButton->setEnabled(false) ;
    }

    ui.profileDetailsGreetingValue->setText(iViewedProfile->iGreetingText);
    ui.profileDetailsFirstNameValue->setText(iViewedProfile->iFirstName);
    ui.profileDetailsFamilyNameValue->setText(iViewedProfile->iFamilyName);
    ui.profileDetailsCityCountryValue->setText(iViewedProfile->iCityCountry);
    ui.profileDetailsBTCValue->setText(iViewedProfile->iBTCAddress);
    ui.profileDetailsStateOfTheWorldValue->setText(iViewedProfile->iStateOfTheWorld);
    QDateTime d ;
    d.setTime_t(iViewedProfile->iTimeOfPublish) ;
    ui.timeOfLastUpdateValue->setText(d.toString(Qt::SystemLocaleShortDate)) ;
    QString toolTipText ( tr("Time of last update ") + d.toString(Qt::SystemLocaleShortDate) ) ;
    ui.profileDetailsNickNameValue->setToolTip(toolTipText) ; 
    ui.profileDetailsGreetingValue->setToolTip(toolTipText) ; 
    if ( !( iViewedProfile->iProfilePicture.isNull()) ) {
      ui.profileDetailsImage->setPixmap(iViewedProfile->iProfilePicture) ; 
    } else {
      QPixmap p ( ":/ui/ui/Lenin_reading_Pravda.png" ) ; 
      ui.profileDetailsImage->setPixmap(p) ;
    }
    if ( iViewedProfileFileListingModel ) {
      delete  iViewedProfileFileListingModel  ; 
      iViewedProfileFileListingModel = NULL ; 
      ui.profileDetailsSharedFilesView->setModel(NULL) ; 
    }
    iViewedProfileFileListingModel = new BinaryFileListingModel(iViewedProfile->iSharedFiles) ; 
    connect(iViewedProfileFileListingModel,
	    SIGNAL(  error(MController::CAErrorSituation,
			   const QString&) ),
	    iController,
	    SLOT(handleError(MController::CAErrorSituation,
			     const QString&)),
	    Qt::QueuedConnection) ;
    ui.profileDetailsSharedFilesView->setModel(iViewedProfileFileListingModel) ; 
  }
}

void FrontWidget::setUpSelectedProfileFileListingModel() {
  // always start by getting rid of possible previous instance:
  if ( iSelectedProfileFileListingModel ) {
    ui.sharedFilesView->setModel(NULL) ; 
    delete iSelectedProfileFileListingModel ; 
    iSelectedProfileFileListingModel = NULL ; 
  }
  //, if we have profile selected, set up model again
  if ( iSelectedProfile ) {
    iSelectedProfileFileListingModel = new BinaryFileListingModel(iSelectedProfile->iSharedFiles) ; 
    connect(iSelectedProfileFileListingModel,
	    SIGNAL(  error(MController::CAErrorSituation,
			   const QString&) ),
	    iController,
	    SLOT(handleError(MController::CAErrorSituation,
			     const QString&)),
	    Qt::QueuedConnection) ;
    ui.sharedFilesView->setModel(iSelectedProfileFileListingModel) ; 
  } 
}

void FrontWidget::setupContactsTab() {
  ui.contactsView->setModel(&iContactsModel) ;   
  connect(ui.contactsView->selectionModel(), SIGNAL(selectionChanged (const QItemSelection &, const QItemSelection &)),
	  this, SLOT(contactsSelectionChangedSlot(const QItemSelection &, const QItemSelection &)));
  connect(&iContactsModel,
	  SIGNAL(  error(MController::CAErrorSituation,
			 const QString&) ),
	  iController,
	  SLOT(handleError(MController::CAErrorSituation,
			   const QString&)),
	  Qt::QueuedConnection) ;
  ui.removeContactBtn->setEnabled(false) ;
  ui.viewProfileBtn->setEnabled(false) ;
  ui.sendMessageBtn->setEnabled(false) ;

  connect(ui.addContactBtn,
          SIGNAL(clicked()),
          this,
	  SLOT(addContactButtonClicked() )) ;

  connect(ui.removeContactBtn,
          SIGNAL(clicked()),
          this,
	  SLOT(removeContactButtonClicked()  )) ;

  connect(ui.viewProfileBtn,
          SIGNAL(clicked()),
          this,
	  SLOT(viewContactProfileButtonClicked()  )) ;

  connect(ui.sendMessageBtn,
          SIGNAL(clicked()),
          this,
	  SLOT(sendMsgToContactButtonClicked()  )) ;

  connect(ui.addToContactsBtn, // this button is in "view contact" tab
          SIGNAL(clicked()),
          this,
	  SLOT(addContactFromContactViewButtonClicked())) ;

  iEditContactAction = new QAction(tr("Edit contact.."),this) ; 
  connect(iEditContactAction, SIGNAL(triggered()),
	  this, SLOT(editContactActionSelected())) ;
  ui.contactsView->setContextMenuPolicy(Qt::ActionsContextMenu);
  ui.contactsView->addAction(iEditContactAction) ;
}

void FrontWidget::setupClassifiedAdsTab() {
  // note that later the index of the combobox must match 
  // the enum. this has things to do with "insert policy" of 
  // the combobox. 
  
  fillCaSelectionCombobox(*ui.caAboutComboBox,true,*iController) ; 
  ui.caAboutComboBox->setEditable(true) ;
  ui.caAboutComboBox->setInsertPolicy(QComboBox::InsertAtBottom) ; 
  connect(ui.caAboutComboBox,
          SIGNAL(currentIndexChanged(int)),
          this,
          SLOT(caTabAboutComboChanged(int))) ;

  fillCaSelectionCombobox(*ui.caRegardingCombobox,false,*iController) ; 

  ui.caRegardingCombobox->setEditable(true) ;
  ui.caRegardingCombobox->setInsertPolicy(QComboBox::InsertAtBottom) ; 
  connect(ui.caRegardingCombobox,
          SIGNAL(currentIndexChanged(int)),
          this,
          SLOT(caTabConcernsComboChanged(int))) ;

  ui.caWhereComboBox->addItems(iController->model().classifiedAdsModel().whereComboBoxTexts()) ; 
  ui.caWhereComboBox->setEditable(true) ;
  ui.caWhereComboBox->setInsertPolicy(QComboBox::InsertAtBottom) ; 
  connect(ui.caWhereComboBox,
          SIGNAL(currentIndexChanged(int)),
          this,
          SLOT(caTabWhereComboChanged(int))) ;

  // for the bottom-screen buttonbox we need only custom buttons:
  ui.CATabBottomButtonBox->clear() ; 

  iReplyToCaButton = new QPushButton(tr("&Reply to sender")); 
  iReplyToCaToForumButton = new QPushButton(tr("Reply to &forum")); 
  iNewCaButton = new QPushButton(tr("&Post new ad")); 
  iCommentCaPosterButton= new QPushButton(tr("&Public comment")); 
  iViewCaPosterProfileButton= new QPushButton(tr("&View profile")); 
  iViewCaAttachmentsButton = new QPushButton(tr("Attachments.."));

  connect(iReplyToCaButton,
          SIGNAL(clicked()),
          this,
	  SLOT(replyToCaButtonClicked())) ;
  connect(iReplyToCaToForumButton,
          SIGNAL(clicked()),
          this,
	  SLOT(replyToCaToForumButtonClicked())) ;
  connect(iNewCaButton,
          SIGNAL(clicked()),
          this,
	  SLOT(postNewClassifiedAd())) ;

  connect(iCommentCaPosterButton,
          SIGNAL(clicked()),
          this,
          SLOT(commentCaPosterProfile())) ;

  connect(iViewCaPosterProfileButton,
          SIGNAL(clicked()),
          this,
          SLOT( viewCaPosterProfile() )) ;

  connect(iViewCaAttachmentsButton,
          SIGNAL(clicked()),
          this,
          SLOT( viewCaAttachments() )) ;


  ui.CATabBottomButtonBox->addButton(iReplyToCaButton, QDialogButtonBox::ActionRole);
  ui.CATabBottomButtonBox->addButton(iReplyToCaToForumButton, QDialogButtonBox::ActionRole);
  ui.CATabBottomButtonBox->addButton(iNewCaButton, QDialogButtonBox::ActionRole);
  ui.CATabBottomButtonBox->addButton(iCommentCaPosterButton, QDialogButtonBox::ActionRole);
  ui.CATabBottomButtonBox->addButton(iViewCaPosterProfileButton, QDialogButtonBox::ActionRole);
  ui.CATabBottomButtonBox->addButton(iViewCaAttachmentsButton, QDialogButtonBox::ActionRole);
  iReplyToCaButton->setEnabled(false) ; 
  iReplyToCaToForumButton->setEnabled(false) ; 
  iCommentCaPosterButton->setEnabled(false) ; 
  iViewCaPosterProfileButton->setEnabled(false) ; 
  iViewCaAttachmentsButton->setEnabled(false) ; 
  ui.caHeadersView->setModel(iCaListingModel.theCaModel()) ; 
  connect(ui.caHeadersView->selectionModel(), SIGNAL(selectionChanged (const QItemSelection &, const QItemSelection &)),
	  this, SLOT(CAselectionChangedSlot(const QItemSelection &, const QItemSelection &)));

  iCaScene.addItem(&iFromField);
  iCaScene.addItem(&iSubjectField ) ; 
  iSubjectField.setPos(0,0) ; 
  iFromField.setPos(0,20) ; 
  iCaScene.addItem(&iCaText)  ;
  iCaText.setPos(iArticleTopLeft) ; 
  ui.caMessageView->setScene(&  iCaScene ) ; 
  iCaScene.invalidate(0,0,100,100) ; 
  ui.caMessageView->horizontalScrollBar()->setSliderPosition(0) ; 
  ui.caMessageView->horizontalScrollBar()->setValue(0) ; 
  //  ui.caMessageView->centerOn(0,0) ;

  // have context-menu for "ca listing" view
  iAddToContactsFromCaList = new QAction(tr("Add selected operator to contacts.."),this) ; 
  ui.caHeadersView->setContextMenuPolicy(Qt::ActionsContextMenu);
  ui.caHeadersView->addAction(iAddToContactsFromCaList) ;
  connect(iAddToContactsFromCaList, SIGNAL(triggered()),
	  this, SLOT(addCaSenderToContacts())) ;
}

void FrontWidget::setCaDocumentSize() {
  QRect messageViewSizeRect ( ui.caMessageView->rect() ) ; 
  messageViewSizeRect.setHeight( messageViewSizeRect.height() - 10 )  ; 
  messageViewSizeRect.setWidth( messageViewSizeRect.width() - 10 )  ; 
  iCaScene.setSceneRect(messageViewSizeRect) ; 
  iCaText.setTextWidth(ui.caMessageView->rect().width()-10);
  const QSizeF documentSize ( iCaText.document()->size() ) ;
  const QSizeF subjectSize ( iSubjectField.boundingRect().size() ) ; 
  if ( documentSize.height() > 
       (messageViewSizeRect.height()-(subjectSize.height()*2))) {
    // for long articles we need to adjust scene size so that all
    // text fits.
    messageViewSizeRect.setHeight(documentSize.height()+(subjectSize.height()*2));
    iCaScene.setSceneRect(messageViewSizeRect) ; 
  }
}

void FrontWidget::setPrivMsgSize() {
  QRect messageViewSizeRect ( ui.privateMessageView->rect() ) ; 
  messageViewSizeRect.setHeight( messageViewSizeRect.height() - 10 )  ; 
  messageViewSizeRect.setWidth( messageViewSizeRect.width() - 10 )  ; 
  iPrivMsgScene.setSceneRect(messageViewSizeRect) ; 
  iPrivMsgText.setTextWidth(ui.caMessageView->rect().width()-10);
  const QSizeF documentSize ( iPrivMsgText.document()->size() ) ;
  const QSizeF subjectSize ( iSubjectField.boundingRect().size() ) ; 
  if ( documentSize.height() > 
       (messageViewSizeRect.height()-(subjectSize.height()*2))) {
    // for long articles we need to adjust scene size so that all
    // text fits.
    messageViewSizeRect.setHeight(documentSize.height()+(subjectSize.height()*2));
    iPrivMsgScene.setSceneRect(messageViewSizeRect) ; 
  }
}

void FrontWidget::setupPrivateMessagesTab() {
  ui.privateMessageListView->setModel(&iPrivMsgSearchModel) ; 
  connect(ui.privateMessageListView->selectionModel(), SIGNAL(selectionChanged (const QItemSelection &, const QItemSelection &)),
	  this, SLOT(privMsgselectionChangedSlot(const QItemSelection &, const QItemSelection &)));
  connect(&iPrivMsgSearchModel,
	  SIGNAL(  error(MController::CAErrorSituation,
			 const QString&) ),
	  iController,
	  SLOT(handleError(MController::CAErrorSituation,
			   const QString&)),
	  Qt::QueuedConnection ) ;

  iPrivMsgScene.addItem( & iPrivMsgFromField);
  iPrivMsgScene.addItem( & iPrivMsgSubjectField ) ; 
  iPrivMsgSubjectField.setPos(0,0) ; 
  iPrivMsgFromField.setPos(0,20) ; 
  iPrivMsgScene.addItem( & iPrivMsgText ) ; 
  iPrivMsgText.setPos(iPrivMsgTopLeft) ; 
  ui.privateMessageView->setScene(&  iPrivMsgScene ) ; 
  iPrivMsgScene.invalidate(0,0,100,100) ; 
  ui.privateMessageView->horizontalScrollBar()->setSliderPosition(0) ; 
  ui.privateMessageView->horizontalScrollBar()->setValue(0) ; 
  ui.privateMessageView->centerOn(0,0) ;

  ui.privateMsgsButtons->clear() ; 

  iReplyToPrivMsgButton = new QPushButton(tr("&Reply")); 
  iNewPrivMsgButton = new QPushButton(tr("&New message")); 
  iCommentPrivMsgPosterButton= new QPushButton(tr("&Send public comment to sender")); 
  iViewPrivMsgPosterProfileButton= new QPushButton(tr("&View profile of sender")); 
  iViewPrivMsgAttachmentsButton = new QPushButton(tr("Attachments..")); 

  connect(iReplyToPrivMsgButton,
	  SIGNAL(clicked()),
	  this,
	  SLOT(replyToPrivMsgButtonClicked())) ;
  connect(iNewPrivMsgButton,
	  SIGNAL(clicked()),
	  this,
	  SLOT(postNewPrivateMessage())) ;
  connect(iCommentPrivMsgPosterButton,
	  SIGNAL(clicked()),
	  this,
	  SLOT(commentPrivMsgPosterProfile())) ;
  connect(iViewPrivMsgPosterProfileButton,
	  SIGNAL(clicked()),
	  this,
	  SLOT( viewPrivMsgPosterProfile() )) ;
  connect(iViewPrivMsgAttachmentsButton,
	  SIGNAL(clicked()),
	  this,
	  SLOT( viewPrivMsgAttachments() )) ;

  iReplyToPrivMsgButton->setEnabled(false) ; 
  iCommentPrivMsgPosterButton->setEnabled(false) ; 
  iViewPrivMsgPosterProfileButton->setEnabled(false) ; 
  iViewPrivMsgAttachmentsButton->setEnabled(false) ; 

  ui.privateMsgsButtons->addButton(iReplyToPrivMsgButton,QDialogButtonBox::ActionRole);
  ui.privateMsgsButtons->addButton(iNewPrivMsgButton,QDialogButtonBox::ActionRole);
  ui.privateMsgsButtons->addButton(iCommentPrivMsgPosterButton,QDialogButtonBox::ActionRole);
  ui.privateMsgsButtons->addButton(iViewPrivMsgPosterProfileButton,QDialogButtonBox::ActionRole);
  ui.privateMsgsButtons->addButton(iViewPrivMsgAttachmentsButton,QDialogButtonBox::ActionRole);
  // have context-menu for "message listing" view
  iAddToContactsFromMsgList = new QAction(tr("Add selected to contacts.."),this) ; 
  ui.privateMessageListView->setContextMenuPolicy(Qt::ActionsContextMenu);
  ui.privateMessageListView->addAction(iAddToContactsFromMsgList) ;
  connect(iAddToContactsFromMsgList, SIGNAL(triggered()),
	  this, SLOT(addMessageSenderToContacts())) ;

}

void FrontWidget::fillCaSelectionCombobox(QComboBox& aComboBox,
					  bool isAboutComboBox,
					  MController& aController) {
  if ( isAboutComboBox ) {
    aComboBox.addItems(aController.model().classifiedAdsModel().aboutComboBoxTexts()) ;
  } else {
    aComboBox.addItems(aController.model().classifiedAdsModel().regardingComboBoxTexts()) ;
  }
}

QString FrontWidget::selectedClassification(const QComboBox& aAboutCombo,
					    const QComboBox& aRegardingCombo,
					    const QComboBox& aWhereCombo,
					    const MController& aController) {
  QString retval ; 
  // check out if pre-defined string from combobox or
  // something that user wrote himself to combobox. 
  // it seems like digging that information from combobox itself
  // is difficult to do .. user would need to hit enter 
  // to set the indexes correctly and she won't
  if ( aController.model().classifiedAdsModel().aboutComboBoxTexts().indexOf(aAboutCombo.currentText()) != -1 ) {
    retval = aController.model().classifiedAdsModel().purposeOfAdString((ClassifiedAdsModel::PurposeOfAd)aController.model().classifiedAdsModel().aboutComboBoxTexts().indexOf(aAboutCombo.currentText())) ; 
  } else {
    retval = aAboutCombo.currentText() ; 
  }
  retval.append(".") ; 
  if ( aController.model().classifiedAdsModel().regardingComboBoxTexts().indexOf( aRegardingCombo.currentText()) != -1 ) {
    retval.append( aController.model().classifiedAdsModel().concernOfAdString((ClassifiedAdsModel::ConcernOfAd)aController.model().classifiedAdsModel().regardingComboBoxTexts().indexOf( aRegardingCombo.currentText()))) ; 
  } else {
    retval.append( aRegardingCombo.currentText() ) ; 
  }
  retval.append(".") ; 
  retval.append( aWhereCombo.currentText() ) ; 
  return retval ; 
}

void FrontWidget::selectClassification(QString aNameOfClassification) {
  Hash groupFingerPrint  ;
  groupFingerPrint.calculate(aNameOfClassification.toUtf8()) ; 
  iCaListingModel.setClassification(groupFingerPrint) ;   
  // and, put a request pending to request for more articles 
  // from this classification:
  NetworkRequestExecutor::NetworkRequestQueueItem req ;
  req.iRequestType = RequestAdsClassified ;
  req.iRequestedItem = groupFingerPrint ;
  req.iState = NetworkRequestExecutor::NewRequest ; 
  req.iMaxNumberOfItems = 1 ; 
  req.iTimeStampOfItem = QDateTime::currentDateTimeUtc().toTime_t() - ( 60 * 60* 24 * 140 ) ; // select 140 days
  req.iDestinationNode = KNullHash ; 
  iController->model().lock() ; 
  iController->model().addNetworkRequest(req) ; 
  iController->model().unlock() ; 
}


void FrontWidget::contactsSelectionChangedSlot(const QItemSelection &, const QItemSelection &) {
  LOG_STR("contactsSelectionChangedSlot") ; 
  iSelectedContact = KNullHash ; 
  foreach(const QModelIndex &index, 
	  ui.contactsView->selectionModel()->selectedIndexes()) {
    iSelectedContact.fromQVariant( iContactsModel.data(index,Qt::UserRole) ) ;    
    break ; 
  }      
  LOG_STR2("Selected contact %s", qPrintable(iSelectedContact.toString())) ;   
  if ( iSelectedContact== KNullHash ) {
    ui.removeContactBtn->setEnabled(false) ;
    ui.viewProfileBtn->setEnabled(false) ;
    ui.sendMessageBtn->setEnabled(false) ;
  }else {
    ui.removeContactBtn->setEnabled(true) ;
    ui.viewProfileBtn->setEnabled(true) ;
    ui.sendMessageBtn->setEnabled(true) ;
  }
}


void FrontWidget::ownProfileCommentselectionChangedSlot(const QItemSelection &, const QItemSelection &) {
  LOG_STR("ownProfileCommentselectionChangedSlot") ; 
  foreach(const QModelIndex &index, 
	  ui.profileCommentsView->selectionModel()->selectedIndexes()) {
    iSelectedCommentFromOwnCommentListing.fromQVariant( iSelectedProfileCommentListingModel.data(index,Qt::UserRole) ) ;    
    break ; 
  }      
  LOG_STR2("Selected comment for display %s", qPrintable(iSelectedCommentFromOwnCommentListing.toString())) ;   
}

void FrontWidget::viewedProfileCommentselectionChangedSlot(const QItemSelection &, const QItemSelection &) {
  LOG_STR("viewedProfileCommentselectionChangedSlot") ; 
  
  foreach(const QModelIndex &index, 
	  ui.profileDetailsCommentsView->selectionModel()->selectedIndexes()) {
    iSelectedCommentFromViewedCommentListing .fromQVariant( iViewedProfileCommentListingModel.data(index,Qt::UserRole) ) ;    
    break ; 
  }      
  LOG_STR2("Selected viewed profile comment for display %s", qPrintable(iSelectedCommentFromViewedCommentListing .toString())) ;   
}

void FrontWidget::viewedProfileCommentDoubleClicked(const QModelIndex &/*aSelection*/ ) 
{
  LOG_STR("viewedProfileCommentDoubleClicked") ; 
  if ( iSelectedCommentFromViewedCommentListing != KNullHash &&
       iViewedProfile ) {
    activateProfileCommentDisplay(&iViewedProfileCommentListingModel,
				  iController->model().profileCommentModel(),
				  iSelectedCommentFromViewedCommentListing,
				  iViewedProfile->iFingerPrint) ;
  }
}

void FrontWidget::viewedProfileCommentKeyPressed( ) 
{
  LOG_STR("viewedProfileCommentKeyPressed") ; 
  if ( iSelectedCommentFromViewedCommentListing != KNullHash &&
       iSelectedProfile )  {
    activateProfileCommentDisplay(&iViewedProfileCommentListingModel,
				  iController->model().profileCommentModel(),
				  iSelectedCommentFromViewedCommentListing,
				  iSelectedProfile->iFingerPrint ) ;
  }
}

void FrontWidget::ownProfileCommentDoubleClicked(const QModelIndex &/* aSelection */) 
{
  LOG_STR("ownProfileCommentDoubleClicked") ; 
  if ( iSelectedCommentFromOwnCommentListing != KNullHash &&
       iSelectedProfile ) {
    activateProfileCommentDisplay(&iSelectedProfileCommentListingModel,
				  iController->model().profileCommentModel(),
				  iSelectedCommentFromOwnCommentListing,
				  iSelectedProfile->iFingerPrint) ;
  }
}


void FrontWidget::CAselectionChangedSlot(const QItemSelection &/*aSelection*/, const QItemSelection &) {
  LOG_STR("CAselectionChangedSlot") ; 
  Hash hashToDisplay ;
  foreach(const QModelIndex &index, 
	  ui.caHeadersView->selectionModel()->selectedIndexes()) {
    hashToDisplay.fromQVariant( iCaListingModel.theCaModel()->data(index,Qt::UserRole) ) ;    
    break ; 
  }      
  LOG_STR2("Display article %s", qPrintable(hashToDisplay.toString())) ;   

  if ( hashToDisplay != KNullHash ) {

    iReplyToCaButton->setEnabled(true) ; 
    iReplyToCaToForumButton->setEnabled(true) ; 
    iCommentCaPosterButton->setEnabled(true) ; 
    iViewCaPosterProfileButton->setEnabled(true) ; 

    iController->model().lock() ; 
    iCaOnDisplay = iController->model().classifiedAdsModel().caByHash(hashToDisplay)  ;
    iController->model().classifiedAdsModel().setTimeLastReference(iCaOnDisplay.iFingerPrint, 
								   QDateTime::currentDateTimeUtc().toTime_t()) ; 
    if ( iCaOnDisplay.iSenderHash != KNullHash ) {
      iController->offerDisplayNameForProfile(iCaOnDisplay.iSenderHash,
					      iCaOnDisplay.iSenderName,
					      true ) ; 
    }	
    iController->model().unlock() ; 
    emit displayedCaChanged() ; 
    iViewCaAttachmentsButton->setEnabled(iCaOnDisplay.iAttachedFiles.count() > 0) ; 
    if ( iCaOnDisplay.iSenderName.length() > 0 ) {
      iFromField.setText(tr("From:") + " " + iCaOnDisplay.iSenderName ) ;  
    } else {
      iFromField.setText(tr("From:") + " " + iController->displayableNameForProfile(iCaOnDisplay.iSenderHash) ) ;  
    }
    iSubjectField.setText(tr("Subject:") + " " + iCaOnDisplay.iSubject ) ;
    iCaText.setHtml(iCaOnDisplay.iMessageText) ; 
    setCaDocumentSize() ;
    ui.caMessageView->horizontalScrollBar()->setSliderPosition(0) ; 
    ui.caMessageView->horizontalScrollBar()->setValue(0) ; 
    ui.caMessageView->centerOn(0,0) ;
  } else {
    iCaText.setHtml("<html></html>") ; 
    iSubjectField.setText("") ; 
    iFromField.setText("") ;
    iCaOnDisplay.iSenderHash = KNullHash ;  
    iReplyToCaButton->setEnabled(false) ; 
    iReplyToCaToForumButton->setEnabled(false) ; 
    iCommentCaPosterButton->setEnabled(false) ; 
    iViewCaPosterProfileButton->setEnabled(false) ; 
    iViewCaAttachmentsButton->setEnabled(false) ; 
  } 

  ui.caHeadersView->resizeColumnToContents(0) ; 
#if QT_VERSION >= 0x050000
  // qt5
  ui.caHeadersView->header()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
#else
  ui.caHeadersView->header()->setResizeMode(0, QHeaderView::ResizeToContents);
#endif

}

void FrontWidget::privMsgselectionChangedSlot(const QItemSelection &, const QItemSelection &) 
{
  LOG_STR("privmsgselectionChangedSlot") ;
  iPrivMsgOnDisplay.iSenderHash = KNullHash ;  
  Hash hashToDisplay ;
  foreach(const QModelIndex &index, 
	  ui.privateMessageListView->selectionModel()->selectedIndexes()) {
    hashToDisplay.fromQVariant( iPrivMsgSearchModel.data(index,Qt::UserRole) ) ;    
    break ; 
  }  

  LOG_STR2("Display private message %s", qPrintable(hashToDisplay.toString())) ;   

  iController->model().lock() ; 
  PrivMessage* msg ( iController->model().privateMessageModel().messageByFingerPrint(hashToDisplay)) ;
  if ( msg ) {
    iPrivMsgOnDisplay = *msg ; 
    delete msg ; 
    iController->model().privateMessageModel().setAsRead(hashToDisplay, true);
    iController->model().privateMessageModel().setTimeLastReference(hashToDisplay, 
								    QDateTime::currentDateTimeUtc().toTime_t()) ; 
    iPrivMsgSearchModel.setAsRead(hashToDisplay, true);
    if ( iPrivMsgOnDisplay.iSenderHash != KNullHash ) {
      iController->offerDisplayNameForProfile(iPrivMsgOnDisplay.iSenderHash,
					      iPrivMsgOnDisplay.iSenderName,
					      true ) ; 
    }
  }
  iController->model().unlock() ; 
  
  if ( iPrivMsgOnDisplay.iSenderHash != KNullHash ) {
    if (iSelectedProfile && iPrivMsgOnDisplay.iSenderHash == iSelectedProfile->iFingerPrint ) {
      iPrivMsgFromField.setText(tr("To:") + " " + iController->displayableNameForProfile(iPrivMsgOnDisplay.iRecipient) ) ;  
    } else {
      if ( iPrivMsgOnDisplay.iSenderName.length() > 0 ) {
	iPrivMsgFromField.setText(tr("From:") + " " + iPrivMsgOnDisplay.iSenderName ) ;  
      } else {
	iPrivMsgFromField.setText(tr("From:") + " " + iController->displayableNameForProfile(iPrivMsgOnDisplay.iSenderHash) ) ;  
      }
    }
    iPrivMsgSubjectField.setText(tr("Subject:") + " " + iPrivMsgOnDisplay.iSubject ) ;
    iPrivMsgText.setHtml(iPrivMsgOnDisplay.iMessageText) ; 
    ui.privateMessageView->horizontalScrollBar()->setSliderPosition(0) ; 
    ui.privateMessageView->horizontalScrollBar()->setValue(0) ; 
    iReplyToPrivMsgButton->setEnabled(true) ; 
    iCommentPrivMsgPosterButton->setEnabled(true) ; 
    iViewPrivMsgPosterProfileButton->setEnabled(true) ; 
  } else {
    iPrivMsgText.setHtml("<html></html>") ; 
    iPrivMsgSubjectField.setText("") ; 
    iPrivMsgFromField.setText("") ;
    iPrivMsgOnDisplay.iSenderHash = KNullHash ;  
    iReplyToPrivMsgButton->setEnabled(false) ; 
    iCommentPrivMsgPosterButton->setEnabled(false) ; 
    iViewPrivMsgPosterProfileButton->setEnabled(false) ; 
  }
  setPrivMsgSize() ; 
  iViewPrivMsgAttachmentsButton->setEnabled(iPrivMsgOnDisplay.iSenderHash != KNullHash &&
					    iPrivMsgOnDisplay.iAttachedFiles.count() ) ; 
  emit displayedPrivMsgChanged() ;
}

void FrontWidget::receiveNotifyOfContentReceived(const Hash& aHashOfContent,
						 const ProtocolItemType aTypeOfReceivdContent){
  bool profileViewTabIsOpen(false) ;
  switch (aTypeOfReceivdContent) {
  case UserProfile:
    {
      if  ( iViewedProfile && ( iViewedProfile->iFingerPrint == aHashOfContent ) ) {
	// ahem, we have this profile on display..
	// do we have also the tab open:
	QWidget* currentTabOnDisplay ( ui.tabWidget->currentWidget() ) ;
	if ( currentTabOnDisplay && currentTabOnDisplay->objectName() == internalNameOfProfileTab ) {
	  profileViewTabIsOpen = true ; 
	}
	// ahem, re-display the profile
	if ( profileViewTabIsOpen ) {
	  iController->userInterfaceAction(MController::ViewProfileDetails, 
					   aHashOfContent ) ; 
	} else {
	  // or just update the data:
	  delete iViewedProfile ;
	  iViewedProfile = NULL ; 
	  iController->model().lock() ;
	  iViewedProfile = iController->model().profileModel().profileByFingerPrint( aHashOfContent ) ;
	  if ( iViewedProfile ) {
	    iController->model().profileModel().setTimeLastReference(iViewedProfile->iFingerPrint, QDateTime::currentDateTimeUtc().toTime_t()) ; 
	  }
	  iController->model().unlock() ;
	  if ( iViewedProfile ) {
	    iController-> sendProfileUpdateQuery(iViewedProfile->iFingerPrint,
						 iViewedProfile->iNodeOfProfile == NULL ? 
						 KNullHash :
						 iViewedProfile->iNodeOfProfile->nodeFingerPrint()) ;
	  }
	  updateUiFromViewedProfile() ; 
	}
      }
    }
    break ; 
  default:
    break ; 
  } ; 
}

void FrontWidget::receiveNotifyOfContentReceived(const Hash& aHashOfContent,
						 const Hash& aHashOfClassificationOrDestination,
						 const ProtocolItemType aTypeOfReceivdContent)
{
  switch (aTypeOfReceivdContent) {
  case ClassifiedAd:
    iCaListingModel.newCaReceived(aHashOfContent,
				  aHashOfClassificationOrDestination) ; 
    break ; 
  case PrivateMessage:
    iPrivMsgSearchModel.newMsgReceived(aHashOfContent,
				       aHashOfClassificationOrDestination) ; 
    break ; 
  case UserProfileComment:
    // listing models will internally check if the content was meant
    // for them so no conditioning here:
    QLOG_STR("Calling newCommentReceived for comment listing models with profile hash " + aHashOfClassificationOrDestination.toString() ) ; 
    iSelectedProfileCommentListingModel.newCommentReceived(aHashOfContent,
							   aHashOfClassificationOrDestination) ; 
    iViewedProfileCommentListingModel.newCommentReceived(aHashOfContent,
							 aHashOfClassificationOrDestination) ; 
    break ; 
  default:
    break ; 
  } ;
}

void FrontWidget::addContactButtonClicked() {
  LOG_STR("addContactButtonClicked"); 
  EditContactDialog *contact_dialog = 
    new EditContactDialog(this, iController,
			  KNullHash,
			  "",
			  false,
			  iContactsModel) ;
  connect(contact_dialog,
	  SIGNAL(  error(MController::CAErrorSituation,
			 const QString&) ),
	  iController,
	  SLOT(handleError(MController::CAErrorSituation,
			   const QString&)),
	  Qt::QueuedConnection ) ;
  contact_dialog->show() ; // the dialog will delete self
}

void FrontWidget::editContactActionSelected() {
  LOG_STR("editContactActionSelected"); 
  if ( iSelectedContact != KNullHash ) {
    EditContactDialog *contact_dialog = 
      new EditContactDialog(this, iController,
			    iSelectedContact,
			    "",
			    false,
			    iContactsModel) ;
    connect(contact_dialog,
	    SIGNAL(  error(MController::CAErrorSituation,
			   const QString&) ),
	    iController,
	    SLOT(handleError(MController::CAErrorSituation,
			     const QString&)),
	    Qt::QueuedConnection ) ;
    contact_dialog->show() ; // the dialog will delete self
  }
}

void FrontWidget::removeContactButtonClicked() {
  if ( iSelectedContact != KNullHash ) {
    iController->model().lock() ;
    iContactsModel.removeContact(iSelectedContact) ; 
    iController->storePrivateDataOfSelectedProfile() ; 
    iController->model().unlock() ;
  }
}
void FrontWidget::viewContactProfileButtonClicked() {
  if ( iSelectedContact != KNullHash ) {
    iController->userInterfaceAction(MController::ViewProfileDetails, 
				     iSelectedContact ) ; 
  }
}
void FrontWidget::sendMsgToContactButtonClicked() {
  LOG_STR("sendMsgToContactButtonClicked"); 
  if ( iSelectedContact != KNullHash ) {
    NewPrivMessageDialog *posting_dialog = 
      new NewPrivMessageDialog(this, iController,
			       iSelectedContact.toString(),
			       "",
			       *iSelectedProfile,
			       iPrivMsgSearchModel) ;
    connect(posting_dialog,
	    SIGNAL(  error(MController::CAErrorSituation,
			   const QString&) ),
	    iController,
	    SLOT(handleError(MController::CAErrorSituation,
			     const QString&)),
	    Qt::QueuedConnection ) ;
    posting_dialog->show() ; // the dialog will delete self
  }
}

void FrontWidget::addMessageSenderToContacts()
{
  if ( iPrivMsgOnDisplay.iSenderHash != KNullHash ) {

    EditContactDialog *contact_dialog = 
      new EditContactDialog(this, iController,
			    iPrivMsgOnDisplay.iSenderHash ,
			    iPrivMsgOnDisplay.iSenderName,
			    false,
			    iContactsModel ) ;
    connect(contact_dialog,
	    SIGNAL(  error(MController::CAErrorSituation,
			   const QString&) ),
	    iController,
	    SLOT(handleError(MController::CAErrorSituation,
			     const QString&)),
	    Qt::QueuedConnection ) ;
    contact_dialog->show() ; // the dialog will delete self
  }
}

void FrontWidget::addCaSenderToContacts()
{
  if ( iCaOnDisplay.iSenderHash != KNullHash ) {
    EditContactDialog *contact_dialog = 
      new EditContactDialog(this, iController,
			    iCaOnDisplay.iSenderHash ,
			    iCaOnDisplay.iSenderName,
			    false,
			    iContactsModel ) ;
    connect(contact_dialog,
	    SIGNAL(  error(MController::CAErrorSituation,
			   const QString&) ),
	    iController,
	    SLOT(handleError(MController::CAErrorSituation,
			     const QString&)),
	    Qt::QueuedConnection ) ;
    contact_dialog->show() ; // the dialog will delete self
  }
}

void FrontWidget::addContactFromContactViewButtonClicked() 
{
  if ( iViewedProfile ) {
    EditContactDialog *contact_dialog = 
      new EditContactDialog(this, iController,
			    iViewedProfile->iFingerPrint,
			    iViewedProfile->displayName(),
			    false,
			    iContactsModel ) ;
    connect(contact_dialog,
	    SIGNAL(  error(MController::CAErrorSituation,
			   const QString&) ),
	    iController,
	    SLOT(handleError(MController::CAErrorSituation,
			     const QString&)),
	    Qt::QueuedConnection ) ;
    contact_dialog->show() ; // the dialog will delete self
  }
}

QVariant FrontWidget::contactDataOfSelectedProfile() {
  return iContactsModel.contactsAsQVariant() ; 
}

void FrontWidget::setContactDataOfSelectedProfile(const QVariantList& aContacts) 
{
  iContactsModel.setContactsFromQVariant(aContacts) ; 
}

bool FrontWidget::isContactInContactList(const Hash& aFingerPrint) const {
  return iContactsModel.isContactContained(aFingerPrint) ; 
}


void FrontWidget::activateProfileCommentDisplay(ProfileCommentListingModel* aListingModel,
						ProfileCommentModel& aCommentModel,
						const Hash& aInitialComment,
						const Hash& aViewedProfile) {

  ProfileCommentDisplay* dialog = this->findChild<ProfileCommentDisplay*>(internalNameOfCommentDialog) ;
  if ( dialog ) {
    dialog->close() ;
    dialog = NULL ; 
  }

  dialog = new 
    ProfileCommentDisplay(this,
			  iController,
			  aListingModel,
			  aCommentModel,
			  aInitialComment,
			  aViewedProfile,
			  *iSelectedProfile) ;

  dialog->setObjectName(internalNameOfCommentDialog) ; 
  connect(dialog,
	  SIGNAL(  error(MController::CAErrorSituation,
			 const QString&) ),
	  iController,
	  SLOT(handleError(MController::CAErrorSituation,
			   const QString&)),
	  Qt::QueuedConnection ) ;


  // if selected profile is deleted (due to change), close
  // dialog also
  connect (iSelectedProfile, 
	   SIGNAL(destroyed()),
	   dialog,
	   SLOT(reject())) ;

  if ( iSelectedProfile->iFingerPrint != aViewedProfile ) {
    // viewing dialog is about "viewed profile" comments ; if
    // that profile is changed, also close the dialog:
    connect (iViewedProfile,
	     SIGNAL(destroyed()),
	     dialog,
	     SLOT(reject())) ;
  }
  dialog->show() ; // the dialog will delete self			  aInitialComment ) ; 
}

