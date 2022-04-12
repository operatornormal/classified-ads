/*    -*-C++-*- -*-coding: utf-8-unix;-*-
  Classified Ads is Copyright (c) Antti Järvinen 2013-2021.

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

#ifndef CLASSIFIED_CADBRECORDMODEL_H
#define CLASSIFIED_CADBRECORDMODEL_H
#include <QPair>
#include "../mcontroller.h" // because enum from there is needed
#include "../net/protocol.h" // for SendQueueItem
#include "datamodelbase.h"
#include <limits>

class Hash ;
class MModelProtocolInterface ;
class CaDbRecord ;

/**
 * @brief This is is part of datamodel for storing database records
 *
 * Because classified ads is a distributed database that holds
 * ads, profiles, private messages and profile comments, the 
 * same storage implementation can be extended to general-purpose
 * database records. This is the implementation of local storage
 * for those records. See @ref CaDbRecord for records. This same
 * API is available via TCL interface with different syntax but
 * same concepts.
 */
class CaDbRecordModel : public ModelBase {
    Q_OBJECT

public:
    CaDbRecordModel(MController *aMController,
                    MModelProtocolInterface &aModel) ;
    ~CaDbRecordModel() ;

    /**
     * @brief method for storing and publishing a database record. 
     * 
     * Database records are published to the network when
     * persisted. There is separate local storage for
     * data not meant to be shared at the network but
     * database records do get published to all nodes
     * in the network. 
     *
     * @param aRecord The db record to be published
     * @param aRecordReaders Optional list of profile fingerprints
     *        to whom the record will be encrypted to. Only those profiles
     *        will be able to read the record. This list is considered only when
     *        @ref CaDbRecord::iIsEncrypted is true. If record is set to
     *        be encrypted, and this list is empty, the readers of the currently
     *        selected profile will be the readers of the record. If profile 
     *        is public ( there is no list of readers ) and record is set to
     *        be encrypted then this list must contain at least one profile. 
     * @return empty string if success, or error message
     */
    QString publishDbRecord(CaDbRecord& aRecord,
                            const QList<Hash>* aRecordReaders = NULL ) ;

    /**
     * Method for retrieving db records. See @ref CaDbRecord for records
     * being fetched. Limiting criteria can be specified using several
     * optional parameters. Those parameters are considered together using
     * "AND" operation, meaning they all need to be satisfied at the
     * same time for a record to be returned. For example, if both "by sender"
     * and "modified after" parameters are given, result set contains only
     * records from that given profile and whose publish time is after
     * given time. 
     *
     * This may have a side-effect when search terms are broad (like, only
     * collection specified) and it may be suspected that the search is
     * not fully served what it comes to potential resultset, this method
     * may give the query to @ref DbRecordRetrievalEngine that will
     * spam the network with the query. More records from neighboring network nodes
     * fitting the search criteria may then start appearing into database. 
     * TCL app that originated the query will receive notifications about 
     * new records but it will still need to use this same method here to 
     * fetch contents of the new records. In particular, if record is queried 
     * using aById parameter and the record is found, then no network-query is 
     * started. 
     *
     * @param aFromCollection Returns records whose @ref CaDbRecord::iCollectionId
     *        match this parameter. Either this or aById argument must be 
     *        given. Specifying only this will return whole collection 
     *        that may be huge. 
     * @param aById Returns record whose @ref CaDbRecord::iRecordId matches. Result-set
     *        is either empty or contains one record. If user passes KNullHash
     *        here then value of this argument is simply ignored and search
     *        method will return records fitting criteria stated by other
     *        method arguments like aBySearchPhrase. In theory it is possible
     *        that 2 users end up publishing a record with same id but that
     *        should be fairly rare occasion. 
     * @param aModifiedAfter Returns records that have last been modified after
     *        given time. Time of modification is stored in 
     *        @ref CaDbRecord::iTimeOfPublish.
     * @param aModifiedBefore Returns records that have last been modified before
     *        given time. 
     * @param aByHavingNumberMoreThan Returns records whose 
     *        @ref CaDbRecord::iSearchNumber
     *        is more or equal than number given here. 
     * @param aByHavingNumberLessThan Returns records whose 
     *        @ref CaDbRecord::iSearchNumber
     *        is less or equal than number given here. 
     * @param aBySearchPhrase Returns records whose 
     *        @ref CaDbRecord::iSearchPhrase matches
     *        the string given here, according to rules of sqlite FTS string matching
     *        implementation. 
     * @param aBySender Returns only records published by profile given in this 
     *        argument.
     * @param aForPublish if set to true, data-parts of the records are 
     *        returned un-altered from database. Normally records are
     *        de-crypted and un-compressed before getting returned
     *        but if record is meant to be sent to another node, the
     *        bitstream must be exactly the one that got signed. 
     * 
     * @return Pointer array of db records. Caller is responsible for deleting the
     *         returned records.
     */
    QList<CaDbRecord *> searchRecords(const Hash& aFromCollection,
                                      const Hash& aById = KNullHash,
                                      const quint32 aModifiedAfter = 0 ,
                                      const quint32 aModifiedBefore = std::numeric_limits<quint32>::max(),
                                      const qint64 aByHavingNumberMoreThan = std::numeric_limits<qint64>::min(),
                                      const qint64 aByHavingNumberLessThan = std::numeric_limits<qint64>::max(),
                                      const QString& aBySearchPhrase = QString(),
                                      const Hash& aBySender = KNullHash,
                                      const bool aForPublish = false ) ; 
                                     

    /**
     * called by protocol parser when a db record is received
     * @param aRecord
     * @param aSignature is digital signature of the payload of 
     *        the record
     * @param aBangPath is list of last nodes where this content 
     *         has been, idea is to prevent re-sending content to 
     *         somewhere where it has already been
     * @param aFromNode hash of node sending the record to us
     * @return true on success
     */
    bool publishedCaDbRecordReceived(CaDbRecord& aRecord,
                                     const QByteArray& aSignature,
                                     const QList<quint32>& aBangPath,
                                     const Hash& aFromNode) ;
    /**
     * called by protocol parser when a record is received by "send" 
     * protocol type
     * @param aRecord
     * @param aSignature is digital signature of the payload of 
     *        the record
     * @param aFromNode hash of node sending the record to us
     * @return true on success
     */
    bool sentCaDbRecordReceived(CaDbRecord& aRecord,
                                const QByteArray& aSignature,
                                const Hash& aFromNode ) ;

    /**
     * Method for filling connected peers send queue with items
     * that we have and that belong to said peers bucket.
     *
     * @param aSendQueue is send-queue of the connection that
     *                   serves particular node
     * @param aStartOfBucket is hash where bucket of that peer
     *                       starts. In practice it is the hash
     *                       of the node itself.
     * @param aEndOfBucket is hash where bucket of that peer ends.
     *                     That in turn depends on network size,
     *                     or number of active nodes in the network.
     *                     See method @ref NodeModel::bucketEndHash
     *                     for details.
     * @param aLastMutualConnectTime time when node was last in
     *                     contact. In practice we'll fill the
     *                     bucket items published after this time.
     */
    void fillBucket(QList<SendQueueItem>& aSendQueue,
                    const Hash& aStartOfBucket,
                    const Hash& aEndOfBucket,
                    quint32 aLastMutualConnectTime,
                    const Hash& aForNode );

private: // methods
    /**
     * Workhorse of methods @ref sentDbRecordReceived and
     * @ref publishedDbRecordReceived
     * @return true on success
     */
    bool doHandlepublishedOrSentRecord(CaDbRecord& aRecord,
                                       const QByteArray& aSignature,
                                       const QList<quint32>& aBangPath,
                                       const Hash& aFromNode,
                                       bool aWasPublish);
    
    QString persistDbRecordIntoDb(const CaDbRecord &aRecord,
                                  bool aIsNew,
                                  const QByteArray& aSignature,
                                  quint32 aReceivedFrom ) ; 

    /**
     * method for checking if db record is in db already.
     * @param aRecord is the record to check
     * @param aResult gets set to true/false as result
     * @param aTimestampOfOldRecord if aResult get set to true,
     *        this gets set to timestamp of previously persisted record
     */
    QString isRecordNew(const CaDbRecord &aRecord,
                        bool* aResult,
                        quint32* aTimestampOfOldRecord = NULL ) ; 

    /**
     * method tries to verify record. if verification
     * is ok, it sets @ref CaDbRecord::iIsSignatureVerified to 
     * true value. 
     *
     * @param aRecord record to verify
     * @param aSignature signature to verify against. The party
     *        assumed to have signed the record is
     *        @ref CaDbRecord::iSenderHash.
     * @param aListOfGoodSignatures contains list of records
     *        that verify ok. Method adds record to list if it
     *        is previously not verified and now verifies ok. 
     * @return true if signature was good or there was no profile
     *         to check against. returns false if profile was there but 
     *         signature did not verify. 
     */
    bool tryVerifyRecord(CaDbRecord& aRecord,
                         const QByteArray& aSignature,
                         QList<QPair<Hash,Hash> >& aListOfGoodSignatures ) ;  

    void deleteRecord(const Hash& aRecordId,
                      const Hash& aSenderId) ; 
    void updateRecordVerification(const Hash& aRecordId,
                                  const Hash& aSenderId,
                                  bool aNewVerificationStatus) ; 
    void updateFTS(const CaDbRecord& aRecord,
                   bool aIsNewRecord ) ; 

signals:
    void error(MController::CAErrorSituation aError,
               const QString& aExplanation) ;
    /** emitted when new record is received */
    void contentReceived(const Hash& aHashOfContent,
                         const Hash& aHashOfCollection,
                         const ProtocolItemType aTypeOfReceivedContent) ;

} ;
#endif
