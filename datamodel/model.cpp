/*   -*-C++-*- -*-coding: utf-8-unix;-*-
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

#include "model.h"

#include <assert.h>
#include <stdio.h>

#include <QDir>
#include <QFile>
#include <QObject>
#include <QSqlError>
#include <QSqlQuery>
#include <QSslCertificate>
#include <QSslKey>
#include <QThread>
#include <QUuid>

#include "../log.h"
#include "../util/hash.h"
#if (QT_VERSION >= QT_VERSION_CHECK(5, 10, 0))
#include <QRandomGenerator>
#endif
#include <openssl/evp.h>   // OpenSSL_add_all_algorithms
#include <openssl/rand.h>  // random seed things
#include <openssl/ssl.h>   // library init

#include <QMutex>
#include <QTimerEvent>
#include <QWaitCondition>

#include "../net/connection.h"
#include "../net/networklistener.h"
#include "../net/node.h"
#include "../net/protocol_message_formatter.h"
#include "binaryfilemodel.h"
#include "cadbrecordmodel.h"
#include "camodel.h"
#include "contentencryptionmodel.h"
#include "netrequestexecutor.h"
#include "privmsgmodel.h"
#include "profilecommentmodel.h"
#include "profilemodel.h"
#include "searchmodel.h"
#include "trusttreemodel.h"
#ifdef WIN32
#include <Wincrypt.h>
#include <windows.h>
#endif
#include "tclmodel.h"

Model::Model(MController* aController)
    : iController(aController),
      iMutex(NULL),
      iConnections(NULL),
      iNodeModel(NULL),
      iNetReqExecutor(NULL),
      iNetReqQueue(NULL),
      iContentEncryptionModel(NULL),
      iProfileModel(NULL),
      iBinaryFileModel(NULL),
      iCaModel(NULL),
      iTimerId(-1),
      iPrivMsgModel(NULL),
      iProfileCommentModel(NULL),
      iSearchModel(NULL),
      iCaDbRecordModel(NULL),
      iTrustTreeModel(NULL),
      iTimeOfLastNetworkAddrCheck(QDateTime::currentDateTimeUtc().toTime_t()),
      iTclModel(NULL)
#if OPENSSL_VERSION_NUMBER >= 0x30000000L
      ,
      iLegacyCryptoProvider(NULL),
      iDefaultCryptoProvider(NULL)
#endif
{
  LOG_STR("Model::Model()");
  connect(this, SIGNAL(error(MController::CAErrorSituation, const QString&)),
          iController,
          SLOT(handleError(MController::CAErrorSituation, const QString&)),
          Qt::QueuedConnection);
  iMutex = new QMutex(QMutex::Recursive);  // same thread can lock recursive
                                           // mutex multiple times
  initPseudoRandom();
  if (!openDB()) {
    QString errorText(lastError().text());
    emit(error(MController::DataBaseNotMountable, errorText));
  }
  iNodeModel = new NodeModel(aController, *this);
  iConnections = new QList<Connection*>();
  iNetReqQueue = new QList<NetworkRequestExecutor::NetworkRequestQueueItem>();
  iNetReqExecutor = new NetworkRequestExecutor(aController, *this);
  iContentEncryptionModel = new ContentEncryptionModel(aController, *this);
  iProfileModel = new ProfileModel(aController, *this);
  iBinaryFileModel = new BinaryFileModel(aController, *this);
  iCaModel = new ClassifiedAdsModel(aController, *this);
  iPrivMsgModel = new PrivMessageModel(aController, *this);
  iSearchModel = new SearchModel(*this, *iController);
  iSearchModel->setObjectName("CA SearchModel");
  iCaDbRecordModel = new CaDbRecordModel(aController, *this);
  iTrustTreeModel = new TrustTreeModel(aController, *this);
  iProfileCommentModel = new ProfileCommentModel(aController, *this);
  iNetReqExecutor->setInterval(1000);  // start is called by controller
  iTclModel = new TclModel(aController, *this);

  connect(iBinaryFileModel,
          SIGNAL(contentReceived(const Hash&, const ProtocolItemType)), this,
          SLOT(notifyOfContentReceived(const Hash&, const ProtocolItemType)),
          Qt::QueuedConnection);
  connect(
      iPrivMsgModel,
      SIGNAL(contentReceived(const Hash&, const Hash&, const ProtocolItemType)),
      this,
      SLOT(notifyOfContentReceived(const Hash&, const Hash&,
                                   const ProtocolItemType)),
      Qt::QueuedConnection);

  connect(
      iProfileCommentModel,
      SIGNAL(contentReceived(const Hash&, const Hash&, const ProtocolItemType)),
      this,
      SLOT(notifyOfContentReceived(const Hash&, const Hash&,
                                   const ProtocolItemType)),
      Qt::QueuedConnection);

  connect(iProfileModel,
          SIGNAL(contentReceived(const Hash&, const ProtocolItemType)), this,
          SLOT(notifyOfContentReceived(const Hash&, const ProtocolItemType)),
          Qt::QueuedConnection);
  connect(
      iCaModel,
      SIGNAL(contentReceived(const Hash&, const Hash&, const ProtocolItemType)),
      this,
      SLOT(notifyOfContentReceived(const Hash&, const Hash&,
                                   const ProtocolItemType)),
      Qt::QueuedConnection);
  connect(
      iPrivMsgModel,
      SIGNAL(contentReceived(const Hash&, const Hash&, const ProtocolItemType)),
      this,
      SLOT(notifyOfContentReceived(const Hash&, const Hash&,
                                   const ProtocolItemType)),
      Qt::QueuedConnection);
  // connect same to controller too:
  connect(iBinaryFileModel,
          SIGNAL(contentReceived(const Hash&, const ProtocolItemType)),
          iController,
          SLOT(notifyOfContentReceived(const Hash&, const ProtocolItemType)),
          Qt::QueuedConnection);
  connect(
      iPrivMsgModel,
      SIGNAL(contentReceived(const Hash&, const Hash&, const ProtocolItemType)),
      iController,
      SLOT(notifyOfContentReceived(const Hash&, const Hash&,
                                   const ProtocolItemType)),
      Qt::QueuedConnection);
  connect(iProfileModel,
          SIGNAL(contentReceived(const Hash&, const ProtocolItemType)),
          iController,
          SLOT(notifyOfContentReceived(const Hash&, const ProtocolItemType)),
          Qt::QueuedConnection);
  connect(
      iCaModel,
      SIGNAL(contentReceived(const Hash&, const Hash&, const ProtocolItemType)),
      iController,
      SLOT(notifyOfContentReceived(const Hash&, const Hash&,
                                   const ProtocolItemType)),
      Qt::QueuedConnection);
  connect(
      iPrivMsgModel,
      SIGNAL(contentReceived(const Hash&, const Hash&, const ProtocolItemType)),
      iController,
      SLOT(notifyOfContentReceived(const Hash&, const Hash&,
                                   const ProtocolItemType)),
      Qt::QueuedConnection);

  connect(
      iProfileCommentModel,
      SIGNAL(contentReceived(const Hash&, const Hash&, const ProtocolItemType)),
      iController,
      SLOT(notifyOfContentReceived(const Hash&, const Hash&,
                                   const ProtocolItemType)),
      Qt::QueuedConnection);

  connect(
      iCaDbRecordModel,
      SIGNAL(contentReceived(const Hash&, const Hash&, const ProtocolItemType)),
      iController,
      SLOT(notifyOfContentReceived(const Hash&, const Hash&,
                                   const ProtocolItemType)),
      Qt::QueuedConnection);

  iTimerId = startTimer(60000 * 10);  // 10-minute timer
}

Model::~Model() {
  LOG_STR("Model::~Model()");
  if (iTimerId != -1) {
    killTimer(iTimerId);
  }
  if (iNetReqExecutor) {
    iNetReqExecutor->stop();
    delete iNetReqExecutor;
  }

  closeAllConnections(true);
  delete iConnections;
  delete iNodeModel;
  delete iProfileCommentModel;
  delete iPrivMsgModel;
  delete iProfileModel;
  delete iNetReqQueue;
  delete iBinaryFileModel;
  delete iCaDbRecordModel;
  delete iSearchModel;
  delete iTrustTreeModel;
  delete iTclModel;
  delete iCaModel;
  delete iContentEncryptionModel;
  while (iDbPool.count()) {
    const QThread* firstKey(iDbPool.firstKey());
    QSqlDatabase dbToClose(iDbPool.take(firstKey));
    if (dbToClose.isOpen()) {
      dbToClose.close();
    }
    QLOG_STR("Removed db at model destructor");
  }
  iController = NULL;  // not owned, just set null
  delete iMutex;
#if OPENSSL_VERSION_NUMBER >= 0x30000000L
  if (iLegacyCryptoProvider) {
    OSSL_PROVIDER_unload(iLegacyCryptoProvider);
  }
  if (iDefaultCryptoProvider) {
    OSSL_PROVIDER_unload(iDefaultCryptoProvider);
  }
#endif
}

void Model::closeAllConnections(bool aDeleteAlso) {
  if (iConnections) {
    LOG_STR2("Model::closeAllConnections %d", iConnections->count());
    // normally networklistener has been deleted before
    // model and networklistener is parent of connections,
    // that now have been deleted too due to parent->child
    // relationship -> usually the loop below is looped 0 times.
    for (int i = iConnections->count() - 1; i >= 0; i--) {
      Connection* c = iConnections->at(i);
      if (c) {
        c->iNeedsToRun = false;
      }
      if (aDeleteAlso) {
        delete c;
      }
    }
  }
}

bool Model::lock() {
#ifdef __DEBUG__
  assert(iMutex->tryLock(1000 * 15) == true)
#else
  iMutex->lock();
#endif
      return true;
}

void Model::unlock() { iMutex->unlock(); }

bool Model::openDB() {
  bool tables_needed = false;
  QSqlDatabase db(dataBaseConnection(&tables_needed));
  if (db.isOpen() && tables_needed) {
    if (createTables(db) == false) {
      return false;
    }
  }
  if (db.isOpen()) {
    // do cleanup to rectify previous bugs:
    QSqlQuery query(db);
    // removes every comment that does not begin with { as all json
    // is supposed but it still is not marked as private (==encrypted)
    bool ret = query.exec(
        "delete from profile where profiledata not like x'7b25' and is_private "
        "= 'false' and hash1 not in ( select hash1 from privatekeys )");
    if (ret == false) {
      QLOG_STR("Profile cleanup: " + query.lastError().text());
    } else {
      QLOG_STR("Cleaned away faulty profiles " +
               QString::number(query.numRowsAffected()));
    }
    ret = query.exec(
        "delete from profilecomment where commentdata not like x'7b25' and "
        "flags&1 = 0");
    if (ret == false) {
      QLOG_STR("Profile comment cleanup: " + query.lastError().text());
    } else {
      QLOG_STR("Cleaned away faulty comments " +
               QString::number(query.numRowsAffected()));
    }
  }

  if (db.isOpen()) {
    createTablesV2(db);
    createTablesV3(db);
    createTablesV4(db);
    QSqlQuery versionQuery("select sqlite_version()", db);
    if (versionQuery.exec() && versionQuery.next()) {
      QLOG_STR("Sqlite version " + versionQuery.value(0).toString());
    }
  }
  return db.isOpen();
}

QSqlError Model::lastError() {
  const QSqlDatabase db(dataBaseConnection());
  QSqlError e = db.lastError();
  QLOG_STR(e.text());
  return e;
}

bool Model::createTables(QSqlDatabase aDb) {
  bool ret = false;

  QSqlQuery query(aDb);
  ret = query.exec(
      "create table settings "
      "(datastore_version unsigned int primary key,"
      "node_listenport unsigned int,"
      "profile_maxrows unsigned int,"
      "private_message_maxrows unsigned int,"
      "binaryfile_maxrows unsigned int,"
      "classified_ad_maxrows unsigned int,"
      "profilecomment_maxrows unsigned int,"
      "node_maxrows unsigned int,"
      "node_key varchar(5000),"
      "node_cert varchar(5000),"
      "dns_name varchar(256))");
  if (ret) {
    QSqlQuery query(aDb);
    ret = query.exec("insert into settings (datastore_version) values (1)");
  }
  LOG_STR2("Table settings created 1 is ok, =  %d", ret);
  if (ret) {
    QSqlQuery q2(aDb);
    ret = q2.exec(
        "create table node "
        "(hash1 unsigned int not null,"
        "hash2 unsigned int not null,"
        "hash3 unsigned int not null,"
        "hash4 unsigned int not null,"
        "hash5 unsigned int not null,"
        "listenport unsigned int,"
        "last_conn_time unsigned int,"
        "does_listen unsigned int,"
        "ipv4addr unsigned int,"
        "ipv6addr1 unsigned int,"
        "ipv6addr2 unsigned int,"
        "ipv6addr3 unsigned int,"
        "ipv6addr4 unsigned int,"
        "time_last_reference unsigned int,"
        "last_nodelist_time unsigned int,"
        "dns_name varchar(256),"
        "tor_name varchar(256),"
        "last_mutual_conn_time unsigned int)");
    LOG_STR2("Table node created, retval =  %d", ret);
  }
  if (ret) {
    QSqlQuery q3(aDb);
    // note how index is not unique. set of hash1+2+3+4+5 is
    // (fairly) unique but setting hash1 to be unique would
    // prevent storage of documents(nodes) where hash collides in
    // first 32 bits - that's fairly bigger chance compared
    // to hash collision in "full" 160 bits.
    ret = q3.exec("create index node_hash1_ind on node(hash1)");
    LOG_STR2("Index node(hash1) created, retval =  %d", ret);
  }
  if (ret) {
    QSqlQuery q4(aDb);
    ret = q4.exec(
        "create table privatekeys(prikey varchar(5000) not null,"
        "hash1 unsigned int not null,"
        "hash2 unsigned int not null,"
        "hash3 unsigned int not null,"
        "hash4 unsigned int not null,"
        "hash5 unsigned int not null,"
        "last_msgpoll_time unsigned int,"
        "private_data blob)");
    LOG_STR2("Table for private keys created %d", ret);
  }
  if (ret) {
    QSqlQuery q5(aDb);
    ret = q5.exec(
        "create table profile(pubkey varchar(5000) not null,"
        "hash1 unsigned int not null,"
        "hash2 unsigned int not null,"
        "hash3 unsigned int not null,"
        "hash4 unsigned int not null,"
        "hash5 unsigned int not null,"
        "profiledata blob,"
        "is_private int,"
        "time_last_reference unsigned int,"
        "signature varchar(1024),"
        "display_name varchar(160),"
        "time_of_publish unsigned int,"
        "recvd_from unsigned int,"
        "time_of_update_poll unsigned int)");
    LOG_STR2("Table for profile data created %d", ret);
  }
  if (ret) {
    QSqlQuery q6(aDb);
    ret = q6.exec("create index profile_hash1_ind on profile(hash1)");
    LOG_STR2("Index for profile data table created %d", ret);
  }
  if (ret) {
    QSqlQuery q7(aDb);
    // usage of the following table is this:
    // when new item (profile,ca,privmsg,binary blob) is published
    // it is inserted into this table.
    // networking module will take items (identified by hash1-5)
    // from this table one-by-one, connect to 5 nodes one-by-one
    // (marked by bangpath1-5 where each bangpath contains the
    // low-order bits of the noderef of node where item
    // has already been sent)
    // when 5th node has been filled, row is removed from table
    // and stuff is considered published
    // itemtype is one of enum ProtocolItemType from protocol.h
    //
    // some object types have actually 2 addresses and are
    // published twice: classified ads to content address and
    // the group controller address ; profile comments to
    // content address and profile address.
    ret = q7.exec(
        "create table publish(itemtype int not null,"
        "hash1 unsigned int not null,"
        "hash2 unsigned int not null,"
        "hash3 unsigned int not null,"
        "hash4 unsigned int not null,"
        "hash5 unsigned int not null,"
        "secondary_addr_hash1 unsigned int ,"
        "secondary_addr_hash2 unsigned int ,"
        "secondary_addr_hash3 unsigned int ,"
        "secondary_addr_hash4 unsigned int ,"
        "secondary_addr_hash5 unsigned int ,"
        "bangpath1 unsigned int,"
        "bangpath2 unsigned int,"
        "bangpath3 unsigned int,"
        "bangpath4 unsigned int,"
        "bangpath5 unsigned int,"
        "hostcount int not null,"
        "last_time_tried unsigned int)");
    LOG_STR2("Table for publish data created %d", ret);
  }

  if (ret) {
    QSqlQuery q8(aDb);
    ret = q8.exec(
        "create table binaryfile("
        "hash1 unsigned int not null,"
        "hash2 unsigned int not null,"
        "hash3 unsigned int not null,"
        "hash4 unsigned int not null,"
        "hash5 unsigned int not null,"
        "publisher_hash1 unsigned int not null,"
        "publisher_hash2 unsigned int not null,"
        "publisher_hash3 unsigned int not null,"
        "publisher_hash4 unsigned int not null,"
        "publisher_hash5 unsigned int not null,"
        "contentdata blob,"
        "is_private int,"
        "is_compressed int,"
        "time_last_reference unsigned int,"
        "signature varchar(1024),"
        "display_name varchar(160),"
        "time_of_publish unsigned int,"
        "metadata blob ,"
        "recvd_from unsigned int  )");

    LOG_STR2("Table for binary files created %d", ret);
  }
  if (ret) {
    QSqlQuery q9(aDb);
    ret = q9.exec("create index binaryfile_hash1_ind on binaryfile(hash1)");
    LOG_STR2("Index for binary blob data table created %d", ret);
  }
  if (ret) {
    QSqlQuery q10(aDb);
    ret = q10.exec(
        "create table classified_ad("
        "hash1 unsigned int not null,"
        "hash2 unsigned int not null,"
        "hash3 unsigned int not null,"
        "hash4 unsigned int not null,"
        "hash5 unsigned int not null,"
        "group_hash1 unsigned int not null,"
        "group_hash2 unsigned int not null,"
        "group_hash3 unsigned int not null,"
        "group_hash4 unsigned int not null,"
        "group_hash5 unsigned int not null,"
        "reply_to unsigned int,"
        "time_last_reference unsigned int,"
        "time_added unsigned int not null,"
        "signature varchar(1024),"
        "display_name varchar(160),"
        "time_of_publish unsigned int,"
        "jsondata blob ,"
        "recvd_from unsigned int  )");

    LOG_STR2("Table for classified_ads created %d", ret);
  }
  if (ret) {
    QSqlQuery q11(aDb);
    ret = q11.exec("create index ca_hash1_ind on classified_ad(hash1)");
    LOG_STR2("Index for classified data table created %d", ret);
  }
  if (ret) {
    QSqlQuery q12(aDb);
    ret = q12.exec("create index ca_ghash1_ind on classified_ad(group_hash1)");
    LOG_STR2("Index for classified data table group hash created %d", ret);
  }

  if (ret) {
    QSqlQuery q13(aDb);
    ret = q13.exec(
        "create table private_message("
        "hash1 unsigned int not null,"
        "hash2 unsigned int not null,"
        "hash3 unsigned int not null,"
        "hash4 unsigned int not null,"
        "hash5 unsigned int not null,"
        "dnode_hash1 unsigned int,"
        "dnode_hash2 unsigned int,"
        "dnode_hash3 unsigned int,"
        "dnode_hash4 unsigned int,"
        "dnode_hash5 unsigned int,"
        "recipient_hash1 unsigned int not null,"
        "recipient_hash2 unsigned int not null,"
        "recipient_hash3 unsigned int not null,"
        "recipient_hash4 unsigned int not null,"
        "recipient_hash5 unsigned int not null,"
        "sender_hash1 unsigned int,"
        "is_read int,"
        "time_last_reference unsigned int,"
        "signature varchar(1024),"
        "time_of_publish unsigned int,"
        "messagedata blob ,"
        "recvd_from unsigned int  )");

    LOG_STR2("Table for private messages created %d", ret);
  }
  if (ret) {
    QSqlQuery q14(aDb);
    ret = q14.exec("create index privmsg_hash1_ind on private_message(hash1)");
    QSqlQuery q15(aDb);
    ret = q15.exec(
        "create index privmsg_dhash1_ind on private_message(dnode_hash1)");
    QSqlQuery q16(aDb);
    ret = q16.exec(
        "create index privmsg_rhash1_ind on private_message(recipient_hash1)");
    LOG_STR2("Index for private message data table created %d", ret);
  }
  if (ret) {
    QSqlQuery q17(aDb);
    ret = q17.exec(
        "create table profilecomment("
        "hash1 unsigned int not null,"
        "hash2 unsigned int not null,"
        "hash3 unsigned int not null,"
        "hash4 unsigned int not null,"
        "hash5 unsigned int not null,"
        "profile_hash1 unsigned int,"
        "profile_hash2 unsigned int,"
        "profile_hash3 unsigned int,"
        "profile_hash4 unsigned int,"
        "profile_hash5 unsigned int,"
        "sender_hash1 unsigned int,"
        "time_last_reference unsigned int,"
        "signature varchar(1024),"
        "time_of_publish unsigned int,"
        "commentdata blob ,"
        "recvd_from unsigned int,"
        "flags unsigned int,"
        "display_name varchar(160) )");  // display name might be message
                                         // subject

    LOG_STR2("Table for profile comments created %d", ret);
  }
  if (ret) {
    QSqlQuery q18(aDb);
    ret =
        q18.exec("create index profcomment_hash1_ind on profilecomment(hash1)");
    QSqlQuery q19(aDb);
    ret = q19.exec(
        "create index profcomment_phash1_ind on profilecomment(profile_hash1)");
  }
  // in sqlite there is special table type for full text search.
  if (SearchModel::queryIfFTSSupported(aDb)) {
    SearchModel::createFTSTables(aDb);
  }
  return ret;
}

// method check datastore version. if is 1, upgrades to version 2.
bool Model::createTablesV2(QSqlDatabase aDb) {
  QSqlQuery query(aDb);
  bool ret;
  ret = query.prepare("select datastore_version from settings");
  ret = query.exec();

  if (!ret) {
    QLOG_STR("select datastore_version from settings: " +
             query.lastError().text());
    emit error(MController::DbTransactionError, query.lastError().text());
  } else {
    if (query.next() && !query.isNull(0)) {
      int datastoreVersion(query.value(0).toInt());
      if (datastoreVersion == 1) {
        // upgrade datastore to version 2
        QSqlQuery q(aDb);
        ret = q.exec(
            "alter table settings add column ringtone int not null default 0");
        if (ret) {
          QSqlQuery q3(aDb);
          ret = q3.exec(
              "alter table settings add column call_acceptance int not null "
              "default 0");
          if (!ret) {
            QLOG_STR("alter table settings: " + q3.lastError().text());
            emit error(MController::DbTransactionError, q3.lastError().text());
          }
        }
        if (ret) {
          QSqlQuery q2(aDb);
          q2.exec("update settings set datastore_version = 2");
        }
      }
    }
  }
  return ret;
}

// method check datastore version. if is 2, upgrades to version 3.
bool Model::createTablesV3(QSqlDatabase aDb) {
  QSqlQuery query(aDb);
  bool ret;
  ret = query.prepare("select datastore_version from settings");
  ret = query.exec();

  if (!ret) {
    QLOG_STR("select datastore_version from settings: " +
             query.lastError().text());
    emit error(MController::DbTransactionError, query.lastError().text());
  } else {
    if (query.next() && !query.isNull(0)) {
      int datastoreVersion(query.value(0).toInt());
      if (datastoreVersion == 2) {
        // upgrade datastore to version 3
        QSqlQuery q(aDb);
        ret = q.exec(
            "create table tclprogs "
            "(hash1 unsigned int not null,"
            "hash2 unsigned int not null,"
            "hash3 unsigned int not null,"
            "hash4 unsigned int not null,"
            "hash5 unsigned int not null,"
            "prog_name varchar(256) not null,"
            "prog_text blob not null,"
            "prog_settings blob,"
            "time_modified unsigned int not null)");
        if (ret) {
          QSqlQuery q2(aDb);
          q2.exec("update settings set datastore_version = 3");
        } else {
          QLOG_STR("alter table settings: " + q.lastError().text());
          emit error(MController::DbTransactionError, q.lastError().text());
        }
      }
    }
  }
  return ret;
}

bool Model::createTablesV4(QSqlDatabase aDb) {
  QLOG_STR("createTablesV4 in");
  QSqlQuery query(aDb);
  bool ret;
  ret = query.prepare("select datastore_version from settings");
  ret = query.exec();

  if (!ret) {
    QLOG_STR("select datastore_version from settings: " +
             query.lastError().text());
    emit error(MController::DbTransactionError, query.lastError().text());
  } else {
    if (query.next() && !query.isNull(0)) {
      int datastoreVersion(query.value(0).toInt());
      if (datastoreVersion == 3) {
        // upgrade datastore to version 4
        QSqlQuery q(aDb);
        ret = q.exec(
            "create table dbrecord ( hash1 unsigned int not null,"
            "hash2 unsigned int not null,"
            "hash3 unsigned int not null,"
            "hash4 unsigned int not null,"
            "hash5 unsigned int not null,"
            "collection_hash1 unsigned int not null,"
            "collection_hash2 unsigned int not null,"
            "collection_hash3 unsigned int not null,"
            "collection_hash4 unsigned int not null,"
            "collection_hash5 unsigned int not null,"
            "sender_hash1 unsigned int not null,"
            "sender_hash2 unsigned int not null,"
            "sender_hash3 unsigned int not null,"
            "sender_hash4 unsigned int not null,"
            "sender_hash5 unsigned int not null,"
            "time_last_reference unsigned int not null,"
            "signature varchar(1024) not null,"
            "time_of_publish unsigned int not null,"
            "data blob,"
            "recvd_from  unsigned int not null,"
            "searchstring varchar(4096),"
            "searchnumber bigint,"
            "isencrypted tinyint not null,"
            "issignatureverified tinyint not null);");  // will store boolean
                                                        // value
        if (!ret) {
          QLOG_STR("create table dbrecord: " + q.lastError().text());
          emit error(MController::DbTransactionError, q.lastError().text());
        }
        if (ret && SearchModel::queryIfFTSSupported(aDb)) {
          QSqlQuery ftsQ(aDb);
          ret = ftsQ.exec(
              "create virtual table dbrecord_search using fts3(searchstring)");
          if (!ret) {
            QLOG_STR("FTS3 table dbrecord_search creation: " +
                     ftsQ.lastError().text());
            return false;
          } else {
            QSqlQuery ftsQ2(aDb);
            ret = ftsQ2.exec(
                "CREATE TRIGGER dbrecord_search_del BEFORE DELETE ON dbrecord "
                "BEGIN"
                "  DELETE FROM dbrecord_search WHERE docid=old.hash1;"
                "END;");
            if (!ret) {
              QLOG_STR("CREATE TRIGGER dbrecord_search_del: " +
                       ftsQ2.lastError().text());
              emit error(MController::DbTransactionError,
                         ftsQ2.lastError().text());
            }
          }
        } else {
          QLOG_STR("FTS was not supported");
        }
        if (ret) {
          QSqlQuery q3(aDb);
          ret = q3.exec(
              "alter table settings add column dbrecord_maxrows int not null "
              "default 1000");
          if (!ret) {
            QLOG_STR("add dbrecord_maxrows: " + q3.lastError().text());
            emit error(MController::DbTransactionError, q3.lastError().text());
            return false;
          }
          QSqlQuery q4(aDb);
          ret = q4.exec("update settings set dbrecord_maxrows = 100000");
          if (!ret) {
            QLOG_STR("update dbrecord_maxrows: " + q4.lastError().text());
            emit error(MController::DbTransactionError, q4.lastError().text());
            return false;
          }
          QSqlQuery q5(aDb);
          ret = q5.exec("alter table tclprogs add column prog_data blob");
          if (!ret) {
            QLOG_STR("alter table tclprogs: " + q5.lastError().text());
            emit error(MController::DbTransactionError, q5.lastError().text());
            return false;
          }
        }
        if (ret) {
          QSqlQuery q2(aDb);
          q2.exec("update settings set datastore_version = 4");
        }
      }
    }
  }
  return ret;
}

void Model::initPseudoRandom() {
  SSL_load_error_strings();
  SSL_library_init();
  LOG_STR2("Openssl %s\n", SSLeay_version(SSLEAY_VERSION));
#ifdef WIN32
  OpenSSL_add_all_algorithms();
#endif
#if OPENSSL_VERSION_NUMBER >= 0x30000000L
  LOG_STR("Loading OpenSSL legacy provider for blowfish supoprt");
  /* Load Multiple providers into the default (NULL) library context */
  iLegacyCryptoProvider = OSSL_PROVIDER_load(NULL, "legacy");
  if (iLegacyCryptoProvider == NULL) {
    emit error(MController::ContentEncryptionError, "OpenSSL failed to load Legacy provider");
    printf("Failed to load Legacy provider\n");
  }
  iDefaultCryptoProvider = OSSL_PROVIDER_load(NULL, "default");
  if (iDefaultCryptoProvider == NULL) {
    printf("Failed to load Default provider\n");
    emit error(MController::ContentEncryptionError, "OpenSSL failed to load Default provider");    
    OSSL_PROVIDER_unload(iLegacyCryptoProvider);
    iLegacyCryptoProvider = NULL;
  }
#endif
  // first initialize random numbah generator
  QString randomFileName("");
  if (QFile("/dev/urandom").exists()) {  // fast
    randomFileName = "/dev/urandom";
  } else if (QFile("/dev/arandom").exists()) {  // bsd?
    randomFileName = "/dev/arandom";
  } else if (QFile("/dev/random").exists()) {  // slow but good
    randomFileName = "/dev/random";
  } else {
#ifdef WIN32
#define RANDOM_LEN (15 * sizeof(quint32))
    // this code is from MSDN examples
    HCRYPTPROV hCryptProv = 0;
    BYTE pbData[RANDOM_LEN] = {0};

    LPCWSTR UserName = L"Zappa, Frank";     // name of the key container
    if (CryptAcquireContext(&hCryptProv,    // handle to the CSP
                            UserName,       // container name
                            NULL,           // use the default provider
                            PROV_RSA_FULL,  // provider type
                            0)) {           // flag values
      QLOG_STR("Win32 seed: A cryptographic context has been acquired.");
    } else {
      if (CryptAcquireContext(&hCryptProv, UserName, NULL, PROV_RSA_FULL,
                              CRYPT_NEWKEYSET)) {
        QLOG_STR("Win32 seed: A new key container has been created.\n");
      } else {
        QLOG_STR("Win32 seed: Could not create a new key container.\n");
      }
    }
    if (hCryptProv) {
      if (CryptGenRandom(hCryptProv, RANDOM_LEN, pbData)) {
        QLOG_STR("Win32 seed: Random sequence generated.");
        RAND_seed(&pbData, RANDOM_LEN);
        unsigned int* pointer1intothemiddle =
            reinterpret_cast<unsigned int*>(pbData);
        srand(*pointer1intothemiddle);
      } else {
        printf("Win32 seed: Error during CryptGenRandom.\n");
      }
      if (CryptReleaseContext(hCryptProv, 0)) {
        printf("Win32 seed: The handle has been released.\n");
      } else {
        printf("Win32 seed:The handle could not be released.\n");
      }
    }
#else
#if (QT_VERSION >= QT_VERSION_CHECK(5, 10, 0))
    // since version 5.10 there is random number generator
    quint32 seed[256];
    QRandomGenerator::global()->fillRange(seed);
    srand(seed[0]);
    RAND_seed(&seed[0], sizeof(seed));
#else
    // Old QT. Oh dear. emphasis is on word pseudo.
    QUuid u = QUuid::createUuid();
    QString eight_letters = u.toString();
    eight_letters.remove(0, 25);
    eight_letters.truncate(8);
    unsigned int seed;
    char buf[30];
    sprintf(buf, "%.8s", eight_letters.toUtf8().constData());
    sscanf(buf, "%x", &seed);
    LOG_STR2("Random seed %x", seed);
    srand(seed);
    RAND_seed(&seed, sizeof(seed));
#endif  // QT_VERSION >= 5.10
#endif
  }
  if (randomFileName.length() > 2) {
    QFile randomFile(randomFileName);
    if (randomFile.open(QIODevice::ReadOnly)) {
      QLOG_STR("Used rnd-file " + randomFileName);
      QByteArray randomBytes = randomFile.read(1024);
      char* randomBytesPointer = randomBytes.data();
      RAND_seed(randomBytesPointer, 1024);
      unsigned int* pointer1intothemiddle;
      unsigned int* pointer2intothemiddle;
      pointer1intothemiddle = (unsigned int*)randomBytesPointer;
      pointer1intothemiddle++;
      pointer1intothemiddle++;
      pointer1intothemiddle++;
      pointer1intothemiddle++;
      pointer2intothemiddle = pointer1intothemiddle;
      pointer2intothemiddle++;
      pointer2intothemiddle++;
      pointer2intothemiddle++;
      pointer2intothemiddle++;
      srand(*pointer1intothemiddle);
      randomFile.close();
    } else {
      QLOG_STR("FATAL: Can't open " + randomFileName);
      assert(1 == 2);
    }
  }
}

// called from networklistener, at least
void Model::addOpenNetworkConnection(Connection* aConnection) {
  iConnections->append(aConnection);
  // first thing for the new connection to send is some random
  // number to add confuzius
  //
  // second thing for the new connection to send is greetings
  // so that the other end can identify.
  //
  // wut next?
  aConnection->iNextProtocolItemToSend.append(
      new QByteArray(ProtocolMessageFormatter::randomNumbers()));
  // previous call to random number runs in same thread with model
  aConnection->iNextProtocolItemToSend.append(new QByteArray(
      ProtocolMessageFormatter::nodeGreeting(iController->getNode())));
  // wyt next? we have node connected. what is she going to need?

  // lets do so that we wait until node greeting. there
  // she'll gossip us her latest node ref timestamp
  // so at least we can then send the node ref around
  // her starting from given timestamp .. for now, do no thing.

  // and did we already have too many connections..
  if (iConnections->size() > KMaxOpenConnections) {
    iNodeModel->closeOldestInactiveConnection();
  }
}

void Model::removeOpenNetworkConnection(Connection* aDeletedConnection) {
  int indexInArray = iConnections->indexOf(aDeletedConnection);
  if (indexInArray > -1) {
    iConnections->removeAt(indexInArray);
    LOG_STR2("Removed client %d", indexInArray);
  } else {
    LOG_STR("deleted client not found from array???");
  }
  if (aDeletedConnection->node()) {
    for (int i = (iNetReqQueue->size() - 1); i >= 0; i--) {
      if (iNetReqQueue->value(i).iDestinationNode ==
          aDeletedConnection->node()->nodeFingerPrint()) {
        iNetReqQueue->removeAt(i);
        LOG_STR2("Removed associated netreq at %d", i);
      }
    }
  }
}

// this is called at least by connection itself, when
// it determines it is open for communication. note that
// when connection class this, datamodel is already locked.
//
void Model::connectionStateChanged(Connection::ConnectionState aState,
                                   Connection* aConnection) {
  // may be needed to call lock() in here..
  LOG_STR2("Connectionstate = %d", aState);
  if (aState == Connection::Open) {
    // check that we don't have duplicate connection, it seems to
    // happen easily if we have broadcast together with multiple
    // ipv6 addresses per single network interface: broadcasts come
    // from one address, address included inside is another..

    // and check self first
    if (aConnection->getPeerHash() ==
        iController->getNode().nodeFingerPrint()) {
      // great, managed to connect to self
      aConnection->iNeedsToRun = false;
      return;
    }
    // then loop through connections
    foreach (const Connection* c, *iConnections) {
      if (c) {
        if (c != aConnection && c->connectionState() == Connection::Open &&
            aConnection->getPeerHash() == c->getPeerHash()) {
          LOG_STR("Double connection to same node detected");
          aConnection->iNeedsToRun = false;
          break;
        }
      }
    }
  }
}

const QList<Connection*>& Model::getConnections() const {
  return *iConnections;
}

QList<NetworkRequestExecutor::NetworkRequestQueueItem>& Model::getNetRequests()
    const {
  return *iNetReqQueue;
}

void Model::addNetworkRequest(
    NetworkRequestExecutor::NetworkRequestQueueItem& aRequest) const {
  if (iController->getNode().nodeFingerPrint() == aRequest.iDestinationNode) {
    QLOG_STR("Skipping network request destined to self");
  } else {
    aRequest.iTimeStampOfLastActivity =
        QDateTime::currentDateTimeUtc().toTime_t();
    if (iNetReqQueue) {
      iNetReqQueue->append(aRequest);
    }
  }
}

void Model::addItemToSend(const Hash& aDestinationNode,
                          QByteArray* aBytesToSend) {
  Connection* c = NULL;
  const Node* nodeOfConnection = NULL;
  // connections are in no particular order so linear search..
  for (int i = iConnections->size() - 1; i >= 0; i--) {
    c = iConnections->value(i);
    nodeOfConnection = c->node();
    if (c->connectionState() == Connection::Open && nodeOfConnection &&
        (nodeOfConnection->nodeFingerPrint() == aDestinationNode)) {
      // yes.
      c->iNextProtocolItemToSend.append(aBytesToSend);
      return;  // work done, get out
    }
  }
  // no match found, just delete bytes
  delete aBytesToSend;
}

MNodeModelProtocolInterface& Model::nodeModel() const { return *iNodeModel; }

ContentEncryptionModel& Model::contentEncryptionModel() const {
  return *iContentEncryptionModel;
}

ProfileModel& Model::profileModel() const { return *iProfileModel; }

BinaryFileModel& Model::binaryFileModel() const { return *iBinaryFileModel; }

ClassifiedAdsModel& Model::classifiedAdsModel() const { return *iCaModel; }

PrivMessageModel& Model::privateMessageModel() const { return *iPrivMsgModel; }
ProfileCommentModel& Model::profileCommentModel() const {
  return *iProfileCommentModel;
}
SearchModel* Model::searchModel() const { return iSearchModel; }
CaDbRecordModel* Model::caDbRecordModel() const { return iCaDbRecordModel; }
TrustTreeModel* Model::trustTreeModel() const { return iTrustTreeModel; }

TclModel& Model::tclModel() const { return *iTclModel; }

NetworkRequestExecutor* Model::getNetReqExecutor() { return iNetReqExecutor; }

/**
 * method for getting next item to publish
 * @param aFound will be set to true if there is an item to publish
 * @return if aFound is true, returned item contains details about
 *         the item to publish.
 */
PublishItem Model::nextItemToPublish(bool* aFound) {
  PublishItem retval;
  *aFound = false;

  QSqlQuery query(iController->model().dataBaseConnection());
  bool ret;
  // don't pick up the same item again if less than 10 minutes passed
  ret = query.prepare(
      "select itemtype,hash1,hash2,hash3,hash4,hash5, "
      " bangpath1,bangpath2,bangpath3,bangpath4,bangpath5,"
      "hostcount,last_time_tried,secondary_addr_hash1,"
      "secondary_addr_hash2,secondary_addr_hash3,"
      "secondary_addr_hash4,secondary_addr_hash5 "
      " from publish where last_time_tried < :now_minus_10_minutes"
      " order by last_time_tried desc limit 1");
  query.bindValue(":now_minus_10_minutes",
                  QDateTime::currentDateTimeUtc().toTime_t() - (60 * 10));
  ret = query.exec();
  if (!ret) {
    QLOG_STR(query.lastError().text() + " " + __FILE__ +
             QString::number(__LINE__));
    emit error(MController::DbTransactionError, query.lastError().text());
  } else {
    if (query.next() && !query.isNull(0) && !query.isNull(1)) {
      retval.iObjectType = (ProtocolItemType)(query.value(0).toInt());
      quint32 h1 = query.value(1).toUInt();
      quint32 h2 = query.value(2).toUInt();
      quint32 h3 = query.value(3).toUInt();
      quint32 h4 = query.value(4).toUInt();
      quint32 h5 = query.value(5).toUInt();
      retval.iObjectHash = Hash(h1, h2, h3, h4, h5);
      if (!query.isNull(13)) {
        // there is secondary addr
        quint32 h1_2nd = query.value(13).toUInt();
        quint32 h2_2nd = query.value(14).toUInt();
        quint32 h3_2nd = query.value(15).toUInt();
        quint32 h4_2nd = query.value(16).toUInt();
        quint32 h5_2nd = query.value(17).toUInt();
        retval.i2NdAddr = Hash(h1_2nd, h2_2nd, h3_2nd, h4_2nd, h5_2nd);
      }
      retval.iAlreadyPushedHosts.clear();
      if (!query.isNull(6)) {
        quint32 bangPath1Value(query.value(6).toUInt());
        LOG_STR2("BangPath1 value from db = %u", bangPath1Value);
        retval.iAlreadyPushedHosts.append(bangPath1Value);
      } else {
        LOG_STR("BangPath1 value in  db was null");
      }
      if (!query.isNull(7)) {
        retval.iAlreadyPushedHosts.append(query.value(7).toUInt());
        LOG_STR2("BangPath2 value from db = %u", query.value(7).toUInt());
      } else {
        LOG_STR("BangPath2 value in  db was null");
      }
      if (!query.isNull(8)) {
        retval.iAlreadyPushedHosts.append(query.value(8).toUInt());
      }
      if (!query.isNull(9)) {
        retval.iAlreadyPushedHosts.append(query.value(9).toUInt());
      }
      if (!query.isNull(10)) {
        retval.iAlreadyPushedHosts.append(query.value(10).toUInt());
      }
      LOG_STR2("Publish item found was %d minutes old",
               (QDateTime::currentDateTimeUtc().toTime_t() -
                query.value(12).toUInt()) /
                   60);
      LOG_STR2("Publish had %u hops in bangpath",
               retval.iAlreadyPushedHosts.size());
      if (retval.iAlreadyPushedHosts.size()) {
        for (int ii = 0; ii < retval.iAlreadyPushedHosts.size(); ii++) {
          LOG_STR2("bangpath content %u", retval.iAlreadyPushedHosts[ii]);
        }
      }
      *aFound = true;
      // because item was found, mark the time when it was found
      // so next time it will appear at the bottom of our list

      QSqlQuery query2(iController->model().dataBaseConnection());
      query2.prepare(
          "update publish set last_time_tried=:time where hash1=:hash1 and "
          "hash2=:hash2 and hash3=:hash3 and hash4=:hash4 and hash5=:hash5");
      query2.bindValue(":hash1", retval.iObjectHash.iHash160bits[0]);
      query2.bindValue(":hash2", retval.iObjectHash.iHash160bits[1]);
      query2.bindValue(":hash3", retval.iObjectHash.iHash160bits[2]);
      query2.bindValue(":hash4", retval.iObjectHash.iHash160bits[3]);
      query2.bindValue(":hash5", retval.iObjectHash.iHash160bits[4]);
      query2.bindValue(":time", QDateTime::currentDateTimeUtc().toTime_t());
      if (query2.exec() == false) {
        QLOG_STR(query2.lastError().text() + " " + __FILE__ +
                 QString::number(__LINE__));
        emit error(MController::DbTransactionError, query2.lastError().text());
      }
    }
  }

  return retval;
}
void Model::addItemToBePublished(ProtocolItemType aType, const Hash& aHash,
                                 const QList<quint32>& aBangPath,
                                 const Hash& aSecondaryAddr) {
  // short-cut handling for private messages that have
  // destination node set:
  if (aType == PrivateMessage && aSecondaryAddr != KNullHash &&
      iNodeModel->isNodeAlreadyConnected(aSecondaryAddr)) {
    struct NetworkRequestExecutor::NetworkRequestQueueItem sendReq;
    sendReq.iDestinationNode = aSecondaryAddr;
    sendReq.iRequestedItem = aHash;
    sendReq.iMaxNumberOfItems = 1;
    sendReq.iBangPath = aBangPath;
    sendReq.iState = NetworkRequestExecutor::NewRequest;
    sendReq.iRequestType = PrivateMessage;
    addNetworkRequest(sendReq);
    LOG_STR("published privmsg sent via short-cut");
  } else {
    // check we don't have the item already:
    if (aBangPath.size() >= 5) {
      return;  // consider it published
    }

    int count(0);
    QSqlQuery query(iController->model().dataBaseConnection());
    bool ret;
    ret = query.prepare(
        "select count (itemtype) from publish where "
        "hash1 = :hash1 and hash2 = :hash2 and hash3 = :hash3 and hash4 = "
        ":hash4 "
        "and hash5 = :hash5 and itemtype = :itemtype");
    query.bindValue(":hash1", aHash.iHash160bits[0]);
    query.bindValue(":hash2", aHash.iHash160bits[1]);
    query.bindValue(":hash3", aHash.iHash160bits[2]);
    query.bindValue(":hash4", aHash.iHash160bits[3]);
    query.bindValue(":hash5", aHash.iHash160bits[4]);
    query.bindValue(":itemtype", (int)aType);
    ret = query.exec();
    if (!ret) {
      LOG_STR2("Error while selecting from publish %s",
               qPrintable(query.lastError().text()));
      emit error(MController::DbTransactionError, query.lastError().text());
    } else {
      if (query.next() && !query.isNull(0)) {
        count = query.value(0).toInt();
      }
      QSqlQuery query2(iController->model().dataBaseConnection());
      if (count == 0) {
        // item did not exist beforehand
        ret = query2.prepare(
            "insert into publish(itemtype,hash1,hash2,hash3,"
            "hash4,hash5,hostcount,last_time_tried,bangpath1,"
            "bangpath2,bangpath3,bangpath4,bangpath5,"
            "secondary_addr_hash1,secondary_addr_hash2,"
            "secondary_addr_hash3,"
            "secondary_addr_hash4,secondary_addr_hash5) values "
            "(:itemtype,:hash1,:hash2,:hash3,:hash4,:hash5,0,0,"
            ":b1,:b2,:b3,:b4,:b5,:sa1,:sa2,:sa3,:sa4,:sa5)");
        query2.bindValue(":itemtype", (int)aType);
      } else {
        ret = query2.prepare(
            "update publish set last_time_tried=0,bangpath1=:b1,"
            "bangpath2=:b2,bangpath3=:b3,bangpath4=:b4,bangpath5=:b5,"
            "hostcount=0,secondary_addr_hash1=:sa1,secondary_addr_hash2=:sa2,"
            "secondary_addr_hash3=:sa3,"
            "secondary_addr_hash4=:sa4,"
            "secondary_addr_hash5=:sa5 "
            "where hash1=:hash1 and hash2=:hash2 and "
            "hash3=:hash3 and hash4=:hash4 and hash5=:hash5");
      }
      if (ret) {
        query2.bindValue(":hash1", aHash.iHash160bits[0]);
        query2.bindValue(":hash2", aHash.iHash160bits[1]);
        query2.bindValue(":hash3", aHash.iHash160bits[2]);
        query2.bindValue(":hash4", aHash.iHash160bits[3]);
        query2.bindValue(":hash5", aHash.iHash160bits[4]);
        if (aSecondaryAddr == KNullHash) {
          query2.bindValue(":sa1", QVariant(QVariant::String));
          query2.bindValue(":sa2", QVariant(QVariant::String));
          query2.bindValue(":sa3", QVariant(QVariant::String));
          query2.bindValue(":sa4", QVariant(QVariant::String));
          query2.bindValue(":sa5", QVariant(QVariant::String));
        } else {
          query2.bindValue(":sa1", aHash.iHash160bits[0]);
          query2.bindValue(":sa2", aHash.iHash160bits[1]);
          query2.bindValue(":sa3", aHash.iHash160bits[2]);
          query2.bindValue(":sa4", aHash.iHash160bits[3]);
          query2.bindValue(":sa5", aHash.iHash160bits[4]);
        }
        if (aBangPath.size() >= 1 && aBangPath[0]) {
          query2.bindValue(":b1", aBangPath[0]);
          LOG_STR2("### Query 2 bindval for bangpath[0] = %u", aBangPath[0]);
        } else {
          query2.bindValue(":b1", QVariant(QVariant::String));  // null value
          LOG_STR("### Query 2 bindval for bangpath[0] = null");
        }
        if (aBangPath.size() >= 2 && aBangPath[1]) {
          query2.bindValue(":b2", aBangPath[1]);
        } else {
          query2.bindValue(":b2", QVariant(QVariant::String));  // null value
        }
        if (aBangPath.size() >= 3 && aBangPath[2]) {
          query2.bindValue(":b3", aBangPath[2]);
        } else {
          query2.bindValue(":b3", QVariant(QVariant::String));  // null value
        }
        if (aBangPath.size() >= 4 && aBangPath[3]) {
          query2.bindValue(":b4", aBangPath[3]);
        } else {
          query2.bindValue(":b4", QVariant(QVariant::String));  // null value
        }
        if (aBangPath.size() >= 5 && aBangPath[4]) {
          query2.bindValue(":b5", aBangPath[4]);
        } else {
          query2.bindValue(":b5", QVariant(QVariant::String));  // null value
        }
        ret = query2.exec();
      }
      if (!ret) {
        QLOG_STR(query2.lastError().text() + " " + __FILE__ +
                 QString::number(__LINE__));
        emit error(MController::DbTransactionError, query2.lastError().text());
      } else {
        LOG_STR2("Added/updated item to be published, type = %d", aType);
      }
    }
  }
}

PublishItem Model::addNodeToPublishedItem(const Hash& aContentHash,
                                          const Hash& aNodeHash) {
  PublishItem retval;

  QSqlQuery query(iController->model().dataBaseConnection());
  bool ret;
  ret = query.prepare(
      "select itemtype, "
      " bangpath1,bangpath2,bangpath3,bangpath4,bangpath5,hostcount"
      " from publish where hash1=:hash1 and hash2=:hash2 and"
      " hash3=:hash3 and hash4=:hash4 and hash5=:hash5");
  query.bindValue(":hash1", aContentHash.iHash160bits[0]);
  query.bindValue(":hash2", aContentHash.iHash160bits[1]);
  query.bindValue(":hash3", aContentHash.iHash160bits[2]);
  query.bindValue(":hash4", aContentHash.iHash160bits[3]);
  query.bindValue(":hash5", aContentHash.iHash160bits[4]);
  ret = query.exec();
  if (!ret) {
    QLOG_STR("addNodeToPublishedItem error 1 " + query.lastError().text());
    emit error(MController::DbTransactionError, query.lastError().text());
  } else {
    if (query.next() && !query.isNull(0)) {
      retval.iObjectType = (ProtocolItemType)(query.value(0).toInt());
      QLOG_STR("addNodeToPublishedItem object type at return =  " +
               retval.iObjectType);
      retval.iAlreadyPushedHosts.clear();
      if (!query.isNull(1)) {
        retval.iAlreadyPushedHosts.append(query.value(1).toUInt());
      }
      if (!query.isNull(2)) {
        retval.iAlreadyPushedHosts.append(query.value(2).toUInt());
      }
      if (!query.isNull(3)) {
        retval.iAlreadyPushedHosts.append(query.value(3).toUInt());
      }
      if (!query.isNull(4)) {
        retval.iAlreadyPushedHosts.append(query.value(4).toUInt());
      }
      if (!query.isNull(5)) {
        retval.iAlreadyPushedHosts.append(query.value(5).toUInt());
      }
    }
    int howManyNodesIsEnough(5);
    if (iNodeModel->getHotAddresses().size() < howManyNodesIsEnough) {
      // really minimalistic network. might be a real condition tough,
      // a private installation of some sort.
      howManyNodesIsEnough = iNodeModel->getHotAddresses().size();
      LOG_STR2("Node-count to decide publishing completed = %d",
               howManyNodesIsEnough);
    }
    if (retval.iAlreadyPushedHosts.size() >= howManyNodesIsEnough) {
      retval.iObjectHash =
          KNullHash;  // set NULL hash to mark that this is now done
      for (int ii = 0; ii < retval.iAlreadyPushedHosts.size(); ii++) {
        QString qStr = QString::number(retval.iAlreadyPushedHosts.at(ii));
        QLOG_STR("Deleteting from publish because " + qStr);
      }

      QSqlQuery q2(iController->model().dataBaseConnection());
      q2.prepare(
          "delete from publish where hash1=:hash1 and hash2=:hash2 and"
          " hash3=:hash3 and hash4=:hash4 and hash5=:hash5");
      q2.bindValue(":hash1", aContentHash.iHash160bits[0]);
      q2.bindValue(":hash2", aContentHash.iHash160bits[1]);
      q2.bindValue(":hash3", aContentHash.iHash160bits[2]);
      q2.bindValue(":hash4", aContentHash.iHash160bits[3]);
      q2.bindValue(":hash5", aContentHash.iHash160bits[4]);
      q2.exec();
      QLOG_STR("Deleteted from publish, nr hosts was " +
               retval.iAlreadyPushedHosts.size());
      retval.iObjectHash = KNullHash;
    } else {
      QSqlQuery q2(iController->model().dataBaseConnection());
      q2.prepare(
          "update publish set bangpath1=:b1,bangpath2=:b2,bangpath3=:b3,"
          "bangpath4=:b4,bangpath5=:b5 where  hash1=:hash1 and hash2=:hash2 and"
          " hash3=:hash3 and hash4=:hash4 and hash5=:hash5");
      q2.bindValue(":hash1", aContentHash.iHash160bits[0]);
      q2.bindValue(":hash2", aContentHash.iHash160bits[1]);
      q2.bindValue(":hash3", aContentHash.iHash160bits[2]);
      q2.bindValue(":hash4", aContentHash.iHash160bits[3]);
      q2.bindValue(":hash5", aContentHash.iHash160bits[4]);
      int i = 0;
      // put the low-order bits into bottom of the list:
      if (!retval.iAlreadyPushedHosts.contains(aNodeHash.iHash160bits[4])) {
        quint32 low_order_bits = aNodeHash.iHash160bits[4];
        LOG_STR2("Pub: adding host low-order bits #1 : %u", low_order_bits);
        retval.iAlreadyPushedHosts.append(low_order_bits);
      }
      // and store list into db:
      for (; i < retval.iAlreadyPushedHosts.size(); i++) {
        q2.bindValue(QString(":b%1").arg(i + 1), retval.iAlreadyPushedHosts[i]);
        LOG_STR2("In addNodeToPublishedItem already contained : %u",
                 retval.iAlreadyPushedHosts[i]);
      }
      // update remaining to be null:
      for (; i < 5; i++) {
        q2.bindValue(QString(":b%1").arg(i + 1), QVariant(QVariant::String));
        LOG_STR2("In addNodeToPublishedItem b %d is null", i);
      }
      if (q2.exec() == true) {
        retval.iObjectHash = aContentHash;
      } else {
        retval.iObjectHash =
            KNullHash;  // set NULL hash to mark that this is now not done
        QLOG_STR("addNodeToPublishedItem error 3 " + q2.lastError().text());
        emit error(MController::DbTransactionError, q2.lastError().text());
      }
    }
  }
  return retval;
}

// slot hit when content is received from remote nodes
void Model::notifyOfContentReceived(
    const Hash& aHashOfContent, const ProtocolItemType aTypeOfReceivdContent) {
  LOG_STR2("Model::notifyOfContentReceived %d", aTypeOfReceivdContent);

  switch (aTypeOfReceivdContent) {
    case BinaryBlob:
      lock();
      for (int i = (iNetReqQueue->size() - 1); i >= 0; i--) {
        if (iNetReqQueue->value(i).iRequestType == RequestForBinaryBlob &&
            iNetReqQueue->value(i).iRequestedItem == aHashOfContent) {
          iNetReqQueue->removeAt(i);
          LOG_STR2("Removed associated netreq bblob at %d", i);
        }
      }
      unlock();
      break;
    case UserProfile:
      lock();
      for (int i = (iNetReqQueue->size() - 1); i >= 0; i--) {
        if (iNetReqQueue->value(i).iRequestType == RequestForUserProfile &&
            iNetReqQueue->value(i).iRequestedItem == aHashOfContent) {
          iNetReqQueue->removeAt(i);
          LOG_STR2("Removed associated netreq prof at %d", i);
        }
      }
      unlock();
      break;
    case ClassifiedAd:
      lock();
      for (int i = (iNetReqQueue->size() - 1); i >= 0; i--) {
        if (iNetReqQueue->value(i).iRequestType == RequestForClassifiedAd &&
            // don't remove requests of other nodes, only UI-initiated
            // (having null destination node)
            iNetReqQueue->value(i).iDestinationNode == KNullHash &&
            iNetReqQueue->value(i).iRequestedItem == aHashOfContent) {
          iNetReqQueue->removeAt(i);
          LOG_STR2("Removed associated netreq ca at %d", i);
        }
      }
      unlock();
      break;
    default:
      // no action
      break;
  }
}

void Model::notifyOfContentReceived(
    const Hash& aHashOfContent, const Hash& /* aHashOfClassification */,
    const ProtocolItemType aTypeOfReceivdContent) {
  this->notifyOfContentReceived(aHashOfContent, aTypeOfReceivdContent);
}

//
// this method is hit every 10 minutes - housekeeping here
//
void Model::timerEvent(QTimerEvent*
#ifdef DEBUG
                           event  // in debug build logging needs event id
#else
/* event */  // in release this is not used for anything
#endif
) {
  LOG_STR2("timerEvent Timer ID: %d", event->timerId());
  const time_t currentTime(QDateTime::currentDateTimeUtc().toTime_t());
  lock();
  // update mutual connect time:
  for (int i = iConnections->size() - 1; i >= 0; i--) {
    Connection* connectedNode = iConnections->at(i);
    if (connectedNode->node()) {
      iNodeModel->updateNodeLastMutualConnectTimeInDb(
          connectedNode->node()->nodeFingerPrint(), currentTime);
    }
  }
  // then check for dead connections:
  for (int i = iConnections->size() - 1; i >= 0; i--) {
    Connection* connectedNode = iConnections->at(i);
    if (connectedNode->iTimeOfLastActivity <
        (currentTime - (60 * (Connection::iMinutesBetweenBucketFill + 2)))) {
      // no traffic for minimum time + 2 minutes -> ask non-gently for closing:
      connectedNode->forciblyCloseSocket();
      LOG_STR2("Connection at array pos %d has been dead -> abort()", i);
    }
  }
  unlock();
  //
  // dead process checking now done
  //
  // continue with database housekeeping
  //

  QThread::yieldCurrentThread();
  lock();
  iNodeModel->truncateDataTableToMaxRows();
  unlock();
  QThread::yieldCurrentThread();
  lock();
  // looks like profiles (or more specifically, public keys) are
  // inserted from multitude of places ; with profile model
  // lets do so that it is asked to update count first from table.
  iProfileModel->updateDbTableRowCount();
  // and then go to work
  iProfileModel->truncateDataTableToMaxRows();
  unlock();
  QThread::yieldCurrentThread();
  lock();
  iCaModel->truncateDataTableToMaxRows();
  unlock();
  QThread::yieldCurrentThread();
  lock();
  iPrivMsgModel->truncateDataTableToMaxRows();
  unlock();
  QThread::yieldCurrentThread();
  lock();
  iBinaryFileModel->truncateDataTableToMaxRows();
  unlock();
  QThread::yieldCurrentThread();
  lock();
  iProfileCommentModel->updateDbTableRowCount();
  // and then go to work
  iProfileModel->truncateDataTableToMaxRows();
  unlock();
  QThread::yieldCurrentThread();
  lock();
  iCaDbRecordModel->truncateDataTableToMaxRows();
  unlock();
  QThread::yieldCurrentThread();
  lock();
  QSqlQuery vacuumQuery(iController->model().dataBaseConnection());
  vacuumQuery.exec("vacuum");
  unlock();
  // then check out for local network addresses:
  if ((iTimeOfLastNetworkAddrCheck + (120 * 60)) <  // every 120 minutes
      QDateTime::currentDateTimeUtc().toTime_t()) {
    iTimeOfLastNetworkAddrCheck = QDateTime::currentDateTimeUtc().toTime_t();
    lock();
    iController->networkListener()->figureOutLocalAddresses();
    unlock();
  }
  LOG_STR2("timerEvent Timer ID: %d out", event->timerId());
}

Model::RingtoneSetting Model::getRingtoneSetting() {
  RingtoneSetting retval(BowRingTone);
  QSqlQuery query(iController->model().dataBaseConnection());
  bool ret;

  ret = query.prepare("select ringtone from settings");
  ret = query.exec();

  if (!ret) {
    QLOG_STR("select ringtone from settings: " + query.lastError().text());
    emit error(MController::DbTransactionError, query.lastError().text());
  } else {
    if (query.next() && !query.isNull(0)) {
      retval = (Model::RingtoneSetting)(query.value(0).toInt());
    }
  }
  return retval;
}

void Model::setRingtoneSetting(Model::RingtoneSetting aRingTone) {
  QSqlQuery query(iController->model().dataBaseConnection());
  bool ret(query.prepare("update settings set ringtone = :r"));
  if (ret) {
    query.bindValue(":r", aRingTone);
    query.exec();
  }
}

Model::CallAcceptanceSetting Model::getCallAcceptanceSetting() {
  CallAcceptanceSetting retval(AcceptAllCalls);
  QSqlQuery query(iController->model().dataBaseConnection());
  bool ret;

  ret = query.prepare("select call_acceptance from settings");
  ret = query.exec();

  if (!ret) {
    QLOG_STR("select call acceptance from settings: " +
             query.lastError().text());
    emit error(MController::DbTransactionError, query.lastError().text());
  } else {
    if (query.next() && !query.isNull(0)) {
      retval = (Model::CallAcceptanceSetting)(query.value(0).toInt());
    }
  }
  return retval;
}

void Model::setCallAcceptanceSetting(Model::CallAcceptanceSetting aAcceptance) {
  QSqlQuery query(iController->model().dataBaseConnection());
  bool ret(query.prepare("update settings set call_acceptance = :a"));
  if (ret) {
    query.bindValue(":a", aAcceptance);
    query.exec();
  }
}

// see also method threadTerminationCleanup
QSqlDatabase Model::dataBaseConnection(bool* aIsFirstTime) {
  const QThread* t(QThread::currentThread());
  if (iDbPool.contains(t)) {
    return iDbPool.value(t);
  }
  // was not in pool, instantiate and put the db connection into
  // pool for further use.
  bool tables_needed = false;
  const QString threadId(
      QString::number((qulonglong)(QThread::currentThreadId())));
  iDbPool.insert(t, QSqlDatabase::addDatabase("QSQLITE", threadId));
  QSqlDatabase db(iDbPool.value(t));
  QLOG_STR("Instantiated new db for thread " + threadId +
           " nr connections = " + QString::number(iDbPool.count()));
  QString path(QDir::home().path());
  path.append(QDir::separator()).append(".classified_ads");
  if (!QDir(path).exists()) {
    QDir().mkdir(path);
    tables_needed = true;
  }
  path.append(QDir::separator()).append("sqlite_db");
  path = QDir::toNativeSeparators(path);
  if (!QFile(path).exists()) {
    tables_needed = true;
  }
  if (aIsFirstTime) {
    *aIsFirstTime = tables_needed;
  }
  db.setDatabaseName(path);
  // Open database
  bool isDbOpen = db.open();
  if (isDbOpen) {
    if (tables_needed) {
      // first time here, set file mode
      QFile databaseFile(path);
      databaseFile.setPermissions(QFile::ReadOwner | QFile::WriteOwner);
    }
    return db;
  } else {
    QLOG_STR("database open did not succeed " + db.lastError().text());
    return db;
  }
}

// see also method dataBaseConnection
void Model::threadTerminationCleanup(const QThread* aTerminatingThread) {
  QLOG_STR("Model::threadTerminationCleanup in " +
           QString::number((qulonglong)(QThread::currentThreadId())));
  lock();
  if (iDbPool.contains(aTerminatingThread)) {
    QSqlDatabase dbToClose(iDbPool.value(aTerminatingThread));
    const QString dbConnectionName(dbToClose.connectionName());
    if (dbToClose.isOpen()) {
      dbToClose.close();
    }
    iDbPool.remove(aTerminatingThread);  // first remove, it de-allocates the db
    QSqlDatabase::removeDatabase(
        dbConnectionName);  // then update static application-wide
    QLOG_STR("Removed db for thread " +
             QString::number((qulonglong)(QThread::currentThreadId())) +
             " connection name " + dbConnectionName +
             " nr connections = " + QString::number(iDbPool.count()));
  }
  unlock();
}
