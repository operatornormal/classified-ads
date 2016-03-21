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

#ifndef AUDIO_MIXER_H
#define AUDIO_MIXER_H

#include <QObject>
#include <QByteArray>
#include <QQueue>

class Model ; 

/**
 * @brief Class for mixing together 0-n audio streams
 *
 * This class has 2 types of input and 2 types of output. 
 *  Input type 1: locally captured microphone data
 *  Input type 2: 0-n streams sent from remote nodes
 *  Output type 1: Audio intended for playback from local loudspeaker
 *  Output type 2: Audio intended to be sent to remote nodes
 * Depending on situation this class will mix a bit differently. 
 * If there is only type 1 input and 1 type 2 stream then no mixing
 * will occur: input type  2 becomes output type 1 and that's it. 
 * If there is more than 1 streams to mix together then mixer will
 * actually mix. 
 */
class AudioMixer : public QObject {
    Q_OBJECT

public:
    /**
     * Constructor.
     */
    AudioMixer(Model& aModel) ;
    ~AudioMixer() ;

    /**
     * method for entering stream metadata so that stream with given
     * callid gets included into mixer output
     */
    void insertStream (quint32 aCallId,
                       quint32 aStartingSeq,
                       bool aIsLocallyCaptured ) ;    
    /**
     * method for removing stream metadata so that stream with given
     * callid no longer gets included into mixer output
     */
    void removeStream (quint32 aCallId) ;
signals:
    /** emitted when mixing done for local playback */
    void frameReadyForLocalSpeaker(const QByteArray& aFrame) ; 
    /** emitted when mixing done for remote nodes */
    void frameReadyForRemoteSend(quint32 aCallId,
                                 quint32 aSeqNo,
                                 const QByteArray& aFrame) ; 
public slots:
    /**
     * Method for inputting an audio frame from local microphone. 
     * Frame is 60 milliseconds raw audio data in floating point format 
     * with 8kHz sample rate. That means there  are 480 samples, totalling 
     * 960*2 bytes. See constants in top of audiomixer.cpp file. Each
     * floating point sample in array needs to be in range [-1,1]
     *
     * @param aFrame is the audio data. floating point, 
     *               native-encoding audio/pcm data.
     * @param aSeqNo Sequence number of frame. This is a incrementing
     *               number that begins from 0 at stream start. Note that
     *               it is entirely possible to start mixing a stream
     *               from middle so when processing starts, this value
     *               does not necessarily start from 0 
     */
    void insertCapturedAudioFrame ( const QByteArray& aFrame,
                                    quint32 aSeqNo ); 

    /**
     * Method for inputting an audio frame that is received from network. 
     * See method @ref insertCapturedAudioFrame for more information, same
     * frame data format must be used. 
     *
     * @param aFrame is the audio data. float, native-encoding audio/pcm data.
     * @param aSeqNo Sequence number of frame. This is a incrementing
     *               number that begins from 0 at stream start. Note that
     *               it is entirely possible to start mixing a stream
     *               from middle so when processing starts, this value
     *               does not necessarily start from 0 
     * @param aCallId Call id for keeping track which frame belongs to which
     *                stream. Typically inside mixer there are both
     *                locally captured frames and frames from remote that
     *                both have same call id ; if there are multiple calls in
     *                progress then one set of frames of local input and
     *                several from remote sources. If aIsLocallyCaptured
     *                is true, then value of this parameter has no effect
     */
    void insertReceivedAudioFrame ( quint32 aCallId,
                                    quint32 aSeqNo,
                                    const QByteArray& aFrame ); 

private: // methods
    /** 
     * workhorse method. will check for buffers and see if we have 
     * frames for all streams to mix and if conditions evaluate true,
     * will output the mixed frame, or several if possible. 
     */
    void tryMixFrames(quint32 aCallId,
                      quint32 aSeqNo) ; 
   /**
    * method for figuring out which stream to mix into which
    * direction
    */
    void configureMixedStreams() ; 
    /** 
     * workhorse of insertCaptured/ReceivedAudioFrame slots 
     *
     * @param aIsLocallyCaptured is true if audio is from local microphone
     *                           and false if received from remote node
     */
    void doInsertAudioFrame ( const QByteArray& aFrame,
                              bool aIsLocallyCaptured,
                              quint32 aSeqNo,
                              quint32 aCallId ); 
    /**
     * Method that clips away samples that do not fall into [-1,1]
     * range
     */
    void clip ( QByteArray& aBuffer ) ;

private: // members
    typedef struct AudioFrameStruct {
        quint32 iCallId ; 
        quint32 iSeqNo ;
        bool iIsLocallyCaptured ; /**< from microphone input */
        QByteArray iFrame ; 
    } AudioFrame ;
    typedef struct AudioStreamMetadata {
        quint32 iCallId ;
        quint32 iStartingSeq ;
        /** 
         * This is remote sequence number. Justfication: seqno is for
         * ordering the packets. Our own microphone data will arrive in
         * correct order. Here store only seqno of streams sent over
         * network that are suspectible to droppage,re-ordering and multiplication
         */
        quint32 iMaxSeqSeen; 
        bool iIsLocallyCaptured ; /**< concerns microphone input */
    } AudioFrameMetadata ;
    /**
     * Voice data buffer. Must share same indexing with iStreams e.g.
     * if stream metadata for call x is in iStreams[y] then call data
     * for that stream needs to go to queue strictly at iAudioDataBuffer[y]
     */
    QList<QQueue<AudioFrame> > iAudioDataBuffer ; 
    /** Metadata of streams being served */
    QList<AudioFrameMetadata> iStreams ;
    /** List of indexes to iStreams that are mixed into speaker output */
    QList<int> iIndexesToMixForLocalSpeaker ;
    /** List of indexes to iStreams that are mixed into remote nodes */
    QList<int> iIndexesToMixForRemoteOutput ;
    Model& iModel ; /**< datamodel */
} ;
#endif
