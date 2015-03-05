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

#ifndef CA_ABOUT_DIALOG_H
#define CA_ABOUT_DIALOG_H

#include <QDialog>
#include "../mcontroller.h"
#include "../ui_aboutDialog.h"

/**
 * @brief class for displaying brief information about this sw
 */
class AboutDialog : public QDialog
{
Q_OBJECT

public:
  /**
   * Constructor.
   */
  AboutDialog(QWidget *aParent,
	      MController& aController );
  /** destructor */
  ~AboutDialog();

private slots:

private:
  Ui_aboutDialog ui ; 
  MController& iController ;
};

#endif
