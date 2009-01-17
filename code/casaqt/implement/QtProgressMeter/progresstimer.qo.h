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


#ifndef PROGRESSTIMER_H
#define PROGRESSTIMER_H

#include <casaqt/QtProgressMeter/progress.qo.h>

#include <graphics/X11/X_enter.h>
#include <fstream.h>
#include <QProgressBar>
#include <QTimer>
#include <QList>
#include <QWidget>
#include <QMutex>
#include <QStringList>
#include <graphics/X11/X_exit.h>

namespace casa {

class ProgressTimer : public QWidget
{
    Q_OBJECT

public:
    ProgressTimer(QWidget *parent = 0);

protected:
    QList<ProgressBar*> progressList;
    QTimer timer;
    QMutex mutex;
    QStringList toBeRemoved;

protected slots:
    void timeout();
    void start();

public slots:
    void removeFinished(const QString&);
};


}
#endif
