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

#ifndef PASSWD_DIALOG_H
#define PASSWD_DIALOG_H

#include <QDialog>
#include "../mcontroller.h"
#include "../ui_passwordDialog.h"

class MController ; 

/**
 * @brief class for querying a password from user. 
 *
 * thanks to Jasmin Blanchette and Mark Summerfield
 * who wrote the article at 
 * http://www.informit.com/articles/article.aspx?p=1405224
 * where this code here is more-or-less stolen from.
 */
class PasswdDialog : public QDialog
{
  Q_OBJECT

    public:
  /**
   * Constructor.
   *
   * @param aParent is owner-window of this dialog
   * @param aController application controller reference
   * @param aPrompt text presented to user
   * @param aIsPwdQueryDialog if set to true, queries. If 
   *        false this dialog will change the password
   */
  PasswdDialog(QWidget *aParent,
	       MController& aController,
	       const QString &aPrompt,
	       bool aIsPwdQueryDialog = true );
  ~PasswdDialog();
  private slots:
  void okClicked();
signals:
  void error(MController::CAErrorSituation aError,
	     const QString& aExplanation) ;
 private:
  Ui_PasswordDialog ui ; 
  MController& iController ; 
  QString iPrompt ; 
  bool iIsPwdQueryDialog ;
};

#endif
