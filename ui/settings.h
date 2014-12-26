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

#ifndef SETTINGS_DIALOG_H
#define SETTINGS_DIALOG_H

#include <QDialog>
#include "../mcontroller.h"
#include "../ui_settingsDialog.h"

/**
 * @brief class for editing node-wide settings
 *
 */
class SettingsDialog : public QDialog
{
  Q_OBJECT

    public:
  /**
   * Constructor.
   */
  SettingsDialog(QWidget *aParent,
		    MController& aController );
  /** destructor */
  ~SettingsDialog();

private slots:
  void okButtonClicked() ;
  void cancelButtonClicked() ;
signals:
  void error(MController::CAErrorSituation aError,
	     const QString& aExplanation) ;
private:
  Ui_settingsDialog ui ; 
  MController& iController ;
};

#endif
