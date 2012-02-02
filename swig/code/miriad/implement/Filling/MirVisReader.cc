//# MirVisReader.cc:  reads data from a Miriad Visibility dataset 
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
//# $Id: MirVisReader.cc,v 1.2 2009/09/03 23:28:32 pteuben Exp $

//# Includes
#include <miriad/Filling/MirVisReader.h>
#include <miriad/Filling/MirVarHandler.h>
#include <miriad/Filling/FillMetadata.h>
#include <miriad/Filling/MirExceptions.h>

#include <casa/OS/File.h>
#include <casa/OS/Time.h>
#include <casa/BasicSL/Constants.h>
#include <casa/Quanta/MVTime.h>
#include <casa/Quanta/MVAngle.h>
//#include <tasking/Glish/GlishRecord.h>
//#include <tasking/Glish/GlishArray.h>
#include <casa/Arrays/ArrayLogical.h>
#include <casa/Utilities/PtrHolder.h>
#include <casa/iostream.h>

// Miriad interface
#include <mirlib/maxdimc.h>
#include <mirlib/miriad.h>

// #define IOCAST(x) ((char *) x)
#define IOCAST(x) reinterpret_cast<char *>(x)

MirDataRecord::MirDataRecord(Int nnarrow, Int nwide, Int npreamble) 
    : np_p(npreamble), nn_p(nnarrow), nw_p(nwide), pol_p(0), marked_p(False), 
      preamble_p(NULL), narrow_p(NULL), wide_p(NULL), flags_p(NULL), 
      wflags_p(NULL)
{
    if (np_p > 0) preamble_p = new Double[np_p];
    if (nn_p > 0) {
        narrow_p = new Float[2*nn_p];
        flags_p = new Int[nn_p];
    }
    if (nw_p > 0) {
        wide_p = new Float[2*nw_p];
        wflags_p = new Int[nw_p];
    }
}

MirDataRecord::~MirDataRecord() {
    if (preamble_p != NULL) delete preamble_p;
    if (narrow_p != NULL) delete narrow_p;
    if (flags_p != NULL) delete flags_p;
    if (wide_p != NULL) delete wide_p;
    if (wflags_p != NULL) delete wflags_p;
}

void MirDataRecord::copyFrom(const MirDataRecord& other) {
    objcopy(preamble_p, other.preamble_p, 
            (np_p < other.np_p) ? np_p : other.np_p);
    objcopy(narrow_p, other.narrow_p, 
            2*((nn_p < other.nn_p) ? nn_p : other.nn_p));
    objcopy(flags_p, other.flags_p, 
            (nn_p < other.nn_p) ? nn_p : other.nn_p);
    objcopy(wide_p, other.wide_p, 
            2*((nw_p < other.nw_p) ? nw_p : other.nw_p));
    objcopy(wflags_p, other.wflags_p, 
            (nw_p < other.nw_p) ? nw_p : other.nw_p);
    marked_p = other.marked_p;
    pol_p = other.pol_p;
}

MirVisReader::MirVisReader(const String& mirfile, Bool doscan, Int dbg, 
                           Bool dopreview) 
    : inname_p(mirfile), uv_handle_p(0), debug(0), 
      log_p(LogOrigin("MirVisReader")), 
      fsetups_p(), sources_p(), telescopes_p(), pol_p(),
      maxchan_p(-1), maxwide_p(-1), maxspect_p(-1),
      narray_p(-1), nrec_p(-1), nobs_p(-1), nscan_p(-1),
      badnsyst_p(-1), badwsyst_p(-1),
      firstmode_p(0), starttime_p(0), endtime_p(0), wideconv_p(NCONV),
      buf_p(), nintrec_p(0), maxrec_p(0), nnarr_p(0), nwide_p(0), time_p(0),
      varhandler_p(NULL), varupd_p(False), hasmore_p(False), dowide_p(True), 
      verbose_p(False), previewed_p(False), scanned_p(False)
{
    setDebugLevel(dbg);

    if (Debug(1)) cout << "MirVisReader::MirVisReader debug_level=" 
                       << getDebugLevel() << endl;
    if (Debug(1)) cout << "*** Opening " << inname_p << endl;

    if (sizeof(double) != sizeof(Double))
        log_p << LogIO::WARN << "Double != double; " 
              << "Filling will probably fail" << LogIO::POST;
    if (sizeof(int) != sizeof(Int))
        log_p << LogIO::WARN << "Int != int; "
              << "Filling will probably fail" << LogIO::POST;

    // check the existence of the input Miriad dataset
    if (inname_p.length() == 0) 
        throw AipsError("No input Miriad dataset given");
    checkIsMiriadDataset(inname_p);

    // attempt an open of the miriad dataset.  This will die via 
    // miriad error handling if an error occurs.
    uvopen_c(&uv_handle_p, inname_p.c_str(), "old");

    // preamble data must be UVW (default miriad is UV)
    uvset_c(uv_handle_p,"preamble","uvw/time/baseline",0,0.0,0.0,0.0);

    // set up miriad variable tracking
    uvtrack_c(uv_handle_p,"nspect","u");
    uvtrack_c(uv_handle_p,"nwide","u");
    uvtrack_c(uv_handle_p,"cormode","u");
    uvtrack_c(uv_handle_p,"corfin","u");
    uvtrack_c(uv_handle_p,"corbw","u");
    uvtrack_c(uv_handle_p,"restfreq","u");

    uvtrack_c(uv_handle_p,"nwide","u");
    uvtrack_c(uv_handle_p,"wfreq","u");
    uvtrack_c(uv_handle_p,"wwidth","u");

    uvtrack_c(uv_handle_p,"nants","u");
    uvtrack_c(uv_handle_p,"antpos","u");
    uvtrack_c(uv_handle_p,"mount","u");

//    uvtrack_c(uv_handle_p,"npol","u");
//    uvtrack_c(uv_handle_p,"pol","u");   

    uvtrack_c(uv_handle_p,"ra","u");        
    uvtrack_c(uv_handle_p,"dec","u");       
    uvtrack_c(uv_handle_p,"dra","u");        
    uvtrack_c(uv_handle_p,"ddec","u");       
    uvtrack_c(uv_handle_p,"source","u");       
    uvtrack_c(uv_handle_p,"epoch","u");       

	uvtrack_c(uv_handle_p,"plangle","u");
	uvtrack_c(uv_handle_p,"plmaj","u");
	uvtrack_c(uv_handle_p,"plmin","u");
	uvtrack_c(uv_handle_p,"pltb","u");
	uvtrack_c(uv_handle_p,"freq","u");

    uvtrack_c(uv_handle_p,"inttime","u");
    uvtrack_c(uv_handle_p,"jyperk","u");
    uvtrack_c(uv_handle_p,"telescop","u");
    uvtrack_c(uv_handle_p,"project","u");

    uvtrack_c(uv_handle_p,"systemp","u");
    uvtrack_c(uv_handle_p,"wsystemp","u");

    if (dopreview) preview(doscan);
}

MirVisReader::~MirVisReader() {
    if (Debug(3)) cout << "MirVisReader::~MirVisReader" << endl;
    if (uv_handle_p != 0) {
        if (Debug(1)) cout << "*** Closing " << inname_p << " ***\n" ;
        uvclose_c(uv_handle_p);
        uv_handle_p = 0;
    }

    if (buf_p.nelements() > 0) {
        for(uInt i=0; i < buf_p.nelements(); i++) {
            delete buf_p[i];
        }
    }
}

void MirVisReader::checkIsMiriadDataset(const String& name) throw(AipsError) {
    // check to be sure that the given name points to a readable miriad 
    // dataset.  If not, throw an exception.
    if (Debug(3)) cout << "MirVisReader::checkIsMiriadDataset()" << endl;
    File t(name);
    if (! t.exists()) {
        throw AipsError(name + ": does not exist");
    } else if (! t.isReadable()) {
        throw AipsError(name + ": Permission denied");
    } 

    AipsError notMiriadDS(name + ": not a Miriad UV dataset");
    if (t.isDirectory()) {
        File hdr(name + "/header");
        File vis(name + "/visdata");
        if (! hdr.exists() || ! vis.exists()) throw notMiriadDS;
    }
    else {
        throw notMiriadDS;
    }
    if (Debug(3)) cout << "leaving checkIsMiriadDataset()" << endl;
}

Int MirVisReader::readIntegration(FillMetadata &fm) {
    if (! hasmore_p) {
        nintrec_p = 0;
        return nintrec_p;
    }

    // The first record of the block has been read in already; move 
    // to the first position.
    MirDataRecord *tmp = buf_p[0];
    buf_p[0] = buf_p[nintrec_p];
    buf_p[nintrec_p] = tmp;
    nintrec_p = 0;

    time_p = (buf_p[0]->preamble()[3] - 2400000.5) * C::day;

    // handle any variable changes
    if (varupd_p) {
        handleVarChanges(fm, time_p);
        varupd_p = False;
    }

//      // load the current system temperatures.  Note that handleVarChanges()
//      // should have already properly resized nsystemp and wsystemp
//      // Note also that this implementation assumes that there is one set of
//      // system temperatures good for all records returned in this call.
//      Char vtype[2];
//      Float tsys[MAXWIDE*fm.nants];
//      Int vlen, vupd;
//      uvprobvr_c(uv_handle_p, "systemp", vtype, &vlen, &vupd);
//      if (vlen > 0 && vlen == Int(fm.nsystemp.nelements())) {
//  	uvgetvr_c(uv_handle_p, H_REAL, "systemp", IOCAST(&tsys), 
//  		  fm.nsystemp.nelements());
//  	fm.nsystemp.takeStorage(fm.nsystemp.shape(), tsys);
//      }
//      else {
//  	vlen = 0;
//  	fm.nsystemp = 0.0;
//      }
//      if (dowide_p) {
//  	uvprobvr_c(uv_handle_p, "wsystemp", vtype, &vlen, &vupd);
//  	if (vlen > 0 && vlen == Int(fm.wsystemp.nelements())) {
//  	    uvgetvr_c(uv_handle_p, H_REAL, "wsystemp", IOCAST(&tsys), 
//  		      fm.wsystemp.nelements());
//  	    fm.wsystemp.takeStorage(fm.wsystemp.shape(), tsys);
//  	}
//  	else {
//  	    vlen = 0;
//  	    fm.wsystemp = 0.0;
//  	}
//      }
//      if (varhandler_p != NULL && vlen > 0) 
//  	varhandler_p->updateTsys(fm, time_p);

    // now read in data until:
    //   a. the block has ended
    //   b. variables change
    //   c. the maximum number of records have been read in.
    Float *corr, *wcorr;
    Int *flags, *wflags;
    Double *preamble;
    hasmore_p = False;
    Int maxrec = (maxrec_p > 0) ? maxrec_p : buf_p.nelements();
    if (maxrec < 0 || nintrec_p <= maxrec) {
        Int i=0, nread;
	while (1) {

	    // process previously read record
	    if (! pol_p.setCorr(buf_p[i]->pol()) && varhandler_p != NULL)
		varhandler_p->updatePolSetup(fm, buf_p[i]->preamble()[3]);
	    if (fm.polrecp != NULL) fm.polrecp->addPolsFor(pol_p.getCurrent());
            i++;
	    nintrec_p++;

	    // read next record and load it into the buffer
	    corr = buf_p[i]->narrow();
	    flags = buf_p[i]->flags();
	    preamble = buf_p[i]->preamble();
	    buf_p[i]->setMarked(False);

	    nread = 0;
	    uvread_c(uv_handle_p, preamble, corr, flags, nnarr_p, &nread);
	    if (nread <= 0) break;
	    if (dowide_p) {
		wcorr = buf_p[i]->wide();
		wflags = buf_p[i]->wflags();
		uvwread_c(uv_handle_p, wcorr, wflags, nwide_p, &nread);
	    }

	    varupd_p = uvupdate_c(uv_handle_p);
	    uvgetvr_c(uv_handle_p,H_INT,"pol", IOCAST(&nread),1);
            buf_p[i]->setPol(nread);

	    if (preamble[3] != buf_p[0]->preamble()[3] ||
		(maxrec >= 0 && nintrec_p >= maxrec)   ||
		varupd_p                                    ) 
	    {
		hasmore_p = True;
		break;
	    }
	}
    }

    return nintrec_p;
}

void MirVisReader::reset(MirVarHandler *filler, Bool verbose, Bool dowide, 
			 Int maxrec) 
{
    verbose_p = verbose;
    maxrec_p = maxrec;
    dowide_p = dowide;
    varhandler_p = filler;

    // update the size of the buffer if necessary
    Int maxchan = (scanned_p) ? maxchan_p : MAXCHAN;
    Int maxwide = (scanned_p) ? maxwide_p : MAXCHAN;
    if (maxrec <= 0) maxrec = 3;
    resizeBufferFor(maxrec, maxchan, maxwide);

    // clear the IDs
    MirFreqSetup::clearIDs(fsetups_p);
    MirSource::clearIDs(sources_p);
    pol_p.clearIDs();

    // cache the first record
    Int nread = 0;
    uvrewind_c(uv_handle_p);
    uvset_c(uv_handle_p, "data", "channel", 0, 1.0, 1.0, 1.0);
    uvread_c(uv_handle_p, buf_p[0]->preamble(), buf_p[0]->narrow(), 
	     buf_p[0]->flags(), maxchan, &nread);
    if (nread > 0) {
	if (Debug(3)) cout << "1st scan: nread=" << nread<<endl;
	if (dowide) uvwread_c(uv_handle_p, buf_p[0]->wide(), 
			      buf_p[0]->wflags(), maxwide, &nread); 
        uvgetvr_c(uv_handle_p,H_INT,"pol", IOCAST(&nread),1);
        buf_p[0]->setPol(nread);
        
	nrec_p = 0;
	hasmore_p = True;
	varupd_p = True;
    }
    buf_p[0]->setMarked(False);
}

void MirVisReader::resizeBufferFor(Int nrec, Int nnarrow, Int nwide) {
    if (nrec <= 0) 
        nrec = (buf_p.nelements() == 0) ? 1 : buf_p.nelements()-1;
    nrec++;
    if (nnarrow <= 0) nnarrow = nnarr_p;
    if (nwide <= 0) nwide = nwide_p;
    if (nrec > Int(buf_p.nelements()) ||
        nnarrow > nnarr_p || nwide > nwide_p)
    {

        Int n = (Int(buf_p.nelements()) < nrec) ? buf_p.nelements() : nrec;

        // First, properly preserve the buffered recorded from the next block
        MirDataRecord *next = NULL;
        if (nintrec_p >= 0) {
            if (nintrec_p > 0) {
                next = buf_p[0];
                buf_p[0] = buf_p[nintrec_p];
                buf_p[nintrec_p] = next;
                nintrec_p = 0;
                next = NULL;
            }
        }
        if (n > 0 && (nnarrow > nnarr_p || nwide > nwide_p)) 
            next = buf_p[0];

        buf_p.resize(++nrec);

        // If the number of channels change, recreate each record buffer
        Int i;
        if (n > 0 && (nnarrow > nnarr_p || nwide > nwide_p)) {
            if (next != NULL) delete buf_p[0];
            buf_p[0] = new MirDataRecord(nnarrow, nwide);
            if (next != NULL) {
                buf_p[0]->copyFrom(*next);
                delete next;
                next = NULL;
            }
            for(i=1; i < n; i++) {
                delete buf_p[i];
                buf_p[i] = new MirDataRecord(nnarrow, nwide);
            }
        }

        for(i=n; i < nrec; i++) {
            buf_p[i] = new MirDataRecord(nnarrow, nwide);
        }
    }

    nnarr_p = nnarrow;
    nwide_p = nwide;
}

void MirVisReader::preview(Int scanlim, Int obslim, Bool scan) 
{
    if (uv_handle_p == 0) 
	throw AipsError("Logic Error: input dataset not open");

    // check if certain calibration tables are present and warn if so
    if (hexists_c(uv_handle_p,"gains")) 
	log_p << LogIO::WARN << "gains table present, but cannot apply them" 
	      << LogIO::POST;
    if (hexists_c(uv_handle_p,"bandpass")) 
        log_p << LogIO::WARN << "bandpass table present, but cannot apply them" 
	      << LogIO::POST;
    if (hexists_c(uv_handle_p,"leakage")) 
        log_p << LogIO::WARN << "leakage table present, but cannot apply them" 
	      << LogIO::POST;

    // rewind the data
    reset(NULL, 0, False);

    if (! hasmore_p) {
	log_p << LogIO::WARN << "No visibility data found in " << inname_p
	      << LogIO::POST;
	return;
    }

    if (scan) {
	// read through the dataset to collect all information
	fullscan(scanlim, obslim);
    }
    else {
	// assemble any available information from the first record
	briefscan();
    }

    if (maxwide_p == 0 && maxspect_p == 0) 
        throw(MiriadFormatError(String("Miriad dataset appears to have ") +
                                       "neither wide nor narrow band data"));

    // guess at the wide channel convention being used
    MirFreqSetup *mfs;
    ListIter<MirFreqSetup *> li(fsetups_p);
    wideconv_p = NCONV;
    for(li.toStart(); ! li.atEnd(); ++li) {
	mfs = li.getRight();
	if (mfs->mode*2 == mfs->nspect && mfs->nwide == mfs->nspect+2) {
	    if (wideconv_p != BIMA && wideconv_p != MIRIAD) 
		wideconv_p = (wideconv_p == NCONV) ? BIMA : MIXED;
	}
	else if (mfs->nwide >= 2) {
	    if (wideconv_p != MIRIAD) 
		wideconv_p = (wideconv_p == NCONV) ? MIRIAD : MIXED;
	} 
	else {
	    if (wideconv_p != NONE) 
		wideconv_p = (wideconv_p == NCONV) ? NONE : MIXED;
	}
    }

    previewed_p = True;
}

void MirVisReader::fullscan(Int scanlim, Int obslim) {
    // assume reset() has been called by preview()
    
    Double time, intv=0, tmp;
    Int vlen, vupd, nread, nscan=0, nobs=0, nrec=0, yy=-6;
    Int badnsyst=-1, badwsyst=-1;
    char vtype[2];
    MirSource *source = NULL;
    FillMetadata fm;
    fm.pol = &pol_p;

    MirFreqSetup::clearIDs(fsetups_p);
    MirSource::clearIDs(sources_p);
    pol_p.clearIDs();

    if (Debug(1)) {
	Int def=1;
	uvrdvr_c(uv_handle_p, H_INT, "npol", IOCAST(&nread), IOCAST(&def), 1);
	if (nread > 1) cout << "npol=" << nread << endl;
    }
    uvprobvr_c(uv_handle_p, "pol", vtype, &vlen, &vupd);
    if (! vupd) {
	log_p << LogIO::WARN << "unspecified polarization found; "
	      << "assuming YY" << LogIO::POST;
    }

    fm.inittime((buf_p[0]->preamble()[3] - 2400000.5) * C::day);
    time = fm.starttime;

    // set to read only one channel for efficiency
    uvset_c(uv_handle_p, "data", "channel", 1, 1.0, 1.0, 1.0);

    Bool varstable = False;
    Bool moredata = hasmore_p;

    while (moredata) {

        handleVarChanges(fm,time);
        varstable = True;

        while (varstable) {
            nrec++;

            // check the system temperatures
            uvprobvr_c(uv_handle_p, "systemp", vtype, &vlen, &vupd);
            if (vlen > 0) {
                if (badnsyst < 0) badnsyst=0;
                if (vlen != Int(fm.nsystemp.nelements())) {
                  if (badnsyst == 0 && badnsyst_p < 1) 
                    log_p << LogIO::WARN<< "Inconsistent number of narrowband "
                          << "system temperatures found (got " << vlen 
                          << "; expected " << fm.nsystemp.nelements() 
                          << ")." << LogIO::POST;
                  badnsyst++;
                }
            }
            uvprobvr_c(uv_handle_p, "wsystemp", vtype, &vlen, &vupd);
            if (vlen > 0) {
                if (badnsyst < 0) badnsyst=0;
                if (vlen != Int(fm.wsystemp.nelements())) {
                  if (badwsyst == 0 && badwsyst_p < 1) 
                    log_p << LogIO::WARN << "Inconsistent number of wideband "
                          << "system temperatures found (got " << vlen 
                          << "; expected " << fm.wsystemp.nelements() 
                          << ")." << LogIO::POST;
                  badwsyst++;
                }
            }

	    // increment scan number if necessary.  We will give consecutive 
	    // observations of the same source within a short time limit 
	    // (usually ~3-5 minutes) the same scan number.  (scanlim_p < 0 
	    // forces all records into the same scan.)
	    if ( scanlim >= 0 && 
		 (intv < 0  || intv > scanlim || 
		  fm.obsupd || source != fm.source) )
	    {
		nscan++;
		source = fm.source;
	    }

	    // observation id.  We will reset the observation number
	    // whenever the time difference between this record and 
	    // the last is greater than some limit (usually hours).
	    // Certain variable changes (i.e. telescope) will also trigger
	    // an obs id update.  Note however that an obslim_p < 0 forces 
	    // all records into the same observation.  (Also, don't bother
            // updating if we have yet to write a 
	    if (obslim >= 0 && (fm.obsupd || intv < 0 || intv > obslim)) {
                nobs++;
	    }
	    fm.obsupd = False;

	    // polarization
	    uvrdvr_c(uv_handle_p,H_INT, "pol", IOCAST(&vlen), IOCAST(&yy), 1);
	    pol_p.setCorr(vlen);

            // read in the next data record
            nread = 0;
            uvread_c(uv_handle_p, buf_p[0]->preamble(), buf_p[0]->narrow(), 
		     &vlen, 1, &nread);
	    if (nread <= 0) {
                moredata = False;
                break;
            }
            varstable = ! uvupdate_c(uv_handle_p);

	    // time key
	    tmp = (buf_p[0]->preamble()[3] - 2400000.5) * C::day;
	    intv = tmp - time;

            // This gets around a disturbing precision error under Linux 
            // when optimization is turned on; I'm not happy with this 
            // workaround, but it's probably harmless.  Intervals between 
            // integrations are forced to be > 1 microsecond.  Absolute
            // precision is not messed with.
            if (intv != 0 && abs(intv) < 1.0e-6) intv = 0;

	    if (intv != 0) {
                fm.lasttime = time;
                time = tmp;

                if (Debug(4)) {   // timeline monitoring...
                    MVTime mjd_date(time/C::day);
                    cout << "DATE=" << mjd_date.string(MVTime::YMD) 
			 << " STEP=" << time - fm.lasttime << endl;
                } // debugging 
            }

        } // end varstable

    }

    narray_p = fm.narrays;
    nrec_p = nrec;
    starttime_p = fm.starttime;
    endtime_p = time;
    nobs_p = nobs;
    nscan_p = nscan;
    scanned_p = True;
    badnsyst_p = badnsyst;
    badwsyst_p = badwsyst;
}

void MirVisReader::briefscan() {
    // assume reset() has been called by preview()
    Int vval=1, vupd, vlen, yy=-6;
    char vtype[2];

    // time range
    starttime_p = (buf_p[0]->preamble()[3] - 2400000.5) * C::day;
    endtime_p = 0;

    // variables
    FillMetadata fm;
    MirFreqSetup::clearIDs(fsetups_p);
    MirSource::clearIDs(sources_p);
    pol_p.clearIDs();
    fm.pol = &pol_p;

    if (Debug(1)) {
	Int def=1;
	uvrdvr_c(uv_handle_p, H_INT, "npol", IOCAST(&vval), IOCAST(&def), 1);
	if (vval > 1) cout << "npol=" << vval << endl;
    }
    uvprobvr_c(uv_handle_p, "pol", vtype, &vval, &vupd);
    if (! vupd) {
	log_p << LogIO::WARN << "unspecified polarization found; "
	      << "assuming YY" << LogIO::POST;
    }

    handleVarChanges(fm, starttime_p);

    // check the system temperatures
    if (badnsyst_p < 0) {
        uvprobvr_c(uv_handle_p, "systemp", vtype, &vlen, &vupd);
        if (vlen > 0) {
            badnsyst_p=0;
            if (vlen != Int(fm.nsystemp.nelements())) {
                if (badnsyst_p == 0) 
                    log_p << LogIO::WARN<< "Inconsistent number of narrowband "
                          << "system temperatures found (got " << vlen 
                          << "; expected " << fm.nsystemp.nelements() 
                          << ")." << LogIO::POST;
                badnsyst_p++;
            }
        }
        uvprobvr_c(uv_handle_p, "wsystemp", vtype, &vlen, &vupd);
        if (vlen > 0) {
            badwsyst_p=0;
            if (vlen != Int(fm.wsystemp.nelements())) {
                if (badwsyst_p == 0) 
                    log_p << LogIO::WARN << "Inconsistent number of wideband "
                          << "system temperatures found (got " << vlen 
                          << "; expected " << fm.wsystemp.nelements() 
                          << ")." << LogIO::POST;
                badwsyst_p++;
            }
        }
    }

    // read through first timestamp to get all;
    // set to read only one channel for efficiency
    Double time = starttime_p;
    Int nread;
    uvset_c(uv_handle_p, "data", "channel", 1, 1.0, 1.0, 1.0);
    while (time == starttime_p) {
	uvrdvr_c(uv_handle_p, H_INT, "pol", IOCAST(&vlen), IOCAST(&yy), 1);
	pol_p.setCorr(vlen);

	// read in the next data record
	nread = 0;
	uvread_c(uv_handle_p, buf_p[0]->preamble(), buf_p[0]->narrow(), &vlen, 
		 1, &nread);
	if (nread <= 0) break;

	time = (buf_p[0]->preamble()[3] - 2400000.5) * C::day;
    }

    // these values are unknown
    narray_p = -1;  
    nrec_p = -1;
    nobs_p = -1;
    nscan_p = -1;

    scanned_p = False;
}

#if 0 
GlishRecord MirVisReader::summary(Bool verbose, Bool scan) {
    return summary(verbose, (scan) ? 0 : -1, 300, 2400);
}
   
GlishRecord MirVisReader::summary(Int scanlim, Int obslim, Bool verbose) {
    return summary(verbose, 1, scanlim, obslim);
}
	
GlishRecord MirVisReader::summary(Bool verbose, Int scan, 
				  Int scanlim, Int obslim) 
{

    if (scan > 0 ||                   // force a scan, or
	(scan == 0 && ! scanned_p)    // only if not scanned before
	) 
    {  
	preview(scanlim, obslim, (scan >= 0));
    } 
    else if (! previewed_p) {
	preview(False);
    }

    GlishRecord out;
    out.add(String("nwide"), GlishArray(maxwide_p));
    out.add(String("nspect"), GlishArray(maxspect_p));
    out.add(String("nchan"), GlishArray(maxchan_p));
    out.add(String("cormode"), GlishArray(firstmode_p));
    out.add(String("npol"), GlishArray( Int(pol_p.getCorrCount()) ));
    out.add(String("pols"), toGlishArray(pol_p));
    out.add(String("starttime"), GlishArray(starttime_p));
    out.add(String("telescope"), toGlishArray(telescopes_p));
    out.add(String("scanned"), GlishArray(scanned_p));
    out.add(String("badnsyst"), GlishArray(badnsyst_p));
    out.add(String("badwsyst"), GlishArray(badwsyst_p));
    if (scanned_p) {
	out.add(String("endtime"), GlishArray(endtime_p));
        out.add(String("narray"), GlishArray(narray_p));
        out.add(String("nrec"), GlishArray(nrec_p));
    }

    Bool hasplanet = False;
    for(ListIter<MirSource*> li(sources_p); ! li.atEnd(); li++) {
	if (li.getRight()->isPlanet()) {
	    hasplanet = True;
	    break;
	}
    }
    out.add(String("hasplanet"), GlishArray(hasplanet));

    if (verbose) {
      log_p << LogIO::NORMAL << replicate("=",80) << endl
            << "Summary of Miriad UV dataset: " << inname_p;
      if (scanned_p) {
	log_p << endl;
        log_p << " Max. no. of visibility records:    " << nrec_p << endl
              << " Max. no. of spectral line windows: " << maxspect_p
              << " (Max no. of channels: " << maxchan_p << ")" << endl
              << " Max. no. of wide-band channels:    " << maxwide_p << endl
              << " No. of array configurations:       " << narray_p << endl;

        log_p << " Telescopes:";
        if (telescopes_p.len() == 0) {
            log_p << " (unspecified)" << endl;
        } else {
            ConstListIter<String> li(telescopes_p);
	    for(li.toStart(); ! li.atEnd(); ++li) {
		log_p << " " << li.getRight();
	    }
            log_p << endl;
	}

	log_p << " Polarizations found:";
	if (pol_p.getCorrCount() == 0) {
	    log_p << " (none)" << endl;
	} else {
	    ConstListIter<MirPolCorr> li(pol_p.getCorrs());
	    for(li.toStart(); ! li.atEnd(); ++li) {
		log_p << " " << li.getRight().toString();
	    }
            log_p << endl;
	}

	log_p << " Time Range: " 
              << MVTime(starttime_p/C::day).string(MVTime::YMD) << " - "
              << MVTime(endtime_p/C::day).string(MVTime::YMD) << endl;
        if (sources_p.len() > 0) {
            log_p << "Sources:" << endl;
            MirSource *src = NULL;
            for(ConstListIter<MirSource *> li(sources_p); ! li.atEnd(); ++li) {
                src = li.getRight();
                if (src != NULL) {
                    log_p.output().width(10); log_p << src->name;
                    log_p.output().width(17); 
                    log_p << MVAngle(Quantity(src->ra, "rad")).
                        string(MVAngle::TIME, 8);
                    log_p.output().width(17); 
                    log_p << MVAngle(Quantity(src->dec, "rad")).
                        string(MVAngle::DIG2, 8);
                    log_p << " (" << src->flds.len() << " field";
                    if (src->flds.len() != 1) log_p << "s";
                    log_p << ")" << endl;
                }
            }
        }
        if (fsetups_p.len() > 0) {
            Int i=1, j;
            MirFreqSetup *fs = NULL;
            if (fsetups_p.len() == 1) {
                log_p << "Frequency Setup:" << endl;
            } else {
                log_p << "Frequency Setups:" << endl;
            }
            for(ConstListIter<MirFreqSetup *> li(fsetups_p); 
                ! li.atEnd(); 
                ++li, ++i) 
            {
                fs = li.getRight();
                if (fsetups_p.len() > 1) log_p << "Setup #" << i << ":";
                log_p << "   Mode " << fs->mode << ", "
                      << fs->nchan << " line channels, "
                      << fs->nwide << " wide channels" << endl;
                log_p << " Window  #chans   start freq.     increment   "
                      << " bandwidth    rest freq."
                      << endl;
                for (j=0; j < fs->nspect; j++) {
                    log_p.output().setf(ios::right, ios::adjustfield);
                    log_p.output().width(5);  log_p << j+1 << "    ";
                    log_p.output().width(5);  log_p << fs->nschan[j] << "   ";
                    log_p.output().setf(ios::fixed, ios::floatfield);
                    log_p.output().precision(5);  
                    log_p.output().width(8); log_p << fs->sfreq[j] << " GHz   ";
                    log_p.output().setf(ios::internal, ios::adjustfield);
                    log_p.output().width(8); 
                    log_p.output().precision(2);  
                    log_p << fs->sdf[j]*1e6 << " kHz  ";
                    log_p.output().setf(ios::right, ios::adjustfield);
                    log_p.output().width(6); 
                    log_p << abs(fs->sdf[j]*fs->nschan[j]*1e3) << " MHz    ";
                    log_p.output().width(8); 
                    log_p.output().precision(5);  
                    log_p << fs->rfreq[j] << " GHz";
                    log_p << endl;
                }
            }
        }
      }
      else {
	log_p << " (based on first timestamp)" << endl;
        log_p << " Estimated no. of spectral line windows: " << maxspect_p
              << " (No. of channels: " << maxchan_p << ")" << endl
              << " Estimated no. of wide-band channels:    " 
              << maxwide_p << endl;

        log_p << " Telescopes:";
        if (telescopes_p.len() == 0) {
            log_p << " (unspecified)" << endl;
        } else {
            ConstListIter<String> li(telescopes_p);
	    for(li.toStart(); ! li.atEnd(); ++li) {
		log_p << " " << li.getRight();
	    }
            log_p << endl;
	}

	log_p << " Polarizations found:";
	if (pol_p.getCorrCount() == 0) {
	    log_p << " (none)" << endl;
	} else {
	    ConstListIter<MirPolCorr> li(pol_p.getCorrs());
	    for(li.toStart(); ! li.atEnd(); ++li) {
		log_p << " " << li.getRight().toString();
	    }
	}

	log_p << " Start Time: " 
              << MVTime(starttime_p/C::day).string(MVTime::YMD) << endl;
      }
      log_p << replicate("-",80) << LogIO::POST;
    }

    return out;
}

GlishArray MirVisReader::toGlishArray(ConstMirPolSetup &pol) {
    ConstListIter<MirPolCorr> li(pol.getCorrs());
    Vector<String> names(pol.getCorrCount());
    Int i;
    for(i=0, li.toStart(); ! li.atEnd(); ++li, ++i) {
	names(i) = li.getRight().toString();
    }
    return GlishArray(names);
}

GlishArray MirVisReader::toGlishArray(const List<String>& list) {
    ConstListIter<String> li(list);
    Vector<String> names(list.len());
    Int i;
    for(i=0, li.toStart(); ! li.atEnd(); ++li, ++i) {
	names(i) = li.getRight();
    }
    return GlishArray(names);
}
#endif

void MirVisReader::handleVarChanges(FillMetadata &fm, Double time) {
    if (Debug(3)) cout << "MirVisReader::adaptToVarChange()" << endl;
    char vtype[10], vdata[1024];    // allows for up to 42 antennas
    int vlen, vupd, i;
    
    if (uv_handle_p == 0) {
	if (Debug(3)) cout << "premature exiting of handleVarChanges()" <<endl;
	return;
    }

    // integration time
    uvprobvr_c(uv_handle_p, "inttime", vtype, &vlen, &vupd);
    if (vupd) {
        uvgetvr_c(uv_handle_p, H_REAL, "inttime", IOCAST(&(fm.inttime)), 1);
	if (varhandler_p != NULL) varhandler_p->updateIntTime(fm, time);
    }

    // telescope efficiency
    uvprobvr_c(uv_handle_p, "jyperk", vtype, &vlen, &vupd);
    if (vupd) {
        uvgetvr_c(uv_handle_p, H_REAL, "jyperk", IOCAST(&(fm.jyperk)), 1);
//	if (varhandler_p != NULL) varhandler_p->updateEfficiency(fm, time);
    }

    // observatory 
    uvprobvr_c(uv_handle_p, "telescop", vtype, &vlen, &vupd);
    if (vupd) {
        uvgetvr_c(uv_handle_p, H_BYTE, "telescop", vdata, 64);
        String telescope(vdata);
	if (Debug(2)) cout << "Found telescope=" << telescope << endl;
        if (telescope == fm.HATCREEK) telescope = fm.BIMA;
        if (fm.telescope != telescope) {
            fm.obsupd = True;
            fm.setTelescope(telescope);
            setTelescope(telescope);
            uvprobvr_c(uv_handle_p, "project", vtype, &vlen, &vupd);
            if (vupd) {
                uvgetvr_c(uv_handle_p, H_BYTE, "project", vdata, 64);
                fm.project = String(vdata);
            }

	    if (varhandler_p != NULL) 
		varhandler_p->updateObservatory(fm, time);
        }
    }

    // antenna configuration change
    uvprobvr_c(uv_handle_p, "antpos", vtype, &vlen, &vupd);
    if (vupd) {

        Int nAnts = vlen/3;
        uvprobvr_c(uv_handle_p, "nants", vtype, &vlen, &vupd);
        if (vlen == 0 && nAnts != fm.nants) {
            log_p << LogIO::WARN << "Miriad variable 'nants' not specified; "
                  << "assuming " << nAnts << LogIO::POST;
        } else {
            uvgetvr_c(uv_handle_p, H_INT, "nants", IOCAST(&nAnts), 1);
        }

        uvgetvr_c(uv_handle_p, H_DBLE, "antpos", vdata, 3*nAnts);
	Vector<Double> antpos(IPosition(1,nAnts*3), 
                              reinterpret_cast<Double *>(&vdata), SHARE);

	if (nAnts != fm.nants || ! allEQ(antpos, fm.antpos)) {
            if (verbose()) 
		log_p << LogIO::NORMAL << "Found new array with " << nAnts 
		      << " antennas" << LogIO::POST;
	    fm.nants = nAnts;
	    fm.narrays++;
	    fm.antpos = antpos;

	    Int *mount = NULL;
	    uvprobvr_c(uv_handle_p, "mount", vtype, &vlen, &vupd);
	    if (vlen > 0) {
		mount = new int[nAnts];
		if (vlen == 1) {
		    uvgetvr_c(uv_handle_p, H_INT, "mount", IOCAST(mount), 1);
		    for(i=1; i < nAnts; i++) mount[i] = mount[0];
		} else if (vlen == nAnts) {   // this may not be correct;
		    uvgetvr_c(uv_handle_p, H_INT, "mount", 
			      IOCAST(mount), nAnts);
		} else {
		    delete mount;
		    throw 
			MiriadFormatError(String("Unable to interpret ") +
					  "'mount' variable data");
		}
	    }

	    // should check UV variable 'antdiam' 

	    if (varhandler_p != NULL) varhandler_p->updateArray(fm, time);
	    if (mount != NULL) delete mount;

            // update the size of the buffer if necessary
            if (maxrec_p == 0) resizeBufferFor(nAnts*(nAnts-1)/2);
	}
    }

    // polarization (must come after check for antenna configuration change)
//      uvprobvr_c(uv_handle_p, "npol", vtype, &vlen, &vupd);
//      if (vupd) {
//  	uvgetvr_c(uv_handle_p, H_INT, "npol", IOCAST(&vlen), 1);
//  	if (Debug(2)) cout << "Found npol=" << vlen << endl;
//  	if (vlen > 1 && fm.ms != NULL) 
//            throw(MiriadUnsupportedFeatureError(String("multiple polarizations")+
//  					      " detected but not supported"));
//          if (fm.pol != NULL) {
//           int pol;
//           uvgetvr_c(uv_handle_p, H_INT, "pol", IOCAST(&pol), 1);
//           if (fm.ms != NULL && 
//               fm.pol->getCorr(0) != 0 && fm.pol->getCorr(0) != pol)
//            throw(MiriadUnsupportedFeatureError(String("multiple polarizations")+
//  					      " detected but not supported"));
//          }
//          else {
//              fm.pol = new MirPol(vlen);
//  	    if (verbose()) 
//  		log_p << LogIO::NORMAL << "Loading single polarization: " 
//  		      << fm.pol->getAipsPol() << LogIO::POST;
//          }

//  	if (varhandler_p != NULL) varhandler_p->updatePolSetup(fm, time);
//      }

    // source
    // 
    // Note:  The "srcupd" flag is set to true if the current source has
    //           changed since the last read.  This is used by the pointing
    //           and field sections later.  
    //        The id of the current source (fm.source->id) is used to 
    //           determine if it has been added to the SOURCE table yet; 
    //           If it has, it will be equal its (0-based) SOURCE_ID 
    //           (note that all records for this source in the SOURCE table
    //           will have the same SOURCE_ID); if fm.source->id < 0, 
    //           then it has not yet been added.
    // 
    char *srcvar[] = { "source", "epoch" };
    int srcupd = 0;
    for(i=0; i < 2 && ! srcupd; i++) 
        uvprobvr_c(uv_handle_p, srcvar[i], vtype, &vlen, &srcupd);
    if (srcupd) {

	// note that this will read in the source position as well
        MirSource *source = new MirSource(uv_handle_p, time);

        try {
            MirSource *found = source->findIn(sources_p);
            if (found == NULL) {
                found = source->findNameIn(sources_p);
                if (found == NULL) {
                    if (verbose())
                        log_p << LogIO::NORMAL << "Found new source=" 
			      << source->name << LogIO::POST;
                } else {
		    // this should no longer get called under sane 
		    // circumstances: we'll be here only if the epoch has 
		    // changed (eek!)
                    source->id = found->id;  // new position for old source
                    srcupd = False;          // do not add a new field
                }
                ListIter<MirSource *> siter(sources_p);
		siter.toEnd();
		siter.addRight(source);
		fm.source = source;
	    }
	    else {
		fm.source = found;
		delete source; source = 0;
	    }

	    if (fm.source->id < 0 && varhandler_p != 0) 
		varhandler_p->updateSource(fm, time);
        } 
	catch (AipsError) { if (source != 0) delete source; throw; }

    }

    // pointing direction (is this a planet?)
    // 
    // This section detects moving objects--sources whose name does not
    // change but whose position does--e.g. planets.   Thus, if the source
    // has changed (srcupd=true), then we should not bother looking for
    // a change in position
    //
    char *posvar[] = { "ra", "dec" };
    if (! srcupd) {
      for(i=0, vupd=0; i < 2 && ! vupd; i++) 
	  uvprobvr_c(uv_handle_p, posvar[i], vtype, &vlen, &vupd);
      if (vupd) {
	Double ra, dec, zero=0.0;
        uvrdvr_c(uv_handle_p, H_DBLE,"ra", IOCAST(&ra), IOCAST(&zero), vlen);
        uvrdvr_c(uv_handle_p, H_DBLE,"dec",IOCAST(&dec),IOCAST(&zero), vlen);
	if (fm.source->ra != ra || fm.source->dec != dec) {
	    fm.movingsrc = True;
	    fm.source->addPosition(time, ra, dec);
	}
      }
    }

    // field change
    //
    // If the source has changed (srcupd), we should go ahead and read in 
    // the offsets so that we can see if they are different from previously
    // read offsets for this source.  
    //
    char *fldvar[] = { "dra", "ddec" };
    for(i=0, vupd=0; i < 2 && ! vupd; i++) 
        uvprobvr_c(uv_handle_p, fldvar[i], vtype, &vlen, &vupd);
    if (vupd || srcupd) {

        // get the new offsets.  Note that if the source changed 
        // but offsets have not been set, defaults of (0,0) are used.
	float dra, ddec, zero=0.0;
        uvrdvr_c(uv_handle_p, H_REAL,"dra", IOCAST(&dra), IOCAST(&zero), vlen);
        uvrdvr_c(uv_handle_p, H_REAL,"ddec",IOCAST(&ddec),IOCAST(&zero), vlen);
	MirField *offset = new MirField(dra, ddec);
	try {
	    MirField *found = offset->findIn(fm.source->flds);
	    if (found == NULL) {

		// add this newly-found field to our list of fields
                ListIter<MirField*> flditer(fm.source->flds);  
		flditer.toEnd();
		flditer.addRight(offset);
		fm.field = offset;
	    }
	    else {
		fm.field = found;
		delete offset;  offset = 0;
	    }
	}
	catch (AipsError) { if (offset != 0) delete offset; throw; }

	if (varhandler_p != NULL) varhandler_p->updateField(fm, time);
    } 

    // correllator setup.  
    char *corrvar[] = { "cormode", "nspect", "nwide", "corfin", "corbw", 
			"restfreq" };
    for(i=0, vupd=0; i < 6 && ! vupd; i++) 
        uvprobvr_c(uv_handle_p, corrvar[i], vtype, &vlen, &vupd);
    if (vupd) {
	MirFreqSetup *setup = new MirFreqSetup(uv_handle_p);
        if (firstmode_p == 0) firstmode_p = setup->mode;
	try {
	    MirFreqSetup *found = setup->findIn(fsetups_p);
	    if (found == NULL) {
		if (verbose()) log_p << LogIO::NORMAL 
				     << "Found new frequency setup with nspec=" 
				     << setup->nspect << ", nwide=" 
				     << setup->nwide << LogIO::POST;

                ListIter<MirFreqSetup *> fsiter(fsetups_p);
		fsiter.toEnd();
		fsiter.addRight(setup);
		fm.fsetup = setup;
                if (setup->nchan  > maxchan_p)  maxchan_p  = setup->nchan;
                if (setup->nwide  > maxwide_p)  maxwide_p  = setup->nwide;
                if (setup->nspect > maxspect_p) maxspect_p = setup->nspect;
	    }
	    else {
		fm.fsetup = found;
		delete setup; setup = 0;
	    }

	}
	catch (AipsError) { if (setup != 0) delete setup; throw; }

	if (varhandler_p != NULL) varhandler_p->updateFreqSetup(fm, time);
    }

    // planet observation information -- dgoscha (NCSA) 04 Feb, 2002
    // read in:
    //	plangle (real, degrees, "Planet Angle")
    //  plmaj (real, arcsec, "Planet major axis")
    //  plmin (real, arcsec, "Planet minor axis")
    //  pltb (real, Kelvin, "Planet brightness")
    // Note: we put this after correlator setup to ensure that all 
    // lines have been added to the source
    char *plvar[] = { "plangle", "plmaj", "plmin", "pltb" };

    for(i=0, vupd=0; i < 4 && ! vupd;  i++) 
	uvprobvr_c(uv_handle_p, plvar[i], vtype, &vlen, &vupd);
    if (vupd) {
	uvgetvr_c(uv_handle_p, H_REAL, "plangle", IOCAST(&(fm.plangle)), 1);
	uvgetvr_c(uv_handle_p, H_REAL, "pltb", IOCAST(&(fm.pltb)), 1);
	uvgetvr_c(uv_handle_p, H_REAL, "plmaj", IOCAST(&(fm.plmaj)), 1);
	uvgetvr_c(uv_handle_p, H_REAL, "plmin", IOCAST(&(fm.plmin)), 1);
	uvgetvr_c(uv_handle_p, H_DBLE, "freq", IOCAST(&(fm.freq)), 1);

//  	log_p << LogIO::WARN << "plangle: " << fm.plangle 
//  	      << " plmaj: " << fm.plmaj
//  	      << " plmin: " << fm.plmin
//  	      << " pltb: " << fm.pltb 
//  	      << " freq: " << fm.freq << LogIO::POST;
    }
    if (varhandler_p != NULL) {
	// Make sure it is a valid planet record.
	if (fm.plmaj > fm.plmin && fm.plmaj != 0) {
	    varhandler_p->updateSourceModel(fm, getTime());
	} 
    }

    Bool resizentsys = False, resizewtsys = False, tsysupd = False;
    PtrHolder<Float> tsys(new Float[MAXWIDE*fm.nants], True);
    if (fm.nsystemp.shape() != IPosition(2, fm.nants, fm.fsetup->nspect)) {
	fm.nsystemp.resize(fm.nants, fm.fsetup->nspect);
        fm.nsystemp = -1.0;  // indicates no value read in
        resizentsys = True;
    }
    if (fm.wsystemp.shape() != IPosition(2, fm.nants, fm.fsetup->nwide)) {
	fm.wsystemp.resize(fm.nants, fm.fsetup->nwide);
        fm.wsystemp = -1.0;  // indicates no value read in
        resizewtsys = True;
    }
    uvprobvr_c(uv_handle_p, "systemp", vtype, &vlen, &vupd);
    if (vlen > 0 && (resizentsys || vupd)) {
        if (vlen == Int(fm.nsystemp.nelements())) {
            tsysupd = True;
            uvgetvr_c(uv_handle_p, H_REAL, "systemp", IOCAST(tsys.ptr()), 
                      fm.nsystemp.nelements());
            fm.nsystemp.takeStorage(fm.nsystemp.shape(), tsys.ptr());
        } 
        else {
            fm.nsystemp = -1.0;
        }
    }
    uvprobvr_c(uv_handle_p, "wsystemp", vtype, &vlen, &vupd);
    if (vlen > 0 && (resizewtsys || vupd)) {
        if (vlen == Int(fm.wsystemp.nelements())) {
            tsysupd = True;
            uvgetvr_c(uv_handle_p, H_REAL, "wsystemp", IOCAST(tsys.ptr()), 
                      fm.wsystemp.nelements());
            fm.wsystemp.takeStorage(fm.wsystemp.shape(), tsys.ptr());
        } 
        else {
            fm.wsystemp = -1.0;
        }
    }

    if (varhandler_p != NULL && tsysupd)
        varhandler_p->updateTsys(fm, time_p);

    if (Debug(3)) cout << "leaving handleVarChange()" << endl;
}
