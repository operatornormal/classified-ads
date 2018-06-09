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

#ifndef PROFILE_READERs_DIALOG_H
#define PROFILE_READERs_DIALOG_H

#include <QDialog>
#include "../mcontroller.h"
#include "../ui_profileReadersDialog.h"

class ProfileSearchModel ;
class QTimerEvent ;
class ProfileReadersListingModel ;
class Profile ;
class QAction ; 

/**
 * @brief class for allowing edit of profile readers list
 *
 */
class ProfileReadersDialog : public QDialog {
    Q_OBJECT

public:
    /**
     * Constructor.
     *
     * @param aParent is owner-window of this dialog
     * @param aController application controller reference
     * @param aProfile is profile whose readers are listed
     */
    ProfileReadersDialog(QWidget *aParent,
                         MController* aController,
                         Profile& aProfile );
    /** destructor */
    ~ProfileReadersDialog();
protected:
    /**
     * method used to check for changes in users search-field ; this way
     * user can keep on typing and we can perform the search when
     * field content changes ..
     */
    void timerEvent(QTimerEvent *event);
private slots:
    void closeButtonClicked() ;
    void addButtonClicked() ;
    void removeButtonClicked() ;
    void viewProfileSelected() ;
signals:
    void error(MController::CAErrorSituation aError,
               const QString& aExplanation) ;
private:
    Ui_profileReadersDialog ui ;
    MController* iController ;
    ProfileSearchModel* iSearchModel ; /**< datamodel that does search */
    int iTimerId ;
    QString iPreviousSearchFieldContent ;
    Profile& iProfile ;
    ProfileReadersListingModel* iListingModel ; /**< datamodel that lists names in nifty manner */
    QAction *iViewProfileAction ;
};

#endif
