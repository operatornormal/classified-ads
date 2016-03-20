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
#include <QObject>
#include <QList>
#include "mockup_voicecallengine.h"
#include "../log.h"
#include "../util/hash.h"
#include "../datamodel/voicecall.h"

MockUpVoiceCallEngine::MockUpVoiceCallEngine() :iCallId(0) {
    QLOG_STR("Voice call engine mockup constructed") ; 
}
MockUpVoiceCallEngine::~MockUpVoiceCallEngine() {
    QLOG_STR("Voice call engine mockup is no more") ; 
}


void MockUpVoiceCallEngine::installObserver(MCallStatusObserver* aObserver) {
    iObservers.append(aObserver) ; 
}


void MockUpVoiceCallEngine::removeObserver(MCallStatusObserver* aObserver) {
    iObservers.removeAll(aObserver) ; 
}


void MockUpVoiceCallEngine::insertCallData(quint32 aCallId,
                                           quint32 aSeqNo,
                                           PayloadType aPayloadType,
                                           const QByteArray& aPayload,
                                           const Hash& aSendingNode) {
    QLOG_STR("MockUpVoiceCallEngine::insertCallData " + 
             QString::number(aCallId )) ; 
    iCallIdOfReceivedRtData = aCallId ; 
    iCalldata.clear() ;
    iCalldata.append(aPayload) ; 
}

void MockUpVoiceCallEngine::insertCallStatusData(const VoiceCall& aCallStatus,
                                                 const Hash& aSendingNode) {
    QLOG_STR("MockUpVoiceCallEngine::insertCallStatusData " + 
             QString::number(aCallStatus.iCallId )) ; 
    iCallId = aCallStatus.iCallId ;
}
QList<quint32> MockUpVoiceCallEngine::onGoingCalls() const {
    QList<quint32> retval ; 
    if ( iCallId != 0 ) {
        retval.append(iCallId) ; 
    }
    return retval ; 
}

MVoiceCallEngine::CallState MockUpVoiceCallEngine::callStatus(quint32 aCallId) const {
    return MVoiceCallEngine::NoCall ;
}

void MockUpVoiceCallEngine::closeCall(quint32 aCallId) {
    iCallId = 0 ; 
}

void MockUpVoiceCallEngine::acceptCall(quint32 aCallId) {
    iCallId = aCallId ; 
}
