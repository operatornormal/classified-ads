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

#include "binaryfile.h"
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

static const char *KJSonBinaryFileName = "fileName" ;
static const char *KJSonBinaryFileFP = "fingerPrint" ;
static const char *KJSonBinaryFileDesc = "desc" ;
static const char *KJSonBinaryFilePublisher = "publisher" ;
static const char *KJSonBinaryFileContentOwner = "contentOwner" ;
static const char *KJSonBinaryFileTime = "time" ;
static const char *KJSonBinaryFileLicense = "license" ;
static const char *KJSonBinaryFileMime = "mimetype" ;
static const char *KJSonBinaryFileEncryption = "encrypted" ;

BinaryFile::BinaryFile(const Hash& aHash) :
    iFingerPrint(aHash),
    iTimeOfPublish(0),
    iIsEncrypted(false), // initially all philes are phublic ; right?
    iIsCompressed(false),
    iLocalStorageStatus(LocalStorageStatusUnknown) {
    LOG_STR("BinaryFile::BinaryFile()") ;
}

BinaryFile::~BinaryFile() {
    LOG_STR("BinaryFile::~BinaryFile()") ;
}

QByteArray BinaryFile::asJSon(const MController& /*aController*/) const {
    // first have a map ; that is the top-level JSon-object
    QMap<QString,QVariant> m ;
    m.insert(KJSonBinaryFileFP, iFingerPrint.toString()) ; // no non-ascii chars
    if ( iDescription.length() > 0 ) {
        m.insert(KJSonBinaryFileDesc, iDescription.toUtf8()) ;
    }
    if ( iFileName.length() > 0 ) {
        m.insert(KJSonBinaryFileName, iFileName.toUtf8()) ;
    }

    if ( iOwner.length() > 0 ) {
        m.insert(KJSonBinaryFilePublisher, iOwner.toUtf8()) ;
    }
    if ( iContentOwner.length() > 0 ) {
        m.insert(KJSonBinaryFileContentOwner, iContentOwner.toUtf8()) ;
    }
    m.insert(KJSonBinaryFileTime, iTimeOfPublish) ;
    if ( iLicense.length() > 0 ) {
        m.insert(KJSonBinaryFileLicense, iLicense.toUtf8()) ;
    }
    if ( iMimeType.length() > 0 ) {
        m.insert(KJSonBinaryFileMime, iMimeType.toUtf8()) ;
    }
    m.insert(  KJSonBinaryFileEncryption, iIsEncrypted) ;

    QJson::Serializer serializer;

    QVariant j (m); // then put the map inside QVariant and that
    // may then be serialized in libqjson-0.7 and 0.8
    QByteArray retval ( serializer.serialize(j) ) ;
    LOG_STR2("blob metadata %s", qPrintable(QString(retval))) ;
    return retval ;
}

bool BinaryFile::fromJSon(const QByteArray &aJSonBytes,
                          const MController& /*aController*/ ) {
    QJson::Parser parser;
    bool ok;

    QVariantMap result = parser.parse (aJSonBytes, &ok).toMap();
    if (!ok) {
        return false ;
    }
    if ( result.contains(KJSonBinaryFileFP) ) {
        QString fingerPrintString = QString::fromUtf8(result[KJSonBinaryFileFP].toByteArray()) ;
        if ( fingerPrintString != iFingerPrint.toString() ) {
            // huhuu, inside is different FP from what the key says??
            LOG_STR2("BinaryFile: Fingerprint in json %s" , qPrintable( fingerPrintString)) ;
            return false ;
        }
    } else {
        LOG_STR("BinaryFile: No fingerprint in json??" ) ;
        return false ;
    }
    if ( result.contains(KJSonBinaryFileName) ) {
        iFileName = QString::fromUtf8(result[KJSonBinaryFileName].toByteArray()) ;
    }
    if ( result.contains(KJSonBinaryFileDesc) ) {
        iDescription = QString::fromUtf8(result[KJSonBinaryFileDesc].toByteArray()) ;
    }
    if ( result.contains(KJSonBinaryFilePublisher) ) {
        iOwner = QString::fromUtf8(result[KJSonBinaryFilePublisher].toByteArray()) ;
    }
    if ( result.contains(KJSonBinaryFileContentOwner) ) {
        iContentOwner = QString::fromUtf8(result[KJSonBinaryFileContentOwner].toByteArray()) ;
    }
    if ( result.contains(KJSonBinaryFileTime) ) {
        iTimeOfPublish = result[KJSonBinaryFileTime].toUInt() ;
    }
    if ( result.contains(KJSonBinaryFileLicense) ) {
        iLicense = QString::fromUtf8(result[KJSonBinaryFileLicense].toByteArray()) ;
    }
    if ( result.contains(KJSonBinaryFileMime) ) {
        iMimeType = QString::fromUtf8(result[KJSonBinaryFileMime].toByteArray()) ;
    }

    if ( result.contains(KJSonBinaryFileEncryption) ) {
        iIsEncrypted = result[KJSonBinaryFileEncryption].toBool() ;
    }

    return ok ;
}

QString BinaryFile::displayName() const {
    QString retval ;
    if ( iIsEncrypted ) {
        retval = iFingerPrint.toString() ;
    } else {
        if ( iFileName.length() > 0  ) {
            retval = iFileName;
        } else {
            if ( iDescription.length() > 0 ) {
                retval = iDescription ;
            }  else {
                // no filename nor description..
                retval = iFingerPrint.toString() ;
            }
        }
        if ( retval.length() > 40 ) {
            retval = retval.left(40) ;
        }
    }
    return retval ;
}
