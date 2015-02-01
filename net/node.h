/*                                      -*-C++-*-
    Classified Ads is Copyright (c) Antti Jarvinen 2013.

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


#ifndef NODE_H
#define NODE_H
#include <QObject>
#include "../util/hash.h"
#include <time.h>
#include <QHostAddress> // for Q_IPV6ADDR
class Hash ;

extern Q_IPV6ADDR KNullIpv6Addr   ;

/**
 * @brief Node is a peer in network.
 *
 * Class that represents a node in network ; typically
 * a physical computer. Nothing prevents having multiuser
 * computer, each user running separate nodes ; but usually
 * only one node per physical computer.
 */
class Node {

public:

  /**
   * constructor
   * @param aNodeFingerPrint is certificate fingerprint of the
   *        node that this class re-presents
   * @param aListenPort is the network port where the
   *        node is supposed to listen
   */
  Node(const Hash &aNodeFingerPrint,
       const int aListenPort) ;
  /**
   * Destructor
   */
  ~Node() ;
  /**
   * sets ipv4 addr of node
   */
  void setIpv4Addr(const quint32 aAddr) ;
  /**
   * sets ipv6 addr of node
   */
  void setIpv6Addr(const Q_IPV6ADDR &aAddr) ;
  /**
   * gets ipv4 addr of node
   */
  quint32 ipv4Addr(void) const ;
  /**
   * gets ipv6 addr of node
   */
  Q_IPV6ADDR ipv6Addr(void) const ;
  /**
   * gets dns-name of node
   */
  QString DNSAddr(void) const ;
  /**
   * sets dns-name of node
   */
  void setDNSAddr(const QString& aAddr) ;
  /**
   * gets tor-name of node
   */
  QString TORAddr(void) const ;
  /**
   * sets tor-name of node
   */
  void setTORAddr(const QString& aAddr) ;
  /**
   * sets listen port of node
   */
  void setPort(const int aPort) ;
  /**
   * gets listen port of node
   */
  int port(void) const ;

  /**
   * sets time of good node list
   */
  void setGoodNodeListTime(const time_t aTime) ;
  /**
   * gets time of good node list
   */
  time_t goodNodeListTime(void) const ;
  /**
   * gets hash of node
   */
  const Hash& nodeFingerPrint(void) const ;
  /**
   * sets last time of connect
   */
  void setLastConnectTime(const time_t aTime) ;
  /**
   * gets last time of connect
   */
  time_t lastConnectTime(void) const ;
  /**
   * sets last time of connect to this particular node here running this code
   */
  void setLastMutualConnectTime(const time_t aTime) ;
  /**
   * gets last time of connect to this node.
   */
  time_t lastMutualConnectTime(void) const ;

  /**
   * sets bool if can receive incoming connections
   */
  void setCanReceiveIncoming(const bool aYesItCan) ;
  /**
   * gets bool if can receive incoming connections
   */
  bool canReceiveIncoming(void) const ;
  /**
   * method intended for setting local address ; this will either
   * set or not set an addr, based on checks inside
   * @param aAddress is the address to try
   * @return true if aAddress was decided to be valid for future advertisment
   */
  bool setIpAddrWithChecks(const QHostAddress& aAddress) ;
  /**
   * Method for getting node reference as JSon / QVariant
   */
  QVariant asQVariant() const ;
  /**
   * reverse of @ref asQVariant()
   * @return node or NULL if
   */
  static Node* fromQVariant(const QVariantMap& aJSonAsQVariant,
                            const bool aIsInitialGreeting ) ;

private:
  /** SHA1 fingerprint of node certificate */
  const Hash iNodeFingerPrint  ;
  /** TCP port number where node is supposed to listen */
  int iListenPort ;
  /** time when node was last successfully contacted */
  time_t iTimeOfLastContact ;
  /** 32 bits of IPv4 addr, 0 if no IPv4 addr present */
  quint32 iIPv4Addr ;
  /** ipv6-addr */
  Q_IPV6ADDR iIPv6Addr ;
  /** dns-name as text */
  QString* iDnsName ;
  /** address within tor-network as text */
  QString* iTorAddr ;
  /** true if node has been successfully contacted from outside */
  bool iCanReceiveIncoming ;
  /** time when node says has last received good node listing */
  time_t iTimeOfGoodNodeList ;
  /** time when node was last seen connecting */
  time_t iLastConnectTime ;
  /** time when node was last connected to this node */
  time_t iLastMutualConnectTime ;
} ;
#endif
