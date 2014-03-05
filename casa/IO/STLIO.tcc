//# ArrayIO.cc: text output and binary IO for an array of any dimensionality.
//# Copyright (C) 1993,1994,1995,1996,1997,1999,2000,2001,2002,2003
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
//# $Id: ArrayIO.tcc 20739 2009-09-29 01:15:15Z Malte.Marquarding $

//# Includes

#include <casa/IO/STLIO.h>
#include <casa/Logging/LogIO.h>
#include <iostream>
#include <iterator> 
// needed for internal IO

namespace casa { //# NAMESPACE CASA - BEGIN

template<class T>
ostream &operator<<(ostream &s, const std::set<T> &a) {
	ostringstream oss;
	std::ostream_iterator<T> out_it (oss,", ");
	copy ( a.begin(), a.end(), out_it );
	String sout = oss.str();
	sout.trim();
	sout.rtrim(',');
	s << "[" << sout << "]";
	return s;
}

template<class T>
LogIO &operator<<(LogIO &os, const std::set<T> &a)
{
    os.output() << a;
    return os;
}

template<class T>
ostream &operator<<(ostream &s, const std::vector<T> &a) {
	ostringstream oss;
	std::ostream_iterator<T> out_it (oss,", ");
	copy ( a.begin(), a.end(), out_it );
	String sout = oss.str();
	sout.trim();
	sout.rtrim(',');
	s << "[" << sout << "]";
	return s;
}

template<class T>
LogIO &operator<<(LogIO &os, const std::vector<T> &a)
{
    os.output() << a;
    return os;
}

template<class T, class U>
ostream &operator<<(ostream &s, const std::map<T, U> &a) {
	ostringstream oss;
	std::ostream_iterator<T> out_it (oss,", ");
	std::copy ( a.begin(), a.end(), out_it );
	String sout = oss.str();
	sout.trim();
	sout.rtrim(',');
	s << "{" << sout << "}";
	return s;
}

template<class T, class U>
LogIO &operator<<(LogIO &os, const std::map<T, U> &a)
{
    os.output() << a;
    return os;
}

template<class T, class U>
ostream &operator<<(ostream &s, const std::pair<T, U> &a) {
	s << "[" << a.first << ", " << a.second << "]";
	return s;
}

template<class T, class U>
LogIO &operator<<(LogIO &os, const std::pair<T, U> &a)
{
    os.output() << a;
    return os;
}


} //# NAMESPACE CASA - END

