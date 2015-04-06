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

#include "trusttreemodel.h"
#include "../log.h"
#include "../util/hash.h"
#include "model.h"
#include <QSqlQuery>
#include <QSqlError>
#include "profile.h"
#include "profilemodel.h"
#include <QStringList>

static const char *KTrustTreeTimeStamp = "treeTs" ;
static const char *KTrustTree = "tree" ;
static const char *KTrustTreeTrustingFp = "fp" ;
static const char *KTrustTreeTrustedFp = "trustedFp" ;
static const char *KTrustTreeList = "list" ;
static const char *KTrustTreeLevel = "level" ;

/**
 * How many levels of trust-lists to dig through
 */
static const char KHowManyLevelsDeep ( 3 ) ;

TrustTreeModel::TrustTreeModel(MController *aController,
                               const MModelProtocolInterface &aModel)
    :     iController(*aController),
          iModel(aModel),
          iTrustTree(NULL),
          iLastUpdateTime(0) {
    LOG_STR("TrustTreeModel::TrustTreeModel()") ;
    connect(this,
            SIGNAL(  error(MController::CAErrorSituation,
                           const QString&) ),
            aController,
            SLOT(handleError(MController::CAErrorSituation,
                             const QString&)),
            Qt::QueuedConnection ) ;
    iTrustTree = new QMap<Hash, TrustTreeItem>() ;
}



TrustTreeModel::~TrustTreeModel() {
    LOG_STR("TrustTreeModel::~TrustTreeModel()") ;
    delete iTrustTree ;
}

void TrustTreeModel::contentReceived(const Hash& aHashOfContent,
                                     const ProtocolItemType aTypeOfReceivdContent) {
    if ( aTypeOfReceivdContent == UserProfile ) {
        QLOG_STR("TrustTreeModel::contentReceived(), profile=" + aHashOfContent.toString() ) ;
        iController.model().lock() ;
        TrustTreeItem operatorFromTrustList ( (*iTrustTree)[aHashOfContent] ) ;
        if ( operatorFromTrustList.iTrustingOperator != KNullHash ) {
            // ok, operator was in trust list.
            operatorFromTrustList.iNeedsUpdate = true ;
            recalculateTrust()  ;
        }
        iController.model().unlock() ;
    }
}

void TrustTreeModel::offerTrustList(const Hash& aTrustlistOwnerProfile,
                                    const QString& aOwnerProfileDisplayName,
                                    const QList<Hash>& aList) {

    TrustTreeItem defaultValue ;
    defaultValue.iTrustingOperator = KNullHash ;
    TrustTreeItem operatorFromTrustList = iTrustTree->value(aTrustlistOwnerProfile,defaultValue) ;
    if ( operatorFromTrustList.iTrustingOperator != KNullHash ) {
        // ok, operator was in trust list.
        if ( operatorFromTrustList.iLevel < KHowManyLevelsDeep ) {
            // ok, we need to consider her trust list too:
            iTrustTree->remove(aTrustlistOwnerProfile) ;
            TrustTreeItem itemToAdd ;
            // initially set trusting operator to self, it gets updated later
            itemToAdd.iTrustingOperator = aTrustlistOwnerProfile ;
            itemToAdd.iOperatorName = aOwnerProfileDisplayName ;
            itemToAdd.iTrustList = aList ;
            itemToAdd.iLevel = operatorFromTrustList.iLevel ;
            iTrustTree->insert(aTrustlistOwnerProfile, itemToAdd ) ;
            recalculateTrust() ;
        }
    }
}


bool TrustTreeModel::isProfileTrusted(const Hash& aProfile,
                                      QString *aDisplayNameOfTrustingProfile,
                                      Hash* aFpOfTrustingProfile ) const {
    TrustTreeItem defaultValue ;
    defaultValue.iTrustingOperator = KNullHash ;
    TrustTreeItem operatorFromTrustList = iTrustTree->value(aProfile,defaultValue) ;
    if ( operatorFromTrustList.iTrustingOperator == KNullHash ) {
        // default value was returned, profile was not found from trust list
        return false ;
    } else {
        if ( aDisplayNameOfTrustingProfile ) {
            // get name of operator who trusts:
            TrustTreeItem trustingOperator = iTrustTree->value(operatorFromTrustList.iTrustingOperator) ;
            *aDisplayNameOfTrustingProfile = trustingOperator.iOperatorName ;
        }
        if ( aFpOfTrustingProfile ) {
            *aFpOfTrustingProfile = operatorFromTrustList.iTrustingOperator ;
        }
        return true ;
    }
}

// work-horse here
void TrustTreeModel::recalculateTrust() {
    Profile *currentlySelectedProfile = iController.model().profileModel().profileByFingerPrint(iController.profileInUse(),false,true) ;
    if ( currentlySelectedProfile ) {

        // stage 0
        // initialize level of every potentially untrustworthy sucker on the list
        QMutableMapIterator<Hash, TrustTreeItem> i(*iTrustTree);
        while (i.hasNext()) {
            i.next() ;
            i.value().iLevel = -1;
        }

        // stage 1
        // initially trust self
        setProfileTrustedInList(iController.profileInUse(),
                                iController.profileInUse(),
                                0) ;
        // stage 2
        // loop given number of levels of trust. must start from
        // 0 as it was given to self as trust level
        for ( char level = 0 ; level < KHowManyLevelsDeep ; level++ ) {
            QLOG_STR("Calculation level = " + QString::number(level)) ;
            QMutableMapIterator<Hash, TrustTreeItem> iter(*iTrustTree);
            while (iter.hasNext()) {
                iter.next() ;
                QLOG_STR("Item " + iter.key().toString() + " has list len " + QString::number(iter.value().iTrustList.count())) ;
                if ( iter.value().iLevel == level ) {
                    foreach( const Hash& trustedProfile, iter.value().iTrustList )    {
                        setProfileTrustedInList(trustedProfile,
                                                iter.key(),
                                                level+1) ;
                    }
                }
            }
        }

        // stage 3
        // should there have been any operators previously on list that still have
        // level -1 it means they're not trusted by anyone -> removal
        QMutableMapIterator<Hash, TrustTreeItem> iter(*iTrustTree);
        while (iter.hasNext()) {
            iter.next() ;
            if (iter.value().iLevel == -1 ) {
                iTrustTree->remove(iter.key()) ;
            }
        }
        delete currentlySelectedProfile ;
    }
}

void TrustTreeModel::setProfileTrustedInList(const Hash& aTrustedProfile,
        const Hash& aTrustingProfile,
        int aLevel ) {
    QLOG_STR("setProfileTrustedInList " + aTrustedProfile.toString() +
             " l " + QString::number(aLevel)) ;
    TrustTreeItem& operatorFromTrustList ( (*iTrustTree)[aTrustedProfile] ) ;
    if ( operatorFromTrustList.iTrustingOperator == KNullHash ) {
        // default value was returned, profile was not found from trust list
        // try obtain profile from model
        TrustTreeItem newItem ;
        newItem.iTrustingOperator = aTrustingProfile ;
        newItem.iLevel = aLevel ;
        newItem.iNeedsUpdate = false ;
        Profile *newProfile = iController.model().profileModel().profileByFingerPrint(aTrustedProfile,false,true) ;
        if ( newProfile ) {
            newItem.iTrustList = newProfile->iTrustList ;
            newItem.iOperatorName = newProfile->displayName() ;
            delete newProfile ;
        }
        iTrustTree->insert(aTrustedProfile, newItem) ;
        QLOG_STR("Operator " + aTrustedProfile.toString() + " was not on list, added, name = " + newItem.iOperatorName + " level " + QString::number(newItem.iLevel) + " list len = " + QString::number(newItem.iTrustList.count())) ;
    } else {
        if ( operatorFromTrustList.iLevel == -1 ) { // do not set 2nd time
            operatorFromTrustList.iLevel = aLevel ;
            operatorFromTrustList.iTrustingOperator = aTrustingProfile ;
            if ( operatorFromTrustList.iOperatorName.isEmpty() ||
                    operatorFromTrustList.iNeedsUpdate  ) {
                // try update name
                Profile *oldProfile = iController.model().profileModel().profileByFingerPrint(aTrustedProfile,false,true) ;
                if ( oldProfile ) {
                    operatorFromTrustList.iTrustList = oldProfile->iTrustList ;
                    operatorFromTrustList.iOperatorName = oldProfile->displayName() ;
                    delete oldProfile ;
                }
                operatorFromTrustList.iNeedsUpdate = false ;
            }
            QLOG_STR("Operator " + aTrustedProfile.toString() + " was on list, modified, name = " + operatorFromTrustList.iOperatorName + " level " + QString::number(operatorFromTrustList.iLevel) + " list len = " + QString::number(operatorFromTrustList.iTrustList.count())) ;
        }
    }
}

void TrustTreeModel::initModel(const QVariant& aPreviousSettings) {
    iTrustTree->clear()  ;

    QVariantMap m (aPreviousSettings.toMap()) ;

    if ( m.contains(KTrustTreeTimeStamp)) {
        iLastUpdateTime = m[KTrustTreeTimeStamp].toUInt() ;
    }
    if ( m.contains(KTrustTree)) {
        QVariantList tree ( m[KTrustTree].toList()) ;
        QListIterator<QVariant> i(tree); // tree is actually stored as a list
        while (i.hasNext()) {
            // item in tree is a map
            QVariantMap itemInTree ( i.next().toMap() ) ;
            if ( itemInTree.contains(KTrustTreeTrustingFp) &&
                    itemInTree.contains(KTrustTreeTrustedFp) &&
                    itemInTree.contains(KTrustTreeLevel) ) {
                TrustTreeItem newItem ;
                newItem.iTrustingOperator.fromQVariant( itemInTree[KTrustTreeTrustingFp] ) ;
                newItem.iLevel = itemInTree[KTrustTreeLevel].toInt() ;
                newItem.iNeedsUpdate = false ;
                Hash trustedFp ;
                trustedFp.fromQVariant(itemInTree[KTrustTreeTrustedFp]) ;
                if ( itemInTree.contains(KTrustTreeList ) ) { // if profile has no trust list, this is missing
                    QVariantList trustList ( m[KTrustTreeList].toList() )  ;
                    foreach ( const QVariant trustListItem , trustList ) {
                        Hash trustListItemHash ;
                        trustListItemHash.fromQVariant(trustListItem) ;
                        newItem.iTrustList.append(trustListItemHash) ;
                    }
                }
            }
        }
    }

    if ( !iTrustTree->contains(iController.profileInUse()) ) {
        // must include self .. should have been there in settings but
        // maybe we're initializing for the very first time
        setProfileTrustedInList(iController.profileInUse(),
                                iController.profileInUse(),
                                0) ;
    }
    // then update profiles that are on list and that have
    // been updated since last time
    QSqlQuery q ;
    QStringList queryString("select hash1,hash2,hash3,hash4,hash5 from profile where time_of_publish > :last_update_time and hash1 in (0") ;

    QMapIterator<Hash, TrustTreeItem> i(*iTrustTree);
    while (i.hasNext()) {
        i.next() ;
        queryString.append(","+QString::number(i.key().iHash160bits[0])) ;
    }
    QList<Hash> profilesThatNeedUpdating ;
    queryString.append(")") ;
    q.prepare (queryString.join("")) ;
    q.bindValue(":last_update_time", iLastUpdateTime) ;
    if ( q.exec() ) {
        while(q.next() ) {
            Hash hashFoundFromDb ( q.value(0).toUInt(),
                                   q.value(1).toUInt(),
                                   q.value(2).toUInt(),
                                   q.value(3).toUInt(),
                                   q.value(4).toUInt()) ;
            if ( iTrustTree->contains(hashFoundFromDb) ) {
                profilesThatNeedUpdating << hashFoundFromDb ;
            }
        }
    }

    foreach ( const Hash& profileInNeedOfUpdate,profilesThatNeedUpdating ) {
        Profile* p ( iController.model().profileModel().profileByFingerPrint(profileInNeedOfUpdate,false,true) );
        if ( p ) {
            TrustTreeItem& operatorFromTrustList ( (*iTrustTree)[profileInNeedOfUpdate] ) ;
            operatorFromTrustList.iTrustList = p->iTrustList ;
            operatorFromTrustList.iOperatorName = p->displayName() ;
            delete p ;
        }
    }
    iLastUpdateTime = QDateTime::currentDateTimeUtc().toTime_t() ;
    recalculateTrust() ;
}

QVariant TrustTreeModel::trustTreeSettings() const {
    // in practice the qvariant is a qvariantmap. data is stored in
    // keys in the map.
    QMap<QString, QVariant> m ;
    m.insert(KTrustTreeTimeStamp, iLastUpdateTime) ;
    // insert the tree as array of qvariants
    if ( iTrustTree->count() > 0 ) {
        QVariantList tree ; // tree is actually stored as a list
        QMapIterator<Hash, TrustTreeItem> i(*iTrustTree);
        while (i.hasNext()) {
            QVariantMap item ;
            i.next() ;
            item.insert(KTrustTreeTrustingFp, i.key().toQVariant()) ;
            item.insert(KTrustTreeTrustedFp, i.value().iTrustingOperator.toQVariant()) ;
            item.insert(KTrustTreeLevel, i.value().iLevel) ;
            if ( i.value().iTrustList.count() > 0 ) {
                QVariantList trustList ;
                foreach ( const Hash& listItem, i.value().iTrustList) {
                    trustList.append(listItem.toQVariant()) ;
                }
                item.insert(KTrustTreeList, trustList) ;
            }
            tree.append(QVariant(item)) ;
        }
        m.insert(KTrustTree, tree) ;
    }
    return QVariant(m) ;
}

void TrustTreeModel::clear(void) {
    if ( iTrustTree ) {
        iTrustTree->clear() ;
    }
}
