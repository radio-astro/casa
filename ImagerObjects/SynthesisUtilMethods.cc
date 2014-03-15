//# SynthesisUtilMethods.cc: Implementation of Imager.h
//# Copyright (C) 1997-2008
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

#include <casa/Exceptions/Error.h>
#include <casa/iostream.h>
#include <casa/sstream.h>

#include <casa/Arrays/Matrix.h>
#include <casa/Arrays/ArrayMath.h>
#include <casa/Arrays/ArrayLogical.h>

#include <casa/Logging.h>
#include <casa/Logging/LogIO.h>
#include <casa/Logging/LogMessage.h>
#include <casa/Logging/LogSink.h>
#include <casa/Logging/LogMessage.h>

#include <casa/OS/DirectoryIterator.h>
#include <casa/OS/File.h>
#include <casa/OS/Path.h>

#include <casa/OS/HostInfo.h>

#include <images/Images/TempImage.h>
#include <images/Images/SubImage.h>
#include <images/Regions/ImageRegion.h>
#include <measures/Measures/MeasTable.h>
#include <ms/MeasurementSets/MSSelection.h>
#include <ms/MeasurementSets/MSColumns.h>
#include <ms/MeasurementSets/MSDopplerUtil.h>
#include <tables/Tables/Table.h>
#include <synthesis/ImagerObjects/SynthesisUtilMethods.h>
#include <synthesis/TransformMachines/Utils.h>

#include <synthesis/MSVis/SubMS.h>

#include <sys/types.h>
#include <unistd.h>
using namespace std;

namespace casa { //# NAMESPACE CASA - BEGIN
  
  SynthesisUtilMethods::SynthesisUtilMethods()
  {
    
  }
  
  SynthesisUtilMethods::~SynthesisUtilMethods() 
  {
    LogIO os( LogOrigin("SynthesisUtilMethods","destructor",WHERE) );
    os << "SynthesisUtilMethods destroyed" << LogIO::POST;
  }
  
  // Data partitioning rules for CONTINUUM imaging
  //  ALL members are strings ONLY.
  Record SynthesisUtilMethods::continuumDataPartition(Record &selpars, const Int npart)
  {
    LogIO os( LogOrigin("SynthesisUtilMethods","continuumDataPartition",WHERE) );

    Record onepart, allparts;
    Vector<Vector<String> > timeSelPerPart;
    timeSelPerPart.resize(selpars.nfields());

    // Duplicate the entire input record npart times, with a specific partition id.
    // Modify each sub-record according to partition id.
    for (uInt msID=0;msID<selpars.nfields();msID++)
      {
	Record thisMS= selpars.subRecord(RecordFieldId("ms"+String::toString(msID)));
	String msName = thisMS.asString("msname");
	timeSelPerPart[msID].resize(npart,True);
	//
	// Make a selected MS and extract the time-column information
	//
	MeasurementSet ms(msName), selectedMS(ms);
	MSInterface msI(ms);	MSSelection msSelObj; 
	msSelObj.reset(msI,MSSelection::PARSE_NOW,
		       thisMS.asString("timestr"),
		       thisMS.asString("antenna"),
		       thisMS.asString("field"),
		       thisMS.asString("spw"),
		       thisMS.asString("uvdist"),
		       thisMS.asString("taql"),
		       "",//thisMS.asString("poln"),
		       thisMS.asString("scan"),
		       "",//thisMS.asString("array"),
		       thisMS.asString("state"),
		       thisMS.asString("obs")//observation
		       );
	msSelObj.getSelectedMS(selectedMS);

	//--------------------------------------------------------------------
	// Use the selectedMS to generate time selection strings per part
	//
	Double Tint;
	ROMSMainColumns mainCols(selectedMS);
	Int nRows=selectedMS.nrow(), dRows=nRows/npart;
	Int rowBeg=0, rowEnd=0;
	rowEnd = rowBeg + dRows;

	MVTime mvInt=mainCols.intervalQuant()(0);
	Time intT(mvInt.getTime());
	Tint = intT.modifiedJulianDay();

	Int partNo=0;
	while(rowEnd < nRows)
	  {
	    MVTime mvt0=mainCols.timeQuant()(rowBeg), mvt1=mainCols.timeQuant()(rowEnd);
	    Time t0(mvt0.getTime()), t1(mvt1.getTime());
	    Double mjdRef=t1.modifiedJulianDay(),
	      mjdT0=t1.modifiedJulianDay();

	    while((fabs(mjdT0 - mjdRef) <= Tint) && (rowEnd < nRows))
	      {
		rowEnd++;
		MVTime mvt=mainCols.timeQuant()(rowEnd);
		Time tt(mvt.getTime());
		mjdT0=tt.modifiedJulianDay();
	      }
	    rowEnd--;
	    MVTime mvtB=mainCols.timeQuant()(rowBeg), mvtE=mainCols.timeQuant()(rowEnd);
	    Time tB(mvtB.getTime()), tE(mvtE.getTime());
	    timeSelPerPart[msID][partNo] = SynthesisUtils::mjdToString(tB) + "~" + SynthesisUtils::mjdToString(tE);
	    // cerr << endl << "Rows = " << rowBeg << " " << rowEnd << " "
	    // 	 << "[P][M]: " << msID << ":" << partNo << " " << timeSelPerPart[msID][partNo]
	    // 	 << endl;	    

	    partNo++;
	    rowBeg = rowEnd+1;
	    rowEnd = min(rowBeg + dRows, nRows-1);
	    if (rowEnd == nRows-1) break;
	  }

	MVTime mvtB=mainCols.timeQuant()(rowBeg), mvtE=mainCols.timeQuant()(rowEnd);
	Time tB(mvtB.getTime()), tE(mvtE.getTime());
	timeSelPerPart[msID][partNo] = SynthesisUtils::mjdToString(tB) + "~" + SynthesisUtils::mjdToString(tE);
	// cerr << endl << "Rows = " << rowBeg << " " << rowEnd << " "
	//      << "[P][M]: " << msID << ":" << partNo << " " << timeSelPerPart[msID][partNo]
	//      << endl;	    
      }
    //
    // The time selection strings for all parts of the current
    // msID are in timeSelPerPart.  
    //--------------------------------------------------------------------
    //
    // Now reverse the order of parts and ME loops. Create a Record
    // per part, get the MS for thisPart.  Put the associated time
    // selection string in it.  Add the thisMS to thisPart Record.
    // Finally add thisPart Record to the allparts Record.
    //
    for(Int iPart=0; iPart<npart; iPart++)
      {
	Record thisPart;
	thisPart.assign(selpars);
	for (uInt msID=0; msID<selpars.nfields(); msID++)	  
	  {
	    Record thisMS = thisPart.subRecord(RecordFieldId("ms"+String::toString(msID)));

	    thisMS.define("timestr",timeSelPerPart[msID][iPart]);
	    thisPart.defineRecord(RecordFieldId("ms"+String::toString(msID)) , thisMS);
	  }
	allparts.defineRecord(RecordFieldId(String::toString(iPart)), thisPart);
      }
    //    cerr << allparts << endl;
    return allparts;

    // for( Int part=0; part < npart; part++)
    //   {

    // 	onepart.assign(selpars);


    // 	//-------------------------------------------------
    // 	// WARNING : This is special-case code for two specific datasets
    // 	for ( uInt msid=0; msid<selpars.nfields(); msid++)
    // 	  {
    // 	    Record onems = onepart.subRecord( RecordFieldId("ms"+String::toString(msid)) );
    // 	    String msname = onems.asString("msname");
    // 	    String spw = onems.asString("spw");
    // 	    if(msname.matches("DataTest/twopoints_twochan.ms"))
    // 	      {
    // 		onems.define("spw", spw+":"+String::toString(part));
    // 	      }
    // 	    if(msname.matches("DataTest/point_twospws.ms"))
    // 	      {
    // 		onems.define("spw", spw+":"+ (((Bool)part)?("10~19"):("0~9"))  );
    // 	      }
    // 	    if(msname.matches("DataTest/reg_mawproject.ms"))
    // 	      {
    // 		onems.define("scan", (((Bool)part)?("1~17"):("18~35"))  );
    // 	      }
    // 	    onepart.defineRecord( RecordFieldId("ms"+String::toString(msid)) , onems );
    // 	  }// end ms loop
    // 	//-------------------------------------------------

    // 	allparts.defineRecord( RecordFieldId(String::toString(part)), onepart );

    //   }// end partition loop

    // return allparts;
  }

  // Data partitioning rules for CUBE imaging
  Record SynthesisUtilMethods::cubeDataPartition(Record &selpars, Int npart)
  {
    LogIO os( LogOrigin("SynthesisUtilMethods","cubeDataPartition",WHERE) );
    // Temporary special-case code. Please replace with actual rules.
    return continuumDataPartition( selpars, npart );

  }

  // Image cube partitioning rules for CUBE imaging
  Record SynthesisUtilMethods::cubeImagePartition(Record &impars, Int npart)
  {
    LogIO os( LogOrigin("SynthesisUtilMethods","cubeImagePartition",WHERE) );

    Record onepart, allparts;

    // Duplicate the entire input record npart times, with a specific partition id.
    // Modify each sub-record according to partition id.
    for( Int part=0; part < npart; part++)
      {

	onepart.assign(impars);

	//-------------------------------------------------
	// WARNING : This is special-case code for two specific datasets
	for ( uInt imfld=0; imfld<impars.nfields(); imfld++)
	  {
	    Record onefld = onepart.subRecord( RecordFieldId(String::toString(imfld)) );
	    Int nchan = onefld.asInt("nchan");
	    //String freqstart = onems.asString("freqstart");

	    onefld.define("nchan", nchan/npart);
	    onefld.define("freqstart", (((Bool)part)?("1.2GHz"):("1.0GHz"))  );

	    String imname = onefld.asString("imagename");
	    onefld.define("imagename", imname+".n"+String::toString(part));

	    onepart.defineRecord( RecordFieldId( String::toString(imfld) ), onefld );
	  }// end ms loop
	//-------------------------------------------------

	allparts.defineRecord( RecordFieldId(String::toString(part)), onepart );

      }// end partition loop

    return allparts;
    

  }

  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  /////////////////    Parameter Containers     ///////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  // Read String from Record
  String SynthesisParams::readVal(Record &rec, String id, String& val)
  {
    if( rec.isDefined( id ) )
      {
	if( rec.dataType( id )==TpString ) { rec.get( id , val ); return String(""); }
	else { return String(id + " must be a string\n"); }
      }
    else { return String("");}
  }

  // Read Integer from Record
  String SynthesisParams::readVal(Record &rec, String id, Int& val)
  {
    if( rec.isDefined( id ) )
      {
	if( rec.dataType( id )==TpInt ) { rec.get( id , val ); return String(""); }
	else  { return String(id + " must be an integer\n"); }
      }
    else { return String(""); }
  }
  
  // Read Float from Record
  String SynthesisParams::readVal(Record &rec, String id, Float& val)
  {
    if( rec.isDefined( id ) )
      {
      if ( rec.dataType( id )==TpFloat || rec.dataType( id )==TpDouble )  
	{ rec.get( id , val ); return String(""); }
      else { return String(id + " must be a float\n"); }
      }
    else { return String(""); }
  }

  // Read Bool from Record
  String SynthesisParams::readVal(Record &rec, String id, Bool& val)
  {
    if( rec.isDefined( id ) )
      {
	if( rec.dataType( id )==TpBool ) { rec.get( id , val ); return String(""); }
	else { return String(id + " must be a bool\n"); }
      }
    else{ return String(""); }
  }

  // Read Vector<Int> from Record
  String SynthesisParams::readVal(Record &rec, String id, Vector<Int>& val)
  {
    if( rec.isDefined( id ) )
      {
	if( rec.dataType( id )==TpArrayInt || rec.dataType( id )==TpArrayUInt ) 
	  { rec.get( id , val ); return String(""); }
	else if ( rec.dataType( id ) == TpArrayBool ) // An empty python vector [] comes in as this.
	  {
	    Vector<Bool> vec; rec.get( id, vec );
	    if( vec.nelements()==0 ){val.resize(0); return String("");}
	    else{ return String(id + " must be a vector of strings.\n"); }
	  }
	else { return String(id + " must be a vector of integers\n"); }
      }
    else{ return String(""); }
  }

  // Read Vector<Float> from Record
  String SynthesisParams::readVal(Record &rec, String id, Vector<Float>& val)
  {
    if( rec.isDefined( id ) )
      {
	if( rec.dataType( id )==TpArrayFloat || rec.dataType( id )==TpArrayDouble ) 
	  { rec.get( id , val ); return String(""); }
	else if ( rec.dataType( id ) == TpArrayBool ) // An empty python vector [] comes in as this.
	  {
	    Vector<Bool> vec; rec.get( id, vec );
	    if( vec.nelements()==0 ){val.resize(0); return String("");}
	    else{ return String(id + " must be a vector of strings.\n"); }
	  }
	else { return String(id + " must be a vector of floats\n"); }
      }
    else{ return String(""); }
  }

  // Read Vector<String> from Record
  String SynthesisParams::readVal(Record &rec, String id, Vector<String>& val)
  {
    if( rec.isDefined( id ) )
      {
	if( rec.dataType( id )==TpArrayString || rec.dataType( id )==TpArrayChar ) 
	  { rec.get( id , val ); return String(""); }
	else if ( rec.dataType( id ) == TpArrayBool ) // An empty python vector [] comes in as this.
	  {
	    Vector<Bool> vec; rec.get( id, vec );
	    if( vec.nelements()==0 ){val.resize(0); return String("");}
	    else{ return String(id + " must be a vector of strings.\n"); }
	  }
	else { return String(id + " must be a vector of strings.\n"); 
	}
      }
    else{ return String(""); }
  }

  // Convert String to Quantity
  String SynthesisParams::stringToQuantity(String instr, Quantity& qa)
  {
    //QuantumHolder qh;
    //String error;
    //    if( qh.fromString( error, instr ) ) { qa = qh.asQuantity(); return String(""); }
    //else { return String("Error in converting " + instr + " to a Quantity : " + error + " \n"); }
    if ( casa::Quantity::read( qa, instr ) ) { return String(""); }
    else  { return String("Error in converting " + instr + " to a Quantity \n"); }
  }

  // Convert String to MDirection
  // UR : TODO :    If J2000 not specified, make it still work.
  String SynthesisParams::stringToMDirection(String instr, MDirection& md)
  {
    try
      {
	String tmpRF, tmpRA, tmpDEC;
	std::istringstream iss(instr);
	iss >> tmpRF >> tmpRA >> tmpDEC;
	if( tmpDEC.length() == 0 )// J2000 may not be specified.
	  {
	    tmpDEC = tmpRA;
	    tmpRA = tmpRF;
	    tmpRF = String("J2000");
	  }
	casa::Quantity tmpQRA;
	casa::Quantity tmpQDEC;
	casa::Quantity::read(tmpQRA, tmpRA);
	casa::Quantity::read(tmpQDEC, tmpDEC);
	MDirection::Types theRF;
	MDirection::getType(theRF, tmpRF);
	md = MDirection (tmpQRA, tmpQDEC, theRF);
	return String("");
      }
    catch(AipsError &x)
      {
	return String("Error in converting '" + instr + "' to MDirection. Need format : 'J2000 19:59:28.500 +40.44.01.50'\n");
      }
  }

  // Read Quantity from a Record string
  String SynthesisParams::readVal(Record &rec, String id, Quantity& val)
  {
    if( rec.isDefined( id ) )
      {
	if( rec.dataType( id )==TpString ) 
	  { String valstr;  rec.get( id , valstr ); return stringToQuantity(valstr, val); }
	else { return String(id + " must be a string in the format : '1.5GHz' or '0.2arcsec'...'\n"); }
      }
    else{ return String(""); }
  }

  // Read MDirection from a Record string
  String SynthesisParams::readVal(Record &rec, String id, MDirection& val)
  {
    if( rec.isDefined( id ) )
      {
	if( rec.dataType( id )==TpString ) 
	  { String valstr;  rec.get( id , valstr ); return stringToMDirection(valstr, val); }
	else { return String(id + " must be a string in the format : 'J2000 19:59:28.500 +40.44.01.50'\n"); }
      }
    else{ return String(""); }
  }

  // Convert MDirection to String
  String SynthesisParams::MDirectionToString(MDirection val)
  {
    MVDirection mvpc( val.getAngle() );
    MVAngle ra = mvpc.get()(0);
    MVAngle dec = mvpc.get()(1);
    // Beware of precision here......( for VLBA / ALMA ). 14 gives 8 digits after the decimal for arcsec.
    return  String(val.getRefString() + " " + ra.string(MVAngle::TIME,14) + " " +  dec.string(MVAngle::ANGLE,14));
  }

  // Convert Quantity to String
  String SynthesisParams::QuantityToString(Quantity val)
  {
    return String::toString( val.getValue(val.getUnit()) ) + val.getUnit() ;
  }


  /////////////////////// Selection Parameters

  SynthesisParamsSelect::SynthesisParamsSelect():SynthesisParams()
  {
    setDefaults();
  }

  SynthesisParamsSelect::~SynthesisParamsSelect()
  {
  }


  void SynthesisParamsSelect::fromRecord(Record &inrec)
  {
    setDefaults();

    String err("");

    try
      {
	
	err += readVal( inrec, String("msname"), msname );
	err += readVal( inrec, String("readonly"), readonly );
	err += readVal( inrec, String("usescratch"), usescratch );
	err += readVal( inrec, String("incrmodel"), incrmodel );

	err += readVal( inrec, String("spw"), spw );

	/// -------------------------------------------------------------------------------------
	// Why are these params here ? Repeats of defineimage.
	err += readVal( inrec, String("freqbeg"), freqbeg);
	err += readVal( inrec, String("freqend"), freqend);

	String freqframestr( MFrequency::showType(freqframe) );
	err += readVal( inrec, String("freqframe"), freqframestr);
	if( ! MFrequency::getType(freqframe, freqframestr) )
	  { err += "Invalid Frequency Frame " + freqframestr ; }
	/// -------------------------------------------------------------------------------------

	err += readVal( inrec, String("field"),field);
	err += readVal( inrec, String("antenna"),antenna);
	err += readVal( inrec, String("timestr"),timestr);
	err += readVal( inrec, String("scan"),scan);
	err += readVal( inrec, String("obs"),obs);
	err += readVal( inrec, String("state"),state);
	err += readVal( inrec, String("uvdist"),uvdist);
	err += readVal( inrec, String("taql"),taql);

	err += verify();

      }
    catch(AipsError &x)
      {
	err = err + x.getMesg() + "\n";
      }
      
      if( err.length()>0 ) throw(AipsError("Invalid Selection Parameter set : " + err));
      
  }

  String SynthesisParamsSelect::verify()
  {
    String err;
    // Does the MS exist on disk.
    Directory thems( msname );
    if( thems.exists() )
      {
	// Is it readable ? 
	if( ! thems.isReadable() )
	  { err += "MS " + msname + " is not readable.\n"; }
	// Depending on 'readonly', is the MS writable ? 
	if( readonly==False && ! thems.isWritable() ) 
	  { err += "MS " + msname + " is not writable.\n"; }
      }
    else 
      { err += "MS does not exist : " + msname + "\n"; }
    
    return err;
  }
  
  void SynthesisParamsSelect::setDefaults()
  {
    msname="";
    spw="";
    freqbeg="";
    freqend="";
    MFrequency::getType(freqframe,"LSRK");
    field="";
    antenna="";
    timestr="";
    scan="";
    obs="";
    state="";
    uvdist="";
    taql="";
    usescratch=True;
    readonly=False;
    incrmodel=False;
  }

  Record SynthesisParamsSelect::toRecord()
  {
    Record selpar;
    selpar.define("msname",msname);
    selpar.define("spw",spw);
    selpar.define("freqbeg",freqbeg);
    selpar.define("freqend",freqend);
    selpar.define("freqframe", MFrequency::showType(freqframe)); // Convert MFrequency::Types to String
    selpar.define("field",field);
    selpar.define("antenna",antenna);
    selpar.define("timestr",timestr);
    selpar.define("scan",scan);
    selpar.define("obs",obs);
    selpar.define("state",state);
    selpar.define("uvdist",uvdist);
    selpar.define("taql",taql);
    selpar.define("usescratch",usescratch);
    selpar.define("readonly",readonly);
    selpar.define("incrmodel",incrmodel);

    return selpar;
  }


  /////////////////////// Image Parameters

  SynthesisParamsImage::SynthesisParamsImage():SynthesisParams()
  {
    setDefaults();
  }

  SynthesisParamsImage::~SynthesisParamsImage()
  {
  }


  void SynthesisParamsImage::fromRecord(Record &inrec)
  {
    setDefaults();

    String err("");

    try
      {

	err += readVal( inrec, String("imagename"), imageName);

	//// imsize
	if( inrec.isDefined("imsize") ) 
	  {
	    DataType tp = inrec.dataType("imsize");
	    
	    if( tp == TpInt ) // A single integer for both dimensions.
	      {
		Int npix; inrec.get("imsize", npix);
		imsize.resize(2);
		imsize.set( npix );
	      }
	    else if( tp == TpArrayInt ) // An integer array : [ nx ] or [ nx, ny ]
	      {
		Vector<Int> ims;
		inrec.get("imsize", ims);
		if( ims.nelements()==1 ) // [ nx ]
		  {imsize.set(ims[0]); }
		else if( ims.nelements()==2 ) // [ nx, ny ]
		  { imsize[0]=ims[0]; imsize[1]=ims[1]; }
		else // Wrong array length
		  {err += "imsize must be either a single integer, or a vector of two integers\n";  }
	      }
	    else // Wrong data type
	      { err += "imsize must be either a single integer, or a vector of two integers\n";   }
	  }//imsize
	
	//// cellsize
	if( inrec.isDefined("cellsize") ) 
	  {
	    try
	      {
		DataType tp = inrec.dataType("cellsize");
		if( tp == TpInt ||  
		    tp == TpFloat || 
		    tp == TpDouble )
		  {
		    Double cell = inrec.asDouble("cellsize");
		    cellsize.set( Quantity( cell , "arcsec" ) );
		  }
		else if ( tp == TpArrayInt ||  
			  tp == TpArrayFloat || 
			  tp == TpArrayDouble )
		  {
		    Vector<Double> cells;
		    inrec.get("cellsize", cells);
		    if(cells.nelements()==1) // [ cellx ]
		      {cellsize.set( Quantity( cells[0], "arcsec" ) ); }
		    else if( cells.nelements()==2 ) // [ cellx, celly ]
		      { cellsize[0]=Quantity(cells[0],"arcsec"); cellsize[1]=Quantity(cells[1],"arcsec"); }
		    else // Wrong array length
		      {err += "cellsize must be a single integer/string, or a vector of two integers/strings\n";  }
		  }
		else if( tp == TpString )
		  {
		    String cell;
		    inrec.get("cellsize",cell);
		    Quantity qcell;
		    err += stringToQuantity( cell, qcell );
		    cellsize.set( qcell );
		  }
		else if( tp == TpArrayString )
		  {
		    Array<String> cells;
		    inrec.get("cellsize", cells);
		    Vector<String> vcells(cells);
		    if(cells.nelements()==1) // [ cellx ]
		      { 
			Quantity qcell; 
			err+= stringToQuantity( vcells[0], qcell ); cellsize.set( qcell ); 
		      }
		    else if( cells.nelements()==2 ) // [ cellx, celly ]
		      { 
			err+= stringToQuantity( vcells[0], cellsize[0] );
			err+= stringToQuantity( vcells[1], cellsize[1] );
		      }
		    else // Wrong array length
		      {err += "cellsize must be a single integer/string, or a vector of two integers/strings\n";  }
		  }
		else // Wrong data type
		  { err += "cellsize must be a single integer/string, or a vector of two integers/strings\n";   }
		
	      } 
	    catch(AipsError &x)
	      {
		err += "Error reading cellsize : " + x.getMesg();
	      }
	  }// cellsize

	//// stokes
	err += readVal( inrec, String("stokes"), stokes);
	    
	////nchan
	err += readVal( inrec, String("nchan"), nchan);

	/// phaseCenter (as a string) . // Add INT support later.
	err += readVal( inrec, String("phasecenter"), phaseCenter );

	//// Projection
	if( inrec.isDefined("projection") )
	  {
	    if( inrec.dataType("projection") == TpString )
	      {
		String pstr;
		inrec.get("projection",pstr);
		try
		  {
		    projection=Projection::type( pstr );
		  }
		catch(AipsError &x)
		  {
		    err += String("Invalid projection code : " + pstr );
		  }
	      }
	    else { err += "projection must be a string\n"; } 
	  }//projection

	// Frequency frame stuff. 
	err += readVal( inrec, String("mode"), mode);
	err += readVal( inrec, String("chanstart"), chanStart);
	err += readVal( inrec, String("chanstep"), chanStep);
	err += readVal( inrec, String("freqstart"), freqStart);
	err += readVal( inrec, String("freqstep"), freqStep);
	err += readVal( inrec, String("reffreq"), refFreq); 
	err += readVal( inrec, String("velstart"), velStart); 
	err += readVal( inrec, String("velstep"), velStep); 
	err += readVal( inrec, String("veltype"), veltype); 

	Vector<String> rfreqs(0);
	err += readVal( inrec, String("restfreq"), rfreqs );
        // case no restfreq is given: set to  

	restFreq.resize( rfreqs.nelements() );
	for( uInt fr=0; fr<rfreqs.nelements(); fr++)
	  {
	    err += stringToQuantity( rfreqs[fr], restFreq[fr] );
	  }

	String freqframestr( MFrequency::showType(freqFrame) );
	err += readVal( inrec, String("freqframe"), freqframestr);
	if( ! MFrequency::getType(freqFrame, freqframestr) )
	  { err += "Invalid Frequency Frame " + freqframestr ; }
	
	err += readVal( inrec, String("overwrite"), overwrite );

	err += readVal( inrec, String("ntaylorterms"), nTaylorTerms );

	err += verify();
	
      }
    catch(AipsError &x)
      {
	err = err + x.getMesg() + "\n";
      }
      
      if( err.length()>0 ) throw(AipsError("Invalid Image Parameter set : " + err));
      
  }

  String SynthesisParamsImage::verify()
  {
    String err;

    if( imsize.nelements() != 2 ){ err += "imsize must be a vector of 2 Ints\n"; }
    if( cellsize.nelements() != 2 ) { err += "cellsize must be a vector of 2 Quantities\n"; }

    if( nchan>1 and nTaylorTerms>1 )
      {err += "Cannot have more than one channel with ntaylorterms>1\n";}

    if( ! stokes.matches("I") && ! stokes.matches("Q") && 
	! stokes.matches("U") && ! stokes.matches("V") && 
	! stokes.matches("RR") && ! stokes.matches("LL") && 
	! stokes.matches("XX") && ! stokes.matches("YY") && 
	! stokes.matches("IV") && ! stokes.matches("IQ") && 
	! stokes.matches("RRLL") && ! stokes.matches("XXYY") &&
	! stokes.matches("QU") && ! stokes.matches("UV") && 
	! stokes.matches("IQU") && ! stokes.matches("IUV") && 
	! stokes.matches("IQUV") ) 
      { err += "Stokes " + stokes + " is an unsupported option \n";}

    
    ///    err += verifySpectralSetup();  
    
	return err;
  }

  /*  
  // Convert all user options to  LSRK freqStart, freqStep, 
  // Could have (optional) log messages coming out of this function, to tell the user what the
  // final frequency setup is ?

  String SynthesisParamsImage::verifySpectralSetup()
  {
  }
  */

  void SynthesisParamsImage::setDefaults()
  {
    // Image definition parameters
    imageName = String("");
    imsize.resize(2); imsize.set(100);
    cellsize.resize(2); cellsize.set( Quantity(1.0,"arcsec") );
    stokes="I";
    phaseCenter=MDirection();
    projection=Projection::SIN;
    startModel=String("");
    overwrite=False;

    // Spectral coordinates
    nchan=1;
    mode="mfs";
    chanStart=0;
    chanStep=1;
    freqStart=Quantity(0,"Hz");
    freqStep=Quantity(0,"Hz");
    velStart=Quantity(0,"m/s");
    velStep=Quantity(0,"m/s");
    veltype=String("radio");
    restFreq.resize(0);
    refFreq = Quantity(0,"Hz");
    freqFrame=MFrequency::LSRK;
    nTaylorTerms=1;

    
  }

  Record SynthesisParamsImage::toRecord()
  {
    Record impar;
    impar.define("imagename", imageName);
    impar.define("imsize", imsize);
    Vector<String> cells(2);
    cells[0] = QuantityToString( cellsize[0] );
    cells[1] = QuantityToString( cellsize[1] );
    impar.define("cellsize", cells );
    impar.define("stokes", stokes);
    impar.define("nchan", nchan);
    impar.define("ntaylorterms", nTaylorTerms);
    impar.define("phasecenter", MDirectionToString( phaseCenter ) );
    impar.define("projection", projection.name() );

    impar.define("mode", mode );
    impar.define("chanstart", chanStart );
    impar.define("chanstep", chanStep );
    impar.define("freqstart", QuantityToString( freqStart ));
    impar.define("freqstep", QuantityToString( freqStep ) );
    impar.define("velstart", QuantityToString( velStart ));
    impar.define("velstep", QuantityToString( velStep ) );
    impar.define("veltype", veltype);
    Vector<String> rfs( restFreq.nelements() );
    for(uInt rf=0; rf<restFreq.nelements(); rf++){rfs[rf] = QuantityToString(restFreq[rf]);}
    impar.define("restfreq", rfs);
    impar.define("reffreq", QuantityToString(refFreq));
    impar.define("freqframe", MFrequency::showType(freqFrame));

    impar.define("overwrite",overwrite );
    impar.define("startmodel", startModel);

    return impar;
  }

  //////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  ///////////////////////////     Build a coordinate system.  ////////////////////////////////////////
  //////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  ////   To be used from SynthesisImager, to construct the images it needs
  ////   To also be connected to a 'makeimage' method of the synthesisimager tool.
  ////       ( need to supply MS only to add  'ObsInfo' to the csys )
  //////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  CoordinateSystem SynthesisParamsImage::buildCoordinateSystem(ROVisibilityIterator* rvi) const
  {
    LogIO os( LogOrigin("SynthesisParamsImage","buildCoordinateSystem",WHERE) );

    /////////////////// Direction Coordinates
    MVDirection mvPhaseCenter(phaseCenter.getAngle());
    // Normalize correctly
    MVAngle ra=mvPhaseCenter.get()(0);
    ra(0.0);

    MVAngle dec=mvPhaseCenter.get()(1);
    Vector<Double> refCoord(2);
    refCoord(0)=ra.get().getValue();    
    refCoord(1)=dec;
    Vector<Double> refPixel(2); 
    refPixel(0) = Double(imsize[0]/2);
    refPixel(1) = Double(imsize[1]/2);

    Vector<Double> deltas(2);
    deltas(0)=-1* cellsize[0].get("rad").getValue();
    deltas(1)=cellsize[1].get("rad").getValue();
    Matrix<Double> xform(2,2);
    xform=0.0;xform.diagonal()=1.0;
    DirectionCoordinate
      myRaDec(MDirection::Types(phaseCenter.getRefPtr()->getType()),
	      projection,
	      refCoord(0), refCoord(1),
	      deltas(0), deltas(1),
	      xform,
	      refPixel(0), refPixel(1));


    //defining observatory...needed for position on earth
    // get the first ms for multiple MSes
    MeasurementSet msobj=rvi->getMeasurementSet();
    ROMSColumns msc(msobj);
    String telescop = msc.observation().telescopeName()(0);
    MEpoch obsEpoch = msc.timeMeas()(0);
    MPosition obsPosition;
    if(!(MeasTable::Observatory(obsPosition, telescop)))
      {
        os << LogIO::WARN << "Did not get the position of " << telescop
           << " from data repository" << LogIO::POST;
        os << LogIO::WARN
           << "Please contact CASA to add it to the repository."
           << LogIO::POST;
        os << LogIO::WARN << "Frequency conversion will not work " << LogIO::POST;
      }

    ObsInfo myobsinfo;
    myobsinfo.setTelescope(telescop);
    myobsinfo.setPointingCenter(mvPhaseCenter);
    myobsinfo.setObsDate(obsEpoch);
    myobsinfo.setObserver(msc.observation().observer()(0));

    /// Attach obsInfo to the CoordinateSystem
    ///csys.setObsInfo(myobsinfo);


    /////////////////// Spectral Coordinate

    //Make sure frame conversion is switched off for REST frame data.
    Bool freqFrameValid=(freqFrame != MFrequency::REST);

    // *** get selected spw ids ***
    Vector<Int> spwids;
    Vector<Int> nvischan;
    rvi->allSelectedSpectralWindows(spwids,nvischan);
    if (spwids.nelements()==0) {
      Int nspw=msc.spectralWindow().nrow();
      spwids.resize(nspw);
      indgen(spwids); 
    }
    MFrequency::Types dataFrame=(MFrequency::Types)msc.spectralWindow().measFreqRef()(spwids[0]);

    Vector<Double> dataChanFreq, dataChanWidth;
    if (spwids.nelements()==1) {
      dataChanFreq=msc.spectralWindow().chanFreq()(spwids[0]);
      dataChanWidth=msc.spectralWindow().chanWidth()(spwids[0]);
    }
    else {
      SubMS thems(msobj);
      if(!thems.combineSpws(spwids,True,dataChanFreq,dataChanWidth)){
        os << LogIO::SEVERE << "Error combining SpWs" << LogIO::POST;
      }
    }
    
    Quantity qrestfreq = restFreq.nelements() >0 ? restFreq[0]: Quantity(0.0, "Hz");
    if ( qrestfreq.getValue("Hz")==0 ) {
      Int fld = rvi->fieldId();
      MSDopplerUtil msdoppler(msobj);
      Vector<Double> restfreqvec;
      msdoppler.dopplerInfo(restfreqvec, spwids[0], fld);
      qrestfreq = restfreqvec.nelements() >0 ? Quantity(restfreqvec[0],"Hz"): Quantity(0.0, "Hz");
    }
    Double refPix;
    Vector<Double> chanFreq;
    Vector<Double> chanFreqStep;
    String specmode;

    Double freqmin=0, freqmax=0;
    rvi->getFreqInSpwRange(freqmin,freqmax,freqFrameValid? freqFrame:MFrequency::REST );

    if (!getImFreq(chanFreq, chanFreqStep, refPix, specmode, obsEpoch, 
      obsPosition, dataChanFreq, dataChanWidth, dataFrame, qrestfreq, freqmin, freqmax))
      throw(AipsError("Failed to determine channelization parameters"));

    Bool nonLinearFreq(false);
    String veltype_p=veltype;
    veltype_p.upcase(); 
    if (veltype_p.contains("OPTICAL") || veltype_p.matches("Z") || veltype_p.contains("BETA") ||
        veltype_p.contains("RELATI") || veltype_p.contains("GAMMA")) {
       nonLinearFreq= true;
    }

    SpectralCoordinate mySpectral;
    Double stepf;
    if (!nonLinearFreq) {
      Double startf=chanFreq[0];
      //Double stepf=chanFreqStep[0];
      if (chanFreq.nelements()==1) {
        stepf=chanFreqStep[0];
      }
      else { 
        stepf=chanFreq[1]-chanFreq[0];
      }
      Double restf=qrestfreq.getValue("Hz");
      //cerr<<" startf="<<startf<<" stepf="<<stepf<<" refPix="<<refPix<<" restF="<<restf<<endl;
      // once NOFRAME is implemented do this 
      //if (mode=="cubedata") {
      //  mySpectral = SpectralCoordinate(freqFrameValid ? MFrequency::NOFRAME : MFrequency::REST, 
      //	startf, stepf, refPix, restf);
      //}
      //else {
        mySpectral = SpectralCoordinate(freqFrameValid ? freqFrame : MFrequency::REST, 
		startf, stepf, refPix, restf);
      //}
    }
    else { // nonlinear freq coords - use tabular setting
      // once NOFRAME is implemented do this 
      //if (mode=="cubedata") { 
      //  mySpectral = SpectralCoordinate(freqFrameValid ? MFrequnecy::NOFRAME : MFrequency::REST,
      //             chanFreq, (Double)qrestfreq.getValue("Hz"));
      //}
      //else {
        mySpectral = SpectralCoordinate(freqFrameValid ? freqFrame : MFrequency::REST,
                 chanFreq, (Double)qrestfreq.getValue("Hz"));
      //}
    }
    //cout << "Rest Freq : " << restFreq << endl;

    for (uInt k=1 ; k < restFreq.nelements(); ++k)
      mySpectral.setRestFrequency(restFreq[k].getValue("Hz"));

    if (freqFrameValid) {
      mySpectral.setReferenceConversion(MFrequency::LSRK,obsEpoch,obsPosition,phaseCenter);   
    }

    //    cout << "RF from coordinate : " << mySpectral.restFrequency() << endl;

    ////////////////// Stokes Coordinate
   
    Vector<Int> whichStokes = decideNPolPlanes(stokes);
    if(whichStokes.nelements()==0)
      throw(AipsError("Stokes selection of " + stokes + " is invalid"));
    StokesCoordinate myStokes(whichStokes);

    //////////////////  Build Full coordinate system. 

    CoordinateSystem csys;
    csys.addCoordinate(myRaDec);
    csys.addCoordinate(myStokes);
    csys.addCoordinate(mySpectral);
    csys.setObsInfo(myobsinfo);

    //////////////// Set Observatory info, if MS is provided.
    // (remove this section after verified...)
    /***
    if( ! msobj.isNull() )
      {
	//defining observatory...needed for position on earth
	ROMSColumns msc(msobj);
	String telescop = msc.observation().telescopeName()(0);
	MEpoch obsEpoch = msc.timeMeas()(0);
	MPosition obsPosition;
	if(!(MeasTable::Observatory(obsPosition, telescop)))
	  {
	    os << LogIO::WARN << "Did not get the position of " << telescop 
	       << " from data repository" << LogIO::POST;
	    os << LogIO::WARN 
	       << "Please contact CASA to add it to the repository."
	       << LogIO::POST;
	    os << LogIO::WARN << "Frequency conversion will not work " << LogIO::POST;
	  }
	
	ObsInfo myobsinfo;
	myobsinfo.setTelescope(telescop);
	myobsinfo.setPointingCenter(mvPhaseCenter);
	myobsinfo.setObsDate(obsEpoch);
	myobsinfo.setObserver(msc.observation().observer()(0));

	/// Attach obsInfo to the CoordinateSystem
	csys.setObsInfo(myobsinfo);

      }// if MS is provided.
      ***/

    return csys;
  }

  Bool SynthesisParamsImage::getImFreq(Vector<Double>& chanFreq, Vector<Double>& chanFreqStep, 
                                       Double& refPix, String& specmode,
                                       const MEpoch& obsEpoch, const MPosition& obsPosition, 
                                       const Vector<Double>& dataChanFreq, 
                                       const Vector<Double>& dataChanWidth,
                                       const MFrequency::Types& dataFrame, 
                                       const Quantity& qrestfreq, const Double& freqmin, const Double& freqmax) const
  {

    String start, step;
    specmode = findSpecMode(mode);
    //cerr<<"specmode="<<specmode<<" mode="<<mode<<endl;
    Bool verbose("true"); // verbose logging messages from calcChanFreqs
    LogIO os( LogOrigin("SynthesisParamsImage","getImFreq",WHERE) );
    //os<<LogIO::NORMAL<<"getImFreq"<<LogIO::POST;
    //cerr<<"qrestfreq="<<qrestfreq<<endl;
    //cerr<<" freqframe="<<MFrequency::showType(freqFrame)<<endl;

    refPix=0.0; 
    Bool descendingfreq(false);

    if ( mode.contains("cube") ) { 
      // For the returned chanfreqs and chanwidths from calcChanFreqs()
      String restfreq=String::toString(qrestfreq.getValue(qrestfreq.getUnit()))+qrestfreq.getUnit();
      String freqframe=MFrequency::showType(freqFrame);

      if ( specmode=="channel" ) {
        start = String::toString(chanStart);
        step = String::toString(chanStep); 
        // negative step -> descending channel indices 
        if (step.contains(casa::Regex("^-"))) descendingfreq=true;
      }
      else if ( specmode=="frequency" ) {
        if ( freqStart.getValue("Hz") == 0 ) {
           start = String::toString( freqmin ) + freqStart.getUnit();
        }
        else {
          start = String::toString( freqStart.getValue(freqStart.getUnit()) )+freqStart.getUnit();  
        }
        step = String::toString( freqStep.getValue(freqStep.getUnit()) )+freqStep.getUnit();  
        // negative freq width -> descending freq ordering
        if (step.contains(casa::Regex("^-"))) descendingfreq=true;
      }
      else if ( specmode=="velocity" ) {
        // if velStart is empty set start to vel of freqmin or freqmax?
        // add freq to vel conversion here
        if ( velStart.getValue(velStart.getUnit()) == 0 && !(velStart.getUnit().contains("m/s")) ) {
          start = "";
        }
        else { 
          start = String::toString( velStart.getValue(velStart.getUnit()) )+velStart.getUnit();  
        }
        step = String::toString( velStep.getValue(velStep.getUnit()) )+velStep.getUnit();  
        // positive velocity width -> descending freq ordering
        if (!step.contains(casa::Regex("^-"))) descendingfreq=true;
      }

      if (step=='0') step="";

      String infreqframe = freqframe;
      if ( infreqframe=="SOURCE" && mode!="cubesrc") {
        os << LogIO::SEVERE << "freqframe=\"SOURCE\" is only allowed for mode=\"cubesrc\""
           << LogIO::EXCEPTION;
        return false; 
      }
      // cubedata mode: input start, step are those of the input data frame
      if ( mode=="cubedata" ) infreqframe=dataFrame; 

      // *** NOTE *** 
      // calcChanFreqs alway returns chanFreq in
      // ascending freq order. 
      // for step < 0 calcChanFreqs returns chanFreq that 
      // contains start freq. in its last element of the vector. 
      //
      Bool rst=SubMS::calcChanFreqs(os,
                           chanFreq, 
                           chanFreqStep,
                           dataChanFreq,
                           dataChanWidth,
                           phaseCenter,
                           dataFrame,
                           obsEpoch,
                           obsPosition,
                           specmode,
                           nchan,
                           start,
                           step,
                           restfreq,
                           infreqframe,
                           veltype,
                           verbose 
                           );

      //cerr<<"chanFreq.nelements()="<<chanFreq.nelements()<<endl;
      //cerr<<"chanFreq 0="<<chanFreq[0]<<endl;
      //cerr<<"chanFreq last="<<chanFreq[chanFreq.nelements()-1]<<endl;
      if (!rst) {
        os << LogIO::SEVERE << "calcChanFreqs failed, check input start and width parameters"
           << LogIO::EXCEPTION;
        return False;
      }
      if (descendingfreq) {
        // reverse the freq vector if necessary so the first element can be
        // used to set spectralCoordinates in all the cases.
        //
        // also do for chanFreqStep..
        std::vector<Double> stlchanfreq;
        chanFreq.tovector(stlchanfreq);
        std::reverse(stlchanfreq.begin(),stlchanfreq.end());
        chanFreq=stlchanfreq;
        chanFreqStep=-chanFreqStep;
      }
    }
    else if ( mode=="mfs" ) {
      chanFreq.resize(1);
      chanFreqStep.resize(1);
      chanFreqStep[0] = freqmax - freqmin;
      Double freqmean = (freqmin + freqmax)/2;
      if (refFreq.getValue("Hz")==0) {
        chanFreq[0] = freqmean;
        refPix = 0.0;
      }
      else { 
        chanFreq[0] = refFreq.getValue("Hz"); 
        refPix  = (refFreq.getValue("Hz") - freqmean)/chanFreqStep[0];
      }
    }
    else {
       // unrecognized mode, error
       os << LogIO::SEVERE << "mode="<<mode<<" is unrecognized."
          << LogIO::EXCEPTION;
       return False;
    }

    return True;

  }//getImFreq

  String SynthesisParamsImage::findSpecMode(const String& mode) const
  {
    String specmode;
    specmode="channel";
    if ( mode.contains("cube") ) {
      // if velstart or velstep is defined -> specmode='vel'
      // else if freqstart or freqstep is defined -> specmode='freq'
      // velocity: assume unset if velStart => 0.0 with no unit
      //           assume unset if velStep => 0.0 with/wiout unit
      if ( !(velStart.getValue()==0.0 and velStart.getUnit()=="" ) ||
           !( velStep.getValue()==0.0)) { 
        specmode="velocity";
      }
      else if ( !(freqStart.getValue()==0.0 and freqStart.getUnit()=="") ||
                !(freqStep.getValue()==0.0)) {
        specmode="frequency";
      }
    }
    return specmode;
  }


  Vector<Int> SynthesisParamsImage::decideNPolPlanes(const String& stokes) const
  {
    Vector<Int> whichStokes(0);
    if(stokes=="I" || stokes=="Q" || stokes=="U" || stokes=="V" || 
       stokes=="RR" ||stokes=="LL" || 
       stokes=="XX" || stokes=="YY" ) {
      whichStokes.resize(1);
      whichStokes(0)=Stokes::type(stokes);
    }
    else if(stokes=="IV" || stokes=="IQ" || 
	    stokes=="RRLL" || stokes=="XXYY" ||
	    stokes=="QU" || stokes=="UV"){
      whichStokes.resize(2);
      
      if(stokes=="IV"){ whichStokes[0]=Stokes::I; whichStokes[1]=Stokes::V;}
      else if(stokes=="IQ"){whichStokes[0]=Stokes::I; whichStokes[1]=Stokes::Q;}
      else if(stokes=="RRLL"){whichStokes[0]=Stokes::RR; whichStokes[1]=Stokes::LL;}
      else if(stokes=="XXYY"){whichStokes[0]=Stokes::XX; whichStokes[1]=Stokes::YY; }
      else if(stokes=="QU"){whichStokes[0]=Stokes::Q; whichStokes[1]=Stokes::U; }
      else if(stokes=="UV"){ whichStokes[0]=Stokes::U; whichStokes[1]=Stokes::V; }
	
    }
  
    else if(stokes=="IQU" || stokes=="IUV") {
      whichStokes.resize(3);
      if(stokes=="IUV")
	{whichStokes[0]=Stokes::I; whichStokes[1]=Stokes::U; whichStokes[2]=Stokes::V;}
      else
	{whichStokes[0]=Stokes::I; whichStokes[1]=Stokes::Q; whichStokes[2]=Stokes::U;}
    }
    else if(stokes=="IQUV"){
      whichStokes.resize(4);
      whichStokes(0)=Stokes::I; whichStokes(1)=Stokes::Q;
      whichStokes(2)=Stokes::U; whichStokes(3)=Stokes::V;
    }
      
    return whichStokes;
  }// decidenpolplanes

  IPosition SynthesisParamsImage::shp() const
  {
    return IPosition( 4, imsize[0], imsize[1], ( decideNPolPlanes(stokes) ).nelements(), nchan );
  }



 /////////////////////// Grid/FTMachine Parameters

  SynthesisParamsGrid::SynthesisParamsGrid():SynthesisParams()
  {
    setDefaults();
  }

  SynthesisParamsGrid::~SynthesisParamsGrid()
  {
  }


  void SynthesisParamsGrid::fromRecord(Record &inrec)
  {
    setDefaults();

    String err("");

    try
      {
	
	// FTMachine parameters
	err += readVal( inrec, String("ftmachine"), ftmachine );
	err += readVal( inrec, String("padding"), padding );
	err += readVal( inrec, String("useautocorr"), useAutoCorr );
	err += readVal( inrec, String("usedoubleprec"), useDoublePrec );
	err += readVal( inrec, String("wprojplanes"), wprojplanes );
	err += readVal( inrec, String("convfunc"), convFunc );
	
	// facets	
	err += readVal( inrec, String("facets"), facets);

	// Track moving source ?
	err += readVal( inrec, String("distance"), distance );
	err += readVal( inrec, String("tracksource"), trackSource );
	err += readVal( inrec, String("trackdir"), trackDir );

	// The extra params for WB-AWP
	err += readVal( inrec, String("aterm"), aTermOn );
	err += readVal( inrec, String("psterm"), psTermOn );
	err += readVal( inrec, String("mterm"), mTermOn );
 	err += readVal( inrec, String("wbawp"), wbAWP );
	err += readVal( inrec, String("cfcache"), cfCache );
	err += readVal( inrec, String("dopointing"), doPointing );
	err += readVal( inrec, String("dopbcorr"), doPBCorr );
	err += readVal( inrec, String("conjbeams"), conjBeams );
	err += readVal( inrec, String("computepastep"), computePAStep );
	err += readVal( inrec, String("rotatepastep"), rotatePAStep );

	// Single or MultiTerm mapper
	err += readVal( inrec, String("mtype"), mType );

	err += verify();
	
      }
    catch(AipsError &x)
      {
	err = err + x.getMesg() + "\n";
      }
      
      if( err.length()>0 ) throw(AipsError("Invalid Gridding/FTM Parameter set : " + err));
      
  }

  String SynthesisParamsGrid::verify()
  {
    String err;

    // Check for valid FTMachine type.
    // Valid other params per FTM type, etc... ( check about nterms>1 )

    if( ftmachine=="mosaicft" && mType=="imagemosaic"  || 
	ftmachine=="awprojectft" && mType=="imagemosaic" )
      {  err +=  "Cannot use " + ftmachine + " with " + mType + " because it is a redundant choice for mosaicing. In the future, we may support the combination to signal the use of single-pointing sized image grids during gridding and iFT, and only accumulating it on the large mosaic image. For now, please set either mappertype='default' to get mosaic gridding  or ftmachine='ft' or 'wprojectft' to get image domain mosaics. \n"; }

    if( facets < 1 )
      {err += "Must have at least 1 facet\n"; }

    if( ftmachine=="awprojectft" && facets>1 )
      {err += "The awprojectft gridder supports A- and W-Projection. Instead of using facets>1 to deal with the W-term, please set the number of wprojplanes to a value > 1 to trigger the combined AW-Projection algorithm. \n";  } // Also, the way the AWP cfcache is managed, even if all facets share a common one so that they reuse convolution functions, the first facet's gridder writes out the avgPB and all others see that it's there and don't compute their own. As a result, the code will run, but the first facet's weight image will be duplicated for all facets.  If needed, this must be fixed in the way the AWP gridder manages its cfcache. But, since the AWP gridder supports joint A and W projection, facet support may never be needed in the first place... 

    if( ftmachine=="mosaicft" && facets>1 )
      { err += "The combination of mosaicft gridding with multiple facets is not supported. Please use the awprojectft gridder instead, and set wprojplanes to a value > 1 to trigger AW-Projection. \n"; }

    return err;
  }

  void SynthesisParamsGrid::setDefaults()
  {
    // FTMachine parameters
    ftmachine="GridFT";
    padding=1.0;
    useAutoCorr=False;
    useDoublePrec=True; 
    wprojplanes=1; 
    convFunc="SF"; 
    
    // facets
    facets=1;

    // Moving phase center ?
    distance=Quantity(0,"m");
    trackSource=False;
    trackDir=MDirection(Quantity(0.0, "deg"), Quantity(90.0, "deg"));

    // The extra params for WB-AWP
    aTermOn    = True;
    psTermOn   = True;
    mTermOn    = False;
    wbAWP      = True;
    cfCache  = "";
    doPointing = False;
    doPBCorr   = True;
    conjBeams  = True;
    computePAStep=360.0;
    rotatePAStep=5.0;

    // Mapper type
    mType = String("default");
    
  }

  Record SynthesisParamsGrid::toRecord()
  {
    Record gridpar;

    // FTMachine params
    gridpar.define("ftmachine", ftmachine);
    gridpar.define("padding", padding);
    gridpar.define("useautocorr",useAutoCorr );
    gridpar.define("usedoubleprec", useDoublePrec);
    gridpar.define("wprojplanes", wprojplanes);
    gridpar.define("convfunc", convFunc);

    gridpar.define("facets", facets);

    gridpar.define("distance", QuantityToString(distance));
    gridpar.define("tracksource", trackSource);
    gridpar.define("trackdir", MDirectionToString( trackDir ));

    gridpar.define("aterm",aTermOn );
    gridpar.define("psterm",psTermOn );
    gridpar.define("mterm",mTermOn );
    gridpar.define("wbawp", wbAWP);
    gridpar.define("cfcache", cfCache);
    gridpar.define("dopointing",doPointing );
    gridpar.define("dopbcorr", doPBCorr);
    gridpar.define("conjbeams",conjBeams );
    gridpar.define("computepastep", computePAStep);
    gridpar.define("rotatepastep", rotatePAStep);

    gridpar.define("mtype", mType);

    return gridpar;
  }



} //# NAMESPACE CASA - END

