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



#ifndef PROTOCOL_MESSAGE_PARSER_H
#define PROTOCOL_MESSAGE_PARSER_H
#include "protocol.h"
#include <QObject>
#include <QHostAddress> // Q_IPV6ADDR
#include "node.h" // KNullIpv6Addr
#include "connection.h"
class MController ;
class MModelProtocolInterface ;
class MNodeModelProtocolInterface ;
class Connection ;
/**
 * @brief Class containing routines for parsing protocol messages.
 * Methods are mainly called from network connection instances
 * and this class will forward the parsed protocol entities
 * to datamodel
 */
class ProtocolMessageParser : public QObject {
    Q_OBJECT
public:
    ProtocolMessageParser (MController &aController,
                           MModelProtocolInterface &aModel) ;

    /**
     * the open mouth
     * @param aGoodFood is the message
     * @return false if such a fatal error that peer needs to be disconnected
     */
    bool parseMessage(const QByteArray& aGoodFood,
                      Connection& aConnection ) ;
private: // private methods for doing the actual work
    /** Peer hello message  */
    bool parseNodeGreetingV1(const QByteArray& aSingleNodeGreeting,
                             const Connection &aConnection ) ;
    /**
     * Peer number of hello messages
     * @param aGoodFood at least one node greeting
     * @param aConnection the orignating connection
     */
    bool parseMultipleNodeGreetingsV1(const QByteArray& aGoodFood,
                                      const Connection& aConnection ) ;
    /**
     * utility method: extract integer from bytearray. integer
     * will be returned in host byte order e.g.
     * @param aGoodFood is the bytearray from which to extract
     * @param aPos is the position where integer is expected to
     *        be found
     * @param aResult is the quint32 where result will be written
     *        e.g. this is the output variable
     * @return true if reading succeeded
     */
    bool uintFromPosition(const QByteArray& aGoodFood,
                          const int aPos,
                          quint32* aResult) const ;
    /**
     * utility method: extract hash from bytearray.
     * @param aGoodFood is the bytearray from which to extract
     * @param aPos is the position where integer is expected to
     *        be found
     * @param aResultHash is the hash that will have its
     *                    value set from contents of aGoodFood
     * @return true if success
     */
    bool hashFromPosition(const QByteArray& aGoodFood,
                          const int aPos,
                          Hash* aResultHash) const ;

    /**
     * Method for parsing request to send one or more objects
     * specified by a hash.
     */
    bool parseRequestForObjectsAroundHash(const QByteArray& aNodeRefRequest,
                                          const Connection &aConnection ) ;
    /**
     * Method for parsing published content. Content-type is given
     * as parameter ; single method as the structure is anyway same,
     * handling is different..
     *
     * This method is also used to parse content sent ; it differs from
     * published content so, that it has no bangpath but otherwise
     * the format is the same.
     */
    bool parseContentPublishedOrSent(const unsigned char aProtocolItemType,
                                     const QByteArray& aPublishedContent,
                                     const Connection &aConnection ) ;
    /**
     * Method for parsing published private message. Does about same
     * work as @ref parseContentPublishedOrSent but structure of
     * private messages is somewhat different.
     */
    bool parsePrivMsgPublishedOrSent(const unsigned char aProtocolItemType,
                                     const QByteArray& aPublishedContent,
                                     const Connection &aConnection ) ;

    /**
     * Method for parsing published profile comments. Does about same
     * work as @ref parseContentPublishedOrSent but structure of
     * profile commentss is somewhat different.
     */
    bool parseProfileCommentPublishedOrSent(const unsigned char aProtocolItemType,
                                            const QByteArray& aPublishedContent,
                                            const Connection &aConnection ) ;


    /**
     * method for parsing request that requests for classified
     * ads whose classification matches given hash
     * @param aQueryBytes contains the query
     * @param aConnection is the requester
     */
    bool  parseAdsClassifiedAtHash( const QByteArray& aQueryBytes,
                                    const Connection &aConnection) ;
    /**
     * Method for parsing list of headers of classified ads.
     * In practice this parses the protocol message sent with
     * id KListOfAdsClassifiedAtHash.
     */
    bool  parseListOfAdsClassifiedAtHash( const QByteArray& aQueryBytes,
                                          const Connection &aConnection) ;
    /**
     * Method for parsing a search request from remote node.
     * If parse is successful, this method will also ask searchmodel
     * to perform the search and place possible results immediately
     * into send-queue of aConnection so search works slightly differently
     * compared to rest of the network requests
     * @param aQueryBytes serialized query-description
     * @param aConnection is connection of the node who sent the request
     * @return true if things went all right.
     */
    bool  parseSearchRequest( const QByteArray& aQueryBytes,
                              Connection& aConnection) ;
    /**
     * Method for parsing a search result from remote node.
     * In this case this node made a search request and this
     * is reply (containing list of objects matching
     * the query).
     * @param aQueryBytes serialized query-description
     * @param aConnection is connection of the node who sent the request
     * @return true if things went all right.
     */
    bool  parseSearchResults( const QByteArray& aQueryBytes,
                              const Connection& aConnection) ;
    /**
     * Method for parsing voice call status data. This parses
     * call status, not call realtime (audio) data. 
     *
     * @param aQueryBytes serialized call-status data
     * @param aConnection is connection of the node who sent the request
     * @return true if things went all right.
     */
    bool  parseVoiceCall( const QByteArray& aQueryBytes,
                          const Connection& aConnection) ;

    /**
     * Method for parsing voice call data. This parses actual 
     * data stream like audio, not call control data. 
     *
     * @param aQueryBytes serialized call data
     * @param aConnection is connection of the node whose operator
     * made a statement. 
     * @return true if things went all right.
     */
    bool  parseCallRtData( const QByteArray& aQueryBytes,
                           const Connection& aConnection) ;
    /**
     * method for parsing record of distributed database
     */
    bool parseDbRecord( const QByteArray& aQueryBytes,
                        const Connection& aConnection) ;
    /**
     * method for parsing search request regarding db record
     */
    bool parseDbRecordSearchTerms( const QByteArray& aQueryBytes,
                                   Connection& aConnection) ;
private:
    MController &iController ;
    MModelProtocolInterface &iModel ;
} ;
#endif
