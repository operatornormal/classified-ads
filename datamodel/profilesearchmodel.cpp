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

#include <QSqlQuery>
#include <QSqlError>
#include "profilesearchmodel.h"
#include "../log.h"
#include "../mcontroller.h"

ProfileSearchModel::ProfileSearchModel(Model& aModel) : 
  iModel(aModel) {
}

ProfileSearchModel::~ProfileSearchModel() {
}

int ProfileSearchModel::rowCount(const QModelIndex& ) const
{
    return iProfiles.size();
}
 
QVariant ProfileSearchModel::data(const QModelIndex &index, int role) const
{
    if(!index.isValid())
        return QVariant();
 
    if(role == Qt::DisplayRole)
    {
      return iProfiles.at(index.row()).second ; 
      //return iProfiles.at(index.row()).toString();
    }
 
    if(role == Qt::DecorationRole)
    {
      // here return QIcon of size 26x26 featuring lenin reading pravda? 
      return QVariant(); 
    }
 
    if(role == Qt::ToolTipRole)
    {
      return iProfiles.at(index.row()).first.toString() ; 
    }
 
    return QVariant();
}

void ProfileSearchModel::setSearchString(const QString& aSearch) {
  iSearchString = aSearch ; 
  // ok .. what next. .. don't search with empty
  if ( iSearchString.length() == 0 ) {
    iProfiles.clear() ; 
  } else {
    performSearch() ; 
  }
  emit dataChanged(createIndex(0,0),createIndex(iProfiles.size(),0)) ;
}

void ProfileSearchModel::performSearch() {
  QSqlQuery query;
  bool ret ;
  ret = query.prepare ("select hash1,hash2,hash3,hash4,hash5,"
		       "display_name from profile where "
		       "display_name like :searchcriteria "
		       "order by display_name limit 100" ) ;
  if ( ret ) {
    QString queryString ("%" + iSearchString + "%") ; 
    query.bindValue(":searchcriteria", queryString.toUtf8() ) ;

    
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
	iProfiles.append(
	QPair<Hash,QString> ( Hash ( hash1,hash2,hash3,hash4,hash5),
			      QString::fromUtf8(query.value(5).toByteArray())) );
      }
    }
  } else {
    QLOG_STR(query.lastError().text() + " "+ __FILE__ + QString::number(__LINE__)) ;
    emit error(MController::DbTransactionError, query.lastError().text()) ;
  }
}
