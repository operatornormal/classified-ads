/*     -*-C++-*- -*-coding: utf-8-unix;-*-
       Classified Ads is Copyright (c) Antti Järvinen 2013.

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

#include "profilecomment.h"
#include "../util/hash.h"
#include "../log.h"
#ifdef WIN32
#include <QJson/Parser>
#include <QJson/Serializer>
#else
#include <qjson/parser.h>
#include <qjson/serializer.h>
#endif
#include <QVariantMap>
#include "../mcontroller.h"
#include "model.h"
#include "contentencryptionmodel.h"

static const char *KCommentJSonProfileFPElement = "commentedProfile" ;
static const char *KCommentJSonTimeElement = "timeOfPublish" ;
static const char *KCommentJSonProfileCommentVersion = "profileVersion" ;
static const char *KCommentJSonEncyptionKey = "key" ;
static const char *KCommentJSonAttachedFiles = "attachedPhiles" ;
static const char *KCommentJSonCommentText = "text" ;
static const char *KCommentJSonCommentSubject = "subject" ;
static const char *KCommentJSonCommentorNick = "nickName" ;
static const char *KCommentJSonReferences = "ref" ;
static const char *KCommentJSonReferencedType = "refType" ;
static const int KJSONProfileCommentVersionValue = 1 ; /**< if our format should change? */

ProfileComment::ProfileComment(const Hash& aHash) :
  iFingerPrint(aHash),
  iIsPrivate(false), // initially all profiles are public ; right? 
  iTimeOfPublish(0) {
  LOG_STR("ProfileComment::ProfileComment()") ;
}

ProfileComment::~ProfileComment() {
  LOG_STR("ProfileComment::~ProfileComment()") ;
}

QByteArray ProfileComment::asJSon(const MController& aController) const {

  // may then be serialized in libqjson-0.7 and 0.8
  QJson::Serializer serializer;
  QByteArray retval ( serializer.serialize(asQVariant(aController)) ) ;
  LOG_STR2("profilecomment %s", qPrintable(QString(retval))) ; 
  return retval ;
}

QVariant ProfileComment::asQVariant(const MController& /* aController */) const 
{
  // first have a map ; that is the top-level JSon-object
  QMap<QString,QVariant> m ;
  if ( iProfileFingerPrint != KNullHash ) {
    m.insert(KCommentJSonProfileFPElement , iProfileFingerPrint.toString()) ;
  }
  if ( iCommentText.length() > 0 ) {
    m.insert(KCommentJSonCommentText , iCommentText.toUtf8()) ;
  }
  if ( iSubject.length() > 0 ) {
    m.insert(KCommentJSonCommentSubject , iSubject.toUtf8()) ;
  }
  m.insert(KCommentJSonTimeElement , iTimeOfPublish) ;
  m.insert( KCommentJSonProfileCommentVersion ,KJSONProfileCommentVersionValue) ;
  if ( iKeyOfCommentor.length() > 0 ) {
    m.insert(KCommentJSonEncyptionKey , iKeyOfCommentor) ;
  }
  if ( iAttachedFiles.size() > 0 ) {
    QVariantList listOfAttachedPhiles ; 
    foreach( const Hash& phile, iAttachedFiles )    { 
      QVariant v ;
      v.setValue(phile.toString()) ; 
      listOfAttachedPhiles.append(v) ; 
    }
    LOG_STR2("Number of attached files in profile comment serialization %d", listOfAttachedPhiles.size()) ; 
    m.insert(KCommentJSonAttachedFiles, listOfAttachedPhiles) ;
  } else {
    LOG_STR("No shared files in profile comment serialization") ; 
  }
  if ( iCommentorNickName.length() > 0 ) {
    m.insert(KCommentJSonCommentorNick  , iCommentorNickName.toUtf8()) ;
  }
  if ( iReferences != KNullHash ) {
    m.insert(KCommentJSonReferences , iReferences.toString()) ;
    m.insert(KCommentJSonReferencedType, (int)iTypeOfObjectReferenced) ;
  }
  return ( QVariant (m) ) ; // then put the map inside QVariant 
}

bool ProfileComment::setFromQVariant(const QVariantMap& aJSonAsQVariant,
				     const MController& aController) {
  if ( aJSonAsQVariant.contains(KCommentJSonProfileFPElement) ) {
    iProfileFingerPrint.fromString((const unsigned char *)aJSonAsQVariant[KCommentJSonProfileFPElement].toByteArray().constData()) ; 
  }
  if (aJSonAsQVariant.contains(KCommentJSonAttachedFiles) ) {
    QVariantList listOfSharedPhiles (aJSonAsQVariant[KCommentJSonAttachedFiles].toList()) ;  

    QListIterator<QVariant> i(listOfSharedPhiles);
    iAttachedFiles.clear() ; 
    while (i.hasNext()) {
      Hash h ; 
      h.fromString((const unsigned char*)(i.next().toString().toLatin1().constData())) ; 
      iAttachedFiles.append(h);
      QLOG_STR("ProfileComment had attached file " + h.toString()) ; 
    }
  }
  if (aJSonAsQVariant.contains(KCommentJSonTimeElement) ) {
    iTimeOfPublish = aJSonAsQVariant[KCommentJSonTimeElement].toUInt() ;
  }
  if (aJSonAsQVariant.contains(KCommentJSonCommentText) ) {
    iCommentText = QString::fromUtf8(aJSonAsQVariant[KCommentJSonCommentText].toByteArray()) ;
  }
  if (aJSonAsQVariant.contains(KCommentJSonCommentSubject) ) {
    iSubject = QString::fromUtf8(aJSonAsQVariant[KCommentJSonCommentSubject].toByteArray()) ;
  }
  if (aJSonAsQVariant.contains(KCommentJSonEncyptionKey) ) {
    iKeyOfCommentor = aJSonAsQVariant[KCommentJSonEncyptionKey].toByteArray() ;
    iCommentorHash = aController.model().contentEncryptionModel().hashOfPublicKey(iKeyOfCommentor); 
  }

  if ( aJSonAsQVariant.contains(KCommentJSonCommentorNick) ) {
    iCommentorNickName = QString::fromUtf8(aJSonAsQVariant[KCommentJSonCommentorNick].toByteArray()) ;
  }
  if ( aJSonAsQVariant.contains(KCommentJSonReferences) &&
       aJSonAsQVariant.contains(KCommentJSonReferencedType) ) {
    iReferences.fromString((const unsigned char *)aJSonAsQVariant[KCommentJSonReferences].toByteArray().constData()) ; 
    switch ( aJSonAsQVariant[KCommentJSonReferencedType].toInt() ) {
    case ClassifiedAd:
      iTypeOfObjectReferenced = ClassifiedAd;
      break ;
    case BinaryBlob:
      iTypeOfObjectReferenced = BinaryBlob;
      break ;
    case UserProfile:
      iTypeOfObjectReferenced = UserProfile;
      break ;
    default:
      iReferences = KNullHash ; 
    }
  }
  return true ; 
}

bool ProfileComment::fromJSon(const QByteArray &aJSonBytes,
			      const MController& aController ) {
  QJson::Parser parser;
  bool ok;

  QVariantMap result = parser.parse (aJSonBytes, &ok).toMap();
  if (!ok) {
    return false ;
  } else {
    ok = setFromQVariant(result,aController) ; 
  }

  return ok ;
}

