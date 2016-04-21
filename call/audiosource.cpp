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

#include "audiosource.h"
#include <QAudioInput> 
#include <QDebug>
#include <QSysInfo>
#include "../log.h"
#include "../datamodel/model.h"
#include <limits>

/** how long time is recorded into each packet */
const int KProcessPacketSizeMilliSec ( 60 ) ; 
/** sound card sample rate */
const int KSampleRate ( 8000 ) ; 
/** how many bits per each sample */
const int KSampleSizeBits ( 16 ) ; 
/** record in mono, stereo would count as multimedia */
const int KChannelCount ( 1 ) ; 

AudioSource::AudioSource( Model& aModel ) : 
    iAudioInput(NULL) ,
    iSeqNo ( 0 )  ,
    iNeedsToRun(true),
    iModel(aModel),
    iTimerId(-1)
{

#if QT_VERSION < 0x050000
    iFormat.setChannels(KChannelCount);
    iFormat.setFrequency(KSampleRate);
#else
    iFormat.setChannelCount(KChannelCount);
    iFormat.setSampleRate(KSampleRate) ; 
#endif
    iFormat.setSampleSize(KSampleSizeBits);
    iFormat.setCodec("audio/pcm");
    // data is later fed to opus codec and that expects samples
    // in native byte order:
#if Q_BYTE_ORDER == Q_BIG_ENDIAN
    iFormat.setByteOrder(QAudioFormat::BigEndian);
    QLOG_STR("QAudioFormat::BigEndian") ; 
#else
    iFormat.setByteOrder(QAudioFormat::LittleEndian);
    QLOG_STR("QAudioFormat::LittleEndian") ; 
#endif


    // There seems to be problems with both Float type and
    // unsigned int type, at least in linux. Opus library in
    // turn wants uint16 or float so some conversion later
    // will be in order. 
    iFormat.setSampleType(QAudioFormat::SignedInt);

    QAudioDeviceInfo info = QAudioDeviceInfo::defaultInputDevice();
    if (!info.isFormatSupported(iFormat)) {
        iFormat = info.nearestFormat(iFormat);
    }
    QLOG_STR(info.deviceName()) ; 
    QAudioDeviceInfo defaultDevice ( QAudioDeviceInfo::defaultInputDevice() ) ;
    iAudioInput = new QAudioInput(defaultDevice, iFormat, this);
    connect(iAudioInput, SIGNAL(stateChanged(QAudio::State)),
	    this, SLOT(inputStatusChanged(QAudio::State))) ; 
    if ( this->open(QIODevice::WriteOnly) ) {
        iAudioInput->start(this);
    } else {
        QLOG_STR("ERROR: Audio buffer open failed??") ; 
    }
    iTimerId = startTimer(KProcessPacketSizeMilliSec) ; 
}


AudioSource::~AudioSource() {
    QLOG_STR("AudioSource::~AudioSource") ; 
    if( iTimerId != -1 ) {
        killTimer(iTimerId) ; 
    }
    if( iAudioInput ) {
        iAudioInput->stop() ; 
        delete iAudioInput ; 
    }
}

void AudioSource::inputStatusChanged(QAudio::State aState) {
  QLOG_STR("AudioSource::inputStatusChanged " + 
	    QString::number(aState)) ; 
  if ( aState == QAudio::StoppedState ) {
      deleteLater() ; 
  }
}

void AudioSource::processCapturedAudio() {
    if ( iNeedsToRun ) {

        /** how many bytes we have in buffer for one frame: */
        const int frameSize ( ( ( KSampleRate / 1000 ) *
                                KProcessPacketSizeMilliSec) *
                              KChannelCount * 
                              sizeof(qint16) ) ;  
        
        const float signed16BitIntMax = 
            (float)(std::numeric_limits<qint16>::max()-1)  ; 
        while ( iAudioBuffer.size() >= frameSize ) {
            QByteArray audioDataAsFloats ;
            QByteArray capturedFrame(iAudioBuffer.left(frameSize)) ;

            int numberOfSamples = capturedFrame.size() / sizeof(qint16) ;
            audioDataAsFloats.fill(0,sizeof(float)*numberOfSamples) ;
            float volumeSum ( 0.0f ) ; 
            qint16* samples((qint16 *)( capturedFrame.data() )) ; 
            float* convertedSamples((float *)( audioDataAsFloats.data() )) ; 
            float value ( 0.0f ) ; 
            numberOfSamples-- ; 
            for ( ; numberOfSamples >= 0 ; numberOfSamples-- ) {
                value = samples[numberOfSamples] ;
                // in float presentation the samples need to have
                // numerical value between [-1,1] so lets scale the
                // values ; 
                value = ( value / signed16BitIntMax ) ;
                convertedSamples[numberOfSamples] = value ; 
                volumeSum = volumeSum + qAbs( value ) ; 
            }
            float volumeLevel ( volumeSum/(float)(capturedFrame.size() / sizeof(qint16) ) ) ;
            emit frameCaptured(audioDataAsFloats, 
                               iSeqNo++,
                               volumeLevel) ; 
            iAudioBuffer.remove(0,frameSize); 
            emit audioMaxLevel(volumeLevel) ; 
        }
    } else if( iAudioInput ) {
        if ( iAudioInput->state() == QAudio::ActiveState ||
             iAudioInput->state() == QAudio::IdleState ) {
            iAudioInput->stop() ; 
        } else {
            deleteLater() ; 
        }
    }
}


void AudioSource::stopCapturing() {
    iNeedsToRun = false ; 
}

// from QIODevice:
qint64 AudioSource::writeData ( const char * data, qint64 maxSize ) {
    if ( iNeedsToRun ) {
        iModel.lock() ; // do mixing behind lock
        iAudioBuffer.append(data,maxSize) ; 
        iModel.unlock() ; // do mixing behind lock
    }
    return maxSize ; 
}

// From QIODevice
qint64 AudioSource::readData ( char * /*data*/, qint64 /*maxSize*/ ) {
    return 0 ;
}

// from QIODevice:
bool AudioSource::open ( OpenMode mode ) {
    if ( mode & QIODevice::WriteOnly ) {
        return QIODevice::open(mode) ; 
    } else {
        return false ; 
    }
}


void AudioSource::timerEvent(QTimerEvent* /* event */ ) {
    iModel.lock() ; // do processing behind a lock
    processCapturedAudio() ; 
    iModel.unlock() ; // do mixing behind lock
}
