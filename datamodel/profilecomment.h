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

#ifndef CLASSIFIED_PROFILE_COMMENT_H
#define CLASSIFIED_PROFILE_COMMENT_H
#include <QString>
#include "../util/hash.h" // for class Hash  
#include <QPixmap>
#include <QVariant> // actually for qvariantmap
#include "../net/protocol.h" // for ProtocolItemType 
#include "../mcontroller.h" // for KNullHash

/**
 * @brief Carrier-class for comments of a profile
 *
 * Profile-comment data-item behaves much in same way as profile.
 * These are either public or encrypted to profile readers,
 * they might follow the same publish/retrieval procedures as
 * profiles do.
 */
class ProfileComment  {
public:
  /**
   * Constructor. Fingerprint must be given, this is normal when
   * fetching a comment from storage. When publishing a new comment,
   * the fingerprint naturally is not known in advance, in which
   * case KNullHash may be used and real fingerprint can be then
   * updated during publish-process
   */
  ProfileComment(const Hash& aHash = KNullHash ) ; /**< constructor */
  ~ProfileComment() ; /**< destructor */

  QByteArray asJSon(const MController& aController) const ; /**< returns profile comment data as JSon stream */
  bool fromJSon(const QByteArray &aJSonBytes,
		const MController& aController ) ; /**< parses json into members*/
  /**
   * Method for getting profile comment as JSon / QVariant
   */
  QVariant asQVariant(const MController& aController) const ;
  /**
   * reverse of @ref asQVariant()
   * @return true if QVariant looked like profile
   */
  bool setFromQVariant(const QVariantMap& aJSonAsQVariant,
		       const MController& aController) ;

  // data is also public
  Hash iFingerPrint ; /**< fingerprint of the comment */
  Hash iProfileFingerPrint ; /**< fingerprint of the profile commented */
  Hash iCommentorHash ; /**< fingerprint of profile that posted the comment */
  QList<Hash> iAttachedFiles ; 
  QString iCommentText ; 
  QString iSubject ;
  bool iIsPrivate ;  /**< if set to true, profile, and this comment too, 
			is published encrypted */
  quint32 iTimeOfPublish ; /**< seconds since 1-jan-1970 */
  QByteArray iKeyOfCommentor ; /**< public key of profile that sent the comment */
  QString iCommentorNickName ; /**< if commentor is public profile, its nick */
  Hash iReferences ; /**< if comment is reply to another comment, this 
			is the hash of the commented comment */
  ProtocolItemType iTypeOfObjectReferenced ; /**< if comment reference
						is CA, or profile, 
						or another comment */
} ;
#endif
