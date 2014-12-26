/****************************************************************************
**
** Copyright (C) 2012 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (qt-info@nokia.com)
**
** This file is part of the demonstration applications of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** GNU Lesser General Public License Usage
** This file may be used under the terms of the GNU Lesser General Public
** License version 2.1 as published by the Free Software Foundation and
** appearing in the file LICENSE.LGPL included in the packaging of this
** file. Please review the following information to ensure the GNU Lesser
** General Public License version 2.1 requirements will be met:
** http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Nokia gives you certain additional
** rights. These rights are described in the Nokia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU General
** Public License version 3.0 as published by the Free Software Foundation
** and appearing in the file LICENSE.GPL included in the packaging of this
** file. Please review the following information to ensure the GNU General
** Public License version 3.0 requirements will be met:
** http://www.gnu.org/copyleft/gpl.html.
**
** Other Usage
** Alternatively, this file may be used in accordance with the terms and
** conditions contained in a signed written agreement between you and Nokia.
**
**
**
**
**
** $QT_END_LICENSE$
**
****************************************************************************/

#ifndef TEXTEDIT_H
#define TEXTEDIT_H


#include <QMap>
#include <QPointer>
#include <QShortcut>
#include "../ui/dialogbase.h"

QT_FORWARD_DECLARE_CLASS(QAction)
QT_FORWARD_DECLARE_CLASS(QComboBox)
QT_FORWARD_DECLARE_CLASS(QFontComboBox)
QT_FORWARD_DECLARE_CLASS(QTextEdit)
QT_FORWARD_DECLARE_CLASS(QTextCharFormat)
QT_FORWARD_DECLARE_CLASS(QMenu)

class QLayout ; 
class QWidget ; 
class QMenuBar ; 
class QPrinter ; 
class QToolBar ;

/**
 * @brief text editor class. 
 * 
 * This is inteneded for inheritance for dialogs that allow editing
 * of content. Implements a very nice rich text editor. 
 */
class TextEdit : public DialogBase
{
    Q_OBJECT

public:
  /**
   * Constructor. Does very basic init only, method 
   * @ref TextEdit::initializeTextEditor must be called
   * before this class is functional
   */
  TextEdit(QWidget* aParent,
	   MController* aController,
	   const Profile& aSelectedProfile);

  /** destructor */
  ~TextEdit() ; 

    /**
     * Actual initialization of the editor. 
     * 
     * @param aTextEdit pointer to actual editing area, allocated
     *        by inheriting class that has UI-layout. Inheriting class
     *        remains responsible to delete the editor after use.
     */
  void initializeTextEditor(QTextEdit *aTextEdit,
			    QLayout *aLayOutForMenu,
			    QLayout* aWidgetForActionsUpper,
			    QLayout* aWidgetForActionsLower) ; 
protected:
    virtual void closeEvent(QCloseEvent *e);

private:
    void setupFileActions();
    void setupEditActions();
    void setupTextActions();
    bool load(const QString &f);
    bool maybeSave();
    void setCurrentFileName(const QString &fileName);

private slots:
    void fileNew();
    void fileOpen();
    bool fileSave();
    bool fileSaveAs();
    void filePrint();
    void filePrintPreview();
    void filePrintPdf();

    void textBold();
    void textUnderline();
    void textItalic();
    void textFamily(const QString &f);
    void textSize(const QString &p);
    void textStyle(int styleIndex);
    void textColor();
    void textAlign(QAction *a);

    void currentCharFormatChanged(const QTextCharFormat &format);
    void cursorPositionChanged();

    void clipboardDataChanged();
    void printPreview(QPrinter *);

private:
    void mergeFormatOnWordOrSelection(const QTextCharFormat &format);
    void fontChanged(const QFont &f);
    void colorChanged(const QColor &c);
    void alignmentChanged(Qt::Alignment a);
    /** method for adding editing action to designated area */
    void addAction(QAction* aAction) ; 
    void addAction(QWidget* aAction,bool aLower=true) ; 
    /** returns dialog menu bar */
    QMenuBar* menuBar() { return iDialogMenuBar ; } ; 
protected: // variables
    QTextEdit *textEdit; /**< holds the text being edited */
private: // variables
    QAction *actionSave,
        *actionTextBold,
        *actionTextUnderline,
        *actionTextItalic,
        *actionTextColor,
        *actionAlignLeft,
        *actionAlignCenter,
        *actionAlignRight,
        *actionAlignJustify,
        *actionUndo,
        *actionRedo,
        *actionCut,
        *actionCopy,
        *actionPaste;

    QComboBox *comboStyle;
    QFontComboBox *comboFont;
    QComboBox *comboSize;

    QToolBar *tb;
    QString fileName;
    QLayout* iWidgetForActionsUpper ; 
    QLayout* iWidgetForActionsLower ; 
    QMenuBar* iDialogMenuBar ; 
};

#endif
