/* -*-C++-*- -*-coding: utf-8-unix;-*-
  Classified Ads is Copyright (c) Antti Järvinen 2013-2016.

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

#ifndef RINGTONE_PLAYER_H
#define RINGTONE_PLAYER_H

#include <QByteArray>
#include <QAudioOutput>
#include <QIODevice>
#include <QTimer>
#include <opus/opus.h> // for data types
#include "../net/voicecallengine.h" // for MCallStatusObserver
#include "../mcontroller.h"
#include "../datamodel/model.h"

class QFile ; 
class QMutex ; 

/**
 * @brief Class for alerting user in case of incoming call
 * 
 * Class that plays selected ringtone or remains silent, 
 * depending on users selections. 
 */
class RingtonePlayer : public QIODevice,
                       public VoiceCallEngine::MCallStatusObserver {
    Q_OBJECT

public:
    /**
     * Constructor. When constructed, immediately begins to 
     * play. There is method for stop, and there is destructor.  
     * Destructor should not be called, calling stop will cause 
     * this class to deleteLater() itself. 
     */
    RingtonePlayer(Model& aModel,
                   MController& aController ) ;
    ~RingtonePlayer() ;
    /**
     * From QObject. Used for feeding ringtone data to audio device
     */
    void timerEvent(QTimerEvent *event);
    /**
     * Method that communicates changes in call state. From 
     * interface MCallStatusObserver.
     */
    virtual void callStatusChanged(quint32 aCallId,
                                   VoiceCallEngine::CallState aState) ; 

// next methods inherited from QIODevice:
public: 
    virtual qint64 bytesAvailable () const {
        return iAudioBuffer.size() + QIODevice::bytesAvailable(); 
    }
protected:
    /**
     * Write-method of QIODevice - QIODevice here is used for
     * feeding the player and data is actually written using
     * @ref insertAudioFrame method so implementation of 
     * this method is actually empty. 
     * 
     * @param data that will not be written anywhere.
     * @param maxSize max data size to be not written
     *
     * @return always 0 
     */
    virtual qint64 writeData ( const char * data, qint64 maxSize ) ;
    /**
     * From QIODevice: method that QAudioOutput class instance will
     * use to read the actual audio data. Data comes from internal
     * buffer and is inserted into buffer using @ref insertAudioFrame.  
     *
     * @param data pointer to buffer where data will be written
     * @param maxSize max data size available in buffer
     *
     * @return number of bytes actually written into buffer
     */
    virtual qint64 readData ( char * data, qint64 maxSize ) ;
    /**
     * method for opening QIODevice: sets the open mode that's about it.
     */
    virtual bool open ( OpenMode mode ) ;
    /*
     * method for opening QIODevice: tells that this is no random-access
     * device.
     *
     * @return true always 
     */
    virtual bool isSequential () const {
        return true ; // is not random-access, 
    }
signals:
    /**
     * this is not method but signal ; if in error, get emit()ted
     */
    void error(int aError,
               const QString& aExplanation) ;

public slots:
    /**
     * Notify slot from actual player device 
     */
    virtual void finishedPlaying ( QAudio::State aState ); 

    /**
     * notify from audio output device
     */
    virtual void notify() ;
    /**
     * method for stopping playback 
     */
    void stop() ; 
private: // methods
    /**
     * initializes input and codec 
     */
    void initInput() ; 
private: // members
    QAudioFormat iFormat;
    QAudioOutput* iAudioOutput ; 
    QByteArray iAudioBuffer ; /**< first buffered here, then written in chunks */
    bool iNeedsToRun ;
    QFile* iRingtoneFile ; 
    int iTimerId ; 
    OpusDecoder* iDecoder ; /**< ringtones are also opus-coded */
    QByteArray iDecodedData ; 
    QMutex* iMutex ; 
    Model& iModel ;
    MController& iController ; 
} ;
#endif
