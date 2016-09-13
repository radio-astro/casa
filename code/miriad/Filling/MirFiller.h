//# MirFiller.h: defines the MirFiller class for filling Miriad data
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
//#
//# $Id: MirFiller.h,v 1.2 2009/09/03 23:28:32 pteuben Exp $

#ifndef BIMA_MIRFILLER_H
#define BIMA_MIRFILLER_H

#include <casa/Containers/List.h>
#include <casa/Arrays/Vector.h>
#include <casa/Arrays/Cube.h>
#include <casa/Arrays/Matrix.h>
#include <casa/Logging/LogIO.h>

#include <miriad/Filling/MirVarHandler.h>
#include <miriad/Filling/MirVisReader.h>

#include <casa/namespace.h>
//# Forward Declarations
namespace casa { //# NAMESPACE CASA - BEGIN
class Record;
class String;
class AipsError;
class MeasurementSet;
//class FillMetadata;
//class GlishRecord;
} //# NAMESPACE CASA - END

class MirPol;
class MirSource;
class MirFreqSetup;
class UnavailableMiriadDataSelectionError;

class FillMetadata;

// <summary>
// BIMA MIRIAD dataset to MeasurementSet2 filler
// </summary>
// 
// <use visibility=export>
// 
// <reviewed reviewer="" date="yyyy/mm/dd" tests="" demos="">
// </reviewed>
// 
// <prerequisite>
//   <li> <linkto class="Record">Record</linkto> -- for holding options
// </prerequisite>
//
// <etymology>
// MirFiller is the successor to the BimaFiller (class and application).  
// It attempts to be more general in its filling of Miriad data while still 
// retaining special functionality for BIMA data specifically.  
// </etymology>
//
// <synopsis>
// MirFiller converts an input Miriad dataset into an output MeasurementSet 
// (ver. 2).  It is attached to its Miriad dataset at construction; it can 
// subsequently be used to fill several output MSes, perhaps with different 
// data from the input dataset, in sequence (i.e. it is not thread-safe).  
//
// Default use of this class would be to construct an instance attached to 
// a Miriad dataset, and then call its <src>fill()</src> method, passing 
// it the output measurement set name.  This will load all data found in 
// the input dataset into the measurement set.  Restricting the filling to 
// specific spectral windows can be controled via the <src>select()</src> 
// function.  
//
// <h3>Miriad Dataset Spectral Layout and 
// its Mapping to the Measurement Set</h3>
// 
// Each record in a Miriad dataset organizes its correlation data by
// spectral windows and so-called wideband channels.  Each Miriad
// spectral window contains data from a band of frequencies, broken up
// into some number of channels.  (Because each channel, then, is
// fairly narrow in bandwidth, these correlations are often referred
// to as "narrow-band" data.)  The wideband channels, in general,
// contain single correlations, averaged over some band at potentially
// arbitrary frequencies; that is, they are bolometer-like
// measurements.
//
// With BIMA Miriad datasets, the wideband channels are used to store
// wide-band averages of the spectral windows.  More specifically, the
// first wideband channel contains the average of all spectral windows
// channels associated with the lower sideband (the first Nwin/2
// windows), and the second wideband channel contains the average of
// the upper sideband windows.  The remaining wideband channels
// contain the averages of each of the spectral windows (in the same
// order).  Thus, if the BIMA Miriad dataset contains Nwin spectral windows, 
// there will be Nwin+2 wideband channels.  The first Nwin/2 windows will
// be from the lower sideband, and the rest, from the upper sideband.  
// 
// When the Miriad data is converted to an MS, each spectral window and each 
// wideband channel is loaded into a separate MS spectral window.  Thus, for
// a BIMA dataset with Nwin windows, the output MS (by default) will contain 
// Nwin*2+2 spectral windows.  The <src>select()</src> function can be used
// to restrict the windows that get written.
//
// <h3>Polarization</h3>
// 
// Miriad datasets stores correlations for different polarizations in
// separate records, even if they were observed simultaneously.
// (Thus, it is up to the Miriad I/O software to properly combine
// records to form new Stokes correlations.)  This is in contrast to
// AIPS++ which can store polarizations observed at the same time in the 
// same record.  Furthermore, BIMA can only observe one polarization at 
// a time.  Thus, how polarizations should be loaded from a Miriad dataset
// into an AIPS++ Measurment Set depends in part on whether the 
// polarizations were (effectively) observed simultaneously.  It might 
// also depend on whether polarization calibration is needed and how it 
// should be done.  
//
// The standard recipe for handling raw BIMA polarization data calls for 
// the sampling of the polarizations to be loaded into separate records.  
// This makes time-based editing easier.  Once the polarization gains are
// applied, one usually time-averages the data over several polarization
// switching cycles; in effect, then, one can consider the polarizations 
// as having been observed simultaneously.  Thus, in this time-averaging
// step, one collapses the different polarizations into single records for
// standard handling by the imaging tools.  (Note: the software to support 
// this operation is still in development as of this writing.)
//
// The "joinpol" option (set with setOptions()) allows one to control 
// whether an attempt is made to load polarizations into the same record.  
// If joinpol is false, all polarizations are loaded into different records;
// each record will point to a different polarization setup in the 
// POLARIZATION table (with each setup containing only one correlation type).
// If joinpol is true, then fill() will look for polarizations with the same 
// time stamp and load them into the same record; the POLARIZATION table will
// then only have one setup containing all correlations.  Any polarizations 
// that are not found are set to zero.  Thus, for a raw BIMA data 
// containing multiple polarizations, joinpol equal to true will produce a
// much large dataset than joinpol equal to false because of all the extra
// zero visibilities that get loaded in.  
// </synopsis>
//
// <example>
// <srcblock>
//    // open Miriad dataset 3c273 for filling
//    MirFiller filler('3c273');
//
//    // if needed, set some options
//    Record opts = filler.getOptions();
//    RecordFieldPtr<Double>(opts, "scanlim").define(600);
//    RecordFieldPtr<Bool>(opts, "verbose").define(True);
//    filler.setOptions(opts);
//
//    // if desired, chose the windows to write
//    Vector<Int> wide(0)    // do not write out any wideband channels
//    Vector<Int> splwin(filler.nspect());
//    for(Int i=0; i < splwin.nelements(); i++) splwin(i) = i;
//    filler.select(wide, splwin);
//
//    // fill the output MS
//    filler.fill('3c273.ms');
// </srcblock>
// </example>
//
// <motivation>
// The important features of this class are:
// <ul>
//    <li> Input spectral windows are loaded into separate output windows.
//         This allows windows to have different rest frequencies associated
//         with them.  It also allows channels within a window to be 
//         contiguous in frequency and having the same width.
//    <li> Wideband channels are each placed in their own window.  This is 
//         particularly important for the sideband averages, which are used
//         to calculate the G-Jones which can be applied to the spectral 
//         windows.
//    <li> Associations are set to connect spectral windows and their averages.
//         This allows the averages to be recalculated if necessary (e.g.
//         after flagging).
//    <li> Different polarizations are by default loaded into separate 
//         records as is appropriate for raw BIMA data; however, if 
//         simultaneously measured polarizations exist, an option can be
//         set to join them into a single record.  
// </ul>
// </motivation>
//
// <thrown>
//    <li> <linkto class="AipsError">AipsError</linkto> -- if the input
//         dataset is not in Miriad format or cannot be opened.
//    <li> <linkto class="MiriadFormatError">MiriadFormatError</linkto>
//         -- if an illegal format condition is found in the input
//         Miriad dataset.
//    <li> <linkto class="UnavailableMiriadDataSelectionError">
//         UnavailableMiriadDataSelectionError</linkto> -- if the data
//         selection is known to be out of range.
// </thrown>
//
// <todo asof="2001/03/01">
//   <li> load planet source models
//   <li> a splitWindows option may be needed
//   <li> a more sophisticated handling of multiple polarization setups
//        may be needed
//   <li> system temperatures are currently loaded into the SYSCAL as 
//        single-element arrays, regardless of the "official" number of 
//        polarization receptors.  This is not strictly in compliance with 
//        the MS2 spec, so it should be fixed.
//   <li> change getOptions() interface to get output from user (?)
// </todo>

class MirFiller : public MirVarHandler {
private:
    uInt debug;  // debugging level

public:

    // create the filler, attaching it to a Miriad dataset.  <src>mirfile</src>
    // is the dataset filename, and <src>dbg</src> is the debug level (see 
    // <linkto class="MirFiller:setDebug"><src>setDebugLevel()</src></linkto>).
    MirFiller(const String& mirfile, Bool scan=True, Int dbg=0);

    // destruct this filler
    virtual ~MirFiller();

    // set the debugging level which controls the amount of debugging 
    // messages that are printed to the terminal (as opposed to the logger).  
    // A value of zero or less will cause no extra messages to be printed; 
    // increasing values will increase the amount of messages.
    void setDebugLevel(Int level) { 
	debug = (level < 0) ? 0 : (uInt) level; 
	rdr_p.setDebugLevel(debug);
    }

    // set the debugging level which controls the amount of debugging 
    // messages that are printed to the terminal.  A value of zero or less 
    // means that no extra messages will be printed; increasing values will 
    // increase the amount of messages.
    Int getDebugLevel() const { return debug; }

    // return true if the debugging level is at least as high as a given 
    // level.
    Bool Debug(Int level) { return (((uInt) level) <= debug); }

    // return true if this filler should be verbose in its messages.  This
    // will be true if the verbose option is enabled or the debug level is
    // greater than 1.
    Bool verbose() { return (verbose_p || debug > 1); }

    // return basic characteristics of the input dataset as a GlishRecord.  If
    // scan is True (the default), the entire file will be (re-) scanned 
    // using the current values for the obslim and scanlim options.
    // If verbose is true (the 
    // default), send a description to the logger.
    //GlishRecord summary(Bool verbose=True, Bool scan=True);

    // fill the output MS according to the current selections and options
    // msfile is the output name to use; it should not already exist.
    void fill(const String& msfile) {
	if (joinpol_p) 
	    joinFill(msfile);
	else 
	    noJoinFill(msfile);
    }

    // select spectral data from the input Miriad dataset.  wideChans is a 
    // list of one-based channel indices.  narrowWins is a list of one-based
    // window indices.  The default is to choose all available wide channels
    // and windows.  polsel is a polarization selection: only the correlation
    // types present will be loaded; a NULL value means take all polarizations
    // found.  (See also the joinpol option under setOptions().)
    void selectSpectra(const Vector<Int>& wideChans, 
		       const Vector<Int>& narrowWins);
//	throw(UnavailableMiriadDataSelectionError);

    // select the polarizations to load.  An empty setup means by default 
    // take all polarizations found in the dataset.
    void selectPols(ConstMirPolSetup &polsel);

    // set the options for filling.  The options are set via a Record object
    // for which the following keys are recognized:
    // <pre>
    // scanlim  the scan time jump limit, in seconds.  If the jump in
    //          time between two consecutive Miriad records is greater 
    //          than this limit the scan number that gets written out
    //          for that record will be incremented.  A change in source
    //          will always increment the scan number unless scanlim is
    //          negative; in this case, all records are forced to have
    //          the same scan number.  The default is 5 minutes.
    // 
    // obslim   the observation ID time jump limit, in seconds.  The 
    //          observation ID is meant to delimit two tracks that might
    //          appear in the same file.  If the jump in time 
    //          between two consecutive Miriad records is greater than 
    //          this limit the scan number that gets written out for 
    //          that record will be incremented.  The ID will always be
    //          incremented if there is a change in telescope or array
    //          configuration unless obslim is negative, in which case,
    //          all records will be forced to have the same observation
    //          ID.  The default is 4 hours.
    // 
    // tilesize the tiling size to use (in channels?) for storing data
    //          in the MS using the TiledStorageManager.  If the value 
    //          is <= 0, the standard (non-tiled) storage manager will
    //          be used.  The default is 32.
    // 
    // wideconv the convention to use for interpreting the wideband channels.
    //          The allowed values are "bima" and "none".  In the "bima" 
    //          convention, the first two wideband channels are the lower & 
    //          upper sideband averages and the remaining channels represent 
    //          averages of each of the spectral line windows.  Currently,
    //          this option only affects the writing out of window 
    //          associations: if wideconv="bima", the associations linking 
    //          wideband windows to spectral line windows will be written
    //          according to the convention.  
    // 
    // joinpol  If true, multiple polarizations are loaded as a single 
    //          polarization setup.  Which polarizations are included
    //          depend on the polarization selections.  If false (the 
    //          default), each polarization is loaded as a separate setup.
    // 
    // tsyswt   If true (the default), the SIGMA and WEIGHT columns will
    //          be filled with values based on the system temperatures, if 
    //          found.  If false, these columns will be filled with values=1.
    //          The SYSCAL table will be filled if system temperatures are
    //          present, regardless of the value of this option.
    //
    // planetfit  The maximum fit order to use when fitting the direction of a 
    //          planet-tracking field.  The actual order used may be less than
    //          this if there is insufficient data.  The default is 3.
	// updmodelint  This is the amount of time to wait before updating the
	//              model.  By default this is 8 hours.
    //
    // movfield if true, exact field positions tracking a moving object will
    //          be written to subtable called BIMA_MOVING_FIELDS.
    //
    // compress if true, selected columns will be compressed using scaled
    //          integers.  This limits the dynamic range to about 65000:1.
    //          The default is true.
    //
    // verbose  if true, send extra messages to the logger
    // </pre>
    void setOptions(const Record &opts);

    // get the current filling options
    Record getOptions();

    // handle an update to the integration time; 
    // this is a callback routine that is part of the MirVarHandler 
    // interface which must be implemented.  This implementation does
    // nothing.
    virtual void updateIntTime(FillMetadata &fm, Double time);

    // handle an update to the observatory; 
    // this is a callback routine that is part of the MirVarHandler 
    // interface which must be implemented.  This implementation flushes
    // records for the FEED subtable for the last observatory, and adds
    // a PROCESSOR subtable record for the next table.
    virtual void updateObservatory(FillMetadata &fm, Double time);

    // handle an update to the array configuration; 
    // this is a callback routine that is part of the MirVarHandler 
    // interface which must be implemented
    virtual void updateArray(FillMetadata &fm, Double time);

	// handle an update to the SOURCE_MODEL column of the SOURCE subtable.
	// Maybe that this gets incorporated into updateSource at sometime, have
	// to examine logistics.  REWRITEME. dgoscha
    virtual void updateSourceModel(FillMetadata &fm, Double time);

    // handle an update to the array configuration; 
    // this is a callback routine that is part of the MirVarHandler 
    // interface which must be implemented
    virtual void updatePolSetup(FillMetadata &fm, Double time);

    // handle an update to the source; 
    // this is a callback routine that is part of the MirVarHandler 
    // interface which must be implemented
    virtual void updateSource(FillMetadata &fm, Double time);

    // handle an update to the observing field; 
    // this is a callback routine that is part of the MirVarHandler 
    // interface which must be implemented
    virtual void updateField(FillMetadata &fm, Double time);

    // handle an update to the frequency setup; 
    // this is a callback routine that is part of the MirVarHandler 
    // interface which must be implemented
    virtual void updateFreqSetup(FillMetadata &fm, Double time);

    // handle an update to system temperatures
    // this is a callback routine that is part of the MirVarHandler 
    // interface which must be implemented
    virtual void updateTsys(FillMetadata &fm, Double time);

private:

    // open and setup the output MS.  This includes all initialization that 
    // can be done without reading any data from the input Miriad dataset.
    MeasurementSet *setupMS(const String& msfile, Int tileSize=0,
			    Bool needSrcModel=True);

    // initialize the output MS.  This is called after the first input record 
    // is read in from the input Miriad dataset.  It will initialize certain
    // subtables (currently, POINTING, PROCESSOR, HISTORY, FEED, and 
    // POLARIZATION) based on static or otherwise non-tracked variable 
    // information.
    void initMS(::FillMetadata& fm);

    void noJoinFill(const String& msfile);
    void joinFill(const String& msfile);

    Bool nearAbs(Double a, Double b, Double tol) {
        return (abs(a-b) < tol);
    }

    // add the current observation information as a new record to the
    // output ms's OBSERVATION subtable
    void flushObsRecord(FillMetadata& fm, Double time);

    // add the records to the FEED subtable for the current set of antennas
    // and polarizations
    void flushFeedRecords(FillMetadata& fm, Double time);

    // check for moving objects in source list and adjust FIELD subtable
    // accordingly
    void flushMovingFields(FillMetadata& fm, Double time);

    // add the records to the POINTING subtable for the current set of antennas
    void flushPointingRecords(FillMetadata& fm, Double time);

    // add a new set of antenna positions
    void addAntennaPositions(FillMetadata &fm, Double time);

    // add the current source to the SOURCE subtable
    uInt addSource(FillMetadata &fm, double time);

    // add the current field to the FIELD subtable
    void addField(FillMetadata &fm, double time);

    // add subtable records for the current correlator setup
    void addCorrelatorSetup(FillMetadata &fm, double time);

    // add a POLARIZATION record for the current polarization type.
    void addPolarization(FillMetadata &fm, Bool addCurrentOnly=False);

    // add DATA_DESCRIPTION records for the current polarization
    // and frequency setup
    void addDataDesc(FillMetadata &fm);

    // write out the miriad history into the HISTORY table
    void flushHistory(FillMetadata &fm);

    // copy the history from a given HISTORY table
    void copyHistoryFrom(FillMetadata &fm, String tablename);

    // add a processor record for the current frequency setup
    void addProcessor(FillMetadata &fm);

    // add a filler message to the HISTORY table
    void addHistoryMessage(FillMetadata &fm, String priority, 
			   String origin, String msg);

    // add a record to the SysCal table
    void addTsysRecords(FillMetadata &fm, Double time, Cube<Float>& tsys);

    // initialize the STATE table
    void initState(FillMetadata &fm);

    // create a polynomial fit for the direction to a moving source 
    //PJT
    Bool fitskymotion(Matrix<Double> &dirfit, const Vector<Double> &time, 
		      const Vector<Double> &ra, const Vector<Double> &dec);

    // info that doesn't change during life of filler
    MirVisReader rdr_p;
    LogIO log_p;

    // selection info
    Vector<Bool> widesel_p, winsel_p;
    Int nwinsel_p, nwidesel_p;
    ConstMirPolSetup *defpolsel_p;

    // options
    String histbl_p;
    Double scanlim_p, obslim_p;
    Int tilesize_p, planetfit_p;
    Bool verbose_p, joinpol_p, tsyswt_p, movfield_p, compress_p;

    enum wideConventions { NONE=0, BIMA, MIRIAD, NCONV };
    static String wideconvnames[];
    wideConventions wideconv_p;

    Double updmodelint_p;
};


#endif


