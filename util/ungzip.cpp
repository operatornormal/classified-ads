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

static const unsigned KZLibBlockSize ( 1024 ) ; 

#include "ungzip.h"
#include <zlib.h> // actual gzip functions and data-types
#include "log.h"

UnGZip::UnGZip() {
    // not really used
}

UnGZip::~UnGZip() {

}

// static method, idea borrowed from zlib code examples.
QByteArray UnGZip::unGZip(const QByteArray& aCompressedContent,
                          bool* aResult ) {
    QByteArray retval ; 
    if ( aCompressedContent.length() < 5 ) {
        if ( aResult ) {
            *aResult = false ; 
        }
        return retval ; 
    }
    z_stream stream ; 
    char uncompressedBlock[KZLibBlockSize] ; 
    
    // initialize zlib stream:
    stream.zalloc = Z_NULL ; 
    stream.zfree = Z_NULL ; 
    stream.opaque = Z_NULL ; 
    stream.avail_in = aCompressedContent.length() ;
    stream.next_in = const_cast<Bytef *>
        (reinterpret_cast<const Bytef *>
         (aCompressedContent.data())) ;
    if ( inflateInit2(&stream, 15 +  32) != Z_OK ) {
        if ( aResult ) {
            *aResult = false ; 
        }
        return retval ; 
    }

    do {
        stream.avail_out = KZLibBlockSize ;
        stream.next_out = reinterpret_cast<Bytef *>(uncompressedBlock) ;
        
        switch ( inflate(&stream, Z_NO_FLUSH) ) {
        case Z_NEED_DICT:
        case Z_DATA_ERROR:
        case Z_MEM_ERROR:
            inflateEnd(&stream) ; 
            if ( aResult ) {
                *aResult = false ; 
            }
            return retval ; 
            break ; // never reached
        default:
            // grab content and continue
            retval.append(uncompressedBlock, 
                          KZLibBlockSize - stream.avail_out);
            break ; 
        }
    } while ( stream.avail_out == 0 ) ;
    
    // all done:
    inflateEnd(&stream) ; 
    if ( aResult ) {
        *aResult = true ; 
    }
    return retval ; 
}
