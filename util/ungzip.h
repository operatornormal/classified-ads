/*    -*-C++-*- -*-coding: utf-8-unix;-*-
  Classified Ads is Copyright (c) Antti Järvinen 2013-2017.

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

#ifndef CLASSIFIED_UNGZIP_H
#define CLASSIFIED_UNGZIP_H

#include <QObject>

/**
 * @brief Class for un-zipping gzip'ed content
 *
 * This class is wrapper around zlib, it is used to de-compress
 * contents previously compressed with gzip. 
 */
class UnGZip : public QObject {
    Q_OBJECT

public:
    UnGZip() ;
    ~UnGZip() ;

    /**
     * Method for doing de-gzip. 
     * 
     * @param aCompressedContent compressed content
     * @param aResult if not-null, will have its value set to true/false
     *                depending on success of de-compression
     * @return un-compressed content.
     */
    static QByteArray unGZip(const QByteArray& aCompressedContent,
                             bool* aResult ) ;
} ;

#endif
