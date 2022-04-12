/*     -*-C++-*- -*-coding: utf-8-unix;-*-
  Classified Ads is Copyright (c) Antti Järvinen 2013-2022.

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

#ifndef FULLTEXTSEARCHMODEL_H
#define FULLTEXTSEARCHMODEL_H

#include <QAbstractListModel>
#include "../util/hash.h"
#include "../net/protocol.h"
#include "../mcontroller.h"
#include <QVariantMap> // is actually typedef

class MModelProtocolInterface ;
class CA ;
class Profile ;
class ProfileComment ;
class QTextDocument ; // for parsing html
class QSqlDatabase ; 

/**
 * @brief Model-class for performing text-based searches on objects in db
 * This datamodel part is dual-use ; it inherits listmodel so that
 * it may be used as underlaying data-container for list views.
 * It may also be used as engine of network searches that happen
 * invisibly to user e.g. those queries do not modify the content
 * shown to UI via QAbstractListModel.
 */
class SearchModel: public QAbstractListModel {
    Q_OBJECT
public:
    /**
     * Data structure for search results.
     */
    typedef struct SearchResultItemStruct {
        ProtocolItemType iItemType ;/**< If is CA, profile or comment */
        Hash iItemHash ; /**< Fingerprint of item found in search */
        Hash iFoundFromNode ; /**< Fingerprint of node where item was found */
        QString iDisplayName ; /**< String to show on UI for this result item */
        /** eevil thing, a method in struct. c++ permits. only the comparison */
        bool operator==(const struct SearchResultItemStruct& a) const ;
    } SearchResultItem ;

    SearchModel(MModelProtocolInterface& aModel,
                MController& aController) ;
    ~SearchModel() ;
    /**
     * Method that performs search from local UI. This
     * will change model contens as is shown via
     * QAbstractListModel inheritance
     *
     * aSearch is the string to offer to FTS implementation
     * aSearchAds if true, classified ads are to be searched
     * aSearchProfiles if true, profiles are to be searched
     * aSearchComments if true, profile comments are to be searched.
     * aNetworkSeacrh if true no local search is performed, and datamodel
     *                is emptied with idea that search results from
     *                remote nodes will populate the model
     */
    void setSearchString(const QString& aSearch,
                         bool aSearchAds,
                         bool aSearchProfiles,
                         bool aSearchComments,
                         bool aNetworkSeacrh = false ) ;
    /**
     * re-implemented from QAbstractListModel
     * @return number of rows in list
     */
    virtual int rowCount(const QModelIndex & parent = QModelIndex())  const  ;
    /**
     * re-implemented from QAbstractListModel
     * @return data to display in list
     */
    virtual QVariant data(const QModelIndex & index, int role = Qt::DisplayRole) const ;
    /**
     * method for actually searching the index. datamodel must
     * be locked when this is called
     */
    QList<SearchModel::SearchResultItem> performSearch(const QString& aForString,
            bool aSearchAds,
            bool aSearchProfiles,
            bool aSearchComments) ;

    /**
     * Method called from protocol parser in purpose of appending
     * search results from remote node into datamodel. Model is
     * locked during this call
     */
    void appendNetworkSearchResults(const QList<SearchModel::SearchResultItem>& aResults,
                                    quint32 aSearchId,
                                    const Hash& aFromPeer) ;

    /**
     * Method for serializing search results to be sent to peering
     * node. Usually items sent over to peering nodes like profiles or
     * comments are classes and they will (de)serialize themselves.
     * Search resultset is list of structures so lets have just
     * serialization methods here in model, these are called from
     * protocol formatter/parser.
     *
     * @param aResults actual results of search
     * @param aSearchId id from peering node
     * @return JSon-obj as QVariant
     */
    static QVariant serializeSearchResults(const QList<SearchModel::SearchResultItem>& aResults,
                                           quint32 aSearchId ) ;

    /**
     * opposite to @ref serializeSearchResults
     *
     * @param aResultJson json object containing search results.
     * @param aResults array where contents of aResultJson will be appended
     * @param aSearchId pointer to unsigned that will contain search identifier
     *                  from aResultJson
     * @return true on success
     */
    static bool deSerializeSearchResults(const QVariantMap& aResultJson,
                                         QList<SearchModel::SearchResultItem>* aResults,
                                         quint32* aSearchId ) ;
    /**
     * method for querying if search is supported at all
     * @return true if search is supported
     */
    bool isFTSSupported () const {
        return iIsFTSSupported ;
    }
    /** method for checking if database has support for full text search */
    static bool queryIfFTSSupported(QSqlDatabase aDataBase) ;
    /** method for creating db tables related to FTS */
    static void createFTSTables(QSqlDatabase aDataBase) ;
    /**
     * method for adding a classified ad into index
     * @param aCa is the classified ad to add to index
     * @return none
     */
    void indexClassifiedAd(const CA& aCa) ;

    /**
     * method for adding a profile-comment into index
     * @param aProfileComment is the comment to add to index
     * @return none
     */
    void indexProfileComment(const ProfileComment& aProfileComment) ;

    /**
     * method for adding a profile into index
     * @param aProfile is the profile to add to index
     * @param aWasUpdate if the profile was updated instead of inserted
     * @return none
     */
    void indexProfile(const Profile& aProfile,
                      bool aWasUpdate = false ) ;
    /**
     * method for obtaining the search criteria inside
     */
    void getSearchCriteria(QString* aSearchStrPtr,
                           bool* aSearchAdsPtr,
                           bool* aSearchProfilesPtr,
                           bool* aSearchCommentsPtr,
                           Hash* aSearchIdPtr ) const ;
public: // data
    /**
     * Magic data that is sent over network search if user gives 
     * empty search string. This causes search with no "match" condition
     * in new version and most likely empty search results in old
     * version of this SW without breaking the protocol. Value is
     * assigned in constructor. 
     */
    const QString iMagicalEmptySearchPhrase ; 
signals:
    void error(MController::CAErrorSituation aError,
               const QString& aExplanation) ;
private: // data
    MModelProtocolInterface& iModel ;
    QString iSearchString ;
    const bool iIsFTSSupported ;
    QList<SearchResultItem> iDisplayedResults ;
    MController& iController ;
    Hash iSearchId ; /**< searches may be lengthty: this connects results */
    bool iSearchAds ;
    bool iSearchProfiles ;
    bool iSearchComments ;
} ;
#endif
