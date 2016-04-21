/*     -*-C++-*- -*-coding: utf-8-unix;-*-
  Classified Ads is Copyright (c) Antti Järvinen 2015.

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

#include "voicecall.h"
#include "../log.h"
#include "../util/jsonwrapper.h"
#include <QVariantMap>
#include "../net/node.h"

static const char *KVoiceCallJSonTimeElement = "timeOfPublish" ;
static const char *KJSonVoiceCallVersion = "version" ;
static const char *KJSonVoiceCallOkToProceed = "okToProceed" ;
static const char *KJSonVoiceCallOriginatorKey = "key" ;
static const char *KJSonVoiceCallDestinationKey = "destkey" ;
static const char *KJSonVoiceCallOriginatingNode = "originating" ;
static const char *KJSonVoiceCallDestinationNode = "dest" ;
static const char *KJSonVoiceCallAesKey = "aeskey" ;
static const char *KJSonVoiceCallId = "callid" ;
static const int KJSONVoiceCallVersionValue ( 1 ) ;

extern Hash KNullHash ; 

VoiceCall::VoiceCall() :
    iCallId(0),
    iOriginatingNode(KNullHash),
    iDestinationNode(KNullHash),
    iOkToProceed(true),
    iTimeOfCallAttempt(0) {
    LOG_STR("VoiceCall::VoiceCall()") ;
}

VoiceCall::~VoiceCall() {
    LOG_STR("VoiceCall::~VoiceCall()") ;
}

QByteArray VoiceCall::asJSon() const {
    // first have a map ; that is the top-level JSon-object
    QMap<QString,QVariant> m ;

    m.insert(KJSonVoiceCallVersion, KJSONVoiceCallVersionValue) ;
    m.insert(KVoiceCallJSonTimeElement, QDateTime::currentDateTimeUtc().toTime_t()) ;
    m.insert(KJSonVoiceCallOkToProceed,iOkToProceed) ;
    m.insert(KJSonVoiceCallId, iCallId) ;
    if ( iOriginatingOperatorKey.length() > 0 ) {
        m.insert(KJSonVoiceCallOriginatorKey, iOriginatingOperatorKey.constData()) ;
    }
    if ( iDestinationOperatorKey.length() > 0 ) {
        m.insert(KJSonVoiceCallDestinationKey, 
                 iDestinationOperatorKey.constData()) ;
    }
    if ( iOriginatingNode != KNullHash ) {
        m.insert(KJSonVoiceCallOriginatingNode, iOriginatingNode.toString()) ;
    }
    if ( iDestinationNode != KNullHash ) {
        m.insert(KJSonVoiceCallDestinationNode, iDestinationNode.toString()) ;
    }

    if ( iSymmetricAESKey.size() > 0 ) {
        m.insert(KJSonVoiceCallAesKey, iSymmetricAESKey.constData()) ;
    }


    QVariant j (m); // then put the map inside QVariant and that
    // may then be serialized in libqjson-0.7 and 0.8
    QByteArray retval ( JSonWrapper::serialize(j) ) ;
    LOG_STR2("voicecall %s", qPrintable(QString(retval))) ;
    return retval ;
}

bool VoiceCall::fromJSon(const QByteArray &aJSonBytes ) {
    bool ok (true);
    QVariantMap result ( JSonWrapper::parse (aJSonBytes, &ok) );
    if (!ok) {
        QLOG_STR("QJson::Parser failed to parse " + QString::number(aJSonBytes.size()) + " bytes") ;
        return false ;
    } else {
        QLOG_STR("VoiceCall parse ok") ;
    }

    if ( result.contains(KVoiceCallJSonTimeElement) ) {
        iTimeOfCallAttempt = result[KVoiceCallJSonTimeElement].toInt() ;
    }

    if ( result.contains(KJSonVoiceCallOkToProceed) ) {
        iOkToProceed = result[KJSonVoiceCallOkToProceed].toBool() ;
    } else {
        ok = false ; 
    }

    if ( result.contains(KJSonVoiceCallId) ) {
        iCallId = result[KJSonVoiceCallId].toInt() ;
    } else {
        ok = false ; 
    }

    if ( result.contains(KJSonVoiceCallOriginatorKey) ) {
        iOriginatingOperatorKey.clear() ;
        iOriginatingOperatorKey.append(result[KJSonVoiceCallOriginatorKey].toByteArray()) ;
    } else {
        ok = false ; 
    }

    if ( result.contains(KJSonVoiceCallDestinationKey) ) {
        iDestinationOperatorKey.clear() ;
        iDestinationOperatorKey.append(result[KJSonVoiceCallDestinationKey].toByteArray()) ;
    } else {
        // it is ok if this data is missing
    }

    if ( result.contains(KJSonVoiceCallOriginatingNode) ) {
        iOriginatingNode.fromString((const unsigned char *)QString::fromUtf8(result[KJSonVoiceCallOriginatingNode].toByteArray()).toLatin1().constData()) ;
    } else {
        ok = false ; 
    }
    if ( result.contains(KJSonVoiceCallDestinationNode) ) {
        iDestinationNode.fromString((const unsigned char *)QString::fromUtf8(result[KJSonVoiceCallDestinationNode].toByteArray()).toLatin1().constData()) ;
    } else {
        ok = false ; 
    }
    if ( iOriginatingNode == KNullHash ||
         iDestinationNode == KNullHash ) {
        ok = false ; 
    }

    if ( result.contains(KJSonVoiceCallAesKey) ) {
        iSymmetricAESKey.clear() ;
        iSymmetricAESKey.append(result[KJSonVoiceCallAesKey].toByteArray()) ;
    }

    return ok ;
}
