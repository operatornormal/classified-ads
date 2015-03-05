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

#ifndef PRIVMSGSEARCHMODEL_H
#define PRIVMSGSEARCHMODEL_H

#include <QAbstractListModel>
#include "../util/hash.h"
#include "../mcontroller.h"
#include <QIcon>

class Model ; 
class PrivMessage ; 
/**
 * @brief Model-class for performing search on private messages.
 * This is supposed to act as an underlying data-container for 
 * an user-interface view (QListView etc.)
 */
class PrivateMessageSearchModel: public QAbstractTableModel {
 Q_OBJECT
public: // types:
  struct PrivateMessageListItem
  {
    Hash iMessageHash ; 
    Hash iRecipientHash ; 
    Hash iSenderHash ; 
    quint32 iMessageTimeStamp ; 
    QString iMessageSubject ; 
    QString iSenderName ; 
    bool iIsRead ; 
    QString iTrustingProfileName ; 
  } ; 
public: 
 PrivateMessageSearchModel(Model& aModel,MController& aController) ;
  ~PrivateMessageSearchModel() ; 

  /**
   * Calling this method will populate the model with
   * messages that are either sent by profile given in
   * parameter, or destined to given profile.
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
   * @return data to display in list
   */
  virtual QVariant data(const QModelIndex & index, int role = Qt::DisplayRole) const ;
  virtual QVariant headerData ( int section, Qt::Orientation orientation, int role = Qt::DisplayRole ) const ;

  void setAsRead(const Hash& aMessage, bool aIsRead) ; 
  /** 
   * notification method called when new private message
   * is inserted into database 
   */
  void newMsgReceived(const Hash& aMessage,const Hash& aRecipient) ; 
  /** 
   * notification method called when new private message
   * is inserted into database , this variant is called
   * from UI
   */
  void newMsgReceived(const PrivMessage& aMessage) ; 
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
private:// methods
  void performSearch() ; 
  void updateSenderAndSubjectOfMsg(PrivateMessageListItem& aItem) ;

private: // data
  Model& iModel ; 
  MController& iController ;
  QList<PrivateMessageListItem> iPrivateMessages ; 
  Hash iSearchHash ;
  QIcon iLeftIcon;
  QIcon iRightIcon;
} ; 
#endif
