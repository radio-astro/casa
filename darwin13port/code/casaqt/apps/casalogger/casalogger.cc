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


#include <casaqt/QtLogger/logviewer.qo.h>
#include <casaqt/QtLogger/logmodel.qo.h>

#include <graphics/X11/X_enter.h>
#include <QApplication>
#include <QtGui>
#include <graphics/X11/X_exit.h>

#include <casa/namespace.h>
#include <casa/iostream.h>
#include <casa/System/Aipsrc.h>
#include <signal.h>

int main( int argc, char ** argv )
{

    // don't let ^C kill the logger...
    signal(SIGINT,SIG_IGN);

    //for (int i=0;i<argc;i++)
    //std::cerr << "Logviewer wants: " 
    //          << i << " " << argv[i] << std::endl;
    Q_INIT_RESOURCE(QtLogger);

    QApplication a( argc, argv );

    //std::cerr << "Logviewer wants: " << argv[0] << std::endl;
    //std::cerr << "Logviewer wants: " << argv[1] << std::endl;

    char *mylog(0);
    if(argc > 1){
	  mylog = argv[1];
    } else {
	casa::String logfileKey="user.logfile";
	casa::String logname2;
	if(!casa::Aipsrc::find(logname2, logfileKey)){
	   mylog = const_cast<char *>("casapy.log");
	} else {
	   mylog = const_cast<char *>(logname2.c_str());
	}
    }

    // std::cerr << "Logviewer wants: " << mylog << std::endl;
    LogViewer mw(mylog);
    mw.setAttribute(Qt::WA_ShowWithoutActivating);
    mw.show();

    return a.exec();
}

