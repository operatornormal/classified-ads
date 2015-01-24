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
#include "connection.h"
#include "../log.h"
#include <QSslSocket>
#include "../datamodel/model.h"
#ifdef WIN32

#else
#include "arpa/inet.h"
#endif
#include <QDateTime>
#include <QCoreApplication>
#include <QThread>
#include "../datamodel/profilemodel.h"
#include "../datamodel/camodel.h"
#include "../datamodel/binaryfilemodel.h"
#include "../datamodel/privmsgmodel.h"
#include "../datamodel/profilecommentmodel.h"

static const QPair<QHostAddress, int> normalNats1 ( QHostAddress::parseSubnet("10.0.0.0/8") );
static const QPair<QHostAddress, int> normalNats2 ( QHostAddress::parseSubnet("172.16.0.0/20"));
static const QPair<QHostAddress, int> normalNats3 ( QHostAddress::parseSubnet("192.168.0.0/16") );

Connection::Connection (const QHostAddress& aAddr,
                        const int aPort,
                        ConnectionObserver& aObserver,
                        Model& aModel,
                        MController& aController,
                        const Hash& aFpOfNodeToTry)
  : iNeedsToRun(true),
    iTimeOfLastActivity(QDateTime::currentDateTimeUtc().toTime_t()),
    iNumberOfPacketsReceived(0),
    iConnectionState(Initial),
    iObserver(aObserver) ,
    iModel(aModel),
    iBytesExpectedInPacketBeingRead (0),
    iBytesRead(NULL),
    iInvocationsSinceLastByteReceived(0),
    iNodeOfConnectedPeer(NULL),
    iAddrToConnect(aAddr),
    iPortToConnect(aPort),
    iSocketDescriptor(-1) ,
    iSocketIsIncoming(false),
    iController(aController),
    iFpOfNodeWeTrying( aFpOfNodeToTry),
    iStageOfBucketFill( OwnNodeGreeting ),
    iTimeOfBucketFill(QDateTime::currentDateTimeUtc().toTime_t()),
    iSocketOpenTime(QDateTime::currentDateTimeUtc().toTime_t()),
    iBytesIn(0), 
    iBytesOut(0),
    iPeerAddress(aAddr)
{
  LOG_STR(QString("0x%1 ").arg((qulonglong)this, 8) +"Connection::Connection outgoing") ;
}

Connection::Connection(int aSocketDescriptor,
                       ConnectionObserver& aObserver,
                       Model &aModel,
                       MController& aController)
  : iNeedsToRun(true),
    iTimeOfLastActivity(QDateTime::currentDateTimeUtc().toTime_t()),
    iNumberOfPacketsReceived(0),
    iConnectionState(Initial),
    iObserver(aObserver) ,
    iModel(aModel),
    iBytesExpectedInPacketBeingRead (0),
    iBytesRead(NULL),
    iInvocationsSinceLastByteReceived(0),
    iNodeOfConnectedPeer(NULL),
    iPortToConnect(-1),
    iSocketDescriptor(aSocketDescriptor),
    iSocketIsIncoming(true),
    iController(aController),
    iStageOfBucketFill( OwnNodeGreeting ),
    iTimeOfBucketFill(QDateTime::currentDateTimeUtc().toTime_t() ),
    iSocketOpenTime(QDateTime::currentDateTimeUtc().toTime_t()),
    iBytesIn(0), 
    iBytesOut(0)  {
  LOG_STR(QString("0x%1 ").arg((qulonglong)this, 8) +"Connection::Connection incoming") ;
}


Connection::~Connection () {
  iModel.lock() ; 
  LOG_STR(QString("0x%1 ").arg((qulonglong)this, 8) +"Connection::~Connection") ;
  // here note that we have member variable iSocket that is a pointer.
  // iSocket is a automatic variable of the run() method and
  // iSocket just points to that ; iSocket naturally will be
  // deleted when context of run() goes out of scope.
  if ( iNextProtocolItemToSend.size() ) {
    for ( int i = iNextProtocolItemToSend.size()-1 ; i >= 0 ; i-- ) {
      QByteArray* item = iNextProtocolItemToSend.value(i) ;
      delete item ;
      iNextProtocolItemToSend.removeAt(i) ;
    }
  }
  if ( iNodeOfConnectedPeer ) {      
    delete iNodeOfConnectedPeer ;
    iNodeOfConnectedPeer = NULL ; 
  }
  if ( iBytesRead ) {
    delete iBytesRead ;
    iBytesRead = NULL ;
  }
  iSocket = NULL ; 
  iModel.unlock() ; 
}

void Connection::run() {
  LOG_STR(QString("0x%1 ").arg((qulonglong)this, 8) +"Connection::run") ;

  iBytesRead = new QByteArray() ;
  iBytesRead->clear() ;

  if ( iSocketIsIncoming ) {
    runForIncomingConnections() ;
  } else {
    runForOutgoingConnections() ;

  }
  iObserver.connectionClosed(this) ; // will call datamodel.lock() inside
  LOG_STR(QString("0x%1 ").arg((qulonglong)this, 8) +"Connection::run out") ;
  emit finished() ; 
}

void Connection::runForIncomingConnections() {
  QSslSocket sock ;
  iSocket = &sock ;
  if (iSocket->setSocketDescriptor(iSocketDescriptor)) {
    connect(iSocket, SIGNAL(encrypted()), this, SLOT(socketReady()));
    // the idea here with model.lock is that
    // this class instance here is owned by datamodel
    // and datamodel may frobnicate our internals so
    // if we touch any part of this class that is somehow
    // visible to outside, we must first lock(), then
    // release(). Frobnication of private parts that
    // are guaranteed to not have a get()ter method
    // or anything should be ok, and of course the
    // automatic variables.
    if ( iModel.lock() == false ) {
      return ;
    }
    iSocket->setLocalCertificate(iModel.nodeModel().nodeCert());
    iSocket->setPrivateKey(iModel.nodeModel().nodeKey());
    iModel.unlock() ;
    iSocket->startServerEncryption();
  } else {
    LOG_STR(QString("0x%1 ").arg((qulonglong)this, 8) +"iSocket->setSocketDescriptor failed?????") ;
    return ;
  }
  if ( iSocket->waitForEncrypted(10*1000) == false ) {
    LOG_STR(QString("0x%1 ").arg((qulonglong)this, 8) +"socket was not encrypted in 10 seconds -> out") ;
    iSocket->abort() ; 
    return ;
  }

  iPeerAddress = iSocket->peerAddress() ;
  if ( iPeerAddress == iSocket->localAddress() ) {
    QLOG_STR(QString("0x%1 ").arg((qulonglong)this, 8) + "Connected to self, addr = " + iSocket->peerAddress().toString()) ;
    // oh great, broadcast?
    // connected to self
    iNeedsToRun = false ; 
    return ; 
  }

  // ok, obviously we can receive, mark that down for future
  // advertisment:
  iModel.lock() ;
  iController.getNode().setIpAddrWithChecks(iSocket->localAddress()) ;
  iModel.unlock() ;
  // ok, we came here, we have SSL handshake done ; lets
  // instantiate some data structures..
  Hash fingerPrintOfPeer (iSocket->peerCertificate()) ;
  // it also may be so that connection was attempted to a particular
  // node - is it possible that we managed to connect other node
  // than that?
  if (  iFpOfNodeWeTrying != KNullHash  ) { // if other than null hash
    if (  iFpOfNodeWeTrying != fingerPrintOfPeer)  {
      iSocket->disconnectFromHost() ;
      iNeedsToRun = false ; // this causes readloop to return immediately
    }
  }
  if ( iController.getNode().nodeFingerPrint() == fingerPrintOfPeer ) {
    // damn, connected to self..
    emit blackListNetworkAddr(iSocket->peerAddress()) ; 
    if ( iSocketIsIncoming == false ) {
      emit blackListNetworkAddr(iAddrToConnect) ; 
    }
    iSocket->disconnectFromHost() ;
    iNeedsToRun = false ; // this causes readloop to return immediately
    LOG_STR(QString("0x%1 ").arg((qulonglong)this, 8) +"Connection::run incoming connected to self") ;
  }
  iNodeOfConnectedPeer= new Node(fingerPrintOfPeer, -1) ;

  // lets here do so that lets not frobnicate any instance
  // variables ; let the iSocket be only one and that in
  // turn will be frobnicated only from here.
  // if we need to send something, we ask datamodel for
  // stuff to send, take it into local variable here
  // and do our sending. this way we can manage with
  // single mutex in model and things will be in sync.
  readLoop() ;
  iModel.lock() ;
  iConnectionState = Connection::Closing ;
  iModel.connectionStateChanged(Connection::Closing, this) ;
  iModel.unlock() ;
  if ( iNeedsToRun == false && iSocket &&  iSocket->state() == QAbstractSocket::ConnectedState ) {
    iSocket->close() ; 
  }
  iSocket->waitForDisconnected(10*1000) ;
  iSocket = NULL ;
  LOG_STR(QString("0x%1 ").arg((qulonglong)this, 8) +"Connection::run(incoming) out") ;
}

void Connection::socketReady() {
  LOG_STR(QString("0x%1 ").arg((qulonglong)this, 8) +"Connection::socketReady") ;
  iModel.lock() ;
  iConnectionState = Open ;
  iModel.connectionStateChanged(iConnectionState, this) ;
  iModel.unlock() ;
}

void Connection::runForOutgoingConnections () {
  LOG_STR(QString("0x%1 ").arg((qulonglong)this, 8) +"Connection::runForOutgoingConnections in") ;
  iModel.lock() ;
  iModel.connectionStateChanged(Connection::Initial ,
                                this) ;
  iModel.unlock() ;
  QSslSocket sock ;
  iSocket = &sock ;

  connect(iSocket, SIGNAL(encrypted()), this, SLOT(socketReady()));
  // the idea here with model.lock is that
  // this class instance here is owned by datamodel
  // and datamodel may frobnicate our internals so
  // if we touch any part of this class that is somehow
  // visible to outside, we must first lock(), then
  // release(). Frobnication of private parts that
  // are guaranteed to not have a get()ter method
  // or anything should be ok, and of course the
  // automatic variables.
  if ( iModel.lock() == false ) {
    return ;
  }
  iSocket->setLocalCertificate(iModel.nodeModel().nodeCert());
  iSocket->setPrivateKey(iModel.nodeModel().nodeKey());
  iModel.unlock() ;
  iSocket->setPeerVerifyMode(QSslSocket::QueryPeer) ;
  QLOG_STR(QString("0x%1 ").arg((qulonglong)this, 8) +"Connecting to " + iAddrToConnect.toString() + " port " + QString::number(iPortToConnect) ) ;
  iSocket->connectToHostEncrypted(iAddrToConnect.toString(), iPortToConnect);

  if(!iSocket->waitForEncrypted(10*1000)) {
    LOG_STR(QString("0x%1 ").arg((qulonglong)this, 8) +"connectToHostEncrypted failed") ;
    iSocket->abort() ; 
    return ;
  }

  if ( iSocket->peerAddress() == iSocket->localAddress() ) {
    QLOG_STR(QString("0x%1 ").arg((qulonglong)this, 8) +"Connected to self, addr = " + iSocket->peerAddress().toString()) ;
    // oh great, broadcast?
    // connected to self
    iNeedsToRun = false ; 
    return ; 
  }

  Hash fingerPrintOfPeer (iSocket->peerCertificate()) ;

  // it also may be so that connection was attempted to a particular
  // node - is it possible that we managed to connect other node
  // than that?
  if (  iFpOfNodeWeTrying != KNullHash  ) { // if other than null hash
    if (  iFpOfNodeWeTrying != fingerPrintOfPeer ) {
      iSocket->disconnectFromHost() ;
      iNeedsToRun = false ; // this causes readloop to return immediately
    }
  }
  if ( iController.getNode().nodeFingerPrint() == fingerPrintOfPeer ) {
    // damn, connected to self..
    emit blackListNetworkAddr(iSocket->peerAddress()) ; 
    if ( iSocketIsIncoming == false ) {
      emit blackListNetworkAddr(iAddrToConnect) ; 
    }
    iSocket->disconnectFromHost() ;
    iNeedsToRun = false ; // this causes readloop to return immediately
    LOG_STR(QString("0x%1 ").arg((qulonglong)this, 8) +"Connection::run outgoing connected to self") ;
  }
  iNodeOfConnectedPeer= new Node(fingerPrintOfPeer, -1) ;

  // ok, we came here, we have SSL handshake done ; lets
  // instantiate some data structures..
  iNodeOfConnectedPeer= new Node(Hash(fingerPrintOfPeer),
                                 -1) ;
  // we just managed to connect, mark that thing down for
  // future advertisement
  iModel.lock() ;
  iNodeOfConnectedPeer->setCanReceiveIncoming(true) ;
  iController.getNode().setIpAddrWithChecks(iSocket->localAddress()) ;
  iModel.unlock() ;

  readLoop() ;
  if ( iNeedsToRun == false && iSocket &&  iSocket->state() == QAbstractSocket::ConnectedState ) {
    iSocket->close() ; 
  }
  if ( iNeedsToRun ) {
    iSocket->waitForDisconnected(30*1000) ;
    iSocket = NULL ;
  }
  LOG_STR(QString("0x%1 ").arg((qulonglong)this, 8) +"Connection::run  (outgoing )out") ;
}


// this is called from run() method when there is data available
void Connection::performRead() {
  if ( iBytesExpectedInPacketBeingRead == 0 ) {
    // we're not reading a packet yet, read packet
    quint32 bytesExpectedNetworkByteOrder = 0 ;
    if ( iSocket->read((char *)&bytesExpectedNetworkByteOrder,
                       sizeof(quint32)) != sizeof(quint32)) {
      iNeedsToRun = false ;
      iModel.lock() ;
      iModel.connectionStateChanged(Connection::Closing, this) ;
      iModel.unlock() ;
      iSocket->disconnectFromHost() ;
      return ;
    } else {
      iBytesIn += sizeof(quint32) ; 
    }
    iBytesExpectedInPacketBeingRead = ntohl ( bytesExpectedNetworkByteOrder);
    QLOG_STR(QString("0x%1 ").arg((qulonglong)this, 8) +"Connection::performRead expecting bytes= "+ QString::number( iBytesExpectedInPacketBeingRead)) ;
  }
  quint64 bytesAvail = 0 ;
  while ( iBytesExpectedInPacketBeingRead > 0 &&
          iBytesExpectedInPacketBeingRead > (quint64)(iBytesRead->size()) &&
          (bytesAvail = iSocket->bytesAvailable()) > 0 ) {
    if ( bytesAvail > iBytesExpectedInPacketBeingRead ) {
      bytesAvail = iBytesExpectedInPacketBeingRead;
    }
    QByteArray bytesReadThisTime ( iSocket->read(bytesAvail) ) ;
    if ( bytesReadThisTime.size() > 0 ) {
      iBytesRead->append(bytesReadThisTime) ;
      iBytesIn += bytesReadThisTime.size() ; 
      iInvocationsSinceLastByteReceived = 0 ;
      // seems like we received something:
      iTimeOfLastActivity = QDateTime::currentDateTimeUtc().toTime_t() ;
    }
  }
  if ( iBytesExpectedInPacketBeingRead == (quint64)(iBytesRead->size()) ) {
    // if we came here, we have one total packet..
    if ( iObserver.dataReceived(*iBytesRead,
                                *this) == false ) {
      QLOG_STR(QString("0x%1 ").arg((qulonglong)this, 8) +"### Closing due to unparseable data, peer = " + peerAddress().toString()) ; 
      iSocket->disconnectFromHost() ;
      iNeedsToRun = false ;
    }
    iBytesExpectedInPacketBeingRead = 0 ;
    iBytesRead->clear() ;
    iInvocationsSinceLastByteReceived = 0 ;
    iNumberOfPacketsReceived++ ;
  }
}

bool Connection::Ipv6AddressesEqual(const Q_IPV6ADDR& aAddr1,
                                    const Q_IPV6ADDR& aAddr2) {
  for ( int i = 0 ; i < 16 ; i++ ) {
    if ( aAddr1.c[i] != aAddr2.c[i]) return false ;
  }
  return true ;
}

void Connection::readLoop() {
  iObserver.connectionReady(this) ; // before starting to read, inform the observer
  QByteArray *itemToSend = NULL ;
  while ( iNeedsToRun && iSocket->state() == QAbstractSocket::ConnectedState ) {
    if ( iSocket->waitForReadyRead(500) == true ) {
      if ( iNeedsToRun ) {
	performRead() ;
      }
    } else {
      QCoreApplication::processEvents(QEventLoop::ExcludeUserInputEvents) ;
      iModel.lock() ;
      checkForBucketFill() ; 
      iModel.unlock() ; 
      // we came here because we have no bytes to read from
      // sock. first check if we do have read operation pending
      if ( iBytesExpectedInPacketBeingRead > 0 &&
           iBytesExpectedInPacketBeingRead > (quint64)(iBytesRead->size())) {
        iInvocationsSinceLastByteReceived++ ; // gets incremented every 500ms
      }
      if ( iInvocationsSinceLastByteReceived > 1000 ) {
        // we have had read operation pending for 500 seconds and
        // not a single byte has got through -> give up
	if ( iNeedsToRun ) {
	  iModel.lock() ;
	  LOG_STR(QString("0x%1 ").arg((qulonglong)this, 8) +"Read operation was pending too long, closing")  ;
	  iConnectionState = Connection::Closing ;
	  iModel.connectionStateChanged(Connection::Closing, this) ;
	  iModel.unlock() ;
	  iSocket->disconnectFromHost() ;
	  iNeedsToRun = false ;
	  LOG_STR(QString("0x%1 ").arg((qulonglong)this, 8) +"Giving up pending read operation after 500 seconds");
	}
        break ;
      }
      // ok, then try write operation, if applicable
      if ( iNeedsToRun ) {
	iModel.lock() ;
	itemToSend = iModel.nodeModel().getNextItemToSend(*this) ;
	iModel.unlock() ;
      }
      // now we own itemToSend
      if ( itemToSend != NULL &&
           iSocket != NULL &&
           iSocket->state() == QAbstractSocket::ConnectedState ) {
        quint32 bytesInPayload = itemToSend->size() ;
        quint32 bytesInPayloadNetworkOrder = htonl(bytesInPayload) ;
        QByteArray messageSizeBytes ;
        messageSizeBytes.append((const char *)&bytesInPayloadNetworkOrder,
                                sizeof(quint32)) ;
        if ( iSocket->write(messageSizeBytes) != sizeof(quint32) ) {
          LOG_STR(QString("0x%1 ").arg((qulonglong)this, 8) +"Write operation did not succeed, closing..")  ;
	  if ( iNeedsToRun ) {
	    iModel.lock() ;
	    iConnectionState = Connection::Closing ;
	    iModel.connectionStateChanged(Connection::Closing, this) ;
	    iModel.unlock() ;
	  }
          iSocket->disconnectFromHost() ;
          iNeedsToRun = false ;
        } else {
	  iBytesOut += sizeof(quint32) ;
	}
        if ( iSocket->write(*itemToSend) != itemToSend->size() ) {
          LOG_STR(QString("0x%1 ").arg((qulonglong)this, 8) +"Write operation did not succeed, closing..")  ;
	  if ( iNeedsToRun ) {
	    iModel.lock() ;
	    iConnectionState = Connection::Closing ;
	    iModel.connectionStateChanged(Connection::Closing, this) ;
	    iModel.unlock() ;
	  }
          iSocket->disconnectFromHost() ;
          iNeedsToRun = false ;
        } else {
	  iBytesOut += itemToSend->size() ; 
	}
        // ok ; how long to wait for write.
        // very slow network here. have 100 connected clients.
        // say, upload rate is 20kb/s, divided by 100 clients
        // we get 204 bytes per sec. .. but at least 30 seconds.
        if ( iSocket->waitForBytesWritten(((itemToSend->size() / 204) * 1000) + (30 * 1000)) == false ) {
          LOG_STR(QString("0x%1 ").arg((qulonglong)this, 8) +"Write wait operation did not succeed, closing..")  ;
	  if ( iNeedsToRun ) {
	    iModel.lock() ;
	    iConnectionState = Connection::Closing ;
	    iModel.connectionStateChanged(Connection::Closing, this) ;
	    iModel.unlock() ;
	  }
          iSocket->disconnectFromHost() ;
          iNeedsToRun = false ;
        }
        delete itemToSend ;
        itemToSend = NULL ;
        // seems like we sent something:
        iTimeOfLastActivity = QDateTime::currentDateTimeUtc().toTime_t() ;
      }
    }
  }
  if ( iNeedsToRun && iSocket ) {
    QLOG_STR(QString("0x%1 ").arg((qulonglong)this, 8) +"Readloop out, addr = " + peerAddress().toString()) ; 
  }
}

// reason for this implementation here is that when connection
// is about to get deleted, there has been SIGSEGV's in calls
// iSocket->peerAddress() ; indication of a locking problem
// or similar, the socket is already partly torn down. 
// have the peer address here as normal variable and avoid the
// dangerous call:
QHostAddress Connection::peerAddress() const {
  return iPeerAddress ; 
}

const Hash& Connection::fingerPrintOfNodeAttempted() {
  return iFpOfNodeWeTrying ;
}

bool Connection::isInPrivateAddrSpace() const {
  bool retval = false ; 
  if ( iSocket ) {
    QHostAddress peer ( iSocket->peerAddress() ) ;
    if ( peer.protocol() == QAbstractSocket::IPv4Protocol ) {
     
      if ( peer.isInSubnet(normalNats1) ||
           peer.isInSubnet(normalNats2) ||
           peer.isInSubnet(normalNats3) )
	retval = true ; 
    }
  }
  return retval ; 
}

void Connection::checkForBucketFill() {
  if ( iTimeOfBucketFill+15 < QDateTime::currentDateTimeUtc().toTime_t() &&
       iNextProtocolItemToSend.size() == 0 && 
       iSendQueue.size() == 0 &&
       iNumberOfPacketsReceived > 1 &&
       iNodeOfConnectedPeer ) {
    // we've been connected for 15 seconds now and there is nothing in queue.
    switch ( iStageOfBucketFill ) {
    case OwnNodeGreeting:
      // initial stage. first queue possible private messages
      iEndOfBucket = iModel.nodeModel().bucketEndHash(iNodeOfConnectedPeer->nodeFingerPrint()) ;
      // here put private messages into queue
      iModel.privateMessageModel().fillBucket(iSendQueue,
					      iNodeOfConnectedPeer->nodeFingerPrint(),
					      iEndOfBucket,
					      iNodeOfConnectedPeer->lastMutualConnectTime(),
					      iNodeOfConnectedPeer->nodeFingerPrint() ) ;
      iStageOfBucketFill = PrivateMessage ; // and set next stage
      break ;
    case PrivateMessage:
      // Next stage after private messages

      iModel.profileModel().fillBucket(iSendQueue,
				       iNodeOfConnectedPeer->nodeFingerPrint(),
				       iEndOfBucket,
				       iNodeOfConnectedPeer->lastMutualConnectTime(),
				       iNodeOfConnectedPeer->nodeFingerPrint() ) ;

      iStageOfBucketFill = UserProfile; // and set next stage
      break ; 
    case UserProfile:
      // Next stage after user profile changes. Note that
      // last mutual connect time is set in protocol parser
      // as a side-effect of parsing the initial node greeting
      iModel.classifiedAdsModel().fillBucket(iSendQueue,
					     iNodeOfConnectedPeer->nodeFingerPrint(),
					     iEndOfBucket,
					     iNodeOfConnectedPeer->lastMutualConnectTime(),
					     iNodeOfConnectedPeer->nodeFingerPrint() ) ;

      iStageOfBucketFill = ClassifiedAd; // and set next stage
      break ; 
    case ClassifiedAd:
      // Next stage after classified ads

      iModel.binaryFileModel().fillBucket(iSendQueue,
					  iNodeOfConnectedPeer->nodeFingerPrint(),
					  iEndOfBucket,
					  iNodeOfConnectedPeer->lastMutualConnectTime(),
					  iNodeOfConnectedPeer->nodeFingerPrint()) ;
      iStageOfBucketFill = BinaryBlob; // and set next stage
      break ; 
    case BinaryBlob:
      // Next stage after binary blob
      iModel.profileCommentModel().fillBucket(iSendQueue,
					  iNodeOfConnectedPeer->nodeFingerPrint(),
					  iEndOfBucket,
					  iNodeOfConnectedPeer->lastMutualConnectTime(),
					  iNodeOfConnectedPeer->nodeFingerPrint()) ;
      iStageOfBucketFill = UserProfileComment; // and set next stage
      break ; 
    case UserProfileComment:
      // we came here, we've sent profiles, ads and binary files profile comments and
      // the queue is empty: set mutual connect time of the node
      // so we won't send the same stuff again next time:
      iNodeOfConnectedPeer->setLastMutualConnectTime(QDateTime::currentDateTimeUtc().toTime_t()) ; 
      iModel.nodeModel().updateNodeLastMutualConnectTimeInDb(iNodeOfConnectedPeer->nodeFingerPrint(),
							     iNodeOfConnectedPeer->lastMutualConnectTime()) ;   
      iStageOfBucketFill = ClassifiedAd2NdAddr; // and set final stage
      break ; 

    case ClassifiedAd2NdAddr: // this is our final stage
      if ( ( iTimeOfBucketFill + (60*30 ))  < QDateTime::currentDateTimeUtc().toTime_t() ) {
	// 30 minutes passed since last bucket fill: re-do
	iTimeOfBucketFill = QDateTime::currentDateTimeUtc().toTime_t() ;
	iStageOfBucketFill = OwnNodeGreeting;
      }
      break ;
    default:
      // finally do no thing
      break ; 
    }
  }
}

Hash Connection::getPeerHash() const {
  if ( iConnectionState == Connection::Open && iSocket ) {
    return Hash(   iSocket->peerCertificate() ) ; 
  } else {
    // this may be KNullHash but for some outgoing connections
    // this is anyway known already when socket is not 
    // yet in connected state.
    return iFpOfNodeWeTrying ; 
  }
}
