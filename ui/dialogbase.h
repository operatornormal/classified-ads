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
#ifndef CA_DIALOG_BASE_H
#define CA_DIALOG_BASE_H
#include <QString>
#include <QDialog>
#include "../mcontroller.h"

class Profile ; 
class QLabel ; 

/**
 * @brief base-class for content-posting dialogs of classified ads
 */
class DialogBase : public QDialog {
  Q_OBJECT


protected: // methods
  /** constructor */
  DialogBase(QWidget* aParent,
	     MController* aController,
	     Profile& aSelectedProfile) ;
  /** 
   * method for publishing an (attachment) file 
   * @param aFileName is name of the file in filesystem
   * @param aForceNoEncryption if set to true, always produce
   *        plain-text binary, with no encryption
   * @param aBinaryRecipientList if non-NULL, contains list of operator
   *        key fingerprints that are the operators that will be able to
   *        read the binary. 
   * @return fingerprint of the published file or KNullHash
   */
  Hash publishBinaryAttachment(const QString& aFileName,
			       bool aForceNoEncryption = false,
			       const QList<Hash>* aBinaryRecipientList = NULL) ; 

signals:
  void error(MController::CAErrorSituation aError,
	     const QString& aExplanation) ;
protected slots:
  void attachButtonClicked() ; 
protected: // variables
  MController* iController ; 
  Profile& iSelectedProfile ;
  QList<QString> iFilesAboutToBeAttached ; /**< used in posting-dialogs */
  /** label used to show list of attached files. pointer must be set
      by inheriting class prior to call to slot attachButtonClicked() */
  QLabel* iAttachmentListLabel ; 
} ; 

#endif
