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

#ifndef MOCKUP_NODEMODEL_H
#define MOCKUP_NODEMODEL_H
#include "../mcontroller.h"
#include "../net/node.h"
#include "../datamodel/mmodelprotocolinterface.h"
#include "../datamodel/mnodemodelprotocolinterface.h"
#include <QMutex>
#include <QSslCertificate>
#include <QSslKey>
/**
 * @brief not a real part datamodel. debugging aid.
 */
class MockUpNodeModel : public QObject, public MNodeModelProtocolInterface {
    Q_OBJECT

public:
    /**
     * constructor
     */
    MockUpNodeModel(MController *aMController) ;
    /**
     * Destructor
     */
    ~MockUpNodeModel() ;
    virtual bool nodeGreetingReceived(Node& aNode ,
                                      bool aWasInitialGreeting = false )  ;
    virtual Hash& nodeFingerPrint() ; /**< returns fingerprint of this node */
    virtual int listenPortOfThisNode()  ; /**< TCP listen port number method */
    virtual const QSslCertificate& nodeCert()  const  ;
    /** getter for ssl certificate of SSL sock */
    virtual const QSslKey& nodeKey()  const  ;
    virtual QByteArray* getNextItemToSend(Connection& aConnection)   ;
    virtual Node* nodeByHash(const Hash& aHash) ;
    virtual QList<Node *>* getNodesBeforeHash(const Hash& aHash,
            int aMaxNodes)  ;
    virtual void       closeOldestInactiveConnection()  ;
    virtual QList<Node *>* getNodesAfterHash(const Hash& aHash,
            int aMaxNodes,
            int aMaxInactivityMinutes = -1 )  ;
    virtual QList<HostConnectQueueItem> getHotAddresses() ;
    virtual bool updateNodeLastConnectTimeInDb(Node& aNode)  ;
    virtual QList<Node *>* getHotNodes(int aMaxNodes)  ;

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
                                      int aPort )  ;
    /**
     * method for adding a node to connection-wishlist.
     * network connector engine will then later pick them up.
     * nodemodel will take ownership of the node and delete
     * the object later.
     */
    virtual bool addNodeToConnectionWishList(Node* aNode)  ;

    /**
     * method for adding a node to connection-wishlist.
     * network connector engine will then later pick them up.
     */
    virtual bool addNodeToConnectionWishList(const Hash& aNode)  ;

    /**
     * method for getting one node from wishlist.
     * caller is obliged to delete the node returned.
     * @return node or null if there is nothing in wishlist
     */
    virtual Node* nextConnectionWishListItem()   ;
    /**
     * method for checking if a node is already connected
     */
    virtual bool isNodeAlreadyConnected(const Node& aNode) const ;
    /**
     * Method for checking if a node is already connected.
     * This version checks only hash, not addresses.
     */
    virtual bool isNodeAlreadyConnected(const Hash& aHash) const  ;

    virtual Hash bucketEndHash(const Hash& aFingerPrintOfNodeAsking)  ;
    /**
     * updates last mutual connect time, used for deciding what content
     * to send automatically upon node connect
     */
    virtual bool updateNodeLastMutualConnectTimeInDb(const Hash& aNodeFp,
            quint32 aTime )  ;
    virtual void setListenPortOfThisNode(int port)  ;
    virtual QList<Node*>* getNodesBeforeHash(const Hash& h, unsigned int i);
    virtual QList<Node*>* getNodesAfterHash(const Hash& h, unsigned int u, int i) ;
    virtual void setDnsName(QString aName) ;
    virtual QString getDnsName()  ;
    /** used to offer node to list of recently failed connections.
     * this model maintains a list of such nodes and tries to
     * not immediately re-connect a recently failed node */
    virtual void offerNodeToRecentlyFailedList(const Hash& aFailedNodeHash) ;
    /**
     * setter for node cert and key 
     */
    bool setNodeCertAndKey ( const QString& aCertPem, 
                             const QString& aKeyPem) ; 
public:
    Node* iLastNodeReceived ;
private:
    MController *iController ;
    Hash* iFingerPrintOfThisNode ; /**< set by method openOrCreateSSLCertificate */
    QString iDnsName ;
    QSslCertificate iCert ; 
    QSslKey iKey ;
} ;
#endif
