/*                                      -*-C++-*-
    Classified Ads is Copyright (c) Antti Jarvinen 2015.

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
#include <QVariant> // Actually for QVariantMap

#ifndef JSONWRAPPER_H
#define JSONWRAPPER_H

/**
 * @brief Class for wrapping engine to (de)serialize json
 *
 * Qt5 has included json routines so if using Qt5 it makes sense to
 * that because it is one dependency less ; with earlier QT versions a 3rd 
 * party library is required. This class provides programmer with single API
 * to use json routines regardless of underlying implementation. 
 */
class JSonWrapper {
public:
  /**
   * Method that returns aJSonText parsed into a QVariantMap.
   *
   * @param aJSonText is text to parse. Must be valid json.
   * @param aIsParseOk is set to true/false depending on parse success
   * @param aUncompressFirst indication if aJSonText is compressed or not.
   *        if compressed, then qUncompress is applied before parser.
   * @return Content of AJSonText as QVariant. Is empty if parse failed
   *         so user should check value of aIsParseOk before assuming
   *         anything about the content.
   */
  static QVariantMap parse(const QByteArray& aJSonText,
			   bool* aIsParseOk = NULL,
			   bool aUncompressFirst = false ) ; 
  /**
   * method that returns QVariant as JSon text.
   *
   * @param aObjectToSerialize is the object, usually QVariantMap but
   *                           basically any QVariant should do.
   * @param aFinallyCompress if set to true, the returned byte-array
   *                         is compressed using qCompress()
   * @return serialized stream e.g. json text.
   */
  static QByteArray serialize(const QVariant& aObjectToSerialize,
			      bool aFinallyCompress = false ) ; 
} ;
#endif /* JSONWRAPPER_H */
