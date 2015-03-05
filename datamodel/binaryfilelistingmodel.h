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

#ifndef BINARYFILELISTINGMODEL_H
#define BINARYFILELISTINGMODEL_H

#include <QAbstractTableModel>
#include "../util/hash.h"
#include "../mcontroller.h"

class BinaryFile ;
class Model ; 
/**
 * @brief Model-class for helping display of shared binary blobs,
 *        usually from a profile
 */
class BinaryFileListingModel: public QAbstractTableModel {
  Q_OBJECT
public: 
  BinaryFileListingModel(QList<Hash>& aFilesToList) ;/**< Constructor */
  ~BinaryFileListingModel() ; /**< Destructor */

  /**
   * method for removing a reader.
   * too.
   * @param aFingerPrint is fingerprint of the file to add
   */
  void addFile(const Hash& aFingerPrint) ; 
  /**
   * method for removing a file.
   * too.
   * @param aFingerPrint is fingerprint of the file to remove
   */
  void removeFile(const Hash& aFingerPrint) ; 

  /**
   * method for retrieving display-name of a file
   */
  QString fileDisplayNameByFingerPrint(const Hash& aFingerPrint)  ;

  /**
   * re-implemented from QAbstractTableModel
   * @return number of rows in list 
   */
   virtual int rowCount(const QModelIndex & parent = QModelIndex())  const  ; 
  /**
   * re-implemented from QAbstractTableModel
   * @return number of columns in model
   */
  virtual int columnCount ( const QModelIndex & parent = QModelIndex() ) const ;
  /**
   * re-implemented from QAbstractTableModel
   * @return data to display in list
   */
  virtual QVariant data(const QModelIndex & index, int role = Qt::DisplayRole) const ;
  virtual QVariant headerData(int aSection, Qt::Orientation orientation, int role) const ;
  /**
   * method for emptying model contents when viewed profile changes
   */
  void clear() ; 
signals:
  void error(MController::CAErrorSituation aError,
             const QString& aExplanation) ;
private: // methods
  void updateFileDataInArray( const Hash& aFingerPrint,bool aEmit ) ; 
private: // data
  QList<Hash>& iFilesToList ;
  QList<QPair<Hash,QString> > iNamesAndFingerPrints ; 
} ; 
#endif
