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

#ifndef CA_METADATA_DIALOG_H
#define CA_METADATA_DIALOG_H

#include <QDialog>
#include "../mcontroller.h"
#include "../ui_metadataQuery.h"

/**
 * @brief class for querying metadata of file about to get published
 */
class MetadataQueryDialog : public QDialog
{
Q_OBJECT

public:
/**
 * This structure contains results of @ref MetadataQueryDialog
 * e.g the dialog queries this dataset from user
 */
  typedef struct MetadataResultSetStruct {
    QString iFileName; /**< name of file in filesystem */
    QString iDescription;
    QString iMimeType;
    QString iOwner;
    QString iLicense;
  } MetadataResultSet ;
  /**
   * Constructor.
   * @param aParent is the parent widget
   * @param aController is application controller instance
   * @param aResultsSet is reference to structure where this dialog
   *                    will store its results. 
   */
  MetadataQueryDialog(QWidget *aParent,
		      MController& aController,
		      MetadataResultSet& aResultsSet );
  /** destructor */
  ~MetadataQueryDialog();
public slots:
  void okButtonClicked() ; 

signals:
  void error(MController::CAErrorSituation aError,
	     const QString& aExplanation) ;
private: // methods
  QString findMimeTypeForFile(const QString& aFileName) ;
private:
  Ui_metadataQuery ui ; 
  MController& iController ;
  MetadataResultSet& iResultsSet ;
};

#endif
