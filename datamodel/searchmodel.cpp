/*     -*-C++-*- -*-coding: utf-8-unix;-*-
       Classified Ads is Copyright (c) Antti Järvinen 2013.

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

#include <QSqlQuery>
#include <QSqlError>
#include "searchmodel.h"
#include "../log.h"
#include <QTextDocument>
#include "ca.h"
#include "profile.h"
#include "profilecomment.h"
#include "model.h"
#include <QVariantMap>

static const char *KJsonSearchResultId = "id" ;
static const char *KJsonSearchResultSet = "set" ;
static const char *KJsonSearchItemType = "type" ;
static const char *KJsonSearchItemTypeCA = "ca" ;
static const char *KJsonSearchItemTypeProfile = "prof" ;
static const char *KJsonSearchItemTypeComment = "comm" ;
static const char *KJsonSearchItemFP = "fp" ;
static const char *KJsonSearchItemSnippet = "text" ;


SearchModel::SearchModel(MModelProtocolInterface& aModel,
			 MController& aController) : 
  iModel(aModel),
  iIsFTSSupported(queryIfFTSSupported()),
  iHtmlParser(NULL),
  iController(aController) {
  iHtmlParser = new QTextDocument() ; 
}

SearchModel::~SearchModel() {
  delete iHtmlParser ; 
}

int SearchModel::rowCount(const QModelIndex& ) const
{
  return iDisplayedResults.count() ; 
}
 
QVariant SearchModel::data(const QModelIndex &index, int role) const
{
  if(!index.isValid()) {
    return QVariant();
  } else if ( role == Qt::UserRole ) {
    return iDisplayedResults.at(index.row()).iItemHash.toQVariant() ;
  } else if ( role == Qt::UserRole+2 ) {
    return iDisplayedResults.at(index.row()).iFoundFromNode.toQVariant() ;
  } if ( role == Qt::UserRole+1 ) {
    return iDisplayedResults.at(index.row()).iItemType ;
  } else  if ( role == Qt::DisplayRole ) {
    return iDisplayedResults.at(index.row()).iDisplayName ;
  } else {
    return QVariant();
  }
}

// datamodel better be locked when calling this method
void SearchModel::setSearchString(const QString& aSearch,
				  bool aSearchAds,
				  bool aSearchProfiles,
				  bool aSearchComments,
				  bool aNetworkSearch) {
#if QT_VERSION >= 0x050000
  // qt5
  beginResetModel() ; 
#endif
  iSearchString = aSearch ; 
  iSearchAds = aSearchAds ;
  iSearchProfiles = aSearchProfiles ; 
  iSearchComments = aSearchComments ; 
  iDisplayedResults.clear() ; 
  if ( aSearch.length() > 0 &&
       ( aSearchAds || aSearchProfiles || aSearchComments ) ) {
    // ok, user has something to search for
    if ( aNetworkSearch == false ) {
      // then search locally
      iDisplayedResults = performSearch(aSearch,
					aSearchAds,
					aSearchProfiles, 
					aSearchComments) ; 
    } else {
      // spam search to every connected node. Here don't use the normal "net request
      // queue" but instead short-circuit the query into each connected nodes 
      // send queue - will be faster, will query only already-connected nodes. this 
      // is all right.
      SendQueueItem itemToSpam ; 
      iSearchId.calculate(aSearch.toUtf8()) ;
      itemToSpam.iItemType = RequestForSearchSend ;
      itemToSpam.iHash = iSearchId ; 
      QLOG_STR("Starting network search, search id = " + QString::number(iSearchId.iHash160bits[0]) + " model = " + objectName()) ; 
      LOG_STR(QString("0x%1 ").arg((qulonglong)this, 8) +"Searchmodel ptr") ;
      const QList <Connection *>& currentConnections ( iController.model().getConnections() ) ;
      for ( int i = currentConnections.size()-1 ; i >= 0 ; i-- ) {
	Connection& c ( *(currentConnections[i]) ) ;
	if ( c.connectionState() == Connection::Open &&
	     c.iNumberOfPacketsReceived > 1 ) {
	  c.iSendQueue.append( itemToSpam ) ;
	}
      }
    }
  }
#if QT_VERSION >= 0x050000
  endResetModel() ; 
#else    
  reset() ; 
#endif
}

bool SearchModel::queryIfFTSSupported() {
  bool retval ( false ) ; 
  QSqlQuery query ; 
  if ( query.prepare ("select trim(upper(sqlite_compileoption_get(:optNumber)))" ) ) {
    int i = 0 ; 
    QString queryRes ; 
    do {
      queryRes = QString() ; 
      query.bindValue(":optNumber",i);
      if ( query.exec() ) {
	if ( query.next() && !query.isNull(0) ) {
	  queryRes = query.value(0).toString() ;
	  if ( queryRes == "ENABLE_FTS3" ) {
	    retval = true ; 
	    QLOG_STR("FTS is supported") ; 
	    break ; 
	  } else {
	    QLOG_STR("Sqlite option #" + QString::number(i) + ":'"+queryRes+"'") ;
	  }
	} else {
	  break ; 
	}
      } else {
	QLOG_STR(query.lastError().text() + " "+ __FILE__ + QString::number(__LINE__)) ;
	break ; // out of loop
      }
      i++ ; 
    } while ( queryRes.length() > 0 ) ;
  }
  return retval ; 
}

void SearchModel::createFTSTables() {
  QSqlQuery q;
  bool ret ; 
  ret = q.exec("create virtual table classified_ad_search using fts3(senderhash,sendername,subject,text)") ;
  if (!ret) {
    QLOG_STR("FTS3 table classified_ad_search creation: " + q.lastError().text()) ; 
    return ;
  }
  // this trigger will handle unfairly documents where there is 
  // hash collision in the first 32 bits, e.g. no full hash collision
  // needed. 
  //
  // but this simplifies things, we don't need to fetch rowid after
  // insert and it is only 1/(2^32) chance so lets go. should be fairly rare
  // and then we lose only the search, the document is still there. 
  ret = q.exec("CREATE TRIGGER classified_ad_search_del BEFORE DELETE ON classified_ad BEGIN"
	       "  DELETE FROM classified_ad_search WHERE docid=old.hash1;"
	       "END;" ) ;
  if (!ret) {
    QLOG_STR("FTS3 classified_ad_search_del trigger: " + q.lastError().text()) ; 
    return ;
  }

  // then search for profile:
  ret = q.exec("create virtual table profile_search using fts3(profilehash,nickname,greetingtext,firstname,familyname,city,btc,state)") ;
  if (!ret) {
    QLOG_STR("FTS3 table profile_search creation: " + q.lastError().text()) ; 
    return ;
  }
  ret = q.exec("CREATE TRIGGER profile_search_del BEFORE DELETE ON profile BEGIN"
	       "  DELETE FROM profile_search WHERE docid=old.hash1;"
	       "END;" ) ;
  if (!ret) {
    QLOG_STR("FTS3 profile_search_del trigger: " + q.lastError().text()) ; 
    return ;
  }

  // then search for profilecomment:
  ret = q.exec("create virtual table profilecomment_search using fts3(profilehash,commentorhash,comment,commentsubject,commentornickname)") ;
  if (!ret) {
    QLOG_STR("FTS3 table profilecomment_search creation: " + q.lastError().text()) ; 
    return ;
  }
  ret = q.exec("CREATE TRIGGER profilecomment_search_del BEFORE DELETE ON profilecomment BEGIN"
	       "  DELETE FROM profilecomment_search WHERE docid=old.hash1;"
	       "END;" ) ;
  if (!ret) {
    QLOG_STR("FTS3 classified_ad_search_del trigger: " + q.lastError().text()) ; 
    return ;
  }
  return ;
}

void SearchModel::indexClassifiedAd(const CA& aCa) {
  if ( iIsFTSSupported && aCa.iMessageText.length() > 0  ) {
    // dig out message text in plain text
    iHtmlParser->setHtml(aCa.iMessageText ) ; 
    QString plainText(iHtmlParser->toPlainText()) ;
    iHtmlParser->clear() ; 
    bool operation_success (false) ; 
    if ( plainText.length() > 0 ) {
      QSqlQuery ins ;
      operation_success= ins.prepare("insert into classified_ad_search("
				     "docid,senderhash,sendername,subject,text) values ("
				     ":hash1,:senderhash,:sendername,:subject,:text)") ;
      if ( operation_success ) {
	ins.bindValue(":hash1", aCa.iFingerPrint.iHash160bits[0]);
	ins.bindValue(":senderhash", aCa.iSenderHash.toString()) ;
	ins.bindValue(":sendername", aCa.iSenderName.length() > 0 ? aCa.iSenderName.toUtf8() : aCa.iSenderHash.toString()) ;
	ins.bindValue(":subject", aCa.iSubject.toUtf8()) ;
	ins.bindValue(":text", plainText.toUtf8()) ;
	if ( ins.exec() == false ) {
	  // if indexing fails, put out only log message, because we'll want
	  // to have the actual document anyway..
	  QLOG_STR("insert into classified_ad_search exec : " + ins.lastError().text()) ; 
	}
      } else {
	QLOG_STR("insert into classified_ad_search : " + ins.lastError().text()) ; 
      }
    }
  }
}

void SearchModel::indexProfileComment(const ProfileComment& aProfileComment) {
  if ( iIsFTSSupported && aProfileComment.iFingerPrint!=KNullHash  ) {
    bool operation_success (false) ; 
    iHtmlParser->setHtml(aProfileComment.iCommentText ) ; 
    QString plainText(iHtmlParser->toPlainText()) ;
    iHtmlParser->clear() ; 
    QSqlQuery ins ;

    operation_success= ins.prepare("insert into profilecomment_search("
				   "docid,profilehash,commentorhash,comment,commentsubject,commentornickname) values ("
				   ":hash1,:profilehash,:commentorhash,:comment,:commentsubject,:commentornickname)") ;

    if ( operation_success ) {
      ins.bindValue(":hash1", aProfileComment.iFingerPrint.iHash160bits[0]);
      ins.bindValue(":profilehash", aProfileComment.iProfileFingerPrint.toString()) ;

      ins.bindValue(":commentorhash",aProfileComment.iCommentorHash.toString()) ;
      ins.bindValue(":comment",plainText.toUtf8()) ;
      ins.bindValue(":commentsubject",aProfileComment.iSubject.toUtf8()) ;
      ins.bindValue(":commentornickname",aProfileComment.iCommentorNickName.toUtf8()) ;

      if ( ins.exec() == false ) {
	// if indexing fails, put out only log message, because we'll want
	// to have the actual document anyway..
	QLOG_STR("insert/update into profilecomment_search exec : " + ins.lastError().text()) ; 
      }
    } else {
      QLOG_STR("insert/update into profilecomment_search : " + ins.lastError().text()) ; 
    }
  }
}


void SearchModel::indexProfile(const Profile& aProfile,
			       bool aWasUpdate) {
  if ( iIsFTSSupported && aProfile.iFingerPrint!=KNullHash  ) {
    bool operation_success (false) ; 
    QSqlQuery ins ;
    if ( aWasUpdate == true ) {
      // was insert ; also do insert into index
      operation_success= ins.prepare("update profile_search set "
				     "profilehash=:profilehash,nickname=:nickname,greetingtext=:greetingtext,firstname=:firstname,familyname=:familyname,city=:city,btc=:btc,state=:state where docid=:hash1") ;
    } else {
      operation_success= ins.prepare("insert into profile_search("
				     "docid,profilehash,nickname,greetingtext,firstname,familyname,city,btc,state) values ("
				     ":hash1,:profilehash,:nickname,:greetingtext,:firstname,:familyname,:city,:btc,:state)") ;
    }
    if ( operation_success ) {
      ins.bindValue(":hash1", aProfile.iFingerPrint.iHash160bits[0]);
      ins.bindValue(":profilehash", aProfile.iFingerPrint.toString()) ;

      ins.bindValue(":nickname",aProfile.iNickName.toUtf8()) ;
      ins.bindValue(":greetingtext",aProfile.iGreetingText.toUtf8()) ;
      ins.bindValue(":firstname",aProfile.iFirstName.toUtf8()) ;
      ins.bindValue(":familyname",aProfile.iFamilyName.toUtf8()) ;
      ins.bindValue(":city",aProfile.iCityCountry.toUtf8()) ;
      ins.bindValue(":btc",aProfile.iBTCAddress.toUtf8()) ;
      ins.bindValue(":state",aProfile.iStateOfTheWorld.toUtf8()) ;

      if ( ins.exec() == false ) {
	// if indexing fails, put out only log message, because we'll want
	// to have the actual document anyway..
	QLOG_STR("insert/update into profile_search exec : " + ins.lastError().text()) ; 
      }
    } else {
      QLOG_STR("insert/update into profile_search : " + ins.lastError().text()) ; 
    }
  }
}


QList<SearchModel::SearchResultItem> SearchModel::performSearch(const QString& aForString,
								bool aSearchAds,
								bool aSearchProfiles,
								bool aSearchComments) {
  QList<SearchModel::SearchResultItem> results ;

  if ( aSearchAds ) {
    QSqlQuery query;
    bool ret ;
    ret = query.prepare ("select classified_ad.hash1,classified_ad.hash2,"
			 "classified_ad.hash3,classified_ad.hash4,"
			 "classified_ad.hash5,"
			 "classified_ad_search.subject from classified_ad , "
			 "classified_ad_search  where "
			 "classified_ad.hash1=classified_ad_search.docid and "
			 "classified_ad_search MATCH :searchcriteria" ) ;
    if ( ret ) {
      query.bindValue(":searchcriteria", aForString.toUtf8() ) ;

      ret = query.exec() ;
      if ( !ret ) {
	QLOG_STR(query.lastError().text() + " "+ __FILE__ + QString::number(__LINE__)) ;
	emit error(MController::DbTransactionError, query.lastError().text()) ;
      } else {
	while ( query.next() ) {
	  quint32 hash1 = query.value(0).toUInt() ;
	  quint32 hash2 = query.value(1).toUInt() ;
	  quint32 hash3 = query.value(2).toUInt() ;
	  quint32 hash4 = query.value(3).toUInt() ;
	  quint32 hash5 = query.value(4).toUInt() ;
	  SearchResultItem i ;
	  i.iItemType = ClassifiedAd ; 
	  i.iItemHash = Hash ( hash1,hash2,hash3,hash4,hash5) ;
	  i.iFoundFromNode = iController.getNode().nodeFingerPrint() ; 
	  if ( query.isNull(5) ) {
	    i.iDisplayName =  i.iItemHash.toString() ; 
	  } else {
	    i.iDisplayName = query.value(5).toString() ; 
	  }

	  results.append(i) ; 
	}
      }
    } else {
      QLOG_STR(query.lastError().text() + " "+ __FILE__ + QString::number(__LINE__)) ;
      emit error(MController::DbTransactionError, query.lastError().text()) ;
    }
  }

  // then search profiles
  if ( aSearchProfiles ) {
    QSqlQuery query;
    bool ret ;
    ret = query.prepare ("select profile.hash1,profile.hash2,"
			 "profile.hash3,profile.hash4,"
			 "profile.hash5,"
			 "profile.display_name from profile , "
			 "profile_search  where "
			 "profile.hash1=profile_search.docid and "
			 "profile_search MATCH :searchcriteria" ) ;
    if ( ret ) {
      query.bindValue(":searchcriteria", aForString.toUtf8() ) ;

      ret = query.exec() ;
      if ( !ret ) {
	QLOG_STR(query.lastError().text() + " "+ __FILE__ + QString::number(__LINE__)) ;
	emit error(MController::DbTransactionError, query.lastError().text()) ;
      } else {
	while ( query.next() ) {
	  quint32 hash1 = query.value(0).toUInt() ;
	  quint32 hash2 = query.value(1).toUInt() ;
	  quint32 hash3 = query.value(2).toUInt() ;
	  quint32 hash4 = query.value(3).toUInt() ;
	  quint32 hash5 = query.value(4).toUInt() ;
	  SearchResultItem i ;
	  i.iItemType = UserProfile ; 
	  i.iItemHash = Hash ( hash1,hash2,hash3,hash4,hash5) ;
	  i.iFoundFromNode = iController.getNode().nodeFingerPrint() ; 
	  if ( query.isNull(5) ) {
	    i.iDisplayName =  i.iItemHash.toString() ; 
	  } else {
	    i.iDisplayName = query.value(5).toString() ; 
	  }
	  results.append(i) ; 
	}
      }
    } else {
      QLOG_STR(query.lastError().text() + " "+ __FILE__ + QString::number(__LINE__)) ;
      emit error(MController::DbTransactionError, query.lastError().text()) ;
    }
  }


  // then search comments
  if ( aSearchComments ) {
    QSqlQuery query;
    bool ret ;
    ret = query.prepare ("select profilecomment.hash1,profilecomment.hash2,"
			 "profilecomment.hash3,profilecomment.hash4,"
			 "profilecomment.hash5,"
			 "profilecomment.display_name from profilecomment , "
			 "profilecomment_search  where "
			 "profilecomment.hash1=profilecomment_search.docid and "
			 "profilecomment_search MATCH :searchcriteria" ) ;
    if ( ret ) {
      query.bindValue(":searchcriteria", aForString.toUtf8() ) ;

      ret = query.exec() ;
      if ( !ret ) {
	QLOG_STR(query.lastError().text() + " "+ __FILE__ + QString::number(__LINE__)) ;
	emit error(MController::DbTransactionError, query.lastError().text()) ;
      } else {
	while ( query.next() ) {
	  quint32 hash1 = query.value(0).toUInt() ;
	  quint32 hash2 = query.value(1).toUInt() ;
	  quint32 hash3 = query.value(2).toUInt() ;
	  quint32 hash4 = query.value(3).toUInt() ;
	  quint32 hash5 = query.value(4).toUInt() ;
	  SearchResultItem i ;
	  i.iItemType = UserProfileComment ; 
	  i.iItemHash = Hash ( hash1,hash2,hash3,hash4,hash5) ;
	  i.iFoundFromNode = iController.getNode().nodeFingerPrint() ; 
	  if ( query.isNull(5) ) {
	    i.iDisplayName =  i.iItemHash.toString() ; 
	  } else {
	    i.iDisplayName = query.value(5).toString() ; 
	  }
	  results.append(i) ; 
	}
      }
    } else {
      QLOG_STR(query.lastError().text() + " "+ __FILE__ + QString::number(__LINE__)) ;
      emit error(MController::DbTransactionError, query.lastError().text()) ;
    }
  }

  return results ; 
}

void SearchModel::getSearchCriteria(QString* aSearchStrPtr,
				    bool* aSearchAdsPtr,
				    bool* aSearchProfilesPtr,
				    bool* aSearchCommentsPtr,
				    Hash* aSearchIdPtr ) const {
  if ( iSearchString.length() > 0 &&
       ( iSearchAds || iSearchProfiles || iSearchComments ) ){
    *aSearchStrPtr = iSearchString ; 
    *aSearchIdPtr = iSearchId ;
    *aSearchAdsPtr = iSearchAds ;
    *aSearchProfilesPtr = iSearchProfiles ;
    *aSearchCommentsPtr = iSearchComments ;
    QLOG_STR("SearchModel::getSearchCriteria, search id = " + QString::number(aSearchIdPtr->iHash160bits[0])) ; 
  } else {
    *aSearchStrPtr = "" ;
    *aSearchIdPtr = KNullHash ;
  } 
}

QVariant SearchModel::serializeSearchResults(const QList<SearchModel::SearchResultItem>& aResults,
					     quint32 aSearchId ) 
{
  QMap<QString,QVariant> m ; 
  m.insert(KJsonSearchResultId, aSearchId) ; 
  QList<QVariant> resultSet ; 
  for ( int i = aResults.size()-1 ; i >= 0 ; i-- ) {
    QMap<QString,QVariant> itemContents ; 
    switch ( aResults[i].iItemType ) {
    case ClassifiedAd:
      itemContents.insert(KJsonSearchItemType,KJsonSearchItemTypeCA) ;
      break ;
    case UserProfile:
      itemContents.insert(KJsonSearchItemType,KJsonSearchItemTypeProfile) ;
      break ;
    case UserProfileComment:
      itemContents.insert(KJsonSearchItemType,KJsonSearchItemTypeComment) ;
      break ;
    default:
      QLOG_STR("Unhandled search result type " + QString::number( aResults[i].iItemType ) ) ; 
      break ; 
    }
    itemContents.insert(KJsonSearchItemFP, aResults[i].iItemHash.toString()) ;
    itemContents.insert(KJsonSearchItemSnippet, aResults[i].iDisplayName.toUtf8()) ;
    QVariant item ( itemContents ) ; 
    resultSet.append(item) ; 
  }
  m.insert(KJsonSearchResultSet, resultSet) ;
  QVariant toplevel(m) ; 
  return toplevel ; 
}

bool SearchModel::deSerializeSearchResults(const QVariantMap& aResultJson, 
					   QList<SearchResultItem>* aResults,
					   quint32* aSearchId ) {
  bool retval ( false ) ;
  if ( aResultJson.contains(KJsonSearchResultId) &&
       aResultJson.contains(KJsonSearchResultSet)) {
    retval = true ; 
    *aSearchId = aResultJson[KJsonSearchResultId].toUInt() ;
    const QList<QVariant> resultset (aResultJson[KJsonSearchResultSet].toList()) ; 
    QLOG_STR("Search results set size = " + QString::number(resultset.size())) ; 
    foreach (const QVariant& item, resultset) {
      const QVariantMap m ( item.toMap() ) ; 
      SearchResultItem resultingItem ; 
      if ( m.contains(KJsonSearchItemType) &&
	   m[KJsonSearchItemType].toString() == KJsonSearchItemTypeCA ) {
	resultingItem.iItemType = ClassifiedAd ; 
      } else if ( m.contains(KJsonSearchItemType) &&
		  m[KJsonSearchItemType].toString() == KJsonSearchItemTypeProfile ) {
	resultingItem.iItemType = UserProfile ; 
      } else if ( m.contains(KJsonSearchItemType) &&
		  m[KJsonSearchItemType].toString() == KJsonSearchItemTypeComment ) {
	resultingItem.iItemType = UserProfileComment ; 
      } else {
	retval = false ; 
	break ; 
      }
      if ( m.contains(KJsonSearchItemFP) ) {
	resultingItem.iItemHash.fromString(reinterpret_cast<const unsigned char *>(m[KJsonSearchItemFP].toByteArray().constData())) ; 
      } else {
	retval = false ; 
	break ;
      }
      if ( m.contains(KJsonSearchItemSnippet) ) {
	QByteArray textOfResult ( m[KJsonSearchItemSnippet].toByteArray() ) ;
	QLOG_STR("Text of search result= " + QString(textOfResult));
	resultingItem.iDisplayName = QString::fromUtf8(textOfResult) ; 
      } else {
	retval = false ; 
	break ;
      }
      aResults->append(resultingItem) ; 
    }
  }
  return retval ;
}

void SearchModel::appendNetworkSearchResults(const QList<SearchResultItem>& aResults,
					     quint32 aSearchId,
					     const Hash& aFromPeer) {
  if ( aSearchId == iSearchId.iHash160bits[0] ) {
    beginInsertRows(QModelIndex(),
		    iDisplayedResults.size(),
		    iDisplayedResults.size()) ; 
    foreach ( const SearchResultItem& i, aResults ) {
      if ( ! iDisplayedResults.contains(i) ) {
	SearchResultItem newItem ( i ) ; 
	newItem.iFoundFromNode = aFromPeer ; 
	iDisplayedResults.append(newItem) ; 
	QLOG_STR("Search result name " + newItem.iDisplayName ) ; 
	QLOG_STR("Search result type " + QString::number(newItem.iItemType) ) ; 
	QLOG_STR("Search result hash " + newItem.iItemHash.toString() ) ; 
      } else {
	QLOG_STR("Offered search result item was already included") ; 
      }
    }
    endInsertRows() ; 
  } else {
    QLOG_STR("appendNetworkSearchResults, our id = " + QString::number(iSearchId.iHash160bits[0]) + 
	     " offered = " + QString::number(aSearchId) + " model = " + objectName()) ; 
    LOG_STR(QString("0x%1 ").arg((qulonglong)this, 8) +"Searchmodel ptr") ;
  }
}

bool SearchModel::SearchResultItemStruct::operator==(const SearchResultItemStruct& aItemToCompare) const {
  return iItemHash == aItemToCompare.iItemHash ; 
}
