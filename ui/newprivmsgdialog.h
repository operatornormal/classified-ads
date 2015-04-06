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

#ifndef NEW_PRIVATE_MESSAGE_DIALOG_H
#define NEW_PRIVATE_MESSAGE_DIALOG_H

#include "../textedit/textedit.h"
#include "../mcontroller.h"
#include "../ui_newPrivMsg.h"
#include "../datamodel/profile.h"

class PrivMessage ;
class PrivMessageModel ;
class PrivateMessageSearchModel ;
/**
 * @brief class for allowing posting of a message to named recipient
 *
 */
class NewPrivMessageDialog : public TextEdit {
    Q_OBJECT

public:
    /**
     * Constructor.
     *
     * @param aParent is owner-window of this dialog
     * @param aController application controller reference
     * @param aRecipient profile that will receive the message
     * @param aSubject if msg is reply to another msg, this is subject of
     *                 the original posting
     * @param aSelectedProfile profile doing the sending
     * @param aReferences if msg is reply to another msg, this is article referenced. NULL
     *                    if article is start of a new thread.

     */
    NewPrivMessageDialog(QWidget *aParent,
                         MController* aController,
                         const QString& aRecipient,
                         const QString& aSubject,
                         Profile& aSelectedProfile,
                         PrivateMessageSearchModel& aSearchModel,
                         const Hash& aReferencesMsg = KNullHash,
                         const Hash& aReferencesCa = KNullHash,
                         const Hash& aRecipientsNode = KNullHash);
    /** destructor */
    ~NewPrivMessageDialog();

private slots:
    void okButtonClicked() ;
    void cancelButtonClicked() ;
signals:
    void error(MController::CAErrorSituation aError,
               const QString& aExplanation) ;
private: // methods
    // if recipients node is not known, here try finding out one
    Hash tryFindRecipientNode(const Hash& aRecipientFingerPrint) ;
private:
    Ui_newPrivMsgDialog ui ;
    Hash iReferencesMsg ; /**< if we're referencing another msg, this is the FP */
    Hash iReferencesCa ; /**< if we're referencing ca, this is the FP */
    PrivateMessageSearchModel& iSearchModel ;
    Hash iRecipientsNode ;
};

#endif
