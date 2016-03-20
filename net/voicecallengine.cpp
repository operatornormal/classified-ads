/*     -*-C++-*- -*-coding: utf-8-unix;-*-
  Classified Ads is Copyright (c) Antti Järvinen 2015.

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
#include <QSize>
#include "voicecallengine.h"
#include "../datamodel/voicecall.h" // for ConnectionObserver
#include "../log.h"
#include "../mcontroller.h"
#include "node.h"
#include "connection.h"
#include "protocol_message_formatter.h" // used here directly 
#include "../datamodel/contentencryptionmodel.h" // for ConnectionObserver
#include "../call/audiomixer.h"
#include "../call/audioplayer.h"
#include "../call/audiosource.h"
#include "../call/audioencoder.h"
#include "../call/ringtoneplayer.h"
#include <assert.h>
#include "../datamodel/profilemodel.h" // for getting operator profile data
#include "../datamodel/nodemodel.h" // for getting nodes of operators
#include "../datamodel/profile.h"
#include "../datamodel/trusttreemodel.h"

VoiceCallEngine::VoiceCallEngine(MController& aController,
                                 Model& aModel) :
    QAbstractTableModel(NULL),
    iController(aController),
    iModel(aModel),
    iMixer(NULL),
    iEncoder(NULL),
    iAudioSource(NULL),
    iAudioPlayer(NULL)  {
    connect (this, SIGNAL(startProcessCallData()),
             this, SLOT(processCallData()),
             Qt::QueuedConnection) ;
}

VoiceCallEngine::~VoiceCallEngine() {
    delete iAudioSource ;
    delete iMixer ;
    delete iEncoder ;
    delete iAudioPlayer ;
}


void VoiceCallEngine::installObserver(MCallStatusObserver* aObserver) {
    if ( !iCallObservers.contains(aObserver) ) {
        iCallObservers.append(aObserver) ;
    }
}

void VoiceCallEngine::removeObserver(MCallStatusObserver* aObserver) {
    iCallObservers.removeAll(aObserver) ;
}
//
// this method is called from protocol part so that datamodel is
// locked so no need to lock again
//
void VoiceCallEngine::insertCallData(quint32 aCallId,
                                     quint32 aSeqNo,
                                     VoiceCallEngine::PayloadType aPayloadType,
                                     const QByteArray& aPayload,
                                     const Hash& aSendingNode )  {
    switch ( aPayloadType ) {
    case VoiceCallEngine::Audio: {
        QLOG_STR("lock " + QString(__FILE__) + " "+ QString::number(__LINE__));
        iModel.lock() ;
        // here process audio data
        for ( int i = iOnGoingCalls.size() -1 ; i >= 0 ; i-- ) {
            const VoiceCallEngine::VoiceCallExtension& call ( iOnGoingCalls.at(i) ) ;
            Hash peerHash ( callPeerFingerPrint(call ) ) ;
            if ( call.iCallId == aCallId &&
                    call.iDecoder &&
                    peerHash == aSendingNode ) {
                call.iDecoder->frameReceived(aCallId,
                                             aSeqNo,
                                             aPayload) ;
                break ; // out of the loop
            }
        }
        iModel.unlock() ;
        QLOG_STR("unlock " + QString(__FILE__) + " "+ QString::number(__LINE__));
    }
    break ;
    case Control:
        // call control data ; maybe informational messages
        break ;
    default:
        break ;
    }
}

// when this is called, iModel.lock() is on, so here it
// is not necessary to lock again
void VoiceCallEngine::insertCallStatusData(const VoiceCall& aCallStatus,
        const Hash& aSendingNode
                                          ) {
    QPair<VoiceCall, Hash> pendingEntry ( aCallStatus, aSendingNode ) ;
    iCallDataPendingProcessing.append(pendingEntry) ;
    QLOG_STR("VoiceCallEngine::insertCallStatusData " + QString::number(aCallStatus.iCallId) + " nick " +
             pendingEntry.first.iPeerOperatorNick) ;
    emit startProcessCallData() ;
    return ;
}

void VoiceCallEngine::processCallData() {
    QLOG_STR("lock " + QString(__FILE__) + " "+ QString::number(__LINE__));
    iModel.lock() ;
    while (iCallDataPendingProcessing.size() ) {
        QPair<VoiceCall, Hash> queueEntry ( iCallDataPendingProcessing.takeAt(0)) ;
        const VoiceCall& callStatus ( queueEntry.first ) ;
        Hash& sendingNode ( queueEntry.second ) ;
        bool previousCallFound(false) ;
        QLOG_STR("VoiceCallEngine::processCallData " + QString::number(callStatus.iCallId)) ;
        for ( int i = iOnGoingCalls.size() -1 ; i >= 0 ; i-- ) {
            VoiceCallEngine::VoiceCallExtension& call ( iOnGoingCalls[i] ) ;
            if ( call.iCallId == callStatus.iCallId ) {
                previousCallFound = true ;

                if ( checkForNodeValidity(call,
                                          sendingNode) ) {
                    QLOG_STR("callStatus.iOkToProceed = " +
                             QString::number(callStatus.iOkToProceed)) ;
                    if ( callStatus.iOkToProceed == false ) {
                        call.iOkToProceed = false ;
                        call.iOnGoingCallState = Closed ;
                        sendCallStatusUpdateToRemote(call) ;
                        removeCallFromMixer(call) ;
                        QLOG_STR("call removed from mixer") ;
                        removeCallFromArray(call) ;
                        QLOG_STR("call removed from array") ;
                        sendCallStatusUpdates(call.iCallId, call.iOnGoingCallState) ;
                    } else {
                        // it is ok to continue.
                        if ( call.iOnGoingCallState == Initial ) {
                            // was our outgoing call that was "answered"
                            if ( callStatus.iSymmetricAESKey.size() > 0 ) {
                                call.iOnGoingCallState = Open ;
                                sendCallStatusUpdates(call.iCallId, call.iOnGoingCallState) ;
                                // 3 and 5 are the columns
                                emit dataChanged(createIndex(i,3),createIndex(i,5)) ;
                                setupLocalAudioCapture(call) ;
                                setupAudioOutput(call) ;
                                addCallToMixer(call) ;
                            }
                        }
                    }
                }
                break ;
            }
        }

        if ( previousCallFound == false &&
                callStatus.iOkToProceed) {
            QLOG_STR("Previous call not found") ;
            // ok ,we do not have call going on, so start as new
            VoiceCallExtension newCall (callStatus);
            newCall.iPeerOperatorNick = callStatus.iPeerOperatorNick ;
            if ( checkForNodeValidity(callStatus,
                                      sendingNode) ) {
                if ( newCall.iOriginatingNode ==
                        iController.getNode().nodeFingerPrint() ) {
                    setupNewOutgoingCall(newCall) ;
                } else {
                    // this is incoming call
                    Model::CallAcceptanceSetting acceptance ( iModel.getCallAcceptanceSetting() ) ;
                    Hash operatorHash ( iController.model().contentEncryptionModel().
                                        hashOfPublicKey(newCall.iOriginatingOperatorKey) ) ;
                    if ( acceptance == Model::AcceptNoCalls ||
                            ( acceptance == Model::AcceptCallsFromTrusted &&
                              iModel.trustTreeModel()->isProfileTrusted(operatorHash,NULL,NULL) == false )) {
                        newCall.iOkToProceed = false ;
                        sendCallStatusUpdateToRemote(newCall) ;
                        QLOG_STR("Silentely rejected call because of setting");
                    } else {
                        setupNewIncomingCall(newCall) ;
                    }
                }
            }
        }
    }
    iModel.unlock() ;
    QLOG_STR("unlock " + QString(__FILE__) + " "+ QString::number(__LINE__));
}

bool VoiceCallEngine::addCallToMixer(const VoiceCallExtension& aCall) {
    if ( iMixer && iAudioSource ) {
        if ( rowCount() == 1 ) {
            // if this is our first call, insert our
            // local capture as one stream into the mixer
            iMixer->insertStream(aCall.iCallId,
                                 iAudioSource->getCurrentSeqNo(),
                                 true ) ;
        }
        // always add the remote stream into the mixer
        iMixer->insertStream(aCall.iCallId,
                             0,
                             false ) ;
    }
    return true ;
}

bool VoiceCallEngine::removeCallFromMixer(const VoiceCallExtension& aCall) {
    if ( iMixer && iAudioSource ) {
        iMixer->removeStream(aCall.iCallId) ;
    }
    return true ;
}

bool VoiceCallEngine::setupNewOutgoingCall(VoiceCallExtension& aCall) {
    QLOG_STR("VoiceCallEngine::setupNewOutgoingCall nick = " +
             aCall.iPeerOperatorNick ) ;

    // check that we're not having a call to node already:
    foreach ( const VoiceCallExtension& call, iOnGoingCalls ) {
        const Hash peerHash ( callPeerFingerPrint(call) ) ;
        if ( peerHash == callPeerFingerPrint(aCall) ) {
            // already connected:
            QLOG_STR("Call was already in progress") ;
            return false ;
        }
    }

    if ( aCall.iOriginatingNode ==
            iController.getNode().nodeFingerPrint() ) {
        // stupid check but try call setup only if the call
        // is originated by this same node.
        aCall.iOnGoingCallState = Initial ;
        addCallToArray(aCall) ;
        sendCallStatusUpdates(aCall.iCallId, aCall.iOnGoingCallState) ;

        // send request to remote node:
        if ( iModel.nodeModel().isNodeAlreadyConnected(aCall.iDestinationNode) ) {
            // is connected, send:
            QByteArray* request = new QByteArray() ;
            request->append(ProtocolMessageFormatter::voiceCall(aCall,
                            iController,
                            iController.profileInUse(),
                            true)) ;
            iModel.addItemToSend(aCall.iDestinationNode,
                                 request) ;
        } else {
            // node is not connected, use network request mechanism
        }
    }
    return true ;
}

bool VoiceCallEngine::setupNewIncomingCall(VoiceCallExtension& aCall) {
    QLOG_STR("VoiceCallEngine::setupNewIncomingCall")  ;

    // check that there is operator operating the node:
    if ( iController.profileInUse() == KNullHash ) {
        aCall.iOkToProceed = false ;
        sendCallStatusUpdateToRemote(aCall,false) ;
        return false ;
    } else {
        aCall.iOnGoingCallState = Incoming ;
        addCallToArray(aCall) ;
        sendCallStatusUpdates(aCall.iCallId, aCall.iOnGoingCallState) ;
        // if it is also the first call, put a ringtone playing:
        // the player is "fire and forget" type, it will take care
        // of itself, according to call status
        if ( iOnGoingCalls.size() == 1 ) {
            new RingtonePlayer(iModel,iController) ;
        }
        return true ;
    }
}

QList<quint32> VoiceCallEngine::onGoingCalls() const {
    QList<quint32> retval ;

    foreach ( const VoiceCallExtension& call, iOnGoingCalls ) {
        retval.append(call.iCallId) ;
    }

    return retval ;
}


VoiceCallEngine::CallState VoiceCallEngine::callStatus(quint32 aCallId) const {

    foreach ( const VoiceCallExtension& call, iOnGoingCalls ) {
        if ( call.iCallId == aCallId ) {
            return call.iOnGoingCallState ;
        }
    }
    return VoiceCallEngine::Error ;
}

void VoiceCallEngine::closeCall(quint32 aCallId) {
    QLOG_STR("VoiceCallEngine::closeCall " + QString::number(aCallId)) ;
    QLOG_STR("lock " + QString(__FILE__) + " "+ QString::number(__LINE__));
    iModel.lock() ;
    for ( int i = iOnGoingCalls.size() -1 ; i >= 0 ; i-- ) {
        VoiceCallEngine::VoiceCallExtension& call ( iOnGoingCalls[i] ) ;
        if ( call.iCallId == aCallId ) {
            call.iOkToProceed = false ;
            sendCallStatusUpdateToRemote(call) ;
            removeCallFromMixer(call) ;
            removeCallFromArray(call) ;
            sendCallStatusUpdates(aCallId, Closed) ;
        }
    }
    iModel.unlock() ;
    QLOG_STR("unlock " + QString(__FILE__) + " "+ QString::number(__LINE__));
}

void VoiceCallEngine::sendCallStatusUpdateToRemote(const VoiceCall& aCall,
        bool aDoSign ) {
    const Hash peerHash ( callPeerFingerPrint(aCall ) ) ;

    if ( iModel.nodeModel().isNodeAlreadyConnected(peerHash) ) {
        // is connected, send:
        QByteArray* request = new QByteArray() ;
        request->append(ProtocolMessageFormatter::voiceCall(aCall,
                        iController,
                        iController.profileInUse(),
                        aDoSign)) ;
        iModel.addItemToSend(peerHash,
                             request) ;
    }
}

void VoiceCallEngine::acceptCall(quint32 aCallId) {
    QLOG_STR("VoiceCallEngine::acceptCall " + QString::number(aCallId)) ;
    QLOG_STR("lock " + QString(__FILE__) + " "+ QString::number(__LINE__));
    iModel.lock()  ;
    for ( int i = iOnGoingCalls.size() -1 ; i >= 0 ; i-- ) {
        VoiceCallEngine::VoiceCallExtension& call ( iOnGoingCalls[i] ) ;
        if ( call.iCallId == aCallId &&
                call.iOnGoingCallState == Incoming ) {
            call.iOkToProceed = true ;
            if ( call.iSymmetricAESKey.size() == 0 ) {
                const int KNumberOfBits ( 256 ) ;
                call.iSymmetricAESKey.append(iController.model()
                                             .contentEncryptionModel()
                                             .randomBytes(KNumberOfBits/8)) ;
            }
            call.iOnGoingCallState = Open ;
            sendCallStatusUpdates(aCallId, Open) ;
            sendCallStatusUpdateToRemote(call) ;
            // 3 and 5 are the columns
            emit dataChanged(createIndex(i,3),createIndex(i,5)) ;
            setupLocalAudioCapture(call) ;
            setupAudioOutput(call) ;
            addCallToMixer(call) ;
        }
    }
    iModel.unlock() ;
    QLOG_STR("unlock " + QString(__FILE__) + " "+ QString::number(__LINE__));
}

void VoiceCallEngine::sendCallStatusUpdates(quint32 aCallId,
        CallState aState)  {
    foreach ( MCallStatusObserver* observer, iCallObservers ) {
        observer->callStatusChanged(aCallId,aState) ;
    }
    emit callStateChanged(aCallId,aState) ;
}

void VoiceCallEngine::nodeConnectionAttemptStatus(Connection::ConnectionState aStatus,
        const Hash aHashOfAttemptedNode ) {
    QLOG_STR("lock " + QString(__FILE__) + " "+ QString::number(__LINE__));
    iModel.lock()  ;
    // check connectivity again because sometimes there are double
    // connections, one connects, another fails but is reported here
    // anyway
    if ( iModel.nodeModel().isNodeAlreadyConnected(aHashOfAttemptedNode) == false ) {
        for ( int i = iOnGoingCalls.size() -1 ; i >= 0 ; i-- ) {
            VoiceCallEngine::VoiceCallExtension& call ( iOnGoingCalls[i] ) ;
            Hash callNodeAddr ( callPeerFingerPrint(call) ) ;
            if ( callNodeAddr == aHashOfAttemptedNode &&
                    ( aStatus == Connection::Closing ||
                      aStatus == Connection::Error ) ) {
                // put call status change into queue and emit
                // a queued signal:
                call.iOkToProceed = false ;
                QPair<VoiceCall, Hash> pendingEntry ( call, aHashOfAttemptedNode ) ;
                iCallDataPendingProcessing.append(pendingEntry) ;
                emit startProcessCallData() ;
            }
        }
    }
    iModel.unlock() ;
    QLOG_STR("unlock " + QString(__FILE__) + " "+ QString::number(__LINE__));
}

Hash VoiceCallEngine::callPeerFingerPrint(const VoiceCall& aCall) const {
    Hash retval ( KNullHash ) ;
    if (  aCall.iOriginatingNode ==
            iController.getNode().nodeFingerPrint() ) {
        // we originated this, so lets return the dest addr
        retval = aCall.iDestinationNode ;

    } else {
        retval = aCall.iOriginatingNode ;
    }
    return retval ;
}

// note how aCall is reference to same array where we're going
// to remove the call from: the actual removal must be very
// last thing to do
void VoiceCallEngine::removeCallFromArray(VoiceCallExtension& aCall) {
    int indexToBeRemoved = -1 ;
    for ( int i = iOnGoingCalls.size() -1 ; i >= 0 ; i-- ) {
        VoiceCallEngine::VoiceCallExtension& call ( iOnGoingCalls[i] ) ;
        if ( aCall.iCallId == call.iCallId ) {
            indexToBeRemoved = i ;
        }
    }

    if ( indexToBeRemoved > -1 ) {
        QLOG_STR("removeCallFromArray array size = " +
                 QString::number(iOnGoingCalls.size()) +
                 " now removed " +
                 QString::number(aCall.iCallId) ) ;
        if ( iOnGoingCalls.size() == 1  ) { // one left, will be removed
            // got no ongoing calls left, lets tear audio down
            setupLocalAudioCapture(aCall) ;
            setupAudioOutput(aCall) ;
        }
        beginRemoveRows(QModelIndex(), indexToBeRemoved, indexToBeRemoved ) ;
        iOnGoingCalls.removeAt(indexToBeRemoved) ;
        endRemoveRows() ;
    }
}

void VoiceCallEngine::addCallToArray(VoiceCallExtension& aCall) {
    bool found ( false ) ;
    for ( int i = iOnGoingCalls.size() -1 ; i >= 0 ; i-- ) {
        VoiceCallEngine::VoiceCallExtension& call ( iOnGoingCalls[i] ) ;
        if ( aCall.iCallId == call.iCallId ) {
            found = true ;
            break ;
        }
    }
    if ( !found ) {
        Hash operatorHash ( iController.model().contentEncryptionModel().
                            hashOfPublicKey(aCall.iOriginatingOperatorKey) ) ;
        aCall.iPeerOperatorHash = operatorHash.toString() ;
        if ( aCall.iPeerOperatorHash == iController.profileInUse().toString() ) {
            // this is call originated by me and the peer name is not known
            if ( aCall.iPeerOperatorNick.length() == 0 ) {
                aCall.iPeerOperatorNick = "-" ;
            }
        } else {
            aCall.iPeerOperatorNick =
                iController.displayableNameForProfile(operatorHash);
        }
        beginInsertRows(QModelIndex(), iOnGoingCalls.size(), iOnGoingCalls.size() ) ;
        iOnGoingCalls.append(aCall) ;
        endInsertRows() ;
    }
}

// methods overridden from QAbstractTableModel:
int VoiceCallEngine::rowCount(const QModelIndex & /*parent*/ )  const {
    return iOnGoingCalls.size() ;
}

int VoiceCallEngine::columnCount(const QModelIndex & /* parent */ )  const {
    // let columns be
    // 1. peer operator
    // 2. peer network addr
    // 3. call status
    // 4. accept/reject -button
    // 5. close-button
    return 5 ;
}

QVariant VoiceCallEngine::data(const QModelIndex & index,
                               int role ) const {
    if(!index.isValid())
        return QVariant();

    if ( role == Qt::UserRole ) {

        return QVariant(iOnGoingCalls.at(index.row()).iCallId) ;
    }
    if ( role == Qt::EditRole ) {
        QLOG_STR("Qt::EditRole at call status " +
                 QString::number(index.row())) ;
        switch ( index.column() ) {
        case 0:
        case 1:
        case 2:
        case 3:
        case 4:
            return QVariant() ;
        }
    } else if ( role == Qt::DisplayRole ) {
        switch ( index.column() ) {
        case 0:
            // 1. peer operator
            return iOnGoingCalls.at(index.row()).iPeerOperatorNick ;
        case 1:
            // 2. peer network addr
            return iOnGoingCalls.at(index.row()).iPeerNodeAddress ;
        case 2:
            // 3. call status
            return callStatusString(iOnGoingCalls.at(index.row())) ;
        case 3:
            // 4. accept/reject -button
            if ( iOnGoingCalls.at(index.row()).iOnGoingCallState ==
                    Incoming ) {
                return tr("Accept") ;
            } else {
                return QVariant() ;
            }
        case 4:
            // 5. close-button
            if ( iOnGoingCalls.at(index.row()).iOnGoingCallState ==
                    Incoming ) {
                return tr("Reject") ;
            } else {
                return tr("Close") ;
            }
            break ;
        }
    }
    return QVariant() ;
}


QVariant VoiceCallEngine::headerData ( int section,
                                       Qt::Orientation /* orientation */,
                                       int role  ) const {
    switch ( role ) {
    case Qt::ToolTipRole:
        switch ( section ) {
        case 0:
            return tr("Actual network address (SHA1) of the peer operator") ;
            break;
        case 1:
            return tr("Network address of the remote node") ;
            break;
        default:
            return QVariant();
            break ;
        }
        break ;
    case Qt::DisplayRole:
        switch ( section ) {
        case 0:
            return tr("Operator") ;
            break;
        case 1:
            return tr("Node address") ;
            break ;
        case 2:
            return tr("Call status") ;
            break ;
        case 4:
            return tr("Controls") ;
            break ;
        case 5:
            return tr("Ending") ;
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
            return QSize(200,25) ;
            break ;
        case 2:
            return QSize(50,25) ;
            break ;
        case 3:
            return QSize(30,25) ;
            break ;
        case 4:
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

QString VoiceCallEngine::callStatusString(const VoiceCallExtension& aCall) const {
    QString retval ;
    switch(aCall.iOnGoingCallState) {
    case Initial:
        retval = tr("Initializing") ;
        break ;
    case Incoming:
        retval = tr("Incoming") ;
        break ;
    case Open:
        retval = tr("Open") ;
        break ;
    case Closing:
        retval = tr("Closing") ;
        break ;
    case Closed:
        retval = tr("Closed") ;
        break ;
    case Error:
        retval = tr("Error") ;
        break ;
    case NoCall:
        retval = tr("No call") ;
        break ;
    }
    return retval ;
}

bool VoiceCallEngine::checkForNodeValidity(const VoiceCall& aCallStatus,
        const Hash& aSendingNode) const {
    if (
        iController.getNode().nodeFingerPrint() == aCallStatus.iOriginatingNode &&
        iController.getNode().nodeFingerPrint() == aSendingNode ) {
        // accept everything from this same node
        return true ;
    } else if (
        (
            aSendingNode == aCallStatus.iOriginatingNode )
        ||
        (
            aCallStatus.iDestinationNode == aSendingNode )
    ) {
        // other possiblity is that aSendingNode is either
        // destination or the originator, we need to accept both
        return true ;
    } else {
        QLOG_STR("Received call status DTO from impossible node") ;
        return false ;
    }
}


bool VoiceCallEngine::setupLocalAudioCapture(VoiceCallExtension& aCall) {
    if( aCall.iOkToProceed ) {
        QLOG_STR("Setting up local audio capture for  " +
                 QString::number(aCall.iCallId) ) ;
        if ( iMixer == NULL ) {
            iMixer = new AudioMixer(iModel) ;
        }
        if ( iEncoder == NULL ) {
            iEncoder = new AudioEncoder() ;
            assert (
                connect ( iMixer, SIGNAL(frameReadyForRemoteSend(quint32,quint32,const QByteArray&)),
                          iEncoder, SLOT(frameReady(quint32,quint32,const QByteArray&))));
            assert (
                connect ( iEncoder, SIGNAL(frameEncoded(quint32,quint32,const QByteArray&)),
                          this, SLOT(audioFrameEncoded(quint32,quint32,const QByteArray&)))) ;
        }
        if ( iAudioSource == NULL ) {
            iAudioSource = new AudioSource(iModel) ;
            assert (
                connect ( iAudioSource, SIGNAL(frameCaptured(const QByteArray&,quint32)),
                          iMixer, SLOT(insertCapturedAudioFrame(const QByteArray&, quint32)))) ;
            connect(iAudioSource, SIGNAL(audioMaxLevel(float)),
                    this, SLOT(setInputLevel(float))) ;

        }
    } else {
        QLOG_STR("Tearing down local audio capture for  " +
                 QString::number(aCall.iCallId) ) ;
        // ending call
        if ( iAudioSource ) {
            iAudioSource->stopCapturing() ; // will delete itself after stop
            iAudioSource = NULL ;
        }
        QLOG_STR("Tearing down local audio capture: audiosource gone" ) ;
        // ending call
        if ( iEncoder ) {
            iEncoder->deleteLater() ;
            iEncoder = NULL ;
        }
        QLOG_STR("Tearing down local audio capture: encoder gone" ) ;
        if ( aCall.iDecoder ) {
            aCall.iDecoder->deleteLater() ;
            aCall.iDecoder = NULL ;
        }
        QLOG_STR("Tearing down local audio capture: decoder gone" ) ;
    }
    QLOG_STR("local audio stopped for call " +
             QString::number(aCall.iCallId) ) ;
    return true ;
}

bool VoiceCallEngine::setupAudioOutput(VoiceCallExtension& aCall) {
    if( aCall.iOkToProceed ) {
        if ( iMixer == NULL ) {
            iMixer = new AudioMixer(iModel) ;
        }
        if ( aCall.iDecoder == NULL ) {
            aCall.iDecoder = new AudioDecoder ( ) ;
            assert (
                connect(aCall.iDecoder, SIGNAL(frameDecoded(quint32,quint32,const QByteArray&)),
                        iMixer,SLOT(insertReceivedAudioFrame ( quint32 ,
                                    quint32 ,
                                    const QByteArray&) ) )) ;
        }
        if ( iAudioPlayer == NULL ) {
            iAudioPlayer = new AudioPlayer(iModel) ;
            assert (
                connect(iMixer,SIGNAL(frameReadyForLocalSpeaker(const QByteArray&)),
                        iAudioPlayer,SLOT(insertAudioFrame(const QByteArray&)))) ;
            connect(iAudioPlayer, SIGNAL(audioMaxLevel(float)),
                    this, SLOT(setOutputLevel(float))) ;
        }
    } else {
        // ending call
        if ( iAudioPlayer ) {
            iAudioPlayer->stop() ; // will delete itself after stopped
            iAudioPlayer = NULL ;
        }
        if ( aCall.iDecoder ) {
            delete aCall.iDecoder ;
            aCall.iDecoder = NULL ;
        }
    }
    return true ;
}

// this is a synchronously connected slot and it is called so
// that datamodel lock is on.
void VoiceCallEngine::audioFrameEncoded ( quint32 
#ifdef DEBUG
                                          aCallId
#endif
                                          ,
                                          quint32 aSeqNo,
                                          const QByteArray& aEncodedVoice ) {

    bool frameSent ( false ) ;
    if ( aEncodedVoice.size() ) {
        foreach ( const VoiceCallExtension& call, iOnGoingCalls ) {
            const Hash peerHash ( callPeerFingerPrint(call ) ) ;
            QByteArray* bytesToSendToThisNode = new QByteArray() ;
            bytesToSendToThisNode->append( ProtocolMessageFormatter::voiceCallRtData( call.iCallId, aSeqNo, VoiceCallEngine::Audio,  aEncodedVoice ) ) ;
            iModel.addItemToSend(peerHash,
                                 bytesToSendToThisNode) ;
            // model takes ownership of bytesToSendToThisNode, it
            // will be free'd after it has been sent
            frameSent = true ;
        }
    }
    if ( frameSent == false ) {
        QLOG_STR("Frame was not sent to any remote node?? callid = " +
                 QString::number(aCallId)) ;
    }
    return ;
}


void VoiceCallEngine::setInputLevel(float aInputLevel) {
    // just re-send the signal, UI does not know the real origin
    emit inputLevel(aInputLevel) ;
}

void VoiceCallEngine::setOutputLevel(float aOutputLevel) {
    // just re-send the signal, UI does not know the real origin
    emit outputLevel(aOutputLevel) ;
}

QString VoiceCallEngine::excuseForCallCreation(const Hash& aOperator,
        const Hash& aNode  ) const {
    QString retval ( tr("Audio call is not possible") ) ;

    Hash nodeHashToUse ;

    if ( aNode != KNullHash &&
            iModel.nodeModel().isNodeAlreadyConnected(aNode) ) {
        retval = tr("Audio call is possible") ;
    } else {
        // excuses follow
        if ( aNode != KNullHash ) {
            nodeHashToUse = aNode ;
        } else {
            if ( aOperator == KNullHash ) {
                retval = tr("Audio call is not possible: operator address unknown") ;
            } else {
                // fetch profile
                iModel.lock()  ;
                Profile* p = iModel.profileModel().profileByFingerPrint(aOperator,false,true) ;
                if ( p == NULL ) {
                    retval = tr("Audio call is not possible: operator profile not found") ;
                } else {
                    if ( p->iNodeOfProfile == NULL ) {
                        retval = tr("Audio call is not possible: operator has no node information in profile") ;
                    } else {
                        nodeHashToUse = p->iNodeOfProfile->nodeFingerPrint() ;
                    }
                    delete p ;
                }
                iModel.unlock()  ;
            }
        }
    }

    if ( nodeHashToUse != KNullHash ) {
        iModel.lock()  ;
        Node* operatorsNode ( iModel.nodeModel().nodeByHash(nodeHashToUse) ) ;
        const bool hasIpv6 = // this means "we have", not the operator
            !Connection::Ipv6AddressesEqual(iController.getNode().ipv6Addr(),
                                            KNullIpv6Addr) ;
        iModel.unlock()  ;

        QString ipv4excuse = tr("Audio call is not possible: operators IPv4 address %1 not connected") ;
        if ( operatorsNode ) {
            const bool operatorHasIpv6 =
                !Connection::Ipv6AddressesEqual(operatorsNode->ipv6Addr(),
                                                KNullIpv6Addr) ;
            if ( hasIpv6 ) {
                if ( operatorHasIpv6 ) {
                    retval = QString(tr("Audio call is not possible: operators IPv6 address %1 not connected")).arg(QHostAddress(operatorsNode->ipv6Addr()).toString()) ;
                } else {
                    // operator operates ipv4 node
                    retval = QString(ipv4excuse).arg(QHostAddress(operatorsNode->ipv4Addr()).toString()) ;
                }
            } else {
                if ( operatorsNode->ipv4Addr() ) {
                    // operator has ipv4 addr
                    retval = QString(ipv4excuse).arg(QHostAddress(operatorsNode->ipv4Addr()).toString()) ;
                } else {
                    if ( operatorHasIpv6 ) {
                        retval = tr("Audio call is not possible: local node has no IPv6 address, operator has only IPv6 addr") ;
                    } else {
                        retval = tr("Audio call is not possible: operator does not publish network address") ;
                    }
                }
            }
            delete operatorsNode ;
        }
    }

    return retval ;
}
