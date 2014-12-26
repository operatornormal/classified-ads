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
#ifdef WIN32
#define NOMINMAX 
#include <WinSock2.h>
#endif
#include "networkconnectorengine.h"
#include "../log.h"
#include "../controller.h"
#include "../datamodel/model.h"
#include "node.h"
#include "connection.h"
#include "networklistener.h"
#include <QNetworkInterface>
#include <QDateTime>
#ifdef WIN32

#else
#include "arpa/inet.h"
#endif
#include "../datamodel/profilemodel.h"
#include "../net/protocol_message_formatter.h"
#include <QHostInfo> // DNS lookups

NetworkConnectorEngine::NetworkConnectorEngine(Controller *aController,
    Model *aModel,
    Connection::ConnectionObserver& aObserver) :
  QThread(aController),
  iNeedsToRun(true),
  iController(aController),
  iModel(aModel),
  iConnectionObserver(aObserver),
  iBroadCastSocket(NULL),
  iTimeOfLastBroadCast(0),
  iTimeOfLastMsgPoll(0),
  iTimeOfLastMsgPollForProfile (0)
{

}

NetworkConnectorEngine::~NetworkConnectorEngine() {
  QLOG_STR("NetworkConnectorEngine::~NetworkConnectorEngine in/out") ; 
}

void NetworkConnectorEngine::run() {
  LOG_STR("NetworkConnectorEngine::run in") ;
  time_t timeOfLastNodeListUpdate (0) ;
  int connectedCount = 0 ; // initially must be, later ask datamodel
  if (   iBroadCastSocket == NULL ) {
    iBroadCastSocket = new QUdpSocket(this) ; 
  }
  while (iNeedsToRun ) {
    sendBroadCast() ;
    createConnectionsToNodesStoringPrivateMessages() ; 
    if ( tryServeWishListItem() == false ) {
      // if there is nothing in wishlist, do "normal procedure"
      // every 350 seconds
      if ( timeOfLastNodeListUpdate+50 < QDateTime::currentDateTimeUtc().toTime_t() ) {
	updateListOfNodesToConnect() ;
	timeOfLastNodeListUpdate = QDateTime::currentDateTimeUtc().toTime_t() ;
      }
      if (   connectedCount < KMaxOpenConnections ) {
	// so, lets start with list above ..
	if ( iAddressesInOrderToConnect.size() > 0 ) {
	  QPair<QHostAddress, int> addr = iAddressesInOrderToConnect.at(0) ;
	  QHostAddress a = addr.first ;
	  int p = addr.second ;
	  spawnNewConnection(a,p,KNullHash) ;
	  iAddressesInOrderToConnect.removeAt(0) ;
	}
      } else {
	// take a break, we have connections..
      }
      // in the end, query open open connections
      iModel->lock() ;
      QList <Connection *> currentConnections = iModel->getConnections()  ;
      connectedCount = currentConnections.size() ;
      iModel->unlock() ;
    } 
    // looks like we don't get "deleteLater()" signals processed
    // without this
    QCoreApplication::processEvents(QEventLoop::ExcludeUserInputEvents) ;
    msleep(800) ;
  } 
  delete iBroadCastSocket ; 
  iBroadCastSocket = NULL ; 
  LOG_STR("NetworkConnectorEngine::run out") ;
}

bool NetworkConnectorEngine::tryServeWishListItem() {
  const bool hasIpv6 =
    !Connection::Ipv6AddressesEqual(iController->getNode().ipv6Addr(),
                                    KNullIpv6Addr) ;
  iModel->lock() ;
  Node* n = iModel->nodeModel().nextConnectionWishListItem()  ;
  iModel->unlock() ;
  if ( n ) {
    QHostAddress addrOfNode ;
    if (n->DNSAddr().length() > 0 && 
	performDNSLookup(&addrOfNode,
			 n->DNSAddr(),
			 hasIpv6 )) {
      addrOfNode.setAddress(n->DNSAddr()) ;
    } else {
      if (!Connection::Ipv6AddressesEqual( n->ipv6Addr(),
					   KNullIpv6Addr ) ) {
	addrOfNode.setAddress( n->ipv6Addr() ) ;
      } else if (  n->ipv4Addr() ) {
	addrOfNode.setAddress( n->ipv4Addr() ) ;	 
      } else {
	// tor addresses? 
	delete n ; 
	LOG_STR("Connection wishlist-item had no ipv4/ipv6/dns addr") ;
	return false ; 
      }
    }
    iModel->lock() ;
    for ( int i = 0 ; i < iAddressesInOrderToConnect.size() ; i++ ) {
      QPair<QHostAddress, int> addrFromList = iAddressesInOrderToConnect.at(0) ;
      if (addrFromList.first == addrOfNode ) {
	// item is already on our list -> discard
	delete n ; 
	LOG_STR("Connection wishlist-item was already on list..") ; 
	iModel->unlock() ;
	return false ; 
      }
    }
    iModel->unlock() ;
    spawnNewConnection(addrOfNode,
		       n->port(),
		       n->nodeFingerPrint()) ;
    delete n ; 
    return true ; 

  } else {
    return false ; // nothing on wishlist
  }
}

void NetworkConnectorEngine::spawnNewConnection(const QHostAddress& aAddr,const int aPort, const Hash& aHashOfNodeToConnect) {
  NetworkListener *networkListener = iController->networkListener() ;
  QThread* t = new QThread();
  Connection *c =
    new   Connection(aAddr,aPort,
                     *networkListener, // connection observer
                     *iModel,
                     *iController,
                     aHashOfNodeToConnect);
  // as "NULL" was passed as parent to new connection, someone
  // (now datamodel destructor) needs to clean away the open
  // outgoing connections

  // datamodel is responsible for deleting outgoing connections,
  // with special connection state that incoming connections in turn
  // do not need use
  iModel->lock() ;
  // here we have a bug .. if IP addr changes, we'll advertise
  // our old address for the first peer to connect .. would
  // need to delay construction of node greeting to the moment
  // when we're already in connected state and can check
  // for local address of the socket.
  iModel->addOpenNetworkConnection(c) ;
  iModel->unlock() ;
  connect(t, SIGNAL(started()), c, SLOT(run()));

  connect(c, SIGNAL(finished()), t, SLOT(quit())) ;
  connect(t, SIGNAL(finished()), t, SLOT(deleteLater()));
  // after thread is gone, remove the connection too
  connect(t, SIGNAL(finished()), c, SLOT(deleteLater()));
  connect(c, SIGNAL(blackListNetworkAddr(QHostAddress ) ),
	  this, SLOT(blackListNetworkAddr(QHostAddress ) ),
	  Qt::QueuedConnection ) ; 
  c->moveToThread(t) ;
  t->start();
  LOG_STR("NetworkConnectorEngine::spawnNewConnection out") ;
}

void NetworkConnectorEngine::threadIsDeleted() {
  LOG_STR("NetworkConnectorEngine::threadIsDeleted") ;
}
//
// this method decides which nodes we actually connect.
//
void NetworkConnectorEngine::updateListOfNodesToConnect() {
  iModel->lock() ;
  const bool hasIpv6 =
    !Connection::Ipv6AddressesEqual(iController->getNode().ipv6Addr(),
                                    KNullIpv6Addr) ;
  QList<Node *>* nodeListFromModel = NULL ;
  iAddressesInOrderToConnect.clear()  ;
  // first check that nodes around us in hash-ring are connected
  // on both sides.
  nodeListFromModel =  iModel->nodeModel().getNodesBeforeHash
                       (iController->getNode().nodeFingerPrint(), 5) ;
  // from above list prune away self and those nodes already connected
  if ( pruneConnectionProspectList(nodeListFromModel) == false &&
       iModel->getConnections().size()  > ( KMaxOpenConnections/2 ) ) {
    // we did not have connection to neighbor but we had plenty
    // of connections -> kick one out
    iModel->nodeModel().closeOldestInactiveConnection() ;
  }
  // add remaining addresses to list..
  for ( int i = nodeListFromModel->size()-1 ; i>= 0 ; i-- ) {
    // prefer DNS name over other options, as this is something
    // that the node operator has manually set ; lets trust the operator
    if ( nodeListFromModel->value(i)->DNSAddr().length() > 0 ) {
      QHostAddress a ; 
      if ( performDNSLookup(&a,
			    nodeListFromModel->value(i)->DNSAddr(),
			    hasIpv6 )  ) {
	QPair<QHostAddress,int> p (a, nodeListFromModel->value(i)->port()) ;
	if ( !iAddressesInOrderToConnect.contains(p) ) {
	  iAddressesInOrderToConnect.append(p) ;
	  QLOG_STR("Appending addr to connect #3 " + a.toString()) ;
	}
      }
    } else {
      if ( !Connection::Ipv6AddressesEqual(nodeListFromModel->value(i)->ipv6Addr(),
					   KNullIpv6Addr) &&
	   hasIpv6 ) {
	// both we and the node have ipv6 -> use that
	QHostAddress a(nodeListFromModel->value(i)->ipv6Addr()) ;
	QPair<QHostAddress,int> p (a, nodeListFromModel->value(i)->port()) ;
	if ( !iAddressesInOrderToConnect.contains(p) ) {
	  iAddressesInOrderToConnect.append(p) ;
	  QLOG_STR("Appending addr to connect #1 " + a.toString()) ;
	}
      } else {
	// suppose ipv4 always..
	if ( nodeListFromModel->value(i)->ipv4Addr() ) {
	  QHostAddress a(nodeListFromModel->value(i)->ipv4Addr()) ;
	  QPair<QHostAddress,int> p (a, nodeListFromModel->value(i)->port()) ;
	  if ( !iAddressesInOrderToConnect.contains(p) ) {
	    iAddressesInOrderToConnect.append(p) ;
	    QLOG_STR("Appending addr to connect #2 " + a.toString()) ;
	  }
	}
      }
    } // end of "had no DNS name"
  }
  for ( int i = nodeListFromModel->size()-1 ; i>= 0 ; i-- ) {
    delete nodeListFromModel->value(i) ;
    nodeListFromModel->removeAt(i) ;
  }
  delete nodeListFromModel ;
  nodeListFromModel = NULL ;

  // then do the same for ring-forward-connections
  nodeListFromModel =  iModel->nodeModel().getNodesAfterHash
                       (iController->getNode().nodeFingerPrint(), 5) ;
  // from above list prune away self and those nodes already connected
  if ( pruneConnectionProspectList(nodeListFromModel) == false &&
       iModel->getConnections().size()  > ( KMaxOpenConnections/2 ) ) {
    // we did not have connection to neighbor but we had plenty
    // of connections -> kick one out
    iModel->nodeModel().closeOldestInactiveConnection() ;
  }
  // add remaining addresses to list..
  for ( int i = nodeListFromModel->size()-1 ; i>= 0 ; i-- ) {
    if ( !Connection::Ipv6AddressesEqual(nodeListFromModel->value(i)->ipv6Addr(),
                                         KNullIpv6Addr) &&
         hasIpv6 ) {
      // both we and the node have ipv6 -> use that
      QHostAddress a(nodeListFromModel->value(i)->ipv6Addr()) ;
      QPair<QHostAddress,int> p (a, nodeListFromModel->value(i)->port()) ;
      if ( !iAddressesInOrderToConnect.contains(p) ) {
        iAddressesInOrderToConnect.append(p) ;
        QLOG_STR("Appending addr to connect #3 " + a.toString()) ;
      }
    } else {
      // suppose ipv4 always..
      if ( nodeListFromModel->value(i)->ipv4Addr() ) {
        QHostAddress a(nodeListFromModel->value(i)->ipv4Addr()) ;
        QPair<QHostAddress,int> p (a, nodeListFromModel->value(i)->port()) ;
        if ( !iAddressesInOrderToConnect.contains(p) ) {
          iAddressesInOrderToConnect.append(p) ;
          QLOG_STR("Appending addr to connect #4 " + a.toString()) ;
        }
      }
    }
  }
  for ( int i = nodeListFromModel->size()-1 ; i>= 0 ; i-- ) {
    delete nodeListFromModel->value(i) ;
    nodeListFromModel->removeAt(i) ;
  }
  delete nodeListFromModel ;
  nodeListFromModel = NULL ;
  QList<QHostAddress> ipAddrList = QNetworkInterface::allAddresses() ;
  // then get hot addresses ..
  QList<QPair<QHostAddress,int> > hotaddresses = iModel->nodeModel().getHotAddresses() ;
  for ( int i = hotaddresses.size()-1 ; i >= 0 ; i-- ) {
    if ( iAddressesInOrderToConnect.contains(hotaddresses.value(i) ) ) {
      // was already
    } else {
      // check that node is not already connected
      bool found = false ;
      Connection* c = NULL ;
      for ( int j = iModel->getConnections().size()-1 ; j >= 0 ; j-- ) {
        // and don't connect to already-connected nodes
        c = iModel->getConnections().value(j) ;
        if ( c->peerAddress() == hotaddresses.value(i).first ) {
          found = true ;
          LOG_STR2("Pruning already connected hot addr index %d", i) ;
          break ;
        } else {
          QLOG_STR("Was not: " + hotaddresses.value(i).first.toString() + " equal to " + c->peerAddress().toString() ) ;
        }
      }
      // check again that our own addr is not among hot addresses:
      for ( int k = 0 ; k < ipAddrList.size() ; k++ ) {
        QHostAddress a = ipAddrList[k] ;
        if (  hotaddresses.value(i).first == a ) {
          found = true ;
          QLOG_STR("Pruned from hotlist own network addr " + a.toString()) ;
          break;
        }
      }
      if ( !found ) {
        iAddressesInOrderToConnect.append(hotaddresses.value(i)) ;
        QLOG_STR("Appending addr to connect #5 " + hotaddresses.value(i).first.toString()) ;
      }
    }
  }
  iModel->unlock() ;
}

bool NetworkConnectorEngine::pruneConnectionProspectList(QList<Node *>* aListToPrune ) const {
  bool retval = false ;
  const Node* nodeOfConnection = NULL ;
  QList<QHostAddress> ipAddrList = QNetworkInterface::allAddresses() ;
  for ( int i = aListToPrune->size()-1 ; i >= 0 ; i-- ) {
    // if our own node got included, remove that from list
    bool prunedAlready = false ;
    if ( iController->getNode().nodeFingerPrint()
         == aListToPrune->value(i)->nodeFingerPrint()   ) {
      delete aListToPrune->value(i) ;
      aListToPrune->removeAt(i) ;
    } else {
      // and remove all nodes already connected
      for ( int j = iModel->getConnections().size()-1 ; j >= 0 ; j-- ) {
        // and don't connect to already-connected nodes
        nodeOfConnection = iModel->getConnections().value(j)->node() ;
        if ( nodeOfConnection &&
             ( nodeOfConnection->nodeFingerPrint() == aListToPrune->value(i)->nodeFingerPrint() ) ) {
          delete aListToPrune->value(i) ;
          aListToPrune->removeAt(i) ;
          retval = true ;
          prunedAlready= true ;
          break; // out from j-loop because i to compare agains is not any more
        }
      }
      // and check that no our own addresses are at list:
      for ( int k = 0 ; !prunedAlready && k < ipAddrList.size() ; k++ ) {
        QHostAddress a = ipAddrList[k] ;
        if ( ( a.protocol() == QAbstractSocket::IPv4Protocol &&
               aListToPrune->value(i)->ipv4Addr() > 0 &&
               QHostAddress(aListToPrune->value(i)->ipv4Addr()) == a ) || ( ! Connection::Ipv6AddressesEqual(KNullIpv6Addr,
                   aListToPrune->value(i)->ipv6Addr()) &&
                   a.protocol() == QAbstractSocket::IPv6Protocol &&
                   QHostAddress(aListToPrune->value(i)->ipv6Addr()) == a )  ) {
          delete aListToPrune->value(i) ;
          aListToPrune->removeAt(i) ;
          retval = true ;
          QLOG_STR("Pruned own network addr " + a.toString()) ;
          break;
        }
      }
    }
  }
  // check blacklist too:
  bool prunedAlready = false ;
  for ( int i = aListToPrune->size()-1 ; i >= 0 ; i-- ) {
    prunedAlready = false ;
    if ( aListToPrune->value(i)->ipv4Addr() ) {
      QHostAddress ip4(aListToPrune->value(i)->ipv4Addr()) ; 
      if ( iAddressBlacklist.contains(ip4) ) {
	delete aListToPrune->value(i) ;
	aListToPrune->removeAt(i) ;
	prunedAlready = true ; 
	QLOG_STR("Pruned due to blacklist " + ip4.toString()) ;
      }
    }
    if ( prunedAlready == false &&
	 ( ! Connection::Ipv6AddressesEqual(KNullIpv6Addr,
					    aListToPrune->value(i)->ipv6Addr()) ) ) {
      QHostAddress ip6(aListToPrune->value(i)->ipv6Addr()) ; 
      if ( iAddressBlacklist.contains(ip6) ) {
	delete aListToPrune->value(i) ;
	aListToPrune->removeAt(i) ;
	prunedAlready = true ; 
	QLOG_STR("Pruned due to blacklist " + ip6.toString()) ;
      }
    }
  }
  return retval ;
}

void NetworkConnectorEngine::sendBroadCast() {
  if ( iTimeOfLastBroadCast + 60 < QDateTime::currentDateTimeUtc().toTime_t()){
    QByteArray dataToSend ; 
    dataToSend.append('c') ;
    dataToSend.append('a') ;
    bool hasIpv6 =
      !Connection::Ipv6AddressesEqual(iController->getNode().ipv6Addr(),
				      KNullIpv6Addr) ;
    if ( hasIpv6 ) {
      dataToSend.append('6') ;
    } else {
      dataToSend.append('4') ;
    }

    quint32 nodeHash1NetworkBO ( htonl ( iController->getNode().nodeFingerPrint().iHash160bits[0]) ) ;
    quint32 nodeHash2NetworkBO ( htonl ( iController->getNode().nodeFingerPrint().iHash160bits[1]) ) ;
    quint32 nodeHash3NetworkBO ( htonl ( iController->getNode().nodeFingerPrint().iHash160bits[2]) ) ;
    quint32 nodeHash4NetworkBO ( htonl ( iController->getNode().nodeFingerPrint().iHash160bits[3]) ) ;
    quint32 nodeHash5NetworkBO ( htonl ( iController->getNode().nodeFingerPrint().iHash160bits[4]) ) ;

    dataToSend.append((const char *)(&nodeHash1NetworkBO), sizeof(quint32) ) ;
    dataToSend.append((const char *)(&nodeHash2NetworkBO), sizeof(quint32) ) ;
    dataToSend.append((const char *)(&nodeHash3NetworkBO), sizeof(quint32) ) ;
    dataToSend.append((const char *)(&nodeHash4NetworkBO), sizeof(quint32) ) ;
    dataToSend.append((const char *)(&nodeHash5NetworkBO), sizeof(quint32) ) ;

    quint32 portNumber  ( iController->getNode().port() ) ;
    if ( portNumber > 0 ) {
      quint32 portNumberNetworkBO ( htonl ( portNumber ) ) ; 
      dataToSend.append((const char *)(&portNumberNetworkBO), sizeof(quint32) ) ;

      if ( hasIpv6 ) {
	// include also our ipv6-addr in broadcase as in ipv6 
	// case the sender of the UDP packet at the receiving
	// end is our link-local addr FE:80:.. and it may be
	// difficult to connect as network interface must be
	// specified too .. so , include our advertised addr:
	for ( int i = 0 ; i < 16 ; i++ ) {
	  dataToSend.append(iController->getNode().ipv6Addr().c[i]) ; 
	}
      }
      iTimeOfLastBroadCast = QDateTime::currentDateTimeUtc().toTime_t() ;
      // hmm, send broadcast only over ipv4. there seems to be 
      // some trouble in binding qudpsocket into ipv6 interface
      // in the receiving end..
      iBroadCastSocket->writeDatagram(dataToSend.data(), dataToSend.size(),
				      QHostAddress::Broadcast, KBroadCastAdvertismentPort);

    }
  }
}

void NetworkConnectorEngine::blackListNetworkAddr(QHostAddress aAddr) 
{
  iModel->lock() ;
  if ( iAddressBlacklist.contains(aAddr) == false ) {
    iAddressBlacklist.append(aAddr) ; 
  }
  iModel->unlock() ;
}


void NetworkConnectorEngine::createConnectionsToNodesStoringPrivateMessages() 
{
  if ( iPrivateMessagePollWishList.size() == 0 ) {
    iProfileToPollPrivateMessages = KNullHash ; // no poll in progress
  }
  if ( ( iProfileToPollPrivateMessages == KNullHash &&
	 (iTimeOfLastMsgPoll + 60) < QDateTime::currentDateTimeUtc().toTime_t())
       || 
       ( iProfileToPollPrivateMessages != KNullHash &&
	 (iTimeOfLastMsgPoll + 600) < QDateTime::currentDateTimeUtc().toTime_t() )
       ) {
    LOG_STR2("Checking private msg poll, seconds since last = %u " ,(unsigned)( ( QDateTime::currentDateTimeUtc().toTime_t() - iTimeOfLastMsgPoll))) ;
    iTimeOfLastMsgPoll = QDateTime::currentDateTimeUtc().toTime_t() ;
    // in here find a profile that we have private keys for and
    // that has not had its private messages polled for given
    // time (say, 15 minutes). when such a profile is found, 
    // initiate connections to nodes supposed to contain private
    // messages destined to profile.
    iModel->lock() ;
    iProfileToPollPrivateMessages = iModel->profileModel().profileWithOldestTimeSinceMsgPoll(&iTimeOfLastMsgPollForProfile) ;
    if ( iProfileToPollPrivateMessages != KNullHash ) {
      iPrivateMessagePollWishList.clear() ;
      QLOG_STR("Polling private messages for " + iProfileToPollPrivateMessages.toString()) ;
      QList<Node *>* nodeListFromModel = NULL ;
      // so try get 5 nodes no older than 60 minutes
      nodeListFromModel =  iModel->nodeModel().getNodesAfterHash
	(iProfileToPollPrivateMessages, 5, 60) ;
      // then from that list check nodes that are already connected
      // and send a query to those nodes
      if ( nodeListFromModel ) {
	iPrivateMessagePollWishList.clear() ; 
	for ( int i = nodeListFromModel->size()-1 ; i >= 0 ; i -- ) {
	  if ( iModel->nodeModel().isNodeAlreadyConnected(nodeListFromModel->value(i)->nodeFingerPrint() ) ) {
	    sendPrivateMessagesPoll(nodeListFromModel->value(i)->nodeFingerPrint()) ;
	  } else {
	    // put node into wishlist
	    iPrivateMessagePollWishList << nodeListFromModel->value(i)->nodeFingerPrint() ;
	    iModel->nodeModel().addNodeToConnectionWishList(nodeListFromModel->value(i)->nodeFingerPrint()) ;
	  }	
	  delete nodeListFromModel->takeAt(i) ; 
	}
	delete nodeListFromModel ; 
	nodeListFromModel = NULL ; 
      }
    }
    iModel->unlock() ;
  }
}

void NetworkConnectorEngine::sendPrivateMessagesPoll(const Hash& aNodeFingerPrint) {
  if ( iProfileToPollPrivateMessages != KNullHash ) {
      QByteArray* resultBytes = new QByteArray() ;
      resultBytes->append(ProtocolMessageFormatter::requestForPrivateMessagesForProfile(iProfileToPollPrivateMessages,
											iTimeOfLastMsgPollForProfile)) ;
      iModel->addItemToSend(aNodeFingerPrint,
			    resultBytes) ;
      QByteArray* resultBytes2 = new QByteArray() ;
      resultBytes2->append(ProtocolMessageFormatter::requestForProfileComments(iProfileToPollPrivateMessages,
									      iTimeOfLastMsgPollForProfile)) ;
      iModel->addItemToSend(aNodeFingerPrint,
			    resultBytes2) ;
      iModel->profileModel().setPrivateMessagePollTimeForProfile(QDateTime::currentDateTimeUtc().toTime_t(),iProfileToPollPrivateMessages) ;
  }
  return ; 
}

void NetworkConnectorEngine::nodeConnectionAttemptStatus(Connection::ConnectionState aStatus,
    const Hash aHashOfAttemptedNode ) {
  iModel->lock() ;
  if ( iPrivateMessagePollWishList.contains(aHashOfAttemptedNode) ) {
    if ( aStatus == Connection::Open ) {
      sendPrivateMessagesPoll(aHashOfAttemptedNode) ;
    }
    iPrivateMessagePollWishList.removeAll(aHashOfAttemptedNode) ;
  }
  if ( iPrivateMessagePollWishList.size() == 0 ) {
    iProfileToPollPrivateMessages  = KNullHash ; // this profile now done
  }
  iModel->unlock() ;
}

bool NetworkConnectorEngine::performDNSLookup(QHostAddress* aAddressToBeSet,
					      const QString& aFromDnsName,
					      bool aDoWeHaveIpv6 ) {
  bool isAddressSet = false ; 
  QHostInfo info = QHostInfo::fromName(aFromDnsName) ;
  if ( info.error() == QHostInfo::NoError ) {
    bool isAddressSet ( false ) ; 
    // check for ipv6 addr if we have one
    if ( aDoWeHaveIpv6 ) {
      foreach ( const QHostAddress& result,
		info.addresses() ) {
	if ( result.protocol() == QAbstractSocket::IPv6Protocol ) {
	  isAddressSet = true ; 
	  aAddressToBeSet->setAddress(result.toIPv6Address()) ; 
	  break  ;
	}
      }
    }
    if ( isAddressSet == false ) {
      foreach ( const QHostAddress& result,
		info.addresses() ) {
	if ( result.protocol() == QAbstractSocket::IPv4Protocol ) {
	  isAddressSet = true ; 
	  aAddressToBeSet->setAddress(result.toIPv4Address()) ; 
	  break  ;
	}
      }
    }
  } else {
    QLOG_STR("DNS lookup failure for " + aFromDnsName ) ; 
  }
  return isAddressSet ; 
}
