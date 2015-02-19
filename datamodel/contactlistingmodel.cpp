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
#include <QSize>
#include "../log.h"
#include "../mcontroller.h"
#include "model.h"
#include "contactlistingmodel.h"

ContactListingModel::ContactListingModel(Model& aModel,
				       MController& aController ) : 
  iModel(aModel),
  iController(aController)
{
}

ContactListingModel::~ContactListingModel() {
  LOG_STR("ContactListingModel::~ContactListingModel") ;
}

int ContactListingModel::rowCount(const QModelIndex& ) const
{
  return iContacts.size();
}

int ContactListingModel::columnCount(const QModelIndex& ) const {
  return 3 ; // what are the columns? Hash, NickName, trust?
}
 
QVariant ContactListingModel::data(const QModelIndex &index, int role) const {
  if(!index.isValid())
    return QVariant();

  if ( role == Qt::UserRole ) {
    return iContacts.at(index.row()).iFingerPrint.toQVariant() ;
  }

  switch ( index.column() ) {
  case 0: // peer hash
    if(role == Qt::DisplayRole) {
      return iContacts.at(index.row()).iFingerPrint.toString() ; 
    } else {
      return QVariant();
    }
    break ;
  case 1: // nickname
    if(role == Qt::DisplayRole) {
      return iContacts.at(index.row()).iNickName ; 
    } else {
      return QVariant();     
    } 
    break ;
  case 2: // is trusted
    {
      if(role == Qt::DisplayRole) {
	if ( iContacts.at(index.row()).iIsTrusted ) {
	  return tr("Yes") ;
	} else {
	  return tr("Unknown") ;
	}

      } else {
	return QVariant();
      }
    }
    break ;
  default:
    return QVariant(); // for unknown columns return empty
  }

  return QVariant();
}

QVariant ContactListingModel::headerData ( int section, Qt::Orientation orientation, int role  ) const 
{
  if (orientation != Qt::Horizontal  ) {
    return QVariant();      
  }
  switch ( role ) {
  case Qt::ToolTipRole:
    switch ( section ) 
      {
      case 0:
	return tr("Actual network address (SHA1) of the contact") ;
	break;
      case 1:
	return tr("Locally given nickname ; user may set her own nickname himself") ;
	break;
      case 2:
	return tr("Public statement if this operator to be trusted in transactions") ;
	break;
      default:
	return QVariant();      
	break ;
      }
    break ; 
  case Qt::DisplayRole:
    switch ( section ) 
      {
      case 0:
	return tr("Address") ;
	break;
      case 1:
	return tr("Nickname") ; 
	break ;
      case 2:
	return tr("Publicly trusted") ; 
	break ;
      default:
	return QVariant();      
      }    
    break ;
  case Qt::SizeHintRole:
    switch ( section ) {
    case 0:
      return QSize(300,25) ; 
      break;
    case 1:
      return QSize(450,25) ; 
      break ;
    case 2:
      return QSize(30,25) ; 
      break ;
    default:
      return QVariant();          
    }
    break ; 
  default:
    return QVariant();      
    break ; 
  }

}


void ContactListingModel::newContactAdded(const Contact& aContact) 
{
  // todo ; insert in correct order
  if ( isContactContained(aContact.iFingerPrint) ) {
    for ( int i ( 0 ) ; i < iContacts.size() ; i++ ) {
      if ( iContacts.at(i).iFingerPrint == aContact.iFingerPrint ) {
	iContacts.replace(i,aContact) ; 
	emit dataChanged(createIndex(i,0),createIndex(i,2)) ;
	return ; 
      }
    }
  } else {
    beginInsertRows(QModelIndex(),iContacts.size(),iContacts.size()) ; 
    iContacts.append(aContact) ; 
    endInsertRows() ; 
  }
} 


void ContactListingModel::removeContact(const Hash& aContact) 
{
  for ( int i ( 0 ) ; i < iContacts.size() ; i++ ) {
    if ( iContacts.at(i).iFingerPrint == aContact ) {
      beginRemoveRows(QModelIndex(),i,i) ; 
      iContacts.takeAt(i) ; 
      endRemoveRows() ; 
      return ; 
    }
  }
}

QVariant ContactListingModel::contactsAsQVariant() const {
  // for the qvariant to work, the list of contacts needs to be 
  // of type QList<QVariant> so we need to make the contact to be
  // QVariant
  QList<QVariant> contactsAsQVariantList ; 
  for ( int i ( 0 ) ; i < iContacts.size() ; i++ ) {
    contactsAsQVariantList.append ( iContacts.at(i).asQVariant() ) ; 
  }  
  QLOG_STR("ContactListingModel::contactsAsQVariant size = " + QString::number(contactsAsQVariantList .size())) ; 
  return contactsAsQVariantList ; 
}

QList<Hash> ContactListingModel::trustList() const {
  QList<Hash> retval ;

  foreach (const Contact& c, iContacts) {
    if ( c.iIsTrusted ) {
      retval << c.iFingerPrint ; 
    }
  }

  return retval ; 
}

void ContactListingModel::setContactsFromQVariant(const QVariantList& aContacts)  {
#if QT_VERSION >= 0x050000
  // qt5
  beginResetModel() ; 
#endif
  iContacts.clear() ; 
  QLOG_STR("ContactListingModel::setContactsFromQVariant size = " + QString::number(aContacts.size())) ; 
  for ( int i = 0  ; i < aContacts.size() ; i++ ) {
    iContacts.append(Contact::fromQVariant(aContacts[i].toMap())) ; 
  }

  // ok, we've loaded the contacts, offer each and every one of
  // those to display-cache kept by the controller
  foreach (Contact c, iContacts) {
    QString fingerprintString(c.iFingerPrint.toString() ) ; 
    QLOG_STR("Offering contact " +fingerprintString + " name = " + c.displayName()) ;  ; 
    iController.offerDisplayNameForProfile(c.iFingerPrint,
					   c.displayName()) ; 
  }
  LOG_STR("offerDisplayNameForProfile finished") ; 
					   
#if QT_VERSION >= 0x050000
  endResetModel() ; 
#else    
  reset() ; 
#endif
}

bool ContactListingModel::isContactContained(const Hash& aFingerPrint) const {
  for ( int i = iContacts.size()-1  ; i >= 0 ; i-- ) {
    if ( iContacts[i].iFingerPrint == aFingerPrint ) {
      return true ; 
    }
  }
  return false; 
}


void ContactListingModel::clearContents() {
  LOG_STR("ContactListingModel::clearContents") ; 
#if QT_VERSION >= 0x050000
  // qt5
  beginResetModel() ; 
#endif
  iContacts.clear() ; 
#if QT_VERSION >= 0x050000
  endResetModel() ; 
#else    
  reset() ; 
#endif
}

bool ContactListingModel::contactByFingerPrint(const Hash& aFingerPrint, Contact* aResultingContact) const {
  if ( aResultingContact ) {
    for ( int i = iContacts.size()-1  ; i >= 0 ; i-- ) {
      if ( iContacts[i].iFingerPrint == aFingerPrint ) {
	*aResultingContact = iContacts[i] ;
	return true ; 
      }
    }
  }
  return false; 
}
