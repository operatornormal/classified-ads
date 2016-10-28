/*     -*-C++-*- -*-coding: utf-8-unix;-*-
  Classified Ads is Copyright (c) Antti Järvinen 2015-2016.

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

#ifndef CA_TCLCONSOLE_DIALOG_H
#define CA_TCLCONSOLE_DIALOG_H

#include <QDialog>
#include <QStringListModel>
#include "../mcontroller.h"
#include "../ui_tclConsole.h"

/**
 * @brief class for displaying tcl interpreter output
 */
class TclConsoleDialog : public QDialog {
    Q_OBJECT

public:
    /**
     * Constructor.
     */
    TclConsoleDialog(QWidget *aParent,
                     MController& aController );
    /** destructor */
    ~TclConsoleDialog();


public slots:
    /**
     * Messages displayed in the console come in via this slot
     */
    void consoleOutput(QString aMessage) ;
private slots:
    /**
     * eval-button click is handled in this method
     */
    void evalButtonPressed() ;
signals:
    void error(MController::CAErrorSituation aError,
               const QString& aExplanation) ;
    /**
     * This signal is emitted if user wishes to have TCL evaluated
     */
    void evalScript(QString aScript,QString* aWindowTitle) ;
private:
    Ui_tclConsoleDialog ui ;
    MController& iController ;
    QStringListModel iConsoleOutputText ; /**< holds output text */
};

#endif
