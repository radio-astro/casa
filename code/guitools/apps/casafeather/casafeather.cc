//# Copyright (C) 2005,2009,2010
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
//# $Id$

#include <ios>
#include <iostream>
#include <casa/aips.h>
#include <casa/Inputs/Input.h>
#include <casa/BasicSL/String.h>
#include <casa/Exceptions/Error.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>

#include <unistd.h>
#include <sys/stat.h>

#include <guitools/Feather/FeatherMain.qo.h>
#include <QApplication>
#include <QDebug>
#include <casa/namespace.h>


int main( int argc, char *argv[] ) {
	int stat = 0;
	QApplication app(argc, argv );
	app.setOrganizationName( "CASA");
	app.setApplicationName( "Feather");
	try {
		FeatherMain featherApplication;
		featherApplication.show();
		/*featherApplication.ensurePolished();
		QSize rect = featherApplication.size();
		qDebug() << "Size of rectangle is "<<rect.width()<<" height="<<rect.height();
		QPixmap featherPixmap( rect.width(), rect.height() );
		featherApplication.render( &featherPixmap);
		featherPixmap.save("/users/slovelan/tmp/feather.png");*/
		stat = app.exec();
		return stat;
	}
	catch (const casa::AipsError& err) {
		cerr<<"**"<<err.getMesg()<<endl;
	}
	catch (...) {
		cerr<<"**non-AipsError exception**"<<endl;
	}
	return stat;
}







