//# FlagPlotServerProxy.h: Flagger's version of PlotServerProxy
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
//# $Id$

#ifndef DBUS_LFPLOTSERVERPROXY_H
#define DBUS_LFPLOTSERVERPROXY_H

#include <stdio.h>
#include <math.h>
#include <casadbus/plotserver/PlotServerProxy.h>
#include <casadbus/utilities/BusAccess.h>
#include <casadbus/session/DBusSession.h>

namespace casa {

class FlagPlotServerProxy : public PlotServerProxy {
    public:
	FlagPlotServerProxy( char *&dbusname ) : PlotServerProxy(dbusname ) { }
	void exiting( ) {
	    casa::DBusSession::instance( ).dispatcher( ).leave( );
            returnvalue = "Quit";
	}
	void closing(const int32_t& /*panel*/, const bool &/*gone*/) {
	    casa::DBusSession::instance( ).dispatcher( ).leave( );
	    //std::cout << "closing panel: " << panel << " (" << gone << ")..." << std::endl;
            returnvalue = "Quit";
	}

	dbus::variant panel( const std::string& title, const std::string &xlabel="", const std::string &ylabel="",
			     const std::string &window_title="", const std::vector<int> &size=std::vector<int>( ),
			     const std::string& legend="bottom", const std::string &zoom="bottom",
			     const int32_t& with_panel=0, const bool& new_row=false, const bool& hidden=false )
         {
	    dbus::variant v = PlotServerProxy::panel( title,xlabel,ylabel,window_title,size,
									     legend,zoom,with_panel,new_row,hidden );
	    if ( v.type( ) == dbus::variant::INT ) {
		p = v.getInt( );
	    }
	    return v;
	}
 
	dbus::variant line( const std::vector< double >& x, const std::vector< double >& y, const std::string& color="black",
			    const std::string& label="", const int32_t& panel=0 ) {
	    dbus::variant v = PlotServerProxy::line( x, y, color, label, panel );
	    if ( v.type( ) == dbus::variant::INT ) {
		l = v.getInt( );
	    }
	    return v;
	}

  dbus::variant raster(  const std::vector<double> &matrix, int sizex, int sizey, const std::string& colormap="Greyscale 1", int panel=0 )
{
  dbus::variant v = PlotServerProxy::raster( matrix, sizex, sizey, colormap, panel );
	    if ( v.type( ) == dbus::variant::INT ) {
		l = v.getInt( );
	    }
	    return v;
	}

	void draw( std::vector<double> x, std::vector<double> y, std::string color="blue" ) {
	    xvals = x;
	    yvals = y;
	    dbus::variant v = line(x,y,color,"",p);
	    if ( v.type( ) == dbus::variant::INT ) {
		l = v.getInt( );
	    }
	}

	void button(const int32_t& /*panel*/, const std::string& name) {
	  //std::cout << "button: " << name << "/" << panel << std::endl;
              returnvalue = name;
	      //	      std::cout << returnvalue << std::endl;
 	      casa::DBusSession::instance( ).dispatcher( ).leave( );
	}

	void check(const int32_t& /*panel*/, const std::string& name, const int32_t& state) {
	  //std::cout << "check: " << name << "/" << panel << " <" << state << ">" << std::endl;
	  stringstream rval;
	  rval << name << ":"<< state;
	  returnvalue = rval.str();
	  casa::DBusSession::instance().dispatcher().leave();
	}

	void radio(const int32_t& panel, const std::string& name, const bool& state) {
	    std::cout << "radio: " << name << "/" << panel << " <" << state << ">" << std::endl;
	}

	void linetext(const int32_t& panel, const std::string& name, const std::string& text) {
	    std::cout << "line: " << name << "/" << panel << " <" << text << ">" << std::endl;
	}

	void slidevalue(const int32_t& panel, const std::string& name, const int32_t& value) {
	    std::cout << "slider: " << name << "/" << panel << " <" << value << ">" << std::endl;
	}

    
  std::string eventloop()
  {
      casa::DBusSession::instance( ).dispatcher( ).enter( );
      return returnvalue;
  }


    private:
	int p;
	int l;
	std::vector<double> xvals;
	std::vector<double> yvals;
 
  std::string  returnvalue;

};

}
#endif
