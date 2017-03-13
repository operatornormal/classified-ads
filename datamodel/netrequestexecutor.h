/*    -*-C++-*- -*-coding: utf-8-unix;-*-
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

#ifndef CLASSIFIED_NETREQEXEC_H
#define CLASSIFIED_NETREQEXEC_H

#include <QTimer>
#include "../net/protocol.h"
#include <time.h> // for time_t
#include "../net/connection.h"

class MController ;
class Model ;
extern Hash KNullHash ;

/**
 * @brief Logic for handling tasks received from other peers or from user.
 *
 * Here is class that takes requests sent (or generated locally)
 * from queue, obtains data needed and puts results into
 * send-queues of nodes that need to see the results.
 */
class NetworkRequestExecutor : public QTimer {
    Q_OBJECT
public:
    /**
     * requests may be new, being processed, or ready
     * to be sent, have enum indicating the state
     */
    enum NetworkRequestState {
        NewRequest, /**< NetworkRequestExecutor has not yet touched */
        Processing, /**< is working in this very moment */
        NodeIsInWishList, /**< node connection in progress */
        RequestBeingSentAround, /**< request is sent to nodes around hash */
        ReadyToSend /**< request completed but not sent */
    };
    /**
     * @brief Work queue item.
     *
     * Have this kind of items in the queue waiting to be processed
     * and finally sent to destination (if still connected)
     */
    struct NetworkRequestQueueItem {
        /**
         * this iRequestType contais dual-use values.
         * if request type is RequestFor..something
         * and the iDestinationNode is KNullHash then
         * it means that it the user that is requesting
         * the content so this request is sent to
         * neighboring nodes.
         *
         * other possibility is that we receive RequestFor..something
         * from neighboring node, in which case we have the
         * iDestinationNode set, pointing back to node that
         * wants the content. In this case we try fetch
         * the content from local reposity and send it to
         * given node.
         */
        ProtocolItemType iRequestType ;
        Hash iDestinationNode ;
        Hash iRequestedItem ;
        quint32 iTimeStampOfItem ;
        quint32 iTimeStampOfLastActivity ;
        NetworkRequestState iState ;
        int iMaxNumberOfItems ;
        /** bang-path is used in publish-operations */
        QList<quint32> iBangPath ;
        /**
         * this is initially empty. when iState is set to
         * RequestBeingSentAround then this list here gets
         * populated with the nodes around
         */
        QList<Hash> iWishListForNodesAround ;
    } ;
    /**
     * constructor
     */
    NetworkRequestExecutor(MController *aController,
                           Model& aModel) ;
    ~NetworkRequestExecutor() ;
private:
    /**
     * method for sending one or more node references to
     * given node
     */
    void processNodeGreeting(NetworkRequestQueueItem& aEntry) ;
    /**
     * method for sending nodegreeting around given hash. user initiates
     * this and this sends around queries about nodes around
     * given hash. the nodes that are sent the query are already-connected
     * nodes
     */
    void processRequestForNodesAroundHash(NetworkRequestQueueItem& entry) ;
    /**
     * method for publishing a profile+ad+binary file+profile comment +
     * db record
     */
    void processRequestForContentPublish(NetworkRequestQueueItem& entry) ;
    /**
     * method for publishing a private message
     */
    void processRequestForPrivateMessagePublish(NetworkRequestQueueItem& aEntry) ;
    /**
     * method for requesting retrieval of a binary file
     */
    void processRequestForBinaryBlob(NetworkRequestQueueItem& aEntry) ;
    /**
     * method for requesting retrieval of a binary file
     */
    void processRequestForClassifiedAd(NetworkRequestQueueItem& aEntry) ;
    /**
     * method for requesting update of profile data and comments
     */
    void processRequestForProfilePoll(NetworkRequestQueueItem& aEntry) ;
    /**
     * method for requesting retrieval of a operator profile. so this
     * means that some neighboring node sent us a request regarding
     * a particular profile and this method then finds it from our
     * local data store, or does not.
     */
    void processRequestForUserProfile(NetworkRequestQueueItem& aEntry) ;
    /**
     * method for requesting retrieval of a comment of operator profile.
     * this is called in situation where user wants to locally view
     * a comment and it is not in local storage so we need to ask
     * other nodes to send it to us.
     */
    void processRequestForUserProfileComment(NetworkRequestQueueItem& aEntry) ;
    /**
     * method for actually sending the bytes to peer
     * @param aEntry is the request to send
     * @param aNodeToSend is fingerprint of the node where to
     *        send ; if KNullHash then aEntry.iDestinationNode
     *        is used. If aNodeSend is not connected at the moment,
     *        request is simply ignored.
     */
    void doSendRequestToNode(NetworkRequestQueueItem& aEntry,
                             const Hash& aNodeToSend = KNullHash ) ;
    /**
     * method for checking nodes around hash, sending request to
     * those and if not already connected, adding those nodes to
     * wishlist
     */
    void sendRequestToNodesAroundHash(NetworkRequestQueueItem& aEntry,
                                      bool aUseContentHashNotDestination = false) ;

    /**
     * method for producing reply to request concerning a binary blob ->
     * e.g. this method sends a binary blob
     */
    void processBinaryBlob(NetworkRequestQueueItem& aEntry) ;
    /**
     * method for producing reply to request concerning a private msg ->
     * e.g. this method sends a private message
     */
    void processPrivateMessage(NetworkRequestQueueItem& aEntry) ;
    /**
     * method for producing reply to request concerning profile comments
     * e.g. this method sends a profile comments whose commented
     * profile matches the one given in the request
     */
    void  processUserProfileCommentsForProfile(NetworkRequestQueueItem& entry) ;
    /**
     * method for producing bytearray to send to other node containing
     * single profile comment
     */
    void processUserProfileComment(NetworkRequestQueueItem& aEntry) ;
    /**
     * method for producing send queue items for every private
     * message that the requesting profile has in queue.
     */
    void processPrivateMessagesForProfile(NetworkRequestQueueItem& aEntry) ;
    /**
     * method for producing reply to request concerning a user profile ->
     * e.g. this method sends a profile to remote node
     */
    void processUserProfile(NetworkRequestQueueItem& aEntry) ;

    /**
     * method for producing reply to request concerning a classified ad ->
     * e.g. this method sends an ad to remote node
     */
    void processClassifiedAd(NetworkRequestQueueItem& aEntry) ;

    /**
     * method for producing reply to request concerning a classified ad
     * classification ->
     * e.g. this method sends an listing of ads to remote node, the
     * classification of the ads match hash given in entry
     */
    void processAdsClassified(NetworkRequestQueueItem& aEntry) ;

public slots:
    /**
     * this class is no thread but lets try pretending..
     */
    void run() ;
    /** when connection is attempted, @ref NetworkListener will
     * emit the status (failed or success) of the connection,
     * emitted signal is connected here
     */
    void nodeConnectionAttemptStatus(Connection::ConnectionState aStatus,
                                     const Hash aHashOfAttemptedNode );
private:
    MController *iController  ;
    Model& iModel ;
    bool iNowRunning ;
    time_t iLastTimeOfNodeConnectedNodeStatusUpdate ;
} ;
#endif
