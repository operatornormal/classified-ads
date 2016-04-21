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



#ifndef PROTOCOL_MESSAGE_FORMATTER_H
#define PROTOCOL_MESSAGE_FORMATTER_H
#include "protocol.h"
#include <QByteArray>
#include <QPair>
#include "../datamodel/searchmodel.h"
#include "voicecallengine.h" // for payload type

class Connection ;
class Node ;
class VoiceCall ; // from datamodel 

/**
 * @brief This class produces items sent to peers over network.
 *
 * Class that contains (static) methods for producing
 * byte-streams of various messages.
 */
class ProtocolMessageFormatter {
public:
    /**
     * method for producing node greeting message
     * @param aNode is greeting about the node
     *        concerned, usually "this" node
     * @return bytearray that is ready to be sent
     *         over socket so that other end will
     *         understand.
     */
    static QByteArray nodeGreeting(const Node& aNode) ;
    /**
     * method for producing random number message
     * @return bytearray that is ready to be sent
     *         over socket so that other end will
     *         understand.
     */
    static QByteArray randomNumbers(void) ;

    /**
     * method for producing nodegreeting request message
     * @return bytearray that is ready to be sent
     *         over socket so that other end will
     *         understand.
     */
    static QByteArray requestForNodesAroundHash (const Hash& aHash) ;

    /**
     * method for producing binary blob request message
     * @return bytearray that is ready to be sent
     *         over socket so that other end will
     *         understand.
     */
    static QByteArray requestForBinaryBlob(const Hash& aHash) ;
    /**
     * method for producing operator profile request message
     * @return bytearray that is ready to be sent
     *         over socket so that other end will
     *         understand.
     */
    static QByteArray requestForUserProfile(const Hash& aHash) ;
    /**
     * method for producing message to other node that asks
     * it to send an individual profile comment
     * @return bytearray that is ready to be sent
     *         over socket so that other end will
     *         understand.
     */
    static QByteArray requestForUserProfileComment(const Hash& aIndividualCommentHash) ;
    /**
     * method for producing operator classified ad request message
     * @return bytearray that is ready to be sent
     *         over socket so that other end will
     *         understand.
     */
    static QByteArray requestForClassifiedAd(const Hash& aHash) ;
    /**
     * method for sending request to other nodes regarding private
     * messages having given hash
     * @param aProfileHash hash of message that is to be found
     * @return bytearray that is ready to be sent
     *         over socket so that other end will
     *         understand.
     */
    static QByteArray requestForPrivateMessages(const Hash& aMessageHash) ;
    /**
     * method for sending request to other nodes regarding private
     * messages destined to given profile
     * @param aProfileHash hash of profile whose messages are searched
     * @param aTimeOfOldesMsgToSend since when to send msgs from
     * @return bytearray that is ready to be sent
     *         over socket so that other end will
     *         understand.
     */
    static QByteArray requestForPrivateMessagesForProfile(const Hash& aProfileHash,
            const quint32 aTimeOfOldestMsgToSend) ;
    /**
     * method for sending request to other nodes regarding comments
     * about specified profile. in practice parser (upon receipt) will
     * make calls that check not only for new comments, but also
     * if there has been an update at the profile itself. Call to this
     * method is result of RequestForProfilePoll being put into network
     * request queue.
     *
     * @param aCommentedProfileHash hash of profile whose comments are searched
     * @param aTimeOfOldesMsgToSend since when to send comments from
     * @return bytearray that is ready to be sent
     *         over socket so that other end will
     *         understand.
     */
    static QByteArray requestForProfileComments(const Hash& aCommentedProfileHash,
            const quint32 aTimeOfOldestMsgToSend) ;
    /**
     * Classified ads have classification, the "intent-object-location"
     * triplet shown in UI and it also has a hash. When user chooses
     * this triplet, it might be nice to query surrounding nodes
     * about classified ads regarding that triplet. This method is for
     * sending such a request. Its response will contain hashes of
     * the ads in that classification, possibly followed by the articles
     * too.
     * @param aHashOfClassification hash calculated over
     *                      "intent-object-location" string
     * @param aStartingTimestamp possible starting timestamp for the articles.
     *                           Currently not implemented in protocol, sender
     *                           will limit the number of ads it sends
     * @param aEndingTimestamp possible ending timestamp for the articles
     *                         Currently not implemented in protocol, sender
     *                         will limit the number of ads it sends
     * @return bytearray that is ready to be sent
     *         over socket so that other end will
     *         understand.
     */
    static QByteArray requestForAdsClassified(const Hash& aHashOfClassification,
            const quint32 aStartingTimestamp,
            const quint32 aEndingTimestamp) ;
    /**
     * method for producing byte-array containing (some) published content
     *
     * @param aContentMagicNumber tells what kind of item this is. For
     *        example KProfilePublish (from protocol.h) if we're publishing
     *        a profile here
     * @param aContentHash hash of the content to be published. Reason this
     *                     Hash here is given explicitly is that different
     *                     object types have different hashing methods - for
     *                     binary blob it is the content itself that makes the
     *                     hash but for profile it is the fingerprint of the
     *                     signing key etc.
     * @param aContent actual content
     * @param aSignature signature verifying aContent
     * @param aBangPath list of low-order bits of hosts where this content has
     *                  already been seen, 5 at max
     * @param aSigningKey contains public encryption key used to sign the
     *                    content ; while it over time will be mostly overhead
     *                    it anyway allows reader to reach the poster that
     *                    is great advantage over some overhead..
     * @param aIsContentEncrypted is true of aContent is protected with
     *                            encryption
     * @param aIsContentCompressed is true of aContent is inside qCompress.
     *                             if both compress+encryption, naturally
     *                             the content is first compressed, only
     *                             after that encrypted
     * @param aTimeStamp may be given for content where actual timestamp
     *                   is inside encryption
     *
     * @return bytearray that is ready to be sent
     *         over socket so that other end will
     *         understand.
     */
    static QByteArray contentPublish(const unsigned char aContentMagicNumber,
                                     const Hash& aContentHash,
                                     const QByteArray& aContent,
                                     const QByteArray& aSignature,
                                     const QList<quint32>& aBangPath,
                                     const QByteArray& aSigningKey,
                                     bool aIsContentEncrypted,
                                     bool aIsContentCompressed,
                                     quint32 aTimeStamp ) ;
    /**
     * separate publish-formatter for private messages as they're
     * quite different from rest of the content
     */
    static QByteArray privMsgPublish(const Hash& aContentHash,
                                     const QByteArray& aContent,
                                     const QByteArray& aSignature,
                                     const QList<quint32>& aBangPath,
                                     const Hash& aDestination,
                                     const Hash& aRecipient,
                                     quint32 aTimeStamp ) ;
    /**
     * separate send-formatter for private messages as they're
     * quite different from rest of the content. Similar to publish
     * but no bangpath
     */
    static QByteArray privMsgSend(const Hash& aContentHash,
                                  const QByteArray& aContent,
                                  const QByteArray& aSignature,
                                  const Hash& aDestination,
                                  const Hash& aRecipient,
                                  quint32 aTimeStamp ) ;
    /**
     * separate publish-formatter for profile comments as they're
     * quite different from rest of the content. in case of comments
     * for public profiles they behave a bit like classified ads
     * but comments commenting private profiles need to behave
     * more-or-less like private messages as that's what they are ;
     * private messages with more than 1 recipient .. or it is
     * possible to have private profile with 0 readers and then
     * comment it self. monologue ensues.
     */
    static QByteArray profileCommentPublish(const Hash& aContentHash,
                                            const QByteArray& aContent,
                                            const QByteArray& aSignature,
                                            const QList<quint32>& aBangPath,
                                            const Hash& aProfileCommented,
                                            quint32 aTimeStamp,
                                            quint32 aFlags ) ;
    /**
     * separate send-formatter for profile comment as they're
     * quite different from rest of the content.
     */
    static QByteArray profileCommentSend(const Hash& aContentHash,
                                         const QByteArray& aContent,
                                         const QByteArray& aSignature,
                                         const Hash& aProfileCommented,
                                         quint32 aTimeStamp,
                                         quint32 aFlags ) ;
    /**
     * this is almost same as @ProtocolMessageFormatter.contentPublish
     * but is reply to another node regarding request of content,
     * not users own publishing so this has no bangpath but otherwise
     * this behaves in about same way.
     */
    static QByteArray contentSend(const unsigned char aContentMagicNumber,
                                  const Hash& aContentHash,
                                  const QByteArray& aContent,
                                  const QByteArray& aSignature,
                                  const QByteArray& aSigningKey,
                                  bool aIsContentEncrypted,
                                  bool aIsContentCompressed,
                                  quint32 aTimeStamp ) ;

    static QByteArray replyToAdsClassified(const Hash& aClassificationHash,
                                           const QList<QPair<Hash,quint32> >& aListOfAds	) ;

    /**
     * method for formatting a network search query
     */
    static QByteArray searchSend(const QString& aSearch,
                                 bool aSearchAds,
                                 bool aSearchProfiles,
                                 bool aSearchComments,
                                 const Hash& aSearchIdentifier) ;

    /**
     * method for formatting a network search results
     */
    static QByteArray searchResultsSend(const QList<SearchModel::SearchResultItem>& aResults,
                                        quint32 aSearchId) ;

    /**
     * Method for formatting voice call status object 
     * @param aCall is the call status object
     * @param aController is application controller instance
     * @param aSelectedProfile is hash of the profile who will sign
     *        the serialized object
     * @param aDoSign if set to true, normal signature will be added.
     *        If false, zero-len signature will be used. 
     * @return serialized bytes of call status
     */
    static QByteArray voiceCall(const VoiceCall& aCall,
                                MController& aController,
                                const Hash& aSelectedProfile,
                                bool aDoSign = true ) ;
    /**
     * Method for formatting voice call real-time data like audio
     *
     * @param aCallId call (stream) identifier
     * @param aSeqNo sequence number of payload in stream
     * @param aPayloadType type payload
     * @param aPayload actual payload bytes to send, like call audio
     * @return serialized bytes of call status
     */
    static QByteArray voiceCallRtData(quint32 aCallId,
                                      quint32 aSeqNo,
                                      VoiceCallEngine::PayloadType aPayloadType,
                                      const QByteArray& aPayload ) ;

private:
    /**
     * workhorse of @ProtocolMessageFormatter.contentPublish and
     * @ProtocolMessageFormatter.contentSend has parameter
     * telling which action to take
     */
    static QByteArray doContentSendOrPublish(QByteArray& retval,
            const unsigned char aContentMagicNumber,
            const Hash& aContentHash,
            const QByteArray& aContent,
            const QByteArray& aSignature,
            const QList<quint32>& aBangPath,
            const QByteArray& aSigningKey,
            bool aIsContentEncrypted,
            bool aIsContentCompressed,
            quint32 aTimeStamp,
            bool aIsPublish) ;
    /**
     * workhorse of @ProtocolMessageFormatter.profileCommentPublish and
     * @ProtocolMessageFormatter.profileCommentSend has parameter
     * telling which action to take
     */
    static QByteArray doCommentSendOrPublish(const Hash& aContentHash,
            const QByteArray& aContent,
            const QByteArray& aSignature,
            const QList<quint32>& aBangPath,
            const Hash& aProfileCommented,
            quint32 aTimeStamp,
            quint32 aFlags,
            bool aIsPublish) ;
} ;

#endif
