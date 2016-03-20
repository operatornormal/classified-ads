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

#ifndef AUDIO_SOURCE_H
#define AUDIO_SOURCE_H

#include <QAudioFormat> 
#include <QAudio> // type definitions 
#include <QByteArray> 
#include <QTimer>
#include <QIODevice>

class QAudioInput ;
class Model ; 

/**
 * @brief opposite of audiosink. 
 *
 * Class for capturing audio. This class provides 16-bit
 * PCM digital audio in 8kHz sample rate in pieces of
 * 480 samples, each exactly 60ms long. Output format is
 * in floating point numbers in range [0,1]. 
 */
class AudioSource : public QIODevice {
    Q_OBJECT

public:
    /**
     * Constructor.
     */
    AudioSource(Model& aModel) ;
    ~AudioSource() ;
    /**
     * getter-method for next sequence number 
     */
    quint32 getCurrentSeqNo() const { return iSeqNo ; } ; 
    /**
     * method for stopping capture
     */
    void stopCapturing() ; 
// next methods inherited from QIODevice:
protected:
    /**
     * From QIODevice: method where audio data will be written
     * from actual audio source: this class AudioSource inherits
     * QIODevice and will offer itself as data sink of iAudioInput
     * @param data audio data
     * @param maxSize max data size to be written
     * @return number of bytes actually written
     */
    virtual qint64 writeData ( const char * data, qint64 maxSize ) ;
    /**
     * From QIODevice: in current implementation this method 
     * does nothing, audio is processed internally and this
     * QIODevice implementation does not allow reading.
     * @return 0 always
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
    /**
     * From QObject. Used for forwarding audio from buffer.
     */
    void timerEvent(QTimerEvent *event);
signals:
    /**
     * Emitted when complete audio frame is ready to be processed
     * @param aFrame is the audio data. Content of the byte array is
     *        floats in range [-1,1]
     * @param aSeqNo sequence number of the captured frame, it will
     *        sequentially increase as time passes 
     */
    void frameCaptured(const QByteArray& aFrame,quint32 aSeqNo) ;
    /**
     * This is actually average audio volume, in range [0,1] 
     */
    void audioMaxLevel(float aMaxVolume) ; 
public slots:
   /**
    * Method called by timer every now and then that does processing
    * of captured data 
    */
    void processCapturedAudio() ; 
  /**
   * Input status changed 
   */
  void inputStatusChanged(QAudio::State aState) ; 
private: // members
    QAudioFormat iFormat;
    QAudioInput* iAudioInput ; 
    QByteArray iAudioBuffer ; 
    quint32 iSeqNo ; 
    bool iNeedsToRun ; 
    Model& iModel ; 
    int iTimerId ; 
} ;
#endif
