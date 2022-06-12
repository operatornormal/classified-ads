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
#ifdef WIN32
#define NOMINMAX
#include <WinSock2.h>
#include <openssl/conf.h>
#include <openssl/pem.h>
#include <stdio.h>
#include <stdlib.h>
#else
#include <arpa/inet.h>  // for ntohl
#include <unistd.h>
#endif
#include <errno.h>
#include <locale.h>
#include <openssl/err.h>
#include <openssl/opensslv.h>
#include <openssl/rand.h>
#include <openssl/rsa.h>
#include <openssl/ssl.h>
#include <openssl/x509.h>
#include <stdlib.h>
#include <string.h>

#include <QSqlError>
#include <QSqlQuery>

#include "../log.h"
#include "../util/hash.h"
#include "../util/jsonwrapper.h"
#include "contentencryptionmodel.h"
#include "model.h"

static const char *KJSonEncryptedMessageEkElement = "ek";
static const char *KJSonEncryptedMessageIvElement = "iv";

ContentEncryptionModel::ContentEncryptionModel(
    MController *aController, const MModelProtocolInterface &aModel)
    : iController(aController), iModel(aModel) {
  LOG_STR("ContentEncryptionModel::ContentEncryptionModel()");
  connect(this, SIGNAL(error(MController::CAErrorSituation, const QString &)),
          iController,
          SLOT(handleError(MController::CAErrorSituation, const QString &)),
          Qt::QueuedConnection);
}

ContentEncryptionModel::~ContentEncryptionModel() {
  LOG_STR("ContentEncryptionModel::~ContentEncryptionModel()");
  iController = NULL;  // not owned, just set null
}

Hash ContentEncryptionModel::generateKeyPair() {
  Hash retval;

  LOG_STR("ContentEncryptionModel::generateKeyPair in");

  EVP_PKEY *pkey(NULL);
  pkey = EVP_PKEY_new();
  RSA *rsa(NULL);
  BIGNUM *bne(NULL);
  int ret = 0;
  bne = BN_new();
  unsigned long e = RSA_F4;
  ret = BN_set_word(bne, e);
  rsa = RSA_new();

  if ((ret = RSA_generate_key_ex(rsa, 2048, bne, NULL)) != 1) {
    QString errmsg(tr("SSL key generation went wrong, calling exit.."));
    emit error(MController::ContentEncryptionError, errmsg);
    BN_free(bne);
    RSA_free(rsa);
    return retval;
  }
  EVP_PKEY_assign_RSA(pkey, rsa);  // after this point rsa can't be free'd
  X509 *x509;
  x509 = X509_new();
  if (x509 == NULL) {
    QString errmsg(tr("x509 cert generation went wrong, calling exit.."));
    emit error(MController::ContentEncryptionError, errmsg);
    return retval;
  }
  ASN1_INTEGER_set(X509_get_serialNumber(x509), 1);

  // key validity time ; starting from now
  X509_gmtime_adj(X509_get_notBefore(x509), 0);
  // and after 100 years someone needs to find a way to
  // 1) crack this encryption, or
  // 2) write feature for migrating the stuff to another node with a new cert
  X509_gmtime_adj(X509_get_notAfter(x509),
                  60L * 60L * 24L * 365L * (long long)(80 + (rand() % 20)) +
                      (long long)((rand() % 10000)));

  X509_set_pubkey(x509, pkey);
  X509_NAME *name;
  name = X509_get_subject_name(x509);
  const char *countryCode = NULL;
  const char *countryCodeFI = "FI";  // Juice Leskinen
  const char *countryCodeSE = "SE";
  const char *countryCodeUS = "US";
  switch (rand() % 3) {
    case 0:
      countryCode = countryCodeFI;
      break;
    case 1:
      countryCode = countryCodeSE;
      break;
    default:
      countryCode = countryCodeUS;
      break;
  }
  char randomName[11] = {0};
  int nameLen = 5 + (rand() % 5);
  for (int i = 0; i < nameLen; i++) {
    randomName[i] = (char)(65 + (rand() % 25));
  }
  char domainName[20] = {0};
  for (int i = 0; i < 4; i++) {
    domainName[i] = (char)(65 + (rand() % 25));
  }
  strcat(domainName, ".com");
  X509_NAME_add_entry_by_txt(name, "C", MBSTRING_ASC,
                             (unsigned char *)countryCode, -1, -1, 0);
  X509_NAME_add_entry_by_txt(name, "O", MBSTRING_ASC,
                             (unsigned char *)randomName, -1, -1, 0);
  X509_NAME_add_entry_by_txt(name, "CN", MBSTRING_ASC,
                             (unsigned char *)domainName, -1, -1, 0);
  X509_set_issuer_name(x509, name);
  X509_sign(x509, pkey, EVP_sha1());

  BIO *pri = BIO_new(BIO_s_mem());
  BIO *pub = BIO_new(BIO_s_mem());

  // EVP_bf_cbc() gives blowfish cipher ; is guud??
  // at least it allows variable key len so user can enter crap of any len??
  if ((pri == NULL || pub == NULL) ||
      (!PEM_write_bio_PrivateKey(
          pri, pkey, EVP_bf_cbc(), NULL, 0, 0,
          (void *)qPrintable(iController->contentKeyPasswd())))) {
    QString errmsg(ERR_reason_error_string(ERR_get_error()));
    QLOG_STR("Write private key: " + errmsg);
    emit error(MController::ContentEncryptionError, errmsg);
    EVP_PKEY_free(pkey);
    X509_free(x509);
    BIO_free(pri);
    BIO_free(pub);
    return retval;
  }

  PEM_write_bio_X509(
      pub, /* write the certificate to the mem-buf we've opened */
      x509 /* our certificate */
  );

  unsigned char md[EVP_MAX_MD_SIZE];
  unsigned int n;
  const EVP_MD *digest = EVP_get_digestbyname("sha1");
  X509_digest(x509, digest, md, &n);
  retval = Hash(md);
  EVP_PKEY_free(pkey);
  X509_free(x509);
  size_t pri_len = BIO_pending(pri);
  size_t pub_len = BIO_pending(pub);

  char *pri_key = (char *)malloc(pri_len + 1);
  char *pub_key = (char *)malloc(pub_len + 1);

  BIO_read(pri, pri_key, pri_len);
  BIO_read(pub, pub_key, pub_len);

  pri_key[pri_len] = '\0';
  pub_key[pub_len] = '\0';
  BIO_free(pri);
  BIO_free(pub);
  QByteArray pubKeyBytes(pub_key, pub_len);
  QByteArray priKeyBytes(pri_key, pri_len);
  free(pub_key);
  free(pri_key);
  if (insertOrUpdatePublicKey(pubKeyBytes, retval) == false ||
      insertOrUpdatePrivateKey(priKeyBytes, retval) == false) {
    QString errmsg(tr("RSA Private key key saving went wrong"));
    emit error(MController::ContentEncryptionError, errmsg);
    return KNullHash;
  } else {
    // success
    return retval;
  }
}

bool ContentEncryptionModel::deleteKeyPair(const Hash &aHash) {
  bool ret;
  QSqlQuery query(iController->model().dataBaseConnection());
  ret = query.prepare(
      "delete from profile where hash1=:hash1 and hash2=:hash2 and "
      "hash3=:hash3 and hash4=:hash4 and hash5=:hash5");
  if (ret) {
    query.bindValue(":hash1", aHash.iHash160bits[0]);
    query.bindValue(":hash2", aHash.iHash160bits[1]);
    query.bindValue(":hash3", aHash.iHash160bits[2]);
    query.bindValue(":hash4", aHash.iHash160bits[3]);
    query.bindValue(":hash5", aHash.iHash160bits[4]);

    ret = query.exec();
    QLOG_STR("deleted profile encryption public key " + aHash.toString());
    if (!ret) {
      QLOG_STR(query.lastError().text() + " " + __FILE__ +
               QString::number(__LINE__));
      emit error(MController::DbTransactionError, query.lastError().text());
    } else {
      QSqlQuery query2(iController->model().dataBaseConnection());
      ret = query2.prepare(
          "delete from privatekeys where hash1=:hash1 and hash2=:hash2 and "
          "hash3=:hash3 and hash4=:hash4 and hash5=:hash5");
      if (ret) {
        query2.bindValue(":hash1", aHash.iHash160bits[0]);
        query2.bindValue(":hash2", aHash.iHash160bits[1]);
        query2.bindValue(":hash3", aHash.iHash160bits[2]);
        query2.bindValue(":hash4", aHash.iHash160bits[3]);
        query2.bindValue(":hash5", aHash.iHash160bits[4]);
      }
      ret = query2.exec();
      if (!ret) {
        QLOG_STR(query2.lastError().text() + " " + __FILE__ +
                 QString::number(__LINE__));
        emit error(MController::DbTransactionError, query2.lastError().text());
      } else {
        QLOG_STR("deleted profile encryption private key " + aHash.toString());
        ret = true;
      }
    }
  }
  return ret;
}

QList<Hash> ContentEncryptionModel::listKeys(bool aPrivateKeys,
                                             char * /*aKeyUidToSearch*/) {
  QList<Hash> retval;
  if (aPrivateKeys) {
    QSqlQuery query(iController->model().dataBaseConnection());
    bool ret =
        query.prepare("select hash1,hash2,hash3,hash4,hash5 from privatekeys");
    if ((ret = query.exec()) == true) {
      while (ret && query.next()) {
        quint32 hash1 = query.value(0).toUInt();
        quint32 hash2 = query.value(1).toUInt();
        quint32 hash3 = query.value(2).toUInt();
        quint32 hash4 = query.value(3).toUInt();
        quint32 hash5 = query.value(4).toUInt();
        Hash hashFoundFromDb(hash1, hash2, hash3, hash4, hash5);

        retval.append(hashFoundFromDb);
      }
    }
    if (!ret) {
      QLOG_STR(query.lastError().text() + " " + __FILE__ +
               QString::number(__LINE__));
      emit error(MController::DbTransactionError, query.lastError().text());
    }
  }
  return retval;
}

int ContentEncryptionModel::changeKeyPassword(const Hash &aFingerPrint,
                                              const QString &aNewPassword) {
  int retval = -1;
  QByteArray pemBytes;
  if (PrivateKey(aFingerPrint, pemBytes)) {
    EVP_PKEY *keyToModify = PrivateKeyFromPem(pemBytes);
    if (keyToModify) {
      // first set new password
      iController->setContentKeyPasswd(aNewPassword);
      // then update the key into database, this now happens with
      // the new password that controller has
      BIO *pri = BIO_new(BIO_s_mem());
      if (!PEM_write_bio_PrivateKey(
              pri, keyToModify, EVP_bf_cbc(), NULL, 0, 0,
              (void *)qPrintable(iController->contentKeyPasswd()))) {
        QString errmsg(ERR_reason_error_string(ERR_get_error()));
        emit error(MController::ContentEncryptionError, errmsg);
      } else {
        size_t pri_len = BIO_pending(pri);
        char *pri_key = (char *)malloc(pri_len + 1);
        BIO_read(pri, pri_key, pri_len);
        pri_key[pri_len] = '\0';
        BIO_free(pri);
        QByteArray priKeyBytes(pri_key, pri_len);
        free(pri_key);
        if (insertOrUpdatePrivateKey(priKeyBytes, aFingerPrint) == false) {
          QString errmsg(tr("RSA Private key key saving went wrong"));
          emit error(MController::ContentEncryptionError, errmsg);
        } else {
          retval = 0;
        }
      }
      EVP_PKEY_free(keyToModify);
    }
  }
  return retval;
}

int ContentEncryptionModel::sign(const Hash &aSigningKey,
                                 const QByteArray &aData,
                                 QByteArray &aResultingSignature,
                                 const QByteArray *aOptionalMetadata) {
  int retval = -1;
  LOG_STR2("ContentEncryptionModel::sign in, bytesToSign=%d", aData.length());
  EVP_MD_CTX *md_ctx = EVP_MD_CTX_create();
  unsigned sig_len;
  unsigned char sig_buf[4096];
  int err;

  QByteArray pemBytes;
  if (PrivateKey(aSigningKey, pemBytes)) {
    EVP_PKEY *keyToUse = PrivateKeyFromPem(pemBytes);
    if (keyToUse) {
      // taken from openssl examples by Eric Young & Sampo Kellomaki
      /* Do the signature */

      EVP_SignInit(md_ctx, EVP_sha1());
      EVP_SignUpdate(md_ctx, aData.data(), aData.length());
      if (aOptionalMetadata) {
        EVP_SignUpdate(md_ctx, aOptionalMetadata->data(),
                       aOptionalMetadata->length());
      }
      sig_len = sizeof(sig_buf);
      err = EVP_SignFinal(md_ctx, sig_buf, &sig_len, keyToUse);

      if (err != 1) {
        QString errmsg(ERR_reason_error_string(ERR_get_error()));
        emit error(MController::ContentEncryptionError, errmsg);
      } else {
        aResultingSignature.clear();
        // EVP-functions want unsigned char and unsigned len, where
        // Q-methods eat signed char and signed length.
        aResultingSignature.append((const char *)(&sig_buf), (int)sig_len);
        retval = 0;
      }
      EVP_PKEY_free(keyToUse);
    }
  }
  EVP_MD_CTX_destroy(md_ctx);
  return retval;
}

bool ContentEncryptionModel::verify(const QByteArray &aPemBytesOfSigningKey,
                                    const QByteArray &aDataToVerify,
                                    const QByteArray &aSignatureToVerify,
                                    const QByteArray *aOptionalMetadata,
                                    bool emitErrorMessage) {
  return doVerify(aPemBytesOfSigningKey, aDataToVerify, aSignatureToVerify,
                  aOptionalMetadata, emitErrorMessage);
}

bool ContentEncryptionModel::verify(const Hash &aPresumedSigningKey,
                                    const QByteArray &aDataToVerify,
                                    const QByteArray &aSignatureToVerify,
                                    const QByteArray *aOptionalMetadata,
                                    bool emitErrorMessage) {
  bool retval = false;
  QByteArray pemBytes;
  if (PublicKey(aPresumedSigningKey, pemBytes)) {
    retval = doVerify(pemBytes, aDataToVerify, aSignatureToVerify,
                      aOptionalMetadata, emitErrorMessage);
  }
  return retval;
}

bool ContentEncryptionModel::doVerify(const QByteArray &aPemBytesOfSigningKey,
                                      const QByteArray &aDataToVerify,
                                      const QByteArray &aSignatureToVerify,
                                      const QByteArray *aOptionalMetadata,
                                      bool emitErrorMessage) {
  bool retval(false);
  if (aPemBytesOfSigningKey.size()) {
    EVP_PKEY *keyToUse = PublicKeyFromPem(aPemBytesOfSigningKey);
    if (keyToUse) {
      EVP_MD_CTX *md_ctx(EVP_MD_CTX_create());
      int err;
      /* Verify the signature */

      EVP_VerifyInit(md_ctx, EVP_sha1());
      EVP_VerifyUpdate(md_ctx, aDataToVerify.data(), aDataToVerify.length());
      if (aOptionalMetadata) {
        EVP_VerifyUpdate(md_ctx, aOptionalMetadata->data(),
                         aOptionalMetadata->length());
      }
      err = EVP_VerifyFinal(md_ctx,
                            (const unsigned char *)aSignatureToVerify.data(),
                            aSignatureToVerify.length(), keyToUse);

      if (err != 1) {
        if (emitErrorMessage) {
          QString errmsg(ERR_reason_error_string(ERR_get_error()));
          emit error(MController::ContentEncryptionError, errmsg);
        }
      } else {
        retval = true;  // ahem
      }
      EVP_PKEY_free(keyToUse);
      EVP_MD_CTX_destroy(md_ctx);
    }
  }
  return retval;
}

bool ContentEncryptionModel::encrypt(const QList<Hash> aRecipients,
                                     const QByteArray &aPlainText,
                                     QByteArray &aResultingCipherText) {
  // significant parts of code here is stolen from openssl wiki.
  bool retval(false);
  // for SealInit
  EVP_CIPHER_CTX *ctx(EVP_CIPHER_CTX_new());
  unsigned char **ek = NULL;
  int *ekl(NULL);
  unsigned char *iv(
      (unsigned char *)malloc(EVP_CIPHER_iv_length(EVP_aes_256_cbc())));
  EVP_PKEY **pubk = NULL;
  int nr_of_valid_pubkeys(0);
  QMap<QString, QVariant> resultJSon;

  if (aRecipients.size() > 0 && aPlainText.length() > 0 && iv) {
#if OPENSSL_VERSION_NUMBER <= 0x30000000L
    EVP_CIPHER_CTX_init(ctx);
#else
    LOG_STR2("Not doing EVP_CIPHER_CTX_init with vers %lx",
             OPENSSL_VERSION_NUMBER);
#endif
    pubk = (EVP_PKEY **)malloc(sizeof(EVP_PKEY *) * aRecipients.size());

    if (pubk) {
      QByteArray pemBytes;
      for (int i = 0; i < aRecipients.size(); i++) {
        if (PublicKey(aRecipients[i], pemBytes)) {
          EVP_PKEY *keyToUse = PublicKeyFromPem(pemBytes);
          pemBytes.clear();
          if (keyToUse) {
            pubk[nr_of_valid_pubkeys++] = keyToUse;
          }
        }
      }
      LOG_STR2("Encrypt nr of valid pubkeys: %d", nr_of_valid_pubkeys);
      if (nr_of_valid_pubkeys) {
        // we got at least one valid pubkey
        ek = (unsigned char **)malloc(nr_of_valid_pubkeys);
        ekl = (int *)malloc(sizeof(int) * nr_of_valid_pubkeys);
        if (ek && ekl) {
          bool alloc_failure(false);
          int k(0);
          for (; k < nr_of_valid_pubkeys; k++) {
            ek[k] = (unsigned char *)malloc(EVP_PKEY_size(pubk[k]));
            if (ek[k] == NULL) {
              alloc_failure = true;
            }
          }
          if (alloc_failure == false) {
            if (!EVP_SealInit(ctx, EVP_aes_256_cbc(), ek, ekl, iv, pubk,
                              nr_of_valid_pubkeys)) {
              QString errmsg(ERR_reason_error_string(ERR_get_error()));
              emit error(MController::ContentEncryptionError, errmsg);
            } else {
              resultJSon.insert(
                  KJSonEncryptedMessageIvElement,
                  QString(QByteArray((const char *)iv,
                                     EVP_CIPHER_iv_length(EVP_aes_256_cbc()))
                              .toBase64()));
              QVariantList listOfEncryptionKeys;
              for (k = 0; k < nr_of_valid_pubkeys; k++) {
                if (ekl[k] && ek[k]) {
                  listOfEncryptionKeys.append(QString(
                      QByteArray((const char *)ek[k], ekl[k]).toBase64()));
                  LOG_STR2(
                      "Ek: %s",
                      qPrintable(QString(
                          QByteArray((const char *)ek[k], ekl[k]).toBase64())));
                  LOG_STR2("Eklen: %d", ekl[k]);
                }
              }
              resultJSon.insert(KJSonEncryptedMessageEkElement,
                                listOfEncryptionKeys);

              QVariant json(
                  resultJSon);  // then put the map inside QVariant and that
              // may then be serialized in libqjson-0.7 and 0.8
              QByteArray cryptoMetaData(JSonWrapper::serialize(json));
              quint32 metadataLenNwBo(htonl(cryptoMetaData.length()));
              aResultingCipherText.append((const char *)&metadataLenNwBo,
                                          sizeof(quint32));
              aResultingCipherText.append(cryptoMetaData);
              cryptoMetaData.clear();

              // only now go on with the actual data as we have keys and iv..
#define FOUR_KBYTES 4096
              unsigned char buffer_out[FOUR_KBYTES + EVP_MAX_IV_LENGTH];
              int pos(0);
              int len_out(0);
              bool failure = false;
              while (pos < aPlainText.size()) {
                QByteArray fourKiloByte(aPlainText.mid(pos, FOUR_KBYTES));
                LOG_STR2("FourKByteLen: %d", fourKiloByte.length());
                if (!EVP_SealUpdate(ctx, buffer_out, &len_out,
                                    (const unsigned char *)fourKiloByte.data(),
                                    fourKiloByte.length())) {
                  QString errmsg(ERR_reason_error_string(ERR_get_error()));
                  emit error(MController::ContentEncryptionError, errmsg);
                  failure = true;
                  break;
                } else {
                  pos += FOUR_KBYTES;
                  LOG_STR2("CipherTextLen: %d", len_out);
                  if (len_out > 0) {
                    aResultingCipherText.append((const char *)buffer_out,
                                                len_out);
                  }
                }
              }

              if (failure == false) {
                if (!EVP_SealFinal(ctx, buffer_out, &len_out)) {
                  QString errmsg(ERR_reason_error_string(ERR_get_error()));
                  emit error(MController::ContentEncryptionError, errmsg);
                } else {
                  LOG_STR2("CipherTextLen final: %d", len_out);
                  retval = true;
                  if (len_out > 0) {
                    aResultingCipherText.append((const char *)buffer_out,
                                                len_out);
                  }
                }
              }
            }
          }
          // free ek contents?
          for (k = 0; k < nr_of_valid_pubkeys; k++) {
            free(ek[k]);
          }
          free(ek);
          free(ekl);
        }
        for (int j = 0; j < nr_of_valid_pubkeys; j++) {
          EVP_PKEY_free(pubk[j]);
        }
      }
      free(pubk);
    }  // if pubk was mallocated ok
    free(iv);
    EVP_CIPHER_CTX_cleanup(ctx);
    EVP_CIPHER_CTX_free(ctx);
  }

  LOG_STR2("Res = %s", qPrintable(QString(aResultingCipherText)));
  return retval;
}

bool ContentEncryptionModel::decrypt(const QByteArray &aCipherText,
                                     QByteArray &aResultingPlainText,
                                     bool aEmitErrorOnFailure) {
  // Yes, Sir.
  bool retval(false);

  // We have come here with high hopes that we're among the
  // recipients of ResultingPlainText, right?
  bool ok;

  if (aCipherText.length() > 256) {
    const quint32 *lenPtr = (const quint32 *)aCipherText.constData();
    quint32 metaDataLen = ntohl(*lenPtr);
    if (metaDataLen < (quint32)(aCipherText.length())) {
      QVariantMap result(JSonWrapper::parse(
          aCipherText.mid(sizeof(quint32), metaDataLen), &ok));
      if (ok && result.contains(KJSonEncryptedMessageEkElement) &&
          result.contains(KJSonEncryptedMessageIvElement)) {
        const QByteArray iv(QByteArray::fromBase64(
            result[KJSonEncryptedMessageIvElement].toByteArray()));
        QVariantList encryptionKeyList(
            result[KJSonEncryptedMessageEkElement].toList());

        // then obtain our private key:
        QByteArray pemBytes;
        if (PrivateKey(iController->profileInUse(), pemBytes) &&
            iv.length() == EVP_CIPHER_iv_length(EVP_aes_256_cbc())) {
          EVP_PKEY *myPrivateKey = PrivateKeyFromPem(pemBytes);
          if (myPrivateKey) {
            pemBytes.clear();
            // de-cryption context
            EVP_CIPHER_CTX *ctx(EVP_CIPHER_CTX_new());
            // try out each key in array that is in the encrypted document,
            // if one of those would happen to be for ours.. ..
            bool keyFound(false);
            foreach (QVariant keyVariant, encryptionKeyList) {
              LOG_STR("Trying private key..");
              const QByteArray key(
                  QByteArray::fromBase64(keyVariant.toByteArray()));
#if OPENSSL_VERSION_NUMBER <= 0x30000000L
              EVP_CIPHER_CTX_init(ctx);
#else
              LOG_STR2("Not doing EVP_CIPHER_CTX_init with vers %lx",
                       OPENSSL_VERSION_NUMBER);
#endif
              if (EVP_OpenInit(
                      ctx, EVP_aes_256_cbc(),
                      (const unsigned char *)(key.constData()), key.length(),
                      (const unsigned char *)(iv.constData()), myPrivateKey)) {
                // yes, success
                keyFound = true;
                break;  // out from foreach-loop
              } else {
                // key not found, cleanup context and try again with next key:
                EVP_CIPHER_CTX_cleanup(ctx);
#if OPENSSL_VERSION_NUMBER <= 0x30000000L
                EVP_CIPHER_CTX_init(ctx);
#else
                LOG_STR2("Not doing EVP_CIPHER_CTX_init with vers %lx",
                         OPENSSL_VERSION_NUMBER);
#endif
              }
            }  // foreach key
            if (keyFound) {
              LOG_STR("Decrypt: key found");
              // allocate space for plaintext:
              int len_out(0);
              unsigned char *plainTextPtr =
                  (unsigned char *)malloc(aCipherText.length() - metaDataLen);
              if (plainTextPtr) {
                if (!EVP_OpenUpdate(
                        ctx, plainTextPtr, &len_out,
                        (const unsigned char *)(aCipherText
                                                    .mid(sizeof(quint32) +
                                                         metaDataLen)
                                                    .constData()),
                        aCipherText.length() -
                            (sizeof(quint32) + metaDataLen))) {
                  if (aEmitErrorOnFailure) {
                    QString errmsg(ERR_reason_error_string(ERR_get_error()));
                    emit error(MController::ContentEncryptionError, errmsg);
                  }
                } else {
                  if (len_out > 0) {
                    aResultingPlainText.append((const char *)plainTextPtr,
                                               len_out);
                  }
                  if (!EVP_OpenFinal(ctx, plainTextPtr, &len_out)) {
                    QString errmsg(ERR_reason_error_string(ERR_get_error()));
                    emit error(MController::ContentEncryptionError, errmsg);
                  } else {
                    retval = true;
                    if (len_out > 0) {
                      aResultingPlainText.append((const char *)plainTextPtr,
                                                 len_out);
                    }
                  }
                }
                free(plainTextPtr);
              }  // if we got plainTextPtr
            } else {
              // too bad dude, you're not in the club..
              if (aEmitErrorOnFailure) {
                QString errmsg(tr("No suitable de-cryption key found"));
                emit error(MController::ContentEncryptionError, errmsg);
              }
              LOG_STR("Decrypt: key not found");
              retval = false;
            }
            EVP_CIPHER_CTX_cleanup(ctx);
            EVP_CIPHER_CTX_free(ctx);
            EVP_PKEY_free(myPrivateKey);
          }
        }
      }  // if parse was ok
    }    // metadata length check
  }      // length check
  return retval;
}

bool ContentEncryptionModel::insertOrUpdatePublicKey(
    const QByteArray &aPublicKey, const Hash &aFingerPrintOfKey,
    const QString *aDisplayName) {
  LOG_STR("ContentEncryptionModel::insertOrUpdatePublicKey() in");
  bool retval = false;
  QByteArray dummy;

  if (hashOfPublicKey(aPublicKey) == aFingerPrintOfKey) {
    if (PublicKey(aFingerPrintOfKey, dummy)) {
      // key is already in db
      bool ret;
      QSqlQuery query(iController->model().dataBaseConnection());
      if (aDisplayName && aDisplayName->length() > 0) {
        ret = query.prepare(
            "update profile set time_last_reference=:time_last_reference, "
            "pubkey = :pubkey,display_name=:display_name where hash1 = :hash1 "
            "and hash2 = :hash2 and hash3 = :hash3 and hash4 = :hash4 and "
            "hash5 = :hash5");
      } else {
        ret = query.prepare(
            "update profile set time_last_reference=:time_last_reference, "
            "pubkey = :pubkey where hash1 = :hash1 and hash2 = :hash2 and "
            "hash3 = :hash3 and hash4 = :hash4 and hash5 = :hash5");
      }
      if (ret) {
        query.bindValue(":hash1", aFingerPrintOfKey.iHash160bits[0]);
        query.bindValue(":hash2", aFingerPrintOfKey.iHash160bits[1]);
        query.bindValue(":hash3", aFingerPrintOfKey.iHash160bits[2]);
        query.bindValue(":hash4", aFingerPrintOfKey.iHash160bits[3]);
        query.bindValue(":hash5", aFingerPrintOfKey.iHash160bits[4]);
        query.bindValue(":pubkey", aPublicKey);
        query.bindValue(":time_last_reference",
                        QDateTime::currentDateTimeUtc().toTime_t());
        if (aDisplayName && aDisplayName->length() > 0) {
          query.bindValue(":display_name", *aDisplayName);
        }
      }
      ret = query.exec();
      QLOG_STR("updated public key " + aFingerPrintOfKey.toString());
      if (!ret) {
        QLOG_STR(query.lastError().text() + " " + __FILE__ +
                 QString::number(__LINE__));
        emit error(MController::DbTransactionError, query.lastError().text());
      } else {
        retval = true;
      }
    } else {
      // key is brand new
      bool ret;
      QSqlQuery query(iController->model().dataBaseConnection());
      if (aDisplayName && aDisplayName->length() > 0) {
        ret = query.prepare(
            "insert into profile "
            "(hash1,hash2,hash3,hash4,hash5,pubkey,time_last_reference,display_"
            "name) values "
            "(:hash1,:hash2,:hash3,:hash4,:hash5,:pubkey,:time_last_reference,:"
            "display_name)");
      } else {
        ret = query.prepare(
            "insert into profile "
            "(hash1,hash2,hash3,hash4,hash5,pubkey,time_last_reference) values "
            "(:hash1,:hash2,:hash3,:hash4,:hash5,:pubkey,:time_last_"
            "reference)");
      }
      if (ret) {
        query.bindValue(":hash1", aFingerPrintOfKey.iHash160bits[0]);
        query.bindValue(":hash2", aFingerPrintOfKey.iHash160bits[1]);
        query.bindValue(":hash3", aFingerPrintOfKey.iHash160bits[2]);
        query.bindValue(":hash4", aFingerPrintOfKey.iHash160bits[3]);
        query.bindValue(":hash5", aFingerPrintOfKey.iHash160bits[4]);
        query.bindValue(":pubkey", aPublicKey);
        query.bindValue(":time_last_reference",
                        QDateTime::currentDateTimeUtc().toTime_t());
        if (aDisplayName && aDisplayName->length() > 0) {
          query.bindValue(":display_name", *aDisplayName);
        }
      }
      ret = query.exec();
      QLOG_STR("### inserted profile encryption public key " +
               aFingerPrintOfKey.toString());
      if (!ret) {
        QLOG_STR(query.lastError().text() + " " + __FILE__ +
                 QString::number(__LINE__));
        emit error(MController::DbTransactionError, query.lastError().text());
      } else {
        retval = true;
      }
    }
  }
  return retval;
}

bool ContentEncryptionModel::insertOrUpdatePrivateKey(
    const QByteArray &aPrivateKey, const Hash &aFingerPrintOfKey) {
  LOG_STR("ContentEncryptionModel::insertOrUpdatePrivateKey() in");
  bool retval = false;
  QByteArray dummy;

  if (PrivateKey(aFingerPrintOfKey, dummy)) {
    // key is already in db
    bool ret;
    QSqlQuery query(iController->model().dataBaseConnection());
    ret = query.prepare(
        "update privatekeys set prikey = :prikey where hash1 = :hash1 and "
        "hash2 = :hash2 and hash3 = :hash3 and hash4 = :hash4 and hash5 = "
        ":hash5");
    if (ret) {
      query.bindValue(":hash1", aFingerPrintOfKey.iHash160bits[0]);
      query.bindValue(":hash2", aFingerPrintOfKey.iHash160bits[1]);
      query.bindValue(":hash3", aFingerPrintOfKey.iHash160bits[2]);
      query.bindValue(":hash4", aFingerPrintOfKey.iHash160bits[3]);
      query.bindValue(":hash5", aFingerPrintOfKey.iHash160bits[4]);
      query.bindValue(":prikey", aPrivateKey);
    }
    ret = query.exec();
    QLOG_STR("updated private key " + aFingerPrintOfKey.toString());
    if (!ret) {
      QLOG_STR(query.lastError().text() + " " + __FILE__ +
               QString::number(__LINE__));
      emit error(MController::DbTransactionError, query.lastError().text());
    } else {
      retval = true;
    }
  } else {
    // key is brand new
    bool ret;
    QSqlQuery query(iController->model().dataBaseConnection());
    ret = query.prepare(
        "insert into privatekeys (hash1,hash2,hash3,hash4,hash5,prikey) values "
        "(:hash1,:hash2,:hash3,:hash4,:hash5,:prikey)");
    if (ret) {
      query.bindValue(":hash1", aFingerPrintOfKey.iHash160bits[0]);
      query.bindValue(":hash2", aFingerPrintOfKey.iHash160bits[1]);
      query.bindValue(":hash3", aFingerPrintOfKey.iHash160bits[2]);
      query.bindValue(":hash4", aFingerPrintOfKey.iHash160bits[3]);
      query.bindValue(":hash5", aFingerPrintOfKey.iHash160bits[4]);
      query.bindValue(":prikey", aPrivateKey);
    }
    ret = query.exec();
    QLOG_STR("inserted private key " + aFingerPrintOfKey.toString());
    if (!ret) {
      QLOG_STR(query.lastError().text() + " " + __FILE__ +
               QString::number(__LINE__));
      emit error(MController::DbTransactionError, query.lastError().text());
    } else {
      retval = true;
    }
  }
  return retval;
}

bool ContentEncryptionModel::PublicKey(const Hash &aFingerPrintOfKeyToFind,
                                       QByteArray &aPossibleKeyFound,
                                       quint32 *aTimeStamp) {
  bool retval = false;

  QLOG_STR("ContentEncryptionModel::PublicKey in " +
           aFingerPrintOfKeyToFind.toString());
  QSqlQuery query(iController->model().dataBaseConnection());
  bool ret = query.prepare(
      "select pubkey,time_of_publish from profile where hash1 = :hash1 and "
      "hash2 = :hash2 and hash3 = :hash3 and hash4 = :hash4 and hash5 = "
      ":hash5");
  query.bindValue(":hash1", aFingerPrintOfKeyToFind.iHash160bits[0]);
  query.bindValue(":hash2", aFingerPrintOfKeyToFind.iHash160bits[1]);
  query.bindValue(":hash3", aFingerPrintOfKeyToFind.iHash160bits[2]);
  query.bindValue(":hash4", aFingerPrintOfKeyToFind.iHash160bits[3]);
  query.bindValue(":hash5", aFingerPrintOfKeyToFind.iHash160bits[4]);
  if (ret && (ret = query.exec()) == true && query.next()) {
    // yes, found
    aPossibleKeyFound = query.value(0).toByteArray();
    if (aTimeStamp) {
      if (query.isNull(1)) {
        *aTimeStamp = 0;  // we had only profile key, no profile data
      } else {
        *aTimeStamp = query.value(1).toUInt();
      }
    }
    retval = true;
  }
  if (!ret) {
    QLOG_STR(query.lastError().text() + " " + __FILE__ +
             QString::number(__LINE__));
    emit error(MController::DbTransactionError, query.lastError().text());
  }
  return retval;
}

bool ContentEncryptionModel::PrivateKey(const Hash &aFingerPrintOfKeyToFind,
                                        QByteArray &aPossibleKeyFound) {
  bool retval = false;

  LOG_STR2("ContentEncryptionModel::PrivateKey in %s",
           qPrintable(aFingerPrintOfKeyToFind.toString()));
  QSqlQuery query(iController->model().dataBaseConnection());
  bool ret = query.prepare(
      "select prikey from privatekeys where hash1 = :hash1 and hash2 = :hash2 "
      "and hash3 = :hash3 and hash4 = :hash4 and hash5 = :hash5");
  query.bindValue(":hash1", aFingerPrintOfKeyToFind.iHash160bits[0]);
  query.bindValue(":hash2", aFingerPrintOfKeyToFind.iHash160bits[1]);
  query.bindValue(":hash3", aFingerPrintOfKeyToFind.iHash160bits[2]);
  query.bindValue(":hash4", aFingerPrintOfKeyToFind.iHash160bits[3]);
  query.bindValue(":hash5", aFingerPrintOfKeyToFind.iHash160bits[4]);
  if (ret && (ret = query.exec()) == true && query.next()) {
    // yes, found
    aPossibleKeyFound = query.value(0).toByteArray();
    retval = true;
  }
  if (!ret) {
    LOG_STR(
        "Error while doing select prikey from privatekeys where hash1 = :hash1 "
        "and hash2 = :hash2 and hash3 = :hash3 and hash4 = :hash4 and hash5 = "
        ":hash5");
    emit error(MController::DbTransactionError, query.lastError().text());
  }
  LOG_STR2("ContentEncryptionModel::PrivateKey out success =%d", (int)retval);
  return retval;
}

EVP_PKEY *ContentEncryptionModel::PrivateKeyFromPem(const QByteArray &aPemBytes,
                                                    bool aEmitErrorMessage) {
  EVP_PKEY *retval = NULL;
  BIO *pri = BIO_new(BIO_s_mem());
  BIO_write(pri, aPemBytes.data(), aPemBytes.length());
  retval = PEM_read_bio_PrivateKey(
      pri, NULL, NULL, (void *)qPrintable(iController->contentKeyPasswd()));
  BIO_free(pri);
  if (!retval) {
    long errorCode = ERR_get_error();

    char errorText[1024] = {0};
    ERR_error_string_n(errorCode, errorText, 1023);
    LOG_STR("PrivateKeyFromPem password = " + iController->contentKeyPasswd());
    LOG_STR2("PrivateKeyFromPem errorcode = %d", (int)errorCode);
    LOG_STR2("PrivateKeyFromPem errorcode = %s", errorText);
    if (aEmitErrorMessage) {
      QString errmsg(ERR_reason_error_string(errorCode));
      emit error(MController::BadPassword, errmsg);
    }
  }
  return retval;
}

EVP_PKEY *ContentEncryptionModel::PublicKeyFromPem(
    const QByteArray &aPemBytes) {
  EVP_PKEY *retval = NULL;
  X509 *x509 = NULL;
  BIO *pri = BIO_new(BIO_s_mem());
  BIO_write(pri, aPemBytes.data(), aPemBytes.length());
  x509 = PEM_read_bio_X509(pri, NULL, NULL, NULL);
  BIO_free(pri);
  if (!x509) {
    long errorCode = ERR_get_error();
    LOG_STR2("PublicKeyFromPem errorcode = %d", (int)errorCode);
    QString errmsg(ERR_reason_error_string(errorCode));
    emit error(MController::BadPassword, errmsg);
  } else {
    retval = X509_get_pubkey(x509);
    if (retval == NULL) {
      long errorCode = ERR_get_error();
      LOG_STR2("PublicKeyFromPem errorcode = %d", (int)errorCode);
      QString errmsg(ERR_reason_error_string(errorCode));
      emit error(MController::BadPassword, errmsg);
    }
    X509_free(x509);
  }
  return retval;
}

Hash ContentEncryptionModel::hashOfPublicKey(const QByteArray &aPemBytes) {
  Hash retval(KNullHash);

  X509 *x509 = NULL;
  BIO *pri = BIO_new(BIO_s_mem());
  BIO_write(pri, aPemBytes.data(), aPemBytes.length());
  x509 = PEM_read_bio_X509(pri, NULL, NULL, NULL);
  BIO_free(pri);
  if (!x509) {
    long errorCode = ERR_get_error();
    LOG_STR2("PublicKeyFromPem errorcode = %d", (int)errorCode);
    QString errmsg(ERR_reason_error_string(errorCode));
    emit error(MController::BadPassword, errmsg);
  } else {
    unsigned char md[EVP_MAX_MD_SIZE];
    unsigned int n;
    const EVP_MD *digest = EVP_get_digestbyname("sha1");
    X509_digest(x509, digest, md, &n);
    retval = Hash(md);
    X509_free(x509);
  }
  return retval;
}

QByteArray ContentEncryptionModel::randomBytes(int aNumberOfBytes) {
  unsigned char *bytes =
      reinterpret_cast<unsigned char *>(malloc(aNumberOfBytes + 1));
  if (bytes) {
    RAND_bytes(bytes, aNumberOfBytes);
    QByteArray retval((const char *)bytes, aNumberOfBytes);
    free(bytes);
    return retval;
  } else {
    // memory allocation failed and random bytes would have
    // been needed: terminate, we're in deep trouble
    exit(1);
    // keep compiler happy, this line
    // should not be reached
    return QByteArray('0', aNumberOfBytes);
  }
}
