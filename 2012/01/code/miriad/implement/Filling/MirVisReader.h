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
namespace casa { //# NAMESPACE CASA - BEGIN
class String;
class AipsError;
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
// Performance during MS reads can be aided by collecting together MS table 
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
    Int np_p, nn_p, nw_p, pol_p;
    Bool marked_p;
    Double *preamble_p;
    Float *narrow_p;
    Float *wide_p;
    Int *flags_p;
    Int *wflags_p;

public:
    MirDataRecord(Int nnarrow=0, Int nwide=0, Int npreamble=5);
    ~MirDataRecord();

    // return the number of preamble elements
    uInt getPreambleCount() { return np_p; }

    // return the number of narrow band channels
    uInt getNarrowCount() { return nn_p; }

    // return the number of wide band channels
    uInt getWideCount() { return nw_p; }

    // return the address of the preamble buffer
    Double *preamble() { return preamble_p; }

    // return the address of the narrow channel data buffer
    Float *narrow() { return narrow_p; }

    // return the address of the wide channel data buffer
    Float *wide() { return wide_p; }

    // return the address of the flags channel data buffer
    Int *flags() { return flags_p; }

    // return the address of the wflags channel data buffer
    Int *wflags() { return wflags_p; }

    // return the polarization code
    Int pol() { return pol_p; }

    // set the polarization code
    void setPol(Int val) { pol_p = val; }

    // return true if this record is marked
    Bool isMarked() { return marked_p; }

    // set the mark
    void setMarked(Bool val) { marked_p = val; }

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
    Int uv_handle_p, eof_p;
    Char hline[256];

public: 

    // create a History reader from a Miriad file handle
    MirHisReader(Int mirhandle) : uv_handle_p(mirhandle), eof_p(0) {
        hisopen_c(uv_handle_p, "read");
    }

    // delete the reader
    ~MirHisReader() {
        hisclose_c(uv_handle_p);
    }

    // return true this reader there is no more history to read
    Bool atEnd() { return (eof_p > 0); }

    // read the next available line into a String.  True is returned 
    // if line was successfully loaded; False is returned 
    // if the last line had already been read.  
    Bool readLine(String &line) {
        if (atEnd()) return False;
        hisread_c(uv_handle_p, hline, 256, &eof_p);
        line = hline;
        return True;
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
// Performance during MS reads can be aided by collecting together MS table 
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
    String inname_p;
    Int uv_handle_p;                   // miriad file handle
    uInt debug;
    LogIO log_p;

    // variable information worth caching
    List<MirFreqSetup*> fsetups_p;     // the correlator setups found
    List<MirSource*> sources_p;        // the sources found
    List<String> telescopes_p;         // the telescopes found
    MirPolSetup pol_p;
    Int maxchan_p, maxwide_p, maxspect_p, narray_p, nrec_p;
    Int nobs_p, nscan_p;               // These are indicative, not invarient
    Int badnsyst_p, badwsyst_p;        // # of recs with bad system temps
    Int firstmode_p;                   // first correlator mode encountered
    Double starttime_p, endtime_p;
    wideConventions wideconv_p;        // apparent wide-channel convention

    // read state
    Block<MirDataRecord *> buf_p;
    Int nintrec_p, maxrec_p, nnarr_p, nwide_p;
    Double time_p;                     // in AIPS++ frame
    MirVarHandler *varhandler_p;
    Bool varupd_p, hasmore_p, dowide_p, verbose_p, previewed_p, scanned_p;

public:
    // construct a reader.  mirfile is the name of Miriad dataset.  If 
    // dopreview is False, the previewing of the dataset's contents will 
    // be delayed.  If doscan is True (and dopreview is True), the entire
    // input dataset will be read to take an accounting of its contents; 
    // set this to False to prevent this for a very large file (see also 
    // preview()).  dbg is the default debug level to set; this value is 
    // passed to setDebugLevel().  
    MirVisReader(const String& mirfile, Bool doscan=True, Int dbg=0, 
                 Bool dopreview=True);

    // destroy the reader
    ~MirVisReader();

    // set the debugging level which controls the amount of debugging 
    // messages that are printed to the terminal (as opposed to the logger).  
    // A value of zero or less will cause no extra messages to be printed; 
    // increasing values will increase the amount of messages.
    void setDebugLevel(Int level) { debug = (level < 0) ? 0 : (uInt) level; }

    // set the debugging level which controls the amount of debugging 
    // messages that are printed to the terminal.  A value of zero or less 
    // means that no extra messages will be printed; increasing values will 
    // increase the amount of messages.
    Int getDebugLevel() const { return debug; }

    // return true if the debugging level is at least as high as a given 
    // level.
    Bool Debug(Int level) { return (level <= Int(debug)); }

    // return the Miriad dataset name
    const String& getName() { return inname_p; }

    // rewind the input dataset and prepare for a new reading.  filler is 
    // the object that will handle updates to variables; if NULL, no handler
    // will be alerted when variables change.  If dowide is True, the 
    // wideband data will be read in, too.  maxrec is the 
    // maximum number of records to read per timestamp; if maxrec<=0, 
    // it will adjust dynamically to the number of baselines being processed.
    void reset(MirVarHandler *filler=NULL, Bool verbose=False, 
	       Bool dowide=True, Int maxrec=0);

    // read in the data for a single timestamp.  fm is the container
    // to use to store retrieved metadata.
    Int readIntegration(::FillMetadata &fm);

    // return the (AIPS++-referenced) timestamp for the current buffered
    // records.  This is only correct after a call to readIntegration().
    Double getTime() { return time_p; }

    // return the number of records buffered for the current integration
    Int getNumRecs() { return nintrec_p; }

    // return the correlation data for the i-th buffered record.
    MirDataRecord *getRecord(Int i) { return buf_p[i]; }

    // set the marked data item for each MirDataRecord to False
    void clearMarks() {
	for(Int i=0; i < nintrec_p; i++) buf_p[i]->setMarked(False);
    }

    // create and return a new pointer to a history reader.  The caller
    // should delete the reader when finished with it.  
    MirHisReader *openHistory() { return new MirHisReader(uv_handle_p); }

    // return the basic characteristics of the input dataset as a Record.  
    // If scan is True, the entire file will be scanned (if necessary) to 
    // all the information; otherwise, an incomplete description may be given
    // based on just the first record.  If verbose is True, a summary is 
    // sent to the logger.  Note that this function may force a call to 
    // preview() if it has not already been called.
    //PJT    GlishRecord summary(Bool verbose=True, Bool scan=True);
	
    // return the basic characteristics of the input dataset as a Record.  
    // A scanning of the entire file will be forced so that the number of 
    // scans and observations can be counted with the given limits:
    // scanlim is the maximum time gap in seconds allowed between records 
    // of the same scan; obslim is maximum gap in seconds between 
    // records of the same observation.  If verbose is True, a summary is 
    // sent to the logger.  
    //PJT GlishRecord summary(Int scanlim, Int obslim, Bool verbose=True);

    // get the full list of polarization correlation types seen thus far
    ConstMirPolSetup& getDefaultPolSetup() { return pol_p; } 

    // return the list of sources
    const List<MirSource*>& getSourceList() { return sources_p; }
	
protected:
    // peek into the contents of the dataset to cache important information.
    // scanlim and obslim are used for counting the number of observations
    // and scans in the dataset.  scanlim is the maximum time gap in seconds 
    // allowed between records of the same scan; obslim is maximum gap 
    // in seconds between records of the same observation.  If scan is True
    // (the default), the dataset will be read through to gather the 
    // information; otherwise, limited information will be gleaned from the
    // first record and scanlim and obslim will be ignored.  One can set 
    // scan=False for very large datasets to avoid this overhead.
    void preview(Int scanlim, Int obslim, Bool scan=True);

    // peek into the contents of the dataset to cache important 
    // information.  If scan is True
    // (the default), the dataset will be read through to gather the 
    // information; otherwise, limited information will be gleaned from the
    // first record.  One can set scan=False for very large datasets to avoid 
    // this overhead.  Five minutes and four hours are used for the scan and
    // observation gap limit (when scan=True).
    void preview(Bool scan=True) { preview(300, 2400, scan); }

    // return True if this dataset has been previewed
    Bool previewed() { return previewed_p; }

    // return the basic characteristics of the input dataset as a Record.  
    // If verbose is True, a summary is 
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
    //GlishRecord summary(Bool verbose, Int scan, Int scanlim, Int obslim);

    // return the contents of a MirPolSetup list as a GlishArray
    //static GlishArray toGlishArray(ConstMirPolSetup &pol);

    // return the contents of a String List as a GlishArray
    //static GlishArray toGlishArray(const List<String>& list);

private:

    // return true if this reader should be verbose in its messages.  This
    // will be true if the verbose option is enabled or the debug level is
    // greater than 1.
    Bool verbose() { return (verbose_p || debug > 1); }

    // check to be sure that the given name points to a readable miriad 
    // dataset.  If not, throw an exception.
    void checkIsMiriadDataset(const String& name) throw(AipsError);

    // resize the data buffer for a new read.  Values less than or equal 
    // to zero mean keep previous value.  
    void resizeBufferFor(Int nrec=0, Int nnarrow=0, Int nwide=0);

    // full and brief scanning of the data--should only be called from preview()
    void fullscan(Int scanlim, Int obslim);
    void briefscan();

    // update the given FillMetadata container with updated variable values.
    // If a VarHandler was provide via reset(), signal the changes to it.
    // fm is the FillMetadata container to update, and time is the current 
    // timestamp.  
    void handleVarChanges(::FillMetadata &fm, Double time);

    // add to a cached list of telescopes
    void setTelescope(String name) {
        ListIter<String> li(telescopes_p);
        for(li.toStart(); ! li.atEnd() && li.getRight() != name; ++li);
        if (li.atEnd()) li.addRight(name);
    }
};

#endif
