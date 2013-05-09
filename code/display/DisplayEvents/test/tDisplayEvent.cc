//# tDisplayEvent.cc: test program for DisplayEvent class
//# Copyright (C) 1999,2000,2002
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

#include <casa/aips.h>
#include <casa/Exceptions/Error.h>
#include <casa/Utilities/Assert.h>
#include <display/DisplayEvents/DisplayEvent.h>
#include <casa/iostream.h>
#include <unistd.h>

#include <casa/namespace.h>
void localAbort(String message) {
	cout << message << endl;
	exit(1);
}

int main(int, char **) {
	try {

		// ------------------------------------------------------------
		// Test that construction gives sensibly ordered events
		DisplayEvent de1;
		sleep (2);
		DisplayEvent de2;
		AlwaysAssert(de2.timeOfEvent() > de1.timeOfEvent(), AipsError);

		// ------------------------------------------------------------
		// Test copy assignment
		de2 = de1;
		AlwaysAssert(de2.timeOfEvent() == de1.timeOfEvent(), AipsError);

		// ------------------------------------------------------------
		// Test copy constructor
		DisplayEvent de3(de2);
		AlwaysAssert(de3.timeOfEvent() == de1.timeOfEvent(), AipsError);

		cout << "OK" << endl;
		return 0;
	} catch (const AipsError &x) {
		localAbort(x.getMesg());
	}
}
