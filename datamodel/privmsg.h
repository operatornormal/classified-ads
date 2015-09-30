/*    -*-C++-*- -*-coding: utf-8-unix;-*-
    Classified Ads is Copyright (c) Antti Järvinen 2013.

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

#ifndef PRIVATE_MESSAGE_H
#define PRIVATE_MESSAGE_H
#include <QString>
#include "../util/hash.h" // for class Hash  

class MController ;
class Profile ;
class Node ;

/**
 * @brief Carrier-class for private msg inside classified ads
 *
 * Instances of this class are stored and retrieved using
 * @ref PrivateMessageModel.
 */
class PrivMessage {
public:
    PrivMessage() ; /**< constructor */
    ~PrivMessage() ; /**< destructor */
    /**
     * method for string that is shown to user about the message.
     *
     * @return string to display to user
     */
    QString displayName() const ;
    QByteArray asJSon(const MController& aController) const ; /**< returns msg data as JSon stream */
    bool fromJSon(const QByteArray &aJSonBytes,
                  const MController& aController ) ; /**< parses json into members*/
    /** hash of the message text .. more specifically hash
     * of the message when it is serialized into json */
    Hash iFingerPrint ;
    QString iSenderName ;  /**< nickname/display-name of sender ; may empty if private profile */
    Hash iSenderHash ; /**< profile fingerprint of the sender */
    quint32 iTimeOfPublish ; /**< seconds since 1-jan-1970 */
    /**
     * file attachements ; they're published as normal binary blobs and
     * then just listed here
     */
    QList<Hash> iAttachedFiles ;
    QString iSubject ; /**< Subject of the post */
    Hash iReplyToMsg ; /**< Possible reference to another private message that this is reply to */
    Hash iReplyToCa ; /**< Possible reference to classified ad post that this is reply to */
    QString iMessageText ; /**< actual message here */
    QByteArray iProfileKey ; /**< key of the message sender */
    Hash iRecipient ; /**< Hash of the profile that will receive the msg */
    Node* iNodeOfSender ; /**< When sending a reply, send it here */
} ;
#endif
