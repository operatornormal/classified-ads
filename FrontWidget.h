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

#ifndef FRONT_H
#define FRONT_H
#include <QtGui>
#include <QGraphicsScene>
#include <QGraphicsTextItem>
#include <QGraphicsSimpleTextItem>
#include "ui_frontWidget.h"
#include "controller.h"
#include "datamodel/calistingmodel.h"
#include "datamodel/ca.h"
#include "datamodel/privmsgsearchmodel.h"
#include "datamodel/privmsg.h"
#include <QHeaderView>
#include "datamodel/contactlistingmodel.h"
#include "datamodel/profilecommentlistingmodel.h"

class Profile ;
class BinaryFileListingModel ;
class QShortcut ; 
class ProfileCommentModel ;

class FrontWidget: public QWidget {
  Q_OBJECT

public:
  FrontWidget(Controller* aController,QMainWindow& aParent ) ;
  ~FrontWidget() ;
  /** puts profile of any operator on display */
  void showDetailsOfProfile(const Hash& aFingerPrint) ;
  /** displays a selected classified ad */
  void showClassifiedAd(const CA& ca) ; 
  /** displays a selected profile comment */
  void showSingleCommentOfProfile(const Hash aCommentFingerPrint) ; 
  /**
   * Method for filling ad-category-selection -related
   * comboboxes. This is implemented here in frontwidget but
   * is used by dialogs as well, and that is the reason
   * why this method is public.
   */
  static void fillCaSelectionCombobox(QComboBox& aComboBox,
				      bool isAboutComboBox,
				      MController& aController) ; 
  /**
   * Method for getting the fully qualified name of the group
   * selection that user has made, for reason or another. Called
   * also from dialogs, thus public
   */
  static QString selectedClassification(const QComboBox& aAboutCombo,
					const QComboBox& aRegardingCombo,
					const QComboBox& aWhereCombo,
					const MController& aController) ; 
  /**
   * kind of "observer" method called from controller when there
   * is new content available ; odds are that we're displaying
   * or waiting some of that
   */
  void receiveNotifyOfContentReceived(const Hash& aHashOfContent,
				      const ProtocolItemType aTypeOfReceivdContent) ; 
  /**
   * kind of "observer" method called from controller when there
   * is new content available ; odds are that we're displaying
   * or waiting some of that. This variant is hit with notifications
   * about classified ads or private messages
   * @param aHashOfContent fingerprint of the object (ca or priv msg)
   *                       received
   * @param aHashOfClassificationOrDestination in case of ca, this is
   *                       fingerprint of the classification, in case of
   *                       private message, this is recipient profile
   *                       fingerprint. 
   */
  void receiveNotifyOfContentReceived(const Hash& aHashOfContent,
				      const Hash& aHashOfClassificationOrDestination,
				      const ProtocolItemType aTypeOfReceivdContent) ; 
  /**
   * method for retrieving contacts list of the profile currently selected 
   */
  QVariant contactDataOfSelectedProfile() ;
  /**
   * method for setting contacts list of the profile currently selected 
   */
  void setContactDataOfSelectedProfile(const QVariantList& aContacts) ;
  /**
   * method for checking if given profile is in contact list of
   * selected profile
   */
  bool isContactInContactList(const Hash& aFingerPrint) const ; 
  /**
   * method for getting currently selected profile or NULL 
   * if no profile selected
   */
  const Profile *selectedProfile() const {
    return iSelectedProfile ; 
  } 
  /**
   * method for opening a binary file, called from url handling and
   * from "save shared file.." selection
   */
  void openBinaryFile(const Hash& aFingerPrint,
		      bool aUseViewedProfileNodeAsNetreqDest = false ) ;
public slots:
  virtual void publishProfileButtonClicked() ; /**< callback of publish btn */
  virtual void revertProfileButtonClicked() ;/**< callback of revert btn */
  virtual void profileImageClicked() ;/**< callback of image selection */
  virtual void profileReadersClicked() ;/**< callback of reader-list edit */
  virtual void profileEdited(const QString& aNewText) ;/**< callback for any UI actions that edit profile */
  virtual void isPrivateSettingChanged(int aState) ;/**< callback of "is private" checkbox */
  virtual void userProfileSelected(const Hash& aProfile) ;/**< called from controller when another user profile is selected */
  virtual void fileToBeSharedAdded() ;/**< operator wants to share a file */
  virtual void fileToBeSharedRemoved() ;/**< operator wants to stop advertasing a shared file */
  virtual void exportSharedFile() ;/**< operator wants to save a shared file */
  virtual void profileSendMsgButtonClicked() ; 
  virtual void profileShowReadersButtonClicked() ; 
  virtual void profileSendCommentButtonClicked() ; 
  virtual void caTabAboutComboChanged(int aNewIndex) ; 
  virtual void caTabConcernsComboChanged(int aNewIndex) ; 
  virtual void caTabWhereComboChanged(int aNewIndex) ; 
  // slots for buttons at bottom of "classified ads" ta
  virtual void replyToCaButtonClicked() ; 
  virtual void replyToCaToForumButtonClicked() ; 
  virtual void commentCaPosterProfile() ; 
  virtual void viewCaPosterProfile() ;
  virtual void viewCaAttachments() ; 
  virtual void postNewClassifiedAd() ;
  virtual void performAdsSearchClicked() ; /**< callback of "search ads" */
  /**
   * this slot is called when classified ads header listing view
   * has new selection
   */
  virtual void CAselectionChangedSlot(const QItemSelection &, const QItemSelection &) ;
  /**
   * this slot is called when private messages header listing view
   * has new selection
   */
  virtual void privMsgselectionChangedSlot(const QItemSelection &, const QItemSelection &) ;
  /**
   * this slot is called when contacts listing view has new selection
   */
  virtual void contactsSelectionChangedSlot(const QItemSelection &, const QItemSelection &) ;
  virtual void replyToPrivMsgButtonClicked() ; /**< from priv msg tab btn */
  virtual void commentPrivMsgPosterProfile() ; /**< from priv msg tab btn */
  virtual void viewPrivMsgPosterProfile() ; /**< from priv msg tab btn */
  virtual void postNewPrivateMessage() ;/**< from priv msg tab btn */
  /** displays list of attachments of a private message */
  virtual void viewPrivMsgAttachments() ; 
  virtual void addMessageSenderToContacts() ;/**< called from messages tab */
  virtual void addCaSenderToContacts() ;/**< called from ca listing tab */
  // slots related to contacts-tab
  virtual void addContactButtonClicked() ; /**< new contact */
  /** new contact from "view contact" tab */
  virtual void addContactFromContactViewButtonClicked() ; 
  virtual void removeContactButtonClicked() ; /**< remove contact */
  virtual void viewContactProfileButtonClicked() ; /**< view contact profile */
  virtual void sendMsgToContactButtonClicked() ; /**< send message to contact list profile */
  virtual void editContactActionSelected() ; /**< edit */
  /**
   * this slot is called when own profile comments listing is
   * changed
   */
  virtual void ownProfileCommentselectionChangedSlot(const QItemSelection &, const QItemSelection &) ;
  /**
   * this slot is called when viewed profile comments listing is
   * changed
   */
  virtual void viewedProfileCommentselectionChangedSlot(const QItemSelection &, const QItemSelection &) ;
  /**
   * this slot is called when user doubleclicks profile comment listing
   * of viewed profile
   */
  virtual void viewedProfileCommentDoubleClicked(const QModelIndex &aSelection) ;
  /**
   * this slot is called when user hits enter while having keyboard
   * focus on profile comment listing
   */
  virtual void viewedProfileCommentKeyPressed() ;
  /**
   * this slot is called when user doubleclicks profile comment listing
   * of own profile
   */
  virtual void ownProfileCommentDoubleClicked(const QModelIndex &aSelection) ;
  /**
   * this slot is called when user clicks on link on document
   */
  virtual void linkActivated ( const QString &aLink ) ; 
signals:
  void error(MController::CAErrorSituation aError,
             const QString& aExplanation) ;
  /** emitted when ca on display is replaced */
  void displayedCaChanged() ;
  /** emitted when privmsg on display is replaced */
  void displayedPrivMsgChanged() ;
private:
  /**
   * method that reads @ref iSelectedProfile and updates UI elements
   * accordingly into "my profile" tab
   */
  void updateUiFromSelectedProfile() ;
  /**
   * method that sets up shared file listing model based on selected profile.
   * if selected profile is NULL, this will tear down the model too.
   */
  void setUpSelectedProfileFileListingModel() ; 
  /**
   * separate init method for setting up the classified-ads tab
   * as it is fairly complex
   */
  void setupClassifiedAdsTab() ; 
  /**
   * separate init method for setting up the private messages tab
   * as it is fairly complex. This must be called after each
   * profile re-selection
   */
  void setupPrivateMessagesTab() ; 
  /**
   * contacts tab setup
   */
  void setupContactsTab() ; 
  /**
   * Method that set selected ad-classification on display
   * @param aNameOfClassification is the classification, like 
   *        "ToBeGivenAway.Clothing.AtYourPlace"
   */
  void selectClassification(QString aNameOfClassification) ; 
  /**
   * method that reads @ref iViewedProfile and updates UI elements
   * accordingly into "viewed profile" tab
   */
  void updateUiFromViewedProfile() ;
  /**
   * method for bringing up dialog for display of profile comments
   * @param aListingModel datamodel holding the comments to list (UI model)
   * @param aCommentModel datamodel holding the comment data (storage model)
   * @param aInitialComment if non-null, hash of comment to initially focus
   * @param aViewedProfile fingerprint of profile whose comments are shown
   * @return none
   */
  void activateProfileCommentDisplay(ProfileCommentListingModel* aListingModel,
				     ProfileCommentModel& aCommentModel,
				     const Hash& aInitialComment,
				     const Hash& aViewedProfile ) ;
private: // methods
  void setCaDocumentSize() ; 
  void setPrivMsgSize() ;
private:
  Ui::frontWidget ui ;
  Controller* iController ;
  QMainWindow& iParent ;
  Profile *iSelectedProfile ; /**< components of selected "My profile" */
  Profile *iViewedProfile ; /**< contents of "view profile" tab */
  QAction* iAddSharedFileAction ; /**< context-menu action for adding shared file */
  QAction* iRemoveSharedFileAction ; /**< context-menu action for adding shared file */
  QAction* iExportSharedFileAction ; /**< context-menu action for saving to filesystem a shared file */
  /** model for displaying file list "operator profile" ; there may be 2nd instance
   * for displaying file list of some other profile that is not operators own 
   */
  BinaryFileListingModel* iSelectedProfileFileListingModel ; 
  /** model for displaying file list of random viewed profile 
   */
  BinaryFileListingModel* iViewedProfileFileListingModel ; 
  QPushButton* iReplyToCaButton ; 
  QPushButton* iReplyToCaToForumButton ; 
  /** button for opening dialog for posting a new CA */
  QPushButton* iNewCaButton ; 
  /** button for opening a dialog for commenting a CA poster profile */
  QPushButton* iCommentCaPosterButton ; 
  /** button for opening view of profile from CA display */
  QPushButton* iViewCaPosterProfileButton ; 
  /** button for displaying attachments of a CA on display */
  QPushButton* iViewCaAttachmentsButton ; 
  QAction* iAddToContactsFromCaList ;
  CAListingModel iCaListingModel ; /**< used for content of ad-listing view */
  QGraphicsScene iCaScene ; /**< this is used to display elements of an ad */
  QGraphicsTextItem iCaText ; /**< this is used to render text of classified ad */
  CA iCaOnDisplay ; /**< Classified ad on */
  QGraphicsSimpleTextItem iFromField ;
  QGraphicsSimpleTextItem iSubjectField ;
  const QPoint iArticleTopLeft ; 
  // private message tab related variables
  PrivateMessageSearchModel iPrivMsgSearchModel ;
  PrivMessage iPrivMsgOnDisplay ; 
  QGraphicsScene iPrivMsgScene ; /**< this is used to display elements of an priv msg */
  QGraphicsTextItem iPrivMsgText ; /**< this is used to render text of priv msg */
  QGraphicsSimpleTextItem iPrivMsgFromField ;
  QGraphicsSimpleTextItem iPrivMsgSubjectField ;
  const QPoint iPrivMsgTopLeft ; 
  QPushButton* iReplyToPrivMsgButton; 
  QPushButton* iNewPrivMsgButton ;
  QPushButton* iCommentPrivMsgPosterButton;
  QPushButton* iViewPrivMsgPosterProfileButton;
  QPushButton* iViewPrivMsgAttachmentsButton ; 
  QAction* iAddToContactsFromMsgList ;
  // variables related to contacts-list
  ContactListingModel iContactsModel ; 
  Hash iSelectedContact ; 
  QAction* iEditContactAction ;
  // variables related to profile-comment display
  ProfileCommentListingModel iSelectedProfileCommentListingModel ; 
  ProfileCommentListingModel iViewedProfileCommentListingModel ; 
  Hash iSelectedCommentFromOwnCommentListing ;
  Hash iSelectedCommentFromViewedCommentListing ;
  QShortcut* iProfileListingKeyboardGrabber ; 
  bool iWindowSizeAdjusted ; 
} ;
#endif
