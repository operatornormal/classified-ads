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
#ifdef WIN32
// include this as very first item?
#define NOMINMAX 
#include <WinSock2.h>
#endif
#include "binaryfilemodel.h"
#include "../log.h"
#include "../util/hash.h"
#include "model.h"
#include <QSqlQuery>
#include <QSqlError>
#include "binaryfile.h"
#include "profile.h"
#include "contentencryptionmodel.h"
#ifdef WIN32
#include <QJson/Parser>
#include <QJson/Serializer>
#else
#include <qjson/parser.h>
#include <qjson/serializer.h>
#include <arpa/inet.h>
#endif
#include "mmodelprotocolinterface.h"
#include "profilemodel.h"
#include "const.h"

BinaryFileModel::BinaryFileModel(MController *aController,
				 const MModelProtocolInterface &aModel) 
  : ModelBase("binaryfile",KMaxRowsInTableBinaryFile),
    iController(aController),
    iModel(aModel) {
  LOG_STR("BinaryFileModel::BinaryFileModel()") ;
  connect(this,
          SIGNAL(  error(MController::CAErrorSituation,
                         const QString&) ),
          iController,
          SLOT(handleError(MController::CAErrorSituation,
                           const QString&)),
          Qt::QueuedConnection ) ;

}



BinaryFileModel::~BinaryFileModel() {
  LOG_STR("BinaryFileModel::~BinaryFileModel()") ;
  iController = NULL ; // not owned, just set null
}

Hash BinaryFileModel::publishBinaryFile(const Profile& aPublishingProfile,
					const QString& aFileName,
					const QString& aDescription,
					const QString& aMimeType,
					const QByteArray& aContents,
					bool aIsCompressed,
					bool aNoEncryption,
					const QList<Hash>* aBinaryRecipientList) {
  LOG_STR("BinaryFileModel::publishBinaryFile()") ;

  Hash retval ;
  bool operation_success (false) ;
  Hash contentFingerPrint ;
  QByteArray encryptedContent ;
  bool encryption_was_used (false) ; 

  QList<Hash> namedRecipientsList ;
  if ( aBinaryRecipientList != NULL ) {
    namedRecipientsList.append(*aBinaryRecipientList) ; 
    // encrypt to self too, or sender can not read the attachment
    if ( ! namedRecipientsList.contains(aPublishingProfile.iFingerPrint) ) {
      namedRecipientsList.append(aPublishingProfile.iFingerPrint) ; 
    }
  }
  if ( ( aNoEncryption == false && aPublishingProfile.iIsPrivate ) ||
       aBinaryRecipientList != NULL ) {
    encryption_was_used = true ; 
    if ( iController->model().contentEncryptionModel().encrypt(aBinaryRecipientList != NULL ? namedRecipientsList :  aPublishingProfile.iProfileReaders,
							       aContents,
							       encryptedContent ) ) {
      contentFingerPrint.calculate(encryptedContent) ; 
    } else {
      return retval ;
    }
  }   else {
    // no encryption
    contentFingerPrint.calculate(aContents) ; 
  } 
  
  // ok, now we have the content, encrypted or not, lets invent
  // some metadata.

  // fingerprint of a file is the content alone, metadata may change
  // but the file remains the same, from this programs perspective.
  BinaryFile metadata(contentFingerPrint) ; 
  metadata.iMimeType = aMimeType ; 
  metadata.iOwner = aPublishingProfile.iFingerPrint.toString() ; 
  metadata.iFileName = aFileName ; 
  metadata.iDescription = aDescription ; 
  metadata.iTimeOfPublish = QDateTime::currentDateTimeUtc().toTime_t() ;
  metadata.iIsEncrypted = encryption_was_used ;
  QByteArray metadataJSon ( metadata.asJSon(*iController) ) ; 
  // now, if the profile is private. .. we do not publish
  // plaintext metadata either. lets encrypt the metadata too
  if ( encryption_was_used ) {
    QByteArray encryptedMetadata ;
    if ( iController->model().contentEncryptionModel().encrypt(aBinaryRecipientList != NULL ? namedRecipientsList :  aPublishingProfile.iProfileReaders,
							       metadataJSon,
							       encryptedMetadata ) ) {
      metadataJSon.clear() ; 
      metadataJSon.append(encryptedMetadata) ;
      encryptedMetadata.clear() ; 
    } else {
      return retval ;
    }
  }
  // ok, now we have content, possibly encrypted, and metadata, 
  // possibly encrypted. ready to publish, except for the signature. 
  QByteArray signature ;
  if ( iController->model().contentEncryptionModel().sign(aPublishingProfile.iFingerPrint,
							  encryption_was_used ? encryptedContent : aContents ,
							  signature,
							  &metadataJSon ) == 0 ) {
    // signature went ok, then insert/update table
    QSqlQuery countQuery ;
    operation_success = countQuery.prepare("select count(hash1) from binaryfile where hash1 = :hash1 and hash2 = :hash2 and hash3 = :hash3 and hash4 = :hash4 and hash5 = :hash5") ;
    if ( operation_success ) {
      countQuery.bindValue(":hash1", contentFingerPrint.iHash160bits[0]);
      countQuery.bindValue(":hash2", contentFingerPrint.iHash160bits[1]);
      countQuery.bindValue(":hash3", contentFingerPrint.iHash160bits[2]);
      countQuery.bindValue(":hash4", contentFingerPrint.iHash160bits[3]);
      countQuery.bindValue(":hash5", contentFingerPrint.iHash160bits[4]);
      operation_success = countQuery.exec() ;
      if ( operation_success && countQuery.next() && !countQuery.isNull(0) ) {
	int count ( countQuery.value(0).toInt() ) ;
	LOG_STR2("Count of hashes in table binaryfile = %d", count) ; 
	if ( count == 0 ) {
	  // content was new -> insert
	  QSqlQuery ins ;
	  operation_success = ins.prepare("insert into binaryfile"
					  "(hash1,hash2,hash3,hash4,hash5,"
					  "publisher_hash1,publisher_hash2,publisher_hash3,"
					  "publisher_hash4,publisher_hash5,"
					  "contentdata,is_private,is_compressed,"
					  "time_last_reference,signature,display_name,"
					  "time_of_publish, metadata ) values ( :hash1,"
					  ":hash2,:hash3,:hash4,:hash5,:publisher_hash1,"
					  ":publisher_hash2,:publisher_hash3,:publisher_hash4,"
					  ":publisher_hash5,:contentdata,"
					  ":is_private,:is_compressed,:time_last_reference,"
					  ":signature,:display_name,:time_of_publish,"
					  ":metadata)" ) ;
	  if ( operation_success ) {
	    ins.bindValue(":hash1", contentFingerPrint.iHash160bits[0]);
	    ins.bindValue(":hash2", contentFingerPrint.iHash160bits[1]);
	    ins.bindValue(":hash3", contentFingerPrint.iHash160bits[2]);
	    ins.bindValue(":hash4", contentFingerPrint.iHash160bits[3]);
	    ins.bindValue(":hash5", contentFingerPrint.iHash160bits[4]);
	    ins.bindValue(":publisher_hash1", aPublishingProfile.iFingerPrint.iHash160bits[0]);
	    ins.bindValue(":publisher_hash2", aPublishingProfile.iFingerPrint.iHash160bits[1]);
	    ins.bindValue(":publisher_hash3", aPublishingProfile.iFingerPrint.iHash160bits[2]);
	    ins.bindValue(":publisher_hash4", aPublishingProfile.iFingerPrint.iHash160bits[3]);
	    ins.bindValue(":publisher_hash5", aPublishingProfile.iFingerPrint.iHash160bits[4]);
	    ins.bindValue(":contentdata", encryption_was_used ? encryptedContent : aContents );
	    ins.bindValue(":is_private", encryption_was_used) ; 
	    ins.bindValue(":is_compressed", aIsCompressed) ; 
	    ins.bindValue(":time_last_reference",metadata.iTimeOfPublish ) ;
	    ins.bindValue(":time_of_publish",metadata.iTimeOfPublish ) ;
	    ins.bindValue(":signature", signature) ; 	    
	    ins.bindValue(":display_name", aPublishingProfile.iIsPrivate ? contentFingerPrint.toString() : metadata.displayName()) ; 	    
	    ins.bindValue(":metadata", metadataJSon) ; 	    
	    if ( ( operation_success = ins.exec() ) == false ) {
	      QLOG_STR(ins.lastError().text() + " "+ __FILE__ + QString::number(__LINE__)) ;
	      emit error(MController::DbTransactionError, ins.lastError().text()) ;
	    } else {
	      iCurrentDbTableRowCount++ ;
	    }
	  } else {
	    QLOG_STR(ins.lastError().text() + " "+ __FILE__ + QString::number(__LINE__)) ;
	    emit error(MController::DbTransactionError, ins.lastError().text()) ;
	  }

	} else {
	  // content we already had -> update

	  QSqlQuery upd ;
	  operation_success = upd.prepare("update binaryfile set contentdata=:contentdata,"
					  "is_private=:is_private,is_compressed=:is_compressed,"
					  "publisher_hash1=:publisher_hash1,"
					  "publisher_hash2=:publisher_hash2,"
					  "publisher_hash3=:publisher_hash3,"
					  "publisher_hash4=:publisher_hash4,"
					  "publisher_hash5=:publisher_hash5,"
					  "time_last_reference=:time_last_reference,"
					  "signature=:signature,display_name=:display_name,"
					  "time_of_publish=:time_of_publish, metadata=:metadata "
					  "where hash1 = :hash1 and hash2 = :hash2 and"
					  " hash3 = :hash3 and hash4 = :hash4 and hash5 = :hash5") ;
	  if ( operation_success ) {
	    upd.bindValue(":hash1", contentFingerPrint.iHash160bits[0]);
	    upd.bindValue(":hash2", contentFingerPrint.iHash160bits[1]);
	    upd.bindValue(":hash3", contentFingerPrint.iHash160bits[2]);
	    upd.bindValue(":hash4", contentFingerPrint.iHash160bits[3]);
	    upd.bindValue(":hash5", contentFingerPrint.iHash160bits[4]);
	    upd.bindValue(":publisher_hash1", aPublishingProfile.iFingerPrint.iHash160bits[0]);
	    upd.bindValue(":publisher_hash2", aPublishingProfile.iFingerPrint.iHash160bits[1]);
	    upd.bindValue(":publisher_hash3", aPublishingProfile.iFingerPrint.iHash160bits[2]);
	    upd.bindValue(":publisher_hash4", aPublishingProfile.iFingerPrint.iHash160bits[3]);
	    upd.bindValue(":publisher_hash5", aPublishingProfile.iFingerPrint.iHash160bits[4]);
	    upd.bindValue(":contentdata", encryption_was_used ? encryptedContent : aContents );
	    upd.bindValue(":is_private", encryption_was_used) ; 
	    upd.bindValue(":is_compressed", aIsCompressed) ; 
	    upd.bindValue(":time_last_reference",metadata.iTimeOfPublish ) ;
	    upd.bindValue(":time_of_publish",metadata.iTimeOfPublish ) ;
	    upd.bindValue(":signature", signature) ; 	    
	    upd.bindValue(":display_name", aPublishingProfile.iIsPrivate ? contentFingerPrint.toString() : metadata.displayName()) ; 	    
	    upd.bindValue(":metadata", metadataJSon) ; 	    
	    if ( ( operation_success = upd.exec() ) == false ) {
	      QLOG_STR(upd.lastError().text() + " "+ __FILE__ + QString::number(__LINE__)) ;
	      emit error(MController::DbTransactionError, upd.lastError().text()) ;
	    } 
	  }
	}
      } else {
	QLOG_STR(countQuery.lastError().text() + " "+ __FILE__ + QString::number(__LINE__)) ;
	emit error(MController::DbTransactionError, countQuery.lastError().text()) ;
      }
    } else {
      QLOG_STR(countQuery.lastError().text() + " "+ __FILE__ + QString::number(__LINE__)) ;
      emit error(MController::DbTransactionError, countQuery.lastError().text()) ;
    } 
  }
  if ( operation_success ) {
    QList<quint32> emptyBangPath ;
    iController->model().addItemToBePublished(BinaryBlob, 
					      contentFingerPrint,
					      emptyBangPath) ;
    retval = contentFingerPrint ; 
  }
  return retval ;
}

BinaryFile* BinaryFileModel::binaryFileByFingerPrint(const Hash& aFingerPrint) {
  LOG_STR("BinaryFileModel::binaryFileByFingerPrint()") ;
  BinaryFile* retval = NULL ;
  QSqlQuery query;
  bool ret ;

  // note: here signature is not retrieved. signature is in
  // file content+metadata so we verify when retrieving the 
  // content too ; from this method the returned metadata
  // might not be from correct sender -> again, this is then
  // verified in content-retrieval mode 

  ret = query.prepare ("select metadata,is_private,is_compressed from binaryfile where hash1 = :hash1 and hash2 = :hash2 and hash3 = :hash3 and hash4 = :hash4 and hash5 = :hash5" ) ;
  if ( ret ) {
    query.bindValue(":hash1", aFingerPrint.iHash160bits[0]);
    query.bindValue(":hash2", aFingerPrint.iHash160bits[1]);
    query.bindValue(":hash3", aFingerPrint.iHash160bits[2]);
    query.bindValue(":hash4", aFingerPrint.iHash160bits[3]);
    query.bindValue(":hash5", aFingerPrint.iHash160bits[4]);
  }
  ret = query.exec() ;
  if ( !ret ) {
    QLOG_STR(query.lastError().text() + " "+ __FILE__ + QString::number(__LINE__)) ;
    emit error(MController::DbTransactionError, query.lastError().text()) ;
  } else {
    if ( query.next() && !query.isNull(0) ) {
      QByteArray metadataData ( query.value(0).toByteArray()) ;
      bool isPrivate ( false ) ;
      isPrivate = !query.isNull(1) && query.value(1).toBool() ;
      retval = new BinaryFile(aFingerPrint) ;
      if ( isPrivate ) {
	QByteArray plainTextMetaData ; 
	if ( iController->model().contentEncryptionModel().decrypt(metadataData,
								   plainTextMetaData ) ) 
	  {
	    retval->fromJSon(plainTextMetaData,*iController) ;
	  }
      } else {
	// was then public and profileData is in plainText:
	retval->fromJSon(metadataData,*iController) ;
      }
      retval->iIsCompressed = !query.isNull(2) && query.value(2).toBool() ;
    }
  }
  return retval ;
}

// unlike previous method, this does check signature. 
// it might be useful to check for presense of aPresumedSender
// public key before calling this method.. for the sake of 
// producing meaningful errormessages. 
bool BinaryFileModel::binaryFileDataByFingerPrint(const Hash& aFingerPrint,
						  const Hash& aPresumedSender,
						  QByteArray& aResultingFileData,
						  QByteArray& aResultingSignature,
						  bool* aIsBinaryFilePrivate ) {
  LOG_STR2("BinaryFileModel::binaryFileDataByFingerPrint() %s", qPrintable(aFingerPrint.toString())) ;
  bool retval = false ; 
  QSqlQuery query ; 
  retval = query.prepare ("select contentdata,signature,is_private,is_compressed,metadata from binaryfile where hash1 = :hash1 and hash2 = :hash2 and hash3 = :hash3 and hash4 = :hash4 and hash5 = :hash5" ) ;
  if ( retval ) {
    query.bindValue(":hash1", aFingerPrint.iHash160bits[0]);
    query.bindValue(":hash2", aFingerPrint.iHash160bits[1]);
    query.bindValue(":hash3", aFingerPrint.iHash160bits[2]);
    query.bindValue(":hash4", aFingerPrint.iHash160bits[3]);
    query.bindValue(":hash5", aFingerPrint.iHash160bits[4]);
  }
  retval = query.exec() ;
  if ( !retval ) {
    QLOG_STR(query.lastError().text() + " "+ __FILE__ + QString::number(__LINE__)) ;
    emit error(MController::DbTransactionError, query.lastError().text()) ;
  } else {
    if ( query.next() && !query.isNull(0) && !query.isNull(1) ) {
      QByteArray fileData ( query.value(0).toByteArray()) ;
      aResultingSignature.clear() ; 
      aResultingSignature.append ( query.value(1).toByteArray()) ;
      QByteArray metadata ( query.value(4).toByteArray()) ;
      bool isCompressed ( false ) ;
      *aIsBinaryFilePrivate = !query.isNull(2) && query.value(2).toBool() ;
      isCompressed = !query.isNull(3) && query.value(3).toBool() ;
      if ( fileData.size() > 0 &&
	   aResultingSignature.size() > 0 &&
	   metadata.size() > 0 && 
	   iController->model().contentEncryptionModel().verify(aPresumedSender, fileData, aResultingSignature, &metadata ) ) {
        // good signature
	
	// so check if de-cryption needed
	if ( *aIsBinaryFilePrivate ) {
	  QByteArray plainTextContents ; 
	  if ( iController->model().contentEncryptionModel().decrypt(fileData,
								     plainTextContents ) )  {
	    fileData.clear() ; 
	    fileData.append(plainTextContents) ; 
	    plainTextContents.clear() ; 
	  } else {
	    // signature was ok but obviously we're not among recipients:
	    retval = false ; 
	    fileData.clear() ; 
	  }
	}
	if ( fileData.size() > 0 ) {
	  aResultingFileData.clear() ; 
	  if ( isCompressed ) {
	    aResultingFileData.append(qUncompress(fileData)) ;   
	  } else {
	    aResultingFileData.append(fileData) ;   
	  }
	  if ( aResultingFileData.size() > 0 ) {
	    retval = true ; // uncompress returns empty if data corrupt - signature to save?
	  }
	} else {
	  // empty phile?
	  retval = false ; 
	}
      } else {
	// signature failed
	retval = false ; 
      }
    }
  }
  return retval ; 
}


bool BinaryFileModel::sentBinaryFileReceived(const Hash& aFingerPrint,
					     const QByteArray& aContent,
					     const QByteArray& aSignature,
					     const QByteArray& aKeyOfPublisher,
					     const unsigned char aFlags,
					     const quint32 aTimeStamp,
					     const Hash& aFromNode) 
{
  const QList<quint32> dummyBangPath; 
  return doHandleReceivedFile(aFingerPrint,
			      aContent,
			      aSignature,
			      dummyBangPath,
			      aKeyOfPublisher,
			      aFlags,
			      aTimeStamp,
			      false,
			      aFromNode) ;
}

bool BinaryFileModel::publishedBinaryFileReceived(const Hash& aFingerPrint,
						  const QByteArray& aContent,
						  const QByteArray& aSignature,
						  const QList<quint32>& aBangPath,
						  const QByteArray& aKeyOfPublisher,
						  const unsigned char aFlags,
						  const quint32 aTimeStamp,
						  const Hash& aFromNode ) 
{
  return doHandleReceivedFile(aFingerPrint,
			      aContent,
			      aSignature,
			      aBangPath,
			      aKeyOfPublisher,
			      aFlags,
			      aTimeStamp,
			      true ,
			      aFromNode) ; 
}

bool BinaryFileModel::doHandleReceivedFile(const Hash& aFingerPrint,
					   const QByteArray& aContent,
					   const QByteArray& aSignature,
					   const QList<quint32>& aBangPath,
					   const QByteArray& aKeyOfPublisher,
					   const unsigned char aFlags,
					   const quint32 aTimeStamp,
					   const bool aWasPublish,
					   const Hash& aFromNode ) 
{
  bool retval ( false ) ; 
  bool hostAlreadyHadContent ( false ) ; 
  QSqlQuery query ; 

  retval = query.prepare ("select count(contentdata) from binaryfile where hash1 = :hash1 and hash2 = :hash2 and hash3 = :hash3 and hash4 = :hash4 and hash5 = :hash5" ) ;
  if ( retval ) {
    query.bindValue(":hash1", aFingerPrint.iHash160bits[0]);
    query.bindValue(":hash2", aFingerPrint.iHash160bits[1]);
    query.bindValue(":hash3", aFingerPrint.iHash160bits[2]);
    query.bindValue(":hash4", aFingerPrint.iHash160bits[3]);
    query.bindValue(":hash5", aFingerPrint.iHash160bits[4]);
  }
  retval = query.exec() ;
  int count(0) ; 
  if ( !retval ) {
    QLOG_STR(query.lastError().text() + " "+ __FILE__ + QString::number(__LINE__)) ;
    emit error(MController::DbTransactionError, query.lastError().text()) ;
  } else {
    if ( query.next() && !query.isNull(0) ) {
      count = query.value(0).toInt() ; 
    }
  }
  if ( count > 0 ) {
    // this is ok situation, we had the content already
    retval = true ; 
    hostAlreadyHadContent = true ; 
  } else if ( (unsigned)(aContent.size()) > 0 ) {

    // ok, we did not have the content ; lets handle the signing key 
    // first, by checking if we have that:
    Hash fingerPrintOfPublisher ( iController->model().contentEncryptionModel().hashOfPublicKey(aKeyOfPublisher) ) ; 
    if ( fingerPrintOfPublisher != KNullHash ) {
      quint32* metadataLenNetworkBo ((quint32*)(aContent.constData())) ; 
      quint32 metadataLen ( ntohl (*metadataLenNetworkBo) ) ;
      if ( metadataLen > (quint32)(aContent.size()) ) {
	return false ; // soromnoo
      }
      QByteArray metaDataBytes ( aContent.mid(sizeof(quint32), metadataLen) ) ;
      if ( iController->model().contentEncryptionModel().verify(aKeyOfPublisher,
								aContent.mid(sizeof(quint32)+metadataLen ),
								aSignature,
								&metaDataBytes,
								false ) ) {
								
	// if we got here then signature in the file did match
	// they key.
	iController->model().contentEncryptionModel().insertOrUpdatePublicKey(aKeyOfPublisher,
									      fingerPrintOfPublisher) ; 
	//     next: 
	//in binary file the content dictates the hash
	Hash calculatedFingerPrint ; 
	calculatedFingerPrint.calculate(aContent.mid(sizeof(quint32)+metadataLen )) ; 
	if ( aFingerPrint != calculatedFingerPrint ) {
	  // err, what?
	  LOG_STR("Binary file received: fingerprint does not match?") ; 
	  return false ; 
	}

	// content was new -> insert
	QSqlQuery ins ;
	retval = ins.prepare("insert into binaryfile(hash1,hash2,hash3,hash4,"
			     "hash5,publisher_hash1,publisher_hash2,"
			     "publisher_hash3,publisher_hash4,"
			     "publisher_hash5,contentdata,is_private,"
			     "is_compressed,"
			     "time_last_reference,signature,display_name,"
			     "time_of_publish, metadata,recvd_from ) "
			     " values ( :hash1,"
			     ":hash2,:hash3,:hash4,:hash5,:publisher_hash1,"
			     ":publisher_hash2,"
			     ":publisher_hash3,:publisher_hash4,"
			     ":publisher_hash5,:contentdata,"
			     ":is_private,:is_compressed,:time_last_reference,"
			     ":signature,:display_name,:time_of_publish,"
			     ":metadata,:recvd_from)" ) ;
	if ( retval ) {
	  ins.bindValue(":hash1", aFingerPrint.iHash160bits[0]);
	  ins.bindValue(":hash2", aFingerPrint.iHash160bits[1]);
	  ins.bindValue(":hash3", aFingerPrint.iHash160bits[2]);
	  ins.bindValue(":hash4", aFingerPrint.iHash160bits[3]);
	  ins.bindValue(":hash5", aFingerPrint.iHash160bits[4]);
	  ins.bindValue(":recvd_from", aFromNode.iHash160bits[4]);
	  ins.bindValue(":publisher_hash1", fingerPrintOfPublisher.iHash160bits[0]);
	  ins.bindValue(":publisher_hash2", fingerPrintOfPublisher.iHash160bits[1]);
	  ins.bindValue(":publisher_hash3", fingerPrintOfPublisher.iHash160bits[2]);
	  ins.bindValue(":publisher_hash4", fingerPrintOfPublisher.iHash160bits[3]);
	  ins.bindValue(":publisher_hash5", fingerPrintOfPublisher.iHash160bits[4]);
	  ins.bindValue(":contentdata", aContent.mid(sizeof(quint32)+metadataLen ));
	  ins.bindValue(":is_private", (aFlags & 0x01) > 0 ) ; 
	  ins.bindValue(":is_compressed",(aFlags & 0x02) > 0 ) ; 
	  ins.bindValue(":time_last_reference",QDateTime::currentDateTimeUtc().toTime_t() ) ;
	  ins.bindValue(":time_of_publish",aTimeStamp ) ;
	  ins.bindValue(":signature", aSignature) ; 	 
	  if ( (aFlags & 0x01) ) {
	    // file is encrypted ; insert fp as displayname 
	    ins.bindValue(":display_name", aFingerPrint.toString() ) ;
	  } else {
	    // file is not encrypted, parse metadata and extract displayname:
	    BinaryFile metaData(aFingerPrint) ; 
	    if ( metaData.fromJSon(metaDataBytes,
				   *iController ) ){
	      ins.bindValue(":display_name",  metaData.displayName()) ;
	    } else {
	      return false ; // metadata did not parse
	    }
	  }
	  ins.bindValue(":metadata", aContent.mid(sizeof(quint32), metadataLen));
	  if ( ( retval = ins.exec() ) == false ) {
	    QLOG_STR(ins.lastError().text() + " "+ __FILE__ + QString::number(__LINE__)) ;
	    emit error(MController::DbTransactionError, ins.lastError().text()) ;
	  }  else {
	    iCurrentDbTableRowCount++ ;
	    retval = true ; // data saved
	  }
	} else {
	  QLOG_STR(ins.lastError().text() + " "+ __FILE__ + QString::number(__LINE__)) ;
	  emit error(MController::DbTransactionError, ins.lastError().text()) ;
	}
      } else {
	// signature did not verify..
	LOG_STR("Binary file signature did not verify") ; 
      }
    } else { // end of "if signing key looked ok"
      LOG_STR("Signing key could not be parsed when processing published/sent blob") ; 
      retval = false ; 
    }
  }

  if ( aWasPublish &&
       hostAlreadyHadContent == false &&
       retval == true ) {
    // re-publish new content until bangpath is full
    iController->model().addItemToBePublished(BinaryBlob, 
					      aFingerPrint,
					      aBangPath) ;
  }
  if ( retval ) {
    emit contentReceived(aFingerPrint,
			 BinaryBlob) ;
  }
  LOG_STR2("BinaryFileModel::published/sentBinaryFileReceived out success = %d",
	   (int)retval) ; 

  return retval ; 
}
  

bool BinaryFileModel::binaryFileDataForPublish(const Hash& aFingerPrint,
					       QByteArray& aResultingBinaryFileData,
					       QByteArray& aResultingSignature,
					       QByteArray& aPublicKeyOfPublisher,
					       bool* aIsBinaryFilePrivate,
					       bool* aIsBinaryFileCompressed ) {
  return doFindBinaryFileForPublishOrSend(aFingerPrint,
					  aResultingBinaryFileData,
					  aResultingSignature,
					  aPublicKeyOfPublisher,
					  aIsBinaryFilePrivate,
					  aIsBinaryFileCompressed,
					  NULL ) ; 
}

bool BinaryFileModel::binaryFileDataForSend(const Hash& aFingerPrint,
					    QByteArray& aResultingBinaryFileData,
					    QByteArray& aResultingSignature,
					    QByteArray& aPublicKeyOfPublisher,
					    bool* aIsBinaryFilePrivate,
					    bool* aIsBinaryFileCompressed,
					    quint32* aTimeOfPublish) {

  return 
    doFindBinaryFileForPublishOrSend(aFingerPrint,
				     aResultingBinaryFileData,
				     aResultingSignature,
				     aPublicKeyOfPublisher,
				     aIsBinaryFilePrivate,
				     aIsBinaryFileCompressed,
				     aTimeOfPublish ) ; 
}

bool BinaryFileModel::doFindBinaryFileForPublishOrSend(const Hash& aFingerPrint,
						       QByteArray& aResultingBinaryFileData,
						       QByteArray& aResultingSignature,
						       QByteArray& aPublicKeyOfPublisher,
						       bool* aIsBinaryFilePrivate,
						       bool* aIsBinaryFileCompressed,
						       quint32* aTimeOfPublish ) {
  bool retval = false ; 
  QSqlQuery query ; 
  Hash fingerPrintOfPublisherKey;
  retval = query.prepare ("select contentdata,signature,is_private,"
			  "is_compressed,metadata,publisher_hash1,"
			  "publisher_hash2,"
			  "publisher_hash3,publisher_hash4,publisher_hash5,"
			  "time_of_publish from binaryfile where "
			  "hash1 = :hash1 and hash2 = :hash2 and hash3 = :hash3 and hash4 = :hash4 and hash5 = :hash5" ) ;
  if ( retval ) {
    query.bindValue(":hash1", aFingerPrint.iHash160bits[0]);
    query.bindValue(":hash2", aFingerPrint.iHash160bits[1]);
    query.bindValue(":hash3", aFingerPrint.iHash160bits[2]);
    query.bindValue(":hash4", aFingerPrint.iHash160bits[3]);
    query.bindValue(":hash5", aFingerPrint.iHash160bits[4]);
  }
  retval = query.exec() ;
  bool contentIsOk ( false ) ; 
  if ( !retval ) {
    QLOG_STR(query.lastError().text() + " "+ __FILE__ + QString::number(__LINE__)) ;
    emit error(MController::DbTransactionError, query.lastError().text()) ;
  } else {
    if ( query.next() && !query.isNull(0) && !query.isNull(1) ) {
      aResultingSignature.clear() ; 
      aResultingSignature.append(query.value(1).toByteArray()) ; 
      *aIsBinaryFilePrivate = query.value(2).toBool() ; 
      *aIsBinaryFileCompressed = query.value(3).toBool() ; 
      QByteArray metaData ( query.value(4).toByteArray()) ;
      quint32 metadataLenNetworkByteOrder ( htonl((quint32)(metaData.size()) ) ) ; 
      aResultingBinaryFileData.clear() ; 
      aResultingBinaryFileData.append((const char *)(&metadataLenNetworkByteOrder), sizeof(quint32)) ; 
      aResultingBinaryFileData.append(metaData) ; 
      metaData.clear() ; 
      aResultingBinaryFileData.append(query.value(0).toByteArray()) ; 
      fingerPrintOfPublisherKey.iHash160bits[0] = query.value(5).toUInt();
      fingerPrintOfPublisherKey.iHash160bits[1] = query.value(6).toUInt();
      fingerPrintOfPublisherKey.iHash160bits[2] = query.value(7).toUInt();
      fingerPrintOfPublisherKey.iHash160bits[3] = query.value(8).toUInt();
      fingerPrintOfPublisherKey.iHash160bits[4] = query.value(9).toUInt();
      if ( aTimeOfPublish && !query.isNull(10)) {
	*aTimeOfPublish = query.value(10).toUInt();
#ifndef WIN32
#ifdef DEBUG
	time_t t ( *aTimeOfPublish ) ; 
	char timebuf[40] ; 
	LOG_STR2("Binary blob time of publish from db: %s", ctime_r(&t,timebuf)) ;
#endif
#endif
      }
      contentIsOk = true; 
    }
  }
  if ( contentIsOk ) {
    // still need to handle they key of the publisher:
    quint32 dummyTimeStamp ; 
    retval = iController->model().contentEncryptionModel().PublicKey(fingerPrintOfPublisherKey,
								     aPublicKeyOfPublisher,
								     &dummyTimeStamp ) ; 
  }
  return retval ;   
}

void BinaryFileModel::fillBucket(QList<SendQueueItem>& aSendQueue,
				 const Hash& aStartOfBucket,
				 const Hash& aEndOfBucket,
				 quint32 aLastMutualConnectTime,
				 const Hash& aForNode ) {
  QSqlQuery query;
  bool ret ; 
  ret = query.prepare ("select hash1,hash2,hash3,hash4,hash5 from binaryfile where hash1 >= :start and hash1 <= :end and time_of_publish > :last_connect_time and time_of_publish < :curr_time and ( recvd_from != :lowbits_of_requester or recvd_from is null ) order by time_of_publish desc limit 1000" ) ;
  if ( ret ) {
    if ( aStartOfBucket == aEndOfBucket ) {
      // this was "small network situation" e.g. start and end
      // are the same ; in practice it means that send all data
      query.bindValue(":start", 0x0);
      query.bindValue(":end", 0xFFFFFFFF);
    } else {
      // normal situation
      query.bindValue(":start", aStartOfBucket.iHash160bits[0]);
      query.bindValue(":end", aEndOfBucket.iHash160bits[0]);
    }
    query.bindValue(":last_connect_time", aLastMutualConnectTime);
    query.bindValue(":lowbits_of_requester", aForNode.iHash160bits[4]);
    query.bindValue(":curr_time", QDateTime::currentDateTimeUtc().toTime_t());
  }
  ret = query.exec() ;
  if ( !ret ) {
    QLOG_STR(query.lastError().text() + " "+ __FILE__ + QString::number(__LINE__)) ;
    emit error(MController::DbTransactionError, query.lastError().text()) ;
  } else {
    while ( query.next() ) {
      Hash hashFoundFromDb ( query.value(0).toUInt(),
			     query.value(1).toUInt(),
			     query.value(2).toUInt(),
			     query.value(3).toUInt(),
			     query.value(4).toUInt()) ; 
      SendQueueItem fileToSend ; 
      fileToSend.iHash = hashFoundFromDb ; 
      fileToSend.iItemType = BinaryBlob ; 
      aSendQueue.append(fileToSend) ; 
    }
  }
}
