/*    -*-C++-*- -*-coding: utf-8-unix;-*-
    Classified Ads is Copyright (c) Antti Jarvinen 2013.

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

#ifndef CLASSIFIED_PROFILE_H
#define CLASSIFIED_PROFILE_H
#include <QString>
#include "../util/hash.h" // for class Hash  
#include <QPixmap>
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
   * @return true if QVariant looked like profile
   */
  bool setFromQVariant(const QVariantMap& aJSonAsQVariant,
		       const MController& aController) ;
  bool fromJSon(const QByteArray &aJSonBytes,
		const MController& aController ) ; /**< parses json into members*/
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
  QPixmap iProfilePicture ; /**< If V. Lenin is too fine for you */
  QList<Hash> iSharedFiles  ; /**< Fingerprints of files shared */ 
  Node* iNodeOfProfile ; /**< physical contact addr of this profile */
} ;
#endif
