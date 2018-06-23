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

#ifndef CLASSIFIED_PROFILECOMMENTMODEL_H
#define CLASSIFIED_PROFILECOMMENTMODEL_H
#include <QSqlDatabase>
#include "../mcontroller.h" // because enum from there is needed
#include "../net/connection.h"
#include "datamodelbase.h"

class Hash ;
class MModelProtocolInterface ;
class ProfileComment ;

/**
 * @brief This is is part of datamodel for storing comments of user profiles
 *
 * This behaves largerly in same manner as profilemodel, data-carrier
 * is different
 */
class ProfileCommentModel : public ModelBase {
    Q_OBJECT

public:
    ProfileCommentModel(MController *aMController,
                        MModelProtocolInterface &aModel) ;
    ~ProfileCommentModel() ;

    /**
     * sends profile comment to selected nodes in network for others
     * to retrieve. As a side-effect will set iFingerPrint of the
     * comment.
     *
     * @param aProfileComment is the profile to publish
     * @return true if things went all right
     */
    bool publishProfileComment(ProfileComment& aProfileComment) ;

    /**
     * gets a comment or null. whoever calls this method is
     * obliged to delete the returned comments.
     * @param aFingerPrint profile serial number
     * @param aEmitOnEncryptionError if set to false, will return NULL
     *        when no suitable encryption key is found and will not
     *        emit error message that would be catched by UI and
     *        shown to user
     * @return profile or NULL
     */
    ProfileComment* profileCommentByFingerPrint(const Hash& aFingerPrint,
            bool aEmitOnEncryptionError=true) ;

    /**
     * Gets a comment. Because profile comment is signed .. and that means that
     * a particular byte-stream (containing actually a serialized json
     * object) gets signed those bytes can't be altered or signature will
     * not verify. This methods gets the actual signed bytestream
     * that has been checked against a public key.
     *
     * Note that this method does not check the signature any more,
     * idea is that we don't store into db profiles that fail the signature
     * check so this method only gets the profile, does not set it.
     *
     * @param aFingerPrint profile comment serial number
     * @param aResultingProfileCommentData will contain comment data
     * @param aResultingSignature will contain comment signature
     * @param aResultingProfileFingerPrint is output variable, that
     *           will have its value set to containt fingerprint of the
     *           profile that the comment is commenting.
     * @param aFlags will be set to flags values ; bit 1 on means
     *        that content is encrypted.
     * @param aTimeOfPublish is also output variable, if non-NULL, will
     *        get its value set to time of publish of the comment
     * @return true if profile was found
     */
    bool commentDataByFingerPrint(const Hash& aFingerPrint,
                                  QByteArray& aResultingProfileCommentData,
                                  QByteArray& aResultingSignature,
                                  Hash* aResultingProfileFingerPrint,
                                  quint32* aFlags,
                                  quint32* aTimeOfPublish = NULL ) ;
    /**
     * called by protocol parser when a profile comment is received.
     * note the absence of signing key. if the profile commented is public, the
     * key is found inside the comment. if the profile commented is private,
     * the comment behaves like  a private message, the recipient needs to
     * first open the encryption before she can verify that the comment
     * is from certain operator
     * @param aFingerPrint is the content @ref Hash received from protocol header
     * @param aCommentedProfileFP is fingerprint hash of the profile that the
     *                               comment concerns
     * @param aContent is actual profile data
     * @param aSignature is digital signature of the aContent
     * @param aBangPath is list of last nodes where this content has been, idea is to
     *        prevent re-sending content to somewhere where it has already
     *        been
     * @param aFlag possible flags telling about encyption and compression
     * @param aTimeStamp timestamp of profile (if encrypted, it must be carried outside..)
     * @return true on success
     */
    bool publishedProfileCommentReceived(const Hash& aFingerPrint,
                                         const Hash& aCommentedProfileFP,
                                         const QByteArray& aContent,
                                         const QByteArray& aSignature,
                                         const QList<quint32>& aBangPath,
                                         const unsigned char aFlags,
                                         const quint32 aTimeStamp,
                                         const Hash& aFromNode) ;
    /**
     * called by protocol parser when a comment is received by "send" protocol type
     * @param aFingerPrint is the profile @ref Hash received from protocol header
     * @param aCommentedProfileFP is fingerprint hash of the profile that the
     *                               comment concerns
     * @param aContent is actual comment data
     * @param aSignature is digital signature of the aContent
     * @param aProfileCommentPublicKey is the key that was used to sign the profile.
     *        that is transferred outside profile just because the recipient
     *        might not have the key  beforehand ; if she does, the existing
     *        key will be used in verifying process.
     * @param aFlag possible flags telling about encyption and compression
     * @param aTimeStamp timestamp of profile (if encrypted, it must be carried outside..)
     * @return true on success
     */
    bool sentProfileCommentReceived(const Hash& aFingerPrint,
                                    const Hash& aCommentedProfileFP,
                                    const QByteArray& aContent,
                                    const QByteArray& aSignature,
                                    const unsigned char aFlags,
                                    const quint32 aTimeStamp,
                                    const Hash& fromNode ) ;

    /**
     * Method for filling connected peers send queue with items
     * that we have and that belong to said peers bucket.
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
     * method for producing list of profile comments that comment
     * aProfileHash and that are more recent than aSinceTimeStamp
     */
    QList<Hash> commentsForProfile(const Hash& aProfileHash,
                                   const quint32 aSinceTimeStamp) ;

    void reIndexAllCommentsIntoFTS() ;
private: // methods
    /**
     * Workhorse of methods @ref publishedProfileCommentReceived and @ref sentProfileCommentReceived
     * @return true on success
     */
    bool doHandlepublishedOrSentProfileComment(const Hash& aFingerPrint,
            const Hash& aCommentedProfileFP,
            const QByteArray& aContent,
            const QByteArray& aSignature,
            const QList<quint32>& aBangPath,
            const unsigned char aFlags,
            const quint32 aTimeStamp,
            bool aWasPublish,
            const Hash& aFromNode ) ;
signals:
    void error(MController::CAErrorSituation aError,
               const QString& aExplanation) ;
    /** emitted when new comment is received */
    void contentReceived(const Hash& aHashOfContent,
                         const Hash& aHashOfProfileCommented,
                         const ProtocolItemType aTypeOfReceivdContent) ;
} ;
#endif
