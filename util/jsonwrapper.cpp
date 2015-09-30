/*  -*-C++-*- -*-coding: utf-8-unix;-*-
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

#include "jsonwrapper.h"
#if QT_VERSION >= 0x050000
#include <QJsonDocument>
#else
#ifdef WIN32
#include <QJson/Parser>
#include <QJson/Serializer>
#else
#include <qjson/parser.h>
#include <qjson/serializer.h>
#endif // WIN32
#endif // QT_VERSION
#include "../log.h"
//
// this parser method is static
//
QVariantMap JSonWrapper::parse(const QByteArray& aJSonText,
                               bool* aIsParseOk ,
                               bool aUncompressFirst  ) {
    if ( aIsParseOk != NULL ) {
        *aIsParseOk = false ; // by default declare a failure
    }
#if QT_VERSION >= 0x050000
    QJsonParseError errorHandler ;
    if ( aUncompressFirst ) {
        QByteArray unCompressed ( qUncompress(aJSonText)) ;
        QJsonDocument d = QJsonDocument::fromJson(unCompressed,&errorHandler);
        if ( errorHandler.error == QJsonParseError::NoError ) {
            if ( aIsParseOk ) {
                *aIsParseOk= true;
            }
            return d.toVariant().toMap() ;
        } else {
            QLOG_STR("Unparseable JSON:" + QString(unCompressed) + ":" + errorHandler.errorString()) ;
            return QVariant().toMap() ;
        }
    } else {
        QJsonDocument d = QJsonDocument::fromJson(aJSonText,&errorHandler);
        if ( errorHandler.error == QJsonParseError::NoError ) {
            if ( aIsParseOk ) {
                *aIsParseOk= true;
            }
            return d.toVariant().toMap() ;
        } else {
            QLOG_STR("Unparseable JSON:" + QString(aJSonText) + ": " + errorHandler.errorString()) ;
            return QVariant().toMap() ;
        }
    }
#else
// with qt4 use qjson, it is ok
    QJson::Parser parser;
    bool ok;
    if ( aUncompressFirst ) {
        QByteArray unCompressed ( qUncompress(aJSonText)) ;
        if ( unCompressed.size() > 0 ) {
            QVariantMap result = parser.parse (unCompressed, &ok).toMap();
            if ( aIsParseOk != NULL ) {
                *aIsParseOk = ok ; // actual parse result
            }
            return result ;
        } else {
            return QVariant().toMap() ; // failure, return empty
        }
    } else {
        QVariantMap result = parser.parse (aJSonText, &ok).toMap();
        if ( aIsParseOk != NULL ) {
            *aIsParseOk = ok ; // actual parse result
        }
        return result ;
    }
#endif
}

QByteArray JSonWrapper::serialize(const QVariant& aObjectToSerialize,
                                  bool aFinallyCompress  ) {
#if QT_VERSION >= 0x050000
    if ( aFinallyCompress ) {
        return qCompress(QJsonDocument::fromVariant(aObjectToSerialize).toJson(QJsonDocument::Compact))  ;
    } else {
        return QJsonDocument::fromVariant(aObjectToSerialize).toJson(QJsonDocument::Compact) ;
    }
#else
    QJson::Serializer serializer;

    if ( aFinallyCompress ) {
        return qCompress(serializer.serialize(aObjectToSerialize))  ;
    } else {
        return serializer.serialize(aObjectToSerialize)  ;
    }
#endif
}

