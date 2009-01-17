//# Copyright (C) 2005
//# Associated Universities, Inc. Washington DC, USA.
//#
//# This library is free software; you can redistribute it and/or modify it
//# under the terms of the GNU Library General Public License as published by
//# the Free Software Foundation; either version 2 of the License, or (at your
//# option) any later version.
//#
//# This library is distributed in the hope that it will be useful, but WITHOUT
//# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
//# FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
//# License for more details.
//#
//# You should have received a copy of the GNU Library General Public License
//# along with this library; if not, write to the Free Software Foundation,
//# Inc., 675 Massachusetts Ave, Cambridge, MA 02139, USA.
//#
//# Correspondence concerning AIPS++ should be addressed as follows:
//#        Internet email: aips2-request@nrao.edu.
//#        Postal address: AIPS++ Project Office
//#                        National Radio Astronomy Observatory
//#                        520 Edgemont Road
//#                        Charlottesville, VA 22903-2475 USA
//#
//# $Id: $


#include <iostream.h>
#include <fstream.h>
#include <casa/System/ProgressMeter.h>
#include <casaqt/QtProgressMeter/progress.qo.h>

#include <graphics/X11/X_enter.h>
#include <QLayout>
#include <QDebug>
#include <QDateTime>
#include <graphics/X11/X_enter.h>

namespace casa {

ProgressBar::ProgressBar(const QStringList& str, QWidget *parent) 
    : QWidget(parent), state(str), last(0)
{
    //setMargin( 10 );
    QVBoxLayout* toplayout = new QVBoxLayout;
    setLayout(toplayout);

    lbl = new QLabel("");
    QVBoxLayout* vb1 = new QVBoxLayout;
    toplayout->addLayout(vb1);
    vb1->addWidget(lbl);

    progress = new QProgressBar;
    toplayout->addWidget(progress);

    //lbl->setFixedWidth(80);
    //setMinimumWidth(300);
    //setMaximumHeight(300);

    //setState();
}

void ProgressBar::setState(const QStringList &str)
{
   //if (str.size() < 5) return;

   if (str.size() == 5 && str[0] == state[0] &&
                          str[1] == state[1]) 
       state = str;

   bool s;
   uint start = state[0].toInt(&s);
   if (!s) return;

   uint current = QDateTime::currentDateTime().toTime_t();
   //qDebug() << "current:" << current << "start:" << start;

   int used;

   //int minimum = progress->minimum();
   int maximum = progress->maximum();
   int value = progress->value();

   used = current - start;
   if (maximum == value) {
      used = last - start;
   }
   else {
      last = current;
   }

   QString st = "";

   if (state[4] == state[3]) {
      progress->setRange(0, 1);
      progress->reset();
      progress->setValue(1);
      st = "Finished";

      if (current - last > 10) 
        emit finished(state[0] + "," + state[1]);
        //qDebug() << "remove " << state[0] << state[1];
   }
   else 
   if (state[2] == state[3]) {
      //qDebug() << "bus------------" ;
      progress->setRange(0, 0);
      progress->setValue(1);
      st = "Busy";
   }
   else {
      double min, max, val;
      bool n, x, l;
      min = state[2].toDouble(&n);   
      max = state[3].toDouble(&x);   
      val = state[4].toDouble(&l);   
      if (n && x && l) {
         //qDebug() << min << max << val ;
         while ((max - min) < 10) {
            min *= 10;
            max *= 10;
            val *= 10;
         }  
         if (min > max) {
            //possible 
            //ProgressMeter creation and _update do not care this
            double tmp = max;
            max = min;
            min = tmp;
         }
         if (val < min) val = min;
         if (val > max) {
            val = max;
            state[4] = state[3];
         }
         progress->setRange((int)min, (int)max);
         progress->setValue((int)val);
         st = "Running";
      }
   }

   lbl->setText(state[1] + " ---> used: " + 
                   QString::number(used) + 
                   "s" + " <--- " + st);

   return;
}

}
