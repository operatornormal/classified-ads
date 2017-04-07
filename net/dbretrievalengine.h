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



#ifndef DB_RECORD_RETRIEVAL_ENG_H
#define DB_RECORD_RETRIEVAL_ENG_H

#include <QHostAddress>
#include <QPair>
#include "connection.h" // for ConnectionObserver
#include "../controller.h"
#include "../datamodel/model.h"
#include "../datamodel/cadbrecord.h"

/**
 * @brief Network-connection logic relating fetching db records from DHT
 *
 * Special retrieval logic for records of distributed db records only.
 * See also @ref RetrievalEngine that is used to fetch classified ads
 * or binary files. @ref RetrievalEngine is limited to fetching one
 * item at time, with db records we may have multiple collections open
 * at the same time and it may be necessary to fetch from all of those,
 * and at the same time honouring possible search terms that user
 * may have given. 
 *
 * Method @startRetrieving is called via controller due to users activity
 * usually inside TCL program. If conditions are met, this class locates
 * nodes in the network that store the desired collection and repeates
 * the query to those nodes in hope to retrieve more records matching the
 * query. 
 */
class DbRecordRetrievalEngine : public QTimer {
    Q_OBJECT
public:
    /**
     * Constructor
     * @param aController application controller. not owned.
     * @param aModel persistent storage.
     */
    DbRecordRetrievalEngine(Controller* aController,
                            Model& aModel ) ;
    /**
     * Destructor
     */
    ~DbRecordRetrievalEngine() ;
    /**
     * command-interface for this class: start to do work
     * @param aSearchTerms specifies the database records to dl.
     */
    void startRetrieving( CaDbRecord::SearchTerms aSearchTerms ) ;

public slots:
    /**
     * Command-interface for this class: cancel all ongoing retrievals.
     * This gets connected to "finished" signal of the tcl interpreter:
     * when program stops, it makes no more sense to serve it with
     * db requests. 
     */
    void stopRetrieving( ) ;

    /**
     * when content is received, we may want to check if it
     * was the content we were waiting for. this method
     * is for performing that check
     */
    void notifyOfContentReceived(const Hash& aHashOfContent,
                                 const ProtocolItemType aTypeOfReceivedContent );

    /**
     * when connection is attempted, @ref NetworkListener will
     * emit the status (failed or success) of the connection,
     * emitted signal is connected here
     */
    void nodeConnectionAttemptStatus(Connection::ConnectionState aStatus,
                                     const Hash aHashOfAttemptedNode );
    /**
     * this class is a not a thread, but QTimer, thus run.
     */
    void run();
private: // methods
    void sendQueryToNode(const CaDbRecord::SearchTerms& aSearchTerms, 
                         const Hash& aNodeFingerPrint) ; 
private: // data
    Controller* iController ; /**< application controller */
    Model &iModel ; /**< persistent storage */
    bool iNowRunning ; 
    QList<Hash> iNodeCandidatesToTryQuery ;
    QList<Hash> iNodesSuccessfullyConnected ;
    QList<Hash> iNodesFailurefullyConnected ;
    /** Type definition for download queue items. See @ref iSearchTerms */
    typedef struct DlQueueItemStruct {
        /** search conditions */
        CaDbRecord::SearchTerms iTerms ; 
        /** nodes that have been sent the conditions */
        QList<Hash> iListOfNodes ; 
        /** last time when any node on iListOfNodes got query */
        QDateTime iTimeOfLastRefresh ; 
    } DlQueueItem ; 
    /**
     * List of records to try retrieve. In pair inside the list
     * the "first" item is actual search condidtions, 
     * second is list of node fingerprints that have already
     * received the search query ; so it won't be sent again
     */
    QList<DlQueueItem> iSearchTerms ;
} ;
#endif
