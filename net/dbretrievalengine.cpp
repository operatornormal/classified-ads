/*  -*-C++-*- -*-coding: utf-8-unix;-*-
  Classified Ads is Copyright (c) Antti Järvinen 2013-2017.

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
#include "dbretrievalengine.h"
#include "../log.h"
#include "../controller.h"
#include "../datamodel/model.h"
#include "node.h"
#include "connection.h"
#include "protocol_message_formatter.h"

DbRecordRetrievalEngine::DbRecordRetrievalEngine(Controller* aController,
                                                 Model& aModel) :
    QTimer(aController),
    iController(aController),
    iModel(aModel),
    iNowRunning(false) {
    LOG_STR("DbRecordRetrievalEngine::DbRecordRetrievalEngine") ;
    connect(this, SIGNAL(timeout()), this, SLOT(run()));
}

DbRecordRetrievalEngine::~DbRecordRetrievalEngine() {
    LOG_STR("DbRecordRetrievalEngine::~DbRecordRetrievalEngine out") ;
}


// very simple algorithm here: for each newly-connected node
// send every query that we have in stock. query.second contains
// book-keeping about nodes that already have been sent the
// query that is kept in query.first. 
void DbRecordRetrievalEngine::run() {
    if ( iNowRunning == false ) {
        iNowRunning = true ;
        iModel.lock() ;
        if ( iSearchTerms.size() > 0 &&
             iNodesSuccessfullyConnected.size() > 0 ) {
            // spam each connected node with all searches that we have pending:
            for ( int i = iSearchTerms.size()-1 ; i >= 0 ; i-- ) {
                DlQueueItem& query ( iSearchTerms[i] ) ; 
                foreach ( const Hash& connectedNode, iNodesSuccessfullyConnected ) {
                    if ( query.second.contains(connectedNode) == false ) {
                        sendQueryToNode(query.first, 
                                        connectedNode) ; 
                        query.second.append(connectedNode) ; 
                    }
                }
            }
        } // if ( iSearchTerms.size() > 0 ) 
        iNodesSuccessfullyConnected.clear() ; 
        iModel.unlock() ;
        iNowRunning = false ;
    }
}


void DbRecordRetrievalEngine::nodeConnectionAttemptStatus(Connection::ConnectionState aStatus,
                                                          const Hash aHashOfAttemptedNode ) {
    LOG_STR2("DbRecordRetrievalEngine::nodeConnectionAttemptStatus %d in", aStatus) ;
    LOG_STR2("DbRecordRetrievalEngine::nodeConnectionAttemptStatus %s ", qPrintable(aHashOfAttemptedNode.toString())) ;
    // use model to lock our own resources too..
    iModel.lock() ;
    for ( int i = 0 ; i < iNodeCandidatesToTryQuery.size() ; i++ ) {
        if ( iNodeCandidatesToTryQuery[i] == aHashOfAttemptedNode ) {
            if ( aStatus == Connection::Open ) {
                if ( ! iNodesSuccessfullyConnected.contains(aHashOfAttemptedNode) ) {
                    iNodesSuccessfullyConnected.append(aHashOfAttemptedNode) ;
                }
            } else {
                if ( ! iNodesFailurefullyConnected.contains(aHashOfAttemptedNode) ) {
                    iNodesFailurefullyConnected.append(aHashOfAttemptedNode) ;
                }
            }
            break ;
        }
    }
    iModel.unlock() ;
    // so just add the hash into 1 of our internal arrays, run() will pick up
    // the items from list and try to do something clever
}


// note that this method is called via datamodel and lock is
// already on during this call -> do not try (un)locking 
// for 2nd time.
void DbRecordRetrievalEngine::startRetrieving(CaDbRecord::SearchTerms aSearchTerms) {
    if ( aSearchTerms.iFromCollection == KNullHash ) {
        return ; // can't search from unspecified collection
    }
    // lets see if our search conditions are already included
    // in at least one query already found from queue. 
    // -> spamming the network with smaller query makes no
    // sense if broader query is already underway
    foreach ( const DlQueueItem& oldQuery, iSearchTerms) {
        if ( aSearchTerms < oldQuery.first ) {
            return ; // proceed no further
        }
    }
    DlQueueItem dlQueueItem ( aSearchTerms, 
                              QList<Hash>() ) ; 

    // as new content is added, add also candidate nodes to connection
    // wishlist:
    QList<Node *>* nodesToTry =
        iModel.nodeModel().getNodesAfterHash(aSearchTerms.iFromCollection,
                                             20, // 20 nodes
                                             300 ) ;// at most 5 hours old

    if ( nodesToTry ) {
        while ( ! nodesToTry->isEmpty() ) {
            Node* connectCandidate ( nodesToTry->takeFirst() ) ;
            // here it is possible that node is already
            // connected, in which case send query straight away
            // and don't put node into connection wishlist
            if ( iModel.nodeModel().isNodeAlreadyConnected (*connectCandidate) ) {
                sendQueryToNode(aSearchTerms, 
                                connectCandidate->nodeFingerPrint()) ; 
                // mark this node to list of nodes already spammed with this query:
                dlQueueItem.second.append(connectCandidate->nodeFingerPrint()) ; 
                delete connectCandidate ; 
            } else {
                // node was not already connected, put it into wishlist
                iNodeCandidatesToTryQuery.append(connectCandidate->nodeFingerPrint()) ;
                iModel.nodeModel().addNodeToConnectionWishList(connectCandidate) ;
            }
        }
        delete nodesToTry; 
    }
    iSearchTerms.append(dlQueueItem) ; 
}

void DbRecordRetrievalEngine::stopRetrieving() {
        iModel.lock() ;
        iSearchTerms.clear() ; 
        iNodeCandidatesToTryQuery.clear() ; 
        iNodesSuccessfullyConnected.clear() ; 
        iNodesFailurefullyConnected.clear() ; 
        iModel.unlock() ;
}

void DbRecordRetrievalEngine::notifyOfContentReceived(const Hash& aHashOfContent,
                                                      const ProtocolItemType aTypeOfReceivedContent ) {
    if ( aTypeOfReceivedContent == DbRecord ) {
        iModel.lock() ;
        for ( int i = iSearchTerms.size()-1 ; i >= 0 ; i-- ) {
            if ( iSearchTerms[i].first.iById == aHashOfContent ) {
                iSearchTerms.removeAt(i) ; 
            }
        }
        iModel.unlock() ;
    }
}

// when this is called, datamodel must be locked
void DbRecordRetrievalEngine::sendQueryToNode(const CaDbRecord::SearchTerms& aSearchTerms, 
                                              const Hash& aNodeFingerPrint) {
    const QList <Connection *>& openConnections = iModel.getConnections() ;
    foreach ( Connection* c, openConnections ) {
        if ( c->connectionState() == Connection::Open) {
            const Node* n = c->node() ; 
            if ( n && n->nodeFingerPrint() == aNodeFingerPrint ) {
                QByteArray* serializedSearch = new QByteArray() ;
                serializedSearch->append( ProtocolMessageFormatter::dbSearchTerms(aSearchTerms)) ;
                if ( serializedSearch->size() > 0 ) {
                    c->iNextProtocolItemToSend.append(serializedSearch) ;
                } else {
                    delete serializedSearch ;
                }
                return ; 
            }
        }
    }
}
