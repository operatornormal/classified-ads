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

#include <QSqlQuery>
#include <QSqlError>
#include "calistingmodel.h"
#include "camodel.h"
#include "mmodelprotocolinterface.h"
#include "../log.h"
#include "ca.h"
#include "mcontroller.h"
#include "model.h"

CAListingModel::CAListingModel(const Hash& aForumToList,
                               const MModelProtocolInterface &aModel,
                               MController* aController)
    : iModel(aModel),
      iItemAndArticleHashRelation(NULL),
      iController(aController) {
    iItemAndArticleHashRelation = new QHash<int, QStandardItem *>() ;
    connect(this,
            SIGNAL(  error(MController::CAErrorSituation,
                           const QString&) ),
            iController,
            SLOT(handleError(MController::CAErrorSituation,
                             const QString&)),
            Qt::QueuedConnection ) ;

    iModel.classifiedAdsModel().installCAObserver(this) ;
    if ( aForumToList != KNullHash ) {
        setClassification(aForumToList) ;
    }
    iListingHeaderDate = new QStandardItem (tr("Date")) ; ;
    iListingHeaderSubject = new QStandardItem (tr("Subject")) ; ;
}

CAListingModel::~CAListingModel() {
    LOG_STR("~CAListingModel()") ;
    // by this time ca-model has already been deleted
    // iModel.classifiedAdsModel().removeCAObserver(this) ;
    delete iItemAndArticleHashRelation ;
    delete iListingHeaderDate ;
    delete iListingHeaderSubject ;
}

QStandardItemModel* CAListingModel::theCaModel()  {
    return &iCaModel ;
}

void CAListingModel::setClassification(const Hash&  aForumToList ) {

    LOG_STR2("CAListingModel::setClassification hash %s", qPrintable(aForumToList.toString())) ;
    iCaModel.clear() ;
    iItemAndArticleHashRelation->clear() ;
    iForumToList = aForumToList ;
    if ( aForumToList != KNullHash ) {
        QSqlQuery q ;
        bool operation_success ;
        operation_success = q.prepare("select hash1,hash2,hash3,hash4,hash5,"
                                      "display_name,time_of_publish,reply_to from classified_ad where "
                                      "group_hash1=:group_hash1 and group_hash2=:group_hash2 and "
                                      "group_hash3=:group_hash3 and group_hash4=:group_hash4 and "
                                      "group_hash5=:group_hash5 and jsondata is not null "
                                      "order by time_of_publish") ;
        q.bindValue(":group_hash1", aForumToList.iHash160bits[0]);
        q.bindValue(":group_hash2", aForumToList.iHash160bits[1]);
        q.bindValue(":group_hash3", aForumToList.iHash160bits[2]);
        q.bindValue(":group_hash4", aForumToList.iHash160bits[3]);
        q.bindValue(":group_hash5", aForumToList.iHash160bits[4]);
        LOG_STR2("Searching for ads with low-order group-bits %u", aForumToList.iHash160bits[4] ) ;
        operation_success = q.exec() ;
        QDateTime d;
        if ( operation_success ) {
            while ( q.next() ) {
                Hash articleFp (q.value(0).toUInt(),
                                q.value(1).toUInt(),
                                q.value(2).toUInt(),
                                q.value(3).toUInt(),
                                q.value(4).toUInt()) ;
                QString displayName ;
                if ( q.isNull(5) ) {
                    QString displayName = "" ;
                } else {
                    displayName = QString::fromUtf8(q.value(5).toByteArray()) ;
                }
                quint32 time_of_publish (q.value(6).toUInt()) ;
                quint32 reply_to (0);
                bool has_reply_to ;
                if ( q.isNull(7) ) {
                    has_reply_to = false ;
                } else {
                    has_reply_to = true ;
                    reply_to  = q.value(7).toUInt() ;
                }
                QList<QStandardItem *> row_in_model ;
                QStandardItem* display_name_item = new QStandardItem (  displayName  );
                display_name_item->setData(articleFp.toQVariant(),Qt::UserRole) ;
                row_in_model << display_name_item ;
                d.setTime_t(time_of_publish) ;
                QStandardItem* date_item = new QStandardItem ( (d.toString(Qt::SystemLocaleShortDate) ) ) ;
                row_in_model << date_item ;
                if ( !has_reply_to ) {
                    LOG_STR("Appending row to ca-model") ;
                    iCaModel.invisibleRootItem()->appendRow(row_in_model);
                } else {
                    QStandardItem* parent = iCaModel.invisibleRootItem() ;

                    if  ( iItemAndArticleHashRelation->contains(reply_to) ) {
                        parent = iItemAndArticleHashRelation->value(reply_to) ;
                        parent->appendRow(row_in_model);
                        LOG_STR("Appending row to ca-model") ;
                    } else {
                        // there is parent but it is not found from our index -> display
                        // as top-level item
                        iCaModel.invisibleRootItem()->appendRow(row_in_model);
                    }
                }
                iItemAndArticleHashRelation->insert(articleFp.iHash160bits[4], display_name_item) ;
            }
        } else {
            emit error(MController::DbTransactionError, q.lastError().text());
        }
    }

    iCaModel.setHeaderData(0, Qt::Horizontal, QVariant(tr("Subject"))) ;
    iCaModel.setHeaderData(1, Qt::Horizontal, QVariant(tr("Date"))) ;
}

void CAListingModel::newCaReceived(const CA& aNewCa) {
    LOG_STR("New ad received") ;
    insertCaIntoModel(aNewCa.iFingerPrint) ;
}

void CAListingModel::newCaReceived(const Hash& aHashNewCa,
                                   const Hash& aHashOfClassification) {
    LOG_STR2("New ad received by hash, class high-bits = %u",
             aHashOfClassification.iHash160bits[0]) ;
    LOG_STR2("Currently listing classification high-bits = %u",
             iForumToList.iHash160bits[0]) ;
    if ( iForumToList == aHashOfClassification ) {
        iController->model().lock() ;
        insertCaIntoModel(aHashNewCa) ;
        iController->model().unlock() ;
    }
}

bool CAListingModel::insertCaIntoModel(const Hash& aArticleFingerPrint) {
    bool retval ( false ) ;

    QSqlQuery q ;
    bool operation_success ;
    operation_success = q.prepare("select display_name,time_of_publish,reply_to from classified_ad where "
                                  "hash1=:hash1 and hash2=:hash2 and "
                                  "hash3=:hash3 and hash4=:hash4 and "
                                  "hash5=:hash5 and jsondata is not null" ) ;
    q.bindValue(":hash1", aArticleFingerPrint.iHash160bits[0]);
    q.bindValue(":hash2", aArticleFingerPrint.iHash160bits[1]);
    q.bindValue(":hash3", aArticleFingerPrint.iHash160bits[2]);
    q.bindValue(":hash4", aArticleFingerPrint.iHash160bits[3]);
    q.bindValue(":hash5", aArticleFingerPrint.iHash160bits[4]);
    LOG_STR2("Searching for ads with low-order article-bits %u", aArticleFingerPrint.iHash160bits[4] ) ;
    operation_success = q.exec() ;
    QDateTime d;
    if ( operation_success ) {
        while ( q.next() ) {
            retval = true ;
            QString displayName ;
            if ( q.isNull(0) ) {
                QString displayName = "" ;
            } else {
                displayName = QString::fromUtf8(q.value(0).toByteArray()) ;
            }
            quint32 time_of_publish (q.value(1).toUInt()) ;
            quint32 reply_to (0);
            bool has_reply_to ;
            if ( q.isNull(2) ) {
                has_reply_to = false ;
            } else {
                has_reply_to = true ;
                reply_to  = q.value(2).toUInt() ;
            }
            QList<QStandardItem *> row_in_model ;
            QStandardItem* display_name_item = new QStandardItem (  displayName  );
            display_name_item->setData(aArticleFingerPrint.toQVariant(),Qt::UserRole) ;
            row_in_model << display_name_item ;
            d.setTime_t(time_of_publish) ;
            QStandardItem* date_item = new QStandardItem ( d.toString(Qt::SystemLocaleShortDate) ) ;
            row_in_model << date_item ;
            if ( !has_reply_to ) {
                LOG_STR("Appending row to ca-model from notify") ;
                iCaModel.invisibleRootItem()->appendRow(row_in_model);
            } else {
                QStandardItem* parent = iCaModel.invisibleRootItem() ;

                if  ( iItemAndArticleHashRelation->contains(reply_to) ) {
                    parent = iItemAndArticleHashRelation->value(reply_to) ;
                    parent->appendRow(row_in_model);
                    LOG_STR("Appending row to ca-model from notify") ;
                } else {
                    // there is parent but it is not found from our index -> display
                    // as top-level item
                    iCaModel.invisibleRootItem()->appendRow(row_in_model);
                }
            }
            iItemAndArticleHashRelation->insert(aArticleFingerPrint.iHash160bits[4], display_name_item) ;
        }
    } else {
        emit error(MController::DbTransactionError, q.lastError().text());
    }


    return retval ;
}
