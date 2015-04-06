/*    -*-C++-*- -*-coding: utf-8-unix;-*-
    Classified Ads is Copyright (c) Antti Jarvinen 2013.

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

#ifndef CLASSIFIED_NODEMODEL_H
#define CLASSIFIED_NODEMODEL_H
#include <QSqlDatabase>
#include <QObject>
#include "../mcontroller.h" // because enum from there is needed
#include "mnodemodelprotocolinterface.h"
#include "../net/connection.h"
#include <QHostAddress>
#include <QPair>
#include "datamodelbase.h"

class QSqlDatabase ;
class QMutex ;
class Hash ;
class QSslCertificate ;
class QSslKey ;
class Model ;

/**
 * @brief This is node-specific part of the datamodel.
 *
 * This is node-specific part of the datamodel ; this handled
 * storage and handling of data that is directly related to
 * nodes themselves. Node is a peer in network.
 */
class NodeModel : public ModelBase,
    public MNodeModelProtocolInterface {
    Q_OBJECT

public:
    NodeModel(MController *aController, const Model &aModel) ;
    ~NodeModel() ;
    virtual Hash& nodeFingerPrint(); /**< returns fingerprint of this node */
    virtual int listenPortOfThisNode() ; /**< TCP listen port number method */
    /** Setting of TCP listen port number method */
    virtual void setListenPortOfThisNode(int port) ;
    /** getter for ssl certificate of SSL sock */
    virtual const QSslCertificate& nodeCert() const ;
    /** getter for ssl certificate of SSL sock */
    virtual const QSslKey& nodeKey() const ;
    /**
     * Method that network connections use to request for more
     * stuff to send to peers
     * @param aConnection is the connection in question
     * @return octets in QByteArray. Ownership of bytes is
     *         transferred, datamodel does not keep a copy
     *         and caller is responsible for deleting the
     *         returned bytes after they've been sent.
     */
    virtual QByteArray* getNextItemToSend(Connection& aConnection) ;

    /**
     * Method for getting node details by hash
     * @param aHash tells which node to retrieve
     * @return node or NULL if not known
     */
    virtual Node* nodeByHash(const Hash& aHash) ;
    /**
     * method for getting node-connection prospects. lock() must
     * be called by caller. This does not return addresses of nodes
     * that are already connected-to. And this does not return
     * address of this same node. Idea is to use this method
     * internally to fish out some addresses that might be
     * worth connecting.
     */
    virtual QList<HostConnectQueueItem> getHotAddresses() ;
    /**
     * method for getting node-connection prospects with idea
     * that we make node-greetings out of those and send to
     * neighboring node. This may and will also return address
     * of this node and this may and will also return addresses
     * of nodes currently connected ; it makes sense to give
     * others list of addresses that are known to be online.
     * @param aMaxNodes max number of nodes to return
     * @return a qlist that the caller must delete after it
     *         is used.
     */
    QList<Node *>* getHotNodes(int aMaxNodes) ;
    /**
     * From MModelProtocolInterface.
     *
     * Method related to node database handling ; generic node
     * reference update method, that is used also to update
     * reference of this node (self). NodeModel must be locked
     * before this is used.
     *
     * Practically this is called on 2 occasions: one is a new
     * connection to peer, there peer sends node greeing as
     * the first thing. This may trigger other things inside
     * datamodel.
     *
     * Second possibility is that we've asked for node
     * references (around some hash) and then we'll receive.
     *
     * @param aNode is the node that had its ref sent
     * @param aWasInitialGreeting is true if this call was due
     *                             to initial node greeting
     *                             sent just as first time
     *                             after connect
     * @return true if data update was successful
     */
    virtual bool nodeGreetingReceived(Node& aNode ,
                                      bool aWasInitialGreeting = false ) ;
    /**
     * Method for getting node-connection prospects around
     * a given hash. This is used to obtain nodes that might
     * contain content with hash-value near hash-values of
     * nodes.
     * @param aHash is hash where to start returning
     * @param aMaxNodes max number of nodes to return
     * @param aMaxInactivityMinutes leave out nodes that have last activity
     *                              time older than this many minutes, if
     *                              negative value specified, just include
     *                              all.
     * @return a qlist that the caller must delete after it
     *         is used.
     */
    virtual QList<Node *>* getNodesAfterHash(const Hash& aHash,
            unsigned aMaxNodes,
            int aMaxInactivityMinutes = -1 ) ;
    /**
     * this is like updateNodeInDb but updates only the time
     * of last connect
     */
    bool updateNodeLastConnectTimeInDb(Node& aNode) ;
    /**
     * method for updating last mutual connect time of
     * a node
     */
    virtual bool updateNodeLastMutualConnectTimeInDb(const Hash& aNodeFp,
            quint32 aTime ) ;
    /**
     * We'll try to maintain the connections in hash-space in
     * ring-like structure where each node is connected to the
     * adjacent node, "adjacency" means that nodes whose hash-values
     * are closest to ours, are our adjacent nodes. For this purpose
     * have methods for retrieving list of nodes just before
     * us in the ring and later list of nodes just after us in the
     * ring.
     */
    virtual QList<Node *>* getNodesBeforeHash(const Hash& aHash,
            unsigned aMaxNodes) ;
    /**
     * method that picks up a connection that may be closed with
     * smallest amount of hassle..
     *
     * method also works a bit conditionally, it may be that
     * if there is for instance a lot of connections from
     * same subnet with us, it will not close any connection..
     */
    virtual void       closeOldestInactiveConnection() ;
    /**
     * Method that returns path of directory where datafiles
     * are kept
     */
    QString dataDir() ;
    /**
     * method for adding node reference from broadcast.
     * this needs difference in handling because in IPv4
     * network this typically contains private addr space
     * addresses that we don't want to permanently store nor
     * give to others as node-references.
     *
     * For making connections inside LANs of organisations
     * having NATs and firewalls and whatnot this might
     * still be a handy feature.
     */
    virtual void addNodeFromBroadcast(const Hash& aNodeFingerPrint,
                                      const QHostAddress& aAddr,
                                      int aPort ) ;
    /**
     * method for adding a node to connection-wishlist.
     * network connector engine will then later pick them up.
     * nodemodel will take ownership of the node and delete
     * the object later.
     * @return true if connection was queued
     */
    virtual bool addNodeToConnectionWishList(Node* aNode) ;
    /**
     * method for adding a node to connection-wishlist.
     * network connector engine will then later pick them up.
     * @return true if connection was queued
     */
    virtual bool addNodeToConnectionWishList(const Hash& aNode) ;

    /**
     * method for getting one node from wishlist.
     * caller is obliged to delete the node returned.
     * @return node or null if there is nothing in wishlist
     */
    virtual Node* nextConnectionWishListItem() ;
    /**
     * method for checking if a node is already connected. This
     * will try matching against hash and network addresses.
     */
    virtual bool isNodeAlreadyConnected(const Node& aNode) const ;
    /**
     * Method for checking if a node is already connected.
     * This version checks only hash, not addresses.
     */
    virtual bool isNodeAlreadyConnected(const Hash& aHash) const ;
    /**
     * Important method regarding churn here. This is called from
     * @ref Connection class and this method is used to determine
     * what content belongs to same bucket with the node that
     * is being served by that Connection. Content in the same bucket
     * with the node is then sent over to node ; to keep the content
     * alive in the network.
     *
     * Intent here is find bucket size where we have approximately
     * 20 live nodes in a bucket. If size of nodes alive in network
     * is less than 20, returned hash is same as the one given
     * in argument, methods using the value must then implement
     * something like "send all data".
     *
     * This is done so that we order the recently-seen nodes by hash
     * value of the node, then start from the fingerprint of the node
     * that is asking, and from that point we count to 20. The fingerprint
     * of the node at position 20 is the end of the bucket. The
     * fingerprint of the node that asks is the start.
     *
     * @param aFingerPrintOfNodeAsking fingerprint of node that wants to
     *                                 have its bucket filled
     * @return Ending-address of the nodes bucket.
     */
    virtual Hash bucketEndHash(const Hash& aFingerPrintOfNodeAsking) ;
    /** called from settings dialog */
    virtual void setDnsName(QString aName) ;
    /** called from settings dialog and own node construction */
    virtual QString getDnsName() ;
    /** used to offer node to list of recently failed connections.
     * this model maintains a list of such nodes and tries to
     * not immediately re-connect a recently failed node */
    virtual void offerNodeToRecentlyFailedList(const Hash& aFailedNodeHash) ;
protected:
    bool insertNodeToDb(Node& aNode) ;
    /**
     * method for updating existing node in db table
     * @param aNode is the node to update
     * @return true if success
     */
    bool updateNodeInDb(Node& aNode) ;
    void retrieveListOfHotConnections() ;
    /**
     * for periodical stuff inside datamodel
     */
    void timerEvent(QTimerEvent *event);
private:
    bool openOrCreateSSLCertificate() ; /**< opens the node cert that is for network traffic */
    bool saveSslCertToDb(const QByteArray& aCert) ;
    bool saveSslKeyToDb(const QByteArray& aKey) ;
    bool loadSslCertFromDb() ;
    bool loadSslKeyFromDb() ;
    void deleteOldestConnectedNode(); /**< prevent db table from overflowing */
    void countNodes() ; /**< internal book-keeping */
    Q_IPV6ADDR ipv6AddrFromUints(quint32 aLeastSignificant,
                                 quint32 aLessSignificant,
                                 quint32 aMoreSignificant,
                                 quint32 aMostSignificant ) const ;
    void removeNodeFromRecentlyFailedList(const Hash& aConnectedHostFingerPrint);
signals:
    void error(MController::CAErrorSituation aError,
               const QString& aExplanation) ;
private:
    MController *iController  ;
    Hash* iFingerPrintOfThisNode ; /**< set by method openOrCreateSSLCertificate */
    QSslCertificate* iThisNodeCert ; /**< set by method openOrCreateSSLCertificate */
    QSslKey* iThisNodeKey ; /**< set by method openOrCreateSSLCertificate */
    QList<HostConnectQueueItem> iHotAddresses ;
    const Model& iModel ;
    QString iDataDir ;
    QList<Node *> iConnectionWishList ;
    /**
     * this variable holds list of nodes that we've tried to
     * connect and failed. purpose of this is that we don't
     * immediately try again
     */
    QList<QPair<Hash,unsigned> > iRecentlyFailedNodes ;
    int iTimerId ; /**< periodical timer */
} ;
#endif
