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

#include "audioencoder.h"
#include "../log.h"
#include "../util/hash.h"

const int KOutputBufferSize ( 2048 ) ; 

AudioEncoder::AudioEncoder(quint32 aSampleRate,
                           quint32 aNumChannels  )  :
    iEncoder(NULL)  ,
    iEncodedData(NULL),
    iNumChannels(aNumChannels)
{
    int errorCode ( 0 ) ; 
    iEncoder = opus_encoder_create(aSampleRate,
                                   aNumChannels,
                                   OPUS_APPLICATION_VOIP,
                                   &errorCode ); 
    opus_encoder_ctl(iEncoder, OPUS_SET_BITRATE(OPUS_AUTO));
    if ( errorCode != OPUS_OK || iEncoder == NULL ) {
        QLOG_STR( "opus_encoder_create " + QString::number(errorCode) ) ; 
    }
    iEncodedData = (unsigned char *) malloc ( KOutputBufferSize ) ; 
}


AudioEncoder::~AudioEncoder() {
    QLOG_STR( "AudioEncoder::~AudioEncoder()" ) ; 
    if ( iEncoder ) {
        opus_encoder_destroy(iEncoder) ; 
        iEncoder = NULL ; 
    }
    if ( iEncodedData ) {
        delete iEncodedData ; 
    }
}

// note, this slot is connected synhronously and called with
// datamodel lock on. it makes sense as there is only one
// encoder and that encodes for all remote outputs. 
void AudioEncoder::frameReady(quint32 aCallId,
                              quint32 aSeqNo,
                              const QByteArray& aAudioData,
                              Hash aForNode ) {
    if ( iEncodedData && iEncoder ) {
        int frameSize = (aAudioData.size()/
                         iNumChannels)/
            sizeof(float) ;
        int resultLen = opus_encode_float(iEncoder, 
                                          (const float*)(aAudioData.constData()), 
                                          frameSize, 
                                          iEncodedData, KOutputBufferSize);
        if (resultLen < 0) {
            switch ( resultLen ) {
            case OPUS_OK:
                QLOG_STR("Opus:No error.") ; 
                break ;
            case OPUS_BAD_ARG:
                QLOG_STR("Opus:One or more invalid/out of range arguments. ") ;
                break ;
            case OPUS_BUFFER_TOO_SMALL:
                QLOG_STR("Opus: Not enough bytes allocated in the buffer.") ;
                break ;
            case OPUS_INTERNAL_ERROR:
                QLOG_STR("Opus: An internal error was detected.") ;
                break ;
            case OPUS_INVALID_PACKET:
                QLOG_STR("Opus: The compressed data passed is corrupted.") ; 
                break ;
            case OPUS_UNIMPLEMENTED:
                QLOG_STR("Opus: Invalid/unsupported request number.") ; 
                break ; 
            case OPUS_INVALID_STATE:
                QLOG_STR("Opus: An encoder or decoder structure is invalid or already freed.") ; 
                break ; 
            case OPUS_ALLOC_FAIL:
                QLOG_STR("Opus: Memory allocation failed") ; 
                break ;
            default:
                QLOG_STR("Opus: error " + QString::number(resultLen)) ; 
                break ;
            }
        } else {
            QLOG_STR("Opus encoded data len " + QString::number(resultLen)) ;
            QByteArray encodedData((const char *)iEncodedData,
                                   resultLen ) ; 
            emit frameEncoded(aCallId,aSeqNo,encodedData,aForNode ) ; 
        }
    }
}

