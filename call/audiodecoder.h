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

#ifndef AUDIO_DECODER_H
#define AUDIO_DECODER_H

#include <QObject>
#include <opus/opus.h> // for data types
#include "../util/hash.h"

/**
 * @brief class for de-compressing audio received from network
 *
 * This class encapsulates opus decoder for packing audio over network.
 */
class AudioDecoder : public QObject {
    Q_OBJECT

public:
    /**
     * Constructor.
     */
    AudioDecoder(quint32 aSampleRate = 8000,
                 quint32 aNumChannels = 1 ) ;
    ~AudioDecoder() ;

signals:
    /**
     * This signal is emitted when decoded frame is ready for
     * further processing, typically by audio mixer.
     * @param aCallId call where frame originates from 
     * @param aSeqNo sequence number of the frame in the call
     * @param aEncodedData raw de-coded audio data. In practice the
     *        byte-array holds array of floats, each preseting one
     *        sample, in range [-1,1]. 
     */
void frameDecoded(quint32 aCallId,
                  quint32 aSeqNo,
                  const QByteArray& aEncodedData,
                  Hash  aOriginatingNode ) ;
public slots: 
/**
 *  Received audio frames to be de-coded are connected to this slot.
 */
void frameReceived(quint32 aCallId,
                   quint32 aSeqNo,
                   const QByteArray& aAudioData,
                   Hash aOriginatingNode ) ;  
private: // members
    OpusDecoder* iDecoder ; 
    QByteArray iDecodedData ; 
    const quint32 iNumChannels ; 
    const quint32 iSampleRate ; 
} ; 
#endif
