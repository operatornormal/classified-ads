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

#ifndef PROFILECOMMENTDISPLAY_DIALOG_H
#define PROFILECOMMENTDISPLAY_DIALOG_H

#include <QDialog>
#include "../mcontroller.h"
#include "../ui_profileCommentDisplay.h"
#include "dialogbase.h"

class ProfileCommentListingModel ;
class ProfileCommentModel ;
class ProfileComment ;
class QPushButton ;
class ProfileCommentItemDelegate ;
class Profile ;
class QAction ; 

/**
 * @brief class for display of single classified ad
 */
class ProfileCommentDisplay : public DialogBase {
    Q_OBJECT

public:
    /**
     * Constructor.
     *
     * @param aParent is owner-window of this dialog
     * @param aController application controller reference
     * @param aListingModel where the content comes from
     * @param aCommentModel needed when posting new comments
     * @param aFirstCommentToDisplay where dialog should scroll initialy
     * @param aViewedProfile profile whose comments are displayed
     */
    ProfileCommentDisplay(QWidget *aParent,
                          MController* aController,
                          ProfileCommentListingModel* aListingModel,
                          ProfileCommentModel& aCommentModel,
                          const Hash& aFirstCommentToDisplay,
                          const Hash& aViewedProfile ,
                          Profile& aSelectedProfile);
    /** destructor */
    ~ProfileCommentDisplay();

private slots:
    void closeButtonClicked() ;
    /**
     * method for handling "add comment"-button
     */
    void newCommentButtonClicked() ;
    void exportSharedFile() ;
    void currentItemChanged(const QModelIndex & current, const QModelIndex & previous ) ;
private:
    Ui_profileCommentDisplay ui ;
    ProfileCommentListingModel* iListingModel;
    ProfileCommentModel& iCommentModel;
    Hash iCommentToDisplay;
    ProfileCommentItemDelegate* iItemDelegate ;
    Hash iViewedProfile ;
    QAction* iExportSharedFileAction ; /**< context-menu action for saving to filesystem a shared file */
    Hash iFingerPrintOfCommentOnFocus ;
};

#endif
