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

#ifndef VOICECALL_ENG_INTERFACE_H
#define VOICECALL_ENG_INTERFACE_H
#include <QList>
class Hash ; 
class VoiceCall ; 

/**
 * @brief Pure-virtual interface for voice call engine
 *
 * This is interface to @ref VoiceCallEngine class. This has methods
 * that public is supposed to access to create and control calls. 
 */
class MVoiceCallEngine {
public:
    /**
     * Enum for different voice call states
     */
    enum CallState {
        Initial,  /**< Call initiated */
        Incoming, /**< Call received, acceptance by user expected */
        Open,     /**< Handshake done, audio open */
        Closing,  /**< Disconnection has been asked */
        Closed,   /**< "normal" final state, after this the call is no more */
        Error,    /**< Connection itself reports an error */
        NoCall    /**< Engine may report that there is no call */
    };

    /** 
     * Observer for call status for tracking changes in 
     * @ref VoiceCallEngine::CallState
     */
    class MCallStatusObserver {
    public:
        /**
         * Method that communicates changes in call state
         */
        virtual void callStatusChanged(quint32 aCallId,
                                       CallState aState) = 0 ; 
    } ;
    /**
     * Enum for different payload types inside voicecall RT-stream
     */
    enum PayloadType {
        Audio=1,  /**< Audio data */
        Control=2 /**< Call control data */
    } ; 
    /**
     * Data transfer object for call data
     */
    class CallData {
    public:
        quint32 iCallId ; /**< identifer for the stream */
        quint32 iSeqNo ; /**< running number for packets in stream */
        quint32 iTimeStamp ; /**< milliseconds since start of the stream */
        PayloadType iPayloadType ; /**< What is inside @ref iPayload */ 
        QByteArray iPayload ; /**< Actual data */
    } ; 
    /**
     * Method for installing call state observer
     */
    virtual void installObserver(MCallStatusObserver* aObserver) = 0 ; 

    /**
     * Method for removing call state observer
     */
    virtual void removeObserver(MCallStatusObserver* aObserver) = 0 ; 

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
                                const Hash& aSendingNode) = 0 ; 

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
                                      const Hash& aSendingNode) = 0 ;
    /**
     * method for getting identifiers of ongoing calls
     */
    virtual QList<quint32> onGoingCalls() const = 0 ; 
    /**
     * method for getting status of a call 
     */
    virtual CallState callStatus(quint32 aCallId) const = 0 ; 
    /**
     * Method called as result UI-action. This terminates ongoing call.
     */
    virtual void closeCall(quint32 aCallId) = 0 ; 
    /**
     * Method called as result UI-action. This accpets an incoming call. 
     */
    virtual void acceptCall(quint32 aCallId) = 0 ; 
} ; 
#endif
