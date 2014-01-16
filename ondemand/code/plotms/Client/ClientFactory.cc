//# PlotMSAction.h: Actions on plotms that can be triggered.
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

#include "ClientFactory.h"
#include <plotms/PlotMS/PlotMS.h>
#include <plotms/Client/Client.h>
#include <plotms/Gui/PlotMSPlotter.qo.h>
#include <plotms/Client/ClientScript.h>
namespace casa {

ClientFactory::ClientFactory() {
	// TODO Auto-generated constructor stub

}

Client* ClientFactory::makeClient(  ClientType type, PlotMSApp* app ){
	Client* client = NULL;
	if ( type == GUI ){
		client = new PlotMSPlotter( app );
	}
	else if ( type == SCRIPT ){
		client = new ClientScript( app );
	}
	else {
		cout << "Unsupported client type="<<type<<endl;
	}
	return client;
}

ClientFactory::~ClientFactory() {
	// TODO Auto-generated destructor stub
}

} /* namespace casa */
