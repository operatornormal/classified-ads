/*                                      -*-C++-*-
    Classified Ads is Copyright (c) Antti Jarvinen 2013.

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

#include "hash.h"
#include <openssl/sha.h>
#include <string.h>
#include "../log.h"
#include <limits.h>
#include <QVariant>

Hash::Hash() {
  iHash160bits[0] =
    iHash160bits[1] =
      iHash160bits[2] =
        iHash160bits[3] =
          iHash160bits[4] = 0 ;
}

Hash::Hash(const quint32 aHash160bits[KNumberOfIntsInHash]) {
  iHash160bits[0] = aHash160bits[0] ;
  iHash160bits[1] = aHash160bits[1] ;
  iHash160bits[2] = aHash160bits[2] ;
  iHash160bits[3] = aHash160bits[3] ;
  iHash160bits[4] = aHash160bits[4] ;
}

Hash::Hash(const quint32 aBits1,
           const quint32 aBits2,
           const quint32 aBits3,
           const quint32 aBits4,
           const quint32 aBits5) {
  iHash160bits[0] = aBits1 ;
  iHash160bits[1] = aBits2 ;
  iHash160bits[2] = aBits3 ;
  iHash160bits[3] = aBits4 ;
  iHash160bits[4] = aBits5 ;
}

Hash::Hash(const unsigned char aDigest[KNumberOfIntsInHash*4]) {
  setFromCharArray(aDigest) ;
}

//
// constructor that takes the hash in ascii string similar that
// printf("%X... produces.
//
Hash::Hash(const char* aHexString, int aHexStringLen) {
  char single_number[9] ;
  if ( aHexStringLen != 40 ) {
    return ;
  }
  quint32 h1 ;
  strncpy(single_number, &(aHexString[0]), 8 ) ;
  sscanf(single_number, "%X", &h1) ;
  quint32 h2 ;
  strncpy(single_number, &(aHexString[8]), 8 ) ;
  sscanf(single_number, "%X", &h2) ;
  quint32 h3 ;
  strncpy(single_number, &(aHexString[16]), 8 ) ;
  sscanf(single_number, "%X", &h3) ;
  quint32 h4 ;
  strncpy(single_number, &(aHexString[24]), 8 ) ;
  sscanf(single_number, "%X", &h4) ;
  quint32 h5 ;
  strncpy(single_number, &(aHexString[32]), 8 ) ;
  sscanf(single_number, "%X", &h5) ;
  iHash160bits[0] = h1;
  iHash160bits[1] = h2 ;
  iHash160bits[2] = h3 ;
  iHash160bits[3] = h4 ;
  iHash160bits[4] = h5 ;
}

Hash::Hash(const QSslCertificate& aSslCert) {
  QByteArray d = aSslCert.digest(QCryptographicHash::Sha1) ;
  setFromCharArray((unsigned char *)(d.data())) ;
}

void Hash::fromString(const unsigned char* aBuf) {
  if ( strlen((const char*)aBuf) == 8*5) {
    unsigned int b1,b2,b3,b4,b5 ; // to make compiler happy
    char str1[9] = {0} ;
    strncpy(str1,(const char*)&(aBuf[8*0]), 8) ;
    char str2[9] = {0} ;
    strncpy(str2,(const char*)&(aBuf[8*1]), 8) ;
    char str3[9] = {0} ;
    strncpy(str3,(const char*)&(aBuf[8*2]), 8) ;
    char str4[9] = {0} ;
    strncpy(str4,(const char*)&(aBuf[8*3]), 8) ;
    char str5[9] = {0} ;
    strncpy(str5,(const char*)&(aBuf[8*4]), 8) ;

    if ( sscanf(str1, "%X", &b1) == 1 &&
	 sscanf(str2, "%X", &b2) == 1 && 
	 sscanf(str3, "%X", &b3) == 1 &&
	 sscanf(str4, "%X", &b4) == 1 &&
	 sscanf(str5, "%X", &b5) == 1 ) {
      iHash160bits[0] = b1 ;
      iHash160bits[1] = b2 ;
      iHash160bits[2] = b3 ;
      iHash160bits[3] = b4 ;
      iHash160bits[4] = b5 ;
    } else {
      iHash160bits[0] = iHash160bits[1] = iHash160bits[2] =  iHash160bits[3] =  iHash160bits[4] = 0 ; 
    }
  }
}

QString Hash::toString() const {
  char cStyle  [KNumberOfIntsInHash*8+1] ;
  sprintf(cStyle, "%.8X%.8X%.8X%.8X%.8X",
          iHash160bits[0],
          iHash160bits[1],
          iHash160bits[2],
          iHash160bits[3],
          iHash160bits[4]) ;
  return QString(cStyle);
}

void Hash::fromQVariant(const QVariant& aQVariantHashValue) {
  int j = 0 ; 
  QListIterator<QVariant> i(aQVariantHashValue.toList());
  while (i.hasNext() && j < 5) {
    iHash160bits[j++] = i.next().toUInt() ; 
  }
}
QVariant Hash::toQVariant() const {
  QVariantList l ; 
  l.append(iHash160bits[0]) ; 
  l.append(iHash160bits[1]) ; 
  l.append(iHash160bits[2]) ; 
  l.append(iHash160bits[3]) ; 
  l.append(iHash160bits[4]) ; 
  return l ; 
}

void Hash::calculate(const QByteArray& aBuf) {
  if ( aBuf.size() == 0 ) {
    iHash160bits[0] = 0 ;
    iHash160bits[1] = 0 ;
    iHash160bits[2] = 0 ;
    iHash160bits[3] = 0 ;
    iHash160bits[4] = 0 ;
  } else {
    unsigned char hashbytearray[KNumberOfIntsInHash*4] ;
    SHA1((const unsigned char *)(aBuf.constData()), aBuf.size(), hashbytearray) ;
    setFromCharArray(hashbytearray) ;
  }
}

void Hash::setFromCharArray(const unsigned char aDigest[KNumberOfIntsInHash*4]) {
  iHash160bits[0] =
    aDigest[3] |
    aDigest[2] << 8 |
    aDigest[1] << 16 |
    aDigest[0] << 24 ;

  iHash160bits[1] =
    aDigest[7] |
    aDigest[6] << 8 |
    aDigest[5] << 16 |
    aDigest[4] << 24 ;

  iHash160bits[2] =
    aDigest[11] |
    aDigest[10] << 8 |
    aDigest[9] << 16 |
    aDigest[8] << 24 ;

  iHash160bits[3] =
    aDigest[15] |
    aDigest[14] << 8 |
    aDigest[13] << 16 |
    aDigest[12] << 24 ;

  iHash160bits[4] =
    aDigest[19] |
    aDigest[18] << 8 |
    aDigest[17] << 16 |
    aDigest[16] << 24 ;

}

bool Hash::operator== (const Hash& aHashToCompare) const {
  if ( iHash160bits[4] == aHashToCompare.iHash160bits[4] &&
       iHash160bits[3] == aHashToCompare.iHash160bits[3] &&
       iHash160bits[2] == aHashToCompare.iHash160bits[2] &&
       iHash160bits[1] == aHashToCompare.iHash160bits[1] &&
       iHash160bits[0] == aHashToCompare.iHash160bits[0] ) {
    return true ;
  } else {
    return false ;
  }
}

bool Hash::operator!= (const Hash& aHashToCompare) const {
  if ( iHash160bits[4] != aHashToCompare.iHash160bits[4] ||
       iHash160bits[3] != aHashToCompare.iHash160bits[3] ||
       iHash160bits[2] != aHashToCompare.iHash160bits[2] ||
       iHash160bits[1] != aHashToCompare.iHash160bits[1] ||
       iHash160bits[0] != aHashToCompare.iHash160bits[0] ) {
    return true ;
  } else {
    return false ;
  }
}

bool Hash::operator< (const Hash& aHashToCompare) const {
  if( iHash160bits[0] < aHashToCompare.iHash160bits[0] ) {
    return true ;
  } else if ( iHash160bits[0] > aHashToCompare.iHash160bits[0] ) {
    return false ;
  } else if( iHash160bits[1] < aHashToCompare.iHash160bits[1] ) {
    return true ;
  } else if ( iHash160bits[1] > aHashToCompare.iHash160bits[1] ) {
    return false ;
  } else if( iHash160bits[2] < aHashToCompare.iHash160bits[2] ) {
    return true ;
  } else if ( iHash160bits[2] > aHashToCompare.iHash160bits[2] ) {
    return false ;
  } else if( iHash160bits[3] < aHashToCompare.iHash160bits[3] ) {
    return true ;
  } else if ( iHash160bits[3] > aHashToCompare.iHash160bits[3] ) {
    return false ;
  } else if( iHash160bits[4] < aHashToCompare.iHash160bits[4] ) {
    return true ;
  } else {
    return false ;
  }
}

bool Hash::operator> (const Hash& aHashToCompare) const {
  return ( ! ( *this < aHashToCompare ) ) ;
}

Hash& Hash::operator= (const Hash& aHashToSubstitute) {
  for ( int i = 0 ; i < KNumberOfIntsInHash ; i++ ) {
    iHash160bits[i] = aHashToSubstitute.iHash160bits[i] ;
  }
  return *this ;
}

Hash Hash::distanceFrom (const Hash& aHash) {
  // we actually here need to calculate distance
  // twice and then return the smaller value
  Hash retvalCandidateBySubstraction ;
  Hash retvalCandidateByRollingOver ;

  // so first, without rolling over, substract the bigger from
  // the smaller and that is one distance ; must compare here
  // because our substraction operator will mis-behave if
  // substracting bigger from smaller
  if ( *this > aHash ) {
    retvalCandidateBySubstraction = *this - aHash ;
  } else {
    retvalCandidateBySubstraction = aHash - *this ;
  }

  // then roll-over part
  // roll-over point ; this is the biggest possible hashcode
  Hash maxHash = Hash(UINT_MAX,
                      UINT_MAX,
                      UINT_MAX,
                      UINT_MAX,
                      UINT_MAX) ;
  // then calculate how far away we're from maxhash:
  if ( *this > aHash ) {
    // so "this" is bigger so it is closer to maxhash than aHash
    Hash distanceFromMaxHash = maxHash - *this ;
    // then add distance from zero-hash to hHash
    retvalCandidateByRollingOver = distanceFromMaxHash + aHash ;
  } else {
    // so hHash is bigger so it is closer to maxhash
    Hash distanceFromMaxHash = maxHash - aHash ;
    // then add distance from zero-hash to hHash
    retvalCandidateByRollingOver = distanceFromMaxHash + *this ;
  }


  // then return smaller
  if ( retvalCandidateByRollingOver < retvalCandidateBySubstraction ) {
    return retvalCandidateByRollingOver ;
  } else {
    return retvalCandidateBySubstraction ;
  }
}

Hash Hash::operator-(const Hash& aToBeSubstracted) const {
  Hash retval ;
  // need to have temporary copy because borrowing operation
  // needs to modify upper-significance bits
  Hash temporary_copy = *this ;
  // so here we play a trick that seems to work.
  // substract digit by digit. our digits in hash are
  // unsigned ints e.g. we're doing substraction in "normal"
  // pen-and-paper manner but our base is 2^32 instead of 10.
  // this way we'll substract the 160-bit hash by looping 5 times.
  for(int i=(KNumberOfIntsInHash-1) ; i>=0 ; i-- ) {
    if ( temporary_copy.iHash160bits[i] >= aToBeSubstracted.iHash160bits[i] ) {
      retval.iHash160bits[i] = temporary_copy.iHash160bits[i] -
                               aToBeSubstracted.iHash160bits[i] ;
    } else {
      // needs to borrow ; find next non-zero digit
      for ( signed char borrow_digit = i-1 ; borrow_digit >= 0 ; borrow_digit-- ) {
        if ( temporary_copy.iHash160bits[borrow_digit] > 0 ) {
          temporary_copy.iHash160bits[borrow_digit]-- ;
          break ;
        } else {
          // while borrowing, the digits that are zero
          // while borrowing must be set 0xFFFFFFFF
          temporary_copy.iHash160bits[borrow_digit] = UINT_MAX ;
        }
      }
      // borrow done, now substract ; now we'll need to substract
      // ( 0xFF + temporary_copy.bits[i] ) -  aToBeSubstracted.bits[i]
      // obviously the 0xFF + temporary_copy.bits[i] will not fit our bits
      // but here it is ok to roll over, looks like we obtain correct result.
      retval.iHash160bits[i] = temporary_copy.iHash160bits[i]-
                               aToBeSubstracted.iHash160bits[i] ;
    }
  }
  return retval ;
}

Hash Hash::operator+(const Hash& aToBeAdded) const {
  Hash retval ;
  int carry_digit ;
  Hash temporary_copy = *this ;
  for(int i=(KNumberOfIntsInHash-1) ; i>=0 ; i-- ) {
    if ( aToBeAdded.iHash160bits[i] >
         UINT_MAX - temporary_copy.iHash160bits[i] ) {
      // need to overflow to next digit:
      carry_digit = i-1 ;
      while ( carry_digit > 0 &&
              temporary_copy.iHash160bits[carry_digit] == UINT_MAX ) {
        temporary_copy.iHash160bits[carry_digit] ++ ;
        carry_digit-- ;
      }
      temporary_copy.iHash160bits[carry_digit] ++ ;
      // ok, we stopped in place, where temporary_copy.iHash160bits[i]
      // is not max value, increment at that position
    }
    retval.iHash160bits[i] = aToBeAdded.iHash160bits[i] + temporary_copy.iHash160bits[i] ;
  }
  return retval ;
}
