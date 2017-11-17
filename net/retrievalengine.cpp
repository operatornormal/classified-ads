/*     -*-C++-*- -*-coding: utf-8-unix;-*-
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
#include "retrievalengine.h"
#include "../log.h"
#include "../controller.h"
#include "../datamodel/model.h"
#include "node.h"
#include "connection.h"

RetrievalEngine::RetrievalEngine(Controller* aController,
                                 Model& aModel) :
    QTimer(aController),
    iController(aController),
    iModel(aModel),
    iNowRunning(false) {
    iNodeCandidatesToTryQuery.clear() ;
    connect(this, SIGNAL(timeout()), this, SLOT(run()));
}

RetrievalEngine::~RetrievalEngine() {
    LOG_STR("RetrievalEngine::~RetrievalEngine") ;
    emptyNodeCandidateList() ;
    LOG_STR("RetrievalEngine::~RetrievalEngine out") ;
}


void RetrievalEngine::run() {
    if ( iNowRunning == false ) {
        iNowRunning = true ;
        if ( iObjectBeingRetrieved.iRequestedItem == KNullHash ) {
            // we had nothing to retrieve, go on and start
            iModel.lock() ;
            if ( iDownloadQueue.size() > 0 ) {
                iObjectBeingRetrieved = iDownloadQueue.takeAt(0) ;
                iObjectBeingRetrieved.iState = NetworkRequestExecutor::NewRequest ;
            }
            iModel.unlock() ;
            iObjectBeingRetrieved.iTimeStampOfLastActivity= QDateTime::currentDateTimeUtc().toTime_t();
        }

        if ( iObjectBeingRetrieved.iRequestedItem != KNullHash ) {
            iModel.lock() ;
            switch ( iObjectBeingRetrieved.iState ) {

            case NetworkRequestExecutor::NewRequest: {
                // do internal initialization:
                iNodesSuccessfullyConnected.clear();
                iNodesFailurefullyConnected.clear();
                emptyNodeCandidateList() ;

                // initial stage, check if we have named node:
                if( iObjectBeingRetrieved.iDestinationNode != KNullHash ) {
                    // we have candidate, check if it is connected, or should
                    // we have it on wishlist:
                    if(iModel.nodeModel().isNodeAlreadyConnected(iObjectBeingRetrieved.iDestinationNode)) {
                        sendQueryToNode(iObjectBeingRetrieved.iDestinationNode) ;
                    } else {
                        iNodeCandidatesToTryQuery.append(iObjectBeingRetrieved.iDestinationNode) ;
                        iModel.nodeModel().addNodeToConnectionWishList(iObjectBeingRetrieved.iDestinationNode);
                    }
                }
                // additionally, to our connected nodes send a query
                // about nodes around that hash we want:
                NetworkRequestExecutor::NetworkRequestQueueItem hashReq ;
                hashReq.iRequestType = RequestForNodesAroundHash ;
                hashReq.iRequestedItem = iObjectBeingRetrieved.iRequestedItem ;
                hashReq.iState = NetworkRequestExecutor::NewRequest ;
                hashReq.iMaxNumberOfItems = 10 ; // ask from 10 nodes currently connected
                iModel.addNetworkRequest(hashReq) ; // datamodel will add to queue
                // and netreq-executor will send to 10 nodes
                iObjectBeingRetrieved.iState = NetworkRequestExecutor::Processing ;
                iObjectBeingRetrieved.iTimeStampOfLastActivity= QDateTime::currentDateTimeUtc().toTime_t();
            }
                break ;
            case NetworkRequestExecutor::Processing: {
                if ( iObjectBeingRetrieved.iTimeStampOfLastActivity + 10 <
                     QDateTime::currentDateTimeUtc().toTime_t() ) {
                    // 10 seconds passed since we asked for node references
                    // around hash: proceed to ask for connections to
                    // given node
                    QList<Node *>* nodesToTry =
                        iModel.nodeModel().getNodesAfterHash(iObjectBeingRetrieved.iRequestedItem,
                                                             20, // 20 nodes
                                                             300 ) ;// at most 5 hours old

                    while ( ! nodesToTry->isEmpty() ) {
                        Node* connectCandidate ( nodesToTry->takeFirst() ) ;
                        if (  connectCandidate->nodeFingerPrint() != iController->getNode().nodeFingerPrint() ) {
                            if ( iModel.nodeModel().isNodeAlreadyConnected(connectCandidate->nodeFingerPrint()) ) {
                                sendQueryToNode(connectCandidate->nodeFingerPrint()) ;
                                QLOG_STR("In retrieve::processing node " + connectCandidate->nodeFingerPrint().toString() + " was already connected") ;
                                delete connectCandidate ;
                            } else {
                                iNodeCandidatesToTryQuery.append(connectCandidate->nodeFingerPrint()) ;
                                iModel.nodeModel().addNodeToConnectionWishList(connectCandidate) ;
                                QLOG_STR("In retrieve::processing node " + connectCandidate->nodeFingerPrint().toString() + " was added to wishlist") ;
                            }
                        }
                    }
                    if ( iNodeCandidatesToTryQuery.size() > 0 ) {
                        iObjectBeingRetrieved.iState = NetworkRequestExecutor::NodeIsInWishList ;
                        iObjectBeingRetrieved.iTimeStampOfLastActivity= QDateTime::currentDateTimeUtc().toTime_t();
                    } else {
                        // did not add anything to wishlist ; set timestamp back
                        // to past so this state-machine will spam the query to
                        // every connected node ; better than nothing
                        iObjectBeingRetrieved.iState = NetworkRequestExecutor::NodeIsInWishList ;
                        iObjectBeingRetrieved.iTimeStampOfLastActivity= QDateTime::currentDateTimeUtc().toTime_t() - 120;
                    }
                }
            }
                break ;
            case NetworkRequestExecutor::NodeIsInWishList: {
                if ( iObjectBeingRetrieved.iTimeStampOfLastActivity + 60 <
                     QDateTime::currentDateTimeUtc().toTime_t() ) {
                    // after one minute decide that we're not gonna get
                    // it from nodes where we requested connection to ..
                    // spam the request to our connected nodes
                    const QList <Connection *>& currentlyOpenConnections ( iModel.getConnections() ) ;
                    Node* n ( NULL ) ;
                    Connection* c (NULL) ;
                    for ( int i = 0 ; i < 10 &&
                              i< currentlyOpenConnections.size() ;
                          i++ ) {
                        c = currentlyOpenConnections.at(i) ;
                        if ( c && ( ( n = c->node() ) != NULL ) ) {
                            sendQueryToNode(n->nodeFingerPrint()) ;
                        }
                    }
                    iObjectBeingRetrieved.iState=NetworkRequestExecutor::RequestBeingSentAround;
                    iObjectBeingRetrieved.iTimeStampOfLastActivity= QDateTime::currentDateTimeUtc().toTime_t();
                } else {
                    checkForSuccessfullyConnectedNodes() ;
                    checkForUnSuccessfullyConnectedNodes() ;
                }
            }
                break ;
            case NetworkRequestExecutor::RequestBeingSentAround: {
                if ( iObjectBeingRetrieved.iTimeStampOfLastActivity + 60 <
                     QDateTime::currentDateTimeUtc().toTime_t() ) {
                    // after one minute+one minute give up
                    emit notifyOfContentNotReceived(iObjectBeingRetrieved.iRequestedItem ,
                                                    iObjectBeingRetrieved.iRequestType ) ;
                    iObjectBeingRetrieved.iRequestedItem= KNullHash ;
                    iObjectBeingRetrieved.iState=NetworkRequestExecutor::ReadyToSend;
                } else {
                    checkForSuccessfullyConnectedNodes() ;
                    checkForUnSuccessfullyConnectedNodes() ;
                }
            }
                break;
            case NetworkRequestExecutor::ReadyToSend:
                // final stage, do nothing here
                break ;
            }
            iModel.unlock() ;
        } // if iRequestedItem was not nullhash
        iNowRunning = false ;
    }
}


void RetrievalEngine::emptyNodeCandidateList() {
    iNodeCandidatesToTryQuery.clear() ;
}


void RetrievalEngine::nodeConnectionAttemptStatus(Connection::ConnectionState aStatus,
        const Hash aHashOfAttemptedNode ) {
    LOG_STR2("RetrievalEngine::nodeConnectionAttemptStatus %d in", aStatus) ;
    LOG_STR2("RetrievalEngine::nodeConnectionAttemptStatus %s ", qPrintable(aHashOfAttemptedNode.toString())) ;
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


void RetrievalEngine::sendQueryToNode(const Hash& aNode) {
    if ( iObjectBeingRetrieved.iRequestedItem != KNullHash ) {
        if ( iNodesFailurefullyConnected.contains ( aNode ) ) {
            iNodesFailurefullyConnected.removeOne(aNode) ;
        }
        if ( iNodesSuccessfullyConnected.contains ( aNode ) ) {
            iNodesSuccessfullyConnected.removeOne(aNode) ;
        }
    }
    struct NetworkRequestExecutor::NetworkRequestQueueItem queryReq = iObjectBeingRetrieved  ;
    queryReq.iDestinationNode = aNode ;
    queryReq.iState = NetworkRequestExecutor::NewRequest ;
    iModel.addNetworkRequest(queryReq) ;
}
//
// this method is for periodically checking if any nodes we've
// asked to be connected to, has actually been connected.
// if such miracle has taken place, send the stuff and remove
// node from list-of-nodes-to-ask (iNodeCandidatesToTryQuery)
//
void RetrievalEngine::checkForSuccessfullyConnectedNodes() {
    for ( int i = iNodeCandidatesToTryQuery.size()-1 ;
            ( i >=0 ) && ( iObjectBeingRetrieved.iRequestedItem != KNullHash ) ;
            i-- ) {
        if ( iNodesSuccessfullyConnected.contains(iNodeCandidatesToTryQuery[i] ) ) {
            // if we came here it means that we've asked for connection to
            // particular node and it happened
            LOG_STR2("RetEng: After asking, node has been connected: %s", qPrintable(iNodeCandidatesToTryQuery[i].toString())) ;
            sendQueryToNode(iNodeCandidatesToTryQuery[i]) ;
        }
    }
    // can be emptied every time ; if there were anything important, it
    // has been processed above
    iNodesSuccessfullyConnected.clear() ;
}

//
// this is for pruning from ask-list those nodes that we've tried
// to connect with no success
//
void RetrievalEngine::checkForUnSuccessfullyConnectedNodes() {
    for ( int i = iNodeCandidatesToTryQuery.size()-1 ;
            ( i >=0 ) && ( iObjectBeingRetrieved.iRequestedItem != KNullHash ) ;
            i-- ) {
        if ( iNodesFailurefullyConnected.contains(iNodeCandidatesToTryQuery[i]) ) {
            // if we came here it means that we've asked for connection to
            // particular node and it did not happen
            LOG_STR2("RetEng: After asking, node has not been connected: %s", qPrintable(iNodeCandidatesToTryQuery[i].toString())) ;
            iNodeCandidatesToTryQuery.removeAt(i) ;
        }
    }
    // can be emptied every time ; if there were anything important, it
    // has been processed above
    iNodesFailurefullyConnected.clear() ;
}

void RetrievalEngine::startRetrieving(  NetworkRequestExecutor::NetworkRequestQueueItem aObject,
                                        bool aIsPriorityWork) {
    bool isAlreadyInQueue ( false ) ;

    for ( int i = 0 ; i < iDownloadQueue.size() ; i++ ) {
        if (iDownloadQueue.at(i).iRequestedItem == aObject.iRequestedItem &&
                iDownloadQueue.at(i).iRequestType == aObject.iRequestType ) {
            isAlreadyInQueue = true ;
            break ;
        }
    }
    if ( isAlreadyInQueue == false && aIsPriorityWork == false ) {
        iDownloadQueue.append(aObject) ;
    } else if (aIsPriorityWork == true &&
               iObjectBeingRetrieved.iRequestedItem != aObject.iRequestedItem) {
        // so we got another priority work: queue the current item and
        // start fetching this new one:

        // insert previously active item in front
        iDownloadQueue.insert(0, iObjectBeingRetrieved) ;
        iObjectBeingRetrieved.iRequestedItem = KNullHash ;
        emptyNodeCandidateList() ;
        iNodesSuccessfullyConnected.clear();
        iNodesFailurefullyConnected.clear() ;
        iDownloadQueue.insert(0, aObject) ; // insert in front
    } else {
        // were obviously already downloading said item, either via queue
        // or in iObjectBeingRetrieved -> do no thing
    }
}

void RetrievalEngine::notifyOfContentReceived(const Hash& aHashOfContent,
        const ProtocolItemType aTypeOfReceivdContent ) {
    ProtocolItemType typeToCheckFromQueue ;
    bool doCheck ( false ) ;
    switch ( aTypeOfReceivdContent ) {
    case BinaryBlob:
        typeToCheckFromQueue = RequestForBinaryBlob ;
        doCheck = true ;
        break ;
    case UserProfile:
        typeToCheckFromQueue = RequestForUserProfile ;
        doCheck = true ;
        break ;
    case ClassifiedAd:
        typeToCheckFromQueue = RequestForClassifiedAd ;
        doCheck = true ;
        break ;
    default:
        doCheck = false ;
        break ;
    }
    if ( doCheck ) {
        iModel.lock() ;
        if ( iObjectBeingRetrieved.iRequestedItem == aHashOfContent &&
                iObjectBeingRetrieved.iRequestType == typeToCheckFromQueue ) {
            // hurray, we got what we were waiting for
            iObjectBeingRetrieved.iRequestedItem = KNullHash ;
            iObjectBeingRetrieved.iState=NetworkRequestExecutor::ReadyToSend;
            iNodesSuccessfullyConnected.clear();
            iNodesFailurefullyConnected.clear() ;
            emptyNodeCandidateList() ;

        }
        // also check queue:
        for ( int i = iDownloadQueue.size()-1 ; i >= 0  ; i-- ) {
            if (iDownloadQueue.at(i).iRequestedItem == aHashOfContent &&
                    iDownloadQueue.at(i).iRequestType == typeToCheckFromQueue ) {
                iDownloadQueue.removeAt(i) ;
            }
        }
        iModel.unlock() ;
    }
}
