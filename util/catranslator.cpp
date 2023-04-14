/*  -*-C++-*- -*-coding: utf-8-unix;-*-
  Classified Ads is Copyright (c) Antti Jarvinen 2015-2022.

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



#include "catranslator.h"
#include "../log.h"
#include <libintl.h>
#include <locale.h>
#ifdef WIN32
#include <QLocale>
#include <QCoreApplication>
#include <QDir>
#endif
/**
 * Translation domain for gnu gettext library
 */
static const char* KProgramName ( "classified-ads"  ) ;
/**
 * This is magick text between gnu gettext context and the string.
 * The | is actuallu Qt magick, for some reason Qt wants to have |
 * at the end of context strings after they come out from lconvert
 * utility but | is missing when QApplication calls ::translate
 * method of this class -> thus add the | here. \004 in turn
 * is gnu gettext magick.
 */
static const char* KGetTextContextGlue ( "|\004" ) ;

CATranslator::CATranslator(QObject* aParent) :
    QTranslator(aParent) {
#ifdef WIN32
    // in win32 user typically has no locale set in environment variables
    // in way that gnu gettext would recognice. offer a bit help:
    QString langVariableName ( "LANGUAGE=" ) ;
    QLocale systemLocale (  QLocale::system() ) ;
    const QString languageName ( systemLocale.name() ) ;
    putenv((langVariableName + languageName).toUtf8().constData()) ;
    setlocale(LC_MESSAGES,languageName.toUtf8().constData());
    // in win32, assume the translation files reside under
    // same directory as the binary
    QString applicationDirectory ( QCoreApplication::applicationDirPath() ) ;
    applicationDirectory.append(QDir::separator()) ;
    bindtextdomain(KProgramName, applicationDirectory.toLocal8Bit().data());
#else
    setlocale(LC_ALL,"");
    // in unix-like systems the installer puts files here:
    bindtextdomain(KProgramName,"/usr/share/locale");
#endif
    textdomain(KProgramName);
    bind_textdomain_codeset(KProgramName, "utf-8") ;
}

CATranslator::~CATranslator() {

}


QString CATranslator::translate ( const char * aContext,
                                  const char * aSourceText,
                                  const char * aDisambiguation
#if QT_VERSION >= 0x050000
                                  ,int aPluralForm
#endif
                                ) const {
    char *contextAndSourceText ((char *)malloc(strlen(aContext) +
                                               strlen(aSourceText) +
                                               5)) ;
    if ( contextAndSourceText == NULL ) {
        // uh, oh, malloc failure
        return QString::fromUtf8(aSourceText) ;
    }
    strcpy(contextAndSourceText, aContext) ;
    strcat(contextAndSourceText, KGetTextContextGlue) ;
    strcat(contextAndSourceText, aSourceText) ;
    char *proposed_string(
        dgettext(KProgramName,
                 contextAndSourceText)
    );
    if ( strcmp(proposed_string, contextAndSourceText) == 0 ) {
        // no match, maybe qt platform string
        free( contextAndSourceText );
        if ( QTranslator::isEmpty() ) {
            // we have no strings in parent class, e.g. native qt
            // format translation strings: just return the source
            // text
            return QString::fromUtf8(aSourceText) ;
        } else {
            // we have qt native format translations available, so try there:
            return QTranslator::translate(aContext,
                                          aSourceText,
                                          aDisambiguation
#if QT_VERSION >= 0x050000
                                          ,aPluralForm
#endif
                                         ) ;
        }
    } else {
        // yes, a match
        QString retval( QString::fromUtf8(proposed_string) ) ;
        free(contextAndSourceText) ;
        return retval ;
    }
}

bool CATranslator::isEmpty() const {
    return false ;
}
