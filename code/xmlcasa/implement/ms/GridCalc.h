//# GridCalc.h: spectral regridding utilities
//# Copyright (C) 2010 by ESO (in the framework of the ALMA collaboration)
//# Copyright (C) 1996,1997,1998,1999,2000,2001
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

#ifndef CASA_GRIDCALC_H
#define CASA_GRIDCALC_H


//# Includes
#include <xmlcasa/impl/casac/casac.h>
#include <xmlcasa/conversions.h>
#include <xmlcasa/variant.h>
#include <casa/aips.h>
#include <casa/aipstype.h>
#include <casa/Arrays/Array.h>
#include <casa/Arrays/Vector.h>
#include <vector>
#include <msvis/MSVis/SubMS.h>
#include <casa/Logging/LogOrigin.h>

namespace casac { //# NAMESPACE CASAC - BEGIN


// <summary>
// calculate the new centers and widths of spectral channels 
// </summary>

// <use visibility=export>

// <reviewed reviewer="" date="yyyy/mm/dd" tests="" demos="">
// </reviewed>

// <prerequisite>
//   <li> <linkto class="Vector">Vector</linkto>
//   <li> <linkto class="Array">Array</linkto>
//   <li> <linkto class="SubMS">SubMS</linkto>
// </prerequisite>

// <etymology>
// self-explanatory
// </etymology>

// <synopsis>
// Perform calculation of channel centers and widths
// </synopsis>

// <example>
// <srcBlock>
// </srcBlock>
// </example>

// <motivation>
// Make this method available both for cvel and clean
// </motivation>

// <todo asof="2010/10/26">
//   <li> 
// </todo>
 

  class GridCalc{

    GridCalc(){}; 
  
  public:
    
    static casa::Bool convertGridPars(casa::LogIO& os,
				      const std::string& mode, 
				      const int nchan, 
				      const casac::variant& start, 
				      const casac::variant& width,
				      const std::string& interp, 
				      const casac::variant& restfreq, 
				      const std::string& outframe,
				      const std::string& veltype,
				      //// output ////////
				      casa::String& t_mode,
				      casa::String& t_outframe,
				      casa::String& t_regridQuantity,
				      casa::Double& t_restfreq,
				      casa::String& t_regridInterpMeth,
				      casa::Double& t_cstart, 
				      casa::Double& t_bandwidth,
				      casa::Double& t_cwidth,
				      casa::Bool& t_centerIsStart, 
				      casa::Bool& t_startIsEnd,			      
				      casa::Int& t_nchan,
				      casa::Int& t_width,
				      casa::Int& t_start
				      );
    

    static casa::Bool calcChanFreqs(casa::LogIO& os,
				    casa::Vector<casa::Double>& newCHAN_FREQ, 
				    casa::Vector<casa::Double>& newCHAN_WIDTH,
				    casa::Vector<casa::Double>& oldCHAN_FREQ, 
				    casa::Vector<casa::Double>& oldCHAN_WIDTH,
				    const casa::MDirection  phaseCenter,
				    const casa::MFrequency::Types theOldRefFrame,
				    const casa::MEpoch theObsTime,
				    const casa::MPosition mObsPos,
				    const std::string& mode, 
				    const int nchan, 
				    const casac::variant& start, 
				    const casac::variant& width,
				    const casac::variant& restfreq, 
				    const std::string& outframe,
				    const std::string& veltype
				    );    
  };
  
}  //# NAMESPACE CASAC - END

#endif
