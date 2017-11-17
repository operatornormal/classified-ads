/*    -*-C++-*- -*-coding: utf-8-unix;-*-
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

#ifndef CA_DBRECORD_H
#define CA_DBRECORD_H
#include <QByteArray>
#include "../util/hash.h" // for class Hash  

/**
 * @brief Data record in distributed database
 *
 * In same manner as classified ads are published in groups,
 * it is possible to publish records in a collection. 
 * This class carries one record in a collection. Db records
 * may be accessed via TCL interface, alternative APIs are
 * possible too. 
 */
class CaDbRecord {
public: // data type definitions
    /**
     * Data structure for expressing db record search conditions
     */
    typedef struct SearchStructure {
        Hash iFromCollection ; /**< Search from which collection */
        Hash iById ; /**< Exact db record identifier */
        quint32 iModifiedAfter; /**< Earliest permissible modification time */
        quint32 iModifiedBefore;/**< Latest permissible modification time */
        /** Smallest permissible special search number */ 
        qint64 iByHavingNumberMoreThan; 
        /** Biggest permissible special search number */ 
        qint64 iByHavingNumberLessThan;
        QString iBySearchPhrase; /**< Search phrase */
        Hash iBySender ; /**< Limits results by publishing profile */
        /** Equivalence-operator */
        bool operator==(const struct SearchStructure& aItemToCompare) const ; 
        /** 
         * Less-than-operator. If from 2 queries A and B the result-set 
         * if A is subset of resultset of B, then A<B. Practical example
         * is search by phrase "the king has left the building" compared
         * to search by phrase "king" where resultset of first phrase includes
         * all records with words "the", "king", "has" etc. while search
         * by phrase "king" only includes all records with words "king". 
         * Because resultset from first searh includes all records
         * with only word "king", thus "king"<"the king has left the building". 
         */
        bool operator<(const struct SearchStructure& aItemToCompare) const ; 
        QByteArray asJSon() const ; /**< returns db record search as JSon stream */
        bool fromJSon(const QByteArray& aJSon) ; /**< initializes values from JSon */
    } SearchTerms ; 
public: // methods
    CaDbRecord() ; /**< constructor */
    ~CaDbRecord() ; /**< destructor */

    QByteArray asJSon() const ; /**< returns db record data as JSon stream */
    bool fromJSon(const QByteArray &aJSonBytes) ; /**< parses json into members*/
    /**
     * About unique id given to db record. Value is assigned when record
     * is first persisted via @ref CaDbRecordModel::publishDbRecord and
     * then never changed no matter what happens to the record
     */
    Hash iRecordId ; 
    /**
     * Records belong to collections. Collection is a Hash usually obtained
     * by calculating SHA1 over a string. 
     */
    Hash iCollectionId ;
    /**
     * Fingerprint of @ref Profile that published the record
     */
    Hash iSenderHash ; 
    /**
     * Helper-string for helping in finding the records once they
     * are published and stored. Because actual record data is only
     * an array of bytes and classified ads does not try to parse 
     * or interpret that data, user of database records may want to
     * assign here any number of strings that may later be used
     * in @ref CaDbRecordModel::searchRecords to find records somehow
     * relevant to business-case in hand.
     */
    QString iSearchPhrase ; 
    /**
     * Similar to @ref iSearchPhrase but a number.
     */
    qint64 iSearchNumber ;
    /**
     * Actual record data. May be empty. Max size is limited by classified
     * ads network object size, slightly less than 2MB. 
     */
    QByteArray iData ;
    /**
     * true if iData needs to be de-crypted before use
     */
    bool iIsEncrypted ; 

    /**
     * time when record was last time published
     */
    quint32 iTimeOfPublish ;
    /**
     * True if signature is checked against profile key.
     * It is possible to receive database records without having
     * key of the profile that published the records. If this
     * happens we have a record that we can't verify so
     * mark the situation also here: we can try to pull the
     * profile when record is used and try to verify it then.
     */
    bool iIsSignatureVerified ; 
    /**
     * Signature of data. Signature is calculated over iData ( may be empty )
     * and string-presentation of recordId/collectionId/senderHash 
     * -string-presentations. See implementation of method 
     * @ref CaDbRecordModel::tryVerifyRecord for details. When record is
     * initially published, this is naturally empty, when record is
     * modified and gets re-published, the publish -process will calculate
     * signature again. 
     */
    QByteArray iSignature ;
} ;
#endif
