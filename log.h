/*     -*-C++-*- -*-coding: utf-8-unix;-*-
  Classified Ads is Copyright (c) Antti Järvinen 2013-2022.

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

#ifndef CLASSIFIED_ADS_LOG_H
#define CLASSIFIED_ADS_LOG_H
#include <stdio.h>

#include <QDateTime>
#ifdef DEBUG
#include <QDebug>
#define LOG_STR(a) \
  qDebug() << QDateTime::currentDateTime().toString("hh:mm:ss") << a
#define LOG_STR2(a, b)                                            \
  {                                                               \
    QString str;                                                  \
    qDebug() << QDateTime::currentDateTime().toString("hh:mm:ss") \
             << str.asprintf(a, b);                               \
  }
#define QLOG_STR(a) \
  qDebug() << QDateTime::currentDateTime().toString("hh:mm:ss") << a
#else
#define LOG_STR(a)
#define LOG_STR2(a, b)
#define QLOG_STR(a)
#endif
#endif
