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

#ifndef PROFILECOMMENTLISTINGMODEL_H
#define PROFILECOMMENTLISTINGMODEL_H

#include <QAbstractListModel>
#include "../util/hash.h"
#include "../mcontroller.h"
#include <QIcon>

class Model ; 
class ProfileComment ; 
/**
 * @brief Model-class for performing search on comments about a profile.
 * This is supposed to act as an underlying data-container for 
 * an user-interface view (QListView etc.)
 */
class ProfileCommentListingModel: public QAbstractTableModel {
 Q_OBJECT
public: // types:
 /**
  * Structure that holds profile data inside model .. could be as well
  * the actual comment but this has some lazy-loading stuff in it
  */
  struct ProfileCommentListItem
  {
    Hash iCommentHash ; /**< from ProfileComment::iFingerPrint */
    Hash iCommentedProfileHash ; 
    Hash iSenderHash ; 
    quint32 iCommentTimeStamp ; 
    QString iCommentSubject ; 
    QString iSenderName ; 
    bool iIsRead ; 
    QString iCommentText ; /**< comments are html so this text is <html>..</html>*/
    bool iIsUpdated ; /**< updated via lazy load */
    unsigned iNrOfAttachedFiles ; 
  } ; 
public: 
 ProfileCommentListingModel(Model& aModel,MController& aController) ;
  ~ProfileCommentListingModel() ; 

  /**
   * Calling this method will populate the model with
   * messages that are either sent by profile given in
   * parameter, or comment a given profile.
   * @param aSearch profile fingerprint whose messages are
   *        the item of intrest
   * @return no thing, always succeeds
   */
  void setSearchHash(const Hash& aSearch) ; 
  /**
   * re-implemented from QAbstractTableModel
   * @return number of rows in list 
   */
  virtual int rowCount(const QModelIndex & parent = QModelIndex())  const  ; 
  /**
   * re-implemented from QAbstractTableModel
   * @return number of columns in view
   */
  virtual int columnCount(const QModelIndex & parent = QModelIndex())  const  ; 
  /**
   * re-implemented from QAbstractListModel
   * @return data to display in list. If Qt::UserRole+1 is used to
   *         obtain data, html of the comment is returned, Qt::UserRole
   *         returns comment hash as QVariant. UserRole+2 returns
   *         header data as single string, intended for list displays.
   *         UserRole+3 returns number of attached files as unsigned.
   */
  virtual QVariant data(const QModelIndex & index, int role = Qt::DisplayRole) const ;
  virtual QVariant headerData ( int section, Qt::Orientation orientation, int role = Qt::DisplayRole ) const ;

  void setAsRead(const Hash& aComment, bool aIsRead) ; 
  /** 
   * notification method called when new comment
   * is inserted into database 
   */
  void newCommentReceived(const Hash& aComment,
			  const Hash& aCommentedProfile) ; 
  /** 
   * notification method called when new comment
   * is inserted into database , this variant is called
   * from UI
   */
  void newCommentReceived(const ProfileComment& aComment) ; 
signals:
  void error(MController::CAErrorSituation aError,
	     const QString& aExplanation) ;
public slots:
  /**
   * method called via QTimer::singleShot() that is used for 
   * filling missing parts (message subjects etc.) from data that
   * model is listing
   */
  void doUpdateDataOnIdle() ;
  /** 
   * called from controller when user profile is selected 
   */
  void profileSelected(const Hash&) ; 
private:// methods
  void performSearch() ; 
  /**
   * updates profile comment subject etc. from dabase.
   * @return false if update was not success indicating that
   *         comment should be removed from list
   */
  bool updateSenderAndSubjectOfMsg(ProfileCommentListItem& aItem) ;

private: // data
  Model& iModel ; 
  MController& iController ;
  QList<ProfileCommentListItem> iProfileComments ; 
  Hash iSearchHash ;
} ; 
#endif
