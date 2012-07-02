//# GBTGOFiller.h: the generic GO filler
//# Copyright (C) 1999,2000,2001,2003,2004
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

//# Includes

#include <nrao/GBTFillers/GBTGOFiller.h>
#include <nrao/GBTFillers/GBTBackendFiller.h>

#include <nrao/FITS/GBTGOFile.h>

#include <casa/Exceptions/Error.h>
#include <fits/FITS/FITSTable.h>
#include <nrao/GBTFillers/GBTSimpleTable.h>
#include <casa/Logging/LogIO.h>
#include <measures/Measures/MEpoch.h>
#include <casa/OS/Path.h>
#include <tables/Tables/ScaColDesc.h>
#include <tables/Tables/SetupNewTab.h>
#include <tables/Tables/Table.h>
#include <tables/Tables/TableDesc.h>
#include <measures/TableMeasures/TableMeasDesc.h>
#include <measures/TableMeasures/TableMeasRefDesc.h>
#include <measures/TableMeasures/TableMeasValueDesc.h>
#include <measures/TableMeasures/TableQuantumDesc.h>
#include <casa/Utilities/Assert.h>
#include <casa/BasicSL/String.h>

GBTGOFiller::GBTGOFiller(Table &parent)
    : itsTable(0)
{
    // new or already existing?
    if (parent.keywordSet().fieldNumber("GBT_GO") < 0) {
	// it does not yet exist yet
	// Make a TableDesc with just the TIME and INTERVAL columns
	TableDesc td;
	// TIME is an MEpoch column
	td.addColumn(ScalarColumnDesc<Double>("TIME","Modified Julian Day"));
	TableMeasDesc<MEpoch> measCol(TableMeasValueDesc(td, "TIME"),
				      TableMeasRefDesc(MEpoch::DEFAULT));
	measCol.write(td);
	// and change the units to "s" from the default of "d"
	TableQuantumDesc timeqd(td,"TIME",Unit("s"));
	timeqd.write(td);

	// INTERVAL is a Quantity
	td.addColumn(ScalarColumnDesc<Double>("INTERVAL","The duration of the scan"));
	TableQuantumDesc intqd(td,"INTERVAL",Unit("s"));
	intqd.write(td);

	SetupNewTable newtab(parent.tableName()+"/GBT_GO", td, Table::New);
	Table tab(newtab, GBTBackendFiller::tableLock());
	parent.rwKeywordSet().defineTable("GBT_GO", tab);
    } 

    itsTable = new GBTSimpleTable(parent.tableName() + "/GBT_GO");
    AlwaysAssert(itsTable, AipsError);
}

GBTGOFiller::~GBTGOFiller() 
{
    delete itsTable;
    itsTable = 0;
}


void GBTGOFiller::fill(const GBTGOFile &goFile, Double time, Double interval,
		       Int trueScanNumber)
{
    Record fieldsToAdd;
    // add in time and interval
    fieldsToAdd.define("TIME", time);
    fieldsToAdd.define("INTERVAL", interval);

    // and the expected contents of the go file
    if (goFile.scan() != trueScanNumber) {
      // emit a warning message here, pretend it was really trueScanNumber
      LogIO os(LogOrigin("GBTGOFiller",
			 "GBTGOFiller(const GBTGOFile &, Double time, Double interval, Int trueScanNumber)"));
      os << LogIO::WARN << WHERE
	 << "The SCAN number found in the GO file for scan " << trueScanNumber
	 << " differs (it is " << goFile.scan() << ")\n"
	 << "The true scan number will be used instead.\n"
	 << "Other information in the GO file may also be incorrect."
	 << LogIO::POST;
      fieldsToAdd.define("SCAN", trueScanNumber);
    } else {
      fieldsToAdd.define("SCAN", goFile.scan());
    }
    fieldsToAdd.define("PROJID", goFile.projid());
    fieldsToAdd.define("OBJECT", goFile.object());
    fieldsToAdd.define("OBSERVER", goFile.observer());
    fieldsToAdd.define("PROCNAME", goFile.procname());
    fieldsToAdd.define("PROCTYPE", goFile.proctype());
    fieldsToAdd.define("PROCSEQN", goFile.procseqn());
    fieldsToAdd.define("PROCSIZE", goFile.procsize());
    fieldsToAdd.define("OBSTYPE", goFile.obstype());
    fieldsToAdd.define("SWSTATE", goFile.swstate());
    fieldsToAdd.define("SWTCHSIG", goFile.swtchsig());
    fieldsToAdd.define("LASTON", goFile.laston());
    fieldsToAdd.define("LASTOFF", goFile.lastoff());
    fieldsToAdd.define("COORDSYS", goFile.coordsys());
    fieldsToAdd.define("RADESYS", goFile.radesys());
    fieldsToAdd.define("EQUINOX", goFile.equinox());
    fieldsToAdd.define("MAJOR", goFile.majorCoord());
    fieldsToAdd.define("MINOR", goFile.minorCoord());
    if (goFile.isDoppler()) {
	fieldsToAdd.define("VELOCITY", goFile.velocityKW());
	fieldsToAdd.define("VELDEF", goFile.veldef());
	fieldsToAdd.define("RESTFRQ", goFile.restfrq().get().getValue());
    } else {
	fieldsToAdd.define("SKYFREQ", goFile.skyfreq().get().getValue());
    }

    itsTable->add(fieldsToAdd);
}
