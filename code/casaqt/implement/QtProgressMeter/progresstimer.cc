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


#include <casa/iostream.h>
#include <casa/fstream.h>
#include <casa/System/ProgressMeter.h>
#include <casaqt/QtProgressMeter/progress.qo.h>
#include <casaqt/QtProgressMeter/progresstimer.qo.h>

#include <graphics/X11/X_enter.h>
#include <QLayout>
#include <QDebug>
#include <QFile>
#include <QTextStream>
#include <graphics/X11/X_enter.h>

namespace casa {

ProgressTimer::ProgressTimer(QWidget *parent)
    : QWidget(parent), progressList(), timer(), 
      mutex(), toBeRemoved()
{
    //setMargin( 10 );
    QVBoxLayout* toplayout = new QVBoxLayout;
    setLayout(toplayout);

    connect(&timer, SIGNAL(timeout()), this, SLOT(timeout()));

    setMinimumWidth(300);
    //setMinimumHeight(80);
    //setResizable(true);

    start();
}

void ProgressTimer::start()
{
    fstream file_op(ProgressMeter::PROGRESSFILE,ios::out);
    file_op.close();

    if (!timer.isActive()) {
        timer.start(1000);
    } 
}

void ProgressTimer::timeout()
{
    //remove the finished
    if (toBeRemoved.size() > 0) {
       mutex.lock();
       for (int j = 0; j < toBeRemoved.size(); j++) {
          for (int i = 0; i < progressList.size(); i++) {
             ProgressBar *pb = progressList[i];
             QStringList lst = pb->getState();
             if (toBeRemoved[j].left(10) == lst[0] && 
                 toBeRemoved[j].mid(11) == lst[1]) {
                layout()->removeWidget(pb);
                progressList.removeAt(i);
                //qDebug() << "remove" << lst[0];
                delete pb;
                layout()->update();
             }
          }
       }
       toBeRemoved.clear();
       mutex.unlock();
       resize(width(), progressList.size() * 80);
    }

    QFile file(ProgressMeter::PROGRESSFILE);
    if (!file.open(QIODevice::ReadWrite | QIODevice::Text))
       return;

    QString qstr;
    QTextStream in(&file);
    qstr = in.readAll();
    //bool rsze = 
    file.resize(0);
    file.close();

    //qDebug() << "Progress:" << qstr;
 
    QStringList item = qstr.split("\n");

    for (int k = 0; k < item.size(); k++) {   
       //qDebug() << "item:" << k << item[k];
       QStringList list = item[k].split(",");

       bool old = false;
       for (int i = 0; i < progressList.size(); i++) {
          ProgressBar *pb = progressList[i];
          QStringList lst = pb->getState();
          pb->setState(list);
          //qDebug() << list[0] << lst[0];
          if (list[0] == lst[0] && list[1] == lst[1]) 
              old = true;
       }
    
       if (!old && list.size() == 5) {
           ProgressBar *pbar = new ProgressBar(list);
           pbar->setState(list);
           progressList.append(pbar);
           layout()->addWidget(pbar);
           connect(pbar, SIGNAL(finished(const QString&)),
                this, SLOT(removeFinished(const QString&)));
       }

    }
    resize(width(), progressList.size() * 80);
    return;

}

void ProgressTimer::removeFinished(const QString& id)
{
    mutex.lock();
    toBeRemoved << id;
    mutex.unlock();
    return;
}

}
