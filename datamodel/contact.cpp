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
#include "contact.h"
#include "../net/node.h"

static const char *KContactJSonFingerPrint = "fp" ;
static const char *KContactJSonNickName = "nick" ;
static const char *KContactJSonIsTrusted = "trusted" ;
static const int KJSONMsgVersionValue ( 1 ) ;

Contact::Contact() :
  iFingerPrint(KNullHash),
  iIsTrusted(false) {
}

Contact::~Contact() {
}

QByteArray Contact::asJSon(const MController& /*aController*/) const {
  // first have a map ; that is the top-level JSon-object
  QMap<QString,QVariant> m ;

  if ( iFingerPrint != KNullHash ) {
    m.insert(KContactJSonFingerPrint, iFingerPrint.toString().toUtf8()) ;
  }

  if ( iNickName.length() > 0 ) {
    m.insert(KContactJSonNickName,  iNickName.toUtf8()) ;
  }
  if ( iIsTrusted ) {
    m.insert(KContactJSonIsTrusted, iIsTrusted) ;
  }
  QJson::Serializer serializer;

  QVariant j (m); // then put the map inside QVariant and that
  // may then be serialized in libqjson-0.7 and 0.8
  QByteArray retval ( serializer.serialize(j) ) ;
  LOG_STR2("contact %s", qPrintable(QString(retval))) ; 
  return retval ;
}

bool Contact::fromJSon(const QByteArray &aJSonBytes,
		       const MController& /*aController*/ ) {
  QJson::Parser parser;
  bool ok;

  QVariantMap result = parser.parse (aJSonBytes, &ok).toMap();
  if (!ok) {
    return false ;
  }
  if ( result.contains(KContactJSonFingerPrint) ) {
    iFingerPrint.fromString((const unsigned char *)QString::fromUtf8(result[KContactJSonFingerPrint].toByteArray()).toLatin1().constData()) ; 
  }

  if ( result.contains(KContactJSonNickName) ) {
    iNickName = QString::fromUtf8(result[KContactJSonNickName].toByteArray()) ;
  } else {
    iNickName.clear() ;
  }
  if ( result.contains(KContactJSonIsTrusted) ) {
    iIsTrusted = result[KContactJSonIsTrusted].toBool() ; 
  }
  return ok ;
}

QString Contact::displayName() const {
  QString retval ;
  if ( iNickName.length() > 0 ) {
    retval = iNickName ; 
  } else {
    retval = iFingerPrint.toString() ; 
  }
  return retval ;
}

QVariant Contact::asQVariant() const {

  QMap<QString,QVariant> m ;
  m.insert(KContactJSonFingerPrint, iFingerPrint.toQVariant()) ;
  if ( iIsTrusted ) {
    m.insert(KContactJSonIsTrusted, iIsTrusted) ;
  }
  if ( iNickName.length() > 0 ) {
    m.insert(KContactJSonNickName, iNickName) ;
  }
  QVariant j (m) ;
  return j ;
}

Contact Contact::fromQVariant(const QVariantMap& aJSonAsQVariant) {
  Contact c ;

  if ( aJSonAsQVariant.contains(KContactJSonFingerPrint) ) {

    c.iFingerPrint.fromQVariant(aJSonAsQVariant[KContactJSonFingerPrint]) ; 

    if ( aJSonAsQVariant.contains(KContactJSonIsTrusted) ) {
      c.iIsTrusted = aJSonAsQVariant[KContactJSonIsTrusted].toBool() ;
    }
    if ( aJSonAsQVariant.contains(KContactJSonNickName) ) {
      c.iNickName = aJSonAsQVariant[KContactJSonNickName].toString() ;
    }
  }
  return c ;
}
