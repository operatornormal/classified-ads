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

#include "profile.h"
#include "../util/hash.h"
#include "../log.h"
#ifdef WIN32
#include <QJson/Parser>
#include <QJson/Serializer>
#else
#include <qjson/parser.h>
#include <qjson/serializer.h>
#endif
#include <QVariantMap>
#include <QBuffer>
#include "../mcontroller.h"
#include "model.h"
#include "contentencryptionmodel.h"

static const char *KJSonNickNameElement = "nickName" ;
static const char *KJSonFPElement = "fingerPrint" ;
static const char *KJSonTimeElement = "timeOfPublish" ;
static const char *KJSonProfileVersion = "profileVersion" ;
static const char *KJSonNodeRef = "nodeRef" ;
static const char *KJSonEncyptionKey = "key" ;
static const char *KJSonIsProfilePrivateKey = "isPrivate" ;
static const char *KJSonStateOfTheWorld = "stateOfTheWorld" ;
static const char *KJSonListOfProfileReaders = "readers" ;
static const char *KJSonProfilePic = "image" ;
static const char *KJSonSharedFiles = "sharedPhiles" ;
static const char *KJSonGreetingText = "greeting" ;
static const char *KJSonFirstName = "given" ;
static const char *KJSonFamilyName = "family" ;
static const char *KJSonCityCountry  = "city" ;
static const char *KJSonBTCAddress = "btcaddr" ;

static const int KJSONProfileVersionValue = 1 ; /**< if our format should change? */

Profile::Profile(const Hash& aHash) :
  iFingerPrint(aHash),
  iIsPrivate(false), // initially all profiles are public ; right? 
  iTimeOfPublish(0),
  iNodeOfProfile(NULL) {
  LOG_STR("Profile::Profile()") ;
}

Profile::~Profile() {
  LOG_STR("Profile::~Profile()") ;
  if ( iNodeOfProfile ) {
    delete iNodeOfProfile ; 
  }
}

QByteArray Profile::asJSon(const MController& aController) const {

  // may then be serialized in libqjson-0.7 and 0.8
  QJson::Serializer serializer;
  QByteArray retval ( serializer.serialize(asQVariant(aController)) ) ;
  LOG_STR2("profile %s", qPrintable(QString(retval))) ; 
  return retval ;
}

QVariant Profile::asQVariant(const MController& aController) const 
{
  // first have a map ; that is the top-level JSon-object
  QMap<QString,QVariant> m ;
  m.insert(KJSonFPElement, iFingerPrint.toString()) ; // no non-ascii chars
  if ( iNickName.length() > 0 ) {
    m.insert(KJSonNickNameElement, iNickName.toUtf8()) ;
  }
  if ( iStateOfTheWorld.length() > 0 ) {
    m.insert(KJSonStateOfTheWorld, iStateOfTheWorld.toUtf8()) ;
  }

  if ( iGreetingText.length() > 0 ) {
    m.insert(KJSonGreetingText , iGreetingText.toUtf8()) ;
  }
  if ( iFirstName.length() > 0 ) {
    m.insert(KJSonFirstName, iFirstName.toUtf8()) ;
  }
  if ( iFamilyName.length() > 0 ) {
    m.insert(KJSonFamilyName, iFamilyName.toUtf8()) ;
  }
  if ( iCityCountry.length() > 0 ) {
    m.insert(KJSonCityCountry, iCityCountry.toUtf8()) ;
  }
  if ( iBTCAddress.length() > 0 ) {
    m.insert(KJSonBTCAddress, iBTCAddress.toUtf8()) ;
  }

  m.insert(KJSonTimeElement, iTimeOfPublish) ;
  m.insert(KJSonProfileVersion, KJSONProfileVersionValue) ;
  m.insert(KJSonNodeRef, aController.getNode().asQVariant()) ;
  QByteArray encryptionKey ;
  if(aController.model().contentEncryptionModel().PublicKey(iFingerPrint, encryptionKey)) {
    m.insert(KJSonEncyptionKey, encryptionKey) ;
  }
  m.insert(  KJSonIsProfilePrivateKey, iIsPrivate) ; 
  if ( iSharedFiles.size() > 0 ) {
    QVariantList listOfSharedPhiles ; 
    foreach( const Hash& phile, iSharedFiles )    { 
      QVariant v ;
      v.setValue(phile.toString()) ; 
      listOfSharedPhiles.append(v) ; 
    }
    LOG_STR2("Number of files in profile serialization %d", listOfSharedPhiles.size()) ; 
    m.insert(KJSonSharedFiles, listOfSharedPhiles) ;
  } else {
    LOG_STR("No shared files in profile serialization") ; 
  }
  if ( iIsPrivate && iProfileReaders.size() > 0 ) {
    // insert the readers so replies always get encrypted to
    // every operator in the list
    QVariantList listOfEncryptionKeys;
    LOG_STR2("iProfileReaders has size %d", iProfileReaders.size()) ; 
    foreach( const Hash& reader, iProfileReaders )    { 
      if ( reader != iFingerPrint ) { // self is reader
	LOG_STR2("adding reader %s", qPrintable(reader.toString())) ; 
	encryptionKey.clear() ;  // but has separate placeholder for key
	if(aController.model().contentEncryptionModel().PublicKey(reader, 
								  encryptionKey)) {
	  listOfEncryptionKeys.append(QString(encryptionKey)) ;
	} else {
	  LOG_STR2("reader pubkey was not found %s", qPrintable(reader.toString())) ; 
	}
      } else {
	LOG_STR("not adding self to readers") ; 
      }
    }
    if ( listOfEncryptionKeys.size() > 0 ) {
      m.insert(KJSonListOfProfileReaders, listOfEncryptionKeys) ;
    }
  }
  // ok, profile picture.
  if ( ! iProfilePicture.isNull() ) {
    QByteArray bytes;
    QBuffer buffer(&bytes);
    buffer.open(QIODevice::WriteOnly);
    // PNG might be politically more correct but replacing JPG
    // below with PNG caused .size() of bytes to jump from 18160
    // to 72368 with my test image. This just won't do. 
    iProfilePicture.save(&buffer, "JPG"); // writes pixmap into bytes in JPG format
    LOG_STR2("Size of saved image %d", bytes.size()) ; 
    m.insert( KJSonProfilePic, bytes.toBase64()) ; 
  }

  return ( QVariant (m) ) ; // then put the map inside QVariant 

}

bool Profile::setFromQVariant(const QVariantMap& aJSonAsQVariant,
			      const MController& aController) {
  if ( aJSonAsQVariant.contains(KJSonFPElement) ) {
    QString fingerPrintString = QString::fromUtf8(aJSonAsQVariant[KJSonFPElement].toByteArray()) ;
    if ( fingerPrintString != iFingerPrint.toString() ) {
      // huhuu, inside is different FP from what the key says??
      LOG_STR2("Profile: Fingerprint in json %s" , qPrintable( fingerPrintString)) ;
      return false ;
    }
  } else {
    LOG_STR("Profile: No fingerprint in json??" ) ;
    return false ;
  }

  if (aJSonAsQVariant.contains(KJSonSharedFiles) ) {
    QVariantList listOfSharedPhiles (aJSonAsQVariant[KJSonSharedFiles].toList()) ;  

    QListIterator<QVariant> i(listOfSharedPhiles);
    iSharedFiles.clear() ; 
    while (i.hasNext()) {
      Hash h ; 
      h.fromString((const unsigned char*)(i.next().toString().toLatin1().constData())) ; 
      iSharedFiles.append(h);
    }
  }

  if ( aJSonAsQVariant.contains(KJSonNickNameElement) ) {
    iNickName = QString::fromUtf8(aJSonAsQVariant[KJSonNickNameElement].toByteArray()) ;
  } else {
    iNickName.clear() ;
  }
  if ( aJSonAsQVariant.contains(KJSonGreetingText) ) {
    iGreetingText = QString::fromUtf8(aJSonAsQVariant[KJSonGreetingText].toByteArray()) ;
  }
  if ( aJSonAsQVariant.contains(KJSonFirstName) ) {
    iFirstName= QString::fromUtf8(aJSonAsQVariant[KJSonFirstName].toByteArray()) ;
  }
  if ( aJSonAsQVariant.contains(KJSonFamilyName) ) {
    iFamilyName= QString::fromUtf8(aJSonAsQVariant[KJSonFamilyName].toByteArray()) ;
  }
  if ( aJSonAsQVariant.contains(KJSonCityCountry) ) {
    iCityCountry=QString::fromUtf8(aJSonAsQVariant[KJSonCityCountry].toByteArray()) ;
  }
  if ( aJSonAsQVariant.contains(KJSonBTCAddress) ) {
    iBTCAddress=QString::fromUtf8(aJSonAsQVariant[KJSonBTCAddress].toByteArray()) ;
  }


  if ( aJSonAsQVariant.contains(KJSonTimeElement) ) {
    iTimeOfPublish = aJSonAsQVariant[KJSonTimeElement].toUInt() ;
  }
  if ( aJSonAsQVariant.contains(KJSonIsProfilePrivateKey) ) {
    iIsPrivate = aJSonAsQVariant[KJSonIsProfilePrivateKey].toBool() ; 
    if ( iIsPrivate ) {
      // add self to profile readers ; later used by encryption
      // routine..
      iProfileReaders.append(iFingerPrint) ; 
      // try to get possible keys of readers..
      if ( aJSonAsQVariant.contains(KJSonListOfProfileReaders) ) {
	QVariantList keysOfProfileReaders ( aJSonAsQVariant[KJSonListOfProfileReaders].toList() ) ;
	LOG_STR2("Number of profile readers is %d" , keysOfProfileReaders.size()) ;
	foreach ( QVariant pemBytesOfKeyOfAReader , keysOfProfileReaders ) {
	  QByteArray bytesOfKey ( pemBytesOfKeyOfAReader.toByteArray() ) ; 
	  Hash hashOfReader ( aController.model().contentEncryptionModel().hashOfPublicKey(bytesOfKey) ) ;
	  if ( hashOfReader != KNullHash ) {
	    if ( ! ( iProfileReaders.contains ( hashOfReader ) ) ) {
	      // ever possible to have duplicates? sw bug?
	      iProfileReaders.append(hashOfReader) ; 
	    }
	    aController.model().contentEncryptionModel().insertOrUpdatePublicKey(bytesOfKey,hashOfReader) ; 
	  } else {
	    LOG_STR2("KNullHash as profile reader out of key %s", qPrintable(QString(bytesOfKey))) ;
	  }
	}
      }
    }
  } else {
    iIsPrivate = false ; 
  }
  if ( aJSonAsQVariant.contains(KJSonProfilePic) ) {
    QByteArray imageBytes ( QByteArray::fromBase64(aJSonAsQVariant[KJSonProfilePic].toByteArray()) ) ;
    if ( imageBytes.size() > 0 ) {
      QPixmap profileImage ;
      if ( profileImage.loadFromData(imageBytes) ) {
	imageBytes.clear() ; 
	iProfilePicture = profileImage ; 
      }
    }
  }
  if ( aJSonAsQVariant.contains(KJSonNodeRef ) ) {
    iNodeOfProfile = Node::fromQVariant (aJSonAsQVariant[KJSonNodeRef].toMap(),false) ;
  }

  if (  aJSonAsQVariant.contains(KJSonStateOfTheWorld) ) {
    iStateOfTheWorld  = QString::fromUtf8(aJSonAsQVariant[KJSonStateOfTheWorld].toByteArray()) ;
  }

  return true ; 
}

bool Profile::fromJSon(const QByteArray &aJSonBytes,
		       const MController& aController ) {
  QJson::Parser parser;
  bool ok;

  QVariantMap result = parser.parse (aJSonBytes, &ok).toMap();
  if (!ok) {
    return false ;
  } else {
    ok = setFromQVariant(result,aController) ; 
  }

  return ok ;
}

QString Profile::displayName() const {
  QString retval ; 
  if ( iIsPrivate ) {
    retval = iFingerPrint.toString() ; 
  } else {
    if ( iNickName.length() > 0  ) {
      retval = iNickName; 
    } else {
      if ( iFirstName.length() > 0 ) {
	retval = iFirstName ; 
	if ( iFamilyName.length() > 0 ) {
	  retval = retval + " " + iFamilyName ; 
	}
      } else {
	if ( iFamilyName.length() > 0 ) {
	  retval =  iFamilyName ; 
	}  else {
	  // no nickname, no first name, no family name..
	  retval = iFingerPrint.toString() ; 
	}
      }
    }
    if ( retval.length() > 40 ) {
      retval = retval.left(40) ; 
    }
  }
  return retval ;
}
