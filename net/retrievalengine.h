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



#ifndef RETRIEVAL_ENG_H
#define RETRIEVAL_ENG_H

#include <QHostAddress>
#include <QPair>
#include "connection.h" // for ConnectionObserver
#include "../controller.h"
#include "../datamodel/model.h"

/**
 * @brief Network-connection logic relating fetching items from DHT
 *
 * This class has some similarity to @ref PublishingEngine but this
 * does somewhat the reverse. This is given a hash of object to 
 * find from network ; in order to so, it will initiate connections
 * nearby nodes and ask for the content in-demand. 
 */
class RetrievalEngine : public QTimer {
  Q_OBJECT
public:
  /**
   * Constructor
   * @param aController application controller. not owned
   * @param aModel persistent storage.
   */
  RetrievalEngine(Controller* aController,
		  Model& aModel ) ;
  /**
   * Destructor
   */
  ~RetrievalEngine() ;
  /**
   * command-interface for this class: start to do work
   * @param aObject specifies the object to dl. 
   * @param aIsPriorityWork is set to true if download should
   *                       start right away, bypassing all other
   *                       stuff that might be in the queue
   */
  void startRetrieving(  NetworkRequestExecutor::NetworkRequestQueueItem aObject,
			 bool aIsPriorityWork) ; 
  /**
   * when content is received, we may want to check if it
   * was the content we were waiting for. this method 
   * is for performing that check
   */
  void notifyOfContentReceived(const Hash& aHashOfContent,
			       const ProtocolItemType aTypeOfReceivdContent );
signals:
  void error(QTcpSocket::SocketError socketError);
  void notifyOfContentNotReceived(const Hash& aHashOfContent,
				  const ProtocolItemType aTypeOfNotReceivdContent );
public slots:
  /** 
   * when connection is attempted, @ref NetworkListener will
   * emit the status (failed or success) of the connection,
   * emitted signal is connected here
   */
  void nodeConnectionAttemptStatus(Connection::ConnectionState aStatus,
                                   const Hash aHashOfAttemptedNode );
  /**
   * this class is a not a thread, but QTimer, thus run.
   */
  void run();
private:
  void emptyNodeCandidateList() ; 
  void askConnectionsForNodesOnConnectList() ; 
  void sendQueryItemToAlreadyConnectedNodes() ; 
  void sendQueryToNode(const Hash& aNode) ; 
  void checkForSuccessfullyConnectedNodes() ; 
  void checkForUnSuccessfullyConnectedNodes() ; 
public:
  /** when this is set to false, thread will terminate and run() return */
  bool iNeedsToRun ;
private: // data
  Controller* iController ; /**< application controller */
  Model &iModel ; /**< persistent storage */
  /** 
   * what kind of stuff we try. ..this class waits for 
   * exactly one item at time. item currently being fetched
   * is stored in this variable
   */
  NetworkRequestExecutor::NetworkRequestQueueItem iObjectBeingRetrieved ; 
  /** list of nodes where iWorkItem might be pushed to */
  QList<Hash> iNodeCandidatesToTryQuery ; 
  QList<Hash> iNodesSuccessfullyConnected ; 
  QList<Hash> iNodesFailurefullyConnected ; 
  bool iNowRunning ;
  /** queue of items that we should get */
  QList<NetworkRequestExecutor::NetworkRequestQueueItem> iDownloadQueue ; 
} ;
#endif
