/*    -*-C++-*- -*-coding: utf-8-unix;-*-
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

#ifndef VOICECALL_H
#define VOICECALL_H
#include <QByteArray>
#include "../util/hash.h" // for class Hash  

class MController ;
class Profile ;
class Node ;

/**
 * @brief Carrier-class handling audio negotiation setup and data
 *
 * DTO object that typically is not stored but may be transferred
 * in network. This contains data related to real-time audio data
 * sent between nodes, either directly, over some protocol, or
 * via proxy host. Here "VoiceCall" means a audio link between 2
 * nodes. If there ever is some kind of "conference call" function,
 * it will include or encapsulate several instances of this class.
 */
class VoiceCall {
public:
    VoiceCall() ; /**< constructor */
    ~VoiceCall() ; /**< destructor */

    QByteArray asJSon() const ; /**< returns voicecall data as JSon stream */
    bool fromJSon(const QByteArray &aJSonBytes) ; /**< parses json into members*/

    /**
     * Call-id. This is identification number chosen by originating
     * party ; subsequent transactions will contain this number to
     * help in keeping track about what belongs where
     */
    quint32 iCallId ;
    /**
     * voice calls have direction at setup phase: one node initiates
     * call attempt that is destined to some other node. Here is
     * originating node
     */
    Hash iOriginatingNode ;
    /**
     * Call destination node
     */
    Hash iDestinationNode ;
    /**
     * Encryption key of the operator who is initiating the call. PEM.
     */
    QByteArray iOriginatingOperatorKey ;
    /**
     * Encryption key of the operator who answered the call. PEM.
     * This is empty when call is initiated and if call accepter 
     * of the call wants to retain her identity.
     */
    QByteArray iDestinationOperatorKey ;
    /**
     * Verdict of the call attempt from destination node: if the recipient
     * does not wish to receive the call, she will set this to false
     * and return this DTO. Initial value is naturally "true".
     */
    bool iOkToProceed ;
    /**
     * time of call attempt
     */
    time_t iTimeOfCallAttempt ;
    /**
     * Symmetric encryption key to use if sending call data over 
     * unsecure channel
     */
    QByteArray iSymmetricAESKey ;
 
    /** 
     * Who are we talking with, friendly name. Used for UI purposes,
     * not transferred over network
     */
    QString iPeerOperatorNick ; 
} ;
#endif
