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

#ifndef CA_STATUS_DIALOG_H
#define CA_STATUS_DIALOG_H

#include <QDialog>
#include "../mcontroller.h"
#include "../ui_statusDialog.h"

class ConnectionListingModel ;

/**
 * @brief class for displaying connections status
 *
 */
class StatusDialog : public QDialog {
    Q_OBJECT

public:
    /**
     * Constructor.
     */
    StatusDialog(QWidget *aParent,
                 MController& aController );
    /** destructor */
    ~StatusDialog();

private slots:
    void addButtonClicked() ;
signals:
    void error(MController::CAErrorSituation aError,
               const QString& aExplanation) ;
private:
    Ui_statusDialog ui ;
    MController& iController ;
    ConnectionListingModel* iListingModel ;
};

#endif
