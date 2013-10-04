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

#ifndef PLOTINFORMATIONMANAGER_H_
#define PLOTINFORMATIONMANAGER_H_

namespace casa {

/**
 * Abstracts directions to the PlotMSPlotTab for GUI implementation
 * independence.
 */

class PlotInformationManager {
public:
	 virtual void insertData( int insertIndex ) = 0;
	 virtual void insertAxes( int insertIndex ) = 0;
	 virtual void insertIterate( int insertIndex ) = 0;
	 virtual void insertTransformations( int insertIndex ) = 0;
	 virtual void insertDisplay( int insertIndex ) = 0;
	 virtual void insertCanvas( int insertIndex ) = 0;
	 virtual void insertExport( int insertIndex ) = 0;
	 virtual void clearAfter( int startClearIndex) = 0;
protected:
	 PlotInformationManager(){};
	 virtual ~PlotInformationManager(){};
};

}

#endif /* PLOTINFORMATIONMANAGER_H_ */
