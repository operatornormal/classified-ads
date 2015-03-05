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



#ifndef NETWORK_CONNECTOR_ENG_H
#define NETWORK_CONNECTOR_ENG_H

#include <QTcpSocket>
#include <QUdpSocket>
#include <QHostAddress>
#include <QPair>
#include "connection.h" // for ConnectionObserver
#include "../controller.h"
#include "../datamodel/model.h"

class ProtocolMessageParser ;

/**
 * @brief Class that will initiate 1 or more network connections.
 *
 * This may be notified about situation where network connection
 * might be in order.
 *
 * This will also initiate first connection to seed node,
 * notifying controller if failed.
 */
class NetworkConnectorEngine : public QThread {
  Q_OBJECT
public:
  /**
   * Constructor
   */
  NetworkConnectorEngine(Controller *aController,
                         Model *aModel,
                         Connection::ConnectionObserver &aObserver) ;
  /**
   * Destructor
   */
  ~NetworkConnectorEngine() ;
  /**
   * this class is a thread, thus run.
   *
   * this method has its own execution context so it won't
   * hurt the UI or the incoming connections.
   *
   * this will ask datamodel for nodes to connect and try to
   * arrange connections to given nodes, some will succeed
   * and some then will be less successful.
   */
  void run();
public slots:
  void threadIsDeleted() ;
  void blackListNetworkAddr(QHostAddress aAddr) ;
  void nodeConnectionAttemptStatus(Connection::ConnectionState aStatus,
                                   const Hash aHashOfAttemptedNode );
signals:
  void error(QTcpSocket::SocketError socketError);
private:
  /** internal method for starting a new connection */
  void spawnNewConnection(const QHostAddress& aAddr,
                          const int aPort,
                          const Hash& aHashOfNodeToConnect) ;
  /** method for updating list of nodes to connect */
  void updateListOfNodesToConnect() ;
  /**
   *  prune from list self and nodes already connected.
   *  @param aListToPrune is list of nodes that check
   *  @return this will return true if there were at least one node
   *          that was already connected
   */
  bool pruneConnectionProspectList(QList<Node *>* aListToPrune) const ;
  /**
   * method for sending advertisment about node in ip protocl
   * broadcast packet
   */
  void sendBroadCast() ; 
  /**
   * Method for checking if there is anything in datamodels
   * connection wishlist
   */
  bool tryServeWishListItem() ; 
  /**
   * method for creating connections to nodes that are supposed to 
   * store private messages destined to profiles whose private
   * keys we have in this node. This is supposed to work in
   * conjunction with @PrivMessageModel::fillBucket() that
   * it will then, in turn, send requests to said nodes about
   * private messages that we're supposed to receive. 
   */
  void createConnectionsToNodesStoringPrivateMessages() ;
  /**
   * Sends query (poll) about private messages to given node.
   * Node must be connected. 
   * This method polls not only private messages but also
   * profile comments - reason for this is simple: they
   * have the same bucket so as we're already making
   * commections to nodes storing private messaeges we'll
   * find the comments from same bucket. 
   * 
   * Naturally this only helps the profile operator as he'll
   * this way find his own comments ; for the rest of the
   * comment-reading crowd a separate mechanism needs to be in
   * place. 
   */
  void sendPrivateMessagesPoll(const Hash& aNodeFingerPrint) ;
private: // methods:
  bool performDNSLookup(QHostAddress* aAddressToBeSet,
			const QString& aFromDnsName,
			bool aDoWeHaveIpv6) ; 
public:
  bool iNeedsToRun ;
private: // data
  Controller *iController ;
  Model *iModel ;
  Connection::ConnectionObserver &iConnectionObserver ;
  QList<MNodeModelProtocolInterface::HostConnectQueueItem> iAddressesInOrderToConnect ;
  QUdpSocket* iBroadCastSocket ; 
  time_t iTimeOfLastBroadCast ;
  /** 
   * this is time when private messages were last polled for any
   * profile ; this is used for sequencing the polls so that 
   * each profile gets decent and fair polling periods 
   */
  time_t iTimeOfLastMsgPoll ; 
  /**
   * this is timestamp of last message poll for individual 
   * profile ; this is sent to remote nodes and the nodes are
   * asked to send messages that they've received after 
   * the timestamp presented here
   */
  quint32 iTimeOfLastMsgPollForProfile ;
  /**
   * Addresses that we're supposed to not connect. This easily
   * gets at least addresses of self in a box that has 
   * multiple network addresses (or multiple temporary 
   * ipv6 addresses)
   */
  QList<QHostAddress> iAddressBlacklist ; 
  Hash iProfileToPollPrivateMessages ;
  QList<Hash> iPrivateMessagePollWishList ; 
} ;
#endif
