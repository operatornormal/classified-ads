/*     -*-C++-*- -*-coding: utf-8-unix;-*-
       Classified Ads is Copyright (c) Antti Järvinen 2013. 

       This file is part of Classified Ads.

       Classified Ads is free software: you can redistribute it and/or modify
       it under the terms of the GNU General Public License as published by
       the Free Software Foundation, either version 3 of the License, or
       (at your option) any later version.

       Classified Ads is distributed in the hope that it will be useful,
       but WITHOUT ANY WARRANTY; without even the implied warranty of
       MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
       GNU General Public License for more details.

       You should have received a copy of the GNU General Public License
       along with Classified Ads.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef SEARCH_DISPLAY_DIALOG_H
#define SEARCH_DISPLAY_DIALOG_H

#include "../mcontroller.h"
#include "../ui_searchDisplay.h"
#include "dialogbase.h"

class SearchModel ;
class Profile ; 
class QAction ; 
class QShortcut ;

/**
 * @brief class for search dialog
 */
class SearchDisplay : public DialogBase
{
  Q_OBJECT

public:
  /**
   * Constructor.
   *
   * @param aParent is owner-window of this dialog
   * @param aController application controller reference
   * @param aSearchModel where the content comes from. ownership is not
   *        claimed e.g. this dialog does not delete the model.
   * @param aSelectedProfile profile of operator currently searching
   */
  SearchDisplay(QWidget *aParent,
		MController* aController,
		SearchModel* aSearchModel ,
		const Profile& aSelectedProfile);
  /** destructor */
  ~SearchDisplay();
signals:
  void error(MController::CAErrorSituation aError,
             const QString& aExplanation) ;
private slots:
  void closeButtonClicked() ;
  void searchButtonClicked() ;/**< performs search */
  void keyEnterClicked() ;/**< conditionally performs search */
  void openButtonClicked() ; /**< opens selected item from results-list */
  void resultListDoubleClicked(const QModelIndex& aIndex);
  void currentItemChanged(const QModelIndex & current, const QModelIndex & previous ) ; 
  void openSelectedContent() ; 
private:
  Ui_searchDisplay ui ; 
  SearchModel* iSearchModel;
  Hash iHashOfFocusedResult ; 
  ProtocolItemType iTypeOfFocusedResult ; 
  Hash iNodeOfFocusedResult ; 
  QAction* iOpenAction ; /**< context-menu action for open */
  QShortcut* iSearchDisplayKeyboardGrabber ; 
}; 

#endif
