//# SubMS.tcc 
//# Copyright (C) 2009
//# Associated Universities, Inc. Washington DC, USA.
//#
//# This library is free software; you can redistribute it and/or modify
//# it under the terms of the GNU General Public License as published by
//# the Free Software Foundation; either version 2 of the License, or
//# (at your option) any later version.
//#
//# This library is distributed in the hope that it will be useful,
//# but WITHOUT ANY WARRANTY; without even the implied warranty of
//# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//# GNU General Public License for more details.
//# 
//# You should have received a copy of the GNU General Public License
//# along with this library; if not, write to the Free Software
//# Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//#
//# Correspondence concerning AIPS++ should be addressed as follows:
//#        Internet email: aips2-request@nrao.edu.
//#        Postal address: AIPS++ Project Office
//#                        National Radio Astronomy Observatory
//#                        520 Edgemont Road
//#                        Charlottesville, VA 22903-2475 USA
//#
#include <msvis/MSVis/SubMS.h>
#include <tables/Tables/Table.h>
#include <tables/Tables/TableDesc.h>
#include <casa/BasicSL/String.h>

namespace casa { //# NAMESPACE CASA - BEGIN

  // Add optional columns to outTab if present in inTab and possColNames.
  // Returns the number of added columns.
  template<class M>
  uInt SubMS::addOptionalColumns(const M& inTab, M& outTab,
				 const Vector<String>& possColNames,
				 const Bool beLazy)
  {
    uInt nAdded = 0;
    uInt nPossOpts = possColNames.nelements();
    
    const TableDesc& inTD = inTab.actualTableDesc();

    // Only rely on the # of columns if you are sure that inTab and outTab
    // can't have the same # of columns without having _different_ columns,
    // i.e. use beLazy if outTD is in its default state.
    uInt nInCol = inTD.ncolumn();
    if(!beLazy || nInCol > outTab.actualTableDesc().ncolumn()){
      Vector<String> oldColNames = inTD.columnNames();

      for(uInt k = 0; k < nInCol; ++k){
	for (uInt j = 0; j < nPossOpts; ++j){
	  if(oldColNames[k].contains(possColNames[j])){
	    TableDesc tabDesc;
	    M::addColumnToDesc(tabDesc, M::columnType(possColNames[j]));
	    outTab.addColumn(tabDesc[0]);
	    ++nAdded;
	  }
	}
      }
    }
    return nAdded;
  }
  
} //# NAMESPACE CASA - END


