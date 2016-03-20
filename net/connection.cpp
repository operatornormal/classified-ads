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
#include "connection.h"
#include "../log.h"
#include <QSslSocket>
#include "../datamodel/model.h"
#ifndef WIN32
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
#include <QWaitCondition>
#include <QMutex>
#include <QSslCipher>

/** nat subnet. see similar variable in node.cpp */
static const QPair<QHostAddress, int> normalNats1 ( QHostAddress::parseSubnet("10.0.0.0/8") );
static const QPair<QHostAddress, int> normalNats2 ( QHostAddress::parseSubnet("172.16.0.0/20"));
static const QPair<QHostAddress, int> normalNats3 ( QHostAddress::parseSubnet("192.168.0.0/16") );
/** non-routable address space that some ISP's seem to offer */
static const QPair<QHostAddress, int> rfc6598 ( QHostAddress::parseSubnet("100.64.0.0/10") );


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
      iPeerAddress(aAddr),
      iSleepBetweenSendOperations(500)  {
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
      iBytesOut(0),
      iSleepBetweenSendOperations(500)  {
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
    if ( iConnectionState == Initial ||
            iConnectionState == Open ) {
        iConnectionState = Closing ;
    }
    iObserver.connectionClosed(this) ; // will call datamodel.lock() inside
    LOG_STR(QString("0x%1 ").arg((qulonglong)this, 8) +"Connection::run out") ;
    emit finished() ;
    delete iNodeOfConnectedPeer ;
    iNodeOfConnectedPeer = NULL ;
    delete iBytesRead ;
    iBytesRead = NULL ;
    iSocket = NULL ; // iSocket was automatic variable inside scope
    // of runForIncomingConnections/runForOutgoingConnections
    // and it is no more -> set to NULL to prevent accidental access
}

void Connection::runForIncomingConnections() {
    QSslSocket sock ;
    iSocket = &sock ; // when this method goes out of
    // scope, iSocket must be set to NULL

    if (iSocket->setSocketDescriptor(iSocketDescriptor)) {
        setupSocketSignals() ;
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
            iSocket->close() ;
            return ;
        }
        iSocket->setLocalCertificate(iModel.nodeModel().nodeCert());
        iSocket->setPrivateKey(iModel.nodeModel().nodeKey());
        iModel.unlock() ;
        iSocket->startServerEncryption();
    } else {
        LOG_STR(QString("0x%1 ").arg((qulonglong)this, 8) +"iSocket->setSocketDescriptor failed?????") ;
        iSocket->close() ;
        return ;
    }
    iSocket->setSocketOption(QAbstractSocket::LowDelayOption,1) ; // immediate send
    for ( int i = 0 ; iConnectionState == Initial ; i++ ) {
        msleep(1000) ;
        QCoreApplication::processEvents(QEventLoop::ExcludeUserInputEvents) ;
        if ( i > 10 ) { // 10 seconds passed and connection state not changed
            iSocket->abort() ;
            iNeedsToRun = false ;
            return ;
        }
    }

    // ok, obviously we can receive, mark that down for future
    // advertisement:
    iModel.lock() ;
    iController.getNode().setIpAddrWithChecks(iSocket->localAddress()) ;
    iModel.unlock() ;
    // ok, we came here, we have SSL handshake done ; lets
    // instantiate some data structures..
    Hash fingerPrintOfPeer (iSocket->peerCertificate()) ;
    iPeerHash = fingerPrintOfPeer ;
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
    if ( iSocket ) {
        iSocket->waitForDisconnected(10*1000) ;
    }
    LOG_STR(QString("0x%1 ").arg((qulonglong)this, 8) +"Connection::run(incoming) out") ;
}

void Connection::socketReady() {
    LOG_STR(QString("0x%1 ").arg((qulonglong)this, 8) +
            "Connection::socketReady auth " +
            iSocket->sessionCipher().authenticationMethod() +
            " keyEx " +
            iSocket->sessionCipher().keyExchangeMethod() +
            " encryption " +
            iSocket->sessionCipher().encryptionMethod() +
            " bits " +
            QString::number(iSocket->sessionCipher().usedBits())) ;
    iModel.lock() ;
    iConnectionState = Open ;
    iModel.connectionStateChanged(iConnectionState, this) ;
    iPeerAddress = iSocket->peerAddress() ;
    iPeerHash = Hash(   iSocket->peerCertificate() );
    if ( iPeerAddress == iSocket->localAddress() ) {
        QLOG_STR(QString("0x%1 ").arg((qulonglong)this, 8) + "Connected to self, addr = " + iSocket->peerAddress().toString()) ;
        // oh great, broadcast?
        // connected to self
        iNeedsToRun = false ;
        iSocket->close() ;
        return ;
    }
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

    setupSocketSignals() ;
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
    iSocket->setSocketOption(QAbstractSocket::LowDelayOption,1) ; // immediate send
    iSocket->setLocalCertificate(iModel.nodeModel().nodeCert());
    iSocket->setPrivateKey(iModel.nodeModel().nodeKey());
    iModel.unlock() ;
    iSocket->setPeerVerifyMode(QSslSocket::QueryPeer) ;
    QLOG_STR(QString("0x%1 ").arg((qulonglong)this, 8) +"Connecting to " + iAddrToConnect.toString() + " port " + QString::number(iPortToConnect) ) ;
    iSocket->connectToHostEncrypted(iAddrToConnect.toString(), iPortToConnect);

    for ( int i = 0 ; iConnectionState == Initial ; i++ ) {
        QCoreApplication::processEvents(QEventLoop::ExcludeUserInputEvents) ;
        msleep(1000) ;
        if ( i > 10 ) { // 10 seconds passed and connection state not changed
            iSocket->abort() ;
            iNeedsToRun = false ;
            QLOG_STR("Connection attempt to " + iAddrToConnect.toString() + " timed out") ;
            QLOG_STR("Connection attempt to " + iFpOfNodeWeTrying.toString() + " timed out") ;
            emit connectionAttemptFailed(iFpOfNodeWeTrying) ; // then don't try again immediately
            return ;
        }
    }

    if ( iSocket->peerAddress() == iSocket->localAddress() ) {
        QLOG_STR(QString("0x%1 ").arg((qulonglong)this, 8) +"Connected to self, addr = " + iSocket->peerAddress().toString()) ;
        // oh great, broadcast?
        // connected to self
        iNeedsToRun = false ;
        iSocket->close() ;
        return ;
    }

    Hash fingerPrintOfPeer (iSocket->peerCertificate()) ;
    iPeerHash = fingerPrintOfPeer ;
    // it also may be so that connection was attempted to a particular
    // node - is it possible that we managed to connect other node
    // than that?
    if (  iFpOfNodeWeTrying != KNullHash  ) { // if other than null hash
        if (  iFpOfNodeWeTrying != fingerPrintOfPeer ) {
            iSocket->abort() ;
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
    if ( iSocket ) {
        iSocket->waitForDisconnected(30*1000) ;
    }
    iSocket = NULL ;
    LOG_STR(QString("0x%1 ").arg((qulonglong)this, 8) +"Connection::run  (outgoing )out") ;
}


// this is called from run() method when there is data available
void Connection::performRead() {
    quint64 bytesAvail (0) ;

    while ( ( bytesAvail = iSocket->bytesAvailable() ) > 0 ) {
        if ( iBytesExpectedInPacketBeingRead == 0 &&
                bytesAvail < sizeof(quint32) ) {
            msleep(50) ; // size not yet received, wait for 50 ms
        }
        if ( iBytesExpectedInPacketBeingRead == 0 &&
                bytesAvail >= (quint64) sizeof(quint32) ) {
            // we're not reading a packet yet, read packet
            quint32 bytesExpectedNetworkByteOrder = 0 ;
            if ( iSocket->read((char *)&bytesExpectedNetworkByteOrder,
                               sizeof(quint32)) != sizeof(quint32)) {
                iNeedsToRun = false ;
                iModel.lock() ;
                iModel.connectionStateChanged(Connection::Closing, this) ;
                iModel.unlock() ;
                iSocket->abort() ;
                return ;
            } else {
                iBytesIn += sizeof(quint32) ;
                bytesAvail = iSocket->bytesAvailable() ;
            }
            iBytesExpectedInPacketBeingRead = ntohl ( bytesExpectedNetworkByteOrder);
            QLOG_STR(QString("0x%1 ").arg((qulonglong)this, 8) +"Connection::performRead expecting bytes= "+ QString::number( iBytesExpectedInPacketBeingRead)) ;
        }

        if ( iBytesExpectedInPacketBeingRead > 0 &&
                iBytesExpectedInPacketBeingRead > (quint64)(iBytesRead->size()) &&
                bytesAvail > 0 ) {
            if ( bytesAvail > iBytesExpectedInPacketBeingRead ) {
                bytesAvail = ( iBytesExpectedInPacketBeingRead -
                               iBytesRead->size() );
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
                iSocket->abort() ;
                iNeedsToRun = false ;
            } else {
                QCoreApplication::processEvents(QEventLoop::ExcludeUserInputEvents) ;
            }
            iBytesExpectedInPacketBeingRead = 0 ;
            iBytesRead->clear() ;
            iInvocationsSinceLastByteReceived = 0 ;
            iNumberOfPacketsReceived++ ;
        }
    }
}

bool Connection::Ipv6AddressesEqual(const Q_IPV6ADDR& aAddr1,
                                    const Q_IPV6ADDR& aAddr2) {
    for ( int i = 0 ; i < 16 ; i++ ) {
        if ( aAddr1.c[i] != aAddr2.c[i]) return false ;
    }
    return true ;
}

// practically this is more like write-loop because actual
// reading and writing is done in SLOT()s that are connected
// to read+write events.
void Connection::readLoop() {
    connect(iSocket, SIGNAL(readyRead ()),
            this, SLOT(readyRead ()));
    iObserver.connectionReady(this) ; // before starting to read, inform the observer
    QByteArray *itemToSend = NULL ;
    while ( iNeedsToRun && iSocket->state() == QAbstractSocket::ConnectedState ) {
        iModel.lock() ;
        checkForBucketFill() ;
        int numberOfQueueItems = iSendQueue.size() ;
        iModel.unlock() ;

        if ( numberOfQueueItems > 0 ) {
            msleep( 10 ) ;
        } else {
            msleep( iSleepBetweenSendOperations ) ;
        }
        QCoreApplication::processEvents(QEventLoop::ExcludeUserInputEvents) ;

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
                iSocket->abort() ;
                iNeedsToRun = false ;
                LOG_STR(QString("0x%1 ").arg((qulonglong)this, 8) +"Giving up pending read operation after 500 seconds");
            }
            break ;
        }
        // ok, then try write operation, if applicable
        if ( iNeedsToRun && iSocket->bytesToWrite() == 0 ) {
            iModel.lock() ;
            itemToSend = iModel.nodeModel().getNextItemToSend(*this) ;
            if ( itemToSend != NULL ) {
                QLOG_STR("Got " + QString::number(itemToSend->size()) +
                         " bytes to send, items in q = " +
                         QString::number(iSendQueue.size())) ;
                iSleepBetweenSendOperations = 30 ; // we got item to send, next time sleep only 30ms
            } else {
                iSleepBetweenSendOperations = 500 ; // queue empty, sleep 500 ms
            }
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
                iSocket->abort() ;
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
                iSocket->abort() ;
                iNeedsToRun = false ;
            } else {
                iBytesOut += itemToSend->size() ;
            }
            // ok ; how long to wait for write.
            // very slow network here. have 100 connected clients.
            // say, upload rate is 20kb/s, divided by 100 clients
            // we get 204 bytes per sec. .. but at least 30 seconds.
            delete itemToSend ;
            itemToSend = NULL ;
            // seems like we sent something:
            iTimeOfLastActivity = QDateTime::currentDateTimeUtc().toTime_t() ;
            flushSocket() ;
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
                    peer.isInSubnet(normalNats3) ||
                    peer.isInSubnet(rfc6598))
                retval = true ;
        }
    }
    return retval ;
}

void Connection::checkForBucketFill() {
    if ( iTimeOfBucketFill+15 < QDateTime::currentDateTimeUtc().toTime_t() &&
            iNextProtocolItemToSend.size() == 0 &&
            iSendQueue.size() == 0 &&
            iSocket &&
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
            if ( ( iTimeOfBucketFill + (60*iMinutesBetweenBucketFill ))  < QDateTime::currentDateTimeUtc().toTime_t() ) {
                // iMinutesBetweenBucketFill minutes passed since last bucket fill: re-do
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
    if ( iPeerHash != KNullHash ) {
        return iPeerHash ;
    } else {
        if ( iConnectionState == Connection::Open && iSocket ) {
            return Hash(   iSocket->peerCertificate() ) ;
        } else {
            // this may be KNullHash but for some outgoing connections
            // this is anyway known already when socket is not
            // yet in connected state.
            return iFpOfNodeWeTrying ;
        }
    }
}

bool Connection::forciblyCloseSocket() {
    iNeedsToRun = false ;
    iFpOfNodeWeTrying = KNullHash ;
    return true ;
}


void Connection::flushSocket() {
    // setting TCP_NODELAY has side-effect of flushing the socket.
    // at least in linux.
    if ( iSocket ) {
        iSocket->flush() ;
        QCoreApplication::processEvents(QEventLoop::ExcludeUserInputEvents) ;
        iSocket->setSocketOption(QAbstractSocket::LowDelayOption,0) ;
        iSocket->setSocketOption(QAbstractSocket::LowDelayOption,1) ;
        QCoreApplication::processEvents(QEventLoop::ExcludeUserInputEvents) ;
    }
}


// Connected from QSslSocket. Called when bytes have been written
void Connection::encryptedBytesWritten ( qint64
#ifdef DEBUG
        written
#endif
                                       ) {
    QLOG_STR("Connection::encryptedBytesWritten " + QString::number(written));
}

// Connected from QSslSocket. Called on handshake errors
void Connection::sslErrors ( const QList<QSslError> & errors  ) {
    QLOG_STR("Connection::sslErrors") ;
    foreach ( const QSslError& e , errors ) {
        switch ( e.error() ) {
        case QSslError::NoError:
        case QSslError::CertificateExpired:
        case QSslError::InvalidNotBeforeField:
        case QSslError::InvalidNotAfterField:
        case QSslError::SelfSignedCertificate:
        case QSslError::SelfSignedCertificateInChain:
        case QSslError::UnableToVerifyFirstCertificate:
        case QSslError::CertificateRevoked:
        case QSslError::InvalidCaCertificate:
        case QSslError::CertificateUntrusted:
        case QSslError::SubjectIssuerMismatch:
        case QSslError::AuthorityIssuerSerialNumberMismatch:
        case QSslError::HostNameMismatch:
        case QSslError::InvalidPurpose:
            QLOG_STR("Ssl error ignored: " + e.errorString()) ;
            break ;
        case QSslError::UnableToGetIssuerCertificate:
        case QSslError::UnableToDecryptCertificateSignature:
        case QSslError::UnableToDecodeIssuerPublicKey:
        case QSslError::CertificateSignatureFailed:
        case QSslError::CertificateNotYetValid:
        case QSslError::UnableToGetLocalIssuerCertificate:
        case QSslError::PathLengthExceeded:
        case QSslError::CertificateRejected:
        case QSslError::NoPeerCertificate:
        case QSslError::UnspecifiedError:
        case QSslError::NoSslSupport:
        case QSslError::CertificateBlacklisted:
            QLOG_STR("Ssl error detected: " + e.errorString()) ;
            iNeedsToRun = false ;
            break ;
        }
    }
}
// Connected from QSslSocket::error. Called on error
void Connection::socketError ( QAbstractSocket::SocketError
#ifdef DEBUG
                               socketError
#endif
                             ) {
    iNeedsToRun = false ;
    QLOG_STR(QString("0x%1 ").arg((qulonglong)this, 8) +
             "Connection::socketError "+ QString::number(socketError)) ;

    if ( iSocketIsIncoming == false && // this is outgoing
            iFpOfNodeWeTrying != KNullHash && // and we know who we tried to reach
            iConnectionState == Connection::Initial // and didn't get past initial stage
       ) {
        emit connectionAttemptFailed(iFpOfNodeWeTrying) ; // then don't try again immediately

    } else {
        QLOG_STR("No emit") ;
    }
}

// Connected from QSslSocket. Called on close
void Connection::disconnected () {
    iNeedsToRun = false ;
    QLOG_STR(QString("0x%1 ").arg((qulonglong)this, 8) +"disconnected") ;
}
// Connected from QSslSocket. Called when bytes are available
void Connection::readyRead () {
    performRead() ;
}

void Connection::setupSocketSignals() {
    connect(iSocket, SIGNAL(encrypted()), this, SLOT(socketReady()));
    connect(iSocket, SIGNAL(encryptedBytesWritten ( qint64 )),
            this, SLOT(encryptedBytesWritten ( qint64 )));
    connect(iSocket, SIGNAL(sslErrors ( const QList<QSslError> & )),
            this, SLOT(sslErrors ( const QList<QSslError> & )));
    connect(iSocket, SIGNAL(error ( QAbstractSocket::SocketError )),
            this, SLOT(socketError ( QAbstractSocket::SocketError )));
    connect(iSocket, SIGNAL(disconnected ()),
            this, SLOT(disconnected ()));
    // note that readyread signal is connected when
    // readloop is entered
}

void Connection::msleep(int aMilliSeconds) {
    QWaitCondition waitCondition;
    QMutex mutex;
    QThread::yieldCurrentThread ();
    mutex.lock() ; // waitCondition needs a mutex initially locked
    waitCondition.wait(&mutex, aMilliSeconds);// give other threads a chance..
    mutex.unlock() ;
}
