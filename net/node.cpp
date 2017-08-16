/*                                      -*-C++-*-
    Classified Ads is Copyright (c) Antti Jarvinen 2013.

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


#include "node.h"
#include "../log.h"
#include "connection.h" // for ipv6-addr comparison
#include <QDateTime>
#include "protocol.h"
#include "../mcontroller.h" // for KNullHash

const static char *KNodeJSonCanReceiveElement = "a" ;
const static char *KNodeJSonTorElement = "t" ;
const static char *KNodeJSonListenPortElement = "p" ;
const static char *KNodeJSonIPv6Element = "v6" ;
const static char *KNodeJSonIPv4Element = "v4" ;
const static char *KNodeJSonDnsElement= "d" ;
const static char *KNodeJSonLastContactTimeElement= "l" ;
const static char *KNodeJSonFPElement= "f" ;

/** nat subnets. see similar variable in connection.cpp */
const static      QPair<QHostAddress, int> nodeNormalNats1 =
    QHostAddress::parseSubnet("10.0.0.0/8");
const static      QPair<QHostAddress, int> nodeNormalNats2 =
    QHostAddress::parseSubnet("172.16.0.0/20");
const static      QPair<QHostAddress, int> nodeNormalNats3 =
    QHostAddress::parseSubnet("192.168.0.0/16");
/** non-routable address space that some ISP's seem to offer */
static const QPair<QHostAddress, int> nodeRfc6598 
    ( QHostAddress::parseSubnet("100.64.0.0/10") );
/** ipv6 non-routable address spaces */
static const QPair<QHostAddress, int> rfc3879 
    ( QHostAddress::parseSubnet("fec0::/10") ); 
static const QPair<QHostAddress, int> rfc4193
    ( QHostAddress::parseSubnet("fc00::/7") );

Node::Node(const Hash &aNodeFingerPrint,
           const int aListenPort) :
    iNodeFingerPrint(aNodeFingerPrint),
    iListenPort(aListenPort),
    iTimeOfLastContact(0) ,
    iIPv4Addr(0),
    iDnsName(NULL) ,
    iTorAddr(NULL) ,
    iCanReceiveIncoming(false),
    iTimeOfGoodNodeList(0),
    iLastConnectTime(0),
    iLastMutualConnectTime(0) {
    iIPv6Addr = KNullIpv6Addr ; // QHostAddress("::0").toIPv6Address () ; // null addr initially
}

Node::~Node() {
    if ( iDnsName ) {
        delete iDnsName ;
    }
    if ( iTorAddr ) {
        delete iTorAddr ;
    }
}

void Node::setIpv4Addr(const quint32 aAddr) {
    iIPv4Addr  = aAddr ;
    QLOG_STR("Ipv4 addr: " + QHostAddress(aAddr).toString()) ;
}

void Node::setIpv6Addr(const Q_IPV6ADDR &aAddr) {
    iIPv6Addr  = aAddr ;
    QLOG_STR("Ipv6 addr of node = "+ QHostAddress(aAddr).toString()) ;
}

quint32 Node::ipv4Addr(void) const {
    return iIPv4Addr ;
}

Q_IPV6ADDR Node::ipv6Addr(void) const {
    return iIPv6Addr ;
}

QString Node::DNSAddr(void) const {
    QString retval ;
    if ( iDnsName && iDnsName->length() > 0 ) {
        retval = *iDnsName ;
    }
    return retval ;
}


void Node::setDNSAddr(const QString& aAddr) {
    if ( iDnsName ) {
        delete iDnsName ;
        iDnsName = NULL ;
    }
    if ( aAddr.length() > 0 ) {
        iDnsName = new QString(aAddr) ;
    }
}


QString Node::TORAddr(void) const {
    QString retval ;
    if ( iTorAddr && iTorAddr->length() > 0 ) {
        retval = *iTorAddr ;
    }
    return retval ;
}


void Node::setTORAddr(const QString& aAddr) {
    if ( iTorAddr ) {
        delete iTorAddr ;
        iTorAddr = NULL ;
    }
    if ( aAddr.length() > 0 ) {
        iTorAddr = new QString(aAddr) ;
    }
}


void Node::setPort(const int aPort) {
    iListenPort = aPort ;
}

int Node::port(void) const {
    return iListenPort ;
}


void Node::setGoodNodeListTime(const time_t aTime) {
    time_t wallClockTime = QDateTime::currentDateTimeUtc().toTime_t() ;
    if ( aTime > ( wallClockTime + 60 ) ) {
        // da phuck. clock skew?
        iTimeOfGoodNodeList = wallClockTime ;
        LOG_STR2("Clock skew? %u", (unsigned)aTime) ;
    } else {
        iTimeOfGoodNodeList= aTime ;
    }
}

time_t Node::goodNodeListTime(void) const {
    return iTimeOfGoodNodeList ;
}

const Hash& Node::nodeFingerPrint(void) const {
    return iNodeFingerPrint ;
}

void Node::setLastConnectTime(const time_t aTime) {
    time_t wallClockTime = QDateTime::currentDateTimeUtc().toTime_t() ;
    if ( aTime > ( wallClockTime + 60 ) ) {
        // da phuck. clock skew?
        iLastConnectTime = wallClockTime ;
        LOG_STR2("Clock skew? %u", (unsigned) aTime) ;
    } else {
        iLastConnectTime = aTime ;
    }
}

time_t Node::lastConnectTime(void) const {
    return iLastConnectTime ;
}

void Node::setLastMutualConnectTime(const time_t aTime) {
    time_t wallClockTime = QDateTime::currentDateTimeUtc().toTime_t() ;
    if ( aTime > ( wallClockTime + 60 ) ) {
        // da phuck. clock skew?
        iLastMutualConnectTime = wallClockTime ;
        LOG_STR2("Clock skew? %u", (unsigned) aTime) ;
    } else {
        iLastMutualConnectTime = aTime ;
    }
}

time_t Node::lastMutualConnectTime(void) const {
    return iLastMutualConnectTime ;
}


void Node::setCanReceiveIncoming(const bool aYesItCan) {
    iCanReceiveIncoming= aYesItCan ;
}

bool Node::canReceiveIncoming(void) const {
    return iCanReceiveIncoming;
}

//
// this one conditionally sets ipv6 or ipv4 addr. this is called
// from connection and from networklistener.
//
bool Node::setIpAddrWithChecks(const QHostAddress& aAddress) {
    bool retval = false ;
    if (!  ( aAddress == QHostAddress::LocalHost ||
             aAddress == QHostAddress::LocalHostIPv6 ) ) {
        if ( QAbstractSocket::IPv6Protocol == aAddress.protocol() ) {
            if ( aAddress.scopeId().toLower() == "global" ) {
                // this should be good addr for us ; if there
                // is multiple, still use only one, it is
                // here advertised as "Global" so supposedly
                // it will get routed..
                setIpv6Addr(aAddress.toIPv6Address() )  ;
                retval = true ;
            } else {

                QString ipv6String ( aAddress.toString() );
                if ( ipv6String.toLower().startsWith("fe80") ) {
                    // is link-local
                } else if ( ipv6String.toLower().startsWith("fec0") ) {
                    // is site-local, is it useful, no?
                } else if ( ipv6String.toLower().startsWith("::") ) {
                    // address starting with all-zeroes, must
                    // not be globally routing, no?
                } else if ( aAddress.isInSubnet(rfc3879) ) {
                    QLOG_STR("Not using rfc3879 private address space") ;
                } else if ( aAddress.isInSubnet(rfc4193) ) {
                    QLOG_STR("Not using rfc4193 unique local address space") ;
                } else if ( ipv6String.toLower().startsWith("2001:0:") ) {
                    // this is a teredo address and it seems to me that
                    // there is more non-functional teredo addresses
                    // than functional .. and host having teredo needs
                    // to have ipv4 anyway so we don't lose much here.
                    QLOG_STR("Not using teredo-ipv6 addr") ;
                } else {
#ifdef WIN32
                    // it seems to me that win32 reports permanent address first
                    // so if we already have one, do not change that:
                    if ( Connection::Ipv6AddressesEqual(iIPv6Addr,
                                                        KNullIpv6Addr) == true ) {
                        setIpv6Addr(aAddress.toIPv6Address() ) ;
                    } else {
                        // we already managed to get one addr so lets not change that
                    }
#else
                    // seems to me that there is no consistent behaviour in linux
                    // so lets just pick up the latest addr..
                    setIpv6Addr(aAddress.toIPv6Address() ) ;
#endif
                    retval = true ;
                }
            }
        } else if ( QAbstractSocket::IPv4Protocol == aAddress.protocol() ) {
            // hmm ..
            if ( aAddress.isInSubnet(nodeNormalNats1) ||
		 aAddress.isInSubnet(nodeNormalNats2) ||
		 aAddress.isInSubnet(nodeNormalNats3) ||
		 aAddress.isInSubnet(nodeRfc6598) ) {
                // is nat addr, no-no
            } else {
                if ( aAddress.toString().startsWith("169.254") ) {
                    QLOG_STR("Got link-local ipv4-addr (zeroconf?) -> not using "+ aAddress.toString()) ;
                } else {
                    setIpv4Addr(aAddress.toIPv4Address ()) ;
                    retval  = true ;
                }
            }

        }
    }
    return retval ;
}


QVariant Node::asQVariant() const {

    QMap<QString,QVariant> m ;
    m.insert(KNodeJSonFPElement, iNodeFingerPrint.toString()) ;
    if ( iListenPort > 0 ) {
        m.insert(KNodeJSonListenPortElement, iListenPort) ;
    }
    m.insert(KNodeJSonLastContactTimeElement,
             (unsigned)iTimeOfLastContact ) ;
    if ( iIPv4Addr ) {
        m.insert(KNodeJSonIPv4Element, iIPv4Addr) ;
    }
    if (!Connection::Ipv6AddressesEqual(iIPv6Addr,
                                        KNullIpv6Addr)) {
        m.insert(KNodeJSonIPv6Element, QHostAddress(iIPv6Addr).toString()) ;
    }
    if ( iDnsName && iDnsName->length() > 0 ) {
        m.insert(KNodeJSonDnsElement, *iDnsName) ;
    }
    if ( iTorAddr && iTorAddr->length() > 0 ) {
        m.insert(KNodeJSonTorElement, *iTorAddr ) ;
    }
    m.insert(KNodeJSonCanReceiveElement, iCanReceiveIncoming ) ;
    QVariant j (m) ;
    return j ;
}

Node* Node::fromQVariant(const QVariantMap& aJSonAsQVariant,
                         const bool aIsInitialGreeting) {
    Node* n = NULL ;

    quint32 nodePort(0) ;
    if ( aJSonAsQVariant.contains(KNodeJSonFPElement) ) {
        const unsigned char *fpStr ( (const unsigned char *) qPrintable(aJSonAsQVariant[KNodeJSonFPElement].toString())) ;
        Hash nodeFp ;
        nodeFp.fromString(fpStr) ;
        if ( aJSonAsQVariant.contains(KNodeJSonListenPortElement) ) {
            nodePort = aJSonAsQVariant[KNodeJSonListenPortElement].toUInt() ;
        }
        if ( (  nodeFp != KNullHash  ) && nodePort > 0 )  {
            n = new Node ( nodeFp,nodePort) ;

            if ( aJSonAsQVariant.contains(KNodeJSonLastContactTimeElement) ) {
                unsigned contactTime (aJSonAsQVariant[KNodeJSonLastContactTimeElement].toUInt());
                if ( contactTime ) {
                    n->setLastConnectTime(contactTime) ;
                }
            }

            if ( aJSonAsQVariant.contains(KNodeJSonIPv4Element) ) {
                quint32 ipv4 (aJSonAsQVariant[KNodeJSonIPv4Element].toUInt());
                if ( ipv4 ) {
                    n->setIpv4Addr(ipv4) ;
                }
            }
            if ( aJSonAsQVariant.contains(KNodeJSonIPv6Element) ) {
                QString ipv6 (aJSonAsQVariant[KNodeJSonIPv6Element].toString());
                if ( ipv6.length() > 0 ) {
                    QHostAddress a ( ipv6 ) ;
                    if (!Connection::Ipv6AddressesEqual(a.toIPv6Address(),
                                                        KNullIpv6Addr)) {
                        n->setIpv6Addr(a.toIPv6Address()) ;
                    }
                }
            }
            if ( aJSonAsQVariant.contains(KNodeJSonDnsElement) ) {
                n->setDNSAddr(aJSonAsQVariant[KNodeJSonDnsElement].toString()) ;
            }
            if ( aJSonAsQVariant.contains(KNodeJSonTorElement) ) {
                n->setTORAddr(aJSonAsQVariant[KNodeJSonTorElement].toString()) ;
            }
            if ( aIsInitialGreeting ) {
                n->setLastConnectTime ( QDateTime::currentDateTimeUtc().toTime_t() ) ;
            }
            if ( aJSonAsQVariant.contains(KNodeJSonCanReceiveElement) ) {
                bool canReceive (aJSonAsQVariant[KNodeJSonCanReceiveElement].toBool());
                n->setCanReceiveIncoming(canReceive) ;
            }
            if ( aJSonAsQVariant.contains(KNodeJSonTorElement) ) {
                n->iTorAddr = new QString(aJSonAsQVariant[KNodeJSonTorElement].toString()) ;
            }
            if ( aJSonAsQVariant.contains(KNodeJSonDnsElement) ) {
                n->iDnsName = new QString(aJSonAsQVariant[KNodeJSonDnsElement].toString()) ;
            }
        }
    }
    return n ;
}
