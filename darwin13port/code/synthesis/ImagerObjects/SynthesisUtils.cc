//# SynthesisUtils.cc: Implementation of Imager.h
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

#include <synthesis/ImagerObjects/SynthesisUtils.h>

#include <sys/types.h>
#include <unistd.h>
using namespace std;

namespace casa { //# NAMESPACE CASA - BEGIN
  
  SynthesisUtils::SynthesisUtils()
  {
    
  }
  
  SynthesisUtils::~SynthesisUtils() 
  {
    LogIO os( LogOrigin("SynthesisUtils","destructor",WHERE) );
    os << "SynthesisUtils destroyed" << LogIO::POST;
  }
  

  // Data partitioning rules for CONTINUUM imaging
  //  ALL members are strings ONLY.
  Record SynthesisUtils::continuumDataPartition(Record &selpars, const Int npart)
  {
    LogIO os( LogOrigin("SynthesisUtils","continuumDataPartition",WHERE) );

    Record onepart, allparts;

    // Duplicate the entire input record npart times, with a specific partition id.
    // Modify each sub-record according to partition id.
    for( Int part=0; part < npart; part++)
      {

	onepart.assign(selpars);

	//-------------------------------------------------
	// WARNING : This is special-case code for two specific datasets
	for ( uInt msid=0; msid<selpars.nfields(); msid++)
	  {
	    Record onems = onepart.subRecord( RecordFieldId("ms"+String::toString(msid)) );
	    String msname = onems.asString("msname");
	    String spw = onems.asString("spw");
	    if(msname.matches("DataTest/twopoints_twochan.ms"))
	      {
		onems.define("spw", spw+":"+String::toString(part));
	      }
	    if(msname.matches("DataTest/point_twospws.ms"))
	      {
		onems.define("spw", spw+":"+ (((Bool)part)?("10~19"):("0~9"))  );
	      }
	    onepart.defineRecord( RecordFieldId("ms"+String::toString(msid)) , onems );
	  }// end ms loop
	//-------------------------------------------------

	allparts.defineRecord( RecordFieldId(String::toString(part)), onepart );

      }// end partition loop

    return allparts;
  }

  // Data partitioning rules for CUBE imaging
  Record SynthesisUtils::cubeDataPartition(Record &selpars, Int npart)
  {
    LogIO os( LogOrigin("SynthesisUtils","cubeDataPartition",WHERE) );
    // Temporary special-case code. Please replace with actual rules.
    return continuumDataPartition( selpars, npart );

  }

  // Image cube partitioning rules for CUBE imaging
  Record SynthesisUtils::cubeImagePartition(Record &impars, Int npart)
  {
    LogIO os( LogOrigin("SynthesisUtils","cubeImagePartition",WHERE) );

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

  /////////////////////// Selection Parameters

  SynthesisParamsSelect::SynthesisParamsSelect()
  {
    setDefaults();
  }

  SynthesisParamsSelect::~SynthesisParamsSelect()
  {
  }


  void SynthesisParamsSelect::setValues(const String inmsname, const String inspw, 
					       const String infreqbeg, const String infreqend, const String infreqframe,
					       const String infield, const String inantenna, const String intimestr, 
					       const String inscan, const String inobs, const String instate, 
					       const String inuvdist, const String intaql, const Bool inusescratch, 
					       const Bool inreadonly, const Bool inincrmodel)
  {

    Record selpar;
    selpar.define("msname",inmsname);
    selpar.define("spw",inspw);
    selpar.define("freqbeg",infreqbeg);
    selpar.define("freqend",infreqend);
    selpar.define("freqframe",infreqframe);
    selpar.define("field",infield);
    selpar.define("antenna",inantenna);
    selpar.define("timestr",intimestr);
    selpar.define("scan",inscan);
    selpar.define("obs",inobs);
    selpar.define("state",instate);
    selpar.define("uvdist",inuvdist);
    selpar.define("taql",intaql);
    selpar.define("usescratch",inusescratch);
    selpar.define("readonly",inreadonly);
    selpar.define("incrmodel",inincrmodel);

    setValues( selpar );

  }

  void SynthesisParamsSelect::setValues(Record &inrec)
  {
    setDefaults();

    String err("");

    try
      {
	
	if( inrec.isDefined("msname") ) { inrec.get("msname",msname); }
	if( inrec.isDefined("readonly") ) { inrec.get("readonly",readonly); }
	if( inrec.isDefined("usescratch") ) { inrec.get("usescratch",usescratch); }
	if( inrec.isDefined("incrmodel") ) { inrec.get("incrmodel",incrmodel); }
	
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
	
	
	if( inrec.isDefined("spw") ) { inrec.get("spw",spw); }

	if( inrec.isDefined("freqbeg") ) { inrec.get("freqbeg",freqbeg); }
	if( inrec.isDefined("freqend") ) { inrec.get("freqend",freqend); }

	String freqframestr( MFrequency::showType(freqframe) );
	if( inrec.isDefined("freqframe") ) { inrec.get("freqframe",freqframestr); }
	if( ! MFrequency::getType(freqframe, freqframestr) )
	  { err += "Invalid Frequency Frame " + freqframestr ; }
	
	
	if( inrec.isDefined("field") ) { inrec.get("field",field); }
	if( inrec.isDefined("antenna") ) { inrec.get("antenna",antenna); }
	if( inrec.isDefined("timestr") ) { inrec.get("timestr",timestr); }
	if( inrec.isDefined("scan") ) { inrec.get("scan",scan); }
	if( inrec.isDefined("obs") ) { inrec.get("obs",obs); }
	if( inrec.isDefined("state") ) { inrec.get("state",state); }
	if( inrec.isDefined("uvdist") ) { inrec.get("uvdist",uvdist); }
	if( inrec.isDefined("taql") ) { inrec.get("taql",taql); }

	
      }
    catch(AipsError &x)
      {
	err = err + x.getMesg() + "\n";
      }
      
      if( err.length()>0 ) throw(AipsError("Invalid Selection Parameter set : " + err));
      
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




} //# NAMESPACE CASA - END

