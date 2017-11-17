/*    -*-C++-*- -*-coding: utf-8-unix;-*-
  Classified Ads is Copyright (c) Antti Järvinen 2013-2016.

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
#include "../mcontroller.h"
#include "netrequestexecutor.h"
#include <QSqlDatabase>
#include "../log.h"
#include "model.h"
#include "../net/protocol_message_formatter.h"
#include <QSqlQuery>
#include <QDateTime>
#include "profilemodel.h"
#include "profile.h"
#include "contentencryptionmodel.h"
#include "binaryfilemodel.h"
#include "camodel.h"
#include "privmsgmodel.h"
#include "profilecommentmodel.h"
#include "cadbrecordmodel.h"
#include "cadbrecord.h"
#include "../net/protocol.h"
#include <QThread>

NetworkRequestExecutor::NetworkRequestExecutor(MController *aController,
        Model& aModel) :
    QTimer(aController),
    iController(aController),
    iModel(aModel),
    iNowRunning(false),
    iLastTimeOfNodeConnectedNodeStatusUpdate(QDateTime::currentDateTimeUtc().toTime_t()) {
    connect(this, SIGNAL(timeout()), this, SLOT(run()));
}

NetworkRequestExecutor::~NetworkRequestExecutor() {
}

void NetworkRequestExecutor::run() {
    if ( iNowRunning ) return ;
    iNowRunning = true ;

    iModel.lock() ;
    QList <NetworkRequestExecutor::NetworkRequestQueueItem>& requests ( iModel.getNetRequests() ) ;
    for ( int i = requests.size()-1 ; i >= 0 ; i-- ) {
        NetworkRequestQueueItem& entry ( requests[i] ) ;
        if ( entry.iState == NewRequest ) {
            // yes, process
            entry.iState = Processing ;
            switch ( entry.iRequestType ) {
            case NodeGreeting:
                LOG_STR("Network request type NodeGreeting") ;
                processNodeGreeting(entry) ;
                break ;
            case RequestForNodesAroundHash:
                LOG_STR("Network request type RequestForNodesAroundHash") ;
                processRequestForNodesAroundHash(entry) ;
                break ;
            case RequestForBinaryBlob:
                LOG_STR("Network request type RequestForBinaryBlob") ;
                processRequestForBinaryBlob(entry) ;
                break ;
            case RequestForUserProfile:
                LOG_STR("Network request type RequestForUserProfile") ;
                processRequestForUserProfile(entry) ;
                break ;
            case RequestForProfileComment:
                LOG_STR("Network request type RequestForProfileComment") ;
                processRequestForUserProfileComment(entry) ;
                break ;
            case RequestForClassifiedAd:
                LOG_STR("Network request type RequestForClassifiedAd") ;
                processRequestForClassifiedAd(entry) ;
                break ;
            case RequestForProfilePoll:
                LOG_STR("Network request type RequestForProfilePoll") ;
                processRequestForProfilePoll(entry) ;
                break ;
            case UserProfilePublish:
                LOG_STR("Network request type UserProfilePublish") ;
                processRequestForContentPublish(entry) ;
                break ;
            case BinaryFilePublish:
                LOG_STR("Network request type BinaryFilePublish") ;
                processRequestForContentPublish(entry) ;
                break ;
            case ProfileCommentPublish:
                LOG_STR("Network request type ProfileCommentPublish") ;
                processRequestForContentPublish(entry) ;
                break ;
            case ClassifiedAdPublish:
                LOG_STR("Network request type ClassifiedAdPublish") ;
                processRequestForContentPublish(entry) ;
                break ;
            case PrivateMessagePublish:
                LOG_STR("Network request type PrivateMessagePublish") ;
                processRequestForPrivateMessagePublish(entry) ;
                break ;
            case BinaryBlob: // in practice, some other node has sent us query
                LOG_STR("Network request type BinaryBlob") ;
                processBinaryBlob(entry) ;
                break ;
            case PrivateMessage: // in practice, some other node has sent us query
                LOG_STR("Network request type PrivateMessage") ;
                processPrivateMessage(entry) ;
                break ;
            case UserProfileCommentsForProfile: // in practice, some other node has sent us query
                LOG_STR("Network request type UserProfileCommentsForProfile") ;
                processUserProfileCommentsForProfile(entry) ;
                break ;
            case UserProfileComment:
                LOG_STR("Network request type UserProfileComment") ;
                processUserProfileComment(entry) ;
                break ;
            case 	PrivateMessagesForProfile:
                LOG_STR("Network request type PrivateMessagesForProfile") ;
                processPrivateMessagesForProfile(entry) ;
                break ;
            case UserProfile: // in practice, some other node has sent us query
                LOG_STR("Network request type UserProfile") ;
                processUserProfile(entry) ;
                break ;
            case ClassifiedAd: // in practice, some other node has sent us query
                LOG_STR("Network request type ClassifiedAd") ;
                processClassifiedAd(entry) ;
                break ;
            case RequestAdsClassified: // some other node has sent us query about CAs by classification
                LOG_STR("Network request type RequestAdsClassified") ;
                processAdsClassified(entry) ;
                break ;
            case DbRecordPublish: // new db record added and will be published
                LOG_STR("Network request type DbRecordPublish") ;
                processRequestForContentPublish(entry) ; 
                break ;
            default:
                LOG_STR2("Unhandled Network request type %d", entry.iRequestType) ;
                entry.iState = ReadyToSend ;
                break;
            }
        }
        // remove entries that are done, or more than 10 minutes old
        if ( entry.iState == ReadyToSend ||
                ( entry.iTimeStampOfLastActivity + 10*60 ) < QDateTime::currentDateTimeUtc().toTime_t() ) {
            requests.removeAt(i) ;
            LOG_STR2("removing completed/old network req at index %d", i) ;
        }
        iModel.unlock() ;
        QThread::yieldCurrentThread ();
        iModel.lock() ;  // if there is a lot of requests in the queue, give other threads
        // also a chance
    }

    // then do a peridical check ; if enough time has passed since
    // last update of statuses of connected peers, update them:
    if ( QDateTime::currentDateTimeUtc().toTime_t() > (iLastTimeOfNodeConnectedNodeStatusUpdate+300)) {
        // every 300 seconds
        iLastTimeOfNodeConnectedNodeStatusUpdate = QDateTime::currentDateTimeUtc().toTime_t() ;

        Node* nodeOfConnection = NULL ;
        // problem here .. the node returned by connection may be
        // incomplete ; it is not based on node greeting of the
        // node itself, it actually contains only hash and
        // ip addr..
        QSqlQuery query;
        // sqlite documentation says that while single sql statement is
        // fast, transaction is slow. because here we after some time
        // running always make both insert and delete, lets have then
        // inside transaction so they hopefully will be handled
        // with single commit then..?
        query.exec("begin transaction ;") ;
        for ( int i = iModel.getConnections().size()-1 ; i >= 0 ; i-- ) {
            nodeOfConnection = iModel.getConnections().value(i)->node() ;
            if ( nodeOfConnection ) {
                nodeOfConnection->setLastConnectTime(QDateTime::currentDateTimeUtc().toTime_t()) ;
                iModel.nodeModel().updateNodeLastConnectTimeInDb(*nodeOfConnection) ;
            }
        }
        query.exec("commit;") ;  // transaction
    }

    // that's all folks
    iModel.unlock() ;
    iNowRunning = false ;
}

void NetworkRequestExecutor::processRequestForNodesAroundHash(NetworkRequestQueueItem& aEntry) {

    const QList <Connection *>& currentlyOpenConnections ( iModel.getConnections() ) ;
    for ( int i = 0 ;
            i < currentlyOpenConnections.size() &&
            i < aEntry.iMaxNumberOfItems ;
            i++ ) {
        const Node* conn_n = currentlyOpenConnections[i]->node() ;
        if ( conn_n && conn_n->nodeFingerPrint() != KNullHash ) {
            QByteArray* resultBytes = new QByteArray(ProtocolMessageFormatter::requestForNodesAroundHash(aEntry.iRequestedItem)) ;
            iModel.addItemToSend(conn_n->nodeFingerPrint(),
                                 resultBytes) ;
        }
    }
    aEntry.iState = ReadyToSend ;
}

void NetworkRequestExecutor::processRequestForContentPublish(NetworkRequestQueueItem& aEntry) {
    LOG_STR("NetworkRequestExecutor::processRequestForContentPublish in") ;
    doSendRequestToNode(aEntry) ;
    aEntry.iState = ReadyToSend ;
}

void NetworkRequestExecutor::processRequestForPrivateMessagePublish(NetworkRequestQueueItem& aEntry) {
    LOG_STR("NetworkRequestExecutor::processRequestForPrivateMessagePublish in") ;
    if ( aEntry.iDestinationNode != KNullHash &&
            iModel.nodeModel().isNodeAlreadyConnected(aEntry.iDestinationNode) ) {
        doSendRequestToNode(aEntry) ;
        aEntry.iState = ReadyToSend ;
    } else {
        if ( aEntry.iDestinationNode != KNullHash ) {
            if ( iModel.nodeModel().addNodeToConnectionWishList(aEntry.iDestinationNode) ) {
                aEntry.iTimeStampOfLastActivity = QDateTime::currentDateTimeUtc().toTime_t();
                aEntry.iState = NodeIsInWishList ;
            } else {
                LOG_STR("Failed to add node to connection wishlist??? when handling privmsg") ;
                aEntry.iState = ReadyToSend ;
            }
        } else {
            // destination node is not known..
            aEntry.iState = ReadyToSend ;
        }
    }
}


void NetworkRequestExecutor::processNodeGreeting
(NetworkRequestQueueItem& aEntry) {
    // here have possibility to have 2 meanings .. if iRequestedItem is
    // null, then just send some nodes. In practice send the hot nodes
    // or something.
    QList<Node* >* nodesToSend = NULL ;
    if ( KNullHash == aEntry.iRequestedItem ) {
        // hot nodes
        nodesToSend= iModel.nodeModel().getHotNodes(aEntry.iMaxNumberOfItems) ;
    } else {
        // nodes around iRequestedItem -> lets do so that when content
        // has some hash, the nodes that store the content are the nodes
        // whose hash is immediately after the hash of the content.
        //
        // this is important tidbit of information if you need to
        // pinpoint a node containing some content
        nodesToSend= iModel.nodeModel().getNodesAfterHash(aEntry.iRequestedItem,
                     aEntry.iMaxNumberOfItems) ;
    }
    if ( nodesToSend ) {
        if ( nodesToSend->count() > 0 ) {
            QByteArray* resultBytes = new QByteArray() ;
            if ( nodesToSend->size() ) {
                for ( int i = nodesToSend->size()-1 ; i>= 0 ; i-- ) {
                    Node* n = nodesToSend->value(i) ;
                    resultBytes->append(ProtocolMessageFormatter::nodeGreeting(*n));
                    delete n ;
                    nodesToSend->removeAt(i) ;
                }
            }
            if ( resultBytes->size() ) {
                // datamodel will delete resultBytes
                iModel.addItemToSend(aEntry.iDestinationNode,
                                     resultBytes) ;
            } else {
                // we obtained zero bytes -> delete empty bytearray
                delete resultBytes ;
            }
        }
        delete nodesToSend ;
    }
    aEntry.iState = ReadyToSend ;
}


void NetworkRequestExecutor::processRequestForBinaryBlob(NetworkRequestQueueItem& aEntry) {
    LOG_STR("NetworkRequestExecutor::processRequestForBinaryBlob in") ;
    // ok, we'll want to send selected neighboring nodes
    // a request regarding a hash.
    if ( aEntry.iDestinationNode != KNullHash ) {
        // check if the punk is lucky right away:
        if ( iModel.nodeModel().isNodeAlreadyConnected(aEntry.iDestinationNode) ) {
            doSendRequestToNode(aEntry) ;
            aEntry.iState = ReadyToSend ;
        } else {
            // add host to connection wishlist:
            if ( iModel.nodeModel().addNodeToConnectionWishList(aEntry.iDestinationNode) ) {
                aEntry.iTimeStampOfLastActivity = QDateTime::currentDateTimeUtc().toTime_t();
                aEntry.iState = NodeIsInWishList ;
            }
        }
    }
}

void NetworkRequestExecutor::processRequestForClassifiedAd(NetworkRequestQueueItem& aEntry) {
    LOG_STR("NetworkRequestExecutor::processRequestForClassifiedAd in") ;
    // neighboring node requested an classified ad
    if ( aEntry.iDestinationNode != KNullHash ) {
        // check if the punk is lucky right away:
        if ( iModel.nodeModel().isNodeAlreadyConnected(aEntry.iDestinationNode) ) {
            doSendRequestToNode(aEntry) ;
            aEntry.iState = ReadyToSend ;
        } else {
            // add host to connection wishlist:
            if ( iModel.nodeModel().addNodeToConnectionWishList(aEntry.iDestinationNode) ) {
                aEntry.iTimeStampOfLastActivity = QDateTime::currentDateTimeUtc().toTime_t();
                aEntry.iState = NodeIsInWishList ;
            }
        }
    }
}

void NetworkRequestExecutor::processRequestForProfilePoll(NetworkRequestQueueItem& aEntry) {
    // profile poll has 2 functions: there may be update to profile itself
    // or new comments posted. we send single request for both those things,
    // and the receiver of the poll-request will send updated profile
    // and possible comments.
    // if node of the profile is known, it is stored at aEntry.iDestinationNode and
    // naturally we send the poll there, as it is the best place for
    // possible profile update. in addition to that, we'll find the normal
    // publish-nodes for the profile and repeat the query there.
    aEntry.iTimeStampOfLastActivity = QDateTime::currentDateTimeUtc().toTime_t();
    // so, first possible destination-node
    if ( aEntry.iDestinationNode != KNullHash ) {
        if ( iModel.nodeModel().isNodeAlreadyConnected(aEntry.iDestinationNode) ) {
            doSendRequestToNode(aEntry,aEntry.iDestinationNode) ;
        } else {
            // add host to connection wishlist:
            if ( iModel.nodeModel().addNodeToConnectionWishList(aEntry.iDestinationNode) ) {
            }
        }
    }
    // then the normal publish-nodes
    sendRequestToNodesAroundHash( aEntry,
                                  true) ; // aUseContentHashNotDestination==true
    // above call will also set 	aEntry.iState so here we have no need ;
    // next state actually depends on that if all required nodes
    // were already connected..
}




// very similar to binary blob handling
void NetworkRequestExecutor::processRequestForUserProfile(NetworkRequestQueueItem& aEntry) {
    LOG_STR("NetworkRequestExecutor::processRequestForUserProfile in") ;

    if ( aEntry.iDestinationNode != KNullHash ) {
        // check if the punk is lucky right away:
        if ( iModel.nodeModel().isNodeAlreadyConnected(aEntry.iDestinationNode) ) {
            doSendRequestToNode(aEntry) ;
            aEntry.iState = ReadyToSend ;
        } else {
            // add host to connection wishlist:
            if ( iModel.nodeModel().addNodeToConnectionWishList(aEntry.iDestinationNode) ) {
                aEntry.iTimeStampOfLastActivity = QDateTime::currentDateTimeUtc().toTime_t();
                aEntry.iState = NodeIsInWishList ;
            }
        }
    }
}

void NetworkRequestExecutor::processRequestForUserProfileComment(NetworkRequestQueueItem& aEntry) {
    LOG_STR("NetworkRequestExecutor::processRequestForUserProfileComment in") ;
    if ( aEntry.iDestinationNode != KNullHash ) {
        // check if the punk is lucky right away:
        if ( iModel.nodeModel().isNodeAlreadyConnected(aEntry.iDestinationNode) ) {
            doSendRequestToNode(aEntry,aEntry.iDestinationNode) ;
            aEntry.iState = ReadyToSend ;
        } else {
            // add host to connection wishlist:
            if ( iModel.nodeModel().addNodeToConnectionWishList(aEntry.iDestinationNode) ) {
                aEntry.iTimeStampOfLastActivity = QDateTime::currentDateTimeUtc().toTime_t();
                aEntry.iState = NodeIsInWishList ;
            }
        }
    } else {
        sendRequestToNodesAroundHash(aEntry, true) ; // true == "use content hash"
    }
}

// in practice ; here we receive notifications about our wishlist-nodes
void NetworkRequestExecutor::nodeConnectionAttemptStatus(Connection::ConnectionState aStatus,
        const Hash aHashOfAttemptedNode ) {
    LOG_STR2("NetworkRequestExecutor::nodeConnectionAttemptStatus %d in", aStatus) ;
    LOG_STR2("NetworkRequestExecutor::nodeConnectionAttemptStatus %s ", qPrintable(aHashOfAttemptedNode.toString())) ;
    // use model to lock our own resources too..
    iModel.lock() ;

    QList <NetworkRequestExecutor::NetworkRequestQueueItem>& requests ( iModel.getNetRequests() ) ;
    for ( int i = requests.size()-1 ; i >= 0 ; i-- ) {
        NetworkRequestQueueItem& entry ( requests[i] ) ;
        if ( aStatus == Connection::Open &&
                entry.iDestinationNode == aHashOfAttemptedNode &&
                entry.iState == NodeIsInWishList  ) {
            // was our wishlist-item and connection was success
            doSendRequestToNode(entry) ;
            entry.iState = ReadyToSend ;
        } else if ( aStatus != Connection::Open &&
                    entry.iDestinationNode == aHashOfAttemptedNode &&
                    entry.iState == NodeIsInWishList  ) {
            // was our wishlist-item and could not make connection
            sendRequestToNodesAroundHash(entry) ;
        } else if ( aStatus == Connection::Open &&
                    entry.iWishListForNodesAround.contains( aHashOfAttemptedNode ) &&
                    entry.iState == RequestBeingSentAround  ) {
            // was our wishlist-item for nodes and connection was success
            doSendRequestToNode(entry,aHashOfAttemptedNode) ;
            entry.iWishListForNodesAround.removeOne(aHashOfAttemptedNode) ;
            if ( entry.iWishListForNodesAround.size() == 0 ) {
                entry.iState = ReadyToSend ;
            }
        } else if ( aStatus != Connection::Open &&
                    entry.iWishListForNodesAround.contains( aHashOfAttemptedNode ) &&
                    entry.iState == RequestBeingSentAround  ) {
            // was our wishlist-item for nodes and connection was not success
            entry.iWishListForNodesAround.removeOne(aHashOfAttemptedNode) ;
            if ( entry.iWishListForNodesAround.size() == 0 ) {
                entry.iState = ReadyToSend ;
            }
        }
    } // end of for requests..
    iModel.unlock() ;
}

void NetworkRequestExecutor::doSendRequestToNode(NetworkRequestQueueItem& aEntry,
        const Hash& aNodeToSend) {
    aEntry.iTimeStampOfLastActivity = QDateTime::currentDateTimeUtc().toTime_t();
    switch ( aEntry.iRequestType ) {
    case NodeGreeting: {
        LOG_STR("doSendRequestToNode type NodeGreeting") ;
        Node* n ( iModel.nodeModel().nodeByHash (      aEntry.iRequestedItem)) ;
        if ( n ) {
            QByteArray* resultBytes = new QByteArray() ;
            resultBytes->append(ProtocolMessageFormatter::nodeGreeting(*n)) ;
            iModel.addItemToSend(aNodeToSend == KNullHash ?
                                 aEntry.iDestinationNode :
                                 aNodeToSend ,
                                 resultBytes) ;
            delete n ;
        }
    }
    break ;
    case PrivateMessagePublish: {
        LOG_STR("doSendRequestToNode PrivateMessagePublish") ;
        QByteArray msg ;
        QByteArray msgSignature ;
        QByteArray* resultBytes = new QByteArray() ;
        quint32 timeOfPublish ;
        Hash resultingDestination ;
        Hash resultingRecipient ;
        if ( iModel.privateMessageModel().messageDataByFingerPrint(aEntry.iRequestedItem,
                msg,
                msgSignature,
                &resultingDestination,
                &resultingRecipient,

                &timeOfPublish) ) {
            // message was found, lets send:
            resultBytes->append(ProtocolMessageFormatter::privMsgPublish(aEntry.iRequestedItem,
                                msg,
                                msgSignature,
                                aEntry.iBangPath,
                                resultingDestination,
                                resultingRecipient,
                                timeOfPublish ));
            msg.clear() ;
            msgSignature.clear() ;
            if ( resultBytes->size() ) {
                // datamodel will delete resultBytes
                iModel.addItemToSend(aNodeToSend == KNullHash ?
                                     aEntry.iDestinationNode :
                                     aNodeToSend,
                                     resultBytes) ;
            } else {
                // we obtained zero bytes -> delete empty bytearray
                delete resultBytes ;
            }
        } else {
            aEntry.iState = ReadyToSend ;
            QLOG_STR("Could not find msg supposed to be published, id=" + aEntry.iRequestedItem.toString()) ;
            delete resultBytes ;
        }
    }
    break ;
    case RequestForBinaryBlob: {
        LOG_STR("doSendRequestToNode RequestForBinaryBlob") ;
        QByteArray* resultBytes = new QByteArray() ;
        resultBytes->append(ProtocolMessageFormatter::requestForBinaryBlob(aEntry.iRequestedItem)) ;
        iModel.addItemToSend(aNodeToSend == KNullHash ?
                             aEntry.iDestinationNode :
                             aNodeToSend ,
                             resultBytes) ;
    }
    break ;
    case RequestForClassifiedAd: {
        LOG_STR2("doSendRequestToNode RequestForClassifiedAd (hash high-bits %u)",aEntry.iRequestedItem.iHash160bits[0]) ;
        QByteArray* resultBytes = new QByteArray() ;
        resultBytes->append(ProtocolMessageFormatter::requestForClassifiedAd(aEntry.iRequestedItem)) ;
        iModel.addItemToSend(aNodeToSend == KNullHash ?
                             aEntry.iDestinationNode :
                             aNodeToSend ,
                             resultBytes) ;
    }
    break ;
    case RequestForUserProfile: {
        LOG_STR("doSendRequestToNode RequestForUserProfile") ;
        if ( ! ( aNodeToSend == KNullHash && aEntry.iDestinationNode == KNullHash ) ) {
            QByteArray* resultBytes = new QByteArray() ;
            resultBytes->append(ProtocolMessageFormatter::requestForUserProfile(aEntry.iRequestedItem)) ;
            iModel.addItemToSend(aNodeToSend == KNullHash ?
                                 aEntry.iDestinationNode :
                                 aNodeToSend ,
                                 resultBytes) ;
        }
    }
    break ;
    case RequestForProfileComment: {
        LOG_STR("doSendRequestToNode RequestForProfileComment") ;
        if ( ! ( aNodeToSend == KNullHash && aEntry.iDestinationNode == KNullHash ) ) {
            QByteArray* resultBytes = new QByteArray() ;
            resultBytes->append(ProtocolMessageFormatter::requestForUserProfileComment(aEntry.iRequestedItem)) ;
            iModel.addItemToSend(aNodeToSend == KNullHash ?
                                 aEntry.iDestinationNode :
                                 aNodeToSend ,
                                 resultBytes) ;
        }
    }
    break ;
    case ClassifiedAdPublish: {
        LOG_STR("doSendRequestToNode RequestForClassifiedAdPublish") ;
        QByteArray ca ;
        QByteArray caSignature ;
        QByteArray* resultBytes = new QByteArray() ;
        QByteArray caKey ;
        quint32 timeOfPublish ;

        if ( iModel.classifiedAdsModel().caDataForPublish(aEntry.iRequestedItem,
                ca,
                caSignature,
                caKey,
                &timeOfPublish) ) {
            resultBytes->append(ProtocolMessageFormatter::contentPublish(KClassifiedAdPublish,
                                aEntry.iRequestedItem,
                                ca,
                                caSignature,
                                aEntry.iBangPath,
                                caKey,
                                false,
                                false,
                                timeOfPublish ));
        }

        caKey.clear() ;
        ca.clear() ;
        caSignature.clear() ;
        if ( resultBytes->size() ) {
            // datamodel will delete resultBytes
            iModel.addItemToSend(aNodeToSend == KNullHash ?
                                 aEntry.iDestinationNode :
                                 aNodeToSend,
                                 resultBytes) ;
        } else {
            // we obtained zero bytes -> delete empty bytearray
            delete resultBytes ;
        }
    }
    break ;
    case UserProfilePublish: {
        LOG_STR("doSendRequestToNode UserProfilePublish") ;
        QByteArray profile ;
        QByteArray profileSignature ;
        QByteArray* resultBytes = new QByteArray() ;
        QByteArray profileKey ;
        bool isContentEncrypted = false ;
        quint32 timeOfPublish (0 ) ;
        if ( iModel.contentEncryptionModel().PublicKey(aEntry.iRequestedItem,
                profileKey ) ) {
            if ( iModel.profileModel().profileDataByFingerPrint(aEntry.iRequestedItem,
                    profile,
                    profileSignature,
                    &isContentEncrypted,
                    &timeOfPublish ) ) {
                resultBytes->append(ProtocolMessageFormatter::contentPublish(KProfilePublish,
                                    aEntry.iRequestedItem,
                                    profile,
                                    profileSignature,
                                    aEntry.iBangPath,
                                    profileKey,
                                    isContentEncrypted,
                                    false,
                                    timeOfPublish ));
            }
        }
        profileKey.clear() ;
        profile.clear() ;
        profileSignature.clear() ;
        if ( resultBytes->size() ) {
            // datamodel will delete resultBytes
            iModel.addItemToSend(aNodeToSend == KNullHash ?
                                 aEntry.iDestinationNode :
                                 aNodeToSend,
                                 resultBytes) ;
        } else {
            // we obtained zero bytes -> delete empty bytearray
            delete resultBytes ;
        }
    }
    break ;
    case BinaryFilePublish: {
        LOG_STR("doSendRequestToNode BinaryFilePublish") ;
        QByteArray binary ;
        QByteArray binarySignature ;
        QByteArray* resultBytes = new QByteArray() ;
        QByteArray binaryKey ;
        bool isContentEncrypted = false ;
        bool isContentCompressed = false ;
        quint32 timeOfPublish ( 0 ) ;
        if ( iModel.binaryFileModel().binaryFileDataForSend(aEntry.iRequestedItem,
                binary,
                binarySignature,
                binaryKey,
                &isContentEncrypted,
                &isContentCompressed,
                &timeOfPublish) ) {
            resultBytes->append(ProtocolMessageFormatter::contentPublish(KBinaryFilePublish,
                                aEntry.iRequestedItem,
                                binary,
                                binarySignature,
                                aEntry.iBangPath,
                                binaryKey,
                                isContentEncrypted,
                                isContentCompressed,
                                timeOfPublish ));
        }

        binaryKey.clear() ;
        binary.clear() ;
        binarySignature.clear() ;
        if ( resultBytes->size() ) {
            // datamodel will delete resultBytes
            iModel.addItemToSend(aNodeToSend == KNullHash ?
                                 aEntry.iDestinationNode :
                                 aNodeToSend,
                                 resultBytes) ;
        } else {
            // we obtained zero bytes -> delete empty bytearray
            delete resultBytes ;
        }
    }
    break ;
    case ProfileCommentPublish: {
        LOG_STR("doSendRequestToNode ProfileCommentPublish") ;
        QByteArray binary ;
        QByteArray binarySignature ;
        QByteArray* resultBytes = new QByteArray() ;
        quint32 flags ;
        quint32 timeOfPublish ( 0 ) ;
        Hash profileHash ;
        if ( iModel.profileCommentModel().commentDataByFingerPrint(aEntry.iRequestedItem,
                binary,
                binarySignature,
                &profileHash,
                &flags,
                &timeOfPublish) ) {
            resultBytes->append(ProtocolMessageFormatter::profileCommentPublish(aEntry.iRequestedItem,
                                binary,
                                binarySignature,
                                aEntry.iBangPath,
                                profileHash,
                                timeOfPublish,
                                flags ));
        }
        binary.clear() ;
        binarySignature.clear() ;
        if ( resultBytes->size() ) {
            // datamodel will delete resultBytes
            iModel.addItemToSend(aNodeToSend == KNullHash ?
                                 aEntry.iDestinationNode :
                                 aNodeToSend,
                                 resultBytes) ;
        } else {
            // we obtained zero bytes -> delete empty bytearray
            delete resultBytes ;
        }
    }
    break ;


    case DbRecordPublish: 
    {
        LOG_STR("doSendRequestToNode DbRecordPublish") ;
        QList<CaDbRecord *> dbRecordsToPublish ( 
            iModel
            .caDbRecordModel()
            ->searchRecords(KNullHash, 
                            aEntry.iRequestedItem,
                            0 , // modified after
                            std::numeric_limits<quint32>::max(), // modified before
                            std::numeric_limits<qint64>::min(), // search number min
                            std::numeric_limits<qint64>::max(), // search number max
                            QString::null, // searchphrase
                            KNullHash, // by sender
                            true) ) ; // is for publish, here "yes"
        foreach ( const CaDbRecord* r, dbRecordsToPublish ) {
            QByteArray* resultBytes = new QByteArray() ;
            resultBytes->append(
                ProtocolMessageFormatter::dbRecordPublish(
                    *r, aEntry.iBangPath ));
            if ( resultBytes->size() ) {
                // datamodel will delete resultBytes
                iModel.addItemToSend(aNodeToSend == KNullHash ?
                                     aEntry.iDestinationNode :
                                     aNodeToSend,
                                     resultBytes) ;
            } else {
                // we obtained zero bytes -> delete empty bytearray
                delete resultBytes ;
            }
        }
        // db record model obliges us to delete the resultset:
        while ( dbRecordsToPublish.isEmpty() == false ) {
            CaDbRecord *deletee = dbRecordsToPublish.takeFirst() ; 
            delete deletee ; 
        }
    }
    break ;

    case RequestAdsClassified: // we come here after nodes have been put to wishlist
        LOG_STR2("doSendRequestToNode RequestAdsClassified %d", aEntry.iRequestType) ;
        if ( aNodeToSend != KNullHash ) {
            QByteArray* resultBytes = new QByteArray() ;
            resultBytes->append(
                ProtocolMessageFormatter::requestForAdsClassified(
                    aEntry.iRequestedItem,
                    aEntry.iTimeStampOfItem,
                    aEntry.iTimeStampOfLastActivity ) ) ;
            if ( resultBytes->size() ) {
                // datamodel will delete resultBytes
                iModel.addItemToSend(aNodeToSend,
                                     resultBytes) ;
            } else {
                // we obtained zero bytes -> delete empty bytearray
                delete resultBytes ;
            }
        }
        break ;
    case RequestForProfilePoll:
        // this sends a poll-type requst to node, asking if there
        // has been any activity noticed regarding a profile.
        // in practice it would mean new profile publish or
        // addition of a new profile-comment.
        LOG_STR2("doSendRequestToNode RequestForProfilePoll %d", aEntry.iRequestType) ;
        if ( aNodeToSend != KNullHash ) {
            QByteArray* resultBytes = new QByteArray() ;
            // in case of RequestForProfilePoll the "iRequestedItem" always
            // carries the hash of the profile whose update is needed
            // while iDestinationNode may have hash of the node where
            // operator is publishing her secrets.
            //
            // message formatter calls this "request for profile comments"
            // but in practice nodes will respond also with
            // updated profile, if one is found lying around.
            resultBytes->append(ProtocolMessageFormatter::requestForProfileComments(aEntry.iRequestedItem,
                                aEntry.iTimeStampOfItem  ) ) ;
            if ( resultBytes->size() ) {
                // datamodel will delete resultBytes
                iModel.addItemToSend(aNodeToSend,
                                     resultBytes) ;
            } else {
                // we obtained zero bytes -> delete empty bytearray
                delete resultBytes ;
            }
        }
        break ;
    default:
        LOG_STR2("doSendRequestToNode unknown type %d", aEntry.iRequestType) ;
        break;
    }
}

void NetworkRequestExecutor::sendRequestToNodesAroundHash(NetworkRequestQueueItem& aEntry,
        bool aUseContentHashNotDestination) {
    // here you see important tidbit of information: if we wish to find
    // an item from DHT we use hash of the item. From the hash we produce
    // list of nodes that are supposed to carry that item. This list
    // is produced in publishing engine ( ../net/publishingengine.cpp method
    // ::run() ) where it right now says that take 20 nodes *after* the hash
    // in question. If we wish to find the same content again, we must
    // use the same formula here.
    QList<Node *>* nodesToTry =
        iModel.nodeModel().getNodesAfterHash(aUseContentHashNotDestination ? aEntry.iRequestedItem : aEntry.iDestinationNode,
                20, // 20 nodes
                300 ) ;// at most 5 hours old
    if ( nodesToTry == NULL || nodesToTry->size() == 0 ) {
        // try widening the requirements a bit:
        nodesToTry =
            iModel.nodeModel().getNodesAfterHash(aUseContentHashNotDestination ? aEntry.iRequestedItem : aEntry.iDestinationNode,
                    30, // 20 nodes
                    1440 ) ;// at most 24 hours old
        // uh, oh. do not try then. the request has failed.
        if ( nodesToTry == NULL || nodesToTry->size() == 0 ) {
            aEntry.iState = ReadyToSend ;
        }
    }
    if ( nodesToTry != NULL && nodesToTry->size() > 0 ) {
        while ( ! nodesToTry->isEmpty() ) {
            Node* connectCandidate ( nodesToTry->takeFirst() ) ;
            if ( iModel.nodeModel().isNodeAlreadyConnected(*connectCandidate) ) {
                doSendRequestToNode(aEntry, connectCandidate->nodeFingerPrint()) ;
            } else {
                // put the node into wishlist
                if ( !aEntry.iWishListForNodesAround.contains(connectCandidate->nodeFingerPrint()) ) {
                    if ( iModel.nodeModel().addNodeToConnectionWishList(connectCandidate->nodeFingerPrint()) ) {
                        aEntry.iTimeStampOfLastActivity = QDateTime::currentDateTimeUtc().toTime_t();
                        aEntry.iWishListForNodesAround.append(connectCandidate->nodeFingerPrint()) ;
                    }
                }
            }
            delete connectCandidate ;
        }
        // if we managed to put any nodes into wishlist,
        // then keep the entry, otherwise mark it as "readytosend"
        // e.g. finished from this classes perspective.
        if ( aEntry.iWishListForNodesAround.size() ) {
            aEntry.iState = RequestBeingSentAround ;
        } else {
            aEntry.iState = ReadyToSend ;
        }
    }
}


// another node requested a binary blob: here is served
void NetworkRequestExecutor::processBinaryBlob(NetworkRequestQueueItem& aEntry) {
    LOG_STR("processBinaryBlob in") ;
    {
        QByteArray binary ;
        QByteArray binarySignature ;
        QByteArray* resultBytes = new QByteArray() ;
        QByteArray binaryKey ;
        bool isContentEncrypted = false ;
        bool isContentCompressed = false ;
        quint32 timeWhenBinaryFileWasPublished ;

        if ( iModel.binaryFileModel().binaryFileDataForSend(aEntry.iRequestedItem,
                binary,
                binarySignature,
                binaryKey,
                &isContentEncrypted,
                &isContentCompressed,
                &timeWhenBinaryFileWasPublished) ) {
            resultBytes->append(ProtocolMessageFormatter::contentSend(KBinaryFileSend,
                                aEntry.iRequestedItem,
                                binary,
                                binarySignature,
                                binaryKey,
                                isContentEncrypted,
                                isContentCompressed,
                                timeWhenBinaryFileWasPublished ));
        }
        binaryKey.clear() ;
        binary.clear() ;
        binarySignature.clear() ;
        if ( resultBytes->size() ) {
            // datamodel will delete resultBytes
            iModel.addItemToSend(aEntry.iDestinationNode ,
                                 resultBytes) ;
        } else {
            // we obtained zero bytes -> delete empty bytearray
            delete resultBytes ;
        }
    }
    aEntry.iState = ReadyToSend ;
}

// another node requested a private message: here is served
void NetworkRequestExecutor::processPrivateMessage(NetworkRequestQueueItem& aEntry) {
    LOG_STR("processPrivateMessage in") ;

    QByteArray msg ;
    QByteArray msgSignature ;
    QByteArray* resultBytes = new QByteArray() ;
    quint32 timeOfPublish ;
    Hash resultingDestination ;
    Hash resultingRecipient ;
    if ( iModel.privateMessageModel().messageDataByFingerPrint(aEntry.iRequestedItem,
            msg,
            msgSignature,
            &resultingDestination,
            &resultingRecipient,
            &timeOfPublish) ) {
        // message was found, lets send:
        resultBytes->append(ProtocolMessageFormatter::privMsgSend(aEntry.iRequestedItem,
                            msg,
                            msgSignature,
                            resultingDestination,
                            resultingRecipient,
                            timeOfPublish ));
        msg.clear() ;
        msgSignature.clear() ;
        if ( resultBytes->size() ) {
            // datamodel will delete resultBytes
            iModel.addItemToSend(aEntry.iDestinationNode,
                                 resultBytes) ;
        } else {
            // we obtained zero bytes -> delete empty bytearray
            delete resultBytes ;
        }
    } else {
        QLOG_STR("Could not find msg supposed to be published, id=" + aEntry.iRequestedItem.toString()) ;
        delete resultBytes ;
    }
    aEntry.iState = ReadyToSend ;
}

void NetworkRequestExecutor::processUserProfileComment(NetworkRequestQueueItem& aEntry) {
    if ( aEntry.iDestinationNode != KNullHash ) {
        QByteArray binary ;
        QByteArray binarySignature ;
        QByteArray* resultBytes = new QByteArray() ;
        quint32 flags ;
        quint32 timeOfPublish ( 0 ) ;
        Hash profileHash ;

        if ( iModel.profileCommentModel().commentDataByFingerPrint(aEntry.iRequestedItem,
                binary,
                binarySignature,
                &profileHash,
                &flags,
                &timeOfPublish)) {
            resultBytes->append(ProtocolMessageFormatter::profileCommentSend(aEntry.iRequestedItem,
                                binary,
                                binarySignature,
                                profileHash,
                                timeOfPublish,
                                flags ));
        }
        binary.clear() ;
        binarySignature.clear() ;
        if ( resultBytes->size() ) {
            // datamodel will delete resultBytes
            iModel.addItemToSend(   aEntry.iDestinationNode , resultBytes) ;
        } else {
            // we obtained zero bytes -> delete empty bytearray
            delete resultBytes ;
        }
    }
    aEntry.iState = ReadyToSend ;
}

// another node requested a private message: here is served
void NetworkRequestExecutor::processPrivateMessagesForProfile(NetworkRequestQueueItem& aEntry) {
    LOG_STR("processPrivateMessagesForProfile in") ;

    // ok, what do we do here?
    // we were requested for all messages for profile .. we can't
    // send them at once, so lets make a network request queue item
    // of each message that we manage to find.

    QList<Hash> messagesToRequest ( iModel.privateMessageModel().messagesForProfile(aEntry.iRequestedItem,
                                    aEntry.iTimeStampOfItem)  ) ;
    foreach ( Hash articleHash, messagesToRequest ) {
        NetworkRequestQueueItem newRequest ;
        newRequest.iRequestType = PrivateMessage ; // single message here
        newRequest.iRequestedItem = articleHash ;
        newRequest.iDestinationNode = aEntry.iDestinationNode ;
        newRequest.iState = NewRequest ;
        newRequest.iMaxNumberOfItems = 1 ;
        iModel.addNetworkRequest(newRequest) ;
    }
    aEntry.iState = ReadyToSend ;
}

void NetworkRequestExecutor::processUserProfileCommentsForProfile(NetworkRequestQueueItem& entry) {
    LOG_STR("processUserProfileCommentsForProfile in") ;

    QList<Hash> messagesToRequest ( iModel.profileCommentModel().commentsForProfile(entry.iRequestedItem,
                                    entry.iTimeStampOfItem)  ) ;
    foreach ( Hash commentHash, messagesToRequest ) {
        NetworkRequestQueueItem newRequest ;
        newRequest.iRequestType = UserProfileComment ; // single comment here
        newRequest.iRequestedItem = commentHash ;
        newRequest.iDestinationNode = entry.iDestinationNode ;
        newRequest.iState = NewRequest ;
        newRequest.iMaxNumberOfItems = 1 ;
        iModel.addNetworkRequest(newRequest) ;
    }
    // in addition to comments, check if there as been update to the
    // profile itself:
    time_t lastUpdateTime = iModel.profileModel().getLastProfileUpdateTime(entry.iRequestedItem);
    if ( lastUpdateTime > (unsigned)(entry.iTimeStampOfItem) ) {
        // yes, the profile itself has been updated too
        NetworkRequestQueueItem profileRequest ;
        profileRequest.iRequestType = UserProfile ; // single comment here
        profileRequest.iRequestedItem = entry.iRequestedItem ;
        profileRequest.iDestinationNode = entry.iDestinationNode ;
        profileRequest.iState = NewRequest ;
        profileRequest.iMaxNumberOfItems = 1 ;
        iModel.addNetworkRequest(profileRequest) ;
    }
    entry.iState = ReadyToSend ;
}
void NetworkRequestExecutor::processUserProfile(NetworkRequestQueueItem& aEntry) {
    LOG_STR("processUserProfile in") ;

    QByteArray profile ;
    QByteArray profileSignature ;
    QByteArray* resultBytes = new QByteArray() ;
    QByteArray profileKey ;
    bool isContentEncrypted = false ;
    bool isContentCompressed = false ;
    quint32 timeWhenProfileWasPublished ;

    if ( iModel.profileModel().profileDataByFingerPrint(aEntry.iRequestedItem,
            profile,
            profileSignature,
            &isContentEncrypted,
            &timeWhenProfileWasPublished,
            &profileKey) ) {
        resultBytes->append(ProtocolMessageFormatter::contentSend(KProfileSend,
                            aEntry.iRequestedItem,
                            profile,
                            profileSignature,
                            profileKey,
                            isContentEncrypted,
                            isContentCompressed,
                            timeWhenProfileWasPublished ));
    }
    profileKey.clear() ;
    profile.clear() ;
    profileSignature.clear() ;
    if ( resultBytes->size() ) {
        // datamodel will delete resultBytes
        iModel.addItemToSend(aEntry.iDestinationNode ,
                             resultBytes) ;
    } else {
        // we obtained zero bytes -> delete empty bytearray
        delete resultBytes ;
    }
    aEntry.iState = ReadyToSend ;
}



void NetworkRequestExecutor::processClassifiedAd(NetworkRequestQueueItem& aEntry) {
    LOG_STR("processClassifiedAd in") ;

    QByteArray caBytes ;
    QByteArray caSignature ;
    QByteArray* resultBytes = new QByteArray() ;
    QByteArray caKey ;
    bool isContentEncrypted = false ;
    bool isContentCompressed = false ;
    quint32 timeWhenCaWasPublished ;

    if ( iModel.classifiedAdsModel().caDataForPublish(aEntry.iRequestedItem,
            caBytes,
            caSignature,
            caKey,
            &timeWhenCaWasPublished) ) {
        resultBytes->append(ProtocolMessageFormatter::contentSend(KClassifiedAdSend,
                            aEntry.iRequestedItem,
                            caBytes,
                            caSignature,
                            caKey,
                            isContentEncrypted,
                            isContentCompressed,
                            timeWhenCaWasPublished ));
    }
    caKey.clear() ;
    caBytes.clear() ;
    caSignature.clear() ;
    if ( resultBytes->size() ) {
        // datamodel will delete resultBytes
        iModel.addItemToSend(aEntry.iDestinationNode ,
                             resultBytes) ;
    } else {
        // we obtained zero bytes -> delete empty bytearray
        delete resultBytes ;
    }
    aEntry.iState = ReadyToSend ;
}

void NetworkRequestExecutor::processAdsClassified(NetworkRequestQueueItem& aEntry) {
    LOG_STR("processAdsClassified in") ;
    /*
     * ok, here make distinction. if aEntry.iDestinationNode is
     * nullhash, then it was UI interface request by local user.
     * if iDestinationNode is some other node, then the request
     * was sent by a peer
     */

    if ( aEntry.iDestinationNode != KNullHash ) {
        QList<QPair<Hash,quint32> > listOfAds ;
        // in request with type RequestAdsClassified the field
        // iTimeStampOfItem contains requested start time and
        // iTimeStampOfLastActivity the end time, not actually
        // the last activity time of the entry itself.
        // this can be done because
        // - it causes confusion to reader
        // - this network request is one-shot type, there is no long life-cycle
        iModel.classifiedAdsModel().caListingByClassification(aEntry.iRequestedItem,
                aEntry.iTimeStampOfItem,
                aEntry.iTimeStampOfLastActivity,
                listOfAds,
                aEntry.iDestinationNode) ;
        LOG_STR2("processAdsClassified finds %d ads in response", (int)(listOfAds.size())) ;
        if ( listOfAds.size() > 0 ) {
            QByteArray* resultBytes = new QByteArray() ;
            resultBytes->append(ProtocolMessageFormatter::replyToAdsClassified(aEntry.iRequestedItem,
                                listOfAds								       ) ) ;
            iModel.addItemToSend(aEntry.iDestinationNode ,
                                 resultBytes) ;
        }
        aEntry.iState = ReadyToSend ;
    } else {
        // was UI request
        sendRequestToNodesAroundHash(aEntry, true) ;
    }
}
