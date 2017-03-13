/*     -*-C++-*- -*-coding: utf-8-unix;-*-
  Classified Ads is Copyright (c) Antti Järvinen 2013-2017.

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

#include "cadbrecord.h"
#include "../log.h"
#include "../util/jsonwrapper.h"
#include <QVariantMap>
#include "../net/node.h"
#include <limits>
#include <QVariantMap>
#include "../util/jsonwrapper.h"

static const char *KCaDbRecordJSonTimeOfPublish = "timeOfPublish" ;
static const char *KCaDbRecordJSonCaVersion = "version" ;
static const char *KCaDbRecordJSonRecordId = "recordId" ;
static const char *KCaDbRecordJSonCollectionId = "collectionId" ;
static const char *KCaDbRecordJSonSearchPhrase = "searchPhrase" ;
static const char *KCaDbRecordJSonSearchNumber = "searchNumber" ;
static const char *KCaDbRecordJSonData = "data" ;
static const char *KCaDbRecordJSonIsEncrypted = "isEncrypted" ;
// search related json keys
static const char *KCaDbSearchJSonCollection = "collectionId" ; 
static const char *KCaDbSearchJSonById = "exactId"; 
static const char *KCaDbSearchJSonModifiedAfter = "modifiedAfter"; 
static const char *KCaDbSearchJSonModifiedBefore = "modifiedBefore";
static const char *KCaDbSearchJSonByHavingNumberMoreThan = "numberMoreThan"; 
static const char *KCaDbSearchJSonByHavingNumberLessThan = "numberLessThan";
static const char *KCaDbSearchJSonBySearchPhrase = "searchPhase"; 
static const char *KCaDbSearchJSonBySender = "senderFp" ;

static const int KJSONCaDbRecordVersionValue ( 1 ) ;

extern Hash KNullHash ; 

CaDbRecord::CaDbRecord() :
    iSearchNumber(std::numeric_limits<qint64>::min()),
    iIsEncrypted(false),
    iTimeOfPublish(0),
    iIsSignatureVerified(false) {
    LOG_STR("CaDbRecord::CaDbRecord()") ;
}

CaDbRecord::~CaDbRecord() {
    LOG_STR("CaDbRecord::~CaDbRecord()") ;
}

QByteArray CaDbRecord::asJSon() const {
    // first have a map ; that is the top-level JSon-object
    QMap<QString,QVariant> m ;

    m.insert(KCaDbRecordJSonCaVersion, KJSONCaDbRecordVersionValue) ;
    m.insert(KCaDbRecordJSonTimeOfPublish, iTimeOfPublish) ;
    m.insert(KCaDbRecordJSonIsEncrypted,iIsEncrypted) ;

    if ( iRecordId != KNullHash ) {
        m.insert(KCaDbRecordJSonRecordId, iRecordId.toString()) ;
    }
    if ( iCollectionId != KNullHash ) {
        m.insert(KCaDbRecordJSonCollectionId, iCollectionId.toString()) ;
    }
    if ( iSearchPhrase.length() > 0 ) {
        m.insert(KCaDbRecordJSonSearchPhrase, iSearchPhrase.toUtf8()) ;
    }
    if ( iSearchNumber != std::numeric_limits<qint64>::min() ) {
        m.insert(KCaDbRecordJSonSearchNumber, iSearchNumber) ;
    }
    if( iData.length() ) {
        m.insert(KCaDbRecordJSonData, iData ) ;
    }

    QVariant j (m); // then put the map inside QVariant and that
    // may then be serialized in libqjson-0.7 and 0.8
    QByteArray retval ( JSonWrapper::serialize(j) ) ;
    LOG_STR2("CaDbRecord %s", qPrintable(QString(retval))) ;
    return retval ;
}

bool CaDbRecord::fromJSon(const QByteArray &aJSonBytes ) {
    bool ok (true);
    QVariantMap result ( JSonWrapper::parse (aJSonBytes, &ok) );
    if (!ok) {
        QLOG_STR("QJson::Parser failed to parse " + QString::number(aJSonBytes.size()) + " bytes") ;
        return false ;
    } else {
        QLOG_STR("CaDbRecord parse ok") ;
    }

    if ( result.contains(KCaDbRecordJSonTimeOfPublish) ) {
        iTimeOfPublish = result[KCaDbRecordJSonTimeOfPublish].toInt() ;
    }
    if ( result.contains(KCaDbRecordJSonIsEncrypted) ) {
        iIsEncrypted = result[KCaDbRecordJSonTimeOfPublish].toBool() ;
    }
    if ( result.contains(KCaDbRecordJSonRecordId) ) {
        iRecordId.fromString((const unsigned char *)result[KCaDbRecordJSonRecordId].toByteArray().constData()) ;
    } else {
        ok = false ; 
    }
    if ( result.contains(KCaDbRecordJSonCollectionId) ) {
        iCollectionId.fromString((const unsigned char *)result[KCaDbRecordJSonCollectionId].toByteArray().constData()) ;
    } else {
        ok = false ; 
    }
    if ( result.contains(KCaDbRecordJSonSearchNumber) ) {
        iSearchNumber = result[KCaDbRecordJSonSearchNumber].toLongLong() ;
    }
    if ( result.contains(KCaDbRecordJSonSearchPhrase) ) {
        iSearchPhrase = result[KCaDbRecordJSonSearchPhrase].toString() ;
    }
    iData.clear() ;
    // note that it is ok for data to be empty. empty record is 
    // possible record. 
    if ( result.contains(KCaDbRecordJSonData) ) {
        iData.append(result[KCaDbRecordJSonData].toByteArray()) ;
    } 

    if ( iRecordId == KNullHash ||
         iCollectionId == KNullHash ) {
        ok = false ; 
    }
    return ok ;
}

bool CaDbRecord::SearchStructure::operator==(const struct SearchStructure& aItemToCompare) const {
    if ( iFromCollection != aItemToCompare.iFromCollection ) return false ; 
    if ( iById != aItemToCompare.iById ) return false ; 
    if ( iModifiedAfter != aItemToCompare.iModifiedAfter ) return false ; 
    if ( iModifiedBefore != aItemToCompare.iModifiedBefore ) return false ; 
    if ( iByHavingNumberMoreThan != aItemToCompare.iByHavingNumberMoreThan ) return false ; 
    if ( iByHavingNumberLessThan != aItemToCompare.iByHavingNumberLessThan ) return false ; 
    if ( iBySearchPhrase != aItemToCompare.iBySearchPhrase ) return false ; 
    if ( iBySender != aItemToCompare.iBySender ) return false ; 
    return true ;
}
bool CaDbRecord::SearchStructure::operator<(const struct SearchStructure& aItemToCompare) const {
    // first test: if collection does not match, the item being
    // compared is no smaller because it includes records from 
    // entirely different collection, definitely outside scope
    // of one being compared so return "false", saying that 
    // record being compared is larger by size of resultset.
    if ( iFromCollection != aItemToCompare.iFromCollection ) return false ; 
    // same for "by identifier" -> different identifier always means
    // different resultset:
    if ( iById != aItemToCompare.iById ) return false ; 
    // if "modifier after" is after the item being compared,
    // say that resultset may be broader:
    if ( iModifiedAfter > aItemToCompare.iModifiedAfter ) return false ; 
    // if "modifier before" is before the item being compared,
    // say that resultset may be broader:
    if ( iModifiedBefore < aItemToCompare.iModifiedBefore ) return false ; 
    // if range of search number is larger, then this instance
    // is not less:
    if ( iByHavingNumberMoreThan < aItemToCompare.iByHavingNumberMoreThan )
        return false ; 
    if ( iByHavingNumberLessThan > aItemToCompare.iByHavingNumberLessThan )
        return false ; 
    // if limit "by sender" is different, then this is not less:
    if ( iBySender != aItemToCompare.iBySender ) return false ; 
    // then try check for search phrase.. if search phrase 
    // of this instance is fully contained in item to compare,
    // then this may be less:
    if (this->iBySearchPhrase.length() > 0 &&
        aItemToCompare.iBySearchPhrase.contains(this->iBySearchPhrase) == false )
        return false ; 
    return true ; // everything is less, so total is also less
}    

QByteArray CaDbRecord::SearchStructure::asJSon() const {
    QMap<QString,QVariant> m ;
    if (  iFromCollection != KNullHash ) {
        m.insert(KCaDbSearchJSonCollection, iFromCollection.toString()) ; 
    }
    if (  iById != KNullHash ) {
        m.insert(KCaDbSearchJSonById, iById.toString()) ; 
    }    
    if (  iById == KNullHash || iFromCollection == KNullHash ) {
        if (  iBySender != KNullHash ) {
            m.insert(KCaDbSearchJSonBySender, iBySender.toString()) ; 
        }    
        if ( iModifiedAfter > std::numeric_limits<quint32>::min() ) {
            m.insert(KCaDbSearchJSonModifiedAfter,iModifiedAfter ) ; 
        }
        if ( iModifiedBefore < std::numeric_limits<quint32>::max() ) {
            m.insert(KCaDbSearchJSonModifiedBefore,iModifiedBefore ) ; 
        }

        if ( iByHavingNumberMoreThan > std::numeric_limits<qint64>::min() ) {
            m.insert(KCaDbSearchJSonByHavingNumberMoreThan,iByHavingNumberMoreThan) ; 
        }
        if ( iByHavingNumberLessThan < std::numeric_limits<qint64>::max() ) {
            m.insert(KCaDbSearchJSonByHavingNumberLessThan,iByHavingNumberLessThan ) ; 
        }
        if ( iBySearchPhrase.isEmpty() == false ) {
            m.insert(KCaDbSearchJSonBySearchPhrase,iBySearchPhrase.toUtf8() ) ; 
        }
    }

    QVariant v ( m ) ; 
    QByteArray result(JSonWrapper::serialize(v)) ; 
    return result ; 
}

bool CaDbRecord::SearchStructure::fromJSon(const QByteArray& aJSon) {
    bool retval ( false ) ;
    QVariantMap result ( JSonWrapper::parse (aJSon, &retval) );
    if (!retval) {
        return retval ;
    } else {
        // start by setting everything to default values:
        iFromCollection = KNullHash ; 
        iById = KNullHash ; 
        iModifiedAfter = 0 ; 
        iModifiedBefore = std::numeric_limits<quint32>::max();
        iByHavingNumberMoreThan = std::numeric_limits<qint64>::min(); 
        iByHavingNumberLessThan = std::numeric_limits<qint64>::max();
        iBySearchPhrase = QString::null ; 
        iBySender = KNullHash ; 
        // then go on checking what we have in json:
        if ( result.contains(KCaDbSearchJSonCollection ) ) {
            iFromCollection.fromString(reinterpret_cast<const unsigned char *>(result[KCaDbSearchJSonCollection].toByteArray().constData())) ; 
            if ( iFromCollection == KNullHash ) retval = false ; 
        } 
        if ( retval && result.contains(KCaDbSearchJSonById) ) {
            iById.fromString(reinterpret_cast<const unsigned char *>(result[KCaDbSearchJSonById].toByteArray().constData())) ; 
        }
        if ( retval && result.contains(KCaDbSearchJSonBySender) ) {
            iBySender.fromString(reinterpret_cast<const unsigned char *>(result[KCaDbSearchJSonBySender].toByteArray().constData())) ; 
        }
        if ( retval && result.contains(KCaDbSearchJSonModifiedAfter) ) {
            iModifiedAfter = result[KCaDbSearchJSonModifiedAfter].toUInt(&retval) ;
        }
        if ( retval && result.contains(KCaDbSearchJSonModifiedBefore) ) {
            iModifiedBefore = result[KCaDbSearchJSonModifiedBefore].toUInt(&retval) ;
        }
        if ( retval && result.contains(KCaDbSearchJSonByHavingNumberMoreThan) ) {
            iByHavingNumberMoreThan = result[KCaDbSearchJSonByHavingNumberMoreThan].toLongLong(&retval) ;
        }
        if ( retval && result.contains(KCaDbSearchJSonByHavingNumberLessThan) ) {
            iByHavingNumberLessThan = result[KCaDbSearchJSonByHavingNumberLessThan].toLongLong(&retval) ;
        }
        if ( result.contains(KCaDbSearchJSonBySearchPhrase) ) {
            iBySearchPhrase = QString::fromUtf8(result[KCaDbSearchJSonBySearchPhrase].toByteArray()) ; 
        }
    }
    return retval ; 
}
