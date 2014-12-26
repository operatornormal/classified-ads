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

#ifndef LIST_ATTACHMENTS_DIALOG_H
#define LIST_ATTACHMENTS_DIALOG_H

#include "dialogbase.h"
#include "../mcontroller.h"
#include "../ui_attachmentListDialog.h"

class BinaryFileListingModel ;
class Hash ; 
class Profile ;
class QAction ; 

/**
 * @brief class for dialog for listing (attached) files
 */
class AttachmentListDialog : public DialogBase
{
  Q_OBJECT

public:
  /**
   * Constructor.
   * @param aParent parent widget
   * @param aController application controller
   * @param aSelectedProfile operator profile currently open on application
   * @param aFilesToDisplay list of hashes that are supposed to be found
   *                        from table of binary blobs
   * @param aNodeToTryForRetrieval In case the selected file is not
   *                               found from local db table, this is
   *                               the node where query for missing file
   *                               could be first sent, in addition to nodes
   *                               determined by the binary blob hash. 
   *                               Use-case for this is CA attachment where
   *                               the posting-node is known from CA metadata.
   *                               If we don't have the binary blob, we 
   *                               could first send query about attachment
   *                               to the originating node. 
   */
  AttachmentListDialog(QWidget *aParent,
		       MController* aController,
		       const Profile& aSelectedProfile,
		       QList<Hash>& aFilesToDisplay,
		       const Hash& aNodeToTryForRetrieval = KNullHash );
  /** destructor */
  ~AttachmentListDialog();
  /**
   * Method that tries to find node fingerprint by profile. This is
   * used to find original posting node of binary attachments
   * that are associated with many objects (ca, privmsg etc.) 
   * @param aProfileFingerPrint fingerprint of profile whose node is sought for
   * @param aController application controller.
   * @return node hash or KNullHash if nothing found
   */
  static Hash tryFindNodeByProfile(const Hash& aProfileFingerPrint, MController& aController); 
private slots:
  void okButtonClicked() ;
  void cancelButtonClicked() ;
  void fileListDoubleClicked(const QModelIndex& aIndex);
  void exportSharedFile() ;
signals:
  void error(MController::CAErrorSituation aError,
	     const QString& aExplanation) ;
private: // members
  Ui_attachmentListDialog ui ; 
  BinaryFileListingModel* iListingModel ;
  QAction* iExportSharedFileAction ; /**< context-menu action for saving to filesystem a shared file */
  const Hash iNodeToTryForRetrieval ;
};

#endif
