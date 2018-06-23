/*    -*-C++-*- -*-coding: utf-8-unix;-*-
  Classified Ads is Copyright (c) Antti Järvinen 2013-2018.

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

#ifndef CLASSIFIED_PRIVMSGMODEL_H
#define CLASSIFIED_PRIVMSGMODEL_H
#include "../mcontroller.h" // because enum from there is needed
#include "../net/protocol.h" // for SendQueueItem
#include "datamodelbase.h"

class Hash ;
class MModelProtocolInterface ;
class PrivMessage ;

/**
 * @brief This is is part of datamodel for storing private messages
 *
 * Encryption keys related to messages need to be done using
 * ContentEncryptionModel but after it has done its job,
 * this part of datamodel handles that actual message-content.
 *
 * Private messages are handled differently in (at least) one
 * way worth mentioning: when other published content is
 * sent over network, the content is accompanied by public key
 * used to sign the content. Private messages have the public
 * key of the content inside the content and the content
 * is encrypted. This means 2 things:
 *  - only the recipient knows who sent
 *  - only the recipient can verify integrity of the data
 * so, while storing messages here, we do not try to verify.
 * When user opens a message, it is first de-crypted and
 * key obtained from inside, then verification takes place.
 */
class PrivMessageModel : public ModelBase {
    Q_OBJECT

public:
    PrivMessageModel(MController *aMController,
                     MModelProtocolInterface &aModel) ;
    ~PrivMessageModel() ;

    /**
     * sends private message to selected nodes in network for others
     * to retrieve. as a side-effect will update aPrivMessage.iFingerPrint,
     * supposing the publish is successful.
     *
     * @param aPrivMessage is the message to publish. Messages are
     *                     "sent" so that they're published into
     *                     network. Recipient then may poll them.
     *                     Networking logick will try to short-cut
     *                     this if correct node seems to be already
     *                     connected but in principle messages are
     *                     sent by process of publish.
     * @param aDestinationNode if node of the recipient is known,
     *                         it may be given here.
     * @return true if things went all right
     */
    bool publishPrivMessage(PrivMessage& aPrivMessage,
                            const Hash& aDestinationNode = KNullHash ) ;

    /**
     * gets a message. caller is supposed to delete the returned
     * profile.
     * @param aFingerPrint messaqe serial number
     * @return message or NULL
     */
    PrivMessage* messageByFingerPrint(const Hash& aFingerPrint) ;

    /**
     * Gets a message. Because message is signed .. and that means that
     * a particular byte-stream (containing actually a serialized json
     * object) gets signed those bytes can't be altered or signature will
     * not verify. This methods gets the actual signed bytestream
     * that has been checked against a public key.
     *
     * Note that this method does not check the signature any more,
     * idea is that we don't store into db messages that fail the signature
     * check so this method only gets the message, does not set it.
     *
     * @param aFingerPrint message serial number
     * @param aResultingPrivMessageData will contain message data
     * @param aResultingSignature will contain message signature
     * @param aResultingDestinationNode output parameter will get its value
     *                                  set to fingerprint of the node where
     *                                  the message is supposed to go.
     *                                  May come out as KNullHash if the
     *                                  node is not known.
     * @param aResultingRecipient output parameter will get its value
     *                            set to fingerprint of the profile that
     *                            is supposed to read the msg
     * @param aTimeOfPublish is output variable, if non-NULL, will
     *        get its value set to time of publish of the message
     * @return true if message was found
     */
    bool messageDataByFingerPrint(const Hash& aFingerPrint,
                                  QByteArray& aResultingPrivMessageData,
                                  QByteArray& aResultingSignature,
                                  Hash* aResultingDestinationNode,
                                  Hash* aResultingRecipient,
                                  quint32* aTimeOfPublish = NULL) ;
    /**
     * called by protocol parser when a message is received
     * @param aFingerPrint is the message @ref Hash received from protocol header
     * @param aContent is actual message data
     * @param aSignature is digital signature of the aContent
     * @param aBangPath is list of last nodes where this content has been, idea is to
     *        prevent re-sending content to somewhere where it has already
     *        been
     * @param aDestinationNode node where the message should be
     *        delivered to ; may be nullhash
     * @param aRecipient recipient profile hash
     * @param aTimeStamp timestamp of message (if encrypted, it must be carried outside..)
     * @param aFromNode hash of node sending the message to us
     * @return true on success
     */
    bool publishedPrivMessageReceived(const Hash& aFingerPrint,
                                      const QByteArray& aContent,
                                      const QByteArray& aSignature,
                                      const QList<quint32>& aBangPath,
                                      const Hash& aDestinationNode,
                                      const Hash& aRecipient,
                                      const quint32 aTimeStamp,
                                      const Hash& aFromNode) ;
    /**
     * called by protocol parser when a private message is received by "send" protocol type
     * @param aFingerPrint is the message @ref Hash received from protocol header
     * @param aContent is actual message data
     * @param aSignature is digital signature of the aContent
     * @param aDestinationNode node where the message should be
     *        delivered to ; may be nullhash
     * @param aRecipient recipient profile hash
     * @param aFlag possible flags telling about encyption and compression
     * @param aTimeStamp timestamp of message (if encrypted, it must be carried outside..)
     * @return true on success
     */
    bool sentPrivMessageReceived(const Hash& aFingerPrint,
                                 const QByteArray& aContent,
                                 const QByteArray& aSignature,
                                 const Hash& aDestinationNode,
                                 const Hash& aRecipient,
                                 const quint32 aTimeStamp,
                                 const Hash& fromNode ) ;

    /**
     * Method for filling connected peers send queue with items
     * that we have and that belong to said peers bucket.
     *
     * Note that for private messages the bucket is kind of
     * 2-way process. This method checks for private messages
     * that are destined for this particular (connecting)
     * node. In addition this method checks for messages
     * where the destination profile belongs to bucket defined
     * by aStartOfBucket/aEndOfBucket. In addition to these
     * mechanisms this is also exploited by 3rd logick
     * (possibly to be implemented in networkconnectorengine)
     * that every-now-and-then creates connections to nodes
     * that (according to hash) are supposed to contain
     * private messages destined to profiles whose private keys
     * we have in storage.
     *
     * @param aSendQueue is send-queue of the connection that
     *                   serves particular node
     * @param aStartOfBucket is hash where bucket of that peer
     *                       starts. In practice it is the hash
     *                       of the node itself.
     * @param aEndOfBucket is hash where bucket of that peer ends.
     *                     That in turn depends on network size,
     *                     or number of active nodes in the network.
     *                     See method @ref NodeModel::bucketEndHash
     *                     for details.
     * @param aLastMutualConnectTime time when node was last in
     *                     contact. In practice we'll fill the
     *                     bucket items published after this time.
     */
    void fillBucket(QList<SendQueueItem>& aSendQueue,
                    const Hash& aStartOfBucket,
                    const Hash& aEndOfBucket,
                    quint32 aLastMutualConnectTime,
                    const Hash& aForNode );
    /**
     * method for getting list of messages destined for given
     * operator that has been posted after the given date.
     * This is called upon network protocol message
     * KPrivMessagesForProfile.
     *
     * @param aProfileHash operator profile fingerprint whose messages are
     *                     requested.
     * @param aLastMutualConnectTime messages that are sent afer aLastMutualConnectTime
     *                               will be returned.
     * @return list of message fingerprints
     */
    QList<Hash> messagesForProfile(const Hash& aProfileHash,
                                   const quint32 aLastMutualConnectTime) ;

    /** method for marking private message as read */
    void setAsRead(const Hash& aMessageHash, bool aIsRead ) ;
private: // methods
    /**
     * Workhorse of methods @ref publishedPrivMessageReceived and
     * @ref sentPrivMessageReceived
     * @return true on success
     */
    bool doHandlepublishedOrSentPrivMessage(const Hash& aFingerPrint,
                                            const QByteArray& aContent,
                                            const QByteArray& aSignature,
                                            const QList<quint32>& aBangPath,
                                            const Hash& aDestinationNode,
                                            const Hash& aRecipient,
                                            const quint32 aTimeStamp,
                                            bool aWasPublish,
                                            const Hash& aFromNode ) ;
signals:
    void error(MController::CAErrorSituation aError,
               const QString& aExplanation) ;
    /** emitted when new message is received */
    void contentReceived(const Hash& aHashOfContent,
                         const Hash& aHashOfRecipientProfile,
                         const ProtocolItemType aTypeOfReceivdContent) ;
} ;
#endif
