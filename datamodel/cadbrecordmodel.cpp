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

#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QUuid>
#include "cadbrecordmodel.h"
#include "cadbrecord.h" // actual record that is handled here
#include "../log.h"
#include "../util/hash.h"
#include "model.h"
#include "contentencryptionmodel.h"
#include "profilemodel.h"
#include "profile.h"
#include "const.h"
#include "searchmodel.h"

CaDbRecordModel::CaDbRecordModel(MController *aController,
        const MModelProtocolInterface &aModel)
    : ModelBase("dbrecord",KMaxRowsInTableDbRecord),
      iController(aController),
      iModel(aModel) {
    LOG_STR("CaDbRecordModel::CaDbRecordModel()") ;
    connect(this,
            SIGNAL(  error(MController::CAErrorSituation,
                           const QString&) ),
            iController,
            SLOT(handleError(MController::CAErrorSituation,
                             const QString&)),
            Qt::QueuedConnection ) ;

}



CaDbRecordModel::~CaDbRecordModel() {
    LOG_STR("CaDbRecordModel::~CaDbRecordModel()") ;
    iController = NULL ; // not owned, just set null
}

QString CaDbRecordModel::publishDbRecord(CaDbRecord& aRecord,
                                         const QList<Hash>* aRecordReaders) {
    LOG_STR("CaDbRecordModel::publishDbRecord() collection = " + 
            aRecord.iCollectionId.toString() ) ;
    if ( aRecord.iCollectionId == KNullHash ) {
        return "Collection is mandatory" ; 
    }
    aRecord.iTimeOfPublish = QDateTime::currentDateTimeUtc().toTime_t() ;
    aRecord.iSenderHash = iController->profileInUse() ; 
    if ( aRecord.iData.size() > 0 ) {
        const QByteArray compressedData ( qCompress(aRecord.iData ) ) ; 
        aRecord.iData.clear() ; 
        aRecord.iData.append(compressedData) ; 
    }
    if ( aRecord.iIsEncrypted ) {
        QByteArray encryptedJson ;
        Profile* ownProfile ( iController->model()
                              .profileModel()
                              .profileByFingerPrint(
                                  iController->profileInUse(),
                                  false /* emit */,
                                  true/* no image */));
        if ( ownProfile == NULL ) {
            return QString("Own profile not found from database") ; 
        }
        // user wants encrypt the record: replace content with
        // encrypted content.
        QList<Hash> recordReaders ; 
        if ( aRecordReaders ) {
            recordReaders.append(*aRecordReaders) ; 
        } else {
            if ( ownProfile->iProfileReaders.size() > 0 ) {
                recordReaders.append(ownProfile->iProfileReaders) ; 
            } else {
                delete ownProfile ; 
                return QString("No profiles to encrypt to") ; 
            }
        }
        if ( recordReaders.contains(ownProfile->iFingerPrint) == false ) {
            recordReaders.append(ownProfile->iFingerPrint) ; 
        }
        delete ownProfile ; 
        ownProfile = NULL ;
        QByteArray encryptedRecord ; 
        if ( iController->model().
             contentEncryptionModel().
             encrypt(recordReaders,
                     aRecord.iData,
                     encryptedRecord ) ) {
            aRecord.iData.clear() ;
            aRecord.iData.append(encryptedRecord) ;
            encryptedRecord.clear() ;
        } else {
            return "Encryption failed" ; 
        }
    }

    bool isRecordNew(false) ; 
    // hmm. record is new if it has no record id.
    // or if it has but is not found from db.
    // if it is has no record id, assign it one:
    if ( aRecord.iRecordId == KNullHash ) {
        isRecordNew = true ; 
        QByteArray recordIdSource ( QUuid::createUuid ().toByteArray() ) ; 
        // but uuid has less bits than SHA1. bartender, please make it double:
        recordIdSource.append ( QUuid::createUuid ().toByteArray() ) ; 
        aRecord.iRecordId.calculate ( recordIdSource ) ; 
    } else {
        // check out if record is in db already
        QString errorMessage (
        this->isRecordNew(aRecord,
                          &isRecordNew )) ; 
    }
    QByteArray digitalSignature ;
    QString additionalMetaDataStringToSign( // see similar code in verify method
        aRecord.iRecordId.toString() + 
        aRecord.iCollectionId.toString() + 
        aRecord.iSenderHash.toString() ) ; 
    QByteArray additionalMetaDataBytesToSign( additionalMetaDataStringToSign.toUtf8()) ; 
    if ( iController->model()
         .contentEncryptionModel()
         .sign(iController->profileInUse(), 
               aRecord.iData, 
               digitalSignature,
               &additionalMetaDataBytesToSign ) == 0 ) {
        aRecord.iIsSignatureVerified = true ; // yes, we trust self
        Hash ownNodeHash ( iController->getNode().nodeFingerPrint() ) ; 
        QString errorMessage = 
            this->persistDbRecordIntoDb(aRecord,
                                        &isRecordNew,
                                        digitalSignature,
                                        ownNodeHash.iHash160bits[4]) ; 
        if ( errorMessage.length() > 0 ) {
            return errorMessage ; 
        }

        // here user is publishing as "brand new" so set bangpath to 0
        QList<quint32> emptyBangPath ;
        // note how comments get published to address of the profile,
        // not to addr of the content
        iController->model().addItemToBePublished(DbRecord,
                                                  aRecord.iRecordId,
                                                  emptyBangPath,
                                                  aRecord.iCollectionId) ;

        // and send notify to have possible ui-datamodels to be updated.
        // normally there is 1 or 2
        emit contentReceived(aRecord.iRecordId,
                             aRecord.iCollectionId,
                             DbRecord) ;
    } else {
        return QString("Sign operation failed") ; 
    }

    return QString::null ;
}

QList<CaDbRecord *> CaDbRecordModel::searchRecords(const Hash& aFromCollection,
                                                   const Hash& aById ,
                                                   const quint32 aModifiedAfter,
                                                   const quint32 aModifiedBefore,
                                                   const qint64 aByHavingNumberMoreThan,
                                                   const qint64 aByHavingNumberLessThan,
                                                   const QString& aBySearchPhrase,
                                                   const Hash& aBySender,
                                                   const bool aForPublish )  {
    LOG_STR("CaDbRecordModel::searchRecords() collection = " + 
            aFromCollection.toString() ) ;
    QList<CaDbRecord *> resultSet ; 
    if ( aFromCollection == KNullHash && aById == KNullHash ) {
        return resultSet ; 
    }

    QSqlQuery query;
    bool ret ;

    QString selectStm = QString("select hash1,hash2,hash3,hash4,hash5,"
        "sender_hash1,"
        "sender_hash2,"
        "sender_hash3,"
        "sender_hash4,"
        "sender_hash5,"
        "time_last_reference,signature,time_of_publish,"
        "data,recvd_from,dbrecord.searchstring,dbrecord.searchnumber,"
        "isencrypted,issignatureverified,"
        "collection_hash1, collection_hash2,"  
        "collection_hash3, collection_hash4,"  
        "collection_hash5 %1 "
        "where ")
        .arg( ( aBySearchPhrase.isEmpty() ||
            iModel.searchModel()->isFTSSupported() == false ) ? 
          " from dbrecord " :
          " from dbrecord,dbrecord_search "  )  ; // if phrase, join also search table

    if ( aFromCollection != KNullHash ) {
        selectStm = selectStm + 
            "collection_hash1 = :collection_hash1 "
            "and collection_hash2 = :collection_hash2 "
            "and collection_hash3 = :collection_hash3 "
            "and collection_hash4 = :collection_hash4 "
            "and collection_hash5 = :collection_hash5 " ; 
    } else {
        selectStm = selectStm + 
            "1 = 1 " ; 
    }
    if ( aModifiedAfter != 0 ) {
        selectStm = selectStm + " and time_of_publish >= :modified_after " ;
    }
    if ( aModifiedBefore != std::numeric_limits<quint32>::max() ) {
        selectStm = selectStm + " and time_of_publish <= :modified_before " ;
    }
    if ( aByHavingNumberMoreThan != std::numeric_limits<qint64>::min() ) {
        selectStm = selectStm + " and searchnumber >= :number_more_than " ;
    }
    if ( aByHavingNumberLessThan != std::numeric_limits<qint64>::max() ) {
        selectStm = selectStm + " and searchnumber <= :number_less_than " ;
    }
    if ( aBySender != KNullHash ) {
        selectStm = selectStm + " and sender_hash1 = :sender_hash1 "
            " and sender_hash2 = :sender_hash2 "
            " and sender_hash3 = :sender_hash3 "
            " and sender_hash4 = :sender_hash4 "
            " and sender_hash5 = :sender_hash5 " ;
    }
    if ( aById != KNullHash ) {
        selectStm = selectStm + " and hash1 = :hash1 "
            " and hash2 = :hash2 "
            " and hash3 = :hash3 "
            " and hash4 = :hash4 "
            " and hash5 = :hash5 " ;
    }
    if ( !(aBySearchPhrase.isEmpty() ||
           iModel.searchModel()->isFTSSupported() == false ) ) {
        selectStm = selectStm + " and dbrecord.hash1 = dbrecord_search.docid "
            " and dbrecord_search.searchstring MATCH :searchphrase " ;
    }
    QLOG_STR(selectStm) ;
    if ( query.prepare(selectStm) == false ) {
        QLOG_STR("Search statement preparation failure " + 
                 selectStm + " " +
                 query.lastError().text() ) ; 
        // return empty resultset. sucks.
        // in practice possible error causes are syntax errors that
        // release testing will catch, or database has been
        // corrupted somehow. in case of corruption we hope that
        // other tables are corrupted too so user will get meaningful
        // error messages in addition to these empty resultsets:
        return resultSet ; 
    }
    if ( aFromCollection != KNullHash ) {
        query.bindValue(":collection_hash1", aFromCollection.iHash160bits[0]);
        query.bindValue(":collection_hash2", aFromCollection.iHash160bits[1]);
        query.bindValue(":collection_hash3", aFromCollection.iHash160bits[2]);
        query.bindValue(":collection_hash4", aFromCollection.iHash160bits[3]);
        query.bindValue(":collection_hash5", aFromCollection.iHash160bits[4]);
        QLOG_STR(":collection_hash1 " + QString::number(aFromCollection.iHash160bits[0])) ; 
        QLOG_STR(":collection_hash2 " + QString::number(aFromCollection.iHash160bits[1])) ; 
        QLOG_STR(":collection_hash3 " + QString::number(aFromCollection.iHash160bits[2])) ; 
        QLOG_STR(":collection_hash4 " + QString::number(aFromCollection.iHash160bits[3])) ; 
        QLOG_STR(":collection_hash5 " + QString::number(aFromCollection.iHash160bits[4])) ; 
    }
    if ( aById != KNullHash ) {
        query.bindValue(":hash1", aById.iHash160bits[0]);
        query.bindValue(":hash2", aById.iHash160bits[1]);
        query.bindValue(":hash3", aById.iHash160bits[2]);
        query.bindValue(":hash4", aById.iHash160bits[3]);
        query.bindValue(":hash5", aById.iHash160bits[4]);
    }
    
    if ( aModifiedAfter != 0 ) {
        query.bindValue(":modified_after", aModifiedAfter);
        QLOG_STR(":modified_after " + QString::number(aModifiedAfter)) ; 
    }
    if ( aModifiedBefore != std::numeric_limits<quint32>::max() ) {
        query.bindValue(":modified_before", aModifiedBefore);
        QLOG_STR(":modified_before " + QString::number(aModifiedBefore)) ; 
    }
    if ( aByHavingNumberMoreThan != std::numeric_limits<qint64>::min() ) {
        query.bindValue(":number_more_than", aByHavingNumberMoreThan);
       QLOG_STR(":number_more_than " + QString::number(aByHavingNumberMoreThan)) ; 
    }
    if ( aByHavingNumberLessThan != std::numeric_limits<qint64>::max() ) {
        query.bindValue(":number_less_than", aByHavingNumberLessThan);
       QLOG_STR(":number_less_than " + QString::number(aByHavingNumberLessThan)) ; 
    }
    if ( aBySender != KNullHash ) {
        query.bindValue(":sender_hash1", aBySender.iHash160bits[0]);
        query.bindValue(":sender_hash2", aBySender.iHash160bits[1]);
        query.bindValue(":sender_hash3", aBySender.iHash160bits[2]);
        query.bindValue(":sender_hash4", aBySender.iHash160bits[3]);
        query.bindValue(":sender_hash5", aBySender.iHash160bits[4]);
    }
    if (  aBySearchPhrase.isEmpty() == false &&
          iModel.searchModel()->isFTSSupported()  ) {
        QLOG_STR("Setting :searchphrase " + aBySearchPhrase) ; 
        query.bindValue(":searchphrase", aBySearchPhrase.toUtf8());
    } else {
        QLOG_STR("Not setting :searchphrase " + aBySearchPhrase) ; 
    }
    QList<QPair<Hash, Hash> > recordsToDeleteDueToBadSignature ; 
    QList<QPair<Hash, Hash> > recordsToUpdateDueToGoodSignature ;     
    ret = query.exec() ;
    if ( !ret ) {
        QLOG_STR(query.lastError().text() + " "+ __FILE__ + QString::number(__LINE__)) ;
        return resultSet ; 
    } else {

//select hash1 0 ,hash2 1 ,hash3 2 ,hash4 3 ,hash5 4 ,"
//        "sender_hash1," 5
//        "sender_hash2," 6
//        "sender_hash3," 7
//        "sender_hash4," 8
//        "sender_hash5," 9
//        "time_last_reference 10 ,signature 11 ,time_of_publish 12 ,"
//        "data 13 ,recvd_from 14 ,searchstring 15 ,searchnumber 16 ,"
//        "isencrypted 17 ,issignatureverified 18

        while ( query.next() ) {
            CaDbRecord* r = new CaDbRecord ; 
            if ( query.isNull(13) == false ) {
                // there is data
                r->iData.append(query.value(13).toByteArray()) ; 
            }

            r->iRecordId = Hash ( query.value(0).toUInt(),
                                 query.value(1).toUInt(),
                                 query.value(2).toUInt(),
                                 query.value(3).toUInt(),
                                 query.value(4).toUInt() ) ; 
            r->iSenderHash = Hash ( query.value(5).toUInt(),
                                   query.value(6).toUInt(),
                                   query.value(7).toUInt(),
                                   query.value(8).toUInt(),
                                   query.value(9).toUInt() ) ; 
            if ( aFromCollection == KNullHash ) {
                r->iCollectionId = Hash ( query.value(19).toUInt(),
                                          query.value(20).toUInt(),
                                          query.value(21).toUInt(),
                                          query.value(22).toUInt(),
                                          query.value(23).toUInt() ) ; 
            } else {
                r->iCollectionId = aFromCollection ; // must be 
            }

            r->iIsSignatureVerified = query.value(18).toBool() ; 
            r->iSignature.append ( query.value(11).toByteArray() ) ;
            if ( r->iIsSignatureVerified == false ) {
                // try verify
                if ( tryVerifyRecord(*r,
                                     r->iSignature,
                                     recordsToUpdateDueToGoodSignature)  == false ) {
                    // bad sig:
                    recordsToDeleteDueToBadSignature.append(QPair<Hash,Hash>(r->iRecordId,r->iSenderHash)) ; 
                    delete r ; 
                    continue ; 
                } 
            }
          
            r->iIsEncrypted = query.value(17).toBool()  ; 
            // for verification we need the key. .. we may have the key
            // if we've been dealing with the sender before, or not.
            //
            // sender information is only inside content so if that is
            // encrypted, we need to de-crypt first, so lets go:
            if ( r->iIsEncrypted && 
                 r->iData.length() > 0 && 
                 aForPublish == false ) {
                QByteArray plainTextCaDbRecordData ;
                if ( iController->model().contentEncryptionModel().decrypt(r->iData,
                                                                           plainTextCaDbRecordData,
                                                                           false) ) {
                    r->iData.clear() ;
                    r->iData.append(qUncompress(plainTextCaDbRecordData)) ;
                } else {
                    delete r ; 
                    r = NULL ; 
                    continue ; // skip rest of the loop
                }
            } 
            if ( query.isNull(15) == false ) {
                r->iSearchPhrase = QString::fromUtf8(query.value(15).toByteArray()) ; 
            }
            if ( query.isNull(16) == false ) {
                r->iSearchNumber = query.value(16).toLongLong() ; 
            }
            r->iTimeOfPublish = query.value(12).toUInt() ; 
            if ( r->iIsEncrypted == false && 
                 r->iData.length() > 0 &&  
                 aForPublish == false)  { // there is data but not encrypted
                QByteArray uncompressed ( qUncompress(r->iData) ) ; 
                r->iData.clear() ;
                r->iData.append(uncompressed) ; 
            }
            setTimeLastReference(r->iRecordId,
                                 QDateTime::currentDateTimeUtc().toTime_t()) ;
            resultSet.append(r) ; 
        }
    }
    query.finish() ; 
    QPair<Hash, Hash> deletee ;
    foreach ( deletee,
              recordsToDeleteDueToBadSignature) {
        deleteRecord(deletee.first,deletee.second) ; 
    }
    QPair<Hash, Hash> updatee ;
    foreach ( updatee,
              recordsToUpdateDueToGoodSignature) {
        updateRecordVerification(deletee.first,deletee.second,true) ; 
    }
    // if user asked something else than "by id" and her
    // query went un-answered, give query to dht query engine
    // and do this only if it was user-originated query ;
    // also if user did not specify record id at all, give
    // query to retrieval engine - it has logic to prune away
    // overlapping queries so spamming the engine is no problem
    if ( aForPublish == false &&
         (
             ( aById != KNullHash && resultSet.size() == 0 ) ||
               aById == KNullHash 
         )         
       ) {
        CaDbRecord::SearchTerms terms ;
        terms.iFromCollection = aFromCollection; 
        terms.iById = aById ; 
        terms.iModifiedAfter = aModifiedAfter; 
        terms.iModifiedBefore = aModifiedBefore;
        terms.iByHavingNumberMoreThan = aByHavingNumberMoreThan; 
        terms.iByHavingNumberLessThan = aByHavingNumberLessThan;
        terms.iBySearchPhrase = aBySearchPhrase ; 
        terms.iBySender = aBySender ; 
        iController->startRetrievingContent ( terms ) ; 
    }
    return resultSet ; 
}


bool CaDbRecordModel::sentCaDbRecordReceived(CaDbRecord& aRecord,
                                             const QByteArray& aSignature,
                                             const Hash& aFromNode ) {
    const QList<quint32> dummy ;
    return doHandlepublishedOrSentRecord( aRecord,
                                          aSignature,
                                          dummy,
                                          aFromNode ,
                                          false) ;
}


bool CaDbRecordModel::publishedCaDbRecordReceived(CaDbRecord& aRecord,
                                     const QByteArray& aSignature,
                                     const QList<quint32>& aBangPath,
                                     const Hash& aFromNode) {
    return doHandlepublishedOrSentRecord( aRecord,
                                          aSignature,
                                          aBangPath,
                                          aFromNode ,
                                          true ) ;
}


bool CaDbRecordModel::doHandlepublishedOrSentRecord(CaDbRecord& aRecord,
                                                    const QByteArray& aSignature,
                                                    const QList<quint32>& aBangPath,
                                                    const Hash& aFromNode,
                                                    bool aWasPublish) {
    bool retval = false ;
    // ok, lets see .. if we have this particular record already in storage
    bool isRecordNew ( false ) ; 
    bool publicKeyWasFound ( false ) ; 
    quint32 timeOfPrevRecord ( 0 ) ; 
    this->isRecordNew(aRecord, &isRecordNew,&timeOfPrevRecord) ; 
    // it is possible that we already have a more recent version
    // of the record:
    if ( isRecordNew == false &&
         timeOfPrevRecord >= aRecord.iTimeOfPublish ) {
        // already got the record, or more recent record.
        return true ; 
    }
    // ok, lets see .. should we verify first.
    // first, if we have profile, we can verify.

    // secondly, if we for some odd reason do not have profile now
    // but have a record in data storage and it is verified to be
    // ok then lets not discard the old record ; this deliberate decision
    // here to treat as "more meaningful" a record that is old
    // but verified, compared new one that is new but one that we
    // can't verify


    QByteArray dummy ;
    if (! iController->model().contentEncryptionModel().PublicKey (aRecord.iSenderHash,
                                                                   dummy) ) {
        // key of sender was not found from data storage. if record is new,
        // go ahead and insert it as new, we just don't know if it was from
        // sender who claimed to have sent it
        publicKeyWasFound = false ; 
    } else {
        publicKeyWasFound = true ; 
    }
    // next step, verify is possible. 
    if ( publicKeyWasFound && 
         aRecord.iSenderHash != iController->profileInUse() ) {
        QList<QPair<Hash, Hash> > recordsToUpdateDueToGoodSignature ;     
        if ( tryVerifyRecord(aRecord,
                             aSignature,
                             recordsToUpdateDueToGoodSignature)  == false  ) {
            aRecord.iIsSignatureVerified = true ; 
        } else {
            // great, we should have been able to verify, verification
            // failed -> record is bad
            return false ;
        }
    } else {
        if ( aRecord.iSenderHash == iController->profileInUse() &&
             aWasPublish == true ) {
            aRecord.iIsSignatureVerified = true ; // trust self
        }
    }

    // ok, we're this far that verification is either done or not
    // done, put record into db and make a notification about new item
    // available
    QString persistErrorMessage (
        persistDbRecordIntoDb(aRecord,
                              &isRecordNew,
                              aSignature,
                              aFromNode.iHash160bits[4]) ) ; 
    if ( persistErrorMessage.isEmpty() ) {
        if ( aWasPublish ) {
            iController->model().addItemToBePublished(DbRecord,
                                                      aRecord.iRecordId,
                                                      aBangPath,
                                                      aRecord.iCollectionId ) ;
        }
        emit contentReceived(aRecord.iRecordId,
                             aRecord.iCollectionId,
                             DbRecord) ;
        retval = true ; 
    } else {
        QLOG_STR("CaDbRecordModel::persistDbRecordIntoDb: " +
                 persistErrorMessage) ;
        retval = false ;
    }

    return retval ;
}

void CaDbRecordModel::fillBucket(QList<SendQueueItem>& aSendQueue,
                                 const Hash& aStartOfBucket,
                                 const Hash& aEndOfBucket,
                                 quint32 aLastMutualConnectTime,
                                 const Hash& aForNode ) {
    QSqlQuery query;
    bool ret ;
    // bucket of database record is determined by collection hash
    ret = query.prepare ("select hash1,hash2,hash3,hash4,hash5,time_of_publish from dbrecord where collection_hash1 >= :start and collection_hash1 <= :end and time_of_publish > :last_connect_time and time_of_publish < :curr_time and ( recvd_from != :lowbits_of_requester or recvd_from is null ) order by time_of_publish desc " ) ;
    if ( ret ) {
        if ( aStartOfBucket == aEndOfBucket ) {
            // this was "small network situation" e.g. start and end
            // are the same ; in practice it means that send all data
            query.bindValue(":start", 0x0);
            query.bindValue(":end", 0xFFFFFFFF);
        } else {
            // normal situation
            query.bindValue(":start", aStartOfBucket.iHash160bits[0]);
            query.bindValue(":end", aEndOfBucket.iHash160bits[0]);
        }
        query.bindValue(":last_connect_time", aLastMutualConnectTime);
        query.bindValue(":lowbits_of_requester", aForNode.iHash160bits[4]);
        query.bindValue(":curr_time", QDateTime::currentDateTimeUtc().toTime_t());
    }
    ret = query.exec() ;
    if ( !ret ) {
        QLOG_STR(query.lastError().text() + " "+ __FILE__ + QString::number(__LINE__)) ;
        emit error(MController::DbTransactionError, query.lastError().text()) ;
    } else {
        while ( query.next() ) {
            Hash hashFoundFromDb ( query.value(0).toUInt(),
                                   query.value(1).toUInt(),
                                   query.value(2).toUInt(),
                                   query.value(3).toUInt(),
                                   query.value(4).toUInt()) ;
            SendQueueItem recordToSend ;
            recordToSend.iHash = hashFoundFromDb ;
            recordToSend.iItemType = DbRecord ;
            time_t timePublish(query.value(5).toUInt()) ;
            time_t timeMutual(aLastMutualConnectTime);
            aSendQueue.append(recordToSend) ;
#ifndef WIN32
            char formatted_mutual [100] = {0} ;
            char formatted_found [100]  = {0} ;
            ctime_r(&timePublish,&formatted_found[0] ) ;
            ctime_r(&timeMutual,&formatted_mutual[0] ) ;
            LOG_STR2("CaDbRecord bucket mutual %s", formatted_mutual) ;
            LOG_STR2("CaDbRecord bucket found %s", formatted_found) ;
#endif
        }
    }
    return  ; 
}

QString CaDbRecordModel::persistDbRecordIntoDb(const CaDbRecord &aRecord,
                                               bool* aIsKnownToBeNew,
                                               const QByteArray& aSignature,
                                               quint32 aReceivedFrom ) {
    bool isNew (false) ; 
    if ( aIsKnownToBeNew ) {
        isNew = *aIsKnownToBeNew ; 
    } else {
        QString errorMessage (
            this->isRecordNew(aRecord,
                              &isNew )) ; 
        if ( errorMessage.length() > 0 ) {
            return errorMessage ; 
        }
    }
    bool retval (false); 
    QSqlQuery query;
    QLOG_STR("CaDbRecordModel::persistDbRecordIntoDb is new = " + QString::number(isNew)) ; 
    if ( isNew ) {
        // record is new, prepare an insert-statement
        retval = query.prepare ("insert into dbrecord "
                                "(hash1,hash2,hash3,hash4,hash5,"
                                "collection_hash1,"
                                "collection_hash2,"
                                "collection_hash3,"
                                "collection_hash4,"
                                "collection_hash5,"
                                "sender_hash1,"
                                "sender_hash2,"
                                "sender_hash3,"
                                "sender_hash4,"
                                "sender_hash5,"
                                "time_last_reference,signature,time_of_publish,"
                                "data,recvd_from,searchstring,searchnumber,"
                                "isencrypted,issignatureverified) values "
                                "(:hash1,:hash2,:hash3,:hash4,:hash5,"
                                ":collection_hash1,"
                                ":collection_hash2,"
                                ":collection_hash3,"
                                ":collection_hash4,"
                                ":collection_hash5,"
                                ":sender_hash1,"
                                ":sender_hash2,"
                                ":sender_hash3,"
                                ":sender_hash4,"
                                ":sender_hash5,"
                                ":time_last_reference,:signature,:time_of_publish,"
                                ":data,:recvd_from,:searchstring,:searchnumber,"
                                ":isencrypted,:issignatureverified)" ) ;
    } else {
        // record is old, prepare an update
        retval = query.prepare ("update dbrecord set "
                                "collection_hash1 = :collection_hash1,"
                                "collection_hash2 = :collection_hash2,"
                                "collection_hash3 = :collection_hash3,"
                                "collection_hash4 = :collection_hash4,"
                                "collection_hash5 = :collection_hash5,"
                                "time_last_reference = :time_last_reference,"
                                "signature = :signature,"
                                "time_of_publish = :time_of_publish,"
                                "data = :data,recvd_from = :recvd_from,"
                                "searchstring = :searchstring,"
                                "searchnumber = :searchnumber,"
                                "recvd_from = :recvd_from,"
                                "isencrypted = :isencrypted"
                                "where hash1=:hash1 and hash2=:hash2 and "
                                "hash3=:hash3 and hash4=:hash4 and hash5=:hash5 and "
                                "sender_hash1 = :sender_hash1 and "
                                "sender_hash2 = :sender_hash2 and "
                                "sender_hash3 = :sender_hash3 and "
                                "sender_hash4 = :sender_hash4 and "
                                "sender_hash5 = :sender_hash5 " ) ;
    }
    QLOG_STR("CaDbRecordModel::persistDbRecordIntoDb after prepare retval = " + QString::number(retval) ) ; 
    if ( retval == false ) {
        QLOG_STR(query.lastError().text()) ; 
        return query.lastError().text() ; 
    }


    query.bindValue(":hash1", aRecord.iRecordId.iHash160bits[0]);
    query.bindValue(":hash2", aRecord.iRecordId.iHash160bits[1]);
    query.bindValue(":hash3", aRecord.iRecordId.iHash160bits[2]);
    query.bindValue(":hash4", aRecord.iRecordId.iHash160bits[3]);
    query.bindValue(":hash5", aRecord.iRecordId.iHash160bits[4]);
    query.bindValue(":collection_hash1", aRecord.iCollectionId.iHash160bits[0]);
    query.bindValue(":collection_hash2", aRecord.iCollectionId.iHash160bits[1]);
    query.bindValue(":collection_hash3", aRecord.iCollectionId.iHash160bits[2]);
    query.bindValue(":collection_hash4", aRecord.iCollectionId.iHash160bits[3]);
    query.bindValue(":collection_hash5", aRecord.iCollectionId.iHash160bits[4]);
    query.bindValue(":sender_hash1", aRecord.iSenderHash.iHash160bits[0]);
    query.bindValue(":sender_hash2", aRecord.iSenderHash.iHash160bits[1]);
    query.bindValue(":sender_hash3", aRecord.iSenderHash.iHash160bits[2]);
    query.bindValue(":sender_hash4", aRecord.iSenderHash.iHash160bits[3]);
    query.bindValue(":sender_hash5", aRecord.iSenderHash.iHash160bits[4]);

    query.bindValue(":time_last_reference", QDateTime::currentDateTimeUtc().toTime_t());
    query.bindValue(":time_of_publish", aRecord.iTimeOfPublish);
    QString kiloByteMax ( aRecord.iSearchPhrase.left(1024) ) ; // actual column size is 4k
    if ( kiloByteMax.toUtf8().length() > 4096 ) {
        query.bindValue(":searchstring", QByteArray());
    } else {
        query.bindValue(":searchstring", kiloByteMax.toUtf8());
    }
    query.bindValue(":searchnumber", aRecord.iSearchNumber);
    query.bindValue(":data", aRecord.iData);
    query.bindValue(":isencrypted", aRecord.iIsEncrypted);
    query.bindValue(":signature", aSignature);
    query.bindValue(":issignatureverified", aRecord.iIsSignatureVerified);
    query.bindValue(":recvd_from", aReceivedFrom);
    
    if ( query.exec()==false ){
        QLOG_STR("query.exec == false") ; 
        return query.lastError().text() ; 
    } else {
        QLOG_STR("query.exec == true") ; 
        updateFTS(aRecord,isNew) ; 
        return QString::null ; // as a sign of success, return empty string
    }
}


QString CaDbRecordModel::isRecordNew(const CaDbRecord &aRecord,
                                     bool* aResult,
                                     quint32* aTimestampOfOldRecord ) {
    if ( aResult == NULL ) {
        return "No pointer?" ;
    }
    *aResult = false ; 
    bool retval ( false ) ; 
    // need to figure out ourselves
    QSqlQuery query ;
    retval = query.prepare("select count(hash1),max(time_of_publish) from dbrecord where "
                           "hash1 = :hash1 and "
                           "hash2 = :hash2 and "
                           "hash3 = :hash3 and "
                           "hash4 = :hash4 and "
                           "hash5 = :hash5 and "
                           "sender_hash1 = :sender_hash1 and "
                           "sender_hash2 = :sender_hash2 and "
                           "sender_hash3 = :sender_hash3 and "
                           "sender_hash4 = :sender_hash4 and "
                           "sender_hash5 = :sender_hash5 ") ; 
    query.bindValue(":hash1", aRecord.iRecordId.iHash160bits[0]);
    query.bindValue(":hash2", aRecord.iRecordId.iHash160bits[1]);
    query.bindValue(":hash3", aRecord.iRecordId.iHash160bits[2]);
    query.bindValue(":hash4", aRecord.iRecordId.iHash160bits[3]);
    query.bindValue(":hash5", aRecord.iRecordId.iHash160bits[4]); 
    query.bindValue(":sender_hash1", aRecord.iSenderHash.iHash160bits[0]);
    query.bindValue(":sender_hash2", aRecord.iSenderHash.iHash160bits[1]);
    query.bindValue(":sender_hash3", aRecord.iSenderHash.iHash160bits[2]);
    query.bindValue(":sender_hash4", aRecord.iSenderHash.iHash160bits[3]);
    query.bindValue(":sender_hash5", aRecord.iSenderHash.iHash160bits[4]); 
    retval = query.exec() ;
    if ( !retval ) {
        QLOG_STR(query.lastError().text() + " "+ __FILE__ + QString::number(__LINE__)) ;
        return query.lastError().text() ; 
    } else {
        if ( query.next() && !query.isNull(0) ) {
            if ( query.value(0).toInt() == 0 ) {
                *aResult = true ; // it was not in db already 
                if ( aTimestampOfOldRecord ) { 
                    *aTimestampOfOldRecord = 0 ; 
                }
            } else {
                if ( aTimestampOfOldRecord &&
                     !query.isNull(1)) {
                    quint32 maxTimeOfPublish ( query.value(1).toUInt() ) ; 
                    *aTimestampOfOldRecord = maxTimeOfPublish ; 
                }
            }
        }
    }
    return QString::null ; // success
}

bool CaDbRecordModel::tryVerifyRecord(CaDbRecord &aRecord,
                                      const QByteArray& aSignature,
                                      QList<QPair<Hash,Hash> >& aListOfGoodSignatures ) {
    QByteArray key ; 
    quint32 timeStampOfPossibleExistingProfile ( 0 ) ;
    if ( iController->model()
         .contentEncryptionModel()
         .PublicKey(aRecord.iSenderHash,
                    key,
                    &timeStampOfPossibleExistingProfile ) == true ) {
        // yes, we have key and can verify
        QString additionalMetaDataStringToSign( // see similar code in publish method
            aRecord.iRecordId.toString() + 
            aRecord.iCollectionId.toString() + 
            aRecord.iSenderHash.toString() ) ; 
        QByteArray additionalMetaDataBytesToSign( additionalMetaDataStringToSign.toUtf8()) ; 
        if( iController->model()
            .contentEncryptionModel()
            .verify(aRecord.iSenderHash, 
                    aRecord.iData, 
                    aSignature,
                    &additionalMetaDataBytesToSign,
                    false ) ) {
            aListOfGoodSignatures .append(QPair<Hash,Hash>(aRecord.iRecordId,aRecord.iSenderHash)) ; 
            aRecord.iIsSignatureVerified = true ; 
            return true ; 
        } else {
            return false ; 
        }
    } else {
        // no profile key found
        return true ; // can't verify, return true anyway
    }
}

void CaDbRecordModel::deleteRecord(const Hash& aRecordId,
                                   const Hash& aSenderId) {
    QSqlQuery query;
    query.prepare("delete from  " + iDataTableName + "  where  "
                  "hash1 = :hash1 and "
                  "hash2 = :hash2 and "
                  "hash3 = :hash3 and "
                  "hash4 = :hash4 and "
                  "hash5 = :hash5 and "
                  "sender_hash1 = :sender_hash1 and "
                  "sender_hash2 = :sender_hash2 and "
                  "sender_hash3 = :sender_hash3 and "
                  "sender_hash4 = :sender_hash4 and "
                  "sender_hash5 = :sender_hash5 " ) ; 
    
    query.bindValue(":hash1", aRecordId.iHash160bits[0]);
    query.bindValue(":hash2", aRecordId.iHash160bits[1]);
    query.bindValue(":hash3", aRecordId.iHash160bits[2]);
    query.bindValue(":hash4", aRecordId.iHash160bits[3]);
    query.bindValue(":hash5", aRecordId.iHash160bits[4]); 
    query.bindValue(":sender_hash1", aSenderId.iHash160bits[0]);
    query.bindValue(":sender_hash2", aSenderId.iHash160bits[1]);
    query.bindValue(":sender_hash3", aSenderId.iHash160bits[2]);
    query.bindValue(":sender_hash4", aSenderId.iHash160bits[3]);
    query.bindValue(":sender_hash5", aSenderId.iHash160bits[4]); 

    if ( query.exec() == false ) {
        QLOG_STR(query.lastError().text() + " "+ __FILE__ + QString::number(__LINE__)) ;
    }
}

void CaDbRecordModel::updateRecordVerification(const Hash& aRecordId,
                                               const Hash& aSenderId,
                                               bool aNewVerificationStatus) {
    QSqlQuery query;
    query.prepare("update " + iDataTableName + " set issignatureverified = :s where  "
                  "hash1 = :hash1 and "
                  "hash2 = :hash2 and "
                  "hash3 = :hash3 and "
                  "hash4 = :hash4 and "
                  "hash5 = :hash5 and "
                  "sender_hash1 = :sender_hash1 and "
                  "sender_hash2 = :sender_hash2 and "
                  "sender_hash3 = :sender_hash3 and "
                  "sender_hash4 = :sender_hash4 and "
                  "sender_hash5 = :sender_hash5 " ) ; 
    
    query.bindValue(":s", aNewVerificationStatus);
    query.bindValue(":hash1", aRecordId.iHash160bits[0]);
    query.bindValue(":hash2", aRecordId.iHash160bits[1]);
    query.bindValue(":hash3", aRecordId.iHash160bits[2]);
    query.bindValue(":hash4", aRecordId.iHash160bits[3]);
    query.bindValue(":hash5", aRecordId.iHash160bits[4]); 
    query.bindValue(":sender_hash1", aSenderId.iHash160bits[0]);
    query.bindValue(":sender_hash2", aSenderId.iHash160bits[1]);
    query.bindValue(":sender_hash3", aSenderId.iHash160bits[2]);
    query.bindValue(":sender_hash4", aSenderId.iHash160bits[3]);
    query.bindValue(":sender_hash5", aSenderId.iHash160bits[4]); 

    if ( query.exec() == false ) {
        QLOG_STR(query.lastError().text() + " "+ __FILE__ + QString::number(__LINE__)) ;
    }    
}

void CaDbRecordModel::updateFTS(const CaDbRecord& aRecord, 
                                bool aIsNewRecord ) {
    if ( iModel.searchModel()->isFTSSupported() == false ) {
        return ; 
    }
    if ( aIsNewRecord && aRecord.iSearchPhrase.isEmpty()) {
        return ; 
    }
    QSqlQuery q ; 
    bool operation_success ( false ) ; 
    if ( aIsNewRecord == false ) {
        operation_success= q.prepare("update dbrecord_search set "
                                       "searchstring=:searchstring where docid=:hash1") ;
    } else {
        operation_success= q.prepare("insert into dbrecord_search("
                                     "docid,searchstring) values ("
                                     ":hash1,:searchstring)") ;
    }
    if ( operation_success ) {
        q.bindValue(":hash1", aRecord.iRecordId.iHash160bits[0]);
        q.bindValue(":searchstring", aRecord.iSearchPhrase.toUtf8()) ;
        if ( q.exec() == false ) {
            QLOG_STR(q.lastError().text()) ; 
        }
    } else {
        QLOG_STR(q.lastError().text()) ; 
    }
    return ; 
}

