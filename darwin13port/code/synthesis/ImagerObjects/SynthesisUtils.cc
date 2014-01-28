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



} //# NAMESPACE CASA - END

