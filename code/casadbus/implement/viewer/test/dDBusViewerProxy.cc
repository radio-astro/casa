//# dDBusViewerProxy.cc: demonstrate the use of DBusSession & ViewerProxy
//# Copyright (C) 2009
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
#include <casadbus/viewer/ViewerProxy.h>
#include <casadbus/session/DBusSession.h>
#include <vector>
#include <string>
#include <iostream>

int main( int argc, const char *argv[ ] ) {

    casa::DBusSession &session = casa::DBusSession::instance( );
    std::vector<std::string> name_list(session.listNames( ));

    std::vector<std::string> viewers;
    std::string casa_prefix("edu.nrao.casa.");
    std::string viewer_prefix(casa_prefix + "viewer_");
    for ( std::vector<std::string>::iterator iter = name_list.begin(); iter != name_list.end( ); ++iter ) {
	if ( ! iter->compare(0,viewer_prefix.size(),viewer_prefix) )
	    viewers.push_back(*iter);
    }
    std::cout << "found " << viewers.size() << (viewers.size() > 1 ? " viewers..." : " viewer...") << std::endl;
    for ( std::vector<std::string>::iterator iter = viewers.begin(); iter != viewers.end( ); ++iter ) {
	std::cout << "\t" << *iter << std::endl;
    }
    std::cout << (viewers.size() > 1 ? "they are " : "it is ") << "described as..." << std::endl;
    for ( std::vector<std::string>::iterator iter = viewers.begin(); iter != viewers.end( ); ++iter ) {
	std::string name = iter->substr(casa_prefix.size());
	casa::ViewerProxy vp( "/casa/" + name, *iter );
	// TODO: replace cwd() with a "status( )" (or something) function
	//       which summarizes what the viewer is showing
	std::cout << "\t" << vp.cwd( ) << std::endl;
    }
    
}
