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

#ifndef CLASSIFIED_CLASSIFIEDADMODEL_H
#define CLASSIFIED_CLASSIFIEDADMODEL_H
#include <QSqlDatabase>
#include "../mcontroller.h" // because enum from there is needed
#include "../net/protocol.h" // for SendQueueItem 
#include "datamodelbase.h"
#include <QStringList>

class Hash ;
class MModelProtocolInterface ;
class Profile ;
class CA ;

/**
 * @brief This is part of datamodel for storage/retrieval of classified ads.
 *
 * This is persistent storage and search and retrieval of objects
 * that are the classified ads that this program is about.
 */
class ClassifiedAdsModel : public ModelBase {
    Q_OBJECT

public:
    /**
     * In classified ads the ads are classified. The classifications
     * are defined here, first start about purpose of the ad
     */
    enum PurposeOfAd {
        ToBeBought=0, /**< buy stuff ad */
        ToBeSold=1, /**< stuff for sale -ad */
        ToBeGivenAway=2, /**< crown jewels about to be given away here */
        IsWanted=3, /**< something is wanted, against payment or not */
        ToBeRented=4, /**< rental agreement, both ways */
        ToBeAnnounced=5  /**< just announcement about topic */
    } ;
    /**
     * Another classification for an classified ad: what item/real life
     * stuff the ad concerns
     */
    enum ConcernOfAd {
        ConcerningCars=0, /**< is about cars */
        ConcerningBoats=1, /**< is about thing floating */
        ConcerningBikes=2, /**< is about 2-wheeled cars with no roof */
        ConcerningOtherVehicles=3, /**< is about other vehicles like moon rockets */
        ConcerningVehicleParts=4, /**< is about (spare) parts for moving things */
        ConcerningHabitation=5, /**< is about places of residence */
        ConcerningHouseholdAppliances=6, /**< is about machinery of residence */
        ConcerningFurniture=7, /**< is about furniture items */
        ConcerningClothing=8, /**< is about textiles */
        ConcerningTools=9, /**< is about tools */
        ConcerningSports=10, /**< is about sports, or items for sport */
        ConcerningMusic=11, /**< is about music or notes or instruments etc. */
        ConcerningBooks=12, /**< is about books */
        ConcerningMovies=13, /**< is about movies */
        ConcerningAnimals=14, /**< is about living creatures or accessories */
        ConcerningElectronics=15, /**< is about electronics */
        ConcerningJobs=16, /**< is about jobs like offers or requests */
        ConcerningTransportation=17, /**< is about moving things or persons */
        ConcerningServices=18, /**< is about services, any kind */
        ConcerningHealthcare=19, /**< is about health-care services or products */
        ConcerningFoodstuff=20, /**< is about things edible */
        ConcerningSoftware=21, /**< is about computer sw */
        ConcerningEvents=22, /**< is about events in time and place */
        ConcerningEducation=23, /**< is about education */
        ConcerningFinance=24, /**< is about financial services or products */
        ConcerningJewelry=25, /**< is about items for accenting ugliness */
        ConcerningReligiousRituals=26, /**< is about acts carrying attached meanings */
        ConcerningPhilosophy=27 /**< is about meanings */
    } ;

    /**
     * Classified ads are stored via this class (ClasifiedAdsModel) ; if
     * other parts of the sw wish to receive notifications about
     * about ads, here is observer that may be installed with
     * a method provided below. Method of the observer will be
     * called while datamodel is lock()ed.
     */
    class CAObserver {
        /**
         * notification method telling about newly-persisted
         * classified ad
         */
        virtual void newCaReceived(const CA& aNewCa) = 0 ;
    } ;
    /**
     * Constructor
     * @param aMController is application controller
     * @param aModel is datamodel
     * @return an instance
     */
    ClassifiedAdsModel(MController *aMController,
                       MModelProtocolInterface &aModel) ;
    /** destructor */
    ~ClassifiedAdsModel() ;

    /** string returned here is used for constructing hash of the classification */
    const QString& purposeOfAdString(PurposeOfAd aPurpose) const ;
    /** string returned is displayed to user. idea is that when hash of the
     * classification is constructed, non-localized string from @ref purposeOfAdString
     * is used and same hash will be obtained regardless of the language used
     */
    QString localizedPurposeOfAdString(PurposeOfAd aPurpose) const ;
    const QString& concernOfAdString(ConcernOfAd aConcern) const ;
    QString localizedConcernOfAdString(ConcernOfAd aConcern) const ;

    /**
     * sends a classified ads to selected nodes in network for others
     * to retrieve.
     *
     * @param aPublishingProfile is the publishing profile ; it is used to
     *        sign the content
     * @param aAd is the meat being thrown
     * @return Fingerprint of the published file or KNullHash if
     *         things went bad.
     */
    Hash publishClassifiedAd( const Profile& aPublishingProfile,CA& aAd ) ;

    /**
     * method for getting ca data for publish purpose
     * @param aTimeOfPublish if non-NULL will have its value
     *        set to publish time of the CA
     */
    bool caDataForPublish(const Hash& aFingerPrint,
                          QByteArray& aResultingCaData,
                          QByteArray& aResultingSignature,
                          QByteArray& aPublicKeyOfPublisher,
                          quint32* aTimeOfPublish = NULL ) ;

    /**
     * method for getting CA data for any purpose, like display
     */
    CA caByHash(const Hash& aFingerPrint) ;

    /**
     * called by protocol parser when a CA is received due to publish
     * @param aFingerPrint is the profile @ref Hash received from protocol header
     * @param aContent is actual ca data
     * @param aSignature is digital signature of the aContent
     * @param aBangPath is list of low-order bits of hashes of nodes
     *        where this content has been. This is for preventing
     *        sending content back to nodes where it has already been.
     * @param aKeyOfPublisher is the key that was used to sign the file.
     *        that is transferred outside file just because the recipient
     *        might not have the key  beforehand ; if she does, the existing
     *        key will be used in verifying process.
     * @param aFlags possible flags telling about encyption and compression
     * @param aTimeStamp timestamp of file (if encrypted, it must be carried outside..)
     * @return true on success
     */
    bool publishedCAReceived(const Hash& aFingerPrint,
                             const QByteArray& aContent,
                             const QByteArray& aSignature,
                             const QList<quint32>& aBangPath,
                             const QByteArray& aKeyOfPublisher,
                             const unsigned char aFlags,
                             const quint32 aTimeStamp,
                             const Hash& aFromNode ) ;

    /**
     * called by protocol parser when a CA is received due to send
     * @param aFingerPrint is the profile @ref Hash received from protocol header
     * @param aContent is actual ca data
     * @param aSignature is digital signature of the aContent
     * @param aKeyOfPublisher is the key that was used to sign the file.
     *        that is transferred outside file just because the recipient
     *        might not have the key  beforehand ; if she does, the existing
     *        key will be used in verifying process.
     * @param aFlags possible flags telling about encyption and compression
     * @param aTimeStamp timestamp of file (if encrypted, it must be carried outside..)
     * @return true on success
     */
    bool sentCAReceived(const Hash& aFingerPrint,
                        const QByteArray& aContent,
                        const QByteArray& aSignature,
                        const QByteArray& aKeyOfPublisher,
                        const unsigned char aFlags,
                        const quint32 aTimeStamp,
                        const Hash& aFromNode ) ;

    /**
     * method for installing observer for new ads
     */
    void installCAObserver(CAObserver* aObserver) ;
    /**
     * method for removing previously installed observer for new ads
     */
    void removeCAObserver(CAObserver* aObserver) ;

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
     * Method for fetching list of classified ads whose classification
     * matches given hash. This method produces content of reply to
     * protocol item KAdsClassifiedAtHash.
     *
     * @param aClassificationHash ad classification asked
     * @param aStartDate start of time period asked
     * @param aEndDate end of time period asked
     * @param aResultingArticles A list where results will be appended.
     *                           Hash is the CA hash, unsigned is its
     *                           timestamp
     * @param aRequestingNode fingerprint of node making the request
     */
    void caListingByClassification(const Hash& aClassificationHash,
                                   quint32 aStartDate,
                                   quint32 aEndDate,
                                   QList<QPair<Hash,quint32> >& aResultingArticles,
                                   const Hash& aRequestingNode ) ;
    /**
     * method for handling list of classified ads produced by method
     * @ref caListingByClassification. The thing is that we come to this
     * method when other node has sent us a list of ads available.
     * We may want to store the article fingerprints and try to obtain
     * them (the actual articles) later. For this reason the classified_ads
     * database table allows null values in content ; that may be fetched
     * later. Justification for this stupidity of not sending the article
     * with the hash is bandwidht: there may be a node keeping the list
     * of articles and rest of the ring keeping the articles.
     *
     * This will modify contents of aReceivedArticles to not contain
     * articles already found from db
     */
    bool caListingByClassificationReceived(QList<QPair<Hash,quint32> >& aReceivedArticles,
                                           const Hash& aRequestingNode,
                                           const Hash& aClassification) ;
    /** debug method, more or less. reads all ads and feeds them to
     * indexer
     */
    void reIndexAllAdsIntoFTS() ;

    const QStringList& aboutComboBoxTexts() const ; /**< returns ui texts */
    const QStringList& regardingComboBoxTexts() const ; /**< returns ui texts */
    const QStringList& whereComboBoxTexts() const ; /**< returns ui texts */
signals:
    void error
    (MController::CAErrorSituation aError,
     const QString& aExplanation) ;
    /** emitted when new classified ad is received.
     * @param aHashOfContent is fingerprint of the article
     * @param aHashOfClassification fingerprint of classification
     * @param aTypeOfReceivdContent Type of content. This model mostly emits
     *                              ClassifiedAd -types
     */
    void contentReceived(const Hash& aHashOfContent,
                         const Hash& aHashOfClassification,
                         const ProtocolItemType aTypeOfReceivdContent) ;
private: // methods
    /**
     * this method is called from @ref publishedCAReceived and
     * also on method where CA that is sent to us is to be
     * handled
     */
    bool doHandleReceivedCA(const Hash& aFingerPrint,
                            const QByteArray& aContent,
                            const QByteArray& aSignature,
                            const QList<quint32>& aBangPath,
                            const QByteArray& aKeyOfPublisher,
                            const unsigned char aFlags,
                            const quint32 aTimeStamp,
                            bool aWasPublish,
                            const Hash& aFromNode ) ;
    /** initializes string lists used for classification comboboxes */
    void initComboBoxTexts() ;
private: // member variables:
    QList<CAObserver*>* iNewCaObservers ;
    QStringList iAboutComboBoxTexts ;
    QStringList iRegardingComboBoxTexts ;
    QStringList iWhereComboBoxTexts ;
} ;
#endif
