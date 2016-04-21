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

#ifndef VOICECALL_ENG_H
#define VOICECALL_ENG_H
#include <QString>
#include <QList>
#include <QAbstractTableModel>
#include <QPair>
#include "../datamodel/model.h"
#include "../datamodel/voicecall.h"
#include "../call/audiodecoder.h"
#include "../call/audioencoder.h"
#include "mvoicecallengine.h"

class AudioMixer ;
class AudioEncoder ;
class AudioSource ;
class AudioPlayer ;

/**
 * @brief Network-related logic for handling voice channel between nodes
 *
 * "Voice call" is a bit different from persisted content handling:
 * It happens between, nodes, not between operators. It has realtime-
 * requirements and does not directly store anything on data
 * storage.
 *
 * This class contains networking-related logic and audio stream
 * handling. Separate UI must be built on top of this. This inherits
 * QAbstractTableModel to provide easy access for UI-components
 * to calldata.
 *
 * For actual call data handling see @ref AudioMixer and other classes
 * in the same directory.
 */
class VoiceCallEngine : public QAbstractTableModel,
    public MVoiceCallEngine  {
    Q_OBJECT
public:

    /**
     * Constructor
     * @param aController application controller. not owned
     * @param aModel persistent storage.
     */
    VoiceCallEngine(MController& aController,
                    Model& aModel ) ;
    /**
     * Destructor
     */
    ~VoiceCallEngine() ;

    /**
     * Method for installing call state observer
     */
    virtual void installObserver(MCallStatusObserver* aObserver) ;

    /**
     * Method for removing call state observer
     */
    virtual void removeObserver(MCallStatusObserver* aObserver) ;

    /**
     * Method for reception of call real-time data. This is called
     * from network-parts that receive the data packet over some
     * protocol and then feed it here to make it heard.
     *
     * Called from protocol parser. Parser does not lock datamodel
     * prior to call.
     *
     * @param aCallId is call ( stream ) identifier
     * @param aSeqNo is sequence number of rt data in stream
     * @param aPayloadType tells what kind of payload it is
     * @param aPayload actual bytes
     * @param aSendingNode node what sent the data package
     */
    virtual void insertCallData(quint32 aCallId,
                                quint32 aSeqNo,
                                PayloadType aPayloadType,
                                const QByteArray& aPayload,
                                Hash& aSendingNode) ;

    /**
     * Method for reception of call status data. This is called
     * from network-parts that receive the data packet over some
     * protocol and then feed it here to move the call engine
     * to some direction
     *
     * Note that the call data inserted via this method may be
     * call that this node originally made and at the receiving
     * end this same method is used to inject the new incoming
     * call into the engine.
     *
     * Another note to take into consideration is the handling
     * if node-data in aCallStatus. Call status is passed as reference
     * so the ownership of the call data object instance is not
     * tranferred into voice call engine. Inside call data there
     * are 2 pointers to originating and destination nodes.
     * Ownership of content of those pointers is not tranferred
     * either. VoiceCallEngine will make local copies (if it sees
     * that necessary) of the node-pointer contents and caller of
     * this method will be responsible for deleting the node-objects
     * in call-data if they were created only for the purpose of
     * making a voice call.
     *
     * DataModel.lock() should be called before calling this method
     *
     * @param aCallStatus contains information about a call.
     * @param aSendingNode is fingerprint of the node where the DTO is
     *        coming from. May be local node also.
     */
    virtual void insertCallStatusData(const VoiceCall& aCallStatus,
                                      const Hash& aSendingNode) ;
    /**
     * method for getting identifiers of ongoing calls
     */
    virtual QList<quint32> onGoingCalls() const ;
    /**
     * method for getting status of a call
     */
    virtual CallState callStatus(quint32 aCallId) const ;
    /**
     * Method called as result UI-action. This terminates ongoing call.
     */
    virtual void closeCall(quint32 aCallId) ;
    /**
     * Method called as result UI-action. This accepts an incoming call.
     */
    virtual void acceptCall(quint32 aCallId) ;
    /**
     * Method for querying for an excuse why a call can (not) be
     * made to particular operator or node
     *
     * @param aOperator identifier of operator whose call-creation
     *        status is queried.
     * @param aNode node of the operator, if known
     *
     * @return String telling if voice call can be made
     */
    QString excuseForCallCreation(const Hash& aOperator,
                                  const Hash& aNode = KNullHash ) const ;
    /**
     * Methods inherited from QAbstractTableModel: rowCount
     * @return number of ongoing calls
     */
    virtual int rowCount(const QModelIndex & parent = QModelIndex())  const  ;
    /**
     * re-implemented from QAbstractTableModel
     * @return number of columns in view
     */
    virtual int columnCount(const QModelIndex & parent = QModelIndex())  const  ;
    /**
     * re-implemented from QAbstractListModel
     * @return data to display in list of ongoing calls
     */
    virtual QVariant data(const QModelIndex & index,
                          int role = Qt::DisplayRole) const ;
    /**
     * re-implemented from QAbstractListModel
     * @return strings for headers of the call list UI
     */
    virtual QVariant headerData ( int section,
                                  Qt::Orientation orientation,
                                  int role = Qt::DisplayRole ) const ;

public slots:
    /**
     * This signal is about node-connection status and closed connections
     * will be communicated using this same signal:
     */
    void nodeConnectionAttemptStatus(Connection::ConnectionState aStatus,
                                     const Hash aHashOfAttemptedNode );

    /**
     * Voice data captured from microphone and mixed with other
     * possible streams is delivered to over-the-network recipients
     * using this slot. In practice input into this method comes
     * from audio encoder and this method is responsible for
     * distributing the encoded audio frame to all call participants.
     */
    void audioFrameEncoded ( quint32 aCallId,
                             quint32 aSeqNo,
                             const QByteArray& aEncodedVoice,
                             Hash aForNode ) ;

    /**
     * This slot is called when mixer when audio frame is ready to be sent.
     * In practice audio mixer emits a singal to this slot once for
     * each node that is connected to same call.
     *
     * @param aCallId is identifier of the call where this frame
     *        belongs to
     * @param aSeqNo sequence number of packets in stream
     * @param aRawAudio is non-encoded audio frame, as array of floats
     * @param aForNode specifies the node that will receive this
     *                 particular audio frame.
     */
    void frameReady ( quint32 aCallId,
                      quint32 aSeqNo,
                      const QByteArray& aRawAudio,
                      Hash aForNode ) ;

    /**
     * slot that is called for call status data processing. reason for
     * this slot is that we want to process status data in UI thread
     * because we create QObjects there and want to delete them also
     * in same thread, this easier to handle in Qt this way.
     *
     * In practice, the call data is put into iCallDataPendingProcessing
     * and this slot is called in queued manner
     */
    void processCallData() ;
    /**
     * slot for setting audio input (microphone) level
     */
    void setInputLevel(float aInputLevel) ;
    /**
     * slot for setting audio output (spekar) level
     */
    void setOutputLevel(float aOutputLevel) ;
signals:
    void error(QTcpSocket::SocketError socketError);
    /** Signal for communicating call state changes */
    void callStateChanged(quint32 aCallId,
                          VoiceCallEngine::CallState aState);
    /** signal for starting @ref processCallData */
    void startProcessCallData() ;
    /**
     * signal for communicating audio input (microphone) level
     */
    void inputLevel(float aInputLevel) ;
    /**
     * signal for communicating audio output (spekar) level
     */
    void outputLevel(float aOutputLevel) ;
private:
    class VoiceCallExtension ;
    // methods
    /**
     * Called when a new call is initiated from local node
     */
    bool setupNewOutgoingCall(VoiceCallExtension& aCall) ;
    /**
     * Called when a new call is initiated from remote node
     */
    bool setupNewIncomingCall(VoiceCallExtension& aCall) ;
    /**
     * call status gossip method
     */
    void sendCallStatusUpdates(quint32 aCallId,
                               CallState aState ) ;
    /**
     * Method that returns the call peer node address: either
     * destination or originating.
     */
    Hash callPeerFingerPrint(const VoiceCall& aCall) const ;
    void removeCallFromArray(VoiceCallExtension& aCall) ;
    void addCallToArray(VoiceCallExtension& aCall) ;
    void sendCallStatusUpdateToRemote(const VoiceCall& aCall,bool aDoSign=true) ;
    QString callStatusString(const VoiceCallExtension& aCall) const ;
    /**
     * Method that checks that aCallStatus can be from aSendingNode
     */
    bool checkForNodeValidity(const VoiceCall& aCallStatus,
                              const Hash& aSendingNode) const ;
    /**
     * Method for setting up local audio capture. If call has
     * @ref VoiceCall::iOkToProceed set to false, then capture
     * will be torn down.
     */
    bool setupLocalAudioCapture(VoiceCallExtension& aCall) ;
    /**
     * Method for setting up local audio output. If call has
     * @ref VoiceCall::iOkToProceed set to false, then output
     * will be torn down.
     */
    bool setupAudioOutput(VoiceCallExtension& aCall) ;
    /**
     * method for conditioanally adding voice streams to mixer module
     */
    bool addCallToMixer(const VoiceCallExtension& aCall) ;
    /**
     * method for conditioanally removing voice streams from mixer module
     */
    bool removeCallFromMixer(const VoiceCallExtension& aCall) ;
private: // data
    /**
     * Have private extension class for call data: extend the
     * datamodel DTO by call state
     */
    class VoiceCallExtension: public VoiceCall {
    public:
        /** constructor that is given a voicecall in */
        VoiceCallExtension(const VoiceCall& aInitialData) :
            iDecoder(NULL),
            iEncoder(NULL) {
            fromJSon(aInitialData.asJSon()) ;
        }
        ~VoiceCallExtension() {
            if ( iDecoder ) {
                iDecoder->deleteLater() ;
                iDecoder = NULL ;
            }
            if ( iEncoder ) {
                iEncoder->deleteLater() ;
                iEncoder = NULL ;
            }
        }
        /** Call state like "incoming", "open" */
        CallState iOnGoingCallState ;
        /** Who are we talking with */
        QString iPeerOperatorHash ;
        /** And in which network address is she */
        QString iPeerNodeAddress ;
        /**
         * Each node will send its own audio stream so
         * each ongoing individual call needs to have its
         * own audio decoder
         */
        AudioDecoder* iDecoder;
        /**
         * Each node may have different stream coming out from
         * mixer, this separate audio encoder for each
         * connection
         */
        AudioEncoder* iEncoder;
    } ;
    MController& iController ; /**< application controller */
    Model &iModel ; /**< persistent storage */
    QList<MCallStatusObserver*> iCallObservers ;
    QList<VoiceCallExtension> iOnGoingCalls ; /**< data about calls currently handled */
    /**
     * Exactly one audio mixer: has list of streams to mix. Idea is
     * that list of mixed streams does not need to be same as list
     * of ongoing calls. At least the local microphone input is
     * additional stream
     */
    AudioMixer* iMixer ;
    AudioSource* iAudioSource;
    AudioPlayer* iAudioPlayer;
    QList<QPair<VoiceCall, Hash> > iCallDataPendingProcessing ;
} ;
#endif
