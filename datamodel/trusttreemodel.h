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

#ifndef CLASSIFIED_TRUSTTREEMODEL_H
#define CLASSIFIED_TRUSTTREEMODEL_H
#include "../mcontroller.h" // because enum from there is needed
#include "datamodelbase.h"
#include "../net/protocol.h" // for ProtocolItemType

class Hash ;
class MModelProtocolInterface ;
class Profile ;

/**
 * @brief This is is part of datamodel for trust tree related operation
 *
 * Trust list of part of operator profile. Simple as that. This class
 * contains algorithms manipulating a tree constructed from lists of
 * operators where operators expores (dis)trust of 
 * each others in complex web of nodes and vertices. Main goal is
 * fast query about trustworthiness of given operator profile. 
 *
 * Classified ads treats trust as boolean ; operator either is trusted
 * or not. There is no distinction between recognizing the operators
 * identity and assigning trust to her - if operator is trusted it must
 * mean both at the same time: trusted operator is identified at 
 * least somehow and in addition is recognized as trustworthy animal, 
 * person or bot. 
 *
 * Output is also boolean. There are algorithms for producing floating
 * point numbers for this purpose .. say, half of your trusted friends
 * give some unknown operator "true" for trust, the other half does not
 * (either recognize or trust) so you could decide that his operator
 * enjoys some 50% trustworthiness - this is not done. Motivation is
 * simplicity both in concept and algorithms, the floating point 
 * methods tend to be computationally rather expensive. 
 */
class TrustTreeModel : public QObject {
  Q_OBJECT

public:
  TrustTreeModel(MController *aMController,
               const MModelProtocolInterface &aModel) ;
  ~TrustTreeModel() ;

  /**
   * method for injecting a trust list into model. This is called
   * when there is suspicion that trust list of a profile may
   * have been changed and this causes re-calculation of the trust
   * tree
   *
   * @param aTrustListOwnerProfile fingerprint of profile
   *        whose list is about to get processed
   * @param aOwnerProfileDisplayName human-readable name for
   *        the profile
   * @param aList is the list. This is a list of fingerprints
   *        of other profiles that aTrustListOwnerProfile indicates
   *        to be trustworthy. 
   */
  void offerTrustList(const Hash& aTrustListOwnerProfile,
		      const QString& aOwnerProfileDisplayName,
		      const QList<Hash>& aList) ; 

  /**
   * Method for making a query about a profile.
   *
   * @param aProfile fingerprint of the profile queried
   * @param aDisplayNameOfTrustingProfile pointer to string that,
   *        if non-null, in case of this method returns true,
   *        will have its value set to display-name of some
   *        profile that trusts aProfile
   * @param aFpOfTrustingProfile behaves the same way as
   *        aDisplayNameOfTrustingProfile but instead of display-name
   *        carries a fingerprint
   *
   * @return true if profile is trusted. 
   */
  bool isProfileTrusted(const Hash& aProfile,
			QString *aDisplayNameOfTrustingProfile,
			Hash* aFpOfTrustingProfile ) const ; 

  /**
   * method for initializing the model when profile changes
   * @param aPreviousSettings settings previously obtained using
   *        @ref trustTreeSettings method
   *
   */
  void initModel(const QVariant& aPreviousSettings) ; 

  /**
   * Method that exports model state as QVariant.
   */
  QVariant trustTreeSettings() const ; 
  /**
   * method for emptying model contents in event of profile change 
   */
  void clear(void) ; 
public slots:
  /** activated when new profile is received */
  void contentReceived(const Hash& aHashOfContent,
		       const ProtocolItemType aTypeOfReceivdContent) ;
private: // methods
  /**
   * method that takes contents of iTrustTree and if there is 
   * any additions etc. takes them into account
   */
  void recalculateTrust() ; 
  /**
   * sets profile trusted in trust list. called from recalculateTrust
   */
  void setProfileTrustedInList(const Hash& aTrustedProfile,
			       const Hash& aTrustingProfile,
			       int aLevel) ; 
signals:
  void error(MController::CAErrorSituation aError,
             const QString& aExplanation) ;

private: // member variables:
  MController& iController  ;
  const MModelProtocolInterface& iModel ;
  /**
   * internal trust list item
   */
  typedef struct TrustTreeItemStruct {
    Hash iTrustingOperator ; /**< who trusts this operator here */
    QString iOperatorName ; /**< name of operator */
    QList<Hash> iTrustList ; /**< trust list of the trusted operator */
    signed char iLevel ; /**< how many levels deep from our own profile */
    bool iNeedsUpdate ; /**< profile needs to be updated from db */
  } TrustTreeItem ; 
  QMap<Hash, TrustTreeItem>* iTrustTree ; 
  unsigned iLastUpdateTime ; /** ts of latest model update */
} ;
#endif
