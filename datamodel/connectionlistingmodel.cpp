/*     -*-C++-*- -*-coding: utf-8-unix;-*-
       Classified Ads is Copyright (c) Antti Järvinen 2013.

       This file is part of Classified Ads.

       Classified Ads is free software: you can redistribute it and/or modify
       it under the terms of the GNU General Public License as published by
       the Free Software Foundation, either version 3 of the License, or
       (at your option) any later version.

       Classified Ads is distributed in the hope that it will be useful,
       but WITHOUT ANY WARRANTY; without even the implied warranty of
       MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
       GNU General Public License for more details.

       You should have received a copy of the GNU General Public License
       along with Classified Ads.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "../log.h"
#include "../mcontroller.h"
#include "model.h"
#include "connectionlistingmodel.h"
#include <QTimerEvent>
#include <QSize>

ConnectionListingModel::ConnectionListingModel(Model& aModel,
					       MController& aController ) : 
  iModel(aModel),
  iController(aController),
  iTimerId(-1)  
{
  iTimerId = startTimer(15000) ; // every 15 sec
  updateModelContents() ;
}

ConnectionListingModel::~ConnectionListingModel() {
  LOG_STR("ConnectionListingModel::~ConnectionListingModel") ;
  if ( iTimerId != -1 ) {
    killTimer(iTimerId) ; 
  }
}

void ConnectionListingModel::timerEvent(QTimerEvent *event)
{
  LOG_STR2( "ConnectionListingModel::timerEvent Timer ID: %d" , event->timerId());
#if QT_VERSION >= 0x050000
  // qt5
  beginResetModel() ; 
#endif
  updateModelContents() ; 
#if QT_VERSION >= 0x050000
  endResetModel() ; 
#else    
  reset() ; 
#endif
}

int ConnectionListingModel::rowCount(const QModelIndex& ) const
{
  return iConnections.size();
}

int ConnectionListingModel::columnCount(const QModelIndex& ) const
{
  return 5 ; 
}
 
QVariant ConnectionListingModel::data(const QModelIndex &index, int role) const {
  if(!index.isValid())
    return QVariant();

  if ( role == Qt::UserRole ) {
    return iConnections.at(index.row()).iNodeFingerPrint.toQVariant() ;
  }

  switch ( index.column() ) {
  case 0: // addr
    if(role == Qt::DisplayRole) {
      return iConnections.at(index.row()).iAddr.toString() ; 
    } else {
      return QVariant();
    }
    break ;
  case 1: // inbound?
    if(role == Qt::DisplayRole) {
      return iConnections.at(index.row()).iIsInBound ; 
    } else {
      return QVariant();     
    } 
    break ;

  case 2: // bytes in
    if(role == Qt::DisplayRole) {
      if ( iConnections.at(index.row()).iBytesIn < 1024 ) {
	return QString::number(iConnections.at(index.row()).iBytesIn) ;
      } else       if ( iConnections.at(index.row()).iBytesIn < (1024*1024) ) {
	return QString::number(iConnections.at(index.row()).iBytesIn/1024)+"k" ;
      } else {
	return QString::number(iConnections.at(index.row()).iBytesIn/(1024*1024))+"M" ;
      }
    } else {
      return QVariant();     
    } 
    break ;

  case 3: // bytes out
    if(role == Qt::DisplayRole) {
      if ( iConnections.at(index.row()).iBytesOut < 1024 ) {
	return QString::number(iConnections.at(index.row()).iBytesOut) ;
      } else       if ( iConnections.at(index.row()).iBytesOut < (1024*1024) ) {
	return QString::number(iConnections.at(index.row()).iBytesOut/1024)+"k" ;
      } else {
	return QString::number(iConnections.at(index.row()).iBytesOut/(1024*1024))+"M" ;
      }
    } else {
      return QVariant();     
    } 
    break ;

  case 4: // time when opened
    if(role == Qt::DisplayRole) {
      QDateTime d ; 
      d.setTime_t(iConnections.at(index.row()).iOpenTime) ;
      return d.toString(Qt::SystemLocaleShortDate) ;
    } else {
      return QVariant();     
    } 
    break ;

  default:
    return QVariant(); // for unknown columns return empty
  }

  return QVariant();
}

QVariant ConnectionListingModel::headerData ( int section, Qt::Orientation orientation, int role  ) const 
{
  if (orientation != Qt::Horizontal  ) {
    return QVariant();      
  }
  switch ( role ) {
  case Qt::ToolTipRole:
    switch ( section ) 
      {
      case 0:
	return tr("Peer network address") ;
	break;
      case 1:
	return tr("Inbound connections are those where peer initiated connection") ;
	break;
      case 2:
	return tr("Data transferred from peer to your node") ;
	break;
      case 3:
	return tr("Data transferred to peer from your node") ;
	break;
      case 4:
	return tr("Time when connection was opened") ;
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
	return tr("Address") ;
	break;
      case 1:
	return tr("Inbound") ; 
	break ;
      case 2:
	return tr("Bytes in") ; 
	break ;
      case 3:
	return tr("Bytes out") ; 
	break ;
      case 4:
	return tr("Open time") ; 
	break ;
      default:
	return QVariant();      
      }    
    break ;
  case Qt::SizeHintRole:
    switch ( section ) {
    case 0:
      return QSize(300,25) ; 
      break;
    case 1:
      return QSize(50,25) ; 
      break ;
    case 2:
      return QSize(50,25) ; 
      break ;
    case 3:
      return QSize(50,25) ; 
      break ;
    case 4:
      return QSize(50,25) ; 
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


void ConnectionListingModel::updateModelContents() {
  iModel.lock() ; 
  const QList <Connection *>& connections = iModel.getConnections() ;
  iConnections.clear() ;
  foreach ( const Connection *c, connections ) {
    if ( c ) {
      ConnectionDisplayItem i ;
      i.iAddr = c->peerAddress() ; 
      i.iIsInBound = c->isInbound() ; 
      i.iBytesIn = c->bytesIn(); 
      i.iBytesOut = c->bytesOut() ; 
      i.iOpenTime = c->getOpenTime() ; 
      iConnections.append(i) ; 
    }
  }
  iModel.unlock() ; 
}
