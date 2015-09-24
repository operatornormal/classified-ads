/*                                      -*-C++-*-
    Classified Ads is Copyright (c) Antti Jarvinen 2013.

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
#include <QTranslator>

#ifndef CATRANSLATOR_H
#define CATRANSLATOR_H

/**
 * @brief Class for translating strings
 *
 * Class for providing translations. In practice this wraps gnu gettext
 * but may have also other sources of translations. Note that Qt allows
 * multiple translators to be in cascade.
 */
class CATranslator : public QTranslator {
    Q_OBJECT
public:
    /**
     * Constructor.
     *
     * Has side-effect of initializing gnu gettext library inside.
     * This same class may contain gnu gettext .po-file that is
     * loaded here at constructor. It is still possible to use
     * the @ref QTranslator::load method to load Qt-format
     * translation and when @ref CATranslator::translate is called,
     * this class will first search for .po file contents and if
     * no match is made, then it will return the translation from
     * possibly loaded Qt translation file.
     */
    CATranslator(QObject* aParent = NULL) ;
    /**
     * Destructor
     */
    ~CATranslator() ;
    /**
     * Actual translator method. This is used to return actual strings
     *
     * @param aContext string that tells in which context the aSourceText
     *                appears in.
     * @param aSourceText the actual string to be translated.
     * @param aDisambiguation if sourceText appears multiple times
     *                       in same context, this may be used to
     *                       further refine the translation.
     * @param aPluralForm Maybe used to select for singular/plural form
     *          of the translation e.g. "message received" or
     *          "messages received"
     * @return Translation. If translation was not available, then
     *                      returns aSourceText as it is.
     */
    virtual QString translate(const char *aContext, 
                              const char *aSourceText,
                              const char *aDisambiguation = 0
#if QT_VERSION >= 0x050000
                              , int aPluralForm = -1
#endif
                             ) const Q_DECL_OVERRIDE;
    virtual bool isEmpty() const;
} ;
#endif /* JSONWRAPPER_H */
