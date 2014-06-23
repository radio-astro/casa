//# tDlHandle.cc: test display library smart pointers.
//# Copyright (C) 2010
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
#include <display/Utilities/DlTarget.h>
#include <string>
#include <iostream>

// Should generate output like:
//
// -------------------- begin test --------------------
// <1> try one
//	  <2> try one
//		    <3> ... 'try one' pointer successfully threw an exception upon a null pointer...
//		    ... handle is null ...
// <4> try two
//	  <5> try two
//		    ... handle is not null ...
// <6> try two
//	  ... handle is not null ...
//	  <7> try two(c)
//		    ... handle is not null ...
//		    ... deleting target...
//		    ... handle is null ...
//		    ... handle is null ...
//		    ... handle is null ...
//----------------------------------------------------
//

class obj : public casa::DlTarget {
public:
	obj( const std::string &s ) : str(s) { }
	std:: string value( ) {
		return str;
	}
	std:: string value( ) const {
		return str + "(c)";
	}
private:
	std::string str;
};

int main( int argc, char **argv ) {
	std::cout << "-------------------- begin test --------------------" << std::endl;
	obj *obj1 = new obj("try one");
	std::cout << "<1> " << obj1->value() << std::endl;
	casa::DlHandle<obj> p1(obj1);
	std::cout << "<2> " << p1->value() << std::endl;
	try {
		delete obj1;
		std::cout << "<3> " << p1->value() << std::endl;
	} catch (...) {
		std::cout << "<3> ... 'try one' pointer successfully threw an exception upon a null pointer..." << std::endl;
		std::cout << "\t\t... handle is " << (p1.null() ? "null" : "not null") << " ..." << std::endl;
	}
	obj *obj2 = new obj("try two");
	{
		casa::DlHandle<obj> p2(obj2);
		std::cout << "<4> " << p2->value() << std::endl;
	}
	casa::DlHandle<obj> p3;
	p3 = obj2;
	std::cout << "<5> " << p3->value() << std::endl;
	std::cout << "\t\t... handle is " << (p3.null() ? "null" : "not null") << " ..." << std::endl;
	casa::DlHandle<obj> p4;
	p4 = p3;
	std::cout << "<6> " << (*p4).value() << std::endl;
	std::cout << "\t\t... handle is " << (p4.null() ? "null" : "not null") << " ..." << std::endl;

	const casa::DlHandle<obj> p5(p4);
	std::cout << "<7> " << (*p5).value() << std::endl;
	std::cout << "\t\t... handle is " << (p5.null() ? "null" : "not null") << " ..." << std::endl;

	std::cout << "\t\t... deleting target..." << std::endl;
	delete obj2;
	std::cout << "\t\t... handle is " << (p3.null() ? "null" : "not null") << " ..." << std::endl;
	std::cout << "\t\t... handle is " << (p4.null() ? "null" : "not null") << " ..." << std::endl;
	std::cout << "\t\t... handle is " << (p5.null() ? "null" : "not null") << " ..." << std::endl;

	std::cout << "----------------------------------------------------" << std::endl;
}
