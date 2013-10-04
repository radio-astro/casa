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
#include <casa/Containers/Record.h>
#include <casadbus/utilities/Conversion.h>
#include <casadbus/utilities/BusAccess.h>
#include <casadbus/types/variant.h>
#include <dbus-c++/types.h>
#include <dbus/dbus-shared.h>

using casa::Record;
using casa::Array;
using casa::Bool;
using casa::IPosition;

void show_variantmap( std::map<std::string, DBus::Variant> &variantmap );
void show_variantvec( std::vector<DBus::Variant> &variantvec );

void show_variant( DBus::Variant &variant ) {
	std::cout << "\t" << variant.signature() << std::endl;
	if ( variant.signature( ) == "s" ) {
	    std::cout << "\t\t\t\t" << (variant.operator std::string( )) << std::endl;
	} else if ( variant.signature( ) == "i" ) {
	    std::cout << "\t\t\t\t" << (variant.operator int( )) << std::endl;
	} else if ( variant.signature( ) == "u") {
	    std::cout << "\t\t\t\t" << (variant.operator unsigned( )) << std::endl;
	} else if ( variant.signature( ) == "d" ) {
	    std::cout << "\t\t\t\t" << (variant.operator double( )) << std::endl;
	} else if ( variant.signature( ) == "b" ) {
	    std::cout << "\t\t\t\t" << (variant.operator bool( )) << std::endl;
	} else if ( ! variant.signature( ).compare( 0, 2, "a{" ) ) {
	    std::map<std::string, DBus::Variant> vm(variant.operator std::map<std::string, DBus::Variant>( ));
	    std::cout << "\t\t\t\t==> " << vm.size() << std::endl;
	    show_variantmap(vm);
	} else if ( ! variant.signature( ).compare( 0, 2, "av" ) ) {
	    std::vector<DBus::Variant> vl(variant.operator std::vector<DBus::Variant>( ));
	    show_variantvec(vl);
	}
}

void show_variantmap( std::map<std::string, DBus::Variant> &variantmap ) {
    for ( std::map<std::string,DBus::Variant>::iterator it=variantmap.begin(); it != variantmap.end(); ++it ) {
	std::cout << "\t\t" << it->first << " -> ";
	show_variant( it->second );
    }
}

void show_variantvec( std::vector<DBus::Variant> &variantvec ) {
    for ( std::vector<DBus::Variant>::iterator it=variantvec.begin(); it != variantvec.end(); ++it ) {
	std::cout << "\t\t";
	show_variant( *it );
	std::cout << std::endl;
    }
}


class mycallback {
    public:
	mycallback( ) { }
	casa::dbus::variant result( ) { return casa::dbus::toVariant(result_); }
	bool callback( const DBus::Message & msg );
    private:
	DBus::Variant result_;
};

bool mycallback::callback( const DBus::Message &msg ) {
    if (msg.is_signal("edu.nrao.casa.viewer","interact")) {
	fprintf( stderr, "\tYESSSSSSSSSSS!!!!!!!!!!!!!!\n" );
	fprintf( stderr, "\tsender:      %s\n", msg.sender( ) );
	fprintf( stderr, "\tdestination: %s\n", msg.destination( ) );
	DBus::MessageIter ri = msg.reader( );
	ri >> result_;
	casa::DBusSession::instance( ).dispatcher( ).leave( );
    }
    return true;
}

int main( int argc, const char *argv[ ] ) {

    casa::ViewerProxy *vp = casa::dbus::launch<casa::ViewerProxy>( "view_server" );

    if ( vp == 0 ) {
	fprintf( stderr, "\t(n) crap, couldn't start viewer service...\n" );
	exit(1);
    }
	      
    std::string cwd = vp->cwd( );
    fprintf( stdout, "current directory: %s\n", cwd.c_str( ) );
    cwd = vp->cwd( "/Users/drs/dev/viewer/code/display/apps/casaviewer/casapy_het_test" );
    fprintf( stdout, "    new directory: %s\n", cwd.c_str( ) );

    casa::dbus::variant panel = vp->panel("clean");
    if ( panel.type() != casa::dbus::variant::INT ) {
	fprintf( stderr, "error: wrong type for panel id" );
	exit(1);
    }
    casa::dbus::variant im3 = vp->load( "test4.image", "raster", panel.getInt( ) );
    if ( im3.type() != casa::dbus::variant::INT ) {
	fprintf( stderr, "error: wrong type for data id" );
	exit(1);
    }

    mycallback *mycb = new mycallback( );
    DBus::MessageSlot filter;
    filter = new DBus::Callback<mycallback,bool,const DBus::Message &>( mycb, &mycallback::callback );
    casa::DBusSession::instance( ).connection( ).add_filter( filter );

    fprintf( stderr, "\t>>>>> ok, starting...\n" );
    sleep(5);

    casa::dbus::variant res = vp->start_interact( panel, panel.getInt( ) );
    casa::DBusSession::instance( ).connection( ).flush( );
    casa::DBusSession::instance( ).dispatcher( ).enter( );
    casa::dbus::variant interact_result = mycb->result( );
    casa::dbus::show( interact_result );

    sleep(5);

    vp->close(panel.getInt( ));
    delete vp;

}
