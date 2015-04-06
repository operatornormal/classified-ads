/*    -*-C++-*- -*-coding: utf-8-unix;-*-
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

#ifndef CLASSIFIED_BINARYFILE_H
#define CLASSIFIED_BINARYFILE_H
#include <QString>
#include "../util/hash.h" // for class Hash  

class MController ;

/**
 * @brief Carrier-class for binary blob
 *
 * Instances of this class are stored and retrieved using
 * @ref BinaryFileModel. Lets decide that this class carries
 * the metadata only, actual octet-stream of the file
 * is retrieved using a QByteArray only, there is method
 * for that in the model.
 */
class BinaryFile {
public:
    /** enum for storing information if we have file locally or not */
    enum LocalStorageStatus {
        FileIsLocallyStored,
        FileIsNotLocallyStored,
        LocalStorageStatusUnknown
    } ;
    BinaryFile(const Hash& aHash) ; /**< constructor */
    ~BinaryFile() ; /**< destructor */
    /**
     * method for string that is shown to user about the file.
     * Most often could be original name from filesystem but
     * other explanation too..
     * @return string to display to user
     */
    QString displayName() const ;
    QByteArray asJSon(const MController& aController) const ; /**< returns file data as JSon stream */
    bool fromJSon(const QByteArray &aJSonBytes,
                  const MController& aController ) ; /**< parses json into members*/
    const Hash iFingerPrint ; /**< file hash */
    QString iMimeType ;  /**< what kind of data */
    QString iDescription ; /**< what is inside */
    QString iOwner ; /**< fingerprint of publisher */
    QString iContentOwner ; /**< if someone owns the content, name or SHA1 fp */
    QString iLicense ; /**< restriction in usage;PD or GPL or C-C or anything? */
    QString iFileName ; /**< name of the file-system file */
    quint32 iTimeOfPublish ; /**< seconds since 1-jan-1970 */
    bool iIsEncrypted ;
    bool iIsCompressed ; /**< needs to be in DB, not in JSon */
    LocalStorageStatus iLocalStorageStatus ; /** file is stored locally, or not */
} ;
#endif
