//# MirVisReader.h: reads data from a Miriad Visibility dataset 
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
//# $Id: MirVisReader.h,v 1.2 2009/09/03 23:28:32 pteuben Exp $

#ifndef BIMA_MIRVISREADER_H
#define BIMA_MIRVISREADER_H

#include <casa/Logging/LogIO.h>
#include <casa/Containers/Block.h>
#include <casa/Containers/List.h>

#include <miriad/Filling/FillMetadata.h>
#include <mirlib/miriad.h>

#include <casa/namespace.h>
//# Forward Declarations
namespace casacore{

class String;
class AipsError;
}

namespace casa { //# NAMESPACE CASA - BEGIN
//class GlishRecord;
} //# NAMESPACE CASA - END

//class GlishArray;
class MirVarHandler;

// <summary>
// a container for a data record read in from a Miriad datatset
// </summary>
// 
// <use visibility=local>
// 
// <reviewed reviewer="" date="yyyy/mm/dd" tests="" demos="">
// </reviewed>
//
// <etymology>
// this class holds the record-dependent data from a Miriad dataset
// </etymology>
//
// <motivation>
// MSes filled from Miriad data feature multiple windows of differing shapes.  
// Performance during casacore::MS reads can be aided by collecting together casacore::MS table 
// records of the same shape (e.g. of the same window).  MirFiller accoplishes
// this by reading all baselines from a single timestamp at once, so that 
// they written out in order of spectral window.  This class provides a 
// container for holding the data from different baselines.  
//
// Access Performance is aided when the data from a Miriad dataset is filled
// in a particular order.  This class provides a container so that the data
// can be buffered for "sorting" prior to filling into the output MS.  
// </motivation>
//
// <synopsis>
// At the moment, this is intended for use only by the MirFiller class.
//
// 
// </synopsis>
//
// <example>
// 
// 
// </example>
//
// <todo asof="2001/02/22">
// 
// </todo>
//
class MirDataRecord : MirTypeAssert {
    casacore::Int np_p, nn_p, nw_p, pol_p;
    casacore::Bool marked_p;
    casacore::Double *preamble_p;
    casacore::Float *narrow_p;
    casacore::Float *wide_p;
    casacore::Int *flags_p;
    casacore::Int *wflags_p;

public:
    MirDataRecord(casacore::Int nnarrow=0, casacore::Int nwide=0, casacore::Int npreamble=5);
    ~MirDataRecord();

    // return the number of preamble elements
    casacore::uInt getPreambleCount() { return np_p; }

    // return the number of narrow band channels
    casacore::uInt getNarrowCount() { return nn_p; }

    // return the number of wide band channels
    casacore::uInt getWideCount() { return nw_p; }

    // return the address of the preamble buffer
    casacore::Double *preamble() { return preamble_p; }

    // return the address of the narrow channel data buffer
    casacore::Float *narrow() { return narrow_p; }

    // return the address of the wide channel data buffer
    casacore::Float *wide() { return wide_p; }

    // return the address of the flags channel data buffer
    casacore::Int *flags() { return flags_p; }

    // return the address of the wflags channel data buffer
    casacore::Int *wflags() { return wflags_p; }

    // return the polarization code
    casacore::Int pol() { return pol_p; }

    // set the polarization code
    void setPol(casacore::Int val) { pol_p = val; }

    // return true if this record is marked
    casacore::Bool isMarked() { return marked_p; }

    // set the mark
    void setMarked(casacore::Bool val) { marked_p = val; }

    void copyFrom(const MirDataRecord& other);
};

// <summary>
// a Miriad history reader
// </summary>
// 
// <use visibility=export>
// 
// <reviewed reviewer="" date="yyyy/mm/dd" tests="" demos="">
// </reviewed>
//
// <etymology>
// Class name is short for Miriad history reader.
// </etymology>
//
// <motivation>
// Miriad history is read independently from the visibility data; thus,
// an independent class is in order.  This class hides the details of how
// to read Miriad history.
// </motivation>
//
// <synopsis>
// This class is used to extract history from a Miriad visibility dataset.  
// Normally, an application does not create this class by itself, but rather
// via MirVisReader::openHistory();
// </synopsis>
//
// <example>
// 
// 
// </example>
//
// <todo asof="2001/05/15">
// </todo>
//
class MirHisReader {
private:
    casacore::Int uv_handle_p, eof_p;
    casacore::Char hline[256];

public: 

    // create a History reader from a Miriad file handle
    MirHisReader(casacore::Int mirhandle) : uv_handle_p(mirhandle), eof_p(0) {
        hisopen_c(uv_handle_p, "read");
    }

    // delete the reader
    ~MirHisReader() {
        hisclose_c(uv_handle_p);
    }

    // return true this reader there is no more history to read
    casacore::Bool atEnd() { return (eof_p > 0); }

    // read the next available line into a String.  true is returned 
    // if line was successfully loaded; false is returned 
    // if the last line had already been read.  
    casacore::Bool readLine(casacore::String &line) {
        if (atEnd()) return false;
        hisread_c(uv_handle_p, hline, 256, &eof_p);
        line = hline;
        return true;
    }
};

// <summary>
// a Miriad visibility dataset reader
// </summary>
// 
// <use visibility=export>
// 
// <reviewed reviewer="" date="yyyy/mm/dd" tests="" demos="">
// </reviewed>
//
// <etymology>
// Class name is short for Miriad visibility reader.
// </etymology>
//
// <motivation>
// MSes filled from Miriad data feature multiple windows of differing shapes.  
// Performance during casacore::MS reads can be aided by collecting together casacore::MS table 
// records of the same shape (e.g. of the same window).  MirFiller accomplishes
// this by reading all baselines from a single timestamp at once, so that 
// they written out in order of spectral window.  This class provides a 
// container for holding the data from different baselines.  It also 
// encapsulates all the knowledge needed for reading Miriad data.
// </motivation>
//
// <synopsis>
// This class is used to extract data from a Miriad visibility dataset.  
//
// 
// </synopsis>
//
// <example>
// 
// 
// </example>
//
// <todo asof="2001/05/15">
//   <li> deal with telescopes, correlator modes </li>
//   <li> support multiple polarizations </li>
//   <li> make handleVarChanges() a protected virtual function </li>
// </todo>
//
class MirVisReader : MirTypeAssert {
public: 
    enum wideConventions { NONE=0, BIMA, MIRIAD, MIXED, NCONV };

private:
    // info that doesn't change during life of filler
    casacore::String inname_p;
    casacore::Int uv_handle_p;                   // miriad file handle
    casacore::uInt debug;
    casacore::LogIO log_p;

    // variable information worth caching
    casacore::List<MirFreqSetup*> fsetups_p;     // the correlator setups found
    casacore::List<MirSource*> sources_p;        // the sources found
    casacore::List<casacore::String> telescopes_p;         // the telescopes found
    MirPolSetup pol_p;
    casacore::Int maxchan_p, maxwide_p, maxspect_p, narray_p, nrec_p;
    casacore::Int nobs_p, nscan_p;               // These are indicative, not invarient
    casacore::Int badnsyst_p, badwsyst_p;        // # of recs with bad system temps
    casacore::Int firstmode_p;                   // first correlator mode encountered
    casacore::Double starttime_p, endtime_p;
    wideConventions wideconv_p;        // apparent wide-channel convention

    // read state
    casacore::Block<MirDataRecord *> buf_p;
    casacore::Int nintrec_p, maxrec_p, nnarr_p, nwide_p;
    casacore::Double time_p;                     // in AIPS++ frame
    MirVarHandler *varhandler_p;
    casacore::Bool varupd_p, hasmore_p, dowide_p, verbose_p, previewed_p, scanned_p;

public:
    // construct a reader.  mirfile is the name of Miriad dataset.  If 
    // dopreview is false, the previewing of the dataset's contents will 
    // be delayed.  If doscan is true (and dopreview is true), the entire
    // input dataset will be read to take an accounting of its contents; 
    // set this to false to prevent this for a very large file (see also 
    // preview()).  dbg is the default debug level to set; this value is 
    // passed to setDebugLevel().  
    MirVisReader(const casacore::String& mirfile, casacore::Bool doscan=true, casacore::Int dbg=0, 
                 casacore::Bool dopreview=true);

    // destroy the reader
    ~MirVisReader();

    // set the debugging level which controls the amount of debugging 
    // messages that are printed to the terminal (as opposed to the logger).  
    // A value of zero or less will cause no extra messages to be printed; 
    // increasing values will increase the amount of messages.
    void setDebugLevel(casacore::Int level) { debug = (level < 0) ? 0 : (casacore::uInt) level; }

    // set the debugging level which controls the amount of debugging 
    // messages that are printed to the terminal.  A value of zero or less 
    // means that no extra messages will be printed; increasing values will 
    // increase the amount of messages.
    casacore::Int getDebugLevel() const { return debug; }

    // return true if the debugging level is at least as high as a given 
    // level.
    casacore::Bool Debug(casacore::Int level) { return (level <= casacore::Int(debug)); }

    // return the Miriad dataset name
    const casacore::String& getName() { return inname_p; }

    // rewind the input dataset and prepare for a new reading.  filler is 
    // the object that will handle updates to variables; if NULL, no handler
    // will be alerted when variables change.  If dowide is true, the 
    // wideband data will be read in, too.  maxrec is the 
    // maximum number of records to read per timestamp; if maxrec<=0, 
    // it will adjust dynamically to the number of baselines being processed.
    void reset(MirVarHandler *filler=NULL, casacore::Bool verbose=false, 
	       casacore::Bool dowide=true, casacore::Int maxrec=0);

    // read in the data for a single timestamp.  fm is the container
    // to use to store retrieved metadata.
    casacore::Int readIntegration(::FillMetadata &fm);

    // return the (AIPS++-referenced) timestamp for the current buffered
    // records.  This is only correct after a call to readIntegration().
    casacore::Double getTime() { return time_p; }

    // return the number of records buffered for the current integration
    casacore::Int getNumRecs() { return nintrec_p; }

    // return the correlation data for the i-th buffered record.
    MirDataRecord *getRecord(casacore::Int i) { return buf_p[i]; }

    // set the marked data item for each MirDataRecord to false
    void clearMarks() {
	for(casacore::Int i=0; i < nintrec_p; i++) buf_p[i]->setMarked(false);
    }

    // create and return a new pointer to a history reader.  The caller
    // should delete the reader when finished with it.  
    MirHisReader *openHistory() { return new MirHisReader(uv_handle_p); }

    // return the basic characteristics of the input dataset as a Record.  
    // If scan is true, the entire file will be scanned (if necessary) to 
    // all the information; otherwise, an incomplete description may be given
    // based on just the first record.  If verbose is true, a summary is 
    // sent to the logger.  Note that this function may force a call to 
    // preview() if it has not already been called.
    //PJT    GlishRecord summary(casacore::Bool verbose=true, casacore::Bool scan=true);
	
    // return the basic characteristics of the input dataset as a Record.  
    // A scanning of the entire file will be forced so that the number of 
    // scans and observations can be counted with the given limits:
    // scanlim is the maximum time gap in seconds allowed between records 
    // of the same scan; obslim is maximum gap in seconds between 
    // records of the same observation.  If verbose is true, a summary is 
    // sent to the logger.  
    //PJT GlishRecord summary(casacore::Int scanlim, casacore::Int obslim, casacore::Bool verbose=true);

    // get the full list of polarization correlation types seen thus far
    ConstMirPolSetup& getDefaultPolSetup() { return pol_p; } 

    // return the list of sources
    const casacore::List<MirSource*>& getSourceList() { return sources_p; }
	
protected:
    // peek into the contents of the dataset to cache important information.
    // scanlim and obslim are used for counting the number of observations
    // and scans in the dataset.  scanlim is the maximum time gap in seconds 
    // allowed between records of the same scan; obslim is maximum gap 
    // in seconds between records of the same observation.  If scan is true
    // (the default), the dataset will be read through to gather the 
    // information; otherwise, limited information will be gleaned from the
    // first record and scanlim and obslim will be ignored.  One can set 
    // scan=false for very large datasets to avoid this overhead.
    void preview(casacore::Int scanlim, casacore::Int obslim, casacore::Bool scan=true);

    // peek into the contents of the dataset to cache important 
    // information.  If scan is true
    // (the default), the dataset will be read through to gather the 
    // information; otherwise, limited information will be gleaned from the
    // first record.  One can set scan=false for very large datasets to avoid 
    // this overhead.  Five minutes and four hours are used for the scan and
    // observation gap limit (when scan=true).
    void preview(casacore::Bool scan=true) { preview(300, 2400, scan); }

    // return true if this dataset has been previewed
    casacore::Bool previewed() { return previewed_p; }

    // return the basic characteristics of the input dataset as a Record.  
    // If verbose is true, a summary is 
    // sent to the logger.  
    // scan specifies the conditions under which the entire dataset may 
    // get read as a result of this request:  if scan < 0, the dataset
    // will not be read; if scan=0, it will only be read if it hasn't 
    // already (using scanlim and obslim if necessary; if scan > 0, the 
    // dataset will be read and scanlim and obslim will be used to count 
    // scans and observations.  
    // scanlim is the maximum time gap in seconds 
    // allowed between records of the same scan; obslim is maximum gap 
    // in seconds between records of the same observation.  
    //GlishRecord summary(casacore::Bool verbose, casacore::Int scan, casacore::Int scanlim, casacore::Int obslim);

    // return the contents of a MirPolSetup list as a GlishArray
    //static GlishArray toGlishArray(ConstMirPolSetup &pol);

    // return the contents of a casacore::String casacore::List as a GlishArray
    //static GlishArray toGlishArray(const casacore::List<casacore::String>& list);

private:

    // return true if this reader should be verbose in its messages.  This
    // will be true if the verbose option is enabled or the debug level is
    // greater than 1.
    casacore::Bool verbose() { return (verbose_p || debug > 1); }

    // check to be sure that the given name points to a readable miriad 
    // dataset.  If not, throw an exception.
    void checkIsMiriadDataset(const casacore::String& name) throw(casacore::AipsError);

    // resize the data buffer for a new read.  Values less than or equal 
    // to zero mean keep previous value.  
    void resizeBufferFor(casacore::Int nrec=0, casacore::Int nnarrow=0, casacore::Int nwide=0);

    // full and brief scanning of the data--should only be called from preview()
    void fullscan(casacore::Int scanlim, casacore::Int obslim);
    void briefscan();

    // update the given FillMetadata container with updated variable values.
    // If a VarHandler was provide via reset(), signal the changes to it.
    // fm is the FillMetadata container to update, and time is the current 
    // timestamp.  
    void handleVarChanges(::FillMetadata &fm, casacore::Double time);

    // add to a cached list of telescopes
    void setTelescope(casacore::String name) {
        casacore::ListIter<casacore::String> li(telescopes_p);
        for(li.toStart(); ! li.atEnd() && li.getRight() != name; ++li);
        if (li.atEnd()) li.addRight(name);
    }
};

#endif
