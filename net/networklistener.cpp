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
#ifdef WIN32
#define NOMINMAX
#include <winsock2.h>
#else
#include "arpa/inet.h"
#endif
#include "networklistener.h"
#include "connection.h"
#include "../log.h"
#include "../mcontroller.h"
#include "../datamodel/model.h"
#include "protocol_message_parser.h"
#include "node.h"
#include <QNetworkInterface>
#include <QThread>
#include <QCoreApplication>
#include "miniupnpc.h"
#include "upnpcommands.h"
#ifndef WIN32
// Difficult issue here. Miniupnp and natpmp share some codebase
// and a common header "declspec.h" that contains common definitions. 
// Different linux distributions package these 2 packages sometimes
// from same original version, sometimes from different, 
// sometimes declspec.h is inclduded twice, etc. 
#if defined(MINIUPNP_LIBSPEC) && !defined(LIBSPEC)
#define LIBSPEC MINIUPNP_LIBSPEC
#endif
// hopefully LIBSPEC is now ok, continue to include natpmp
// header. 
#include "natpmp.h"
#endif
#include <errno.h>

NetworkListener::NetworkListener(MController *aController,
                                 Model *aModel) :
    QTcpServer(aController),
    iController(aController),
    iModel(aModel),
    iBroadCastListener(this),
    iCanAccept(true) {
    iParser = new ProtocolMessageParser(*iController,*iModel) ;
    setMaxPendingConnections(50) ;
    qRegisterMetaType<Connection::ConnectionState>("Connection::ConnectionState");
    figureOutLocalAddresses() ;
}

NetworkListener::~NetworkListener() {
    if ( iParser ) {
        delete iParser ;
        iParser = NULL ;
    }
}

bool NetworkListener::startListen(bool aIpv6) {
    if ( aIpv6 ) {
        if (!listen (  QHostAddress::AnyIPv6, iModel->nodeModel().listenPortOfThisNode() )) {
            QLOG_STR("Socket listen v6: " + errorString()) ;
            return false ;
        }
    } else {

        if(!listen (  QHostAddress::Any, iModel->nodeModel().listenPortOfThisNode() )) {
            QLOG_STR("Socket listen v4: " + errorString()) ;
            return false ;
        }
    }
    iBroadCastListener.bind(KBroadCastAdvertismentPort);
    connect(&iBroadCastListener, SIGNAL(readyRead()), this, SLOT(broadCastReceived()));
    return true ;
}

void NetworkListener::incomingConnection (int aSocketDescriptor ) {
    LOG_STR2("NetworkListener::incomingConnection desc = %d", aSocketDescriptor);
    if ( iCanAccept ) {
        QThread* t = new QThread();
        Connection *c = new   Connection(aSocketDescriptor,
                                         *this, // connection observer, is this
                                         *iModel,
                                         *iController);
        iModel->lock() ;
        // here we have a bug .. if IP addr changes, we'll advertise
        // our old address for the first peer to connect .. would
        // need to delay construction of node greeting to the moment
        // when we're already in connected state and can check
        // for local address of the socket.
        iModel->addOpenNetworkConnection(c) ;
        iModel->unlock() ;
        connect(t, SIGNAL(started()), c, SLOT(run()));

        connect(c, SIGNAL(finished()), t, SLOT(quit())) ;
        connect(t, SIGNAL(finished()), t, SLOT(deleteLater()));
        // after thread is gone, remove the connection too
        connect(t, SIGNAL(finished()), c, SLOT(deleteLater()));

        LOG_STR(QString("0x%1 ").arg((qulonglong)c, 8) +"Signals set for connection (in)") ;

        c->moveToThread(t) ;
        t->start();

        // as "this" was passed as parent to new connection, note that
        // libQtCore will delete connections when parent is deleted, e.g.
        // ~NetworkListener() is hit. Threads seem to be usually running
        // at that stage but haven't seen SIGSEGV:s at closing, not yet..

        // if we managed to get this far, we obviously can receive
        // connections:
        iController->getNode().setCanReceiveIncoming(true) ;
    } else {
        LOG_STR("Un-handled incoming connection at application close") ;
    }
    // looks like we don't get "deleteLater()" signals processed
    // without this
    QCoreApplication::processEvents(QEventLoop::ExcludeUserInputEvents) ;
}

void NetworkListener::threadIsDeleted() {
    QLOG_STR("Thread deleted from networklistener (Was connection close)") ;
}

bool NetworkListener::dataReceived(const QByteArray& aData,
                                   Connection& aConnection ) {
    return iParser->parseMessage(aData,aConnection) ;
}

void NetworkListener::connectionClosed(Connection *aDeletee) {
    LOG_STR("NetworkListener::connectionClosed, obtaining lock..") ;
    iModel->lock() ;
    Hash hashOfClosedConnection ( aDeletee->fingerPrintOfNodeAttempted() ) ;
    Node* nodeOfConnection = NULL ;
    if ( hashOfClosedConnection == KNullHash ) {
        // connection was made to no particular node. check if node
        // connection has node in place
        if ((nodeOfConnection = aDeletee->node())!=NULL) {
            // note that pointer returned by ->node() is not to be deleted.
            hashOfClosedConnection = nodeOfConnection->nodeFingerPrint() ;
        }
    }
    iModel->removeOpenNetworkConnection(aDeletee)  ;
    // if we got meaningful hash, emit that:
    if ( hashOfClosedConnection != KNullHash ) {
        emit nodeConnectionAttemptStatus(aDeletee->connectionState(),
                                         hashOfClosedConnection ) ;
    }
    if ( ( aDeletee->connectionState() == Connection::Initial ||
            aDeletee->connectionState() == Connection::Error ) &&
            aDeletee->fingerPrintOfNodeAttempted() != KNullHash ) {
        iModel->nodeModel().offerNodeToRecentlyFailedList(aDeletee->fingerPrintOfNodeAttempted()) ;
    }
    iModel->unlock() ;
    LOG_STR("NetworkListener::connectionClosed out, releasing lock..") ;
}

void NetworkListener::connectionReady(Connection *aBusinessEntity) {
    iModel->lock() ;
    emit nodeConnectionAttemptStatus(aBusinessEntity->connectionState(),
                                     aBusinessEntity->node()->nodeFingerPrint() ) ;
    LOG_STR("NetworkListener::connectionReady did emit") ;
    iModel->unlock() ;
}

void NetworkListener::figureOutLocalAddresses() {
    bool natAddrSeen = false ;
    bool globalIpv4AddrSeen = false ;
    bool globalIpv6AddrSeen = false ;

    QList<QHostAddress> ipAddrList = QNetworkInterface::allAddresses() ;
    for ( int i = 0 ; i < ipAddrList.size() ; i++ ) {
        if (!  ( ipAddrList.at(i) == QHostAddress::LocalHost ||
                 ipAddrList.at(i) == QHostAddress::LocalHostIPv6 ) ) {
            switch ( ipAddrList.at(i).protocol() ) {
            case QAbstractSocket::IPv4Protocol: {
                if (iController->getNode().setIpAddrWithChecks(ipAddrList.at(i))) {
                    // was ok addr
                    globalIpv4AddrSeen = true ;
                } else {
                    // was nat addr
                    natAddrSeen = true ;
                }
            }
            break ;
            case QAbstractSocket::IPv6Protocol:
                if ( iController->getNode().setIpAddrWithChecks(ipAddrList.at(i)) ) {
                    globalIpv6AddrSeen = true ;
                }
                break ;
            default:
                // QAbstractSocket::LU6.2, anyone?
                break ;
            }
        }
    }
    // if we did not see ipv6 addr, set it to zero so we don't
    // accidentally advertise old address
    if ( !globalIpv6AddrSeen ) {
        iController->getNode().setIpv6Addr(KNullIpv6Addr)  ;
    }
    if (   natAddrSeen &&
            !globalIpv4AddrSeen  ) {
        // ok, looks bad. we're behind nat-box. no incoming connections.
        // suxxors.
        LOG_STR("Ipv4 nat. Do something") ;
        {
            int error = 0;
            // following preprocessor directive tries to detect
            // miniupnpc at least version v1.23 (found in ubuntu 14.04)
            // that has ipv6+error arguments in upnpDiscover
#if defined(MINIUPNPC_API_VERSION) || defined(UPNPDISCOVER_MEMORY_ERROR)
            struct UPNPDev *upnp_dev = upnpDiscover(
                                           2000    , // time to wait (milliseconds)
                                           NULL , // multicast interface (or null defaults to 239.255.255.250)
                                           NULL , // path to minissdpd socket (or null defaults to /var/run/minissdpd.sock)
                                           0       , // source port to use (or zero defaults to port 1900)
                                           0       , // 0==IPv4, 1==IPv6
                                           &error  ); // error condition
#else
            // ancient upnpc in wheezy has  no API_VERSION defined
            // and the api, indeed, is different
            struct UPNPDev *upnp_dev = upnpDiscover(
                                           2000    , // time to wait (milliseconds)
                                           NULL , // multicast interface (or null defaults to 239.255.255.250)
                                           NULL , // path to minissdpd socket (or null defaults to /var/run/minissdpd.sock)
                                           0      ); // source port (0=default 1900)
#endif
            if ( error == 0 ) {
                char lan_address[64];
                struct UPNPUrls upnp_urls;
                struct IGDdatas upnp_data;
                int status = UPNP_GetValidIGD(upnp_dev, &upnp_urls, &upnp_data, lan_address, sizeof(lan_address));
                // look up possible "status" values, the number "1" indicates a valid IGD was found
                QLOG_STR("UPNP_GetValidIGD = " + QString::number(status)) ;
                if ( status > 0 ) {
                    // get the external (WAN) IP address
                    char wan_address[64] = { 0 } ;
                    int getExternalError =
                        UPNP_GetExternalIPAddress(upnp_urls.controlURL, upnp_data.first.servicetype, wan_address);
                    QLOG_STR("Wan addr: " + QString(wan_address) + " error = " + QString::number(getExternalError)) ;
                    if ( getExternalError == 0 ) {
                        QString externalIpString(wan_address) ;
                        QHostAddress remoteAddr(externalIpString) ;
                        QLOG_STR("Maybe setting wan addr to node: " + remoteAddr.toString() ) ;
                        if ( iController->getNode().setIpAddrWithChecks(remoteAddr) ) {
                            char port_number_str[30] = { 0 } ;
                            sprintf(port_number_str, "%d", iModel->nodeModel().listenPortOfThisNode()) ;
                            QLOG_STR("Calling UPNP_AddPortMapping with port = " + QString(port_number_str));
                            error = UPNP_AddPortMapping(
                                        upnp_urls.controlURL,
                                        upnp_data.first.servicetype,
                                        port_number_str     ,  // external (WAN) port requested
                                        port_number_str     ,  // internal (LAN) port to which packets will be redirected
                                        lan_address , // internal (LAN) address to which packets will be redirected
                                        "Classified ads", // text description to indicate why or who is responsible for the port mapping
                                        "TCP"       , // protocol must be either TCP or UDP
                                        NULL      // remote (peer) host address or nullptr for no restriction
#if defined(MINIUPNPC_API_VERSION) || defined(UPNPDISCOVER_MEMORY_ERROR)
                                        ,"0"    // port map lease duration (in seconds) or zero for "as long as possible"
#endif
                                    );

                            // list all port mappings
                            int index = 0;
                            // how many port mappings can you have in large installation?
                            for ( int portmappingcounter = 0 ; portmappingcounter < 1024 ; portmappingcounter++ ) {
                                char map_wan_port           [200] = "";
                                char map_lan_address        [200] = "";
                                char map_lan_port           [200] = "";
                                char map_protocol           [200] = "";
                                char map_description        [200] = "";
                                char map_mapping_enabled    [200] = "";
                                char map_remote_host        [200] = "";
                                char map_lease_duration     [200] = ""; // original time, not remaining time :(
                                char index_str[200] = { 0 } ;
                                sprintf(index_str, "%d", index) ;
                                QLOG_STR("Calling UPNP_GetGenericPortMappingEntry with index = " + QString::number(index)) ;
                                error = UPNP_GetGenericPortMappingEntry(
                                            upnp_urls.controlURL            ,
                                            upnp_data.first.servicetype     ,
                                            index_str                       ,
                                            map_wan_port                    ,
                                            map_lan_address                 ,
                                            map_lan_port                    ,
                                            map_protocol                    ,
                                            map_description                 ,
                                            map_mapping_enabled             ,
                                            map_remote_host                 ,
                                            map_lease_duration              );
                                index++ ;
                                QLOG_STR("UPNP_GetGenericPortMappingEntry error " + QString::number(error)) ;
                                QLOG_STR("map_remote_host = " + QString(map_remote_host)) ;
                                QLOG_STR("map_wan_port = " + QString(map_wan_port)) ;
                                QLOG_STR("map_protocol = " + QString(map_protocol)) ;
                                if ( strncmp(map_wan_port, port_number_str , 5 ) == 0 ) {
                                    iController->getNode().setCanReceiveIncoming(true) ;
                                    QLOG_STR("Our port forwarding found, getting out..") ;
                                    globalIpv4AddrSeen = true ;
                                    break ;
                                }
                                if (error) {
                                    break; // no more port mappings available
                                }
                            }
                        } else {
                            QLOG_STR("Did not use IP-addr from UPnP, maybe it too was behind nat?") ;
                        }
                    }
                }
            }
        }
        if ( !globalIpv4AddrSeen ) {
            // following code did not work in WIN32
#ifndef WIN32
            // kudos to http://miniupnp.free.fr/libnatpmp.html
            int r = -1;
            natpmp_t natpmp;
            natpmpresp_t response;
            in_addr_t gateway = 0;
            int forcegw = 0;
            int loopCount = 0 ;
            initnatpmp(&natpmp,forcegw,gateway);
            sendnewportmappingrequest(&natpmp, NATPMP_PROTOCOL_TCP, iModel->nodeModel().listenPortOfThisNode(), iModel->nodeModel().listenPortOfThisNode(),INT_MAX-1);
            do {
                fd_set fds;
                struct timeval timeout;
                FD_ZERO(&fds);
                FD_SET(natpmp.s, &fds);
                getnatpmprequesttimeout(&natpmp, &timeout);
                select(FD_SETSIZE, &fds, NULL, NULL, &timeout);
                r = readnatpmpresponseorretry(&natpmp, &response);
            } while(r==NATPMP_TRYAGAIN && ++loopCount < 100 );
            if(r>=0) {
                printf("Mapped public port %hu protocol %s to local port %hu "
                       "lifetime %u resp = %d\n",
                       response.pnu.newportmapping.mappedpublicport,
                       response.type == NATPMP_RESPTYPE_UDPPORTMAPPING ? "UDP" :
                       (response.type == NATPMP_RESPTYPE_TCPPORTMAPPING ? "TCP" :
                        "UNKNOWN"),
                       response.pnu.newportmapping.privateport,
                       response.pnu.newportmapping.lifetime,
                       r);
                // port done, obtain our public ip addr
                int addrQueryRespCode = 0  ;
                if(( addrQueryRespCode = sendpublicaddressrequest(&natpmp)) > 0 ) {
                    LOG_STR2("sendpublicaddressrequest ret = %d", addrQueryRespCode);
                    loopCount = 0 ;
                    do {
                        fd_set fds;
                        struct timeval timeout;
                        FD_ZERO(&fds);
                        FD_SET(natpmp.s, &fds);
                        getnatpmprequesttimeout(&natpmp, &timeout);
                        select(FD_SETSIZE, &fds, NULL, NULL, &timeout);
                        r = readnatpmpresponseorretry(&natpmp, &response);
                    } while(r==NATPMP_TRYAGAIN && ++loopCount < 100 );
                    if ( r>= 0 ) {
                        LOG_STR("Managed to obtain public ip addr via NATPMP") ;
                        LOG_STR2("Response type = %d", response.type) ;
                        if ( response.type == NATPMP_RESPTYPE_PUBLICADDRESS ) {
                            in_addr publicaddress = response.pnu.publicaddress.addr;
                            iController->getNode().setIpv4Addr(ntohl(publicaddress.s_addr)) ;
                            // if we got port forwarded and ip addr from upnp-box
                            // we can fairly safely say that we can receive
                            // incoming connections too:
                            iController->getNode().setCanReceiveIncoming(true) ;
                        }
                    } else {
                        LOG_STR2("NATPMP addr retrieval not success, r = %d", r );
                    }
                } else {
                    LOG_STR2("sendpublicaddressrequest ret = %d", addrQueryRespCode);
                }
            } else {
                LOG_STR2("NATPMP Mapping did not succeed, r = %d", r );
                switch (r) {
                case NATPMP_ERR_INVALIDARGS:
                    LOG_STR("NATPMP ERR_INVALIDARGS") ;
                    break ;
                case NATPMP_ERR_SOCKETERROR:
                    LOG_STR("NATPMP_ERR_SOCKETERROR : socket() failed. check errno for details") ;
                    break ;
                case NATPMP_ERR_CANNOTGETGATEWAY:
                    LOG_STR("NATPMP_ERR_CANNOTGETGATEWAY : can't get default gateway IP");
                    break ;
                case NATPMP_ERR_CLOSEERR:
                    LOG_STR2("NATPMP_ERR_CLOSEERR : close() failed.  error = %s", strerror(errno));
                    break ;
                case NATPMP_ERR_RECVFROM:
                    LOG_STR2("NATPMP_ERR_RECVFROM : recvfrom() failed. error = %s", strerror(errno));
                    break ;
                case NATPMP_ERR_NOPENDINGREQ:
                    LOG_STR("readnatpmpresponseorretry() called while no NAT-PMP request was pending") ;
                    break ;
                case NATPMP_ERR_NOGATEWAYSUPPORT:
                    LOG_STR("NATPMP_ERR_NOGATEWAYSUPPORT : the gateway does not support NAT-PMP") ;
                    break ;
                case NATPMP_ERR_CONNECTERR:
                    LOG_STR2("NATPMP_ERR_RECVFROM : connect() failed.  error %s", strerror(errno)) ;
                    break ;
                case NATPMP_ERR_WRONGPACKETSOURCE :
                    LOG_STR("NATPMP_ERR_WRONGPACKETSOURCE : packet not received from the network gateway") ;
                    break ;
                case NATPMP_ERR_SENDERR:
                    LOG_STR2("NATPMP_ERR_RECVFROM : send() failed.  error %s", strerror(errno)) ;
                    break ;
                case NATPMP_ERR_FCNTLERROR:
                    LOG_STR2("NATPMP_ERR_RECVFROM : fcntl() failed.  error %s", strerror(errno)) ;
                    break ;
                case NATPMP_ERR_GETTIMEOFDAYERR :
                    LOG_STR2("NATPMP_ERR_RECVFROM : gettimeofday() failed.  error %s", strerror(errno)) ;
                    break ;
                case NATPMP_ERR_UNSUPPORTEDVERSION:
                    LOG_STR("NATPMP_ERR_WRONGPACKETSOURCE : packet not received from the network gateway") ;
                    break ;
                case NATPMP_ERR_UNSUPPORTEDOPCODE:
                    LOG_STR("NATPMP_ERR_WRONGPACKETSOURCE : packet not received from the network gateway") ;
                    break ;
                case NATPMP_ERR_UNDEFINEDERROR :
                    LOG_STR("NATPMPErrors from the server : undefined") ;
                    break ;
                case NATPMP_ERR_NOTAUTHORIZED :
                    LOG_STR("NATPMPErrors from the server : not authorized") ;
                    break ;
                case NATPMP_ERR_NETWORKFAILURE :
                    LOG_STR("NATPMPErrors from the server : network failure") ;
                    break ;
                case NATPMP_ERR_OUTOFRESOURCES :
                    LOG_STR("NATPMPErrors from the server : out of resources") ;
                    break ;
                }
            }
            closenatpmp(&natpmp);
#endif // else-part of WIN32
        }
    }

    // lets greet self to have our own record in db updated
    // as it may later appear in query results..
    iModel->nodeModel().nodeGreetingReceived(iController->getNode(),true) ;
}

void  NetworkListener::broadCastReceived() {
    QByteArray buffer;
    int bytesToExpect ( iBroadCastListener.pendingDatagramSize() )  ;
    buffer.resize(bytesToExpect);
    quint16 senderPort ;
    QHostAddress sender;
    if ( bytesToExpect == iBroadCastListener.readDatagram(buffer.data(), buffer.size(),
            &sender, &senderPort) ) {
        if ( buffer.size() > 3 &&
                buffer[0] == 'c' &&
                buffer[1] == 'a' &&
                (
                    ( buffer[2] == '4' && buffer.size() == 27 )
                    ||
                    ( buffer[2] == '6' && buffer.size() == 43 )
                )
           ) {
            const char* ptr ( buffer.constData() ) ;
            ptr ++ ; // skip c
            ptr ++ ; // skip a
            ptr ++ ; // skip 4 or 6
            quint32* hash1ptr ( (quint32*)ptr ) ;
            ptr += sizeof(quint32) ; // skip hash1
            quint32* hash2ptr ( (quint32*)ptr ) ;
            ptr += sizeof(quint32) ; // skip hash1
            quint32* hash3ptr ( (quint32*)ptr ) ;
            ptr += sizeof(quint32) ; // skip hash1
            quint32* hash4ptr ( (quint32*)ptr ) ;
            ptr += sizeof(quint32) ; // skip hash1
            quint32* hash5ptr ( (quint32*)ptr ) ;
            ptr += sizeof(quint32) ; // skip hash1
            Hash hashOfNodeAdvertising ( ntohl (*hash1ptr),
                                         ntohl (*hash2ptr),
                                         ntohl (*hash3ptr),
                                         ntohl (*hash4ptr),
                                         ntohl (*hash5ptr) ) ;
            quint32* portPtr ( (quint32*)ptr ) ;
            ptr += sizeof(quint32) ; // skip port
            quint32 portHostByteOrder ( ntohl (*portPtr ) ) ;
            LOG_STR2("Port %u", portHostByteOrder) ;
            LOG_STR2("Hash %s", qPrintable(hashOfNodeAdvertising.toString())) ;
            // if ipv6, then dig also the addr
            const bool hasIpv6 =
                !Connection::Ipv6AddressesEqual(iController->getNode().ipv6Addr(),
                                                KNullIpv6Addr) ;
            if ( buffer[2] == '6' && hasIpv6 ) {
                Q_IPV6ADDR addr ;
                for ( int i=0 ; i < 16; i++ ) {
                    addr.c[i] = *ptr ;
                    ptr++ ;
                }
                sender.setAddress(addr) ;
            }
            if ( iCanAccept ) {
                QLOG_STR( "Message from: " + sender.toString() );
                if ( hashOfNodeAdvertising == iController->getNode().nodeFingerPrint()) {
                    // was my own ad
                    return ;
                } else {
                    iModel->lock() ;
                    iModel->nodeModel().addNodeFromBroadcast(hashOfNodeAdvertising,
                            sender,
                            portHostByteOrder) ;
                    iModel->unlock() ;
                }
            }
        }
    }
}

void NetworkListener::stopAccepting() {
    iCanAccept = false ;
    close() ;
}
