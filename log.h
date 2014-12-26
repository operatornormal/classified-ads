#ifndef CLASSIFIED_ADS_LOG_H
#define CLASSIFIED_ADS_LOG_H
#include <stdio.h>
#include <QDateTime>
#ifdef DEBUG
#define LOG_STR(a)  qDebug() << QDateTime::currentDateTime().toString("hh:mm:ss") << a
#define LOG_STR2(a,b) { QString str ; qDebug() << QDateTime::currentDateTime().toString("hh:mm:ss") << str.sprintf(a,b) ; }
#define QLOG_STR(a) qDebug() << QDateTime::currentDateTime().toString("hh:mm:ss") << a
#else
#define LOG_STR(a)
#define LOG_STR2(a,b)
#define QLOG_STR(a)
#endif
#endif
