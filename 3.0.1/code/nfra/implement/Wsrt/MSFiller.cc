//# MSFfiller.cc : class for filling a MeasurementSet from a DataSource
//# Copyright (C) 1996,1997,1999,2000,2001,2002
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
//# $Id$
 

#include <nfra/Wsrt/MSFiller.h>
#include <ms/MeasurementSets.h>
#include <casa/Arrays.h>
#include <casa/Arrays/Slicer.h>
#include <casa/Arrays/Slice.h>
#include <scimath/Mathematics.h>
#include <measures/Measures/Stokes.h>
#include <tables/Tables/TiledDataStManAccessor.h>
#include <casa/Containers/Record.h>
#include <casa/OS/Timer.h>
#include <casa/iostream.h>

MSFiller::MSFiller (DataSource* pds,MeasurementSet* pms)
: itspDS  (pds),
  itspMS  (pms),
  itsBand (0)
{}

MSFiller::~MSFiller ()
{}

void MSFiller::fill(MeasurementSet& ms,
		    Int nChanPerTile,
		    Int nBasePerTile)
{
    cout << " " << endl;
    cout << " Main table " << endl;
    cout << " ---------- " << endl;
    cout << className() << "::fill(MS&): filling " << ms.tableName() << endl;

    // Instantiate the column objects we need.
    // This gets us all the required columns
    MSColumns msc(ms); 

    // == Create and fill the first MeasurementSet row
    uInt row;
    row = 0;
    ms.addRow();
    cout << "Row nr = " << row << endl;

    // Get the values that are constant for the whole dataset (for now,
    //most of them are hardcoded values, but at some time they may be
    //read from the dataset's observation header) and put them in the
    //first row.
    // (WSRT single OH block)

    Int msArrayId;
    msArrayId = 0;
    msc.arrayId().put(row,msArrayId);
    cout << "arrayId            : " << msc.arrayId()(row) << endl;

    Int msProcessorId;
    //not used
    msProcessorId = 0;
    msc.processorId().put(row,msProcessorId);
    cout << "processorId       : " << msc.processorId()(row) << endl;

    Int msStateId;
    //not used
    msStateId = 0;
    msc.stateId().put(row,msStateId);
    cout << "stateId       : " << msc.stateId()(row) << endl;

    Vector<Int> msFeedId(2);
    msFeedId = 0;
    msc.feed1().put(row,msFeedId(0));
    msc.feed2().put(row,msFeedId(1));
    cout << "feed1, feed2       : " << msc.feed1()(row)
	<< ", " << msc.feed2()(row) << endl;

    Int msFieldId;
    msFieldId = 0;
    msc.fieldId().put(row,msFieldId);
    cout << "fieldId            : " << msc.fieldId()(row) << endl;
    
    Bool msFlagRow;
    msFlagRow = False;
    msc.flagRow().put(row,msFlagRow);
    cout << "flagRow           : " << msc.flagRow()(row) << endl;
    
    Int msObservationId;
    msObservationId = 0;
    msc.observationId().put(row,msObservationId);
    cout << "observationId     : " << msc.observationId()(row) << endl;
    
    Int msScanNumber; 
    msScanNumber = 0; 
    msc.scanNumber().put(row,msScanNumber);
    cout << "scanNumber        : " << msc.scanNumber()(row) << endl;

    Vector<Float> msSigma(itspDS->numCorr());
    msSigma = 0.0;
    msc.sigma().put(row,msSigma);
    cout << "sigma             : " << msc.sigma()(row) << endl;

    Vector<Float> msWeight(itspDS->numCorr());
    msWeight = 1.0;
    msc.weight().put(row,msWeight);
    cout << "weight            : " << msc.weight()(row) << endl;

    //WSRT tape format (NOT TIME ORDERED)
    
    // == Loop through the dataset's data blocks
    // (WSRT IH/DB blocks) 
    
    // The MeasurementSet consists of nTime "time blocks", each with
    // its own time. Each time block consists of nRowsPerTime =
    // nBase*nBand rows, one for each baseline/sub-band
    // combination. Each row has a data field which contains a
    // 2-dimensional array of complex numbers with shape nCorr (nr of
    // correlations) by nChan (nr of frequency channels).

    // For the moment we only allow a single band (nBand=1). The
    // number of that band is kept in MSFiller::itsBand (0-based). For
    // DCB the band is selected by the (1-based) argument "band" in
    // the main program wsrtfiller.cc (default=4). For DLB and DXB
    // there is only one band, so itsBand=0 is enforced, ignoring a
    // possible selection via the "band" argument.

    // The WSRT data are organised per IH block, in which all time
    // samples are assembled per baseline (interferometer), per
    // frequency band/channel and per polarization. So, each WSRT data
    // block maps onto the (iCorr,iChan)-th elements of the MS data
    // array for all rows with the same iRow (or iBand and iBase).

    Int nTime = itspDS->nextDataBlock();
    Int nTimeFirst = nTime;
    Int nBase = itspDS->numBase();
//    Int nBand = itspDS->numBand();
    Int nBand = 1;
    Int nCorr = itspDS->numCorr();
    Int nChan = itspDS->numChan();
    Int nAnt  = itspDS->numAnt();
    Int nRowsPerTime = nBase*nBand;
    
//    Vector<Float> weightSpectrum(nChan);
//    weightSpectrum = 1.0;
    
    //indices for time, baseline, correlation, channel and band
    Int iTime, iRow, iCorr=0, iChan=0, iBand=0;
    Int iCorrPrevious=0, iChanPrevious=0, iBandPrevious=0;
    
    Stokes::StokesTypes corrType; 
    
    Vector<Int>         msAntennaId(2);
    Vector<Complex>     msData(nTimeFirst);
    Vector<Bool>        msFlag(nTimeFirst);
    Double              msSampleTime;
    Vector<Double>      msTime(3);
    Vector<Double>      msUVW(3);
    
    //data/flag caching/putting
    Cube<Complex>       msDataCache(IPosition(3,1,1,nTimeFirst*nRowsPerTime));
    Cube<Bool>          msFlagCache(IPosition(3,1,1,nTimeFirst*nRowsPerTime));
    Matrix<Bool>        flag(nCorr,nChan);
    
    //baseline and band to row assignment
    Cube<Int>         rowId(nAnt,nAnt,nBand);
    rowId = -1;
    uInt nextRow = 0;
    
    Bool nextBlock   = (nTime>0);
    Bool firstBlock  = nextBlock;
    Bool newSet      = False;
    Bool filledCache = False;
    Bool selected    = False;

    //
    // Get access to the TiledData storage manager.
    //
    TiledDataStManAccessor accessor(ms,"TiledData");

    //
    // First block (WSRT IH block) only: initialize MS
    //
    if (firstBlock) {

	//
	// Add all the rows to the MeasurementSet we need.
	//
	ms.addRow(nTimeFirst*nRowsPerTime-1);
	
	//
        // Calculate the best matching number of channels per data tile.
	//
	if (nChanPerTile > 0) {
	    Int nTilesInChan = (nChan+nChanPerTile-1)/nChanPerTile;
	    nChanPerTile = (nChan+nTilesInChan-1)/nTilesInChan;
	} else {
	    nChanPerTile = nChan;
	}

	//
        // Calculate the best matching number of baselines per data tile.
	//
	if (nBasePerTile > 0) {
	    Int nTilesInBase = (nBase+nBasePerTile-1)/nBasePerTile;
	    nBasePerTile = (nBase+nTilesInBase-1)/nTilesInBase;
	} else {
	    nBasePerTile = nBase;
	}

	//
        // Calculate the best matching number of times per data tile.
	//
	Int nTimePerTile = 16348/nChanPerTile/nBasePerTile/nCorr;
	Int nTilesInTime = (nTimeFirst+nTimePerTile-1)/nTimePerTile;
	nTimePerTile = (nTimeFirst+nTilesInTime-1)/nTilesInTime;

	//
	// Create the Record with the id-value.
	//
	Record values; 
//	values.define("DATA_DESC_ID",Int(itspDS->bandNr()));
	values.define("DATA_DESC_ID",Int(0));

	//
	// Create a hypercube in the TiledData storage manager.
	//
	accessor.addHypercube(IPosition(4,nCorr,nChan,nBase,nTimeFirst),
			      IPosition(4,nCorr,nChanPerTile,
					nBasePerTile,nTimePerTile),
			      values);

	cout << "shape data " << msc.data().shape(0) << endl;
	// Preset all flags to True.
	// Individual flags will be switched off when the
	// corresponding data are actually filled in
	flag = True;
	msc.flag().fillColumn(flag);
	
	// initialize data and flag caches
	msDataCache = Complex(0.,0.);
	msFlagCache = True;
    }
    
    while (nextBlock || filledCache) {

	// For all blocks: Get the indices for this block's data entries.  
	// iBand and iBase determine the row number within each time block.
	// iChan and iCorr determine the element position in the data array.

	// A WSRT SH block contains all data for a specific
	// combination of frequency band, frequency channel and
	// polarization. The last two are coordinates in the MS data
	// and flag matrices, the first one is (together with the
	// baseline) a "row coordinate".

	// For now (only a single band with nr MSFiller::itsBand) we
	// let only the matching band pass.

	if (nextBlock) {
	    iBand = itspDS->bandNr();
	    if (iBand == itsBand) {
		selected = True;
//	        cout << className() << "::fill(MS&): iBand,itsBand,selected = "
//		     << iBand << ", " << itsBand << ", " << selected << endl;
	    }else{
		selected = False;
	    }
	    if (selected) {
//		cout << className() << "::fill(MS&):  OK, selected" << endl;
		iChan = itspDS->chanNr();

		corrType = itspDS->corrType();
		// get the correlation types
		Vector<Int> cType(nCorr);
		cType = itspDS->corrTypes();
		iCorr = 0;
		for (Int i=0; i<nCorr; i++) {
		    if (corrType == cType(i)) iCorr = i;
		}	
    
		// First block: initialize indices
		if (firstBlock) {
		    cout << className() << "::fill(MS&):  OK, firstBlock" << endl;
		    firstBlock = False;
		    iBandPrevious = iBand;
		    iChanPrevious = iChan;
		    iCorrPrevious = iCorr;
		}
		// New set (new iCorr, iChan or iBand) ?
		// Note that newSet = False for the first block,
		// there is nothing to flush yet!
		newSet = (iCorr!=iCorrPrevious
				|| iChan!=iChanPrevious
				|| iBand!=iBandPrevious);
//		cout << className() << "::fill(MS&): iBand, iChan, iCorr = "
//		     << iBand << ", " << iChan << ", " << iCorr << endl;
//		cout << className() << "::fill(MS&): previously            "
//		     << iBandPrevious << ", " << iChanPrevious
//		     << ", " << iCorrPrevious << endl;
		if (newSet)
		    cout << className() << "::fill(MS&):  OK, newSet" << endl;
	    }
	}

	// New set or no more blocks: flush cache
	if ( (nextBlock && newSet && selected)
	     || (!nextBlock && filledCache) ) {
	    // slicer for filling in the proper position in the data array
	    Slicer slicer(IPosition(2,iCorrPrevious,iChanPrevious));
	    cout << className() << "::fill(MS&): slicer coordinates: "
		 << "IPosition(2," << iCorrPrevious
		 << "," << iChanPrevious << ")" << endl;
	    // Fill columns slice-wise
	    cout << className() << "::fill(MS&): Fill columns slice-wise" << endl;
	    cout << "                            put msDataCache";
	    msc.data().putColumn(slicer,msDataCache);
	    cout << ", msFlagCache";
	    msc.flag().putColumn(slicer,msFlagCache);
	    cout << ", done" << endl;
	    filledCache = False;
	    
	    // Reset
	    if (nextBlock) {
		msDataCache = Complex(0.,0.);
		msFlagCache = True;
		iBandPrevious = iBand;
		iChanPrevious = iChan;
		iCorrPrevious = iCorr;
	    }
	}
	
	if (nextBlock && selected) {
	    // Do the row-specific stuff only once per baseline/band
	    // pair, i.e., only when a new baseline/band combination
	    // appears; and then for all times. Currently every time
	    // block has the same nr of rows (nRowsPerTime).

	    // Get antenna (and later also feed) id's
	    msAntennaId = itspDS->msAntennaId();
	    // Block with new baseline/band combination:
	    // initialize corresponding rows
	    cout << className() << "::fill(MS&): ant1,ant2,iBand = " 
		 << msAntennaId(0) << ", " << msAntennaId(1)
		 << ", " << iBand << endl;
//	    if (rowId(msAntennaId(0),msAntennaId(1),iBand) < 0) {
	    if (rowId(msAntennaId(0),msAntennaId(1),0) < 0) {
		//assign next free row
		iRow = nextRow++;
		cout << className() << "::fill(MS&): next row assigned: "
		     << iRow << endl;		
		//remember assignment
//		rowId(msAntennaId(0),msAntennaId(1),iBand) = iRow;
		rowId(msAntennaId(0),msAntennaId(1),0) = iRow;
		
		//put values in for each time
		msTime = itspDS->msTime();
		for (iTime=0; iTime<nTime; iTime++) {
		    //current row number
		    row = iTime*nRowsPerTime + iRow;
		    
		    if (iTime==0) {
			msSampleTime = msTime(2) - (msTime(1)*nTime)/2;
		    }else{
			msSampleTime += msTime(1);
		    }

		    msc.time().put(row,msSampleTime);
		    msc.antenna1().put(row,msAntennaId(0));
		    msc.antenna2().put(row,msAntennaId(1)); 
		    msc.exposure().put(row,msTime(0));
		    msc.interval().put(row,msTime(1));

		    //get and put UVW for the current sample
                    msUVW = itspDS->msUVW(iTime);
		    msc.uvw().put(row,msUVW);

// This went wrong in filling OLNON.000001 for the third row (row=80)
//		    msc.weightSpectrum().put(row,weightSpectrum);
		}
		
		// Block with "old" baseline
	    }else{
//		iRow = rowId(msAntennaId(0),msAntennaId(1),iBand);
		iRow = rowId(msAntennaId(0),msAntennaId(1),0);
		cout << className() << "::fill(MS&): old rownr used: "
		     << iRow << endl;		
	    }

	    cout << className() << "::fill(MS&): iBand, iChan, iCorr, iRow: "
		 << iBand << ", " << iChan << ", "
		 << iCorr << ", " << iRow << endl;
	    
	    // Every selected block: add in the data and flag fields
	    msData = itspDS->msData(msFlag);
	    cout << className() << "::fill(MS&): got msData" << endl;

	    // Put values into local cache
	    for (iTime=0; iTime<nTime; iTime++) {
		//current row number
		row = iTime*nRowsPerTime + iRow;
		//put data value and flag for the current sample
		msDataCache(0,0,row) = msData(iTime);
		msFlagCache(0,0,row) = msFlag(iTime);
	    }
	    filledCache = True;
	    cout << className() << "::fill(MS&): filled cache" << endl;
	}//end if (selected nextBlock)

	// Every block: Move to next block (WSRT IH block)
	if (nextBlock) {
	    nTime = itspDS->nextDataBlock();
	    nextBlock = (nTime>0);
	}//#end if (nextBlock)
    }//#end while (nextBlock || filledCache)

    accessor.showCacheStatistics (cout);

    cout << className() << "::fill(MS&): ready " << endl;
}

void MSFiller::fill(MSAntenna& msant)
{
    cout << " " << endl;
    cout << " Antenna subtable " << endl;
    cout << " ---------------- " << endl;
    cout << className() << ": filling " << msant.tableName() << endl;

    // Get access to the columns of the subtable
    MSAntennaColumns antc(msant);

    // Get the necessary values
    //
    //     from xxxDataSource:
    //
    // Number of antennas
    Int nAnt = itspDS->numAnt();
    // Antenna positions
    Matrix<Double> antPosition(nAnt,3);
    antPosition = itspDS->antPosition();
    Vector<Double> antXYZ(3);
    // Antenna names
    Vector<String> antName(nAnt);
    antName = itspDS->antName();
    //
    //     hardcoded here:
    //
    // Axes offsets
    Vector<Double> offsets(3);
    offsets = 0.0;

    // Fill the columns
    for (Int i=0; i<nAnt; i++) {
	msant.addRow();

	antc.dishDiameter().put(i,25.0f); //# fixed for wsrt
	antc.mount().put(i,"equatorial"); //# fixed for wsrt
	antc.name().put(i,antName(i));    //# from DataSource
	antc.offset().put(i,offsets);     //# hardcoded
	antXYZ = antPosition.row(i);      //# from DataSource
	antc.position().put(i,antXYZ);
	antc.station().put(i,"WESTERBORK");
	antc.type().put(i,"GROUND-BASED");
	antc.flagRow().put(i,False);      //# row is valid
    }

    // Show the result
    cout << " dish diameter : " << antc.dishDiameter()(0) << " m" << endl;
    cout << " mount type : " << antc.mount()(0) << endl;
    cout << " axes offset of mount: " << antc.offset()(0) << " m" << endl;
    cout << " station : " << antc.station()(0) << endl;
    for (Int j=0; j<nAnt; j++) {
	cout << " Antenna " << antc.name()(j)
	     << " : id = " << j
	     << ", position = " << antc.position()(j) << endl;
    }
}

void MSFiller::fill(MSDataDescription& msdd)
{
    cout << " " << endl;
    cout << " Data description subtable " << endl;
    cout << " -------------- " << endl;
    cout << className() << ": filling " << msdd.tableName() << endl;

    // Get access to the columns of the subtable
    MSDataDescColumns ddc(msdd);

    // Fill the columns
//    Int nBand = itspDS->numBand();
    Int nBand = 1;
    for (Int i=0; i<nBand; i++) {
        msdd.addRow();
	ddc.spectralWindowId().put(i, i);
	ddc.polarizationId().put(i, 0);
	ddc.flagRow().put(i, False);
    }
}

void MSFiller::fill(MSFeed& msfeed) 
{
    cout << " " << endl;
    cout << " Feed subtable " << endl;
    cout << " ------------- " << endl;
    cout << className() << ": filling " << msfeed.tableName() << endl;

    // Get access to the columns of the subtable
    MSFeedColumns feedc(msfeed);

    // Get the necessary values
    //
    //     from xxxDataSource:
    //
    Int nAnt = itspDS->numAnt();
    Vector<Double> time(2);
    time = itspDS->timeInterval();
    //
    //     hardcoded here:
    //
    Int nRec = 2;
    Matrix<Double> offset(2,nRec);
    offset = 0.0;
    Matrix<Complex> response(nRec,nRec);
    response = Complex(0.0,0.0);
    for (Int rec=0; rec<nRec; rec++) {
	response(rec,rec) = Complex(1.0,0.0);
    }
    Vector<String> rec_type(nRec);
    rec_type(0) = "X";
    rec_type(1) = "Y";
    Vector<Double> feedPos(3);
    feedPos = 0.0;
    Vector<Double> feedAngle(nRec);
    feedAngle = -C::pi_4;    //0 for parallel dipoles; see nsclod

    // Fill the columns
    for (Int i=0; i<nAnt; i++) {
	msfeed.addRow();

	feedc.antennaId().put(i,i);       //# id = rownr (0-relative)
	feedc.feedId().put(i,0);          //# only one
	feedc.spectralWindowId().put(i,0);//# only one
	feedc.time().put(i,time(0));      //# from DataSource
	feedc.interval().put(i,time(1));  //# from DataSource

	feedc.numReceptors().put(i,nRec); //#hardcoded

	feedc.beamId().put(i,-1);         //# not used
	feedc.beamOffset().put(i,offset); //# hardcoded
	feedc.polarizationType().put(i,rec_type); //# hardcoded
	feedc.polResponse().put(i,response);      //# hardcoded
	feedc.position().put(i,feedPos);          //# hardcoded
	feedc.receptorAngle().put(i,feedAngle);   //# hardcoded
    }

    // Show the result for the first row
    cout << " --SHOW FIRST ROW ONLY:-- " << endl;
    cout << " antenna id : " << feedc.antennaId()(0) << endl;
    cout << " beam id : " << feedc.beamId()(0) << endl;
    cout << " beam offset : " << feedc.beamOffset()(0) << " rad" << endl;
    cout << " feed id : " << feedc.feedId()(0) << endl; 
    cout << " time interval : " << feedc.interval()(0) << " s" << endl;
    cout << " nr of receptors : " << feedc.numReceptors()(0) << endl;
    cout << " polarization response : " << feedc.polResponse()(0) << endl;
    cout << " polarization type : " << feedc.polarizationType()(0) << endl;
    cout << " relative position : " << feedc.position()(0) << " m" << endl;
    cout << " receptor angle : " << feedc.receptorAngle()(0) << " rad" << endl;
    cout << " spectral window id : " << feedc.spectralWindowId()(0) << endl;
    cout << " time (midpoint) : " << feedc.time()(0) << " s" << endl;
}

void MSFiller::fill(MSField& msfield)
{
    cout << " " << endl;
    cout << " Field subtable " << endl;
    cout << " -------------- " << endl;
    cout << className() << ": filling " << msfield.tableName() << endl;

    // Get access to the columns of the subtable
    MSFieldColumns fieldc(msfield);

    // Get the necessary values
    //
    //     from xxxDataSource:
    //
    String fieldName = itspDS->fieldName();
    String fieldCode = itspDS->fieldCode();
    //
    // Use 2 polynomial terms if a rate is not zero.
    Int numPoly = 2;
    Vector<Double> fieldDelayDirRate = itspDS->fieldDelayDirRate();
    Vector<Double> fieldPhaseDirRate = itspDS->fieldPhaseDirRate();
    Vector<Double> fieldReferenceDirRate = itspDS->fieldReferenceDirRate();
    if (allEQ(fieldDelayDirRate, 0.)  &&  allEQ(fieldPhaseDirRate, 0.)
    &&  allEQ(fieldReferenceDirRate, 0.)) {
        numPoly = 1;
    }
    Matrix<Double> fieldDelayDir(2,numPoly);
    Matrix<Double> fieldPhaseDir(2,numPoly);
    Matrix<Double> fieldReferenceDir(2,numPoly);
    fieldDelayDir.column(0) = itspDS->fieldDelayDir();
    fieldPhaseDir.column(0) = itspDS->fieldPhaseDir();
    fieldReferenceDir.column(0) = itspDS->fieldReferenceDir();
    if (numPoly == 2) {
        fieldDelayDir.column(1) = fieldDelayDirRate;
	fieldPhaseDir.column(1) = fieldPhaseDirRate;
	fieldReferenceDir.column(1) = fieldReferenceDirRate;
    }
    //
    Vector<Double> time = itspDS->timeInterval();
    //
    //     hardcoded here:
    //

    // Fill the columns
    msfield.addRow();

    fieldc.name().put(0,fieldName);                //# from DataSource
    fieldc.code().put(0,fieldCode);                //# from DataSource
    fieldc.time().put(0,time(0));                  //# from DataSource
    fieldc.numPoly().put(0,numPoly);               //# #terms in directions
    fieldc.delayDir().put(0,fieldDelayDir);        //# from DataSource
    fieldc.phaseDir().put(0,fieldPhaseDir);        //# from DataSource
    fieldc.referenceDir().put(0,fieldReferenceDir);     //# from DataSource
    fieldc.sourceId().put(0,-1);                   //# SOURCE table absent
    fieldc.flagRow().put(0,False);                 //# row is valid

    // Show the result
    cout << " special characteristics : " << fieldc.code()(0) << endl;
    cout << " delay direction : " << fieldc.delayDir()(0) << endl;
    cout << " field name : " << fieldc.name()(0) << endl;
    cout << " phase direction : " << fieldc.phaseDir()(0) << endl;
    cout << " reference direction : " << fieldc.referenceDir()(0) << endl;
    cout << " source id  : " << fieldc.sourceId()(0) << endl;
    cout << " time (midpoint) : " << fieldc.time()(0) << " s" << endl;
}

void MSFiller::fill(MSObservation& msobs)
{
    cout << " " << endl;
    cout << " Observation subtable " << endl;
    cout << " -------------------- " << endl;
    cout << className() << ": filling " << msobs.tableName() << endl;

    // Get access to the columns of the subtable
    MSObservationColumns obsc(msobs);

    // Get the necessary values
    //
    //     from xxxDataSource:
    //
    String project = itspDS->project();
    //
    //     hardcoded here:
    //
    Vector<String> corrSchedule(1);
    corrSchedule = "corrSchedule";
    String observer = itspDS->observer();
    // Convert time/interval to start/end time.
    Vector<Double> times = itspDS->timeInterval();
    Vector<Double> timeRange(2);
    timeRange(0) = times(0) - times(1)/2;
    timeRange(1) = times(0) + times(1)/2;
    // Data is public one year after end of observation.
    Double releaseDate = timeRange(1) + 365.25*24*60*60;

    // Fill the columns
    msobs.addRow();

    obsc.telescopeName().put(0,"WSRT");       //# hardcoded
    obsc.timeRange().put(0,timeRange);        //# from DataSource
    obsc.observer().put(0,observer);          //# hardcoded
    obsc.scheduleType().put(0,"WSRT");        //# hardcoded
    obsc.schedule().put(0,corrSchedule);      //# hardcoded
    obsc.project().put(0,project);            //# from DataSource
    obsc.releaseDate().put(0,releaseDate);    //# from DataSource
    obsc.flagRow().put(0,False);              //# row is valid

    // Show the result
    cout << " Correlator script  : " << obsc.scheduleType()(0) << endl;
    cout << " Name of observer(s): " << obsc.observer()(0) << endl;
    cout << " Observing schedule : " << obsc.schedule()(0) << endl;
    cout << " Project identification string : " << obsc.project()(0) << endl;
}

void MSFiller::fill(MSPointing& mspointing)
{
    cout << " " << endl;
    cout << " Pointing subtable " << endl;
    cout << " --------------- " << endl;
    cout << className() << ": filling " << mspointing.tableName() << endl;

    // Get access to the columns of the subtable
    MSPointingColumns pointingc(mspointing);

    // Number of antennas
    Int nAnt = itspDS->numAnt();
    // Get the necessary values
    //
    //     from xxxDataSource:
    // Use 2 polynomial terms if a rate is not zero.
    Int numPoly = 2;
    Vector<Double> fieldPointDirRate = itspDS->fieldPointDirRate();
    if (allEQ(fieldPointDirRate, 0.)) {
        numPoly = 1;
    }
    Matrix<Double> fieldPointDir(2,numPoly);
    fieldPointDir.column(0) = itspDS->fieldPointDir();
    if (numPoly == 2) {
        fieldPointDir.column(1) = fieldPointDirRate;
    }
    // Get centre time and interval.
    Vector<Double> times = itspDS->timeInterval();
    Double timeOrigin = times(0) - times(1)/2;
    //
    //     hardcoded here:
    //

    // Fill the columns
    mspointing.addRow(nAnt);
    for (Int i=0; i<nAnt; i++) {
        pointingc.antennaId().put(i,i);
	pointingc.time().put(i,times(0));            //# from DataSource
	pointingc.interval().put(i,times(1));        //# from DataSource
        pointingc.name().put(i,"");
        pointingc.numPoly().put(i,numPoly);          //# #terms in directions
	pointingc.timeOrigin().put(i,timeOrigin);    //# dummy
	pointingc.direction().put(i,fieldPointDir);  //# from DataSource
	pointingc.target().put(i,fieldPointDir);     //# from DataSource
	pointingc.tracking().put(i, True);           //# on position
    }

    // Show the result
    cout << " time               : " << pointingc.time()(0) << endl;
    cout << " pointing direction : " << pointingc.direction()(0) << endl;
}

void MSFiller::fill(MSPolarization& mspol)
{
    cout << " " << endl;
    cout << " Polarization subtable " << endl;
    cout << " --------------- " << endl;
    cout << className() << ": filling " << mspol.tableName() << endl;

    // Get access to the columns of the subtable
    MSPolarizationColumns polc(mspol);

    // Get the necessary values
    //
    //     from xxxDataSource:
    // Use 2 polynomial terms if a rate is not zero.
    Int numCorr = itspDS->numCorr();
    Vector<Int> corrType(numCorr);
    corrType = itspDS->corrTypes();
    Matrix<Int> corrProduct(2,numCorr);
    for (Int i=0; i<numCorr; i++) {
	corrProduct(0,i) = Stokes::receptor1(Stokes::type(corrType(i)));
	corrProduct(1,i) = Stokes::receptor2(Stokes::type(corrType(i)));
    }
      
    //
    //     hardcoded here:
    //

    // Fill the columns
    mspol.addRow();
    polc.numCorr().put(0,numCorr);
    polc.corrType().put(0,corrType);
    polc.corrProduct().put(0,corrProduct);
    polc.flagRow().put(0,False);

    // Show the result
    cout << " nr of correlations:  " << polc.numCorr()(0) << endl;
    cout << " correlation type:    " << polc.corrType()(0) << endl;
    cout << " correlation product: " << polc.corrProduct()(0) << endl;
}

void MSFiller::fill(MSProcessor& msproc)
{
    cout << " " << endl;
    cout << " Processor subtable " << endl;
    cout << " -------------- " << endl;
    cout << className() << ": filling " << msproc.tableName() << endl;

    // Get access to the columns of the subtable
    MSProcessorColumns procc(msproc);

    // Fill the columns
    msproc.addRow();
    procc.type().put(0, "CORRELATOR");
    procc.subType().put(0, "");
    procc.typeId().put(0, -1);
    procc.modeId().put(0, -1);
    procc.flagRow().put(0, False);
}

void MSFiller::fill(MSSpectralWindow& msspwin)
{
    cout << " " << endl;
    cout << " SpectralWindow subtable " << endl;
    cout << " ----------------------- " << endl;
    cout << className() << ": filling " << msspwin.tableName() << endl;

    // Get access to the columns of the subtable
    MSSpWindowColumns spwinc(msspwin);

    // Get the necessary values
    //
    //     from xxxDataSource:
    //

    // Its is assumed that either nChan=1 (DCB) or nBand=1 (DLB and
    // DXB), so that we can always use the product as vector size and
    // know how to separate bands from channels.

    Int nBand = itspDS->numBand();
    Int nChan = itspDS->numChan();
    Vector<Double> tmpVD(1);

    //
    Vector<Double> chanFreq(nBand*nChan);
    chanFreq = itspDS->chanFreq();
    //
    Double refFreq    = itspDS->refFrequency();
    Vector<Double> resolution(nBand*nChan);
    resolution        = itspDS->freqResolution();
    Vector<Double> bandwidth(nBand);
    bandwidth         = itspDS->freqBandwidth(nBand);
    Vector<Double> chanWidth(nChan);
    //
    //     hardcoded here:
    //
    
    // Fill the columns
    // For the moment enforce only a single spectral window
//    msspwin.addRow(nBand);
    msspwin.addRow(1);
    
//    for (Int iRow=0; iRow<nBand; iRow++) {
    Int iRow = 0;
	spwinc.numChan().put(iRow,nChan);            //# from DataSource
	spwinc.name().put(iRow,"");
	//spwinc.molecule().put(iRow,"optional");
	spwinc.refFrequency().put(iRow,refFreq);     //# from DataSource

	if (nBand == 1) {
	    spwinc.chanFreq().put(iRow,chanFreq);    //# from DataSource
	    spwinc.resolution().put(iRow,resolution);//# from DataSource
	    chanWidth = bandwidth(0) / nChan;
	    spwinc.chanWidth().put(iRow,chanWidth);
	    spwinc.effectiveBW().put(iRow,chanWidth);
	}else{
//	    tmpVD = chanFreq(iRow);
	    tmpVD = chanFreq(itsBand);
	    spwinc.chanFreq().put(iRow,tmpVD);
//	    tmpVD = resolution(iRow);
	    tmpVD = resolution(itsBand);
	    spwinc.resolution().put(iRow,tmpVD);
//	    tmpVD = bandwidth(iRow);
	    tmpVD = bandwidth(itsBand);
	    spwinc.chanWidth().put(iRow,tmpVD);
	    spwinc.effectiveBW().put(iRow,tmpVD);
	}
	spwinc.measFreqRef().put (iRow,MFrequency::TOPO);
//	spwinc.totalBandwidth().put(iRow,bandwidth(iRow));
        spwinc.totalBandwidth().put(iRow,bandwidth(itsBand));
	spwinc.netSideband().put(iRow,0);
	spwinc.freqGroup().put(iRow,-1);
	spwinc.freqGroupName().put(iRow,"");
	spwinc.ifConvChain().put(iRow,0);
	spwinc.flagRow().put(iRow,False);
	
	// Show the result
//	cout << " Spectral window nr " << iRow << ": " << endl;
	cout << " Spectral window nr 0: " << endl;
	cout << " channel frequencies = " << spwinc.chanFreq()(iRow) << endl;
	cout << " IF conversion chain = " << spwinc.ifConvChain()(iRow) << endl;
	cout << " nr of channels      = " << spwinc.numChan()(iRow) << endl;
	cout << " reference frequency = " << spwinc.refFrequency()(iRow) << " Hz" << endl;
	cout << " resolution          = " << spwinc.resolution()(iRow) << " Hz" << endl;
	cout << " total bandwidth     = " << spwinc.totalBandwidth()(iRow) << " Hz" << endl;
//    }//#end for

/* ====== Start baseline-0 patch ========
    // Add continuum band-spectral window

    Double contBandwidth = itspDS->contFreqBandwidth();
    Double contChanFreq  = itspDS->contChanFreq();

    msspwin.addRow();
    iRow = nBand;
    spwinc.corrType().put(iRow,cType);
    spwinc.corrProduct().put(iRow,corrproduct);
    spwinc.ifConvChain().put(iRow,0);
    spwinc.numChan().put(iRow,1);
    spwinc.numCorr().put(iRow,nCorr);
    tmpVD = contChanFreq;
    spwinc.chanFreq().put(iRow,tmpVD);    //# 3.47292e+08
    spwinc.refFrequency().put(iRow,refFreq);
    tmpVD = contBandwidth;
    spwinc.resolution().put(iRow,tmpVD);   //# 2.5e+06
    spwinc.totalBandwidth().put(iRow,contBandwidth);

    // Show the result
    cout << " Spectral window nr " << iRow << ": " << endl;
    cout << " channel frequencies = " << spwinc.chanFreq()(iRow) << endl;
    cout << " correlation product = " << spwinc.corrProduct()(iRow) << endl;
    cout << " correlation type    = " << spwinc.corrType()(iRow) << endl;
    cout << " IF conversion chain = " << spwinc.ifConvChain()(iRow) << endl;
    cout << " nr of channels      = " << spwinc.numChan()(iRow) << endl;
    cout << " nr of correlations  = " << spwinc.numCorr()(iRow) << endl;
    cout << " reference frequency = " << spwinc.refFrequency()(iRow) << " Hz" << endl;
    cout << " resolution          = " << spwinc.resolution()(iRow) << " Hz" << endl;
    cout << " total bandwidth     = " << spwinc.totalBandwidth()(iRow) << " Hz" << endl;
 ====== End baseline-0 patch ======== */
}

void MSFiller::fill(MSState& msstate)
{
    cout << " " << endl;
    cout << " State subtable " << endl;
    cout << " -------------- " << endl;
    cout << className() << ": filling " << msstate.tableName() << endl;

    // Get access to the columns of the subtable
    MSStateColumns statec(msstate);

    // Fill the columns
    msstate.addRow();
    statec.sig().put(0, True);
    statec.ref().put(0, False);
    statec.cal().put(0, 0.);
    statec.load().put(0, 0.);
    statec.subScan().put(0, 0);
    statec.obsMode().put(0, "");
    statec.flagRow().put(0, False);
}

void MSFiller::fill(MSSysCal& mssyscal)
{
    cout << " " << endl;
    cout << " SysCal subtable " << endl;
    cout << " -------------- " << endl;
    cout << className() << ": filling " << mssyscal.tableName() << endl;

    // Get access to the columns of the subtable
    MSSysCalColumns syscalc(mssyscal);

    // Get the necessary values
    Int nAnt = itspDS->numAnt();
    Int nRec = 2;
    Vector<Double> time(2);
    time = itspDS->timeInterval();
    Matrix<Float> tSys(nAnt,nRec);
    tSys = itspDS->sysTemp();
    Vector<Float> temp(nRec);

    // Fill the columns
    for (Int i=0; i<nAnt; i++) {
	mssyscal.addRow();

	syscalc.antennaId().put(i,i);
	syscalc.feedId().put(i,0);
	syscalc.spectralWindowId().put(i,0);
	syscalc.time().put(i,time(0));
	syscalc.interval().put(i,time(1));

	for (Int j=0; j<nRec; j++) {
	    temp(j) = tSys(i,j);     //<- SC::TNOISI (K), offset 792 
	}
	syscalc.tcal().put(i,temp);
	syscalc.trx().put(i,temp);
	syscalc.tsys().put(i,temp);

	syscalc.tcalFlag().put(i,True);
	syscalc.trxFlag().put(i,True);
	syscalc.tsysFlag().put(i,False);
    }

    // Show the result
    cout << " --SHOW FIRST ROW ONLY:-- " << endl;
    cout << " antenna id        : " << syscalc.antennaId()(0) << endl;
    cout << " feed id           : " << syscalc.feedId()(0) << endl;
    cout << " time interval     : " << syscalc.interval()(0) << " s" << endl;
    cout << " spectral window id: " << syscalc.spectralWindowId()(0) << endl;
    cout << " receptor temp     : " << syscalc.tcal()(0) << " K" << endl;
    cout << " receptor temp flag: " << syscalc.tcalFlag()(0) << endl;
    cout << " time (midpoint)   : " << syscalc.time()(0) << " s" << endl;
    cout << " receiver temp     : " << syscalc.trx()(0) << " K" << endl;
    cout << " rec temp flag     : " << syscalc.trxFlag()(0) << endl;
    cout << " system temp       : " << syscalc.tsys()(0) << " K" << endl;
    cout << " system temp flag  : " << syscalc.tsysFlag()(0) << " " << endl;

}

void MSFiller::fill(MSWeather& msweather)
{
    cout << " " << endl;
    cout << " Weather subtable " << endl;
    cout << " ---------------- " << endl;
    cout << className() << ": filling " << msweather.tableName() << endl;

    // Get access to the columns of the subtable
    MSWeatherColumns weathc(msweather);

    // Get the necessary values
    Int nAnt = itspDS->numAnt();
    Vector<Double> time(2);
    time = itspDS->timeInterval();
    Float cdElectrons = 0;
    Float cdWater = 0;               //itspDS->cdWater();     //SC+2560_float
    Float airTemp = 0;               //itspDS->airTemp();     //SC+2540_short
    Float airPress = 0;              //itspDS->airPressure(); //SC+2542_short
    Float humidity = 0;              //itspDS->humidity();    //SC+2544_short
    Float windDir = 0;
    Float windSpeed = 0;

    // Fill the columns
    for (Int i=0; i<nAnt; i++) {
	msweather.addRow();

	weathc.antennaId().put(i,i);
	weathc.interval().put(i,time(1));
	weathc.time().put(i,time(0));

	weathc.H2O().put(i,cdWater);
	weathc.ionosElectron().put(i,cdElectrons);
	weathc.pressure().put(i,airPress);
	weathc.relHumidity().put(i,humidity);
	weathc.temperature().put(i,airTemp);
	weathc.windDirection().put(i,windDir);
	weathc.windSpeed().put(i,windSpeed);
    }

    // Show the result
    cout << " --SHOW FIRST ROW ONLY:-- " << endl;
    cout << " antenna id        : " << weathc.antennaId()(0) << endl;
    cout << " water column dens : " << weathc.H2O()(0) << " m" << endl;
    cout << " time interval     : " << weathc.interval()(0) << " s" << endl;
    cout << " electron col dens : " << weathc.ionosElectron()(0) << " m^-2" << endl;
    cout << " presssure         : " << weathc.pressure()(0) << " Pa" << endl;
    cout << " relative humidity : " << weathc.relHumidity()(0) << endl;
    cout << " air temperature   : " << weathc.temperature()(0) << " K" << endl;
    cout << " time (midpoint)   : " << weathc.time()(0) << " s" << endl;
    cout << " wind direction    : " << weathc.windDirection()(0) << " rad" << endl;
    cout << " wind speed        : " << weathc.windSpeed()(0) << " m/s" << endl;
}

void MSFiller::run(Int bandnr,
		   Int nChanPerTile,
		   Int nBasePerTile)
{
    if (itspDS->numChan() > 1) {
	// DLB or DXB dataset
	itsBand = 0;
    }else{
	// DCB dataset
	itsBand = bandnr;
    }
    cout << className() << "::run(): Band nr "
	 << itsBand << " selected" << endl; 

    Bool shortcut(False);
    cout << className() << ": Try if DataSource can fill MS straight-away " << endl;
    shortcut = itspDS->fill(*itspMS);
    if (!shortcut) {
	cout << className() << ": No? OK, fill per subtable " << endl;

	itspDS->show();

	// fill the subtables
	fill(itspMS->antenna());
	fill(itspMS->dataDescription());
	fill(itspMS->feed());
	fill(itspMS->field());
	fill(itspMS->observation());
	fill(itspMS->pointing());
	fill(itspMS->polarization());
	fill(itspMS->processor());
	fill(itspMS->spectralWindow());
	fill(itspMS->sysCal());
	fill(itspMS->state());
	fill(itspMS->weather());
	// fill the main table
	fill(*itspMS, nChanPerTile, nBasePerTile);
	cout << className() << ": main table filled" << endl;
    }
}
