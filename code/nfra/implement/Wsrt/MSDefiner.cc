//# MSDefiner.cc : class for the creation of a MeasurementSet
//# Copyright (C) 1996,1997,2000,2002
//# Associated Universities, Inc. Washington DC, USA.
//#
//# This program is free software; you can redistribute it and/or modify it
//# under the terms of the GNU General Public License as published by the Free
//# Software Foundation; either version 2 of the License, or (at your option)
//# any later version.
//#
//# This program is distributed in the hope that it will be useful, but WITHOUT
//# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
//# FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
//# more details.
//#
//# You should have received a copy of the GNU General Public License along
//# with this program; if not, write to the Free Software Foundation, Inc.,
//# 675 Massachusetts Ave, Cambridge, MA 02139, USA.
//#
//# Correspondence concerning AIPS++ should be addressed as follows:
//#        Internet email: aips2-request@nrao.edu.
//#        Postal address: AIPS++ Project Office
//#                        National Radio Astronomy Observatory
//#                        520 Edgemont Road
//#                        Charlottesville, VA 22903-2475 USA
//#
//# $Id$

#include <nfra/Wsrt/MSDefiner.h>
#include <tables/Tables.h>
#include <casa/Arrays/ArrayUtil.h>
#include <ms/MeasurementSets/MSColumns.h>
#include <casa/iostream.h>

MSDefiner::MSDefiner(const String& msName, DataSource* pds)
: itsMSName(msName),
  itspDS   (pds)
{}

MSDefiner::~MSDefiner() 
{}

MeasurementSet MSDefiner::run() const
{
    TableDesc td = msTableDesc();
    SetupNewTable newTab(itsMSName, td, Table::New);
    IncrementalStMan incrStMan ("ISMData");
    StandardStMan    stanStMan;
    TiledDataStMan   tiledStMan("TiledData");
    TiledColumnStMan tiledStManUVW("TiledUVW", IPosition(3,1,3,1024));

    newTab.bindAll (incrStMan);
    newTab.bindColumn(MS::columnName(MS::ANTENNA1),stanStMan);
    newTab.bindColumn(MS::columnName(MS::ANTENNA2),stanStMan);
    newTab.bindColumn(MS::columnName(MS::DATA_DESC_ID),tiledStMan);
    newTab.bindColumn(MS::columnName(MS::DATA),tiledStMan);
    newTab.bindColumn(MS::columnName(MS::FLAG),tiledStMan);
    newTab.bindColumn(MS::columnName(MS::UVW),tiledStManUVW);

    MeasurementSet ms(newTab);
    {
	TableDesc td = MSSysCal::requiredTableDesc();
	MSSysCal::addColumnToDesc (td, MSSysCal::TCAL);
	MSSysCal::addColumnToDesc (td, MSSysCal::TRX);
	MSSysCal::addColumnToDesc (td, MSSysCal::TSYS);
	MSSysCal::addColumnToDesc (td, MSSysCal::TCAL_FLAG);
	MSSysCal::addColumnToDesc (td, MSSysCal::TRX_FLAG);
	MSSysCal::addColumnToDesc (td, MSSysCal::TSYS_FLAG);
	td.addColumn (ScalarColumnDesc<Double> ("NFRA_TPON",
						"Total power ON"));
	td.addColumn (ScalarColumnDesc<Double> ("NFRA_TPOFF",
						"Total power OFF"));
	SetupNewTable syscalSetup(ms.sysCalTableName(), td, Table::New);
	ms.rwKeywordSet().defineTable(MS::keywordName(MS::SYSCAL),
				      Table(syscalSetup));
    }
    {
	TableDesc td = MSWeather::requiredTableDesc();
	MSWeather::addColumnToDesc (td, MSWeather::H2O);
	MSWeather::addColumnToDesc (td, MSWeather::IONOS_ELECTRON);
	MSWeather::addColumnToDesc (td, MSWeather::PRESSURE);
	MSWeather::addColumnToDesc (td, MSWeather::REL_HUMIDITY);
	MSWeather::addColumnToDesc (td, MSWeather::TEMPERATURE);
	MSWeather::addColumnToDesc (td, MSWeather::WIND_DIRECTION);
	MSWeather::addColumnToDesc (td, MSWeather::WIND_SPEED);
        SetupNewTable weatherSetup(ms.weatherTableName(), td, Table::New);
	ms.rwKeywordSet().defineTable(MS::keywordName(MS::WEATHER),
				      Table(weatherSetup));
    }
    // Do this after the creation of optional subtables,
    // so the MS will know about those optional sutables.
    ms.createDefaultSubtables(Table::New);
    
    TableDesc tmsTD;
    tmsTD.addColumn(ScalarColumnDesc<String>("NAME"));
    tmsTD.addColumn(ScalarColumnDesc<String>("VALUE"));
    SetupNewTable tmsSNT(ms.tableName()+"/NFRA_TMS_PARAMETERS",
			 tmsTD,Table::New);
    ms.rwKeywordSet().defineTable("NFRA_TMS_PARAMETERS",Table(tmsSNT,0));


    //
    // Add measure reference keywords to column descriptors.
    //
    MSColumns msc(ms);
    msc.timeMeas().setDescRefCode (MEpoch::UTC);
    msc.timeCentroidMeas().setDescRefCode (MEpoch::UTC);
    msc.uvwMeas().setDescRefCode (MDirection::B1950);
    msc.antenna().positionMeas().setDescRefCode (MPosition::ITRF);
    msc.feed().timeMeas().setDescRefCode (MEpoch::UTC);
    msc.field().timeMeas().setDescRefCode (MEpoch::UTC);
    msc.field().delayDirMeasCol().setDescRefCode (MDirection::B1950);
    msc.field().phaseDirMeasCol().setDescRefCode (MDirection::B1950);
    msc.field().referenceDirMeasCol().setDescRefCode (MDirection::B1950);
    msc.flagCmd().timeMeas().setDescRefCode (MEpoch::UTC);
    msc.history().timeMeas().setDescRefCode (MEpoch::UTC);
    msc.observation().timeRangeMeas().setDescRefCode (MEpoch::UTC);
    msc.observation().releaseDateMeas().setDescRefCode (MEpoch::UTC);
    msc.pointing().timeMeas().setDescRefCode (MEpoch::UTC);
    msc.pointing().timeOriginMeas().setDescRefCode (MEpoch::UTC);
    msc.pointing().directionMeasCol().setDescRefCode (MDirection::B1950);
    msc.pointing().targetMeasCol().setDescRefCode (MDirection::B1950);
    msc.sysCal().timeMeas().setDescRefCode (MEpoch::UTC);
    msc.weather().timeMeas().setDescRefCode (MEpoch::UTC);
    return ms;
}

TableDesc MSDefiner::msTableDesc() const
{
    TableDesc td = MS::requiredTableDesc();

    // add the weight_spectrum column which will soon be required
    MS::addColumnToDesc(td,MS::WEIGHT_SPECTRUM,1);
    Int numCorr = itspDS->numCorr();
    Int numChan = itspDS->numChan();
    cout << "numCorr, numChan = " << numCorr << ", " << numChan << endl;
    
    MS::addColumnToDesc(td, MS::DATA, 2);
    //and its unit
    td.rwColumnDesc(MS::columnName(MS::DATA)).rwKeywordSet().
                                                      define("UNIT","Jy");
    
    // columns for a TiledColumnStMan must be fixed shape, so
    td.removeColumn(MS::columnName(MS::FLAG));
    IPosition flagShape(2,numCorr,numChan);
    MS::addColumnToDesc(td, MS::FLAG, 2);
    
    td.removeColumn(MS::columnName(MS::SIGMA));
    IPosition sigmaShape(1,numCorr);
    MS::addColumnToDesc(td, MS::SIGMA, sigmaShape, ColumnDesc::Direct);
    
    cout << className() << ": define tiled hypercube for the data." << endl;
    td.defineHypercolumn("TiledData",4,
			 stringToVector(MS::columnName(MS::DATA)+","+
					MS::columnName(MS::FLAG)),
			 Vector<String>(),
			 stringToVector(MS::columnName(MS::DATA_DESC_ID)));
    cout << className() << ": define tiled hypercube for the UVW." << endl;
    td.defineHypercolumn("TiledUVW",2,
			 stringToVector(MS::columnName(MS::UVW)));
    
    return td;
}
