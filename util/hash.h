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

#ifndef HASH_H
#define HASH_H

#include <QSslCertificate>


/**
 * @brief Hash is class that carries 160-bit digest.
 *
 * Hash number class. This is usually an encryption
 * key fingerprint or something returned by SHA1
 * function. The number that this class carries
 * identifies an object inside classified ads, such
 * objects as "classified ad", "user", "node",
 * "attachment file" or anything that is present in
 * the network. Here the hash is number between 0-2^160.
 */
class Hash {
public:
  /**
   * This constant KNumberOfIntsInHash defines size of
   * our hash ; we take this many unsigned ints each
   * 32bits in size so with value 5 w get 5*32=160.
   */
  const static int KNumberOfIntsInHash = 5 ;
  /**
   * constructor. produces zero-value hash.
   */
  Hash() ;
  /**
   * constructor.
   * @param aHash160bits is array of KNumberOfIntsInHash
   *        containing 32 bits each. The most-significant
   *        bits come in first, least-significant come in
   *        last. .. e.g. for constructing a hash
   *        that has value (decimal) 1  you say
   *        Hash(0,0,0,0,1).
   */
  Hash(const quint32 aHash160bits[KNumberOfIntsInHash]) ;
  /**
   * constructor.
   * @param aBits1 most significant bits
   * @param aBits2 quite significant bits
   * @param aBits3 somewhat significant bits
   * @param aBits4 less significant bits
   * @param aBits5 least significant bits
   */
  Hash(const quint32 aBits1,
       const quint32 aBits2,
       const quint32 aBits3,
       const quint32 aBits4,
       const quint32 aBits5) ;
  /**
   * constructor.
   * @param aDigest 160 bits in array of unsigned 8-bit octets.
   *        Most significant bits first. If you take SHA1 function
   *        of openssl library, this is the format the method returns.
   */
  Hash(const unsigned char aDigest[KNumberOfIntsInHash * 4]) ;
  /**
   * constructor that takes the hash in ascii string similar that
   * printf("%X... produces.
   * @param aHexString is the string containing hash value
   * @param aHexStringLen is length of said string.
   *        if this param does not have value of 40
   *        then a null hash will be instantiated.
   *        Purpose of this parameter is mainly to
   *        clearly distinguis this constuctor from the
   *        one that has single "unsigned char *" param
   *        but expects different lenght for the string
   */
  Hash(const char* aHexString, int aHexStringLen) ;
  /**
   * constructor
   * @param aSslCert is ssl certificate. Its SHA1 digest is used
   *        to initialize value of this hash
   */
  Hash(const QSslCertificate& aSslCert) ;

  /**
   * Sets hash value from string produced by @ref toString() method
   * of this same class.
   */
  void fromString(const unsigned char *aBuf) ;

  /**
   * Returns the hash value as a string.
   */
  QString toString() const ;

  /**
   * Sets hash value from QVariant produced by @ref toQVariant() method
   * of this same class.
   */
  void fromQVariant(const QVariant& aQVariantHashValue) ;

  /**
   * Returns the hash value as a QVariant
   */
  QVariant toQVariant() const ;

  /**
   * calculates SHA1 hash of data and sets value of this
   * class instance to be the result
   */
  void calculate(const QByteArray& aBuf) ;
  /**
   * Comparison operator enabling us to use sort operations
   */
  bool operator== (const Hash& aHashToCompare) const ;
  bool operator!= (const Hash& aHashToCompare) const ;
  bool operator< (const Hash& aHashToCompare) const ;
  bool operator> (const Hash& aHashToCompare) const ;

  /**
   * substitution too, it might make code more readable
   */
  Hash& operator= (const Hash& aHashToSubstitute) ;

  /**
   * Method for calculating distance between 2 hashes.
   *
   * Obligatory J.R.R. Tolkien quote belongs here.
   *
   * In this program distance is defined in "number of rings
   * to rule them all" meaning that
   * 1) hash values roll over e.g. distance between
   *    Hash(0) and Hash(max value) is 1. Distance between
   *    Hash(1) and Hash(max value) is 2. Distance between
   *    Hash(1) and Hash(5) is 4. As is to be expected.
   * 2) We may invent many rings to address same objects
   *    because from articles (for instance) we can record
   *    the article content hash, pgp-key fingerprint of
   *    the sender, node used to send, hash of the forum
   *    containing the article etc. and in different situations
   *    it may be useful to search through different ring.
   * This method distanceFrom tells how far away the
   * number we're seeking for is and notable thing here
   * is that number may roll over when going via path of
   * shortest distance.
   * @param aHash is the number from which we calculate distance
   * @return return value is also a hash, that is, a number
   */
  Hash distanceFrom (const Hash& aHash) ;

  /**
   * Man-kinds greatest invention ever follows: bignum operations for
   * small bignums.
   *
   * Substraction operator. As our hash if you think of it as
   * a number, is unsigned, this operator will definitely return
   * undefined-kind-of-crap if you try to substract bigger number
   * from a smaller one. Something you'll get but it can be anything.
   *
   * Other way around it seems to do something.
   */
  Hash operator-(const Hash& aToBeSubstracted) const ;
  /**
   * addition operation for integers of 160 bits
   */
  Hash operator+(const Hash& aToBeAdded) const ;

private:
  void setFromCharArray(const unsigned char aDigest[KNumberOfIntsInHash * 4]) ;
public:
  /**
   * this public array carries the calculated
   * (SHA1) hash. there is 5*32 bits and in db
   * too we're supposed to use 5 columns for
   * storing a hash ; we can still compare and do
   * ordering if we decide that low-order bits go
   * into iHash160bits[0] and the highest into
   * iHash160bits[4].
   *
   * In 64-bit boxen we could fit the bits into
   * 64-bit integers and have this thing working
   * a bit faster - hopefully this thing will still
   * work fast enough to work also in less-powerful
   * 32-bit architechtures for which we don't here
   * want to invent a separate implementation.
   */
  quint32 iHash160bits[KNumberOfIntsInHash] ;
} ;
#endif
