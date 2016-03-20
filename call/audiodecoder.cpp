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

#include "audiodecoder.h"
#include <QDebug>
#include "../log.h"

const int KDecodedBufferSize ( 1024 * 6 ) ; 

AudioDecoder::AudioDecoder(quint32 aSampleRate,
                           quint32 aNumChannels  )  :
    iDecoder(NULL)  ,
    iNumChannels(aNumChannels),
    iSampleRate(aSampleRate) 
{
    int errorCode ( 0 ) ; 
    iDecoder = opus_decoder_create((opus_int32)aSampleRate,
                                   (int)aNumChannels,
                                   &errorCode ); 
    if ( errorCode != OPUS_OK || iDecoder == NULL ) {
        QLOG_STR( "opus_decoder_create " + QString::number(errorCode)) ; 
    }
    QLOG_STR( "decoder samplerate " + QString::number(aSampleRate)) ; 
    QLOG_STR( "decoder channels " + QString::number(aNumChannels)) ; 
    iDecodedData.fill ( 0, KDecodedBufferSize ) ; 
}


AudioDecoder::~AudioDecoder() {
    QLOG_STR( "~AudioDecoder" )  ; 
    if ( iDecoder ) {
        opus_decoder_destroy(iDecoder) ; 
        iDecoder = NULL ; 
    }
}

void AudioDecoder::frameReceived(quint32 aCallId,
                                 quint32 aSeqNo,
                                 const QByteArray& aAudioData) {
    if ( iDecoder && aAudioData.size() ) {
        int frameSize = ( iSampleRate / 1000 ) * 60 ;
        QLOG_STR( "Number of frames in packet " + QString::number(frameSize) )  ; 
        int resultLen = opus_decode_float(iDecoder, 
                                          (const unsigned char*)(aAudioData.constData()), 
                                          aAudioData.size() , 
                                          (float*)(iDecodedData.data()), 
                                          frameSize,
                                          0);
        if (resultLen < 0) {
            QLOG_STR( "opus_decode() failed with error code: " +
                      QString::number(resultLen));
        } else {
            // this is not queued connection, will be processed
            // synchronously by mixer. note that in this
            // stage datamodel lock is *on*. 
            emit frameDecoded(aCallId,aSeqNo,iDecodedData.mid(0,resultLen*iNumChannels*sizeof(float))) ; 
        }
    }
}

