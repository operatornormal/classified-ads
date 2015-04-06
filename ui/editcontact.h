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

#ifndef EDIT_CONTACT_DIALOG_H
#define EDIT_CONTACT_DIALOG_H

#include <QDialog>
#include "../mcontroller.h"
#include "../ui_editContact.h"
class ContactListingModel ;

class Contact ;

/**
 * @brief class for editing a contact-list item
 *
 */
class EditContactDialog : public QDialog {
    Q_OBJECT

public:
    /**
     * Constructor.
     */
    EditContactDialog(QWidget *aParent,
                      MController* aController,
                      const Hash& aProfileFingerPrint,
                      const QString& aNickName,
                      bool aIsTrusted,
                      ContactListingModel& aContactsModel );
    /** destructor */
    ~EditContactDialog();

private slots:
    void okButtonClicked() ;
    void cancelButtonClicked() ;
signals:
    void error(MController::CAErrorSituation aError,
               const QString& aExplanation) ;
private:
    Ui_editContactDialog ui ;
    MController* iController ;
    ContactListingModel& iContactsModel ;
};

#endif
