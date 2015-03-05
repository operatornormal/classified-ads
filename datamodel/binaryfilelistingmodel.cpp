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
#include "binaryfilelistingmodel.h"
#include "../log.h"
#include "binaryfile.h"

BinaryFileListingModel::BinaryFileListingModel(QList<Hash>& aFilesToList) : 
  iFilesToList(aFilesToList){
  for ( int i = 0 ; i < iFilesToList.size() ; i++ ) {
    updateFileDataInArray( iFilesToList[i],false ) ;
  }
  beginInsertColumns(QModelIndex(),0,0) ;
  endInsertColumns() ; 
  beginInsertRows(QModelIndex(),0,iNamesAndFingerPrints.size()) ; 
  endInsertRows() ; 
  emit dataChanged(createIndex( 0  ,0),
		   createIndex( iNamesAndFingerPrints.size() ,0)) ;
}

BinaryFileListingModel::~BinaryFileListingModel() {
}

int BinaryFileListingModel::rowCount(const QModelIndex& ) const
{
  return iNamesAndFingerPrints.size();
}

int BinaryFileListingModel::columnCount(const QModelIndex& ) const
{
  return 1 ; // for the time being .. maybe this could be configurable?
  // add one column for possible icon
  // add another for optional delete-button with a drawing delegate?
}
 
QVariant BinaryFileListingModel::data(const QModelIndex &index, int role) const
{
    if(!index.isValid())
        return QVariant();
 
    if(role == Qt::DisplayRole)
    {
      return iNamesAndFingerPrints.at(index.row()).second ; 
      //return iProfiles.at(index.row()).toString();
    }
 
    if(role == Qt::DecorationRole)
    {
      // here return QIcon of size 26x26 featuring lenin reading pravda? 
      // maybe iLocalStorageStatus could be used here?
      return QVariant(); 
    }
 
    if(role == Qt::ToolTipRole)
    {
      return iNamesAndFingerPrints.at(index.row()).first.toString() ; 
    }
 
    return QVariant();
}



void BinaryFileListingModel::addFile(const Hash& aFingerPrint) {
  if ( ! iFilesToList.contains(aFingerPrint) ) {
    iFilesToList.append(aFingerPrint) ;
    updateFileDataInArray(aFingerPrint,true) ;
  }
}

void BinaryFileListingModel::removeFile(const Hash& aFingerPrint) {
  int i ; 
  if (iFilesToList.contains(aFingerPrint) ) {
    iFilesToList.removeOne(aFingerPrint) ;
    for ( i = 0 ; i < iNamesAndFingerPrints.size() ; i++ ) {
      if ( iNamesAndFingerPrints.at(i).first == aFingerPrint ) {
	beginRemoveRows(QModelIndex(),i,i) ; 
	iNamesAndFingerPrints.removeAt(i);
	endRemoveRows() ; 
	break ; 
      }
    }
  }
}

void BinaryFileListingModel::updateFileDataInArray( const Hash& aFingerPrint,bool aEmit ) {
  bool found = false ;
  int i ; 
  for ( i = 0 ; i < iNamesAndFingerPrints.size() ; i++ ) {
    if ( iNamesAndFingerPrints.at(i).first == aFingerPrint ) {
      iNamesAndFingerPrints.replace(i,
				    QPair<Hash,QString> ( aFingerPrint,
							  fileDisplayNameByFingerPrint(aFingerPrint) ) );
      found = true ; 
      break ; 
    }
  }
  
  if(!found) {
    // was not contained
    beginInsertRows(QModelIndex(),iNamesAndFingerPrints.size(),iNamesAndFingerPrints.size()) ; 
    iNamesAndFingerPrints.append( QPair<Hash,QString> ( aFingerPrint,
							fileDisplayNameByFingerPrint(aFingerPrint) ));
    i = iNamesAndFingerPrints.size() ;
    endInsertRows() ; 
    found = true ; 
  }
  
  if ( aEmit && found ) {
    int minRow = i>0                            ? i-1                         : 0 ;
    int maxRow = i>=iNamesAndFingerPrints.size()? iNamesAndFingerPrints.size():i+1 ;
    LOG_STR2("Emit from row %d", minRow) ; 
    LOG_STR2("Emit to row %d", maxRow) ; 
    emit dataChanged(createIndex( minRow,0),
		     createIndex( maxRow,0));
  }
}

QString BinaryFileListingModel::fileDisplayNameByFingerPrint(const Hash& aFingerPrint) {
  // some kind of caching could be in order here? we'll repeatedly list
  // same profilenames, querying db for each and every listing is waste
  // of resources..
  QSqlQuery query;
  bool ret ;
  QString retval ; 
  ret = query.prepare (
		       "select display_name from binaryfile where "
		       "hash1=:hash1 and hash2=:hash2 and hash3=:hash3 "
		       "and hash4=:hash4 and hash5=:hash5 " ) ;
  if ( ret ) {
    query.bindValue(":hash1", aFingerPrint.iHash160bits[0]);
    query.bindValue(":hash2", aFingerPrint.iHash160bits[1]);
    query.bindValue(":hash3", aFingerPrint.iHash160bits[2]);
    query.bindValue(":hash4", aFingerPrint.iHash160bits[3]);
    query.bindValue(":hash5", aFingerPrint.iHash160bits[4]);
    ret = query.exec() ;
    if ( !ret ) {
      emit error(MController::DbTransactionError, query.lastError().text()) ;
    } else {
      if ( query.next() ) {
	retval = QString::fromUtf8(query.value(0).toByteArray()) ; 
      }
    }
  } else {
    emit error(MController::DbTransactionError, query.lastError().text()) ;
  }
  if ( retval.length() == 0 ) {
    retval = aFingerPrint.toString() ; 
  }
  return retval ; 
}

QVariant BinaryFileListingModel::headerData(int aSection, Qt::Orientation orientation, int role) const {
  if (role != Qt::DisplayRole)
    return QVariant();

  if (orientation == Qt::Horizontal) {
    switch (aSection) {
    case 0:
      return QVariant(tr("Name or fingerprint of file"));
      break ; 
    default:
      return QVariant();
    }
  }
  return QVariant();
}

void BinaryFileListingModel::clear() {
#if QT_VERSION >= 0x050000
  // qt5
  beginResetModel() ; 
#endif
  iFilesToList.clear() ; 
  iNamesAndFingerPrints.clear() ; 
#if QT_VERSION >= 0x050000
  endResetModel() ; 
#else    
  reset() ; 
#endif
}
