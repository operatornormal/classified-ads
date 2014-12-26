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

#include <QtGui>
#include "../mcontroller.h"
#include "../log.h"
#include "aboutdialog.h"

AboutDialog::AboutDialog(QWidget *aParent,
			 MController& aController)
  : QDialog(aParent),
    iController(aController)
{
  ui.setupUi(this) ; 

  connect (this, SIGNAL(rejected()), this, SLOT(deleteLater())) ;
}

AboutDialog::~AboutDialog()
{
  LOG_STR("AboutDialog::~AboutDialog\n") ;
}





