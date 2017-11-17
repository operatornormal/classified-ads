/*  -*-C++-*- -*-coding: utf-8-unix;-*-
  Classified Ads is Copyright (c) Antti Järvinen 2013-2017.

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
#ifndef CA_DIALOG_BASE_H
#define CA_DIALOG_BASE_H
#include <QString>
#include <QDialog>
#include "../mcontroller.h"
#include "metadataQuery.h" // for metadata data structure
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
     * @param aFileMetadata describes the file. The iFileName member of
     *        the structure must contain valid filesystem filename
     * @param aForceNoEncryption if set to true, always produce
     *        plain-text binary, with no encryption
     * @param aBinaryRecipientList if non-NULL, contains list of operator
     *        key fingerprints that are the operators that will be able to
     *        read the binary.
     * @return fingerprint of the published file or KNullHash
     */
    Hash publishBinaryAttachment(const MetadataQueryDialog::MetadataResultSet& aFileMetadata,
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
    /** list of files used in posting-dialogs */
    QList<MetadataQueryDialog::MetadataResultSet> iFilesAboutToBeAttached ;
    /** label used to show list of attached files. pointer must be set
        by inheriting class prior to call to slot attachButtonClicked() */
    QLabel* iAttachmentListLabel ;
} ;

#endif
