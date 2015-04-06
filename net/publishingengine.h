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



#ifndef PUBLISHING_ENG_H
#define PUBLISHING_ENG_H

#include <QHostAddress>
#include <QPair>
#include "connection.h" // for ConnectionObserver
#include "../controller.h"
#include "../datamodel/model.h"

/**
 * @brief Network-connection logic relating content publish demands
 *
 * Class that will ask for connections to nodes destined to
 *        be holders of stuff published in this node. Once
 *       connection is established, stuff to be published
 *       is copied into send-queue of the newly-connected
 *       node.
 */
class PublishingEngine : public QTimer {
    Q_OBJECT
public:
    /**
     * Constructor
     * @param aController application controller. not owned
     * @param aModel persistent storage.
     */
    PublishingEngine(Controller* aController,
                     Model& aModel ) ;
    /**
     * Destructor
     */
    ~PublishingEngine() ;
signals:
    void error(QTcpSocket::SocketError socketError);
public slots:
    /** when connection is attempted, @ref NetworkListener will
     * emit the status (failed or success) of the connection,
     * emitted signal is connected here
     */
    void nodeConnectionAttemptStatus(Connection::ConnectionState aStatus,
                                     const Hash aHashOfAttemptedNode );
    /**
     * this class is a not a thread, but QTimer, thus run.
     */
    void run();
private:
    void emptyNodeCandidateList() ;
    void askConnectionsForNodesOnPublishList() ;
    enum StageOfPublish {
        InitialStage,
        AwaitingConnection
    } ;
    void sendPublishItemToAlreadyConnectedNodes() ;
    void publishToNode(const Hash& aNode) ;
    void checkForSuccessfullyConnectedNodes() ;
    void checkForUnSuccessfullyConnectedNodes() ;
public:
    /** when this is set to false, thread will terminate and run() return */
    bool iNeedsToRun ;
private: // data
    Controller* iController ; /**< application controller */
    Model &iModel ; /**< persistent storage */
    PublishItem iWorkItem ; /**< what we're trying to publish */
    /** list of nodes where iWorkItem might be pushed to */
    QList<Node *> iNodeCandidatesToTryPush ;
    QList<Hash> iNodesSuccessfullyConnected ;
    QList<Hash> iNodesFailurefullyConnected ;
    StageOfPublish iStageOfPublish ;
    bool iNowRunning ;
} ;
#endif
