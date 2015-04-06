/*     -*-C++-*- -*-coding: utf-8-unix;-*-
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
#include "privmsg.h"
#include "../net/node.h"

static const char *KPrivMsgJSonTimeElement = "timeOfPublish" ;
static const char *KPrivMsgJSonAttachedFiles = "attachedPhiles" ;
static const char *KPrivMsgJSonSenderName = "senderName" ;
static const char *KPrivMsgJSonSubject = "subject" ;
static const char *KPrivMsgJSonReplyToMsg = "replyToMsg" ;
static const char *KPrivMsgJSonReplyToCa = "replyToCa" ;
static const char *KPrivMsgJSonText = "text" ;
static const char *KPrivMsgJSonProfileKey = "key" ;
static const char *KPrivMsgJSonRecipient = "to" ;
static const char *KPrivMsgJSonSenderNode = "senderNode" ;
static const char *KPrivMsgJSonMsgVersion = "v" ;
static const int KJSONMsgVersionValue ( 1 ) ;

PrivMessage::PrivMessage() :
    iFingerPrint(KNullHash),
    iTimeOfPublish(0),
    iReplyToMsg(KNullHash),
    iReplyToCa(KNullHash),
    iNodeOfSender(NULL) {
    LOG_STR("PrivMessage::PrivMessage()") ;
}

PrivMessage::~PrivMessage() {
    LOG_STR("PrivMessage::~PrivMessage()") ;
    delete iNodeOfSender ;
}

QByteArray PrivMessage::asJSon(const MController& /*aController*/) const {
    // first have a map ; that is the top-level JSon-object
    QMap<QString,QVariant> m ;

    if ( iAttachedFiles.size() > 0 ) {
        QVariantList listOfPhiles;
        LOG_STR2("iAttachedFiles has size %d", iAttachedFiles.size()) ;
        foreach( const Hash& phile, iAttachedFiles )    {
            listOfPhiles.append(phile.toString()) ;
        }
        if ( listOfPhiles.size() > 0 ) {
            m.insert(KPrivMsgJSonAttachedFiles, listOfPhiles) ;
        }
    }
    m.insert(KPrivMsgJSonMsgVersion, KJSONMsgVersionValue) ;

    if ( iSenderName.length() > 0 ) {
        m.insert(KPrivMsgJSonSenderName, iSenderName.toUtf8()) ;
    }
    if ( iRecipient != KNullHash ) {
        m.insert(KPrivMsgJSonRecipient, iRecipient.toString().toUtf8()) ;
    }
    if ( iSubject.length() > 0 ) {
        m.insert(KPrivMsgJSonSubject, iSubject.toUtf8()) ;
    }
    if ( iReplyToMsg != KNullHash ) {
        m.insert(KPrivMsgJSonReplyToMsg, iReplyToMsg.toString().toUtf8()) ;
    }
    if ( iReplyToCa != KNullHash ) {
        m.insert(KPrivMsgJSonReplyToCa, iReplyToCa.toString().toUtf8()) ;
    }

    if ( iMessageText.length() > 0 ) {
        m.insert(KPrivMsgJSonText, iMessageText.toUtf8()) ;
    }
    m.insert(KPrivMsgJSonTimeElement, iTimeOfPublish) ;
    if ( iNodeOfSender ) {
        m.insert(KPrivMsgJSonSenderNode, iNodeOfSender->asQVariant()) ;
    }

    if ( iProfileKey.length() > 0 ) {
        m.insert(KPrivMsgJSonProfileKey, iProfileKey) ;
    }

    QJson::Serializer serializer;

    QVariant j (m); // then put the map inside QVariant and that
    // may then be serialized in libqjson-0.7 and 0.8
    QByteArray retval ( serializer.serialize(j) ) ;
    LOG_STR2("msg %s", qPrintable(QString(retval))) ;
    return retval ;
}

bool PrivMessage::fromJSon(const QByteArray &aJSonBytes,
                           const MController& aController ) {
    QJson::Parser parser;
    bool ok;

    QVariantMap result = parser.parse (aJSonBytes, &ok).toMap();
    if (!ok) {
        return false ;
    }
    if ( result.contains(KPrivMsgJSonReplyToMsg) ) {
        iReplyToMsg.fromString((const unsigned char *)QString::fromUtf8(result[KPrivMsgJSonReplyToMsg].toByteArray()).toLatin1().constData()) ;
    }
    if ( result.contains(KPrivMsgJSonReplyToCa) ) {
        iReplyToCa.fromString((const unsigned char *)QString::fromUtf8(result[KPrivMsgJSonReplyToCa].toByteArray()).toLatin1().constData()) ;
    }
    if ( result.contains(KPrivMsgJSonRecipient) ) {
        iRecipient.fromString((const unsigned char *)QString::fromUtf8(result[KPrivMsgJSonRecipient].toByteArray()).toLatin1().constData()) ;
    }

    if ( result.contains( KPrivMsgJSonSenderNode ) ) {
        iNodeOfSender = Node::fromQVariant (result[KPrivMsgJSonSenderNode].toMap(),false) ;
    }

    if ( result.contains(KPrivMsgJSonAttachedFiles) ) {
        QVariantList listOfSharedPhiles (result[KPrivMsgJSonAttachedFiles].toList()) ;

        QListIterator<QVariant> i(listOfSharedPhiles);
        iAttachedFiles.clear() ;
        while (i.hasNext()) {
            Hash h ;
            h.fromString((const unsigned char*)(i.next().toString().toLatin1().constData())) ;
            iAttachedFiles.append(h);
        }
    }

    if ( result.contains(KPrivMsgJSonSenderName) ) {
        iSenderName = QString::fromUtf8(result[KPrivMsgJSonSenderName].toByteArray()) ;
    } else {
        iSenderName.clear() ;
    }
    if ( result.contains(KPrivMsgJSonTimeElement) ) {
        iTimeOfPublish = result[KPrivMsgJSonTimeElement].toUInt() ;
    }
    if ( result.contains(KPrivMsgJSonSubject) ) {
        iSubject = QString::fromUtf8(result[KPrivMsgJSonSubject].toByteArray()) ;
    } else {
        iSubject.clear() ;
    }

    if ( result.contains(KPrivMsgJSonText) ) {
        iMessageText = QString::fromUtf8(result[KPrivMsgJSonText].toByteArray()) ;
    } else {
        iMessageText.clear() ;
    }

    if ( result.contains(KPrivMsgJSonProfileKey) ) {
        iProfileKey.clear() ;
        iProfileKey.append(result[KPrivMsgJSonProfileKey].toByteArray()) ;

        // set sender hash from the key:
        iSenderHash = aController.model().contentEncryptionModel().hashOfPublicKey(iProfileKey) ;
    }

    return ok ;
}

QString PrivMessage::displayName() const {
    QString retval ;
    if ( iSubject.length() > 0 ) {
        retval = iSubject ;
    } else {
        retval = iFingerPrint.toString() ;
    }
    return retval ;
}
