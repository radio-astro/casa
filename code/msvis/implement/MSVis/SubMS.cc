//# SubMS.cc 
//# Copyright (C) 1996-2007
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
//# $Id: $
#include <msvis/MSVis/SubMS.h>
#include <ms/MeasurementSets/MSSelection.h>
#include <tables/Tables/ExprNode.h>
#include <tables/Tables/RefRows.h>
#include <ms/MeasurementSets/MSColumns.h>
#include <coordinates/Coordinates/CoordinateUtil.h>
#include <casa/Arrays/Matrix.h>
#include <casa/Arrays/Cube.h>
#include <casa/Arrays/ArrayMath.h>
#include <casa/Arrays/ArrayOpsDiffShapes.h>
#include <casa/Arrays/ArrayLogical.h>
#include <casa/Arrays/ArrayUtil.h>
#include <casa/Arrays/IPosition.h>
#include <casa/Arrays/Slice.h>
#include <casa/Logging/LogIO.h>
#include <casa/OS/File.h>
#include <casa/OS/HostInfo.h>
#include <casa/Containers/Record.h>
#include <casa/BasicSL/String.h>
#include <casa/Utilities/Assert.h>
#include <casa/Utilities/GenSort.h>
#include <casa/System/AppInfo.h>
#include <msvis/MSVis/VisSet.h>
#include <msvis/MSVis/VisBuffer.h>
#include <msvis/MSVis/VisibilityIterator.h>
#include <tables/Tables/IncrementalStMan.h>
#include <tables/Tables/ScalarColumn.h>
#include <tables/Tables/ScaColDesc.h>
#include <tables/Tables/SetupNewTab.h>
#include <tables/Tables/StandardStMan.h>
#include <tables/Tables/Table.h>
#include <tables/Tables/TableDesc.h>
#include <tables/Tables/TableInfo.h>
#include <tables/Tables/TableLock.h>
#include <tables/Tables/TableRecord.h>
#include <tables/Tables/TableCopy.h>
#include <tables/Tables/TableRow.h>
#include <tables/Tables/TiledColumnStMan.h>
#include <tables/Tables/TiledShapeStMan.h>
#include <tables/Tables/TiledDataStMan.h>
#include <tables/Tables/TiledStManAccessor.h>
#include <ms/MeasurementSets/MSTileLayout.h>
#include <scimath/Mathematics/InterpolateArray1D.h>
#include <casa/sstream.h>
#include <casa/iomanip.h>
#include <functional>
#include <set>


namespace casa {
  
  SubMS::SubMS(String& theMS) :
    ms_p(MeasurementSet(theMS, Table::Update)),
    mssel_p(ms_p),
    msc_p(NULL),
    mscIn_p(NULL),
    chanModification_p(False),
    antennaSel_p(False),
    sameShape_p(True),
    timeBin_p(-1.0),
    numOutRows_p(0),
    scanString_p(""),
    uvrangeString_p(""),
    taqlString_p(""),
    timeRange_p(""),
    arrayExpr_p(""),
    nant_p(0)
  {
  }
  
  SubMS::SubMS(MeasurementSet& ms) :
    ms_p(ms),
    mssel_p(ms_p),
    msc_p(NULL),
    mscIn_p(NULL),
    chanModification_p(False),
    antennaSel_p(False),
    sameShape_p(True),
    timeBin_p(-1.0),
    numOutRows_p(0),
    scanString_p(""),
    uvrangeString_p(""),
    taqlString_p(""),
    timeRange_p(""),
    arrayExpr_p(""),
    nant_p(0)
  {
  }
  
  SubMS::~SubMS()
  {
    if(!msOut_p.isNull())
      msOut_p.flush();

    delete msc_p;
    msc_p = NULL;
    
    delete mscIn_p;
    mscIn_p = NULL;

    msOut_p=MeasurementSet();

    // parseColumnNames unavoidably has a static String and Vector<String>.
    // Collapse them down to free most of that memory.
    parseColumnNames("None");
  }
  
  // This is the version used by split.
  Bool SubMS::selectSpw(const String& spwstr, const Vector<Int>& steps,
                        const Bool averchan)
  {
    LogIO os(LogOrigin("SubMS", "selectSpw()"));

    MSSelection mssel;
    mssel.setSpwExpr(spwstr);

    Vector<Int> widths = steps.copy();
    if(widths.nelements() < 1){
      widths.resize(1);
      widths[0] = 1;
    }
    if(widths[0] == 0){
      os << LogIO::WARN
         << "0 cannot be used for channel width...using 1 instead."
         << LogIO::POST;
      widths[0] = 1;
    }

    // Each row should have spw, start, stop, step
    Matrix<Int> chansel = mssel.getChanList(&ms_p, widths[0]);

    if(chansel.nrow() > 0) {         // Use spwstr if it selected anything...
      spw_p       = chansel.column(0);
      chanStart_p = chansel.column(1);
      nchan_p     = chansel.column(2);
      chanStep_p  = chansel.column(3);

      // SubMS uses a different meaning for nchan_p from MSSelection.  For
      // SubMS it is the # of output channels for each output spw.  For
      // MSSelection it is end input chan - start input chan + 1 for each
      // output spw.
      for(uInt k = 0; k < nchan_p.nelements(); ++k){
        nchan_p[k] = (nchan_p[k] - chanStart_p[k] + 1) / chanStep_p[k];
        if(nchan_p[k] < 1)
          nchan_p[k] = 1;
      }
    }
    else{                            // select everything and rely on widths.
      ROMSSpWindowColumns mySpwTab(ms_p.spectralWindow());
      uInt nspw = mySpwTab.nrow();
      
      spw_p.resize(nspw);
      indgen(spw_p);
      
      chanStart_p.resize(nspw);
      for(uInt k = 0; k < nspw; ++k)
        chanStart_p[k] = 0;
      
      nchan_p.resize(nspw);
      for(uInt k = 0; k < nspw; ++k)
        nchan_p[k] = mySpwTab.numChan()(spw_p[k]);
      
      if(widths.nelements() != spw_p.nelements()){
        if(widths.nelements() == 1){
          widths.resize(spw_p.nelements(), True);
          for(uInt k = 1; k < spw_p.nelements(); ++k)
            widths[k] = widths[0];
	}
        else{
          os << LogIO::SEVERE
             << "Mismatch between the # of widths specified by width and the # of spws."
             << LogIO::POST;
          return false;
        }
      }
      chanStep_p = widths;
    }
    
    // Check for and filter out selected spws that aren't included in
    // DATA_DESCRIPTION.  (See CAS-1673 for an example.)
    ROScalarColumn<Int> spws_in_dd(ms_p.dataDescription(), 
	     MSDataDescription::columnName(MSDataDescription::SPECTRAL_WINDOW_ID));
    std::set<Int> uniqSpwsInDD;
    uInt nspwsInDD = spws_in_dd.nrow();
    for(uInt ddrow = 0; ddrow < nspwsInDD; ++ddrow)
      uniqSpwsInDD.insert(spws_in_dd(ddrow));
    std::set<Int>::iterator ddend = uniqSpwsInDD.end();
    std::set<Int> badSelSpwSlots;
    uInt nSelSpw = spw_p.nelements();
    for(uInt k = 0; k < nSelSpw; ++k){
      if(uniqSpwsInDD.find(spw_p[k]) == ddend){
        badSelSpwSlots.insert(k);
      }
    }
    uInt nbadSelSpwSlots = badSelSpwSlots.size();
    if(nbadSelSpwSlots > 0){
      os << LogIO::WARN << "Selected input spw(s)\n";
      for(std::set<Int>::iterator bbit = badSelSpwSlots.begin();
          bbit != badSelSpwSlots.end(); ++bbit)
        os << spw_p[*bbit] << " ";
      os << "\nnot found in DATA_DESCRIPTION and being excluded."
         << LogIO::POST;

      uInt ngoodSelSpwSlots = nSelSpw - nbadSelSpwSlots;
      Vector<Int> spwc(ngoodSelSpwSlots);
      Vector<Int> chanStartc(ngoodSelSpwSlots);
      Vector<Int> nchanc(ngoodSelSpwSlots);
      Vector<Int> chanStepc(ngoodSelSpwSlots);
      std::set<Int>::iterator bsend = badSelSpwSlots.end();
      
      uInt j = 0;
      for(uInt k = 0; k < nSelSpw; ++k){
        if(badSelSpwSlots.find(k) == bsend){
          spwc[j]       = spw_p[k];
          chanStartc[j] = chanStart_p[k];
          nchanc[j]     = nchan_p[k];
          chanStepc[j]  = chanStep_p[k];
          ++j;
        }
      }
      spw_p.resize(ngoodSelSpwSlots);
      spw_p = spwc;
      chanStart_p.resize(ngoodSelSpwSlots);
      chanStart_p = chanStartc;
      nchan_p.resize(ngoodSelSpwSlots);
      nchan_p = nchanc;
      chanStep_p.resize(ngoodSelSpwSlots);
      chanStep_p = chanStepc;
    }
    
    averageChannel_p = averchan;
    return true;
  }
  
  // This older version is used elsewhere.
  void SubMS::selectSpw(Vector<Int> spw, Vector<Int> nchan, Vector<Int> start, 
			Vector<Int> step, const Bool averchan)
  {
    spw_p.resize();
    spw_p = spw;
    
    //check for default
    if(spw_p.nelements() == 1 && spw_p[0] < 0){
      spw_p.resize(ms_p.spectralWindow().nrow());
      indgen(spw_p);

      //no may be we have to redo the chan selection
      
      if (nchan.nelements() != spw_p.nelements()){
	nchan.resize(spw_p.nelements(), True);
	for(uInt k = 1; k < spw_p.nelements(); ++k){
	  nchan[k] = nchan[0];
	}
      }
      if (start.nelements() != spw_p.nelements()){
	start.resize(spw_p.nelements(), True);
	for(uInt k = 1; k < spw_p.nelements(); ++k){
	  start[k] = start[0];
	}
      }
      if (step.nelements() != spw_p.nelements()){
	step.resize(spw_p.nelements(), True);
	for(uInt k = 1; k < spw_p.nelements(); ++k){
	  step[k] = step[0];
	}
      }
    }
        
    nchan_p.resize();
    nchan_p = nchan;
    chanStart_p.resize();
    chanStart_p = start;
    chanStep_p.resize();
    chanStep_p = step;
    averageChannel_p = averchan;
    // check for defaults
    if(nchan_p[0]<=0 || (nchan_p.nelements() != spw_p.nelements())){
      nchan_p.resize(spw_p.nelements());
      ROMSSpWindowColumns mySpwTab(ms_p.spectralWindow());
      for (uInt k =0; k < spw_p.nelements(); ++k){
	if(nchan[0]<=0)
	  nchan_p[k]=mySpwTab.numChan()(spw_p[k]);
	else
	  nchan_p[k]=nchan[0];
      }
      chanStart_p.resize(spw_p.nelements());
      chanStep_p.resize(spw_p.nelements());
      if(chanStart_p.nelements() == start.nelements()){
	chanStart_p=start;
      }
      else{
	chanStart_p.set(start[0]);
      }
      if(chanStep_p.nelements() == step.nelements()){
	chanStep_p=step;
      }
      else{
	chanStep_p.set(step[0]);
      } 
    }    
  }

  // This is the one used by split.
  Bool SubMS::setmsselect(const String& spw, const String& field,
                          const String& baseline, const String& scan,
                          const String& uvrange, const String& taql,
                          const Vector<Int>& step, const Bool averchan,
                          const String& subarray)
  {
    LogIO os(LogOrigin("SubMS", "setmsselect()"));
    Bool  ok;
    
    Record selrec = ms_p.msseltoindex(spw, field);

    ok = selectSource(selrec.asArrayInt("field"));

    // All of the requested selection functions will be tried, even if an
    // earlier one has indicated its failure.  This allows all of the selection
    // strings to be tested, yielding more complete feedback for the user
    // (fewer retries).  This is a matter of taste, though.  If the selections
    // turn out to be slow, this function should return on the first false.

    if(!selectSpw(spw, step, averchan)){
      os << LogIO::SEVERE << "No channels selected." << LogIO::POST;
      ok = false;
    }
    
    if(baseline != ""){
      Vector<Int> antid(0);
      Vector<String> antstr(1,baseline);
      selectAntenna(antid, antstr);
    }
    scanString_p    = scan;
    uvrangeString_p = uvrange;
    taqlString_p    = taql;

    if(subarray != "")
      selectArray(subarray);

    return ok;
  }

  // This is the older version, used elsewhere.
  void SubMS::setmsselect(const String& spw, const String& field, 
			  const String& baseline, 
			  const String& scan, const String& uvrange, 
			  const String& taql, const Vector<Int>& nchan, 
			  const Vector<Int>& start, const Vector<Int>& step,
			  const Bool averchan, const String& subarray)
  {
    Vector<Int> inchan(1, -1);
    Vector<Int> istart(1, 0);
    Vector<Int> istep(1, 1);
    Record      selrec = ms_p.msseltoindex(spw, field);
    Vector<Int> spwids = selrec.asArrayInt("spw");

    selectSource(selrec.asArrayInt("field"));
    if(spwids.nelements() < 1)
      spwids=Vector<Int>(1, -1);

    //use nchan if defined else use caret-column syntax of  msselection 
    if((nchan.nelements()>0) && nchan[0] > 0){
      inchan.resize(); inchan=nchan;
      istep.resize(); istep=step;
      istart.resize(); istart=start;
    }
    else{
      Matrix<Int> chansel=selrec.asArrayInt("channel");
      if(chansel.nelements() != 0){
	inchan.resize(chansel.nrow());
	istep.resize(chansel.nrow());
	istart.resize(chansel.nrow());
	// if the vector step is used ..for averaging ..let's use it
	Bool stepused=False;
	if( (step.nelements() >= 1) && (max(step) > 1))
	  stepused=True;
	for (uInt k =0 ; k < chansel.nrow(); ++k){
	  if(stepused){
	    if(step.nelements() == 1)
	      istep[k] = step[0];
	    else if(step.nelements() == istep.nelements())
	      istep[k] = step[k];
	    else //confused at this stage
	      istep[k] = 1;
	  }
	  else{
	    istep[k] = chansel.row(k)(3);
	    if(istep[k] < 1)
	      istep[k] = 1;
	  }
	  istart[k] = chansel.row(k)(1);
	  inchan[k] = (chansel.row(k)(2) - istart[k] + 1) / istep[k];
	  if(inchan[k] < 1)
	    inchan[k] = 1;
	}
      } 
    }
    selectSpw(spwids, inchan, istart, istep, averchan);
    
    if(baseline != ""){
      Vector<Int> antid(0);
      Vector<String> antstr(1,baseline);
      selectAntenna(antid, antstr);
    }
    scanString_p    = scan;
    uvrangeString_p = uvrange;
    taqlString_p    = taql;

    if(subarray != "")
      selectArray(subarray);
  }  

  
  Bool SubMS::selectSource(const Vector<Int>& fieldid)
  {
    LogIO os(LogOrigin("SubMS", "selectSource()"));
    Bool cando = true;

    if(fieldid.nelements() < 1){
      fieldid_p = Vector<Int>(1, -1);
    }
    else if(fieldid.nelements() > ms_p.field().nrow()){
      os << LogIO::SEVERE
         << "More fields were requested than are in the input MS.\n"
         << LogIO::POST;
      cando = false;
    }
    else if(max(fieldid) >= static_cast<Int>(ms_p.field().nrow())){
      // Arriving here is very unlikely since if fieldid came from MSSelection
      // bad fields were presumably already quietly dropped.
      os << LogIO::SEVERE
         << "At least 1 field was requested that is not in the input MS.\n"
         << LogIO::POST;      
      cando = false;
    }
    else{
      fieldid_p = fieldid;
    }

    if(fieldid_p.nelements() == 1 && fieldid_p[0] < 0){
      fieldid_p.resize(ms_p.field().nrow());
      indgen(fieldid_p);
    }
    return cando;
  }
  
  
  void SubMS::selectAntenna(Vector<Int>& antennaids, Vector<String>& antennaSel){
    if((antennaids.nelements()==1) && (antennaids[0]=-1) && antennaSel[0]==""){
      antennaSel_p=False;
      return;
    }
    
    antennaSel_p=True;
    if((antennaids.nelements()==1) && (antennaids[0]=-1))
      antennaId_p.resize();
    else
      antennaId_p=antennaids;
    antennaSelStr_p=antennaSel; 
  }
  
  void SubMS::selectArray(const String& subarray)
  {
    arrayExpr_p = subarray;
    if(arrayExpr_p == ""){      // Zap any old ones.
      arrayId_p.resize();
      arrayRemapper_p.resize();
    }
    // else arrayId_p will get set in makeSelection().

  }
  
  void SubMS::selectTime(Double timeBin, String timerng)
  {  
    timeBin_p   = timeBin;
    timeRange_p = timerng;
  }
  
  
  Bool SubMS::makeSubMS(String& msname, String& colname,
                        const Vector<Int>& tileShape)
  {
    LogIO os(LogOrigin("SubMS", "makeSubMS()"));
    try{
      if((spw_p.nelements()>0) && (max(spw_p) >= Int(ms_p.spectralWindow().nrow()))){
	os << LogIO::SEVERE 
	   << "SpectralWindow selection contains elements that do not exist in "
	   << "this MS"
	   << LogIO::POST;
	ms_p=MeasurementSet();
	return False;	
      }
      
      // Watch out!  This throws an AipsError if ms_p doesn't have the
      // requested columns.
      const Vector<String> colNamesTok=SubMS::parseColumnNames(colname);
      verifyColumns(ms_p, colNamesTok);

      if(!makeSelection()){
	os << LogIO::SEVERE 
	   << "Failed on selection: combination of spw and/or field and/or time chosen"
	   << " may be invalid." 
	   << LogIO::POST;
	ms_p=MeasurementSet();
	return False;
      }
      mscIn_p=new ROMSColumns(mssel_p);
      // Note again the verifyColumns() a few lines back that stops setupMS()
      // from being called if the MS doesn't have the requested columns.
      MeasurementSet* outpointer=0;

      if(tileShape.nelements() == 3){
	outpointer=setupMS(msname, nchan_p[0], npol_p[0],  
			   String(colname), tileShape);
      }
      else if((tileShape.nelements()==1) && (tileShape[0]==0 || tileShape[0]==1)){
	outpointer=setupMS(msname, nchan_p[0], npol_p[0],mscIn_p->observation().telescopeName()(0),  
			   String(colname), tileShape[0]);
      }
      else{
	//Sweep all other cases of bad tileshape to a default one.
	outpointer=setupMS(msname, nchan_p[0], npol_p[0],mscIn_p->observation().telescopeName()(0),  
		   String(colname), 0);

      }
      
      msOut_p= *outpointer;
      msc_p=new MSColumns(msOut_p);
      
      if(!fillAllTables(colname)){
	delete outpointer;
	os << LogIO::WARN << msname << " left unfinished." << LogIO::POST;
	ms_p=MeasurementSet();
	return False;
      }
      
      //  msOut_p.relinquishAutoLocks (True);
      //  msOut_p.unlock();
      //Detaching the selected part
      ms_p=MeasurementSet();
      
      //
      // If all columns are in the new MS, set the CHANNEL_SELECTION
      // keyword for the MODEL_DATA column.  This is apparently used
      // in at least imager to decide if MODEL_DATA and CORRECTED_DATA
      // columns should be initialized or not.
      //
      if (isAllColumns(colNamesTok))
	{
	  MSSpWindowColumns msSpW(msOut_p.spectralWindow());
	  Int nSpw=msOut_p.spectralWindow().nrow();
	  if(nSpw==0) nSpw=1;
	  Matrix<Int> selection(2,nSpw);
	  selection.row(0)=0; //start
	  selection.row(1)=msSpW.numChan().getColumn();
	  ArrayColumn<Complex> mcd(msOut_p,MS::columnName(MS::MODEL_DATA));
	  mcd.rwKeywordSet().define("CHANNEL_SELECTION",selection);
	}

      delete outpointer;
      return True;
    }
    catch(AipsError x){
      ms_p=MeasurementSet();
      throw(x);
    }
    catch(...){
      ms_p=MeasurementSet();
      throw(AipsError("Unknown exception caught"));
    }
  }
  
  
  MeasurementSet* SubMS::makeScratchSubMS(String& colname, Bool forceInMemory){
    
    LogIO os(LogOrigin("SubMS", "makeSubMS()"));
    
    if(max(fieldid_p) >= Int(ms_p.field().nrow())){
      os << LogIO::SEVERE 
	 << "Field selection contains elements that do not exist in "
	 << "this MS"
	 << LogIO::POST;
      ms_p=MeasurementSet();
      return 0;
      
      
    }
    if(max(spw_p) >= Int(ms_p.spectralWindow().nrow())){
      os << LogIO::SEVERE 
	 << "SpectralWindow selection contains elements that do not exist in "
	 << "this MS"
	 << LogIO::POST;
      ms_p=MeasurementSet();
      return 0;
      
      
    }
    
    if(!makeSelection()){
      os << LogIO::SEVERE 
	 << "Failed on selection: combination of spw and/or field and/or time "
	 << "chosen may be invalid."
	 << LogIO::POST;
      ms_p=MeasurementSet();
      return 0;
    }
    mscIn_p=new ROMSColumns(mssel_p);
    Double sizeInMB= 1.5 * n_bytes() / (1024.0 * 1024.0);
    String msname=AppInfo::workFileName(uInt(sizeInMB), "TempSubMS");
    
    MeasurementSet* outpointer=setupMS(msname, nchan_p[0], npol_p[0],  
				       mscIn_p->observation().telescopeName()(0),
				       "DATA");
    
    outpointer->markForDelete();
    //Hmmmmmm....memory...... 
    if(sizeInMB <  (Double)(HostInfo::memoryTotal())/(2048.0) 
       || forceInMemory){
      MeasurementSet* a = outpointer;
      outpointer= new MeasurementSet(a->copyToMemoryTable("TmpMemoryMS"));
      outpointer->initRefs();
      delete a;
    }
    
    msOut_p = *outpointer;
    msc_p = new MSColumns(msOut_p);
    
    if(!fillAllTables(colname)){
      delete outpointer;
      outpointer = 0;
      ms_p = MeasurementSet();
      return 0;
    }
    //Detaching the selected part
    ms_p=MeasurementSet();
    return outpointer;
  }
  
  
  Bool SubMS::fillAllTables(const String& colname)
  {
    //LogIO os(LogOrigin("SubMS", "makeSubMS()"));
    LogIO os(LogOrigin("SubMS", "fillAllTables()"));
    
    // Should take care of Measures frames for all the time type columns below.
    // It should be safe to avoid the empty table check (with false) since this
    // is explicitly a case of a column full of numbers that should be in the
    // right reference frame, but the column could have, or end up with, the
    // wrong reference code if nothing is done.  However, the table is still
    // empty if the reference codes are set up here.
    msc_p->setEpochRef(MEpoch::castType(mscIn_p->timeMeas().getMeasRef().getType()));

    // UVW is the only other Measures column in the main table.
    msc_p->uvwMeas().setDescRefCode(Muvw::castType(mscIn_p->uvwMeas().getMeasRef().getType()));

    // fill or update
    if(!fillDDTables()){
      return False;
    }

    // SourceIDs need to be remapped around here.  It could not be done in
    // selectSource() because mssel_p was not setup yet.
    relabelSources();

    fillFieldTable();
    copySource();

    copyAntenna();
    copyFeed();         // Feed table writing has to be after antenna 
    copyObservation();
    copyPointing();
    
    sameShape_p = checkSpwShape();
    const Vector<String> columnNames(parseColumnNames(colname));
    doImgWts_p = doWriteImagingWeight(*mscIn_p, columnNames);
    
    if(timeBin_p <= 0.0){
      fillMainTable(columnNames);
    }
    else{
      if(!sameShape_p){
	os << LogIO::WARN 
	   << "Time averaging of differing spw shapes is not handled yet."
	   << LogIO::POST;
	os << LogIO::WARN
	   << "Work around: average differently shaped spws separately and then concatenate." 
           << LogIO::POST;
	return False;
      }
      else{
	fillAverMainTable(columnNames);
      }
    }
    return True;
  }
  
  
  Bool SubMS::makeSelection(){
    
    LogIO os(LogOrigin("SubMS", "makeSelection()"));
    
    //VisSet/MSIter will check if the SORTED exists
    //and resort if necessary
    {
      Matrix<Int> noselection;
      VisSet vs(ms_p, noselection);
    }
    
   
    const MeasurementSet *elms;
    elms=&ms_p;
    MeasurementSet sorted;
    if (ms_p.keywordSet().isDefined("SORTED_TABLE")) {
      sorted=ms_p.keywordSet().asTable("SORTED_TABLE");
      //If ms is not writable and sort is a subselection...use original ms
      if( ms_p.nrow() == sorted.nrow())
	elms=&sorted;
    }
 
    MSSelection thisSelection;
    if(fieldid_p.nelements() > 0)
      thisSelection.setFieldExpr(MSSelection::indexExprStr(fieldid_p));
    if(spw_p.nelements() > 0)
      thisSelection.setSpwExpr(MSSelection::indexExprStr(spw_p));
    if(antennaSel_p){
      if(antennaId_p.nelements() > 0){
	thisSelection.setAntennaExpr(MSSelection::indexExprStr( antennaId_p ));
      }
      if(antennaSelStr_p[0] != "")
        thisSelection.setAntennaExpr(MSSelection::nameExprStr( antennaSelStr_p));
    }
    if(timeRange_p != "")
      thisSelection.setTimeExpr(timeRange_p);
    
    thisSelection.setUvDistExpr(uvrangeString_p);
    thisSelection.setScanExpr(scanString_p);
    if(arrayExpr_p != "")
      thisSelection.setArrayExpr(arrayExpr_p);
    thisSelection.setTaQLExpr(taqlString_p);
    
    TableExprNode exprNode=thisSelection.toTableExprNode(elms);
    
    {      
      const MSDataDescription ddtable = ms_p.dataDescription();
      ROScalarColumn<Int> polId(ddtable, 
				MSDataDescription::columnName(MSDataDescription::POLARIZATION_ID));
      const MSPolarization poltable = ms_p.polarization();
      ROArrayColumn<Int> pols(poltable, 
			      MSPolarization::columnName(MSPolarization::CORR_TYPE));
      
      ROScalarColumn<Int> spwId(ddtable, 
				MSDataDescription::columnName(MSDataDescription::SPECTRAL_WINDOW_ID));

      uInt nddids = polId.nrow();
      uInt nSpws = spw_p.nelements();

      Vector<uInt> npols_per_spw;
      Int highestSpw = max(spw_p);
      if(highestSpw < 0)
        highestSpw = 0;
      spw2ddid_p.resize(highestSpw + 1);
      npols_per_spw.resize(highestSpw + 1);
      spw2ddid_p.set(0);                 // This is a row #, so must be >= 0.
      npols_per_spw.set(0);
      for(uInt j = 0; j < nddids; ++j){
        Int spw = spwId(j);
        for(uInt k = 0; k < nSpws; ++k){
          if(spw == spw_p[k]){
            ++npols_per_spw[spw_p[k]];
            spw2ddid_p[spw_p[k]] = j;
          }
        }
      }

      Bool ddidprob = false;
      for(uInt k = 0; k < nSpws; ++k){
        if(npols_per_spw[spw_p[k]] != 1){
          ddidprob = true;
          os << LogIO::SEVERE
             << "Selected input spw " << spw_p[k] << " matches "
             << npols_per_spw[spw_p[k]] << " POLARIZATION_IDs." << LogIO::POST;
        }
      }
      if(ddidprob){
          os << LogIO::SEVERE
             << "split currently requires one POLARIZATION_ID per selected "
             << "\nSPECTRAL_WINDOW_ID in the DATA_DESCRIPTION table."
             << LogIO::POST;
          return false;
      }

      Vector<Int> ddids;
      ddids.resize(nSpws);

      npol_p.resize(nSpws); 
      for(uInt k = 0; k < nSpws; ++k)
	npol_p[k] = pols(polId(spw2ddid_p[spw_p[k]])).nelements();
    }
    
    // Now remake the selected ms
    if(!(exprNode.isNull())){
      mssel_p = MeasurementSet((*elms)(exprNode));
    }
    else{
      // Null take all the ms ...setdata() blank means that
      mssel_p = MeasurementSet((*elms));
    }
    //mssel_p.rename(ms_p.tableName()+"/SELECTED_TABLE", Table::Scratch);
    if(mssel_p.nrow() == 0)
      return False;

    if(mssel_p.nrow() < ms_p.nrow()){
      os << LogIO::NORMAL
	 << mssel_p.nrow() << " out of " << ms_p.nrow() << " rows are going to be" 
	 << " considered due to the selection criteria." 
	 << LogIO::POST;
    }
    return True;
    
  }
  

  MeasurementSet* SubMS::setupMS(String MSFileName, Int nchan, Int nCorr, 
				 String telescop, String colName, Int obstype){

    //Choose an appropriate tileshape
    IPosition dataShape(2,nCorr,nchan);
    IPosition tileShape = MSTileLayout::tileShape(dataShape,obstype, telescop);
    return setupMS(MSFileName, nchan, nCorr, colName, tileShape.asVector());
  }
  MeasurementSet* SubMS::setupMS(String MSFileName, Int nchan, Int nCorr, 
				 String colName, const Vector<Int>& tshape){
    
    
    
    if(tshape.nelements() != 3)
      throw(AipsError("TileShape has to have 3 elememts ") );
    const Vector<String> colNamesTok=SubMS::parseColumnNames(colName);
    // Make the MS table
    TableDesc td = MS::requiredTableDesc();
    
    // Even though we know the data is going to be the same shape throughout I'll
    // still create a column that has a variable shape as this will permit MS's
    // with other shapes to be appended.
    uInt ncols = colNamesTok.nelements();
    if (ncols < 2)
      {
	MS::addColumnToDesc(td, MS::DATA, 2);
	String hcolName=String("Tiled")+String("DATA");
	td.defineHypercolumn(hcolName,3,
			     stringToVector("DATA"));
      }
    else
      for(uInt i=0; i < ncols;i++)
	{
	  if (colNamesTok[i]==MS::columnName(MS::DATA))
	    MS::addColumnToDesc(td, MS::DATA, 2);
	
	  if (colNamesTok[i]==MS::columnName(MS::MODEL_DATA))
	    MS::addColumnToDesc(td, MS::MODEL_DATA, 2);
	  
	  if (colNamesTok[i]==MS::columnName(MS::CORRECTED_DATA))
	    MS::addColumnToDesc(td, MS::CORRECTED_DATA, 2);
	  String hcolName=String("Tiled")+String(colNamesTok[i]);
	  td.defineHypercolumn(hcolName,3,
			       stringToVector(colNamesTok[i]));
	}
    if (isAllColumns(colNamesTok))
      {
	MS::addColumnToDesc(td, MS::IMAGING_WEIGHT, 1);
	td.defineHypercolumn("TiledImagingWeight", 2, 
			     stringToVector(MS::columnName(MS::IMAGING_WEIGHT)));
      }
    
    
    // add this optional column because random group fits has a
    // weight per visibility
    MS::addColumnToDesc(td, MS::WEIGHT_SPECTRUM, 2);
    
    //     td.defineHypercolumn("TiledDATA",3,
    //  			 stringToVector(MS::columnName(MS::DATA)));
    td.defineHypercolumn("TiledFlag",3,
 			 stringToVector(MS::columnName(MS::FLAG)));
    td.defineHypercolumn("TiledFlagCategory",4,
 			 stringToVector(MS::columnName(MS::FLAG_CATEGORY)));
    td.defineHypercolumn("TiledWgtSpectrum",3,
 			 stringToVector(MS::columnName(MS::WEIGHT_SPECTRUM)));
    td.defineHypercolumn("TiledUVW",2,
 			 stringToVector(MS::columnName(MS::UVW)));
    td.defineHypercolumn("TiledWgt",2,
			 stringToVector(MS::columnName(MS::WEIGHT)));
    td.defineHypercolumn("TiledSigma", 2,
			 stringToVector(MS::columnName(MS::SIGMA)));
    
    SetupNewTable newtab(MSFileName, td, Table::New);
    
    // Set the default Storage Manager to be the Incr one
    IncrementalStMan incrStMan ("ISMData");
    newtab.bindAll(incrStMan, True);
    
    // Bind ANTENNA1, ANTENNA2 and DATA_DESC_ID to the standardStMan 
    // as they may change sufficiently frequently to make the
    // incremental storage manager inefficient for these columns.
    
    StandardStMan aipsStMan(32768);
    newtab.bindColumn(MS::columnName(MS::ANTENNA1), aipsStMan);
    newtab.bindColumn(MS::columnName(MS::ANTENNA2), aipsStMan);
    newtab.bindColumn(MS::columnName(MS::DATA_DESC_ID), aipsStMan);
    
    // Choose an appropriate tileshape
    //IPosition dataShape(2,nCorr,nchan);
    //IPosition tileShape = MSTileLayout::tileShape(dataShape,obsType, telescop);
    //////////////////
    
    IPosition tileShape(tshape);


      ///////////////////
    //    itsLog << LogOrigin("MSFitsInput", "setupMeasurementSet");
    //itsLog << LogIO::NORMAL << "Using tile shape "<<tileShape <<" for "<<
    //  array_p<<" with obstype="<< obsType<<LogIO::POST;
    
    //    TiledShapeStMan tiledStMan1("TiledData",tileShape);
    TiledShapeStMan tiledStMan1Data("TiledDATA",tileShape);
    TiledShapeStMan tiledStMan1Corrected("TiledCORRECTED_DATA",tileShape);
    TiledShapeStMan tiledStMan1Model("TiledMODEL_DATA",tileShape);
    TiledShapeStMan tiledStMan1f("TiledFlag",tileShape);
    TiledShapeStMan tiledStMan1fc("TiledFlagCategory",
				  IPosition(4,tileShape(0),tileShape(1),1,
					    tileShape(2)));
    TiledShapeStMan tiledStMan2("TiledWgtSpectrum",tileShape);
    TiledColumnStMan tiledStMan3("TiledUVW",IPosition(2,3,1024));
    TiledShapeStMan tiledStMan4("TiledWgt", 
				IPosition(2,tileShape(0),tileShape(2)));
    TiledShapeStMan tiledStMan5("TiledSigma", 
				IPosition(2,tileShape(0),tileShape(2)));
    
    // Bind the DATA, FLAG & WEIGHT_SPECTRUM columns to the tiled stman
    
    if (colNamesTok.nelements() == 1)
      {
	newtab.bindColumn(MS::columnName(MS::DATA),tiledStMan1Data);
      }
    else
    for(uInt i=0;i<colNamesTok.nelements();i++)
      {
	if (colNamesTok[i] == MS::columnName(MS::DATA))
	  newtab.bindColumn(MS::columnName(MS::DATA),
			    tiledStMan1Data);
	else if (colNamesTok[i]==MS::columnName(MS::MODEL_DATA))
	  newtab.bindColumn(MS::columnName(MS::MODEL_DATA),
			    tiledStMan1Model);
	else 
	  newtab.bindColumn(MS::columnName(MS::CORRECTED_DATA),
			    tiledStMan1Corrected);
      }
    
    newtab.bindColumn(MS::columnName(MS::FLAG),tiledStMan1f);
    newtab.bindColumn(MS::columnName(MS::FLAG_CATEGORY),tiledStMan1fc);
    newtab.bindColumn(MS::columnName(MS::WEIGHT_SPECTRUM),tiledStMan2);
    newtab.bindColumn(MS::columnName(MS::UVW),tiledStMan3);
    newtab.bindColumn(MS::columnName(MS::WEIGHT),tiledStMan4);
    newtab.bindColumn(MS::columnName(MS::SIGMA),tiledStMan5);

    if (isAllColumns(colNamesTok))
      {
	TiledShapeStMan tiledStManImgWts("TiledImagingWeight",
					 IPosition(1,tileShape(1)));
	newtab.bindColumn(MS::columnName(MS::IMAGING_WEIGHT),tiledStManImgWts);
      }
    
    // avoid lock overheads by locking the table permanently
    TableLock lock(TableLock::AutoLocking);
    MeasurementSet *ms = new MeasurementSet (newtab,lock);
    
    // Set up the subtables for the UVFITS MS
    // we make new tables with 0 rows
    Table::TableOption option=Table::New;
    ms->createDefaultSubtables(option); 
    // add the optional Source sub table to allow for 
    // specification of the rest frequency
    TableDesc sourceTD=MSSource::requiredTableDesc();
    SetupNewTable sourceSetup(ms->sourceTableName(),sourceTD,option);
    ms->rwKeywordSet().defineTable(MS::keywordName(MS::SOURCE),
				   Table(sourceSetup,0));
    // update the references to the subtable keywords
    ms->initRefs();
    
    { // Set the TableInfo
      TableInfo& info(ms->tableInfo());
      info.setType(TableInfo::type(TableInfo::MEASUREMENTSET));
      info.setSubType(String("UVFITS"));
      info.readmeAddLine
	("This is a measurement set Table holding astronomical observations");
    }
    
    
    return ms;
  }
  
  
  Bool SubMS::fillDDTables(){
    
    LogIO os(LogOrigin("SubMS", "fillDDTables()"));
    
    MSSpWindowColumns& msSpW(msc_p->spectralWindow());
    MSDataDescColumns& msDD(msc_p->dataDescription());
    MSPolarizationColumns& msPol(msc_p->polarization());
    
    //DD table
    const MSDataDescription ddtable= mssel_p.dataDescription();
    ROScalarColumn<Int> polId(ddtable, 
			      MSDataDescription::columnName(MSDataDescription::POLARIZATION_ID));
    
    //Fill in matching spw to datadesc in old ms 
    {
      ROMSDataDescColumns msOldDD(ddtable);
      oldDDSpwMatch_p=msOldDD.spectralWindowId().getColumn();
    }
    //POLARIZATION table 
    
    
    const MSPolarization poltable= mssel_p.polarization();
    ROScalarColumn<Int> numCorr (poltable, 
				 MSPolarization::columnName(MSPolarization::NUM_CORR));
    ROArrayColumn<Int> corrType(poltable, 
				MSPolarization::columnName(MSPolarization::CORR_TYPE));
    ROArrayColumn<Int> corrProd(poltable, MSPolarization::columnName(MSPolarization::CORR_PRODUCT));
    ROScalarColumn<Bool> polFlagRow(poltable, MSPolarization::columnName(MSPolarization::FLAG_ROW));
    
    //SPECTRAL_WINDOW table
    const MSSpectralWindow spwtable=mssel_p.spectralWindow();
    spwRelabel_p.resize(mscIn_p->spectralWindow().nrow());
    spwRelabel_p.set(-1);
    
    ROArrayColumn<Double> chanFreq(spwtable, MSSpectralWindow::columnName(MSSpectralWindow::CHAN_FREQ));
    ROArrayColumn<Double> chanWidth(spwtable, MSSpectralWindow::columnName(MSSpectralWindow::CHAN_WIDTH));
    ROArrayColumn<Double> effBW(spwtable, MSSpectralWindow::columnName(MSSpectralWindow::EFFECTIVE_BW));
    ROScalarColumn<Bool> spwFlagRow(spwtable, MSSpectralWindow::columnName(MSSpectralWindow::FLAG_ROW));
    ROScalarColumn<Int> freqGroup(spwtable, MSSpectralWindow::columnName(MSSpectralWindow::FREQ_GROUP));
    ROScalarColumn<String> freqGroupName(spwtable, MSSpectralWindow::columnName(MSSpectralWindow::FREQ_GROUP_NAME));
    ROScalarColumn<Int> ifConvChain(spwtable, MSSpectralWindow::columnName(MSSpectralWindow::IF_CONV_CHAIN));
    ROScalarColumn<Int> measFreqRef(spwtable, MSSpectralWindow::columnName(MSSpectralWindow::MEAS_FREQ_REF));
    ROScalarColumn<String> spwName(spwtable, MSSpectralWindow::columnName(MSSpectralWindow::NAME));
    ROScalarColumn<Int> netSideband(spwtable, MSSpectralWindow::columnName(MSSpectralWindow::NET_SIDEBAND)); 
    ROScalarColumn<Int> numChan(spwtable, MSSpectralWindow::columnName(MSSpectralWindow::NUM_CHAN));
    ROScalarColumn<Double> refFreq(spwtable, MSSpectralWindow::columnName(MSSpectralWindow::REF_FREQUENCY));
    ROArrayColumn<Double> spwResol(spwtable, MSSpectralWindow::columnName(MSSpectralWindow::RESOLUTION));
    ROScalarColumn<Double> totBW(spwtable, MSSpectralWindow::columnName(MSSpectralWindow::TOTAL_BANDWIDTH));
    inNumChan_p.resize(spw_p.nelements()); 
    
    Vector<Int> ddPolId=polId.getColumn();
    Bool dum;
    Sort sort( ddPolId.getStorage(dum),sizeof(Int) );
    sort.sortKey((uInt)0,TpInt);
    Vector<uInt> index,uniq;
    sort.sort(index,ddPolId.nelements());
    uInt nPol = sort.unique(uniq,index);
    Vector<Int> selectedPolId(nPol);
    for(uInt k = 0; k < nPol; ++k)
      selectedPolId[k]=ddPolId[index[uniq[k]]];
    
    // Make map from input to output spws.
    Sort sortSpws(spw_p.getStorage(dum), sizeof(Int));
    sortSpws.sortKey((uInt)0, TpInt);
    Vector<uInt> spwsortindex, spwuniqinds;
    sortSpws.sort(spwsortindex, spw_p.nelements());
    uInt nuniqSpws = sortSpws.unique(spwuniqinds, spwsortindex);
    spw_uniq_p.resize(nuniqSpws);
    for(uInt k = 0; k < nuniqSpws; ++k){
      spw_uniq_p[k] = spw_p[spwuniqinds[k]];
      spwRelabel_p[spw_uniq_p[k]] = k;
    }
    if(nuniqSpws < spw_p.nelements()){
      os << LogIO::WARN
         << "Multiple channel ranges within an spw may not work.  SOME DATA MAY BE OMITTED!"
         << "\nConsider splitting them individually and optionally combining the output MSes with concat."
         << "\nEven then, expect problems if exporting to uvfits."
         << LogIO::POST;
    }

    Vector<Int> newPolId(spw_uniq_p.nelements());
    for(uInt k = 0; k < nuniqSpws; ++k){
      for (uInt j=0; j < nPol; ++j){ 
	if(selectedPolId[j]==ddPolId[k])
	  newPolId[k]=j;
      }
      msOut_p.polarization().addRow();
      msPol.numCorr().put(k, numCorr(polId(spw2ddid_p[spw_uniq_p[k]])));
      msPol.corrType().put(k, corrType(polId(spw2ddid_p[spw_uniq_p[k]])));
      msPol.corrProduct().put(k, corrProd(polId(spw2ddid_p[spw_uniq_p[k]])));
      msPol.flagRow().put(k, polFlagRow(polId(spw2ddid_p[spw_uniq_p[k]])));
    }

    for(uInt k = 0; k < spw_p.nelements(); ++k)
      inNumChan_p[k]=numChan(spw_p[k]);
    
    Vector<Vector<Int> > spwinds_of_uniq_spws(nuniqSpws);

    totnchan_p.resize(nuniqSpws);
    for(uInt k = 0; k < nuniqSpws; ++k){
      Int maxchan = 0;
      uInt j = 0;

      msOut_p.spectralWindow().addRow();
      msOut_p.dataDescription().addRow();

      totnchan_p[k] = 0;
      spwinds_of_uniq_spws[k].resize();
      for(uInt spwind = 0; spwind < spw_p.nelements(); ++spwind){
        if(spw_p[spwind] == spw_uniq_p[k]){
          Int highchan = nchan_p[spwind] * chanStep_p[spwind]
            + chanStart_p[spwind];

          if(highchan > maxchan)
            maxchan = highchan;

          totnchan_p[k] += nchan_p[spwind];

          // The true is necessary to avoid scrambling previously assigned
          // values.
          spwinds_of_uniq_spws[k].resize(j + 1, true);

          // Warning!  spwinds_of_uniq_spws[k][j] will compile without warning,
          // but dump core at runtime.
          (spwinds_of_uniq_spws[k])[j] = spwind;
          ++j;
        }
      }
      if(maxchan > numChan(spw_uniq_p[k])){
        os << LogIO::SEVERE
           << " Channel settings wrong; exceeding number of channels in spw "
           << spw_uniq_p[k] << LogIO::POST;
        return False;
      }
    }

    // min_k is an index for getting an spw index via spw_uniq_p[min_k].
    // k is an index for getting an spw index via spw_p[k].
    for(uInt min_k = 0; min_k < nuniqSpws; ++min_k){
      uInt k = spwinds_of_uniq_spws[min_k][0];

      if(spwinds_of_uniq_spws[min_k].nelements() > 1 ||
         nchan_p[k] != numChan(spw_p[k])){
        Vector<Double> chanFreqOut(totnchan_p[min_k]);
        Vector<Double> chanFreqIn = chanFreq(spw_uniq_p[min_k]);
        Vector<Double> spwResolOut(totnchan_p[min_k]);
        Vector<Double> spwResolIn = spwResol(spw_uniq_p[min_k]);
        Vector<Double> effBWOut(totnchan_p[min_k]);
        Vector<Double> effBWIn = effBW(spw_uniq_p[min_k]);
        Int outChan = 0;

        chanModification_p = true;

        effBWOut.set(0.0);
        for(uInt rangeNum = 0;
            rangeNum < spwinds_of_uniq_spws[min_k].nelements(); ++rangeNum){
          k = spwinds_of_uniq_spws[min_k][rangeNum];

          for(Int j = 0; j < nchan_p[k]; ++j){
            Int inpChan = chanStart_p[k] + j * chanStep_p[k];

            if(averageChannel_p){
              chanFreqOut[outChan] = (chanFreqIn[inpChan] +
                                      chanFreqIn[inpChan + chanStep_p[k]
                                                 - 1])/2;
              spwResolOut[outChan] = spwResolIn[inpChan] * chanStep_p[k];

              for(Int avgChan = inpChan; avgChan < inpChan + chanStep_p[k];
                  ++avgChan)
                effBWOut[outChan] += effBWIn[avgChan];
            }
            else{
              chanFreqOut[outChan] = chanFreqIn[inpChan];
              spwResolOut[outChan] = spwResolIn[inpChan];
              effBWOut[outChan]    = effBWIn[inpChan];
            }
            ++outChan;
          }
        }
        --outChan;
        
        Double totalBW = chanFreqOut[outChan] - chanFreqOut[0] +
          0.5 * (spwResolOut[outChan] + spwResolOut[0]);

        msSpW.chanFreq().put(min_k, chanFreqOut);
        msSpW.resolution().put(min_k, spwResolOut);
        msSpW.numChan().put(min_k, totnchan_p[min_k]);
        msSpW.chanWidth().put(min_k, spwResolOut);
        msSpW.effectiveBW().put(min_k, spwResolOut);
        msSpW.refFrequency().put(min_k, chanFreqOut[0]);
        msSpW.totalBandwidth().put(min_k, totalBW);
      }
      else{
	msSpW.chanFreq().put(min_k, chanFreq(spw_p[k]));
	msSpW.resolution().put(min_k, spwResol(spw_p[k]));
	msSpW.numChan().put(min_k, numChan(spw_p[k]));    
	msSpW.chanWidth().put(min_k, chanWidth(spw_p[k]));
	msSpW.effectiveBW().put(min_k, effBW(spw_p[k]));
	msSpW.refFrequency().put(min_k, refFreq(spw_p[k]));
	msSpW.totalBandwidth().put(min_k, totBW(spw_p[k]));
      }
      
      msSpW.flagRow().put(min_k, spwFlagRow(spw_p[k]));
      msSpW.freqGroup().put(min_k, freqGroup(spw_p[k]));
      msSpW.freqGroupName().put(min_k, freqGroupName(spw_p[k]));
      msSpW.ifConvChain().put(min_k, ifConvChain(spw_p[k]));
      msSpW.measFreqRef().put(min_k, measFreqRef(spw_p[k]));
      msSpW.name().put(min_k, spwName(spw_p[k]));
      msSpW.netSideband().put(min_k, netSideband(spw_p[k])); 
      
      msDD.flagRow().put(min_k, False);
      msDD.polarizationId().put(min_k, newPolId[min_k]);
      msDD.spectralWindowId().put(min_k, min_k);
    }
    
    return true;
    
  }
  
  
  Bool SubMS::fillFieldTable() 
  {  
    LogIO os(LogOrigin("SubMS", "fillFieldTable()"));
    MSFieldColumns& msField(msc_p->field());
    
    //MSField fieldtable= mssel_p.field();
    Vector<String> optionalCols(1);
    optionalCols[0] = "EPHEMERIS_ID";
    uInt nAddedCols = addOptionalColumns(mssel_p.field(), msOut_p.field(),
                                         optionalCols, true);

    const ROMSFieldColumns& fieldIn = mscIn_p->field(); 
    ROScalarColumn<String> code(fieldIn.code());
    ROArrayColumn<Double>  delayDir(fieldIn.delayDir());
    ROScalarColumn<Bool>   flagRow(fieldIn.flagRow());
    ROScalarColumn<String> name(fieldIn.name());
    ROScalarColumn<Int>    numPoly(fieldIn.numPoly());
    ROArrayColumn<Double>  phaseDir(fieldIn.phaseDir());
    ROArrayColumn<Double>  refDir(fieldIn.referenceDir());
    ROScalarColumn<Int>    sourceId(fieldIn.sourceId());
    ROScalarColumn<Double> time(fieldIn.time());
    
    String refstr;
    // Need to correctly define the direction measures.
    delayDir.keywordSet().asRecord("MEASINFO").get("Ref", refstr);
    //  MDirection::getType(dir1, refstr);
    msField.delayDir().rwKeywordSet().asrwRecord("MEASINFO").define("Ref",
                                                                    refstr);
    phaseDir.keywordSet().asRecord("MEASINFO").get("Ref", refstr);
    msField.phaseDir().rwKeywordSet().asrwRecord("MEASINFO").define("Ref",
                                                                    refstr);
    refDir.keywordSet().asRecord("MEASINFO").get("Ref", refstr);
    msField.referenceDir().rwKeywordSet().asrwRecord("MEASINFO").define("Ref",
                                                                        refstr);
    
    // ...and the time measure...
    time.keywordSet().asRecord("MEASINFO").get("Ref", refstr);
    msField.time().rwKeywordSet().asrwRecord("MEASINFO").define("Ref", refstr);

    fieldRelabel_p.resize(mscIn_p->field().nrow());
    fieldRelabel_p.set(-1);

    os << LogIO::DEBUG1
       << fieldid_p.nelements() << " fields selected out of "
       << mscIn_p->field().nrow()
       << LogIO::POST;

    try{
      msOut_p.field().addRow(fieldid_p.nelements());
      for(uInt k = 0; k < fieldid_p.nelements(); ++k){
        fieldRelabel_p[fieldid_p[k]] = k;
      
        msField.code().put(k, code(fieldid_p[k]));
        msField.delayDir().put(k, delayDir(fieldid_p[k]));
        msField.flagRow().put(k, flagRow(fieldid_p[k]));
        msField.name().put(k, name(fieldid_p[k]));
        msField.numPoly().put(k, numPoly(fieldid_p[k]));
        msField.phaseDir().put(k, phaseDir(fieldid_p[k]));
        msField.referenceDir().put(k, refDir(fieldid_p[k]));
        msField.time().put(k, time(fieldid_p[k]));

        Int inSrcID = sourceId(fieldid_p[k]);
        if(inSrcID < 0)
          msField.sourceId().put(k, -1);
        else
          msField.sourceId().put(k, sourceRelabel_p[inSrcID]);
      }

      if(nAddedCols > 0){
        ROScalarColumn<Int> eID(fieldIn.ephemerisId());

        for(uInt k = 0; k < fieldid_p.nelements(); ++k)
          msField.ephemerisId().put(k, eID(fieldid_p[k]));
      }
    }
    catch(AipsError x){
      os << LogIO::EXCEPTION
         << "Error " << x.getMesg() << " setting up the output FIELD table."
         << LogIO::POST;
    }
    catch(...){
      throw(AipsError("Unknown exception caught and released in fillFieldTable()"));
    }
    
    return True;    
  }

  // Sets up sourceRelabel_p for mapping input SourceIDs (if any) to output
  // ones.  Must be called after fieldid_p is set and before calling
  // fillFieldTable() or copySource().
  void SubMS::relabelSources()
  {
    LogIO os(LogOrigin("SubMS", "relabelSources()"));

    //Source is an optional table, so it may not exist
    if(Table::isReadable(mssel_p.sourceTableName())){
      // Note that mscIn_p->field().sourceId() has ALL of the sourceIDs in
      // the input MS, not just the selected ones.
      const Vector<Int>& inSrcIDs = mscIn_p->field().sourceId().getColumn();

      Int highestInpSrc = max(inSrcIDs);
    
      if(highestInpSrc < 0)                   // Ensure space for -1.
        highestInpSrc = 0;
      sourceRelabel_p.resize(highestInpSrc + 1);
      sourceRelabel_p.set(-1);   	          // Default to "any".

      // Enable sourceIDs that are actually referred to by selected fields, and
      // remap them using j.
      uInt j = 0;
      for(uInt k = 0; k < fieldid_p.nelements(); ++k){
        if(inSrcIDs[fieldid_p[k]] > -1){
          sourceRelabel_p[inSrcIDs[fieldid_p[k]]] = j;
          ++j;
        }
      }
    }
    else{
      os << LogIO::NORMAL
         << "The input MS does not have the optional SOURCE table.\n"
         << "-1 will be used as a generic source ID."
         << LogIO::POST;
      sourceRelabel_p.resize(1);
      sourceRelabel_p.set(-1);   	          // Default to "any".      
    }
  }

  //
  // The method is called only in makeSubMS().  Should really be
  // called in setupMS.  But the latter has been made into a static
  // method and verifyColumns() cannot be called there.
  //
  void SubMS::verifyColumns(const MeasurementSet& ms, const Vector<String>& colNames)
  {
    LogIO os(LogOrigin("SubMS", "verifyColumns()"));
    for(uInt i=0;i<colNames.nelements();i++)
      if (!ms_p.tableDesc().isColumn(colNames[i]))
	{
	  ostringstream ostr;
	  ostr << "Desired column (" << colNames[i] << ") not found in the input MS ("
	       << ms_p.tableName() << ").";
	  throw(AipsError(ostr.str()));
	}
  }

  Bool SubMS::doWriteImagingWeight(const ROMSColumns& inMsc, const Vector<String>& columnName)
  {
    Bool allCols=isAllColumns(columnName);
    Bool inputImgWtsExist=(!inMsc.imagingWeight().isNull() &&
			inMsc.imagingWeight().isDefined(0));
    if (allCols && !inputImgWtsExist)
      {
	LogIO os(LogOrigin("SubMS", "doWriteImgingWeight()"));
	os << LogIO::WARN 
	   << "All data columns found, but not the IMAGING_WEIGHT column in the input MS. "
	   << "This is strange.  Brace for unnatural results."
	   << LogIO::POST;
      }
    return  (allCols && inputImgWtsExist);
  }

  Int SubMS::regridSpw(String& regridMessage,
		       const String& outframe,
		       const String& regridQuant,
		       const Double regridVeloRestfrq,
		       const String& regridInterpMeth,
		       const Double regridCenter, 
		       const Double regridBandwidth, 
		       const Double regridChanWidth,
		       const Int phaseCenterFieldId,
		       MDirection phaseCenter,
		       const Bool centerIsStart,
		       const Int nchan,
		       const Int width,
		       const Int start
		  ){
    
    LogIO os(LogOrigin("SubMS", "regridSpw()"));

    Int rval = -1; // return values: -1 = MS not modified, 1 = MS modified and OK, 
                   // 0 = MS modified but not OK 

    // get the original table description of the MS 
    TableDesc origMSTD(ms_p.actualTableDesc());
    if(!origMSTD.isColumn("CORRECTED_DATA") &&  !origMSTD.isColumn("DATA") 
       && !origMSTD.isColumn("FLOAT_DATA") && !origMSTD.isColumn("MODEL_DATA")){ 
      os << LogIO::WARN << "MS has no DATA columns. Nothing to regrid." << LogIO::POST;
    }	  

    // Set up a little database to keep track of which pairs (FieldId, SPWId) have already
    // been dealt with and what paramters were used

    vector<Int> oldSpwId;
    vector<Int> oldFieldId;
    vector<Int> newDataDescId;
    vector<Bool> regrid;
    vector<Bool> transform;
    vector<MDirection> theFieldDirV;
    vector<MPosition> mObsPosV;
    vector<MFrequency::Types> fromFrameTypeV;
    vector<MFrequency::Ref> outFrameV;
    vector< Vector<Double> > xold; 
    vector< Vector<Double> > xout; 
    vector< Vector<Double> > xin; 
    //    vector< InterpolateArray1D<Double,Complex>::InterpolationMethod > method;
    // This is a temporary fix until InterpolateArray1D<Double, Complex> works.
    vector< InterpolateArray1D<Float,Complex>::InterpolationMethod > method;
    vector< InterpolateArray1D<Double,Float>::InterpolationMethod > methodF;
    

    Bool msModified = False;

    // Loop 1: Verify the input parameters, no modification of the MS
    if(!setRegridParameters(oldSpwId,
			    oldFieldId,
			    newDataDescId,
			    regrid,
			    transform,
			    theFieldDirV,
			    mObsPosV,
			    fromFrameTypeV,
			    outFrameV,
			    xold,
			    xout, 
			    xin, 
			    method,
			    methodF,
			    msModified,
			    outframe,
			    regridQuant,
			    regridVeloRestfrq,
			    regridInterpMeth,
			    regridCenter,
			    regridBandwidth, 
			    regridChanWidth,
			    phaseCenterFieldId,
			    phaseCenter,
			    False, // <-----
			    os,
			    regridMessage,
			    centerIsStart,
			    nchan,
			    width,
			    start
			    )){ // an error occured
      return -1;
    }

    // Loop 2: Write modified DD, SPW, and SOURCE tables

    if(!setRegridParameters(oldSpwId,
			    oldFieldId,
			    newDataDescId,
			    regrid,
			    transform,
			    theFieldDirV,
			    mObsPosV,
			    fromFrameTypeV,
			    outFrameV,
			    xold,
			    xout, 
			    xin, 
			    method,
			    methodF,
			    msModified,
			    outframe,
			    regridQuant,
			    regridVeloRestfrq,
			    regridInterpMeth,
			    regridCenter,
			    regridBandwidth, 
			    regridChanWidth,
			    phaseCenterFieldId,
			    phaseCenter,
			    True, // <-----
			    os,
			    regridMessage,
			    centerIsStart,
			    nchan,
			    width,
			    start
			    )){ // an error occured
      if(msModified){
	return 0;
      }
      else{
	return -1;
      }
    }
    
    if(!msModified){ // nothing to be done
      return -1;
    }
    
    // now we need to modify the main table ...

    Bool needRegridding = False;
    for(uInt i=0; i<regrid.size(); i++){
      if(regrid[i]){
	needRegridding = True;
      }
    }

    if(needRegridding){

      os << LogIO::NORMAL << "Main table data array columns will be rewritten ..." <<  LogIO::POST;

      // create the "partner" columns, i.e. rename the old array columns to old...
      // and create new empty columns with the original names to hold the regridded values

      IPosition tileShape = MSTileLayout::tileShape(IPosition(2,1,xout[0].size()));

      createPartnerColumn(origMSTD, "CORRECTED_DATA", "oldCORRECTED_DATA", 3, tileShape);
      createPartnerColumn(origMSTD, "DATA", "oldDATA", 3, tileShape);
      createPartnerColumn(origMSTD, "FLOAT_DATA", "oldFLOAT_DATA", 3, tileShape);
      createPartnerColumn(origMSTD, "IMAGING_WEIGHT", "oldIMAGING_WEIGHT", 3, tileShape);
      createPartnerColumn(origMSTD, "LAG_DATA", "oldLAG_DATA", 3, tileShape);
      createPartnerColumn(origMSTD, "MODEL_DATA", "oldMODEL_DATA", 3, tileShape);
      createPartnerColumn(origMSTD, "SIGMA_SPECTRUM", "oldSIGMA_SPECTRUM", 3, tileShape);
      createPartnerColumn(origMSTD, "WEIGHT_SPECTRUM", "oldWEIGHT_SPECTRUM", 3, tileShape);
      createPartnerColumn(origMSTD, "FLAG", "oldFLAG", 3, tileShape);

      createPartnerColumn(origMSTD, "FLAG_CATEGORY", "oldFLAG_CATEGORY", 4,
			  IPosition(4,tileShape(0),tileShape(1),1, tileShape(2)));
    }

    MSMainColumns mainCols(ms_p);

    // columns which depend on the number of frequency channels and may need to be regridded:
    // DATA, FLOAT_DATA, CORRECTED_DATA, MODEL_DATA, IMAGING_WEIGHT, LAG_DATA, SIGMA_SPECTRUM,
    // WEIGHT_SPECTRUM, FLAG, and FLAG_CATEGORY    
    ArrayColumn<Complex> CORRECTED_DATACol =  mainCols.correctedData();
    ArrayColumn<Complex>* oldCORRECTED_DATAColP  = 0;
    ArrayColumn<Complex>  DATACol =  mainCols.data();
    ArrayColumn<Complex>* oldDATAColP = 0;
    ArrayColumn<Float> FLOAT_DATACol =  mainCols.floatData();
    ArrayColumn<Float>* oldFLOAT_DATAColP = 0;
    ArrayColumn<Float> IMAGING_WEIGHTCol =  mainCols.imagingWeight();
    ArrayColumn<Float>* oldIMAGING_WEIGHTColP = 0;
    ArrayColumn<Complex> LAG_DATACol =  mainCols.lagData();
    ArrayColumn<Complex>* oldLAG_DATAColP = 0;
    ArrayColumn<Complex> MODEL_DATACol =  mainCols.modelData();
    ArrayColumn<Complex>* oldMODEL_DATAColP = 0;
    ArrayColumn<Float> SIGMA_SPECTRUMCol =  mainCols.sigmaSpectrum();
    ArrayColumn<Float>* oldSIGMA_SPECTRUMColP = 0;
    ArrayColumn<Float> WEIGHT_SPECTRUMCol =  mainCols.weightSpectrum();
    ArrayColumn<Float>* oldWEIGHT_SPECTRUMColP = 0;
    ArrayColumn<Bool> FLAGCol =  mainCols.flag();
    ArrayColumn<Bool>* oldFLAGColP = 0;
    ArrayColumn<Bool> FLAG_CATEGORYCol =  mainCols.flagCategory();
    ArrayColumn<Bool>* oldFLAG_CATEGORYColP = 0;

    if(needRegridding){
      // (create column objects for all "partners" of the array columns to be modified)
      if(!CORRECTED_DATACol.isNull()){
	oldCORRECTED_DATAColP = new ArrayColumn<Complex>(ms_p, "oldCORRECTED_DATA");
      }
      if(!DATACol.isNull()){
	oldDATAColP = new ArrayColumn<Complex>(ms_p, "oldDATA");
      }
      if(!FLOAT_DATACol.isNull()){
	oldFLOAT_DATAColP = new ArrayColumn<Float>(ms_p, "oldFLOAT_DATA");
      }
      if(!IMAGING_WEIGHTCol.isNull()){
	oldIMAGING_WEIGHTColP = new ArrayColumn<Float>(ms_p, "oldIMAGING_WEIGHT");
      }
      if(!LAG_DATACol.isNull()){
	oldLAG_DATAColP = new ArrayColumn<Complex>(ms_p, "oldLAG_DATA");
      }
      if(!MODEL_DATACol.isNull()){
	oldMODEL_DATAColP = new ArrayColumn<Complex>(ms_p, "oldMODEL_DATA");
      }
      if(!SIGMA_SPECTRUMCol.isNull()){
	oldSIGMA_SPECTRUMColP = new ArrayColumn<Float>(ms_p, "oldSIGMA_SPECTRUM");
      }
      if(!WEIGHT_SPECTRUMCol.isNull()){
	oldWEIGHT_SPECTRUMColP = new ArrayColumn<Float>(ms_p, "oldWEIGHT_SPECTRUM");
      }
      if(!FLAGCol.isNull()){
	oldFLAGColP = new ArrayColumn<Bool>(ms_p, "oldFLAG"); 
      }
      if(!FLAG_CATEGORYCol.isNull()){
	oldFLAG_CATEGORYColP = new ArrayColumn<Bool>(ms_p, "oldFLAG_CATEGORY");
      }
    } // end if needRegridding

    // administrational columns needed from the main table
    ScalarColumn<Int> fieldIdCol = mainCols.fieldId();
    ScalarColumn<Int> DDIdCol = mainCols.dataDescId();
    ScalarMeasColumn<MEpoch> mainTimeMeasCol = mainCols.timeMeas();
   
    // columns needed from subtables
    MSDataDescription ddtable=ms_p.dataDescription();
    MSDataDescColumns DDCols(ddtable);
    ScalarColumn<Int> SPWIdCol = DDCols.spectralWindowId(); 
    
    // Loop 3: Apply to MAIN table rows
    
    //    cout << "Modifying main table ..." << endl;

    uInt nMainTabRows = ms_p.nrow();

    // prepare progress meter
    Float progress = 0.4;
    Float progressStep = 0.4;
    if(ms_p.nrow()>1000000){
      progress = 0.2;
      progressStep = 0.2;
    }

    for(uInt mainTabRow=0; mainTabRow<nMainTabRows; mainTabRow++){
      
      // For each MAIN table row, the FIELD_ID cell and the DATA_DESC_ID cell are read 
      Int theFieldId = fieldIdCol(mainTabRow);
      Int theDataDescId = DDIdCol(mainTabRow);
      // and the SPW_ID extracted from the corresponding row in the DATA_DESCRIPTION table.
      Int theSPWId = SPWIdCol(theDataDescId);

      MEpoch theObsTime = mainTimeMeasCol(mainTabRow);
      
      //  The pair (theFieldId, theSPWId) is looked up in the "done table". 
      Int iDone = -1;
      for (uInt i=0; i<oldSpwId.size(); i++){
	if(oldSpwId[i]==theSPWId && (oldFieldId[i]==theFieldId || phaseCenterFieldId>=-1)){
	  // if common phase center is given, treat all fields the same
	  iDone = i;
	  break;
	}
      }
      if(iDone<0){ // should not occur
	os << LogIO::SEVERE << "Internal error: Did not find regrid parameters for field ==" 
	   << theFieldId << " spw ==" <<  theSPWId << LogIO::POST;
	return 0;
      }
      
      
      if (DDIdCol(mainTabRow)!=newDataDescId[iDone]){
	// If the data description actually changed, then DATA_DESC_ID 
	//	of this main table row is set to the new value given in the "done" table
	DDIdCol.put(mainTabRow, newDataDescId[iDone]);
	
      }
      
      //Furthermore, if regrid[iDone] is true, the visibilities and all 
      // channel-number-dependent arrays need to be regridded.
      if(regrid[iDone]){

	Bool doExtrapolate = False;

	// regrid the complex columns
	Array<Complex> yout;
	Array<Bool> youtFlags;
	Bool youtFlagsWritten(False);
	Array<Complex> yin;
	Array<Bool> yinFlags((*oldFLAGColP)(mainTabRow));
	
	// Note: to use a  Vector<Float> here instead of the original Vector<Double>
	// is a temporary fix until InterpolateArray1D<Double, Complex> works.
	Vector<Float> xinff(xold[iDone].size());
	Vector<Double> xindd(xold[iDone].size());
	Vector<Float> xoutff(xout[iDone].size()); 
	for(uInt i=0; i<xout[iDone].size(); i++){
	  xoutff[i] = xout[iDone][i];
	}

	if(transform[iDone]){
	  doExtrapolate = True;
	  // create frequency machine for this time stamp
	  MFrequency::Ref fromFrame = MFrequency::Ref(fromFrameTypeV[iDone], MeasFrame(theFieldDirV[iDone], mObsPosV[iDone], theObsTime));
	  Unit unit(String("Hz"));
	  MFrequency::Convert freqTrans2(unit, fromFrame, outFrameV[iDone]);
	
	  // transform from this timestamp to the one of the output SPW
	  for(uInt i=0; i<xindd.size(); i++){
	    xindd[i] = freqTrans2(xold[iDone][i]).get(unit).getValue();
	    xinff[i] = xindd[i];
	  }
// 	  if(mainTabRow==0){ // debug output
// 	    Int i = 25;
// 	    cout << "i " << i << " xin " << setprecision(9) << xin[iDone][i] << " xindd " << setprecision(9) << xindd[i] 
// 		 << " xout " << setprecision(9) << xout[iDone][i] << endl;
// 	    cout << "i " << i << " vradxin " << setprecision(9) << vrad(xin[iDone][i], regridVeloRestfrq) 
// 		 << " vradxindd " << setprecision(9) << vrad(xindd[i], regridVeloRestfrq)  
// 		 << " xout " << setprecision(9) << vrad(xout[iDone][i], regridVeloRestfrq) << endl;
// 	  }
	}
	else{ // no additional transformation of input grid
	  for(uInt i=0; i<xin[iDone].size(); i++){ // cannot use assign due to different data type
	    xindd[i] = xin[iDone][i];
	    xinff[i] = xindd[i];
	  }
	}
	
	if(!CORRECTED_DATACol.isNull()){
	  yin.assign((*oldCORRECTED_DATAColP)(mainTabRow));
	  InterpolateArray1D<Float, Complex>::interpolate(yout, // the new visibilities
							  youtFlags, // the new flags
							  xoutff, // the new channel centers
							  xinff, // the old channel centers
							  yin, // the old visibilities 
							  yinFlags,// the old flags
							  method[iDone], // the interpol method
							  False, // for flagging: good is not true
							  doExtrapolate // do not extrapolate
							  );
	  CORRECTED_DATACol.put(mainTabRow, yout);
	  if(!youtFlagsWritten){ 
	    FLAGCol.put(mainTabRow, youtFlags);
	    youtFlagsWritten = True;
	  }
	}
	if(!DATACol.isNull()){
	  yin.assign((*oldDATAColP)(mainTabRow));
	  InterpolateArray1D<Float, Complex>::interpolate(yout, youtFlags, xoutff, xinff, 
							  yin, yinFlags, method[iDone], False, doExtrapolate);
	  DATACol.put(mainTabRow, yout);
	  if(!youtFlagsWritten){ 
	    FLAGCol.put(mainTabRow, youtFlags);
	    youtFlagsWritten = True;
	  }	  
	}
	if(!LAG_DATACol.isNull()){
	  yin.assign((*oldLAG_DATAColP)(mainTabRow));
	  InterpolateArray1D<Float, Complex>::interpolate(yout, youtFlags, xoutff, xinff, 
							  yin, yinFlags, method[iDone], False, doExtrapolate);
	  LAG_DATACol.put(mainTabRow, yout);
	}
	if(!MODEL_DATACol.isNull()){
	  yin.assign((*oldMODEL_DATAColP)(mainTabRow));
	  InterpolateArray1D<Float, Complex>::interpolate(yout, youtFlags, xoutff, xinff, 
							  yin, yinFlags, method[iDone], False, doExtrapolate);
	  MODEL_DATACol.put(mainTabRow, yout);
	  if(!youtFlagsWritten){ 
	    FLAGCol.put(mainTabRow, youtFlags);
	    youtFlagsWritten = True;
	  }
	}
	
	// regrid the Float columns
	Array<Float> yinf;
	Array<Float> youtf;
	if(!FLOAT_DATACol.isNull()){
	  yinf.assign((*oldFLOAT_DATAColP)(mainTabRow));
	  InterpolateArray1D<Double, Float>::interpolate(youtf, youtFlags, xout[iDone], xindd, 
							 yinf, yinFlags, methodF[iDone], False, doExtrapolate);
	  FLOAT_DATACol.put(mainTabRow, youtf);
	  if(!youtFlagsWritten){ 
	    FLAGCol.put(mainTabRow, youtFlags);
	    youtFlagsWritten = True;
	  }
	}
	if(!IMAGING_WEIGHTCol.isNull()){
	  yinf.assign((*oldIMAGING_WEIGHTColP)(mainTabRow));
	  InterpolateArray1D<Double, Float>::interpolate(youtf, youtFlags, xout[iDone], xindd, 
							 yinf, yinFlags, methodF[iDone], False, doExtrapolate);
	  IMAGING_WEIGHTCol.put(mainTabRow, youtf);
	}
	if(!SIGMA_SPECTRUMCol.isNull()){
	  yinf.assign((*oldSIGMA_SPECTRUMColP)(mainTabRow));
	  InterpolateArray1D<Double, Float>::interpolate(youtf, youtFlags, xout[iDone], xindd, 
							 yinf, yinFlags, methodF[iDone], False, doExtrapolate);
	  SIGMA_SPECTRUMCol.put(mainTabRow, youtf);
	}
	if(!WEIGHT_SPECTRUMCol.isNull() && oldWEIGHT_SPECTRUMColP->isDefined(mainTabRow)){ // required column, but can be empty
	  yinf.assign((*oldWEIGHT_SPECTRUMColP)(mainTabRow));
	  InterpolateArray1D<Double, Float>::interpolate(youtf, youtFlags, xout[iDone],
                                                         xindd, yinf, yinFlags,
                                                         methodF[iDone], False, doExtrapolate);
	  WEIGHT_SPECTRUMCol.put(mainTabRow, youtf);
	}
	
	// deal with FLAG_CATEGORY
	// note: FLAG_CATEGORY is a required column, but it can be undefined (empty)
	
	if(FLAG_CATEGORYCol.isDefined(mainTabRow)){
	  Array<Bool> flagCat((*oldFLAG_CATEGORYColP)(mainTabRow));  
	  IPosition flagCatShape = (*oldFLAG_CATEGORYColP).shape(mainTabRow);
	  Int nCorrelators = flagCatShape(0); // get the dimension of the first axis
	  Int nChannels = flagCatShape(1); // get the dimension of the second axis
	  Int nCat = flagCatShape(2); // the dimension of the third axis ==
                                      // number of categories
	  Int nOutChannels = xout[iDone].size();
	  
	  Vector<Float> dummyYin(nChannels);
	  Vector<Float> dummyYout(nOutChannels);
	  Array<Bool> flagCatOut(IPosition(3, nCorrelators, nOutChannels, nCat)); 
	  
	  for(Int i=0; i<nCat; i++){
	    IPosition start(0,0,i), length (nCorrelators,nChannels,i), stride (1,1,0);
	    Slicer slicer (start, length, stride, Slicer::endIsLast);
	    yinFlags.assign(flagCat(slicer));
	    InterpolateArray1D<Double, Float>::interpolate(dummyYout, youtFlags,
                                                           xout[iDone], xindd, 
							   dummyYin, yinFlags,
                                                           methodF[iDone], False, False);
	    // write the slice to the array flagCatOut
	    for(Int j=0; j<nCorrelators; j++){
	      for(Int k=0; k<nOutChannels; k++){
		flagCatOut(IPosition(3, j, k, i)) = youtFlags(IPosition(2,j,k));
	      }
	    }
	  }
	  
	  FLAG_CATEGORYCol.put(mainTabRow, flagCatOut);
	  
	}
	
	msModified = True;
	
      } // end if regridding necessary

      if(mainTabRow>nMainTabRows*progress){
	cout << "regridSpw progress: " << progress*100 << "% processed ... " << endl;
	progress += progressStep;
      }
      
    } // end loop over main table rows
    cout << "regridSpw progress: 100% processed." << endl;

    if(msModified){
      if(needRegridding){

	// remove the "partner" columns
	if(!CORRECTED_DATACol.isNull()){
	  ms_p.removeColumn("oldCORRECTED_DATA");
	}
	if(!DATACol.isNull()){
	  ms_p.removeColumn("oldDATA");
	}
	if(!FLOAT_DATACol.isNull()){
	  ms_p.removeColumn("oldFLOAT_DATA");
	}
	if(!IMAGING_WEIGHTCol.isNull()){
	  ms_p.removeColumn("oldIMAGING_WEIGHT");
	}
	if(!LAG_DATACol.isNull()){
	  ms_p.removeColumn("oldLAG_DATA");
	}
	if(!MODEL_DATACol.isNull()){
	  ms_p.removeColumn("oldMODEL_DATA");
	}
	if(!SIGMA_SPECTRUMCol.isNull()){
	  ms_p.removeColumn("oldSIGMA_SPECTRUM");
	}
	if(!WEIGHT_SPECTRUMCol.isNull()){
	  ms_p.removeColumn("oldWEIGHT_SPECTRUM");
	}
	if(!FLAGCol.isNull()){
	  ms_p.removeColumn("oldFLAG");
	} 
	if(!FLAG_CATEGORYCol.isNull()){
	  ms_p.removeColumn("oldFLAG_CATEGORY");
	}
      }
      
      //
      // If all scratch columns are in the new MS, set the CHANNEL_SELECTION
      // keyword for the MODEL_DATA column.  This is apparently used
      // in at least imager to decide if MODEL_DATA and CORRECTED_DATA
      // columns should be initialized or not.
      //
      if (!CORRECTED_DATACol.isNull() && !MODEL_DATACol.isNull() && !IMAGING_WEIGHTCol.isNull()){
	MSSpWindowColumns msSpW(ms_p.spectralWindow());
	Int nSpw=ms_p.spectralWindow().nrow();
	if(nSpw==0) nSpw=1;
	Matrix<Int> selection(2,nSpw);
	selection.row(0)=0; //start
	selection.row(1)=msSpW.numChan().getColumn();
	ArrayColumn<Complex> mcd(ms_p,MS::columnName(MS::MODEL_DATA));
	mcd.rwKeywordSet().define("CHANNEL_SELECTION",selection);
      }

      ms_p.flush();
      rval = 1; // successful modification
    }
    return rval;

  }


  Bool SubMS::createPartnerColumn(TableDesc& modMSTD,
				  const String& oldName, 
				  const String& newName,
				  const Int& hypercolumnDim,
				  const IPosition& tileShape
				  ){
    Bool rval = False;
    if(modMSTD.isColumn(oldName)){
      // get the old column desc
      ColumnDesc myColDesc(modMSTD.columnDesc(oldName));
      // move the column away
      ms_p.renameColumn(newName, oldName);
      // rename the hypercolumn
      String hcName(myColDesc.dataManagerGroup());
      String oldHcName = hcName;
      String newHcName = hcName + "B";
      if(!oldHcName.empty() && ms_p.actualTableDesc().isHypercolumn(oldHcName)){
	ms_p.renameHypercolumn(newHcName, oldHcName);
      }
      // rename the datamanager
      DataManager* myDM = ms_p.findDataManager(oldHcName);
      ((TiledStMan*) myDM)->setDataManagerName(newHcName);
      // create new hypercolumn and a new column with new data manager
      TiledShapeStMan*  tiledStMan = new TiledShapeStMan(oldHcName, tileShape);
      ms_p.addColumn(myColDesc, *tiledStMan);
      modMSTD.defineHypercolumn(oldHcName, hypercolumnDim, stringToVector(oldName));

      rval = True;
    }
    return rval;
  }


  Bool SubMS::regridChanBounds(Vector<Double>& newChanLoBound, 
			       Vector<Double>& newChanHiBound,
			       const Double regridCenterC,
			       const Double regridBandwidth, 
			       const Double regridChanWidthC, 
			       const Double regridVeloRestfrq, 
			       const String regridQuant,
			       const Vector<Double>& transNewXin, 
			       const Vector<Double>& transCHAN_WIDTH,
			       String& message,
			       const Bool centerIsStartC,
			       const Int nchanC,
			       const Int width,
			       const Int start
			       ){
    ostringstream oss;

    // verify regridCenter, regridBandwidth, and regridChanWidth 
    // Note: these are in the units given by regridQuant!

    Int oldNUM_CHAN = transNewXin.size();

    Bool centerIsStart = centerIsStartC;
    Double regridChanWidth = regridChanWidthC;
    Double regridCenter = regridCenterC;
    Int nchan = nchanC;
    
    if(regridQuant=="chan"){ ////////////////////////
      // channel numbers ...
      Int regridCenterChan = -1;
      Int regridBandwidthChan = -1;
      Int regridChanWidthChan = -1;

      if(regridCenter<-1E30){ // not set
	// find channel center closest to center of bandwidth
	Double BWCenterF = (transNewXin[0]+transNewXin[oldNUM_CHAN-1])/2.;
	for(Int i=0; i<oldNUM_CHAN; i++){
	  if(transNewXin[i] >= BWCenterF){
	    regridCenterChan = i;
	    break;
	  }
	}
	centerIsStart = False;
      }
      else if(0. <= regridCenter && regridCenter < Double(oldNUM_CHAN)){ // valid input
	regridCenterChan = (Int) floor(regridCenter);  
      }
      else { // invalid
	if(centerIsStart){
	  oss << "SPW start ";
	}
	else{
	  oss << "SPW center ";
	}
	oss << regridCenter << " outside valid range which is "
	    << 0 << " - " << oldNUM_CHAN-1 <<".";
	message = oss.str();
	return False;  
      }  
      
      if(regridBandwidth<=0.|| nchan>0){ // not set or nchan set
	if(nchan>0){
	  regridBandwidthChan = nchan;
	}
	else{
	  regridBandwidthChan = oldNUM_CHAN;
	}
      }
      else{
	regridBandwidthChan = (Int) floor(regridBandwidth);
      }

      if(centerIsStart){
	regridCenterChan = regridCenterChan + regridBandwidthChan/2;
	centerIsStart = False;
      }

      if(regridCenterChan-regridBandwidthChan/2 < 0) { // center too close to lower edge
	regridBandwidthChan = 2 * regridCenterChan + 1;
	oss << " *** Requested output SPW width too large." << endl;
      }
      if( oldNUM_CHAN < regridCenterChan+regridBandwidthChan/2){  // center too close to upper edge
	regridBandwidthChan = 2*(oldNUM_CHAN - regridCenterChan);
	oss << " *** Requested output SPW width too large." << endl;
      } 
      
      if(regridChanWidth < 1.){
	regridChanWidthChan = 1;
      }
      else if(regridChanWidth > Double(regridBandwidthChan)){
	regridChanWidthChan = regridBandwidthChan; // i.e. SPW = a single channel
	oss << " *** Requested output channel width too large. Adjusted to maximum possible value." << endl;
      }
      else { // valid input
	regridChanWidthChan = (Int) floor(regridChanWidth);
	if(nchan>0){
	  regridBandwidthChan = nchan * regridChanWidthChan;
	}
      }
      
      if(regridBandwidthChan != floor(regridBandwidth)){
	oss << " *** Output SPW width set to " << regridBandwidthChan << " original channels" << endl;
	oss << "     in an attempt to keep center of output SPW close to center of requested SPW." << endl;
      } 
      
      // calculate newChanLoBound and newChanHiBound from regridCenterChan, regridBandwidthChan, and regridChanWidthChan
      Int bwLowerEndChan = regridCenterChan - regridBandwidthChan/2;
      Int bwUpperEndChan = bwLowerEndChan + regridBandwidthChan - 1;
      Int numNewChanDown = 0;
      Int numNewChanUp = 0;
      
      if(regridChanWidthChan == regridBandwidthChan){ // only one new channel
	newChanLoBound.resize(1);
	newChanHiBound.resize(1);
	newChanLoBound[0] = transNewXin[bwLowerEndChan]-transCHAN_WIDTH[bwLowerEndChan]/2.;
	newChanHiBound[0] = transNewXin[bwUpperEndChan]+transCHAN_WIDTH[bwUpperEndChan]/2.;
	numNewChanUp = 1;
      }
      else { // have more than one new channel
	// Need to accomodate the possibility that the original channels are
	// not contiguous!

        // the numbers of the Channels from which the lower bounds will be taken for the new channels 
	vector<Int> loNCBup;
	// starting from the central channel going up
	vector<Int> hiNCBup; // the numbers of the Channels from which the high
                             // bounds will be taken for the new channels
	// starting from the central channel going up
	vector<Int> loNCBdown; // the numbers of the Channels from which the
                               // lower bounds will be taken for the new
                               // channels
	// starting from the central channel going down
	vector<Int> hiNCBdown; // the numbers of the Channels from which the
                               // high bounds will be taken for the new
                               // channels
	// starting from the central channel going down
	//    Want to keep the center of the center channel at the center of
	//    the new center channel if the bandwidth is an odd multiple of the
        //    new channel width 
	//    otherwise the center channel is the lower edge of the new center channel
	Int startChan;
	Double tnumChan = regridBandwidthChan/regridChanWidthChan;
	if((Int)tnumChan/2 != tnumChan/2.){
          // odd multiple 
	  startChan = regridCenterChan-regridChanWidthChan/2;
	}
	else{
	  startChan = regridCenterChan;
	}
	for(Int i=startChan; i<=bwUpperEndChan; i+=regridChanWidthChan){ // upper half
	  loNCBup.push_back(i);
	  if(i+regridChanWidthChan-1<=bwUpperEndChan){
            // can go one more normal step up
	    hiNCBup.push_back(i+regridChanWidthChan-1);
	  }
	  else{
            // create narrower channels at the edges if necessary
	    oss << " *** Last channel at upper edge of new SPW made only " << bwUpperEndChan-i+1 
		<< " original channels wide to fit given total bandwidth." << endl;
	    hiNCBup.push_back(bwUpperEndChan);
	  }
	}

        // lower half
	for(Int i=startChan - 1; i>=bwLowerEndChan; i-=regridChanWidthChan){ 
	  hiNCBdown.push_back(i);
	  if(i-regridChanWidthChan+1>=bwLowerEndChan){
            // can go one more normal step down
	    loNCBdown.push_back(i-regridChanWidthChan+1);
	  }
	  else{
            // create narrower channels at the edges if necessary
	    oss << " *** First channel at lower edge of new SPW made only " << i-bwLowerEndChan+1 
		<< " original channels wide to fit given total bandwidth." << endl;
	    loNCBdown.push_back(bwLowerEndChan);
	  }
	}

        // the number of channels below the central one
	numNewChanDown = loNCBdown.size();

        // the number of channels above and including the central one
	numNewChanUp = loNCBup.size();

	newChanLoBound.resize(numNewChanDown+numNewChanUp);
	newChanHiBound.resize(numNewChanDown+numNewChanUp);
	for(Int i=0; i<numNewChanDown; i++){
	  Int k = numNewChanDown-i-1; // need to assign in reverse
	  newChanLoBound[i] = transNewXin[loNCBdown[k]] -
            transCHAN_WIDTH[loNCBdown[k]]/2.; 
	  newChanHiBound[i] = transNewXin[hiNCBdown[k]] +
            transCHAN_WIDTH[hiNCBdown[k]]/2.;
	}
	for(Int i=0; i<numNewChanUp; i++){
	  newChanLoBound[i+numNewChanDown] = transNewXin[loNCBup[i]] -
            transCHAN_WIDTH[loNCBup[i]]/2.;
	  newChanHiBound[i+numNewChanDown] = transNewXin[hiNCBup[i]] +
            transCHAN_WIDTH[hiNCBup[i]]/2.;
	}
      } // end if 

      oss << " New channels defined based on original channels" << endl
	  << " Central channel contains original channel " <<  regridCenterChan << endl 
	  << " Channel width = " << regridChanWidthChan
          << " original channels" << endl
	  << " Total width of SPW = " <<  regridBandwidthChan << " original channels == " 
	  << numNewChanDown + numNewChanUp << " new channels" << endl;
      uInt nc = newChanLoBound.size();
      oss << " Total width of SPW (in output frame) = " << newChanHiBound[nc-1] - newChanLoBound[0] 
	  << " Hz" << endl;
      oss << " Lower edge = " << newChanLoBound[0] << " Hz,"
	  << " upper edge = " << newChanHiBound[nc-1] << " Hz" << endl;

      message = oss.str();
      return True;
    }
    else { // we operate on real numbers /////////////////
      // first transform them to frequencies
      Double regridCenterF = -1.; // initialize as "not set"
      Double regridBandwidthF = -1.;
      Double regridChanWidthF = -1.;

      if(regridQuant=="vrad"){ ///////////////
	// radio velocity ...
	// need restfrq
	if(regridVeloRestfrq<-1E30){ // means "not set"
	  oss << "Parameter \"restfreq\" needs to be set if regrid_quantity==vrad. Cannot proceed with regridSpw ..."; 
	  message = oss.str();
	  return False;
	}
	else if(regridVeloRestfrq < 0. || regridVeloRestfrq > 1E30){
	  oss << "Parameter \"restfreq\" value " << regridVeloRestfrq << " is invalid.";
	  message = oss.str();
	  return False;
	}	  
	Double regridCenterVel; 
	if(regridCenter>-C::c){
          // (we deal with invalid values later)
	  regridCenterF = freq_from_vrad(regridCenter,regridVeloRestfrq);

	  regridCenterVel = regridCenter;
	}
	else{ // center was not specified
	  regridCenterF = (transNewXin[0]+transNewXin[oldNUM_CHAN-1])/2.;
	  regridCenterVel = vrad(regridCenterF,regridVeloRestfrq);
	  centerIsStart = False;
	}
	if(regridBandwidth > 0.){
	  // can convert start to center
	  if(centerIsStart){
	    regridCenterVel = regridCenter - regridBandwidth/2.;
	    regridCenterF = freq_from_vrad(regridCenterVel,regridVeloRestfrq);
	    centerIsStart = False;
	  }
	  Double bwUpperEndF = freq_from_vrad(regridCenterVel - regridBandwidth/2.,
                                              regridVeloRestfrq);
	  regridBandwidthF = 2.* (bwUpperEndF - regridCenterF); 
	}
	if(regridChanWidth > 0.){
	  Double chanUpperEdgeF = freq_from_vrad(regridCenterVel - regridChanWidth/2.,
                                                 regridVeloRestfrq);
	  regridChanWidthF = 2.* (chanUpperEdgeF - regridCenterF); 
	}

      }
      else if(regridQuant=="vopt"){ ///////////
	// optical velocity ...
	// need restfrq
	if(regridVeloRestfrq < -1E30){ // means "not set"
	  oss << "Parameter \"restfreq\" needs to be set if regrid_quantity==vopt. Cannot proceed with regridSpw ...";
	  message = oss.str();
	  return False;
	}
	else if(regridVeloRestfrq <= 0. || regridVeloRestfrq > 1E30){
	  oss << "Parameter \"restfreq\" value " << regridVeloRestfrq
              << " is invalid."; 
	  message = oss.str();
	  return False;
	}
	Double regridCenterVel; 
	if(regridCenter > -C::c){
          // (we deal with invalid values later)
	  regridCenterF = freq_from_vopt(regridCenter,regridVeloRestfrq);
	  regridCenterVel = regridCenter;
	}
	else{ // center was not specified
	  regridCenterF = (transNewXin[0]+transNewXin[oldNUM_CHAN-1])/2.;
	  regridCenterVel = vopt(regridCenterF,regridVeloRestfrq);
	  centerIsStart = False;
	}
	if(nchan>0){
	  Double cw;
	  Double divbytwo = 0.5;
	  if(centerIsStart){
	    divbytwo = 1.;
	  }
	  if(regridChanWidth > 0.){
	    cw = regridChanWidth;
	  }
	  else{ // determine channel width from first channel
	    Double upEdge = vopt(transNewXin[0]-transCHAN_WIDTH[0],regridVeloRestfrq);
	    Double loEdge = vopt(transNewXin[0]+transCHAN_WIDTH[0],regridVeloRestfrq);
	    cw = abs(upEdge-loEdge); 
	  }
	  Double bwUpperEndF = freq_from_vopt(regridCenterVel - (Double)nchan*cw*divbytwo,
					 regridVeloRestfrq);
	  regridBandwidthF = (bwUpperEndF-regridCenterF)/divbytwo; 
	  // can convert start to center
	  if(centerIsStart){
	    regridCenterVel = regridCenterVel - (Double)nchan*cw/2.;
	    regridCenterF = freq_from_vopt(regridCenterVel,regridVeloRestfrq);
	    centerIsStart = False;
	  }
	  nchan=0; // indicate that nchan should not be used in the following
	}
	else if(regridBandwidth > 0.){
	  // can convert start to center
	  if(centerIsStart){
	    regridCenterVel = regridCenter - regridBandwidth/2.;
	    regridCenterF = freq_from_vopt(regridCenterVel,regridVeloRestfrq);
	    centerIsStart = False;
	  }
	  Double bwUpperEndF =  freq_from_vopt(regridCenterVel - regridBandwidth/2.,
                                               regridVeloRestfrq);
	  regridBandwidthF = 2.* (bwUpperEndF- regridCenterF); 
	}
	if(regridChanWidth > 0.){
	  Double chanUpperEdgeF = freq_from_vopt(regridCenterVel - regridChanWidth/2.,
                                                 regridVeloRestfrq);
	  regridChanWidthF = 2.* (chanUpperEdgeF - regridCenterF); 
	}
      } 
      else if(regridQuant=="freq"){ ////////////////////////
	if(start>=0){
	  Int firstChan = start;
	  if(start >= (Int)transNewXin.size()){
	    oss << " *** Parameter start exceeds total number of channels which is "
		<< transNewXin.size() << ". Set to 0." << endl;
	    firstChan = 0;
	  }	    
	  regridCenter = transNewXin[firstChan]-transCHAN_WIDTH[firstChan]/2.;
	  centerIsStart = True;
	}
	regridCenterF = regridCenter;
	regridBandwidthF = regridBandwidth;
	if(width>0){ // width parameter overrides regridChanWidth
	  regridChanWidth = width*transCHAN_WIDTH[0];
	}
	regridChanWidthF = regridChanWidth;
      }
      else if(regridQuant=="wave"){ ///////////////////////
	// wavelength ...
	Double regridCenterWav; 
	if(regridCenter > 0.){
	  regridCenterF = freq_from_lambda(regridCenter); 
	  regridCenterWav = regridCenter;
	}
	else{ // center was not specified
	  regridCenterF = (transNewXin[0] + transNewXin[oldNUM_CHAN-1])/2.;
	  regridCenterWav = lambda(regridCenterF);
	  centerIsStart = False;
	}
	if(nchan>0){
	  Double cw;
	  Double divbytwo = 0.5;
	  if(centerIsStart){
	    divbytwo = 1.;
	  }
	  if(regridChanWidth > 0.){
	    cw = regridChanWidth;
	  }
	  else{ // determine channel width from first channel
	    Double upEdge = lambda(transNewXin[0]-transCHAN_WIDTH[0]);
	    Double loEdge = lambda(transNewXin[0]+transCHAN_WIDTH[0]);
	    cw = abs(upEdge-loEdge); 
	  }
	  Double bwUpperEndF = freq_from_lambda(regridCenterWav - (Double)nchan*cw*divbytwo);
	  regridBandwidthF = (bwUpperEndF-regridCenterF)/divbytwo; 
	  // can convert start to center
	  if(centerIsStart){
	    regridCenterWav = regridCenterWav - (Double)nchan*cw/2.;
	    regridCenterF = freq_from_lambda(regridCenterWav);
	    centerIsStart = False;
	  }
	  nchan=0; // indicate that nchan should not be used in the following
	}
	else if(regridBandwidth > 0. && regridBandwidth/2. < regridCenterWav){
	  // can convert start to center
	  if(centerIsStart){
	    regridCenterWav = regridCenter - regridBandwidth/2.;
	    regridCenterF = freq_from_lambda(regridCenterWav);
	    centerIsStart = False;
	  }
	  Double bwUpperEndF =  lambda(regridCenterWav - regridBandwidth/2.);
	  regridBandwidthF = 2.* (bwUpperEndF - regridCenterF); 
	}
	if(regridChanWidth>0. && regridChanWidth/2.< regridCenterWav){
	  Double chanUpperEdgeF =  lambda(regridCenterWav - regridChanWidth/2.);
	  regridChanWidthF = 2.* (chanUpperEdgeF - regridCenterF); 
	}
      }
      else{
	oss << "Invalid value " << regridQuant << " for parameter \"mode\".";
	message = oss.str();
	return False;
      }
      // (transformation of regrid parameters to frequencies completed)
      
      // then determine the actually possible parameters
      Double theRegridCenterF;
      Double theRegridBWF;
      Double theCentralChanWidthF;
      
      // for vrad and vopt also need to keep this adjusted value
      Double theChanWidthX = -1.;

      if(regridCenterF < 0.){ //  means "not set"
	// keep regrid center as it is in the data
	theRegridCenterF = (transNewXin[0] + transNewXin[oldNUM_CHAN-1])/2.;
	centerIsStart = False;
      }
      else { // regridCenterF was set
	// keep center in limits
	theRegridCenterF = regridCenterF;
	if(theRegridCenterF > transNewXin[oldNUM_CHAN-1]+transCHAN_WIDTH[oldNUM_CHAN-1]/2.){
	  theRegridCenterF = transNewXin[oldNUM_CHAN-1]+transCHAN_WIDTH[oldNUM_CHAN-1]/2.;
	}
	else if(theRegridCenterF < transNewXin[0]-transCHAN_WIDTH[0]/2.){
	  theRegridCenterF = transNewXin[0]-transCHAN_WIDTH[0]/2.;
	}
      }
      if(regridBandwidthF<=0.|| nchan!=0){ // "not set" or use nchan instead
	// keep bandwidth as is
	theRegridBWF = transNewXin[oldNUM_CHAN-1] - transNewXin[0] 
	  + transCHAN_WIDTH[0]/2. + transCHAN_WIDTH[oldNUM_CHAN-1]/2.;
	if(nchan!=0){ // use nchan parameter if available
	  if(nchan<0){
	    // define via width of first channel to avoid numerical problems
	    theRegridBWF = transCHAN_WIDTH[0]*floor((theRegridBWF+transCHAN_WIDTH[0]*0.01)/transCHAN_WIDTH[0]);
	  }
	  else if(regridChanWidthF <= 0.){ // channel width not set
	    theRegridBWF = transCHAN_WIDTH[0]*nchan;
	  }
	  else{ 
	    theRegridBWF = regridChanWidthF*nchan;
	  }	    
	}
	// now can convert start to center
	if(centerIsStart){
	  theRegridCenterF = theRegridCenterF + theRegridBWF/2.;
	  centerIsStart = False;
	}
      }
      else { // regridBandwidthF was set
	// determine actually possible bandwidth:
	// width will be truncated to the maximum width possible symmetrically
	// around the value given by "regrid_center"
	theRegridBWF = regridBandwidthF;
	// now can convert start to center
	if(centerIsStart){
	  theRegridCenterF = theRegridCenterF + theRegridBWF/2.;
	  centerIsStart = False;
	}
	if(theRegridCenterF + theRegridBWF / 2. >
           transNewXin[oldNUM_CHAN-1] + transCHAN_WIDTH[oldNUM_CHAN-1]/2.){
	  theRegridBWF = (transNewXin[oldNUM_CHAN-1] +
                          transCHAN_WIDTH[oldNUM_CHAN-1]/2. - theRegridCenterF)*2.;
	  oss << " *** Defined new spectral window exceeds upper end of original window." << endl
	      << " Shrinking it to maximum possible size keeping defined center (may reduce number of channels)." << endl;
	}
	if(theRegridCenterF - theRegridBWF/2. < transNewXin[0] - transCHAN_WIDTH[0]/2.){
	  theRegridBWF = (theRegridCenterF - transNewXin[0] + transCHAN_WIDTH[0]/2.)*2.;
	  oss << " *** Defined new spectral window exceeds lower end of original window." << endl
	      << " Shrinking it to maximum possible size keeping defined center (may reduce number of channels)." << endl;
	}
      }
      if(regridChanWidthF <= 0.){ // "not set"
	if(nchan!=0 || centerIsStartC){ // use first channel
	  theCentralChanWidthF = transCHAN_WIDTH[0];
	}
	else{
	  // keep channel width similar to the old one 
	  theCentralChanWidthF = transCHAN_WIDTH[oldNUM_CHAN/2]; // use channel width from
	                                                         // near central channel
	}
      }
      else { // regridChanWidthF was set
	// keep in limits
	theCentralChanWidthF = regridChanWidthF;
	if(theCentralChanWidthF>theRegridBWF){ // too large => make a single channel
	  theCentralChanWidthF = theRegridBWF;
	  oss << " *** Requested new channel width exceeds defined SPW width. Shrinking to maximum size." << endl;
	}
	else{ // check if too small
	  // determine smallest channel width
	  Double smallestChanWidth = 1E30;
	  for(Int i=0; i<oldNUM_CHAN; i++){
	    if(transCHAN_WIDTH[i] < smallestChanWidth){ 
	      smallestChanWidth = transCHAN_WIDTH[i];
	    }
	  }
	  if(theCentralChanWidthF < smallestChanWidth){
	    // too small => make as small as the smallest channel
	    theCentralChanWidthF = smallestChanWidth;
	    oss << " *** Requested new channel width is smaller than smallest original channel width." << endl
		<< " Increasing channel width to minimum possible size." << endl;
	  }
	  else { // input channel width was OK, memorize
	    theChanWidthX = regridChanWidth;
	  }
	}   	    
      }
      oss << " Channels equidistant in " << regridQuant << endl
	  << " Central frequency (in output frame) = " << theRegridCenterF
          << " Hz" << endl 
	  << " Width of central channel (in output frame) = "
          << theCentralChanWidthF << " Hz" << endl;
      
      // now calculate newChanLoBound, and newChanHiBound from
      // theRegridCenterF, theRegridBWF, theCentralChanWidthF
      vector<Double> loFBup; // the lower bounds for the new channels 
                             // starting from the central channel going up
      vector<Double> hiFBup; // the lower bounds for the new channels 
	                     // starting from the central channel going up
      vector<Double> loFBdown; // the lower bounds for the new channels
                               // starting from the central channel going down
      vector<Double> hiFBdown; // the lower bounds for the new channels
                               // starting from the central channel going down
      
      Double edgeTolerance = theCentralChanWidthF*0.01; // needed to avoid numerical accuracy problems

      if(regridQuant=="vrad"){
	// regridding in radio velocity ...
	
	// create freq boundaries equidistant and contiguous in radio velocity
	Double upperEndF = theRegridCenterF + theRegridBWF/2.;
	Double lowerEndF = theRegridCenterF - theRegridBWF/2.;
	Double upperEndV = vrad(upperEndF,regridVeloRestfrq);
	Double lowerEndV = vrad(lowerEndF,regridVeloRestfrq);
	Double velLo;
	Double velHi;


	//    Want to keep the center of the center channel at the center of
	//    the new center channel if the bandwidth is an odd multiple of the
        //    new channel width,
	//    otherwise the center channel is the lower edge of the new center channel
	Double tnumChan = floor((theRegridBWF+edgeTolerance)/theCentralChanWidthF);
	if((tnumChan/2. - (Int)tnumChan/2)>0.1){
          // odd multiple 
	  loFBup.push_back(theRegridCenterF-theCentralChanWidthF/2.);
	  hiFBup.push_back(theRegridCenterF+theCentralChanWidthF/2.);
	  loFBdown.push_back(theRegridCenterF-theCentralChanWidthF/2.);
	  hiFBdown.push_back(theRegridCenterF+theCentralChanWidthF/2.);
	}
	else{
	  loFBup.push_back(theRegridCenterF);
	  hiFBup.push_back(theRegridCenterF+theCentralChanWidthF);
	  loFBdown.push_back(theRegridCenterF);
	  hiFBdown.push_back(theRegridCenterF+theCentralChanWidthF);
	}

	if(theChanWidthX<0){ // cannot use original channel width in velocity units
	  // need to calculate back from central channel width in Hz
	  theChanWidthX = vrad(loFBup[0],
                               regridVeloRestfrq) - vrad(hiFBup[0],
                                                         regridVeloRestfrq);
	}
	// calc velocity corresponding to the upper end (in freq) of the last
	// added channel which is the lower end of the next channel
	velLo = vrad(hiFBup[0],regridVeloRestfrq);
	// calc velocity corresponding to the upper end (in freq) of the next channel
	velHi = velLo - theChanWidthX; // vrad goes down as freq goes up!
	while(upperEndV - theChanWidthX/10. < velHi){ // (preventing accuracy problems)
	  // calc frequency of the upper end (in freq) of the next channel
	  Double freqHi = freq_from_vrad(velHi,regridVeloRestfrq);
	  if(freqHi<=upperEndF+edgeTolerance){ // end of bandwidth not yet reached
	    loFBup.push_back(hiFBup.back());
	    hiFBup.push_back(freqHi);
	  }
	  else if(freqHi<upperEndF+edgeTolerance){ 
	    loFBup.push_back(hiFBup.back());
	    hiFBup.push_back(upperEndF);
	    break;
	  }
	  else{
	    break;
	  }
	  // calc velocity corresponding to the upper end (in freq) of the added channel
	  velLo = vrad(hiFBup.back(),regridVeloRestfrq);
	  // calc velocity corresponding to the upper end (in freq) of the next channel
	  velHi = velLo - theChanWidthX; // vrad goes down as freq goes up
	}

	// calc velocity corresponding to the lower end (in freq) of the last
	// added channel which is the upper end of the next channel
	velHi = vrad(loFBdown[0],regridVeloRestfrq);
	// calc velocity corresponding to the lower end (in freq) of the next channel
	velLo = velHi + theChanWidthX; // vrad goes up as freq goes down!
	while(velLo < lowerEndV + theChanWidthX/10.){ // (preventing accuracy problems)  
	  // calc frequency of the lower end (in freq) of the next channel
	  Double freqLo = freq_from_vrad(velLo,regridVeloRestfrq);
	  if(freqLo>=lowerEndF-edgeTolerance){ // end of bandwidth not yet reached
	    hiFBdown.push_back(loFBdown.back());
	    loFBdown.push_back(freqLo);
	  }
	  else if(freqLo>lowerEndF-edgeTolerance){ 
	    hiFBdown.push_back(loFBdown.back());
	    loFBdown.push_back(lowerEndF);
	    break;
	  }
	  else {
	    break;
	  }
	  // calc velocity corresponding to the upper end of the next channel
	  velHi = vrad(loFBdown.back(),regridVeloRestfrq);
	  // calc velocity corresponding to the lower end (in freq) of the next channel
	  velLo = velHi + theChanWidthX; // vrad goes up as freq goes down
	}	  
      }
      else if(regridQuant=="vopt"){
	// regridding in optical velocity ...
	
	// create freq boundaries equidistant and contiguous in optical velocity
	Double upperEndF = theRegridCenterF + theRegridBWF/2.;
	Double lowerEndF = theRegridCenterF - theRegridBWF/2.;
	Double upperEndV = vopt(upperEndF,regridVeloRestfrq);
	Double lowerEndV = vopt(lowerEndF,regridVeloRestfrq);
	Double velLo;
	Double velHi;

	//    Want to keep the center of the center channel at the center of
	//    the new center channel if the bandwidth is an odd multiple of the
	//    new channel width,
	//    otherwise the center channel is the lower edge of the new center
	//    channel
	Double tnumChan = floor((theRegridBWF+edgeTolerance)/theCentralChanWidthF);
	if((tnumChan/2. - (Int)tnumChan/2)>0.1){
          // odd multiple 
	  loFBup.push_back(theRegridCenterF-theCentralChanWidthF/2.);
	  hiFBup.push_back(theRegridCenterF+theCentralChanWidthF/2.);
	  loFBdown.push_back(theRegridCenterF-theCentralChanWidthF/2.);
	  hiFBdown.push_back(theRegridCenterF+theCentralChanWidthF/2.);
	}
	else{
	  loFBup.push_back(theRegridCenterF);
	  hiFBup.push_back(theRegridCenterF+theCentralChanWidthF);
	  loFBdown.push_back(theRegridCenterF);
	  hiFBdown.push_back(theRegridCenterF+theCentralChanWidthF);
	}

	if(theChanWidthX<0){ // cannot use original channel width in velocity units
	  // need to calculate back from central channel width in Hz
	  theChanWidthX = vopt(loFBup[0],
                               regridVeloRestfrq) - vopt(hiFBup[0],regridVeloRestfrq);
	}
	// calc velocity corresponding to the upper end (in freq) of the last
	// added channel which is the lower end of the next channel
	velLo = vopt(hiFBup[0],regridVeloRestfrq);
	// calc velocity corresponding to the upper end (in freq) of the next channel
	velHi = velLo - theChanWidthX; // vopt goes down as freq goes up!
	while(upperEndV - velHi < theChanWidthX/10.){ // (preventing accuracy problems)
	  // calc frequency of the upper end (in freq) of the next channel
	  Double freqHi = freq_from_vopt(velHi,regridVeloRestfrq);
	  if(freqHi<=upperEndF+edgeTolerance){ // end of bandwidth not yet reached
	    loFBup.push_back(hiFBup.back());
	    hiFBup.push_back(freqHi);
	  }
	  else if(freqHi<upperEndF+edgeTolerance){ 
	    loFBup.push_back(hiFBup.back());
	    hiFBup.push_back(upperEndF);
	    break;
	  }
	  else{
	    break;
	  }
	  // calc velocity corresponding to the upper end (in freq) of the added channel
	  velLo = vopt(hiFBup.back(),regridVeloRestfrq);
	  // calc velocity corresponding to the upper end (in freq) of the next channel
	  velHi = velLo - theChanWidthX; // vopt goes down as freq goes up
	}

	// calc velocity corresponding to the lower end (in freq) of the last
	// added channel which is the upper end of the next channel
	velHi = vopt(loFBdown[0],regridVeloRestfrq);
	// calc velocity corresponding to the lower end (in freq) of the next channel
	velLo = velHi + theChanWidthX; // vopt goes up as freq goes down!
	while(velLo - lowerEndV < theChanWidthX/10.){ // (preventing accuracy problems)  
	  // calc frequency of the lower end (in freq) of the next channel
	  Double freqLo = freq_from_vopt(velLo,regridVeloRestfrq);
	  if(freqLo>=lowerEndF-edgeTolerance){ // end of bandwidth not yet reached
	    hiFBdown.push_back(loFBdown.back());
	    loFBdown.push_back(freqLo);
	  }
	  else if(freqLo>lowerEndF-edgeTolerance){ 
	    hiFBdown.push_back(loFBdown.back());
	    loFBdown.push_back(lowerEndF);
	    break;
	  }
	  else {
	    break;
	  }
	  // calc velocity corresponding to the upper end of the next channel
	  velHi = vopt(loFBdown.back(),regridVeloRestfrq);
	  // calc velocity corresponding to the lower end (in freq) of the next channel
	  velLo = velHi + theChanWidthX; // vopt goes up as freq goes down
	}	  
      }
      else if(regridQuant=="freq"){
	// regridding in frequency  ...
	
	// create freq boundaries equidistant and contiguous in frequency
	Double upperEndF = theRegridCenterF + theRegridBWF/2.;
	Double lowerEndF = theRegridCenterF - theRegridBWF/2.;

	//    Want to keep the center of the center channel at the center of
	//    the new center channel if the bandwidth is an odd multiple of the
        //    new channel width, 
	//    otherwise the center channel is the lower edge of the new center channel
	Double tnumChan = floor((theRegridBWF+edgeTolerance)/theCentralChanWidthF);
	if((tnumChan/2. - (Int)tnumChan/2)>0.1){
          // odd multiple 
	  loFBup.push_back(theRegridCenterF-theCentralChanWidthF/2.);
	  hiFBup.push_back(theRegridCenterF+theCentralChanWidthF/2.);
	  loFBdown.push_back(theRegridCenterF-theCentralChanWidthF/2.);
	  hiFBdown.push_back(theRegridCenterF+theCentralChanWidthF/2.);
	}
	else{
	  loFBup.push_back(theRegridCenterF);
	  hiFBup.push_back(theRegridCenterF+theCentralChanWidthF);
	  loFBdown.push_back(theRegridCenterF);
	  hiFBdown.push_back(theRegridCenterF+theCentralChanWidthF);
	}

	while(hiFBup.back()< upperEndF+edgeTolerance){
	  // calc frequency of the upper end of the next channel
	  Double freqHi = hiFBup.back() + theCentralChanWidthF;
	  if(freqHi<=upperEndF+edgeTolerance){ // end of bandwidth not yet reached
	    loFBup.push_back(hiFBup.back());
	    hiFBup.push_back(freqHi);
	  }
	  else{
	    break;
	  }
	}

	while(loFBdown.back() > lowerEndF-edgeTolerance){
	  // calc frequency of the lower end of the next channel
	  Double freqLo = loFBdown.back() - theCentralChanWidthF;
	  if(freqLo>=lowerEndF-edgeTolerance){ // end of bandwidth not yet reached
	    hiFBdown.push_back(loFBdown.back());
	    loFBdown.push_back(freqLo);
	  }
	  else {
	    break;
	  }
	}	  
      }
      else if(regridQuant=="wave"){
	// regridding in wavelength  ...
	
	// create freq boundaries equidistant and contiguous in wavelength
	Double upperEndF = theRegridCenterF + theRegridBWF/2.;
	Double lowerEndF = theRegridCenterF - theRegridBWF/2.;
	Double upperEndL = lambda(upperEndF);
	Double lowerEndL = lambda(lowerEndF);
	Double lambdaLo;
	Double lambdaHi;

	//    Want to keep the center of the center channel at the center of
	//    the new center channel if the bandwidth is an odd multiple of the
	//    new channel width, 
	//    otherwise the center channel is the lower edge of the new center
	//    channel
	Double tnumChan = floor((theRegridBWF+edgeTolerance)/theCentralChanWidthF);
	if((tnumChan/2. - (Int)tnumChan/2)>0.1){
          // odd multiple 
	  loFBup.push_back(theRegridCenterF-theCentralChanWidthF/2.);
	  hiFBup.push_back(theRegridCenterF+theCentralChanWidthF/2.);
	  loFBdown.push_back(theRegridCenterF-theCentralChanWidthF/2.);
	  hiFBdown.push_back(theRegridCenterF+theCentralChanWidthF/2.);
	}
	else{
	  loFBup.push_back(theRegridCenterF);
	  hiFBup.push_back(theRegridCenterF+theCentralChanWidthF);
	  loFBdown.push_back(theRegridCenterF);
	  hiFBdown.push_back(theRegridCenterF+theCentralChanWidthF);
	}

	if(theChanWidthX<0){ // cannot use original channel width in wavelength units
	  // need to calculate back from central channel width in Hz
	  theChanWidthX = lambda(loFBup[0]) - lambda(hiFBup[0]);
	}
	// calc wavelength corresponding to the upper end (in freq) of the last
	// added channel which is the lower end of the next channel
	lambdaLo = lambda(hiFBup[0]);
	// calc wavelength corresponding to the upper end (in freq) of the next channel
	lambdaHi = lambdaLo - theChanWidthX; // lambda goes down as freq goes up!
	while(upperEndL - lambdaHi < theChanWidthX/10.){ // (preventing accuracy problems)
	  // calc frequency of the upper end (in freq) of the next channel
	  Double freqHi = freq_from_lambda(lambdaHi);
	  if(freqHi<=upperEndF+edgeTolerance){ // end of bandwidth not yet reached
	    loFBup.push_back(hiFBup.back());
	    hiFBup.push_back(freqHi);
	  }
	  else if(freqHi<upperEndF+edgeTolerance){ 
	    loFBup.push_back(hiFBup.back());
	    hiFBup.push_back(upperEndF);
	    break;
	  }
	  else{
	    break;
	  }
	  // calc wavelength corresponding to the upper end (in freq) of the
	  // added channel
	  lambdaLo = lambda(hiFBup.back());
	  // calc wavelength corresponding to the upper end (in freq) of the next channel
	  lambdaHi = lambdaLo - theChanWidthX; // lambda goes down as freq goes up
	}

	// calc wavelength corresponding to the lower end (in freq) of the last
	// added channel which is the upper end of the next channel
	lambdaHi = lambda(loFBdown[0]);
	// calc wavelength corresponding to the lower end (in freq) of the next channel
	lambdaLo = lambdaHi + theChanWidthX; // lambda goes up as freq goes down!
	while(lambdaLo - lowerEndL < theChanWidthX/10.){  // (preventing accuracy problems) 
	  // calc frequency of the lower end (in freq) of the next channel
	  Double freqLo = freq_from_lambda(lambdaLo);
	  if(freqLo>=lowerEndF-edgeTolerance){ // end of bandwidth not yet reached
	    hiFBdown.push_back(loFBdown.back());
	    loFBdown.push_back(freqLo);
	  }
	  else if(freqLo>lowerEndF-edgeTolerance){ 
	    hiFBdown.push_back(loFBdown.back());
	    loFBdown.push_back(lowerEndF);
	    break;
	  }
	  else {
	    break;
	  }
	  // calc wavelength corresponding to the upper end of the next channel
	  lambdaHi = lambda(loFBdown.back());
	  // calc wavelength corresponding to the lower end (in freq) of the next channel
	  lambdaLo = lambdaHi + theChanWidthX; // wavelength goes up as freq goes down
	}	  

      }
      else{ // should not get here
	oss << "Invalid value " << regridQuant << " for parameter \"mode\".";
	message = oss.str();
	return False;
      }

      Int numNewChanDown = loFBdown.size();
      Int numNewChanUp = loFBup.size();

      // central channel contained in both vectors
      newChanLoBound.resize(numNewChanDown+numNewChanUp - 1);

      newChanHiBound.resize(numNewChanDown+numNewChanUp - 1);
      for(Int i=0; i<numNewChanDown; i++){ 
	Int k = numNewChanDown-i-1; // need to assign in reverse
	newChanLoBound[i] = loFBdown[k];
	newChanHiBound[i] = hiFBdown[k];
      }
      for(Int i=1; i<numNewChanUp; i++){ // start at 1 to omit the central channel here
	newChanLoBound[i+numNewChanDown-1] = loFBup[i];
	newChanHiBound[i+numNewChanDown-1] = hiFBup[i];
      }
      
      uInt nc = newChanLoBound.size();
      oss << " Number of channels = " << nc << endl;
      oss << " Total width of SPW (in output frame) = " << newChanHiBound[nc-1] - newChanLoBound[0] 
	  << " Hz" << endl;
      oss << " Lower edge = " << newChanLoBound[0] << " Hz,"
	  << " upper edge = " << newChanHiBound[nc-1] << " Hz" << endl;

      message = oss.str();
      return True;
      
    } // end if (regridQuant=="chan")

  }

  Bool SubMS::setRegridParameters(vector<Int>& oldSpwId,
				  vector<Int>& oldFieldId,
				  vector<Int>& newDataDescId,
				  vector<Bool>& regrid,
				  vector<Bool>& transform,
				  vector<MDirection>& theFieldDirV,
				  vector<MPosition>& mObsPosV,
				  vector<MFrequency::Types>& fromFrameTypeV,
				  vector<MFrequency::Ref>& outFrameV,
				  vector< Vector<Double> >& xold, 
				  vector< Vector<Double> >& xout, 
				  vector< Vector<Double> >& xin, 
				  // This is a temporary fix until
				  // InterpolateArray1D<Double, Complex>&
				  // works.
				  vector<InterpolateArray1D<Float, Complex>::InterpolationMethod >& method,
				  vector<InterpolateArray1D<Double, Float>::InterpolationMethod >& methodF,
				  Bool& msModified,
				  const String& outframe,
				  const String& regridQuant,
				  const Double regridVeloRestfrq,
				  const String& regridInterpMeth,
				  const Double regridCenter, 
				  const Double regridBandwidth, 
				  const Double regridChanWidth,
				  const Int regridPhaseCenterFieldId,
				  const MDirection regridPhaseCenter,
				  const Bool writeTables,
				  LogIO& os,
				  String& regridMessage,
				  const Bool centerIsStart,
				  const Int nchan,
				  const Int width,
				  const Int start
				  )
  {
    Bool rval = True;

    // reset the "done" table.
    newDataDescId.resize(0);
    oldSpwId.resize(0);
    oldFieldId.resize(0);
    xold.resize(0);
    xin.resize(0);
    xout.resize(0);
    theFieldDirV.resize(0);
    mObsPosV.resize(0);
    fromFrameTypeV.resize(0);
    outFrameV.resize(0);
    MFrequency::Ref outFrame;
    method.resize(0);
    methodF.resize(0);
    regrid.resize(0);	
    transform.resize(0);	
    
    // Determine the highest data_desc_id from the DATA_DESCRIPTION table
    // (= number of rows).
    MSDataDescription ddtable = ms_p.dataDescription();
    Int origNumDataDescs = ddtable.nrow();
    Int nextDataDescId = origNumDataDescs - 1;
    Int numNewDataDesc = 0;

    // Determine the highest spw_id in the SPW table
    MSSpectralWindow spwtable = ms_p.spectralWindow();
    Int origNumSPWs = spwtable.nrow();
    Int nextSPWId = origNumSPWs - 1;
    Int numNewSPWIds = 0;

    // Determine the highest row number in the SOURCE table
    Int origNumSourceRows = 0;
    Int nextSourceRow = -1;
    Int numNewSourceRows = 0;
    MSSource* p_sourcetable = NULL;
    MSSourceColumns* p_sourceCol = NULL;
    if(Table::isReadable(ms_p.sourceTableName())){
      p_sourcetable = &(ms_p.source());
      p_sourceCol = new MSSourceColumns(*p_sourcetable);
      origNumSourceRows = p_sourcetable->nrow();
      nextSourceRow = origNumSourceRows - 1;
    }
    else if(!writeTables) { // there is no source table
      os << LogIO::NORMAL << "Note: MS contains no SOURCE table ..." << LogIO::POST;
      nextSourceRow = -1;
    }

    MSMainColumns mainCols(ms_p);
    ScalarColumn<Int> fieldIdCol = mainCols.fieldId();
    ScalarColumn<Int> DDIdCol = mainCols.dataDescId();
    ScalarMeasColumn<MEpoch> mainTimeMeasCol = mainCols.timeMeas();
    // other administrational tables

    MSDataDescColumns DDCols(ddtable);
    ScalarColumn<Int> SPWIdCol = DDCols.spectralWindowId(); 

    MSSpWindowColumns SPWCols(spwtable);
    ScalarColumn<Int> numChanCol = SPWCols.numChan(); 
    ArrayColumn<Double> chanFreqCol = SPWCols.chanFreq(); 
    ArrayMeasColumn<MFrequency> chanFreqMeasCol = SPWCols.chanFreqMeas();
    ScalarColumn<Int> measFreqRefCol = SPWCols.measFreqRef();
    ArrayColumn<Double> chanWidthCol = SPWCols.chanWidth(); 
    ArrayColumn<Double> effectiveBWCol = SPWCols.effectiveBW();   
    ScalarColumn<Double> refFrequencyCol = SPWCols.refFrequency(); 
    ScalarMeasColumn<MFrequency> refFrequencyMeasCol = SPWCols.refFrequencyMeas(); 
    ArrayColumn<Double> resolutionCol = SPWCols.resolution(); 
    ScalarColumn<Double> totalBandwidthCol = SPWCols.totalBandwidth();

    MSField fieldtable = ms_p.field();
    MSFieldColumns FIELDCols(fieldtable);
    //ArrayMeasColumn<MDirection> referenceDirMeasCol = FIELDCols.referenceDirMeasCol(); 
    //ScalarMeasColumn<MEpoch>& timeMeasCol = FIELDCols.timeMeas();
    ScalarColumn<Int> FIELDsourceIdCol = FIELDCols.sourceId(); 

    // calculate mean antenna position for TOPO transformation
    MSAntenna anttable = ms_p.antenna();
    ROMSAntennaColumns ANTCols(anttable);
    ROScalarMeasColumn<MPosition> ANTPositionMeasCol = ANTCols.positionMeas(); 
    ROScalarColumn<Bool> ANTflagRowCol = ANTCols.flagRow();
    Int nAnt = 0;
    Vector<Double> pos(3); pos=0;
    for (uInt i=0; i<anttable.nrow(); i++) {
      if(!ANTflagRowCol(i)){
	pos+=ANTPositionMeasCol(i).getValue().get();
	nAnt++;
      }
    }
    if(nAnt>0){
      pos /= Double(nAnt);
    }
    else {
      os << LogIO::WARN << "No unflagged antennas in this MS. Cannot proceed with regridSpw ..." 
	 << LogIO::POST;
      return rval; 
    }

//     if(!writeTables){
//       cout << "Verifying parameters and MS ..." << endl;
//     }
//     else {
//       cout << "Writing modified spectral window parameters ..." << endl;
//     }

    MPosition mObsPos = ANTPositionMeasCol(0); // transfer reference frame
    mObsPos.set(MVPosition(pos)); // set coordinates
    
    for(uInt mainTabRow=0; mainTabRow<ms_p.nrow(); mainTabRow++){
    
      // For each MAIN table row, the FIELD_ID cell and the DATA_DESC_ID cell are read 
      Int theFieldId = fieldIdCol(mainTabRow);
      Int theDataDescId = DDIdCol(mainTabRow);
      // and the SPW_ID extracted from the corresponding row in the
      // DATA_DESCRIPTION table.
      Int theSPWId = -2;
      if (theDataDescId < origNumDataDescs){
	theSPWId = SPWIdCol(theDataDescId);
      }
      else {
	os << LogIO::SEVERE
           << "Incoherent MS: Found at main table row " << mainTabRow
	   << " reference to non-existing DATA_DESCRIPTION table entry "
           << theDataDescId
	   << LogIO::POST;
	rval = False;
	return rval;
      }

      // variables saying what has to be done for this row
      Bool needTransform = False;
      Bool doRegrid = False;
      Int equivalentSpwFieldPair = -1;

      String message;

      //  The pair (theFieldId, theSPWId) is looked up in the "done table". 
      Int iDone = -1;
      for (uInt i=0; i<oldSpwId.size(); i++){
	if(oldSpwId[i]==theSPWId && (oldFieldId[i]==theFieldId || regridPhaseCenterFieldId>=-1)){ 
	  // if common phase center is given, treat all fields the same
	  iDone = i;
	  break;
	}
      }

      if(iDone<0){ // this (theFieldId, theSPWId) pair was not yet encountered 

	// Determine information for new row in "done" table
	//   The information necessary for the transformation is extracted:  
	//   1) center frequency of each channel (taken from the CHAN_FREQ cell
	//      corresponding to theSPWId in the SPW table)
	Vector<Double> newXin;
	newXin.assign(chanFreqCol(theSPWId));
	//      -> store in  xin (further below)
	//   2) reference frame for these frequencies (taken from the
	//      MEAS_FREQ_REF cell corresponding to theSPWId in the SPW table)
	MFrequency::Types theOldRefFrame = MFrequency::castType(measFreqRefCol(theSPWId));
	//      -> store in oldRefFrame[numNewDataDesc] (further below)
	//   3) in case either the original or the destination reference frame
	//      is TOPO or GEO, we need the observation time
	//      (taken from the time of the first integration for this (theFieldId, theSPWId) pair)
	//      -> store in obsTime[numNewDataDesc] (further below)
       	MEpoch theObsTime = mainTimeMeasCol(mainTabRow);
	//   4) direction of the field, i.e. the phase center
	MDirection theFieldDir;
	if(regridPhaseCenterFieldId<-1){ // take it from the PHASE_DIR cell
	                                 // corresponding to theFieldId in the FIELD table)
	  theFieldDir = FIELDCols.phaseDirMeas(theFieldId, 0);
	}
	else if(regridPhaseCenterFieldId==-1){ // use the given direction
	  theFieldDir = regridPhaseCenter;
	}
	else if((uInt)regridPhaseCenterFieldId < fieldtable.nrow()){ // use this valid field ID
	  theFieldDir = FIELDCols.phaseDirMeas(regridPhaseCenterFieldId, 0);
	}
	else{
	  os << LogIO::SEVERE << "Field to be used as phase center, id " 
	     << regridPhaseCenterFieldId 
	     << ", does not exist." << LogIO::POST;
	  return False;
	}
	//	cout << "theFieldId = " << theFieldId << ", theObsTime = " << theObsTime
	//	     << ", theFieldDir = " << theFieldDir.getAngle() << endl;

	//      -> store in fieldDir[numNewDataDesc] (further below)
	//   5) in case either the original or the destination reference frame
	//      (but not both) are TOPO, we need the observatory position
	//      (from the mean antenna position calculated above) 
	//      -> store in obsPos[numNewDataDesc] (further below)

	// Determine if a reference frame transformation is necessary
	// Bool 	getType (MFrequency::Types &tp, const String &in)
	needTransform = True;
	MFrequency::Types theFrame;
	if(outframe==""){ // no ref frame given 
	  // keep the reference frame as is
	  theFrame = theOldRefFrame;
	  needTransform = False;
	}
	else if(!MFrequency::getType(theFrame, outframe)){
	  os << LogIO::SEVERE
             << "Parameter \"outframe\" value " << outframe << " is invalid." 
	     << LogIO::POST;
	  return False;
	}
	else if (theFrame == theOldRefFrame){
	  needTransform = False;
	}

	// Perform the pure frequency transformation (no regridding yet)
	Vector<Double> transNewXin;
	// also take care of the other parameters of the spectral window
	Int oldNUM_CHAN = numChanCol(theSPWId); 
	Vector<Double> oldCHAN_WIDTH = chanWidthCol(theSPWId);
	MFrequency oldREF_FREQUENCY = refFrequencyMeasCol(theSPWId);
	Double oldTOTAL_BANDWIDTH = totalBandwidthCol(theSPWId);
	Vector<Double> oldEFFECTIVE_BW = effectiveBWCol(theSPWId);   
	Vector<Double> oldRESOLUTION = resolutionCol(theSPWId);

	// storage for values with pure freq trafo applied
	Vector<Double> transCHAN_WIDTH(oldNUM_CHAN);
	MFrequency transREF_FREQUENCY;
	Double transTOTAL_BANDWIDTH;
	Vector<Double> transRESOLUTION(oldNUM_CHAN);;

	if(needTransform){

	  transNewXin.resize(oldNUM_CHAN);
	  // set up conversion
	  Unit unit(String("Hz"));
	  MFrequency::Ref fromFrame = MFrequency::Ref(theOldRefFrame, MeasFrame(theFieldDir, mObsPos, theObsTime));
	  MFrequency::Ref toFrame = MFrequency::Ref(theFrame, MeasFrame(theFieldDir, mObsPos, theObsTime));
	  MFrequency::Convert freqTrans(unit, fromFrame, toFrame);
	  
	  // also create the reference for storage in the "Done" table
	  outFrame = MFrequency::Ref(theFrame, MeasFrame(theFieldDir, mObsPos, theObsTime));

	  for(Int i=0; i<oldNUM_CHAN; i++){
	    transNewXin[i] = freqTrans(newXin[i]).get(unit).getValue();
	    transCHAN_WIDTH[i] = freqTrans(newXin[i] +
					   oldCHAN_WIDTH[i]/2.).get(unit).getValue()
	      - freqTrans(newXin[i] -
                          oldCHAN_WIDTH[i]/2.).get(unit).getValue(); // eliminate possible offsets
	    transRESOLUTION[i] = freqTrans(newXin[i] +
                                           oldRESOLUTION[i]/2.).get(unit).getValue() 
	      - freqTrans(newXin[i]
                          - oldRESOLUTION[i] / 2.0).get(unit).getValue(); // eliminate possible offsets
	  }
	  transREF_FREQUENCY = freqTrans(oldREF_FREQUENCY);
	  transTOTAL_BANDWIDTH = transNewXin[oldNUM_CHAN-1] +
            transCHAN_WIDTH[oldNUM_CHAN-1] - transNewXin[0] + transCHAN_WIDTH[0];
	}
	else {
	  // just copy
	  transNewXin.assign(newXin);
	  transCHAN_WIDTH.assign(oldCHAN_WIDTH);
	  transRESOLUTION.assign(oldRESOLUTION);
	  transREF_FREQUENCY = oldREF_FREQUENCY;
	  transTOTAL_BANDWIDTH = oldTOTAL_BANDWIDTH;
	}

	// (reference frame transformation completed)


	// storage for values with complete freq trafo + regridding applied
	// (set to default values for the case of no regridding)
	Vector<Double> newXout;
	newXout.assign(transNewXin);
	Int newNUM_CHAN = oldNUM_CHAN;
	Vector<Double> newCHAN_WIDTH;
	newCHAN_WIDTH.assign(transCHAN_WIDTH);
	MFrequency newREF_FREQUENCY = transREF_FREQUENCY;
	Vector<Double> newRESOLUTION;
	newRESOLUTION.assign(transRESOLUTION);
	Double newTOTAL_BANDWIDTH = transTOTAL_BANDWIDTH;
	Vector<Double> newEFFECTIVE_BW;
	newEFFECTIVE_BW.assign(oldEFFECTIVE_BW);
	//	InterpolateArray1D<Double,Complex>::InterpolationMethod theMethod;
	// This is a temporary fix until InterpolateArray1D<Double, Complex> works.
	InterpolateArray1D<Float,Complex>::InterpolationMethod theMethod;
	InterpolateArray1D<Double,Float>::InterpolationMethod theMethodF;

	// check if theSPWId was already handled
	Int iDone2 = -1;
	for (uInt i=0; i<oldSpwId.size(); i++){
	  if(oldSpwId[i]==theSPWId){
	    iDone2 = i;
	    break;
	  }
	}
	equivalentSpwFieldPair = iDone2;

	if(equivalentSpwFieldPair>=0 && !needTransform){ 
          // a transformation was not needed, 
          // i.e. the operation on this SPW is independent of the FIELD
	  // and (since equivalentSpwFieldPair>=0) this SPW was already processed
	  // so we can reuse a previous SpwFieldPair

	  // get the parameters from the "done table"
	  theSPWId = oldSpwId[equivalentSpwFieldPair];
	  // don't get the field id!
	  theDataDescId = newDataDescId[equivalentSpwFieldPair] + origNumDataDescs; 
	  transNewXin.assign(xin[equivalentSpwFieldPair]);
	  newXout.assign(xout[equivalentSpwFieldPair]);
	  theMethod = method[equivalentSpwFieldPair];
	  theMethodF = methodF[equivalentSpwFieldPair];
	  doRegrid = regrid[equivalentSpwFieldPair];

	}
	else {
	  // Determine if regridding is necessary and set the parameters (at
	  // the same time, determine if the transformation is non-linear. If
	  // so set trafoNonlin (further below).)
	  
	  String methodName;
	  
	  if(regridQuant=="" ||
	     (regridCenter<-1E30 &&  regridBandwidth <= 0. && regridChanWidth <= 1. 
	      && regridQuant=="chan")
	     ){
	    // No regridding will take place.
	    // Set the interpol methods to some dummy value
	    //	  theMethod = InterpolateArray1D<Double,Complex>::linear;
	    // This is a temporary fix until InterpolateArray1D<Double, Complex> works.
	    theMethod = InterpolateArray1D<Float,Complex>::linear;
	    theMethodF = InterpolateArray1D<Double,Float>::linear;
	    methodName = "linear";
	    message = " output frame = " + MFrequency::showType(theFrame) + " (pure transformation of the channel definition)";
	    // cout <<  regridQuant << " " << regridCenter << " " << regridBandwidth << " " << regridChanWidth << endl;
	  }
	  else { // a regrid quantity was chosen
	    // determine interpolation method (this is common for all possible
	    // values of regridQuant)
	    String meth=regridInterpMeth;
	    meth.downcase();
	    if(meth.contains("nearest")){
	      //	    theMethod = InterpolateArray1D<Double,Complex>::linear;
	      // This is a temporary fix until InterpolateArray1D<Double, Complex> works.
	      theMethod = InterpolateArray1D<Float,Complex>::nearestNeighbour;
	      theMethodF = InterpolateArray1D<Double,Float>::nearestNeighbour;
	      methodName = "nearestNeighbour";
	    }
	    else if(meth.contains("splin")){
	      //	    theMethod = InterpolateArray1D<Double,Complex>::spline;
	      theMethod = InterpolateArray1D<Float,Complex>::spline;
	      theMethodF = InterpolateArray1D<Double,Float>::spline;
	      methodName = "spline";
	    }	    
	    else if(meth.contains("cub")){
	      //	    theMethod = InterpolateArray1D<Double,Complex>::cubic;
	      theMethod = InterpolateArray1D<Float,Complex>::cubic;
	      theMethodF = InterpolateArray1D<Double,Float>::cubic;
	      methodName = "cubic spline";
	    }
	    else {
	      if(!meth.contains("linear") && meth!=""){
		os << LogIO::WARN
                   << "Parameter \"interpolation\" value \"" << meth << "\" is invalid." 
		   << LogIO::POST;
		return False;
	      }
	      // theMethod = InterpolateArray1D<Double,Complex>::linear;
	      // This is a temporary fix until InterpolateArray1D<Double, Complex> works.
	      theMethod = InterpolateArray1D<Float,Complex>::linear;
	      theMethodF = InterpolateArray1D<Double,Float>::linear;
	      methodName = "linear";
	    }
	    
	    Vector<Double> newChanLoBound; 
	    Vector<Double> newChanHiBound;
	    
	    if(!regridChanBounds(newChanLoBound, 
				 newChanHiBound,
				 regridCenter,  
				 regridBandwidth, 
				 regridChanWidth, 
				 regridVeloRestfrq,
				 regridQuant,
				 transNewXin, 
				 transCHAN_WIDTH,
				 message,
				 centerIsStart,
				 nchan,
				 width,
				 start
				 )
	       ){ // there was an error
	      os << LogIO::SEVERE << message << LogIO::POST;
	      return False;
	    }
	    
	    // we have a useful set of channel boundaries
	    newNUM_CHAN = newChanLoBound.size();
	    
	    message = "input frame = " + MFrequency::showType(theOldRefFrame) 
	      + ", output frame = " + MFrequency::showType(theFrame)
              + "\n" + message + " Interpolation Method = " + methodName;
	    
	    // complete the calculation of the new spectral window parameters
	    // from newNUM_CHAN, newChanLoBound, and newChanHiBound 
	    newXout.resize(newNUM_CHAN);
	    newCHAN_WIDTH.resize(newNUM_CHAN);
	    newRESOLUTION.resize(newNUM_CHAN);
 	    newEFFECTIVE_BW.resize(newNUM_CHAN);
	    for(Int i=0; i<newNUM_CHAN; i++){
	      newXout[i] = (newChanLoBound[i]+newChanHiBound[i])/2.;
	      newCHAN_WIDTH[i] = newChanHiBound[i]-newChanLoBound[i];
	      newRESOLUTION[i] = newCHAN_WIDTH[i]; // to be revisited
	      newEFFECTIVE_BW[i] = newCHAN_WIDTH[i]; // to be revisited
	    }
	    // set the reference frequency to the central frequency of the first channel,
	    // keeping the already changed frame
	    MVFrequency mvf(newXout[0]);
	    newREF_FREQUENCY.set(mvf);
	    
	    // trivial definition of the bandwidth
	    newTOTAL_BANDWIDTH = newChanHiBound[newNUM_CHAN-1]-newChanLoBound[0];
	    
// 	    // effective bandwidth needs to be interpolated in quadrature
// 	    Vector<Double> newEffBWSquared(newNUM_CHAN);
// 	    Vector<Double> oldEffBWSquared(oldEFFECTIVE_BW);
// 	    for(Int i=0; i<oldNUM_CHAN; i++){
// 	      oldEffBWSquared[i] *= oldEffBWSquared[i];
// 	    }
// 	    InterpolateArray1D<Double, Double>::interpolate(newEffBWSquared, newXout,
//                                                             transNewXin,
//                                                             oldEffBWSquared,
//                                                             InterpolateArray1D<Double,Double>::linear);
// 	    for(Int i=0; i<newNUM_CHAN; i++){
// 	      newEFFECTIVE_BW[i] = sqrt(newEffBWSquared[i]);
// 	    }

	    if(!allEQ(newXout, transNewXin)){ // grids are different
	      doRegrid = True;
	    }
	    
	  } // end if (regridQuant=="" ... 
	  
	  if(writeTables && (needTransform || doRegrid)){
            // new SPW amd DD table rows may need to be created
	    
	    // Create new row in the SPW table (with ID nextSPWId) by copying
	    // all information from row theSPWId
	    if(!spwtable.canAddRow()){
	      os << LogIO::WARN
                 << "Unable to add new row to SPECTRAL_WINDOW table. Cannot proceed with regridSpw ..." 
		 << LogIO::POST;
	      return False; 
	    }
	    
	    numNewSPWIds++;
	    nextSPWId++;
	    
	    // prepare parameter string for later entry into MS history and report to logger
	    {    
	      ostringstream param;
	      param << "Regridded spectral window "
		    << nextSPWId - origNumSPWs << " will be created for field " 
		    << theFieldId << " with parameters " << endl
                    << message << endl;
	      regridMessage += param.str(); // append
	      os << LogIO::NORMAL << param.str() << LogIO::POST;
	    }
	    
	    spwtable.addRow();
	    TableRow SPWRow(spwtable);
	    TableRecord spwRecord = SPWRow.get(theSPWId);
	    // TODO        Warn if the original channels are not contiguous or overlap!
	    SPWRow.putMatchingFields(nextSPWId, spwRecord);
	    
	    // and replacing the following columns with updated information:
	    // Store xout as new value of CHAN_FREQ.
	    chanFreqCol.put(nextSPWId, newXout);
	    numChanCol.put(nextSPWId, newNUM_CHAN);
	    chanWidthCol.put(nextSPWId,  newCHAN_WIDTH);
	    refFrequencyCol.put(nextSPWId, newREF_FREQUENCY.getValue());
	    measFreqRefCol.put(nextSPWId, (Int)theFrame);
	    totalBandwidthCol.put(nextSPWId, newTOTAL_BANDWIDTH);
	    effectiveBWCol.put(nextSPWId, newEFFECTIVE_BW);
	    resolutionCol.put(nextSPWId, newRESOLUTION);
	    
	    msModified = True;
	    //   Create a new row in the DATA_DESCRIPTION table and enter
	    //   nextSPWId in the SPW_ID column, copy the polarization id and
	    //   the flag_row content from the old DATA_DESCRIPTION row.
	    if(!ddtable.canAddRow()){
	      os << LogIO::WARN
                 << "Unable to add new row to DATA_DESCRIPTION table.  Cannot proceed with regridSpw ..." 
		 << LogIO::POST;
	      return False; 
	    }
	    numNewDataDesc++;
	    nextDataDescId++;
	    ddtable.addRow();
	    TableRow DDRow(ddtable);
	    TableRecord DDRecord = DDRow.get(theDataDescId);
	    DDRow.putMatchingFields(nextDataDescId, DDRecord);

            // anticipate the deletion of the original SPW table rows
	    SPWIdCol.put(nextDataDescId, nextSPWId - origNumSPWs); 
	    
	    // writing the value of nextDataDescId into the DATA_DESC_ID cell
	    // of the present MAIN table row.  will be done in the main regirdSpw
	    // method
	    theDataDescId = nextDataDescId;
	  
	  } // end if(writeTables && (needTransform || doRegrid)

	} // end if there is a reusable SPW row

	if(writeTables && (needTransform || doRegrid)){
          // a new SOURCE table row has to be created

          // Add a row to the SOURCE table by copying the contents from the row
          // identified by the SOURCE_ID cell in the row theFieldId from the
          // FIELD table. Set the value of the cell SPECTRAL_WINDOW_ID in this
          // new row to the value nextSPWId column.
	  if(nextSourceRow>=0){ // there is a source table
	    if(!p_sourcetable->canAddRow()){
	      os << LogIO::WARN
                 << "Unable to add new row to SOURCE table. Cannot proceed with regridSpw ..." 
		 << LogIO::POST;
	      return False; 
	    }
	    numNewSourceRows++;
	    nextSourceRow++;
	    // find the row in the SOURCE table which has
	    // SOURCE_ID==theSOURCEId and SPW_ID==theSPWId
	    Int theSOURCEId = FIELDsourceIdCol(theFieldId);
	    ScalarColumn<Int> SOURCEsourceIdCol = p_sourceCol->sourceId();
	    ScalarColumn<Int> SOURCESPWIdCol = p_sourceCol->spectralWindowId();
	    Int foundRow = -1;
	    for(int i=0; i<nextSourceRow; i++){
	      if(SOURCEsourceIdCol(i) == theSOURCEId && SOURCESPWIdCol(i)==theSPWId){
		foundRow = i;
		break;
	      }
	    }
	    if(foundRow<0){ 
	      os << LogIO::SEVERE << "Incoherent MS: Did not find SOURCE table entry with SOURCE_ID == " 
		 << theSOURCEId << " and  SPECTRAL_WINDOW_ID == " << theSPWId << endl
		 <<" even though the FIELD and the DATA_DESCRIPTION table entries for main table row " 
		 << mainTabRow << " refer to it." 
		 << LogIO::POST;
	      return False;
	    }
	    else { // found matching row
	      p_sourcetable->addRow();
	      TableRow SOURCERow(*p_sourcetable);
	      TableRecord SOURCERecord = SOURCERow.get(foundRow);
	      SOURCERow.putMatchingFields(nextSourceRow, SOURCERecord);

              // anticipate the deletion of the original SPW rows
	      SOURCESPWIdCol.put(nextSourceRow, nextSPWId - origNumSPWs);
	    }
	      
	  } // end if there is a source table
	}

	//Put a new row into the "done" table.
	// (do all the push_backs in one place)
	oldSpwId.push_back(theSPWId);
	oldFieldId.push_back(theFieldId);

        // anticipate the deletion of the original DD rows
	newDataDescId.push_back(theDataDescId - origNumDataDescs);

	xold.push_back(newXin);
	xin.push_back(transNewXin);
	xout.push_back(newXout);
	method.push_back(theMethod);
	methodF.push_back(theMethodF);
	regrid.push_back(doRegrid);
	transform.push_back(needTransform);
	theFieldDirV.push_back(theFieldDir);
	mObsPosV.push_back(mObsPos);
	fromFrameTypeV.push_back(theOldRefFrame);
	outFrameV.push_back(outFrame);

      } // end if(!alreadyDone)
      // reference frame transformation and regridding of channel definition completed
      ////////////////////

    } // end loop over main table

    if(writeTables && msModified){
      // delete the original rows in DD, SPW, and SOURCE if necessary
      if(numNewDataDesc>0){
	for(Int i=0; i<origNumDataDescs; i++){
	  ddtable.removeRow(0);
	}
      }
      if(numNewSPWIds>0){
	for(Int i=0; i<origNumSPWs; i++){
	  spwtable.removeRow(0);
	}
      }
      if(numNewSourceRows>0){
	for(Int i=0; i<origNumSourceRows; i++){
	  p_sourcetable->removeRow(0);
	}
      }

      // prepare parameter string for later entry into MS history
      ostringstream param;
      param << "Added " << numNewDataDesc
            << " new rows to the DATA_DESCRIPTION table and deleted "
            << origNumDataDescs << " old ones." << endl
            << "Added " << numNewSPWIds
            << " rows to the SPECTRAL_WINDOW table and deleted " << origNumSPWs 
            << " old ones." << endl
            << "Added " << numNewSourceRows
            << " rows to the SOURCE table and deleted " << origNumSourceRows
            << " old ones.";
      regridMessage += param.str() + "\n"; // append

      os << LogIO::NORMAL << param.str() <<  LogIO::POST;
    }

    delete p_sourceCol;

    return rval;
  }

  Bool SubMS::combineSpws(const Vector<Int>& spwids){
    
    LogIO os(LogOrigin("SubMS", "combineSpws()"));
      
    // Analyse spwids

    if(spwids.nelements()==0){
      os << LogIO::WARN << "No SPWs selected for combination ..." <<  LogIO::POST;
      return True;
    }

    String tempNewName = ms_p.tableName()+".spwCombined"; // temporary name for the MS to store the result

    Bool allScratchColsPresent = False;

    { // begin scope for MS related objects

      // find all existing spws, 
      MSSpectralWindow spwtable = ms_p.spectralWindow();
      Int origNumSPWs = spwtable.nrow();
      Int newSPWId = origNumSPWs;

      vector<Int> spwsToCombine;

      if(spwids(0) == -1){
	for(Int i=0; i<origNumSPWs; i++){
	  spwsToCombine.push_back(i);
	}
      }
      else {
	for(uInt i=0; i<spwids.nelements(); i++){
	  if(spwids(i)<origNumSPWs && spwids(i)>=0){
	    spwsToCombine.push_back(spwids(i));
	  }
	  else{
	    os << LogIO::SEVERE << "Invalid SPW ID selected for combination " << spwids(i) 
	       << "valid range is 0 - " << origNumSPWs-1 << ")" << LogIO::POST;
	    return False;
	  }
	}
      }
      if(spwsToCombine.size()<=1){
	os << LogIO::NORMAL << "Less than two SPWs selected. No combination necessary."
	   << LogIO::POST;
	return True;
      }
      
      // sort the spwids
      std::sort(spwsToCombine.begin(), spwsToCombine.end());

      uInt nSpwsToCombine = spwsToCombine.size();

      // prepare access to the SPW table
      MSSpWindowColumns SPWCols(spwtable);
      ScalarColumn<Int> numChanCol = SPWCols.numChan(); 
      ArrayColumn<Double> chanFreqCol = SPWCols.chanFreq(); 
      ArrayColumn<Double> chanWidthCol = SPWCols.chanWidth(); 
      //    ArrayMeasColumn<MFrequency> chanFreqMeasCol = SPWCols.chanFreqMeas();
      ScalarColumn<Int> measFreqRefCol = SPWCols.measFreqRef();
      ArrayColumn<Double> effectiveBWCol = SPWCols.effectiveBW();   
      ScalarColumn<Double> refFrequencyCol = SPWCols.refFrequency(); 
      //    ScalarMeasColumn<MFrequency> refFrequencyMeasCol = SPWCols.refFrequencyMeas(); 
      ArrayColumn<Double> resolutionCol = SPWCols.resolution(); 
      ScalarColumn<Double> totalBandwidthCol = SPWCols.totalBandwidth();

      // create a list of the spw ids sorted by first channel frequency
      vector<Int> spwsSorted(origNumSPWs);
      {
	Double* firstFreq = new Double[origNumSPWs];
	for(uInt i=0; (Int)i<origNumSPWs; i++){
	  Vector<Double> CHAN_FREQ(chanFreqCol(i));
	  firstFreq[i] = CHAN_FREQ(0);
	}
	Sort sort;
	sort.sortKey (firstFreq, TpDouble); // define sort key
	Vector<uInt> inx(origNumSPWs);
	sort.sort(inx, (uInt)origNumSPWs);
	for (uInt i=0; (Int)i<origNumSPWs; i++) {
	  spwsSorted[i] = spwsToCombine[inx(i)];
	}
	delete[] firstFreq;
      }

      // Create new row in the SPW table (with ID nextSPWId) by copying
      // all information from row theSPWId
      if(!spwtable.canAddRow()){
	os << LogIO::WARN
	   << "Unable to add new row to SPECTRAL_WINDOW table. Cannot proceed with spwCombine ..." 
	   << LogIO::POST;
	return False; 
      }
      TableRow SPWRow(spwtable);
      Int id0 = spwsSorted[0];
      TableRecord spwRecord = SPWRow.get(id0);

      Int newNUM_CHAN = numChanCol(id0);
      Vector<Double> newCHAN_FREQ(chanFreqCol(id0));
      Vector<Double> newCHAN_WIDTH(chanWidthCol(id0));
      Vector<Double> newEFFECTIVE_BW(effectiveBWCol(id0));
      Double newREF_FREQUENCY(refFrequencyCol(id0));
      //MFrequency newREF_FREQUENCY = refFrequencyMeasCol(id0);
      Int newMEAS_FREQ_REF = measFreqRefCol(id0);
      Vector<Double> newRESOLUTION(resolutionCol(id0));
      Double newTOTAL_BANDWIDTH = totalBandwidthCol(id0);

      vector<Int> averageN; // for each new channel store the number of old channels to average over
      vector<vector<Int> > averageWhichSPW; // for each new channel store the
      // (old) SPWs to average over  
      vector<vector<Int> > averageWhichChan; // for each new channel store the
      // channel numbers to av. over
      vector<vector<Double> > averageChanFrac; // for each new channel store the
      // channel fraction for each old channel
      // initialise the averaging vectors
      for(Int i=0; i<newNUM_CHAN; i++){
	averageN.push_back(1);
	vector<Int> tv; // just a temporary auxiliary vector
	tv.push_back(id0);
	averageWhichSPW.push_back(tv);
	tv[0] = i;
	averageWhichChan.push_back(tv);
	vector<Double> tvd; // another one
	tvd.push_back(1.);
	averageChanFrac.push_back(tvd);
      }


      os << LogIO::NORMAL << "Original SPWs sorted by first channel frequency:" << LogIO::POST;
      {
	ostringstream oss; // needed for iomanip functions
	oss << "   SPW " << std::setw(3) << id0 << ": " << std::setw(5) << newNUM_CHAN 
	    << " channels, first channel = " << std::setprecision(9) << std::setw(14) << std::scientific << newCHAN_FREQ(0) << " Hz";
	if(newNUM_CHAN>1){
	  oss << ", last channel = " << std::setprecision(9) << std::setw(14) << std::scientific << newCHAN_FREQ(newNUM_CHAN-1) << " Hz";
	}
	os << LogIO::NORMAL << oss.str() << LogIO::POST;
      }

      // loop over remaining given spws
      for(uInt i=1; i<nSpwsToCombine; i++){
	Int idi = spwsSorted[i];
      
	Int newNUM_CHANi = numChanCol(idi);
	Vector<Double> newCHAN_FREQi(chanFreqCol(idi));
	Vector<Double> newCHAN_WIDTHi(chanWidthCol(idi));
	Vector<Double> newEFFECTIVE_BWi(effectiveBWCol(idi));
	//Double newREF_FREQUENCYi(refFrequencyCol(idi));
	//MFrequency newREF_FREQUENCYi = refFrequencyMeasCol(idi);
	Int newMEAS_FREQ_REFi = measFreqRefCol(idi);
	Vector<Double> newRESOLUTIONi(resolutionCol(idi));
	//Double newTOTAL_BANDWIDTHi = totalBandwidthCol(idi);

	ostringstream oss; // needed for iomanip functions
	oss << "   SPW " << std::setw(3) << idi << ": " << std::setw(5) << newNUM_CHANi 
	    << " channels, first channel = " << std::setprecision(9) << std::setw(14) << std::scientific << newCHAN_FREQi(0) << " Hz";
	if(newNUM_CHANi>1){
	  oss << ", last channel = " << std::setprecision(9) << std::setw(14) << std::scientific << newCHAN_FREQi(newNUM_CHANi-1) << " Hz";
	}
	os << LogIO::NORMAL << oss.str() << LogIO::POST;
      
	vector<Double> mergedChanFreq;
	vector<Double> mergedChanWidth;
	vector<Double> mergedEffBW;
	vector<Double> mergedRes;
	vector<Int> mergedAverageN;
	vector<vector<Int> > mergedAverageWhichSPW;
	vector<vector<Int> > mergedAverageWhichChan;
	vector<vector<Double> > mergedAverageChanFrac;

	// check for compatibility
	if(newMEAS_FREQ_REFi != newMEAS_FREQ_REF){
	  os << LogIO::WARN
	     << "SPW " << idi << " cannot be combined with SPW " << id0 << ". Non-matching ref. frame."
	     << LogIO::POST;
	  return False; 
	}

	// append or prepend spw to new spw
	// overlap at all?
	if(newCHAN_FREQ(newNUM_CHAN-1) + newCHAN_WIDTH(newNUM_CHAN-1)/2. 
	   < newCHAN_FREQi(0) - newCHAN_WIDTHi(0)/2.) {
	  // no overlap, and need to append
	  for(Int j=0; j<newNUM_CHAN; j++){
	    mergedChanFreq.push_back(newCHAN_FREQ(j));
	    mergedChanWidth.push_back(newCHAN_WIDTH(j));
	    mergedEffBW.push_back(newEFFECTIVE_BW(j));
	    mergedRes.push_back(newRESOLUTION(j));
	    mergedAverageN.push_back(averageN[j]);
	    mergedAverageWhichSPW.push_back(averageWhichSPW[j]);
	    mergedAverageWhichChan.push_back(averageWhichChan[j]);
	    mergedAverageChanFrac.push_back(averageChanFrac[j]);
	  }
	  vector<Int> tv;
	  tv.push_back(idi); // origin is spw idi
	  vector<Int> tv2;
	  tv2.push_back(0);
	  vector<Double> tvd;
	  tvd.push_back(1.); // fraction is 1.
	  for(Int j=0; j<newNUM_CHANi; j++){
	    mergedChanFreq.push_back(newCHAN_FREQi(j));
	    mergedChanWidth.push_back(newCHAN_WIDTHi(j));
	    mergedEffBW.push_back(newEFFECTIVE_BWi(j));
	    mergedRes.push_back(newRESOLUTIONi(j));
	    mergedAverageN.push_back(1); // so far only one channel
	    mergedAverageWhichSPW.push_back(tv);
	    tv2[0] = j;
	    mergedAverageWhichChan.push_back(tv2); // channel number is j
	    mergedAverageChanFrac.push_back(tvd);
	  }
	}
	else if( newCHAN_FREQ(0) - newCHAN_WIDTH(0)/2. 
		 > newCHAN_FREQi(newNUM_CHANi-1) + newCHAN_WIDTHi(newNUM_CHANi-1)/2.){ 
	  // no overlap, need to prepend
	  vector<Int> tv;
	  tv.push_back(idi); // origin is spw idi
	  vector<Int> tv2;
	  tv2.push_back(0);
	  vector<Double> tvd;
	  tvd.push_back(1.); // fraction is 1.
	  for(Int j=0; j<newNUM_CHANi; j++){
	    mergedChanFreq.push_back(newCHAN_FREQi(j));
	    mergedChanWidth.push_back(newCHAN_WIDTHi(j));
	    mergedEffBW.push_back(newEFFECTIVE_BWi(j));
	    mergedRes.push_back(newRESOLUTIONi(j));
	    mergedAverageN.push_back(1); // so far only one channel
	    mergedAverageWhichSPW.push_back(tv);
	    tv2[0] = j;
	    mergedAverageWhichChan.push_back(tv2); // channel number is j
	    mergedAverageChanFrac.push_back(tvd);
	  }
	  for(Int j=0; j<newNUM_CHAN; j++){
	    mergedChanFreq.push_back(newCHAN_FREQ(j));
	    mergedChanWidth.push_back(newCHAN_WIDTH(j));
	    mergedEffBW.push_back(newEFFECTIVE_BW(j));
	    mergedRes.push_back(newRESOLUTION(j));
	    mergedAverageN.push_back(averageN[j]);
	    mergedAverageWhichSPW.push_back(averageWhichSPW[j]);
	    mergedAverageWhichChan.push_back(averageWhichChan[j]);
	    mergedAverageChanFrac.push_back(averageChanFrac[j]);
	  }
	}
	else{ // there is overlap
	  Int id0StartChan = 0;
	  if(newCHAN_FREQi(0) - newCHAN_WIDTHi(0)/2. < 
	     newCHAN_FREQ(newNUM_CHAN-1) - newCHAN_WIDTH(newNUM_CHAN-1)/2.){
	    // spw idi starts before spw id0

	    // some utilities for the averaging info
	    vector<Int> tv; // temporary vector
	    tv.push_back(idi); // origin is spw idi
	    vector<Int> tv2;
	    tv2.push_back(0);
	    vector<Double> tvd;
	    tvd.push_back(1.); // fraction is 1.

	    // find the first overlapping channel and prepend non-overlapping channels
	    Double ubound0 = newCHAN_FREQ(0) + newCHAN_WIDTH(0)/2.;
	    Double lbound0 = newCHAN_FREQ(0) - newCHAN_WIDTH(0)/2.;
	    Double uboundk = 0.;
	    Double lboundk = 0.;	      
	    Int k;
	    for(k=0; k<newNUM_CHANi; k++){
	      uboundk = newCHAN_FREQi(k) + newCHAN_WIDTHi(k)/2.;
	      lboundk = newCHAN_FREQi(k) - newCHAN_WIDTHi(k)/2.;	      
	      if(lbound0 < uboundk){
		break;
	      }
	      mergedChanFreq.push_back(newCHAN_FREQi(k));
	      mergedChanWidth.push_back(newCHAN_WIDTHi(k));
	      mergedEffBW.push_back(newEFFECTIVE_BWi(k));
	      mergedRes.push_back(newRESOLUTIONi(k));
	      mergedAverageN.push_back(1); // so far only one channel
	      mergedAverageWhichSPW.push_back(tv);
	      tv2[0] = k;
	      mergedAverageWhichChan.push_back(tv2); // channel number is k
	      mergedAverageChanFrac.push_back(tvd);	    
	    }
	    // k's the one
	    if(lbound0 < uboundk && lboundk < lbound0){ // actual overlap, need to merge channel k with channel 0
	      Double newWidth = ubound0 - lboundk;
	      Double newCenter = lboundk + newWidth/2.;
	      mergedChanFreq.push_back(newCenter);
	      mergedChanWidth.push_back(newWidth);
	      mergedEffBW.push_back(newWidth); 
	      mergedRes.push_back(newWidth); 
	      mergedAverageN.push_back(averageN[0]+1); // one more channel contributes
	      tv2[0] = k; // channel k from spw idi
	      for(int j=0; j<averageN[0]; j++){
		tv.push_back(averageWhichSPW[0][j]); // additional contributors
		tv2.push_back(averageWhichChan[0][j]); // channel 0 from spw id0
		tvd.push_back(averageChanFrac[0][j]);
	      }
	      mergedAverageWhichSPW.push_back(tv);
	      mergedAverageWhichChan.push_back(tv2); 
	      mergedAverageChanFrac.push_back(tvd);
	      id0StartChan = 1;
	    }
	  }
	  // now move along SPW id0 and merge until end of id0 is reached, then just copy
	  for(Int j=id0StartChan; j<newNUM_CHAN; j++){
	    mergedChanFreq.push_back(newCHAN_FREQ(j));
	    mergedChanWidth.push_back(newCHAN_WIDTH(j));
	    mergedEffBW.push_back(newEFFECTIVE_BW(j));
	    mergedRes.push_back(newRESOLUTION(j));
	    for(Int k=0; k<newNUM_CHANi; k++){
	      Double overlap_frac = 0.;
	      // does channel j in spw id0 overlap with channel k in spw idi?
	      Double uboundj = newCHAN_FREQ(j) + newCHAN_WIDTH(j)/2.;
	      Double uboundk = newCHAN_FREQi(k) + newCHAN_WIDTHi(k)/2.;
	      Double lboundj = newCHAN_FREQ(j) - newCHAN_WIDTH(j)/2.;
	      Double lboundk = newCHAN_FREQi(k) - newCHAN_WIDTHi(k)/2.;
	      // determine fraction 
	      if(lboundj <= lboundk && uboundk <= uboundj){ // chan k is completely covered by chan j
		overlap_frac = 1.;
		//cout << "j " << j << " k " << k << " case 1" << endl;
		//cout << "overlap " << overlap_frac << endl;
	      }
	      else if(lboundk <= lboundj && uboundj <= uboundk){ // chan j is completely covered by chan k 
		overlap_frac = newCHAN_WIDTH(j)/newCHAN_WIDTHi(k);
		//cout << "j " << j << " k " << k << " case 2" << endl;
		//cout << "overlap " << overlap_frac << endl;
	      }
	      else if(lboundj < lboundk && lboundk < uboundj && uboundj < uboundk){ // lower end of k is overlapping with j
		overlap_frac = (uboundj - lboundk)/newCHAN_WIDTHi(k);
		//cout << "j " << j << " k " << k << " case 3" << endl;
		//cout << "overlap " << overlap_frac << endl;
	      }
	      else if(lboundk < lboundj && lboundj < uboundk && lboundj < uboundk){ // upper end of k is overlapping with j 
		overlap_frac = (uboundk - lboundj)/newCHAN_WIDTHi(k);
		//cout << "j " << j << " k " << k << " case 4" << endl;
		//cout << "overlap " << overlap_frac << endl;
	      }
	      if(overlap_frac > 0.){ // update averaging info
		averageN[j] += 1;
		averageWhichSPW[j].push_back(idi);
		averageWhichChan[j].push_back(k);
		averageChanFrac[j].push_back(overlap_frac);
	      }
	    } // end loop over spw idi
	    // append this channel with updated averaging info
	    mergedAverageN.push_back(averageN[j]);
	    mergedAverageWhichSPW.push_back(averageWhichSPW[j]);
	    mergedAverageWhichChan.push_back(averageWhichChan[j]);
	    mergedAverageChanFrac.push_back(averageChanFrac[j]);  
	  } // end loop over spw id0
	  if(newCHAN_FREQ(newNUM_CHAN-1) + newCHAN_WIDTH(newNUM_CHAN-1)/2.
	     < newCHAN_FREQi(newNUM_CHANi-1) + newCHAN_WIDTHi(newNUM_CHANi-1)/2.){// spw idi still continues!
	    // find the last overlapping channel
	    Int j = newNUM_CHAN-1;
	    Double uboundj = newCHAN_FREQ(j) + newCHAN_WIDTH(j)/2.;
	    Double lboundj = newCHAN_FREQ(j) - newCHAN_WIDTH(j)/2.;
	    Double uboundk = 0;
	    Double lboundk = 0;	      
	    Int k;
	    for(k=newNUM_CHANi-1; k>=0; k--){
	      uboundk = newCHAN_FREQi(k) + newCHAN_WIDTHi(k)/2.;
	      lboundk = newCHAN_FREQi(k) - newCHAN_WIDTHi(k)/2.;	      
	      if(lboundk <= uboundj){
		break;
	      }
	    }
	    // k's the one 
 	    if(lboundk < uboundj && uboundj < uboundk ){ // actual overlap
	      Double overlap_frac = (uboundj - lboundk)/newCHAN_WIDTHi(k);
	      if(overlap_frac>0.5){ // merge channel k completely with channel j 
		Double newWidth = uboundk - lboundj;
		Double newCenter = (lboundj+uboundk)/2.;
		mergedChanFreq[j] =  newCenter;
		mergedChanWidth[j] = newWidth;
		mergedEffBW[j] = newWidth; 
		mergedRes[j] = newWidth; 
		mergedAverageChanFrac[j][mergedAverageN[j]-1] = 1.; 
	      }
	      else{ // create separate, more narrow channel
		Double newWidth = uboundk - uboundj;
		Double newCenter = (uboundj+uboundk)/2.;
		vector<Int> tv;
		tv.push_back(idi); // origin is spw idi
		vector<Int> tv2;
		tv2.push_back(0);
		vector<Double> tvd;
		tvd.push_back(1.); // fraction is 1.
		mergedChanFreq.push_back(newCenter);
		mergedChanWidth.push_back(newWidth);
		mergedEffBW.push_back(newWidth);
		mergedRes.push_back(newWidth);
		mergedAverageN.push_back(1); // so far only one channel
		mergedAverageWhichSPW.push_back(tv);
		tv2[0] = k;
		mergedAverageWhichChan.push_back(tv2); // channel number is k
		mergedAverageChanFrac.push_back(tvd);
	      }
	      k++; // start appending remaining channels after k
 	    }
	    // append the remaining channels
	    vector<Int> tv;
	    tv.push_back(idi); // origin is spw idi
	    vector<Int> tv2;
	    tv2.push_back(0);
	    vector<Double> tvd;
	    tvd.push_back(1.); // fraction is 1.
	    for(Int m=k; m<newNUM_CHANi; m++){
	      mergedChanFreq.push_back(newCHAN_FREQi(m));
	      mergedChanWidth.push_back(newCHAN_WIDTHi(m));
	      mergedEffBW.push_back(newEFFECTIVE_BWi(m));
	      mergedRes.push_back(newRESOLUTIONi(m));
	      mergedAverageN.push_back(1); // so far only one channel
	      mergedAverageWhichSPW.push_back(tv);
	      tv2[0] = m;
	      mergedAverageWhichChan.push_back(tv2); // channel number is m
	      mergedAverageChanFrac.push_back(tvd);
	    }
	  } // end if spw idi still continues
	} // end if there is overlap    


	newNUM_CHAN = mergedChanFreq.size();
	newCHAN_FREQ.assign(Vector<Double>(mergedChanFreq));
	newCHAN_WIDTH.assign(Vector<Double>(mergedChanWidth));
	newEFFECTIVE_BW.assign(Vector<Double>(mergedEffBW));
	newREF_FREQUENCY = newCHAN_FREQ(0); 
	newTOTAL_BANDWIDTH = newCHAN_FREQ(newNUM_CHAN-1) + newCHAN_WIDTH(newNUM_CHAN-1)/2.
	  - newCHAN_FREQ(0) + newCHAN_WIDTH(0)/2.;
	newRESOLUTION.assign(Vector<Double>(mergedRes));
	averageN = mergedAverageN;
	averageWhichSPW = mergedAverageWhichSPW;
	averageWhichChan = mergedAverageWhichChan;
	averageChanFrac = mergedAverageChanFrac;

      } // end loop over SPWs
      
      os << LogIO::NORMAL << "Combined SPW will have " << newNUM_CHAN << " channels. May change in later regridding." << LogIO::POST;

      // normalise channel fractions
      Vector<Double> newNorm(newNUM_CHAN, 0); 
      for(Int i=0; i<newNUM_CHAN; i++){
	for(Int j=0; j<averageN[i]; j++){
	  newNorm(i) += averageChanFrac[i][j];
	  //cout << " i, j " << i << ", " << j << " averageWhichChan[i][j] " << averageWhichChan[i][j]
	  //     << " averageWhichSPW[i][j] " << averageWhichSPW[i][j] << endl;
	  //cout << " averageChanFrac[i][j] " << averageChanFrac[i][j] << endl;
	}
      }	
      for(Int i=0; i<newNUM_CHAN; i++){
	//cout << "i " << i << " newNorm[i] " << newNorm[i] << endl;
	for(Int j=0; j<averageN[i]; j++){
          if(newNorm[i] != 0.0){
            averageChanFrac[i][j] /= newNorm[i];
	  }
	  //cout << "j " << j << " averageChanFrac[i][j] " << averageChanFrac[i][j] << endl;
	}
      }	

      // write new spw to spw table (ID =  newSpwId)
      spwtable.addRow();
      SPWRow.putMatchingFields(newSPWId, spwRecord);

      chanFreqCol.put(newSPWId, newCHAN_FREQ);
      numChanCol.put(newSPWId, newNUM_CHAN);
      chanWidthCol.put(newSPWId,  newCHAN_WIDTH);
      refFrequencyCol.put(newSPWId, newREF_FREQUENCY);
      totalBandwidthCol.put(newSPWId, newTOTAL_BANDWIDTH);
      effectiveBWCol.put(newSPWId, newEFFECTIVE_BW);
      resolutionCol.put(newSPWId, newRESOLUTION);

      // delete unwanted spws and memorize the new ID of the new merged one.
      // (remember the IDs were sorted above)
      for(int i=nSpwsToCombine-1; i>=0; i--){ // remove highest row numbers first
	spwtable.removeRow(spwsToCombine[i]);
      }
      newSPWId -= nSpwsToCombine; 

      // other tables to correct: MAIN, FREQ_OFFSET, SYSCAL, FEED, DATA_DESCRIPTION, SOURCE

      // 1) SOURCE (an optional table)
      uInt numSourceRows = 0;
      MSSource* p_sourcetable = NULL;
      MSSourceColumns* p_sourceCol = NULL;
      if(Table::isReadable(ms_p.sourceTableName())){
	p_sourcetable = &(ms_p.source());
	p_sourceCol = new MSSourceColumns(*p_sourcetable);
	numSourceRows = p_sourcetable->nrow();
	ScalarColumn<Int> SOURCESPWIdCol = p_sourceCol->spectralWindowId();
	// loop over source table rows
	for(uInt i=0; i<numSourceRows; i++){
	  for(uInt j=0; j<nSpwsToCombine; j++){
	    // if spw id affected, replace by newSpwId
	    if(SOURCESPWIdCol(i) == spwsToCombine[j]){ // source row i is affected
	      SOURCESPWIdCol.put(i, newSPWId);
	    }
	  } // end for j
	}// end for i
      }
      else { // there is no source table
	os << LogIO::NORMAL << "Note: MS contains no SOURCE table ..." << LogIO::POST;
      }

      // 2) DATA_DESCRIPTION
      MSDataDescription ddtable = ms_p.dataDescription();
      uInt numDataDescs = ddtable.nrow();
      MSDataDescColumns DDCols(ddtable);
      ScalarColumn<Int> SPWIdCol = DDCols.spectralWindowId();
      ScalarColumn<Int> PolIdCol = DDCols.polarizationId();
      vector<uInt> affDDIds;  
      vector<Bool> DDRowsToDelete(numDataDescs, False);
      SimpleOrderedMap <Int, Int> tempDDIndex(-1); // store relation between old and new DD Ids
      SimpleOrderedMap <Int, Int> DDtoSPWIndex(-1); // store relation between old DD Ids and old SPW Ids 
      //  (only for affected SPW IDs)
      // loop over DD table rows
      for(uInt i=0; i<numDataDescs; i++){
	// if spw id affected, replace by newSpwId
	for(uInt j=0; j<nSpwsToCombine; j++){
	  // if spw id affected, replace by newSpwId
	  if(SPWIdCol(i) == spwsToCombine[j]){ // DD row i is affected
	    // correct the SPW Id in the DD table
	    SPWIdCol.put(i, newSPWId);
	    // memorize affected DD IDs in affDDIds
	    affDDIds.push_back(i);
	    // store relation between old DD Id and old SPW ID for later use in the modification of the MAIN table
	    DDtoSPWIndex.define(i, spwsToCombine[j]); // note: this relation can be many-to-one  
	  }     
	}
      }
      // Find redundant DD IDs
      // loop over DD table rows
      for(uInt i=0; i<numDataDescs; i++){
	Bool affected = False;
	for(uInt j=0; j<affDDIds.size(); j++){
	  if(i == affDDIds[j] && !DDRowsToDelete[i]){
	    affected = True;
	    break;
	  }
	}
	if(!affected){
	  continue;
	}
	else { // i is an affected row
	  Int PolIDi = PolIdCol(i);
	  Int SpwIDi = SPWIdCol(i);
	  // loop over following DD table rows
	  for(uInt j=i+1; j<numDataDescs; j++){
	    // if row i and row j redundant?
	    if(PolIDi == PolIdCol(j) && SpwIDi == SPWIdCol(j)){
	      // mark for deletion
	      DDRowsToDelete[j] = True;
	      // fill map for DDrenumbering
	      tempDDIndex.define(j, i);
	    }
	  }    
	} // end if affected 
      }
      // delete redundant DD rows
      Int removed = 0;
      for(uInt i=0; i<numDataDescs; i++){
	if(DDRowsToDelete[i]){
	  ddtable.removeRow(i-removed);
	  removed++;
	}
	else{ // this row is not deleted but changes its number by <removed> due to removal of others
	  tempDDIndex.define(i, i-removed);
	}
      }

      // 3) FEED  
      MSFeed feedtable = ms_p.feed();
      uInt numFeedRows = feedtable.nrow();
      MSFeedColumns feedCols(feedtable);
      ScalarColumn<Int> feedSPWIdCol = feedCols.spectralWindowId();
 
      // loop over FEED table rows
      for(uInt i=0; i<numFeedRows; i++){
	// if spw id affected, replace by newSpwId
	for(uint j=0; j<nSpwsToCombine; j++){
	  // if spw id affected, replace by newSpwId
	  if(feedSPWIdCol(i) == spwsToCombine[j]){ // feed row i is affected
	    feedSPWIdCol.put(i, newSPWId);
	  }     
	}
      }

      // TODO: (possibly, not clear if necessary) remove redundant FEED rows and propagate

      // 4) SYSCAL

      // note: syscal is optional

      if(!ms_p.sysCal().isNull()){
	MSSysCal sysCaltable = ms_p.sysCal();
	uInt numSysCalRows = sysCaltable.nrow();
	MSSysCalColumns sysCalCols(sysCaltable);
	ScalarColumn<Int> sysCalSPWIdCol = sysCalCols.spectralWindowId();
      
	// loop over SYSCAL table rows
	for(uInt i=0; i<numSysCalRows; i++){
	  // if spw id affected, replace by newSpwId
	  for(uInt j=0; j<nSpwsToCombine; j++){
	    // if spw id affected, replace by newSpwId
	    if(sysCalSPWIdCol(i) == spwsToCombine[j]){ // SysCal row i is affected
	      sysCalSPWIdCol.put(i, newSPWId);
	    }     
	  }
	}
      }

      // 5) FREQ_OFFSET

      // note: freq_offset is optional

      if(!ms_p.freqOffset().isNull()){
	MSFreqOffset freqOffsettable = ms_p.freqOffset();
	uInt numFreqOffsetRows = freqOffsettable.nrow();
	MSFreqOffsetColumns freqOffsetCols(freqOffsettable);
	ScalarColumn<Int> freqOffsetSPWIdCol = freqOffsetCols.spectralWindowId();
      
	// loop over FREQ_OFFSET table rows
	for(uInt i=0; i<numFreqOffsetRows; i++){
	  // if spw id affected, replace by newSpwId
	  for(uInt j=0; j<nSpwsToCombine; j++){
	    // if spw id affected, replace by newSpwId
	    if(freqOffsetSPWIdCol(i) == spwsToCombine[j]){ // FreqOffset row i is affected
	      freqOffsetSPWIdCol.put(i, newSPWId);
	    }     
	  }
	}
      }

      // 6) MAIN

      // expect a time-sorted main table
      os << LogIO::NORMAL5 << "Note: combineSpw assumes the input MAIN table to be sorted in TIME ..." << LogIO::POST;
      
      ms_p.flush(True); // with fsync
      
      Table newMain(TableCopy::makeEmptyTable( tempNewName,
					       Record(),
					       (Table) ms_p,
					       Table::New,
					       Table::AipsrcEndian,
					       True, // replaceTSM 
					       True // noRows
					       )
		    );
      
      TableCopy::copySubTables(newMain, ms_p, False);
      
      MSMainColumns mainCols((MeasurementSet&)newMain);
      MSMainColumns oldMainCols(ms_p);
      
      uInt nMainTabRows = ms_p.nrow();
      
      // columns which depend on the number of frequency channels and may need to be combined:
      // DATA, FLOAT_DATA, CORRECTED_DATA, MODEL_DATA, IMAGING_WEIGHT, LAG_DATA, SIGMA_SPECTRUM,
      // WEIGHT_SPECTRUM, FLAG, and FLAG_CATEGORY    
      ArrayColumn<Complex> CORRECTED_DATACol =  mainCols.correctedData();
      ArrayColumn<Complex> oldCORRECTED_DATACol = oldMainCols.correctedData();
      ArrayColumn<Complex>  DATACol =  mainCols.data();
      ArrayColumn<Complex>  oldDATACol = oldMainCols.data();
      ArrayColumn<Float> FLOAT_DATACol =  mainCols.floatData();
      ArrayColumn<Float> oldFLOAT_DATACol = oldMainCols.floatData();
      ArrayColumn<Float> IMAGING_WEIGHTCol =  mainCols.imagingWeight();
      ArrayColumn<Float> oldIMAGING_WEIGHTCol = oldMainCols.imagingWeight();
      ArrayColumn<Complex> LAG_DATACol =  mainCols.lagData();
      ArrayColumn<Complex> oldLAG_DATACol = oldMainCols.lagData();
      ArrayColumn<Complex> MODEL_DATACol =  mainCols.modelData();
      ArrayColumn<Complex> oldMODEL_DATACol = oldMainCols.modelData();
      ArrayColumn<Float> SIGMA_SPECTRUMCol =  mainCols.sigmaSpectrum();
      ArrayColumn<Float> oldSIGMA_SPECTRUMCol = oldMainCols.sigmaSpectrum();
      ArrayColumn<Float> WEIGHT_SPECTRUMCol =  mainCols.weightSpectrum();
      ArrayColumn<Float> oldWEIGHT_SPECTRUMCol = oldMainCols.weightSpectrum();
      ArrayColumn<Bool> FLAGCol =  mainCols.flag();
      ArrayColumn<Bool> oldFLAGCol = oldMainCols.flag();
      ArrayColumn<Bool> FLAG_CATEGORYCol =  mainCols.flagCategory();
      ArrayColumn<Bool> oldFLAG_CATEGORYCol = oldMainCols.flagCategory();
      
      // columns which may be different for otherwise matching main table rows
      //  and need to be combined
      ScalarColumn<Bool> flagRowCol = oldMainCols.flagRow();

      // administrational columns needed from the main table
      ArrayColumn<Float> SIGMACol =  oldMainCols.sigma();
      ScalarColumn<Int> fieldCol = oldMainCols.fieldId();
      ScalarColumn<Int> DDIdCol = oldMainCols.dataDescId();
      ScalarColumn<Int> antenna1Col = oldMainCols.antenna1();
      ScalarColumn<Int> antenna2Col = oldMainCols.antenna2();
      ScalarColumn<Double> timeCol = oldMainCols.time(); 
      ScalarColumn<Double> intervalCol = oldMainCols.interval();
      ScalarColumn<Double> exposureCol = oldMainCols.exposure();
      ScalarMeasColumn<MEpoch> mainTimeMeasCol = oldMainCols.timeMeas();

      // arrays for composing the combined columns 
      // model them on the first affected row of the main table
      
      Matrix<Complex> newCorrectedData; 
      Matrix<Complex> newData;
      Matrix<Float> newFloatData;
      Vector<Float> newImagingWeight; // imaging weight is independent of the correlator
      Matrix<Complex> newLagData;
      Matrix<Complex> newModelData;
      Matrix<Float> newSigmaSpectrum;
      Matrix<Float> newWeightSpectrum;
      Matrix<Bool> newFlag;
      Array<Bool> newFlagCategory; // has three dimensions
      Bool newFlagRow; 

      // find the first row affected by the spw combination
      Int firstAffRow = 0;
      for(uInt mRow=0; mRow<nMainTabRows; mRow++){
	if(DDtoSPWIndex.isDefined(DDIdCol(mRow))){
	  firstAffRow = mRow;
	  break;
	}
      }
      
      // get the number of correlators from the
      // dimension of the first axis of the sigma column
      uInt nCorrelators = SIGMACol(firstAffRow).shape()(0); 
      
      IPosition newShape = IPosition(2, nCorrelators, newNUM_CHAN);
      
      Bool CORRECTED_DATAColIsOK = !CORRECTED_DATACol.isNull();
      Bool DATAColIsOK = !DATACol.isNull();
      Bool FLOAT_DATAColIsOK = !FLOAT_DATACol.isNull();
      Bool IMAGING_WEIGHTColIsOK = !IMAGING_WEIGHTCol.isNull();
      Bool LAG_DATAColIsOK = !LAG_DATACol.isNull();
      Bool MODEL_DATAColIsOK = !MODEL_DATACol.isNull();
      Bool SIGMA_SPECTRUMColIsOK = !SIGMA_SPECTRUMCol.isNull();
      Bool WEIGHT_SPECTRUMColIsOK = !WEIGHT_SPECTRUMCol.isNull(); // rechecked further below
      Bool FLAGColIsOK = !FLAGCol.isNull();
      Bool FLAG_CATEGORYColIsOK = False; // to be set to the correct value further below
      
      allScratchColsPresent = CORRECTED_DATAColIsOK && MODEL_DATAColIsOK && IMAGING_WEIGHTColIsOK;

      // initialize arrays to store combined column data
      if(CORRECTED_DATAColIsOK){
	newCorrectedData.resize(newShape);
      }
      if(DATAColIsOK){
	newData.resize(newShape);
      }
      if(FLOAT_DATAColIsOK){
	newFloatData.resize(newShape);
      }
      if(IMAGING_WEIGHTColIsOK){
	if(oldIMAGING_WEIGHTCol.shape(firstAffRow).nelements() == 1){
	  newImagingWeight.resize(newNUM_CHAN);
	}
	else{
	  os << LogIO::SEVERE << "Error: unexpected shape of imaging weight column: " 
	     << oldIMAGING_WEIGHTCol.shape(firstAffRow) << LogIO::POST;
	  return False;
	}
      }
      if(LAG_DATAColIsOK){
	newLagData.resize(newShape); 
      }
      if(MODEL_DATAColIsOK){
	newModelData.resize(newShape);
      }
      if(SIGMA_SPECTRUMColIsOK){
	newSigmaSpectrum.resize(newShape);
      }
      if(WEIGHT_SPECTRUMColIsOK){
	if(oldWEIGHT_SPECTRUMCol.isDefined(firstAffRow)){ // required column but may be empty
	  newWeightSpectrum.resize(newShape);
	}
	else{
	  WEIGHT_SPECTRUMColIsOK = False;
	}
      }
      if(FLAGColIsOK){ // required but one never knows (there may be bugs elsewhere)
	newFlag.resize(newShape);
      }
      IPosition flagCatShape;
      uInt nCat  = 0;
      if(!FLAG_CATEGORYCol.isNull()){ 
	if(oldFLAG_CATEGORYCol.isDefined(firstAffRow)){ // required column but may be empty
	  FLAG_CATEGORYColIsOK = True;
	  flagCatShape = oldFLAG_CATEGORYCol.shape(firstAffRow);
	  nCat = flagCatShape(2); // the dimension of the third axis ==
	  // number of categories
	  newFlagCategory.resize(IPosition(3, nCorrelators, newNUM_CHAN, nCat));
	} 
      }
      
      ///////////////////////////////////////// 
      // Loop over main table rows
      uInt mainTabRow = 0;
      uInt newMainTabRow = 0;
      uInt nIncompleteCoverage = 0; // number of rows with incomplete SPW coverage
      // prepare progress meter
      Float progress = 0.4;
      Float progressStep = 0.4;
      if(nMainTabRows>1000000){
	progress = 0.2;
	progressStep = 0.2;
      }

      while(mainTabRow<nMainTabRows){
	
	// should row be combined with others, i.e. has SPW changed?
	// no -> just renumber DD ID (because of shrunk DD ID table)
	
	// yes-> find rows from the spws tobe combined with same timestamp, antennas and field
	//       merge these rows
	//       write merged row over first one, correcting DD ID at the same time
	//       set TIME to 0 in other merged rows
	//       reduce nMainTabRows accordingly
	
	// continue
	//
	// when finished, delete all rows with TIME = 0
	
	Double theTime = timeCol(mainTabRow);
	
	// row was already combined with a previous row?
	if(theTime == 0){
	  //	  cout << "skipping row with zero time " << mainTabRow << endl;
	  mainTabRow++;
	  continue;
	}
	
	Int theDataDescId = DDIdCol(mainTabRow);
	
	// row affected by the spw combination? (uses the old DD numbering)
	if(DDtoSPWIndex.isDefined(theDataDescId)){
	  // find matching affected rows with same time stamp, antennas and field
	  Int theAntenna1 = antenna1Col(mainTabRow);
	  Int theAntenna2 = antenna2Col(mainTabRow);
	  Int theField = fieldCol(mainTabRow);
	  Double theInterval = intervalCol(mainTabRow);
	  Double toleratedTimeDiff = theInterval/10.;
	  Double theExposure = exposureCol(mainTabRow);
	  vector<Int> matchingRows;
	  matchingRows.push_back(mainTabRow);
	  vector<Int> matchingRowSPWIds;
	  matchingRowSPWIds.push_back(DDtoSPWIndex(theDataDescId));
	  SimpleOrderedMap <Int, Int> SPWtoRowIndex(-1);
	  SPWtoRowIndex.define(matchingRowSPWIds[0], mainTabRow);

	  //	  cout << "theRow = " << mainTabRow << ", time = " << theTime << " DDID " << theDataDescId << endl;
	  
	  uInt nextRow = mainTabRow+1;
	  //	  cout << "nextRow = " << nextRow << ", time diff  = " << timeCol(nextRow) - theTime << " DDID " << DDIdCol(nextRow) << endl;
	  while(nextRow<nMainTabRows &&
		(timeCol(nextRow) - theTime)< toleratedTimeDiff &&
		matchingRows.size() < nSpwsToCombine // there should be one matching row per SPW
		){

	    if(!DDtoSPWIndex.isDefined(DDIdCol(nextRow)) ||
	       antenna1Col(nextRow) != theAntenna1 ||
	       antenna2Col(nextRow) != theAntenna2 ||
	       fieldCol(nextRow) != theField ){ // not a matching row
	      nextRow++;
	      continue;
	    }
	    // check that the intervals are the same
	    if(intervalCol(nextRow) != theInterval){
	      os << LogIO::SEVERE << "Error: for time " <<  theTime << ", baseline (" << theAntenna1 << ", "
		 << theAntenna2 << "), field "<< theField << ", DataDescID " << DDIdCol(mainTabRow)
		 << " found matching row with DataDescID " << DDIdCol(nextRow) << endl
		 << " but the two rows have different intervals: " << theInterval
		 << " vs. " << intervalCol(nextRow)
		 << LogIO::POST;
	      return False;
	    }
	    // check that the exposures are the same
	    if(exposureCol(nextRow) != theExposure){
	      os << LogIO::SEVERE << "Error: for time " <<  theTime << ", baseline (" << theAntenna1 << ", "
		 << theAntenna2 << "), field "<< theField << ", DataDescID " << DDIdCol(mainTabRow)
		 << " found matching row with DataDescID " << DDIdCol(nextRow) << endl
		 << " but the two rows have different exposures: " << theExposure
		 << " vs. " << exposureCol(nextRow)
		 << LogIO::POST;
	      return False;
	    }
	    // found a matching row
	    Int theSPWId = DDtoSPWIndex(DDIdCol(nextRow));
	    if(SPWtoRowIndex.isDefined(theSPWId)){ // there should be a one-to-one relation: SPW <-> matching row
	      os << LogIO::SEVERE << "Error: for time " <<  theTime << ", baseline (" << theAntenna1 << ","
		 << theAntenna2 << "), field "<< theField << " found more than one row for SPW "
		 << theSPWId << LogIO::POST;
	      return False;
	    }
	    else{ // this SPW not yet covered, memorize SPWId, row number, and relation
	      matchingRowSPWIds.push_back(theSPWId);
	      matchingRows.push_back(nextRow);
	      SPWtoRowIndex.define(theSPWId, nextRow);
	      //	      cout << "matching nextRow = " << nextRow << ", time = " << timeCol(nextRow) << " DDID " << DDIdCol(nextRow) << endl;
	    }
	    nextRow++;
	  } // end while nextRow ...
	  
	  // now we have a set of matching rows
	  uInt nMatchingRows = matchingRows.size();
	  
	  if(nMatchingRows < nSpwsToCombine){
	    if(nIncompleteCoverage==0){
	      ostringstream oss;
	      oss << mainTimeMeasCol(mainTabRow);
	      os << LogIO::WARN << "Incomplete coverage of combined SPW starting at timestamp " << oss.str()
		 << ", baseline ( " << theAntenna1 << ", " << theAntenna2 << " )" << endl
		 << "In this and further affected rows, the data arrays will be padded with zeros and corresponding channels flagged." <<  LogIO::POST;
	    }
	    nIncompleteCoverage++;
	  }

	  // reset arrays and prepare input data matrices
	  
	  if(CORRECTED_DATAColIsOK){
	    newCorrectedData.set(0);
	  }
	  if(DATAColIsOK){
	    newData.set(0);
	  }
	  if(FLOAT_DATAColIsOK){
	    newFloatData.set(0);
	  }
	  if(IMAGING_WEIGHTColIsOK){
	    newImagingWeight.set(0);
	  }
	  if(LAG_DATAColIsOK){
	    newLagData.set(0);
	  }
	  if(MODEL_DATAColIsOK){
	    newModelData.set(0);
	  }
	  if(SIGMA_SPECTRUMColIsOK){
	    newSigmaSpectrum.set(0);
	  }
	  if(WEIGHT_SPECTRUMColIsOK){
	    newWeightSpectrum.set(0);
	  }
	  if(FLAGColIsOK){
	    newFlag.set(0);
	  }
	  if(FLAG_CATEGORYColIsOK){
	    newFlagCategory.set(0);
	  }

	  vector<Matrix<Complex> > newCorrectedDataI(nSpwsToCombine); 
	  vector<Matrix<Complex> > newDataI(nSpwsToCombine);
	  vector<Matrix<Float> > newFloatDataI(nSpwsToCombine);
	  vector<Vector<Float> > newImagingWeightI(nSpwsToCombine);
	  vector<Matrix<Complex> > newLagDataI(nSpwsToCombine);
	  vector<Matrix<Complex> > newModelDataI(nSpwsToCombine);
	  vector<Matrix<Float> > newSigmaSpectrumI(nSpwsToCombine);
	  vector<Matrix<Float> > newWeightSpectrumI(nSpwsToCombine);
	  vector<Matrix<Bool> > newFlagI(nSpwsToCombine);
	  vector<Array<Bool> > newFlagCategoryI(nSpwsToCombine); // has three dimensions
	  vector<Bool> newFlagRowI(nSpwsToCombine);

	  for(uInt i=0; i<nSpwsToCombine; i++){
	    Int theRowSPWId = spwsToCombine[i];
	    if(SPWtoRowIndex.isDefined(theRowSPWId)){ // there actually is a matching row for this SPW
	      Int theRow = SPWtoRowIndex(theRowSPWId);
	      if(CORRECTED_DATAColIsOK){
		newCorrectedDataI[theRowSPWId].reference(oldCORRECTED_DATACol(theRow));
	      }
	      if(DATAColIsOK){
		newDataI[theRowSPWId].reference(oldDATACol(theRow));
	      }
	      if(FLOAT_DATAColIsOK){
		newFloatDataI[theRowSPWId].reference(oldFLOAT_DATACol(theRow));
	      }
	      if(IMAGING_WEIGHTColIsOK){
		newImagingWeightI[theRowSPWId].reference(oldIMAGING_WEIGHTCol(theRow));
	      }
	      if(LAG_DATAColIsOK){
		newLagDataI[theRowSPWId].reference(oldLAG_DATACol(theRow));
	      }
	      if(MODEL_DATAColIsOK){
		newModelDataI[theRowSPWId].reference(oldMODEL_DATACol(theRow));
	      }
	      if(SIGMA_SPECTRUMColIsOK){
		newSigmaSpectrumI[theRowSPWId].reference(oldSIGMA_SPECTRUMCol(theRow));
	      }
	      if(WEIGHT_SPECTRUMColIsOK){
		newWeightSpectrumI[theRowSPWId].reference(oldWEIGHT_SPECTRUMCol(theRow));
	      }
	      if(FLAGColIsOK){
		newFlagI[theRowSPWId].reference(oldFLAGCol(theRow));
	      }
	      if(FLAG_CATEGORYColIsOK){
		newFlagCategoryI[theRowSPWId].reference(oldFLAG_CATEGORYCol(theRow));
	      }
	      newFlagRowI[theRowSPWId] = flagRowCol(theRow);
	    } // end if
	  } // end for i
	  
	  // merge data columns from all rows found using the averaging info from above
	  // averageN[], averageWhichSPW[], averageWhichChan[], averageChanFrac[]
	  
	  // loop over new channels
	  for(Int i=0; i<newNUM_CHAN; i++){
	    // initialise special treatment for Bool columns
	    if(FLAGColIsOK){
	      for(uInt k=0; k<nCorrelators; k++){ 
		newFlag(k,i) =  True; // overwritten with False below if there is a SPW where this channel is not flagged for this correlator
	      }
	    }
	    if(FLAG_CATEGORYColIsOK){
	      for(uInt k=0; k<nCorrelators; k++){ 
		for(uInt m=0; m<nCat; m++){ 
		  newFlagCategory(IPosition(3,k,i,m)) = False;
		}
	      }
	    }

	    Bool haveCoverage = False;
	    Vector<Double> modNorm(nCorrelators, 0.); // normalization for the averaging of the contributions from the SPWs
	    for(Int j=0; j<averageN[i]; j++){
	      if(SPWtoRowIndex.isDefined(averageWhichSPW[i][j])){
		for(uInt k=0; k<nCorrelators; k++){
		  if(!newFlagI[ averageWhichSPW[i][j] ]( k, averageWhichChan[i][j] )){
		    haveCoverage = True;
		    modNorm(k) += averageChanFrac[i][j];
		    if(FLAGColIsOK){
		      newFlag(k,i) = False; // there is valid data for this channel => don't flag in output
		    }
		  }
		}
	      }
	    }

	    if(haveCoverage){ // there is unflagged data for this channel
	      // loop over SPWs
	      for(Int j=0; j<averageN[i]; j++){
		// new channel value i 
		//   = SUM{j=0 to averageN[i]}( channelValue(SPW = averageWhichSPW[i][j], CHANNEL = averageWhichChan[i][j]) * averageChanFrac[i][j])
		if(SPWtoRowIndex.isDefined(averageWhichSPW[i][j])){

		  Double weight = 0.;

		  // loop over first dimension (number of correlators)
		  for(uInt k=0; k<nCorrelators; k++){
		    if(!newFlagI[ averageWhichSPW[i][j] ]( k, averageWhichChan[i][j] )){ // this channel is not flagged for the given SPW and correlator

                      // renormalize for the case of missing SPW coverage
		      weight = averageChanFrac[i][j] / modNorm(k);

		      if(CORRECTED_DATAColIsOK){
			newCorrectedData(k,i) += newCorrectedDataI[ averageWhichSPW[i][j] ]( k, averageWhichChan[i][j] ) * weight;
// 			cout << "row " << SPWtoRowIndex(averageWhichSPW[i][j]) << "averageWhichSPW[i][j] " 
// 			     << averageWhichSPW[i][j] << "  averageWhichChan[i][j] " << averageWhichChan[i][j]
// 			     << " i, j, k " << i << ", " << j << ", " << k << " averageChanFrac[i][j] " << averageChanFrac[i][j] 
// 			     << " modNorm(k) " << modNorm(k) << " newCorrectedDataI[ averageWhichSPW[i][j] ]( k, averageWhichChan[i][j] ) "
// 			     << newCorrectedDataI[ averageWhichSPW[i][j] ]( k, averageWhichChan[i][j] ) 
// 			     << " newCorrectedData(k,i) " << newCorrectedData(k,i) 
// 			     << " weight " << weight << endl; 
		      }
		      if(DATAColIsOK){
			newData(k,i) += newDataI[ averageWhichSPW[i][j] ]( k, averageWhichChan[i][j] ) * weight;
		      }
		      if(FLOAT_DATAColIsOK){
			newFloatData(k,i) += newFloatDataI[ averageWhichSPW[i][j] ]( k, averageWhichChan[i][j] ) * weight;
		      }
		      if(LAG_DATAColIsOK){
			newLagData(k,i) += newLagDataI[ averageWhichSPW[i][j] ]( k, averageWhichChan[i][j] ) * weight;
		      }
		      if(MODEL_DATAColIsOK){
			newModelData(k,i) += newModelDataI[ averageWhichSPW[i][j] ]( k, averageWhichChan[i][j] ) * weight;
		      }
		      if(SIGMA_SPECTRUMColIsOK){
			newSigmaSpectrum(k,i) += newSigmaSpectrumI[ averageWhichSPW[i][j] ]( k, averageWhichChan[i][j] ) * weight;
		      }
		      if(WEIGHT_SPECTRUMColIsOK){
			newWeightSpectrum(k,i) += newWeightSpectrumI[ averageWhichSPW[i][j] ]( k, averageWhichChan[i][j] ) * weight;
		      }

		    } // end if flagged
		  } // end for k = 0
		
		  // imaging weight is independent of the correlator
		  if(IMAGING_WEIGHTColIsOK){
		    newImagingWeight(i) += newImagingWeightI[ averageWhichSPW[i][j] ]( averageWhichChan[i][j] ) * weight;
		  }
		  
		  // special treatment for flag cat
		  if(FLAG_CATEGORYColIsOK){
		    for(uInt k=0; k<nCorrelators; k++){ // logical OR of all input spws
		      for(uInt m=0; m<nCat; m++){ 
			newFlagCategory(IPosition(3,k,i,m)) = 
			  newFlagCategory(IPosition(3,k,i,m)) || newFlagCategoryI[ averageWhichSPW[i][j] ](IPosition(3,k,averageWhichChan[i][j],m));
		      }
		    }
		  }

		} // end if there is a row for this SPW	      
	      } // end for j=0, loop over SPWs
	    } // if there is coverage for this channel 
	  } // end for i=0, loop over new channels

	  // calculate FLAG_ROW as logical OR of all input rows
	  newFlagRow = newFlagRowI[0];
	  for(uInt i=1; i<nMatchingRows; i++){
	    newFlagRow = newFlagRow || newFlagRowI[i];
	  }

	  // write data into the new main table
	  newMain.addRow(1,False);

	  //	  cout << "writing new row " << newMainTabRow << endl;
	  
	  if(CORRECTED_DATAColIsOK){
	    CORRECTED_DATACol.put(newMainTabRow, newCorrectedData);
	  }
	  if(DATAColIsOK){
// 	    cout << "old " << oldDATACol(mainTabRow).shape() << endl;
// 	    cout << "new " << newData.shape() << endl;
	    DATACol.put(newMainTabRow, newData);
	  }
	  if(FLOAT_DATAColIsOK){
	    FLOAT_DATACol.put(newMainTabRow, newFloatData);
	  }
	  if(IMAGING_WEIGHTColIsOK){
	    IMAGING_WEIGHTCol.put(newMainTabRow, newImagingWeight);
	  }
	  if(LAG_DATAColIsOK){
	    LAG_DATACol.put(newMainTabRow, newLagData);
	  }
	  if(MODEL_DATAColIsOK){
	    MODEL_DATACol.put(newMainTabRow, newModelData);
	  }
	  if(SIGMA_SPECTRUMColIsOK){
	    SIGMA_SPECTRUMCol.put(newMainTabRow, newSigmaSpectrum);
	  }
	  if(WEIGHT_SPECTRUMColIsOK){
	    WEIGHT_SPECTRUMCol.put(newMainTabRow, newWeightSpectrum);
	  }
	  if(FLAGColIsOK){
	    FLAGCol.put(newMainTabRow, newFlag);
	  }
	  if(FLAG_CATEGORYColIsOK){
	    FLAG_CATEGORYCol.put(newMainTabRow, newFlagCategory);
	  }

	  mainCols.flagRow().put(newMainTabRow, newFlagRow);
	  
	  if(tempDDIndex.isDefined(theDataDescId)){
	    // do DD ID renumbering (due to shrunk DD table and spw combination )
	    mainCols.dataDescId().put(newMainTabRow, tempDDIndex(theDataDescId)); 
	  }
	  else{
	    mainCols.dataDescId().put(newMainTabRow,  DDIdCol(mainTabRow)); 
	  }	  
	  // copy the rest of the row contents from mainTabRow
	  mainCols.sigma().put(newMainTabRow, SIGMACol(mainTabRow));
	  mainCols.fieldId().put(newMainTabRow, fieldCol(mainTabRow));
	  mainCols.antenna1().put(newMainTabRow, antenna1Col(mainTabRow));
	  mainCols.antenna2().put(newMainTabRow, antenna2Col(mainTabRow));
	  mainCols.time().put(newMainTabRow, timeCol(mainTabRow));
	  mainCols.exposure().put(newMainTabRow, exposureCol(mainTabRow));
	  mainCols.interval().put(newMainTabRow, intervalCol(mainTabRow));

	  mainCols.uvw().put(newMainTabRow, oldMainCols.uvw()(mainTabRow));
	  mainCols.weight().put(newMainTabRow, oldMainCols.weight()(mainTabRow));
	  mainCols.arrayId().put(newMainTabRow, oldMainCols.arrayId()(mainTabRow));
	  mainCols.feed1().put(newMainTabRow, oldMainCols.feed1()(mainTabRow));
	  mainCols.feed2().put(newMainTabRow, oldMainCols.feed2()(mainTabRow));
	  mainCols.observationId().put(newMainTabRow, oldMainCols.observationId()(mainTabRow));
	  mainCols.processorId().put(newMainTabRow, oldMainCols.processorId()(mainTabRow));
	  mainCols.scanNumber().put(newMainTabRow, oldMainCols.scanNumber()(mainTabRow));
	  mainCols.stateId().put(newMainTabRow, oldMainCols.stateId()(mainTabRow));
	  mainCols.timeCentroid().put(newMainTabRow, oldMainCols.timeCentroid()(mainTabRow));

	  //	  cout << "Wrote new row " << newMainTabRow << endl;
	  newMainTabRow++;	  
	  // mark other found rows to be ignored, i.e. set their time to zero
	  for(uInt i=1; i<nMatchingRows; i++){ // don't mark the first
	    //	    cout << "setting time to zero in row " << matchingRows[i] << endl;
	    timeCol.put(matchingRows[i], 0);
	  }
	  
	} // end if row is affected
	
	mainTabRow++;
	if(mainTabRow>nMainTabRows*progress){
	  cout << "combineSpws progress: " << progress*100 << "% processed ... " << endl;
	  progress += progressStep;
	}
      
      } // end loop over main table rows
      cout << "combineSpws progress: 100% processed." << endl;
      ////////////////////////////////////////////

      os << LogIO::NORMAL << "Processed " << mainTabRow << " original rows, wrote "
	 << newMainTabRow << " new ones." << LogIO::POST;

      if(nIncompleteCoverage>0){
	os << LogIO::WARN << "Incomplete coverage of combined SPW in " << nIncompleteCoverage
	   << " of " <<  newMainTabRow << " output rows." <<  LogIO::POST;
      }

      newMain.flush(True); 

    } // end scope for MS related objects
 
    String oldName(ms_p.tableName());

    // detach old MS
    ms_p.flush(True);
    ms_p = MeasurementSet();
    mssel_p = MeasurementSet();

    // rename the result MS overwriting the old MS
    {
      Table tab(tempNewName, Table::Update);
      tab.rename(oldName, Table::New);
    }

    // attach new MS
    ms_p = MeasurementSet(oldName, Table::Update);
    mssel_p = ms_p;

    //
    // If all scratch columns are in the new MS, set the CHANNEL_SELECTION
    // keyword for the MODEL_DATA column.  This is apparently used
    // in at least imager to decide if MODEL_DATA and CORRECTED_DATA
    // columns should be initialized or not.
    //
    if(allScratchColsPresent){
      MSSpWindowColumns msSpW(ms_p.spectralWindow());
      Int nSpw=ms_p.spectralWindow().nrow();
      if(nSpw==0) nSpw=1;
      Matrix<Int> selection(2,nSpw);
      selection.row(0)=0; //start
      selection.row(1)=msSpW.numChan().getColumn();
      ArrayColumn<Complex> mcd(ms_p, MS::columnName(MS::MODEL_DATA));
      mcd.rwKeywordSet().define("CHANNEL_SELECTION",selection);
    }

    os << LogIO::NORMAL << "Spectral window combination complete." << LogIO::POST;

    return True;

  }


  const Vector<String>& SubMS::parseColumnNames(const String col)
  {
    // Memoize both for efficiency and so that the info message at the bottom
    // isn't unnecessarily repeated.
    static String my_colNameStr = "";
    static Vector<String> my_colNameVect;
    if(col == my_colNameStr && col != ""){
      return my_colNameVect;
    }    
    else if(col == "None"){
      my_colNameStr = "";
      my_colNameVect.resize(0);
      return my_colNameVect;
    }
 
    LogIO os(LogOrigin("SubMS", "parseColumnNames()"));
    Bool allCols=False;
    
    String tmpNames(col);
    Vector<String> tokens;
    uInt  nNames;
    tmpNames.upcase();
    
    if (tmpNames.contains("ALL")) allCols=True;

    { // split name string into individual names
	char * pch;
	Int i = 0;
	pch = strtok((char*)tmpNames.c_str()," ,");
	while (pch != NULL){
	    tokens.resize(i+1,True);
	    tokens(i) = String(pch);
	    i++;
	    pch = strtok(NULL, " ,");
	}
    }

    nNames=tokens.nelements();
    
    if (allCols){
	my_colNameVect.resize(3);
	my_colNameVect[0] = MS::columnName(MS::DATA);
	my_colNameVect[1] = MS::columnName(MS::MODEL_DATA);
	my_colNameVect[2] = MS::columnName(MS::CORRECTED_DATA);
    }
    else{
	for(uInt i=0; i<nNames; i++){
	    my_colNameVect.resize(i+1,True);
	    my_colNameVect[i]="Unrecognized";
	    
	    if (tokens[i]=="OBSERVED" || 
		tokens[i]=="DATA" || 
		tokens[i]==MS::columnName(MS::DATA)) 
	    {
		my_colNameVect[i] = MS::columnName(MS::DATA);
	    }
	    else if(tokens[i]=="MODEL" || 
		    tokens[i]=="MODEL_DATA" || 
		    tokens[i]==MS::columnName(MS::MODEL_DATA)) 
	    {
		my_colNameVect[i] = "MODEL_DATA";
	    } 
	    else if(tokens[i]=="CORRECTED" || 
		    tokens[i]=="CORRECTED_DATA" || 
		    tokens[i]==MS::columnName(MS::CORRECTED_DATA)) 
	    {
		my_colNameVect[i] = "CORRECTED_DATA";
	    }
	    else
	    {
		my_colNameVect[0] = MS::columnName(MS::DATA);
		os << LogIO::SEVERE << "Unrecognized column " << tokens[i]
		   << " ...using DATA."
		   << LogIO::POST;
	    }
	}
    } 

    // Whether or not the MS has the columns is checked by verifyColumns().
    // Unfortunately it cannot be done here because this is a static method.

    os << LogIO::NORMAL
       << "Using ";     // Don't say "Splitting"; this is used elsewhere.
    for(uInt i=0;i<my_colNameVect.nelements();i++)
      os << my_colNameVect[i] << " ";
    os << " column" << (my_colNameVect.nelements() > 1 ? "s." : ".")
       << LogIO::POST;
    
    my_colNameStr = col;
    return my_colNameVect;
  };
  
  Bool SubMS::putDataColumn(MSColumns& msc, ROArrayColumn<Complex>& data, 
			    const String& colName, const Bool writeToDataCol)
  {
    if ((colName == MS::columnName(MS::DATA)) || (writeToDataCol))
      msc.data().putColumn(data);
    else if (colName ==  MS::columnName(MS::MODEL_DATA))
      msc.modelData().putColumn(data);
    else if (colName == MS::columnName(MS::CORRECTED_DATA))
      msc.correctedData().putColumn(data);
    return True;
  }

  Bool SubMS::putDataColumn(MSColumns& msc, Cube<Complex>& data, 
			    const String& colName, const Bool writeToDataCol)
  {
    if ((colName == MS::columnName(MS::DATA)) || (writeToDataCol))
      msc.data().putColumn(data);
    else if (colName ==  MS::columnName(MS::MODEL_DATA))
      msc.modelData().putColumn(data);
    else if (colName == MS::columnName(MS::CORRECTED_DATA))
      msc.correctedData().putColumn(data);
    return True;
  }
  
  Bool SubMS::getDataColumn(ROArrayColumn<Complex>& data,
			    const String& colName)
  {
    if(colName == MS::columnName(MS::DATA))
      data.reference(mscIn_p->data());
    else if(colName == MS::columnName(MS::MODEL_DATA))
      data.reference(mscIn_p->modelData());
    else
      data.reference(mscIn_p->correctedData());
    return True;
  }

  Bool SubMS::fillMainTable(const Vector<String>& colNames){
    
    LogIO os(LogOrigin("SubMS", "fillMainTable()"));
    
    msOut_p.addRow(mssel_p.nrow(), True);
    
    if(!antennaSel_p){
      msc_p->antenna1().putColumn(mscIn_p->antenna1());
      msc_p->antenna2().putColumn(mscIn_p->antenna2());
    }
    else{
      Vector<Int> ant1  = mscIn_p->antenna1().getColumn();
      Vector<Int> ant2  = mscIn_p->antenna2().getColumn();
      
      for(uInt k = 0; k < ant1.nelements(); ++k){
	ant1[k] = antNewIndex_p[ant1[k]];
	ant2[k] = antNewIndex_p[ant2[k]];
      }
      msc_p->antenna1().putColumn(ant1);
      msc_p->antenna2().putColumn(ant2);
    }
    msc_p->feed1().putColumn(mscIn_p->feed1());
    msc_p->feed2().putColumn(mscIn_p->feed2());

    msc_p->exposure().putColumn(mscIn_p->exposure());
    //  msc_p->flag().putColumn(mscIn_p->flag());
    // if(!(mscIn_p->flagCategory().isNull()))
    //  if(mscIn_p->flagCategory().isDefined(0))
    //    msc_p->flagCategory().putColumn(mscIn_p->flagCategory());
    msc_p->flagRow().putColumn(mscIn_p->flagRow());
    msc_p->interval().putColumn(mscIn_p->interval());
    msc_p->scanNumber().putColumn(mscIn_p->scanNumber());
    msc_p->time().putColumn(mscIn_p->time());
    msc_p->timeCentroid().putColumn(mscIn_p->timeCentroid());

    // ScalarMeasColumn doesn't have a putColumn() for some reason.
    //msc_p->uvwMeas().putColumn(mscIn_p->uvwMeas());
    msc_p->uvw().putColumn(mscIn_p->uvw());

    msc_p->weight().putColumn(mscIn_p->weight());
    msc_p->sigma().putColumn(mscIn_p->sigma());

    relabelIDs();

    //Deal with data
    if(!chanModification_p){
      ROArrayColumn<Complex> data;

      for(uInt ni = 0;ni < colNames.nelements(); ++ni){
	getDataColumn(data, colNames[ni]);
	putDataColumn(*msc_p, data, colNames[ni], (colNames.nelements() == 1));
      }
      if(doImgWts_p){
        ROArrayColumn<Float> imgWts(mscIn_p->imagingWeight());
        msc_p->imagingWeight().putColumn(imgWts);
      }

      msc_p->flag().putColumn(mscIn_p->flag());
      if(!(mscIn_p->weightSpectrum().isNull()) &&
         mscIn_p->weightSpectrum().isDefined(0))
        msc_p->weightSpectrum().putColumn(mscIn_p->weightSpectrum());
    }
    else{
      if(sameShape_p){
	//
	//Checking to make sure we have in memory capability else 
	// use visbuffer
	//
	Double memAvail= Double (HostInfo::memoryTotal())*(1024);
	//Factoring in 30% for flags and other stuff
	if(1.3 * n_bytes() >  memAvail)
	  sameShape_p = False;
      }
      if(sameShape_p)
	writeSimilarSpwShape(colNames);
      else
	writeDiffSpwShape(colNames);
    }
    
    return True;
  }
  
  // Sets outcol to row numbers in the corresponding subtable of its ms that
  // correspond to the values of incol.
  //
  // Can only be used when incol and outcol have the same # of rows!
  //
  void SubMS::remapColumn(const ROScalarColumn<Int>& incol, ScalarColumn<Int>& outcol)
  {
    Vector<Int> colvec = incol.getColumn();
    Vector<Int> mapper;
    
    make_map(colvec, mapper);
    if(mapper.nelements() == 1){
      outcol.fillColumn(0);        // Just a little optimization.
    }
    else{
      for(uInt k = 0; k < colvec.nelements(); ++k)
	colvec[k] = static_cast<Int>(remapped(colvec[k], mapper,
					      k > 0 ? colvec[k - 1] + 1 : 0));
      outcol.putColumn(colvec);
    }
  }
  
// Realigns some _ID vectors so that the output looks like a whole dataset
// instead of part of one.  (i.e. we don't want references to unselected spws,
// etc.)
void SubMS::relabelIDs()
{
  const Vector<Int>& inDDID = mscIn_p->dataDescId().getColumn();
  Vector<Int> outDDID;
  
  outDDID.resize(inDDID.nelements());
  for(uInt k = 0; k < inDDID.nelements(); ++k)
    outDDID[k] = spwRelabel_p[oldDDSpwMatch_p[inDDID[k]]];
  msc_p->dataDescId().putColumn(outDDID);

  Vector<Int> fieldId = mscIn_p->fieldId().getColumn();
  for(uInt k = 0; k < fieldId.nelements(); ++k)
    fieldId[k] = fieldRelabel_p[fieldId[k]];
  msc_p->fieldId().putColumn(fieldId);

  remapColumn(mscIn_p->arrayId(), msc_p->arrayId());
  remapColumn(mscIn_p->stateId(), msc_p->stateId());
  remapColumn(mscIn_p->processorId(), msc_p->processorId());
  remapColumn(mscIn_p->observationId(), msc_p->observationId());
}

Bool SubMS::fillAverMainTable(const Vector<String>& colNames)
{    
  LogIO os(LogOrigin("SubMS", "fillAverMainTable()"));
    
  Double timeBin=timeBin_p;

  //// fill time and timecentroid and antennas
  nant_p = fillAntIndexer(mscIn_p, antIndexer_p);
  if(nant_p < 1)
    return False;

  //Int numBaselines=numOfBaselines(ant1, ant2, False);
  Int numTimeBins = numOfTimeBins(timeBin);  // Sets up remappers as a side-effect.
    
  if(numTimeBins < 1)
    os << LogIO::SEVERE
       << "Number of time bins is < 1: time averaging bin size is not > 0"
       << LogIO::POST;

  msOut_p.addRow(numOutRows_p, True);
        
  //    relabelIDs();

  msc_p->interval().fillColumn(timeBin);
    
  //things to be taken care in fillTimeAverData...
  // flagRow		ScanNumber	uvw		weight		
  // sigma		ant1		ant2		time
  // timeCentroid	feed1 		feed2		exposure
  // stateId		processorId	observationId	arrayId
  // imaging_weight (optional)
  return fillTimeAverData(colNames); 
}
  
  Bool SubMS::copyAntenna(){
    const MSAntenna& oldAnt = mssel_p.antenna();
    MSAntenna& newAnt = msOut_p.antenna();
    const ROMSAntennaColumns incols(oldAnt);
    MSAntennaColumns         outcols(newAnt);
    
    outcols.setOffsetRef(MPosition::castType(incols.offsetMeas().getMeasRef().getType()));
    outcols.setPositionRef(MPosition::castType(incols.positionMeas().getMeasRef().getType()));

    if(!antennaSel_p){
      TableCopy::copyRows(newAnt, oldAnt);
    }
    else{
      //Now we try to re-index the antenna list;
      Vector<Int> ant1 = mscIn_p->antenna1().getColumn();
      Int nAnt1=GenSort<Int>::sort(ant1,Sort::Ascending, Sort::NoDuplicates);
      ant1.resize(nAnt1, True);
      Vector<Int> ant2 = mscIn_p->antenna2().getColumn();
      Int nAnt2=GenSort<Int>::sort(ant2,Sort::Ascending, Sort::NoDuplicates);
      ant2.resize(nAnt2, True);
      ant1.resize(nAnt2+nAnt1, True);
      ant1(Slice(nAnt1,nAnt2))=ant2;
      nAnt1 = GenSort<Int>::sort(ant1,Sort::Ascending, Sort::NoDuplicates);
      ant1.resize(nAnt1, True);
      antNewIndex_p.resize(oldAnt.nrow());
      antNewIndex_p.set(-1); //So if you see -1 in the main, feed, or pointing
			     //tables, fix it
      for (Int k=0; k < nAnt1; ++k){
	antNewIndex_p[ant1[k]]=k;
	TableCopy::copyRows(newAnt, oldAnt, k, ant1[k], 1);
      }
      
      return True;
    }
    return False;    
  }


  Bool SubMS::copyFeed()
  {
    const MSFeed& oldFeed = mssel_p.feed();
    MSFeed& newFeed = msOut_p.feed();
    const ROMSFeedColumns incols(oldFeed);
    MSFeedColumns         outcols(newFeed);
    
    outcols.setDirectionRef(MDirection::castType(incols.beamOffsetMeas().getMeasRef().getType()));
    outcols.setEpochRef(MEpoch::castType(incols.timeMeas().getMeasRef().getType()));
    outcols.setPositionRef(MPosition::castType(incols.positionMeas().getMeasRef().getType()));

    if(!antennaSel_p && allEQ(spwRelabel_p, spw_p)){
      TableCopy::copyRows(newFeed, oldFeed);
    }
    else{
      if(!antennaSel_p){        // Prep antNewIndex_p.
        antNewIndex_p.resize(mssel_p.antenna().nrow());
        indgen(antNewIndex_p);
      }
      
      const Vector<Int>& antIds = incols.antennaId().getColumn();
      const Vector<Int>& spwIds = incols.spectralWindowId().getColumn();

      // Copy selected rows.
      uInt totNFeeds = antIds.nelements();
      uInt totalSelFeeds = 0;
      for (uInt k = 0; k < totNFeeds; ++k){
        // antenna must be selected, and spwId must be -1 (any) or selected.
	if(antNewIndex_p[antIds[k]] > -1 &&
           (spwIds[k] < 0 || spwRelabel_p[spwIds[k]] > -1)){
          //                  outtab   intab    outrow       inrow nrows
	  TableCopy::copyRows(newFeed, oldFeed, totalSelFeeds, k, 1);
          ++totalSelFeeds;
	}
      }

      // Remap antenna and spw #s.
      ScalarColumn<Int>& antCol = outcols.antennaId();
      ScalarColumn<Int>& spwCol = outcols.spectralWindowId();
      Vector<Int> newAntIds = antCol.getColumn();
      Vector<Int> newSpwIds = spwCol.getColumn();

      for(uInt k = 0; k < totalSelFeeds; ++k){
	newAntIds[k] = antNewIndex_p[newAntIds[k]];
	newSpwIds[k] = spwRelabel_p[newSpwIds[k]];
      }
      antCol.putColumn(newAntIds);
      spwCol.putColumn(newSpwIds);
    }
    return True;
  }
  
  
  Bool SubMS::copySource(){
    //Source is an optional table, so it may not exist
    if(Table::isReadable(mssel_p.sourceTableName())){
      LogIO os(LogOrigin("SubMS", "copySource()"));

      const MSSource& oldSource = mssel_p.source();
      MSSource& newSource = msOut_p.source();
      
      // Add optional columns if present in oldSource.
      Vector<String> optionalCols(6);
      optionalCols[0] = "TRANSITION";
      optionalCols[1] = "REST_FREQUENCY";
      optionalCols[2] = "SYSVEL";
      optionalCols[3] = "SOURCE_MODEL";
      optionalCols[4] = "PULSAR_ID";
      optionalCols[5] = "POSITION";
      uInt nAddedCols = addOptionalColumns(oldSource, newSource, optionalCols,
                                           true);
      os << LogIO::DEBUG1 << "SOURCE has " << nAddedCols
	 << " optional columns." << LogIO::POST;
      
      const ROMSSourceColumns incols(oldSource);
      MSSourceColumns         outcols(newSource);

      // Copy the Measures frame info.  This has to be done before filling the
      // rows.
      outcols.setEpochRef(MEpoch::castType(incols.timeMeas().getMeasRef().getType()));
      outcols.setDirectionRef(MDirection::castType(incols.directionMeas().getMeasRef().getType()));
      outcols.setPositionRef(MPosition::castType(incols.positionMeas().getMeasRef().getType()));
      outcols.setFrequencyRef(MFrequency::castType(incols.restFrequencyMeas().getMeasRef().getType()));
      outcols.setRadialVelocityRef(MRadialVelocity::castType(incols.sysvelMeas().getMeasRef().getType()));

      const ROScalarColumn<Int>& inSId   = incols.sourceId();
      ScalarColumn<Int>& 	 outSId  = outcols.sourceId();
      const ROScalarColumn<Int>& inSPW   = incols.spectralWindowId();
      ScalarColumn<Int>& 	 outSPW  = outcols.spectralWindowId();

      // 2009-06-09: It is hard to say whether to remap pulsarID when the
      // PULSAR table is not described in the MS v2.0 def'n.
//       const ROScalarColumn<Int>& inPId   = incols.pulsarId();
//       ScalarColumn<Int>& 	 outPId  = outcols.pulsarId();

      uInt outrn = 0; 		   	   		// row number in output.
      uInt nInputRows = inSId.nrow();
      for(uInt inrn = 0; inrn < nInputRows; ++inrn){
	Int inSidVal = inSId(inrn);
	Int inSPWVal = inSPW(inrn);  // -1 means the source is valid for any SPW.
	
	if(inSidVal > -1 && sourceRelabel_p[inSidVal] > -1
           && (inSPWVal == -1 || spwRelabel_p[inSPWVal] > -1)){
	  // Copy inrn to outrn.
	  TableCopy::copyRows(newSource, oldSource, outrn, inrn, 1);
	  outSId.put(outrn, sourceRelabel_p[inSidVal]);
	  outSPW.put(outrn, inSPWVal > -1 ? spwRelabel_p[inSPWVal] : -1);
	  
	  ++outrn;
	}
      }

      return True;
    }
  
    return False;
  }
  
  Bool SubMS::copyObservation()
  {  
    const MSObservation& oldObs = mssel_p.observation();
    MSObservation& newObs = msOut_p.observation();
    const ROMSObservationColumns oldObsCols(oldObs);
    MSObservationColumns newObsCols(newObs);
    newObsCols.setEpochRef(MEpoch::castType(oldObsCols.releaseDateMeas().getMeasRef().getType()));

    TableCopy::copyRows(newObs, oldObs);
    //W TableCopy::deepCopy(newObs, oldObs, false);
    
    return True;
  }
  
  Bool SubMS::copyPointing(){
    //Pointing is allowed to not exist
    Bool pointExists=Table::isReadable(mssel_p.pointingTableName());
    if(pointExists){
      //Wconst Table oldPoint(mssel_p.pointingTableName(), Table::Old);
      const MSPointing& oldPoint = mssel_p.pointing();

      if(oldPoint.nrow() > 0){
	MSPointing& newPoint = msOut_p.pointing();  // Could be declared as
						    // Table&

	LogIO os(LogOrigin("SubMS", "copyPointing()"));

	// Add optional columns if present in oldPoint.
	Vector<String> optionalCols(6);
	optionalCols[0] = "POINTING_OFFSET";
	optionalCols[1] = "SOURCE_OFFSET";
	optionalCols[2] = "ENCODER";
	optionalCols[3] = "POINTING_MODEL_ID";
	optionalCols[4] = "ON_SOURCE";
	optionalCols[5] = "OVER_THE_TOP";
	uInt nAddedCols = addOptionalColumns(oldPoint, newPoint, optionalCols, true);
	os << LogIO::DEBUG1 << "POINTING has " << nAddedCols
	   << " optional columns." << LogIO::POST;
	
	// W = Works, DW = Doesn't Work
//DW  	msOut_p.pointing() = mssel_p.pointing();	
//DW  	//TableCopy::copyInfo(newPoint, oldPoint);
//W  	TableColumn newTC(newPoint, "DIRECTION");
//W  	const ROScalarColumn<MDirection> oldTC(oldPoint, "DIRECTION");
//W  	const TableColumn oldTC(oldPoint, "DIRECTION");
//W  	newTC.rwKeywordSet() = oldTC.keywordSet();

  	const ROMSPointingColumns oldPCs(oldPoint);
	MSPointingColumns newPCs(newPoint);
	newPCs.setEpochRef(MEpoch::castType(oldPCs.timeMeas().getMeasRef().getType()));
	newPCs.setDirectionRef(MDirection::castType(oldPCs.directionMeasCol().getMeasRef().getType()));
	newPCs.setEncoderDirectionRef(MDirection::castType(oldPCs.encoderMeas().getMeasRef().getType()));

	
	if(!antennaSel_p){
	  TableCopy::copyRows(newPoint, oldPoint);
	}
	else{
	  const ROScalarColumn<Int>& antIds  = oldPCs.antennaId();
	  ScalarColumn<Int>& 	     outants = newPCs.antennaId();

	  uInt selRow = 0;
	  for (uInt k = 0; k < antIds.nrow(); ++k){
	    Int newAntInd = antNewIndex_p[antIds(k)];
	    
	    if(newAntInd > -1){
	      TableCopy::copyRows(newPoint, oldPoint, selRow, k, 1);
	      outants.put(selRow, newAntInd);
	      ++selRow;
	    }
	  }
	}
//DW 	//	TableCopy::copySubTables(newPoint, oldPoint);
//DW	oldPoint.deepCopy(msOut_p.pointingTableName(), Table::NewNoReplace);
      }
    }
    return True;
  }
  
  Bool SubMS::writeDiffSpwShape(const Vector<String>& columnNames)
  {  
    Bool doSpWeight = !(mscIn_p->weightSpectrum().isNull()) &&
                      mscIn_p->weightSpectrum().isDefined(0);
    
    Int rowsdone=0;
    Int rowsnow=0;
    Block<Int> sort(0);
    Matrix<Int> noselection;
    Bool idelete;
    
    VisSet vs(mssel_p, noselection);
    ROVisIter& vi(vs.iter());
    VisBuffer vb(vi);
    Vector<Int> spwindex(max(spw_p)+1);
    spwindex.set(-1);
    for (uInt k=0; k < spw_p.nelements() ; ++k){
      spwindex[spw_p[k]]=k;
    }
    
    Cube<Complex> vis;
    Cube<Bool> inFlag;
    Cube<Float> inSpWeight;
    Matrix<Bool> chanFlag;
    Cube<Float> spWeight;
    Matrix<Float> inImgWts;
    Cube<Bool> locflag;

    const uInt ntok = columnNames.nelements();
    for (vi.originChunks();vi.moreChunks();vi.nextChunk()) 
      {
	for (vi.origin(); vi.more(); vi++) 
	  {
	    Int spw=spwindex[vb.spectralWindow()];
	    rowsnow=vb.nRow();
	    RefRows rowstoadd(rowsdone, rowsdone+rowsnow-1);
	    //	    Cube<Bool> locflag(npol_p[spw], nchan_p[spw], rowsnow);
	    Matrix<Float> avgImgWts(nchan_p[spw],rowsnow);
	    inImgWts.reference(vb.imagingWeight());
	    avgImgWts.set(0.0);
	    Float *oproxyImgWtsP = avgImgWts.getStorage(idelete);
	    Float *iproxyImgWtsP = inImgWts.getStorage(idelete);

            Cube<Complex> averdata(npol_p[spw], nchan_p[spw], rowsnow);
		
	    for(uInt ni=0;ni<ntok;ni++)
	      {
		if(columnNames[ni]== MS::columnName(MS::DATA))
		  vis.reference(vb.visCube());
		else if(columnNames[ni] == MS::columnName(MS::MODEL_DATA))
		  vis.reference(vb.modelVisCube());
		else
		  vis.reference(vb.correctedVisCube());
		
		chanFlag.reference(vb.flag());
		inFlag.reference(vb.flagCube());
                inSpWeight.reference(vb.weightSpectrum());
		averdata.set(Complex(0.0, 0.0));
		
		if (doSpWeight){
		  spWeight.resize(npol_p[spw], nchan_p[spw], rowsnow);
                  spWeight.set(0.0);
                }
		
		locflag.resize(npol_p[spw], nchan_p[spw], rowsnow);
                locflag.set(false);
		const Bool* iflag=inFlag.getStorage(idelete);
		const Complex* idata=vis.getStorage(idelete);
		const Float* iweight=inSpWeight.getStorage(idelete);
		Complex* odata=averdata.getStorage(idelete);
		Bool* oflag=locflag.getStorage(idelete);		
		Float* oSpWt = spWeight.getStorage(idelete);
		
		for(Int r=0; r < rowsnow; ++r)
		  {
		    for(Int c=0; c < nchan_p[spw]; ++c)
		      {
			for (Int pol=0; pol < npol_p[spw]; ++pol)
			  {
			    Int outoffset=r*nchan_p[spw]*npol_p[spw]+c*npol_p[spw]+pol;
			    if(!averageChannel_p)
			      {
				Int whichChan=chanStart_p[spw]+ c*chanStep_p[spw];
				averdata.xyPlane(r).column(c) = vis.xyPlane(r).column(whichChan); 
				locflag.xyPlane(r).column(c)  = inFlag.xyPlane(r).column(whichChan);
                                spWeight.xyPlane(r).column(c) = inSpWeight.xyPlane(r).column(whichChan);
			      }
			    else{
                              if(doSpWeight){
                                for(Int m = 0; m < chanStep_p[spw]; ++m){
                                  Int inoffset=r*inNumChan_p[spw]*npol_p[spw]+
                                    (c*chanStep_p[spw]+m)*npol_p[spw]+ pol;
				    
                                  if(!iflag[inoffset]){
                                    odata[outoffset] += iweight[inoffset] *
                                                        idata[inoffset];
                                    oSpWt[outoffset] += iweight[inoffset];
                                  }
                                }
                                if(oSpWt[outoffset] != 0.0)
                                  odata[outoffset] /= oSpWt[outoffset];
                                else
                                  oflag[outoffset] = True;
                              }
                              else{
                                uInt counter = 0;
                                
                                for(Int m = 0; m < chanStep_p[spw]; ++m){
                                  Int inoffset=r*inNumChan_p[spw]*npol_p[spw]+
                                    (c*chanStep_p[spw]+m)*npol_p[spw]+ pol;
				    
                                  if(!iflag[inoffset]){
                                    odata[outoffset] += idata[inoffset];
                                    ++counter;
                                  }
                                }
                                if(counter > 0)
				  odata[outoffset] /= counter;
                                else
				  oflag[outoffset] = True;
				  // odata[outoffset]=0; // It's 0 anyway.
                              }
                            }
                          }
		      }
		  }  
		
		if (ntok==1)
		  msc_p->data().putColumnCells(rowstoadd, averdata);
		else
		  {
		    if(columnNames[ni]== MS::columnName(MS::DATA))
		      msc_p->data().putColumnCells(rowstoadd, averdata);
		    if (columnNames[ni]==MS::columnName(MS::MODEL_DATA))
		      msc_p->modelData().putColumnCells(rowstoadd, averdata);
		    else
		      msc_p->correctedData().putColumnCells(rowstoadd, averdata);
		  }
	      } // End ntok loop

	    //
	    //----------------------------------------------------------------------
	    // Average the imaging weights - if required.
	    //
	    if (doImgWts_p)
	      {
		for(Int r=0; r < rowsnow; ++r)
		  for(Int c=0; c < nchan_p[spw]; ++c)
		    {
		      Int outImgWtsoffset=r*nchan_p[spw]+c,
			imgWtsCounter=0,
			whichChan=chanStart_p[spw]+ c*chanStep_p[spw];
		      if(!averageChannel_p)
			avgImgWts(r,c) = inImgWts(r,whichChan);
		      else
			for (Int m=0; m < chanStep_p[spw]; ++m)
			  {
			    Int inImgWtsoffset=r*inNumChan_p[spw]+(c*chanStep_p[spw]+m);
			    oproxyImgWtsP[outImgWtsoffset] += iproxyImgWtsP[inImgWtsoffset];
			    imgWtsCounter++;
			  }
		      
		      if(averageChannel_p)
			if(imgWtsCounter >0)
			  oproxyImgWtsP[outImgWtsoffset] /= imgWtsCounter;
			else
			  oproxyImgWtsP[outImgWtsoffset]=0;
		    }
		msc_p->imagingWeight().putColumnCells(rowstoadd,avgImgWts);
	      }
	    //----------------------------------------------------------------------

	    msc_p->flag().putColumnCells(rowstoadd, locflag);
            msc_p->weightSpectrum().putColumnCells(rowstoadd, spWeight);
	    rowsdone += rowsnow;

	  }
      }
    
    return True;
  }
  
Bool SubMS::writeSimilarSpwShape(const Vector<String>& columnNames){
  Int nrow=mssel_p.nrow();

  Bool deleteIptr,  deleteIWptr;
  Bool deleteIFptr;
  Matrix<Complex> indatatmp(npol_p[0], inNumChan_p[0]);
  const Complex *iptr = indatatmp.getStorage(deleteIptr);
  Matrix<Bool> inflagtmp(npol_p[0], inNumChan_p[0]);
  const Bool *iflg = inflagtmp.getStorage(deleteIFptr);
  Vector<Complex> outdatatmp(npol_p[0]);
  //    const Complex *optr = outdatatmp.getStorage(deleteOptr);
  Matrix<Float> inwgtspectmp(npol_p[0], inNumChan_p[0]);
  const Float *inwptr = inwgtspectmp.getStorage(deleteIWptr);
  Vector<Float> outwgtspectmp(npol_p[0]);
  //   const Float *owptr = outwgtspectmp.getStorage(deleteOWptr);
  
  Bool doSpWeight = !mscIn_p->weightSpectrum().isNull() &&
                    mscIn_p->weightSpectrum().isDefined(0);

  Cube<Complex> outdata(npol_p[0], nchan_p[0], nrow);
  Cube<Bool> outflag(npol_p[0], nchan_p[0], nrow);
  outflag.set(false);

  Cube<Float> outspweight;
    
  const uInt ntok = columnNames.nelements();
  ROArrayColumn<Complex> data;
  ROArrayColumn<Float> wgtSpec;

  ROArrayColumn<Bool> flag(mscIn_p->flag());
  for(uInt colind = 0; colind < ntok; colind++){
    data.reference(right_column(mscIn_p, columnNames[colind]));
    if(doSpWeight){ 
      outspweight.resize(npol_p[0], nchan_p[0], nrow);
      wgtSpec.reference(mscIn_p->weightSpectrum());
    }

    for (Int row = 0; row < nrow; ++row){
      data.get(row, indatatmp);
      flag.get(row, inflagtmp);

      if(doSpWeight)
        wgtSpec.get(row, inwgtspectmp);

      Int ck = 0;
      Int chancounter = 0;
      Vector<Int> avcounter(npol_p[0]);
      outdatatmp.set(0); outwgtspectmp.set(0);
      avcounter.set(0);
      
      for(Int k = chanStart_p[0]; k < (nchan_p[0] * chanStep_p[0] +
                                       chanStart_p[0]); ++k){
        if(chancounter == chanStep_p[0]){
          outdatatmp.set(0); outwgtspectmp.set(0);
          chancounter = 0;
          avcounter.set(0);
        }
        ++chancounter;
        for(Int j = 0; j < npol_p[0]; ++j){
          Int offset = j + k * npol_p[0];
          if(!iflg[offset]){
            if(doSpWeight){
              outdatatmp[j] += iptr[offset] * inwptr[offset];
              outwgtspectmp[j] += inwptr[offset];
            }
            else
              outdatatmp[j] += iptr[offset];	   
            ++avcounter[j];
          }

          if(chancounter == chanStep_p[0]){
            if(avcounter[j] != 0){
              // Should there be a warning if one data column wants flagging
              // and another does not?
              if(doSpWeight){
                if(outwgtspectmp[j] != 0.0)
                  outdata(j, ck, row) = outdatatmp[j] / outwgtspectmp[j];
                else{
                  outdata(j, ck, row) = 0.0;
                  outflag(j, ck, row) = True;
                }
                outspweight(j, ck, row) = outwgtspectmp[j];
              }
              else{
                outdata(j, ck, row) = outdatatmp[j] / avcounter[j];	    
              }
            }
            else{
              outdata(j, ck, row) = 0;
              outflag(j, ck, row) = True;
              if(doSpWeight)
                outspweight(j, ck, row) = 0;
            }	
          }
        }
        if(chancounter == chanStep_p[0])
          ++ck;
      }
    }
    
    putDataColumn(*msc_p, outdata, columnNames[colind], (ntok == 1));
  }
  
  msc_p->flag().putColumn(outflag);
  if(doSpWeight)
    msc_p->weightSpectrum().putColumn(outspweight);

  //-----------------------------------------------------------------------------
  // Write the IMAGING_WEIGHTs column as well if required.
  if(doImgWts_p){
    //      ROArrayColumn<Bool> rowFlag(mscIn_p->rowFlags());
    ROArrayColumn<Float> inImgWts(mscIn_p->imagingWeight());

    Float outImgWtsAccum = 0;
    Vector<Float> inImgWtsSpectrum;
    Matrix<Float> outImgWts(nchan_p[0], nrow);
    Vector<Bool> rowFlag(mscIn_p->flagRow().getColumn());

    for(Int row = 0; row < nrow; ++row){
      inImgWts.get(row, inImgWtsSpectrum);

      Int ck = 0;
      Int chancounter = 0;
      Int counter = 0;
      outdatatmp = 0; 

      Int chanStop = nchan_p[0] * chanStep_p[0] + chanStart_p[0];
      for (Int c = chanStart_p[0]; c < chanStop; c++){
        if(chancounter == chanStep_p[0]){
          chancounter = 0;
          counter = 0;
        }
        chancounter++;
        //	      Int offset= p + c*npol_p[0];
        if(!rowFlag(row)){
          outImgWtsAccum += inImgWtsSpectrum(c);
          counter++;

          if(chancounter == chanStep_p[0])
            if(counter != 0)
              outImgWts(ck, row) = outImgWtsAccum/counter;
        }
        if(chancounter == chanStep_p[0])
          ck++;
      }
    }
    msc_p->imagingWeight().putColumn(outImgWts);
  }
  //-----------------------------------------------------------------------------
  return True;
}

const ROArrayColumn<Complex>& SubMS::right_column(const ROMSColumns *msclala,
						  const String& colName)
{
  const String myColName(upcase(colName));
  
  if(myColName == MS::columnName(MS::DATA))
    return msclala->data();
  else if(myColName == MS::columnName(MS::MODEL_DATA))
    return msclala->modelData();
  else
    return msclala->correctedData();
}

//   Int SubMS::numOfBaselines(Vector<Int>& ant1, Vector<Int>& ant2, 
// 			    Bool includeAutoCorr)
//   {
//     fillAnt(mscIn_p->antenna1().getColumn(), ant1, nant1, maxant1, ant1Indexer);
//     fillAnt(mscIn_p->antenna2().getColumn(), ant2, nant2, maxant2, ant2Indexer);
    
//     Int numBasl=0;
//     Bool hasAuto = ((nant1 == nant2) && allEQ(ant1, ant2));
    
//     //For now we are splitting data without autocorrelation
    
//     if(static_cast<uInt>(nant2 / 2.0) * 2 != nant2)
//       numBasl = hasAuto ? (nant2 - 1) / 2 * nant1 : (nant2 + 1) / 2 * nant1;
//     else if(static_cast<uInt>(nant1 / 2.0) * 2 != nant1)
//       numBasl = hasAuto ? (nant1 - 1) / 2 * nant2 : (nant1 + 1) / 2 * nant2;
//     else
//       numBasl = hasAuto ?  nant1 * nant2 / 2 : (nant1 + 1) * nant2 / 2;
    
//     return numBasl;   
//   }

// Sets mapper to the distinct values of mscol, in increasing order.
// A static method that is used by SubMS, but doesn't necessarily have to go
// with it.  It may belong in something more MSColumnsish.
void SubMS::make_map(const Vector<Int>& mscol, Vector<Int>& mapper)
{
  std::set<Int> valSet;
  
  for(uInt i = mscol.nelements() - 1; i--;)  // Strange, but slightly more
                                        // efficient than going forward.
    valSet.insert(mscol[i]);

  mapper.resize(valSet.size());

  uInt remaval = 0;
  for(std::set<Int>::const_iterator vs_iter = valSet.begin();
      vs_iter != valSet.end(); ++vs_iter){
    mapper[remaval] = *vs_iter;
    ++remaval;
  }
}

// Returns rv s.t. mapper[rv] == ov, assuming that mapper[i + 1] >= mapper[i]
// for 0 <= i < mapper.nelements() - 1.
// i can be supplied as the first guess.
uInt SubMS::remapped(const Int ov, const Vector<Int>& mapper, uInt i=0)
{
  uInt nvals = mapper.nelements();
  uInt nvalsm1 = nvals - 1;

  if(i >= nvals)	// Second guess the guess.
    i = nvals >> 1;
  
  // Do a widening search for the brackets, since i is probably close to the
  // answer.
  if(mapper[i] == ov)
    return i;
  
  uInt inc = 1;
  uInt lb = i;
  uInt ub;
  if(ov > mapper[lb]){	// Hunt up.
    if(lb == nvalsm1)
      return lb;
    ub = lb + 1;
    while(ov >= mapper[ub]){
      lb = ub;
      inc += inc;
      ub = lb + inc;
      if(ub > nvalsm1){
	ub = nvals;
	break;
      }
    }
  }
  else{			// Hunt down.
    ub = lb;
    --lb;
    while(ov < mapper[lb]){
      ub = lb;
      inc += inc;
      if(inc >= ub){
	lb = 0;
	break;
      }
      else
	lb = ub - inc;
    }
  }
        
  i = lb + ((ub - lb) >> 1);			// (lb + ub) / 2 might overflow. 
  while(mapper[i] != ov && ub - lb > 1){
    if(mapper[i] < ov)
      lb = i;
    else
      ub = i;
    i = lb + ((ub - lb) >> 1);
  }
  return i;  
}

// Maps the properties of a MS row to a unique key for a slot (= rows within a
// bin that should be averaged together).  The vector of the properties is
// mapped to a uInt, at some risk of overflow, so that later the key can be mapped to
// an output row number in the bin.
//
// The order of the properties is a convention, but it is arranged so that in
// the output ant2 will be the fastest varying index, ant1 the next fastest,
// and so on.
//
// Some properties are deliberately not included here because they are
// redundant for this purpose, and it is important to minimize the number of
// factors going into the key.  
// If this ignored # changes, => >= 1 of these watched ones changes.
// -------------------------     -----------------------------------
//           feed                  field, polarization, or spw
//	obs, array			antenna, spw, field
//       processor                       Should I care?
//
// The mapping also remaps (if necessary) each property to a range from 0 to max - 1,
// assuming that the corresponding remapper vectors have already been set up!
uInt SubMS::rowProps2slotKey(const Int ant1, const Int ant2,
			     const Int dd,   const Int field,
			     const Int scan, const Int state,
                             const uInt array)
{
  // Fastest slotnum (farthest apart), but slowest index.
  uInt slotKey = remapped(array, arrayRemapper_p, array);
  
  slotKey *= stateRemapper_p.nelements();
  slotKey += remapped(state, stateRemapper_p, state);

  slotKey *= scanRemapper_p.nelements();                // Must be set before calling
  slotKey += remapped(scan, scanRemapper_p, abs(scan)); // rowProps2slotKey().

  slotKey *= fieldid_p.nelements();
  slotKey += fieldRelabel_p[field];

  slotKey *= spw_p.nelements();  // ndds;
  slotKey += spwRelabel_p[oldDDSpwMatch_p[dd]];

  slotKey *= nant_p;            // Must be set before calling rowProps2slotKey().
  slotKey += antIndexer_p[ant1];

  slotKey *= nant_p;   // Must be set before calling rowProps2slotKey().

  // Slowest slotnum (closest together), but fastest index.
  slotKey += antIndexer_p[ant2];

  return slotKey;
}

  Int SubMS::numOfTimeBins(const Double timeBin)
  {
    if(timeBin > 0.0){
      Int numrows = mssel_p.nrow();
      const Vector<Int>&    ant1         = mscIn_p->antenna1().getColumn();
      const Vector<Int>&    ant2         = mscIn_p->antenna2().getColumn();
      const Vector<Double>& timeRows     = mscIn_p->time().getColumn();
      const Vector<Double>& intervalRows = mscIn_p->interval().getColumn();
      const Vector<Int>&    datDesc      = mscIn_p->dataDescId().getColumn();
      const Vector<Int>&    fieldId      = mscIn_p->fieldId().getColumn();
      const Vector<Int>&    arrayIDs     = mscIn_p->arrayId().getColumn();
      const Vector<Int>&    scan         = mscIn_p->scanNumber().getColumn();
      const Vector<Int>&    state        = mscIn_p->stateId().getColumn();
      const Vector<Bool>&   rowFlag      = mscIn_p->flagRow().getColumn();

      //std::set<Int> slotSet;
      
      GenSortIndirect<Double>::sort(tOI_p, timeRows);

      newTimeVal_p.resize(numrows);
      bin_slots_p.resize(numrows);

      make_map(arrayIDs, arrayRemapper_p);
      make_map(scan,     scanRemapper_p);  // This map is only implicitly used.
      make_map(state,    stateRemapper_p);

      Int numBin = 0;

      // A (potentially large) batch of flagged rows at the start is an
      // annoyingly common case that must be dealt with.
      uInt start_k = 0;
      while(start_k < uInt(numrows) && rowFlag[tOI_p[start_k]])
	++start_k;

      uInt   oldtoik   = tOI_p[start_k];      
      Double startoftb = timeRows[oldtoik] - 0.5 * intervalRows[oldtoik];
      Double endoftb   = startoftb + timeBin;

      newTimeVal_p[0] = 0.5 * (startoftb + endoftb);
      numOutRows_p = 0;
      for(uInt k = start_k; k < uInt(numrows); ++k){
	uInt toik = tOI_p[k];

	if(!rowFlag[toik]){
	  Double time_toik = timeRows[toik];
	  
	  if(time_toik >= endoftb){	// Start a new bin.
	    // Finish the old one
	    newTimeVal_p[numBin] = 0.5 * (startoftb +
					  timeRows[oldtoik] +
					  0.5 * intervalRows[oldtoik]);
	    startoftb = time_toik - 0.5 * intervalRows[toik];
	    endoftb   = startoftb + timeBin;
	  
	    if(bin_slots_p[numBin].size() > 0){
	      numOutRows_p += bin_slots_p[numBin].size();
	      ++numBin;
	    }
	  }
	  bin_slots_p[numBin][rowProps2slotKey(ant1[toik], ant2[toik],
					       datDesc[toik], fieldId[toik],
					       scan[toik],	// Don't remap!
					       state[toik],
                                               arrayIDs[toik])].push_back(toik);
	  oldtoik = toik;
	}
      }
      
      // Finish the last bin.
      if(bin_slots_p[numBin].size() > 0){
	newTimeVal_p[numBin] = 0.5 * (startoftb + timeRows[oldtoik] +
				      0.5 * intervalRows[oldtoik]);
	numOutRows_p += bin_slots_p[numBin].size();
	++numBin;
      }

      newTimeVal_p.resize(numBin, true);
      bin_slots_p.resize(numBin, true);
      return numBin;
    }
    return -1;
  }

uInt SubMS::fillAntIndexer(const ROMSColumns *msc, Vector<Int>& antIndexer)
{
  const Vector<Int>& ant1 = msc->antenna1().getColumn();
  const Vector<Int>& ant2 = msc->antenna2().getColumn();

  std::set<Int> ants;
  uInt endrow = ant1.nelements() - 1;
  for(uInt i = endrow; i--;){   // Strange, but slightly more
    ants.insert(ant1[i]);	// efficient than going forward.
    ants.insert(ant2[i]);
  }
  uInt nant = ants.size();
  
  Vector<Int> selAnt(nant);
  Int remaval = 0;
  for(std::set<Int>::const_iterator ant_iter = ants.begin();
      ant_iter != ants.end(); ++ant_iter){
    selAnt[remaval] = *ant_iter;
    ++remaval;
  }
    
  antIndexer.resize(max(selAnt) + 1);
  antIndexer = -1;
  for(uInt j = 0; j < nant; ++j)
    antIndexer[selAnt[j]] = static_cast<Int>(j);
  return nant;
}
  
Bool SubMS::fillTimeAverData(const Vector<String>& columnNames)
{
  LogIO os(LogOrigin("SubMS", "fillTimeAverData()"));

  //No channel averaging with time averaging ... it's better this way.
  if(chanStep_p[0] > 1){
    throw(AipsError("Simultaneous time and channel averaging is not handled."));
    return False;
  }

  Int outNrow = msOut_p.nrow();

  const uInt ntok = columnNames.nelements();

  //Vector<ROArrayColumn<Complex> > data(ntok);
  ROArrayColumn<Complex> data[ntok];

  const ROArrayColumn<Float> inImagingWeight(mscIn_p->imagingWeight());
  Vector<Float> inImgWtsSpectrum;

  //const ROScalarColumn<Double> time(mscIn_p->time());
  const ROScalarColumn<Double> inTC(mscIn_p->timeCentroid());
  const ROScalarColumn<Double> inExposure(mscIn_p->exposure());

  ROArrayColumn<Float> wgtSpec;
    
  const ROScalarColumn<Int> ant1(mscIn_p->antenna1());
  const ROScalarColumn<Int> ant2(mscIn_p->antenna2());
  const ROScalarColumn<Int> inFeed1(mscIn_p->feed1());
  const ROScalarColumn<Int> inFeed2(mscIn_p->feed2());
  const ROScalarColumn<Int> fieldID(mscIn_p->fieldId());
  const ROScalarColumn<Int> state(mscIn_p->stateId());

  const ROScalarColumn<Int> inProc(mscIn_p->processorId());
  Vector<Int> procMapper;
  make_map(inProc.getColumn(), procMapper);

  const ROScalarColumn<Int> inObs(mscIn_p->observationId());
  Vector<Int> obsMapper;
  make_map(inObs.getColumn(), obsMapper);

  const ROScalarColumn<Int> inArr(mscIn_p->arrayId());

  const ROArrayColumn<Bool> flag(mscIn_p->flag());

  // Flagged rows have already been excluded from the bins, so there is no
  // need to worry about them here.
  //const ROScalarColumn<Bool> rowFlag(mscIn_p->flagRow());

  // ...but new row flags can be made for completely flagged or unweighted rows.
  Vector<Bool> outRowFlag(outNrow);
  outRowFlag.set(false);

  const ROScalarColumn<Int> scanNum(mscIn_p->scanNumber());
  const ROScalarColumn<Int> dataDescIn(mscIn_p->dataDescId());
  const ROArrayColumn<Double> inUVW(mscIn_p->uvw());
 
  for(uInt datacol = 0; datacol < ntok; ++datacol)
    data[datacol].reference(right_column(mscIn_p, columnNames[datacol]));
  os << LogIO::NORMAL << "Writing time averaged data of "
     << newTimeVal_p.nelements()<< " time slots" << LogIO::POST;

  const Bool doSpWeight = !mscIn_p->weightSpectrum().isNull() &&
                           mscIn_p->weightSpectrum().isDefined(0) &&
    mscIn_p->weightSpectrum().shape(0).isEqual(IPosition(2, npol_p[0],
                                                         nchan_p[0]));
    
  
  Vector<Double> outTime;
  outTime.resize(outNrow);

  Vector<Double> outTC;
  outTC.resize(outNrow);
  outTC.set(0.0);

  Vector<Double> outExposure;
  outExposure.resize(outNrow);
  outExposure.set(0.0);

  //Vector<Cube<Complex> > outData(ntok);
  Cube<Complex> outData[ntok];
  for(uInt datacol = 0; datacol < ntok; ++datacol){
    outData[datacol] = Cube<Complex>(npol_p[0], nchan_p[0], outNrow);
    outData[datacol].set(0.0);
  }
  
  Matrix<Float> outRowWeight(npol_p[0], outNrow);
  outRowWeight.set(0.0);

  Matrix<Float> outImagingWeight(doImgWts_p ? nchan_p[0] : 0,
				 doImgWts_p ? outNrow    : 0);
  if(doImgWts_p)
    outImagingWeight.set(0.0);

  Cube<Bool> outFlag(npol_p[0], nchan_p[0], outNrow);
  outFlag.set(True);

  Vector<Int> outAnt1(outNrow);
  outAnt1.set(-1);

  Vector<Int> outAnt2(outNrow);
  outAnt2.set(-1);

  Vector<Int> outFeed1(outNrow);
  outFeed1.set(-1);

  Vector<Int> outFeed2(outNrow);
  outFeed2.set(-1);

  Vector<Int> outScanNum(outNrow);
  outScanNum.set(0);

  Vector<Int> outProc(outNrow);
  outProc.set(0);

  Vector<Int> outObs(outNrow);
  outObs.set(0);

  Vector<Int> outArr(outNrow);
  outArr.set(0);

  Vector<Int> outField(outNrow);
  outField.set(0);

  Vector<Int> outState(outNrow);
  outState.set(0);

  Vector<Int> dataDesc(outNrow);
  dataDesc.set(-1);

  Cube<Float> outSpWeight;
  Vector<Float> outSpWtTmp(npol_p[0]);
  if(doSpWeight){
    outSpWeight.resize(npol_p[0], nchan_p[0], outNrow);
    outSpWeight.set(0.0);
    wgtSpec.reference(mscIn_p->weightSpectrum());
  }

  Double totrowwt;

  Matrix<Double> outUVW(3, outNrow);
  outUVW.set(0.0);

  Matrix<Float> outSigma(npol_p[0], outNrow);
  outSigma.set(0.0);

  const ROArrayColumn<Float> inRowWeight(mscIn_p->weight());
  os << LogIO::NORMAL << "outNrow = " << outNrow << LogIO::POST;
  os << LogIO::NORMAL << "inUVW.nrow() = " << inUVW.nrow() << LogIO::POST;
  //os << LogIO::NORMAL << "npol_p = " << npol_p << LogIO::POST;
  //os << LogIO::NORMAL << "nchan_p = " << nchan_p << LogIO::POST;

  IPosition blc, trc, sliceShape;

  // The real initialization is inside the loop - this just prevents a compiler warning.
  uInt chanStop = nchan_p[0] * chanStep_p[0] + chanStart_p[0];
  Array<Float> unflgWtSpec;
  Array<Complex> data_toikit;
  Vector<Float> unflaggedwt;
    
  // Iterate through timebins.
  uInt orn = 0; 		      // row number in output.
  // Guarantee oldDDID != ddID on 1st iteration.
  Int oldDDID = spwRelabel_p[oldDDSpwMatch_p[dataDescIn(bin_slots_p[0].begin()->second[0])]] - 1;
  for(uInt tbn = 0; tbn < bin_slots_p.nelements(); ++tbn){
    // Iterate through slots.
    for(ui2vmap::iterator slotit = bin_slots_p[tbn].begin();
        slotit != bin_slots_p[tbn].end(); ++slotit){
      uivector& slotv = slotit->second;
      uInt slotv0 = slotv[0];
      Double totslotwt = 0.0;

      Int ddID = spwRelabel_p[oldDDSpwMatch_p[dataDescIn(slotv0)]];
      Bool newDDID = (ddID != oldDDID);
      if(newDDID){
        oldDDID = ddID;

        if(ddID < 0){                      // Paranoia
          if(newDDID)
            os << LogIO::WARN
               << "Treating DATA_DESCRIPTION_ID " << ddID << " as 0."
               << LogIO::POST;
          ddID = 0;
        }
      
        blc = IPosition(2, 0, chanStart_p[ddID]);
        trc = IPosition(2, npol_p[ddID] - 1, nchan_p[ddID] + chanStart_p[ddID] - 1);
        chanStop = nchan_p[ddID] * chanStep_p[ddID] + chanStart_p[ddID];

        IPosition sliceShape(trc - blc + 1);
        if(doSpWeight)
          unflgWtSpec.resize(sliceShape);
        unflaggedwt.resize(npol_p[ddID]);
        data_toikit.resize(sliceShape);
        os << LogIO::DEBUG1
           << "sliceShape = " << sliceShape
           << LogIO::POST;
      }

      // Iterate through mscIn_p's rows that belong to the slot.
      Double swv = 0.0; // Sum of the weighted visibilities.
      outUVW.column(orn).set(0.0);
      for(uivector::iterator toikit = slotv.begin();
          toikit != slotv.end(); ++toikit){
        // chanModification_p==true means the input channels cannot simply
        // be copied through to the output channels.
        // It's a bit faster if no slicing is done...so avoid it if possible.

        // Set flagged weights to 0 in the unflagged weights...
        unflaggedwt = inRowWeight(*toikit);
        for(Int polind = 0; polind < npol_p[ddID]; ++polind){
          if(allTrue(flag(*toikit)(IPosition(2, polind, chanStart_p[ddID]),
                                   IPosition(2, polind,
                                             nchan_p[ddID] + chanStart_p[ddID] - 1))))
            unflaggedwt[polind] = 0.0;
        }
        if(doSpWeight){
          if(chanModification_p){
            unflgWtSpec = wgtSpec(*toikit)(blc, trc);
            unflgWtSpec(flag(*toikit)(blc, trc)) = 0.0;
          }
          else{
            unflgWtSpec = wgtSpec(*toikit);
            unflgWtSpec(flag(*toikit)) = 0.0;
          }            
        }
        
        // The input row may be completely flagged even if the row flag is
        // false.
        totrowwt = sum(doSpWeight ? unflgWtSpec : unflaggedwt);
        if(totrowwt > 0.0){
          // Accumulate the averaging values from *toikit.
          //  	    os << LogIO::DEBUG1 << "inRowWeight(*toikit).shape() = "
          //  	       << inRowWeight(*toikit).shape() << LogIO::POST;
          //  	    os << LogIO::DEBUG1 << "unflaggedwt.shape() = "
          //  	       << unflaggedwt.shape() << LogIO::POST;
          //  	    os << LogIO::DEBUG1 << "flag(*toikit).shape() = "
          //  	       << flag(*toikit).shape() << LogIO::POST;
          //  	    os << LogIO::DEBUG1 << "data(*toikit).shape() = "
          //  	       << data(*toikit).shape() << LogIO::POST;

          outRowWeight.column(orn) = outRowWeight.column(orn) + unflaggedwt;
          
          totslotwt += totrowwt;
        
          if(chanModification_p)
            outFlag.xyPlane(orn) = outFlag.xyPlane(orn) * flag(*toikit)(blc,
                                                                        trc);
          else
            outFlag.xyPlane(orn) = outFlag.xyPlane(orn) * flag(*toikit);

          if(doSpWeight){
            //    os << LogIO::DEBUG1
            // //    << "wgtSpec(*toikit).shape() = " << wgtSpec(*toikit).shape()
            // //    << "\nflag(*toikit).shape() = " << flag(*toikit).shape()
            // //    << "outSpWeight.xyPlane(orn).shape() = "
            // //    << outSpWeight.xyPlane(orn).shape()
            //       << "\noutSpWeight.xyPlane(orn)(blc) (before) =\t"
            //       << outSpWeight.xyPlane(orn)(blc)
            //       << LogIO::POST;
            
            outSpWeight.xyPlane(orn) = outSpWeight.xyPlane(orn) + unflgWtSpec;
            
            // os << LogIO::DEBUG1
            //    << "outSpWeight.xyPlane(orn)(blc) (after) = "
            //    << outSpWeight.xyPlane(orn)(blc)
            //    << "\nunflgWtSpec(blc) = " << unflgWtSpec(blc)
            //    << "\nwgtSpec(*toikit)(blc) = " << wgtSpec(*toikit)(blc)
            //    << "\nflag(*toikit)(blc) = " << flag(*toikit)(blc)
            //    << LogIO::POST;
            for(uInt datacol = 0; datacol < ntok; ++datacol){
              if(chanModification_p){
                data_toikit = data[datacol](*toikit)(blc, trc) * unflgWtSpec;

                // It's already multiplied by a zero weight, but zero it again
                // just in case some flagged NaNs or infinities snuck in there.
                data_toikit(flag(*toikit)(blc, trc)) = 0.0;
              }
              else{
                data_toikit = data[datacol](*toikit) * unflgWtSpec;
                data_toikit(flag(*toikit)) = 0.0;
              }
              
              outData[datacol].xyPlane(orn) = outData[datacol].xyPlane(orn)
                + data_toikit;
            }
          }
          else{
            for(uInt datacol = 0; datacol < ntok; ++datacol){
              if(chanModification_p){
                data_toikit = data[datacol](*toikit)(blc, trc);
                data_toikit(flag(*toikit)(blc, trc)) = 0.0;
              }
              else{
                data_toikit = data[datacol](*toikit);
                data_toikit(flag(*toikit)) = 0.0;
              }
              binOpExpandInPlace(data_toikit, unflaggedwt,
                                 Multiplies<Complex, Float>());
            
              outData[datacol].xyPlane(orn) = outData[datacol].xyPlane(orn)
                + data_toikit;
            }
          }

          Double wv = 0.0;
          Array<Complex>::const_iterator dataEnd = data_toikit.end();
          for(Array<Complex>::const_iterator dit = data_toikit.begin();
              dit != dataEnd; ++dit)
            wv += fabs(*dit);
          if(wv > 0.0){
            swv += wv;
            outUVW.column(orn) = outUVW.column(orn)
              + (wv / swv) * (inUVW(*toikit) - outUVW.column(orn));
          }

          // totrowwt > 0.0 implies totslotwt > 0.0
          outTC[orn] += totrowwt * (inTC(*toikit) - outTC[orn]) / totslotwt;
          outExposure[orn] += totrowwt * inExposure(*toikit);
        
          if(doImgWts_p){
            inImagingWeight.get(*toikit, inImgWtsSpectrum);
          
            for(uInt c = chanStart_p[ddID]; c < chanStop; ++c)
              outImagingWeight(c, orn) += totrowwt * inImgWtsSpectrum[c];
          }
        }
      } // End of loop through the slot's rows.

      // If there were no weights > 0, plop in a reasonable values just for
      // appearance's sake.
      if(swv <= 0.0)
        outUVW.column(orn) = inUVW(slotv0);

      // Average the accumulated values.
      //totslotwt = sum(outRowWeight.column(orn));  // Uncomment for debugging
      if(totslotwt > 0.0){
        outExposure[orn] *= slotv.size() / totslotwt;
        if(doImgWts_p){
          for(uInt c = chanStart_p[ddID]; c < chanStop; ++c)
            outImagingWeight(c, orn) /= totslotwt;
        }
      }
      else{
        // outExposure[orn] = 0.0;      // Already is.
        // outImagingWeight(c, orn) = 0.0;  // Ditto.
        outTC[orn] = inTC(slotv0);      // Looks better than 1858.
        outRowFlag[orn] = true;
      }

      slotv.clear();  // Free some memory.
      if(doSpWeight){
        Matrix<Float>::const_iterator oSpWtIter;
        const Matrix<Float>::const_iterator oSpWtEnd(outSpWeight.xyPlane(orn).end());
          
        for(uInt datacol = 0; datacol < ntok; ++datacol){
          oSpWtIter = outSpWeight.xyPlane(orn).begin();            
          for(Matrix<Complex>::iterator oDIter = outData[datacol].xyPlane(orn).begin();
              oSpWtIter != oSpWtEnd; ++oSpWtIter){
            if(*oSpWtIter != 0.0)
              *oDIter /= *oSpWtIter;
            ++oDIter;
          }
        }
      }
      else{
        uInt npol  = outRowWeight.column(orn).nelements();
        uInt nchan = chanStop - chanStart_p[ddID];
        
        for(uInt polind = 0; polind < npol; ++polind){
          Float rowwtpol = outRowWeight.column(orn)[polind];
          for(uInt datacol = 0; datacol < ntok; ++datacol){

            if(rowwtpol != 0.0){
              for(uInt c = 0; c < nchan; ++c)
                outData[datacol](polind, c, orn) /= rowwtpol;
            }
            else{
              for(uInt c = 0; c < nchan; ++c)
                outData[datacol](polind, c, orn) = 0.0;
            }
          }
        }
      }

      for(uInt polind = 0; polind < outRowWeight.column(orn).nelements();
          ++polind){
        Float orw = outRowWeight(polind, orn);
        
        if(orw > 0.0)
          outSigma(polind, orn) = sqrt(1.0 / orw);
        else
          outSigma(polind, orn) = -1.0; // Seems safer than 0.0.
      }

      // Fill in the nonaveraging values from slotv0.
      // In general, _IDs which are row numbers in a subtable must be
      // remapped, and those which are not probably shouldn't be.
      outTime[orn]    = newTimeVal_p[tbn];
      outScanNum[orn] = scanNum(slotv0);	// Don't remap!
      if(antennaSel_p){
        outAnt1[orn]  = antIndexer_p[ant1(slotv0)];
        outAnt2[orn]  = antIndexer_p[ant2(slotv0)];
      }
      else{
        outAnt1[orn]  = ant1(slotv0);
        outAnt2[orn]  = ant2(slotv0);
      }		
      outFeed1[orn] = inFeed1(slotv0);
      outFeed2[orn] = inFeed2(slotv0);
      outField[orn]   = fieldRelabel_p[fieldID(slotv0)];
      outState[orn]   = remapped(state(slotv0), stateRemapper_p,
                                 abs(state(slotv0)));
      outProc[orn]    = remapped(inProc(slotv0), procMapper,
                                 abs(inProc(slotv0)));
      outObs[orn]     = remapped(inObs(slotv0), obsMapper,
                                 abs(inObs(slotv0)));
      outArr[orn]     = inArr(slotv0);	        // Don't remap!
      dataDesc[orn]   = spwRelabel_p[oldDDSpwMatch_p[dataDescIn(slotv0)]];

      ++orn;  // Advance the output row #.
    } // End of iterating through the bin's slots.
  }
  os << LogIO::NORMAL << "Data binned." << LogIO::POST; 

  bin_slots_p.resize(0);           // Free some memory

  msc_p->uvw().putColumn(outUVW);
  outUVW.resize();			// Free some memory

  msc_p->flag().putColumn(outFlag);
  outFlag.resize();
  msc_p->weight().putColumn(outRowWeight);
  outRowWeight.resize();
  msc_p->sigma().putColumn(outSigma);
  outSigma.resize();

  if(doImgWts_p){
    msc_p->imagingWeight().putColumn(outImagingWeight);
    outImagingWeight.resize();
  }

  msc_p->antenna1().putColumn(outAnt1);
  msc_p->antenna2().putColumn(outAnt2);
  msc_p->arrayId().putColumn(outArr);
  msc_p->dataDescId().putColumn(dataDesc);
  msc_p->exposure().putColumn(outExposure);
  msc_p->feed1().putColumn(outFeed1);
  msc_p->feed2().putColumn(outFeed2);
  msc_p->fieldId().putColumn(outField);

  msc_p->flagRow().putColumn(outRowFlag);

  // interval is done in fillAverMainTable().
  msc_p->observationId().putColumn(outObs);
  msc_p->processorId().putColumn(outProc);
  msc_p->scanNumber().putColumn(outScanNum);
  msc_p->stateId().putColumn(outState);
  msc_p->time().putColumn(outTime);
  msc_p->timeCentroid().putColumn(outTC);

  for(uInt datacol = 0; datacol < ntok; ++datacol)
    putDataColumn(*msc_p, outData[datacol], columnNames[datacol], (ntok == 1));

  if(doSpWeight)
    msc_p->weightSpectrum().putColumn(outSpWeight);
  return True;
}

inline Bool SubMS::checkSpwShape()
{
  return allSame(inNumChan_p) && allSame(nchan_p);
}

  Bool isAllColumns(const Vector<String>& colNames)
  {
    Bool dCol=False, mCol=False, cCol=False;
    for(uInt i=0;i<colNames.nelements();i++)
      {
	if (colNames[i]==MS::columnName(MS::DATA))           dCol=True;
	if (colNames[i]==MS::columnName(MS::MODEL_DATA))     mCol=True;
	if (colNames[i]==MS::columnName(MS::CORRECTED_DATA)) cCol=True;
      }
    return (dCol && mCol && cCol);
  }

} //#End casa namespace
