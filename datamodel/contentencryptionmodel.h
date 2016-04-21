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

#ifndef CLASSIFIED_CONTENTENCRYPTIONMODEL_H
#define CLASSIFIED_CONTENTENCRYPTIONMODEL_H
#include <QSqlDatabase>
#include "../mcontroller.h" // because enum from there is needed
#include "../net/connection.h"
#include <openssl/pem.h> // for EVP_PKEY


class Hash ;
class MModelProtocolInterface ;

/**
 * @brief This is content-crypto-operations-specific part of the datamodel.
 *
 * This handles encryption keys and encryption operations too.
 */
class ContentEncryptionModel : public QObject {
    Q_OBJECT

public:
    ContentEncryptionModel(MController *aMController,
                           const MModelProtocolInterface &aModel ) ;
    ~ContentEncryptionModel() ;
    /**
     * method for generating a new RSA key-pair.
     * @return Fingerprint of new key, null-key on failure
     */
    Hash generateKeyPair() ;
    /**
     * method for permanently deleting keypair (and related data)
     * @param aHash is fingerprint of the profile to get rid of
     * @return true on success
     */
    bool deleteKeyPair(const Hash& aHash) ;

    /**
     * method for changing password of (private) key.
     *
     * Old valid password of aFingerPrint key must be stored
     * inside controller before this method is called. Upon
     * successful pwd change, this method will replace the
     * password stored inside controller with the new one.
     *
     * @param aFingerPrint fingerprint of the key that is to have the passwd changed
     * @return 0 on success
     */
    int changeKeyPassword(const Hash& aFingerPrint,
                          const QString& aNewPassword ) ;
    /**
     * for signing content
     * @param aSigningKey is fingerprint of the private key that
     *        will be used to sign aData
     * @param aData is octets to sign
     * @param aResultingSignature will contain resulting digital signature saying
     *                      that aSigningKey was most likely present when
     *                      aData was sent around..
     * @param aOptionalMetadata is optional part of aData that will
     *        be included into octets from which the hash is calculated.
     *        This is because binary blobs have separate content and
     *        metadata but we want't to sign both of them -> 2 separate
     *        signatures is stupid, contatenating content+metadata would
     *        be possible but heap-consuming so lets have a 2nd bytearray
     * @return 0 on success
     */
    int sign(const Hash& aSigningKey,
             const QByteArray& aData,
             QByteArray& aResultingSignature,
             const QByteArray* aOptionalMetadata = NULL ) ;
    /**
     * for verifying content
     * @return true if aDataToVerify as indeed signed by aPresumedSigningKey
     */
    bool  verify(const Hash& aPresumedSigningKey,
                 const QByteArray& aDataToVerify,
                 const QByteArray& aSignatureToVerify,
                 const QByteArray* aOptionalMetadata = NULL,
                 bool emitErrorMessage = true ) ;
    /**
     * for verifying content
     * @return true if aDataToVerify as indeed signed by aPresumedSigningKey
     */
    bool  verify(const QByteArray& aPemBytesOfSigningKey,
                 const QByteArray& aDataToVerify,
                 const QByteArray& aSignatureToVerify,
                 const QByteArray* aOptionalMetadata = NULL,
                 bool emitErrorMessage = true ) ;
    /**
     * for encrypting content.
     * @param aRecipients is list of fingerprints for that we wish
     *        to have public keys for ; those fingerprints listed
     *        but having no public key in database will be silently
     *        omitted. If less than 1 valid public key is found,
     *        this method returns false.
     * @param aPlainText contains the data to encrypt
     * @param aResultingCipherText on successful completion, this
     *        bytearray will contain the encrypted data ready to
     *        be signed.
     * @return true on success.
     */
    bool encrypt(const QList<Hash> aRecipients,
                 const QByteArray& aPlainText,
                 QByteArray& aResultingCipherText) ;
    /**
     * Reverse of method @ref ContentEncryptionModel::encrypt.
     * @param aCipherText contains the data to decrypt using
     *        private key whose fingerprint is returned by
     *        @ref MController::profileInUse method.
     * @param aResultingCipherText on successful completion, this
     *        bytearray will contain the decryped data ready to
     *        be opened.
     * @param aEmitErrorOnFailure if true, will emit an error signal
     *        so that UI can respond accordingly. There are situations
     *        where decrypt is likely to fail because selected operator
     *        is not in list of readers and in those obvious cases
     *        we don't want to flood the UI with error messages, so
     *        "false" value is supplied there.
     * @return true on success.
     */
    bool decrypt(const QByteArray& aCipherText,
                 QByteArray& aResultingPlainText,
                 bool aEmitErrorOnFailure = true ) ;
    /**
     * method for retrieving list of private keys ; these
     * are supposed to have something to with user profiles
     * we have
     * @param aPrivateKeys if set to true, returns list of known
     *        private keys
     * @param aKeyUidToSearch is a string that is matched against
     *        UID:s of keys. If NULL is passed, then all keys are
     *        returned.
     * @return list of private key fingerprints
     */
    QList<Hash> listKeys(bool aPrivateKeys,
                         char *aKeyUidToSearch) ;
    /**
     * Inserts or updates a public key into storage.
     * @param aPublicKey Key must be in X509 PEM.
     * @param aFingerPrintOfKey SHA1 of given key
     * @param aInsertWasDone output variable ; if set to non-null on calling,
     *        this method will set value of pointed boolean to be true if
     *        a new row was inserted into table
     * @param true on success
     */
    bool insertOrUpdatePublicKey (const QByteArray& aPublicKey,
                                  const Hash& aFingerPrintOfKey,
                                  const QString* aDisplayName = NULL ) ;
    /**
     * Inserts or updates a pŕivate key into storage.
     * @param aPrivateKey Key must be in PEM-format private key.
     * @param aFingerPrintOfKey expected fingerprint of aPrivateKey
     */
    bool insertOrUpdatePrivateKey (const QByteArray& aPrivateKey,
                                   const Hash& aFingerPrintOfKey) ;
    /**
     * Method for finding a public encryption key.
     *
     * @param aFingerPrintOfKeyToFind fingerprint of key to seek
     * @aPossibleKeyFound if key is found, this bytearray will after
     *                    return contain the key
     * @param aTimeStampOfKeyFound pointer to quint32 that, if not null,
     *                             will be written to value of time when
     *                             associated profile was last published.
     *                             If we have only key, no profile, then
     *                             0 will be returned in this parameter.
     * @return true if key is found
     */
    bool PublicKey (const Hash& aFingerPrintOfKeyToFind,
                    QByteArray& aPossibleKeyFound,
                    quint32 *aTimeStampOfKeyFound = NULL) ;
    /**
     * Method for finding a private encryption key.
     *
     * @param aFingerPrintOfKeyToFind fingerprint of key to seek
     * @aPossibleKeyFound if key is found, this bytearray will after
     *                    return contain the key
     * @return true if key is found
     */
    bool PrivateKey (const Hash& aFingerPrintOfKeyToFind,
                     QByteArray& aPossibleKeyFound) ;

    /**
     * Method for opening PEM bytes previously obtained using
     * @ref PrivateKey method.
     * @param aPemByteArray the key that routine tries to open+return
     * @aEmitErrorMessage if set to true, will emit() error messages
     *                    to application controller to handle.
     *                    In profile change situation it hurts.
     * @return key if success, NULL if error. Caller of this method
     *         is responsible for free()ing the key using EVP_PKEY_free()
     */
    EVP_PKEY *PrivateKeyFromPem(const QByteArray& aPemBytes,
                                bool aEmitErrorMessage = true ) ;

    /**
     * Method for opening PEM bytes previously obtained using
     * @ref PublicKey method.
     * @param aPemByteArray
     * @return key if success, NULL if error. Caller of this method
     *         is responsible for free()ing the key using EVP_PKEY_free()
     */
    EVP_PKEY *PublicKeyFromPem(const QByteArray& aPemBytes) ;

    /**
     * method for getting x509 fingerprint of a PEM key
     */
    Hash hashOfPublicKey(const QByteArray& aPemBytes) ;

    /**
     * method for getting (pseudo) random bytes
     */
    QByteArray randomBytes(int aNumberOfBytes) ;

signals:
    /**
     * this is not method but signal ; if in error, get emit()ted
     */
    void error(MController::CAErrorSituation aError,
               const QString& aExplanation) ;

private: // methods
    /**
     * work-horse of verify-methods,
     * this is called from actual "verify" variants
     */
    bool  doVerify(const QByteArray& aPemBytesOfSigningKey,
                   const QByteArray& aDataToVerify,
                   const QByteArray& aSignatureToVerify,
                   const QByteArray* aOptionalMetadata = NULL,
                   bool emitErrorMessage = true ) ;
private: // member variables:
    MController *iController  ;
    const MModelProtocolInterface& iModel ;
} ;
#endif
