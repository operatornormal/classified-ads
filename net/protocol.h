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

#ifndef CA_PROTOCOL_H
#define CA_PROTOCOL_H
#include "../util/hash.h" // for Hash

/**
 * @file protocol.h
 * this file protocol.h defines constants that are passed
 * between peers as protocol entries.
 *
 * Possible messages are
 * - Node greeting in the beginning. There node may advertise
 *   other available addresses and date when last in contact
 *   with this node or peers in the vicinity. This should
 *   be sent first by both parties.
 * - Node listing. Basically this is just up-to-date list of
 *   known peers, in immediate vicinity and then around
 *   the ring ; each node should try to produce list
 *   of high-quality nodes for this list.
 */

// node greeting has following fields, each is unsigned
// 8-bit if not otherwise stated, 32-bit integers are
// in network byte order:
// 1. protocol version for this node. Note that identifies
//    both this message and the protocol version. Later we
//    may invent protocol version xyz that will then
//    identify node and its protocol with some other
//    number.
static const unsigned char KProtocolVersion (1) ; /**< node greeting msg identifier */
// 2. number of bytes that follows for the whole message
// 3. node greeting as qCompress()ed JSon
//
// ok, next thing to define is our easiest message type, random numbers.
// idea for this is this: upon connect node sends node greeting.
// node greeting contains hash and outside ip addr. suppose someone
// is eavesdropping the tsl tcp connection she already has some known
// plaintext in there, in the very first real datapacket transmitted,
// in known offset so .. say, the original tsl key was done using bad
// random number and in addition the listener knows some plaintext
// inside it might be possible to brute-force the key.
//
// lets make her job a bit more interesting by first transmitting
// short random sequence of random length in the beginning.
static const unsigned char KRandomNumbersPacket (100) ; /**< rnd number msg identifier */
// rest of the packet is msg len and then the bytes that are supposed
// to be discarded (or written into your weekly lottery ticket
// if you consider yourself a lucky jerk :)

// then have packet for requesting nodes that might be held liable
// for keeping alive content with given hash
// the packet consists of the identifier and a 160 bit hash. nothing else.

// following items are for requesting various things from peer:
static const unsigned char KNodesAroundHash (101) ; /**< Message id for requesting nodes around hash */
static const unsigned char KProfileAtHash (103) ; /**< Message id for a profile request */
static const unsigned char KBinaryFileAtHash (104) ; /**< Message id for a binary file request */
static const unsigned char KClassifiedAdAtHash (109) ; /**< Message id for CA request */

static const unsigned char KProfilePublish (102) ; /**< Profile is published with this number */
static const unsigned char KBinaryFilePublish (105) ; /**< Binary blob is published with this number */
static const unsigned char KBinaryFileSend (106) ; /**< Binary blob is sent (compare to publish) with this number */
static const unsigned char KClassifiedAdPublish (107) ; /**< Classified ad is published with this number */
static const unsigned char KProfileSend (108) ; /**< Operator profile is sent (compare to publish) with this number */
static const unsigned char KClassifiedAdSend (110) ; /**< CA is sent (compare to publish) with this number */
static const unsigned char KAdsClassifiedAtHash (111) ; /**< Query about CAs whose classification matches the hash */
static const unsigned char KListOfAdsClassifiedAtHash (112) ; /**< Reply to Query about CAs whose classification matches the hash */
static const unsigned char KPrivMessagePublish (113) ; /**< user is publishing a new private message */
static const unsigned char KPrivMessageSend (114) ; /**< node is sending a private message */
static const unsigned char KPrivMessagesAtHash (115) ; /**< Request for private message having given hash */
static const unsigned char KPrivMessagesForProfile (116) ; /**< Request for private message destined so given profile */
static const unsigned char KProfileCommentPublish (117) ; /**< user is publishing a new comment to profile */
static const unsigned char KProfileCommentSend (118) ; /**< node is sending a profile comment */
/**
 * sent as a request to send profile comments. This is not about
 * sending individual comment but instead comments of a profile
 * there is separate request for single comment
 */
static const unsigned char KProfileCommentAtHash (119) ;
/**
 * sent as a request to send single profile comment.
 */
static const unsigned char KSingleProfileCommentAtHash (120) ;
/**
 * sent as a request to perform search
 */
static const unsigned char KSearchRequest (121) ;
/**
 * sent as a response to KSearchRequest (if there were any results)
 */
static const unsigned char KSearchResults (122) ;
/**
 * send as request to open audio channel to operators node
 */
static const unsigned char KVoiceCallStart (123) ;
/**
 * send as indication about closing of audio channel to operators node
 */
static const unsigned char KVoiceCallEnd (124) ;
/**
 * This is used to send rt-data inside ssl socket. Typically RT data 
 * would belong to UDP packet but sometimes UDP won't work while
 * TCP does. Use-case for this is voice call audio packets but there
 * may be other types also ; the data anyway contains a sub-type,
 * for instance for sending separate audio, chat messages or even
 * moving pictures. 
 */
static const unsigned char KRealtimeData (125) ;
/**
 * Protocol constants for future use
 */
static const unsigned char KFutureUse4 (126) ;
static const unsigned char KFutureUse5 (127) ;
static const unsigned char KFutureUse6 (128) ;


/**
 * next thing is rt-data subtypes. First audio. 
 */
static const unsigned char KRTDataAudioSubtype (1) ;
/**
 * Call control real-time data packet subtype
 */
static const unsigned char KRTDataControlSubtype (2) ;

/**
 * this enum lists possible items that we send over socket
 * from one node to another. These get serialized into bitstream
 * whose values are #defined above.
 */
enum ProtocolItemType {
    OwnNodeGreeting=1, /**< normal node greeting but inside program special handling */
    NodeGreeting=2, /**< node connectivity details */
    ClassifiedAd=3, /**< normal public posting */
    PrivateMessage=4, /**< a message destined to owner of specific profile */
    BinaryBlob=5, /**< just data, relates to something, has SHA1 */
    UserProfile=6, /**< user RSA key and possible other related data */
    RequestForClassifiedAd=7,/**< query for other nodes if there is anything about classified ads */
    RequestForPrivateMessage=8,/**< query for other nodes if there is any private msgs */
    RequestForBinaryBlob=9,/**< query for other nodes if there is specific binary avail */
    RequestForUserProfile=10,/**< query for other nodes if user has been in da hood */
    RandomNumbers=11, /**< explained later. this really contains 1 or more random numbers */
    RequestForNodesAroundHash=12, /**< request that other nodes use to retrieve node-refs around given hash */
    UserProfilePublish=13, /**< publish message is different from normal UserProfile */
    BinaryFilePublish=14, /**< publish binary blob is different from normal BinaryBlob */
    ClassifiedAdPublish=15, /**< publish CA is different from normal ad */
    ClassifiedAd2NdAddr=16, /**< publish of CA to group controller */
    RequestAdsClassified=17,/**< query for articles whose classification is known */
    PrivateMessagePublish=18,/**< local user is sending a privmsg */
    PrivateMessagesForProfile=19, /**< specifies a request to send messages destined to profile */
    UserProfileComment=20, /**< invididual profile comment to be sent */
    ProfileCommentPublish=21, /**< profile comment publish item type */
    UserProfileCommentsForProfile=22, /**< request to queue profile comments commenting given profile */
    RequestForProfilePoll=23, /**< UI request regarding profile update poll */
    RequestForProfileComment=24, /**< UI request regarding individual comment */
    RequestForSearchSend=25, /**< UI request about network search */
    RequestForVoiceCallStart=26,/**< node<->node audio control start */
    RequestForVoiceCallEnd=27 /**< node<->node audio control stop */
};
/**
 * @brief send-queue item.
 *
 * For each connected node, there is list of items to send,
 * entry in that list looks like this
 */
struct SendQueueItem {
    ProtocolItemType iItemType ; /**< what kind data sits in queue */
    Hash iHash ; /**< and its id, if applicable */
} ;
/**
 * @brief Carrier for keeping state of item about to be published.
 *
 * This struct carries data needed to publish-operation. This
 * is just collection of identifier of the actual object
 * and brief list of nodes where it has been already pushed to
 */
struct PublishItem {
    ProtocolItemType iObjectType ; /**< profile,classified ad,priv-msg etc*/
    Hash iObjectHash ; /**< Identifier of the data itself */
    /** low order hash bits of nodes that already have the content */
    QList<quint32> iAlreadyPushedHosts ;
    Hash i2NdAddr ; /**< CAs and comments get published twice, here is the 2nd addr */
} ;

/**
 * How many open connections we wish to have open at same time
 */
static const int KMaxOpenConnections( 100 );
/**
 * how many noderefs to send automatically after peer has
 * connected. Noderef with ipv4+ipv6 as compressed json is ~100bytes so 300
 * mean some 30 kilobytes -> can handle.
 */
static const int KNumberOfNodesToSendToEachPeer ( 300 ) ;
/**
 * how many classified ads to send to remote peer without
 * the peer asking.
 */
static const int KNumberOfClassifiedAdsToSendToEachPeer ( 300 ) ;
/**
 * max size of single published item
 */
static const quint32 KMaxProtocolItemSize ( 1024*1024*2 ) ;

/**
 * node may advertise itself in local ethernet segment with broadcast.
 * the port where ads are sent is this
 */
static const quint16 KBroadCastAdvertismentPort ( 23432 ) ;
#endif /* CA_PROTOCOL_H */
