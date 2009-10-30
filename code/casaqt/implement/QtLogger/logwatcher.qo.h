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
//# $Id:  $

#ifndef LOGWATCHER_H
#define LOGWATCHER_H


#include <graphics/X11/X_enter.h>
#include <QDebug>
#include <QTimer>
#include <QDateTime>
#include <QFileInfo>
#include <graphics/X11/X_exit.h>

namespace casa {

class FileWatcher : public QObject {
   Q_OBJECT

public:
   FileWatcher(QString &file):watchFile(file), size(0){
      info.setFile(watchFile);
      size = info.size(); 
      //qDebug() << "path=" << info.path() << " file=" << file;
      info.setCaching(false);
      dt = info.lastModified();
      //qDebug() << "dt=" << dt.toString();
      QTimer* timer = new QTimer(this);
      connect(timer, SIGNAL(timeout()), this, SLOT(update()));
      timer->start(500);
   }
   ~FileWatcher(){}
public slots:
   void update() {
      info.refresh();
      /*
      QDateTime dlt = info.lastModified();
      //qDebug() << "dlt=" << dlt << " dt=" << dt; 
      if (dlt != dt) {
        dt = dlt;
        //qDebug() << "emit file changed";
        emit fileChanged(watchFile); 
      }
      */
      int sz = info.size(); 
      if (sz > size) {
        size = sz;
        //qDebug() << "emit file changed";
        emit fileChanged(watchFile); 
      }
   } 
signals:
   void fileChanged(const QString &file); 
private: 
   QString watchFile;
   QDateTime dt;
   QFileInfo info;
   int size;
};


}
#endif
