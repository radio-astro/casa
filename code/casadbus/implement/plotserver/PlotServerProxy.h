//# PlotServerProxy.h: allows control of the viewer from C++ via DBus
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

#ifndef DBUS_PLOTSERVERPROXY_H_
#define DBUS_PLOTSERVERPROXY_H_

#include <vector>
#include <string>
#include <casadbus/plotserver/PlotServerProxy.dbusproxy.h>
#include <casadbus/utilities/Conversion.h>
#include <casa/Containers/Record.h>
#include <xmlcasa/variant.h>

namespace casa {
    class PlotServerProxy :
	private edu::nrao::casa::plotserver_proxy,
	public DBus::IntrospectableProxy,
	public DBus::ObjectProxy {

    public:

	static const char **execArgs( );

	PlotServerProxy( );

	dbus::variant panel( const std::string& title, const std::string &xlabel="", const std::string &ylabel="",
			     const std::string &window_title="", const std::string& legend="bottom", const bool& hidden=false )
	  { return dbus::toVariant( edu::nrao::casa::plotserver_proxy::panel(title,xlabel,ylabel,window_title,legend,hidden) ); }
	std::vector< std::string > colors( )
			{ return edu::nrao::casa::plotserver_proxy::colors( ); }
	std::vector< std::string > symbols( )
			{ return edu::nrao::casa::plotserver_proxy::symbols( ); }
	dbus::variant line( const std::vector< double >& x, const std::vector< double >& y, const std::string& color="black",
			    const std::string& label="", const int32_t& panel=0 )
			{ return dbus::toVariant(edu::nrao::casa::plotserver_proxy::line( x, y, color, label, panel)); }
	dbus::variant scatter( const std::vector< double >& x, const std::vector< double >& y, const std::string& color="black",
			       const std::string& label="", const std::string& symbol="", const int32_t& symbol_size=-1,
			       const int32_t& dot_size=-1, const int32_t& panel=0)
			{ return dbus::toVariant(edu::nrao::casa::plotserver_proxy::scatter( x, y, color, label, symbol,
											     symbol_size, dot_size, panel)); }
	dbus::variant histogram( const std::vector< double >& values, int bins=0, const std::string& color="blue",
				 const std::string& label="", const int panel=0 )
			{ return dbus::toVariant(edu::nrao::casa::plotserver_proxy::histogram( values, bins, color, label, panel)); }
	dbus::variant raster( const std::vector<double> &matrix, int sizex, int sizey, int panel=0 )
			{ return dbus::toVariant(edu::nrao::casa::plotserver_proxy::raster( matrix, sizex, sizey, panel )); }

	dbus::variant erase(const int32_t& data_or_panel=0)
			{ return dbus::toVariant(edu::nrao::casa::plotserver_proxy::erase(data_or_panel)); }
	dbus::variant close(const int32_t& panel=0)
			{ return dbus::toVariant(edu::nrao::casa::plotserver_proxy::close(panel)); }
	dbus::variant release(const int32_t& panel=0)
			{ return dbus::toVariant(edu::nrao::casa::plotserver_proxy::release(panel)); }

	dbus::variant show(const int32_t& panel=0)
			{ return dbus::toVariant(edu::nrao::casa::plotserver_proxy::show(panel)); }
	dbus::variant hide(const int32_t& panel=0)
			{ return dbus::toVariant(edu::nrao::casa::plotserver_proxy::hide(panel)); }

	bool done() { return edu::nrao::casa::plotserver_proxy::done( ); }

    };
}
#endif
