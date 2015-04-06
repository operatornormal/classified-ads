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

#include "profilemodel.h"
#include "../log.h"
#include "../util/hash.h"
#include "model.h"
#include <QSqlQuery>
#include <QSqlError>
#include "profile.h"
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

ProfileModel::ProfileModel(MController *aController,
                           const MModelProtocolInterface &aModel)
    : ModelBase("profile",KMaxRowsInTableProfile),
      iController(aController),
      iModel(aModel) {
    LOG_STR("ProfileModel::ProfileModel()") ;
    connect(this,
            SIGNAL(  error(MController::CAErrorSituation,
                           const QString&) ),
            iController,
            SLOT(handleError(MController::CAErrorSituation,
                             const QString&)),
            Qt::QueuedConnection ) ;

}



ProfileModel::~ProfileModel() {
    LOG_STR("ProfileModel::~ProfileModel()") ;
    iController = NULL ; // not owned, just set null
}

bool ProfileModel::publishProfile(const Profile& aProfile) {
    LOG_STR("ProfileModel::publishProfile()") ;
    bool retval = false ;
    quint32 timeOfPublish(aProfile.iTimeOfPublish) ;
    QByteArray profileJSon ( aProfile.asJSon(*iController) ) ;
    if ( aProfile.iIsPrivate ) {
        QByteArray encryptedJson ;
        if ( iController->model().contentEncryptionModel().encrypt(aProfile.iProfileReaders,
                profileJSon,
                encryptedJson ) ) {
            profileJSon.clear() ;
            profileJSon.append(encryptedJson) ;
            encryptedJson.clear() ;
            // so ; now ; the profile json is encrypted -> proceed by making signature
            // of that
        } else {
            return false ; // encryption failed ?
        }
    }
    if ( profileJSon.length() > 0 ) {
        QByteArray digitalSignature ;
        if ( iController->model().contentEncryptionModel().sign(aProfile.iFingerPrint, profileJSon, digitalSignature) == 0 ) {
            QSqlQuery query;
            retval = query.prepare ("update profile set profiledata = :profiledata,signature=:signature,display_name=:display_name,is_private=:is_private,time_of_publish=:time_of_publish where hash1 = :hash1 and hash2 = :hash2 and hash3 = :hash3 and hash4 = :hash4 and hash5 = :hash5" ) ;
            if ( retval ) {
                query.bindValue(":hash1", aProfile.iFingerPrint.iHash160bits[0]);
                query.bindValue(":hash2", aProfile.iFingerPrint.iHash160bits[1]);
                query.bindValue(":hash3", aProfile.iFingerPrint.iHash160bits[2]);
                query.bindValue(":hash4", aProfile.iFingerPrint.iHash160bits[3]);
                query.bindValue(":hash5", aProfile.iFingerPrint.iHash160bits[4]);
                query.bindValue(":profiledata", profileJSon);
                query.bindValue(":signature", digitalSignature);
                query.bindValue(":is_private", aProfile.iIsPrivate);
                query.bindValue(":display_name", aProfile.displayName().toUtf8());
                query.bindValue(":time_of_publish", timeOfPublish);
            }
            retval = query.exec() ;
            if ( !retval ) {
                LOG_STR2("Error while profile update %s", qPrintable(query.lastError().text())) ;
                emit error(MController::DbTransactionError, query.lastError().text()) ;
            } else {
                // here user is publishing as "brand new" so set bangpath to 0
                QList<quint32> emptyBangPath ;
                iController->model().addItemToBePublished(UserProfile,
                        aProfile.iFingerPrint,
                        emptyBangPath) ;
                if ( aProfile.iIsPrivate == false ) {
                    iModel.searchModel()->indexProfile(aProfile) ;
                }
            }
        }
    }
    return retval ;
}

Profile* ProfileModel::profileByFingerPrint(const Hash& aFingerPrint,
        bool aEmitErrorOnEncryptionErrors,
        bool aOmitImage ) {
    LOG_STR("ProfileModel::profileByFingerPrint()") ;
    Profile* retval = NULL ;
    QSqlQuery query;
    bool ret ;
    ret = query.prepare ("select profiledata,signature,is_private from profile where hash1 = :hash1 and hash2 = :hash2 and hash3 = :hash3 and hash4 = :hash4 and hash5 = :hash5" ) ;
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
            QByteArray profileData ( query.value(0).toByteArray()) ;
            QByteArray signature ( query.value(1).toByteArray()) ;
            bool isPrivate ( false ) ;
            isPrivate = !query.isNull(2) && query.value(2).toBool() ;
            if ( iController->model().contentEncryptionModel().verify(aFingerPrint, profileData, signature) == true ) {
                // good signature
                retval = new Profile(aFingerPrint) ;
                if ( isPrivate ) {
                    QByteArray plainTextProfileData ;
                    if ( iController->model().contentEncryptionModel().decrypt(profileData,
                            plainTextProfileData,
                            aEmitErrorOnEncryptionErrors ) ) {
                        LOG_STR2("prof: %s", qPrintable(QString(plainTextProfileData)));
                        retval->fromJSon(plainTextProfileData,*iController,aOmitImage) ;
                    } else {
                        delete retval ;
                        retval = NULL ;
                    }
                } else {
                    // was then public and profileData is in plainText:
                    LOG_STR2("prof: ->%s<-", qPrintable(QString(profileData)));
                    retval->fromJSon(profileData,*iController,aOmitImage) ;
                }
            }
        }
    }
    return retval ;
}

bool ProfileModel::profileDataByFingerPrint(const Hash& aFingerPrint,
        QByteArray& aResultingProfileData,
        QByteArray& aResultingSignature,
        bool* aIsProfilePrivate,
        quint32* aTimeOfPublish,
        QByteArray* aResultingPublicKey) {
    LOG_STR("ProfileModel::profileDataByFingerPrint()") ;
    bool retval = false ;
    QSqlQuery query;
    bool ret ;
    ret = query.prepare ("select profiledata,signature,is_private,time_of_publish,pubkey from profile where hash1 = :hash1 and hash2 = :hash2 and hash3 = :hash3 and hash4 = :hash4 and hash5 = :hash5" ) ;
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
            aResultingProfileData.clear() ;
            aResultingSignature.clear() ;
            aResultingProfileData.append ( query.value(0).toByteArray()) ;
            aResultingSignature.append ( query.value(1).toByteArray()) ;
            if ( !query.isNull(2) && query.value(2).toBool() ) {
                *aIsProfilePrivate = true ;
            }
            if ( !query.isNull(3) && aTimeOfPublish != NULL ) {
                *aTimeOfPublish = query.value(3).toUInt() ;
            }
            if ( aResultingPublicKey != NULL ) {
                aResultingPublicKey->clear() ;
                aResultingPublicKey->append(query.value(4).toByteArray()) ;
            }
            retval = true ;
        }
    }
    return retval ;
}

bool ProfileModel::sentProfileReceived(const Hash& aFingerPrint,
                                       const QByteArray& aContent,
                                       const QByteArray& aSignature,
                                       const QByteArray& aProfilePublicKey,
                                       const unsigned char aFlags,
                                       const quint32 aTimeStamp,
                                       const Hash& aFromNode ) {
    const QList<quint32> dummy ;
    return doHandlepublishedOrSentProfile(aFingerPrint,
                                          aContent,
                                          aSignature,
                                          dummy,
                                          aProfilePublicKey,
                                          aFlags,
                                          aTimeStamp,
                                          false,
                                          aFromNode) ;
}


bool ProfileModel::publishedProfileReceived(const Hash& aFingerPrint,
        const QByteArray& aContent,
        const QByteArray& aSignature,
        const QList<quint32>& aBangPath,
        const QByteArray& aProfilePublicKey,
        const unsigned char aFlags,
        const quint32 aTimeStamp,
        const Hash& aFromNode ) {
    return doHandlepublishedOrSentProfile(aFingerPrint,
                                          aContent,
                                          aSignature,
                                          aBangPath,
                                          aProfilePublicKey,
                                          aFlags,
                                          aTimeStamp,
                                          true,
                                          aFromNode ) ;
}


bool ProfileModel::doHandlepublishedOrSentProfile(const Hash& aFingerPrint,
        const QByteArray& aContent,
        const QByteArray& aSignature,
        const QList<quint32>& aBangPath,
        const QByteArray& aProfilePublicKey,
        const unsigned char aFlags,
        const quint32 aTimeStamp,
        bool aWasPublish,
        const Hash& aFromNode ) {
    bool retval = false ;

    // ok, lets see .. should we verify first.
    QByteArray key ;

    // ok first stupid check:

    // lets say we have a node in network that says that profile is profile.
    // it has data and signature and public key, all is made public.
    // information about encryptedness of the profile is outside the
    // signing process. .. it is just a bit in the binary procotol between
    // peers. malicious node in the network starts flipping those bits,
    // causing non-encrypted pages to be tried with encryption and
    // nice-versa. so .. if it parses it is not encrypted. then check the
    // flag.
    //
    // we still have that malicious node that deliberatedly changes
    // content of the profile to be non-encrypted while it was originally
    // made encypted but that will then fail in verification stage

    bool parseResult(false) ;
    if ( ( aFlags & 1 ) == 0 ) { // try parsing only if non-encrypted
        // looks like QJson will set parseResult to true also
        // when content is actually encrypted so we can check only
        // for non-encrypted data here. it still needs to parse.
        QJson::Parser* jsonParser = new QJson::Parser();
        QVariantMap dummyResult = jsonParser->parse (aContent, &parseResult).toMap();
        delete jsonParser ;
        jsonParser = NULL ;
    }
    bool flagThisNodeAlreadyHadContent ( false ) ;
    if ( ( parseResult &&  ( ( aFlags & 1 ) == 0  ) ) || (( aFlags & 1 ) == 1)  ) { // it did parse and encryption flag was 0
        quint32 timeStampOfPossibleExistingProfile ( 0 ) ;
        if ( iController->model().contentEncryptionModel().PublicKey(aFingerPrint,
                key,
                &timeStampOfPossibleExistingProfile ) == true ) {
            // we have got the key,
            // check if we already had this version:
            if ( aTimeStamp > timeStampOfPossibleExistingProfile ) {
                // lets try verifying:
                if ( iController->model().contentEncryptionModel().verify(aFingerPrint, aContent, aSignature) == true ) {
                    // good signature
                    QSqlQuery query;
                    retval = query.prepare ("update profile set profiledata = :profiledata,signature=:signature,is_private=:is_private,display_name=:display_name,time_of_publish=:time_of_publish,recvd_from=:recvd_from where hash1 = :hash1 and hash2 = :hash2 and hash3 = :hash3 and hash4 = :hash4 and hash5 = :hash5" ) ;
                    if ( retval ) {
                        query.bindValue(":hash1", aFingerPrint.iHash160bits[0]);
                        query.bindValue(":hash2", aFingerPrint.iHash160bits[1]);
                        query.bindValue(":hash3", aFingerPrint.iHash160bits[2]);
                        query.bindValue(":hash4", aFingerPrint.iHash160bits[3]);
                        query.bindValue(":hash5", aFingerPrint.iHash160bits[4]);
                        query.bindValue(":profiledata", aContent);
                        query.bindValue(":signature", aSignature);
                        query.bindValue(":recvd_from", aFromNode.iHash160bits[4]);
                        LOG_STR2("### profile update, is private = %d", ( aFlags & 1 ) ) ;
                        query.bindValue(":is_private", (bool)( aFlags & 1 ) );
                        query.bindValue(":time_of_publish", aTimeStamp);
                        if (  aFlags & 1 ) {
                            // profile is private; use hash as displayname
                            query.bindValue(":display_name", aFingerPrint.toString().toUtf8()) ;
                        } else {
                            // profile is public ; dig out user-friendly name
                            Profile p (aFingerPrint) ;
                            p.fromJSon(aContent,*iController) ;
                            query.bindValue(":display_name", p.displayName().toUtf8()) ;
                            iModel.searchModel()->indexProfile(p,true) ;
                        }

                        retval = query.exec() ;
                    }
                    if ( !retval ) {
                        QLOG_STR(query.lastError().text() + " "+ __FILE__ + QString::number(__LINE__)) ;
                        emit error(MController::DbTransactionError, query.lastError().text()) ;
                    }
                }
            } else {
                LOG_STR("We already had the profile -> not continuing ..") ;
                retval = flagThisNodeAlreadyHadContent = true ;
            }
        } else {
            // we did not have the key already. published content is supplied with
            // key so we must trust that key is the key. lets try verifying.. ;
            // first check fingerprint, then save key..
            Hash hashOfKey (iController->model().contentEncryptionModel().hashOfPublicKey(aProfilePublicKey) ) ;
            if ( hashOfKey == aFingerPrint ) {
                iController->model().contentEncryptionModel().insertOrUpdatePublicKey(aProfilePublicKey,aFingerPrint) ;
                if ( iController->model().contentEncryptionModel().verify(aFingerPrint, aContent, aSignature) == true ) {
                    // good signature
                    QSqlQuery query;
                    retval = query.prepare ("update profile set profiledata=:profiledata,signature=:signature,is_private=:is_private,display_name=:display_name,time_of_publish=:time_of_publish,recvd_from=:recvd_from where hash1=:hash1 and hash2=:hash2 and hash3=:hash3 and hash4=:hash4 and hash5=:hash5" ) ;
                    if ( retval ) {
                        query.bindValue(":profiledata", aContent);
                        query.bindValue(":signature", aSignature);
                        query.bindValue(":is_private", (bool)( aFlags & 1 ) );
                        query.bindValue(":hash1", aFingerPrint.iHash160bits[0]);
                        query.bindValue(":hash2", aFingerPrint.iHash160bits[1]);
                        query.bindValue(":hash3", aFingerPrint.iHash160bits[2]);
                        query.bindValue(":hash4", aFingerPrint.iHash160bits[3]);
                        query.bindValue(":hash5", aFingerPrint.iHash160bits[4]);
                        query.bindValue(":recvd_from", aFromNode.iHash160bits[4]);
                        if ( aTimeStamp + 60 > QDateTime::currentDateTimeUtc().toTime_t() ) {
                            query.bindValue(":time_of_publish", (quint32)(QDateTime::currentDateTimeUtc().toTime_t()));
                        } else {
                            query.bindValue(":time_of_publish", aTimeStamp);
                        }

                        if (  aFlags & 1 ) {
                            // profile is private; use hash as displayname
                            query.bindValue(":display_name", aFingerPrint.toString().toUtf8()) ;
                        } else {
                            // profile is public ; dig out user-friendly name
                            Profile p (aFingerPrint) ;
                            p.fromJSon(aContent,*iController) ;
                            query.bindValue(":display_name", p.displayName().toUtf8()) ;
                            iModel.searchModel()->indexProfile(p) ;
                        }

                        retval = query.exec() ;
                        if ( !retval ) {
                            QLOG_STR(query.lastError().text() + " "+ __FILE__ + QString::number(__LINE__)) ;
                            emit error(MController::DbTransactionError, query.lastError().text()) ;
                        }
                    }
                }
            } else {
                LOG_STR("Hash of published/sent public key did not match the fp supplied in protocol") ;
            }

            // ok we got that far e.g. possible fingerprint+verify is done.
            // try save the actual stuff

        }
    } else {
        LOG_STR("Non-encrypted content did not parse") ;
        retval = false ;
    }
    if ( aWasPublish ) {
        if ( retval && (flagThisNodeAlreadyHadContent == false) ) {
            iController->model().addItemToBePublished(UserProfile,
                    aFingerPrint,
                    aBangPath) ;
        }
    }
    if ( retval && ( flagThisNodeAlreadyHadContent == false ) ) {
        emit contentReceived(aFingerPrint,
                             UserProfile) ;
    }
    LOG_STR2("ProfileModel::published/sentProfileReceived out success = %d",
             (int)retval) ;
    return retval ;
}

void ProfileModel::fillBucket(QList<SendQueueItem>& aSendQueue,
                              const Hash& aStartOfBucket,
                              const Hash& aEndOfBucket,
                              quint32 aLastMutualConnectTime,
                              const Hash& aForNode ) {
    QSqlQuery query;
    bool ret ;
    ret = query.prepare ("select hash1,hash2,hash3,hash4,hash5,time_of_publish from profile where hash1 >= :start and hash1 <= :end and time_of_publish > :last_connect_time and time_of_publish < :curr_time and ( recvd_from != :lowbits_of_requester or recvd_from is null ) order by time_of_publish desc limit 1000" ) ;
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
            SendQueueItem profileToSend ;
            profileToSend.iHash = hashFoundFromDb ;
            profileToSend.iItemType = UserProfile ;
            time_t timePublish(query.value(5).toUInt()) ;
            time_t timeMutual(aLastMutualConnectTime);
            aSendQueue.append(profileToSend) ;
#ifndef WIN32
            char formatted_mutual [100] = {0} ;
            char formatted_found [100]  = {0} ;
            ctime_r(&timePublish,&formatted_found[0] ) ;
            ctime_r(&timeMutual,&formatted_mutual[0] ) ;
            LOG_STR2("Profile bucket mutual %s", formatted_mutual) ;
            LOG_STR2("Profile bucket found %s", formatted_found) ;
#endif
        }
    }
}

Hash ProfileModel::profileWithOldestTimeSinceMsgPoll(quint32* aLastPollTime) {
    Hash retval ;
    QSqlQuery query;
    quint32 timestampFromDb ( 0 ) ;
    bool ret ;
    ret = query.prepare ("select hash1,hash2,hash3,hash4,hash5,last_msgpoll_time from privatekeys where last_msgpoll_time = ( select min(last_msgpoll_time) from privatekeys ) or last_msgpoll_time is null order by last_msgpoll_time asc limit 1" ) ;
    if ( !ret ) {
        emit error(MController::DbTransactionError, query.lastError().text()) ;
        QLOG_STR("ProfileModel::profileWithOldestTimeSinceMsgPoll " + query.lastError().text()) ;
    } else {
        if ( query.exec() && query.next() ) {
            Hash hashFoundFromDb ( query.value(0).toUInt(),
                                   query.value(1).toUInt(),
                                   query.value(2).toUInt(),
                                   query.value(3).toUInt(),
                                   query.value(4).toUInt()) ;
            if ( query.isNull(5) ) {
                retval = hashFoundFromDb ;
            } else {
                timestampFromDb = query.value(5).toUInt();
                if ( ( timestampFromDb  + ( 60*10 ) ) < QDateTime::currentDateTimeUtc().toTime_t() ) {
                    // was older than 600 seconds
                    retval = hashFoundFromDb ;
                    *aLastPollTime = timestampFromDb ;
                }
            }
        }
    }
    return retval ;
}

void ProfileModel::setPrivateMessagePollTimeForProfile(const quint32 aTimeStamp,

        const Hash& aProfile ) {
    bool ret ;
    QSqlQuery query2 ;
    ret = query2.prepare("update privatekeys set last_msgpoll_time = :last_msgpoll_time where hash1=:h1 and hash2=:h2 and hash3=:h3 and hash4=:h4 and hash5=:h5") ;
    if ( !ret ) {
        QLOG_STR(query2.lastError().text() + " "+ __FILE__ + QString::number(__LINE__)) ;
        emit error(MController::DbTransactionError, query2.lastError().text()) ;
    } else {
        query2.bindValue(":h1", aProfile.iHash160bits[0]);
        query2.bindValue(":h2", aProfile.iHash160bits[1]);
        query2.bindValue(":h3", aProfile.iHash160bits[2]);
        query2.bindValue(":h4", aProfile.iHash160bits[3]);
        query2.bindValue(":h5", aProfile.iHash160bits[4]);
        query2.bindValue(":last_msgpoll_time", aTimeStamp);
        ret = query2.exec() ;
        if ( !ret ) {
            QLOG_STR(query2.lastError().text() + " "+ __FILE__ + QString::number(__LINE__)) ;
            emit error(MController::DbTransactionError, query2.lastError().text()) ;
        }
    }
}

void  ProfileModel::setPrivateDataForProfile( const Hash& aProfile,
        const QVariant& aPrivateData ) {

    QJson::Serializer serializer;
    QByteArray dataJSon (serializer.serialize(aPrivateData) ) ;
    QByteArray encryptedJson ;
    QList<Hash> privateDataReaders ;
    privateDataReaders << aProfile ; // private data is encrypted only to self
    QLOG_STR("private data = " + QString(dataJSon)) ;
    if ( iController->model().contentEncryptionModel().encrypt(privateDataReaders,
            dataJSon,
            encryptedJson ) ) {
        bool ret ;
        QSqlQuery query2 ;
        ret = query2.prepare("update privatekeys set private_data = :private_data where hash1=:h1 and hash2=:h2 and hash3=:h3 and hash4=:h4 and hash5=:h5") ;
        if ( !ret ) {
            QLOG_STR(query2.lastError().text() + " "+ __FILE__ + QString::number(__LINE__)) ;
            emit error(MController::DbTransactionError, query2.lastError().text()) ;
        } else {
            query2.bindValue(":h1", aProfile.iHash160bits[0]);
            query2.bindValue(":h2", aProfile.iHash160bits[1]);
            query2.bindValue(":h3", aProfile.iHash160bits[2]);
            query2.bindValue(":h4", aProfile.iHash160bits[3]);
            query2.bindValue(":h5", aProfile.iHash160bits[4]);
            query2.bindValue(":private_data", encryptedJson);
            ret = query2.exec() ;
            if ( !ret ) {
                QLOG_STR(query2.lastError().text() + " "+ __FILE__ + QString::number(__LINE__)) ;
                emit error(MController::DbTransactionError, query2.lastError().text()) ;
            }
        }
    }
}


QVariant ProfileModel::privateDataOfProfile( const Hash& aProfile ) {
    QVariant retval ;

    QSqlQuery query;
    bool ret ;
    ret = query.prepare ("select private_data from privatekeys where hash1 = :hash1 and hash2 = :hash2 and hash3 = :hash3 and hash4 = :hash4 and hash5 = :hash5") ;
    if ( ret ) {
        query.bindValue(":hash1", aProfile.iHash160bits[0]);
        query.bindValue(":hash2", aProfile.iHash160bits[1]);
        query.bindValue(":hash3", aProfile.iHash160bits[2]);
        query.bindValue(":hash4", aProfile.iHash160bits[3]);
        query.bindValue(":hash5", aProfile.iHash160bits[4]);
    }
    ret = query.exec() ;
    if ( !ret ) {
        QLOG_STR(query.lastError().text() + " "+ __FILE__ + QString::number(__LINE__)) ;
        emit error(MController::DbTransactionError, query.lastError().text()) ;
    } else {
        if ( query.next() && !query.isNull(0) ) {
            QByteArray encryptedPrivateData ( query.value(0).toByteArray() ) ;
            QByteArray plainTextProfileData ;
            if ( iController->model().contentEncryptionModel().decrypt(encryptedPrivateData,
                    plainTextProfileData ) ) {
                QJson::Parser parser;
                QLOG_STR("priv data: " + plainTextProfileData) ;
                retval = parser.parse (plainTextProfileData, &ret).toMap();
                if ( !ret ) {
                    retval = QVariant() ;
                }
            }
        }
    }

    return retval ;
}


time_t ProfileModel::getLastProfileUpdateTime( const Hash& aProfile ) {
    time_t retval ( 0 ) ;
    QSqlQuery query;
    bool ret ;
    ret = query.prepare ("select time_of_update_poll from profile where hash1 = :hash1 and hash2 = :hash2 and hash3 = :hash3 and hash4 = :hash4 and hash5 = :hash5") ;
    if ( ret ) {
        query.bindValue(":hash1", aProfile.iHash160bits[0]);
        query.bindValue(":hash2", aProfile.iHash160bits[1]);
        query.bindValue(":hash3", aProfile.iHash160bits[2]);
        query.bindValue(":hash4", aProfile.iHash160bits[3]);
        query.bindValue(":hash5", aProfile.iHash160bits[4]);
    }
    ret = query.exec() ;
    if ( !ret ) {
        QLOG_STR(query.lastError().text() + " "+ __FILE__ + QString::number(__LINE__)) ;
        emit error(MController::DbTransactionError, query.lastError().text()) ;
    } else {
        if ( query.next() && !query.isNull(0) ) {
            retval = query.value(0).toUInt() ;
        }
    }
    return retval ;
}


void ProfileModel::setLastProfileUpdateTime( const Hash& aProfile,time_t aTime ) {
    QSqlQuery query;
    bool ret ;
    ret = query.prepare ("update profile set time_of_update_poll = :time_of_update_poll where hash1 = :hash1 and hash2 = :hash2 and hash3 = :hash3 and hash4 = :hash4 and hash5 = :hash5") ;
    if ( ret ) {
        query.bindValue(":hash1", aProfile.iHash160bits[0]);
        query.bindValue(":hash2", aProfile.iHash160bits[1]);
        query.bindValue(":hash3", aProfile.iHash160bits[2]);
        query.bindValue(":hash4", aProfile.iHash160bits[3]);
        query.bindValue(":hash5", aProfile.iHash160bits[4]);
        query.bindValue(":time_of_update_poll", (unsigned) aTime) ;
    }
    ret = query.exec() ;
    if ( !ret ) {
        QLOG_STR(query.lastError().text() + " "+ __FILE__ + QString::number(__LINE__)) ;
        emit error(MController::DbTransactionError, query.lastError().text()) ;
    }
}

void ProfileModel::reIndexAllProfilesIntoFTS() {
    QList <Hash> profiles ;
    QSqlQuery query;
    bool ret ;
    query.exec("delete from profile_search") ;
    ret = query.prepare ("select hash1,hash2,hash3,hash4,hash5 from profile where is_private='false'") ;
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
            profiles.append(hashFoundFromDb) ;
        }
    }
    foreach ( const Hash& a, profiles ) {
        Profile* p ( profileByFingerPrint(a) ) ;
        if ( p ) {
            iModel.searchModel()->indexProfile(    *p ) ;
            delete p ;
        }
    }
}
bool ProfileModel::deleteOldestDataRowInTable() {
    bool ret = false ;
    QSqlQuery query;
    // this overridden version from datamodelbase is different in
    // that way that this looks also into table privatekeys
    // and does not delete profiles that we have private keys for.
    ret = query.exec("delete from  " + iDataTableName + "  where rowid = ( select max(rowid) from  (select rowid from " + iDataTableName + " where time_last_reference = ( select min(time_last_reference) from  " + iDataTableName + "  where hash1 not in ( select hash1 from privatekeys ) ) ) ) ") ;
    if ( ret  ) {
        iCurrentDbTableRowCount-- ;
    }
    return ret ;
}
