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

#include "mockup_nodemodel.h"
#include "../log.h"
#include "../net/node.h"
#include <QSslKey>

MockUpNodeModel::MockUpNodeModel( MController *aController ) : 
  iLastNodeReceived(NULL),
  iController(aController)
{
  iFingerPrintOfThisNode = new Hash(1,2,3,4,5) ; 
  LOG_STR("MockUpNodeModel::MockUpNodeModel in\n") ; 
  LOG_STR("MockUpNodeModel::MockUpNodeModel out\n") ; 
}

MockUpNodeModel::~MockUpNodeModel()
{
  LOG_STR("MockUpNodeModel::~MockUpNodeModel\n") ;
  iController = NULL ; 
  if ( iLastNodeReceived ) {
    delete iLastNodeReceived ; 
  }
  delete  iFingerPrintOfThisNode ; 
}

bool MockUpNodeModel::nodeGreetingReceived(Node& aNode ,
			  bool aWasInitialGreeting  )  
{
  QLOG_STR("MockUpNodeModel::nodeGreetingReceived " + aNode.nodeFingerPrint().toString()) ; 
  if ( iLastNodeReceived) {
    delete iLastNodeReceived ;
  }
  iLastNodeReceived = new Node(aNode.nodeFingerPrint(), aNode.port()) ; 
  iLastNodeReceived->setIpv4Addr(aNode.ipv4Addr()) ; 
  iLastNodeReceived->setIpv6Addr(aNode.ipv6Addr()) ; 
  iLastNodeReceived->setGoodNodeListTime(aNode.goodNodeListTime()) ;   
  iLastNodeReceived->setLastConnectTime(aNode.lastConnectTime()) ; 
  iLastNodeReceived->setLastMutualConnectTime(aNode.lastMutualConnectTime()) ; 
  iLastNodeReceived->setCanReceiveIncoming(aNode.canReceiveIncoming()) ; 
  return true ; 
}

Hash& MockUpNodeModel::nodeFingerPrint()  /**< returns fingerprint of this node */
{
  return *iFingerPrintOfThisNode ; 
}

int MockUpNodeModel::listenPortOfThisNode()   /**< TCP listen port number method */
{
  return 3 ; 
}

const QSslCertificate& MockUpNodeModel::nodeCert()  const  
{
  static QSslCertificate retval ; 
  return retval ; 
}

/** getter for ssl certificate of SSL sock */
const QSslKey& MockUpNodeModel::nodeKey()  const  

{
  static QSslKey retval ; 
  return retval ; 
}

QByteArray* MockUpNodeModel::getNextItemToSend(Connection& aConnection)   
{
  return NULL ; 
}

Node* MockUpNodeModel::nodeByHash(const Hash& aHash) 
{
  return NULL ; 
}
QList<Node *>* MockUpNodeModel::getNodesBeforeHash(const Hash& aHash,
				  int aMaxNodes)  
{
  return NULL ; 
}

void MockUpNodeModel::closeOldestInactiveConnection()  
{

}

QList<Node *>* MockUpNodeModel::getNodesAfterHash(const Hash& aHash,
				 int aMaxNodes,
				 int aMaxInactivityMinutes )  
{
  return NULL ; 
}

QList<MNodeModelProtocolInterface::HostConnectQueueItem > MockUpNodeModel::getHotAddresses() 
{
  static QList<MNodeModelProtocolInterface::HostConnectQueueItem> retval ; 
  return retval ; 
}

bool MockUpNodeModel::updateNodeLastConnectTimeInDb(Node& aNode)  
{
  return false ;
}

QList<Node *>* MockUpNodeModel::getHotNodes(int aMaxNodes)  
{
  return NULL ; 
}

void MockUpNodeModel::addNodeFromBroadcast(const Hash& /*aNodeFingerPrint*/,
					   const QHostAddress& /*aAddr*/,
					   int aPort ) {
  LOG_STR2("addNodeFromBroadcast %d\n", aPort) ; 
}

bool MockUpNodeModel::addNodeToConnectionWishList(Node* aNode) {
  LOG_STR("addNodeToConnectionWishList \n") ; 
  delete aNode ; 
  return true ; 
}

bool MockUpNodeModel::addNodeToConnectionWishList(const Hash& aNode) {
  LOG_STR("addNodeToConnectionWishList hash-reference version \n") ; 
  return true ; 
}

Node* MockUpNodeModel::nextConnectionWishListItem()  {
  return NULL; // means that no more connections in list
}

bool MockUpNodeModel::isNodeAlreadyConnected(const Node& aNode) const {
  return true ; 
}

bool MockUpNodeModel::isNodeAlreadyConnected(const Hash& aHash) const {
  return false ; 
}

Hash MockUpNodeModel::bucketEndHash(const Hash& aFingerPrintOfNodeAsking)  {
  return aFingerPrintOfNodeAsking    ;
}

bool MockUpNodeModel::updateNodeLastMutualConnectTimeInDb(const Hash& aNodeFp,
							  quint32 aTime ) {
  return true ; 
}

void MockUpNodeModel::setListenPortOfThisNode(int port) {
  // yes
  return ; 
}
QList<Node*>* MockUpNodeModel::getNodesBeforeHash(const Hash& h, unsigned int i) {
  QList<Node*>* r = new QList<Node*>() ;
  return r ; 
}
QList<Node*>* MockUpNodeModel::getNodesAfterHash(const Hash& h, unsigned int u, int i) {
  QList<Node*>* r = new QList<Node*>() ;
  return r ; 
}
void MockUpNodeModel::setDnsName(QString s){
  iDnsName = s ;
}

QString MockUpNodeModel::getDnsName() {
  return iDnsName ; 
}


void MockUpNodeModel::offerNodeToRecentlyFailedList(const Hash& /*aFailedNodeHash*/) {
  // have very thin implementation here
}
