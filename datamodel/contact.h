/*    -*-C++-*- -*-coding: utf-8-unix;-*-
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

#ifndef CA_CONTACT_H
#define CA_CONTACT_H
#include <QString>
#include "../util/hash.h" // for class Hash  

class MController ;
class Node ;

/**
 * @brief Carrier-class for a profile that is in contacts-list
 *
 * Instances of this class are stored and retrieved by controller
 * directly. Also note @ref ContactsListingModel.
 */
class Contact {
public:
    Contact() ; /**< constructor */
    ~Contact() ; /**< destructor */
    /**
     * method for string that is shown to user about the message.
     *
     * @return string to display to user
     */
    QString displayName() const ;
    QByteArray asJSon(const MController& aController) const ; /**< returns msg data as JSon stream */
    bool fromJSon(const QByteArray &aJSonBytes,
                  const MController& aController ) ; /**< parses json into members*/
    /**
     * Method for getting contact as JSon / QVariant
     */
    QVariant asQVariant() const ;
    /**
     * reverse of @ref asQVariant()
     * @return node or NULL if
     */
    static Contact fromQVariant(const QVariantMap& aJSonAsQVariant) ;
    /** hash of the profile that is presented here */
    Hash iFingerPrint ;
    QString iNickName ;  /**< nickname/display-name of contact ; may empty if private profile */
    bool iIsTrusted ; /**< In addition to being in contacts-list, this user is also trusted */
} ;
#endif
