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

#ifndef M_NODEMODEL_PROTOCOL_INTERFACE_H
#define M_NODEMODEL_PROTOCOL_INTERFACE_H
#include <QPair>
class Connection ;
class QHostAddress ;
class Node ;
class QSslKey ;
class QSslCertificate ;
class Hash ;


/**
 * @brief Pure-virtual interface of nodemodel.
 *
 * Interface for node-specific part of datamodel. Exists Mainly for testing
 * purposes. Contains all methods of node-model that are of interest
 * to parties outside datamodel. See @ref NodeModel.
 */
class MNodeModelProtocolInterface  {
public:
  /**
   * structure used for communicating lists of nodes to connect
   * between datamodel and networking parts. Basically these 
   * are nodes but reduced to plain addr/hash. 
   */
  typedef struct HostConnectQueueItemStructure {
    QHostAddress iAddress ; /**< ip addr */
    int iPort ; /**< tcp port */
    Hash iNodeHash ; /**< fingerprint of node, if known */
    bool operator==(const struct HostConnectQueueItemStructure& a) const ;
  } HostConnectQueueItem ; 
  virtual bool nodeGreetingReceived(Node& aNode ,
                                    bool aWasInitialGreeting = false ) = 0 ;
  virtual Hash& nodeFingerPrint() = 0 ; /**< returns fingerprint of this node */
  virtual int listenPortOfThisNode() = 0 ; /**< TCP listen port number method */
  /** for setting of tcp listen port */
  virtual void setListenPortOfThisNode(int port) = 0 ; 
  virtual const QSslCertificate& nodeCert()  const = 0 ;
  /** getter for ssl certificate of SSL sock */
  virtual const QSslKey& nodeKey()  const = 0 ;
  virtual QByteArray* getNextItemToSend(Connection& aConnection) = 0  ;
  virtual Node* nodeByHash(const Hash& aHash) = 0 ;
  virtual QList<Node *>* getNodesBeforeHash(const Hash& aHash,
      unsigned aMaxNodes) = 0 ;
  virtual void       closeOldestInactiveConnection() = 0 ;
  virtual QList<Node *>* getNodesAfterHash(const Hash& aHash,
      unsigned aMaxNodes,
      int aMaxInactivityMinutes = -1 ) = 0 ;
  virtual QList<HostConnectQueueItem> getHotAddresses() = 0 ;
  virtual bool updateNodeLastConnectTimeInDb(Node& aNode) =0 ;
  virtual QList<Node *>* getHotNodes(int aMaxNodes) = 0 ;
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
				    int aPort ) = 0 ;
  /**
   * method for adding a node to connection-wishlist.
   * network connector engine will then later pick them up.
   * nodemodel will take ownership of the node and delete
   * the object later.
   */
  virtual void addNodeToConnectionWishList(Node* aNode) = 0 ;

  /**
   * method for adding a node to connection-wishlist.
   * network connector engine will then later pick them up.
   */
  virtual bool addNodeToConnectionWishList(const Hash& aNode) = 0 ;

  /**
   * method for getting one node from wishlist.
   * caller is obliged to delete the node returned.
   * @return node or null if there is nothing in wishlist
   */
  virtual Node* nextConnectionWishListItem() = 0  ;
  /**
   * method for checking if a node is already connected 
   */
  virtual bool isNodeAlreadyConnected(const Node& aNode) const = 0 ; 
  /**
   * Method for checking if a node is already connected.
   * This version checks only hash, not addresses.
   */
  virtual bool isNodeAlreadyConnected(const Hash& aHash) const = 0 ; 
  /**
   * Important method regarding churn here. This is called from
   * @ref Connection class and this method is used to determine
   * what content belongs to same bucket with the node that
   * is being served by that Connection. Content in the same bucket
   * with the node is then sent over to node ; to keep the content
   * alive in the network. 
   *
   * Intent here is find bucket size where we have approximately
   * 20 live nodes in a bucket. 
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
  virtual Hash bucketEndHash(const Hash& aFingerPrintOfNodeAsking) = 0 ;
  /**
   * updates last mutual connect time, used for deciding what content
   * to send automatically upon node connect
   */ 
  virtual bool updateNodeLastMutualConnectTimeInDb(const Hash& aNodeFp,
						   quint32 aTime ) = 0 ;
  /** called from settings dialog */
  virtual void setDnsName(QString aName) = 0 ;
  /** called from settings dialog and own node construction */
  virtual QString getDnsName() = 0 ;
  /** used to offer node to list of recently failed connections.
   * this model maintains a list of such nodes and tries to 
   * not immediately re-connect a recently failed node */
  virtual void offerNodeToRecentlyFailedList(const Hash& aFailedNodeHash) = 0 ; 
};
#endif
