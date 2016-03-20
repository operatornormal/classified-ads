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

#include "audioplayer.h"
#include <QQueue>
#include "../log.h"
#include <QCoreApplication>
#include "../datamodel/model.h"

AudioPlayer::AudioPlayer(Model& aModel ,
                         int aFrequency ,
                         int aNumChannels ,
                         int aSampleSize ) :
    iAudioOutput(NULL),
    iNeedsToRun(true) ,
    iModel(aModel),
    iFrequency(aFrequency),
    iNumChannels(aNumChannels),
    iSampleSize(aSampleSize)  
{
    QLOG_STR("AudioPlayer::AudioPlayer() fr " + 
             QString::number(aFrequency) + " size " + 
             QString::number(aSampleSize)) ;  
#if QT_VERSION < 0x050000
    iFormat.setChannels(aNumChannels);
    iFormat.setFrequency(aFrequency);
#else
    iFormat.setChannelCount(aNumChannels);
    iFormat.setSampleRate(aFrequency) ; 
#endif
    iFormat.setSampleSize(aSampleSize);
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

    iFormat.setSampleType(QAudioFormat::SignedInt);
    QAudioDeviceInfo defaultDevice ( QAudioDeviceInfo::defaultOutputDevice() ) ; 
    QAudioDeviceInfo info( defaultDevice );
    if (!info.isFormatSupported(iFormat)) {
        emit error(0,tr("raw audio format not supported by backend, cannot play audio."));
        return;
    }
    QLOG_STR("Output dev " + info.deviceName()) ; 
    iAudioOutput = new QAudioOutput(defaultDevice,iFormat, this);

    if ( iAudioOutput != NULL ) {
        iAudioOutput->setNotifyInterval(60) ; 
        connect(iAudioOutput, SIGNAL(notify()),
                this, SLOT(notify())) ; 
        if ( iAudioOutput->format() != iFormat ) {
            QLOG_STR("Format was not supported") ; 
        }
        connect(iAudioOutput,SIGNAL(stateChanged(QAudio::State)),SLOT(finishedPlaying(QAudio::State)));
        open(QIODevice::ReadOnly) ;
        iAudioOutput->start(this);
        iAudioOutput->suspend() ; 
    } else {
        emit error(0,tr("Could not initialize audio player."));
    }
}

AudioPlayer::~AudioPlayer() {
    QLOG_STR("AudioPlayer::~AudioPlayer()") ;  
    if ( iAudioOutput ) {
        if ( iAudioOutput->state() != QAudio::StoppedState ) {
            iAudioOutput->stop() ; 
        }
        delete iAudioOutput ;
        iAudioOutput = NULL ;
    }
}

// this slot is connected in synhronous way and called so
// that datamodel lock is on. 
void AudioPlayer::insertAudioFrame ( const QByteArray& aFrame ){
    // always append in into buffer .. but if buffer is very large
    // already, just discard the frame and some of buffers content
    if ( ( iAudioBuffer.size() / (int)( iNumChannels * sizeof(qint16) ) ) >
         iFrequency * 5 ) {
        iAudioBuffer.clear() ; 
        QLOG_STR( "buffer reset" ) ; 
    } else {
        const float signed16BitIntMax = 
            (float)(std::numeric_limits<qint16>::max()-1)  ;
        float audioSum ( 0 )  ;
        QByteArray convertedSamples ; 
        int numberOfSamples = aFrame.size() / sizeof(float) ;
        float* samples((float *)( aFrame.data() )) ; 
        convertedSamples.fill(0,numberOfSamples*sizeof(qint16)) ; 
        qint16* convertedSamplesData((qint16 *)( convertedSamples.data() )) ; 
        for ( int i = 0 ; i < numberOfSamples ; i++ ) {
            if ( samples[i] < 1.0 &&
                 samples[i] > -1.0 ) {
                convertedSamplesData[i] = (qint16)((samples[i]) * signed16BitIntMax ) ;
                audioSum += qAbs(samples[i]) ; 
            }
        }
        if ( convertedSamples.size() > 0 ) {

            iAudioBuffer.append(convertedSamples) ;
            emit readyRead() ; 
            emit audioMaxLevel(audioSum/(float)numberOfSamples) ; 
        }
    }
    if (iAudioOutput && 
        iAudioOutput->state() == QAudio::SuspendedState &&
        iAudioBuffer.size() > 2000 ) {
        iAudioOutput->resume() ;
    }
}

void AudioPlayer::finishedPlaying ( QAudio::State aState ) {
    QLOG_STR("AudioPlayer::finishedPlaying state " + 
             QString::number(aState)) ;  
    if ( aState == QAudio::StoppedState ) {
        deleteLater() ; 
    }
    if ( aState == QAudio::IdleState && iAudioOutput ) {
      iAudioOutput->suspend() ; // lets resume when there is bytes in buffer
    }
}

void AudioPlayer::stop() {
    iNeedsToRun = false ; 
}

void AudioPlayer::notify() {
    if ( iNeedsToRun == false &&         
         iAudioOutput) {
        QLOG_STR("AudioPlayer:: notify stopped player ") ; 
        iAudioOutput->stop() ; 
    }
}

// from QIODevice:
qint64 AudioPlayer::writeData ( const char * /*data*/, qint64 /*maxSize*/ ) {
    return 0 ;
}

// From QIODevice: audio data goes to sound card via this method
qint64 AudioPlayer::readData ( char * data, qint64 maxSize ) {
    qint64 bytesToWrite = maxSize ; // bytes to write into buffer of player
    iModel.lock() ; // do mixing behind lock
    if ( iAudioBuffer.size() < bytesToWrite ) {
        bytesToWrite = iAudioBuffer.size() ;
    }
    if ( bytesToWrite > 0 ) {
        memcpy(data, iAudioBuffer.constData(), bytesToWrite) ;
        iAudioBuffer.remove(0,bytesToWrite) ; 
    }
    iModel.unlock() ; 
    return bytesToWrite ;
}

// from QIODevice:
bool AudioPlayer::open ( OpenMode mode ) {
    if ( mode & QIODevice::ReadOnly ) {
        return QIODevice::open(mode) ; 
    } else {
        return false ; 
    }
}
