/*     -*-C++-*- -*-coding: utf-8-unix;-*-
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

#include <QtGui>
#include <QMessageBox>
#include "../mcontroller.h"
#include "../log.h"
#include "../datamodel/model.h"
#include "callstatus.h"
#include "callbuttondelegate.h"

CallStatusDialog::CallStatusDialog(QWidget *aParent,
                                   MController& aController)
    : QDialog(aParent),
      iController(aController),
      iCallButtonDelegate(NULL) {
    ui.setupUi(this) ;
    VoiceCallEngine* eng (iController.voiceCallEngine()) ;
    if ( eng ) {
        eng->installObserver(this) ;
        connect ( eng, SIGNAL( inputLevel(float) ),
                  this, SLOT( audioInputLevel(float) ),
                  Qt::QueuedConnection ) ;
        connect ( eng, SIGNAL( outputLevel(float) ),
                  this, SLOT( audioOutputLevel(float) ),
                  Qt::QueuedConnection ) ;
    }
    iCallButtonDelegate = new CallButtonDelegate(*eng) ;
    ui.callsView->setItemDelegateForColumn ( 3, iCallButtonDelegate ) ;
    ui.callsView->setItemDelegateForColumn ( 4, iCallButtonDelegate ) ;
    ui.callsView->setModel(eng) ;
    ui.callsView->horizontalHeader()->setStretchLastSection(true);
    setMinimumWidth(550) ;
    adjustSize() ;
    ui.inputLevelSlider->setMinimum(0) ;
    ui.outputLevelSlider->setMinimum(0) ;
    ui.inputLevelSlider->setMaximum(100) ; // slider works with integers
    ui.outputLevelSlider->setMaximum(100) ;
    connect (this, SIGNAL(rejected()), this, SLOT(deleteLater())) ;
}

CallStatusDialog::~CallStatusDialog() {
    LOG_STR("CallStatusDialog::~CallStatusDialog\n") ;
    ui.callsView->setModel(NULL) ;
    VoiceCallEngine* eng (iController.voiceCallEngine()) ;
    if ( eng ) {
        eng->removeObserver(this) ;
    }
    delete iCallButtonDelegate ;
}


void CallStatusDialog::callStatusChanged(quint32 /*aCallId*/,
        VoiceCallEngine::CallState aState) {
    QLOG_STR("CallStatusDialog::callStatusChanged " +
             QString::number(aState)) ;
    if ( aState == VoiceCallEngine::Closed ) {
        VoiceCallEngine* eng (iController.voiceCallEngine()) ;
        if ( eng && eng->rowCount() == 0 ) {
            reject() ;
        }
    }
}

void  CallStatusDialog::audioInputLevel(float aInputLevel) {
    // slider range is 0-100 integer
    ui.inputLevelSlider->setValue(aInputLevel*100) ;
}

void  CallStatusDialog::audioOutputLevel(float aOutputLevel) {
    // slider range is 0-100 integer
    ui.outputLevelSlider->setValue(aOutputLevel*100) ;
}
