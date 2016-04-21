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
const int KRingtoneBufSize ( 4096 ) ; 
const int KRingtoneSampleRate ( 8000 ) ; 
const int KPlayerBufferSize ( 1024 * 16 ) ; 

#ifdef WIN32
#define NOMINMAX
#include <winsock2.h> // for ntohl
#else
#include "arpa/inet.h" // for ntohl
#endif
#include "ringtoneplayer.h"
#include "../log.h"
#include <QFile>
#include <QMutex>
#include <QApplication> // for beep()

RingtonePlayer::RingtonePlayer(Model& aModel,
                               MController& aController) :
    iAudioOutput(NULL),
    iNeedsToRun(true) ,
    iRingtoneFile(NULL),
    iTimerId (-1),
    iDecoder(NULL),
    iMutex(NULL),
    iModel(aModel),
    iController(aController)
{
    switch ( aModel.getRingtoneSetting() ) {
    case Model::BowRingTone:
    case Model::ElectricalRingTone:
    case Model::AcousticRingTone :
    case Model::BeepRingTone :
    {
        // normal ringtones
#if QT_VERSION < 0x050000
        iFormat.setChannels(1);
        iFormat.setFrequency(KRingtoneSampleRate);
#else
        iFormat.setChannelCount(1);
        iFormat.setSampleRate(KRingtoneSampleRate) ; 
#endif
        iFormat.setSampleSize(16);
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
        iMutex = new QMutex() ; 
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
        } else {
            emit error(0,tr("Could not initialize audio player."));
        }
        iTimerId = startTimer(30) ; 
        VoiceCallEngine* eng (iController.voiceCallEngine()) ;
        if ( eng ) {
            eng->installObserver(this) ; 
        }
    }
    break ; 
    case Model::NoRingTone :
        // user requested silence so get out immediately
        deleteLater() ; 
        break ; 
    default:
        // should not happen
        break ; 
    }
}

RingtonePlayer::~RingtonePlayer() {
    QLOG_STR("RingtonePlayer::~RingtonePlayer()") ;  
    VoiceCallEngine* eng (iController.voiceCallEngine()) ;
    if ( eng ) {
        eng->removeObserver(this) ; 
    }
    if ( iTimerId > -1 ) {
        killTimer(iTimerId) ; 
    }
    if ( iAudioOutput ) {
        if ( iAudioOutput->state() != QAudio::StoppedState ) {
            iAudioOutput->stop() ; 
        }
        delete iAudioOutput ;
        iAudioOutput = NULL ;
    }
    if ( iRingtoneFile ) {
        iRingtoneFile->close() ; 
        delete iRingtoneFile ; 
        iRingtoneFile = NULL ; 
    }
    if ( iDecoder ) {
        opus_decoder_destroy(iDecoder) ; 
        iDecoder = NULL ; 
    }
    if ( iMutex ) {
        delete iMutex ; 
    }
}


void RingtonePlayer::finishedPlaying ( QAudio::State aState ) {
    QLOG_STR("RingtonePlayer::finishedPlaying state " + 
             QString::number(aState)) ;  
    if ( aState == QAudio::StoppedState  ) {
        deleteLater() ; 
    }
    if ( aState == QAudio::IdleState && iAudioOutput ) {
      iAudioOutput->suspend() ; // lets resume when there is bytes in buffer
    }
}

void RingtonePlayer::stop() {
    iNeedsToRun = false ; 
}

void RingtonePlayer::notify() {
    if ( iNeedsToRun == false &&         
         iAudioOutput) {
        QLOG_STR("RingtonePlayer:: notify stopped player ") ; 
        iAudioOutput->stop() ; 
    }
}

// from QIODevice:
qint64 RingtonePlayer::writeData ( const char * /*data*/, qint64 /*maxSize*/ ) {
    return 0 ;
}

// From QIODevice: audio data goes to sound card via this method
qint64 RingtonePlayer::readData ( char * data, qint64 maxSize ) {
    qint64 bytesToWrite = maxSize; // bytes to write into buffer of player
    iMutex->lock() ; 
    if ( iAudioBuffer.size() < bytesToWrite ) {
        bytesToWrite = iAudioBuffer.size() ;
    }
    if ( bytesToWrite > 0 ) {
        memcpy(data, iAudioBuffer.constData(), bytesToWrite) ;
        iAudioBuffer.remove(0,bytesToWrite) ; 
        QLOG_STR("RingtonePlayer::readData " + 
                 QString::number(bytesToWrite) + " asked " + 
                 QString::number(maxSize)   ) ; 
    }
    iMutex->unlock() ; 
    return bytesToWrite ;
}

// from QIODevice:
bool RingtonePlayer::open ( OpenMode mode ) {
    if ( mode & QIODevice::ReadOnly ) {
        return QIODevice::open(mode) ; 
    } else {
        return false ; 
    }
}

void RingtonePlayer::timerEvent(QTimerEvent* /* event */ ) {
    if ( iNeedsToRun == false &&         
         iAudioOutput) {
        QLOG_STR("RingtonePlayer:: timerEvent stopped player ") ; 
        iAudioOutput->stop() ; 
    } else {
        iMutex->lock() ; 
        int audioBufferSize = iAudioBuffer.size() ;
        iMutex->unlock() ; 
        while (audioBufferSize <= (KPlayerBufferSize*2) ) {
            if ( iRingtoneFile == NULL || iRingtoneFile->atEnd()== true ) {
                initInput() ; 
            }
            QByteArray audioCompressed ; 
            if ( iRingtoneFile && iRingtoneFile->isOpen() ) {
                qint32 bytesToReadNetworkBo ( 0 ) ; 
                if ( iRingtoneFile->read((char *)&bytesToReadNetworkBo,
                                         sizeof(quint32)) == sizeof(quint32)) {
                    qint32 bytesToRead ( ntohl(bytesToReadNetworkBo ) ) ; 
                    audioCompressed.append ( iRingtoneFile->read(bytesToRead) ) ;
                }
            }
            if ( audioCompressed.size() && iDecoder ) {
                int resultLen = opus_decode(iDecoder, 
                                            (const unsigned char*)(audioCompressed.constData()), 
                                            audioCompressed.size() , 
                                            (qint16*)(iDecodedData.data()), 
                                            iDecodedData.size(),
                                            0);
                if (resultLen < 0) {
                    QLOG_STR( "opus_decode() failed with error code: " +
                              QString::number(resultLen));
                } else {
                    iMutex->lock() ; 
                    iAudioBuffer.append(iDecodedData.mid(0,resultLen*sizeof(qint16)));
                    if (iAudioOutput && 
                        iNeedsToRun &&
                        iAudioOutput->state() == QAudio::SuspendedState &&
                        iAudioBuffer.size() > KPlayerBufferSize ) {
                        iAudioOutput->resume() ;
                    } else if (iAudioOutput && 
                               iNeedsToRun &&
                               iAudioOutput->state() == QAudio::StoppedState &&
                               iAudioBuffer.size() > KPlayerBufferSize ) {
                        iAudioOutput->start(this) ;
                    }
                    iMutex->unlock() ; 
                }
            }
            iMutex->lock() ; 
            audioBufferSize = iAudioBuffer.size() ;
            iMutex->unlock() ; 
        }
    }
}

void RingtonePlayer::initInput() {
    if ( iRingtoneFile == NULL ) {
        switch ( iModel.getRingtoneSetting() ) {
        default: // also the default
        case Model::BowRingTone:
            QLOG_STR("Opening :/ui/ui/bow.rawopus") ; 
            iRingtoneFile = new QFile(":/ui/ui/bow.rawopus") ; // in resource
            break ; 
        case Model::ElectricalRingTone:
            QLOG_STR("Opening :/ui/ui/electrical.rawopus") ; 
            iRingtoneFile = new QFile(":/ui/ui/electrical.rawopus") ; 
            break ; 
        case Model::AcousticRingTone :
            QLOG_STR("Opening :/ui/ui/acoustic.rawopus") ; 
            iRingtoneFile = new QFile(":/ui/ui/acoustic.rawopus") ; 
            break ;
        case Model::BeepRingTone :
            QLOG_STR("Opening :/ui/ui/beep.rawopus") ; 
            iRingtoneFile = new QFile(":/ui/ui/beep.rawopus") ; 
            break ;
        }
        if ( iRingtoneFile && iRingtoneFile->open(QIODevice::ReadOnly) ) {
            QLOG_STR("Ringtone file open") ; 
        } else {
            QLOG_STR("Ringtone file NOT open!!, error = " + 
                     QString::number(iRingtoneFile==NULL ? -999 : iRingtoneFile->error()) + 
                     " " + 
                     ( iRingtoneFile==NULL ? " " : iRingtoneFile->errorString() ) ) ; 
            deleteLater() ;
            iNeedsToRun = false ; 
        }
    } else {
        iRingtoneFile->seek(0) ; 
        QLOG_STR("Ringtone file seeked") ; 
    }
    int errorCode ; 
    if( iDecoder == NULL ) {
        iDecoder = opus_decoder_create(KRingtoneSampleRate,
                                       1,
                                       &errorCode ); 
        if ( errorCode != OPUS_OK || iDecoder == NULL ) {
            QLOG_STR( "opus_decoder_create " + QString::number(errorCode)) ; 
        }
        iDecodedData.fill ( 0, KRingtoneBufSize ) ; 
        QLOG_STR("Ringtone decoder created") ; 
    } else {
        opus_decoder_init(iDecoder,KRingtoneSampleRate,1) ; 
        QLOG_STR("Ringtone decoder reset") ; 
    }
}

void RingtonePlayer::callStatusChanged(quint32 /* aCallId */,
                                       VoiceCallEngine::CallState aState) {
    if ( aState != MVoiceCallEngine::Incoming ) {
        stop() ; 
    }
}
