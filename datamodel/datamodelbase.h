/*    -*-C++-*- -*-coding: utf-8-unix;-*-
  Classified Ads is Copyright (c) Antti Jarvinen 2013-2018.

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

#ifndef CLASSIFIED_DATAMODEL_BASE_H
#define CLASSIFIED_DATAMODEL_BASE_H
#include <QObject>
#include "../mcontroller.h" // because enum from there is needed

class QMutex ;
class Hash ;
class ContentEncryptionModel ;
class MController ; 
class MModelProtocolInterface ; 

/**
 * @brief datamodel-parts common part. this is inherited and contains common funcs
 *
 * There are several datamodel parts (profiles, files, ads etc.) that
 * need some almost-equivalent functions ; this class is an attempt to
 * provide those with minimal code duplication.
 */
class ModelBase : public QObject {
    Q_OBJECT

public:
    /**
     * Constructor.
     *
     * @param aDataTableName specifies the database table where (most) of the
     *         data related to this model-part is stored in
     * @param aMaxRowsToKeep initial value (from const.h) for max
     *         number of rows to keep in @ref aDataTableName table ;
     *         class will check for modified value in settings table
     *         and if there is any, it will be used.
     */
    ModelBase(QString aDataTableName,
              unsigned aMaxRowsToKeep,
	      MController* iController,
              MModelProtocolInterface& aModel ) ;
    ~ModelBase() ;
    /**
     * this method sets time of last reference for an object.
     * this is supposed to be called in situations where we
     * know that a human requested the object to be displayed ;
     * calls from UI layer are natural, also maybe situations
     * where neighboring node sends a request that can only
     * result from UI action.
     *
     * Reason for the UI-requirement is this: we use the time of
     * last reference to remove content from database, starting
     * from data that has not been referenced for some time.
     */
    bool setTimeLastReference(const Hash& aObjectFingerPrint,
                              quint32 aTimeWhenLastReferenced) ;
    /**
     * method for controlling table truncation length
     */
    unsigned getMaxRowsToKeep() ;
    /**
     * method for controlling table truncation length variable
     */
    void setMaxRowsToKeep(unsigned aRows)  ;
    /**
     * method that truncates the main data table size to
     * @ref iCurrentDbTableRowCount rows. This is to be
     * periodically called, currently via timer in model.
     */
    void truncateDataTableToMaxRows(void)  ;
    /** method for getting initial value for iCurrentDbTableRowCount */
    void updateDbTableRowCount() ;
signals:
    void error(MController::CAErrorSituation aError,
               const QString& aExplanation) ;

private: // methods
    virtual bool deleteOldestDataRowInTable() ;
protected:
    /**
     * name of primary table where data related to each model part is
     * kept ; in runtime this gets values like "binaryfile" or "profle"
     */
    QString iDataTableName ;
    /**
     * max number of rows to keep in db table whose name is in @ref iDataTableName
     */
    unsigned iMaxRowsToKeep ;
    /**
     * Number of rows currently on table. This may get incremented by
     * classes that inherint this class.
     */
    unsigned iCurrentDbTableRowCount ;
protected:
    MController *iController  ;
    MModelProtocolInterface& iModel ; 
} ;
#endif
