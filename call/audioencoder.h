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

#ifndef AUDIO_ENCODER_H
#define AUDIO_ENCODER_H

#include <QObject>
#include <opus/opus.h> // for data types

/**
 * @brief class for compressing audio to be sent over network
 *
 * This class encapsulates opus encoder for packing audio over network.
 * Future releases may include other options regarding the codec or 
 * type of input (audio/video/feeling/touch/smell/other?)
 */
class AudioEncoder : public QObject {
    Q_OBJECT

public:
    /**
     * Constructor.
     */
    AudioEncoder(quint32 aSampleRate = 8000,
                 quint32 aNumChannels = 1 ) ;
    ~AudioEncoder() ;

signals:
    /**
     * This signal is emitted after audio frame has been encoded
     * into opus format. When audio data is sent in via slot
     * @ref frameReady then this signal is emitted as result
     * of succesful processing. 
     * 
     * @param aCallId call identifer of the audio stream
     * @param aSeqNo sequence number of frame inside stream
     * @param aEncodedData raw opus frame, as it comes out
     *        from opus library call opus_encode_float().
     */
void frameEncoded(quint32 aCallId,
                  quint32 aSeqNo,
                  const QByteArray& aEncodedData) ;
public slots: 
    /**
     * Input to encoder cames in via this method.  
     *
     * @param aCallId call identifer of the audio stream
     * @param aSeqNo sequence number of frame inside stream
     * @param aAudioData raw, unpacked audio samples. The
     *        byte-array must contain an array of floats
     *        in [-1,1] range. 
     */
void frameReady(quint32 aCallId,
                quint32 aSeqNo,
                const QByteArray& aAudioData) ;  
private: // members
    OpusEncoder *iEncoder ; 
    unsigned char *iEncodedData ; 
    const quint32 iNumChannels ; 
} ; 
#endif
