/*     -*-C++-*- -*-coding: utf-8-unix;-*-
  Classified Ads is Copyright (c) Antti Järvinen 2015-2017.

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

#ifndef CA_TCLPROGRAMLISTING_DIALOG_H
#define CA_TCLPROGRAMLISTING_DIALOG_H

#include <QDialog>
#include <QStandardItemModel>
#include "../mcontroller.h"
#include "../ui_tclPrograms.h"

class QAbstractButton ;

/**
 * @brief class for displaying listing of stored TCL scripts
 */
class TclProgramsDialog : public QDialog {
    Q_OBJECT

public:
    /**
     * Constructor.
     */
    TclProgramsDialog(QWidget *aParent,
                      MController& aController );
    /** destructor */
    ~TclProgramsDialog();
public slots:
    /**
     * used to communicate tcl program start event so dialog
     * can adjust accordingly
     */
    void tclProgramStarted() ;
    /**
     * used to communicate tcl program stop event so dialog
     * can adjust accordingly
     */
    void tclProgramStopped() ;
private slots:
    /**
     * eval-button click is handled in this method
     */
    void evalButtonPressed() ;
    /**
     * save-button click is handled in this method
     */
    void saveButtonPressed() ;
    /**
     * discard-button click is handled in this method
     */
    void discardButtonPressed() ;
    /**
     * dialog buttonbox click
     */
    void dialogButtonClicked(QAbstractButton *aButton) ;
    /**
     * slot for signaling focus changes on program list
     */
    void programInListActivated(const QModelIndex & aIndex) ;
    /**
     * Slot called when user edits code in editor
     */
    void editorModificationChanged ( bool aChanged );
signals:
    void error(MController::CAErrorSituation aError,
               const QString& aExplanation) ;
    /**
     * This signal is emitted if user wishes to have TCL evaluated
     */
    void evalScript(QString aScript, QString* aWindowTitle) ;
private:
    Ui_tclProgramsDialog ui ;
    MController& iController ;
    QStandardItemModel iProgramListingModel ; /**< holds display-list of progs */
    QAbstractButton* iEvalButton ;
    QAbstractButton* iDeleteButton ; /**< button for deleting a script */
    QString iEvalButtonStartText ;/**< button text when program not running */
    QString iEvalButtonStopText ; /**< button text when program running */
    QString iNameOfCurrentProgram ;
    Hash iFingerPrintOfCurrentProgram ;
    bool iIsProgramSaved ; /**< true if program is saved to local storage */
    bool iIsProgramRunning ; /**< true if program is being interpreted */
};

#endif
