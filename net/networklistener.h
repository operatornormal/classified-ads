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



#ifndef NETWORK_LISTENER_H
#define NETWORK_LISTENER_H

#include <QTcpServer>
#include <QList>
#include "connection.h" // for ConnectionObserver
#include <QUdpSocket>
#include <QNetworkSession>

class MController ;
class Model ;
class ProtocolMessageParser ;

/**
 * @brief class for listening to incoming network connections.
 *
 * This class will spawn a @ref Connection for each received
 * network connection.
 */
class NetworkListener :
    public QTcpServer,
    public Connection::ConnectionObserver {
    Q_OBJECT
public:
    /**
     * Constructor
     * @param aController application controller for app state frobnication
     * @param aModel data model reference for data storage
     * @param aIpv6 if set to true, this class will listen in Ipv6 instead
     *        of v4 and will not enumerate local network interfaces.
     */
    NetworkListener(MController *aController,
                    Model *aModel) ;
    /**
     * Destructor
     */
    ~NetworkListener() ;
    /**
     * separate method for starting the listen. this now works with qt4.8
     * but it seems like semantics of listen may change in qt5, where
     * listening in QHostAddress::Any automatically means both v4 and v6.
     * @param aIpv6 if true, tries to listen on both IPv6
     *        and ipv4.
     * @return true if listening started.
     */
    bool startListen(bool aIpv6) ;

    /**
     * from ConnectionObserver
     */
    virtual bool dataReceived(const QByteArray& aData,
                              Connection& aConnection) ;
    /**
     * From ConnectionObserver
     * this is called by peer connection at close
     */
    virtual void connectionClosed(Connection *aDeletee) ;
    /**
     * From ConnectionObserver
     * this is called by peer connection at successful open
     */
    virtual void connectionReady(Connection *aBusinessEntity)  ;
    /**
     * used in closing of app: stops accepting connections
     */
    void stopAccepting() ;
    /** 
     * Method that loops network interfaces and tries to find a valid
     * local address to advertise 
     */
    void figureOutLocalAddresses() ;
protected:
#if QT_VERSION >= 0x050000
    virtual void incomingConnection(qintptr handle);
#else
    void incomingConnection (int aSocketDescriptor ) ;
#endif
signals:
    void error(QTcpSocket::SocketError socketError);
    /**
     * this signal is used to communicate (at least to publishing engine)
     * status of connection attempt to particular node. network connection
     * engine will spawn connections to more-or-less random nodes but
     * publishing logic may ask for connections to specific nodes.
     * this signal will communicate outcome of such requests
     */
    void nodeConnectionAttemptStatus(Connection::ConnectionState aStatus,
                                     const Hash aHashOfAttemptedNode );
private slots:
    void broadCastReceived() ;
    void threadIsDeleted() ;
    void networkStateChanged( QNetworkSession::State aState ) ; 
private: // data
    MController *iController ;
    Model *iModel ;
    /** This animal here knows all incoming bytearrays */
    ProtocolMessageParser *iParser ;
    QUdpSocket iBroadCastListener ;
    /**
     * used in closing of application: flag for not accepting
     * connections any more
     */
    bool iCanAccept ;
    QNetworkConfiguration iConnectionConfig ;
    QNetworkSession* iNetworkSession ; 
} ;
#endif
