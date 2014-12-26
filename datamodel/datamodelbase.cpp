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

#include "datamodelbase.h"
#include "../log.h"
#include <QObject>
#include <QSqlQuery>
#include <QSqlError>

ModelBase::ModelBase(QString aDataTableName,
		     unsigned aMaxRowsToKeep ) 
  : iDataTableName(aDataTableName),
    iMaxRowsToKeep(0),
    iCurrentDbTableRowCount(0) {
  unsigned maxRowsFromDb ( getMaxRowsToKeep() ) ; 
  if ( maxRowsFromDb == 0 ) {
    setMaxRowsToKeep(aMaxRowsToKeep) ; // side-effectively sets iMaxRowsToKeep 
  } else {
    iMaxRowsToKeep  = maxRowsFromDb  ; 
  }
  updateDbTableRowCount() ; 
}


ModelBase::~ModelBase() {
}

bool ModelBase::setTimeLastReference(const Hash& aObjectFingerPrint,
				     quint32 aTimeWhenLastReferenced) 
{
  QLOG_STR("ModelBase::setTimeLastReference table " + iDataTableName ) ; 
  bool retval (false) ; 
  QSqlQuery query;
  retval = query.prepare ("update "+iDataTableName+" set  time_last_reference=:time_last_reference where hash1 = :hash1 and hash2 = :hash2 and hash3 = :hash3 and hash4 = :hash4 and hash5 = :hash5" ) ;
  if ( retval ) {
    query.bindValue(":hash1", aObjectFingerPrint.iHash160bits[0]);
    query.bindValue(":hash2", aObjectFingerPrint.iHash160bits[1]);
    query.bindValue(":hash3", aObjectFingerPrint.iHash160bits[2]);
    query.bindValue(":hash4", aObjectFingerPrint.iHash160bits[3]);
    query.bindValue(":hash5", aObjectFingerPrint.iHash160bits[4]);
    query.bindValue(":time_last_reference", aTimeWhenLastReferenced);
    retval = query.exec() ;
  }
  if ( !retval ) {
    QLOG_STR(query.lastError().text() + " "+ __FILE__ + QString::number(__LINE__)) ;
    emit error(MController::DbTransactionError, query.lastError().text()) ;
  }
  return retval ; 
} 

unsigned ModelBase::getMaxRowsToKeep() {
  QSqlQuery query;
  bool retval ;
  unsigned ret ( 0 ) ; 
  retval = query.prepare ("select "+iDataTableName+"_maxrows from settings");
  if ( retval && (retval = query.exec()) && query.next ()) {
    if ( !query.isNull(0) ) {
      ret = query.value(0).toUInt() ; 
    }
  }
  if ( !retval ) {
    QLOG_STR(query.lastError().text() + " "+ __FILE__ + QString::number(__LINE__)) ;
    emit error(MController::DbTransactionError, query.lastError().text()) ;
  }
  return ret ; 
}

void ModelBase::setMaxRowsToKeep(unsigned aRows) {
  QSqlQuery query;
  bool retval ; 
  retval = query.prepare ("update settings set "+iDataTableName+"_maxrows = :rows");
  if ( retval ) {
    query.bindValue(":rows", aRows) ; 
    retval = query.exec() ;
    iMaxRowsToKeep = aRows ; 
  }
  if ( !retval ) {
    QLOG_STR(query.lastError().text() + " "+ __FILE__ + QString::number(__LINE__)) ;
    emit error(MController::DbTransactionError, query.lastError().text()) ;
  }
}

void ModelBase::updateDbTableRowCount() {
  QSqlQuery query;
  bool retval ;
  retval = query.prepare ("select count(hash1) from "+iDataTableName);
  if ( retval && (retval = query.exec()) && query.next ()) {
    if ( !query.isNull(0) ) {
      iCurrentDbTableRowCount = query.value(0).toUInt() ; 
    }
  }
  if ( !retval ) {
    QLOG_STR(query.lastError().text() + " "+ __FILE__ + QString::number(__LINE__)) ;
    emit error(MController::DbTransactionError, query.lastError().text()) ;
  }
  QLOG_STR("Rows in db table " + iDataTableName + " = " + QString::number(iCurrentDbTableRowCount)) ; 
}

void ModelBase::truncateDataTableToMaxRows(void)  
{
  QLOG_STR("truncateDataTableToMaxRows in, rows in db table " + iDataTableName + " = " + QString::number(iCurrentDbTableRowCount)) ; 
  QSqlQuery q ; 
  q.exec("begin transaction") ; 
  while(iCurrentDbTableRowCount > iMaxRowsToKeep) {
    if ( deleteOldestDataRowInTable() == false ) {
      break ; // out of the loop, if this happens we would get eternal loop
    }
  }
  q.exec("commit") ; 
  QLOG_STR("truncateDataTableToMaxRows out, rows in db table " + iDataTableName + " = " + QString::number(iCurrentDbTableRowCount)) ; 
}

bool ModelBase::deleteOldestDataRowInTable() {
  bool ret = false ;
  QSqlQuery query;
  // odd-looking sqlite-specific operation here. intention is to 
  // delete exactly one row from table. we could delete several but
  // then we'll lose count because qt documentation says that
  // numRowsAffected() works only for select statements .. that might
  // be true for old sqlite versions. anyway, this should work
  // regardless of sqlite versions
  ret = query.exec("delete from  " + iDataTableName + "  where rowid = ( select max(rowid) from  (select rowid from " + iDataTableName + " where time_last_reference = ( select min(time_last_reference) from  " + iDataTableName + "  ) ) ) ") ;
  if ( ret  ) {
    iCurrentDbTableRowCount-- ;
  }
  return ret ; 
}


