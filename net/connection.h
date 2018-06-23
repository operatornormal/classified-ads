/*     -*-C++-*- -*-coding: utf-8-unix;-*-
  Classified Ads is Copyright (c) Antti Järvinen 2013-2018.

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


#ifndef CONNECTION_H
#define CONNECTION_H
#include <QTcpSocket> // for SocketError
#include <QHostAddress> // for ipv6 addr type
#include "protocol.h"
#include "node.h"

class QSslSocket ;
class QByteArray ;
class MController ;
class Model ;
class QSslError ; 

/**
 * @brief Class that represents a network connection.
 *
 * At runtime
 * we'll have 1-n of these. This is a base-class that allows us
 * to do some operations like "send", "receive" without needing
 * to know if we're having a socket conn or some other kind.
 */
class Connection : public QObject {
    Q_OBJECT
public:
    /** Enumeration of the connection states */
    enum ConnectionState {
        Initial, /**< SYN packet sent */
        Open,    /**< SSL handshake was success, peerCert is avail */
        Closing, /**< Disconnection has been asked */
        Error    /**< Connection itself reports an error */
    };
    /**
     * @brief Interface for receiving network traffic.
     *
     * class that must be implemented by animals that wish to receive
     * data from connection
     */
    class ConnectionObserver {
    public:
        /** method for sending data received */
        virtual bool dataReceived(const QByteArray& aData,
                                  Connection& aConnection ) = 0 ;
        /** method for communicating fact that connection ends */
        virtual void connectionClosed(Connection *aDeletee) = 0 ;
        /** method for communicating fact that connection is open for
        *  business
        */
        virtual void connectionReady(Connection *aBusinessEntity) = 0 ;
    } ;
    /**
     * Constructor to use when incoming connection
     */
    Connection(int aSocketDescriptor,
               ConnectionObserver& aObserver,
               Model& aModel,
               MController& aController);
    /**
     * Constructor. Class will initiate connetion to
     * given addr.
     * @param aAddr network address to try
     * @param aPort port in said addr
     * @param aObserver class wishing to receive status updates about this
     *                  connection
     * @param aModel datamodel reference
     * @param aController application controller reference
     * @param aFpOfNodeToTry sometimes we need to connect some particular
     *        node ; hash of the attempted host may be given here. It is
     *        not used by connection itself but the observers may query it
     *        in case of failed connection of the connection was
     *        attempted to some particular node. In particular this logic
     *        is used by publishing engine that needs to know not only
     *        successful connections but also failed ones and for failed
     *        ones we don't get node FP via normal mechanism (node greeting)
     */
    Connection(const QHostAddress& aAddr,
               const int aPort,
               ConnectionObserver &aObserver,
               Model& aModel,
               MController& aController,
               const Hash& aFpOfNodeToTry ) ;
    /**
     * Method for tearing down a connection
     */
    ~Connection() ;
    /**
     * helper method for comparing ipv6 addresses
     */
    static bool Ipv6AddressesEqual(const Q_IPV6ADDR& aAddr1,
                                   const Q_IPV6ADDR& aAddr2) ;
    /**
     * getter method for node of this connection. value
     * may be null if connection is still in early stage
     * so non-nulliness must be tested
     * @param none
     * @return node of connection or NULL if handshake still pending.
     *         ownership is NOT transferred, it is not ok to delete
     *         the returned pointer.
     */
    Node* node() const {
        return iNodeOfConnectedPeer ;
    }

    /**
     * for testing purposes only: this should never be called
     * in production setup. this does get called from test-code
     */
    void setNode(Node* aNode) {
        iNodeOfConnectedPeer = aNode ;
    }

    /**
     * method for getting peer addr
     */
    QHostAddress peerAddress() const ;

    /**
     * method for getting the hash this connection was
     * trying to attempt .. if connection was originally
     * asked to be to some particular node, this will
     * return hash of the node ; if connection was to
     * no particular node, a zero hash will be returned
     */
    const Hash& fingerPrintOfNodeAttempted() ;
    /**
     * getter method for connection state
     */
    ConnectionState connectionState() const  {
        return iConnectionState;
    }
    /**
     * Method for getting the bucket fill stage. See explanation
     * at documentation of the variable @ref Connection::iStageOfBucketFill
     * for idea how this should be used
     */
    ProtocolItemType stageOfBucketFill() const {
        return iStageOfBucketFill ;
    }
    /**
     * Method for setting bucket fill stage. See also @ref stageOfBucketFill.
     */
    void setStageOfBucketFill(ProtocolItemType aStage)  {
        iStageOfBucketFill = aStage ;
    }

    /**
     * method for getting information if connection is inbound
     */
    bool isInbound() const {
        return iSocketIsIncoming ;
    }  ;
    /**
     * method for getting open time of connection
     */
    time_t getOpenTime() const {
        return iSocketOpenTime ;
    }  ;
    /**
     * method for getting peering node fingerprint.
     * if connection is not yet open, returns KNullHash
     */
    Hash getPeerHash() const ;
    /**
     * returs data transfer amount
     */
    unsigned long bytesIn() const {
        return iBytesIn ;
    }
    /**
     * returns data transfer amount
     */
    unsigned long bytesOut() const {
        return iBytesOut ;
    }
    /**
     * Forcibly closes socket.
     * @return true if socket was still allocated and its "abort" method
     *         got called. 
     */
    bool forciblyCloseSocket() ;
    /**
     * method for checking if connection is in private (non-routable) ipv4-addr
     * space like 192.168.. network
     */
    bool isInPrivateAddrSpace() const ;
public slots:
    /** 
     * Called when socket is encrypted and ready to transmit
     */
    void socketReady() ;
    /**
     * this class is a kind of a thread, we need to have run ; note
     * that right now QThread is not inherited but
     * our worker-method is still named run, called
     * by thread when it is started ( from networklistener or
     * networkconnectorengine)
     */
    void run() ;
    /**
     * Connected from QSslSocket. Called when bytes have been written
     */
    void encryptedBytesWritten ( qint64 written ) ; 
    /**
     * Connected from QSslSocket. Called on handshake errors
     */
    void sslErrors ( const QList<QSslError> & errors ) ; 
    /**
     * Connected from QSslSocket::error. Called on error
     */
    void socketError ( QAbstractSocket::SocketError socketError ) ; 
    /**
     * Connected from QSslSocket. Called on close
     */
    void disconnected () ; 
    /**
     * Connected from QSslSocket. Called when bytes are available
     */
    void readyRead () ; 
    /**
     * Slot called at thread finish. Contains cleanup operations. 
     */
    void aboutToFinish() ; 
signals:
    void error(QTcpSocket::SocketError socketError);
    void finished() ;
    void blackListNetworkAddr(QHostAddress aAddr) ;
    /** Emitted when outgoing connectio fails. Used for keeping count on
     * nodes not to try immediately again */
    void connectionAttemptFailed(const Hash& aNodeHash) ;
protected:
    void performRead() ;
    void readLoop() ; /**< called from run() */
    /**
     * 2 versions of run, called from run(),first for incoming connections
     */
    void runForIncomingConnections() ;
    /**
     * 2 versions of run, called from run(),then for outgoing connections
     */
    void runForOutgoingConnections() ;
    /**
     * method for keeping buckets filled. this checks if we have
     * content that according to its network addr should belong
     * to that peer ; and then adds it to send queue
     */
    void checkForBucketFill() ;
    /**
     * method that tries to flush pending output, if platform supports
     */
    void flushSocket() ; 
    /**
     * method that sets up signals/slots of socket
     */
    void setupSocketSignals() ; 
public:// these are public because datamodel access these, via lock()
    /**
     * many things going on inside classified ads are about
     * the connected node ; when node is disconnected, we may
     * forget about many things that may have been in progress.
     * therefore it may make sense to store also the queue
     * of things to send in here ; when connection goes away,
     * so goes the queue and no further processing needed.
     *
     * The order of things in this list is significant,
     * the first item to be append()ed should be sent first.
     */
    QList <SendQueueItem> iSendQueue ;
    /**
     * This is the next item to send. This does not include the length.
     * .. as protocol items over socket are sent so that length goes
     * first, as uint32, followed by that many bytes. This QByteArray
     * contains the bytes, not the uint32 that goes first. Reason is
     * that this is wanted this way is that the class doing the
     * sending to be aware of the length, forcing this way it to
     * calculate it itself.
     */
    QList<QByteArray *> iNextProtocolItemToSend ;
public:
    bool iNeedsToRun ; /**< if set to false, connection closes itself */
    time_t iTimeOfLastActivity ; /**< last time there was any traffic with peer */
    unsigned iNumberOfPacketsReceived ; /**< nr of received complete protocol packets*/
  /** 
   * how often peers are queried for new content to send/receive ; this is also
   * max permitted lenght for data transmission inactivity ; nodes that have been
   * idle longer than this are considered "dead"
   */
  static const unsigned iMinutesBetweenBucketFill = 30  ; 
protected: // these are not public
    void msleep(int aMilliSeconds) ; /**< stops execution for some time */
    ConnectionState iConnectionState /**< closed/open etc. */ ;
    /** received data is sent to @ref ConnectionObserver */
    ConnectionObserver &iObserver ;
    /** socket for data transfer: the actual socket is owned by
     *  stack frame of the thread so even tough we have
     *  this pointer here, it may not be deleted
     */
    QSslSocket *iSocket ;
    Model& iModel ; /**< datamodel reference */
    /** when starting to receive packet from peer, length is stored here */
    quint32 iBytesExpectedInPacketBeingRead ;
    /** when reading packet from peer, this contains bytes being read */
    QByteArray *iBytesRead ;
    /**
     * this variable here is used as counter during pending read
     * of data packet, zeroed every time bytes are received ;
     * if it reaches high number (a sign of stalled download) do
     * drop connection to that peer
     */
    int iInvocationsSinceLastByteReceived;
    /**
     * node-data-structure of the connected node
     */
    Node *iNodeOfConnectedPeer ;
    /** network address to connect, if connection is outgoing connection */
    QHostAddress iAddrToConnect ;
    /** port to connect, if connection is outgoing connection */
    const int iPortToConnect ;
    /** descriptor of already-connected socket if connection is incoming-connection */
    const int iSocketDescriptor ;
    /** true if incoming socket */
    const bool iSocketIsIncoming ;
    /** application controller reference */
    MController& iController ;
    /** It may be that connection is asked to specific node, in addition
     * to being into some known network address. If the node is known,
     * store the target node fingerprintin here. This is used at least
     * in wishlist-connections where the connection itself will
     * signal its open-for-business state when connection to asked
     * node is ready
     */
    Hash iFpOfNodeWeTrying ;
    /**
     * method of handling around network churn is implemented in
     * several places. .. churn requires us to copy stuff
     * beloging to NodesAroundHash ( see nodemodel ) to nodes
     * that are in the same bucket with us. Our concept of bucket
     * is a bit stretched but there is kind of bucket and we will
     * send of over new data to nodes that we think belongs
     * to the bucket of connecting node.
     *
     * sending the data happens in several stages, connection itself
     * stores the stage here. filling the bucket according to
     * stage then happens in @ref NodeModel::getNextItemToSend method
     */
    ProtocolItemType iStageOfBucketFill ;
    /**
     * time of last bucket fill
     */
    quint32 iTimeOfBucketFill ;
    /**
     * Where our bucket ends. Our own node fp is the start of the bucket,
     * here is the end
     */
    Hash iEndOfBucket ;
    /** time of socket open */
    const time_t iSocketOpenTime ;
    /** transferred data amount */
    unsigned long iBytesIn ;
    /** transferred data amount */
    unsigned long iBytesOut ;
    /** storage for peeraddress */
    QHostAddress iPeerAddress ;
    /** 
     * indication if write operation has been started but is
     * not yet complete
     */
    int iBytesPendingWrite ; 
    /**
     * How many milliseconds to sleep between send operations
     */
    int iSleepBetweenSendOperations ; 
    /**
     * hash of peering node
     */
    Hash iPeerHash;
} ;
#endif
