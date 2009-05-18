//# SubMS.cc 
//# Copyright (C) 1996-2007
//# Associated Universities, Inc. Washington DC, USA.
//#
//# This program is free software; you can redistribute it and/or modify
//# it under the terms of the GNU General Public License as published by
//# the Free Software Foundation; either version 2 of the License, or
//# (at your option) any later version.
//#
//# This program is distributed in the hope that it will be useful,
//# but WITHOUT ANY WARRANTY; without even the implied warranty of
//# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//# GNU General Public License for more details.
//# 
//# You should have received a copy of the GNU General Public License
//# along with this program; if not, write to the Free Software
//# Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//#
//# Correspondence concerning AIPS++ should be addressed as follows:
//#        Internet email: aips2-request@nrao.edu.
//#        Postal address: AIPS++ Project Office
//#                        National Radio Astronomy Observatory
//#                        520 Edgemont Road
//#                        Charlottesville, VA 22903-2475 USA
//#
//# $Id$
#include <msvis/MSVis/SubMS.h>
#include <ms/MeasurementSets/MSSelection.h>
#include <tables/Tables/ExprNode.h>
#include <tables/Tables/RefRows.h>
#include <ms/MeasurementSets/MSColumns.h>
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
#include <tables/Tables/TiledColumnStMan.h>
#include <tables/Tables/TiledShapeStMan.h>
#include <tables/Tables/TiledDataStMan.h>
#include <tables/Tables/TiledStManAccessor.h>
#include <ms/MeasurementSets/MSTileLayout.h>

#include <casa/sstream.h>

#include <functional>
#include <set>

namespace casa {
  
  SubMS::SubMS(String& theMS) :
    ms_p(MeasurementSet(theMS, Table::Update)),
    mssel_p(ms_p),
    msc_p(NULL),
    mscIn_p(NULL),
    doChanAver_p(False),
    antennaSel_p(False),
    sameShape_p(True),
    timeBin_p(-1.0),
    numOutRows_p(0),
    scanString_p(""),
    uvrangeString_p(""),
    taqlString_p(""),
    nant_p(0)
  {
  }
  
  SubMS::SubMS(MeasurementSet& ms) :
    ms_p(ms),
    mssel_p(ms_p),
    msc_p(NULL),
    mscIn_p(NULL),
    doChanAver_p(False),
    antennaSel_p(False),
    sameShape_p(True),
    timeBin_p(-1.0),
    numOutRows_p(0),
    scanString_p(""),
    uvrangeString_p(""),
    taqlString_p(""),
    nant_p(0)
  {
  }
  
  SubMS::~SubMS(){
    if(!msOut_p.isNull())
      msOut_p.flush();
    msOut_p=MeasurementSet();

    // parseColumnNames unavoidably has a static String and Vector<String>.
    // Collapse them down to free most of that memory.
    parseColumnNames("None");
  }
  
  
  void SubMS::selectSpw(Vector<Int> spw, Vector<Int> nchan, Vector<Int> start, 
			Vector<Int> step, const Bool averchan) {
    
    
    spw_p.resize();
    spw_p=spw;
    
    //check for default
    if(spw_p.nelements() == 1 && spw_p[0] < 0){
      spw_p.resize(ms_p.spectralWindow().nrow());
      for (uInt k =0 ; k < spw_p.nelements() ; ++k){
	spw_p[k]=k;
	
      }
      //no may be we have to redo the chan selection
      
      if (nchan.nelements() != spw_p.nelements()){
	nchan.resize(spw_p.nelements(), True);
	for(uInt k=1; k < spw_p.nelements(); ++k){
	  nchan[k]=nchan[0];
	}
      }
      if (start.nelements() != spw_p.nelements()){
	start.resize(spw_p.nelements(), True);
	for(uInt k=1; k < spw_p.nelements(); ++k){
	  start[k]=start[0];
	}
      }
      if (step.nelements() != spw_p.nelements()){
	step.resize(spw_p.nelements(), True);
	for(uInt k=1; k < spw_p.nelements(); ++k){
	  step[k]=step[0];
	}
      }
    }
    
    
    nchan_p.resize();
    nchan_p=nchan;
    chanStart_p.resize();
    chanStart_p=start;
    chanStep_p.resize();
    chanStep_p=step;
    averageChannel_p=averchan;
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
  
  void SubMS::setmsselect(const String& spw, const String& field, 
			  const String& baseline, 
			  const String& scan, const String& uvrange, 
			  const String& taql, const Vector<Int>& nchan, 
			  const Vector<Int>& start, const Vector<Int>& step,
			  const Bool averchan){
    Vector<Int> inchan(1,-1);
    Vector<Int> istart(1,0);
    Vector<Int> istep(1,1);
    Record selrec=ms_p.msseltoindex(spw, field);
    Vector<Int>fldids=selrec.asArrayInt("field");
    Vector<Int>spwids=selrec.asArrayInt("spw");
    if(fldids.nelements() < 1){
      fldids=Vector<Int>(1,-1);
    }
    selectSource(fldids);
    if(spwids.nelements() < 1){
      spwids=Vector<Int>(1,-1);
    }
    //use nchan f defined else use caret-column syntax of  msselection 
    if((nchan.nelements()>0) && nchan[0] > 0){
      inchan.resize(); inchan=nchan;
      istep.resize(); istep=step;
      istart.resize(); istart=start;
    }
    else{
      Matrix<Int> chansel=selrec.asArrayInt("channel");
      if(chansel.nelements() !=0){
	inchan.resize(chansel.nrow());
	istep.resize(chansel.nrow());
	istart.resize(chansel.nrow());
	// if the vector step is used ..for averaging ..let's use it
	Bool stepused=False;
	if( (step.nelements() >= 1) && (max(step) > 1))
	  stepused=True;
	for (uInt k =0 ; k < chansel.nrow(); ++k){
	  if(stepused){
	    if(step.nelements()==1)
	      istep[k]=step[0];
	    else if(step.nelements()==istep.nelements())
	      istep[k]=step[k];
	    else //confused at this stage
	      istep[k]=1;
	  }
	  else{
	    istep[k]=chansel.row(k)(3);
	    if(istep[k] < 1)
	      istep[k]=1;
	  }
	  istart[k]=chansel.row(k)(1);
	  inchan[k]=(chansel.row(k)(2)-istart[k]+1)/istep[k];
	  if(inchan[k]<1)
	    inchan[k]=1;	  
	}
      } 
    }
    selectSpw(spwids, inchan, istart, istep, averchan);
    
    if(baseline != ""){
      Vector<Int> antid(0);
      Vector<String> antstr(1,baseline);
      selectAntenna(antid, antstr);
    }
    scanString_p=scan;
    uvrangeString_p=uvrange;
    taqlString_p=taql;
    
  }
  
  void SubMS::selectSource(Vector<Int> fieldid){
    
    
    fieldid_p.resize();
    fieldid_p=fieldid;
    if(fieldid.nelements()==1 && fieldid(0)<0){
      fieldid_p.resize(ms_p.field().nrow());
      for (uInt k =0 ; k < fieldid_p.nelements() ; ++k){
	fieldid_p[k]=k;
	
      }
    }
    
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
  
  void SubMS::selectTime(Double timeBin, String timerng){
    
    timeBin_p=timeBin;
    timeRange_p=timerng;
  }
  
  
  Bool SubMS::makeSubMS(String& msname, String& colname){
    
    LogIO os(LogOrigin("SubMS", "makeSubMS()"));
    try{
      if ((fieldid_p.nelements()>0) && 
	  ((max(fieldid_p) >= Int(ms_p.field().nrow())))){
	os << LogIO::SEVERE 
	   << "Field selection contains elements that do not exist in "
	   << "this MS"
	   << LogIO::POST;
	ms_p=MeasurementSet();
	return False;
      }
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
      verifyColumns(ms_p, parseColumnNames(colname));

      if(!makeSelection()){
	os << LogIO::SEVERE 
	   << "Failed on selection: combination of spw and/or field and/or time chosen"
	   << " may be invalid." 
	   << LogIO::POST;
	ms_p=MeasurementSet();
	return False;
      }
      mscIn_p=new MSColumns(mssel_p);
      // Note again the verifyColumns() a few lines back that stops setupMS()
      // from being called if the MS doesn't have the requested columns.
      MeasurementSet* outpointer=setupMS(msname, nchan_p[0], npol_p[0],  
					 mscIn_p->observation().telescopeName()(0),
					 String(colname));
      
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
    mscIn_p=new MSColumns(mssel_p);
    Double sizeInMB= 1.5 * n_bytes() / (1024.0 * 1024.0);
    String msname=AppInfo::workFileName(uInt(sizeInMB), "TempSubMS");
    
    MeasurementSet* outpointer=setupMS(msname, nchan_p[0], npol_p[0],  
				       mscIn_p->observation().telescopeName()(0));
    
    outpointer->markForDelete();
    //Hmmmmmm....memory...... 
    if(sizeInMB <  (Double)(HostInfo::memoryTotal())/(2048.0) 
       || forceInMemory){
      MeasurementSet* a = outpointer;
      outpointer= new MeasurementSet(a->copyToMemoryTable("TmpMemoryMS"));
      outpointer->initRefs();
      delete a;
    }
    
    msOut_p= *outpointer;
    msc_p=new MSColumns(msOut_p);
    
    if(!fillAllTables(colname)){
      delete outpointer;
      outpointer=0;
      ms_p=MeasurementSet();
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
    
    // fill or update
    if(!fillDDTables()){
      return False;
    }
    fillFieldTable();
    copySource();
    copyAntenna();
    copyFeed();    // Feed table writing has to be after antenna 
    copyObservation();
    copyPointing();
    
    //check the spw shapes
    sameShape_p = checkSpwShape();
    
    if(timeBin_p <= 0.0){
      fillMainTable(colname);
    }
    else{
      if(!sameShape_p){
	os << LogIO::WARN 
	   << "Time averaging of varying spw shapes is not handled yet"
	   << LogIO::POST;
	os << LogIO::WARN
	   << "Work around: split-average different shape spw separately and then concatenate " << LogIO::POST;
	return False;
      }
      else{
	fillAverMainTable(colname);
	
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
      VisSet vs(ms_p,noselection);
    }
    const MeasurementSet sorted=ms_p.keywordSet().asTable("SORTED_TABLE");
    
    MSSelection thisSelection;
    if(fieldid_p.nelements() > 0)
      thisSelection.setFieldExpr(MSSelection::indexExprStr(fieldid_p));
    if(spw_p.nelements() > 0)
      thisSelection.setSpwExpr(MSSelection::indexExprStr(spw_p));
    if(antennaSel_p){
      if(antennaId_p.nelements() >0){
	thisSelection.setAntennaExpr( MSSelection::indexExprStr( antennaId_p ));
      }
      if(antennaSelStr_p[0] != ""){
        thisSelection.setAntennaExpr(MSSelection::nameExprStr( antennaSelStr_p));
	
	
      }
      
    }
    if(timeRange_p != ""){
      thisSelection.setTimeExpr(timeRange_p);
    }
    
    thisSelection.setUvDistExpr(uvrangeString_p);
    thisSelection.setScanExpr(scanString_p);
    thisSelection.setTaQLExpr(taqlString_p);
    
    TableExprNode exprNode=thisSelection.toTableExprNode(&sorted);
    
    {
      
      MSDataDescription ddtable=ms_p.dataDescription();
      ROScalarColumn<Int> polId(ddtable, 
				MSDataDescription::columnName(MSDataDescription::POLARIZATION_ID));
      MSPolarization poltable= ms_p.polarization();
      ROArrayColumn<Int> pols(poltable, 
			      MSPolarization::columnName(MSPolarization::CORR_TYPE));
      
      npol_p.resize(spw_p.shape()); 
      for (uInt k=0; k < npol_p.nelements(); ++k){  
	npol_p[k]=pols(polId(spw_p[k])).nelements();
      }
    }
    
    // Now remake the selected ms
    if(!(exprNode.isNull())){
      mssel_p =  MeasurementSet(sorted(exprNode));
    }
    else{
      // Null take all the ms ...setdata() blank means that
      mssel_p = MeasurementSet(sorted);
    }
    //mssel_p.rename(ms_p.tableName()+"/SELECTED_TABLE", Table::Scratch);
    if(mssel_p.nrow()==0){
      return False;
    }
    if(mssel_p.nrow() < ms_p.nrow()){
      os << LogIO::NORMAL
	 << mssel_p.nrow() << " out of " << ms_p.nrow() << " rows are going to be" 
	 << " considered due to the selection criteria." 
	 << LogIO::POST;
    }
    return True;
    
  }
  
  MeasurementSet* SubMS::setupMS(String MSFileName, Int nchan, Int nCorr, 
				 String telescop, String colName,Int obsType){
    
    
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
    IPosition dataShape(2,nCorr,nchan);
    IPosition tileShape = MSTileLayout::tileShape(dataShape,obsType, telescop);
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
    MSDataDescription ddtable= mssel_p.dataDescription();
    ROScalarColumn<Int> polId(ddtable, 
			      MSDataDescription::columnName(MSDataDescription::POLARIZATION_ID));
    
    //Fill in matching spw to datadesc in old ms 
    {
      ROMSDataDescColumns msOldDD(ddtable);
      oldDDSpwMatch_p=msOldDD.spectralWindowId().getColumn();
    }
    //POLARIZATION table 
    
    
    MSPolarization poltable= mssel_p.polarization();
    ROScalarColumn<Int> numCorr (poltable, 
				 MSPolarization::columnName(MSPolarization::NUM_CORR));
    ROArrayColumn<Int> corrType(poltable, 
				MSPolarization::columnName(MSPolarization::CORR_TYPE));
    ROArrayColumn<Int> corrProd(poltable, MSPolarization::columnName(MSPolarization::CORR_PRODUCT));
    ROScalarColumn<Bool> polFlagRow(poltable, MSPolarization::columnName(MSPolarization::FLAG_ROW));
    
    //SPECTRAL_WINDOW table
    MSSpectralWindow spwtable=mssel_p.spectralWindow();
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
    for(uInt k=0; k < nPol; ++k){
      selectedPolId[k]=ddPolId[index[uniq[k]]];
    }
    
    Vector<Int> newPolId(spw_p.nelements());
    for(uInt k=0; k < spw_p.nelements(); ++k){
      for (uInt j=0; j < nPol; ++j){ 
	if(selectedPolId[j]==ddPolId[k])
	  newPolId[k]=j;
      }
    }
    
    for(uInt k=0; k < newPolId.nelements(); ++k){
      msOut_p.polarization().addRow();
      msPol.numCorr().put(k,numCorr(polId(spw_p[k])));
      msPol.corrType().put(k,corrType(polId(spw_p[k])));
      msPol.corrProduct().put(k,corrProd(polId(spw_p[k])));
      msPol.flagRow().put(k,polFlagRow(polId(spw_p[k])));
    }
    for(uInt k=0; k < spw_p.nelements(); ++k){
      inNumChan_p[k]=numChan(spw_p[k]);
      msOut_p.spectralWindow().addRow();
      msOut_p.dataDescription().addRow();
      spwRelabel_p[spw_p[k]]=k;
      if(nchan_p[k] != numChan(spw_p[k])){
	Int totchan=nchan_p[k]*chanStep_p[k]+chanStart_p[k];
	if(totchan >  numChan(spw_p[k])){
	  os << LogIO::SEVERE
	     << " Channel settings wrong; exceeding number of channels in spw "
	     << spw_p[k]+1 << LogIO::POST;
	  return False;
	}
	doChanAver_p=True; 
	Vector<Double> chanFreqOut(nchan_p[k]);
	Vector<Double> chanFreqIn= chanFreq(spw_p[k]);
	Vector<Double> spwResolOut(nchan_p[k]);
	Vector<Double> spwResolIn= spwResol(spw_p[k]);
	for(Int j=0; j < nchan_p[k]; ++j){
	  if(averageChannel_p){
	    chanFreqOut[j]=(chanFreqIn[chanStart_p[k]+j*chanStep_p[k]]+
			    chanFreqIn[chanStart_p[k]+(j+1)*chanStep_p[k]-1])/2;
	    spwResolOut[j]= spwResolIn[chanStart_p[k]+ 
				       j*chanStep_p[k]]*chanStep_p[k];
	  }
	  else{
	    chanFreqOut[j]=chanFreqIn[chanStart_p[k]+j*chanStep_p[k]];
	    spwResolOut[j]=spwResolIn[chanStart_p[k]+ 
				      j*chanStep_p[k]];
	  }
	}
	Double totalBW=chanFreqOut[nchan_p[k]-1]-chanFreqOut[0]+spwResolOut[0];
	msSpW.chanFreq().put(k, chanFreqOut);
	msSpW.resolution().put(k, spwResolOut);
	msSpW.numChan().put(k, nchan_p[k]);
	msSpW.chanWidth().put(k, spwResolOut);
	msSpW.effectiveBW().put(k, spwResolOut);
	msSpW.refFrequency().put(k,chanFreqOut[0]);
	msSpW.totalBandwidth().put(k, totalBW);
	
	
      }
      else{
	msSpW.chanFreq().put(k, chanFreq(spw_p[k]));
	msSpW.resolution().put(k, spwResol(spw_p[k]));
	msSpW.numChan().put(k, numChan(spw_p[k]));    
	msSpW.chanWidth().put(k, chanWidth(spw_p[k]));
	msSpW.effectiveBW().put(k, effBW(spw_p[k]));
	msSpW.refFrequency().put(k, refFreq(spw_p[k]));
	msSpW.totalBandwidth().put(k, totBW(spw_p[k]));
      }
      
      msSpW.flagRow().put(k,spwFlagRow(spw_p[k]));
      msSpW.freqGroup().put(k, freqGroup(spw_p[k]));
      msSpW.freqGroupName().put(k, freqGroupName(spw_p[k]));
      msSpW.ifConvChain().put(k, ifConvChain(spw_p[k]));
      msSpW.measFreqRef().put(k, measFreqRef(spw_p[k]));
      msSpW.name().put(k, spwName(spw_p[k]));
      msSpW.netSideband().put(k, netSideband(spw_p[k]));
      
      
      msDD.flagRow().put(k, False);
      msDD.polarizationId().put(k,newPolId[k]);
      msDD.spectralWindowId().put(k,k);
      
      
    }
    
    
    
    return True;
    
  }
  
  
  Bool SubMS::fillFieldTable() {
    
    
    MSFieldColumns& msField(msc_p->field());
    
    //MSField fieldtable= mssel_p.field();
    ROMSFieldColumns & fieldIn= mscIn_p->field(); 
    
    String dirref;
    // Need to define the direction measures right
    fieldIn.delayDir().keywordSet().asRecord("MEASINFO").
      get("Ref", dirref);
    //  MDirection::getType(dir1, dirref);
    msField.delayDir().rwKeywordSet().asrwRecord("MEASINFO").
      define("Ref", dirref);
    fieldIn.phaseDir().keywordSet().asRecord("MEASINFO").
      get("Ref", dirref);
    msField.phaseDir().rwKeywordSet().asrwRecord("MEASINFO").
      define("Ref", dirref);
    fieldIn.referenceDir().keywordSet().asRecord("MEASINFO").
      get("Ref", dirref);
    msField.referenceDir().rwKeywordSet().asrwRecord("MEASINFO").
      define("Ref", dirref);
    
    
    ROScalarColumn<String> code(fieldIn.code());
    ROArrayColumn<Double> delayDir(fieldIn.delayDir());
    ROScalarColumn<Bool> flagRow(fieldIn.flagRow());
    ROScalarColumn<String> name(fieldIn.name());
    ROScalarColumn<Int> numPoly(fieldIn.numPoly());
    ROArrayColumn<Double> phaseDir(fieldIn.phaseDir());
    ROArrayColumn<Double> refDir(fieldIn.referenceDir());
    ROScalarColumn<Int> sourceId(fieldIn.sourceId());
    ROScalarColumn<Double> time(fieldIn.time());
    
    
    fieldRelabel_p.resize(mscIn_p->field().nrow());
    fieldRelabel_p.set(-1);
    
    
    for(uInt k=0; k < fieldid_p.nelements(); ++k){
      fieldRelabel_p[fieldid_p[k]]=k;
      msOut_p.field().addRow();
      
      msField.code().put(k,code(fieldid_p[k]));
      msField.delayDir().put(k, delayDir(fieldid_p[k]));
      msField.flagRow().put(k, flagRow(fieldid_p[k]));
      msField.name().put(k, name(fieldid_p[k]));
      msField.numPoly().put(k, numPoly(fieldid_p[k]));
      msField.phaseDir().put(k, phaseDir(fieldid_p[k]));
      msField.referenceDir().put(k, refDir(fieldid_p[k]));
      msField.sourceId().put(k, sourceId(fieldid_p[k]));
      
      
      
    }
    
    return True;
    
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
    String sep(",");
    uInt  nNames;
    String::size_type tokpos,startpos=0;
    tmpNames.upcase();
    
    if (tmpNames.contains("ALL")) allCols=True;
    
    while ((tokpos=tmpNames.index(sep,startpos)))
      {
	tokens.resize(tokens.nelements()+1,True);
	if (tokpos==String::npos)
	  tokens(tokens.nelements()-1)=tmpNames.after(startpos-1);
	else
	  tokens(tokens.nelements()-1)=tmpNames.before(sep,startpos);
	
	if (tokpos==String::npos) break;
	
	startpos=tokpos+1;
      }

    nNames=tokens.nelements();
    
    if (allCols)
      {
	my_colNameVect.resize(3);
	my_colNameVect[0] = MS::columnName(MS::DATA);
	my_colNameVect[1] = MS::columnName(MS::MODEL_DATA);
	my_colNameVect[2] = MS::columnName(MS::CORRECTED_DATA);
      }
    else
      for(uInt i=0; i<nNames; i++)
	{
	  my_colNameVect.resize(i+1,True);
	  my_colNameVect[i]="Unrecognized";
	  
	  if (tokens[i]=="OBSERVED" || 
	      tokens[i]=="DATA" || 
	      tokens[i]==MS::columnName(MS::DATA)) 
	    
	    my_colNameVect[i] = MS::columnName(MS::DATA);
	  
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

    // Whether or not the MS has the columns is checked by verifyColumns().
    // Unfortunately it cannot be done here because this is a static method.

    os << LogIO::NORMAL << "Splitting ";
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
  
  Bool SubMS::getDataColumn(MSColumns& msc, ROArrayColumn<Complex>& data,
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

  Bool SubMS::fillMainTable(const String& whichCol){
    
    LogIO os(LogOrigin("SubMS", "fillMainTable()"));
    
    msOut_p.addRow(mssel_p.nrow(), True);
    
    if(!antennaSel_p){
      msc_p->antenna1().putColumn(mscIn_p->antenna1());
      msc_p->antenna2().putColumn(mscIn_p->antenna2());
      msc_p->feed1().putColumn(mscIn_p->feed1());
      msc_p->feed2().putColumn(mscIn_p->feed2());
    }
    else{
      Vector<Int> ant1=mscIn_p->antenna1().getColumn();
      Vector<Int> ant2=mscIn_p->antenna2().getColumn();
      Vector<Int> feed1=mscIn_p->feed1().getColumn();
      Vector<Int> feed2=mscIn_p->feed2().getColumn();
      
      for (uInt k=0; k < ant1.nelements(); ++k){
	ant1[k]=antNewIndex_p[ant1[k]];
	ant2[k]=antNewIndex_p[ant2[k]];
	feed1[k]=feedNewIndex_p[feed1[k]];
	feed2[k]=feedNewIndex_p[feed2[k]];
      }
      msc_p->antenna1().putColumn(ant1);
      msc_p->antenna2().putColumn(ant2);
      msc_p->feed1().putColumn(feed1);
      msc_p->feed2().putColumn(feed2);
    }
    msc_p->arrayId().putColumn(mscIn_p->arrayId());
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
    msc_p->uvw().putColumn(mscIn_p->uvw());
    msc_p->weight().putColumn(mscIn_p->weight());
    msc_p->sigma().putColumn(mscIn_p->sigma());

    relabelIDs();

    //Deal with data

    // RR 5/15/2009: whichCol has already been vetted higher up in the calling
    // chain by verifyColumns().
    const Vector<String> columnName = parseColumnNames(whichCol);
    
    if(!doChanAver_p){
      ROArrayColumn<Complex> data;
      for(uInt ni = 0;ni < columnName.nelements(); ++ni){
	getDataColumn(*msc_p, data, columnName[ni]);
	putDataColumn(*msc_p, data, columnName[ni], (columnName.nelements() == 1));
      }
      msc_p->flag().putColumn(mscIn_p->flag());
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
	writeSimilarSpwShape(whichCol);
      else
	writeDiffSpwShape(whichCol);
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
  Vector<Int> datDesc = mscIn_p->dataDescId().getColumn();

  for(uInt k = 0; k < datDesc.nelements(); ++k)
    datDesc[k] = spwRelabel_p[oldDDSpwMatch_p[datDesc[k]]];
  msc_p->dataDescId().putColumn(datDesc);

  Vector<Int> fieldId = mscIn_p->fieldId().getColumn();
  for(uInt k = 0; k < fieldId.nelements(); ++k)
    fieldId[k] = fieldRelabel_p[fieldId[k]];
  msc_p->fieldId().putColumn(fieldId);

  remapColumn(mscIn_p->stateId(), msc_p->stateId());
  remapColumn(mscIn_p->processorId(), msc_p->processorId());
  remapColumn(mscIn_p->observationId(), msc_p->observationId());
}

  Bool SubMS::fillAverMainTable(const String& whichCol){
    
    LogIO os(LogOrigin("SubMS", "fillAverMainTable()"));
    
    Double timeBin=timeBin_p;

    //// fill time and timecentroid and antennas
    //if(!fillAverAntTime())
    nant_p = fillAntIndexer(mscIn_p, antIndexer_p);
    if(nant_p < 1)
      return False;

    //Int numBaselines=numOfBaselines(ant1, ant2, False);
    Int numTimeBins=numOfTimeBins(timeBin);  // Sets up remappers as a side-effect.
    
    if(numTimeBins < 1)
      os << LogIO::SEVERE
	 << "Number of time bins is < 1: time averaging bin size is not > 0"
	 << LogIO::POST;

    msOut_p.addRow(numOutRows_p, True);
        
    //    relabelIDs();

//     //Fill array id with first value of input ms for now.
//     msc_p->arrayId().fillColumn(mscIn_p->arrayId()(0));
//     msc_p->observationId().fillColumn(mscIn_p->observationId()(0));
//     msc_p->processorId().fillColumn(mscIn_p->processorId()(0));
//     msc_p->stateId().fillColumn(mscIn_p->stateId()(0));
    
    msc_p->interval().fillColumn(timeBin);
    
    //things to be taken care in averData... (1) flagRow, (2) ScanNumber 
    //(3) uvw (4) weight (5) sigma (6) ant1 (7) ant2 (8) time (9) timeCentroid
    //    (10) feed1 (11) feed2 (12) exposure (13) stateId (14) processorId
    //         (15) observationId (16) arrayId
    if(!fillTimeAverData(whichCol))
      return False;
    
    return True;
    
  }
  
  
  
  Bool SubMS::copyAntenna(){
    Table oldAnt(mssel_p.antennaTableName(), Table::Old);
    Table& newAnt = msOut_p.antenna();
    
    if(!antennaSel_p){
      
      
      TableCopy::copyRows(newAnt, oldAnt);
      return True;
    }
    else{
      //Now we try to re-index the antenna list;
      Vector<Int> ant1 = mscIn_p->antenna1().getColumn();
      Int nAnt1=GenSort<Int>::sort(ant1,Sort::Ascending,
				   Sort::NoDuplicates);
      ant1.resize(nAnt1, True);
      Vector<Int> ant2 = mscIn_p->antenna2().getColumn();
      Int nAnt2=GenSort<Int>::sort(ant2,Sort::Ascending,
				   Sort::NoDuplicates);
      ant2.resize(nAnt2, True);
      ant1.resize(nAnt2+nAnt1, True);
      ant1(Slice(nAnt1,nAnt2))=ant2;
      nAnt1=GenSort<Int>::sort(ant1,Sort::Ascending,
			       Sort::NoDuplicates);
      ant1.resize(nAnt1, True);
      antNewIndex_p.resize(oldAnt.nrow());
      antNewIndex_p.set(-1); //So if you see -1 in the main table or feed fix it
      for (Int k=0; k < nAnt1; ++k){
	antNewIndex_p[ant1[k]]=k;
	TableCopy::copyRows(newAnt, oldAnt, k, ant1[k], 1);
      }
      
      return True;
    }
    return False;
    
  }
  
  Bool SubMS::copyFeed(){
    
    Table oldFeed(mssel_p.feedTableName(), Table::Old);
    Table& newFeed = msOut_p.feed();
    if(!antennaSel_p){
      TableCopy::copyRows(newFeed, oldFeed);
      return True;
    }
    else{
      Vector<Bool> feedRowSel(oldFeed.nrow());
      feedRowSel.set(False);
      Vector<Int> antIds=ROScalarColumn<Int> (oldFeed, "ANTENNA_ID").getColumn();
      Vector<Int> feedIds=ROScalarColumn<Int> (oldFeed, "FEED_ID").getColumn();
      feedNewIndex_p.resize(max(feedIds)+1);
      feedNewIndex_p.set(-1);
      uInt feedSelected=0;
      for (uInt k=0; k < antIds.nelements(); ++k){
	if(antNewIndex_p[antIds[k]] > -1){
	  feedRowSel[k]=True;
	  feedNewIndex_p[feedIds[k]]=feedSelected;
	  TableCopy::copyRows(newFeed, oldFeed, feedSelected, k, 1);
	  ++feedSelected;
	}
      }
      ScalarColumn<Int> antCol(newFeed, "ANTENNA_ID");
      ScalarColumn<Int> feedCol(newFeed, "FEED_ID");
      Vector<Int> newAntIds=antCol.getColumn();
      Vector<Int> newFeedIds=feedCol.getColumn();
      for (uInt k=0; k< feedSelected; ++k){
	newAntIds[k]=antNewIndex_p[newAntIds[k]];
	newFeedIds[k]=feedNewIndex_p[newFeedIds[k]];
      } 
      antCol.putColumn(newAntIds);
      feedCol.putColumn(newFeedIds);
      
      return True;
      
    }
    
    return True;
    
  }
  
  
  Bool SubMS::copySource(){
    //Source is an optinal table..so it may not exist
    if(Table::isReadable(mssel_p.sourceTableName())){
      Table oldSource(mssel_p.sourceTableName(), Table::Old);
      Table& newSource=msOut_p.source();
      
      if(newSource.actualTableDesc().ncolumn() != 
	 oldSource.actualTableDesc().ncolumn()){      
	Vector<String> oldColumnNames=oldSource.actualTableDesc().columnNames();
	Vector<String> optionalCols(6);
	optionalCols[0]="TRANSITION";
	optionalCols[1]="REST_FREQUENCY";
	optionalCols[2]="SYSVEL";
	optionalCols[3]="SOURCE_MODEL";
	optionalCols[4]="PULSAR_ID";
	optionalCols[5]="POSITION";
	for (uInt k=0; k< oldSource.actualTableDesc().ncolumn(); ++k){
	  for (uInt j=0; j < optionalCols.nelements(); ++j){
	    if(oldColumnNames[k].contains(optionalCols[j])){
	      TableDesc tabDesc;
	      MSSource::addColumnToDesc(tabDesc, MSSource::columnType(optionalCols[j]));
	      newSource.addColumn(tabDesc[0]);
	    }
	  }
	}
      }
      TableCopy::copyRows(newSource, oldSource);
      return True;
    }
    
    
    return False;
    
  }
  
  Bool SubMS::copyObservation(){
    
    Table oldObs(mssel_p.observationTableName(), Table::Old);
    Table& newObs=msOut_p.observation();
    TableCopy::copyRows(newObs, oldObs);
    
    
    return True;
    
  }
  
  Bool SubMS::copyPointing(){
    //Pointing is allowed to not exist
    Bool pointExists=Table::isReadable(mssel_p.pointingTableName());
    if(pointExists){
      Table oldPoint(mssel_p.pointingTableName(), Table::Old);
      if(oldPoint.nrow() > 0){
	Table& newPoint=msOut_p.pointing();
	TableCopy::copyRows(newPoint, oldPoint);
      }
    }
    return True;
    
  }
  
  
  
  Bool SubMS::writeDiffSpwShape(const String& columnName){
    
    Bool doSpWeight = !(mscIn_p->weightSpectrum().isNull()) &&
                      mscIn_p->weightSpectrum().isDefined(0);
    
    Int rowsdone=0;
    Int rowsnow=0;
    Block<Int> sort(0);
    Matrix<Int> noselection;
    
    VisSet *vs= new VisSet(mssel_p, noselection);
    ROVisIter& vi(vs->iter());
    VisBuffer vb(vi);
    Vector<Int> spwindex(max(spw_p)+1);
    spwindex.set(-1);
    for (uInt k=0; k < spw_p.nelements() ; ++k){
      spwindex[spw_p[k]]=k;
    }
    
    // RR 5/15/2009: columnName has already been vetted higher up in the
    // calling chain by verifyColumns().
    const Vector<String> colNameTok=parseColumnNames(columnName);
    const uInt ntok=colNameTok.nelements();
    for (vi.originChunks();vi.moreChunks();vi.nextChunk()) 
      {
	for (vi.origin(); vi.more(); vi++) 
	  {
	    rowsnow=vb.nRow();
	    RefRows rowstoadd(rowsdone, rowsdone+rowsnow-1);
	    //	    Cube<Bool> locflag(npol_p[spw], nchan_p[spw], rowsnow);
	    Cube<Bool> locflag;
	    for(uInt ni=0;ni<ntok;ni++)
	      {
		Int spw=spwindex[vb.spectralWindow()];
		Cube<Complex> vis;

		if(colNameTok[ni]== MS::columnName(MS::DATA))
		  vis.reference(vb.visCube());
		else if(colNameTok[ni] == MS::columnName(MS::MODEL_DATA))
		  vis.reference(vb.modelVisCube());
		else
		  vis.reference(vb.correctedVisCube());
		
		Cube<Bool> inFlag;
		Cube<Float> inSpWeight;
		Matrix<Bool> chanFlag;
		Cube<Complex> averdata(npol_p[spw], nchan_p[spw], rowsnow);
		Cube<Float> spWeight;
		
		chanFlag.reference(vb.flag());
		inFlag.reference(vb.flagCube());
		averdata.set(Complex(0,0));
		
		if (doSpWeight)
		  spWeight.resize(npol_p[spw], nchan_p[spw], rowsnow);
		
		locflag.resize(npol_p[spw], nchan_p[spw], rowsnow);
		Bool idelete;
		const Bool* iflag=inFlag.getStorage(idelete);
		const Complex* idata=vis.getStorage(idelete);
		//      const Float* iweight=inSpWeight.getStorage(idelete);
		Complex* odata=averdata.getStorage(idelete);
		Bool* oflag=locflag.getStorage(idelete);
		// We have to revisit this once visBuffer provides the spectral Weights
		
		
		for(Int k=0; k < rowsnow; ++k){
		  for(Int j=0; j < nchan_p[spw]; ++j){
		    Vector<Int>counter(npol_p[spw]);
		    counter.set(0);
		    for (Int pol=0; pol < npol_p[spw]; ++pol){
		      Int outoffset=k*nchan_p[spw]*npol_p[spw]+j*npol_p[spw]+pol;
		      if(!averageChannel_p){
			averdata.xyPlane(k).column(j)=
			  vis.xyPlane(k).column(chanStart_p[spw]+ j*chanStep_p[spw]); 
			locflag.xyPlane(k).column(j)=
			  inFlag.xyPlane(k).column(chanStart_p[spw]+ j*chanStep_p[spw]); 
		      }
		      else{  
			for (Int m=0; m < chanStep_p[spw]; ++m){
			  Int inoffset=k*inNumChan_p[spw]*npol_p[spw]+
			    (j*chanStep_p[spw]+m)*npol_p[spw]+ pol;
			  
			  if(!iflag[inoffset]){
			    odata[outoffset] += idata[inoffset];
			    ++counter[pol];
			  }
			}
		      }
		      
		      if(averageChannel_p){
			if(counter[pol] >0){
			  odata[outoffset] = odata[outoffset]/counter[pol];
			  oflag[outoffset]=False;
			  
			}
			else{
			  odata[outoffset]=0;
			  oflag[outoffset]=True;
			}
		      }
		    }
		  }
		}  
		
		if (ntok==1)
		  msc_p->data().putColumnCells(rowstoadd, averdata);
		else
		  {
		    if(colNameTok[ni]== MS::columnName(MS::DATA))
		      msc_p->data().putColumnCells(rowstoadd, averdata);
		    if (colNameTok[ni]==MS::columnName(MS::MODEL_DATA))
		      msc_p->modelData().putColumnCells(rowstoadd, averdata);
		    else
		      msc_p->correctedData().putColumnCells(rowstoadd, averdata);
		  }
	      }
	    msc_p->flag().putColumnCells(rowstoadd, locflag);
	    rowsdone+=rowsnow;

	  }
      }
    
    return True;
  }
  
Bool SubMS::writeSimilarSpwShape(const String& columnName){
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
  Cube<Float> outspweight;
    
  // RR 5/15/2009: columnName has already been vetted higher up in the
  // calling chain by verifyColumns().
  const Vector<String> colNameTok=parseColumnNames(columnName);
  const uInt ntok=colNameTok.nelements();
  ROArrayColumn<Complex> data;
  ROArrayColumn<Float> wgtSpec;

  for(uInt ni=0; ni<ntok; ni++){
    data.reference(right_column(mscIn_p, colNameTok[ni]));
    ROArrayColumn<Bool> flag(mscIn_p->flag());
    if(doSpWeight){ 
      outspweight.resize(npol_p[0], nchan_p[0], nrow);
      wgtSpec.reference(mscIn_p->weightSpectrum());
    }

    for (Int row=0; row < nrow; ++row){
      data.get(row, indatatmp);
      flag.get(row, inflagtmp);

      if(doSpWeight)
	wgtSpec.get(row, inwgtspectmp);

      Int ck=0;
      Int chancounter=0;
      Vector<Int> avcounter(npol_p[0]);
      outdatatmp.set(0); outwgtspectmp.set(0);
      avcounter.set(0);
      
      for (Int k=chanStart_p[0]; k< (nchan_p[0]*chanStep_p[0]+chanStart_p[0]);
	   ++k) {
	if(chancounter == chanStep_p[0]){
	  outdatatmp.set(0); outwgtspectmp.set(0);
	  chancounter=0;
	  avcounter.set(0);
	}
	++chancounter;
	for (Int j=0; j< npol_p[0]; ++j){
	  Int offset= j + k*npol_p[0];
	  if(!iflg[offset]){
	    if(doSpWeight){
	      outdatatmp[j] += iptr[offset]*inwptr[offset];
	      outwgtspectmp[j] += inwptr[offset];
	    }
	    else
	      outdatatmp[j] += iptr[offset];	   
	    ++avcounter[j];
	  }

	  if(chancounter==chanStep_p[0]){
	    if(avcounter[j] !=0){
	      if(doSpWeight){
		outdata(j,ck,row)=outdatatmp[j]/outwgtspectmp[j];	 
		outspweight(j,ck,row)=outwgtspectmp[j];
	      }
	      else{
		outdata(j,ck,row)=outdatatmp[j]/avcounter[j];	    
	      }
	      outflag(j,ck,row)=False;
	    } 
	    else{	    
	      outdata(j,ck,row)=0;
	      outflag(j,ck,row)=True;
	      if(doSpWeight)
		outspweight(j, ck, row) = 0;
	    }	
	  }
	}
	if(chancounter == chanStep_p[0])
	  ++ck;
      }
    }
    
    putDataColumn(*msc_p, outdata, colNameTok[ni], (ntok==1));
    
    msc_p->flag().putColumn(outflag);
    if(doSpWeight)
      msc_p->weightSpectrum().putColumn(outspweight);
  }
  return True;
}

const ROArrayColumn<Complex>& SubMS::right_column(const MSColumns *ms_p,
						  const String& colName)
{
  const String myColName(upcase(colName));
  
  if(myColName == MS::columnName(MS::DATA))
    return ms_p->data();
  else if(myColName == "MODEL_DATA")
    return ms_p->modelData();
  else
    return ms_p->correctedData();
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

  Int remaval = 0;
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
// The mapping also assumes that each property ranges from 0 to max - 1,
// i.e. has been relabeled (= remapped).  This is not appropriate for array_ID,
// and possibly some others.
uInt SubMS::rowProps2slotKey(const Int ant1, const Int ant2,
			     const Int dd,   const Int field,
			     const Int scan, const Int state)
{
  Int slotKey = state;  // Fastest slotnum (farthest apart), but slowest index.

  slotKey *= scanRemapper_p.nelements();   // Must be set before calling rowProps2slotKey().
  slotKey += remapped(scan, scanRemapper_p, abs(scan));

  slotKey *= fieldid_p.nelements();
  slotKey += field;

  slotKey *= spw_p.nelements();  // ndds;
  slotKey += dd;

  slotKey *= nant_p;   // Must be set before calling rowProps2slotKey().
  slotKey += ant1;

  slotKey *= nant_p;   // Must be set before calling rowProps2slotKey().
  slotKey += ant2;     // Slowest slotnum (closest together), but fastest index.

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
      const Vector<Int>&    scan         = mscIn_p->scanNumber().getColumn();
      const Vector<Int>&    state        = mscIn_p->stateId().getColumn();
      const Vector<Bool>&   rowFlag      = mscIn_p->flagRow().getColumn();

      std::set<Int> slotSet;
      
      GenSortIndirect<Double>::sort(tOI_p, timeRows);

      newTimeVal_p.resize(numrows);
      bin_slots_p.resize(numrows);

      make_map(scan,  scanRemapper_p);  // This map is only implicitly used.
      make_map(state, stateRemapper_p);

      Int    numBin    = 0;

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
	  bin_slots_p[numBin][rowProps2slotKey(antIndexer_p[ant1[toik]],
					       antIndexer_p[ant2[toik]],
					       spwRelabel_p[oldDDSpwMatch_p[datDesc[toik]]],
					       fieldRelabel_p[fieldId[toik]],
					       scan[toik],	// Don't remap!
					       remapped(state[toik], stateRemapper_p,
							abs(state[toik])))].push_back(toik);
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

uInt SubMS::fillAntIndexer(const MSColumns *msc, Vector<Int>& antIndexer)
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

//   Bool SubMS::fillAverAntTime()
//   {
//     //uInt nrows = msOut_p.nrow();
//     //Vector<Int> antenna1(nrows);
//     //Vector<Int> antenna2(nrows);
//     //Vector<Double> rowTime(nrows);

//     // Do stuff that can be done outside the time loop.
//     nant_p = fillAntIndexer(mscIn_p, antIndexer_p);

//     Vector<Int> startAnt2Inds(nant1_p);
//     for (uInt ant1Index = 0; ant1Index < nant1_p; ++ant1Index){
//       const Int ant1indexed = ant1[ant1Index];

//       // be careful as selection may have ant1 which is bigger than max(ant2)
//       if(ant1indexed < maxant2_p){
// 	Int somecounter;      // Would be uInt but too many antenna things are Int.
// 	//startAnt2Ind=ant2Indexer[ant1indexed+somcounter];
	  
// 	for(somecounter = ant1indexed + 1;
// 	    somecounter <= maxant2_p && ant2Indexer_p[somecounter] == -1;
// 	    ++somecounter);
	  
// 	startAnt2Inds[ant1Index] = (somecounter <= maxant2_p) ?
// 	                           ant2Indexer_p[somecounter] :
// 	                           nant2_p;  
//       }
//     }

//     //Will need to do the weighted averaging of time in the future.
//     uInt k = 0;
//     //Double timeStart=mscIn_p->time()(0)+0.5*timeBin;
//     for(uInt t = 0; t < nTimeBins; ++t){
//       for(uInt ant1Index = 0; ant1Index < nant1_p; ++ant1Index){
// 	// be careful as selection may have ant1 which is bigger than max
// 	// ant2
// 	const Int ant1indexed = ant1[ant1Index];
	
// 	if(ant1indexed < maxant2_p){
// 	  if(k == nrows)
// 	    throw(AipsError("Something not expected by the programmer happened; Please file a bug report"));

// 	  for(uInt ant2Index = startAnt2Inds[ant1Index]; ant2Index < nant2_p; 
// 	      ++ant2Index){ 
// 	    if(!antennaSel_p){
// 	      antenna1[k] = ant1indexed;
// 	      antenna2[k] = ant2[ant2Index];
// 	    }
// 	    else{
// 	     antenna1[k] = antNewIndex_p[ant1indexed];
// 	     antenna2[k] = antNewIndex_p[ant2[ant2_pIndex]]; 
// 	    }
// 	    rowTime[k] = newTimeVal_p[t];
// 	    ++k;
// 	  }
// 	}
//       }
//     }
    
//     msc_p->antenna1().putColumn(antenna1);
//     msc_p->antenna2().putColumn(antenna2);
//     // Feed Ids are not being handled properly...
//     // Will work for arrays with one feed setting for all antennas
//     // but will need to be fixed for multi feed setting in one array/ms
//     //Multi-feed antennas and/or multi-feed setting will be messed up
//     msc_p->feed1().fillColumn(mscIn_p->feed1()(0));
//     msc_p->feed2().fillColumn(mscIn_p->feed2()(0)); 
//     msc_p->time().putColumn(rowTime);
//     msc_p->timeCentroid().putColumn(rowTime);
    
//     return True;
//   }
  
Bool SubMS::fillTimeAverData(const String& columnName)
{
  LogIO os(LogOrigin("SubMS", "fillTimeAverData()"));

  //No channel averaging with time averaging
  if(chanStep_p[0]==1){
    //Need to deal with uvw too

    Int outNrow=msOut_p.nrow();

    ROArrayColumn<Complex> data;
    const ROScalarColumn<Double> time(mscIn_p->time());
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

    ROArrayColumn<Bool> flag(mscIn_p->flag());
    const ROScalarColumn<Bool> rowFlag(mscIn_p->flagRow());
    const ROScalarColumn<Int> scanNum(mscIn_p->scanNumber());
    const ROScalarColumn<Int> dataDescIn(mscIn_p->dataDescId());
    const ROArrayColumn<Double> inUVW(mscIn_p->uvw());
 
    data.reference(right_column(mscIn_p, columnName));
    os << LogIO::NORMAL
       << "Writing time averaged data of " << newTimeVal_p.nelements() << " time slots"
       << LogIO::POST;

    const Bool doSpWeight = !mscIn_p->weightSpectrum().isNull() &&
                            mscIn_p->weightSpectrum().isDefined(0);

    Vector<Double> outTime;
    outTime.resize(outNrow);

    Vector<Double> outTC;
    outTC.resize(outNrow);
    outTC.set(0.0);

    Vector<Double> outExposure;
    outExposure.resize(outNrow);
    outExposure.set(0.0);

    Cube<Complex> outData(npol_p[0], nchan_p[0], outNrow);
    outData.set(0.0);

    Matrix<Float> outRowWeight(npol_p[0], outNrow);
    outRowWeight.set(0.0);

    Cube<Bool> outFlag(npol_p[0], nchan_p[0], outNrow);
    outFlag.set(True);

    Vector<Bool> outRowFlag(outNrow);
    outRowFlag.set(True);

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

    Cube<Float> outSpWeight ;
    if(doSpWeight){ 
      outSpWeight.resize(npol_p[0], nchan_p[0], outNrow);
      outSpWeight.set(0.0);
      wgtSpec.reference(mscIn_p->weightSpectrum());
    }

    Double totweight;

    Matrix<Double> outUVW(3,outNrow);
    outUVW.set(0.0);

    Matrix<Float> outSigma(npol_p[0], outNrow);
    outSigma.set(0.0);

    ROArrayColumn<Float> inRowWeight(mscIn_p->weight());
    os << LogIO::NORMAL << "outNrow = " << outNrow << LogIO::POST;
    os << LogIO::NORMAL << "inUVW.nrow() = " << inUVW.nrow() << LogIO::POST;
    //os << LogIO::NORMAL << "npol_p = " << npol_p << LogIO::POST;
    //os << LogIO::NORMAL << "nchan_p = " << nchan_p << LogIO::POST;

    IPosition blc(2, 0, chanStart_p[0]);
    IPosition trc(2, npol_p[0]-1, nchan_p[0]+chanStart_p[0]-1);
    
    // Iterate through timebins.
    uInt orn = 0; 		      // row number in output.
    for(uInt tbn = 0; tbn < bin_slots_p.nelements(); ++tbn){
      // Iterate through slots.
      for(ui2vmap::iterator slotit = bin_slots_p[tbn].begin();
	  slotit != bin_slots_p[tbn].end(); ++slotit){
	uivector& slotv = slotit->second;
	uInt besttoik = slotv[0]; // Closest match to newTimeVal_p[tbn]
	Double bestTimeDiff = fabs(time(besttoik) - newTimeVal_p[tbn]);

	// Iterate through mscIn_p's rows that belong to the slot.
	for(uivector::iterator toikit = slotv.begin();
	    toikit != slotv.end(); ++toikit){
	  // Find the slot's row which is closest to newTimeVal_p[tbn].
	  Double timeDiff = fabs(time(*toikit) - newTimeVal_p[tbn]);

	  if(timeDiff < bestTimeDiff){
	    bestTimeDiff = timeDiff;
	    besttoik = *toikit;
	  }

	  // Accumulate the averaging values from *toikit.
	  // doChanAver_p...wrong name but it means that channel 
	  // selection is a sub selection
	  // its a bit faster if no slicing is done...so avoid it if possible
	  Array<Float> unflaggedwt(doChanAver_p ?
				   inRowWeight(*toikit)(blc, trc).shape() :
				   inRowWeight(*toikit).shape());
	  Array<Complex> data_toikit(doChanAver_p ? data(*toikit)(blc, trc) :
				     data(*toikit));
	  
	  if(doChanAver_p)	    
	    unflaggedwt = MaskedArray<Float>(inRowWeight(*toikit)(blc, trc),
					     reformedMask(flag(*toikit)(blc, trc),
							  false,
							  unflaggedwt.shape()));
	  else
	    unflaggedwt = MaskedArray<Float>(inRowWeight(*toikit),
					     reformedMask(flag(*toikit), false,
							  unflaggedwt.shape()));
	    
//  	    os << LogIO::DEBUG1 << "inRowWeight(*toikit).shape() = "
//  	       << inRowWeight(*toikit).shape() << LogIO::POST;
//  	    os << LogIO::DEBUG1 << "unflaggedwt.shape() = "
//  	       << unflaggedwt.shape() << LogIO::POST;
//  	    os << LogIO::DEBUG1 << "flag(*toikit).shape() = "
//  	       << flag(*toikit).shape() << LogIO::POST;
//  	    os << LogIO::DEBUG1 << "data(*toikit).shape() = "
//  	       << data(*toikit).shape() << LogIO::POST;

	  outRowWeight.column(orn) = outRowWeight.column(orn) + unflaggedwt;
	  totweight = sum(unflaggedwt);

	  if(doChanAver_p)
	    outFlag.xyPlane(orn) = outFlag.xyPlane(orn) * flag(*toikit)(blc, trc);
	  else
	    outFlag.xyPlane(orn) = outFlag.xyPlane(orn) * flag(*toikit);

	  binOpExpandInPlace(data_toikit, unflaggedwt,
			     Multiplies<Complex, Float>());
	    
	  outData.xyPlane(orn) = outData.xyPlane(orn) + data_toikit;

	  outTC[orn] += totweight * inTC(*toikit);
	  outExposure[orn] += totweight * inExposure(*toikit);
	} // End of iterating through the slot's rows.

	// Average the accumulated values.
	totweight = sum(outRowWeight.column(orn));
	if(totweight > 0.0){
	  outTC[orn] /= totweight;
	  outExposure[orn] *= slotv.size() / totweight;
	}
	slotv.clear();  // Free some memory.
	if(product(outRowWeight.column(orn)) > 0.0){
	  Array<Complex> grumble(outData.xyPlane(orn));   // Referenceable
	  Vector<Float>  groan(outRowWeight.column(orn)); // copies
	  
	  binOpExpandInPlace(grumble, groan, casa::Divides<Complex, Float>());
	  outData.xyPlane(orn) = grumble;
	
	  for(uInt polind = 0; polind < outRowWeight.column(orn).nelements();
	      ++polind)
	    outSigma(polind, orn) = 1.0 / sqrt(outRowWeight(polind, orn));
	}

	// Fill in the nonaveraging values from besttoik.
	// In general, _IDs which are row numbers in a subtable must be
	// remapped, and those which are not probably shouldn't be.
	// UVW shouldn't be averaged (we don't want the average of an ellipse).
	outTime[orn]       = newTimeVal_p[tbn];
	outUVW.column(orn) = inUVW(besttoik);
	outScanNum[orn]    = scanNum(besttoik);	// Don't remap!
	if(antennaSel_p){
	  outAnt1[orn]  = antIndexer_p[ant1(besttoik)];
	  outAnt2[orn]  = antIndexer_p[ant2(besttoik)];
	  outFeed1[orn] = feedNewIndex_p[inFeed1(besttoik)];
	  outFeed2[orn] = feedNewIndex_p[inFeed2(besttoik)];
	}
	else{
	  outAnt1[orn]  = ant1(besttoik);
	  outAnt2[orn]  = ant2(besttoik);
	  outFeed1[orn] = inFeed1(besttoik);
	  outFeed2[orn] = inFeed2(besttoik);
	}		
	outField[orn]   = fieldRelabel_p[fieldID(besttoik)];
	outState[orn]   = remapped(state(besttoik), stateRemapper_p,
				   abs(state(besttoik)));
	outProc[orn]    = remapped(inProc(besttoik), procMapper,
				   abs(inProc(besttoik)));
	outObs[orn]     = remapped(inObs(besttoik), obsMapper,
				   abs(inObs(besttoik)));
	outArr[orn]     = inArr(besttoik);	        // Don't remap!
	dataDesc[orn]   = spwRelabel_p[oldDDSpwMatch_p[dataDescIn(besttoik)]];
	outRowFlag[orn] = false;

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
    msc_p->data().putColumn(outData);
  }
  else{
    throw(AipsError("Channel averaging with time averaging is not handled yet"));
    return False;
  }
  return True;
}

inline Bool SubMS::checkSpwShape()
{
  return allSame(inNumChan_p) && allSame(nchan_p);
}

} //#End casa namespace
