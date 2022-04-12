/*     -*-C++-*- -*-coding: utf-8-unix;-*-
  Classified Ads is Copyright (c) Antti Järvinen 2013-2021.

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
#include <QFont>
#include "profilecommentlistingmodel.h"
#include "../log.h"
#include "../mcontroller.h"
#include "model.h"
#include "profilecommentmodel.h"
#include "profilecomment.h"

/**
 * as default content give user some indication that more is
 * to be expected..later
 */
static const QString KDefaultProfileCommentContent (
    "<html><body>...</body></html>") ;

ProfileCommentListingModel::ProfileCommentListingModel(Model& aModel,
        MController& aController ) :
    iModel(aModel),
    iController(aController) {

}

ProfileCommentListingModel::~ProfileCommentListingModel() {
    LOG_STR("ProfileCommentListingModel::~ProfileCommentListingModel") ;
}

int ProfileCommentListingModel::rowCount(const QModelIndex& ) const {
    return iProfileComments.size();
}

int ProfileCommentListingModel::columnCount(const QModelIndex& ) const {
    return 3 ; // what are the columns?  Sender, time, subject?
}

QVariant ProfileCommentListingModel::data(const QModelIndex &index, int role) const {
    if(!index.isValid())
        return QVariant();

    if ( role == Qt::UserRole ) {
        return iProfileComments.at(index.row()).iCommentHash.toQVariant() ;
    }
    if ( role == Qt::UserRole+1 ) {
        return iProfileComments.at(index.row()).iCommentText ; // here return html
    }
    if ( role == Qt::UserRole+2 ) {
        // return sender+date+subject as single string
        QString headerTextForListings  ;
        if ( iProfileComments.at(index.row()).iSenderName.length() > 0 ) {
            headerTextForListings =  iProfileComments.at(index.row()).iSenderName ;
        } else {
            headerTextForListings =  iController.displayableNameForProfile(iProfileComments.at(index.row()).iSenderHash ) ;  ;
        }
        QDateTime d ;
        d.setTime_t(iProfileComments.at(index.row()).iCommentTimeStamp) ;
	QLocale locale ; 
        headerTextForListings = headerTextForListings + " | " + locale.toString(d, QLocale::ShortFormat) ;
        headerTextForListings = headerTextForListings + " | " + iProfileComments.at(index.row()).iCommentSubject ;
        return headerTextForListings ;
    }
    if ( role == Qt::UserRole+3 ) {
        return iProfileComments.at(index.row()).iNrOfAttachedFiles ;
    }
    if ( role == Qt::FontRole ) {
        QFont font;
        if ( iProfileComments.at(index.row()).iIsRead == false ) {
            font.setBold(true);
            font.setItalic(true);
        }
        return font;
    }

    switch ( index.column() ) {

    case 0: // peer, either name or hash
        if(role == Qt::DisplayRole) {
            if ( iProfileComments.at(index.row()).iSenderName.length() > 0 ) {
                return iProfileComments.at(index.row()).iSenderName ;
            } else {
                return iController.displayableNameForProfile(iProfileComments.at(index.row()).iSenderHash) ;
            }
        } else  if(role == Qt::DecorationRole) {
            // here return QIcon of size 26x26 featuring lenin reading pravda?
            return QVariant();
        } else  if(role == Qt::ToolTipRole) {
            return iProfileComments.at(index.row()).iCommentHash.toString() ;
        } else if ( role == Qt::ForegroundRole ) {
            if ( ( iController.profileInUse() ==
                    iProfileComments.at(index.row()).iCommentedProfileHash ) &&
                    iController.isContactInContactList(iProfileComments.at(index.row()).iSenderHash ) ) {
                return QColor(Qt::blue); // color blue if sender is in contacts
            } else {
                return QVariant() ;
            }
        } else {
            return QVariant() ;
        }
        break ;
    case 1: { // message time
        if(role == Qt::DisplayRole) {
            QDateTime d ;
            d.setTime_t(iProfileComments.at(index.row()).iCommentTimeStamp) ;
	    QLocale locale ; 
            return locale.toString(d, QLocale::ShortFormat) ; 
        } else {
            return QVariant();
        }
    }
    break ;
    case 2:
        if(role == Qt::DisplayRole) {
            return iProfileComments.at(index.row()).iCommentSubject ;
        } else {
            return QVariant();
        }
        break ;
    default:
        return QVariant(); // for unknown columns return empty
    }

    return QVariant();
}

void ProfileCommentListingModel::setSearchHash(const Hash& aSearch) {
    emit beginResetModel ();
    iSearchHash = aSearch ;
    // ok .. what next. .. don't search with empty
    if ( iSearchHash == KNullHash ) {
        iProfileComments.clear() ;
    } else {
        performSearch() ;
    }
    emit endResetModel () ;
    if ( iProfileComments.size() ) {
        QTimer::singleShot(0, this, SLOT(doUpdateDataOnIdle()));
    }
}

void ProfileCommentListingModel::performSearch() {
    QSqlQuery query(iController.model().dataBaseConnection());
    bool ret ;
    iProfileComments.clear() ;
    ret = query.prepare ("select hash1,hash2,hash3,hash4,hash5,profile_hash1,profile_hash2,profile_hash3,profile_hash4,profile_hash5,time_of_publish,sender_hash1 from profilecomment where profile_hash1 = :hash1 and profile_hash2 = :hash2 and profile_hash3 = :hash3 and profile_hash4 = :hash4 and profile_hash5 = :hash5 order by time_of_publish" ) ;
    if ( ret ) {
        query.bindValue(":hash1", iSearchHash.iHash160bits[0]); // destined to me
        query.bindValue(":hash2", iSearchHash.iHash160bits[1]);
        query.bindValue(":hash3", iSearchHash.iHash160bits[2]);
        query.bindValue(":hash4", iSearchHash.iHash160bits[3]);
        query.bindValue(":hash5", iSearchHash.iHash160bits[4]);
    }
    if ( ret && query.exec() ) {
        while ( query.next() ) {
            ProfileCommentListItem item ;
            item.iCommentText = KDefaultProfileCommentContent ;
            item.iIsUpdated = false ;
            item.iCommentHash =   Hash(query.value(0).toUInt(),
                                       query.value(1).toUInt(),
                                       query.value(2).toUInt(),
                                       query.value(3).toUInt(),
                                       query.value(4).toUInt()) ;
            if ( !query.isNull(5) ) {
                item.iCommentedProfileHash =   Hash(query.value(5).toUInt(),
                                                    query.value(6).toUInt(),
                                                    query.value(7).toUInt(),
                                                    query.value(8).toUInt(),
                                                    query.value(9).toUInt()) ;
            }
            item.iCommentTimeStamp = query.value(10).toUInt() ;
            if ( !query.isNull(11) && query.value(11).toUInt() == iController.profileInUse().iHash160bits[0] ) {
                // was sent by me, yes
                item.iSenderHash = iController.profileInUse() ;
            }
            item.iNrOfAttachedFiles = 0 ;
            item.iIsRead = false ;
            item.iCommentSubject = "-" ;
            iProfileComments.append(item) ;
            QLOG_STR("Appended into model a profile comment with ts = " + QString::number(item.iCommentTimeStamp) + " from " + item.iSenderHash.toString()) ;
        }
    } else {
        QLOG_STR("At profilecommentlisting " + query.lastError().text() + " line " + QString::number(__LINE__)) ;
        emit error(MController::DbTransactionError, query.lastError().text()) ;
    }
}

QVariant ProfileCommentListingModel::headerData ( int section, Qt::Orientation orientation, int role  ) const {
    if (orientation != Qt::Horizontal  ) {
        return QVariant();
    }
    switch ( role ) {
    case Qt::ToolTipRole:
        switch ( section ) {
        default:
            return QVariant();
            break ;
        }
        break ;
    case Qt::DisplayRole:
        switch ( section ) {
        case 0:
            return tr("Sender") ;
            break;
        case 1:
            return tr("Time") ;
            break ;
        case 2:
            return tr("Subject") ;
            break ;
        default:
            return QVariant();
        }
        break ;
    case Qt::SizeHintRole:
        switch ( section ) {
        default:
            return QVariant();
        }
        break ;
    default:
        return QVariant();
        break ;
    }

}

bool ProfileCommentListingModel::updateSenderAndSubjectOfMsg(ProfileCommentListItem& aItem) {
    ProfileComment* msg ( iModel.profileCommentModel().profileCommentByFingerPrint(aItem.iCommentHash,false));
    if ( msg ) {
        if ( msg->iCommentorNickName.length() ) {
            aItem.iSenderName = msg->iCommentorNickName ;
        } else {
            aItem.iSenderName = iController.displayableNameForProfile(msg->iCommentorHash) ;
        }
        aItem.iSenderHash = msg->iCommentorHash ;
        aItem.iCommentSubject = msg->iSubject ;
        aItem.iCommentTimeStamp = msg->iTimeOfPublish ;
        aItem.iCommentText = msg->iCommentText ;
        aItem.iNrOfAttachedFiles = msg->iAttachedFiles.count() ;
        QLOG_STR("Nr of attachments in comment " + aItem.iCommentHash.toString() + " = " + QString::number(aItem.iNrOfAttachedFiles)) ;
        delete msg ;
        return true ;
    } else {
        return false ;
    }
}


void   ProfileCommentListingModel::doUpdateDataOnIdle() {
    int indexUpdated(-1) ;
    int indexDeleted(-1) ;
    iModel.lock() ;
    for ( int i ( 0 ) ; i < iProfileComments.size() ; i++ ) {
        if ( iProfileComments.at(i).iIsUpdated == false) {
            indexUpdated = i ;
            ProfileCommentListItem& item ( iProfileComments[i] ) ;
            if ( updateSenderAndSubjectOfMsg(item) ) {
                LOG_STR2("ProfileCommentListingModel::doUpdateDataOnIdle index = %d", indexUpdated) ;
                item.iIsUpdated = true ;
            } else {
                indexUpdated = -1 ;
                indexDeleted = i ;
            }
            break ; // out of loop, e.g. do only one
        }
    }
    iModel.unlock() ;
    if ( indexUpdated > -1 ) {
        // so, if we updated something, give UI a clue
        emit dataChanged(createIndex(indexUpdated,0),
                         createIndex(indexUpdated,3) ) ;
        // and re-schedule self to check if there is more
        // messages to handle
        QTimer::singleShot(0, this, SLOT(doUpdateDataOnIdle()));
    }
    if ( indexDeleted > -1 ) {
        beginRemoveRows(createIndex(indexDeleted,0),0,2) ;
        iProfileComments.removeAt(indexDeleted) ;
        endRemoveRows() ;
        // and re-schedule self to check if there is more
        // messages to handle
        QTimer::singleShot(0, this, SLOT(doUpdateDataOnIdle()));
    }

}

void  ProfileCommentListingModel::setAsRead(const Hash& aComment, bool aIsRead) {
    for ( int i ( 0 ) ; i < iProfileComments.size() ; i++ ) {
        if ( iProfileComments.at(i).iCommentHash == aComment ) {
            ProfileCommentListItem item = iProfileComments.at(i) ;
            item.iIsRead = aIsRead ;
            iProfileComments.replace(i, item) ;
            break ;
        }
    }
}


void  ProfileCommentListingModel::newCommentReceived(const Hash& aComment,
        const Hash& aCommentedProfile) {
    bool wasAlreadyContained ( false ) ;
    if ( aCommentedProfile == iSearchHash ) {
        QLOG_STR("Got new comment for profile hash " + iSearchHash.toString()) ;
        // was for profile in display
        iModel.lock() ;
        for ( int i ( 0 ) ; i < iProfileComments.size() ; i++ ) {
            if ( iProfileComments.at(i).iCommentHash == aComment ) {
                wasAlreadyContained = true ;
                QLOG_STR("but comment was already found..") ;
                break ; // out of loop, e.g. do only one
            }
        }
        iModel.unlock() ;
        if ( !wasAlreadyContained ) {
            beginInsertRows(QModelIndex(),iProfileComments.size(),iProfileComments.size()) ;
            iModel.lock() ;
            ProfileCommentListItem item ;
            item.iIsRead = false ;
            item.iIsUpdated = false ;
            item.iCommentTimeStamp = QDateTime::currentDateTimeUtc().toTime_t() ;
            item.iCommentHash = aComment ;
            item.iCommentedProfileHash = aCommentedProfile ;
            item.iCommentSubject = " " ;
            item.iCommentText = KDefaultProfileCommentContent ;
            iProfileComments.append(item) ;
            iModel.unlock() ;
            endInsertRows() ;
            QTimer::singleShot(0, this, SLOT(doUpdateDataOnIdle()));
            QLOG_STR("Comment added to model.") ;
        }
    } else {
        QLOG_STR("Got new comment for profile hash that we don't search " + iSearchHash.toString()) ;
    }
}

// note that this method is called from ui/newprofilecommentdialog and
// it does the calling so that datamodel is already locked -> no locking again
void ProfileCommentListingModel::newCommentReceived(const ProfileComment& aComment) {
    if ( aComment.iProfileFingerPrint == iSearchHash ) {
        // was for profile in display
        bool wasAlreadyContained ( false ) ;
        for ( int i ( 0 ) ; i < iProfileComments.size() ; i++ ) {
            if ( iProfileComments.at(i).iCommentHash == aComment.iFingerPrint ) {
                wasAlreadyContained = true ;
                break ; // out of loop, e.g. do only one
            }
        }
        if ( !wasAlreadyContained ) {
            beginInsertRows(QModelIndex(),iProfileComments.size(),iProfileComments.size()) ;
            ProfileCommentListItem item ;
            item.iIsRead = true ;
            item.iIsUpdated = true ; // no need to update, we already had it all
            item.iSenderName = aComment.iCommentorNickName ;
            item.iCommentedProfileHash = aComment.iProfileFingerPrint ;
            item.iSenderHash = aComment.iCommentorHash ;
            item.iCommentSubject = aComment.iSubject ;
            item.iCommentTimeStamp = aComment.iTimeOfPublish ;
            item.iCommentHash = aComment.iFingerPrint ;
            item.iCommentText = aComment.iCommentText ;
            item.iNrOfAttachedFiles = aComment.iAttachedFiles.count() ;
            iProfileComments.append(item) ;
            QLOG_STR("ProfileCommentListingModel::newMsgReceived iSenderName = " + item.iSenderName) ;
            QLOG_STR("ProfileCommentListingModel::newMsgReceived iCommentedProfileHash = " + item.iCommentedProfileHash.toString()) ;
            QLOG_STR("ProfileCommentListingModel::newMsgReceived iSenderHash = " + item.iSenderHash.toString()) ;
            endInsertRows() ;
        }
    }
}

// this gets called when we're listing the "own profile" comments and
// the selection of the profile changes.
void ProfileCommentListingModel::profileSelected(const Hash& aProfileHash) {
    QLOG_STR("ProfileCommentListingModel::profileSelected " + aProfileHash.toString()) ;
    setSearchHash(aProfileHash)   ;
}
