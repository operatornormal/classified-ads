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

#ifndef AUDIO_PLAYER_H
#define AUDIO_PLAYER_H

#include <QObject>
#include <QByteArray>
#include <QAudioOutput>
#include <QIODevice>

class Model ; 

/**
 * @brief Class for playback of audio stream, via speaker or other audio dev.
 * 
 *
 * This class encapsulates audio output device and expects input 
 * as array of floats.
 */
class AudioPlayer : public QIODevice {
    Q_OBJECT

public:
    /**
     * Constructor. Currently only samplesize 16 is supported,
     * all other sizes will output only noise. Also see input data
     * format at @ref insertAudioFrame method. 
     * 
     * @param aFrequency is number of samples per second
     * @param aNumChannels is number of channels in stream
     * @param aSampleSize is number of bits in each sample.
     *        As currently the audio format is in array of floats
     *        that are anyway in native format and in [-1,1] range
     *        this parameter is a little bit meaningless. 
     */
    AudioPlayer(Model& aModel,
                int aFrequency = 8000,
                int aNumChannels = 1,
                int aSampleSize = 16 ) ;
    ~AudioPlayer() ;
    /**
     * method for stopping playback 
     */
    void stop() ; 
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
    /**
     * signals that communicates max value of a outputted frame
     */
    void audioMaxLevel(float aMaxVolume) ; 
public slots:
    /**
     * Method/slot for playing back snippet of audio.
     *
     * @param aFrame 32-bit, native-encoding, floating point format 
     *        audio/pcm data in 8kHz. Floating point range in [-1,1]. 
     */
    void insertAudioFrame ( const QByteArray& aFrame ); 

    /**
     * Notify slot from actual player device 
     */
    void finishedPlaying ( QAudio::State aState ); 

    /**
     * notify from audio output device
     */
    void notify() ;

private: // members
    QAudioFormat iFormat;
    QAudioOutput* iAudioOutput ; 
    QByteArray iAudioBuffer ; /**< first buffered here, then written in chunks */
    bool iNeedsToRun ;
    Model& iModel ; /** datamodel */
    const int iFrequency ; 
    const int iNumChannels ;
    const int iSampleSize ; 
} ;
#endif
