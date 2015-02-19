/*    -*-C++-*- -*-coding: utf-8-unix;-*-
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

#ifndef CLASSIFIED_DATAMODEL_H
#define CLASSIFIED_DATAMODEL_H
#include <QSqlDatabase>
#include <QObject>
#include "../mcontroller.h" // because enum from there is needed
#include "../net/connection.h"
#include <QHostAddress>
#include <QPair>
#include "netrequestexecutor.h"
#include "nodemodel.h"
#include "mmodelprotocolinterface.h"
#include "mnodemodelprotocolinterface.h"

class QSqlDatabase ;
class QMutex ;
class Hash ;
class QSslCertificate ;
class QSslKey ;
class SearchModel ; 
class TrustTreeModel ; 
/**
 * @brief M of the MVC pattern. Handles permanent storage.
 *
 * All permanent data storage inside classified ads goes somehow
 * via this class here. If user wants to display data, view gets
 * it from here, if a peer wants to leech a phile, it comes from here
 */
class Model : public MModelProtocolInterface {
  Q_OBJECT

public:
  Model(MController *aMController) ;
  ~Model() ;
  ContentEncryptionModel& contentEncryptionModel() const ; /**< method for getting content-encryption-specific datamodel */
  virtual ProfileModel& profileModel() const ; /**< method for getting profile-specific datamodel */
  virtual BinaryFileModel& binaryFileModel() const ; /**< method for getting binary-blob datamodel */
  virtual ClassifiedAdsModel& classifiedAdsModel() const ; /**< method for getting the ads datamodel */
  virtual PrivMessageModel& privateMessageModel() const ; /**< method for getting the priv msg datamodel */
  virtual ProfileCommentModel& profileCommentModel() const ; /**< method for getting the comment datamodel */
  virtual SearchModel* searchModel() const ; /**< method for getting the full text search datamodel */
  virtual TrustTreeModel* trustTreeModel() const ; /**< method for getting the trust tree datamodel */
  void addOpenNetworkConnection(Connection *aConnection) ;
  /**
   * Method for telling datamodel about a existing network connection
   * has been closed for any reason that is valid for a socket.
   * @param aConnection is pointer to the connection
   * @return none
   */
  void removeOpenNetworkConnection(Connection *aDeletedConnection) ;

  /**
   * Method that other parts of this program use to append
   * items into send queue of node connection.
   * @param aDestinationNode specifies the node to receive bytes
   * @param aBytesToSend contains the message. Ownership of bytes
   *        is transferred to datamodel i.e. after this method is
   *        called, the caller is not supposed to delete the
   *        bytes itself but that task is left to datamodel to do.
   * @return none.
   */
  void addItemToSend(const Hash& aDestinationNode,
                     QByteArray* aBytesToSend) ;

  /**
   * Currently open connections.
   */
  const QList <Connection *>& getConnections() const ;
  /**
   * Currently pending network requests.
   * Even as this returns a pointer, not a reference,
   * ownership of the list is not transferred ; caller
   * may modiify content but is not supposed to delete
   */
  QList <NetworkRequestExecutor::NetworkRequestQueueItem>& getNetRequests() const ;
  /**
   * From MModelProtocolInterface
   *
   * method for adding a network request
   * @param aRequest is the request to add
   * @return none
   */
  virtual void addNetworkRequest(NetworkRequestExecutor::NetworkRequestQueueItem&
                                 aRequest) const ;
  /**
   * From MModelProtocolInterface.
   * thread sync: this claims access to datamodel
   */
  virtual bool lock() ;
  /**
   * From MModelProtocolInterface.
   * thread sync: releases data model to other threads
   */
  virtual void unlock() ;
  /**
   * From MModelProtocolInterface.
   *
   * method for getting node-specific datamodel
   */
  virtual MNodeModelProtocolInterface& nodeModel() const ;
  /**
   * pointer to class handling network requests
   */
  NetworkRequestExecutor* getNetReqExecutor() ;
  /**
   * Close all connections that might be open. Called before
   * deletia.
   * @param aDeleteAlso if set to true, will delete remaining
   *                    connections. if that needs to be done,
   *                    results easily in SEGFAULT as connections
   *                    are threads .. they should be closed so
   *                    that threads no  longer run, run() returns
   *                    and thread cleans up itself..
   */
  void closeAllConnections(bool aDeleteAlso) ;
  /**
   * method for communicating the fact that socket is open or closed
   */
  void connectionStateChanged(Connection::ConnectionState aState,
                              Connection *aConnection) ;
  /**
   * method for getting next item to publish
   * @param aFound will be set to true if there is an item to publish
   * @return if aFound is true, returned item contains details about
   *         the item to publish.
   */
  PublishItem nextItemToPublish(bool* aFound) ;
  /**
   * Method for marking item already in datamodel to be published via
   * @ref PublishingEngine class. For example, if publishing a profile
   * user must first insert the profile into profilemodel, and only
   * after that call this method here.
   * @param aType tells type of an item - profile,privmsg,binary blob etc.
   * @param aHash fingerprint for identifies object
   * @param aBangPath list of low-order hash-bits of nodes where this content
   *                  is already supposed to be found 
   * @param aSecondaryAddr is possible 2nd publish addr, used for CA and comment.
   *                       for private messages this is the destination
   *                       node, if known. 
   */
  void addItemToBePublished(ProtocolItemType aType, 
			    const Hash& aHash,
			    const QList<quint32>& aBangPath,
			    const Hash& aSecondaryAddr = KNullHash ) ;
  /**
   * When publishing-engine is doing its job, it uses this method to
   * update the list of nodes that have received the published content.
   * 
   * @param aContentHash identifies the published content that has been
   *                     pushed into one node
   * @param aNodeHash identifies the node where content was pushed to
   * @return The same item to publish, now updated. If the content is
   *         already published into enough hosts (model decides) it will
   *         return a PublishItem will null hash to tell publishing-engine
   *         that this piece of content is now done and it is time to
   *         move to next.
   */
  PublishItem addNodeToPublishedItem(const Hash& aContentHash,const Hash& aNodeHash) ;
public slots:
  /**
   * notification of content received
   */
  void notifyOfContentReceived(const Hash& aHashOfContent,
			       const ProtocolItemType aTypeOfReceivdContent) ;
  /**
   * notification of classified ads received
   */
  void notifyOfContentReceived(const Hash& aHashOfContent,
			       const Hash& aHashOfClassification,
			       const ProtocolItemType aTypeOfReceivdContent) ;
protected:
  /**
   * for periodical stuff inside datamodel
   */
  void timerEvent(QTimerEvent *event);
private:
  bool openDB();
  QSqlError lastError();
  // 1st-time initialization methods ahead
  bool createTables() ;
  void initPseudoRandom() ; /**< just calls srand() */
signals:
  void error(MController::CAErrorSituation aError,
             const QString& aExplanation) ;
private:
  MController *iController  ;
  QSqlDatabase iDb;/**< actual storage here */
  QMutex* iMutex ; /**< this mutex protects access to this datamodel */
  QList <Connection *> *iConnections ; /** Network connections currently open */
  NodeModel* iNodeModel ;/**< datamodel about our peers */
  NetworkRequestExecutor* iNetReqExecutor ;
  /**
   * have datamodel own network request queue, this is our queue
   * of pending tasks
   */
  QList <NetworkRequestExecutor::NetworkRequestQueueItem>* iNetReqQueue ;
  ContentEncryptionModel* iContentEncryptionModel ;/**< cryptographic animal */
  ProfileModel* iProfileModel ; /**< Profile data storage */
  BinaryFileModel* iBinaryFileModel ; /**< Random binary blob storage */
  ClassifiedAdsModel* iCaModel ; /**< Classified ads storage */
  int iTimerId ; /**< id of our periodical timer */
  PrivMessageModel* iPrivMsgModel ; /**< storage of private messages */
  ProfileCommentModel* iProfileCommentModel ; /**< comments data storage */
  SearchModel* iSearchModel ; /**< full text search model part */
  TrustTreeModel *iTrustTreeModel ; /**< trust list handling model */
} ;
#endif
