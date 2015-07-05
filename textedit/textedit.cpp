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

#include "textedit.h"

#include <QAction>
#include <QApplication>
#include <QClipboard>
#include <QColorDialog>
#include <QComboBox>
#include <QFontComboBox>
#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
#include <QFontDatabase>
#include <QMenu>
#include <QMenuBar>
#include <QPrintDialog>
#include <QPrinter>
#include <QTextCodec>
#include <QTextEdit>
#include <QToolBar>
#include <QTextCursor>
#include <QTextDocumentWriter>
#include <QTextList>
#include <QtDebug>
#include <QCloseEvent>
#include <QMessageBox>
#include <QPrintPreviewDialog>
#include <QLayout>
#include <QMimeData>
#include "../log.h"
#include "../ui/insertlinkdialog.h"
#include <assert.h>
#if QT_VERSION >= 0x050000
// qt5 has its own mime-type-handling so lets depend on that, easier..
#include <QMimeDatabase>
#else
// with qt4 use libmagic that does the same job
#include <magic.h> // from libmagic
#endif
#ifdef Q_WS_MAC
const QString rsrcPath = ":/images/mac";
#else
const QString rsrcPath = ":/images/win";
#endif

TextEdit::TextEdit(QWidget* aParent,
                   MController* aController,
                   Profile& aSelectedProfile)
    : DialogBase(aParent,aController,aSelectedProfile),
      textEdit(NULL),
      iWidgetForActionsUpper(NULL),
      iWidgetForActionsLower(NULL),
      iDialogMenuBar(NULL) {

}

TextEdit::~TextEdit() {
    delete iDialogMenuBar ;
}

void TextEdit::addAction(QWidget* aAction,bool aLower) {
    if ( aLower ) {
        iWidgetForActionsLower->addWidget(aAction) ;
    } else {
        iWidgetForActionsUpper->addWidget(aAction) ;
    }
}

void TextEdit::addAction(QAction* aAction) {
    tb->addAction(aAction) ;
}

void TextEdit::initializeTextEditor(QTextEdit *aTextEdit,
                                    QLayout *aLayOutForMenu,
                                    QLayout* aWidgetForActionsUpper,
                                    QLayout* aWidgetForActionsLower) {
    //    setToolButtonStyle(Qt::ToolButtonFollowStyle);
    iDialogMenuBar = new QMenuBar() ;
    aLayOutForMenu->setMenuBar(iDialogMenuBar) ;
    iWidgetForActionsUpper = aWidgetForActionsUpper ;
    iWidgetForActionsLower = aWidgetForActionsLower ;
    setupFileActions();
    setupEditActions();
    setupTextActions();

    {
        QMenu *helpMenu = new QMenu(tr("Help"), this);
        menuBar()->addMenu(helpMenu);
        helpMenu->addAction(tr("About &Qt"), qApp, SLOT(aboutQt()));
    }

    textEdit = aTextEdit ;
    connect(textEdit, SIGNAL(currentCharFormatChanged(QTextCharFormat)),
            this, SLOT(currentCharFormatChanged(QTextCharFormat)));
    connect(textEdit, SIGNAL(cursorPositionChanged()),
            this, SLOT(cursorPositionChanged()));

    //    setCentralWidget(textEdit);
    textEdit->setFocus();
    setCurrentFileName(QString());

    fontChanged(textEdit->font());
    colorChanged(textEdit->textColor());
    alignmentChanged(textEdit->alignment());

    connect(textEdit->document(), SIGNAL(modificationChanged(bool)),
            actionSave, SLOT(setEnabled(bool)));
    connect(textEdit->document(), SIGNAL(modificationChanged(bool)),
            this, SLOT(setWindowModified(bool)));
    connect(textEdit->document(), SIGNAL(undoAvailable(bool)),
            actionUndo, SLOT(setEnabled(bool)));
    connect(textEdit->document(), SIGNAL(redoAvailable(bool)),
            actionRedo, SLOT(setEnabled(bool)));

    setWindowModified(textEdit->document()->isModified());
    actionSave->setEnabled(textEdit->document()->isModified());
    actionUndo->setEnabled(textEdit->document()->isUndoAvailable());
    actionRedo->setEnabled(textEdit->document()->isRedoAvailable());

    connect(actionUndo, SIGNAL(triggered()), textEdit, SLOT(undo()));
    connect(actionRedo, SIGNAL(triggered()), textEdit, SLOT(redo()));

    actionCut->setEnabled(false);
    actionCopy->setEnabled(false);

    connect(actionCut, SIGNAL(triggered()), textEdit, SLOT(cut()));
    connect(actionCopy, SIGNAL(triggered()), textEdit, SLOT(copy()));
    connect(actionPaste, SIGNAL(triggered()), textEdit, SLOT(paste()));

    connect(textEdit, SIGNAL(copyAvailable(bool)), actionCut, SLOT(setEnabled(bool)));
    connect(textEdit, SIGNAL(copyAvailable(bool)), actionCopy, SLOT(setEnabled(bool)));

#ifndef QT_NO_CLIPBOARD
    connect(QApplication::clipboard(), SIGNAL(dataChanged()), this, SLOT(clipboardDataChanged()));
#endif

    fileNew();
}

void TextEdit::closeEvent(QCloseEvent *e) {
    e->accept();
}

void TextEdit::setupFileActions() {
    tb = new QToolBar(this);
    tb->setWindowTitle(tr("Edit Actions"));
    iWidgetForActionsUpper->addWidget(tb) ;
    QMenu *menu = new QMenu(tr("&File"), this);
    menuBar()->addMenu(menu);

    QAction *a;

    a = new QAction(QIcon::fromTheme("document-open", QIcon(rsrcPath + "/fileopen.png")),
                    tr("&Open..."), this);
    a->setShortcut(QKeySequence::Open);
    connect(a, SIGNAL(triggered()), this, SLOT(fileOpen()));
    addAction(a);
    menu->addAction(a);

    menu->addSeparator();

    actionSave = a = new QAction(QIcon::fromTheme("document-save", QIcon(rsrcPath + "/filesave.png")),
                                 tr("&Save locally before send"), this);
    a->setShortcut(QKeySequence::Save);
    connect(a, SIGNAL(triggered()), this, SLOT(fileSave()));
    a->setEnabled(false);
    addAction(a);
    menu->addAction(a);

    a = new QAction(tr("Save locally &As..."), this);
    a->setPriority(QAction::LowPriority);
    connect(a, SIGNAL(triggered()), this, SLOT(fileSaveAs()));
    menu->addAction(a);
    menu->addSeparator();

#ifndef QT_NO_PRINTER
    a = new QAction(QIcon::fromTheme("document-print", QIcon(rsrcPath + "/fileprint.png")),
                    tr("&Print..."), this);
    a->setPriority(QAction::LowPriority);
    a->setShortcut(QKeySequence::Print);
    connect(a, SIGNAL(triggered()), this, SLOT(filePrint()));
    addAction(a);
    menu->addAction(a);

    a = new QAction(QIcon::fromTheme("fileprint", QIcon(rsrcPath + "/fileprint.png")),
                    tr("Print Preview..."), this);
    connect(a, SIGNAL(triggered()), this, SLOT(filePrintPreview()));
    menu->addAction(a);

    a = new QAction(QIcon::fromTheme("exportpdf", QIcon(rsrcPath + "/exportpdf.png")),
                    tr("&Export PDF..."), this);
    a->setPriority(QAction::LowPriority);
    a->setShortcut(Qt::CTRL + Qt::Key_D);
    connect(a, SIGNAL(triggered()), this, SLOT(filePrintPdf()));
    addAction(a);
    menu->addAction(a);

    menu->addSeparator();
#endif

}

void TextEdit::setupEditActions() {
    QMenu *menu = new QMenu(tr("&Edit"), this);
    menuBar()->addMenu(menu);

    QAction *a;
    a = actionUndo = new QAction(QIcon::fromTheme("edit-undo", QIcon(rsrcPath + "/editundo.png")),
                                 tr("&Undo"), this);
    a->setShortcut(QKeySequence::Undo);
    addAction(a);
    menu->addAction(a);
    a = actionRedo = new QAction(QIcon::fromTheme("edit-redo", QIcon(rsrcPath + "/editredo.png")),
                                 tr("&Redo"), this);
    a->setPriority(QAction::LowPriority);
    a->setShortcut(QKeySequence::Redo);
    addAction(a);
    menu->addAction(a);
    menu->addSeparator();
    a = actionCut = new QAction(QIcon::fromTheme("edit-cut", QIcon(rsrcPath + "/editcut.png")),
                                tr("Cu&t"), this);
    a->setPriority(QAction::LowPriority);
    a->setShortcut(QKeySequence::Cut);
    addAction(a);
    menu->addAction(a);
    a = actionCopy = new QAction(QIcon::fromTheme("edit-copy", QIcon(rsrcPath + "/editcopy.png")),
                                 tr("&Copy"), this);
    a->setPriority(QAction::LowPriority);
    a->setShortcut(QKeySequence::Copy);
    addAction(a);
    menu->addAction(a);
    a = actionPaste = new QAction(QIcon::fromTheme("edit-paste", QIcon(rsrcPath + "/editpaste.png")),
                                  tr("&Paste"), this);
    a->setPriority(QAction::LowPriority);
    a->setShortcut(QKeySequence::Paste);
    addAction(a);
    menu->addAction(a);
#ifndef QT_NO_CLIPBOARD
    if (const QMimeData *md = QApplication::clipboard()->mimeData())
        actionPaste->setEnabled(md->hasText());
#endif
    menu->addSeparator();
    actionInsertLink = new QAction(tr("Insert link"), this) ;
    connect(actionInsertLink, SIGNAL(triggered()),
            this, SLOT(insertLinkSelected())) ;
    menu->addAction(actionInsertLink) ;
    actionInsertImage = new QAction(tr("Embed image"), this) ;
    connect(actionInsertImage, SIGNAL(triggered()),
            this, SLOT(insertImageSelected())) ;
    menu->addAction(actionInsertImage) ;
}

void TextEdit::setupTextActions() {
    QMenu *menu = new QMenu(tr("F&ormat"), this);
    menuBar()->addMenu(menu);

    actionTextBold = new QAction(QIcon::fromTheme("format-text-bold", QIcon(rsrcPath + "/textbold.png")),
                                 tr("&Bold"), this);
    actionTextBold->setShortcut(Qt::CTRL + Qt::Key_B);
    actionTextBold->setPriority(QAction::LowPriority);
    QFont bold;
    bold.setBold(true);
    actionTextBold->setFont(bold);
    connect(actionTextBold, SIGNAL(triggered()), this, SLOT(textBold()));
    addAction(actionTextBold);
    menu->addAction(actionTextBold);
    actionTextBold->setCheckable(true);

    actionTextItalic = new QAction(QIcon::fromTheme("format-text-italic", QIcon(rsrcPath + "/textitalic.png")),
                                   tr("&Italic"), this);
    actionTextItalic->setPriority(QAction::LowPriority);
    actionTextItalic->setShortcut(Qt::CTRL + Qt::Key_I);
    QFont italic;
    italic.setItalic(true);
    actionTextItalic->setFont(italic);
    connect(actionTextItalic, SIGNAL(triggered()), this, SLOT(textItalic()));
    addAction(actionTextItalic);
    menu->addAction(actionTextItalic);
    actionTextItalic->setCheckable(true);

    actionTextUnderline = new QAction(QIcon::fromTheme("format-text-underline", QIcon(rsrcPath + "/textunder.png")),
                                      tr("&Underline"), this);
    actionTextUnderline->setShortcut(Qt::CTRL + Qt::Key_U);
    actionTextUnderline->setPriority(QAction::LowPriority);
    QFont underline;
    underline.setUnderline(true);
    actionTextUnderline->setFont(underline);
    connect(actionTextUnderline, SIGNAL(triggered()), this, SLOT(textUnderline()));
    addAction(actionTextUnderline);
    menu->addAction(actionTextUnderline);
    actionTextUnderline->setCheckable(true);

    menu->addSeparator();

    QActionGroup *grp = new QActionGroup(this);
    connect(grp, SIGNAL(triggered(QAction*)), this, SLOT(textAlign(QAction*)));

    // Make sure the alignLeft  is always left of the alignRight
    if (QApplication::isLeftToRight()) {
        actionAlignLeft = new QAction(QIcon::fromTheme("format-justify-left", QIcon(rsrcPath + "/textleft.png")),
                                      tr("&Left"), grp);
        actionAlignCenter = new QAction(QIcon::fromTheme("format-justify-center", QIcon(rsrcPath + "/textcenter.png")), tr("C&enter"), grp);
        actionAlignRight = new QAction(QIcon::fromTheme("format-justify-right", QIcon(rsrcPath + "/textright.png")), tr("&Right"), grp);
    } else {
        actionAlignRight = new QAction(QIcon::fromTheme("format-justify-right", QIcon(rsrcPath + "/textright.png")), tr("&Right"), grp);
        actionAlignCenter = new QAction(QIcon::fromTheme("format-justify-center", QIcon(rsrcPath + "/textcenter.png")), tr("C&enter"), grp);
        actionAlignLeft = new QAction(QIcon::fromTheme("format-justify-left", QIcon(rsrcPath + "/textleft.png")), tr("&Left"), grp);
    }
    actionAlignJustify = new QAction(QIcon::fromTheme("format-justify-fill", QIcon(rsrcPath + "/textjustify.png")), tr("&Justify"), grp);

    actionAlignLeft->setShortcut(Qt::CTRL + Qt::Key_L);
    actionAlignLeft->setCheckable(true);
    actionAlignLeft->setPriority(QAction::LowPriority);
    actionAlignCenter->setShortcut(Qt::CTRL + Qt::Key_E);
    actionAlignCenter->setCheckable(true);
    actionAlignCenter->setPriority(QAction::LowPriority);
    actionAlignRight->setShortcut(Qt::CTRL + Qt::Key_R);
    actionAlignRight->setCheckable(true);
    actionAlignRight->setPriority(QAction::LowPriority);
    actionAlignJustify->setShortcut(Qt::CTRL + Qt::Key_J);
    actionAlignJustify->setCheckable(true);
    actionAlignJustify->setPriority(QAction::LowPriority);


    addAction(actionAlignLeft) ;
    addAction(actionAlignCenter) ;
    addAction(actionAlignRight);
    addAction(actionAlignJustify) ;

    menu->addActions(grp->actions());

    menu->addSeparator();

    QPixmap pix(16, 16);
    pix.fill(Qt::black);
    actionTextColor = new QAction(pix, tr("&Color..."), this);
    connect(actionTextColor, SIGNAL(triggered()), this, SLOT(textColor()));
    addAction(actionTextColor);
    menu->addAction(actionTextColor);

    comboStyle = new QComboBox(tb);
    addAction(comboStyle,true);
    comboStyle->addItem(tr("Standard"));
    comboStyle->addItem(tr("Bullet List (Disc)"));
    comboStyle->addItem(tr("Bullet List (Circle)"));
    comboStyle->addItem(tr("Bullet List (Square)"));
    comboStyle->addItem(tr("Ordered List (Decimal)"));
    comboStyle->addItem(tr("Ordered List (Alpha lower)"));
    comboStyle->addItem(tr("Ordered List (Alpha upper)"));
    comboStyle->addItem(tr("Ordered List (Roman lower)"));
    comboStyle->addItem(tr("Ordered List (Roman upper)"));
    connect(comboStyle, SIGNAL(activated(int)),
            this, SLOT(textStyle(int)));

    comboFont = new QFontComboBox(tb);
    addAction(comboFont,true);
    connect(comboFont, SIGNAL(activated(QString)),
            this, SLOT(textFamily(QString)));

    comboSize = new QComboBox(tb);
    comboSize->setObjectName("comboSize");
    addAction(comboSize,true);
    comboSize->setEditable(true);

    QFontDatabase db;
    foreach(int size, db.standardSizes())
    comboSize->addItem(QString::number(size));

    connect(comboSize, SIGNAL(activated(QString)),
            this, SLOT(textSize(QString)));
    comboSize->setCurrentIndex(comboSize->findText(QString::number(QApplication::font()
                               .pointSize())));
}

bool TextEdit::load(const QString &f) {
    if (!QFile::exists(f))
        return false;
    QFile file(f);
    if (!file.open(QFile::ReadOnly))
        return false;

    QByteArray data = file.readAll();
    QTextCodec *codec = Qt::codecForHtml(data);
    QString str = codec->toUnicode(data);
    if (Qt::mightBeRichText(str)) {
        textEdit->setHtml(str);
    } else {
        str = QString::fromLocal8Bit(data);
        textEdit->setPlainText(str);
    }

    setCurrentFileName(f);
    return true;
}

bool TextEdit::maybeSave() {
    if (!textEdit->document()->isModified())
        return true;
    if (fileName.startsWith(QLatin1String(":/")))
        return true;
    QMessageBox::StandardButton ret;
    ret = QMessageBox::warning(this, tr("Application"),
                               tr("The document has been modified.\n"
                                  "Do you want to save your changes?"),
                               QMessageBox::Save | QMessageBox::Discard
                               | QMessageBox::Cancel);
    if (ret == QMessageBox::Save)
        return fileSave();
    else if (ret == QMessageBox::Cancel)
        return false;
    return true;
}

void TextEdit::setCurrentFileName(const QString &fileName) {
    this->fileName = fileName;
    textEdit->document()->setModified(false);
}

void TextEdit::fileNew() {
    if (maybeSave()) {
        textEdit->clear();
        setCurrentFileName(QString());
    }
}

void TextEdit::fileOpen() {
    QString fn = QFileDialog::getOpenFileName(this, tr("Open File..."),
                 QString(), tr("HTML-Files (*.htm *.html);;All Files (*)"));
    if (!fn.isEmpty())
        load(fn);
}

bool TextEdit::fileSave() {
    if (fileName.isEmpty())
        return fileSaveAs();

    QTextDocumentWriter writer(fileName);
    bool success = writer.write(textEdit->document());
    if (success)
        textEdit->document()->setModified(false);
    return success;
}

bool TextEdit::fileSaveAs() {
    QString fn = QFileDialog::getSaveFileName(this, tr("Save as..."),
                 QString(), tr("ODF files (*.odt);;HTML-Files (*.htm *.html);;All Files (*)"));
    if (fn.isEmpty())
        return false;
    if (! (fn.endsWith(".odt", Qt::CaseInsensitive) || fn.endsWith(".htm", Qt::CaseInsensitive) || fn.endsWith(".html", Qt::CaseInsensitive)) )
        fn += ".odt"; // default
    setCurrentFileName(fn);
    return fileSave();
}

void TextEdit::filePrint() {
#ifndef QT_NO_PRINTER
    QPrinter printer(QPrinter::HighResolution);
    QPrintDialog *dlg = new QPrintDialog(&printer, this);
    if (textEdit->textCursor().hasSelection())
        dlg->addEnabledOption(QAbstractPrintDialog::PrintSelection);
    dlg->setWindowTitle(tr("Print Document"));
    if (dlg->exec() == QDialog::Accepted) {
        textEdit->print(&printer);
    }
    delete dlg;
#endif
}

void TextEdit::filePrintPreview() {
#ifndef QT_NO_PRINTER
    QPrinter printer(QPrinter::HighResolution);
    QPrintPreviewDialog preview(&printer, this);
    connect(&preview, SIGNAL(paintRequested(QPrinter*)), SLOT(printPreview(QPrinter*)));
    preview.exec();
#endif
}

void TextEdit::insertLinkSelected() {
    QLOG_STR("TextEdit::insertLinkSelected") ;

    InsertLinkDialog *link_dialog =
        new InsertLinkDialog(this, iController) ;
    connect(link_dialog,
            SIGNAL(  error(MController::CAErrorSituation,
                           const QString&) ),
            iController,
            SLOT(handleError(MController::CAErrorSituation,
                             const QString&)),
            Qt::QueuedConnection ) ;
    connect(this,
            SIGNAL( accepted() ),
            link_dialog,
            SLOT(deleteLater()),
            Qt::QueuedConnection ) ;
    connect(this,
            SIGNAL( rejected() ),
            link_dialog,
            SLOT(deleteLater()),
            Qt::QueuedConnection ) ;

    // don't connect as queued, it transfers pointers
    assert(
        connect ( link_dialog,
                  SIGNAL(  linkReady(const QString& ,
                                     const QString& ) ),
                  this,
                  SLOT(    linkReady(const QString& ,
                                     const QString& ) ) ) );

    link_dialog->show() ; // the dialog will delete self
}
//
// motivation here: we want to insert html snippet that
// looks like this:
// <img alt="Embedded Image" src="data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAADIA..." />
//
void TextEdit::insertImageSelected() {
    QLOG_STR("TextEdit::insertImageSelected") ;
    QString fileName = QFileDialog::getOpenFileName(this, tr("Open File"),
                       "",
                       tr("Files (*.*)"));
    bool imageIsOk (false) ;
    if ( fileName.length() > 0 ) {
        QPixmap pic ;
        if ( ! pic.load(fileName) ) {
            QMessageBox::about(this,tr("Error"),
                               tr("Can't load image"));
            return ;
        } else {
            imageIsOk = true ;
        }
    }
    if ( imageIsOk ) {
        QString mimeType ;
#if QT_VERSION >= 0x050000
        QMimeDatabase db ;
        QMimeType detectedType ( db.mimeTypeForFile(fileName) ) ;
        if ( detectedType.isValid() ) {
            mimeType = detectedType.name() ;
        }
#else
        const char *fileNameCStyleHeap (fileName.toUtf8().constData()) ;

        char fileNameCStyle[1024]  ;
        if ( strlen(fileNameCStyleHeap) < 1024 ) {
            strncpy(fileNameCStyle, fileNameCStyleHeap, 1023) ;
            // code stolen from http://vivithemage.co.uk/blog/?p=105 so thanks Vivi.
            magic_t magic_cookie;
            /*MAGIC_MIME tells magic to return a mime of the file, but you can specify different things*/
            magic_cookie = magic_open(MAGIC_MIME);
            if (magic_cookie == NULL) {
                QLOG_STR("unable to initialize magic library");
                return ;
            }
            if (magic_load(magic_cookie, NULL) != 0) {
                LOG_STR2("cannot load magic database - %s", magic_error(magic_cookie));
                magic_close(magic_cookie);
                return ;
            }
            LOG_STR2("Filename for magic = >%s<", fileNameCStyle);
            const char *magic_full( magic_file(magic_cookie, fileNameCStyle) ) ;
            if ( magic_full == NULL ) {
                LOG_STR2("Magic_file error - %s", magic_error(magic_cookie));
            } else {
                mimeType = QString::fromUtf8(magic_full) ;
            }
            magic_close(magic_cookie);
        }
#endif

        if ( mimeType.length() > 0 ) {
            int indexOfSemicolon = mimeType.indexOf(';') ;
            if ( indexOfSemicolon > 1 ) {
                QLOG_STR("Pruning semicolon from mime type " + mimeType) ;
                mimeType = mimeType.left(indexOfSemicolon) ;
            }
            QLOG_STR("mime type " + mimeType) ;
            QFile f ( fileName ) ;
            if ( f.open(QIODevice::ReadOnly) ) {
                if ( f.size() >  KMaxProtocolItemSize  ) {
                    emit error ( MController::FileOperationError, tr("File way too big")) ;
                } else {
                    QByteArray content ( f.read(f.size() ) ) ;
                    QString snippet =
                        "<img alt=\""+QFileInfo(f).fileName()+"\" src=\"data:"+mimeType+";base64,"+content.toBase64()+"\"/>" ;
                    content.clear() ;
                    if ( ( (quint32)(snippet.length()) +
                            (quint32)(textEdit->toHtml().length()) +
                            10240 )
                            >  KMaxProtocolItemSize  ) {
                        emit error ( MController::FileOperationError, tr("File way too big")) ;
                    } else {
                        textEdit->insertHtml(snippet) ;
                    }
                }
            }
        }
    }
}

void TextEdit::printPreview(QPrinter *printer) {
#ifdef QT_NO_PRINTER
    Q_UNUSED(printer);
#else
    textEdit->print(printer);
#endif
}


void TextEdit::filePrintPdf() {
#ifndef QT_NO_PRINTER
    //! [0]
    QString fileName = QFileDialog::getSaveFileName(this, "Export PDF",
                       QString(), "*.pdf");
    if (!fileName.isEmpty()) {
        if (QFileInfo(fileName).suffix().isEmpty())
            fileName.append(".pdf");
        QPrinter printer(QPrinter::HighResolution);
        printer.setOutputFormat(QPrinter::PdfFormat);
        printer.setOutputFileName(fileName);
        textEdit->document()->print(&printer);
    }
    //! [0]
#endif
}

void TextEdit::textBold() {
    QTextCharFormat fmt;
    fmt.setFontWeight(actionTextBold->isChecked() ? QFont::Bold : QFont::Normal);
    mergeFormatOnWordOrSelection(fmt);
}

void TextEdit::textUnderline() {
    QTextCharFormat fmt;
    fmt.setFontUnderline(actionTextUnderline->isChecked());
    mergeFormatOnWordOrSelection(fmt);
}

void TextEdit::textItalic() {
    QTextCharFormat fmt;
    fmt.setFontItalic(actionTextItalic->isChecked());
    mergeFormatOnWordOrSelection(fmt);
}

void TextEdit::textFamily(const QString &f) {
    QTextCharFormat fmt;
    fmt.setFontFamily(f);
    mergeFormatOnWordOrSelection(fmt);
}

void TextEdit::textSize(const QString &p) {
    qreal pointSize = p.toFloat();
    if (p.toFloat() > 0) {
        QTextCharFormat fmt;
        fmt.setFontPointSize(pointSize);
        mergeFormatOnWordOrSelection(fmt);
    }
}

void TextEdit::textStyle(int styleIndex) {
    QTextCursor cursor = textEdit->textCursor();

    if (styleIndex != 0) {
        QTextListFormat::Style style = QTextListFormat::ListDisc;

        switch (styleIndex) {
        default:
        case 1:
            style = QTextListFormat::ListDisc;
            break;
        case 2:
            style = QTextListFormat::ListCircle;
            break;
        case 3:
            style = QTextListFormat::ListSquare;
            break;
        case 4:
            style = QTextListFormat::ListDecimal;
            break;
        case 5:
            style = QTextListFormat::ListLowerAlpha;
            break;
        case 6:
            style = QTextListFormat::ListUpperAlpha;
            break;
        case 7:
            style = QTextListFormat::ListLowerRoman;
            break;
        case 8:
            style = QTextListFormat::ListUpperRoman;
            break;
        }

        cursor.beginEditBlock();

        QTextBlockFormat blockFmt = cursor.blockFormat();

        QTextListFormat listFmt;

        if (cursor.currentList()) {
            listFmt = cursor.currentList()->format();
        } else {
            listFmt.setIndent(blockFmt.indent() + 1);
            blockFmt.setIndent(0);
            cursor.setBlockFormat(blockFmt);
        }

        listFmt.setStyle(style);

        cursor.createList(listFmt);

        cursor.endEditBlock();
    } else {
        QTextBlockFormat bfmt;
        bfmt.setObjectIndex(-1);
        cursor.mergeBlockFormat(bfmt);
    }
    textEdit->setFocus() ;
}

void TextEdit::textColor() {
    QColor col = QColorDialog::getColor(textEdit->textColor(), this);
    if (!col.isValid())
        return;
    QTextCharFormat fmt;
    fmt.setForeground(col);
    mergeFormatOnWordOrSelection(fmt);
    colorChanged(col);
    textEdit->setFocus() ;
}

void TextEdit::textAlign(QAction *a) {
    if (a == actionAlignLeft)
        textEdit->setAlignment(Qt::AlignLeft | Qt::AlignAbsolute);
    else if (a == actionAlignCenter)
        textEdit->setAlignment(Qt::AlignHCenter);
    else if (a == actionAlignRight)
        textEdit->setAlignment(Qt::AlignRight | Qt::AlignAbsolute);
    else if (a == actionAlignJustify)
        textEdit->setAlignment(Qt::AlignJustify);
    textEdit->setFocus() ;
}

void TextEdit::currentCharFormatChanged(const QTextCharFormat &format) {
    fontChanged(format.font());
    colorChanged(format.foreground().color());
    textEdit->setFocus();
}

void TextEdit::cursorPositionChanged() {
    alignmentChanged(textEdit->alignment());
}

void TextEdit::clipboardDataChanged() {
#ifndef QT_NO_CLIPBOARD
    if (const QMimeData *md = QApplication::clipboard()->mimeData())
        actionPaste->setEnabled(md->hasText());
#endif
}

void TextEdit::mergeFormatOnWordOrSelection(const QTextCharFormat &format) {
    QTextCursor cursor = textEdit->textCursor();
    if (!cursor.hasSelection())
        cursor.select(QTextCursor::WordUnderCursor);
    cursor.mergeCharFormat(format);
    textEdit->mergeCurrentCharFormat(format);
}

void TextEdit::fontChanged(const QFont &f) {
    comboFont->setCurrentIndex(comboFont->findText(QFontInfo(f).family()));
    comboSize->setCurrentIndex(comboSize->findText(QString::number(f.pointSize())));
    actionTextBold->setChecked(f.bold());
    actionTextItalic->setChecked(f.italic());
    actionTextUnderline->setChecked(f.underline());
}

void TextEdit::colorChanged(const QColor &c) {
    QPixmap pix(16, 16);
    pix.fill(c);
    actionTextColor->setIcon(pix);
}

void TextEdit::alignmentChanged(Qt::Alignment a) {
    if (a & Qt::AlignLeft) {
        actionAlignLeft->setChecked(true);
    } else if (a & Qt::AlignHCenter) {
        actionAlignCenter->setChecked(true);
    } else if (a & Qt::AlignRight) {
        actionAlignRight->setChecked(true);
    } else if (a & Qt::AlignJustify) {
        actionAlignJustify->setChecked(true);
    }
}

void TextEdit::linkReady(const QString& aLinkAddress,
                         const QString& aLinkLabel) {
    QLOG_STR("link ready " + aLinkAddress) ;
    QLOG_STR("link label " + aLinkLabel) ;
    textEdit->insertHtml("<a href='"+aLinkAddress+"'>"+aLinkLabel+"</a> ") ;
}

