//# SimObservations.h: container for descriptions of simulated observations
//# Copyright (C) 2002,2003
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

#ifndef SIMULATORS_SIMOBSERVATIONS_H
#define SIMULATORS_SIMOBSERVATIONS_H

#include <simulators/Simulators/SimTelescope.h>
#include <simulators/Simulators/SimSource.h>
#include <simulators/Simulators/SimDoppler.h>
#include <simulators/Simulators/SimField.h>
#include <simulators/Simulators/SimDataDesc.h>

namespace casa { //# NAMESPACE CASA - BEGIN

// <summary> 
// A container for descriptions of simulated observations
// </summary>
// <reviewed reviewer="" date="" tests="">
//
// <prerequisite>
// </prerequisite>
//
// <etymology>
// This class describes simulated observations
// </etymology>
//
// <synopsis> 
// This class serves as a container for describing a simulated observations
// with an interferometer which, when used in tandem with 
// <linkto class="SimTelescope">SimTelescope</linkto>, can be used to 
// describe simulated data.  The difference between SimObservations and 
// <linkto class="SimTelescope">SimTelescope</linkto> is somewhat 
// arbitrary: this class focuses on the parameters that in principle are 
// determined at observation time--namely, the desired sources, fields, 
// and spectral lines, as well as the mapping of polarization correllations 
// to spectral windows.  These classes provide methods for
// defining the telescope and observations; a simulator class, such as 
// <linkto class="SimpleSimulator">SimpleSimulator</linkto> uses them to 
// actually carry out the simulated observations.  <p>
// 
// This class uses some underlying container classes for storing the 
// specific parts of the configuration which correspond roughly to records 
// in the relevent sub-tables of a Measurement Set:  
// <ul>
//   <li> DATA_DESCRIPTION:  <linkto class="SimDataDesc">SimDataDesc</linkto>
//   <li> SOURCE:  <linkto class="SimSource">SimSource</linkto>
//   <li> FIELD:  <linkto class="SimField">SimField</linkto>
//   <li> DOPPLER:  <linkto class="SimDoppler">SimDoppler</linkto>
// Normally, these classes are not handled directly by users of this class; 
// however, it is possible gain direct access when finer read or write control 
// over the data they hold is necessary.  <p>
//
// The data contained in this object will ultimately be written out to a 
// Measurement Set.  To accomplish this, one must "attach" an output 
// MeasurementSet to this object.  This can be done either at construction
// time or later via the attach() or initAndAttach() function.  If 
// initAndAttach() is used or attachment is done at construction, any data
// already recorded in the MS, such as a source list, are read-in and 
// loaded into this object.  New data is written out via the flush() function.
// The "attachment" model allows the class to keep track of what's been 
// written to disk already.  Thus, one can update the telescope configuration,
// e.g. add additional sources, and write out only the new data via the 
// flush() command.  <p>
//
// This class also manages a 
// <linkto class="SimTelescope">SimTelescope</linkto> object describing 
// the telescope that the observations are to be carried out on.  The 
// SimTelescope should be set at construction time.  One can access the 
// SimTelescope directly via the telescope() function.  Attaching 
// an MS to a SimObservations object will in turn attach it to its 
// underlying SimTelescope.  Accordingly, calling SimObservations::flush() 
// will implicitly call SimTelescope::flush().  <p>
// </synopsis> 
//
// <motivation>
// By breaking out observation description from MSSimulator, the simulator can 
// take on more complex sequences of observations from multiple telescopes,
// subarrays, configurations, etc.
// </motivation>
//
// <todo asof="03/03/25">
//   <li> make sure Data Descriptions are valid
//   <li> loading from an existing MS needs more testing.
//   <li> may replace SimDoppler with SimSpLine which would not 
//        cause a DOPPLER table to be written out.
// </todo>
class SimObservations {
public:

    // Create an undescribed set of observation setups on a defined telescope.
    // If dodel is True, this object will delete the pointer to tel when it 
    // is done with it.  
    SimObservations(SimTelescope *tel, Bool dodel=False);

    // Create an observational setup that is a copy of another setup
    SimObservations(const SimObservations& obs);

    // Create a set of observation setups described by the data in a 
    // MeasurementSet.  A telescope description is generated automatically.
    // The first version of this function (taking a const MeasurmentSet) 
    // simply calls init(ms), while the second one also calls initAndAttach().
    // <group>
    SimObservations(const MeasurementSet &ms);
    SimObservations(MeasurementSet &ms);
    // </group>

    virtual ~SimObservations();

    // assignment operator
    SimObservations& operator=(const SimObservations& s);

    // Load the description given by the data in a MeasurementSet
    // The SOURCE and FIELD
    // subtables will be read to determine what was observed,
    // then the main table is scanned to determine the currently defined 
    // subarrays.  (This constructor will read the MeasurementSet as if it 
    // describes a single telescope; thus, some complexity may be lost.)
    // The second version will also call attach(ms, False);
    // <group>
    void init(const MeasurementSet &ms);
    void initAndAttach(MeasurementSet &ms);
    // </group>

    // Attach this description to a MeasurementSet so that when flush() is 
    // called, the description will get written to it.  
    //  clear, if set to True, will cause all internal flags that mark 
    //         which descriptions have been written so far to be cleared.
    //         Thus, the next time flush is called, all configuration records
    //         currently set will be written to the attached MS.
    void attach(MeasurementSet &ms, Bool clear=False);

    // Write out the current description to a given MeasurementSet.  Only 
    // those "new" records that have not already been written out to the MS 
    // will be flushed.  
    void flush();

    // return a pointer to the attached telescope description
    // <group>
    SimTelescope *telescope() { return tel_p; }
    const SimTelescope *telescope() const { return tel_p; }
    // </group>

    // return a pointer to the currently attached Measurement Set or null
    // if none is currently attached.
    MeasurementSet *ms() { return tel_p->ms(); }

    // define a data description set that connects a polarization setup to 
    // a set of spectral windows.
    //  polId    is the ID for the polarization setup
    //  spwin    points to the set of spectral windows to associate with this
    //             polarization setup.  If the pointer is empty, the 
    //             polarization setup will be associated with all currently
    //             defined windows.
    // <group>
    uInt addDataDesc(uInt polId, uInt spw, Bool upto=False) {
	dds_p.addDataDesc(polId, spw, upto);
	return dds_p.numDesc()-1;
    }
    uInt addDataDesc(uInt polId, const Vector<uInt>& spwids) {
	dds_p.addDataDesc(polId, spwids);
	return dds_p.numDesc()-1;
    }
    // </group>

    // return the number of data descriptions.  This number represents the 
    // number of SimDataDesc objects that can be accessed via dataDesc(uInt).
    // Each description potentially covers a range of data description IDs.
    uInt numDataDesc() const { return dds_p.numDesc(); }

    // return the total number of data description IDs defined.
    uInt numDataDescIds() const { return dds_p.numIds(); }

    // return an existing Data Description
    // <group>
    const SimDataDesc& dataDesc(uInt i) const { return dds_p.dataDesc(i); }
    SimDataDesc& dataDesc(uInt i) { return dds_p.dataDesc(i); }
    // </group>

    // create a mapping from the data description id to the spectral window
    // id for the current telescope and observation configurations
    void loadDataDesc2SpwId(Vector<uInt>& map) { 
	dds_p.loadSpWinMap(map, tel_p->numPolSetups(), tel_p->numSpWins());
    }

    // add an observable source.  The size of the input arrays should be 
    // equal to the number of spectral line transitions to be observed
    // with this source.
    //  name   	  is the name of the source.
    //  dir    	  is the source's direction in the sky.
    //  code   	  is a a string used to identify the special 
    //         	     characteristics of the source or its role in the 
    //         	     observations.
    //  spWinId   is the spectral window to associate with this source
    //               description; an ID less than 0 refers to all windows.
    uInt addSource(const String& name, const MDirection& dir, 
		   const String& code="", Int spWinId=-1)
    {
	uInt idx = srcs_p.numSources();
	srcs_p.addSource(name, dir, code, spWinId).setId(idx);
	return idx;
    }

    // return the number of Source descriptions
    uInt numSources() const { return srcs_p.numSources(); }

    // return an existing Source
    // <group>
    SimSource& source(uInt i) { return srcs_p[i]; }
    const SimSource& source(uInt i) const { return srcs_p[i]; }
    // </group>

    // set a field center for observing a source
    //  srcId       is the source ID that this field is associated with.
    //  dir         is the field's direction in the sky.
    //  repetition  is the number of consecutive integrations to perform on 
    //                 this field.  A value of zero means a suitable default 
    //                 value should be used.  
    uInt addField(uInt srcId, const MDirection& dir, uInt repetition=0) {
	flds_p.addField(srcId, dir).setRepetition(repetition);
	return flds_p.numFields()-1;
    }

    // set a field center for observing a source
    //  srcId       is the source ID that this field is associated with.
    //  offRA       is the true angular offset in the direction
    //                perpendicular to the celestial pole (i.e. along a great
    //                circle) of the field center from the source position
    //                given by dir.  
    //  offDec      is the angular offset in the direction of the 
    //                celestial (north) pole of the field center from the 
    //                source position.  When given as a Double, radians is 
    //                assumed as the unit.
    //  repetition  is the number of consecutive integrations to perform on 
    //                this field.  A value of zero means a suitable default 
    //                value should be used.  
    // <group>
    uInt addField(uInt srcId, const Quantity& offRA, const Quantity& offDec, 
		  uInt repetition=0) 
    {
	return addField(srcId, offRA.getValue("rad"), offDec.getValue("rad"),
			repetition);
    }
    uInt addField(uInt srcId, const Double offRA, const Double offDec, 
		  uInt repetition=0) 
    {
	MDirection dir = srcs_p[srcId].direction();
	flds_p.addField(srcId, dir, offRA, offDec).setRepetition(repetition);
	return flds_p.numFields()-1;
    }
    // </group>

    // return the total number of fields currently defined
    uInt numFields() const { return flds_p.numFields(); }

    // return the number of fields currently defined for a given source
    uInt numFields(uInt srcid) const { 
	uInt nf=0;
	for(uInt i=0; i < flds_p.numFields(); i++) {
	    if (flds_p[i].sourceId() == srcid) nf++;
	}
	return nf;
    }

    // return an existing Field
    // <group>
    SimField& field(uInt i) { return flds_p[i]; }
    const SimField& field(uInt i) const { return flds_p[i]; }
    // </group>

    // associate a spectral line with of a subset of the spectral windows,
    // consequently setting the doppler tracking.  
    //  srcId      is the source the line is being observed in.
    //  trans      is the name of the spectral line transition (e.g. 
    //                "CO J=1-0"); this string should be unique for the 
    //                the rest frequency.
    //  restfreq   is the rest frequency of the line.  
    //  spWinId    is the ID of a single window to associate line with;
    //  spWinIds   is a list of window IDs.
    // Returned is the Doppler ID to be associated with the line.  
    // An exception is thrown if any of the spectral windows are already
    // associated with a spectral line.  
    // <group>
    uInt addSpectralLine(uInt srcId, const String& trans, 
			 const MFrequency &restfreq, uInt spWinId=-1) 
    {
	dops_p.addDoppler(srcId, restfreq, trans, spWinId);
	return dops_p.numDopplers()-1;
    }
    uInt addSpectralLine(uInt srcId, const String& trans, 
			 const MFrequency &restfreq, Vector<uInt> spWinIds)
    {
	dops_p.addDoppler(srcId, restfreq, trans, spWinIds);
	return dops_p.numDopplers()-1;
    }
    // </group>

    // return the number of spectral lines currently defined
    uInt numSpectralLines() { return dops_p.numDopplers(); }

    // return an existing Source
    // <group>
    SimDoppler& spectralLine(uInt i) { return dops_p[i]; }
    const SimDoppler& spectralLine(uInt i) const { return dops_p[i]; }
    // </group>

private:
    Bool deltel_p;
    SimTelescope *tel_p;
    
    SimSourceList srcs_p;
    SimFieldList flds_p;
    SimDopplerList dops_p;
    SimDataDescList dds_p;
};

} //# NAMESPACE CASA - END

#endif
