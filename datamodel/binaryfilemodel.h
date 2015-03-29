/*    -*-C++-*- -*-coding: utf-8-unix;-*-
    Classified Ads is Copyright (c) Antti Jarvinen 2013.

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

#ifndef CLASSIFIED_BINARYFILEMODEL_H
#define CLASSIFIED_BINARYFILEMODEL_H
#include <QSqlDatabase>
#include "../mcontroller.h" // because enum from there is needed
#include "../net/connection.h"
#include "datamodelbase.h"

class Hash ;
class BinaryFile ;
class MModelProtocolInterface ; 
class Profile ; 

/**
 * @brief This is part of datamodel for any binary blobs
 *
 * Initially this is for "shared files" profile-functionality but
 * this practically lets you share any octet-stream over DHT.
 */
class BinaryFileModel : public ModelBase {
  Q_OBJECT

public:
  BinaryFileModel(MController *aMController,
		  const MModelProtocolInterface &aModel) ;
  ~BinaryFileModel() ;

  /**
   * sends a file to selected nodes in network for others
   * to retrieve.
   *
   * Don't forget to re-publish the profile after successful
   * completion of file publish as list of files is part of
   * the profile itself. 
   *
   * @param aPublishingProfile is the publishing profile ; it is used to
   *        sign the content and its privacy-settings are used
   *        decide if file content will be encrypted or not.
   * @param aFileName is string telling original file-system file name
   * @param aDescription is free text describing the content
   * @param aMimeType is mime type, like application/octet-stream or image/gif
   * @param aOwner who owns the file (content)
   * @param aLicense string for specifying usage permission, something like
   *                 "public domain" or "creative commons share-alike"
   * @param aContents is the meat being thrown
   * @param aIsCompressed indicates if aContent should be uncompressed
   *                      before use. 
   * @param aNoEncryption If false then privateness of aPublishingProfile
   *                      is used to decide if content gets encrypted during
   *                      publish. If true then content will always go
   *                      out without encryption. Use-case for this is attachment
   *                      of public posting where everybody needs to be able to
   *                      read, also when poster has private profile. 
   * @param aBinaryRecipientList Is list of fingerprints of profiles whose keys will
   *                          be used to encrypt the content. If these keys
   *                          are given, then normal "readers of publishing
   *                          profile" recipient-key-list method is not used.
   *                          Use-case for this parameter is attachment
   *                          of a private message; there is dual
   *                          recipient (sender and receiver) and that's it. 
   * @return Fingerprint of the published file or KNullHash if
   *         things went bad.
   */
  Hash publishBinaryFile(const Profile& aPublishingProfile,
			 const QString& aFileName,
			 const QString& aDescription,
			 const QString& aMimeType,
			 const QString& aOwner,
			 const QString& aLicense,
			 const QByteArray& aContents,
			 bool aIsCompressed,
			 bool aNoEncryption = false,
			 const QList<Hash>* aBinaryRecipientList = NULL ) ;

  /**
   * Gets a file. Caller is supposed to delete the returned
   * object. Note that returned data contains metadata only,
   * for actual content use method 
   * @refbinaryFileDataByFingerPrint and because signature
   * is calculated over metadata+content, this method does
   * not yet check for signature -> it is checked when
   * data is retrieved. 
   *
   * @param aFingerPrint file serial number (hash)
   * @return file or NULL
   */
  BinaryFile* binaryFileByFingerPrint(const Hash& aFingerPrint) ;

  /**
   * Method for getting file contents. Returned contents are un-encrypted
   * already when returned by this method, and un-compressed. And signature
   * is checked. Need to think a strategy around situation where we have 
   * a file but we don't have public key of its publisher ; in this case
   * we just have no clue about origins of the file and this is possible
   * situation too. 
   *
   * @param aFingerPrint profile serial number
   * @param aPresumedSender is fingerprint of the file sender from file
   *        metadata -> file content verification against this key
   *        must succeed. 
   * @param aResultingBinaryFileData will contain profile data
   * @param aResultingSignature will contain profile signature
   * @param aIsBinaryFilePrivate will be set to true if aResultingBinaryFileData
   *                          is encrypted.
   * @return true if file was found
   */
  bool binaryFileDataByFingerPrint(const Hash& aFingerPrint,
				   const Hash& aPresumedSender,
				   QByteArray& aResultingBinaryFileData,
				   QByteArray& aResultingSignature,
				   bool* aIsBinaryFilePrivate ) ;
  /**
   * called by protocol parser when a file is received due to publish
   * @param aFingerPrint is the profile @ref Hash received from protocol header
   * @param aContent is actual file data
   * @param aSignature is digital signature of the aContent
   * @param aBangPath is list of low-order bits of hashes of nodes
   *        where this content has been. This is for preventing
   *        sending content back to nodes where it has already been.
   * @param aKeyOfPublisher is the key that was used to sign the file.
   *        that is transferred outside file just because the recipient
   *        might not have the key  beforehand ; if she does, the existing
   *        key will be used in verifying process.
   * @param aFlags possible flags telling about encyption and compression
   * @param aTimeStamp timestamp of file (if encrypted, it must be carried outside..)
   * @param aFromNode hash of the node that sent the file. low-order bits
   *        of the hash will be stored along with the content, serving
   *        as bang-path of length 1.
   * @return true on success
   */
  bool publishedBinaryFileReceived(const Hash& aFingerPrint,
				   const QByteArray& aContent,
				   const QByteArray& aSignature,
				   const QList<quint32>& aBangPath,
				   const QByteArray& aKeyOfPublisher,
				   const unsigned char aFlags,
				   const quint32 aTimeStamp,
				   const Hash& aFromNode ) ;
  /**
   * called by protocol parser when a file is received due to send
   * @param aFingerPrint is the profile @ref Hash received from protocol header
   * @param aContent is actual file data
   * @param aSignature is digital signature of the aContent
   * @param aKeyOfPublisher is the key that was used to sign the file.
   *        that is transferred outside file just because the recipient
   *        might not have the key  beforehand ; if she does, the existing
   *        key will be used in verifying process.
   * @param aFlags possible flags telling about encyption and compression
   * @param aTimeStamp timestamp of file (if encrypted, it must be carried outside..)
   * @param aFromNode fingerprint of the node that sent the content
   * @return true on success
   */
  bool sentBinaryFileReceived(const Hash& aFingerPrint,
			      const QByteArray& aContent,
			      const QByteArray& aSignature,
			      const QByteArray& aKeyOfPublisher,
			      const unsigned char aFlags,
			      const quint32 aTimeStamp,
			      const Hash& aFromNode ) ;

  /**
   * method that returns unaltered binary file metadata+data
   * in single bytearray for purpose of publishing to other node
   * 
   * @return true if content was found
   */
  bool binaryFileDataForPublish(const Hash& aFingerPrint,
				QByteArray& aResultingBinaryFileData,
				QByteArray& aResultingSignature,
				QByteArray& aPublicKeyOfPublisher,
				bool* aIsBinaryFilePrivate,
				bool* iIsBinaryFileCompressed ) ;
  /**
   * method that returns unaltered binary file metadata+data
   * in single bytearray for purpose of sending to other node
   * 
   * @return true if content was found
   */
  bool binaryFileDataForSend(const Hash& aFingerPrint,
			     QByteArray& aResultingBinaryFileData,
			     QByteArray& aResultingSignature,
			     QByteArray& aPublicKeyOfPublisher,
			     bool* aIsBinaryFilePrivate,
			     bool* aIsBinaryFileCompressed,
			     quint32* aTimeOfPublish) ;

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
   * @param aForNode fingerprint of the node that asks for the bucket
   *                 fill ; reason for this is the recvd_from column
   *                 for preventing immediately re-sending content 
   *                 sent from a peer
   */
  void fillBucket(QList<SendQueueItem>& aSendQueue,
		  const Hash& aStartOfBucket,
		  const Hash& aEndOfBucket,
		  quint32 aLastMutualConnectTime,
		  const Hash& aForNode ); 
signals:
  void error(MController::CAErrorSituation aError,
             const QString& aExplanation) ;
  void contentReceived(const Hash& aHashOfContent,
		       const ProtocolItemType aTypeOfReceivdContent) ;
private: // methods
/** workhorse of @binaryFileDataForSend and @binaryFileDataForPublish */
bool doFindBinaryFileForPublishOrSend(const Hash& aFingerPrint,
				      QByteArray& aResultingBinaryFileData,
				      QByteArray& aResultingSignature,
				      QByteArray& aPublicKeyOfPublisher,
				      bool* aIsBinaryFilePrivate,
				      bool* aIsBinaryFileCompressed,
				      quint32* aTimeOfPublish) ;
/** 
 * workhorse of methods publishedBinaryFileReceived and sentBinaryFileReceived
 */
bool doHandleReceivedFile(const Hash& aFingerPrint,
			  const QByteArray& aContent,
			  const QByteArray& aSignature,
			  const QList<quint32>& aBangPath,
			  const QByteArray& aKeyOfPublisher,
			  const unsigned char aFlags,
			  const quint32 aTimeStamp,
			  const bool aWasPublish,
			  const Hash& aFromNode ) ;
private: // member variables:
  MController *iController  ;
  const MModelProtocolInterface& iModel ;
} ;
#endif
