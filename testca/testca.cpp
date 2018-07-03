/*     -*-C++-*- -*-coding: utf-8-unix;-*-
  Classified Ads is Copyright (c) Antti Järvinen 2013-2018.

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

#include <QtTest/QtTest>
#include "../util/hash.h"
#include <gcrypt.h>
#include "mockup_controller.h"
#include "mockup_model.h"
#include "mockup_nodemodel.h"
#include "mockup_voicecallengine.h"
#include "../datamodel/model.h"
#include "../datamodel/contentencryptionmodel.h"
#include "../log.h"
#include "../net/protocol_message_formatter.h"
#include "../net/protocol_message_parser.h"
#include <QHostAddress> // for Q_IPV6ADDR
#include <QThread>
#include <QSqlQuery>
#include <QSqlError>
#include "../datamodel/camodel.h"
#include "../datamodel/ca.h"
#include "../datamodel/profilemodel.h"
#include "../datamodel/profile.h"
#include "../datamodel/tclprogram.h"
#include "../datamodel/tclmodel.h"
#include "../datamodel/trusttreemodel.h"
#include "../util/jsonwrapper.h"
#include "../datamodel/cadbrecord.h"
#include "../datamodel/cadbrecordmodel.h"
#include "../tcl/tclWrapper.h"
#include <unistd.h>

Q_IPV6ADDR KNullIpv6Addr ( QHostAddress("::0").toIPv6Address () ) ;
Hash KNullHash ;
MController* controllerInstanceEx ; /**< Application controller,
                                       here as static for signal handlers */
/**
 * This class includes test cases for those functionalities
 * that can be automatically tested
 */
class TestClassifiedAds: public QObject, public Connection::ConnectionObserver {
    Q_OBJECT
    // from ConnectionObserver:
    /** method for sending data received */
    virtual bool dataReceived(const QByteArray& aData,
                              Connection& aConnection )  ;
    /** method for communicating fact that connection ends */
    virtual void connectionClosed(Connection *aDeletee)  ;
    /** method for communicating fact that connection is open for
    *  business
    */
    virtual void connectionReady(Connection *aBusinessEntity)  ;
private slots:
    void tryHash1();
    void tryHash2();
    void tryHash3();
    void trySHA1();
    void tryHashAddition() ;
    void tryHashSubstraction() ;
    void tryCreatingController() ;
    void tryContentEncryptionModel() ;
    void tryContentEncryptionModelPwdChange() ;
    void tryContentEncryptionModelPwdChangeFailure() ;
    void trySign() ;
    void tryVerify() ;
    void tryVerifyFailure() ;
    void tryNodeGreetingParse() ;
    void tryCompression() ;
    void tryEncrypt() ;
    void tryDecrypt() ;
    void tryDecryptFailure() ;
    void tryHashQVariant() ;
    void tryHashQVariantComparison() ;
    void try3NodeGreetingParsing() ;
    void tryListOfAdsParsing() ;
    void trySearchRequest() ;
    void tryTrustTreeModel() ;
    void tryJSonParse() ;
    void tryJSonParseWithCompress() ;
    void tryJSonParseFailure() ;
    void tryJSonSerialize() ;
    /**
     * Voice call rt-data format+parse test 
     */
    void tryCallDataParse() ; 
    /**
     * Voice call status format+parse test 
     */
    void tryCallStatusDataParse() ; 
    /**
     * db record publish via TCL test
     */
    void tryDbRecordPublishViaTCL() ; 
    /**
     * db record publish via TCL test, encrypted variation
     */
    void tryEncryptedDbRecordPublish() ; 
    /**
     * db record network send/parse operation test
     */
    void tryDbRecordParse()  ;
    /**
     * test for local storage of tcl programs
     */
    void tryTCLLocalStorage() ; 
    void tryDbRecordSearchCompare();
    void tryDbRecordSearchParse();
    void tryDeletingController() ;
private:
    Hash iHashOfPrivateKey ;
    MockUpController* iController ;
    QByteArray iResultSignature ;
    QByteArray iPlaintextSigned ;
    QByteArray iCipherText ;
};

void TestClassifiedAds::tryHash1() {
    // highest significant bit first, so
    Hash h1(0,0,0,0,0) ; // this is zero-hash
    Hash h2(0,1,0,0,0) ; // this is big
    Hash h3(0,0,1,0,0) ; // this is less big
    Hash h4(0,0,0,1,0) ; // this is even smaller
    Hash h5(0,0,0,0,1) ; // this has the least-significant bit set
    QVERIFY(h2 > h1 &&
            h3 < h2 &&
            h4 < h3
           );
}

void TestClassifiedAds::tryHash2() {
    Hash h1(0,0,0,0,0) ;
    Hash h2(0,1,0,0,0) ;
    QVERIFY(h1 < h2);
}

void TestClassifiedAds::tryHash3() {
    Hash moroccan(0,0,0,0,0) ;
    Hash lebanese(1,1,1,1,1) ;
    moroccan = lebanese ;
    QVERIFY(moroccan == lebanese);
}

void TestClassifiedAds::tryHashQVariant() {
    Hash h1(0,30,40,50,0xFFFFFFFF) ;
    QVariant q ( h1.toQVariant() ) ;
    Hash h2 ;
    h2.fromQVariant ( q ) ;
    QVERIFY(h1 == h2 ) ;
}

void TestClassifiedAds::tryHashQVariantComparison() {
    Hash h1(0,31,40,50,0xFFFFFFFF) ;
    Hash h2(0,31,40,50,0xFFFFFFFF) ;
    QVariant q1 ( h1.toQVariant() ) ;
    QVariant q2 ( h2.toQVariant() ) ;
    QVERIFY(q1 == q2 ) ;
}


void TestClassifiedAds::trySHA1() {
    QByteArray plainText ( "quick brown fox and some diipadaapa\n") ;
    Hash h ;
    h.calculate(plainText) ;
    QLOG_STR("Calculated: " + h.toString()) ;
    // so, this next string is calculated using "openssl sha1"
    // command so we trust it might be right .. or usable to our
    // situation.
    QLOG_STR("Given:       2af8d4e46fc6fa45e84bb6f007d75e90f259630c") ;
    QVERIFY(h.toString() == QString("2af8d4e46fc6fa45e84bb6f007d75e90f259630c").toUpper());
}
// idea here is that we try the same operation
// using libgcrypt (that we trust) and our own
// hash-class and we are supposed to yield same
// results here.
void TestClassifiedAds::tryHashAddition() {
    const char* val = "Some-Sample-Input" ;
    const char* val2 = "jurgeli burgeli ja toinen strink" ;
    int msg_length = strlen( val );
    int msg_length2 = strlen( val2 );
    int hash_length = gcry_md_get_algo_dlen( GCRY_MD_SHA1 );
    unsigned char hash[ hash_length ];
    unsigned char hash2[ hash_length ];
    char *out = (char *) malloc( sizeof(char) * ((hash_length*2)+1) );
    char *p = out;
    gcry_md_hash_buffer( GCRY_MD_SHA1, hash, val, msg_length );
    gcry_md_hash_buffer( GCRY_MD_SHA1, hash2, val2, msg_length2 );
    gcry_mpi_t hash_mpi;
    gcry_mpi_t hash_mpi2;
    gcry_mpi_t summa  = gcry_mpi_new (160) ;
    gcry_mpi_scan( &hash_mpi, GCRYMPI_FMT_USG, hash, hash_length,
                   NULL );
    gcry_mpi_scan( &hash_mpi2, GCRYMPI_FMT_USG, hash2, hash_length,
                   NULL );
    gcry_mpi_add (summa, hash_mpi, hash_mpi2) ;
    unsigned char tuloste[100] = { 0 } ;
    size_t printed ;
    gcry_mpi_print (GCRYMPI_FMT_HEX, tuloste, 99, &printed, hash_mpi) ;
    printf("first hash (%lu): %s\n",printed, tuloste) ;
    gcry_mpi_print (GCRYMPI_FMT_HEX, tuloste, 99, &printed, hash_mpi2) ;
    printf("second hash (%lu): %s\n",printed, tuloste) ;
    gcry_mpi_print (GCRYMPI_FMT_HEX, tuloste, 99, &printed, summa) ;
    printf("sum (%lu): %s\n",printed, tuloste) ;

    // then same using Hash class
    Hash val1_hash ( hash ) ;
    Hash val2_hash ( hash2 ) ;
    printf("\n") ;
    qDebug() << val1_hash.toString() ;
    qDebug() << val2_hash.toString() ;
    Hash summa_hash = val1_hash + val2_hash;
    qDebug() << summa_hash.toString() ;
    printf("\n") ;
    gcry_mpi_release( hash_mpi );
    gcry_mpi_release( hash_mpi2 );
    gcry_mpi_release( summa );
    int i;
    for ( i = 0; i < hash_length; i++, p += 2 ) {
        snprintf ( p, 3, "%02x", hash[i] );
    }
    QVERIFY(strcmp((const char *)summa_hash.toString().toUtf8().constData(),
                   (const char *)tuloste) == 0);
}

void TestClassifiedAds::tryHashSubstraction() {
    const char* val = "Some-Sample-Input" ;
    const char* val2 = "jurgeli burgeli ja toinen strink" ;
    int msg_length = strlen( val );
    int msg_length2 = strlen( val2 );
    int hash_length = gcry_md_get_algo_dlen( GCRY_MD_SHA1 );
    unsigned char hash[ hash_length ];
    unsigned char hash2[ hash_length ];
    char *out = (char *) malloc( sizeof(char) * ((hash_length*2)+1) );
    char *p = out;
    gcry_md_hash_buffer( GCRY_MD_SHA1, hash, val, msg_length );
    gcry_md_hash_buffer( GCRY_MD_SHA1, hash2, val2, msg_length2 );
    gcry_mpi_t hash_mpi;
    gcry_mpi_t hash_mpi2;
    gcry_mpi_t summa  = gcry_mpi_new (160) ;
    gcry_mpi_scan( &hash_mpi, GCRYMPI_FMT_USG, hash, hash_length,
                   NULL );
    gcry_mpi_scan( &hash_mpi2, GCRYMPI_FMT_USG, hash2, hash_length,
                   NULL );
    gcry_mpi_sub (summa, hash_mpi, hash_mpi2) ;
    unsigned char tuloste[100] = { 0 } ;
    size_t printed ;
    gcry_mpi_print (GCRYMPI_FMT_HEX, tuloste, 99, &printed, hash_mpi) ;
    printf("first hash (%lu): %s\n",printed, tuloste) ;
    gcry_mpi_print (GCRYMPI_FMT_HEX, tuloste, 99, &printed, hash_mpi2) ;
    printf("second hash (%lu): %s\n",printed, tuloste) ;
    gcry_mpi_print (GCRYMPI_FMT_HEX, tuloste, 99, &printed, summa) ;
    printf("subs (%lu): %s\n",printed, tuloste) ;

    // then same using Hash class
    Hash val1_hash ( hash ) ;
    Hash val2_hash ( hash2 ) ;
    printf("\n") ;
    qDebug() << val1_hash.toString() ;
    qDebug() << val2_hash.toString() ;
    Hash subs_hash = val1_hash - val2_hash;
    qDebug() << subs_hash.toString() ;
    printf("\n") ;
    gcry_mpi_release( hash_mpi );
    gcry_mpi_release( hash_mpi2 );
    gcry_mpi_release( summa );
    int i;
    for ( i = 0; i < hash_length; i++, p += 2 ) {
        snprintf ( p, 3, "%02x", hash[i] );
    }
    QVERIFY(strcmp((const char *)subs_hash.toString().toUtf8().constData(),
                   (const char *)tuloste) == 0);
}

void TestClassifiedAds::tryContentEncryptionModel() {
    Hash nullHash ( 0,0,0,0,0) ;
    QList<Hash> privateKeys = iController->model().contentEncryptionModel().listKeys(true,NULL)  ;
    if ( privateKeys.size() == 0 ) {
        iHashOfPrivateKey = iController->model().contentEncryptionModel().generateKeyPair() ;
        QVERIFY(!(iHashOfPrivateKey == nullHash) ) ;
        if ( iHashOfPrivateKey != nullHash  ) {
            // put some profile data in there too
            Profile p(iHashOfPrivateKey) ;
            p.iNickName = "test profile" ;
            p.iGreetingText = "zap" ;
            p.iIsPrivate = false ;
            p.iTimeOfPublish = QDateTime::currentDateTimeUtc().toTime_t() ;
            iController->model().profileModel().publishProfile(p) ;
            QSqlQuery query(iController->model().dataBaseConnection());
            query.prepare("delete from publish where hash1 = " + QString::number(iHashOfPrivateKey.iHash160bits[0])) ;
            query.exec() ;

        }
    } else {
        for ( int i = privateKeys.size()-1 ; i >= 0 ; i-- ) {
            QLOG_STR("\nHash found " + privateKeys[i].toString()) ;
            iHashOfPrivateKey = privateKeys[0] ;
        }
        QVERIFY(privateKeys.size() > 0 ) ;
    }
}

void TestClassifiedAds::tryContentEncryptionModelPwdChange() {
    if ( iController->model().contentEncryptionModel().changeKeyPassword(iHashOfPrivateKey, QString("el password nuevo")) == 0 ) {
        QVERIFY(   iController->model().contentEncryptionModel().changeKeyPassword(iHashOfPrivateKey,QString("das password nuevo")) == 0 );
    } else {
        QVERIFY ( 1 == 2 ) ;
    }
    // last
}

void TestClassifiedAds::tryContentEncryptionModelPwdChangeFailure() {
    iController->setContentKeyPasswd("salasana") ;
    // changeKeyPassword must return non-zero when failing.
    QVERIFY(   iController->model().contentEncryptionModel().changeKeyPassword(iHashOfPrivateKey,QString("das password nuevo")) != 0 );
    iController->setContentKeyPasswd("das password nuevo") ;
    iController->model().contentEncryptionModel().changeKeyPassword(iHashOfPrivateKey,QString("salasana"));
    iController->setContentKeyPasswd("salasana") ;
    // last
}

void TestClassifiedAds::trySign() {
    QByteArray plaintext("foobar and brown fox did some tricks involving jumping") ;
    iPlaintextSigned = plaintext ;
    QVERIFY( iController->model().contentEncryptionModel().sign(iHashOfPrivateKey,
             iPlaintextSigned,
             iResultSignature) == 0 );
    LOG_STR2("resultSignature len: %d\n" , iResultSignature.length()) ;
}

void TestClassifiedAds::tryVerify() {
    QVERIFY( iController->model().contentEncryptionModel().verify(iHashOfPrivateKey,
             iPlaintextSigned,
             iResultSignature) == true );
}
void TestClassifiedAds::tryVerifyFailure() {
    QByteArray plaintext("something completely different") ;
    iPlaintextSigned = plaintext ;
    QVERIFY( iController->model().contentEncryptionModel().verify(iHashOfPrivateKey,
             iPlaintextSigned,
             iResultSignature) == false );
}

#define EXAMPLE_TEXT "Quick brown and rest of the diipadaapa. This goes via.."
void TestClassifiedAds::tryEncrypt() {
    QList<Hash> listOfRecipients ;
    QByteArray plainText ( QString(EXAMPLE_TEXT).toUtf8()) ;
    QByteArray result ;
    listOfRecipients.append(iHashOfPrivateKey) ;
    QVERIFY( iController->model().contentEncryptionModel().encrypt(listOfRecipients,
             plainText,
             result) == true );
    iCipherText.append(result) ;
}

void TestClassifiedAds::tryDecrypt() {
    QByteArray result ;
    iController->setProfileInUse(iHashOfPrivateKey) ;
    QVERIFY( iController->model().contentEncryptionModel().decrypt(iCipherText,
             result) == true );
    LOG_STR2("After de-crypt %s\n", qPrintable(QString(result))) ;
    QCOMPARE( result, QString(EXAMPLE_TEXT).toUtf8()) ;
}

void TestClassifiedAds::tryDecryptFailure() {
    QByteArray result ;
    // cause a failure here:
    iCipherText[100] = 'A' ;
    // then check that we're not ok any more..
    QVERIFY( iController->model().contentEncryptionModel().decrypt(iCipherText,
             result) == false );
}



void TestClassifiedAds::tryNodeGreetingParse() {
    MockUpModel* m = new MockUpModel(iController) ;
    ProtocolMessageParser* p = new ProtocolMessageParser(*iController,*m) ;
    Node* n = new Node(Hash(1,2,3,4,5),12345) ;
    n->setIpv4Addr(345435324) ;
    QHostAddress ip6("3::aa:dd") ;
    LOG_STR2("ip6: %s\n", qPrintable(ip6.toString())) ;
    n->setIpv6Addr(ip6.toIPv6Address()) ;
    n->setLastConnectTime(2342343) ;
    n->setLastMutualConnectTime(43223112) ;
    QByteArray serializedNode = ProtocolMessageFormatter::nodeGreeting(*n) ;

    // need to have connection&thread for parsing.
    QThread t;
    Connection* c = new Connection(1,
                                   *this,
                                   iController->model(),
                                   *iController) ;
    QString jiison ( serializedNode.mid(5) ) ;
    int len = jiison.length() ;
    QLOG_STR("Jii before parse: " + jiison) ;
    LOG_STR2("len = %d\n", len) ;
    p->parseMessage(serializedNode,*c) ;

    // after successful parse our mock-up of datamodel should
    // contain the parsed node in its member variable -> verify
    // that data fields got parsed correctly
    QVERIFY(
        m->iNodeModel->iLastNodeReceived != NULL &&
        m->iNodeModel->iLastNodeReceived->nodeFingerPrint() == n->nodeFingerPrint()&&
        m->iNodeModel->iLastNodeReceived->port() == n->port()&&
        m->iNodeModel->iLastNodeReceived->ipv4Addr() == n->ipv4Addr()&&
        Connection::Ipv6AddressesEqual(m->iNodeModel->iLastNodeReceived->ipv6Addr(),
                                       n->ipv6Addr())==true) ;

    delete n ;
    delete p ;
    delete m ;
}


// like tryNodeGreetingParse but tries with list of 3 nodes
void   TestClassifiedAds::try3NodeGreetingParsing() {
    MockUpModel* m = new MockUpModel(iController) ;
    ProtocolMessageParser* p = new ProtocolMessageParser(*iController,*m) ;

    Node* n = new Node(Hash(1,2,3,4,5),12345) ;
    n->setIpv4Addr(345435324) ;
    QHostAddress ip6("3::aa:dd") ;
    LOG_STR2("ip6: %s\n", qPrintable(ip6.toString())) ;
    n->setIpv6Addr(ip6.toIPv6Address()) ;
    n->setLastConnectTime(2342343) ;
    n->setLastMutualConnectTime(43223112) ;
    QByteArray serializedNode = ProtocolMessageFormatter::nodeGreeting(*n) ;
    LOG_STR2("Len of first greeting bytes = %d\n", serializedNode.size()) ;

    Node* n2 = new Node(Hash(11,22,33,44,55),12345) ;
    n2->setIpv4Addr(345435326) ;
    QHostAddress ip6_2("3::aa:df") ;
    LOG_STR2("ip6_2: %s\n", qPrintable(ip6_2.toString())) ;
    n2->setIpv6Addr(ip6_2.toIPv6Address()) ;
    n2->setLastConnectTime(1342343) ;
    n2->setLastMutualConnectTime(33223112) ;
    QByteArray serializedNode2 = ProtocolMessageFormatter::nodeGreeting(*n2) ;
    serializedNode.append(serializedNode2) ;
    LOG_STR2("Len of 2nd greeting bytes = %d\n", serializedNode2.size()) ;
    Node* n3 = new Node(Hash(12,23,34,45,57),12345) ;
    n3->setIpv4Addr(345435226) ;
    QHostAddress ip6_3("3::aa:de") ;
    LOG_STR2("ip6_3: %s\n", qPrintable(ip6_3.toString())) ;
    n3->setIpv6Addr(ip6_3.toIPv6Address()) ;
    n3->setLastConnectTime(1342341) ;
    n3->setLastMutualConnectTime(33223122) ;
    QByteArray serializedNode3 = ProtocolMessageFormatter::nodeGreeting(*n3) ;
    serializedNode.append(serializedNode3) ;
    LOG_STR2("Len of 3rd greeting bytes = %d\n", serializedNode3.size()) ;

    // need to have connection&thread for parsing.
    Connection* c = new Connection(1,
                                   *this,
                                   iController->model(),
                                   *iController) ;
    QString jiison ( serializedNode.mid(5) ) ;
    int len = jiison.length() ;
    QLOG_STR("Jii before parse: " + jiison) ;
    LOG_STR2("len = %d\n", len) ;
    p->parseMessage(serializedNode,*c) ;

    // after successful parse our mock-up of datamodel should
    // contain the parsed node in its member variable -> verify
    // that data fields got parsed correctly
    QVERIFY(
        m->iNodeModel->iLastNodeReceived != NULL &&
        m->iNodeModel->iLastNodeReceived->nodeFingerPrint() == n3->nodeFingerPrint()&&
        m->iNodeModel->iLastNodeReceived->port() == n3->port()&&
        m->iNodeModel->iLastNodeReceived->ipv4Addr() == n3->ipv4Addr()&&
        Connection::Ipv6AddressesEqual(m->iNodeModel->iLastNodeReceived->ipv6Addr(),
                                       n3->ipv6Addr())==true) ;

    delete n ;
    delete n2 ;
    delete n3 ;
    delete c ;
    delete p ;
    delete m ;
}

void   TestClassifiedAds::tryListOfAdsParsing() {
    MockUpModel* m = new MockUpModel(iController) ;
    QList<NetworkRequestExecutor::NetworkRequestQueueItem>* list = new
    QList<NetworkRequestExecutor::NetworkRequestQueueItem> ;
    m->iNetworkRequests = list ;
    ProtocolMessageParser* p = new ProtocolMessageParser(*iController,*m) ;


    Connection* c = new Connection(1,
                                   *this,
                                   iController->model(),
                                   *iController) ;

    Hash classificationHash ( 0x5,0x6,0x7,0x8,0x9 ) ;
    QList<QPair<Hash,quint32> > listOfAds ;
    Hash articleHash1 (0x100,51,61,71,81 ) ;
    Hash articleHash2 (0x101,52,62,72,82 ) ;
    Hash articleHash3 (0x102,53,63,73,83 ) ;
    Hash articleHash4 (0x103,54,64,74,84 ) ;
    Hash articleHash5 (0x104,55,65,75,85 ) ;
    quint32 articleTimeStamp =  QDateTime::currentDateTimeUtc().toTime_t() ;
    QPair<Hash,quint32> p1 ( articleHash1, articleTimeStamp) ;
    QPair<Hash,quint32> p2 ( articleHash2, articleTimeStamp-1) ;
    QPair<Hash,quint32> p3 ( articleHash3, articleTimeStamp-2) ;
    QPair<Hash,quint32> p4 ( articleHash4, articleTimeStamp-3) ;
    QPair<Hash,quint32> p5 ( articleHash5, articleTimeStamp-4) ;
    listOfAds.append(p1) ;
    listOfAds.append(p2) ;
    listOfAds.append(p3) ;
    listOfAds.append(p4) ;
    listOfAds.append(p5) ;

    Node* n = new Node(Hash(1,2,3,4,5),12345) ;
    n->setIpv4Addr(345435324) ;
    QHostAddress ip6("3::aa:dd") ;
    n->setIpv6Addr(ip6.toIPv6Address()) ;
    n->setLastConnectTime(2342343) ;
    n->setLastMutualConnectTime(43223112) ;
    c->setNode(n) ; // ownership transferred

    QByteArray serializedAds = ProtocolMessageFormatter::replyToAdsClassified(classificationHash,
                               listOfAds) ;
    p->parseMessage(serializedAds,*c) ;
    QSqlQuery query(iController->model().dataBaseConnection());
    query.prepare("delete from classified_ad where group_hash1 = 5 and group_hash2 = 6 and group_hash3 = 7 and group_hash4 = 8 and group_hash5 = 9") ;
    query.exec() ;
    QCOMPARE( m->iNetworkRequests->size() , 5 ) ;
    QVERIFY( query.numRowsAffected () == 5  ) ;

    delete c;
    delete p ;
    delete m ;
}

// a bit more complex test case. it does publish of a ca,
// then a search against the ca.
void   TestClassifiedAds::trySearchRequest() {
    MockUpModel* m = new MockUpModel(iController) ;
    QList<NetworkRequestExecutor::NetworkRequestQueueItem>* list = new
    QList<NetworkRequestExecutor::NetworkRequestQueueItem> ;
    bool testCaseSuccess ( false ) ;
    m->iNetworkRequests = list ;
    ProtocolMessageParser* p = new ProtocolMessageParser(*iController,iController->model()) ;
    Connection* c = new Connection(1,
                                   *this,
                                   iController->model(),
                                   *iController) ;
    CA ca ;
    ca.iSenderName = "person, test person" ;
    ca.iSubject = "1234" ;
    ca.iGroup = "a.b.c" ;
    quint32 dummy ;

    ca.iMessageText = "<html><body>tottamooses,tassoisfraasikunneiesiinnymissaan,eihelepollaainakkaan</body></html>" ;
    QLOG_STR("Trying to get profile with key " + iHashOfPrivateKey.toString() + " " + QString::number(iHashOfPrivateKey.iHash160bits[0])) ;
    Profile *prof = iController->model().profileModel().profileByFingerPrint(iHashOfPrivateKey) ;
    if ( prof  ) {
        QLOG_STR("Got profile") ;
        if (
            iController->model().contentEncryptionModel().PublicKey(iHashOfPrivateKey,
                    ca.iProfileKey,
                    &dummy )) {
            QLOG_STR("Got public key ") ;
            ca.iFingerPrint = iController->model().classifiedAdsModel().publishClassifiedAd(*prof, ca) ;
            if(ca.iFingerPrint != KNullHash ) {
                // ok, the CA is in, lets try to perform a search on it.. first in "network search" manner:
                QByteArray serializedSearchString(
                    ProtocolMessageFormatter::searchSend("tottamooses,tassoisfraasikunneiesiinnymissaan,eihelepollaainakkaan",
                            true,
                            true,
                            true,
                            Hash(3,4,5,6,7)) ) ;
                p->parseMessage(serializedSearchString,*c) ;
                // at this point our connection "c" will have on protocol item to send
                // in its queue ( the results ) and there is test for that later


                // then test the search function of the model in manner how
                // UI will do the trick:
                iController->model().searchModel()->setSearchString("tottamooses,tassoisfraasikunneiesiinnymissaan,eihelepollaainakkaan",
                        true,
                        true,
                        true,
                        true) ;
                QString searchPhrase ;
                bool searchAds(false) ;
                bool searchProfiles(false) ;
                bool searchComments(false) ;
                Hash searchId ;
                iController->model().searchModel()->getSearchCriteria(&searchPhrase,
                        &searchAds,
                        &searchProfiles,
                        &searchComments,
                        &searchId) ;
                if ( searchAds&&searchProfiles&&searchComments ) {

                    QList<SearchModel::SearchResultItem> items (
                        iController->model().searchModel()->performSearch("tottamooses,tassoisfraasikunneiesiinnymissaan,eihelepollaainakkaan",
                                true,
                                true,
                                true)) ;
                    if(items.count() == 1 ) {
                        QByteArray resultsSerialized ( ProtocolMessageFormatter::searchResultsSend(items,searchId.iHash160bits[0]) ) ;
                        if(p->parseMessage(resultsSerialized,*c)) {
                            testCaseSuccess = true ;
			    QLOG_STR("p->parseMessage(resultsSerialized,*c) returned true") ; 
                        } else {
			  QLOG_STR("p->parseMessage(resultsSerialized,*c) returned false") ; 
			}
                    }
                }
            } else {
                QLOG_STR("CA got null fingerprint? ") ;
            }
        }
        delete prof ;
    } else {
        QLOG_STR("Profile was null??") ;
    }

    // interestingly numRowsAffected() with delete returns always 0 
    // if used under qt5.2.1 but under qt4 it returns the number
    // of rows deleted. in order to assess the number of rows under
    // qt5 we need a separate query:
    QSqlQuery query0(iController->model().dataBaseConnection());
    query0.prepare("select count(hash1) from classified_ad where hash1 = :h1 and hash2 = :h2 and hash3 = :h3 ") ;
    query0.bindValue(":hash1", ca.iFingerPrint.iHash160bits[0]);
    query0.bindValue(":hash2", ca.iFingerPrint.iHash160bits[1]);
    query0.bindValue(":hash3", ca.iFingerPrint.iHash160bits[2]);
    bool querySuccess = query0.exec() ;

    if ( querySuccess && query0.next() && !query0.isNull(0) ) {
      if ( testCaseSuccess == true && query0.value(0).toInt() == 1 )   {
	testCaseSuccess = true ; 
      }
    } else {
      testCaseSuccess = false ; 
      QLOG_STR("failure: " + query0.lastError().text() + " querySuccess = " + QString::number(querySuccess) + " isnull = " + QString::number(query0.isNull(0))) ; 
    }
    // after count is checked, the row may go:
    QSqlQuery query(iController->model().dataBaseConnection());
    query.prepare("delete from classified_ad where hash1 = :h1 and hash2 = :h2 and hash3 = :h3 ") ;
    query.bindValue(":hash1", ca.iFingerPrint.iHash160bits[0]);
    query.bindValue(":hash2", ca.iFingerPrint.iHash160bits[1]);
    query.bindValue(":hash3", ca.iFingerPrint.iHash160bits[2]);
    query.exec() ;

    QSqlQuery query2(iController->model().dataBaseConnection());
    query2.prepare("delete from publish where hash1 = :h1 and hash2 = :h2 and hash3 = :h3 ") ;
    query2.bindValue(":hash1", ca.iFingerPrint.iHash160bits[0]);
    query2.bindValue(":hash2", ca.iFingerPrint.iHash160bits[1]);
    query2.bindValue(":hash3", ca.iFingerPrint.iHash160bits[2]);
    query.exec() ;
    delete c;
    delete p ;
    delete m ;
    QVERIFY(testCaseSuccess) ;
}

void TestClassifiedAds::tryTrustTreeModel() {
    MockUpModel* m = new MockUpModel(iController) ;
    TrustTreeModel* tl = new TrustTreeModel(iController,*m) ;

    tl->initModel(QVariant()) ;
    Profile unTrustedProfile1 ( Hash(1,1,1,1,3) ) ;
    unTrustedProfile1.iNickName = "Einari E. Epaluotettava" ;
    QString displayName ;
    Hash trustingHash ;


    Profile *self = iController->model().profileModel().profileByFingerPrint(iHashOfPrivateKey) ;
    if ( self ) {
        Profile trustedProfile1 ( Hash(1,1,1,1,1) ) ;
        trustedProfile1.iNickName = "Taisto T. Trusted" ;
        if ( !self->iTrustList.contains(trustedProfile1.iFingerPrint) ) {
            self->iTrustList << trustedProfile1.iFingerPrint ;
            QLOG_STR("Setting trust on " + trustedProfile1.iNickName) ;
        }
        Profile trustedProfile2 ( Hash(1,1,1,1,2) ) ;
        trustedProfile2.iNickName = "Tenho T. Trusted" ;
        if ( !self->iTrustList.contains(trustedProfile2.iFingerPrint) ) {
            self->iTrustList << trustedProfile2.iFingerPrint ;
            QLOG_STR("Setting trust on " + trustedProfile2.iNickName) ;
        }

        iController->model().profileModel().publishProfile(*self) ;
        QSqlQuery query(iController->model().dataBaseConnection());
        query.prepare("delete from publish where hash1 = " + QString::number(iHashOfPrivateKey.iHash160bits[0]));
        query.exec() ;

        tl->offerTrustList(self->iFingerPrint, self->iNickName, self->iTrustList ) ;
        // then, cause calculation of the trust tree:
        tl->offerTrustList(trustedProfile1.iFingerPrint, trustedProfile1.iNickName, trustedProfile1.iTrustList ) ;
        tl->offerTrustList(trustedProfile2.iFingerPrint, trustedProfile2.iNickName, trustedProfile2.iTrustList ) ;
        // now, self is trusted and it has 2 trusted profiles.
        QCOMPARE(tl->isProfileTrusted(self->iFingerPrint,&displayName,&trustingHash), true) ;
        QCOMPARE(tl->isProfileTrusted(trustedProfile1.iFingerPrint,&displayName,&trustingHash), true) ;
        QCOMPARE(tl->isProfileTrusted(trustedProfile2.iFingerPrint,&displayName,&trustingHash), true) ;

        QVariant modelState ( tl->trustTreeSettings() ) ;
        tl->clear() ;
        tl->initModel(modelState) ;
        // and now, Taneli should again be trusted?
        QCOMPARE(tl->isProfileTrusted(trustedProfile2.iFingerPrint,&displayName,&trustingHash), true) ;

        // then add 2nd level trusted profile:
        Profile trustedProfile4 ( Hash(1,1,1,1,4) ) ;
        if ( !trustedProfile2.iTrustList.contains(trustedProfile4.iFingerPrint) ) {
            trustedProfile2.iTrustList << trustedProfile4.iFingerPrint ;
        }
        trustedProfile4.iNickName = "Sigvard S. Semitrusted" ;
        // update model as Tenho now expresses trust regarding Sigvard
        tl->offerTrustList(trustedProfile2.iFingerPrint, trustedProfile2.iNickName, trustedProfile2.iTrustList ) ;
        QCOMPARE(tl->isProfileTrusted(trustedProfile4.iFingerPrint,&displayName,&trustingHash), true) ;
        QLOG_STR("Sigvard is trusted by " + displayName) ;
        // then drop trust on Tenho and that should revoke trust
        // from Sigvard too:
        self->iTrustList.removeAll(trustedProfile2.iFingerPrint) ;
        iController->model().profileModel().publishProfile(*self) ;
        query.exec() ;
        tl->offerTrustList(self->iFingerPrint, self->iNickName, self->iTrustList ) ;
        QCOMPARE(tl->isProfileTrusted(trustedProfile4.iFingerPrint,&displayName,&trustingHash), false) ;

    }
    QVERIFY(tl->isProfileTrusted(unTrustedProfile1.iFingerPrint,&displayName,&trustingHash) == false) ;
    delete self ;
    delete tl ;
    delete m ;
}
/**
 * "ok" case of json text parsing
 */
void TestClassifiedAds::tryJSonParse() {
    QByteArray json("{ \"dataItem\": { \"description\": \"is descriptive\" }}");
    bool ok (false);
    QVariantMap result = JSonWrapper::parse(json, &ok) ;
    QCOMPARE(ok, true) ; 
    QVERIFY(result.contains("dataItem") == true) ; 
}
void TestClassifiedAds::tryJSonParseWithCompress() {
    QByteArray json("{ \"dataItem\": { \"description\": \"is descriptive\" }}");
    bool ok (false);
    QVariantMap result = JSonWrapper::parse(qCompress(json), &ok,true) ;
    QCOMPARE(ok, true) ; 
    QVERIFY(result.contains("dataItem") == true) ; 
}
/**
 * tests that if parser is given text that is not json,
 * it will correctly reject the result
 */
void TestClassifiedAds::tryJSonParseFailure() {
    QByteArray json("{\"is just text, no json text,trolloloo");
    bool ok (false);
    QVariantMap result = JSonWrapper::parse(json, &ok) ;
    QVERIFY(ok == false) ; 
}
/**
 * tests that qvariant does get converted into json text
 */
void TestClassifiedAds::tryJSonSerialize() {
    QMap<QString,QVariant> m ;
    m.insert("stringKey", "stringValue") ; 
    QByteArray serialized ( JSonWrapper::serialize(QVariant(m).toMap())) ; 
    QVERIFY(serialized.indexOf("stringValue") > 1) ; 
}

// call rt data parse
void TestClassifiedAds::tryCallDataParse() {

    MockUpModel* m = new MockUpModel(iController) ;
    ProtocolMessageParser* p = new ProtocolMessageParser(*iController,*m) ;


    Connection* c = new Connection(1,
                                   *this,
                                   iController->model(),
                                   *iController) ;

    QByteArray rtDataBytes;
    QByteArray callData ;
    callData.append("zap!") ; 
    rtDataBytes.append(ProtocolMessageFormatter::voiceCallRtData(51,
                                                                 8,
                                                                 MVoiceCallEngine::Audio,
                                                                 callData)) ;
    p->parseMessage(rtDataBytes,*c) ;

    QVERIFY( iController->voiceCallEngineMockUp()->iCallIdOfReceivedRtData == 51 &&
        iController->voiceCallEngineMockUp()->iCalldata == callData ) ;

    delete c;
    delete p ;
    delete m ;

}

// call rt data parse
void TestClassifiedAds::tryCallStatusDataParse() {

    MockUpModel* m = new MockUpModel(iController) ;
    ProtocolMessageParser* p = new ProtocolMessageParser(*iController,*m) ;


    Connection* c = new Connection(1,
                                   *this,
                                   iController->model(),
                                   *iController) ;

    VoiceCall callStatus ; 
    callStatus.iCallId = 81 ;
    callStatus.iOriginatingNode = Hash ( 51,52,53,54,55) ;
    callStatus.iDestinationNode = Hash ( 61,62,63,65,65) ; 
    iController->model().contentEncryptionModel().PrivateKey(iHashOfPrivateKey,
                                                             callStatus.iOriginatingOperatorKey  );
    callStatus.iOkToProceed = true ;
    callStatus.iTimeOfCallAttempt = 18 ; 
                                                                     
    QByteArray callData ;
    callData.append(ProtocolMessageFormatter::voiceCall(callStatus,
                                                        *iController,
                                                        iHashOfPrivateKey,
                                                        false)) ;
    p->parseMessage(callData,*c) ;

    QVERIFY( iController->voiceCallEngineMockUp()->iCallId == 81 ) ;

    delete c;
    delete p ;
    delete m ;

}

// this is a complex test case that runs a lot of functionality:
// this will try to publish a db record using TCL interface so
// it will test-drive both db publish methods and the whole TCL
// chain that is rather complex itself. to verify the publish operation
// it will then try db record search to assert that the published
// record is found in repository
void TestClassifiedAds::tryDbRecordPublishViaTCL() {
    MockUpModel* m = new MockUpModel(iController) ;
    QString tclProg = "dict set r collectionId [ calculateSHA1 foobar-collection ]\n"
        "dict set r searchPhrase {chicken is a bird}\n"
        "dict set r searchNumber 552\n"
        "dict set r data {crocodile is distant relative of chicken}\n"
        "dict set r encrypted 0\n" 
        "set newRecordId [ publishDbRecord $r ]\n" 
        "exit\n" ; 
    
    iController->tclWrapper().evalScript(tclProg) ; 
    QThread::yieldCurrentThread ();
    QLOG_STR("Enter sleep\n") ; 
    sleep(2) ;
    QLOG_STR("sleep end\n") ; 
    iController->tclWrapper().stopScript() ; 
    QCoreApplication::processEvents() ; 
    QLOG_STR("Enter sleep\n") ; 
    sleep(2) ;
    QLOG_STR("sleep end\n") ; 
    // if things went all right, we now should have a record in the database
    // so lets check it out:


    Hash collection ;
    collection.calculate("foobar-collection") ; 
    QString searchPhrase = "bird" ; 
    QList<CaDbRecord *> recordsFound ( 
        m->caDbRecordModel()->searchRecords(collection,
                                            KNullHash, // record id, but we don't know
                                            1, std::numeric_limits<quint32>::max()-1,
                                            -3, 1000,
                                            searchPhrase )) ;  
    bool isRecordFound ( false ) ; 
    foreach ( const CaDbRecord * recordFound,
              recordsFound ) {
        if ( recordFound->iSearchNumber == 552 ) {
            isRecordFound = true ; 
            QVERIFY(recordFound->iData == QByteArray("crocodile is distant relative of chicken") ) ; 
            QVERIFY(recordFound->iSearchPhrase == "chicken is a bird" ) ;
            QLOG_STR("recordFound->iSenderHash 0 " + 
                     QString::number(recordFound->iSenderHash.iHash160bits[0])) ; 
            QLOG_STR("profile in use 0 " + 
                     QString::number(iController->profileInUse().iHash160bits[0])) ; 
            QVERIFY(recordFound->iSenderHash == iController->profileInUse() ) ;                 
            QVERIFY(recordFound->iIsSignatureVerified == true ) ; 
        }
    }
    while ( recordsFound.size() > 0 ) {
        CaDbRecord* recordToDelete = recordsFound.takeAt(0) ; 
        delete recordToDelete ; 
    }
    QVERIFY ( isRecordFound == true ) ; 
    delete m ; 
}


void TestClassifiedAds::tryEncryptedDbRecordPublish() {
    MockUpModel* m = new MockUpModel(iController) ;
    QString tclProg = 
        QString("dict set r collectionId [ calculateSHA1 foobar-collection ]\n"
                "dict set r searchPhrase {bird is serious animal}\n"
                "dict set r searchNumber 553\n"
                "dict set r data {crocodile lives in Nile}\n"
                "dict set r encrypted 1\n" 
                "set recipientList [ list %1 ]\n"
                "dict set r recordRecipients $recipientList\n"
                "set newRecordId [ publishDbRecord $r ]\n" 
                "exit\n")
        .arg(iController->profileInUse().toString()) ; 
    
    iController->tclWrapper().evalScript(tclProg) ; 
    QThread::yieldCurrentThread ();
    QLOG_STR("Enter sleep\n") ; 
    sleep(2) ;
    QLOG_STR("sleep end\n") ; 
    iController->tclWrapper().stopScript() ; 
    QCoreApplication::processEvents() ; 
    QLOG_STR("Enter sleep\n") ; 
    sleep(2) ;
    QLOG_STR("sleep end\n") ; 
    // if things went all right, we now should have a record in the database
    // so lets check it out:


    Hash collection ;
    collection.calculate("foobar-collection") ; 
    QString searchPhrase = "bird" ; 
    QList<CaDbRecord *> recordsFound ( 
        m->caDbRecordModel()->searchRecords(collection,
                                            KNullHash, // record id, but we don't know
                                            1, std::numeric_limits<quint32>::max()-1,
                                            553, 553,
                                            searchPhrase )) ;  
    bool isRecordFound ( false ) ; 
    foreach ( const CaDbRecord * recordFound,
              recordsFound ) {
        if ( recordFound->iSearchNumber == 553 ) {
            isRecordFound = true ; 
            QVERIFY(recordFound->iData == QByteArray("crocodile lives in Nile") ) ; 
            QVERIFY(recordFound->iSearchPhrase == "bird is serious animal" ) ;
            QLOG_STR("recordFound->iSenderHash 0 " + 
                     QString::number(recordFound->iSenderHash.iHash160bits[0])) ; 
            QLOG_STR("profile in use 0 " + 
                     QString::number(iController->profileInUse().iHash160bits[0])) ; 
            QVERIFY(recordFound->iSenderHash == iController->profileInUse() ) ;                 
            QVERIFY(recordFound->iIsSignatureVerified == true ) ; 
        }
    }
    while ( recordsFound.size() > 0 ) {
        CaDbRecord* recordToDelete = recordsFound.takeAt(0) ; 
        delete recordToDelete ; 
    }
    QVERIFY ( isRecordFound == true ) ; 
    delete m ; 
}


// test method for checking compatibility of database record/parse
// methods in networking interface code and checking that the 
// transferred record is all right. 
void TestClassifiedAds::tryDbRecordParse() {
    // here utilize db record send instead of publish because there
    // is another (larger) test case for publish 
    MockUpModel* m = new MockUpModel(iController) ;
    ProtocolMessageParser* p = new ProtocolMessageParser(*iController,*m) ;


    Connection* c = new Connection(1,
                                   *this,
                                   iController->model(),
                                   *iController) ;

    const QByteArray unCompressedData ("el data grande") ; 
    CaDbRecord r ; 
    r.iRecordId = Hash(4,3,2,1,1) ; 
    r.iCollectionId.calculate(QByteArray("record collection, elvis anyone?")); 
    r.iSearchPhrase = "The king has entered the building" ; 
    r.iSearchNumber = 3 ; // carefully selected with random source
    r.iIsEncrypted = false ; 
    r.iTimeOfPublish = 25 ; // not at epoch but a testable time anyway
    r.iData = qCompress(unCompressedData) ; 
    r.iSenderHash = iController->profileInUse() ; 
    QByteArray digitalSignature; 
    QString additionalMetaDataStringToSign( // see similar code in verify method of cadbrecordmodel.cpp
        r.iRecordId.toString() + 
        r.iCollectionId.toString() + 
        r.iSenderHash.toString() ) ; 
    QByteArray additionalMetaDataBytesToSign( additionalMetaDataStringToSign.toUtf8()) ; 
    if ( iController->model()
         .contentEncryptionModel()
         .sign(iController->profileInUse(), 
               r.iData, 
               digitalSignature,
               &additionalMetaDataBytesToSign ) == 0 ) {
        r.iSignature = digitalSignature ; 
        QList<quint32> bangPath ; 
        bangPath.append(1) ; 
        
        QByteArray bytesOfRecord ( 
            ProtocolMessageFormatter::dbRecordSend(r) ) ;
        p->parseMessage(bytesOfRecord,*c) ;

        QList<CaDbRecord *> recordsFound ( m->caDbRecordModel()->searchRecords(KNullHash, 
                                                                               r.iRecordId)) ; 
        bool isRecordFound ( false ) ; 
        foreach ( const CaDbRecord * recordFound,
                  recordsFound ) {
            if ( recordFound->iRecordId == r.iRecordId ) {
                isRecordFound = true ; 
                QVERIFY(recordFound->iCollectionId == r.iCollectionId ) ; 
                QVERIFY(recordFound->iSearchPhrase == r.iSearchPhrase ) ; 
                QVERIFY(recordFound->iSearchNumber == r.iSearchNumber ) ;                 
                QVERIFY(recordFound->iTimeOfPublish == r.iTimeOfPublish ) ; 
                QVERIFY(recordFound->iData == unCompressedData ) ;                 
                QVERIFY(recordFound->iIsSignatureVerified == true ) ; 
            }
        }
        while ( recordsFound.size() > 0 ) {
            CaDbRecord* recordToDelete = recordsFound.takeAt(0) ; 
            delete recordToDelete ; 
        }
        QVERIFY ( isRecordFound == true ) ; 
    } else {
        QLOG_STR("Sign of record failed") ; 
        QVERIFY ( 1 == 2 ) ; 
    }
    delete c;
    delete p ;
    delete m ;
}

void TestClassifiedAds::tryTCLLocalStorage() {

    MockUpModel* m = new MockUpModel(iController) ;
    
    QString tclProg = 
        QString("dict set r key1 value1\n" // dictionary value 1
                "dict set r key2 value2\n" // dictionary value 2
                "storeLocalData $r\n"      // store dict into local storage
                "puts {storeLocalData done}\n"      // store dict into local storage
                "set r2 [ retrieveLocalData ]\n" // get it back
                "puts {retrieveLocalData done}\n"      // store dict into local storage
                "set numberOfKeys [ llength [ dict keys $r2 ] ]\n"  // count keys
                "storeLocalData $numberOfKeys\n" // save number of keys (==2) into storage
                "exit\n") ;                      // and get out
    TclProgram p ; 
    p.setProgramName("unit test prog") ; 
    p.setProgramText(tclProg) ; 
    m->tclModel().locallyStoreTclProgram(p) ; 
    iController->tclWrapper().evalScript(tclProg) ; 
    QThread::yieldCurrentThread ();
    QLOG_STR("Enter sleep\n") ; 
    sleep(2) ;
    QLOG_STR("sleep end\n") ; 
    iController->tclWrapper().stopScript() ; 
    QByteArray storageContent ( 
        m->tclModel().retrieveTCLProgLocalData(p.iFingerPrint)) ;
    QString storageAsString (storageContent) ;  
    QVERIFY( storageAsString == "2" ) ; 
}

void TestClassifiedAds::tryDbRecordSearchCompare() {
    CaDbRecord::SearchTerms specific ; 
    CaDbRecord::SearchTerms small ; 
    CaDbRecord::SearchTerms medium ; 
    CaDbRecord::SearchTerms large ; 
    specific.iById = Hash ( 8,8,7,7,1 ) ; 
    small.iModifiedAfter = 0 ;
    small.iModifiedBefore = 1 ; 
    small.iByHavingNumberMoreThan = 2 ; 
    small.iByHavingNumberLessThan = 3 ; 
    small.iBySearchPhrase = QString::null;
    medium = small ;
    medium.iBySearchPhrase = "king" ; 
    large = small ; 
    large.iBySearchPhrase = "the king has left the building" ;
    QVERIFY((specific < small) == false ) ; 
    QVERIFY(small < medium) ; 
    QVERIFY(medium < large ) ; 
    QVERIFY((specific == large) == false ) ; 
}

void TestClassifiedAds::tryDbRecordSearchParse() {
    MockUpModel* m = new MockUpModel(iController) ;
    ProtocolMessageParser* p = new ProtocolMessageParser(*iController,*m) ;

    Connection* c = new Connection(1,
                                   *this,
                                   iController->model(),
                                   *iController) ;

    CaDbRecord::SearchTerms t ; 
    t.iFromCollection = KNullHash ; 
    t.iById = Hash(4,3,2,1,1) ; // used in previous test case, is found
    t.iModifiedAfter = 0 ; 
    t.iModifiedBefore = std::numeric_limits<quint32>::max();
    t.iByHavingNumberMoreThan = std::numeric_limits<qint64>::min(); 
    t.iByHavingNumberLessThan = std::numeric_limits<qint64>::max();
    t.iBySearchPhrase = QString::null ; 
    t.iBySender = KNullHash ; 

    QByteArray bytesOfRecord ( 
        ProtocolMessageFormatter::dbSearchTerms(t) ) ;
    p->parseMessage(bytesOfRecord,*c) ;

    // now the connection should have record in send queue.
    bool recordFound = false ; 
    foreach ( const SendQueueItem& i, 
              c->iSendQueue ) {
        if ( i.iHash == t.iById &&
             i.iItemType == DbRecord ) {
            recordFound = true ; 
        }
    }
    delete c;
    delete p ;
    delete m ;
    QVERIFY(recordFound == true) ; 
}


void TestClassifiedAds::tryCompression() {
    QByteArray tdata = QString("Here is a string with unique content").toUtf8();
    QByteArray cdata (qCompress(tdata)) ;
    QByteArray tdata2 (qUncompress(cdata)) ;
    QVERIFY(tdata2 == tdata) ;
}

void TestClassifiedAds::tryDeletingController() {
    delete iController ;
    iController = NULL ;
    QVERIFY(iController == NULL ) ;
}

void TestClassifiedAds::tryCreatingController() {
    iController = new MockUpController() ;
    controllerInstanceEx = iController ; 
    iController->setContentKeyPasswd("salasana") ;
    QVERIFY(iController != NULL ) ;

}
bool TestClassifiedAds::dataReceived(const QByteArray& aData,
                                     Connection& aConnection ) {
    LOG_STR2("TestClassifiedAds::dataReceived len = %d\n", aData.size()) ;
    return true ;
}
void TestClassifiedAds::connectionClosed(Connection *aDeletee) {
    LOG_STR("TestClassifiedAds::connectionClosed\n") ;
}
void TestClassifiedAds::connectionReady(Connection *aBusinessEntity) {
    LOG_STR("TestClassifiedAds::connectionReady\n") ;
}



QTEST_MAIN(TestClassifiedAds)
#include "testca.moc"
