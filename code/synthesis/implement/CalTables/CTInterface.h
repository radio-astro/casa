//# CTInterface.h: Class to present a CalTable with MS interface
//# Copyright (C) 1996,1997,1998,1999,2001
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
//#
//# $Id$

#ifndef CT_CTINTERFACE_H
#define CT_CTINTERFACE_H

#include <casa/aips.h>
#include <casa/BasicSL/String.h>
#include <ms/MeasurementSets/MSSelectableTable.h>
#include <ms/MeasurementSets/MSMainEnums.h>
#include <synthesis/CalTables/NewCalTable.h>
namespace casa { //# NAMESPACE CASA - BEGIN

  class CTInterface: public MSSelectableTable
  {
  public:
    CTInterface() {};
    CTInterface(const Table& table);

    virtual ~CTInterface();
    virtual const CTAntenna& antenna()                {return asCT()->antenna();}
    virtual const CTField& field()                    {return asCT()->field();}
    virtual const CTSpectralWindow& spectralWindow()  {return asCT()->spectralWindow();}
    virtual const MSDataDescription& dataDescription() {return (MSDataDescription)asCT()->spectralWindow();}
    virtual String columnName(MSMainEnums::PredefinedColumns nameEnum) 
                                                      {return MS::columnName(nameEnum);}
    virtual Bool isMS()                               {return False;};

    virtual const MeasurementSet* asMS() 
    {
      //{throw(AipsError("asMS() called from CTInterface class"));}
      cerr << "asMS() called from CTInterface class" << endl;
      //return static_cast<const MeasurementSet *>(table());
      return NULL;
    }
    virtual const NewCalTable* asCT();
  };
} //# NAMESPACE CASA - END

#endif
