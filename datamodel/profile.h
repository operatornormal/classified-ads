/*    -*-C++-*- -*-coding: utf-8-unix;-*-
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

#ifndef CLASSIFIED_PROFILE_H
#define CLASSIFIED_PROFILE_H
#include <QString>
#include "../util/hash.h" // for class Hash  
#include <QImage>
#include <QVariant> // actually for qvariantmap

class MController ;
class Node ;

/**
 * @brief Carrier-class for user-profile data
 *
 * Instances of this class are stored and retrieved using
 * @ref ProfileModel.
 */
class Profile : public QObject {
    Q_OBJECT

public:
    Profile(const Hash& aHash) ; /**< constructor */
    ~Profile() ; /**< destructor */
    /**
     * method for string that is shown to user about the profile.
     * displays nickname or real name etc. or hash fingerprint
     * if no other data avail. the string returned by this method
     * must be no longer than 160 bytes in utf-8. some chinese
     * glyphs take up to 4 bytes so lets here limit max-len to
     * 40 characters.
     * @return string to display to user
     */
    QString displayName() const ;
    QByteArray asJSon(const MController& aController) const ; /**< returns profile data as JSon stream */
    /**
     * Method for getting profile as JSon / QVariant
     */
    QVariant asQVariant(const MController& aController) const ;
    /**
     * reverse of @ref asQVariant()
     * @param aJSonAsQVariant is qvariant supposedly containing the profile data
     * @param aController application controller
     * @param aOmitImage if set to true, possible image will not be loaded from
     *        data ; in cases where we know that we'll need only subset of
     *        information and no image, we can skip this costly operation
     * @return true if QVariant looked like profile
     */
    bool setFromQVariant(const QVariantMap& aJSonAsQVariant,
                         const MController& aController,
                         bool aOmitImage = false) ;
    /**
     * parses json into members
     * @param aJSonBytes is json text supposedly containing the profile data
     * @param aController application controller
     * @param aOmitImage if set to true, possible image will not be loaded from
     *        data ; in cases where we know that we'll need only subset of
     *        information and no image, we can skip this costly operation
     */
    bool fromJSon(const QByteArray &aJSonBytes,
                  const MController& aController,
                  bool aOmitImage = false ) ;
    const Hash iFingerPrint ; /**< profile encryption key fingerprint */
    QString iNickName ;  /**< nickname selected by user */
    QString iGreetingText ; /**< short hello-world by user */
    QString iFirstName ; /**< given name */
    QString iFamilyName ; /**< family name */
    QString iCityCountry ; /**< location of residence */
    QString iBTCAddress ; /**< payment addr */
    QString iStateOfTheWorld ; /**< State of the world as explained by user.. */
    bool iIsPrivate ;  /**< if set to true, profile is published encrypted */
    quint32 iTimeOfPublish ; /**< seconds since 1-jan-1970 */
    /**
     * in case of private profile, list of reader encryption
     * key fingerprints
     */
    QList<Hash> iProfileReaders  ;
    QImage iProfilePicture ; /**< If V. Lenin is too fine for you */
    QList<Hash> iSharedFiles  ; /**< Fingerprints of files shared */
    Node* iNodeOfProfile ; /**< physical contact addr of this profile */
    QList<Hash> iTrustList  ; /**< Fingerprints of trusted profiles */
} ;
#endif
