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

#ifndef ACTIONSUMMARY_H_
#define ACTIONSUMMARY_H_

#include <plotms/Actions/PlotMSAction.h>

namespace casa {

class ActionSummary  : public PlotMSAction {
public:
	ActionSummary( Client* client );
	void setSummaryType( PMS::SummaryType type );
	void setFile( String file );
	void setVerbose( bool verbose );

	virtual ~ActionSummary();
protected:

	virtual bool doActionSpecific(PlotMSApp* plotms);
	virtual bool loadParameters();
private:
	//vector<PlotMSPlot*> plots;
	PMS::SummaryType summaryType;
	String filename;
	bool verbose;
};

} /* namespace casa */
#endif /* PLOTMSACTIONSUMMARY_H_ */
