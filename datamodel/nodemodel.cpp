/*  -*-C++-*- -*-coding: utf-8-unix;-*-
    Classified Ads is Copyright (c) Antti Järvinen 2013-2016.

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
#endif
#include "nodemodel.h"
#include "../log.h"
#include <QObject>
#include <QSqlError>
#include <QFile>
#include <assert.h>
#include <QSqlQuery>
#include "../util/hash.h"
#include <openssl/x509.h>
#include <openssl/rsa.h>
#include <openssl/pem.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <QHostInfo>
#include <QUuid>
#include <QSslCertificate>
#include <QSslKey>
#include "../net/connection.h"
#include "../net/protocol_message_formatter.h"
#include "../net/node.h"
#include "model.h"
#include "camodel.h"
#include "binaryfilemodel.h"
#include "profilemodel.h"
#include "privmsgmodel.h"
#include "profilecommentmodel.h"
#include "const.h"
#include "searchmodel.h"
#include <QTimerEvent>

/**
 * How many nodes to keep
 */
static const int KMaxNodesInDbTable( 10000 );

NodeModel::NodeModel(MController *aController,
                     const Model &aModel)
    : ModelBase("node", KMaxRowsInTableNode),
      iController(aController),
      iFingerPrintOfThisNode(NULL) ,
      iThisNodeCert(NULL),
      iThisNodeKey(NULL),
      iModel(aModel),
      iTimerId(-1) {
    LOG_STR("NodeModel::NodeModel()") ;
    connect(this,
            SIGNAL(  error(MController::CAErrorSituation,
                           const QString&) ),
            iController,
            SLOT(handleError(MController::CAErrorSituation,
                             const QString&)),
            Qt::QueuedConnection ) ;
    openOrCreateSSLCertificate() ; // this method emit possible errors itself
    iTimerId = startTimer(20000*2) ; // 2-minute timer
}

NodeModel::~NodeModel() {
    LOG_STR("NodeModel::~NodeModel()") ;
    delete iThisNodeKey ;
    if ( iThisNodeCert ) {
        delete iThisNodeCert ;
    }
    if ( iFingerPrintOfThisNode ) {
        delete iFingerPrintOfThisNode ;
    }
    iController = NULL ; // not owned, just set null
    while ( !iConnectionWishList.isEmpty() ) {
        Node* wishListItem =  iConnectionWishList.takeAt(0) ;
        delete wishListItem ;
    }
    if ( iTimerId != -1 ) {
        killTimer(iTimerId) ;
    }
}

bool NodeModel::openOrCreateSSLCertificate() {
    if ( loadSslCertFromDb()&&
            loadSslKeyFromDb() ) {
        return true ;
    } else {
        LOG_STR("No ssl cert in place, generating") ;

        EVP_PKEY * pkey(NULL);


        pkey = EVP_PKEY_new();
        RSA * rsa (NULL);
        BIGNUM          *bne ( NULL );
        int ret = 0 ;
        bne = BN_new();
        unsigned long   e = RSA_F4;
        ret = BN_set_word(bne,e);
        rsa = RSA_new() ;

        if ( rsa != NULL &&
                ( ret = RSA_generate_key_ex(rsa,
                                            2048,
                                            bne,
                                            NULL
                                           ) ) != 1 ) {
            QString errmsg(tr("SSL key generation went wrong, calling exit..")) ;
            emit error(MController::OwnCertNotFound, errmsg) ;
            RSA_free(rsa) ;
            BN_free(bne) ;
            return false ;
        }
        EVP_PKEY_assign_RSA(pkey, rsa); // after this point rsa can't be free'd
        X509 * x509;
        x509 = X509_new();
        if ( x509 == NULL ) {
            QString errmsg(tr("x509 cert generation went wrong, calling exit..")) ;
            emit error(MController::OwnCertNotFound, errmsg) ;
            return false ;
        }
        ASN1_INTEGER_set(X509_get_serialNumber(x509), 1);

        // key validity time ; starting from now
        X509_gmtime_adj(X509_get_notBefore(x509), 0);
        // and after 100 years someone needs to find a way to
        // 1) crack this encryption, or
        // 2) write feature for migrating the stuff to another node with a new cert
        X509_gmtime_adj(X509_get_notAfter(x509), 60L  * 60L  * 24L  * 365L * (long long)(80+(qrand()%20)) + (long long)((qrand() % 10000)) );


        X509_set_pubkey(x509, pkey);
        X509_NAME * name;
        name = X509_get_subject_name(x509);
        const char *countryCode = NULL ;
        const char *countryCodeFI = "FI" ; // Juice Leskinen
        const char *countryCodeSE = "SE" ;
        const char *countryCodeUS = "US" ;
        switch (rand() % 3 ) {
        case 0:
            countryCode = countryCodeFI;
            break ;
        case 1:
            countryCode = countryCodeSE;
            break ;
        default:
            countryCode = countryCodeUS;
            break ;
        }
        char randomName [11] = { 0 };
        int nameLen = 5 + (rand() % 5 ) ;
        for ( int i = 0 ; i < nameLen ; i++ ) {
            randomName[i] = (char) (65 + ( rand() % 25) ) ;
        }
        char domainName [20] = { 0 } ;
        for ( int i = 0 ; i < 4 ; i++ ) {
            domainName[i] = (char) (65 + ( rand() % 25) ) ;
        }
        strcat (domainName, ".com") ;
        X509_NAME_add_entry_by_txt(name, "C",  MBSTRING_ASC,
                                   (unsigned char *)countryCode, -1, -1, 0);
        X509_NAME_add_entry_by_txt(name, "O",  MBSTRING_ASC,
                                   (unsigned char *)randomName, -1, -1, 0);
        X509_NAME_add_entry_by_txt(name, "CN", MBSTRING_ASC,
                                   (unsigned char *)domainName, -1, -1, 0);
        X509_set_issuer_name(x509, name);
        X509_sign(x509, pkey, EVP_sha1());

        BIO *pri = BIO_new(BIO_s_mem());
        BIO *pub = BIO_new(BIO_s_mem());

        if((pri == NULL || pub == NULL ) ||
                (!PEM_write_bio_PrivateKey(pri, pkey, NULL, NULL, 0, 0, NULL))) {
            QString errmsg(ERR_reason_error_string(ERR_get_error())) ;
            emit error(MController::ContentEncryptionError, errmsg) ;
            EVP_PKEY_free(pkey) ;
            X509_free(x509) ;
            BIO_free(pri) ;
            BIO_free(pub) ;
            // rsa i associated with x509, no need to free()
            // bne is associated with RSA, no need to free()
            return false ;
        }

        PEM_write_bio_X509( pub, /* write the certificate to the mem-buf we've opened */
                            x509 /* our certificate */
                          );

        unsigned char         md[EVP_MAX_MD_SIZE];
        unsigned int          n;
        const EVP_MD        * digest = EVP_get_digestbyname("sha1");
        X509_digest(x509, digest, md, &n);
        iFingerPrintOfThisNode = new Hash(md) ;
        EVP_PKEY_free(pkey) ;
        X509_free(x509) ;
        size_t pri_len = BIO_pending(pri);
        size_t pub_len = BIO_pending(pub);

        char *pri_key = (char *)malloc(pri_len + 1);
        char *pub_key = (char *)malloc(pub_len + 1);

        BIO_read(pri, pri_key, pri_len);
        BIO_read(pub, pub_key, pub_len);

        pri_key[pri_len] = '\0';
        pub_key[pub_len] = '\0';
        BIO_free(pri) ;
        BIO_free(pub) ;
        QByteArray pubKeyBytes(pub_key,pub_len) ;
        QByteArray priKeyBytes(pri_key,pri_len) ;
        free(pub_key) ;
        free(pri_key) ;
        saveSslCertToDb(pubKeyBytes) ;
        saveSslKeyToDb(priKeyBytes) ;
    }
    return ( loadSslCertFromDb() && loadSslKeyFromDb() )  ;
}

Hash& NodeModel::nodeFingerPrint() {
    return *iFingerPrintOfThisNode ;
}

int NodeModel::listenPortOfThisNode() {
    bool ret = false ;
    int port = -1  ;
    QSqlQuery query;
    ret = query.exec("select node_listenport from settings") ;
    if ( ret && query.next() && !query.isNull(0) ) {
        port = query.value(0).toInt() ;
        LOG_STR2("Our listen port previously set to %d", port) ;
    }
    if ( port < 0 ) {
        // had no port, invent one.
        port = ( rand() % 15000 ) + 20000 ;
        LOG_STR2("Our listen port now set to %d", port) ;
        setListenPortOfThisNode(port) ;
    }
    return port ;
}

void NodeModel::setListenPortOfThisNode(int port) {
    bool ret ( true ) ;
    QSqlQuery q2;
    ret = q2.prepare("update settings set node_listenport = :port") ;
    if (ret) {
        q2.bindValue(":port", port) ;
        q2.exec() ;
    }
}


const QSslCertificate& NodeModel::nodeCert() const {
    return *iThisNodeCert ;
}

const QSslKey& NodeModel::nodeKey() const {
    return *iThisNodeKey ;
}


// As is the case with most (all?) of the methods of this
// datamodel, the caller must have called lock() first.
//
// This method here actually pretty much determines the way
// how classified ads network works. This gives the next item,
// always. Items are sent in certain order and certain
// intervals and implementation of order and interval etc.
// lies inside this method .. note also part where
// aConnection.SendQueue is filled
QByteArray* NodeModel::getNextItemToSend(Connection& aConnection) {
    //  LOG_STR("NodeModel::getNextItemToSend in ") ;
    QByteArray* retval = NULL ;
    // a trick played here ; connection actually already has
    // the data but lets route it via datamodel so we can
    // start preparing for the next
    if (aConnection.iNextProtocolItemToSend.size()) {
        retval = aConnection.iNextProtocolItemToSend.value(0) ;
        aConnection.iNextProtocolItemToSend.removeAt(0) ;
    } else {
        if ( aConnection.iSendQueue.size() > 0 ) {
            SendQueueItem itemToPrepare = aConnection.iSendQueue.takeAt(0) ;
            switch ( itemToPrepare.iItemType ) {
            case OwnNodeGreeting: {
                retval = new QByteArray(ProtocolMessageFormatter::nodeGreeting(iController->getNode())) ;
            }
            break ;
            case NodeGreeting: {
                Node* nodeToSend =  nodeByHash(itemToPrepare.iHash) ;
                if ( nodeToSend ) {
                    retval = new QByteArray(ProtocolMessageFormatter::nodeGreeting(*nodeToSend)) ;
                    delete nodeToSend ;
                }
            }
            break ;
            case ClassifiedAd: {
                QByteArray caBytes ;
                QByteArray caSignature ;
                QByteArray* resultBytes = new QByteArray() ;
                QByteArray caKey ;
                bool isContentEncrypted = false ;
                bool isContentCompressed = false ;
                quint32 timeWhenCaWasPublished ;

                if ( iModel.classifiedAdsModel().caDataForPublish(itemToPrepare.iHash,
                        caBytes,
                        caSignature,
                        caKey,
                        &timeWhenCaWasPublished) ) {
                    resultBytes->append(ProtocolMessageFormatter::contentSend(KClassifiedAdSend,
                                        itemToPrepare.iHash,
                                        caBytes,
                                        caSignature,
                                        caKey,
                                        isContentEncrypted,
                                        isContentCompressed,
                                        timeWhenCaWasPublished ));
                }
                caKey.clear() ;
                caBytes.clear() ;
                caSignature.clear() ;
                if ( resultBytes->size() ) {
                    retval = resultBytes ;
                } else {
                    // we obtained zero bytes -> delete empty bytearray
                    delete resultBytes ;
                }
            }
            break ;
            case BinaryBlob: {
                QByteArray binary ;
                QByteArray binarySignature ;
                QByteArray* resultBytes = new QByteArray() ;
                QByteArray binaryKey ;
                bool isContentEncrypted = false ;
                bool isContentCompressed = false ;
                quint32 timeWhenBinaryFileWasPublished ;

                if ( iModel.binaryFileModel().binaryFileDataForSend(itemToPrepare.iHash,
                        binary,
                        binarySignature,
                        binaryKey,
                        &isContentEncrypted,
                        &isContentCompressed,
                        &timeWhenBinaryFileWasPublished) ) {
                    resultBytes->append(ProtocolMessageFormatter::contentSend(KBinaryFileSend,
                                        itemToPrepare.iHash,
                                        binary,
                                        binarySignature,
                                        binaryKey,
                                        isContentEncrypted,
                                        isContentCompressed,
                                        timeWhenBinaryFileWasPublished ));
                }
                binaryKey.clear() ;
                binary.clear() ;
                binarySignature.clear() ;
                if ( resultBytes->size() ) {
                    retval = resultBytes ;
                } else {
                    // we obtained zero bytes -> delete empty bytearray
                    delete resultBytes ;
                }
            }
            break ;
            case UserProfile: {
                QByteArray profile ;
                QByteArray profileSignature ;
                QByteArray* resultBytes = new QByteArray() ;
                QByteArray profileKey ;
                bool isContentEncrypted = false ;
                bool isContentCompressed = false ;
                quint32 timeWhenProfileWasPublished ;

                if ( iModel.profileModel().profileDataByFingerPrint(itemToPrepare.iHash,
                        profile,
                        profileSignature,
                        &isContentEncrypted,
                        &timeWhenProfileWasPublished,
                        &profileKey) ) {
                    resultBytes->append(ProtocolMessageFormatter::contentSend(KProfileSend,
                                        itemToPrepare.iHash,
                                        profile,
                                        profileSignature,
                                        profileKey,
                                        isContentEncrypted,
                                        isContentCompressed,
                                        timeWhenProfileWasPublished ));
                }
                profileKey.clear() ;
                profile.clear() ;
                profileSignature.clear() ;
                if ( resultBytes->size() ) {
                    retval = resultBytes ;
                } else {
                    // we obtained zero bytes -> delete empty bytearray
                    delete resultBytes ;
                }
            }
            break ;
            case UserProfileComment: {
                QByteArray comment ;
                QByteArray commentSignature ;
                QByteArray* resultBytes = new QByteArray() ;
                quint32 flags (0) ;
                quint32 timeWhenProfileWasPublished ;
                Hash profileHash  ;

                if ( iModel.profileCommentModel().commentDataByFingerPrint(itemToPrepare.iHash,
                        comment,
                        commentSignature,
                        &profileHash,
                        &flags,
                        &timeWhenProfileWasPublished ) ) {
                    resultBytes->append(ProtocolMessageFormatter::profileCommentSend(itemToPrepare.iHash,
                                        comment,
                                        commentSignature,
                                        profileHash,
                                        timeWhenProfileWasPublished,
                                        flags ));
                }
                comment.clear() ;
                commentSignature.clear() ;
                if ( resultBytes->size() ) {
                    retval = resultBytes ;
                } else {
                    // we obtained zero bytes -> delete empty bytearray
                    delete resultBytes ;
                }
            }
            break ;
            case PrivateMessage: {
                LOG_STR("NodeModel sendqueue handling for PrivateMessage") ;
                QByteArray msg ;
                QByteArray msgSignature ;
                QByteArray* resultBytes = new QByteArray() ;
                quint32 timeOfPublish ;
                Hash resultingDestination ;
                Hash resultingRecipient ;
                if ( iModel.privateMessageModel().messageDataByFingerPrint(itemToPrepare.iHash,
                        msg,
                        msgSignature,
                        &resultingDestination,
                        &resultingRecipient,
                        &timeOfPublish) ) {
                    // message was found, lets send:
                    resultBytes->append(ProtocolMessageFormatter::privMsgSend(itemToPrepare.iHash,
                                        msg,
                                        msgSignature,
                                        resultingDestination,
                                        resultingRecipient,
                                        timeOfPublish ));
                    msg.clear() ;
                    msgSignature.clear() ;

                    if ( resultBytes->size() ) {
                        retval = resultBytes ;
                    } else {
                        // we obtained zero bytes -> delete empty bytearray
                        delete resultBytes ;
                        QLOG_STR("Got 0 bytes to send as message, id=" + itemToPrepare.iHash.toString()) ;
                    }
                } else {
                    QLOG_STR("Could not find msg supposed to be sent, id=" + itemToPrepare.iHash.toString()) ;
                    delete resultBytes ;
                }
            }
            break ;
            case RequestForSearchSend: {
                // this request has been put into queue locally and is
                // from here spammed to connected nodes:
                QString searchString ;
                bool searchAds ;
                bool searchProfiles ;
                bool searchComments ;
                Hash searchIdentifier ;
                iModel.searchModel()->getSearchCriteria(&searchString,
                                                        &searchAds,
                                                        &searchProfiles,
                                                        &searchComments,
                                                        &searchIdentifier ) ;
                if ( searchIdentifier != KNullHash && searchString.length() > 0 ) {
                    // there were something to search for
                    QByteArray* resultBytes = new QByteArray() ;
                    resultBytes->append(ProtocolMessageFormatter::searchSend(searchString,
                                        searchAds,
                                        searchProfiles,
                                        searchComments,
                                        searchIdentifier));
                    retval = resultBytes ;
                }
            }
            break ;
            default:
                // for rest of the types, there is no need to send
                LOG_STR2("Unhandled send queue item type %d", (int)(itemToPrepare.iItemType)) ;
                break ;
            }
        }
    }
    return retval ;
}

Node* NodeModel::nodeByHash(const Hash& aHash) {
    LOG_STR("NodeModel::nodeByHash in ") ;
    Node *retval = NULL ;
    QSqlQuery query;
    bool ret = query.prepare("select hash1,hash2,hash3,hash4,hash5,listenport,last_conn_time,does_listen,ipv4addr,ipv6addr1,ipv6addr2,ipv6addr3,ipv6addr4,last_nodelist_time,last_mutual_conn_time from node where hash1 = :hash1") ;
    query.bindValue(":hash1", aHash.iHash160bits[0]);
    if ( ret && (ret = query.exec() ) == true && query.next()  ) {
        quint32 hash1 = query.value(0).toUInt() ;
        quint32 hash2 = query.value(1).toUInt() ;
        quint32 hash3 = query.value(2).toUInt() ;
        quint32 hash4 = query.value(3).toUInt() ;
        quint32 hash5 = query.value(4).toUInt() ;
        Hash hashFoundFromDb ( hash1,hash2,hash3,hash4,hash5) ;
        QLOG_STR("Comparing hash " + hashFoundFromDb.toString()) ;
        if ( hashFoundFromDb == aHash ) {
            // yes, found
            retval = new Node ( hashFoundFromDb, (int)(query.value(5).toUInt()) ) ;
            retval->setLastConnectTime(query.value(6).toUInt()) ;
            retval->setCanReceiveIncoming(query.value(7).toUInt() > 0) ;
            if ( !query.isNull(8) ) {
                retval->setIpv4Addr(query.value(8).toUInt()) ;
            }
            if ( !query.isNull(9) ) {
                Q_IPV6ADDR addr = ipv6AddrFromUints ( query.value(9).toUInt(),
                                                      query.value(10).toUInt(),
                                                      query.value(11).toUInt(),
                                                      query.value(12).toUInt()) ;
                retval->setIpv6Addr(addr) ;
            }
            if ( !query.isNull(13) ) {
                retval->setGoodNodeListTime(query.value(13).toUInt()) ;
            }
            if ( !query.isNull(14) ) {
                retval->setLastMutualConnectTime(query.value(14).toUInt()) ;
            }
        }
    }
    if ( !ret ) {
        QLOG_STR(query.lastError().text() + " "+ __FILE__ + QString::number(__LINE__)) ;
        emit error(MController::DbTransactionError, query.lastError().text()) ;
    }
    return retval ;
}

QList<MNodeModelProtocolInterface::HostConnectQueueItem> NodeModel::getHotAddresses() {
    LOG_STR("NodeModel::getHotAddresses in ") ;

    if ( iHotAddresses.size() == 0 ) {
        retrieveListOfHotConnections() ; // fetch recent nodes from db
    }

    if ( iHotAddresses.size() == 0 ) { // if no nodes found
        // then try hard-coded one..
        //      QHostAddress seedNode("canode.katiska.org") ;
        int seedPort ( 31111 ) ;
        const bool hasIpv6 =
            !Connection::Ipv6AddressesEqual(iController->getNode().ipv6Addr(),
                                            KNullIpv6Addr) ;

        QHostAddress addrToConnect ;
        QHostInfo info = QHostInfo::fromName("canode.katiska.org") ;
        if ( info.error() == QHostInfo::NoError ) {
            // check for ipv6 addr if we have one
            foreach ( const QHostAddress& result,
                      info.addresses() ) {
                if ( result.protocol() == QAbstractSocket::IPv6Protocol && hasIpv6 ) {
                    MNodeModelProtocolInterface::HostConnectQueueItem p ;
                    p.iAddress = result ;
                    p.iPort = seedPort ;
                    p.iNodeHash = KNullHash ;
                    iHotAddresses.append(p) ;
                    QLOG_STR("Added seednode IPv6 addr " + result.toString()) ;

                }
                if ( result.protocol() == QAbstractSocket::IPv4Protocol ) {
                    MNodeModelProtocolInterface::HostConnectQueueItem p ;
                    p.iAddress = result ;
                    p.iPort = seedPort ;
                    p.iNodeHash = KNullHash ;
                    iHotAddresses.append(p) ;
                    QLOG_STR("Added seednode IPv4 addr " + result.toString()) ;
                }
            }
        } else {
            QLOG_STR("DNS lookup error for canode.katiska.org") ;
        }
    }
    return iHotAddresses ;
}

QList<Node *>* NodeModel::getHotNodes(int aMaxNodes) {
    LOG_STR("getHotNodes in") ;
    QList<Node* >* retval = new QList<Node *>() ;

    QSqlQuery query;
    bool ret ;

    QString conditional_recently_failed_condition ;
    if ( iRecentlyFailedNodes.size() > 0 ) {
        conditional_recently_failed_condition = " where hash1 not in ( " ;
        for ( int i = iRecentlyFailedNodes.size()-1 ; i >= 0 ; i-- ) {
            conditional_recently_failed_condition += QString::number(iRecentlyFailedNodes[i].first.iHash160bits[0] ) + "," ;
        }
        conditional_recently_failed_condition += "-1 )" ;
    } else {
        conditional_recently_failed_condition  = " " ;
    }
    QLOG_STR("Failed nodes  " + conditional_recently_failed_condition) ;
    ret = query.exec("select hash1,hash2,hash3,hash4,hash5,listenport,last_conn_time,does_listen,ipv4addr,ipv6addr1,ipv6addr2,ipv6addr3,ipv6addr4,last_nodelist_time from node "+conditional_recently_failed_condition+" order by last_conn_time desc") ;
    while ( ret &&
            query.next() &&
            !query.isNull(0) &&
            retval->size() < aMaxNodes ) {
        quint32 hash1,hash2,hash3,hash4,hash5 ;
        hash1 = query.value(0).toUInt() ;
        hash2 = query.value(1).toUInt() ;
        hash3 = query.value(2).toUInt() ;
        hash4 = query.value(3).toUInt() ;
        hash5 = query.value(4).toUInt() ;
        Hash nodeHash (hash1,hash2,hash3,hash4,hash5 ) ;
        QLOG_STR("Hotnodes got node " + nodeHash.toString()) ;
        Node* n = new Node (nodeHash, query.value(5).toUInt()) ;
        if ( !query.isNull(10)) {
            Q_IPV6ADDR addr = ipv6AddrFromUints (query.value(9).toUInt(),
                                                 query.value(10).toUInt(),
                                                 query.value(11).toUInt(),
                                                 query.value(12).toUInt() ) ;
            n->setIpv6Addr(addr) ;
        }
        if ( !query.isNull(8) ) {
            n->setIpv4Addr((quint32)(query.value(8).toUInt())) ;
        }
        if ( !query.isNull(13) ) {
            n->setGoodNodeListTime((time_t)(query.value(13).toUInt())) ;
        }
        if ( !query.isNull(6) ) {
            n->setLastConnectTime((time_t)(query.value(6).toUInt())) ;
        }
        if ( !query.isNull(7) ) {
            n->setCanReceiveIncoming((bool)(query.value(7).toUInt())) ;
        }
        retval->append(n) ;
    }
    LOG_STR("getHotNodes out") ;
    return retval ;
}


QList<Node *>* NodeModel::getNodesAfterHash(const Hash& aHash,
        unsigned aMaxNodes,
        int aMaxInactivityMinutes) {
    LOG_STR("getNodesAfterHash in") ;
    // hmm hmm.
    // this program will do a lot of hash-matching, but in practice
    // the matches are made so that we know hash of some content
    // and that hash is exactly known in advance so search from db
    // table is easy. the "matching" part is about finding a node
    // whose hash is near the content hash and this is where this
    // method here comes into picture. ..if we keep only 10k
    // nodes in db table we can quite safely do things resulting
    // in full table scan and the performance will still be
    // in ok level .. or lets see. if not we'll need an extra column
    // that will somehow contain distance from hash in form that
    // is usable also when hash number rolls over
    QList<Node* >* retval = new QList<Node *>() ;

    QSqlQuery query;
    bool ret ;

    if ( aMaxNodes > iCurrentDbTableRowCount ) {
        aMaxNodes = iCurrentDbTableRowCount ; // don't try returning more than we have
    }
    // this is fine, will use index as there is index for hash1
    QString conditional_inactivity_condition ;
    if ( aMaxInactivityMinutes > 0 ) {
        conditional_inactivity_condition = "and last_conn_time > :last_conn_time ";
    } else {
        conditional_inactivity_condition = " ";
    }
    QString conditional_recently_failed_condition ;
    if ( iRecentlyFailedNodes.size() > 0 ) {
        conditional_recently_failed_condition = " and hash1 not in ( " ;
        for ( int i = iRecentlyFailedNodes.size()-1 ; i >= 0 ; i-- ) {
            conditional_recently_failed_condition += QString::number(iRecentlyFailedNodes[i].first.iHash160bits[0] ) + "," ;
        }
        conditional_recently_failed_condition += " -1)" ;
    } else {
        conditional_recently_failed_condition  = " " ;
    }
    QLOG_STR("Failed nodes  " + conditional_recently_failed_condition) ;
    ret = query.prepare("select hash1,hash2,hash3,hash4,hash5,listenport,last_conn_time,does_listen,ipv4addr,ipv6addr1,ipv6addr2,ipv6addr3,ipv6addr4,last_nodelist_time,dns_name,tor_name from node where ( ipv4addr is not null or ipv6addr1 is not null ) and hash1 >= :hash_to_seek "+
                        conditional_inactivity_condition +
                        conditional_recently_failed_condition +
                        " order by hash1 limit :maxrows") ;
    query.bindValue(":hash_to_seek", aHash.iHash160bits[0]);
    query.bindValue(":maxrows", aMaxNodes);
    if ( aMaxInactivityMinutes > 0 ) {
        query.bindValue(":last_conn_time", (unsigned)(QDateTime::currentDateTimeUtc().toTime_t()-(aMaxInactivityMinutes*60)));
    }
    if ( ret && query.exec() ) {
        while ( ret &&
                query.next() &&
                !query.isNull(0) &&
                ((unsigned)(retval->size())) < aMaxNodes ) {
            quint32 hash1,hash2,hash3,hash4,hash5 ;
            hash1 = query.value(0).toUInt() ;
            hash2 = query.value(1).toUInt() ;
            hash3 = query.value(2).toUInt() ;
            hash4 = query.value(3).toUInt() ;
            hash5 = query.value(4).toUInt() ;
            Hash nodeHash (hash1,hash2,hash3,hash4,hash5 ) ;
            QLOG_STR("getNodesAfterHash got node " + nodeHash.toString()) ;
            Node* n = new Node (nodeHash, query.value(5).toUInt()) ;
            if ( !query.isNull(10)) {
                Q_IPV6ADDR addr = ipv6AddrFromUints (query.value(9).toUInt(),
                                                     query.value(10).toUInt(),
                                                     query.value(11).toUInt(),
                                                     query.value(12).toUInt() ) ;
                n->setIpv6Addr(addr) ;
            }
            if ( !query.isNull(8) ) {
                n->setIpv4Addr((quint32)(query.value(8).toUInt())) ;
            }
            if ( !query.isNull(13) ) {
                n->setGoodNodeListTime((time_t)(query.value(13).toUInt())) ;
            }
            if ( !query.isNull(6) ) {
                n->setLastConnectTime((time_t)(query.value(6).toUInt())) ;
            }
            if ( !query.isNull(7) ) {
                n->setCanReceiveIncoming((bool)(query.value(7).toUInt())) ;
            }
            if ( !query.isNull(14) ) {
                n->setDNSAddr(query.value(14).toString())  ;
            }
            if ( !query.isNull(15) ) {
                n->setTORAddr(query.value(15).toString())  ;
            }
            retval->append(n) ;
        }
    }
    if ( (unsigned)(retval->size()) < aMaxNodes ) {
        // we got less nodes so that means that hash we were trying
        // to seek was so close to end of list that there were less
        // than aMaxNodes nodes in table after the hash we were
        // seeking. so so .. here roll over, start from node zero
        QSqlQuery query2;

        // note here: no where-condition but only order by.
        // this will start from first node, return nodes
        // starting in order from zero, until maxrows is
        // reached. .. so in case we need to roll-over the
        // hash-space, this 2nd query here is our implementation
        // for the rolling-over.
        if ( aMaxInactivityMinutes > 0 ) {
            conditional_inactivity_condition = " and last_conn_time > :last_conn_time ";
        } else {
            conditional_inactivity_condition = " ";
        }

        ret = query2.prepare("select hash1,hash2,hash3,hash4,hash5,listenport,last_conn_time,does_listen,ipv4addr,ipv6addr1,ipv6addr2,ipv6addr3,ipv6addr4,last_nodelist_time,dns_name,tor_name from node where ( ipv4addr is not null or ipv6addr1 is not null ) "+
                             conditional_inactivity_condition +
                             conditional_recently_failed_condition +
                             " order by hash1 limit :maxrows") ;
        query2.bindValue(":maxrows", aMaxNodes - retval->size());
        if ( aMaxInactivityMinutes > 0 ) {
            query2.bindValue(":last_conn_time", (unsigned)(QDateTime::currentDateTimeUtc().toTime_t()-(aMaxInactivityMinutes*60)));
        }
        if ( ret && query2.exec() ) {
            while ( ret &&
                    query2.next() &&
                    !query2.isNull(0) &&
                    (((unsigned)(retval->size())) < aMaxNodes) ) {
                quint32 hash1,hash2,hash3,hash4,hash5 ;
                hash1 = query2.value(0).toUInt() ;
                hash2 = query2.value(1).toUInt() ;
                hash3 = query2.value(2).toUInt() ;
                hash4 = query2.value(3).toUInt() ;
                hash5 = query2.value(4).toUInt() ;
                Hash nodeHash (hash1,hash2,hash3,hash4,hash5 ) ;
                Node* n = new Node (nodeHash, query2.value(5).toUInt()) ;
                if ( !query2.isNull(10)) {
                    Q_IPV6ADDR addr = ipv6AddrFromUints (query2.value(9).toUInt(),
                                                         query2.value(10).toUInt(),
                                                         query2.value(11).toUInt(),
                                                         query2.value(12).toUInt() ) ;
                    n->setIpv6Addr(addr) ;
                }
                if ( !query2.isNull(8) ) {
                    n->setIpv4Addr((quint32)(query2.value(8).toUInt())) ;
                }
                if ( !query2.isNull(13) ) {
                    n->setGoodNodeListTime((time_t)(query2.value(13).toUInt())) ;
                }
                if ( !query2.isNull(6) ) {
                    n->setLastConnectTime((time_t)(query2.value(6).toUInt())) ;
                }
                if ( !query2.isNull(7) ) {
                    n->setCanReceiveIncoming((bool)(query2.value(7).toUInt())) ;
                }
                if ( !query2.isNull(14) ) {
                    n->setDNSAddr(query2.value(14).toString())  ;
                }
                if ( !query2.isNull(15) ) {
                    n->setTORAddr(query2.value(15).toString())  ;
                }
                retval->append(n) ;
            }
        }
    }
    // still got less nodes than expected: return any connected nodes
    int nodesMissing ( aMaxNodes - retval->size() ) ;
    if ( (unsigned)(retval->size()) < aMaxNodes ) {
        const bool hasIpv6 =
            !Connection::Ipv6AddressesEqual(iController->getNode().ipv6Addr(),
                                            KNullIpv6Addr) ;
        const QList<Connection *>& openConnections ( iModel.getConnections());
        foreach ( const Connection *connection, openConnections ) {
            if ( connection->node() ) {
                const Node* connectedNode ( connection->node() ) ;
                if ( connectedNode->ipv4Addr() ||
                        (
                            !Connection::Ipv6AddressesEqual(connectedNode->ipv6Addr(),
                                    KNullIpv6Addr) && hasIpv6
                        )
                   ) {
                    Node* n = new Node (connectedNode->nodeFingerPrint(), connectedNode->port()) ;
                    n->setIpv6Addr(connectedNode->ipv6Addr()) ;
                    n->setIpv4Addr(connectedNode->ipv4Addr()) ;
                    n->setGoodNodeListTime(connectedNode->goodNodeListTime()) ;
                    // is connected now:
                    n->setLastConnectTime(QDateTime::currentDateTimeUtc().toTime_t()) ;
                    n->setCanReceiveIncoming(connectedNode->canReceiveIncoming()) ;
                    n->setDNSAddr(connectedNode->DNSAddr())  ;
                    n->setTORAddr(connectedNode->TORAddr())  ;
                    retval->append(n) ;

                }
                if ( --nodesMissing < 0 ) {
                    break ;
                }
            }
        }
    }
    return retval ;
}

//
// see also getNodesAfterHash
//
QList<Node *>* NodeModel::getNodesBeforeHash(const Hash& aHash,
        unsigned aMaxNodes) {
    QList<Node* >* retval = new QList<Node *>() ;

    QSqlQuery query;
    bool ret ;

    if ( aMaxNodes > iCurrentDbTableRowCount ) {
        aMaxNodes = iCurrentDbTableRowCount ; // don't try returning more than we have
    }
    QString conditional_recently_failed_condition ;
    if ( iRecentlyFailedNodes.size() > 0 ) {
        conditional_recently_failed_condition = " and hash1 not in ( " ;
        for ( int i = iRecentlyFailedNodes.size()-1 ; i >= 0 ; i-- ) {
            conditional_recently_failed_condition += QString::number(iRecentlyFailedNodes[i].first.iHash160bits[0] ) + "," ;
        }
        conditional_recently_failed_condition += " -1)" ;
    } else {
        conditional_recently_failed_condition  = " " ;
    }
    QLOG_STR("Failed nodes  " + conditional_recently_failed_condition) ;
    // this is fine, will use index as there is index for hash1
    ret = query.prepare("select hash1,hash2,hash3,hash4,hash5,listenport,last_conn_time,does_listen,ipv4addr,ipv6addr1,ipv6addr2,ipv6addr3,ipv6addr4,last_nodelist_time,dns_name,tor_name from node where ( ipv4addr is not null or ipv6addr1 is not null ) and hash1 <= :hash_to_seek "+conditional_recently_failed_condition+" order by hash1 desc limit :maxrows") ;
    query.bindValue(":hash_to_seek", aHash.iHash160bits[0]);
    query.bindValue(":maxrows", aMaxNodes);
    if ( ret && query.exec() ) {
        while ( ret &&
                query.next() &&
                !query.isNull(0) &&
                (unsigned)(retval->size()) < aMaxNodes ) {
            quint32 hash1,hash2,hash3,hash4,hash5 ;
            hash1 = query.value(0).toUInt() ;
            hash2 = query.value(1).toUInt() ;
            hash3 = query.value(2).toUInt() ;
            hash4 = query.value(3).toUInt() ;
            hash5 = query.value(4).toUInt() ;
            Hash nodeHash (hash1,hash2,hash3,hash4,hash5 ) ;
            Node* n = new Node (nodeHash, query.value(5).toUInt()) ;
            if ( !query.isNull(10)) {
                Q_IPV6ADDR addr = ipv6AddrFromUints (query.value(9).toUInt(),
                                                     query.value(10).toUInt(),
                                                     query.value(11).toUInt(),
                                                     query.value(12).toUInt() ) ;
                n->setIpv6Addr(addr) ;
            }
            if ( !query.isNull(8) ) {
                n->setIpv4Addr((quint32)(query.value(8).toUInt())) ;
            }
            if ( !query.isNull(13) ) {
                n->setGoodNodeListTime((time_t)(query.value(13).toUInt())) ;
            }
            if ( !query.isNull(6) ) {
                n->setLastConnectTime((time_t)(query.value(6).toUInt())) ;
            }
            if ( !query.isNull(7) ) {
                n->setCanReceiveIncoming((bool)(query.value(7).toUInt())) ;
            }
            if ( !query.isNull(14) ) {
                n->setDNSAddr(query.value(14).toString())  ;
            }
            if ( !query.isNull(15) ) {
                n->setTORAddr(query.value(15).toString())  ;
            }
            retval->append(n) ;
        }
    }
    if ( (unsigned)(retval->size()) < aMaxNodes ) {
        // we got less nodes so that means that hash we were trying
        // to seek was so close to end of list that there were less
        // than aMaxNodes nodes in table after the hash we were
        // seeking. so so .. here roll over, start from node zero
        QSqlQuery query2;

        // note here: no where-condition but only order by.
        // this will start from first node, return nodes
        // starting in order from max value, until maxrows is
        // reached. .. so in case we need to roll-over the
        // hash-space, this 2nd query here is our implementation
        // for the rolling-over.
        ret = query2.prepare("select hash1,hash2,hash3,hash4,hash5,listenport,last_conn_time,does_listen,ipv4addr,ipv6addr1,ipv6addr2,ipv6addr3,ipv6addr4,last_nodelist_time from node where ( ipv4addr is not null or ipv6addr1 is not null ) " + conditional_recently_failed_condition + " order by hash1 desc limit :maxrows") ;
        query2.bindValue(":maxrows", aMaxNodes - (unsigned)(retval->size()));
        if ( ret && query2.exec() ) {
            while ( ret &&
                    query2.next() &&
                    !query2.isNull(0) &&
                    ((unsigned)(retval->size())) < aMaxNodes ) {
                quint32 hash1,hash2,hash3,hash4,hash5 ;
                hash1 = query2.value(0).toUInt() ;
                hash2 = query2.value(1).toUInt() ;
                hash3 = query2.value(2).toUInt() ;
                hash4 = query2.value(3).toUInt() ;
                hash5 = query2.value(4).toUInt() ;
                Hash nodeHash (hash1,hash2,hash3,hash4,hash5 ) ;
                Node* n = new Node (nodeHash, query2.value(5).toUInt()) ;
                if ( !query2.isNull(10)) {
                    Q_IPV6ADDR addr = ipv6AddrFromUints (query2.value(9).toUInt(),
                                                         query2.value(10).toUInt(),
                                                         query2.value(11).toUInt(),
                                                         query2.value(12).toUInt() ) ;
                    n->setIpv6Addr(addr) ;
                }
                if ( !query2.isNull(8) ) {
                    n->setIpv4Addr((quint32)(query2.value(8).toUInt())) ;
                }
                if ( !query2.isNull(13) ) {
                    n->setGoodNodeListTime((time_t)(query2.value(13).toUInt())) ;
                }
                if ( !query2.isNull(6) ) {
                    n->setLastConnectTime((time_t)(query2.value(6).toUInt())) ;
                }
                if ( !query2.isNull(7) ) {
                    n->setCanReceiveIncoming((bool)(query2.value(7).toUInt())) ;
                }
                if ( !query2.isNull(14) ) {
                    n->setDNSAddr(query2.value(14).toString())  ;
                }
                if ( !query2.isNull(15) ) {
                    n->setTORAddr(query2.value(15).toString())  ;
                }
                retval->append(n) ;
            }
        }
    }
    return retval ;
}

Q_IPV6ADDR NodeModel::ipv6AddrFromUints(quint32 aLeastSignificant,
                                        quint32 aLessSignificant,
                                        quint32 aMoreSignificant,
                                        quint32 aMostSignificant ) const {
    Q_IPV6ADDR addr ;
    quint32 ipv6addrPart = aLeastSignificant ;
    addr.c[0] = (ipv6addrPart & 0x000000FF) ;
    addr.c[1] = (ipv6addrPart & 0x0000FF00) >> 8 ;
    addr.c[2] = (ipv6addrPart & 0x00FF0000) >> 16 ;
    addr.c[3] = (ipv6addrPart & 0xFF000000) >> 24 ;
    ipv6addrPart = aLessSignificant ;
    addr.c[4] = (ipv6addrPart & 0x000000FF) ;
    addr.c[5] = (ipv6addrPart & 0x0000FF00) >> 8 ;
    addr.c[6] = (ipv6addrPart & 0x00FF0000) >> 16 ;
    addr.c[7] = (ipv6addrPart & 0xFF000000) >> 24 ;
    ipv6addrPart = aMoreSignificant ;
    addr.c[8]  = (ipv6addrPart & 0x000000FF) ;
    addr.c[9]  = (ipv6addrPart & 0x0000FF00) >> 8 ;
    addr.c[10] = (ipv6addrPart & 0x00FF0000) >> 16 ;
    addr.c[11] = (ipv6addrPart & 0xFF000000) >> 24 ;
    ipv6addrPart =aMostSignificant ;
    addr.c[12] = (ipv6addrPart & 0x000000FF) ;
    addr.c[13] = (ipv6addrPart & 0x0000FF00) >> 8 ;
    addr.c[14] = (ipv6addrPart & 0x00FF0000) >> 16 ;
    addr.c[15] = (ipv6addrPart & 0xFF000000) >> 24 ;
    return addr ;
}

bool NodeModel::nodeGreetingReceived(Node& aNode,
                                     bool aWasInitialGreeting ) {
    LOG_STR("NodeModel::nodeGreetingReceived in ") ;
    bool retval = true ;
    Node *previousEntry = nodeByHash(aNode.nodeFingerPrint() ) ;
    time_t previousConnectTime ( 0 ) ;
    if ( aWasInitialGreeting && previousEntry ) {
        aNode.setLastMutualConnectTime(previousEntry->lastMutualConnectTime()) ;
    }
    if ( aWasInitialGreeting ) {
        aNode.setLastConnectTime(QDateTime::currentDateTimeUtc().toTime_t()) ;
        removeNodeFromRecentlyFailedList(aNode.nodeFingerPrint()) ;
    }
    if ( previousEntry ) {
        previousConnectTime= previousEntry->lastConnectTime() ;
        // ok, we had previous information, now we have new ; what
        // data to merge?
        if ( aNode.lastConnectTime() > previousEntry->lastConnectTime() ) {
            // aNode is more recent, trust that more. Note that if
            // we came here via node greeting up-on connect then
            // time of last connect has been set to wall-clock time
            // just recently so most likely we have more up-to-date
            // information here.
            aNode.setCanReceiveIncoming(aNode.canReceiveIncoming() ||
                                        previousEntry->canReceiveIncoming() ) ;
            if ( aNode.lastMutualConnectTime() < previousEntry->lastMutualConnectTime()) {
                aNode.setLastMutualConnectTime(previousEntry->lastMutualConnectTime());
            }
            retval = this->updateNodeInDb(aNode) ;
        } else {
            // our own entry was more recent
            previousEntry->setCanReceiveIncoming(aNode.canReceiveIncoming() ||
                                                 previousEntry->canReceiveIncoming() ) ;
            if ( aNode.lastMutualConnectTime() > previousEntry->lastMutualConnectTime()) {
                previousEntry->setLastMutualConnectTime(aNode.lastMutualConnectTime());
            }
            this->updateNodeInDb(*previousEntry) ;
        }
        delete previousEntry ;
    } else {
        LOG_STR("Node was new to us") ;
        retval = this->insertNodeToDb(aNode) ;
    }
    if ( aWasInitialGreeting ) {
        // this was not normal node ref but the initial greeting
        // sent as first time after connect -> lets grab stuff
        // that needs to be sent to that node

        // so send node greetings so that network stays alive.
        // actual bucket-filling code is in connection..
        struct NetworkRequestExecutor::NetworkRequestQueueItem hotNodesSendingReq ;
        hotNodesSendingReq.iRequestType = NodeGreeting ;
        hotNodesSendingReq.iDestinationNode = aNode.nodeFingerPrint() ;
        // so, automatically send noderefs around that node so
        // that it will know its own neighborhood
        hotNodesSendingReq.iTimeStampOfItem = previousConnectTime ;
        hotNodesSendingReq.iState = NetworkRequestExecutor::NewRequest ;
        hotNodesSendingReq.iMaxNumberOfItems = KNumberOfNodesToSendToEachPeer;
        iModel.addNetworkRequest(hotNodesSendingReq) ;
    }
    return retval ;
}

bool NodeModel::updateNodeLastConnectTimeInDb(Node& aNode) {
    QSqlQuery query;

    bool ret = query.prepare("update node set last_conn_time=:last_conn_time,time_last_reference=:time_last_reference where hash1=:hash1 and hash2=:hash2 and hash3=:hash3 and hash4=:hash4 and hash5=:hash5") ;
    if ( ret ) {
        query.bindValue(":hash1", aNode.nodeFingerPrint().iHash160bits[0]);
        query.bindValue(":hash2", aNode.nodeFingerPrint().iHash160bits[1]);
        query.bindValue(":hash3", aNode.nodeFingerPrint().iHash160bits[2]);
        query.bindValue(":hash4", aNode.nodeFingerPrint().iHash160bits[3]);
        query.bindValue(":hash5", aNode.nodeFingerPrint().iHash160bits[4]);

        query.bindValue(":last_conn_time", (quint32)(aNode.lastConnectTime()));
        query.bindValue(":time_last_reference", (quint32)(aNode.lastConnectTime()));
        ret = query.exec() ;
    }
    if ( !ret ) {
        QLOG_STR(query.lastError().text() + " "+ __FILE__ + QString::number(__LINE__)) ;
        emit error(MController::DbTransactionError, query.lastError().text()) ;
    }
    return ret ;
}

bool NodeModel::updateNodeLastMutualConnectTimeInDb(const Hash& aNodeFp,
        quint32 aTime ) {
    QSqlQuery query;

    bool ret = query.prepare("update node set last_mutual_conn_time=:last_mutual_conn_time,last_conn_time=:last_conn_time,time_last_reference=:time_last_reference where hash1=:hash1 and hash2=:hash2 and hash3=:hash3 and hash4=:hash4 and hash5=:hash5") ;
    if ( ret ) {
        query.bindValue(":hash1", aNodeFp.iHash160bits[0]);
        query.bindValue(":hash2", aNodeFp.iHash160bits[1]);
        query.bindValue(":hash3", aNodeFp.iHash160bits[2]);
        query.bindValue(":hash4", aNodeFp.iHash160bits[3]);
        query.bindValue(":hash5", aNodeFp.iHash160bits[4]);

        query.bindValue(":last_conn_time", aTime);
        query.bindValue(":last_mutual_conn_time", aTime);
        query.bindValue(":time_last_reference", aTime);

        ret = query.exec() ;
    }
    if ( !ret ) {
        QLOG_STR(query.lastError().text() + " "+ __FILE__ + QString::number(__LINE__)) ;
        emit error(MController::DbTransactionError, query.lastError().text()) ;
    }
    return ret ;
}

bool NodeModel::updateNodeInDb(Node& aNode) {
    QSqlQuery query;

    bool ret = query.prepare("update node set listenport=:listenport,last_conn_time=:last_conn_time,does_listen=:does_listen,ipv4addr=:ipv4addr,ipv6addr1=:ipv6addr1,ipv6addr2=:ipv6addr2,ipv6addr3=:ipv6addr3,ipv6addr4=:ipv6addr4,last_nodelist_time=:last_nodelist_time,last_mutual_conn_time=:last_mutual_conn_time,dns_name=:dns_name,tor_name=:tor_name where hash1=:hash1 and hash2=:hash2 and hash3=:hash3 and hash4=:hash4 and hash5=:hash5") ;
    if ( ret ) {
        query.bindValue(":hash1", aNode.nodeFingerPrint().iHash160bits[0]);
        query.bindValue(":hash2", aNode.nodeFingerPrint().iHash160bits[1]);
        query.bindValue(":hash3", aNode.nodeFingerPrint().iHash160bits[2]);
        query.bindValue(":hash4", aNode.nodeFingerPrint().iHash160bits[3]);
        query.bindValue(":hash5", aNode.nodeFingerPrint().iHash160bits[4]);
        // ..listenport is not supposed to change but .. maybe we'll have
        // UI feature for setting the port manually?
        query.bindValue(":listenport", (quint32)(aNode.port()));
        query.bindValue(":last_conn_time", (quint32)(aNode.lastConnectTime()));
        // if node has no network addr of any kind, don't set
        // "can receive incoming" as clearly there is no
        // addr to connect to:
        if ( aNode.ipv4Addr() == 0 &&
                Connection::Ipv6AddressesEqual(aNode.ipv6Addr(), KNullIpv6Addr )) {
            query.bindValue(":does_listen", 0); // here 0 means false
        } else {
            query.bindValue(":does_listen", aNode.canReceiveIncoming()>0);
        }
        query.bindValue(":last_mutual_conn_time", (quint32)(aNode.lastMutualConnectTime()));
        if ( aNode.ipv4Addr()>0 ) {
            query.bindValue(":ipv4addr", aNode.ipv4Addr());
        } else {
            // interesting ; null value is null string. even if the
            // db column is integer.
            query.bindValue(":ipv4addr", QVariant(QVariant::String));
        }
        Q_IPV6ADDR ipv6addr = aNode.ipv6Addr()  ;
        if ( Connection::Ipv6AddressesEqual(ipv6addr, KNullIpv6Addr ) ) {
            // has no ipv6, set null
            query.bindValue(":ipv6addr1", QVariant(QVariant::String));
            query.bindValue(":ipv6addr2", QVariant(QVariant::String));
            query.bindValue(":ipv6addr3", QVariant(QVariant::String));
            query.bindValue(":ipv6addr4", QVariant(QVariant::String));
        } else {
            query.bindValue(":ipv6addr1",
                            (quint32)(ipv6addr.c[0]) +
                            ((quint32)(ipv6addr.c[1]) << 8) +
                            ((quint32)(ipv6addr.c[2]) << 16) +
                            ((quint32)(ipv6addr.c[3]) << 24));
            query.bindValue(":ipv6addr2",
                            (quint32)(ipv6addr.c[4]) +
                            ((quint32)(ipv6addr.c[5]) << 8) +
                            ((quint32)(ipv6addr.c[6]) << 16) +
                            ((quint32)(ipv6addr.c[7]) << 24));
            query.bindValue(":ipv6addr3",
                            (quint32)(ipv6addr.c[8]) +
                            ((quint32)(ipv6addr.c[9]) << 8) +
                            ((quint32)(ipv6addr.c[10]) << 16) +
                            ((quint32)(ipv6addr.c[11]) << 24));
            query.bindValue(":ipv6addr4",
                            (quint32)(ipv6addr.c[12]) +
                            ((quint32)(ipv6addr.c[13]) << 8) +
                            ((quint32)(ipv6addr.c[14]) << 16) +
                            ((quint32)(ipv6addr.c[15]) << 24));
        }
        query.bindValue(":last_nodelist_time", (quint32)(aNode.goodNodeListTime())) ;
        if ( aNode.DNSAddr().length() > 0 ) {
            query.bindValue(":dns_name", aNode.DNSAddr());
        } else {
            query.bindValue(":dns_name", QVariant(QVariant::String));
        }
        if ( aNode.TORAddr().length() > 0 ) {
            query.bindValue(":tor_name", aNode.TORAddr());
        } else {
            query.bindValue(":tor_name", QVariant(QVariant::String));
        }
        ret = query.exec() ;
        QLOG_STR("updated node port "+QString::number(aNode.port())+" hash " + aNode.nodeFingerPrint().toString()) ;
    }
    if ( !ret ) {
        QLOG_STR(query.lastError().text() + " "+ __FILE__ + QString::number(__LINE__)) ;
        emit error(MController::DbTransactionError, query.lastError().text()) ;
    }
    return ret ;
}

bool NodeModel::insertNodeToDb(Node& aNode) {
    bool ret ;
    QSqlQuery query;
    ret = query.prepare ("insert into node (hash1,hash2,hash3,hash4,hash5,listenport,last_conn_time,does_listen,ipv4addr,ipv6addr1,ipv6addr2,ipv6addr3,ipv6addr4,last_nodelist_time,last_mutual_conn_time,time_last_reference,dns_name,tor_name) values (:hash1,:hash2,:hash3,:hash4,:hash5,:listenport,:last_conn_time,:does_listen,:ipv4addr,:ipv6addr1,:ipv6addr2,:ipv6addr3,:ipv6addr4,:last_nodelist_time,:last_mutual_conn_time,:time_last_reference,:dns_name,:tor_name)" ) ;
    if ( ret ) {
        query.bindValue(":hash1", aNode.nodeFingerPrint().iHash160bits[0]);
        query.bindValue(":hash2", aNode.nodeFingerPrint().iHash160bits[1]);
        query.bindValue(":hash3", aNode.nodeFingerPrint().iHash160bits[2]);
        query.bindValue(":hash4", aNode.nodeFingerPrint().iHash160bits[3]);
        query.bindValue(":hash5", aNode.nodeFingerPrint().iHash160bits[4]);
        // ..listenport is not supposed to change but .. maybe we'll have
        // UI feature for setting the port manually?
        query.bindValue(":listenport", (quint32)(aNode.port()));
        query.bindValue(":last_conn_time", (quint32)(aNode.lastConnectTime()));
        // for nodes put connection time also into time_last_reference
        // as we use that for removing old entries from table, see
        // base-class ModelBase for details
        query.bindValue(":time_last_reference", (quint32)(aNode.lastConnectTime()));
        query.bindValue(":last_mutual_conn_time", (quint32)(aNode.lastMutualConnectTime()));
        query.bindValue(":does_listen", aNode.canReceiveIncoming()>0);
        if ( aNode.ipv4Addr()>0 ) {
            query.bindValue(":ipv4addr", aNode.ipv4Addr());
        } else {
            // interesting ; null value is null string. even if the
            // db column is integer.
            query.bindValue(":ipv4addr", QVariant(QVariant::String));
        }
        Q_IPV6ADDR ipv6addr = aNode.ipv6Addr()  ;
        if ( Connection::Ipv6AddressesEqual(ipv6addr, KNullIpv6Addr ) ) {
            // has no ipv6, set null
            query.bindValue(":ipv6addr1", QVariant(QVariant::String));
            query.bindValue(":ipv6addr2", QVariant(QVariant::String));
            query.bindValue(":ipv6addr3", QVariant(QVariant::String));
            query.bindValue(":ipv6addr4", QVariant(QVariant::String));
        } else {
            query.bindValue(":ipv6addr1",
                            (quint32)(ipv6addr.c[0]) +
                            ((quint32)(ipv6addr.c[1]) << 8) +
                            ((quint32)(ipv6addr.c[2]) << 16) +
                            ((quint32)(ipv6addr.c[3]) << 24));
            query.bindValue(":ipv6addr2",
                            (quint32)(ipv6addr.c[4]) +
                            ((quint32)(ipv6addr.c[5]) << 8) +
                            ((quint32)(ipv6addr.c[6]) << 16) +
                            ((quint32)(ipv6addr.c[7]) << 24));
            query.bindValue(":ipv6addr3",
                            (quint32)(ipv6addr.c[8]) +
                            ((quint32)(ipv6addr.c[9]) << 8) +
                            ((quint32)(ipv6addr.c[10]) << 16) +
                            ((quint32)(ipv6addr.c[11]) << 24));
            query.bindValue(":ipv6addr4",
                            (quint32)(ipv6addr.c[12]) +
                            ((quint32)(ipv6addr.c[13]) << 8) +
                            ((quint32)(ipv6addr.c[14]) << 16) +
                            ((quint32)(ipv6addr.c[15]) << 24));
        }
        if ( aNode.DNSAddr().length() > 0 ) {
            query.bindValue(":dns_name", aNode.DNSAddr());
        } else {
            query.bindValue(":dns_name", QVariant(QVariant::String));
        }
        if ( aNode.TORAddr().length() > 0 ) {
            query.bindValue(":tor_name", aNode.TORAddr());
        } else {
            query.bindValue(":tor_name", QVariant(QVariant::String));
        }
        query.bindValue(":last_nodelist_time", (quint32)(aNode.goodNodeListTime())) ;

        ret = query.exec() ;
        QLOG_STR("inserted node port "+QString::number(aNode.port())+" hash " + aNode.nodeFingerPrint().toString()) ;
    }
    if ( !ret ) {
        QLOG_STR(query.lastError().text() + " "+ __FILE__ + QString::number(__LINE__)) ;
        emit error(MController::DbTransactionError, query.lastError().text()) ;
    } else {
        iCurrentDbTableRowCount++ ;
    }
    return ret ;
}



void NodeModel::retrieveListOfHotConnections() {
    bool ret = false ;
    iHotAddresses.clear() ;
    int port = -1  ;
    QSqlQuery query;
    QLOG_STR("Hot addresses in") ;
    const bool hasIpv6 =
        !Connection::Ipv6AddressesEqual(iController->getNode().ipv6Addr(),
                                        KNullIpv6Addr) ;
    if ( iFingerPrintOfThisNode ) { // must be self initialized
        QLOG_STR("Hot addresses, my hash = " + iFingerPrintOfThisNode->toString()) ;

        QString conditional_recently_failed_condition ;
        if ( iRecentlyFailedNodes.size() > 0 ) {
            conditional_recently_failed_condition = " where hash1 not in ( " ;
            for ( int i = iRecentlyFailedNodes.size()-1 ; i >= 0 ; i-- ) {
                conditional_recently_failed_condition += QString::number(iRecentlyFailedNodes[i].first.iHash160bits[0] ) + "," ;
            }
            conditional_recently_failed_condition += " -1)" ;
        } else {
            conditional_recently_failed_condition  = " " ;
        }
        QLOG_STR("Failed nodes  " + conditional_recently_failed_condition) ;
        ret = query.exec("select listenport,ipv4addr,ipv6addr1,ipv6addr2,ipv6addr3,ipv6addr4,hash1,hash2,hash3,hash4,hash5 from node "+conditional_recently_failed_condition+" order by last_conn_time desc") ;
        while ( ret &&
                query.next() &&
                !query.isNull(0) &&
                iHotAddresses.size() < 300 ) {
            port = query.value(0).toInt() ;

            Hash hashOfRetrievedNode ( query.value(6).toUInt(),
                                       query.value(7).toUInt(),
                                       query.value(8).toUInt(),
                                       query.value(9).toUInt(),
                                       query.value(10).toUInt() ) ;


            if (  hashOfRetrievedNode != *iFingerPrintOfThisNode  ) { // don't connect to self
                bool alreadyConnected = false ;
                for ( int i = iModel.getConnections().size()-1 ; i >= 0 ; i-- ) {
                    // and don't connect to already-connected nodes
                    const Node* nodeOfConnection = iModel.getConnections().value(i)->node() ;
                    if ( nodeOfConnection &&
                            ( nodeOfConnection->nodeFingerPrint() == hashOfRetrievedNode  ) ) {
                        QLOG_STR("Hot: Was already connected " + hashOfRetrievedNode.toString()) ;
                        alreadyConnected = true ;
                        break ;
                    }
                }
                if ( !alreadyConnected ) {
                    if ( hasIpv6 && !query.isNull(2)) {
                        // prefer ipv6
                        Q_IPV6ADDR addr = ipv6AddrFromUints (query.value(2).toUInt(),
                                                             query.value(3).toUInt(),
                                                             query.value(4).toUInt(),
                                                             query.value(5).toUInt() ) ;

                        QHostAddress a(addr) ;
                        MNodeModelProtocolInterface::HostConnectQueueItem p ;
                        p.iAddress = a ;
                        p.iPort = port ;
                        p.iNodeHash = hashOfRetrievedNode ;
                        if ( !iHotAddresses.contains(p) ) {
                            iHotAddresses.append(p) ;
                            QLOG_STR("Hot: Added hot addr " + a.toString()) ;
                        }
                    } else if ( !query.isNull(1) ) {
                        QHostAddress a((quint32)(query.value(1).toUInt())) ;
                        MNodeModelProtocolInterface::HostConnectQueueItem p ;
                        p.iAddress = a ;
                        p.iPort = port ;
                        p.iNodeHash = hashOfRetrievedNode ;
                        if ( !iHotAddresses.contains(p) ) {
                            iHotAddresses.append(p) ;
                            QLOG_STR("Hot: Added hot addr " + a.toString()) ;
                        }
                    }
                }
            } else {
                QLOG_STR("Not adding self node to list of hot addresses") ;
            }
        }
    }
    QLOG_STR("Hot addresses out") ;
}


void NodeModel::closeOldestInactiveConnection() {
    Connection *c = NULL ;
    int indexFound = -1 ;
    int numberOfNodesInPrivateAddressSpace ( 0 ) ;
    QList <NetworkRequestExecutor::NetworkRequestQueueItem>& requests ( iModel.getNetRequests() ) ;
    // construct a map^H^H^Hlist for faster lookups of hashes:
    QVector<Hash> nodesOfNetworkRequests ;
    for ( int i = requests.size()-1 ; i >= 0 ; i-- ) {
        if ( requests.at(i).iDestinationNode != KNullHash ) {
            if ( !nodesOfNetworkRequests.contains(requests.at(i).iDestinationNode) ) {
                nodesOfNetworkRequests.append(requests.at(i).iDestinationNode) ;
            }
        }
    }

    time_t oldestActivityTimeFound = QDateTime::currentDateTimeUtc().toTime_t() ;
    for ( int j = iModel.getConnections().size()-1 ; j >= 0 ; j-- ) {
        c = iModel.getConnections().value(j) ;
        bool isInSameLAN ( c->isInPrivateAddrSpace() ) ;
        if ( isInSameLAN ) {
            numberOfNodesInPrivateAddressSpace++ ;
        }
        const Node* nodeOfConnection = c->node() ;
        if ( isInSameLAN ||
                ( nodeOfConnection &&
                  nodesOfNetworkRequests.contains( nodeOfConnection->nodeFingerPrint() )) ) {
            // do nothing here, node is in our local lan or it
            // had pending network request so lets not kick him out..
        } else {
            if ( oldestActivityTimeFound > c->iTimeOfLastActivity ) {

                oldestActivityTimeFound = c->iTimeOfLastActivity ;
                indexFound = j ;
            }
        }
    }
    // we have a constant in protocol.h telling how many connections
    // we wish to maintain ; from that count out the nodes that are
    // in our local lan i.e. never disconnect them unless they
    // disconnect self..
    int numberOfRealOutSideConnections =
        iModel.getConnections().size() - numberOfNodesInPrivateAddressSpace    ;
    if ( numberOfRealOutSideConnections > KMaxOpenConnections ) {
        if ( indexFound != -1 ) {
            iModel.getConnections().value(indexFound)->iNeedsToRun = false ;
        }
    }
}

QString NodeModel::dataDir() {
    return iDataDir ;
}

void NodeModel::addNodeFromBroadcast(const Hash& aNodeFingerPrint,
                                     const QHostAddress& aAddr,
                                     int aPort ) {
    // first check if node is already connected ; our neighboring
    // node will broadcast its presence often so odds are that we've
    // already made a connection:

    const QList <Connection *>& currentConnections ( iModel.getConnections() ) ;
    for ( int i = currentConnections.size()-1 ; i >= 0 ; i-- ) {
        // and don't connect to already-connected nodes
        if ( aNodeFingerPrint == currentConnections.value(i)->getPeerHash() ) {
            QLOG_STR("Broadcast: According to ssl fp, this node is already connected") ;
            return ; // was already connected
        }
    }
    // if we got here, the aNodeFingerPrint is not self,
    // and we don't have connection already.
    Node* n = new Node(aNodeFingerPrint,aPort) ;
    if ( aAddr.protocol() == QAbstractSocket::IPv6Protocol ) {
        n->setIpv6Addr(aAddr.toIPv6Address () ) ;
    } else {
        n->setIpv4Addr(aAddr.toIPv4Address () ) ;
    }
    addNodeToConnectionWishList(n) ;
}


bool NodeModel::addNodeToConnectionWishList(Node* aNode) {
    if ( iController->getNode().nodeFingerPrint() == aNode->nodeFingerPrint() ) {
        delete aNode ;
        return false ; // don't queue connection to self
    }
    for ( int i = 0 ; i < iConnectionWishList.size() ; i++ ) {
        if ( aNode->nodeFingerPrint() == iConnectionWishList.value(i)->nodeFingerPrint()) {
            // node already found from wishlist, don't add again:
            delete aNode ;
            return false ;
        }
    }
    iConnectionWishList.append(aNode) ;
    return true ;
}

bool NodeModel::addNodeToConnectionWishList(const Hash& aNode) {
    if ( iController->getNode().nodeFingerPrint() == aNode ) {
        return false ; // don't queue connection to self
    }
    for ( int i = 0 ; i < iConnectionWishList.size() ; i++ ) {
        if ( aNode == iConnectionWishList.value(i)->nodeFingerPrint()) {
            return true ;
        }
    }
    Node *n = nodeByHash(aNode) ;
    if ( n ) {
        iConnectionWishList.append(n) ;
        return true ;
    } else {
        return false ;
    }
}



Node* NodeModel::nextConnectionWishListItem() {
    if ( iConnectionWishList.isEmpty() ) {
        return NULL ;
    } else {
        return iConnectionWishList.takeAt(0) ;
    }
}

bool NodeModel::isNodeAlreadyConnected(const Node& aNode) const {
    if ( *iFingerPrintOfThisNode == aNode.nodeFingerPrint() ) {
        // "self" is considered connected
        return true ;
    }

    const QList <Connection *>& currentConnections ( iModel.getConnections() ) ;
    for ( int i = currentConnections.size()-1 ; i >= 0 ; i-- ) {
        // and don't connect to already-connected nodes
        const Node* nodeOfConnection ( currentConnections.value(i)->node() );
        if ( nodeOfConnection &&
                currentConnections.value(i)->connectionState() == Connection::Open ) {
            if ( nodeOfConnection->nodeFingerPrint() == aNode.nodeFingerPrint() ) {
                return true ; // was already connected
            }
        }
        // there may also be connections in early stage where connection
        // is there but node greeting has not yet been processed:
        QHostAddress addrOfConnection (currentConnections.value(i)->peerAddress());
        if ( aNode.ipv4Addr() &&
                addrOfConnection.protocol()== QAbstractSocket::IPv4Protocol &&
                addrOfConnection.toIPv4Address() == aNode.ipv4Addr() ) {
            return true ; // was already connected
        }
        if ( !Connection::Ipv6AddressesEqual(aNode.ipv6Addr(),
                                             KNullIpv6Addr ) &&
                addrOfConnection.protocol()== QAbstractSocket::IPv6Protocol &&
                Connection::Ipv6AddressesEqual(addrOfConnection.toIPv6Address(), aNode.ipv6Addr() ) ) {
            return true ;
        }
    }
    return false ;
}

bool NodeModel::isNodeAlreadyConnected(const Hash& aHash) const {
    if ( *iFingerPrintOfThisNode == aHash ) {
        // "self" is considered connected
        return true ;
    }

    const QList <Connection *>& currentConnections ( iModel.getConnections() ) ;
    for ( int i = currentConnections.size()-1 ; i >= 0 ; i-- ) {
        // and don't connect to already-connected nodes
        const Node* nodeOfConnection( currentConnections.value(i)->node() ) ;
        if ( nodeOfConnection  &&
                currentConnections.value(i)->connectionState() == Connection::Open) {
            if ( nodeOfConnection->nodeFingerPrint() == aHash ) {
                return true ; // was already connected
            }
        }
    }
    return false ;
}

Hash NodeModel::bucketEndHash(const Hash& aFingerPrintOfNodeAsking) {
    Hash retval ;
    // here it is good idea to use same method that publishing-process
    // uses for getting the nodes .. we get a bit extra but
    // for sake of consistency it might still be good idea.
    QList<Node *>* twentyNodes ( getNodesAfterHash(aFingerPrintOfNodeAsking,
                                 20, // ask for 20
                                 5*60)) ;  // that have connect time 5*60 minutes or less

    if ( twentyNodes && twentyNodes->size() < 19 ) {
        retval =  aFingerPrintOfNodeAsking ; // we got less than 19 nodes so our network is really small
    } else if ( twentyNodes && twentyNodes->size() >= 19 ) {
        retval = twentyNodes->at(twentyNodes->size()-1)->nodeFingerPrint() ;
    } else {
        retval =  aFingerPrintOfNodeAsking ; // something odd happened. send all data.
    }

    // we have ownership of the array, it needs to be purged.

    while ( twentyNodes && twentyNodes->size() > 0 ) {
        Node* n = twentyNodes->takeAt(0) ;
        delete n ;
    }
    delete twentyNodes ;
    return retval ;
}

bool NodeModel::saveSslCertToDb(const QByteArray& aCert) {
    bool ret ( false ) ;
    QSqlQuery q2;
    ret = q2.prepare("update settings set node_cert = :cert") ;
    if (ret) {
        q2.bindValue(":cert", aCert) ;
        ret = q2.exec() ;
    }
    if ( !ret ) {
        QLOG_STR(q2.lastError().text() + " "+ __FILE__ + QString::number(__LINE__)) ;
        emit error(MController::DbTransactionError, q2.lastError().text()) ;
    }
    return ret ;
}

bool NodeModel::saveSslKeyToDb(const QByteArray& aKey) {
    bool ret ( false ) ;
    QSqlQuery q2;
    ret = q2.prepare("update settings set node_key = :key") ;
    if (ret) {
        q2.bindValue(":key", aKey) ;
        ret = q2.exec() ;
    }
    if ( !ret ) {
        QLOG_STR(q2.lastError().text() + " "+ __FILE__ + QString::number(__LINE__)) ;
        emit error(MController::DbTransactionError, q2.lastError().text()) ;
    }
    return ret ;
}

bool NodeModel::loadSslCertFromDb() {
    QSqlQuery query ;
    bool ret ( false ) ;
    QByteArray certBytes ;
    ret = query.exec("select node_cert from settings") ;
    if ( ret &&
            query.next() ) {
        if ( query.isNull(0) ) {
            // normal situation in first time: there is no key, column is null
            return false ;
        } else {
            certBytes = query.value(0).toByteArray() ;
        }
    }
    if ( !ret ) {
        emit error(MController::DbTransactionError, query.lastError().text()) ;
    }
    if ( certBytes.size() > 0 ) {
        iThisNodeCert = new QSslCertificate(certBytes) ;
    }
    if ( iThisNodeCert == NULL ) {
        QString errmsg(tr("Cant load SSL cert")) ;
        emit error(MController::OwnCertNotFound, errmsg) ;
        return false ;
    } else {
        iFingerPrintOfThisNode = new Hash(*iThisNodeCert) ;
        return true ;
    }
}
bool NodeModel::loadSslKeyFromDb() {

    QSqlQuery query ;
    bool ret ( false ) ;
    QByteArray keyBytes ;
    ret = query.exec("select node_key from settings") ;
    if ( ret &&
            query.next() ) {
        if ( query.isNull(0) ) {
            // normal situation in first time: there is no key, column is null
            return false ;
        } else {
            keyBytes = query.value(0).toByteArray() ;
        }
    }
    if ( !ret ) {
        emit error(MController::DbTransactionError, query.lastError().text()) ;
    }
    if ( keyBytes.size() > 0 ) {
        iThisNodeKey = new QSslKey(keyBytes,QSsl::Rsa) ;
    }
    if ( iThisNodeCert == NULL ) {
        QString errmsg(tr("Cant load SSL key")) ;
        emit error(MController::OwnCertNotFound, errmsg) ;
        return false ;
    } else {
        return true ;
    }
}


QString NodeModel::getDnsName() {
    QSqlQuery query;
    QString retval ;
    bool ret (false) ;
    ret = query.prepare ("select dns_name from settings");
    if ( ret && (ret = query.exec()) && query.next ()) {
        if ( !query.isNull(0) ) {
            retval = query.value(0).toString() ;
        }
    }
    if ( !ret ) {
        QLOG_STR(query.lastError().text() + " "+ __FILE__ + QString::number(__LINE__)) ;
        emit error(MController::DbTransactionError, query.lastError().text()) ;
    }
    return retval ;
}

void NodeModel::setDnsName(QString aName) {
    QSqlQuery query;
    bool retval ;
    retval = query.prepare ("update settings set dns_name = :name");
    if ( retval ) {
        if ( aName.length() > 0 ) {
            query.bindValue(":name", aName) ;
        } else {
            query.bindValue(":name", QVariant(QVariant::String)) ; // null value
        }
        retval = query.exec() ;
    }
    if ( !retval ) {
        QLOG_STR(query.lastError().text() + " "+ __FILE__ + QString::number(__LINE__)) ;
        emit error(MController::DbTransactionError, query.lastError().text()) ;
    }
}

void NodeModel::offerNodeToRecentlyFailedList(const Hash& aFailedNodeHash) {
    bool seen = false ;
    for ( int i = iRecentlyFailedNodes.size()-1 ; i >= 0 ; i-- ) {
        if ( iRecentlyFailedNodes[i].first == aFailedNodeHash ) {
            seen = true ;
            break ;
        }
    }
    if ( !seen ) {
        QPair<Hash,unsigned> newItem ( aFailedNodeHash, QDateTime::currentDateTimeUtc().toTime_t() ) ;
        iRecentlyFailedNodes.append(newItem) ;
        QLOG_STR("Failed node " + aFailedNodeHash.toString() +
                 " time " + QString::number(newItem.second)) ;
        iHotAddresses.clear() ; // causes re-fetch, without the failed node

    }
}


void NodeModel::removeNodeFromRecentlyFailedList(const Hash& aConnectedHostFingerPrint) {
    for ( int i = iRecentlyFailedNodes.size()-1 ; i >= 0 ; i-- ) {
        if ( iRecentlyFailedNodes[i].first == aConnectedHostFingerPrint ) {
            iRecentlyFailedNodes.removeAt(i) ;
            break ;
        }
    }
}
//
// this method is hit every 2 minutes - housekeeping here
//
void NodeModel::timerEvent(QTimerEvent*  /* event */ ) {
    const unsigned time10MinAgo = QDateTime::currentDateTimeUtc().toTime_t()-(60*10) ;
    iController->model().lock() ;
    // remove all nodes that have been more than 10 minutes on the list
    for ( int i = iRecentlyFailedNodes.size()-1 ; i >= 0 ; i-- ) {
        if ( iRecentlyFailedNodes[i].second < time10MinAgo ) {
            QLOG_STR("Removing failed node " + iRecentlyFailedNodes[i].first.toString() +
                     " time " +  QString::number(iRecentlyFailedNodes[i].second) +
                     " because " +  QString::number(time10MinAgo) ) ;
            iRecentlyFailedNodes.removeAt(i) ;
        }
    }
    iController->model().unlock() ;
}

bool MNodeModelProtocolInterface::HostConnectQueueItem::operator==(const MNodeModelProtocolInterface::HostConnectQueueItem& aItemToCompare) const {
    if ( iAddress == aItemToCompare.iAddress &&
            iPort == aItemToCompare.iPort ) {
        return true ; // match
    } else {
        if ( iNodeHash != KNullHash &&
                iNodeHash == aItemToCompare.iNodeHash ) {
            return true ;
        }
    }
    return false ;
}
