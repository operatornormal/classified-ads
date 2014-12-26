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
#include "publishingengine.h"
#include "../log.h"
#include "../controller.h"
#include "../datamodel/model.h"
#include "node.h"
#include "connection.h"

PublishingEngine::PublishingEngine(Controller* aController,
                                   Model& aModel) :
  QTimer(aController),
  iNeedsToRun(true),
  iController(aController),
  iModel(aModel),
  iNowRunning(false) {
  iNodeCandidatesToTryPush.clear() ; 
  iWorkItem.iObjectHash = KNullHash ; // NULL hash in the beginning,
  connect(this, SIGNAL(timeout()), this, SLOT(run()));
}

PublishingEngine::~PublishingEngine() {
  LOG_STR("PublishingEngine::~PublishingEngine") ;
  emptyNodeCandidateList() ; 
  LOG_STR("PublishingEngine::~PublishingEngine out") ;
}

// lets have the logic work like this:
//  quite often poll the table of published works.
//  when there is an item in the table, ask for nodes
//  that are destination addresses for the published items.
//  match that list with list of already-connected nodes ;
//  if we already have connections to nodes that are on
//  our list, do the following loop:
//     1. put published item into send queue
//     2. ask for notification that item has been sent ;
//        upon receiving, update the bangpath in publish-tables
//     3. select next already-connected host
//  when already-connected hosts run out and we still have
//  nodes in our list where content should be sent, run following loop
//     1. ask network-connection engine to obtain connection to host
//     2. if unsuccessful, move to next host
//     3. if success, put published item into send queue
//     4. ask for notificatoin that item has been sent ;
//        upon receiving, update the bangpath in publish-tables
//     5. select next non-connected host
//  with these steps this method should push the published content
//  into 5 hosts. each of these will do the same with publish-count=4,
//  each of those 4 will do the same with publish-count=3, each of
//  those 3 will do the same with publish-count=2
void PublishingEngine::run() {
  if ( iNowRunning == false ) {
    iNowRunning = true ;
    if (iNeedsToRun ) {
      if ( iWorkItem.iObjectHash == KNullHash ) {
	// we have nothing particular to do, sleep for 10s and
	// query
	iModel.lock() ;
	bool found = false ; 
	iNodesSuccessfullyConnected.clear();
	iNodesFailurefullyConnected.clear(); 
	iWorkItem = iModel.nextItemToPublish(&found) ;
	if ( !found ) {
	  iWorkItem.iObjectHash = KNullHash ;
	} else {
	  LOG_STR2("PublishingEngine::run got item to pub %d ", iWorkItem.iObjectType) ;
	  QList<Node *>* nodesToTry = NULL ;  
	  switch ( iWorkItem.iObjectType ) {
	  case ClassifiedAd2NdAddr:
	    nodesToTry    = 
	      iModel.nodeModel().getNodesAfterHash(iWorkItem.i2NdAddr,
						   20, // 20 nodes
						   300 ) ;// at most 5 hours old	    
	    break ;
	  case UserProfileComment:
	    // user profile commments get published to profile address, 
	    // not content address
	    nodesToTry    = 
	      iModel.nodeModel().getNodesAfterHash(iWorkItem.i2NdAddr,
						   20, // 20 nodes
						   300 ) ;// at most 5 hours old	    
	    break ; 
	  default:
	    nodesToTry    = 
	      iModel.nodeModel().getNodesAfterHash(iWorkItem.iObjectHash,
						 20, // 20 nodes
						   300 ) ;// at most 5 hours old	    
	    break ; 
	  }
	  
	  if ( nodesToTry == NULL || nodesToTry->size() == 0 ) {
	    // uh, oh. do not try then.
	    iWorkItem.iObjectHash = KNullHash ;
	    LOG_STR("But got no nodes to try? ") ;
	  } else {
	    emptyNodeCandidateList() ; 	  
	    while ( ! nodesToTry->isEmpty() ) {
	      Node* connectCandidate ( nodesToTry->takeFirst() ) ; 
	      if ( iModel.nodeModel().isNodeAlreadyConnected(*connectCandidate) ) {
		publishToNode(connectCandidate->nodeFingerPrint()) ; 
		delete connectCandidate ;
	      } else {
		iNodeCandidatesToTryPush.append(connectCandidate) ; 
	      }
	    }
	    iStageOfPublish = InitialStage; 
	  }
	  delete nodesToTry ; 
	}
	iModel.unlock() ;
      }
      // ok, we've got something..?
      if ( iWorkItem.iObjectHash != KNullHash ) {
	switch ( iStageOfPublish ) {
	case InitialStage:
	  // ok, we just started. first look how many our desired nodes
	  // are already connected and do those:
	  sendPublishItemToAlreadyConnectedNodes() ;
	  iStageOfPublish = AwaitingConnection; 
	  askConnectionsForNodesOnPublishList() ;
	  break ; 
	case AwaitingConnection:
	  checkForSuccessfullyConnectedNodes() ; 
	  checkForUnSuccessfullyConnectedNodes() ;
	  // check if we managed to run out of nodes:
	  if ( iNodeCandidatesToTryPush.isEmpty() ) {
	    // then pick up next publish-item and leave this behind
	    iWorkItem.iObjectHash = KNullHash ; 
	    LOG_STR("PubEng: No more nodes to try ; picking up next item..") ; 
	  }
	  break ; 
	}
      }
      iNowRunning = false ;
    }
  }
}

void PublishingEngine::emptyNodeCandidateList() {
  while ( ! iNodeCandidatesToTryPush.isEmpty() ) {
    LOG_STR2("in emptyNodeCandidateList len of list is %d", (int)(iNodeCandidatesToTryPush.size())) ;
    Node* n = iNodeCandidatesToTryPush.takeFirst() ; 
    delete n ; 
  }
}

void 	  PublishingEngine::askConnectionsForNodesOnPublishList() {
  // this is run at publish begin: we have item, we have
  // connected nodes. publish happens automatically to
  // nodes that are connected but some nodes then remain..
  // here add the remaining nodes to network engines wishlist
  // so that it will attempt to connect:
  iModel.lock() ;
  for ( int i = 0 ; i < iNodeCandidatesToTryPush.size() ; i++ ) {
    Node* n = iNodeCandidatesToTryPush.at(i) ; 
    // make copy of the node, because datamodel takes ownership:
    Node* wishListItem = Node::fromQVariant(n->asQVariant().toMap(), false) ;
    if ( wishListItem ) {
      iModel.nodeModel().addNodeToConnectionWishList(wishListItem) ;
    }
  }
  iModel.unlock() ;
}

void PublishingEngine::nodeConnectionAttemptStatus(Connection::ConnectionState aStatus,
    const Hash aHashOfAttemptedNode ) {
  LOG_STR2("PublishingEngine::nodeConnectionAttemptStatus %d in", aStatus) ;
  LOG_STR2("PublishingEngine::nodeConnectionAttemptStatus %s ", qPrintable(aHashOfAttemptedNode.toString())) ;
  // use model to lock our own resources too..
  iModel.lock() ;
  for ( int i = 0 ; i < iNodeCandidatesToTryPush.size() ; i++ ) {
    if ( iNodeCandidatesToTryPush[i]->nodeFingerPrint() == aHashOfAttemptedNode ) {
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

void PublishingEngine::sendPublishItemToAlreadyConnectedNodes() {
  iModel.lock() ;
  const QList <Connection *>& currentlyOpenConnections ( iModel.getConnections() ) ; 
  for ( int i = iNodeCandidatesToTryPush.size()-1 ; 
	( i >=0 ) && ( iWorkItem.iObjectHash != KNullHash ) ; 
	i-- ) {
    for ( int j = currentlyOpenConnections.size()-1 ; 
	  ( j >= 0 ) && ( iWorkItem.iObjectHash != KNullHash ) ; 
	  j-- ) {
      const Node* conn_n = currentlyOpenConnections[j]->node() ;
      if ( ( conn_n!= NULL ) && 
	   ( conn_n->nodeFingerPrint() == 
	     iNodeCandidatesToTryPush[i]->nodeFingerPrint() ) ) {
	// yes, a match. 
	Node* n = iNodeCandidatesToTryPush[i] ;
	// here check if that node already got it:
	for ( int k = 0 ; k < iWorkItem.iAlreadyPushedHosts.size() ; k++ ) {
	  LOG_STR2("iWorkItem.iAlreadyPushedHosts contains %u", 
		   iWorkItem.iAlreadyPushedHosts[k]) ; 
	}
	if ( iWorkItem.iAlreadyPushedHosts.size() == 0) {
	  LOG_STR("iWorkItem.iAlreadyPushedHosts is empty") ;
	}
	if ( iWorkItem.iAlreadyPushedHosts.contains(n->nodeFingerPrint().iHash160bits[4])) {
	  LOG_STR("Node already had this content ; skipping in publish") ; 
	  // and also remove from list:
	  iNodeCandidatesToTryPush.takeAt(i) ; 
	  delete n ;
	  n = NULL ;
	} else {
	  // note that publishtonode takes node from iAlreadyPushedHosts
	  // and deletes it ; after the line above, do not try
	  // to reference it again ..
	  publishToNode(n->nodeFingerPrint()) ; 
	  n = NULL ; // and now gone
	}
	break ; // out from loop of j (so that i won't get tried again because it was just deleted
      }
    }
  }
  iModel.unlock() ;
}

void PublishingEngine::publishToNode(const Hash& aNode) {
  if ( iWorkItem.iObjectHash != KNullHash ) {
    if ( iWorkItem.iAlreadyPushedHosts.contains(aNode.iHash160bits[4])) {
      LOG_STR("Node already had this content ; skipping in publish") ; 
    } else {
      struct NetworkRequestExecutor::NetworkRequestQueueItem sendReq ;
      sendReq.iDestinationNode = aNode ;
      sendReq.iRequestedItem = iWorkItem.iObjectHash;
      sendReq.iMaxNumberOfItems = 1 ; 
      sendReq.iBangPath = iWorkItem.iAlreadyPushedHosts ; 
      sendReq.iState = NetworkRequestExecutor::NewRequest ;

      switch ( iWorkItem.iObjectType ) {
      case UserProfile:
	sendReq.iRequestType =  UserProfilePublish ;
	iModel.addNetworkRequest(sendReq) ;
	iWorkItem = iModel.addNodeToPublishedItem(iWorkItem.iObjectHash, aNode) ;
	LOG_STR2("Published profile to node %s", qPrintable(aNode.toString())) ; 
	LOG_STR2("Published profile to node low-order bits %u", aNode.iHash160bits[4]) ; 
	LOG_STR2("Now work-item type is %d",(int) iWorkItem.iObjectType) ; 
	for ( int i = 0 ; i <iWorkItem.iAlreadyPushedHosts.size() ; i++ ) {
	  LOG_STR2("Already published to to node low-order bits %u", iWorkItem.iAlreadyPushedHosts[i]) ; 
	}
	break ;
      case BinaryBlob:
	sendReq.iRequestType =  BinaryFilePublish ;
	iModel.addNetworkRequest(sendReq) ;
	iWorkItem = iModel.addNodeToPublishedItem(iWorkItem.iObjectHash, aNode) ;
	LOG_STR2("Published binary file to node %s", qPrintable(aNode.toString())) ; 
	LOG_STR2("Published binary file to node low-order bits %u", aNode.iHash160bits[4]) ; 
	LOG_STR2("Now work-item type is %d",(int) iWorkItem.iObjectType) ; 
	for ( int i = 0 ; i <iWorkItem.iAlreadyPushedHosts.size() ; i++ ) {
	  LOG_STR2("Already published to to node low-order bits %u", iWorkItem.iAlreadyPushedHosts[i]) ; 
	}
	break ;
      case UserProfileComment:
	sendReq.iRequestType =  ProfileCommentPublish ;
	iModel.addNetworkRequest(sendReq) ;
	iWorkItem = iModel.addNodeToPublishedItem(iWorkItem.iObjectHash, aNode) ;
	LOG_STR2("Published profile comment to node %s", qPrintable(aNode.toString())) ; 
	LOG_STR2("Published profile comment to node low-order bits %u", aNode.iHash160bits[4]) ; 
	LOG_STR2("Now work-item type is %d",(int) iWorkItem.iObjectType) ; 
	for ( int i = 0 ; i <iWorkItem.iAlreadyPushedHosts.size() ; i++ ) {
	  LOG_STR2("Already published to to node low-order bits %u", iWorkItem.iAlreadyPushedHosts[i]) ; 
	}
	break ;
      case ClassifiedAdPublish:
      case ClassifiedAd2NdAddr:
	sendReq.iRequestType = ClassifiedAdPublish ;
	iModel.addNetworkRequest(sendReq) ;
	iWorkItem = iModel.addNodeToPublishedItem(iWorkItem.iObjectHash, aNode) ;
	LOG_STR2("Published ca to node %s", qPrintable(aNode.toString())) ; 
	LOG_STR2("Published ca to node low-order bits %u", aNode.iHash160bits[4]) ; 
	LOG_STR2("Now work-item type is %d",(int) iWorkItem.iObjectType) ; 
	for ( int i = 0 ; i <iWorkItem.iAlreadyPushedHosts.size() ; i++ ) {
	  LOG_STR2("Already published to to node low-order bits %u", iWorkItem.iAlreadyPushedHosts[i]) ; 
	}
	break ;
      case PrivateMessage:
	sendReq.iRequestType = PrivateMessagePublish ;
	iModel.addNetworkRequest(sendReq) ;
	iWorkItem = iModel.addNodeToPublishedItem(iWorkItem.iObjectHash, aNode) ;
	LOG_STR2("Published private msg to node %s", qPrintable(aNode.toString())) ; 
	LOG_STR2("Published private msg to node low-order bits %u", aNode.iHash160bits[4]) ; 
	LOG_STR2("Now work-item type is %d",(int) iWorkItem.iObjectType) ; 
	for ( int i = 0 ; i <iWorkItem.iAlreadyPushedHosts.size() ; i++ ) {
	  LOG_STR2("Already published to to node low-order bits %u", iWorkItem.iAlreadyPushedHosts[i]) ; 
	}
	break ; 
      default:
	LOG_STR2("TODO: unimplemented publish of type %d", iWorkItem.iObjectType) ; 
	break ; 
      }
    }
    for ( int i = iNodeCandidatesToTryPush.size()-1 ; i >= 0 ; i-- ) {
      if ( aNode == iNodeCandidatesToTryPush[i]->nodeFingerPrint() ) {
	Node* n = iNodeCandidatesToTryPush.takeAt(i) ; 
	delete n ; 
	break ; 
      }
    }
    if ( iNodesFailurefullyConnected.contains ( aNode ) ) {
      iNodesFailurefullyConnected.removeOne(aNode) ; 
    }
    if ( iNodesSuccessfullyConnected.contains ( aNode ) ) {
      iNodesSuccessfullyConnected.removeOne(aNode) ; 
    }
  }
}
//
// this method is for periodically checking if any nodes we've
// asked to be connected to, has actually been connected.
// if such miracle has taken place, send the stuff and remove
// node from list-of-nodes-to-ask (iNodeCandidatesToTryPush)
// 
void PublishingEngine::checkForSuccessfullyConnectedNodes() {
  iModel.lock() ;
  for ( int i = iNodeCandidatesToTryPush.size()-1 ; 
	( i >=0 ) && ( iWorkItem.iObjectHash != KNullHash ) ; 
	i-- ) {
    if ( iNodesSuccessfullyConnected.contains(iNodeCandidatesToTryPush[i]->nodeFingerPrint()) ) {
      // if we came here it means that we've asked for connection to
      // particular node and it happened
      LOG_STR2("PubEng: After asking, node has been connected: %s", qPrintable(iNodeCandidatesToTryPush[i]->nodeFingerPrint().toString())) ; 
      publishToNode(iNodeCandidatesToTryPush[i]->nodeFingerPrint()) ; 
    }
  }
  // can be emptied every time ; if there were anything important, it 
  // has been processed above
  iNodesSuccessfullyConnected.clear() ; 
  iModel.unlock() ;
}

//
// this is for pruning from ask-list those nodes that we've tried
// to connect with no success
//
void PublishingEngine::checkForUnSuccessfullyConnectedNodes() {
  iModel.lock() ;
  for ( int i = iNodeCandidatesToTryPush.size()-1 ; 
	( i >=0 ) && ( iWorkItem.iObjectHash != KNullHash ) ; 
	i-- ) {
    if ( iNodesFailurefullyConnected.contains(iNodeCandidatesToTryPush[i]->nodeFingerPrint()) ) {
      // if we came here it means that we've asked for connection to
      // particular node and it did not happen
      LOG_STR2("PubEng: After asking, node has not been connected: %s", qPrintable(iNodeCandidatesToTryPush[i]->nodeFingerPrint().toString())) ; 
      Node* n = iNodeCandidatesToTryPush.takeAt(i) ; 
      delete n ; 
    }
  }
  // can be emptied every time ; if there were anything important, it 
  // has been processed above
  iNodesFailurefullyConnected.clear() ; 
  iModel.unlock() ;
}
