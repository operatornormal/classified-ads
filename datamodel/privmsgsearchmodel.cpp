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
#include <QFont>
#include "privmsgsearchmodel.h"
#include "trusttreemodel.h"
#include "../log.h"
#include "../mcontroller.h"
#include "model.h"
#include "privmsgmodel.h"
#include "privmsg.h"

PrivateMessageSearchModel::PrivateMessageSearchModel(Model& aModel,
						     MController& aController ) : 
  iModel(aModel),
  iController(aController),
  iLeftIcon(":/ui/ui/leftarrow.png") ,
  iRightIcon(":/ui/ui/rightarrow.png") 
{

}

PrivateMessageSearchModel::~PrivateMessageSearchModel() {
  LOG_STR("PrivateMessageSearchModel::~PrivateMessageSearchModel") ;
}

int PrivateMessageSearchModel::rowCount(const QModelIndex& ) const
{
  return iPrivateMessages.size();
}

int PrivateMessageSearchModel::columnCount(const QModelIndex& ) const
{
  return 4 ; // what are the columns? Direction, Peer, time, subject?
}
 
QVariant PrivateMessageSearchModel::data(const QModelIndex &index, int role) const {
  if(!index.isValid())
    return QVariant();

  if ( role == Qt::UserRole ) {
    return iPrivateMessages.at(index.row()).iMessageHash.toQVariant() ;
  }

  if ( role == Qt::FontRole ) {
    QFont font;
    if ( iPrivateMessages.at(index.row()).iIsRead == false ) {
      font.setBold(true);
      font.setItalic(true);
    } 
    return font;
  }

  switch ( index.column() ) {
  case 0: // direction, show icon
    if(role == Qt::DecorationRole) {
      if ( iController.profileInUse() == 
	   iPrivateMessages.at(index.row()).iRecipientHash ) {
	return QVariant(iLeftIcon); // here return image telling direction of message: to me
      } else {
	return QVariant(iRightIcon); // here return image telling direction of message: from me
      }
    } else {
      return QVariant();
    }
    break ;
  case 1: // peer, either name or hash
    if(role == Qt::DisplayRole) {
      if ( iController.profileInUse() == 
	   iPrivateMessages.at(index.row()).iRecipientHash ) {
	// to me, from recipient, so recipient is peer
	if ( iPrivateMessages.at(index.row()).iSenderName.length() > 0 ) {
	  return iPrivateMessages.at(index.row()).iSenderName ;
	} else {
	  return iPrivateMessages.at(index.row()).iSenderHash.toString() ; 
	}
      } else {
	// from me, so destination hash is peer
	if ( iPrivateMessages.at(index.row()).iSenderName.length() > 0 ) {
	  return iPrivateMessages.at(index.row()).iSenderName ;
	} else {
	  return iController.displayableNameForProfile(iPrivateMessages.at(index.row()).iRecipientHash) ;  
	}
      }
    } else  if(role == Qt::DecorationRole) {
      // here return QIcon of size 26x26 featuring lenin reading pravda? 
      return QVariant(); 
    } else  if(role == Qt::ToolTipRole) {
      Hash peerHash ( iController.profileInUse() == 
		      iPrivateMessages.at(index.row()).iRecipientHash ? 
		      iPrivateMessages.at(index.row()).iSenderHash : 
		      iPrivateMessages.at(index.row()).iRecipientHash ) ; 
      if ( iPrivateMessages.at(index.row()).iTrustingProfileName.length() > 0 ) {
	return QString(tr("%1\nTrusted by %2")).arg(peerHash.toString()).arg(iPrivateMessages.at(index.row()).iTrustingProfileName) ; 
      } else {
	return peerHash.toString() ; 
      }
    } else if ( role == Qt::ForegroundRole ) {
      Hash peerHash ( iController.profileInUse() == 
		      iPrivateMessages.at(index.row()).iRecipientHash ? 
		      iPrivateMessages.at(index.row()).iSenderHash : 
		      iPrivateMessages.at(index.row()).iRecipientHash ) ; 

      if ( iPrivateMessages.at(index.row()).iTrustingProfileName.length() > 0 ||
	   iController.isContactInContactList(peerHash ) ) {
	return QColor(Qt::blue); // color blue if sender is in contacts or trusted
      } else {
	return QVariant() ; 
      }
    } else {
      return QVariant() ; 
    }
    break ;
  case 2: // message time
    {
      if(role == Qt::DisplayRole) {
	QDateTime d ; 
	d.setTime_t(iPrivateMessages.at(index.row()).iMessageTimeStamp) ; 
	return d.toString(Qt::SystemLocaleShortDate) ; 
      } else {
	return QVariant();
      }
    }
    break ;
  case 3:
    if(role == Qt::DisplayRole) {
      return iPrivateMessages.at(index.row()).iMessageSubject ; 
    } else {
      return QVariant();
    }
    break ;
  default:
    return QVariant(); // for unknown columns return empty
  }

  return QVariant();
}

void PrivateMessageSearchModel::setSearchHash(const Hash& aSearch) {
  emit beginResetModel ();
  iSearchHash = aSearch ; 
  // ok .. what next. .. don't search with empty
  if ( iSearchHash == KNullHash ) {
    iPrivateMessages.clear() ; 
  } else {
    performSearch() ; 
  }
  emit endResetModel () ;
  if ( iPrivateMessages.size() ) {
    QTimer::singleShot(0, this, SLOT(doUpdateDataOnIdle()));
  }
}

void PrivateMessageSearchModel::performSearch() 
{
  QSqlQuery query;
  bool ret ;
  iPrivateMessages.clear() ; 
  ret = query.prepare ("select hash1,hash2,hash3,hash4,hash5,recipient_hash1,recipient_hash2,recipient_hash3,recipient_hash4,recipient_hash5,time_of_publish,sender_hash1,is_read from private_message where ( recipient_hash1 = :hash1 and recipient_hash2 = :hash2 and recipient_hash3 = :hash3 and recipient_hash4 = :hash4 and recipient_hash5 = :hash5) or sender_hash1 = :sender_hash1 order by time_of_publish" ) ;
  if ( ret ) {
    query.bindValue(":hash1", iSearchHash.iHash160bits[0]); // destined to me
    query.bindValue(":hash2", iSearchHash.iHash160bits[1]);
    query.bindValue(":hash3", iSearchHash.iHash160bits[2]);
    query.bindValue(":hash4", iSearchHash.iHash160bits[3]);
    query.bindValue(":hash5", iSearchHash.iHash160bits[4]);
    query.bindValue(":sender_hash1", iSearchHash.iHash160bits[0]); // or sent by me
  }
  if ( ret && query.exec() ) {
    while ( query.next() ) {
      PrivateMessageListItem item ;
      item.iMessageHash =   Hash(query.value(0).toUInt(),
				 query.value(1).toUInt(),
				 query.value(2).toUInt(),
				 query.value(3).toUInt(),
				 query.value(4).toUInt()) ;
      if ( !query.isNull(5) ) {
	item.iRecipientHash =   Hash(query.value(5).toUInt(),
				     query.value(6).toUInt(),
				     query.value(7).toUInt(),
				     query.value(8).toUInt(),
				     query.value(9).toUInt()) ;
      }
      item.iMessageTimeStamp = query.value(10).toUInt() ; 
      if ( !query.isNull(11) && query.value(11).toUInt() == iController.profileInUse().iHash160bits[0] ) {
	// was sent by me, yes 
	item.iSenderHash = iController.profileInUse() ; 
	QString trustingProfileName ; 
	Hash trustingProfileHash ; 
	if ( iController.model().trustTreeModel()->isProfileTrusted(item.iRecipientHash,
								    &trustingProfileName,
								    &trustingProfileHash) ) {
	  if ( trustingProfileName.length() > 0 ) {
	    item.iTrustingProfileName = trustingProfileName ; 
	  } else {
	    item.iTrustingProfileName = trustingProfileHash.toString() ; 
	  }
	}
      } else {
	// was destined to me, cant't check for trust yet
      }
      if ( !query.isNull(12) && query.value(12).toInt() > 0 ) {
	item.iIsRead = true ; 
      } else {
	item.iIsRead = false ; 
      }
      item.iMessageSubject = "-" ; 
      iPrivateMessages.append(item) ; 
      LOG_STR2("Appended into model a private message with ts = %u", (unsigned)(item.iMessageTimeStamp)) ; 
    }
  } else {
    emit error(MController::DbTransactionError, query.lastError().text()) ;
  }
}

QVariant PrivateMessageSearchModel::headerData ( int section, Qt::Orientation orientation, int role  ) const 
{
  if (orientation != Qt::Horizontal  ) {
    return QVariant();      
  }
  switch ( role ) {
  case Qt::ToolTipRole:
    switch ( section ) 
      {
      case 0:
	return tr("Direction of message, sent/received") ;
	break;
      default:
	return QVariant();      
	break ;
      }
    break ; 
  case Qt::DisplayRole:
    switch ( section ) 
      {
      case 0:
	return tr("Dir") ;
	break;
      case 1:
	return tr("Peer") ; 
	break ;
      case 2:
	return tr("Time") ; 
	break ;
      case 3:
	return tr("Subject") ; 
	break ;
      default:
	return QVariant();      
      }    
    break ;
  case Qt::SizeHintRole:
    switch ( section ) {
    case 0:
      return QSize(30,25) ; 
      break;
    case 1:
      return QSize(100,25) ; 
      break ;
    case 2:
      return QSize(150,25) ; 
      break ;
    case 3:
      return QSize(150,25) ; 
      break ;
    default:
      return QVariant();          
    }
    break ; 
  default:
    return QVariant();      
    break ; 
  }

}

void PrivateMessageSearchModel::updateSenderAndSubjectOfMsg(PrivateMessageListItem& aItem) 
{
  PrivMessage* msg ( iModel.privateMessageModel().messageByFingerPrint(aItem.iMessageHash));
  if ( msg ) {
    aItem.iSenderName = msg->iSenderName ; 
    aItem.iSenderHash = msg->iSenderHash ; 
    aItem.iMessageSubject = msg->iSubject ; 
    aItem.iMessageTimeStamp = msg->iTimeOfPublish ; 

    QString trustingProfileName ; 
    Hash trustingProfileHash ; 
    if ( iController.model().trustTreeModel()->isProfileTrusted( aItem.iSenderHash == iController.profileInUse() ? aItem.iRecipientHash : aItem.iSenderHash ,
								 &trustingProfileName,
								 &trustingProfileHash)) {
      if ( trustingProfileName.length() > 0 ) {
	aItem.iTrustingProfileName = trustingProfileName ; 
      } else {
	aItem.iTrustingProfileName = trustingProfileHash.toString() ; 
      }
    } else {
      aItem.iTrustingProfileName = QString() ; 
    }
  
    delete msg ; 
  }
}


void   PrivateMessageSearchModel::doUpdateDataOnIdle() 
{
  LOG_STR("PrivateMessageSearchModel::doUpdateDataOnIdle") ; 
  int indexUpdated(-1) ; 
  iModel.lock() ; 
  for ( int i ( 0 ) ; i < iPrivateMessages.size() ; i++ ) {
    if ( iPrivateMessages.at(i).iSenderHash == KNullHash ) {
      indexUpdated = i ; 
      PrivateMessageListItem& item ( iPrivateMessages[i] ) ;     
      updateSenderAndSubjectOfMsg(item) ; 
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
}

void  PrivateMessageSearchModel::setAsRead(const Hash& aMessage, bool aIsRead) 
{
  for ( int i ( 0 ) ; i < iPrivateMessages.size() ; i++ ) {
    if ( iPrivateMessages.at(i).iMessageHash == aMessage ) {
      PrivateMessageListItem item = iPrivateMessages.at(i) ;     
      item.iIsRead = aIsRead ; 
      iPrivateMessages.replace(i, item) ; 
      break ; 
    }
  }
}


void  PrivateMessageSearchModel::newMsgReceived(const Hash& aMessage,const Hash& aRecipient) 
{
  bool wasAlreadyContained ( false ) ; 
  if ( aRecipient == iController.profileInUse() ) {
    // was for me
    iModel.lock() ; 
    for ( int i ( 0 ) ; i < iPrivateMessages.size() ; i++ ) {
      if ( iPrivateMessages.at(i).iMessageHash == aMessage ) {
	wasAlreadyContained = true ; 
	break ; // out of loop, e.g. do only one
      }
    }  
    iModel.unlock() ; 
    if ( !wasAlreadyContained ) {
      beginInsertRows(QModelIndex(),iPrivateMessages.size(),iPrivateMessages.size()) ; 
      iModel.lock() ; 
      PrivateMessageListItem item ;
      item.iIsRead = false ; 
      item.iMessageTimeStamp = QDateTime::currentDateTimeUtc().toTime_t() ;
      item.iMessageHash = aMessage ; 
      item.iRecipientHash = aRecipient ; 
      item.iMessageSubject = " " ; 
      iPrivateMessages.append(item) ; 
      iModel.unlock() ; 
      endInsertRows() ; 
      QTimer::singleShot(0, this, SLOT(doUpdateDataOnIdle()));
    }
  }
}

void PrivateMessageSearchModel::newMsgReceived(const PrivMessage& aMessage) 
{
  beginInsertRows(QModelIndex(),iPrivateMessages.size(),iPrivateMessages.size()) ; 
  PrivateMessageListItem item ;
  item.iIsRead = true ;
  item.iSenderName = aMessage.iSenderName ; 
  item.iRecipientHash = aMessage.iRecipient ;
  item.iSenderHash = aMessage.iSenderHash ; 
  item.iMessageSubject = aMessage.iSubject ; 
  item.iMessageTimeStamp = aMessage.iTimeOfPublish ; 
  item.iMessageHash = aMessage.iFingerPrint ; 
  iPrivateMessages.append(item) ; 
  QLOG_STR("PrivateMessageSearchModel::newMsgReceived iSenderName = " + item.iSenderName) ;
  QLOG_STR("PrivateMessageSearchModel::newMsgReceived iRecipientHash = " + item.iRecipientHash.toString()) ;
  QLOG_STR("PrivateMessageSearchModel::newMsgReceived iSenderHash = " + item.iSenderHash.toString()) ;
  endInsertRows() ; 
} 
