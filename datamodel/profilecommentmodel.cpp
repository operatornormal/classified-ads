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

#include "profilecommentmodel.h"
#include "profilemodel.h" // we'll need that also
#include "profile.h" // profilemodel deals with profile so we need to include that
#include "../log.h"
#include "../util/hash.h"
#include "model.h"
#include <QSqlQuery>
#include <QSqlError>
#include "profilecomment.h"
#include "contentencryptionmodel.h"
#ifdef WIN32
#include <QJson/Parser>
#include <QJson/Serializer>
#else
#include <qjson/parser.h>
#include <qjson/serializer.h>
#endif
#include "const.h"
#include "searchmodel.h"

ProfileCommentModel::ProfileCommentModel(MController *aController,
        const MModelProtocolInterface &aModel)
    : ModelBase("profilecomment",KMaxRowsInTableProfileComment),
      iController(aController),
      iModel(aModel) {
    LOG_STR("ProfileCommentModel::ProfileCommentModel()") ;
    connect(this,
            SIGNAL(  error(MController::CAErrorSituation,
                           const QString&) ),
            iController,
            SLOT(handleError(MController::CAErrorSituation,
                             const QString&)),
            Qt::QueuedConnection ) ;

}



ProfileCommentModel::~ProfileCommentModel() {
    LOG_STR("ProfileCommentModel::~ProfileCommentModel()") ;
    iController = NULL ; // not owned, just set null
}

bool ProfileCommentModel::publishProfileComment(ProfileComment& aProfileComment) {
    LOG_STR("ProfileCommentModel::publishProfileComment()") ;
    bool retval = false ;
    QByteArray commentJSon ( aProfileComment.asJSon(*iController) ) ;
    if ( aProfileComment.iIsPrivate ) {
        QByteArray encryptedJson ;
        // dig out the profile that is commented, readers of that profile
        // will be the recipients of this comment:
        Profile* profileCommented ( iController->model().profileModel().profileByFingerPrint(aProfileComment.iProfileFingerPrint,true /* emit */,true/* no image */));
        if ( profileCommented == NULL ) {
            emit error(MController::ContentEncryptionError, tr("Profile related to comment not found from database")) ;
            return false ;
        }
        QList<Hash> profileCommentReaders = profileCommented->iProfileReaders ;
        if ( !profileCommentReaders.contains(aProfileComment.iCommentorHash ) ) {
            const Hash& commentorHash ( aProfileComment.iCommentorHash ) ;
            profileCommentReaders.append(commentorHash) ;
        }
        delete profileCommented ;
        profileCommented = NULL ;
        if ( iController->model().contentEncryptionModel().encrypt(profileCommentReaders,
                commentJSon,
                encryptedJson ) ) {
            commentJSon.clear() ;
            commentJSon.append(encryptedJson) ;
            encryptedJson.clear() ;
            // so ; now ; the profile json is encrypted -> proceed by making signature
            // of that
        } else {
            return false ; // encryption failed ?
        }
    }
    if ( commentJSon.length() > 0 ) {
        Hash messageFingerPrint ;
        QByteArray digitalSignature ;
        messageFingerPrint.calculate(commentJSon) ;
        if ( iController->model().contentEncryptionModel().sign(aProfileComment.iCommentorHash, commentJSon, digitalSignature) == 0 ) {
            QSqlQuery query;
            retval = query.prepare ("insert into profilecomment (hash1,hash2,hash3,hash4,hash5,profile_hash1,profile_hash2,profile_hash3,profile_hash4,profile_hash5,sender_hash1,time_last_reference,signature,time_of_publish,commentdata,recvd_from,flags,display_name) values (:hash1,:hash2,:hash3,:hash4,:hash5,:profile_hash1,:profile_hash2,:profile_hash3,:profile_hash4,:profile_hash5,:sender_hash1,:time_last_reference,:signature,:time_of_publish,:commentdata,:recvd_from,:flags,:display_name)" ) ;
            if ( retval ) {
                quint32 flags ( 0 ) ;
                query.bindValue(":hash1", messageFingerPrint.iHash160bits[0]);
                query.bindValue(":hash2", messageFingerPrint.iHash160bits[1]);
                query.bindValue(":hash3", messageFingerPrint.iHash160bits[2]);
                query.bindValue(":hash4", messageFingerPrint.iHash160bits[3]);
                query.bindValue(":hash5", messageFingerPrint.iHash160bits[4]);
                query.bindValue(":commentdata", commentJSon);
                QLOG_STR("Length of profile data at insert = " + commentJSon.size() );
                query.bindValue(":signature", digitalSignature);
                if ( aProfileComment.iIsPrivate ) {
                    flags = flags | 1 ;
                }
                query.bindValue(":profile_hash1", aProfileComment.iProfileFingerPrint.iHash160bits[0]);
                query.bindValue(":profile_hash2", aProfileComment.iProfileFingerPrint.iHash160bits[1]);
                query.bindValue(":profile_hash3", aProfileComment.iProfileFingerPrint.iHash160bits[2]);
                query.bindValue(":profile_hash4", aProfileComment.iProfileFingerPrint.iHash160bits[3]);
                query.bindValue(":profile_hash5", aProfileComment.iProfileFingerPrint.iHash160bits[4]);
                query.bindValue(":sender_hash1", aProfileComment.iCommentorHash.iHash160bits[0]);
                query.bindValue(":time_last_reference", QDateTime::currentDateTimeUtc().toTime_t());
                query.bindValue(":time_of_publish", aProfileComment.iTimeOfPublish);
                query.bindValue(":recvd_from", iController->getNode().nodeFingerPrint().iHash160bits[4]);
                query.bindValue(":flags", flags);
                if ( aProfileComment.iIsPrivate ) {
                    query.bindValue(":display_name",QVariant(QVariant::String)) ;
                } else {
                    if ( aProfileComment.iSubject.length() > 0 ) {
                        query.bindValue(":display_name", aProfileComment.iSubject.left(40));
                    } else {
                        query.bindValue(":display_name", aProfileComment.iCommentText.left(40));
                    }

                }
            }
            retval = query.exec() ;
            if ( !retval ) {
                LOG_STR2("Error while comment insert %s", qPrintable(query.lastError().text())) ;
                emit error(MController::DbTransactionError, query.lastError().text()) ;
            } else {
                aProfileComment.iFingerPrint = messageFingerPrint ;
                // here user is publishing as "brand new" so set bangpath to 0
                QList<quint32> emptyBangPath ;
                // note how comments get published to address of the profile,
                // not to addr of the content
                iController->model().addItemToBePublished(UserProfileComment,
                        messageFingerPrint,
                        emptyBangPath,
                        aProfileComment.iProfileFingerPrint) ;
                // and send notify to have possible ui-datamodels to be updated.
                // normally there is 1 or 2
                emit contentReceived(aProfileComment.iFingerPrint,
                                     aProfileComment.iProfileFingerPrint,
                                     UserProfileComment) ;
                if ( aProfileComment.iIsPrivate == false ) {
                    // index it too
                    iModel.searchModel()->indexProfileComment(aProfileComment) ;
                }
            }
        }
    }
    return retval ;
}

ProfileComment* ProfileCommentModel::profileCommentByFingerPrint(const Hash& aFingerPrint,
        bool aEmitOnEncryptionError) {
    LOG_STR("ProfileCommentModel::profileCommentByFingerPrint()") ;
    ProfileComment* retval = NULL ;
    QSqlQuery query;
    bool ret ;
    ret = query.prepare ("select commentdata,signature,flags from profilecomment where hash1 = :hash1 and hash2 = :hash2 and hash3 = :hash3 and hash4 = :hash4 and hash5 = :hash5" ) ;
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
        if ( query.next() &&
                !query.isNull(0) &&
                !query.isNull(1) &&
                !query.isNull(2) ) {
            QByteArray commentData ( query.value(0).toByteArray()) ;
            QByteArray signature ( query.value(1).toByteArray()) ;
            quint32 flags ( query.value(2).toUInt() ) ;
            bool isPrivate ( flags & 1 ) ; // if bit 1 is on, it is encrypted
            // for verification we need the key. .. we may have the key
            // if we've been dealing with the sender before, or not.
            //
            // sender information is only inside content so if that is
            // encrypted, we need to de-crypt first, so lets go:
            if ( isPrivate ) {
                QByteArray plainTextProfileCommentData ;
                if ( iController->model().contentEncryptionModel().decrypt(commentData,
                        plainTextProfileCommentData,
                        aEmitOnEncryptionError) ) {
                    LOG_STR2("profcomment: %s", qPrintable(QString(plainTextProfileCommentData)));
                    commentData.clear() ;
                    commentData = plainTextProfileCommentData ;
                } else {
                    return NULL ; // no need to go further
                }
            }

            retval = new ProfileComment(aFingerPrint) ;
            if ( retval->fromJSon(commentData,*iController) == false ) {
                delete retval ;
                return NULL ;
            }

            // ok, now we have the content, also the sender ; so lets try verifying
            // it is possible that we don't have the key yet.
            QByteArray dummy ;
            if (! iController->model().contentEncryptionModel().PublicKey (retval->iCommentorHash,
                    dummy) ) {
                // didn't have key
                iController->model().contentEncryptionModel().insertOrUpdatePublicKey (retval->iKeyOfCommentor,
                        retval->iCommentorHash,
                        retval->iCommentorNickName.length() > 0 ?
                        &(retval->iCommentorNickName) :
                        NULL ) ;
            }
            dummy.clear() ;
            // ok, now we supposedly have key in the table, lets verify the bytes
            if ( iController->model().contentEncryptionModel().verify(retval->iCommentorHash,
                    commentData,
                    signature) == false ) {
                // bad signature
                delete retval ;
                retval = NULL ;
            } else {
                // this method was called from UI so lets update the reference
                // time too
                setTimeLastReference(retval->iFingerPrint,
                                     QDateTime::currentDateTimeUtc().toTime_t()) ;
            }
        }
    }
    return retval ;
}

bool ProfileCommentModel::commentDataByFingerPrint(const Hash& aFingerPrint,
        QByteArray& aResultingProfileCommentData,
        QByteArray& aResultingSignature,
        Hash* aResultingProfileFingerPrint,
        quint32* aFlags,
        quint32* aTimeOfPublish) {
    LOG_STR("ProfileCommentModel::commentDataByFingerPrint()") ;
    bool retval = false ;
    QSqlQuery query;
    bool ret ;
    ret = query.prepare ("select commentdata,signature,flags,time_of_publish,profile_hash1,profile_hash2,profile_hash3,profile_hash4,profile_hash5 from profilecomment where hash1 = :hash1 and hash2 = :hash2 and hash3 = :hash3 and hash4 = :hash4 and hash5 = :hash5" ) ;
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
            aResultingProfileCommentData.clear() ;
            aResultingSignature.clear() ;
            aResultingProfileCommentData.append ( query.value(0).toByteArray()) ;
            aResultingSignature.append ( query.value(1).toByteArray()) ;
            if ( !query.isNull(2) ) {
                *aFlags = query.value(2).toUInt() ;
            } else {
                *aFlags = 0 ;
            }
            if ( !query.isNull(3) && aTimeOfPublish != NULL ) {
                *aTimeOfPublish = query.value(3).toUInt() ;
            }
            if ( aResultingProfileFingerPrint ) {
                aResultingProfileFingerPrint->iHash160bits[0] = query.value(4).toUInt() ;
                aResultingProfileFingerPrint->iHash160bits[1] = query.value(5).toUInt() ;
                aResultingProfileFingerPrint->iHash160bits[2] = query.value(6).toUInt() ;
                aResultingProfileFingerPrint->iHash160bits[3] = query.value(7).toUInt() ;
                aResultingProfileFingerPrint->iHash160bits[4] = query.value(8).toUInt() ;
            }
            retval = true ;
        }
    }
    return retval ;
}

bool ProfileCommentModel::sentProfileCommentReceived(const Hash& aFingerPrint,
        const Hash& aCommentedProfileFP,
        const QByteArray& aContent,
        const QByteArray& aSignature,
        const unsigned char aFlags,
        const quint32 aTimeStamp,
        const Hash& aFromNode ) {
    const QList<quint32> dummy ;
    return doHandlepublishedOrSentProfileComment(aFingerPrint,
            aCommentedProfileFP,
            aContent,
            aSignature,
            dummy,
            aFlags,
            aTimeStamp,
            false,
            aFromNode) ;
}


bool ProfileCommentModel::publishedProfileCommentReceived(const Hash& aFingerPrint,
        const Hash& aCommentedProfileFP,
        const QByteArray& aContent,
        const QByteArray& aSignature,
        const QList<quint32>& aBangPath,
        const unsigned char aFlags,
        const quint32 aTimeStamp,
        const Hash& aFromNode ) {
    return doHandlepublishedOrSentProfileComment(aFingerPrint,
            aCommentedProfileFP,
            aContent,
            aSignature,
            aBangPath,
            aFlags,
            aTimeStamp,
            true,
            aFromNode ) ;
}


bool ProfileCommentModel::doHandlepublishedOrSentProfileComment(const Hash& aFingerPrint,
        const Hash& aCommentedProfileFP,
        const QByteArray& aContent,
        const QByteArray& aSignature,
        const QList<quint32>& aBangPath,
        const unsigned char aFlags,
        const quint32 aTimeStamp,
        bool aWasPublish,
        const Hash& aFromNode ) {
    bool retval = false ;
    QString displayNameForComment ;
    quint32 senderHashLowBits (0);
    // ok, lets see .. if we have this particular comment already in storage
    QSqlQuery existenceQuery ;
    retval = existenceQuery.prepare("select count(hash1) from profilecomment where hash1 = :hash1 and hash2 = :hash2 and hash3 = :hash3 and hash4 = :hash4 and hash5 = :hash5");
    if ( retval ) {
        existenceQuery.bindValue(":hash1", aFingerPrint.iHash160bits[0]);
        existenceQuery.bindValue(":hash2", aFingerPrint.iHash160bits[1]);
        existenceQuery.bindValue(":hash3", aFingerPrint.iHash160bits[2]);
        existenceQuery.bindValue(":hash4", aFingerPrint.iHash160bits[3]);
        existenceQuery.bindValue(":hash5", aFingerPrint.iHash160bits[4]);
        retval = existenceQuery.exec() ;
        if ( retval ) {
            if ( existenceQuery.next() && !existenceQuery.isNull(0) ) {
                int countOfComments = existenceQuery.value(0).toInt() ;
                if ( countOfComments > 0 ) {
                    // we already had the comment, so lets not continue
                    return true ; // return with success
                }
            }
        } else {
            QLOG_STR(existenceQuery.lastError().text() + " "+ __FILE__ + QString::number(__LINE__)) ;
            emit error(MController::DbTransactionError, existenceQuery.lastError().text()) ;
            return false ;
        }
    }
    // ok, lets see .. should we verify first.

    // verify, if we can, e.g. the profile is public.
    ProfileComment c (aFingerPrint) ;
    if ( ( aFlags & 0x01 ) == 0 ) {
        // no encryption, lets go
        if ( c.fromJSon(aContent,*iController) == false ) {
            LOG_STR("Profile comment did not parse, returning false from datamodel..") ;
            return false ; // didn't parse
        }

        // it is possible that we don't have the key yet.
        QByteArray dummy ;
        if (! iController->model().contentEncryptionModel().PublicKey (c.iCommentorHash,
                dummy) ) {
            // didn't have key
            // lets see that the key parses..
            EVP_PKEY* key = NULL ;
            if ( ( key = iController->model().contentEncryptionModel().PublicKeyFromPem(c.iKeyOfCommentor) ) == NULL ) {
                // did not parse
                LOG_STR("Encryption key from profile comment did not parse") ;
                return false ;
            } else {
                EVP_PKEY_free(key) ;
                key = NULL ;
            }
            iController->model().contentEncryptionModel().insertOrUpdatePublicKey (c.iKeyOfCommentor,
                    c.iCommentorHash,
                    c.iCommentorNickName.length() > 0 ?
                    &(c.iCommentorNickName) :
                    NULL ) ;
        }
        // ok, now we're supposed to have the key in storage, so lets try verifying
        if ( iController->model().contentEncryptionModel().verify(c.iKeyOfCommentor,
                aContent,
                aSignature) == false ) {
            // soromnoo..
            LOG_STR("Profile comment signature verify failed") ;
            return false ;
        } else {
            displayNameForComment = c.iSubject.left(40) ;
            senderHashLowBits = c.iCommentorHash.iHash160bits[0] ;
        }
    }

    // also content of the comment should match the content:
    Hash checkFingerPrint ;
    checkFingerPrint.calculate(aContent) ;
    if ( checkFingerPrint != aFingerPrint ) {
        retval = false ;
    } else {
        // content had hash that was claimed to have..

        QSqlQuery query;
        retval = query.prepare ("insert into profilecomment (hash1,hash2,hash3,hash4,hash5,profile_hash1,profile_hash2,profile_hash3,profile_hash4,profile_hash5,sender_hash1,time_last_reference,signature,time_of_publish,commentdata,recvd_from,flags,display_name) values (:hash1,:hash2,:hash3,:hash4,:hash5,:profile_hash1,:profile_hash2,:profile_hash3,:profile_hash4,:profile_hash5,:sender_hash1,:time_last_reference,:signature,:time_of_publish,:commentdata,:recvd_from,:flags,:display_name)" ) ;
        if ( retval ) {
            query.bindValue(":hash1", aFingerPrint.iHash160bits[0]);
            query.bindValue(":hash2", aFingerPrint.iHash160bits[1]);
            query.bindValue(":hash3", aFingerPrint.iHash160bits[2]);
            query.bindValue(":hash4", aFingerPrint.iHash160bits[3]);
            query.bindValue(":hash5", aFingerPrint.iHash160bits[4]);
            query.bindValue(":commentdata", aContent);
            query.bindValue(":signature", aSignature);
            query.bindValue(":profile_hash1", aCommentedProfileFP.iHash160bits[0]);
            query.bindValue(":profile_hash2", aCommentedProfileFP.iHash160bits[1]);
            query.bindValue(":profile_hash3", aCommentedProfileFP.iHash160bits[2]);
            query.bindValue(":profile_hash4", aCommentedProfileFP.iHash160bits[3]);
            query.bindValue(":profile_hash5", aCommentedProfileFP.iHash160bits[4]);
            if ( senderHashLowBits ) {
                query.bindValue(":sender_hash1", senderHashLowBits);
            } else {
                query.bindValue(":sender_hash1", QVariant(QVariant::String));
            }
            query.bindValue(":time_last_reference", QDateTime::currentDateTimeUtc().toTime_t());
            query.bindValue(":time_of_publish", aTimeStamp);
            query.bindValue(":recvd_from", aFromNode.iHash160bits[4]); //NULL
            query.bindValue(":flags", aFlags);
            if ( displayNameForComment.length() == 0  ) {
                query.bindValue(":display_name", QVariant(QVariant::String)); //NULL
            } else {
                query.bindValue(":display_name", displayNameForComment);
            }
        }
        retval = query.exec() ;
        if ( !retval ) {
            LOG_STR2("Error while profile comment insert %s", qPrintable(query.lastError().text())) ;
            emit error(MController::DbTransactionError, query.lastError().text()) ;
            // .. so. we failed to insert, that was our fault. it still might be
            // better to disconnect the peer by returning false, so she will not
            // waste her time sending more content to HD with 0 bytes left?
            retval = false ;
        } else {
            emit contentReceived(aFingerPrint,
                                 aCommentedProfileFP,
                                 UserProfileComment) ;
            QLOG_STR("Emitting contentReceived for profile " + aCommentedProfileFP.toString()) ;
            if ( ( aFlags & 0x01 ) == 0 ) {
                // index it too, as it is public comment
                c.iFingerPrint = aFingerPrint ;
                iModel.searchModel()->indexProfileComment(c) ;
            }
        }
    }

    if ( aWasPublish && retval ) {
        iController->model().addItemToBePublished(UserProfileComment,
                aFingerPrint,
                aBangPath) ;
    }
    LOG_STR2("ProfileCommentModel::published/sentProfileCommentReceived out success = %d",
             (int)retval) ;
    return retval ;
}

void ProfileCommentModel::fillBucket(QList<SendQueueItem>& aSendQueue,
                                     const Hash& aStartOfBucket,
                                     const Hash& aEndOfBucket,
                                     quint32 aLastMutualConnectTime,
                                     const Hash& aForNode ) {
    QSqlQuery query;
    bool ret ;
    // in bucket filling we actually have 2 cases .. we may be bucket determined by the
    // comment content fingerprint or in the bucket determined by the commented
    // profile fingerprint. here fetch comments based on profile fingerprint
    ret = query.prepare ("select hash1,hash2,hash3,hash4,hash5,time_of_publish from profilecomment where profile_hash1 >= :start and profile_hash1 <= :end and time_of_publish > :last_connect_time and time_of_publish < :curr_time and ( recvd_from != :lowbits_of_requester or recvd_from is null ) order by time_of_publish desc limit 1000" ) ;
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
            SendQueueItem commentToSend ;
            commentToSend.iHash = hashFoundFromDb ;
            commentToSend.iItemType = UserProfileComment ;
            time_t timePublish(query.value(5).toUInt()) ;
            time_t timeMutual(aLastMutualConnectTime);
            aSendQueue.append(commentToSend) ;
#ifndef WIN32
            char formatted_mutual [100] = {0} ;
            char formatted_found [100]  = {0} ;
            ctime_r(&timePublish,&formatted_found[0] ) ;
            ctime_r(&timeMutual,&formatted_mutual[0] ) ;
            LOG_STR2("ProfileComment bucket mutual %s", formatted_mutual) ;
            LOG_STR2("ProfileComment bucket found %s", formatted_found) ;
#endif
        }
    }
}

QList<Hash> ProfileCommentModel::commentsForProfile(const Hash& aProfileHash,
        const quint32 aSinceTimeStamp) {
    QList<Hash> retval ;

    QSqlQuery query;
    bool ret ;
    // (like almost all other content)
    ret = query.prepare ("select hash1,hash2,hash3,hash4,hash5 from profilecomment where profile_hash1 = :h1 and profile_hash2 = :h2 and profile_hash3 = :h3 and profile_hash4 = :h4 and profile_hash5 = :h5 and time_of_publish > :time_of_publish" ) ;
    if ( ret ) {
        query.bindValue(":h1", aProfileHash.iHash160bits[0]);
        query.bindValue(":h2", aProfileHash.iHash160bits[1]);
        query.bindValue(":h3", aProfileHash.iHash160bits[2]);
        query.bindValue(":h4", aProfileHash.iHash160bits[3]);
        query.bindValue(":h5", aProfileHash.iHash160bits[4]);
        query.bindValue(":time_of_publish", aSinceTimeStamp);
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
    QLOG_STR("Returning list of profile comments len = " + QString::number(retval.size()) + " for profile " + aProfileHash.toString() + " since timestamp " + QString::number(aSinceTimeStamp)) ;
    return retval ;
}

void ProfileCommentModel::reIndexAllCommentsIntoFTS() {
    QList <Hash> articles ;
    QSqlQuery query;
    bool ret ;
    query.exec("delete from profilecomment_search") ;
    ret = query.prepare ("select hash1,hash2,hash3,hash4,hash5 from profilecomment where flags & 1 = 0") ;
    ret = query.exec() ;
    if ( !ret ) {
        emit error(MController::DbTransactionError, query.lastError().text()) ;
    } else {
        while ( query.next() ) {
            Hash hashFoundFromDb ( query.value(0).toUInt(),
                                   query.value(1).toUInt(),
                                   query.value(2).toUInt(),
                                   query.value(3).toUInt(),
                                   query.value(4).toUInt()) ;
            articles.append(hashFoundFromDb) ;
        }
    }
    foreach ( const Hash& a, articles ) {
        ProfileComment *c ( profileCommentByFingerPrint(a) ) ;
        if ( c ) {
            iModel.searchModel()->indexProfileComment(*c) ;
            delete c ;
        }
    }
}
