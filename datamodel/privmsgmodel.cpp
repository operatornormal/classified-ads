/*     -*-C++-*- -*-coding: utf-8-unix;-*-
  Classified Ads is Copyright (c) Antti Järvinen 2013-2018.

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

#include "privmsgmodel.h"
#include "../log.h"
#include "../util/hash.h"
#include "model.h"
#include <QSqlQuery>
#include <QSqlError>
#include "privmsg.h"
#include "contentencryptionmodel.h"
#include "const.h"

PrivMessageModel::PrivMessageModel(MController *aController,
                                   MModelProtocolInterface &aModel) :
    ModelBase("private_message",KMaxRowsInTablePrivate_Message,aController,aModel) {
    LOG_STR("PrivMessageModel::PrivMessageModel()") ;
    connect(this,
            SIGNAL(  error(MController::CAErrorSituation,
                           const QString&) ),
            iController,
            SLOT(handleError(MController::CAErrorSituation,
                             const QString&)),
            Qt::QueuedConnection ) ;

}



PrivMessageModel::~PrivMessageModel() {
    LOG_STR("PrivMessageModel::~PrivMessageModel()") ;
    iController = NULL ; // not owned, just set null
}

bool PrivMessageModel::publishPrivMessage(PrivMessage& aPrivMessage,
        const Hash& aDestinationNode) {
    LOG_STR("PrivMessageModel::publishPrivMessage()") ;
    Hash messageFingerPrint ;
    bool retval = false ;
    QByteArray messageJSon ( aPrivMessage.asJSon(*iController) ) ;
    QList<Hash> keysToEncryptMessageTo ;
    keysToEncryptMessageTo << aPrivMessage.iRecipient ;
    keysToEncryptMessageTo << aPrivMessage.iSenderHash ; // so that sender can also read later

    for ( int i = 0 ; i < keysToEncryptMessageTo.size() ; i++ ) {
        QLOG_STR("keysToEncryptMessageTo["+QString::number(i)+"]="+keysToEncryptMessageTo[i].toString()) ;
    }
    QByteArray encryptedJson ;
    // practically this also means that before we can send a private
    // message to any profile, the profile key must somehow magically
    // be stored in db.. this should be the case in every situation?
    if ( iController->model().contentEncryptionModel().encrypt(keysToEncryptMessageTo,
            messageJSon,
            encryptedJson ) ) {
        messageJSon.clear() ;
        messageJSon.append(encryptedJson) ;
        encryptedJson.clear() ;
        // so ; now ; the message json is encrypted -> proceed by making signature
        // of that
    } else {
        return false ; // encryption failed ?
    }

    if ( messageJSon.length() > 0 ) {
        QByteArray digitalSignature ;
        messageFingerPrint.calculate(messageJSon) ;

        if ( iController->model().contentEncryptionModel().sign(aPrivMessage.iSenderHash, messageJSon, digitalSignature) == 0 ) {
            QSqlQuery query(iModel.dataBaseConnection());
            retval = query.prepare ("insert into private_message("
                                    "hash1,hash2,hash3,hash4,hash5,"
                                    "dnode_hash1,dnode_hash2,dnode_hash3,dnode_hash4,dnode_hash5,"
                                    "recipient_hash1,recipient_hash2,recipient_hash3,recipient_hash4,recipient_hash5,"
                                    "time_last_reference,signature,"
                                    "time_of_publish,messagedata,"
                                    "sender_hash1,is_read) values ("
                                    ":hash1,:hash2,:hash3,:hash4,:hash5,"
                                    ":dnode_hash1,:dnode_hash2,:dnode_hash3,:dnode_hash4,:dnode_hash5,"
                                    ":recipient_hash1,:recipient_hash2,:recipient_hash3,:recipient_hash4,:recipient_hash5,"
                                    ":time_last_reference,:signature,"
                                    ":time_of_publish,:messagedata,"
                                    ":sender_hash1,1)") ;
            if ( retval ) {
                query.bindValue(":hash1", messageFingerPrint.iHash160bits[0]);
                query.bindValue(":hash2", messageFingerPrint.iHash160bits[1]);
                query.bindValue(":hash3", messageFingerPrint.iHash160bits[2]);
                query.bindValue(":hash4", messageFingerPrint.iHash160bits[3]);
                query.bindValue(":hash5", messageFingerPrint.iHash160bits[4]);
                if ( aDestinationNode != KNullHash ) {
                    query.bindValue(":dnode_hash1", aDestinationNode.iHash160bits[0]);
                    query.bindValue(":dnode_hash2", aDestinationNode.iHash160bits[1]);
                    query.bindValue(":dnode_hash3", aDestinationNode.iHash160bits[2]);
                    query.bindValue(":dnode_hash4", aDestinationNode.iHash160bits[3]);
                    query.bindValue(":dnode_hash5", aDestinationNode.iHash160bits[4]);
                } else {
                    query.bindValue(":dnode_hash1", QVariant(QVariant::String)) ; //null
                    query.bindValue(":dnode_hash2", QVariant(QVariant::String)) ; //value
                    query.bindValue(":dnode_hash3", QVariant(QVariant::String)) ;
                    query.bindValue(":dnode_hash4", QVariant(QVariant::String)) ;
                    query.bindValue(":dnode_hash5", QVariant(QVariant::String)) ;
                }
                query.bindValue(":recipient_hash1", aPrivMessage.iRecipient.iHash160bits[0]);
                query.bindValue(":recipient_hash2", aPrivMessage.iRecipient.iHash160bits[1]);
                query.bindValue(":recipient_hash3", aPrivMessage.iRecipient.iHash160bits[2]);
                query.bindValue(":recipient_hash4", aPrivMessage.iRecipient.iHash160bits[3]);
                query.bindValue(":recipient_hash5", aPrivMessage.iRecipient.iHash160bits[4]);

                query.bindValue(":time_last_reference", aPrivMessage.iTimeOfPublish);
                query.bindValue(":messagedata", messageJSon);
                query.bindValue(":signature", digitalSignature);
                query.bindValue(":time_of_publish", aPrivMessage.iTimeOfPublish);
                query.bindValue(":sender_hash1", aPrivMessage.iSenderHash.iHash160bits[0]);
                retval = query.exec() ;
            }

            if ( !retval ) {
                LOG_STR2("Error while private message insert %s", qPrintable(query.lastError().text())) ;
                emit error(MController::DbTransactionError, query.lastError().text()) ;
            } else {
                // here user is publishing as "brand new" so set bangpath to 0
                QList<quint32> emptyBangPath ;
                iController->model().addItemToBePublished(PrivateMessage,
                        messageFingerPrint,
                        emptyBangPath,
                        aDestinationNode ) ;
                iCurrentDbTableRowCount++ ;
            }
        }
    }
    if ( retval ) {
        aPrivMessage.iFingerPrint = messageFingerPrint ;
    }
    return retval ;
}

PrivMessage* PrivMessageModel::messageByFingerPrint(const Hash& aFingerPrint) {
    LOG_STR("PrivMessageModel::messageByFingerPrint()") ;
    PrivMessage* retval = NULL ;
    QSqlQuery query(iModel.dataBaseConnection());
    bool ret ;
    ret = query.prepare ("select messagedata,signature from private_message where hash1 = :hash1 and hash2 = :hash2 and hash3 = :hash3 and hash4 = :hash4 and hash5 = :hash5" ) ;
    if ( ret ) {
        query.bindValue(":hash1", aFingerPrint.iHash160bits[0]);
        query.bindValue(":hash2", aFingerPrint.iHash160bits[1]);
        query.bindValue(":hash3", aFingerPrint.iHash160bits[2]);
        query.bindValue(":hash4", aFingerPrint.iHash160bits[3]);
        query.bindValue(":hash5", aFingerPrint.iHash160bits[4]);
    }
    ret = query.exec() ;
    if ( !ret ) {
        QLOG_STR(query.lastError().text() + " "+ __FILE__ + QString::number(__LINE__)) ;
        emit error(MController::DbTransactionError, query.lastError().text()) ;
    } else {
        if ( query.next() && !query.isNull(0) && !query.isNull(1) ) {
            QByteArray messageData ( query.value(0).toByteArray()) ;
            QByteArray signature ( query.value(1).toByteArray()) ;


            QByteArray plainTextPrivMessageData ;
            retval = new PrivMessage() ;
            if ( iController->model().contentEncryptionModel().decrypt(messageData,
                    plainTextPrivMessageData ) ) {
                LOG_STR2("message: %s", qPrintable(QString(plainTextPrivMessageData)));
                ret = retval->fromJSon(plainTextPrivMessageData,*iController) ;
            } else {
                delete retval ;
                retval = NULL ;
                ret = false ; 
            }

            if ( ret )  {
                // hmm hmm. public key of the sender is inside the message
                // itself. it is possible that we do not have the key in
                // advance so the verification process may need it. verification
                // still ensures message integrity but origin of the message
                // surely is now unknown.
                QByteArray key ;
                quint32 timeStampOfPossibleExistingProfile ( 0 ) ;
                if ( iController->model().contentEncryptionModel().PublicKey(retval->iSenderHash,
                        key,
                        &timeStampOfPossibleExistingProfile ) == false ) {
                    // we did not have the key: store it
                    iController->model().contentEncryptionModel().insertOrUpdatePublicKey(retval->iProfileKey,retval->iSenderHash) ;
                }

                if ( iController->model().contentEncryptionModel().verify(retval->iSenderHash, messageData, signature) == false ) {
                    // bad signature
                    ret = false ;
                    delete retval ;
                    retval = NULL ;
                } else {
                    // good signature
                    retval->iFingerPrint = aFingerPrint;
                }
            }
        }
    }
    return retval ;
}

bool PrivMessageModel::messageDataByFingerPrint(const Hash& aFingerPrint,
        QByteArray& aResultingPrivMessageData,
        QByteArray& aResultingSignature,

        Hash* aResultingDestinationNode,
        Hash* aResultingRecipient,
        quint32* aTimeOfPublish) {
    LOG_STR("PrivMessageModel::messageDataByFingerPrint()") ;
    bool retval = false ;
    QSqlQuery query(iModel.dataBaseConnection());
    bool ret ;
    ret = query.prepare ("select messagedata,signature,time_of_publish,dnode_hash1,dnode_hash2,dnode_hash3,dnode_hash4,dnode_hash5,recipient_hash1,recipient_hash2,recipient_hash3,recipient_hash4,recipient_hash5 from private_message where hash1 = :hash1 and hash2 = :hash2 and hash3 = :hash3 and hash4 = :hash4 and hash5 = :hash5" ) ;
    if ( ret ) {
        query.bindValue(":hash1", aFingerPrint.iHash160bits[0]);
        query.bindValue(":hash2", aFingerPrint.iHash160bits[1]);
        query.bindValue(":hash3", aFingerPrint.iHash160bits[2]);
        query.bindValue(":hash4", aFingerPrint.iHash160bits[3]);
        query.bindValue(":hash5", aFingerPrint.iHash160bits[4]);
    }
    ret = query.exec() ;
    if ( !ret ) {
        QLOG_STR(query.lastError().text() + " "+ __FILE__ + QString::number(__LINE__)) ;
        emit error(MController::DbTransactionError, query.lastError().text()) ;
    } else {
        if ( query.next() && !query.isNull(0) && !query.isNull(1) ) {
            aResultingPrivMessageData.clear() ;
            aResultingSignature.clear() ;
            aResultingPrivMessageData.append ( query.value(0).toByteArray()) ;
            aResultingSignature.append ( query.value(1).toByteArray()) ;
            if ( !query.isNull(2) && aTimeOfPublish != NULL ) {
                *aTimeOfPublish = query.value(2).toUInt() ;
            }
            if ( aResultingDestinationNode != NULL &&
                    !query.isNull(3) &&
                    !query.isNull(4) &&
                    !query.isNull(5) &&
                    !query.isNull(6) &&
                    !query.isNull(7) ) {
                aResultingDestinationNode->iHash160bits[0] = query.value(3).toUInt() ;
                aResultingDestinationNode->iHash160bits[1] = query.value(4).toUInt() ;
                aResultingDestinationNode->iHash160bits[2] = query.value(5).toUInt() ;
                aResultingDestinationNode->iHash160bits[3] = query.value(6).toUInt() ;
                aResultingDestinationNode->iHash160bits[4] = query.value(7).toUInt() ;
            } else {
                if ( aResultingDestinationNode ) {
                    *aResultingDestinationNode  = KNullHash ;
                }
            }

            if ( aResultingRecipient != NULL  ) { // in db receiver hash is not null
                aResultingRecipient->iHash160bits[0] = query.value(8).toUInt() ;
                aResultingRecipient->iHash160bits[1] = query.value(9).toUInt() ;
                aResultingRecipient->iHash160bits[2] = query.value(10).toUInt() ;
                aResultingRecipient->iHash160bits[3] = query.value(11).toUInt() ;
                aResultingRecipient->iHash160bits[4] = query.value(12).toUInt() ;
            }

            retval = true ;
        }
    }
    return retval ;
}

bool PrivMessageModel::sentPrivMessageReceived(const Hash& aFingerPrint,
        const QByteArray& aContent,
        const QByteArray& aSignature,
        const Hash& aDestinationNode,
        const Hash& aRecipient,
        const quint32 aTimeStamp,
        const Hash& aFromNode ) {
    const QList<quint32> dummy ;
    return doHandlepublishedOrSentPrivMessage(aFingerPrint,
            aContent,
            aSignature,
            dummy,
            aDestinationNode,
            aRecipient,
            aTimeStamp,
            false,
            aFromNode) ;
}


bool PrivMessageModel::publishedPrivMessageReceived(const Hash& aFingerPrint,
        const QByteArray& aContent,
        const QByteArray& aSignature,
        const QList<quint32>& aBangPath,
        const Hash& aDestinationNode,
        const Hash& aRecipient,
        const quint32 aTimeStamp,
        const Hash& aFromNode ) {
    return doHandlepublishedOrSentPrivMessage(aFingerPrint,
            aContent,
            aSignature,
            aBangPath,
            aDestinationNode,
            aRecipient,
            aTimeStamp,
            true,
            aFromNode ) ;
}


bool PrivMessageModel::doHandlepublishedOrSentPrivMessage(const Hash& aFingerPrint,
        const QByteArray& aContent,
        const QByteArray& aSignature,
        const QList<quint32>& aBangPath,
        const Hash& aDestinationNode,
        const Hash& aRecipient,
        const quint32 aTimeStamp,
        bool aWasPublish,
        const Hash& aFromNode ) {
    bool retval = false ;
    QByteArray key ;
    bool flagThisNodeAlreadyHadContent ( false ) ;

    // messages do not change nor get re-published. either we have
    // it or do not. if we did not, save it.

    QSqlQuery query(iModel.dataBaseConnection());
    bool ret ;
    ret = query.prepare ("select hash1 from private_message where hash1 = :hash1 and hash2 = :hash2 and hash3 = :hash3 and hash4 = :hash4 and hash5 = :hash5" ) ;
    if ( ret ) {
        query.bindValue(":hash1", aFingerPrint.iHash160bits[0]);
        query.bindValue(":hash2", aFingerPrint.iHash160bits[1]);
        query.bindValue(":hash3", aFingerPrint.iHash160bits[2]);
        query.bindValue(":hash4", aFingerPrint.iHash160bits[3]);
        query.bindValue(":hash5", aFingerPrint.iHash160bits[4]);
    }
    ret = query.exec() ;
    if ( !ret ) {
        QLOG_STR(query.lastError().text() + " "+ __FILE__ + QString::number(__LINE__)) ;
        emit error(MController::DbTransactionError, query.lastError().text()) ;
    } else {
        if ( query.next() && !query.isNull(0) ) {
            flagThisNodeAlreadyHadContent = true ;
            retval = true ; // this is normal situation, report as success
        }
    }

    if ( ret && flagThisNodeAlreadyHadContent == false ) {
        // store the message


        QSqlQuery query2(iModel.dataBaseConnection());
        retval = query2.prepare ("insert into private_message("
                                 "hash1,hash2,hash3,hash4,hash5,"
                                 "dnode_hash1,dnode_hash2,dnode_hash3,dnode_hash4,dnode_hash5,"
                                 "recipient_hash1,recipient_hash2,recipient_hash3,recipient_hash4,recipient_hash5,"
                                 "time_last_reference,signature,"
                                 "time_of_publish,messagedata,recvd_from) values ("
                                 ":hash1,:hash2,:hash3,:hash4,:hash5,"
                                 ":dnode_hash1,:dnode_hash2,:dnode_hash3,:dnode_hash4,:dnode_hash5,"
                                 ":recipient_hash1,:recipient_hash2,:recipient_hash3,:recipient_hash4,:recipient_hash5,"
                                 ":time_last_reference,:signature,"
                                 ":time_of_publish,:messagedata,:recvd_from)") ;

        if ( retval ) {
            query2.bindValue(":hash1", aFingerPrint.iHash160bits[0]);
            query2.bindValue(":hash2", aFingerPrint.iHash160bits[1]);
            query2.bindValue(":hash3", aFingerPrint.iHash160bits[2]);
            query2.bindValue(":hash4", aFingerPrint.iHash160bits[3]);
            query2.bindValue(":hash5", aFingerPrint.iHash160bits[4]);
            query2.bindValue(":recvd_from", aFromNode.iHash160bits[4]);
            if ( aDestinationNode != KNullHash ) {
                query2.bindValue(":dnode_hash1", aDestinationNode.iHash160bits[0]);
                query2.bindValue(":dnode_hash2", aDestinationNode.iHash160bits[1]);
                query2.bindValue(":dnode_hash3", aDestinationNode.iHash160bits[2]);
                query2.bindValue(":dnode_hash4", aDestinationNode.iHash160bits[3]);
                query2.bindValue(":dnode_hash5", aDestinationNode.iHash160bits[4]);
            } else {
                query2.bindValue(":dnode_hash1", QVariant(QVariant::String)) ; //null
                query2.bindValue(":dnode_hash2", QVariant(QVariant::String)) ; //value
                query2.bindValue(":dnode_hash3", QVariant(QVariant::String)) ;
                query2.bindValue(":dnode_hash4", QVariant(QVariant::String)) ;
                query2.bindValue(":dnode_hash5", QVariant(QVariant::String)) ;
            }
            query2.bindValue(":recipient_hash1", aRecipient.iHash160bits[0]);
            query2.bindValue(":recipient_hash2", aRecipient.iHash160bits[1]);
            query2.bindValue(":recipient_hash3", aRecipient.iHash160bits[2]);
            query2.bindValue(":recipient_hash4", aRecipient.iHash160bits[3]);
            query2.bindValue(":recipient_hash5", aRecipient.iHash160bits[4]);

            query2.bindValue(":time_last_reference", QDateTime::currentDateTimeUtc().toTime_t());
            query2.bindValue(":messagedata", aContent);
            query2.bindValue(":signature", aSignature);
            query2.bindValue(":time_of_publish", aTimeStamp);
            retval = query2.exec() ;
        }

    } // if we already had the content or not

    if ( retval && ( flagThisNodeAlreadyHadContent == false ) ) {
        emit contentReceived(aFingerPrint,
                             aRecipient,
                             PrivateMessage) ;
        iCurrentDbTableRowCount++ ;
    }
    if ( aWasPublish ) {
        if ( retval && (flagThisNodeAlreadyHadContent == false) ) {
            iController->model().addItemToBePublished(PrivateMessage,
                    aFingerPrint,
                    aBangPath,
                    aDestinationNode ) ;
        }
    }
    LOG_STR2("PrivMessageModel::published/sentPrivMessageReceived out success = %d",
             (int)retval) ;
    return retval ;
}

void PrivMessageModel::fillBucket(QList<SendQueueItem>& aSendQueue,
                                  const Hash& aStartOfBucket,
                                  const Hash& aEndOfBucket,
                                  quint32 aLastMutualConnectTime,
                                  const Hash& aForNode ) {
    QSqlQuery query(iModel.dataBaseConnection());
    bool ret ;
    // note how the private messages are placed into buckets
    // based on recipient hash, not by hash of the content
    // (like almost all other content)
    ret = query.prepare ("select hash1,hash2,hash3,hash4,hash5,time_of_publish from private_message where ( recipient_hash1 >= :start and recipient_hash1 <= :end and time_of_publish > :last_connect_time and time_of_publish < :curr_time and ( recvd_from != :lowbits_of_requester or recvd_from is null ) ) or ( dnode_hash1=:dnode_hash1 and dnode_hash2=:dnode_hash2 and dnode_hash3=:dnode_hash3 and dnode_hash4=:dnode_hash4 and dnode_hash5=:dnode_hash5 and time_of_publish > :last_connect_time2 and time_of_publish < :curr_time2 ) order by time_of_publish desc limit 1000" ) ;
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
        query.bindValue(":last_connect_time2", aLastMutualConnectTime);
        query.bindValue(":lowbits_of_requester", aForNode.iHash160bits[4]);
        query.bindValue(":curr_time", QDateTime::currentDateTimeUtc().addSecs(5*60).toTime_t());
        query.bindValue(":curr_time2", QDateTime::currentDateTimeUtc().addSecs(5*60).toTime_t());
        query.bindValue(":dnode_hash1", aForNode.iHash160bits[0]);
        query.bindValue(":dnode_hash2", aForNode.iHash160bits[1]);
        query.bindValue(":dnode_hash3", aForNode.iHash160bits[2]);
        query.bindValue(":dnode_hash4", aForNode.iHash160bits[3]);
        query.bindValue(":dnode_hash5", aForNode.iHash160bits[4]);
    }
    ret = query.exec() ;
    if ( !ret ) {
        QLOG_STR(query.lastError().text() + " "+ __FILE__ + QString::number(__LINE__));
        emit error(MController::DbTransactionError, query.lastError().text()) ;
    } else {
        while ( query.next() ) {
            Hash hashFoundFromDb ( query.value(0).toUInt(),
                                   query.value(1).toUInt(),
                                   query.value(2).toUInt(),
                                   query.value(3).toUInt(),
                                   query.value(4).toUInt()) ;
            SendQueueItem privMsgToSend ;
            privMsgToSend.iHash = hashFoundFromDb ;
            privMsgToSend.iItemType = PrivateMessage ;
            time_t timePublish(query.value(5).toUInt()) ;
            time_t timeMutual(aLastMutualConnectTime);
            aSendQueue.append(privMsgToSend) ;
            char formatted_mutual [100] = {0} ;
            char formatted_found [100]  = {0} ;
#ifndef WIN32
            ctime_r(&timePublish,&formatted_found[0] ) ;
            ctime_r(&timeMutual,&formatted_mutual[0] ) ;
            LOG_STR2("PrivMessage bucket mutual %s", formatted_mutual) ;
            LOG_STR2("PrivMessage bucket found %s", formatted_found) ;
#endif
        }
    }
}

QList<Hash> PrivMessageModel::messagesForProfile(const Hash& aProfileHash,
        const quint32 aLastMutualConnectTime) {
    QList<Hash> retval ;

    QSqlQuery query(iModel.dataBaseConnection());
    bool ret ;
    // note how the private messages are placed into buckets
    // based on recipient hash, not by hash of the content
    // (like almost all other content)
    ret = query.prepare ("select hash1,hash2,hash3,hash4,hash5 from private_message where  recipient_hash1 = :h1 and recipient_hash2 = :h2 and recipient_hash3 = :h3 and recipient_hash4 = :h4 and recipient_hash5 = :h5 and time_of_publish > :time_of_publish" ) ;
    if ( ret ) {
        query.bindValue(":h1", aProfileHash.iHash160bits[0]);
        query.bindValue(":h2", aProfileHash.iHash160bits[1]);
        query.bindValue(":h3", aProfileHash.iHash160bits[2]);
        query.bindValue(":h4", aProfileHash.iHash160bits[3]);
        query.bindValue(":h5", aProfileHash.iHash160bits[4]);
        query.bindValue(":time_of_publish", aLastMutualConnectTime);
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
                retval.append(hashFoundFromDb) ;
            }
        }
    } else {
        QLOG_STR(query.lastError().text() + " "+ __FILE__ + QString::number(__LINE__)) ;
        emit error(MController::DbTransactionError, query.lastError().text()) ;
    }
    QLOG_STR("Returning list of private message len = " + QString::number(retval.size()) + " for profile " + aProfileHash.toString() + " since timestamp " + QString::number(aLastMutualConnectTime)) ;
    return retval ;
}

void PrivMessageModel::setAsRead(const Hash& aMessageHash, bool aIsRead ) {
    bool retval (false) ;
    QSqlQuery query(iModel.dataBaseConnection());
    retval = query.prepare ("update private_message set is_read = :is_read, time_last_reference=:time_last_reference where hash1 = :hash1 and hash2 = :hash2 and hash3 = :hash3 and hash4 = :hash4 and hash5 = :hash5" ) ;
    if ( retval ) {
        query.bindValue(":hash1", aMessageHash.iHash160bits[0]);
        query.bindValue(":hash2", aMessageHash.iHash160bits[1]);
        query.bindValue(":hash3", aMessageHash.iHash160bits[2]);
        query.bindValue(":hash4", aMessageHash.iHash160bits[3]);
        query.bindValue(":hash5", aMessageHash.iHash160bits[4]);
        query.bindValue(":is_read", ( aIsRead == true ? 1 : 0 ) );
        query.bindValue(":time_last_reference", QDateTime::currentDateTimeUtc().toTime_t());
        retval = query.exec() ;
    }
    if ( !retval ) {
        QLOG_STR(query.lastError().text() + " "+ __FILE__ + QString::number(__LINE__)) ;
        emit error(MController::DbTransactionError, query.lastError().text()) ;
    }
}

