//# tDisplayTool.cc: test program for DisplayTool class
//# Copyright (C) 1999,2000,2001
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
#include <display/DisplayEvents/DisplayTool.h>
#include <casa/iostream.h>

#include <casa/namespace.h>
class tDisplayTool : public DisplayTool {
public:
	// Constructor.
	tDisplayTool(const Display::KeySym &keysym = Display::K_Pointer_Button1) :
		DisplayTool(keysym) { } ;
	// Copy constructor.
	tDisplayTool(const tDisplayTool &other) :
		DisplayTool(other) { };
	// Copy assignment.
	tDisplayTool &operator=(const tDisplayTool &other) {
		if (this != &other) {
			DisplayTool::operator=(other);
		}
		return *this;
	}
	// Destructor.
	virtual ~tDisplayTool() { };
	// Enable/disable.
	virtual void enable() { };
	virtual void disable() { };
	// return key modifier
	Display::KeyModifier getModifiers() {
		return keyModifiers();
	}
};

void localAbort(String message) {
	cout << message << endl;
	exit(1);
}

int main(int, char **) {
	try {

		// test default constructor gives correct button (ie. button 1)
		tDisplayTool tDT1;
		AlwaysAssert(tDT1.getKey() == Display::K_Pointer_Button1, AipsError);

		// test constructor with non-default button
		tDisplayTool tDT2(Display::K_Pointer_Button3);
		AlwaysAssert(tDT2.getKey() == Display::K_Pointer_Button3, AipsError);

		// test copy constructor
		tDisplayTool tDT3(tDT1);
		AlwaysAssert(tDT3.getKey() == tDT1.getKey(), AipsError);

		// test copy assignment
		tDisplayTool *tDT4 = new tDisplayTool(tDT2);
		AlwaysAssert(tDT4, AipsError);
		AlwaysAssert(tDT4->getKey() == tDT2.getKey(), AipsError);

		// test setkey/getkey functions
		tDT1.setKey(Display::K_Pointer_Button2);
		AlwaysAssert(tDT1.getKey() == Display::K_Pointer_Button2, AipsError);

		// test keymodifier function for button 2, and then for keyboard 'L'
		AlwaysAssert(tDT1.getModifiers(), AipsError);
		tDisplayTool tDT5(Display::K_L);
		AlwaysAssert(!tDT5.getModifiers(), AipsError);

		cout << "OK" << endl;
		return 0;
	} catch (const AipsError &x) {
		localAbort(x.getMesg());
	}
}
