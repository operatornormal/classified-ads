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

#ifndef CLASSIFIED_PROFILEMODEL_H
#define CLASSIFIED_PROFILEMODEL_H
#include <QSqlDatabase>
#include "../mcontroller.h" // because enum from there is needed
#include "../net/connection.h"
#include "datamodelbase.h"

class Hash ;
class MModelProtocolInterface ;
class Profile ;

/**
 * @brief This is is part of datamodel for storing user profiles
 *
 * Encryption keys related to profiles need to be done using
 * ContentEncryptionModel but after it has done its job,
 * this part of datamodel handles that actual profile-content
 */
class ProfileModel : public ModelBase {
    Q_OBJECT

public:
    ProfileModel(MController *aMController,
                 const MModelProtocolInterface &aModel) ;
    ~ProfileModel() ;

    /**
     * sends profile to selected nodes in network for others
     * to retrieve.
     *
     * The reason why this method is in model (and not for instance,
     * in Controller) is that "publish" means that a particular
     * profile is stored into and marked as "needs to be published".
     * Before that happens we need to open network connections to
     * nodes that are supposed to contain the profile and once
     * connections are open, then same profile is pushed into several
     * nodes until it is considered "published". That opening and
     * pushing-part happens by networking-parts.
     *
     * @param aProfile is the profile to publish
     * @return true if things went all right
     */
    bool publishProfile(const Profile& aProfile) ;

    /**
     * gets a profile. caller is supposed to delete the returned
     * profile.
     * @param aFingerPrint profile serial number
     * @param aEmitErrorOnEncryptionErrors if true and profile cannot
     *        be opened because selected operator is not in list of
     *        readers then an error message shall be emit()ted.
     *        This is set to false for trust-tree related operations
     *        where it is likely that selected operator is not reader
     *        of every operator trusted by selected operator.
     * @param aOmitImage if set to true, possible image will not be loaded from
     *        data ; in cases where we know that we'll need only subset of
     *        information and no image, we can skip this costly operation
     * @return profile or NULL
     */
    Profile* profileByFingerPrint(const Hash& aFingerPrint,
                                  bool aEmitErrorOnEncryptionErrors = true,
                                  bool aOmitImage = false ) ;

    /**
     * Gets a profile. Because profile is signed .. and that means that
     * a particular byte-stream (containing actually a serialized json
     * object) gets signed those bytes can't be altered or signature will
     * not verify. This methods gets the actual signed bytestream
     * that has been checked against a public key.
     *
     * Note that this method does not check the signature any more,
     * idea is that we don't store into db profiles that fail the signature
     * check so this method only gets the profile, does not set it.
     *
     * @param aFingerPrint profile serial number
     * @param aResultingProfileData will contain profile data
     * @param aResultingSignature will contain profile signature
     * @param aIsProfilePrivate will be set to true if aResultingProfileData
     *                          is encrypted.
     * @param aTimeOfPublish is also output variable, if non-NULL, will
     *        get its value set to time of publish of the profile
     * @param aResultingPublicKey if non-NULL, will be written to
     *        contain the public key of the profile
     * @return true if profile was found
     */
    bool profileDataByFingerPrint(const Hash& aFingerPrint,
                                  QByteArray& aResultingProfileData,
                                  QByteArray& aResultingSignature,
                                  bool* aIsProfilePrivate,
                                  quint32* aTimeOfPublish = NULL,
                                  QByteArray* aResultingPublicKey = NULL ) ;
    /**
     * called by protocol parser when a profile is received
     * @param aFingerPrint is the profile @ref Hash received from protocol header
     * @param aContent is actual profile data
     * @param aSignature is digital signature of the aContent
     * @param aBangPath is list of last nodes where this content has been, idea is to
     *        prevent re-sending content to somewhere where it has already
     *        been
     * @param aProfilePublicKey is the key that was used to sign the profile.
     *        that is transferred outside profile just because the recipient
     *        might not have the key  beforehand ; if she does, the existing
     *        key will be used in verifying process.
     * @param aFlag possible flags telling about encyption and compression
     * @param aTimeStamp timestamp of profile (if encrypted, it must be carried outside..)
     * @return true on success
     */
    bool publishedProfileReceived(const Hash& aFingerPrint,
                                  const QByteArray& aContent,
                                  const QByteArray& aSignature,
                                  const QList<quint32>& aBangPath,
                                  const QByteArray& aProfilePublicKey,
                                  const unsigned char aFlags,
                                  const quint32 aTimeStamp,
                                  const Hash& aFromNode) ;
    /**
     * called by protocol parser when a profile is received by "send" protocol type
     * @param aFingerPrint is the profile @ref Hash received from protocol header
     * @param aContent is actual profile data
     * @param aSignature is digital signature of the aContent
     * @param aProfilePublicKey is the key that was used to sign the profile.
     *        that is transferred outside profile just because the recipient
     *        might not have the key  beforehand ; if she does, the existing
     *        key will be used in verifying process.
     * @param aFlag possible flags telling about encyption and compression
     * @param aTimeStamp timestamp of profile (if encrypted, it must be carried outside..)
     * @return true on success
     */
    bool sentProfileReceived(const Hash& aFingerPrint,
                             const QByteArray& aContent,
                             const QByteArray& aSignature,
                             const QByteArray& aProfilePublicKey,
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
     * method for find a profile that we have private keys for and
     * that has not had its private messages polled for given
     * time (say, 15 minutes). See method
     * @ref NetworkConnectorEngine::createConnectionsToNodesStoringPrivateMessages
     * @param aLastPollTime output variable. this points to quint32 that
     *        (if profile is found) will have its value set to time
     *        of last message poll for the profile returned.
     * @return fingerprint of node or KNullHash if every profile
     *         with private keys has had its messages polled recently
     */
    Hash profileWithOldestTimeSinceMsgPoll(quint32* aLastPollTime) ;

    /**
     * method that is called after successful message poll ; it set
     * the message poll timestamp for a profile
     */
    void setPrivateMessagePollTimeForProfile(const quint32 aTimeStamp,
            const Hash& aProfile ) ;

    /**
     * Updates private data of a profile whose private key we
     * stored locally. In first stage this private data includes
     * contact list, later maybe trust settings etc.
     */
    void setPrivateDataForProfile( const Hash& aProfile,
                                   const QVariant& aPrivateData ) ;

    /**
     * Getter-method for private data of a profile. This "Private data" is
     * locally-stored settings-type of data specific to profile and it is
     * not shared inside network.
     */
    QVariant privateDataOfProfile( const Hash& aProfile ) ;

    /**
     * Gets time of last (successful) profile update+comment poll.
     */
    time_t getLastProfileUpdateTime( const Hash& aProfile ) ;

    /**
     * sets time of last (successful) profile update+comment poll.
     */
    void setLastProfileUpdateTime( const Hash& aProfile,time_t aTime ) ;

    void reIndexAllProfilesIntoFTS() ;
private: // methods
    /**
     * Workhorse of methods @ref publishedProfileReceived and @ref sentProfileReceived
     * @return true on success
     */
    bool doHandlepublishedOrSentProfile(const Hash& aFingerPrint,
                                        const QByteArray& aContent,
                                        const QByteArray& aSignature,
                                        const QList<quint32>& aBangPath,
                                        const QByteArray& aProfilePublicKey,
                                        const unsigned char aFlags,
                                        const quint32 aTimeStamp,
                                        bool aWasPublish,
                                        const Hash& aFromNode ) ;
    /**
     * Overridden from ModelBase
     */
    virtual bool deleteOldestDataRowInTable() ;
signals:
    void error(MController::CAErrorSituation aError,
               const QString& aExplanation) ;
    /** emitted when new profile is received */
    void contentReceived(const Hash& aHashOfContent,
                         const ProtocolItemType aTypeOfReceivdContent) ;
private: // member variables:
    MController *iController  ;
    const MModelProtocolInterface& iModel ;
} ;
#endif
