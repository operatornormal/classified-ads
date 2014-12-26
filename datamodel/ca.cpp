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

#include "ca.h"
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
#include "profile.h"

static const char *KCaJSonTimeElement = "timeOfPublish" ;
static const char *KCaJSonAttachedFiles = "attachedPhiles" ;
static const char *KCaJSonSenderName = "senderName" ;
static const char *KCaJSonSenderHash = "senderFP" ;
static const char *KCaJSonSubject = "subject" ;
static const char *KCaJSonReplyTo = "replyTo" ;
static const char *KCaJSonGroup = "group" ;
static const char *KCaJSonText = "text" ;
static const char *KCaJSonProfileKey = "key" ;
static const char *KCaJSonCaVersion = "ver" ;

static const char *KCaJSonCaAboutIndex = "aidx" ;
static const char *KCaJSonCaConcernsIndex = "cidx" ;
static const char *KCaJSonCaInIndex = "iidx" ;
static const char *KCaJSonCaAboutTxt = "atxt" ;
static const char *KCaJSonCaConcernsTxt = "ctxt" ;
static const char *KCaJSonCaInTxt = "itxt" ;

static const int KJSONCaVersionValue = 1 ; /**< if our format should change? */

CA::CA() :
  iFingerPrint(KNullHash),
  iTimeOfPublish(0),
  iReplyTo(KNullHash),
  iAboutComboBoxIndex(-1),
  iConcernsComboBoxIndex(-1),
  iInComboBoxIndex(-1) {
  LOG_STR("ClassifiedAd::ClassifiedAd()") ;
}

CA::~CA() {
  LOG_STR("ClassifiedAd::~ClassifiedAd()") ;
}

QByteArray CA::asJSon(const MController& /*aController*/) const {
  // first have a map ; that is the top-level JSon-object
  QMap<QString,QVariant> m ;

  if ( iAttachedFiles.size() > 0 ) {
    QVariantList listOfPhiles;
    LOG_STR2("iAttachedFiles has size %d", iAttachedFiles.size()) ; 
    foreach( const Hash& phile, iAttachedFiles )    {
      listOfPhiles.append(phile.toString()) ; 
    }
    if ( listOfPhiles.size() > 0 ) {
      m.insert(KCaJSonAttachedFiles, listOfPhiles) ;
    }
  }
  m.insert(KCaJSonCaVersion, KJSONCaVersionValue) ;

  if ( iSenderName.length() > 0 ) {
    m.insert(KCaJSonSenderName, iSenderName.toUtf8()) ;
  }
  if ( iSenderHash != KNullHash ) {
    m.insert(KCaJSonSenderHash, iSenderHash.toString().toUtf8()) ;
  }
  if ( iSubject.length() > 0 ) {
    m.insert(KCaJSonSubject, iSubject.toUtf8()) ;
  }
  if ( iReplyTo != KNullHash ) {
    m.insert(KCaJSonReplyTo, iReplyTo.toString().toUtf8()) ;
  }
  if ( iGroup.length() > 0 ) {
    m.insert(KCaJSonGroup, iGroup.toUtf8()) ;
  }
  if ( iMessageText.length() > 0 ) {
    m.insert(KCaJSonText, iMessageText.toUtf8()) ;
  }
  m.insert(KCaJSonTimeElement, iTimeOfPublish) ; 

  QByteArray encryptionKey ;
  if ( iProfileKey.length() > 0 ) {
    m.insert(KCaJSonProfileKey, iProfileKey) ;
  }

  if ( iAboutComboBoxIndex > -1 ) {
    m.insert(KCaJSonCaAboutIndex, iAboutComboBoxIndex) ;
  } else  if ( iAboutComboBoxText.length() > 0 ) {
    m.insert(KCaJSonCaAboutTxt, iAboutComboBoxText.toUtf8()) ;
  }
  if ( iConcernsComboBoxIndex > -1 ) {
    m.insert(KCaJSonCaConcernsIndex, iConcernsComboBoxIndex) ;
  } else if ( iConcernsComboBoxText.length() > 0 ) {
    m.insert(KCaJSonCaConcernsTxt, iConcernsComboBoxText.toUtf8()) ;
  }
  if ( iInComboBoxIndex > -1 ) {
    m.insert(KCaJSonCaInIndex, iInComboBoxIndex) ;
  } else if ( iInComboBoxText.length() > 0 ) {
    m.insert(KCaJSonCaInTxt, iInComboBoxText.toUtf8()) ;
  }


  QJson::Serializer serializer;

  QVariant j (m); // then put the map inside QVariant and that
  // may then be serialized in libqjson-0.7 and 0.8
  QByteArray retval ( serializer.serialize(j) ) ;
  LOG_STR2("ca %s", qPrintable(QString(retval))) ; 
  return retval ;
}

bool CA::fromJSon(const QByteArray &aJSonBytes,
		  const MController& /*aController*/ ) {
  QJson::Parser parser;
  bool ok;
  parser.allowSpecialNumbers(true) ; 
  QVariantMap result = parser.parse (aJSonBytes, &ok).toMap();
  if (!ok) {
    QLOG_STR("QJson::Parser failed to parse " + QString::number(aJSonBytes.size()) + " bytes") ; 
    return false ;
  } else {
    QLOG_STR("CA parse ok") ; 
  }
  if ( result.contains(KCaJSonReplyTo) ) {
    iReplyTo.fromString((const unsigned char *)QString::fromUtf8(result[KCaJSonReplyTo].toByteArray()).toLatin1().constData()) ; 
  }
  if ( result.contains(KCaJSonSenderHash) ) {
    iSenderHash.fromString((const unsigned char *)QString::fromUtf8(result[KCaJSonSenderHash].toByteArray()).toLatin1().constData()) ; 
  }

  if ( result.contains(KCaJSonAttachedFiles) ) {
    QVariantList listOfSharedPhiles (result[KCaJSonAttachedFiles].toList()) ;  

    QListIterator<QVariant> i(listOfSharedPhiles);
    iAttachedFiles.clear() ; 
    while (i.hasNext()) {
      Hash h ; 
      h.fromString((const unsigned char*)(i.next().toString().toLatin1().constData())) ; 
      iAttachedFiles.append(h);
    }
  }

  if ( result.contains(KCaJSonSenderName) ) {
    iSenderName = QString::fromUtf8(result[KCaJSonSenderName].toByteArray()) ;
  } else {
    iSenderName.clear() ;
  }
  if ( result.contains(KCaJSonTimeElement) ) {
    iTimeOfPublish = result[KCaJSonTimeElement].toUInt() ; 
    QLOG_STR("In ca time of publish = " + QString::number(iTimeOfPublish));
  } else {
    QLOG_STR("Ca contained no time of publish" );
  }
  if ( result.contains(KCaJSonSubject) ) {
    iSubject = QString::fromUtf8(result[KCaJSonSubject].toByteArray()) ;
  } else {
    iSubject.clear() ;
  }
  if ( result.contains(KCaJSonGroup) ) {
    iGroup = QString::fromUtf8(result[KCaJSonGroup].toByteArray()) ;
  } else {
    iGroup.clear() ;
  }
  if ( result.contains(KCaJSonText) ) {
    iMessageText = QString::fromUtf8(result[KCaJSonText].toByteArray()) ;
  } else {
    iMessageText.clear() ;
  }

  if ( result.contains(KCaJSonProfileKey) ) {
    iProfileKey.clear() ; 
    iProfileKey.append(result[KCaJSonProfileKey].toByteArray()) ; 
  }

  if ( result.contains(KCaJSonCaAboutIndex) ) {
    iAboutComboBoxIndex = result[KCaJSonCaAboutIndex].toInt() ; 
  } else if ( result.contains(KCaJSonCaAboutTxt) ) {
    iAboutComboBoxText = QString::fromUtf8(result[KCaJSonCaAboutTxt].toByteArray()) ;
  }

  if ( result.contains(KCaJSonCaConcernsIndex) ) {
    iConcernsComboBoxIndex = result[KCaJSonCaConcernsIndex].toInt() ; 
  } else if ( result.contains(KCaJSonCaConcernsTxt) ) {
    iConcernsComboBoxText = QString::fromUtf8(result[KCaJSonCaConcernsTxt].toByteArray()) ;
  }

  if ( result.contains(KCaJSonCaInIndex) ) {
    iInComboBoxIndex = result[KCaJSonCaInIndex].toInt() ; 
  } else if ( result.contains(KCaJSonCaInTxt) ) {
    iInComboBoxText = QString::fromUtf8(result[KCaJSonCaInTxt].toByteArray()) ;
  }

  return ok ;
}

QString CA::displayName() const {
  QString retval ;
  if ( iSubject.length() > 0 ) {
    retval = iSubject ; 
  } else {
    retval = iFingerPrint.toString() ; 
  }
  return retval ;
}
