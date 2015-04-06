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
#ifdef WIN32
#define NOMINMAX
#include <WinSock2.h>
#endif
#include "camodel.h"
#include "../log.h"
#include "../util/hash.h"
#include "model.h"
#include <QSqlQuery>
#include <QSqlError>
#include "contentencryptionmodel.h"
#ifdef WIN32
#include <QJson/Parser>
#include <QJson/Serializer>
#else
#include <qjson/parser.h>
#include <qjson/serializer.h>
#include <arpa/inet.h>
#endif
#include "mmodelprotocolinterface.h"
#include "ca.h"
#include "profile.h"
#include "const.h"
#include "searchmodel.h"

/** KStrToBeBought name of intent of ad: to be not localized */
const static QString KStrToBeBought("ToBeBought") ;
const static QString KStrToBeSold("ToBeSold") ;
const static QString KStrToBeGivenAway ("ToBeGivenAway") ;
const static QString KStrIsWanted ("IsWanted") ;
const static QString KStrToBeRented ("ToBeRented") ;
const static QString KStrToBeAnnounced ("ToBeAnnounced") ;

/** KStrConcerningCars name of object of ad: to be not localized */
const static QString KStrConcerningCars ("Cars");
const static QString KStrConcerningBoats ("Boats");
const static QString KStrConcerningBikes ("Bikes");
const static QString KStrConcerningOtherVehicles ("OtherVehicles");
const static QString KStrConcerningVehicleParts ("VehicleParts");
const static QString KStrConcerningHabitation ("Habitation");
const static QString KStrConcerningHouseholdAppliances ("HouseholdAppliances");
const static QString KStrConcerningFurniture ("Furniture");
const static QString KStrConcerningClothing ("Clothing");
const static QString KStrConcerningTools ("Tools");
const static QString KStrConcerningSports ("Sports");
const static QString KStrConcerningMusic ("Music");
const static QString KStrConcerningBooks ("Books");
const static QString KStrConcerningMovies ("Movies");
const static QString KStrConcerningAnimals ("Animals");
const static QString KStrConcerningElectronics ("Electronics");
const static QString KStrConcerningJobs ("Jobs");
const static QString KStrConcerningTransportation ("Transportation");
const static QString KStrConcerningServices ("Services");
const static QString KStrConcerningHealthcare ("Healthcare");
const static QString KStrConcerningFoodstuff ("Foodstuff");
const static QString KStrConcerningSoftware ("Software");
const static QString KStrConcerningEvents ("Events");
const static QString KStrConcerningEducation ("Education");
const static QString KStrConcerningFinance ("Finance");
const static QString KStrConcerningJewelry ("Jewelry");
const static QString KStrConcerningReligiousRituals ("ReligiousRituals");
const static QString KStrConcerningPhilosophy ("Philosophy");

ClassifiedAdsModel::ClassifiedAdsModel(MController *aController,
                                       const MModelProtocolInterface &aModel)
    : ModelBase("classified_ad",KMaxRowsInTableClassified_Ad),
      iController(aController),
      iModel(aModel),
      iNewCaObservers(NULL) {
    LOG_STR("ClassifiedAdsModel::ClassifiedAdsModel()") ;
    iNewCaObservers = new QList<CAObserver*> () ;
    connect(this,
            SIGNAL(  error(MController::CAErrorSituation,
                           const QString&) ),
            iController,
            SLOT(handleError(MController::CAErrorSituation,
                             const QString&)),
            Qt::QueuedConnection ) ;
    initComboBoxTexts() ;
}



ClassifiedAdsModel::~ClassifiedAdsModel() {
    LOG_STR("ClassifiedAdsModel::~ClassifiedAdsModel()") ;
    iController = NULL ; // not owned, just set null
    delete iNewCaObservers;
    iNewCaObservers = NULL ;
}

const QString& ClassifiedAdsModel::purposeOfAdString(PurposeOfAd aPurpose) const {
    switch ( aPurpose ) {
    case  ToBeBought:
        return KStrToBeBought ;
    case  ToBeSold:
        return KStrToBeSold;
    case  ToBeGivenAway:
        return KStrToBeGivenAway;
    case  IsWanted:
        return KStrIsWanted;
    case  ToBeRented:
        return KStrToBeRented;
    case  ToBeAnnounced:
        return KStrToBeAnnounced;
    }
    return KStrToBeAnnounced;// this statement never reached, but makes compiler
    // happy
}
QString ClassifiedAdsModel::localizedPurposeOfAdString(PurposeOfAd aPurpose) const {
    switch ( aPurpose ) {
    case  ToBeBought:
        return tr("Buying") ;
    case  ToBeSold:
        return tr("Selling") ;
    case  ToBeGivenAway:
        return tr("Giving away") ;
    case  IsWanted:
        return tr("Wanting") ;
    case  ToBeRented:
        return tr("Renting") ;
    case  ToBeAnnounced:
        return tr("Announcement") ;
    }
    return KStrToBeAnnounced;// this statement never reached, but makes compiler
    // happy
}

const QString& ClassifiedAdsModel::concernOfAdString(ConcernOfAd aConcern) const {
    switch ( aConcern ) {
    case    ConcerningCars:
        return KStrConcerningCars ;
    case    ConcerningBoats:
        return KStrConcerningBoats ;
    case    ConcerningBikes:
        return KStrConcerningBikes ;
    case    ConcerningOtherVehicles:
        return KStrConcerningOtherVehicles ;
    case    ConcerningVehicleParts:
        return KStrConcerningVehicleParts ;
    case    ConcerningHabitation:
        return KStrConcerningHabitation ;
    case    ConcerningHouseholdAppliances:
        return KStrConcerningHouseholdAppliances ;
    case    ConcerningFurniture:
        return KStrConcerningFurniture ;
    case    ConcerningClothing:
        return KStrConcerningClothing ;
    case    ConcerningTools:
        return KStrConcerningTools ;
    case    ConcerningSports:
        return KStrConcerningSports ;
    case    ConcerningMusic:
        return KStrConcerningMusic ;
    case    ConcerningBooks:
        return KStrConcerningBooks ;
    case    ConcerningMovies:
        return KStrConcerningMovies ;
    case    ConcerningAnimals:
        return KStrConcerningAnimals ;
    case    ConcerningElectronics:
        return KStrConcerningElectronics ;
    case    ConcerningJobs:
        return KStrConcerningJobs ;
    case    ConcerningTransportation:
        return KStrConcerningTransportation ;
    case    ConcerningServices:
        return KStrConcerningServices ;
    case    ConcerningHealthcare:
        return KStrConcerningHealthcare ;
    case    ConcerningFoodstuff:
        return KStrConcerningFoodstuff ;
    case    ConcerningSoftware:
        return KStrConcerningSoftware ;
    case    ConcerningEvents:
        return KStrConcerningEvents ;
    case    ConcerningEducation:
        return KStrConcerningEducation ;
    case    ConcerningFinance:
        return KStrConcerningFinance ;
    case    ConcerningJewelry:
        return KStrConcerningJewelry ;
    case    ConcerningReligiousRituals:
        return KStrConcerningReligiousRituals ;
    case    ConcerningPhilosophy:
        return KStrConcerningPhilosophy ;
    }
    // this statement never reached, but changes mood
    // of the compiler to be less angry
    return KStrConcerningPhilosophy;
}

QString ClassifiedAdsModel::localizedConcernOfAdString(ConcernOfAd aConcern) const {
    switch ( aConcern ) {
    case    ConcerningCars:
        return tr("Cars") ;
    case    ConcerningBoats:
        return tr("Boats") ;
    case    ConcerningBikes:
        return tr("Bikes") ;
    case    ConcerningOtherVehicles:
        return tr("Other vehicles") ;
    case    ConcerningVehicleParts:
        return tr("Vehicle parts") ;
    case    ConcerningHabitation:
        return tr("Habitation") ;
    case    ConcerningHouseholdAppliances:
        return tr("Household appliances") ;
    case    ConcerningFurniture:
        return tr("Furniture") ;
    case    ConcerningClothing:
        return tr("Clothing") ;
    case    ConcerningTools:
        return tr("Tools") ;
    case    ConcerningSports:
        return tr("Sports") ;
    case    ConcerningMusic:
        return tr("Music") ;
    case    ConcerningBooks:
        return tr("Books") ;
    case    ConcerningMovies:
        return tr("Movies") ;
    case    ConcerningAnimals:
        return tr("Animals") ;
    case    ConcerningElectronics:
        return tr("Electronics") ;
    case    ConcerningJobs:
        return tr("Jobs") ;
    case    ConcerningTransportation:
        return tr("Transportation") ;
    case    ConcerningServices:
        return tr("Services") ;
    case    ConcerningHealthcare:
        return tr("Healthcare") ;
    case    ConcerningFoodstuff:
        return tr("Foodstuff") ;
    case    ConcerningSoftware:
        return tr("Software") ;
    case    ConcerningEvents:
        return tr("Events") ;
    case    ConcerningEducation:
        return tr("Education") ;
    case    ConcerningFinance:
        return tr("Finance") ;
    case    ConcerningJewelry:
        return tr("Jewelry") ;
    case    ConcerningReligiousRituals:
        return tr("Religious rituals") ;
    case    ConcerningPhilosophy:
        return tr("Philosophy") ;
    }
    // this statement never reached, but changes mood
    // of the compiler to be almost cheerful
    return KStrConcerningPhilosophy;
}

Hash ClassifiedAdsModel::publishClassifiedAd(const Profile& aPublishingProfile, CA& aAd ) {
    Hash retval ( KNullHash ) ;
    QByteArray articleJson ( aAd.asJSon(*iController) ) ;
    QByteArray signature ;
    Hash articleFingerPrint ;
    Hash groupFingerPrint ;
    bool operation_success ( false ) ;
    if (aAd.iGroup.length() &&
            articleJson.length() &&
            iController->model().contentEncryptionModel().sign(aPublishingProfile.iFingerPrint,
                    articleJson,
                    signature ) == 0 ) {

        articleFingerPrint.calculate(articleJson) ;
        groupFingerPrint.calculate(aAd.iGroup.toUtf8()) ;
        QSqlQuery ins ;
        operation_success= ins.prepare("insert into classified_ad("
                                       "hash1,hash2,hash3,hash4,hash5,"
                                       "group_hash1,group_hash2,group_hash3,"
                                       "group_hash4,group_hash5,"
                                       "time_last_reference,time_added,"
                                       "signature,display_name,time_of_publish,"
                                       "jsondata,reply_to,time_last_reference) values ("
                                       ":hash1,:hash2,:hash3,:hash4,:hash5,"
                                       ":group_hash1,:group_hash2,:group_hash3,"
                                       ":group_hash4,:group_hash5,"
                                       ":time_last_reference,:time_added,"
                                       ":signature,:display_name,:time_of_publish,"
                                       ":jsondata,:reply_to,:time_last_reference)") ;
        if ( operation_success ) {
            ins.bindValue(":hash1", articleFingerPrint.iHash160bits[0]);
            ins.bindValue(":hash2", articleFingerPrint.iHash160bits[1]);
            ins.bindValue(":hash3", articleFingerPrint.iHash160bits[2]);
            ins.bindValue(":hash4", articleFingerPrint.iHash160bits[3]);
            ins.bindValue(":hash5", articleFingerPrint.iHash160bits[4]);
            ins.bindValue(":group_hash1", groupFingerPrint.iHash160bits[0]);
            ins.bindValue(":group_hash2", groupFingerPrint.iHash160bits[1]);
            ins.bindValue(":group_hash3", groupFingerPrint.iHash160bits[2]);
            ins.bindValue(":group_hash4", groupFingerPrint.iHash160bits[3]);
            ins.bindValue(":group_hash5", groupFingerPrint.iHash160bits[4]);
            ins.bindValue(":time_last_reference",QDateTime::currentDateTimeUtc().toTime_t());
            ins.bindValue(":time_added",QDateTime::currentDateTimeUtc().toTime_t());
            ins.bindValue(":signature",signature);
            ins.bindValue(":display_name",aAd.displayName().toUtf8());
            ins.bindValue(":time_of_publish",aAd.iTimeOfPublish);
            ins.bindValue(":jsondata",articleJson);
            ins.bindValue(":time_last_reference",QDateTime::currentDateTimeUtc().toTime_t());
            if ( aAd.iReplyTo == KNullHash ) {
                ins.bindValue(":reply_to",QVariant(QVariant::String)); // NULL value
            } else {
                ins.bindValue(":reply_to",aAd.iReplyTo.iHash160bits[4]); // put low-order bits there
            }
            if ( ( operation_success = ins.exec() ) == false ) {
                emit error(MController::DbTransactionError, ins.lastError().text()) ;
            }  else {
                // insert was successfull: index the article
                aAd.iFingerPrint = articleFingerPrint ;
                iModel.searchModel()->indexClassifiedAd(aAd) ;
            }
        } else {
            emit error(MController::DbTransactionError, ins.lastError().text()) ;
        }
    }
    if ( operation_success ) {
        QList<quint32> emptyBangPath ;
        iController->model().addItemToBePublished(ClassifiedAdPublish,
                articleFingerPrint,
                emptyBangPath ) ;
        // twist, twist.
        //
        // classified ads get published twice. first to content addr,
        // second time to group controller addr.
        iController->model().addItemToBePublished(ClassifiedAd2NdAddr,
                articleFingerPrint,
                emptyBangPath,
                groupFingerPrint) ;
        retval = articleFingerPrint ;
        iCurrentDbTableRowCount++ ;
    }
    return retval ;
}


bool ClassifiedAdsModel::caDataForPublish(const Hash& aFingerPrint,
        QByteArray& aResultingCaData,
        QByteArray& aResultingSignature,
        QByteArray& aPublicKeyOfPublisher,
        quint32 *aTimeOfPublish ) {
    bool retval = false ;
    QSqlQuery query ;
    Hash fingerPrintOfPublisherKey;
    retval = query.prepare ("select jsondata,signature from classified_ad where jsondata is not null and "
                            "hash1 = :hash1 and hash2 = :hash2 and hash3 = :hash3 and hash4 = :hash4 and hash5 = :hash5" ) ;
    if ( retval ) {
        query.bindValue(":hash1", aFingerPrint.iHash160bits[0]);
        query.bindValue(":hash2", aFingerPrint.iHash160bits[1]);
        query.bindValue(":hash3", aFingerPrint.iHash160bits[2]);
        query.bindValue(":hash4", aFingerPrint.iHash160bits[3]);
        query.bindValue(":hash5", aFingerPrint.iHash160bits[4]);
    }
    retval = query.exec() ;
    if ( !retval ) {
        emit error(MController::DbTransactionError, query.lastError().text()) ;
    } else {
        if ( query.next() && !query.isNull(1) ) {
            aResultingSignature.clear() ;
            aResultingSignature.append(query.value(1).toByteArray()) ;
            aResultingCaData.append(query.value(0).toByteArray()) ;
            CA ca ;
            if ( ca.fromJSon(aResultingCaData,*iController) ) {
                if ( aTimeOfPublish ) {
                    *aTimeOfPublish = ca.iTimeOfPublish ;
                }
                aPublicKeyOfPublisher.clear() ;
                aPublicKeyOfPublisher.append(ca.iProfileKey) ;
                retval = aPublicKeyOfPublisher.size() > 0 ;
            }
        }
    }
    return retval ;
}

bool ClassifiedAdsModel::publishedCAReceived(const Hash& aFingerPrint,
        const QByteArray& aContent,
        const QByteArray& aSignature,
        const QList<quint32>& aBangPath,
        const QByteArray& aKeyOfPublisher,
        const unsigned char aFlags,
        const quint32 aTimeStamp,
        const Hash& aFromNode ) {
    return doHandleReceivedCA(aFingerPrint,
                              aContent,
                              aSignature,
                              aBangPath,
                              aKeyOfPublisher,
                              aFlags,
                              aTimeStamp,
                              true,
                              aFromNode ) ;
}

bool ClassifiedAdsModel::sentCAReceived(const Hash& aFingerPrint,
                                        const QByteArray& aContent,
                                        const QByteArray& aSignature,
                                        const QByteArray& aKeyOfPublisher,
                                        const unsigned char aFlags,
                                        const quint32 aTimeStamp,
                                        const Hash& aFromNode ) {
    const QList<quint32> dummy ;
    return doHandleReceivedCA(aFingerPrint,
                              aContent,
                              aSignature,
                              dummy,
                              aKeyOfPublisher,
                              aFlags,
                              aTimeStamp,
                              false,
                              aFromNode ) ;
}

bool ClassifiedAdsModel::doHandleReceivedCA(const Hash& aFingerPrint,
        const QByteArray& aContent,
        const QByteArray& aSignature,
        const QList<quint32>& aBangPath,
        const QByteArray& aKeyOfPublisher,
        const unsigned char /*aFlags*/,
        const quint32 /*aTimeStamp*/,
        bool aWasPublish,
        const Hash& aFromNode ) {
    bool retval ( false ) ;
    bool hostAlreadyHadContent ( false ) ;
    bool operation_success(false) ;
    bool contentWasInserted ( false ) ;
    QSqlQuery query ;
    Hash groupFingerPrint ;
    retval = query.prepare ("select count(jsondata),count(hash1) from classified_ad where hash1 = :hash1 and hash2 = :hash2 and hash3 = :hash3 and hash4 = :hash4 and hash5 = :hash5" ) ;
    if ( retval ) {
        query.bindValue(":hash1", aFingerPrint.iHash160bits[0]);
        query.bindValue(":hash2", aFingerPrint.iHash160bits[1]);
        query.bindValue(":hash3", aFingerPrint.iHash160bits[2]);
        query.bindValue(":hash4", aFingerPrint.iHash160bits[3]);
        query.bindValue(":hash5", aFingerPrint.iHash160bits[4]);
    }
    retval = query.exec() ;
    int count(0) ;
    int countOfHashes(0) ;
    if ( !retval ) {
        emit error(MController::DbTransactionError, query.lastError().text()) ;
    } else {
        if ( query.next() && !query.isNull(0) ) {
            count = query.value(0).toInt() ;
            countOfHashes = query.value(1).toInt() ;
        }
    }
    QLOG_STR("Saving AD,count of data = " +
             QString::number(count) +
             " count of hash 1 = " +
             QString::number(countOfHashes) +
             " for hash1 = " +
             QString::number(aFingerPrint.iHash160bits[0])) ;
    bool needsToInsert (false) ;
    if ( countOfHashes > 0 ) {
        if ( count == 0 ) {
            // ok, we had the hash in the db but we did not have the content.
            needsToInsert = true ;
            QSqlQuery deletia ;
            deletia.prepare("delete from classified_ad where hash1 = :hash1 and hash2 = :hash2 and hash3 = :hash3 and hash4 = :hash4 and hash5 = :hash5") ;
            deletia.bindValue(":hash1", aFingerPrint.iHash160bits[0]);
            deletia.bindValue(":hash2", aFingerPrint.iHash160bits[1]);
            deletia.bindValue(":hash3", aFingerPrint.iHash160bits[2]);
            deletia.bindValue(":hash4", aFingerPrint.iHash160bits[3]);
            deletia.bindValue(":hash5", aFingerPrint.iHash160bits[4]);
            deletia.exec() ;
            LOG_STR2("Delete done, rows = %d", deletia.numRowsAffected());
        } else {
            // this is ok situation, we had the content already
            retval = true ;
            hostAlreadyHadContent = true ;
        }
    } else {
        needsToInsert = true ;
    }

    if ( needsToInsert ) {
        // content not found -> insert
        Hash fingerPrintOfPublisher ( iController->model().contentEncryptionModel().hashOfPublicKey(aKeyOfPublisher) ) ;
        if ( iController->model().contentEncryptionModel().verify(aKeyOfPublisher,
                aContent,
                aSignature,
                NULL,
                false ) ) {

            // if we got here then signature in the file did match
            // they key.
            iController->model().contentEncryptionModel().insertOrUpdatePublicKey(aKeyOfPublisher,
                    fingerPrintOfPublisher);


            CA ca;
            ca.fromJSon(aContent, *iController) ;
            if ( ca.iProfileKey.size() > 0 &&
                    iController->model().contentEncryptionModel().hashOfPublicKey(aKeyOfPublisher) ==
                    fingerPrintOfPublisher ) {

                groupFingerPrint.calculate(ca.iGroup.toUtf8()) ;

                QSqlQuery ins ;
                operation_success= ins.prepare("insert into classified_ad("
                                               "hash1,hash2,hash3,hash4,hash5,"
                                               "group_hash1,group_hash2,group_hash3,"
                                               "group_hash4,group_hash5,"
                                               "time_last_reference,time_added,"
                                               "signature,display_name,time_of_publish,"
                                               "jsondata,reply_to,recvd_from) values ("
                                               ":hash1,:hash2,:hash3,:hash4,:hash5,"
                                               ":group_hash1,:group_hash2,:group_hash3,"
                                               ":group_hash4,:group_hash5,"
                                               ":time_last_reference,:time_added,"
                                               ":signature,:display_name,:time_of_publish,"
                                               ":jsondata,:reply_to,:recvd_from)") ;
                if ( operation_success ) {
                    ins.bindValue(":hash1", aFingerPrint.iHash160bits[0]);
                    ins.bindValue(":hash2", aFingerPrint.iHash160bits[1]);
                    ins.bindValue(":hash3", aFingerPrint.iHash160bits[2]);
                    ins.bindValue(":hash4", aFingerPrint.iHash160bits[3]);
                    ins.bindValue(":hash5", aFingerPrint.iHash160bits[4]);
                    ins.bindValue(":group_hash1", groupFingerPrint.iHash160bits[0]);
                    ins.bindValue(":group_hash2", groupFingerPrint.iHash160bits[1]);
                    ins.bindValue(":group_hash3", groupFingerPrint.iHash160bits[2]);
                    ins.bindValue(":group_hash4", groupFingerPrint.iHash160bits[3]);
                    ins.bindValue(":group_hash5", groupFingerPrint.iHash160bits[4]);
                    ins.bindValue(":time_last_reference",QDateTime::currentDateTimeUtc().toTime_t());
                    ins.bindValue(":time_added",QDateTime::currentDateTimeUtc().toTime_t());
                    ins.bindValue(":signature",aSignature);
                    ins.bindValue(":display_name",ca.displayName().toUtf8());
                    ins.bindValue(":time_of_publish",ca.iTimeOfPublish);
                    ins.bindValue(":jsondata",aContent);
                    ins.bindValue(":recvd_from", aFromNode.iHash160bits[4]);
                    if ( ca.iReplyTo == KNullHash ) {
                        ins.bindValue(":reply_to",QVariant(QVariant::String)); // NULL value
                    } else {
                        ins.bindValue(":reply_to",ca.iReplyTo.iHash160bits[4]); // put low-order bits there
                    }
                    if ( ( operation_success = ins.exec() ) == false ) {
                        emit error(MController::DbTransactionError, ins.lastError().text()) ;
                    } else {
                        retval = true ; // yes, yes
                        contentWasInserted = true ;
                        /* also check that profile key is stored in table profile, it will be referended
                         * later when sending messages to user
                         */
                        iModel.contentEncryptionModel().insertOrUpdatePublicKey(ca.iProfileKey,
                                ca.iSenderHash,
                                &ca.iSenderName) ;
                        ca.iFingerPrint = aFingerPrint ;
                        iModel.searchModel()->indexClassifiedAd(ca) ;
                    }
                } else {
                    emit error(MController::DbTransactionError, ins.lastError().text()) ;
                }
            } else {
                LOG_STR("No encryption key inside CA") ;
            }
        }  else {
            // signature did not verify..
            LOG_STR("CA signature did not verify") ;
        }
    }

    if ( aWasPublish &&
            hostAlreadyHadContent == false &&
            retval == true ) {
        // re-publish new content until bangpath is full
        iController->model().addItemToBePublished(ClassifiedAdPublish,
                aFingerPrint,
                aBangPath ) ;
        // twist, twist.
        //
        // classified ads get published twice. first to content addr,
        // second time to group controller addr.
        iController->model().addItemToBePublished(ClassifiedAd2NdAddr,
                aFingerPrint,
                aBangPath,
                groupFingerPrint) ;
    }

    if ( contentWasInserted ) {
        emit contentReceived(aFingerPrint,
                             groupFingerPrint,
                             ClassifiedAd) ;
        iCurrentDbTableRowCount++ ;
    }


    LOG_STR2("CAModel::published/sentCAReceived out success = %d",
             (int)retval) ;

    return retval ;
}

void ClassifiedAdsModel::installCAObserver(CAObserver* aObserver) {
    LOG_STR2("Installing new CA observer, this = %lx", (unsigned  long) this) ;
    if ( !iNewCaObservers->contains(aObserver) ) {
        iNewCaObservers->append(aObserver) ;
    }
}

void ClassifiedAdsModel::removeCAObserver(CAObserver* aObserver) {
    if ( iNewCaObservers->contains(aObserver) ) {
        iNewCaObservers->removeAll(aObserver) ;
    }
}

CA ClassifiedAdsModel::caByHash(const Hash& aFingerPrint) {
    CA retval ;
    bool operation_success ;
    if ( aFingerPrint != KNullHash ) {
        QSqlQuery query ;
        operation_success = query.prepare ("select jsondata from classified_ad where "
                                           "jsondata is not null and hash1 = :hash1 and hash2 = :hash2 and "
                                           "hash3=:hash3 and hash4=:hash4 and hash5=:hash5" ) ;
        if ( operation_success ) {
            query.bindValue(":hash1", aFingerPrint.iHash160bits[0]);
            query.bindValue(":hash2", aFingerPrint.iHash160bits[1]);
            query.bindValue(":hash3", aFingerPrint.iHash160bits[2]);
            query.bindValue(":hash4", aFingerPrint.iHash160bits[3]);
            query.bindValue(":hash5", aFingerPrint.iHash160bits[4]);
        }
        operation_success = query.exec() ;
        if ( !operation_success ) {
            emit error(MController::DbTransactionError, query.lastError().text()) ;
        } else {
            if ( query.next() && !query.isNull(0)  ) {
                retval.fromJSon(query.value(0).toByteArray(),*iController) ;
                retval.iFingerPrint = aFingerPrint ;
            }
        }
    }
    return retval ;
}

void ClassifiedAdsModel::reIndexAllAdsIntoFTS() {
    QList <Hash> articles ;
    QSqlQuery query;
    bool ret ;
    query.exec("delete from classified_ad_search") ;
    ret = query.prepare ("select hash1,hash2,hash3,hash4,hash5 from classified_ad") ;
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
        iModel.searchModel()->indexClassifiedAd(    caByHash(a)) ;
    }
}

void ClassifiedAdsModel::fillBucket(QList<SendQueueItem>& aSendQueue,
                                    const Hash& aStartOfBucket,
                                    const Hash& aEndOfBucket,
                                    quint32 aLastMutualConnectTime,
                                    const Hash& aForNode ) {
    QSqlQuery query;
    bool ret ;
    ret = query.prepare ("select hash1,hash2,hash3,hash4,hash5 from classified_ad where hash1 >= :start and hash1 <= :end and time_of_publish > :last_connect_time and time_of_publish < :curr_time and jsondata is not null and ( recvd_from != :lowbits_of_requester or recvd_from is null ) order by time_of_publish desc limit 1000" ) ;
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
        query.bindValue(":lowbits_of_requester", aForNode.iHash160bits[4]);
        query.bindValue(":last_connect_time", aLastMutualConnectTime);
        // shall a node misbehave (merely by having its clock grossly wrong)
        // its published works would be sent over and over again and again
        // if we didn't prevent future times from appearing here:
        query.bindValue(":curr_time", QDateTime::currentDateTimeUtc().toTime_t());
    }
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
            SendQueueItem caToSend ;
            caToSend.iHash = hashFoundFromDb ;
            caToSend.iItemType = ClassifiedAd ;
            aSendQueue.append(caToSend) ;
        }
    }
}

void ClassifiedAdsModel::caListingByClassification(const Hash& aClassificationHash,
        quint32 aStartDate,
        quint32 aEndDate,
        QList<QPair<Hash,quint32> >& aResultingArticles,
        const Hash& aRequestingNode) {
    QSqlQuery query;
    bool ret ;
    ret = query.prepare ("select hash1,hash2,hash3,hash4,hash5,time_of_publish from classified_ad where group_hash1 = :g1 and group_hash2 = :g2 and group_hash3 = :g3 and group_hash4 = :g4 and group_hash5 = :g5 and time_of_publish >= :startTime and time_of_publish <= :endTime and jsondata is not null and ( recvd_from != :lowbits_of_requester or recvd_from is null ) order by time_of_publish desc limit 1000" ) ;
    if ( ret ) {
        query.bindValue(":lowbits_of_requester", aRequestingNode.iHash160bits[4]);
        query.bindValue(":g1", aClassificationHash.iHash160bits[0]);
        query.bindValue(":g2", aClassificationHash.iHash160bits[1]);
        query.bindValue(":g3", aClassificationHash.iHash160bits[2]);
        query.bindValue(":g4", aClassificationHash.iHash160bits[3]);
        query.bindValue(":g5", aClassificationHash.iHash160bits[4]);
        query.bindValue(":startTime", aStartDate);
        query.bindValue(":endTime", aEndDate);
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
                quint32 ts ( query.value(5).toUInt() ) ;
                QPair<Hash,quint32> p (hashFoundFromDb,ts) ;
                aResultingArticles.append(p) ;
            }
        }
    }
    return ;
}
bool ClassifiedAdsModel::caListingByClassificationReceived(QList<QPair<Hash,quint32> >& aReceivedArticles,
        const Hash& aRequestingNode,
        const Hash& aClassification) {

    // start by figuring out which articles we already have
    QSqlQuery query;
    bool ret ;
    QByteArray selectStatement ("select hash1,hash2,hash3,hash4,hash5,time_of_publish from classified_ad where group_hash1 = :g1 and group_hash2 = :g2 and group_hash3 = :g3 and group_hash4 = :g4 and group_hash5 = :g5 and jsondata is not null and hash1 in (") ;
    const QByteArray closingParenthese(")") ;
    const QByteArray comma(",") ;
    for ( int i = 0 ; i <aReceivedArticles.size() ; i++ ) {
        selectStatement.append(QByteArray::number(aReceivedArticles[i].first.iHash160bits[0]) +
                               (i==(aReceivedArticles.size()-1) ? closingParenthese : comma)) ;
    }
    QLOG_STR(QString(selectStatement)) ;
    ret = query.prepare ( selectStatement ) ;
    if ( !ret ) {
        emit error(MController::DbTransactionError, query.lastError().text()) ;
    } else {
        query.bindValue(":g1", aClassification.iHash160bits[0]);
        query.bindValue(":g2", aClassification.iHash160bits[1]);
        query.bindValue(":g3", aClassification.iHash160bits[2]);
        query.bindValue(":g4", aClassification.iHash160bits[3]);
        query.bindValue(":g5", aClassification.iHash160bits[4]);
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
                quint32 tsFromDb ( query.value(5).toUInt() ) ;
                QPair<Hash, quint32> p ( hashFoundFromDb, tsFromDb ) ;
                aReceivedArticles.removeAll(p) ;  // removes 0 or more
            }
        }
    }
    // now aReceivedArticles contains only articles that we do not have
    if ( ret && ( aReceivedArticles.size() > 0 ) ) {
        LOG_STR2("List of ads: we did not have %d articles", aReceivedArticles.size()) ;
        quint32 current_time ( QDateTime::currentDateTimeUtc().toTime_t() ) ;
        QSqlQuery query2;
        ret = query2.exec("begin transaction ;") ;
        for ( int i = 0 ; ret && i <aReceivedArticles.size() ; i++ ) {
            QSqlQuery countQuery;
            int numberOfRowsAlready ( 0 ) ;

            if ( countQuery.prepare("select count(hash1) from classified_ad where hash1=:hash1,"
                                    "hash2=:hash2,hash3=:hash3,hash4=:hash4,hash5=:hash5") ) {
                countQuery.bindValue(":hash1", aReceivedArticles[i].first.iHash160bits[0]);
                countQuery.bindValue(":hash2", aReceivedArticles[i].first.iHash160bits[1]);
                countQuery.bindValue(":hash3", aReceivedArticles[i].first.iHash160bits[2]);
                countQuery.bindValue(":hash4", aReceivedArticles[i].first.iHash160bits[3]);
                countQuery.bindValue(":hash5", aReceivedArticles[i].first.iHash160bits[4]);
                if ( countQuery.exec() ) {
                    if ( countQuery.next() ) {
                        numberOfRowsAlready = countQuery.value(0).toInt() ;
                    }
                }
            }
            if ( numberOfRowsAlready == 0 ) {
                QSqlQuery query3 ;
                ret = query3.prepare ( "insert into classified_ad ("
                                       "hash1,hash2,hash3,hash4,hash5,"
                                       "group_hash1,group_hash2,group_hash3,group_hash4,group_hash5,"
                                       "time_of_publish,time_last_reference,time_added) values ( "
                                       ":hash1,:hash2,:hash3,:hash4,:hash5,"
                                       ":group_hash1,:group_hash2,:group_hash3,:group_hash4,:group_hash5,"
                                       ":time_of_publish,:time_last_reference,:time_added)") ;
                if ( !ret ) {
                    emit error(MController::DbTransactionError, query3.lastError().text()) ;
                } else {
                    query3.bindValue(":hash1", aReceivedArticles[i].first.iHash160bits[0]);
                    query3.bindValue(":hash2", aReceivedArticles[i].first.iHash160bits[1]);
                    query3.bindValue(":hash3", aReceivedArticles[i].first.iHash160bits[2]);
                    query3.bindValue(":hash4", aReceivedArticles[i].first.iHash160bits[3]);
                    query3.bindValue(":hash5", aReceivedArticles[i].first.iHash160bits[4]);
                    query3.bindValue(":group_hash1", aClassification.iHash160bits[0]);
                    query3.bindValue(":group_hash2", aClassification.iHash160bits[1]);
                    query3.bindValue(":group_hash3", aClassification.iHash160bits[2]);
                    query3.bindValue(":group_hash4", aClassification.iHash160bits[3]);
                    query3.bindValue(":group_hash5", aClassification.iHash160bits[4]);
                    query3.bindValue(":time_of_publish", aReceivedArticles[i].second);
                    query3.bindValue(":time_last_reference", current_time);
                    query3.bindValue(":time_added", current_time);
                    ret = query3.exec() ;
                }
                if ( !ret ) {
                    QLOG_STR("caListingByClassificationReceived " + query3.lastError().text() ) ;
                    emit error(MController::DbTransactionError, query3.lastError().text()) ;
                } else {
                    iCurrentDbTableRowCount++ ;
                }
            }
            // article saved, in addition put a request into queue to ask
            // for fetching of that article from node that provided us
            // with the list
            NetworkRequestExecutor::NetworkRequestQueueItem req ;
            req.iRequestType = RequestForClassifiedAd ;
            req.iState = NetworkRequestExecutor::NewRequest ;
            req.iDestinationNode = aRequestingNode ;
            req.iRequestedItem = aReceivedArticles[i].first ;
            req.iTimeStampOfItem = aReceivedArticles[i].second ;
            req.iMaxNumberOfItems = 1 ;
            iModel.addNetworkRequest(req) ;
        }
        QSqlQuery query4;
        ret = query4.exec("commit ;") ;
    }
    return ret ;
}

void ClassifiedAdsModel::initComboBoxTexts() {
    iAboutComboBoxTexts.append(localizedPurposeOfAdString(ClassifiedAdsModel::ToBeBought)) ;
    iAboutComboBoxTexts.append(localizedPurposeOfAdString(ClassifiedAdsModel::ToBeSold)) ;
    iAboutComboBoxTexts.append(localizedPurposeOfAdString(ClassifiedAdsModel::ToBeGivenAway)) ;
    iAboutComboBoxTexts.append(localizedPurposeOfAdString(ClassifiedAdsModel::IsWanted)) ;
    iAboutComboBoxTexts.append(localizedPurposeOfAdString(ClassifiedAdsModel::ToBeRented)) ;
    iAboutComboBoxTexts.append(localizedPurposeOfAdString(ClassifiedAdsModel::ToBeAnnounced)) ;

    iRegardingComboBoxTexts.append(localizedConcernOfAdString(ClassifiedAdsModel::ConcerningCars)) ;
    iRegardingComboBoxTexts.append(localizedConcernOfAdString(ClassifiedAdsModel::ConcerningBoats)) ;
    iRegardingComboBoxTexts.append(localizedConcernOfAdString(ClassifiedAdsModel::ConcerningBikes)) ;
    iRegardingComboBoxTexts.append(localizedConcernOfAdString(ClassifiedAdsModel::ConcerningOtherVehicles)) ;
    iRegardingComboBoxTexts.append(localizedConcernOfAdString(ClassifiedAdsModel::ConcerningVehicleParts)) ;
    iRegardingComboBoxTexts.append(localizedConcernOfAdString(ClassifiedAdsModel::ConcerningHabitation)) ;
    iRegardingComboBoxTexts.append(localizedConcernOfAdString(ClassifiedAdsModel::ConcerningHouseholdAppliances)) ;
    iRegardingComboBoxTexts.append(localizedConcernOfAdString(ClassifiedAdsModel::ConcerningFurniture)) ;
    iRegardingComboBoxTexts.append(localizedConcernOfAdString(ClassifiedAdsModel::ConcerningClothing)) ;
    iRegardingComboBoxTexts.append(localizedConcernOfAdString(ClassifiedAdsModel::ConcerningTools)) ;
    iRegardingComboBoxTexts.append(localizedConcernOfAdString(ClassifiedAdsModel::ConcerningSports)) ;
    iRegardingComboBoxTexts.append(localizedConcernOfAdString(ClassifiedAdsModel::ConcerningMusic)) ;
    iRegardingComboBoxTexts.append(localizedConcernOfAdString(ClassifiedAdsModel::ConcerningBooks)) ;
    iRegardingComboBoxTexts.append(localizedConcernOfAdString(ClassifiedAdsModel::ConcerningMovies)) ;
    iRegardingComboBoxTexts.append(localizedConcernOfAdString(ClassifiedAdsModel::ConcerningAnimals)) ;
    iRegardingComboBoxTexts.append(localizedConcernOfAdString(ClassifiedAdsModel::ConcerningElectronics)) ;
    iRegardingComboBoxTexts.append(localizedConcernOfAdString(ClassifiedAdsModel::ConcerningJobs)) ;
    iRegardingComboBoxTexts.append(localizedConcernOfAdString(ClassifiedAdsModel::ConcerningTransportation)) ;
    iRegardingComboBoxTexts.append(localizedConcernOfAdString(ClassifiedAdsModel::ConcerningServices)) ;
    iRegardingComboBoxTexts.append(localizedConcernOfAdString(ClassifiedAdsModel::ConcerningHealthcare)) ;
    iRegardingComboBoxTexts.append(localizedConcernOfAdString(ClassifiedAdsModel::ConcerningFoodstuff)) ;
    iRegardingComboBoxTexts.append(localizedConcernOfAdString(ClassifiedAdsModel::ConcerningSoftware)) ;
    iRegardingComboBoxTexts.append(localizedConcernOfAdString(ClassifiedAdsModel::ConcerningEvents)) ;
    iRegardingComboBoxTexts.append(localizedConcernOfAdString(ClassifiedAdsModel::ConcerningEducation)) ;
    iRegardingComboBoxTexts.append(localizedConcernOfAdString(ClassifiedAdsModel::ConcerningFinance)) ;
    iRegardingComboBoxTexts.append(localizedConcernOfAdString(ClassifiedAdsModel::ConcerningJewelry)) ;
    iRegardingComboBoxTexts.append(localizedConcernOfAdString(ClassifiedAdsModel::ConcerningReligiousRituals)) ;
    iRegardingComboBoxTexts.append(localizedConcernOfAdString(ClassifiedAdsModel::ConcerningPhilosophy)) ;

    iWhereComboBoxTexts.append(tr("Any country")) ;
    for ( int c = QLocale::AnyCountry+1 ;
            c <= QLocale::LatinAmericaAndTheCaribbean ;
            c ++ ) {
        iWhereComboBoxTexts.append(QLocale::countryToString((QLocale::Country)c)) ;
    }
}

const QStringList& ClassifiedAdsModel::aboutComboBoxTexts() const {
    return iAboutComboBoxTexts;
}

const QStringList& ClassifiedAdsModel::regardingComboBoxTexts() const {
    return iRegardingComboBoxTexts ;
}

const QStringList& ClassifiedAdsModel::whereComboBoxTexts() const {
    return iWhereComboBoxTexts;
}
