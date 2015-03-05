/*     -*-C++-*- -*-coding: utf-8-unix;-*-
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

#ifndef CALISTINGMODEL_H
#define CALISTINGMODEL_H

#include <QStandardItemModel>
#include "../util/hash.h"
#include "../mcontroller.h"
#include "camodel.h"

class BinaryFile ;
class Model ; 
class MController ; 
/**
 * @brief Model-class for helping display of classified ads.
 */
class CAListingModel: public QObject , public ClassifiedAdsModel::CAObserver {
  Q_OBJECT
public: 
  /**
   * Constructor
   * @param aForumToList specifies the classification of the ads 
   *        to include into model
   */
  CAListingModel(const Hash& aForumToList,
		 const MModelProtocolInterface &aModel,
		 MController* aController ) ;
  ~CAListingModel() ; /**< Destructor */

  /**
   * method for getting the actual model for view to consume.
   * ownership of the pointer is not transferred.
   */
  QStandardItemModel* theCaModel() ; 

  /**
   * method for setting another forum to be listed
   * @param aForumToList specifies the forum
   */
  void setClassification(const Hash& aForumToList) ; 
  /**
   * notification method telling abuot newly-persisted
   * classified ad. From CAObserver. 
   */
  virtual void newCaReceived(const CA& aNewCa)  ;
  /**
   * Notification method telling abuot newly-persisted
   * classified ad. Called from via controller.
   */
  virtual void newCaReceived(const Hash& aHashNewCa,
			     const Hash& aHashOfClassification)  ;

signals:
  void error(MController::CAErrorSituation aError,
             const QString& aExplanation) ;
private: // methods
  bool insertCaIntoModel(const Hash& aArticleFingerPrint) ;
private: // data
  QStandardItemModel iCaModel ; /**< this model is exposed to UI */
  Hash iForumToList ; /**< this model lists exactly one forum at time, given here */
  const MModelProtocolInterface &iModel ;  /**< datamodel reference */
  /**
   * This qhash contains pointers to items inside iCaModel
   * but the key in hash is the low-order bits of the fingerprints
   * of the articles as there seems to be no convenient way
   * of finding items from model based on fingerprint, only
   * search seems to be text-based
   */
  QHash<int, QStandardItem *>* iItemAndArticleHashRelation ; 
  MController *iController  ;
  QStandardItem *iListingHeaderDate ; 
  QStandardItem *iListingHeaderSubject ; 
} ; 
#endif
