/* -*-C++-*- -*-coding: utf-8-unix;-*-
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

#include "audiomixer.h"
#include "audiosource.h"
#include "../datamodel/model.h"
#include <QDebug>
#include <QQueue>
#include "../log.h"

/**
 * How many packets to keep in buffer before mixing starts occurring.
 * This is not used in 1-1 situation (when there is only single call
 * in progress and no need to actually mix) but in multicall situations
 * this is used.
 */
const int KPreferredBufferLen ( 5 ) ;
/**
 * If there is any queue with more packets than this, packets
 * start getting discarded. With 60ms frame size the queue len
 * 32 presents 2 full seconds.
 */
const int KMaxBufferLen ( 32 ) ;

AudioMixer::AudioMixer( Model& aModel) : iModel(aModel) {

}


AudioMixer::~AudioMixer() {

}

// when this is called,  datamodel is locked
void AudioMixer::insertCapturedAudioFrame ( const QByteArray& aFrame,
        quint32 aSeqNo,
        float aVolumeLevel ) {
    doInsertAudioFrame (  aFrame,
                          true,
                          aSeqNo,
                          0 ,
                          aVolumeLevel,
                          KNullHash );
}

// when this is called,  datamodel is locked
void AudioMixer::insertReceivedAudioFrame ( quint32 aCallId,
        quint32 aSeqNo,
        const QByteArray& aFrame,
        Hash aOriginatingNode ) {
    // audio frames coming from remote nodes have no average
    // volume level calculated so lets do it here:
    float* samples((float *)( aFrame.data() )) ;
    float value ( 0.0f ) ;
    float volumeSum ( 0.0f ) ;
    int numberOfSamples = aFrame.size() / sizeof(float) ;
    numberOfSamples-- ;
    for ( ; numberOfSamples >= 0 ; numberOfSamples-- ) {
        value = samples[numberOfSamples] ;
        // in float presentation the samples need to have
        // numerical value between [-1,1] so lets scale the
        // values ;
        volumeSum = volumeSum + qAbs( value ) ;
    }
    numberOfSamples = aFrame.size() / sizeof(float) ;
    float volumeLevel ( volumeSum/(float)numberOfSamples ) ;

    doInsertAudioFrame (  aFrame,
                          false,
                          aSeqNo,
                          aCallId,
                          volumeLevel,
                          aOriginatingNode );
}




void AudioMixer::doInsertAudioFrame ( const QByteArray& aFrame,
                                      bool aIsLocallyCaptured,
                                      quint32 aSeqNo,
                                      quint32 aCallId,
                                      float aVolumeLevel,
                                      Hash &aOriginatingNode ) {
    bool inserted ( false ) ;
    int indexFound ( -1 ) ;

    for ( int i = iStreams.size()-1 ; i >= 0 ; i-- ) {
        const AudioFrameMetadata& s ( iStreams[i] ) ;
        if ( s.iNode == aOriginatingNode ||
                ( ( s.iIsLocallyCaptured == aIsLocallyCaptured ) &&
                  aIsLocallyCaptured == true )) {
            indexFound = i ;
            if ( s.iIsLocallyCaptured == aIsLocallyCaptured ) {
                // locally captured audio comes in with no callId
                aCallId = s.iCallId ;
            }
            break ; // out of the loop
        }
    }
    if ( indexFound > -1 ) {
        if( iAudioDataBuffer.size() > indexFound ) {
            QQueue<AudioFrame>& buf ( iAudioDataBuffer[indexFound] ) ;
            if ( buf.size() < KMaxBufferLen ) { // if buffer is full
                AudioStreamMetadata& metaData ( iStreams[indexFound] ) ;
                AudioFrame f ;
                f.iCallId = aCallId  ;
                f.iSeqNo = aSeqNo ;
                f.iFrame.append ( aFrame ) ;
                f.iVolumeLevel = aVolumeLevel ;
                f.iIsLocallyCaptured = aIsLocallyCaptured ;
                if ( metaData.iMaxSeqSeen == 0 ||
                        ( metaData.iMaxSeqSeen + 1 ) == aSeqNo ) {
                    // easy, just enqueue normally
                    buf.enqueue(f) ;
                    if ( aIsLocallyCaptured == false ) {
                        metaData.iMaxSeqSeen = aSeqNo ; // only remote stream seqno
                    }
                    inserted = true ;
                } else {
                    // difficult. we got the packet in wrong order: try
                    // to put it into queue into right position
                    if ( buf.size() == 0 ) {
                        buf.enqueue(f) ;
                        inserted = true ;
                        metaData.iMaxSeqSeen = aSeqNo ;
                    } else if ( buf.first().iSeqNo > aSeqNo ) {
                        // we got the packet that is earlier than
                        // our head, it goes to pole position (index 0)
                        // and becomes new first()
                        buf.insert(0,f) ;
                        inserted = true ;
                        if ( aIsLocallyCaptured == false ) {
                            metaData.iMaxSeqSeen = buf.last().iSeqNo ;
                        }
                    } else if ( buf.last().iSeqNo < aSeqNo ) {
                        // we got the packet that is later than our latest
                        // on queue so it will go to tail
                        buf.append(f) ;
                        inserted = true ;
                    } else {
                        // our packet is not first, not last but belongs
                        // into the middle. If we came here, it also means
                        // that buf.size() is at least 2.
                        for ( int i = buf.size()-1 ; i >= 1 ; i-- ) {
                            if ( buf[i].iSeqNo > aSeqNo &&
                                    buf[i-1].iSeqNo < aSeqNo ) {
                                // our pos is i:
                                buf.insert(i,f) ;
                                inserted = true ;
                                if ( aIsLocallyCaptured == false ) {
                                    metaData.iMaxSeqSeen = buf.last().iSeqNo ;
                                }
                                break ; // out of for loop
                            }
                        }
                    }
                }
            } // end if of if buffer was already full
        } // end-if of buffer was found
        if ( inserted && aIsLocallyCaptured ) {
            tryMixFrames(aCallId,
                         aSeqNo) ;
        }
    }
}

void AudioMixer::insertStream (quint32 aCallId,
                               quint32 aStartingSeq,
                               bool aIsLocallyCaptured,
                               const Hash& aOriginatingNode) {
    QLOG_STR("AudioMixer::insertStream " + QString::number(aCallId) +
             " local " + QString::number(aIsLocallyCaptured) +
             " node " + aOriginatingNode.toString() ) ;
    for ( int i = iStreams.size()-1 ; i >= 0 ; i-- ) {
        AudioFrameMetadata& s ( iStreams[i] ) ;
        if ( s.iCallId == aCallId &&
                s.iIsLocallyCaptured == aIsLocallyCaptured ) {
            return ; // was there already..
        }
    }
    // add metadata
    AudioFrameMetadata newStream ;
    newStream.iCallId = aCallId ;
    newStream.iStartingSeq = aStartingSeq ;
    newStream.iMaxSeqSeen = aStartingSeq ;
    newStream.iIsLocallyCaptured = aIsLocallyCaptured ;
    newStream.iNode = aOriginatingNode ;
    QLOG_STR("Appending stream to mixer callid = " +
             QString::number(newStream.iCallId) +
             " local cap = "  +
             QString::number(newStream.iIsLocallyCaptured) +
             " hash of orig node = " +
             newStream.iNode.toString()) ;

    iStreams.append(newStream) ;
    // add queue for voice data
    QQueue<AudioFrame> buf ;
    iAudioDataBuffer.append(buf) ;
    configureMixedStreams()  ;
}

void AudioMixer::removeStream (quint32 aCallId) {
    int indexFound ( -1 ) ;
    for ( int i = iStreams.size()-1 ; i >= 0 ; i-- ) {
        AudioFrameMetadata& s ( iStreams[i] ) ;
        if ( s.iCallId == aCallId &&
                s.iIsLocallyCaptured == false ) {
            indexFound = i ;
            break ; // out of the loop
        }
    }
    if ( indexFound > -1 ) {
        iStreams.takeAt(indexFound) ;
        iAudioDataBuffer.takeAt(indexFound) ;
    }
    configureMixedStreams()  ;
    // now, after configuration check that if we
    // were left with only local microphone stream.. it means
    // that it shall be removed too:
    if ( iIndexesToMixForRemoteOutput.size() == 1 &&
            iIndexesToMixForLocalSpeaker.size() == 0 ) {
        iStreams.clear() ;
        iIndexesToMixForRemoteOutput.clear() ;
    }
}

// note: when this is called, datamodel is locked.
// this is actual mixer workhorse, audio frames go through this method.
// this will try to fetch packets from queues, mix them together
// and emit() the resulting frames to both local output and
// to networked remote parties
void AudioMixer::tryMixFrames(quint32 aCallId,
                              quint32 aSeqNo) {
    // lets see, we have several options for our mode of work..
    if ( ( iIndexesToMixForLocalSpeaker.size() == 1 &&
            iIndexesToMixForRemoteOutput.size() == 1 ) ) {
        // in this case we have single call and there is no need to mix

        while( iAudioDataBuffer[iIndexesToMixForLocalSpeaker[0]].size()) {
            AudioFrame frame ( iAudioDataBuffer[iIndexesToMixForLocalSpeaker[0]].dequeue()) ;
            emit frameReadyForLocalSpeaker( frame.iFrame ) ;
        }
        while( iAudioDataBuffer[iIndexesToMixForRemoteOutput[0]].size()) {
            AudioFrame frame ( iAudioDataBuffer[iIndexesToMixForRemoteOutput[0]].dequeue()) ;
            if ( iStreams.size() > 0 ) {
                emit frameReadyForRemoteSend(
                    aCallId,
                    aSeqNo,
                    frame.iFrame,
                    // here: we have 2 streams, one is local capture
                    // and it has iNode value 0. This is the stream
                    // that we're taking from. So the destination address
                    // actually is found from the stream, that we're
                    // not handling here, e.g. "for local speaker":
                    iStreams[iIndexesToMixForLocalSpeaker[0]].iNode ) ;
            }
        }
    } else {
        // then non-trivial cases where mixing actually happens, previous case just
        // copies the frame as it is.

        // first check if we mix at all. The trigger is that there is at least one queue with enough
        // packets in there
        int longestQueueLengthDiscovered ( 0 ) ;
        for ( int i = 0 ; i < iAudioDataBuffer.size() ; i++ ) {
            QQueue<AudioFrame>& queueInBuffer ( iAudioDataBuffer[i] ) ;
            if ( queueInBuffer.size() > longestQueueLengthDiscovered ) {
                longestQueueLengthDiscovered = queueInBuffer.size() ;
            }
        }
        if ( longestQueueLengthDiscovered > KPreferredBufferLen ) {
            // there is enough packets in at least one buffer, go
            // ahead and mix
            int packetsToMixFromEachQueue = longestQueueLengthDiscovered - KPreferredBufferLen ;
            QLOG_STR("Going to mix " +
                     QString::number(packetsToMixFromEachQueue) +
                     " packets from queues") ;
            for ( int qIndex = 0 ; qIndex < packetsToMixFromEachQueue ; qIndex++ ) {
                // first mix for remote outputs because every stream gets mixed
                // to every remote output
                quint32 seqNo ( 0 ) ;
                bool callIdIsSet ( false ) ;
                int indexOfQueueWithHighestVolume ( -1 ) ;
                int indexOfQueueWith2ndHighestVolume ( -1 ) ;
                float highestVolumeSeen ( -100.0f ) ;
                float secondHighestVolumeSeen ( -100.0f ) ;

                int streamCountToRemoteOutput ( iAudioDataBuffer.size() ) ;
                if (  streamCountToRemoteOutput ) {
                    // find queue with highest volume:
                    for ( int i = 0 ; i < streamCountToRemoteOutput ; i++ ) {
                        QQueue<AudioFrame>& queueToTry ( iAudioDataBuffer[i] ) ;
                        if ( queueToTry.size() ) {
                            AudioFrame& frame ( queueToTry.head() ) ;
                            if ( frame.iIsLocallyCaptured ) {
                                seqNo = frame.iSeqNo ;
                                callIdIsSet = true ;
                            }
                            if ( frame.iVolumeLevel > highestVolumeSeen ) {
                                highestVolumeSeen = frame.iVolumeLevel ;
                                indexOfQueueWithHighestVolume = i ;
                            }
                        }
                    }
                    // find queue with 2nd highest volume:
                    for ( int i = 0 ; i < streamCountToRemoteOutput ; i++ ) {
                        QQueue<AudioFrame>& queueToTry ( iAudioDataBuffer[i] ) ;
                        if ( queueToTry.size() ) {
                            AudioFrame& frame ( queueToTry.head() ) ;
                            if ( frame.iVolumeLevel >= secondHighestVolumeSeen &&
                                    frame.iVolumeLevel <= highestVolumeSeen &&
                                    i != indexOfQueueWithHighestVolume ) {
                                secondHighestVolumeSeen = frame.iVolumeLevel ;
                                indexOfQueueWith2ndHighestVolume = i ;
                            }
                        }
                    }
                } // end of loop finding noisiest queues
                QLOG_STR("Loudest frame indexes " +
                         QString::number(indexOfQueueWithHighestVolume) +
                         " " + QString::number(indexOfQueueWith2ndHighestVolume) ) ;
                if ( callIdIsSet &&
                        indexOfQueueWithHighestVolume >= 0 &&
                        indexOfQueueWith2ndHighestVolume >= 0 ) {
                    // now, do no mixing. Instead copy the queue with max volume
                    // to everybody but not to originator. originator in turn gets
                    // contents of the 2nd noisiest queue. this prevents very frustrating
                    // echoes ; here nobody should hear echo of own voice.

                    QQueue<AudioFrame> & loudestQueue ( iAudioDataBuffer[indexOfQueueWithHighestVolume] ) ;
                    QQueue<AudioFrame> & secondLoudestQueue ( iAudioDataBuffer[indexOfQueueWith2ndHighestVolume] ) ;
                    AudioFrame& loudestFrame ( loudestQueue.head() ) ;
                    AudioFrame& secondLoudestFrame (secondLoudestQueue.head() );
                    Hash& originOfLoudestFrame (iStreams[indexOfQueueWithHighestVolume].iNode) ;
                    Hash& originOf2ndLoudestFrame (iStreams[indexOfQueueWith2ndHighestVolume].iNode) ;

                    foreach ( const AudioFrameMetadata& stream , iStreams ) {
                        // first check if we have stream of local output:
                        if ( stream.iIsLocallyCaptured ) {
                            // if it is us that is the loudest, then take 2nd:
                            if ( originOfLoudestFrame == KNullHash ) {
                                // it was me making loudest noise:
                                emit frameReadyForLocalSpeaker(secondLoudestFrame.iFrame) ;
                            } else {
                                // normally copy to local speaker the loudest
                                // speaker:
                                emit frameReadyForLocalSpeaker(loudestFrame.iFrame) ;
                            }
                        } else {
                            // was not locally captured
                            Hash fingerPrintOfStream ( stream.iNode ) ;
                            if ( stream.iNode != originOfLoudestFrame ) {
                                emit( frameReadyForRemoteSend(stream.iCallId,
                                                              seqNo,
                                                              loudestFrame.iFrame,
                                                              fingerPrintOfStream ) ) ;
                            } else if ( stream.iNode != originOf2ndLoudestFrame ) {
                                emit( frameReadyForRemoteSend(stream.iCallId,
                                                              seqNo,
                                                              secondLoudestFrame.iFrame,
                                                              fingerPrintOfStream ) ) ;
                            } else {
                                QLOG_STR("Audiomixer: Frame was not sent anywhere??") ;
                            }

                        }
                    }



                    // now both local and remote mixing done: remove one
                    // frame from each queue
                    for ( int i = 0 ; i < iAudioDataBuffer.size() ; i++ ) {
                        QQueue<AudioFrame>& queue ( iAudioDataBuffer[i] ) ;
                        if ( queue.size() ) {
                            queue.dequeue() ; // one
                        }
                    }
                } // end if of "if call id was set"
            } // Qindex e.g. queue index loop end
        }
    }
    return ;
}


void AudioMixer::configureMixedStreams() {
    iIndexesToMixForLocalSpeaker.clear() ;
    iIndexesToMixForRemoteOutput.clear() ;

    // trivial cases first..
    if ( iStreams.size() == 1 && iStreams[0].iIsLocallyCaptured ) {
        iIndexesToMixForRemoteOutput.append(0) ;
        QLOG_STR("Case 1 Mixing for remote node " + QString::number(iStreams[0].iCallId) +
                 " is locally captured " + QString::number(iStreams[0].iIsLocallyCaptured) +
                 " node addr " +
                 iStreams[0].iNode.toString()) ;
    }
    // another trivial
    if ( iStreams.size() == 1 && iStreams[0].iIsLocallyCaptured == false ) {
        iIndexesToMixForLocalSpeaker.append(0) ;
        QLOG_STR("Case 2 Mixing for local speaker " + QString::number(iStreams[0].iCallId) +
                 " is locally captured " + QString::number(iStreams[0].iIsLocallyCaptured)+
                 " node addr " +
                 iStreams[0].iNode.toString()) ;
    }
    // then 1-1 cases, e.g. 2 streams and one is local
    if ( iStreams.size() == 2 &&
            iStreams[0].iIsLocallyCaptured == false &&
            iStreams[1].iIsLocallyCaptured == true ) {
        iIndexesToMixForLocalSpeaker.append(0) ;
        iIndexesToMixForRemoteOutput.append(1) ;
        QLOG_STR("Case 3 Mixing for local speaker " + QString::number(iStreams[0].iCallId) +
                 " is locally captured " + QString::number(iStreams[0].iIsLocallyCaptured)+
                 " node addr " +
                 iStreams[0].iNode.toString()) ;
        QLOG_STR("Mixing for remote node " + QString::number(iStreams[1].iCallId) +
                 " is locally captured " + QString::number(iStreams[1].iIsLocallyCaptured)+
                 " node addr " +
                 iStreams[1].iNode.toString()) ;
    }
    if ( iStreams.size() == 2 &&
            iStreams[0].iIsLocallyCaptured == true &&
            iStreams[1].iIsLocallyCaptured == false ) {
        iIndexesToMixForLocalSpeaker.append(1) ;
        iIndexesToMixForRemoteOutput.append(0) ;
        QLOG_STR("Case 4 Mixing for local speaker " + QString::number(iStreams[1].iCallId) +
                 " is locally captured " + QString::number(iStreams[1].iIsLocallyCaptured)+
                 " node addr " +
                 iStreams[1].iNode.toString()) ;
        QLOG_STR("Mixing for remote node " + QString::number(iStreams[0].iCallId) +
                 " is locally captured " + QString::number(iStreams[0].iIsLocallyCaptured)+
                 " node addr " +
                 iStreams[0].iNode.toString()) ;
    }
    // then non-trivial cases. there are more than 2 streams.
    if ( iStreams.size() > 2 ) {
        // this is only situation where actual audio mixing takes
        // place. Int remote put everything we have:
        for ( int i = iStreams.size()-1 ; i >= 0 ; i-- ) {
            iIndexesToMixForRemoteOutput.append(i) ;
            QLOG_STR("Mixing for remote node " + QString::number(iStreams[i].iCallId) +
                     " is locally captured " + QString::number(iStreams[i].iIsLocallyCaptured)) ;
        }
        // into local put everything but the local itself:
        for ( int i = iStreams.size()-1 ; i >= 0 ; i-- ) {
            if ( iStreams[i].iIsLocallyCaptured == false ) {
                iIndexesToMixForLocalSpeaker.append(i) ;
                QLOG_STR("Mixing for local speaker " + QString::number(iStreams[i].iCallId) +
                         " is locally captured " + QString::number(iStreams[i].iIsLocallyCaptured)) ;
            }
        }
    }
}
