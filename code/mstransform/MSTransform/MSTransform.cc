///# MSTransform.cc: this defines MSTransform
//# Copyright (C) 2000,2001,2002
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
#include <stdarg.h>
#include <sstream>
#include <iostream>
#include <vector>
#include <casacore/casa/Utilities/Regex.h>
#include <casacore/casa/OS/HostInfo.h>
#include <casacore/casa/Exceptions/Error.h>
#include <mstransform/MSTransform/MSTransform.h>
#include <casacore/casa/stdio.h>
#include <casacore/casa/math.h>

namespace casa {


// -----------------------------------------------------------------------
// Default Constructor
// -----------------------------------------------------------------------
MSTransform::MSTransform ()
{
	mdh_p = NULL;

	done();
}


// -----------------------------------------------------------------------
// Default Destructor
// -----------------------------------------------------------------------
MSTransform::~MSTransform ()
{
	done();
}

void
MSTransform::done()
{
	if(mdh_p){
		delete mdh_p;
		mdh_p = NULL;
	}

	// Default values of parameters
	msname_p = "";
	outputms_p = "";
	spw_p = "";
	scan_p = "";
	field_p = "";
	antenna_p = "";
	timerange_p = "";
	correlation_p = "";
	intent_p = "";
	feed_p = "";
	array_p = "";
	uvrange_p = "";
	observation_p = "";

	config_p = Record();
	datacolumn_p = "CORRECTED";
	isconfigured_p = false;

/*
	if (! dataselection_p.empty()) {
		Record temp;
		dataselection_p = temp;
	}
*/

	return;
}

/*
// ---------------------------------------------------------------------
// MSTransform::defaultOptions
// Set the defaults for all the parameters
// Returns a Record with the default of each parameter
// ---------------------------------------------------------------------
Record &
MSTransform::defaultOptions()
{
	Record defaults;

	defaults.define("inputms", "");
	defaults.define("outputms", "");
	defaults.define("createmms", true);
	defaults.define("separationaxis", "both");
	defaults.define("numsubms", "");
	defaults.define("tileshape", "");
	defaults.define("spw", "");
	defaults.define("scan", "");
	defaults.define("antenna", "");
	defaults.define("array", "");
	defaults.define("correlation", "");
	defaults.define("field", "");
	defaults.define("timerange", "");
	defaults.define("uvrange", "");
	defaults.define("state", "");
	defaults.define("observation", "");
	defaults.define("datacolumn", "CORRECTED");
	defaults.define("realmodelcol", false);
	defaults.define("combinespws", false);
	defaults.define("freqaverage", false);
	defaults.define("freqbin", "");
	defaults.define("useweights", "");
	defaults.define("hanning", false);
	defaults.define("regridms", false);
	defaults.define("mode", "");
	defaults.define("nchan", "");
	defaults.define("start", "");
	defaults.define("width", "");
	defaults.define("interpolation", "");
	defaults.define("phasecenter", "");
	defaults.define("restfreq", "");
	defaults.define("outframe", "");
	defaults.define("veltype", "");
	defaults.define("separatespws", false);
	defaults.define("nspws", "");
	defaults.define("timeaverage", false);
	defaults.define("timebing", "");
	defaults.define("timespan", "");
	defaults.define("quantize_c", "");
	defaults.define("minbaselines", "");

	return defauts;
}
*/


// ---------------------------------------------------------------------
// MSTransform::configure
// Configure the MSTransformDataHandler and the parameters.
// The config Record is mandatory and needs to have at least
// the parameters for the input and output MSs.
// This method may be called again to add or change parameters.
// ---------------------------------------------------------------------
bool
MSTransform::configure(Record config)
{

	LogIO os(LogOrigin("MSTransform", __FUNCTION__));

	if (config.empty()){
		os << LogIO::SEVERE << "There is no configuration for the tool"
				<< LogIO::POST;
		return False;
	}

	// First time configuration
	if (!isconfigured_p){

		// The minimum configuration are the input and output MS names.
		if (config.isDefined("inputms"))
			config.get("inputms", msname_p);
		else{
			os << LogIO::SEVERE << "There is no \"inputms\" in configuration Record"
				<< LogIO::POST;
			return False;
		}

		if (config.isDefined("outputms"))
			config.get("outputms", outputms_p);
		else{
			os << LogIO::SEVERE << "There is no \"outputms\" in configuration Record"
				<< LogIO::POST;
			return False;
		}

		if(mdh_p) delete mdh_p;

		// Create an object for the MSTransformDataHandler
		mdh_p = new MSTransformDataHandler();
	}

	config_p = config;

	// The datacolumn must exist
	if (config_p.isDefined("datacolumn")){
		config_p.get("datacolumn", datacolumn_p);
		datacolumn_p.upcase();
		config_p.define("datacolumn", datacolumn_p);
	}
	else {
		// Add the default column to the Record
		config_p.define("datacolumn", datacolumn_p);
	}

	// Configure the MSTransformDataHandler object
	mdh_p->configure(config_p);
	isconfigured_p = true;

	// Check if all the data selection parameters are in the record.
	// If not, use the default values. Populate the class members
/*
	if (config_p.isDefined("spw"))
		config_p.get("spw", spw_p);
	if (config_p.isDefined("scan"))
		config_p.get("scan", scan_p);
	if (config_p.isDefined("field"))
		config_p.get("field", field_p);
	if (config_p.isDefined("antenna"))
		config_p.get("antenna", antenna_p);
	if (config_p.isDefined("timerange"))
		config_p.get("timerange", timerange_p);
	if (config_p.isDefined("correlation"))
		config_p.get("correlation", correlation_p);
	if (config_p.isDefined("intent"))
		config_p.get("intent", intent_p);
	if (config_p.isDefined("feed"))
		config_p.get("feed", feed_p);
	if (config_p.isDefined("array"))
		config_p.get("array", array_p);
	if (config_p.isDefined("uvrange"))
		config_p.get("uvrange", uvrange_p);
	if (config_p.isDefined("observation"))
		config_p.get("observation", observation_p);

*/

	return true;
}


// ---------------------------------------------------------------------
// MSTransform::setup
// Setup the MSTranformDataHandler and generate the iterators
// It assumes that MSTransform::configure is run first
// ---------------------------------------------------------------------
bool
MSTransform::open()
{

	LogIO os(LogOrigin("MSTransform", __FUNCTION__));

	if (! isconfigured_p){
		os << LogIO::SEVERE << "There is no configuration for the tool"
				<< LogIO::POST;
		return False;
	}

	if(!mdh_p){
		os << LogIO::SEVERE << "The tool was not configured" << LogIO::POST;
		return false;
	}

	// Open the MS and select the data
	mdh_p->open();


	// Setup the DataHandler
	mdh_p->setup();

	return true;
}

// ---------------------------------------------------------------------
// MSTransform::run
// Run the tool
// ---------------------------------------------------------------------
Record
MSTransform::run()
{

	LogIO os(LogOrigin("MSTransform", __FUNCTION__));

	if (! mdh_p){
		os << LogIO::SEVERE << "The tool is configured. Please run mt.config and mt.open first." << LogIO::POST;
		return Record();
	}

	vi::VisibilityIterator2 *visIter = mdh_p->getVisIter();
	vi::VisBuffer2 *vb = visIter->getVisBuffer();
	visIter->originChunks();
	while (visIter->moreChunks())
	{
		visIter->origin();
		while (visIter->more())
		{
			mdh_p->fillOutputMs(vb);
				visIter->next();
		}

		visIter->nextChunk();
	}

	mdh_p->close();
	delete mdh_p;
	mdh_p = NULL;

	return Record();
}


// ---------------------------------------------------------------------
// MSTransform::validateDataColumn
// Check if datacolumn is valid
// Return validated datacolumn
// ---------------------------------------------------------------------
/*
String
MSTransform::validateDataColumn(String datacol)
{
	String ret = "";
	Bool checkcol = false;
	datacol.upcase();

	LogIO os(LogOrigin("MSTransform", __FUNCTION__, WHERE));

	if (mdh_p->checkIfColumnExists(datacol))
		ret = datacol;
	else {
		// Check if default column exist
		if (mdh_p->checkIfColumnExists("CORRECTED"))
			ret = "CORRECTED";
	}

	return ret;
}
*/



} //#end casa namespace


