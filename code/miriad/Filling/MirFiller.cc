//# ClassFileName.cc:  this defines ClassName, which ...
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
//# $Id: MirFiller.cc,v 1.3 2011/08/11 18:03:26 pteuben Exp $

//# Includes
#include <miriad/Filling/MirFiller.h>
#include <miriad/Filling/FillMetadata.h>
#include <miriad/Filling/MirExceptions.h>

#include <measures/Measures.h>
#include <measures/Measures/MeasData.h>
#include <measures/Measures/Stokes.h>
#include <casa/Quanta/MVDirection.h>
#include <casa/Quanta/MVAngle.h>
#include <casa/Quanta/MVFrequency.h>
#include <tables/Tables.h>
#include <tables/Tables/TableInfo.h>
#include <measures/TableMeasures.h>
#include <casa/Arrays/ArrayMath.h>
#include <casa/Arrays/MatrixMath.h>
#include <casa/Arrays/ArrayUtil.h>
#include <casa/Arrays/ArrayLogical.h>
#include <casa/Arrays/MaskedArray.h>

#include <ms/MeasurementSets.h>              // Measurementset and MSColumns
#include <casa/Logging/LogIO.h>

#include <casa/OS/File.h>
#include <casa/OS/Time.h>
#include <casa/Containers/SimOrdMap.h>
//#include <tasking/Glish/GlishRecord.h>
//#include <tasking/Glish/GlishArray.h>

#include <scimath/Functionals/Polynomial.h>
#include <scimath/Mathematics/AutoDiff.h>
#include <scimath/Fitting/LinearFitSVD.h>

#include <components/ComponentModels/ComponentList.h>
#include <components/ComponentModels/DiskShape.h>
#include <components/ComponentModels/SpectralIndex.h>
#include <components/ComponentModels/Flux.h>

// Miriad interface
#include <mirlib/maxdimc.h>
#include <mirlib/miriad.h>

#include <casa/stdio.h>
#include <casa/sstream.h>

//  the maximum number of fields in mosaicing observations, careful, miriad
//  often has a small value, like 64 or 128.
#ifndef MAXFIELD
# define MAXFIELD  256
#endif

#define FILLERNAME "MirFiller"

String MirFiller::wideconvnames[] = { "none", "bima", "miriad", "mixed" };

MirFiller::MirFiller(const String& mirfile, Bool scan, Int dbg) 
    : rdr_p(mirfile, scan, dbg), log_p(LogOrigin(FILLERNAME)), 
      defpolsel_p(NULL), histbl_p(), scanlim_p(5*60), obslim_p(4*60*60), 
      tilesize_p(32), // 256
      planetfit_p(4), 
      verbose_p(False), joinpol_p(False), tsyswt_p(True), 
      movfield_p(True), compress_p(False), wideconv_p(MirFiller::NONE),
      updmodelint_p(8*60*60)
{
    // create the filler

    setDebugLevel(dbg);

    if (Debug(1)) cout << FILLERNAME << "::" << FILLERNAME << " debug_level=" 
                       << getDebugLevel() << endl;
    if (Debug(1)) cout << "*** Opening " << mirfile << endl;

//    rdr_p = new MirVisReader(mirfile, scan, dbg);
#if 0
    //TODO
    GlishRecord sum = rdr_p.summary(False, False);
    Int nwide, nspect;
    ((GlishArray) sum.get("nwide")).get(nwide);
    ((GlishArray) sum.get("nspect")).get(nspect);
    widesel_p.resize(IPosition(1,nwide));
    widesel_p = True;
    winsel_p.resize(IPosition(1,nspect));
    winsel_p = True;
#endif
}

MirFiller::~MirFiller() 
{
    // destruct this filler
    if (Debug(3)) cout << FILLERNAME << "::~" << FILLERNAME << endl;
    if (defpolsel_p != NULL) delete defpolsel_p;
}

void MirFiller::noJoinFill(const String& msfile) { 
    // fill the output MS according to the current selections and options

    if (Debug(3)) cout << FILLERNAME << "::noJoinFill()" << endl;
#if 0

    Int i,k, a,s,w,r, offset, scan=0, accepted=0, nvis=0, 
	row, brow, nrow, nspw, nacc, mant1, mant2, aant1, aant2, p1, p2;
    Double time=0, intv=0, timecent;
    Vector<Double> uvw(3);
    MirSource *source = NULL;
    MirDataRecord *rec = NULL;

    // initialize the reader 
    //PJT    GlishRecord sum = rdr_p.summary(False, False);
    //rdr_p.reset(this, verbose_p, nwidesel_p > 0, 0);

    Bool prescanned = False;
    //((GlishArray) sum.get("scanned")).get(prescanned);

    // determine if we need a SOURCE_MODEL column (for planets).  Assume
    // we do if we have not prescanned (as "hasplanet" may not be correct).
    Bool needSrcMods = ! prescanned;
    if (! needSrcMods) ((GlishArray) sum.get("hasplanet")).get(needSrcMods);

    // set up the fill metadata container and output MS
    FillMetadata fm(msfile);
    fm.polotf = ! prescanned;
    ((GlishArray) sum.get("starttime")).get(time);
    fm.inittime(time, updmodelint_p);
    fm.ms = setupMS(msfile, tilesize_p, needSrcMods);
    fm.msc = new MSColumns(*(fm.ms));
    initMS(fm);
    nrow = fm.ms->nrow();

    // set the flagging catagories
    Vector<String> flgCat(2);
    flgCat(0)="FLAG_CMD";
    flgCat(1)="ORIGINAL";
    fm.msc->flagCategory().rwKeywordSet().define("CATEGORY",flgCat);

    // get weight and rms buffers
    Vector<Float> &wt = fm.buf.weight();
    Vector<Float> &rms = fm.buf.sigma();

    Int nints = 0;
    while ((nints = rdr_p.readIntegration(fm)) > 0) {
	// the reader calls back to this filler in readIntegration() 
	// when variables have changed; the filler updates the subtables 
	// accordingly.  All the records read in are guaranteed to have
	// the same variable state and the same timestamp

        nspw=fm.fsetup->nspect+fm.fsetup->nwide;    
//	accepted += nints;

	// update time
 	intv = rdr_p.getTime() - time;

	// This gets around a disturbing precision error under Linux 
	// when optimization is turned on; I'm not happy with this 
	// workaround, but it's probably harmless.  Intervals between 
	// integrations are forced to be > 1 microsecond.  Absolute
	// precision is not messed with.
	if (intv != 0 && abs(intv) < 1.0e-6) intv = 0;

	if (intv != 0) {
	    fm.lasttime = time;
	    time = rdr_p.getTime();

	    if (Debug(4)) {   // timeline monitoring...
		Double step = (fm.lasttime == 0) ? 0 : time - fm.lasttime;
		MVTime mjd_date(time/C::day);
		cout << "DATE=" << mjd_date.string(MVTime::YMD) 
		     << " STEP=" << step << endl;
	    } // debugging 
	}
	timecent = time + fm.inttime/2;

	// update scan number if necessary.  We will give consecutive 
	// observations of the same source within a short time limit 
	// (usually ~3-5 minutes) the same scan number.  (scanlim_p < 0 
	// forces all records into the same scan.)
	if ( scanlim_p >= 0 && 
	     (intv < 0  || intv > scanlim_p || 
	      fm.obsupd || source != fm.source) )
	{
	    scan++;
	    if (verbose()) {
		MVTime mjd_date(time/C::day);
		log_p << LogIO::POST << "Scan " << scan
		      << ": DATE=" << mjd_date.string(MVTime::YMD) 
		      << " (Jump since last record: " << intv << " s)" 
		      << LogIO::POST;
	    }
	}

	// observation id.  We will reset the observation number
	// whenever the time difference between this record and 
	// the last is greater than some limit (usually hours).
	// Certain variable changes (i.e. telescope) will also trigger
	// an obs id update.  Note however that an obslim_p < 0 forces 
	// all records into the same observation.  (Also, don't bother
	// updating if we have yet to write a 
	if (obslim_p >= 0 && (fm.obsupd || intv < 0 || intv > obslim_p)) {

	    // Don't bother updating if we have yet to write out any 
	    // visibilities
	    if (nvis > 0) {
		flushObsRecord(fm, time);
		fm.obsid++;
	    }
	    fm.obstime = time;
	}
	fm.obsupd = False;

	// figure out proper polarization setup
	if (source != fm.source) {
	    // swap in desired setup for this source if one has been defined;
	    // otherwise, use the default
	    source = fm.source;
	    if (0 /* we have set by-source polarization selections */) 
		1; /* fm.pol = source selection */
	    else if (defpolsel_p != NULL) 
		fm.pol = defpolsel_p;
	    else 
		fm.pol = &(rdr_p.getDefaultPolSetup());
	}

	// remember the number of the first row in this block
	brow = row = nrow;

	// The output sort order will be TIME, DATA_DESC_ID, BASELINE.
	// This collects spectral windows together so that the shape 
	// of the DATA column changes less frequently and thus (hopefully)
	// improving access performance.
	//
	// Add information that is constant or window-dependent over this
	// integration by looping first over window and then over record
	for(s=0; s < nspw; s++) {
	    if ( (s < fm.fsetup->nspect && ! winsel_p(s)) ||
		 (s >=fm.fsetup->nspect && 
		  ! widesel_p(s-fm.fsetup->nspect)) )
		continue;

	    // get data-loading buffers
	    Matrix<Complex> &vis = fm.buf.visForSpWin(s);
	    Cube<Bool> &flgs = fm.buf.flagsForSpWin(s);

	    for(r=0, a=0; r < rdr_p.getNumRecs(); r++) {
		rec = rdr_p.getRecord(r);

		// polarization
		if (! fm.pol->hasCorr(rec->pol())) continue;
		
		// copy data and flags into loadable containers
                if (s < fm.fsetup->nspect) {     // spectral data
		    nvis += fm.fsetup->nschan[s];
                    offset = (fm.fsetup->ischan)[s]-1;
                    for(k=0; k < fm.fsetup->nschan[s]; k++) {

			// visibility is conjugated because miriad uses 
			// bl=ant1-ant2 but AIPS++ uses bl=ant2-ant1
                        vis(0,k) = Complex( rec->narrow()[2*(k+offset)], 
                                           -rec->narrow()[2*(k+offset)+1] );
                        flgs(0,k,1) = (rec->flags()[k+offset] == 0);
                    }
                }
                else {                          // wideband data
                    k = s-fm.fsetup->nspect;
		    nvis++;
                    vis = Complex(rec->wide()[2*k], -rec->wide()[2*k+1]);
                    flgs(0,0,1) = (rec->wflags()[k] == 0);
                }

		// add a record to the MAIN table
		fm.ms->addRow();

		fm.msc->time().put(row, timecent);
                fm.msc->processorId().put(row, 0);
                fm.msc->fieldId().put(row, fm.field->id);
                fm.msc->interval().put(row, fm.inttime);
                fm.msc->exposure().put(row, fm.inttime);
                fm.msc->timeCentroid().put(row, timecent);
                fm.msc->scanNumber().put(row, scan);
                fm.msc->arrayId().put(row, fm.narrays-1);
                fm.msc->observationId().put(row, fm.obsid);
                fm.msc->feed1().put(row, 0);
                fm.msc->feed2().put(row, 0);
                fm.msc->stateId().put(row, 0);
                fm.msc->data().put(row, vis);
                fm.msc->flag().put(row, flgs.xyPlane(1));
                fm.msc->flagCategory().put(row, flgs);
                fm.msc->flagRow().put(row, False);
                row++; a++;
	    } // end rec iter

        } // end spect iter
	nrow = row;
	nacc = a;
	accepted += a;

	// Add information that is record-dependent over this
	// integration by looping first over record and then over window 
	for(r=0, a=0; r < rdr_p.getNumRecs(); r++) {
	    rec = rdr_p.getRecord(r);

	    // polarization
	    if (! fm.pol->setCorr(rec->pol())) continue;
	    if (fm.pol->getCurrent().getID() < 0) {
		addPolarization(fm, True);
	    }
	    if (fm.getDataDescID(fm.fsetup->id, 
				 fm.pol->getCurrent().getID()) < 0) 
                addDataDesc(fm);

	    // baseline: baseline = 256*A1 + A2
	    mant1 = Int(rec->preamble()[4])/256;
	    mant2 = Int(rec->preamble()[4]) - mant1*256;
	    if (Debug(5)) cout << "baseline("<<mant1 << "," << mant2 << ") ";
	    aant1 = fm.getAntennaID(mant1);
	    aant2 = fm.getAntennaID(mant2);
	    if (Debug(5)) cout << "==> (" << aant1 << "," << aant2 << ") " 
			       << endl;

	    p1 = fm.polrecp->find(fm.pol->getCurrent().getFirstPolType()); 
	    p2 = fm.polrecp->find(fm.pol->getCurrent().getSecondPolType()); 

	    // baseline vector
	    uvw(0) = -rec->preamble()[0] * 1e-9; // convert to seconds
	    uvw(1) = -rec->preamble()[1] * 1e-9; //  (miriad uses nanosec)
	    uvw(2) = -rec->preamble()[2] * 1e-9; //
	    uvw   *= C::c;                // Convert to meters

	    for(s=0, w=0; s < nspw; s++) {
		if ( (s < fm.fsetup->nspect && ! winsel_p(s)) ||
		     (s >=fm.fsetup->nspect && 
		      ! widesel_p(s-fm.fsetup->nspect)) )
		    continue;

		// load the system temperature into its loading buffer
		rms = 1;
		wt = 1;
		if (tsyswt_p) {
		    Int nchan=0;
		    Double fwidth=0;
		    Float tsys1, tsys2;
		    if (s < fm.fsetup->nspect) {
			if (fm.nsystemp.nelements() > 0) {
			    fwidth = abs(fm.fsetup->sdf[s]);
			    nchan = fm.fsetup->nschan[s];
			    tsys1 = fm.nsystemp(mant1-1, s);
			    tsys2 = fm.nsystemp(mant2-1, s);
			}
		    } else if (fm.wsystemp.nelements() > 0) {
			fwidth = abs(fm.fsetup->wwidth[s-fm.fsetup->nspect]);
			nchan = 1;
			tsys1 = fm.wsystemp(mant1-1, s-fm.fsetup->nspect);
			tsys2 = fm.wsystemp(mant2-1, s-fm.fsetup->nspect);
		    }
		    if (nchan > 0) {
			rms(0) = square(fm.jyperk) * tsys1 * tsys2 /
			           (fm.inttime*fwidth*2.0e9);
			if (rms(0) > 0) {
			    wt(0) = nchan/rms(0);
			    rms(0) = sqrt(rms(0));
			} else {
			    wt(0) = rms(0) = 0;
			}
		    }
		}

		row = brow + nacc*w + a;

		fm.msc->antenna1().put(row, aant1);
		fm.msc->antenna2().put(row, aant2);
                fm.msc->dataDescId().put(row, 
		    fm.getDataDescID(fm.fsetup->id + w++, 
				     fm.pol->getCurrent().getID()));
                fm.msc->uvw().put(row, uvw);
                fm.msc->sigma().put(row, rms);
                fm.msc->weight().put(row, wt); 
	    }
            a++;
	}

    } // end more data

    // flush all pending subtable data
    fm.lasttime = time;
    flushObsRecord(fm, time);
    flushFeedRecords(fm, time);
    flushMovingFields(fm, time);

    ostringstream status;
    status << "Accepted " << accepted << " input MIRIAD records.\n"
	   << "Loaded " << fm.ms->nrow() << " data records for\n"
	   << "       " << nvis << " visibilities,\n"
	   << "       " << (i = fm.ms->spectralWindow().nrow());
    status << " spectral window" << ((i == 1) ? ",\n" : "s,\n");
    status << "       " << (i = fm.ms->polarization().nrow());
    status << " polarization" << ((i == 1) ? ",\n" : "s,\n");
    status << "       " << (i = fm.ms->field().nrow());
    status << " field" << ((i == 1) ? ", and\n" : "s, and\n")
	   << "       " << fm.narrays << " array configuration";
    if (fm.narrays != 1) status << "s";
    status << "." ;
    log_p << LogIO::NORMAL << String(status.str()) << LogIO::POST;
    addHistoryMessage(fm, "NORMAL", "fill", status.str());
#endif        
    if (Debug(3)) cout << "leaving noJoinFill()" << endl;
}

void MirFiller::joinFill(const String& msfile) { 
    // fill the output MS according to the current selections and options

    if (Debug(3)) cout << FILLERNAME << "::joinFill()" << endl;

    Int i,j,k, a,r,s,w, offset, scan=0, accepted=0, nvis=0, 
	row, nspw, mant1, mant2, aant1, aant2, p1, p2;
    Double time=0, intv=0, timecent, baseline;
    Bool matched = False;
    Vector<Double> uvw(3);
    MirSource *source = NULL;
    MirDataRecord *rec = NULL;

    // initialize the reader 
    //PJT 
    //GlishRecord sum = rdr_p.summary(False, True);
    //rdr_p.reset(this, verbose_p, nwidesel_p > 0, 0);

    // determine if we need a SOURCE_MODEL column (for planets).  Assume
    // we do if we have not prescanned (as "hasplanet" may not be correct).
    Bool prescanned = False;
    // ((GlishArray) sum.get("scanned")).get(prescanned);
    Bool needSrcMods = ! prescanned;
    // if (! needSrcMods) ((GlishArray) sum.get("hasplanet")).get(needSrcMods);

    // set up the fill metadata container and output MS
    FillMetadata fm(msfile);
    //((GlishArray) sum.get("starttime")).get(time);
    time = 0;//PJT
    fm.inittime(time, updmodelint_p);
    fm.ms = setupMS(msfile, tilesize_p, needSrcMods);
    fm.msc = new MSColumns(*(fm.ms));
    initMS(fm);
    row = fm.ms->nrow();

    // set the flagging catagories
    Vector<String> flgCat(2);
    flgCat(0)="FLAG_CMD";
    flgCat(1)="ORIGINAL";
    fm.msc->flagCategory().rwKeywordSet().define("CATEGORY",flgCat);

    // get weight and rms buffers
    Vector<Float> &wt = fm.buf.weight();
    Vector<Float> &rms = fm.buf.sigma();

    Int nints = 0;
    while ((nints = rdr_p.readIntegration(fm)) > 0) {
	// the reader calls back to this filler in readIntegration() 
	// when variables have changed; the filler updates the subtables 
	// accordingly.  All the records read in are guaranteed to have
	// the same variable state and the same timestamp

        nspw=fm.fsetup->nspect+fm.fsetup->nwide;    
//	accepted += nints;

	// update time
 	intv = rdr_p.getTime() - time;

	// This gets around a disturbing precision error under Linux 
	// when optimization is turned on; I'm not happy with this 
	// workaround, but it's probably harmless.  Intervals between 
	// integrations are forced to be > 1 microsecond.  Absolute
	// precision is not messed with.
	if (intv != 0 && abs(intv) < 1.0e-6) intv = 0;

	if (intv != 0) {
	    fm.lasttime = time;
	    time = rdr_p.getTime();

	    if (Debug(4)) {   // timeline monitoring...
		Double step = (fm.lasttime == 0) ? 0 : time - fm.lasttime;
		MVTime mjd_date(time/C::day);
		cout << "DATE=" << mjd_date.string(MVTime::YMD) 
		     << " STEP=" << step << endl;
	    } // debugging 
	}
	timecent = time + fm.inttime/2;

	// update scan number if necessary.  We will give consecutive 
	// observations of the same source within a short time limit 
	// (usually ~3-5 minutes) the same scan number.  (scanlim_p < 0 
	// forces all records into the same scan.)
	if ( scanlim_p >= 0 && 
	     (intv < 0  || intv > scanlim_p || 
	      fm.obsupd || source != fm.source) )
	{
	    scan++;
	    if (verbose()) {
		MVTime mjd_date(time/C::day);
		log_p << LogIO::POST << "Scan " << scan
		      << ": DATE=" << mjd_date.string(MVTime::YMD) 
		      << " (Jump since last record: " << intv << " s)" 
		      << LogIO::POST;
	    }
	}

	// observation id.  We will reset the observation number
	// whenever the time difference between this record and 
	// the last is greater than some limit (usually hours).
	// Certain variable changes (i.e. telescope) will also trigger
	// an obs id update.  Note however that an obslim_p < 0 forces 
	// all records into the same observation.  (Also, don't bother
	// updating if we have yet to write a 
	if (obslim_p >= 0 && (fm.obsupd || intv < 0 || intv > obslim_p)) {

	    // Don't bother updating if we have yet to write out any 
	    // visibilities
	    if (nvis > 0) {
		flushObsRecord(fm, time);
		fm.obsid++;
	    }
	    fm.obstime = time;
	}
	fm.obsupd = False;

	// figure out proper polarization setup
	if (source != fm.source) {
	    source = fm.source;

	    // swap in desired setup for this source if one has been defined;
	    // otherwise, use the default
	    ConstMirPolSetup *use = NULL;
	    if (0 /* we have set by-source polarization selections */) 
		1; /* use = source selection */
	    else if (defpolsel_p != NULL) 
		use = defpolsel_p;
	    else 
		use = &(rdr_p.getDefaultPolSetup());

	    if (use != fm.pol) {
		fm.pol = use;
		fm.buf.resizeForNpol(fm.pol->getCorrCount());
		fm.polrecp->addPolsFor(*(fm.pol));
	    }
	}

	if (fm.pol->getCurrent().getID() < 0) {
	    addPolarization(fm);
	}
	if (fm.getDataDescID(fm.fsetup->id, fm.pol->getCurrent().getID()) < 0) 
            addDataDesc(fm);

	// The output sort order will be TIME, DATA_DESC_ID, BASELINE.
	// This collects spectral windows together so that the shape 
	// of the DATA column changes less frequently and thus (hopefully)
	// improving access performance.
	//
	// Add information that is constant or window-dependent over this
	// integration by looping first over window and then over record
	for(s=0, w=0; s < nspw; s++) {
	    if ( (s < fm.fsetup->nspect && ! winsel_p(s)) ||
		 (s >=fm.fsetup->nspect && 
		  ! widesel_p(s-fm.fsetup->nspect)) )
		continue;

	    rdr_p.clearMarks();
	    Int nchan=0;
	    Double fwidth=0;

	    // get data-loading buffers
	    Matrix<Complex> &vis = fm.buf.visForSpWin(s);
	    Cube<Bool> &flgs = fm.buf.flagsForSpWin(s);

	    // step by baseline
	    ConstListIter<MirPolCorr> pi(fm.pol->getCorrs());
	    for(r=0, a=0; r < rdr_p.getNumRecs(); ++r) {
		rec = rdr_p.getRecord(r);
		if (rec->isMarked()) continue;
		baseline = rec->preamble()[4];

		// baseline: baseline = 256*A1 + A2
		mant1 = Int(baseline)/256;
		mant2 = Int(baseline) - mant1*256;
		if (Debug(5)) cout << "baseline("<<mant1 << "," <<mant2 << ") ";
		aant1 = fm.getAntennaID(mant1);
		aant2 = fm.getAntennaID(mant2);
		if (Debug(5)) cout << "==> (" << aant1 << "," << aant2 << ") " 
				   << endl;

		p1 = fm.polrecp->find(fm.pol->getCurrent().getFirstPolType()); 
		p2 = fm.polrecp->find(fm.pol->getCurrent().getSecondPolType()); 
		if (tsyswt_p) {
		    rms = 0;
		    wt = 0;
		} else {
		    rms = 1;
		    wt = 1;
		}

		// baseline vector
		uvw(0) = -rec->preamble()[0] * 1e-9; // convert to seconds
		uvw(1) = -rec->preamble()[1] * 1e-9; //  (miriad uses nanosec)
		uvw(2) = -rec->preamble()[2] * 1e-9; //
		uvw   *= C::c;                // Convert to meters

		// Buffer correlation data for this baseline.
		// We need to gather all the polarizations for the current
		// baseline; step by polarization correlation type
		for(i=0, pi.toStart(); ! pi.atEnd(); ++pi, ++i) {

		    // find the record with the current baseline and 
		    // polarization
                    rec = rdr_p.getRecord(r);
		    for(j=r+1; rec->preamble()[4] != baseline ||
			       rec->pol() != pi.getRight().getType(); j++) 
		    {
			if (j == rdr_p.getNumRecs()) j = 0;
			rec = rdr_p.getRecord(j);
			if (j==r) break;
			if (rec->isMarked()) continue;
		    }
		    matched = rec->preamble()[4] == baseline && 
			      rec->pol() == pi.getRight().getType();

                    if (Debug(4)) {
                        cout << "row=" << row << ", s=" << s 
                             << ", ant1=" << mant1 << ", ant2=" << mant2
                             << ", i=" << i << ", p1=" << p1 << ", p2=" 
                             << p2 << endl;
                        if (row > 32) {
                            cout << "pause" << endl;
                        }
                    }

		    // copy the data and flags for the current pol corr type
		    // into the loadable containers
		    if (s < fm.fsetup->nspect) {     // spectral data
			nvis += fm.fsetup->nschan[s];
			offset = (fm.fsetup->ischan)[s]-1;
			if (matched) {
			    for(k=0; k < fm.fsetup->nschan[s]; k++) {

				// visibility is conjugated because miriad 
				// uses bl=ant1-ant2 but AIPS++ uses 
                                // bl=ant2-ant1
				vis(i,k)=Complex( rec->narrow()[2*(k+offset)], 
 					       -rec->narrow()[2*(k+offset)+1]);
				flgs(i,k,1) = (rec->flags()[k+offset] == 0);
			    }
			    rec->setMarked(True);

			    if (tsyswt_p && fm.nsystemp.nelements() > 0) {
				fwidth = abs(fm.fsetup->sdf[s]);
				nchan = fm.fsetup->nschan[s];
				rms(i) = square(fm.jyperk) * 
				    fm.nsystemp(mant1-1, s) *
				    fm.nsystemp(mant2-1, s) /
				    (fm.inttime*fwidth*2.0e9);
				if (rms(i) > 0) {
				    wt(i) = nchan/rms(i);
				    rms(i) = sqrt(rms(i));
				} else {
				    wt(i) = rms(i) = 0;
				}
			    }
			}
			else {

			    // polarization was not found; flag zero values
			    vis.row(i) = Complex(0, 0);
			    flgs.xyPlane(1).row(i) = True;
			}
		    }
		    else {                          // wideband data
			if (matched) {
			    k = s-fm.fsetup->nspect;
			    nvis++;
			    vis(i,0) = Complex( rec->wide()[2*k], 
                                               -rec->wide()[2*k+1]);
			    flgs(i,0,1) = (rec->wflags()[k] == 0);
			    rec->setMarked(True);

			    if (tsyswt_p && fm.wsystemp.nelements() > 0) {
				fwidth = 
				    abs(fm.fsetup->wwidth[s-fm.fsetup->nspect]);
				nchan = 1;
				Int wd = s-fm.fsetup->nspect;
				rms(i) = square(fm.jyperk) * 
				    fm.wsystemp(mant1-1, wd) *
				    fm.wsystemp(mant2-1, wd) /
				    (fm.inttime*fwidth*2.0e9);
				if (rms(i) > 0) {
				    wt(i) = nchan/rms(i);
				    rms(i) = sqrt(rms(i));
				} else {
				    wt(i) = rms(i) = 0;
				}
			    }
			} 
			else {
			    vis(i,0) = Complex(0, 0);
			    flgs(i,0,1) = True;
			}
		    }
		}

		// add a record to the MAIN table
		fm.ms->addRow();

		fm.msc->time().put(row, timecent);
                fm.msc->processorId().put(row, 0);
                fm.msc->fieldId().put(row, fm.field->id);
                fm.msc->interval().put(row, fm.inttime);
                fm.msc->exposure().put(row, fm.inttime);
                fm.msc->timeCentroid().put(row, timecent);
                fm.msc->scanNumber().put(row, scan);
                fm.msc->arrayId().put(row, fm.narrays-1);
                fm.msc->observationId().put(row, fm.obsid);
                fm.msc->stateId().put(row, 0);
                fm.msc->data().put(row, vis);
                fm.msc->sigma().put(row, wt);  // temporary
                fm.msc->weight().put(row, wt); 
                fm.msc->flag().put(row, flgs.xyPlane(1));
                fm.msc->flagCategory().put(row, flgs);
                fm.msc->flagRow().put(row, False);

		fm.msc->antenna1().put(row, aant1);
		fm.msc->antenna2().put(row, aant2);
                fm.msc->feed1().put(row, 0);
                fm.msc->feed2().put(row, 0);
                fm.msc->dataDescId().put(row, 
		    fm.getDataDescID(w+fm.fsetup->id, 
				     fm.pol->getCurrent().getID()));
                fm.msc->uvw().put(row, uvw);

                row++; a++;
	    } // end rec iter
            w++;

        } // end spect iter
	accepted += a;

    } // end more data

    // flush all pending subtable data
    fm.lasttime = time;
    flushObsRecord(fm, time);
    flushFeedRecords(fm, time);
    flushMovingFields(fm, time);

    ostringstream status;
    status << "Accepted " << accepted << " input MIRIAD records.\n"
	   << "Loaded " << fm.ms->nrow() << " data records for\n"
	   << "       " << nvis << " visibilities,\n"
	   << "       " << (i = fm.ms->spectralWindow().nrow());
    status << " spectral window" << ((i == 1) ? ",\n" : "s,\n");
    status << "       " << (i = fm.ms->polarization().nrow());
    status << " polarization setup";
    if (i > 1) status << "s";
    i = max(fm.msc->polarization().numCorr().getColumn());
    if (i > 1) {
        status << " (joining up to " << i 
               << " polarizations in each setup),\n";
    } else { 
        status << " (with 1 polarization per setup),\n";
    }
    status << "       " << (i = fm.ms->field().nrow());
    status << " field" << ((i == 1) ? ", and\n" : "s, and\n")
	   << "       " << fm.narrays << " array configuration";
    if (fm.narrays != 1) status << "s";
    status << ".";
    log_p << LogIO::NORMAL << String(status.str()) << LogIO::POST;
    addHistoryMessage(fm, "NORMAL", "fill", status.str());
        
    if (Debug(3)) cout << "leaving fill()" << endl;
}

MeasurementSet *MirFiller::setupMS(const String& msfile, Int tileSize,
				   Bool needSrcModel) 
{
    // open and setup the output MS.  This includes all initialization that 
    // can be done without reading any data from the input Miriad dataset.

    if (Debug(3)) cout << FILLERNAME << "::setupMS()" << endl;
    if (Debug(1)) cout << "*** Attempting to create " << msfile << " ***" 
		       << endl;

    Bool useTSM = (tileSize > 0);

    // set up the MeasurementSet
    TableDesc td = MS::requiredTableDesc();
    MS::addColumnToDesc(td, MS::DATA, 2);
    if (compress_p) MS::addColumnCompression(td, MS::DATA);

    if (useTSM) {
	td.defineHypercolumn("TiledData",3,
			     stringToVector(MS::columnName(MS::DATA)));
	td.defineHypercolumn("TiledFlag",3,
			     stringToVector(MS::columnName(MS::FLAG)));
	td.defineHypercolumn("TiledFlagCategory",4,
			     stringToVector(MS::columnName(MS::FLAG_CATEGORY)));
	td.defineHypercolumn("TiledUVW",2,
			     stringToVector(MS::columnName(MS::UVW)));
	td.defineHypercolumn("TiledWgt",2,
			     stringToVector(MS::columnName(MS::WEIGHT)));
	td.defineHypercolumn("TiledSigma",2,
			     stringToVector(MS::columnName(MS::SIGMA)));
    }

    SetupNewTable newtab(msfile, td, Table::NewNoReplace);
  
    // Set the default Storage Manager to be the Incr one
    IncrementalStMan incrStMan ("ISMData");;
    newtab.bindAll(incrStMan, True);
    StandardStMan aipsStMan;
    newtab.bindColumn(MS::columnName(MS::ANTENNA1), aipsStMan);
    newtab.bindColumn(MS::columnName(MS::ANTENNA2), aipsStMan);
    newtab.bindColumn(MS::columnName(MS::DATA_DESC_ID), aipsStMan);

    if (useTSM) {

	// Int tileSize=nChan/10+1;
	// make the tile about 128k big
	TiledShapeStMan tiledStMan1("TiledData",
				    IPosition(3,1,tileSize,
					      16384/tileSize));
	TiledShapeStMan tiledStMan1f("TiledFlag",
				     IPosition(3,1,tileSize,
					       16384/tileSize));
	TiledShapeStMan tiledStMan1fc("TiledFlagCategory",
				      IPosition(4,1,tileSize,1,
						16384/tileSize));
	TiledColumnStMan tiledStMan3("TiledUVW",
				     IPosition(2,3,1024));
	TiledShapeStMan tiledStMan4("TiledWgt",
				    IPosition(2,1,1024));
	TiledShapeStMan tiledStMan5("TiledSigma",
				    IPosition(2,1,1024));

	// Bind the DATA, FLAG columns to the tiled stman
	newtab.bindColumn(MS::columnName(MS::DATA),tiledStMan1);
	newtab.bindColumn(MS::columnName(MS::FLAG),tiledStMan1f);
	newtab.bindColumn(MS::columnName(MS::FLAG_CATEGORY),tiledStMan1fc);
	newtab.bindColumn(MS::columnName(MS::UVW),tiledStMan3);
	newtab.bindColumn(MS::columnName(MS::WEIGHT),tiledStMan4);
	newtab.bindColumn(MS::columnName(MS::SIGMA),tiledStMan5);
    } else {
	newtab.bindColumn(MS::columnName(MS::DATA),aipsStMan);
	newtab.bindColumn(MS::columnName(MS::FLAG),aipsStMan);
	newtab.bindColumn(MS::columnName(MS::UVW),aipsStMan);
	newtab.bindColumn(MS::columnName(MS::WEIGHT),aipsStMan);
	newtab.bindColumn(MS::columnName(MS::SIGMA),aipsStMan);
    }
  
    // avoid lock overheads by locking the table permanently
    TableLock lock(TableLock::PermanentLocking);
    MeasurementSet *ms = new MeasurementSet(newtab,lock);

    try {
	// create all required subtables (each will have 0 rows)
	ms->createDefaultSubtables(Table::New);
 
	// add the SOURCE table
	TableDesc tdesc = MSSource::requiredTableDesc();
	MSSource::addColumnToDesc(tdesc, MSSourceEnums::REST_FREQUENCY, 1);
	if (needSrcModel) 
	    MSSource::addColumnToDesc(tdesc, MSSourceEnums::SOURCE_MODEL, 1);
	SetupNewTable sourceSetup(ms->sourceTableName(),tdesc,Table::New);
	ms->rwKeywordSet().defineTable(MS::keywordName(MS::SOURCE),
				       Table(sourceSetup));

	// add the DOPPLER table
	SetupNewTable dopplerSetup(ms->dopplerTableName(),
				   MSDoppler::requiredTableDesc(),Table::New);
	ms->rwKeywordSet().defineTable(MS::keywordName(MS::DOPPLER),
				       Table(dopplerSetup));

	// and the SYSCAL table
	TableDesc syscalDesc = MSSysCal::requiredTableDesc();
	MSSysCal::addColumnToDesc(syscalDesc,MSSysCalEnums::TSYS);
	MSSysCal::addColumnToDesc(syscalDesc,MSSysCalEnums::TSYS_FLAG,1);
	ScalarColumnDesc<Float> jyperk("BIMA_JYPERK", 
				       "Estimated antenna efficiency");
	syscalDesc.addColumn(jyperk);
	SetupNewTable syscalSetup(ms->sysCalTableName(),syscalDesc,Table::New);
	syscalSetup.bindColumn("BIMA_JYPERK", incrStMan);
	ms->rwKeywordSet().defineTable(MS::keywordName(MS::SYSCAL),
					  Table(syscalSetup));

#if 0
	// and the WEATHER table
	SetupNewTable weatherSetup(ms->weatherTableName(),
				   MSWeather::requiredTableDesc(),Table::New);
	ms->rwKeywordSet().defineTable(MS::keywordName(MS::WEATHER),
				       Table(weatherSetup));
#endif

	// add optional columns to SPECTRAL_WINDOW table
        ms->spectralWindow().addColumn(
            ArrayColumnDesc<Int>(
                MSSpectralWindow::columnName(MSSpectralWindow::ASSOC_SPW_ID),
                MSSpectralWindow::columnStandardComment(
                    MSSpectralWindow::ASSOC_SPW_ID)));
        ms->spectralWindow().addColumn(
            ArrayColumnDesc<String>(
                MSSpectralWindow::columnName(MSSpectralWindow::ASSOC_NATURE),
                MSSpectralWindow::columnStandardComment(
                    MSSpectralWindow::ASSOC_NATURE)));
        ms->spectralWindow().addColumn(
            ScalarColumnDesc<Int>(
                MSSpectralWindow::columnName(MSSpectralWindow::DOPPLER_ID),
                MSSpectralWindow::columnStandardComment(
                    MSSpectralWindow::DOPPLER_ID)));

//	MSSpectralWindow::addColumnToDesc(ms->spectralWindow().tableDesc(), 
//                                          MSSpectralWindow::ASSOC_SPW_ID, 1);
//	MSSpectralWindow::addColumnToDesc(ms->spectralWindow().tableDesc(), 
//                                          MSSpectralWindow::ASSOC_NATURE, 1);
    
	// update the references to the subtable keywords
	ms->initRefs();

	TableInfo& info(ms->tableInfo());
	info.setType(TableInfo::type(TableInfo::MEASUREMENTSET));
	info.setSubType(String("MIRIAD/BIMA"));
	info.readmeAddLine
	    (String("This is a measurement set Table holding observations ") +
	     "from the BIMA Array");
	info.readmeAddLine(String("Made with ") + FILLERNAME);
    }
    catch (AipsError) { delete ms; throw; }

    if (Debug(3)) cout << "leaving setMS()" << endl;
    return ms;
} 

#if 0
GlishRecord MirFiller::summary(Bool verbose, Bool scan) {
    if (scan) 
	return rdr_p.summary(scanlim_p, obslim_p, verbose);
    return rdr_p.summary(verbose, False);
}
#endif

void MirFiller::initMS(FillMetadata &fm) {
    // initialize the output MS.  This is called after the first input record 
    // is read in from the input Miriad dataset.  It will initialize certain
    // subtables (currently, POINTING, PROCESSOR, HISTORY, FEED, and 
    // POLARIZATION) based on static or otherwise non-tracked variable 
    // information.

    if (Debug(3)) cout << FILLERNAME << "::initMS()" << endl;

    // set the Epoch reference
    fm.msc->setEpochRef(MEpoch::UTC, False);

    // initialize the HISTORY table
    if (histbl_p.length() > 0) {
        copyHistoryFrom(fm, histbl_p);
    }
    else {
        flushHistory(fm);
    }

    // initialize the STATE table
    initState(fm);

    if (Debug(3)) cout << "leaving initMS()" << endl;
}

void MirFiller::selectSpectra(const Vector<Int>& wideChans, 
			      const Vector<Int>& narrowWins) 
//    throw(UnavailableMiriadDataSelectionError)
{
    // select data from the input Miriad dataset.  wideChans is a list of 
    // one-based channel indices.  narrowWins is a list of zero-based
    // window indices.  The default is to choose all available wide channels
    // and windows.
    // GlishRecord sum = rdr_p.summary(False, False);
    Int nwide, nspect;
    //((GlishArray) sum.get("nwide")).get(nwide);
    //((GlishArray) sum.get("nspect")).get(nspect);
    nwide = 16;   // PJT TODO
    nspect = 16;

    Vector<Bool> widesel(nwide, False), winsel(nspect, False);
    Int i, nsel, sel, nwidesel=0, nwinsel=0;

    nsel = narrowWins.nelements();
    for(i=0; i < nsel; i++) {
	sel = narrowWins(i);
	if (sel > nspect) winsel.resize(sel);
	if (sel > 0) {
	    if (! winsel(sel-1)) {
		winsel(sel-1) = true;
		nwinsel++;
	    }
	}
	else 
	    throw UnavailableMiriadDataSelectionError(
                    String("spectral line window selection out of range"), sel);
    }

    nsel = wideChans.nelements();
    for(i=0; i < nsel; i++) {
	sel = wideChans(i);
	if (sel > 0 && sel <= nwide) {
	    if (! widesel(sel-1)) {
		widesel(sel-1) = true;
		nwidesel++;
	    }
	}
	else 
	    throw UnavailableMiriadDataSelectionError(
		    String("wideband channel selection out of range"), sel);
    }

    if (nwinsel == 0 && nwidesel == 0) 
	throw UnavailableMiriadDataSelectionError("No data selected");

    winsel_p = winsel;
    nwinsel_p = nwinsel;
    widesel_p = widesel;
    nwidesel_p = nwidesel;
}

void MirFiller::selectPols(ConstMirPolSetup &polsel) {
    if (defpolsel_p != NULL) { 
	delete defpolsel_p;
	defpolsel_p = NULL;
    }
    if (polsel.getCorrCount() > 0) {
	defpolsel_p = new MirPolSetup(polsel);
    }
}

Record MirFiller::getOptions() {
    // get the current filling options

    RecordDesc optrec;
    optrec.addField("tilesize", TpInt);
    optrec.addField("scanlim", TpDouble);
    optrec.addField("obslim", TpDouble);
    optrec.addField("wideconv", TpString);
    optrec.addField("joinpol", TpBool);
    optrec.addField("tsyswt", TpBool);
    optrec.addField("planetfit", TpInt);
    optrec.addField("movfield", TpBool);
	optrec.addField("updmodelint", TpDouble);
//    optrec.addField("histbl", TpString);
//    optrec.addField("nosplit", TpBool);
    optrec.addField("verbose", TpBool);

    Record out(optrec);
    RecordFieldPtr<Int>(out, "tilesize").define(tilesize_p);
    RecordFieldPtr<Double>(out, "scanlim").define(scanlim_p);
    RecordFieldPtr<Double>(out, "obslim").define(obslim_p);
//    RecordFieldPtr<String>(out, "histbl").define(histbl_p);
//    RecordFieldPtr<Bool>(out, "nosplit").define(! splitWindows_p);
    RecordFieldPtr<Bool>(out, "joinpol").define(joinpol_p);
    RecordFieldPtr<Bool>(out, "tsyswt").define(tsyswt_p);
    RecordFieldPtr<Int>(out, "planetfit").define(planetfit_p);
	RecordFieldPtr<Double>(out, "updmodelint").define(updmodelint_p);
    RecordFieldPtr<Bool>(out, "movfield").define(movfield_p);
    RecordFieldPtr<Bool>(out, "verbose").define(verbose_p);
    RecordFieldPtr<String>(out, "wideconv").define(wideconvnames[wideconv_p]);

    return out;
}

void MirFiller::setOptions(const Record &opts) {
    // set the options for filling.  The options are set via a Record object
    // for which the following keys are recognized:

    const RecordDesc &desc = opts.description();
    Int fn;

    if ((fn = desc.fieldNumber("tilesize")) >= 0) {
        RORecordFieldPtr<Int> rfp(opts, fn);
        tilesize_p = *rfp;
    }
    if ((fn = desc.fieldNumber("scanlim")) >= 0) {
        RORecordFieldPtr<Double> rfp(opts, fn);
        scanlim_p = *rfp;
    }
    if ((fn = desc.fieldNumber("obslim")) >= 0) {
        RORecordFieldPtr<Double> rfp(opts, fn);
        obslim_p = *rfp;
    }
//      if ((fn = desc.fieldNumber("nosplit")) >= 0) {
//          RORecordFieldPtr<Bool> rfp(opts, fn);
//          splitWindows_p = ! *rfp;
//      }
    if ((fn = desc.fieldNumber("histbl")) >= 0) {
        RORecordFieldPtr<String> rfp(opts, fn);
        histbl_p = *rfp;
    }
    if ((fn = desc.fieldNumber("joinpol")) >= 0) {
        RORecordFieldPtr<Bool> rfp(opts, fn);
        joinpol_p = *rfp;
    }
    if ((fn = desc.fieldNumber("tsyswt")) >= 0) {
        RORecordFieldPtr<Bool> rfp(opts, fn);
        tsyswt_p = *rfp;
    }
    if ((fn = desc.fieldNumber("planetfit")) >= 0) {
        RORecordFieldPtr<Int> rfp(opts, fn);
        planetfit_p = *rfp;
    }
	if ((fn = desc.fieldNumber("updmodelint")) >= 0) {
		RORecordFieldPtr<Double> rfp(opts, fn);
		updmodelint_p = *rfp;
	}
    if ((fn = desc.fieldNumber("movfield")) >= 0) {
        RORecordFieldPtr<Bool> rfp(opts, fn);
        movfield_p = *rfp;
    }
    if ((fn = desc.fieldNumber("compress")) >= 0) {
        RORecordFieldPtr<Bool> rfp(opts, fn);
        compress_p = *rfp;
    }
    if ((fn = desc.fieldNumber("verbose")) >= 0) {
        RORecordFieldPtr<Bool> rfp(opts, fn);
        verbose_p = *rfp;
    }
    if ((fn = desc.fieldNumber("wideconv")) >= 0) {
        RORecordFieldPtr<String> rfp(opts, fn);
        Int i;
        for(i=0; i < NCONV && *rfp != wideconvnames[i]; i++);
        if (i == NCONV) {
            log_p << LogIO::WARN << "Wide-band channel convention '"
                  << *rfp << "' not recognized; setting wideconv='none'"
                  << LogIO::POST;
            wideconv_p = NONE;
        } 
        else {
            wideconv_p = (wideConventions) i;
        }
    }

}
 
void MirFiller::flushObsRecord(FillMetadata &fm, Double time) {
    // add the current observation information as a new record to the
    // output ms's OBSERVATION subtable
    if (Debug(3)) cout << FILLERNAME << "::flushObsRecord()" << endl;

    if (fm.starttime < 0 || fm.lasttime < 0)
        throw AipsError("Programmer Error: no data apparently read yet");
    if (fm.obstime == fm.lasttime) {
	if (Debug(3)) cout << "quickly exiting flushObsRecord()" << endl;
	return;  // no new data, yet.
    }

    MSObservationColumns& obs = fm.msc->observation();
    MSObservation& obstbl = fm.ms->observation();

    uInt row = obstbl.nrow();
    obstbl.addRow();

    obs.telescopeName().put(row, fm.telescope);

    Vector<Double> range(2);
    range(0) = fm.obstime;
    range(1) = fm.lasttime;
    obs.timeRange().put(row, range);

    if (fm.telescope == "BIMA") {
	Vector<String> log(1, String("See HISTORY for BIMA observing log"));
	obs.log().put(row, log);
    }

    obs.project().put(row, fm.project);
    obs.releaseDate().put(row, fm.lasttime + 18*31*24*3600); // ~ 18 months
    if (Debug(3)) cout << "leaving flushObsRecord()" << endl;
}

void MirFiller::addAntennaPositions(FillMetadata &fm, Double time) {
    if (Debug(3)) cout << FILLERNAME << "::addAntennaPositions()" << endl;

    MSAntennaColumns& ant = fm.msc->antenna();
    MSAntenna& anttbl = fm.ms->antenna();

    // With the BIMA telescope system, the number of antennas written into
    // the antpos variable is controled by the capacity of the correlator.
    // Thus, it will tend to claim there are more antennas than really are.
    // Avoid adding those non-existent antennas to this table.
    Int nants=fm.nants;
    if (fm.telescope == fm.BIMA) nants = fm.bimaAntCount();

    // reference position is set when the telescope is set
    if(Debug(1)) cout << "adding " << nants << " antennas" << endl;
    if(Debug(4)) cout << "array ref pos: " << fm.arrayXYZ << endl;

    // Antenna diameter
    Float defdiam = 25;
    Float diameter;
    if (fm.telescope==fm.ATCA) defdiam=22;     //# only at 'low' freq !!
    if (fm.telescope==fm.BIMA) defdiam=6;

    // update the antenna offset
    Int row = ant.nrow();
    fm.arrayAnt = row;

    // add antenna info to table
    if (row < 0)                       // only on empty table
        ant.setPositionRef(MPosition::ITRF);

    Vector<Double> antXYZ(3);
    String mtype;                           // really should consult
    Int i, mounti = 0;
    Vector<Double> offsets(3);
    offsets=0.0;




    for (i=0; i < nants; i++) {
        anttbl.addRow();

        // dish diameter
        diameter = (fm.diam != NULL && fm.diam[i] > 0) ? fm.diam[i] : defdiam;
        ant.dishDiameter().put(row,diameter);   

        // antenna mounts
        if (fm.mount == NULL) {
            if (fm.telescope == fm.BIMA || fm.telescope == fm.VLA) {
                mounti = 0;   // ALT-AZ mount
            } else {
                mounti = -1;
            }
        }
	else {
	    log_p << LogIO::WARN
		  << "Miriad mount identification found but not used"
		  << LogIO::POST;
	}
        switch (mounti) {                 	    // the "mount" uv-variable
        case  0: mtype="ALT-AZ";      break;
        case  1: mtype="EQUATORIAL";  break;
        case  2: mtype="X-Y";         break;
        case  3: mtype="ORBITING";    break;
        case  4: mtype="BIZARRE";     break;
        // case  5: mtype="SPACE-HALCA"; break;
        default: mtype="UNKNOWN";     break;
        }
        ant.mount().put(row,mtype);

        ant.name().put(row,"ANT" + String::toString(i+1));
        ant.station().put(row,"UNKNOWN");  // station names unknown at HatCreek
        ant.type().put(row,"GROUND-BASED");

        // antenna positions
        antXYZ(0) = fm.antpos(i);              //# these are now in nano-sec
        antXYZ(1) = fm.antpos(i+fm.nants);
        antXYZ(2) = fm.antpos(i+fm.nants*2);
        antXYZ *= 1e-9 * C::c;;             //# and now in meters
	if (Debug(4)) cout << "Ant " << i+1 << ":" << antXYZ << " (m)." 
                           << endl;
#if 0
        ant.position().put(row,antXYZ);
#else

	if (fm.telescope==fm.BIMA) {
	  // We must rotate local positions into frame which
	  //  is parallel to earth-centered one before translating
	  Double bimaLong=atan2(fm.arrayXYZ(1),fm.arrayXYZ(0));
	  Matrix<Double> posRot=Rot3D(2,bimaLong);
	  antXYZ=product(posRot,antXYZ);
	}

	ant.position().put(row,fm.arrayXYZ+antXYZ); // (see setPositionRef() 
	                                            //  above)
#endif

        // mount-feed offsets
        ant.offset().put(row,offsets);

        ant.flagRow().put(row,False);
        row++;
    }

    if (Debug(3)) cout << "leaving addAntennaPositions()" << endl;
}

void MirFiller::addField(FillMetadata &fm, double time) {
    // add the current field to the FIELD subtable
    if (Debug(3)) cout << FILLERNAME << "::addField()" << endl;

    MSFieldColumns& msField = fm.msc->field();
    MSField& fldtbl = fm.ms->field();

    Int row = fldtbl.nrow();
    fm.field->id = row;

    MDirection::Types epochRef = MDirection::J2000;
    if (nearAbs(fm.source->epoch,1950.0,0.01)) {
	epochRef = MDirection::B1950;
    } else if (! nearAbs(fm.source->epoch,2000.0,0.01)) {
	log_p << LogIO::WARN << "Unable to support epoch=" << fm.source->epoch 
	      << " for field " << row << " (" << fm.field->dra << ", " 
	      << fm.field->ddec << ")" << LogIO::POST;
    }
    if (row == 0) msField.setDirectionRef(epochRef);
    fldtbl.addRow();

    String src(fm.source->name);
    if (src.length() == 0) 
        src = String("SOURCE") + String::toString(fm.source->id);
    msField.name().put(row, src+"-F"+String::toString(row));

//    // Need to convert epoch in years to MJD time // SHOULD BE ~OBSTIME
//      if (epochRef == MDirection::J2000) {
//  	msField.time().put(row, MeasData::MJD2000*C::day);
//  	// assume UTC epoch
//      } else if (epochRef == MDirection::B1950) {
//  	msField.time().put(row, MeasData::MJDB1950*C::day);
//      } 
    msField.time().put(row, time);   // obs time

    msField.numPoly().put(row,0);

    // RA, Dec in radians
    Double ra  = fm.source->ra  + fm.field->dra/cos(fm.source->dec);
    Double dec = fm.source->dec + fm.field->ddec;

    Vector<MDirection> dir(1, MDirection(MVDirection(ra, dec), 
                                         MDirection::Ref(epochRef)));
    msField.delayDirMeasCol().put(row,dir);
    msField.phaseDirMeasCol().put(row,dir);
    msField.referenceDirMeasCol().put(row,dir);

    msField.sourceId().put(row, fm.source->id);

    if (Debug(3)) cout << "leaving addField()" << endl;
}

void MirFiller::addCorrelatorSetup(FillMetadata &fm, double time) {
    // add subtable records for the current correlator setup
    if (Debug(3)) cout << FILLERNAME << "::addCorrelatorSetup()" << endl;

    // we need access to the SOURCE, DOPPLER, and SPECTRAL_WINDOW subtables
    MSSourceColumns& src = fm.msc->source();
    MSDopplerColumns& dop = fm.msc->doppler();
    MSDoppler& doptbl = fm.ms->doppler();
    MSSpWindowColumns& msSpW = fm.msc->spectralWindow();
    MSSpectralWindow& spwtbl = fm.ms->spectralWindow();

    String emptyName;
    Int i,j,assoc,did,tid;
    Int row = spwtbl.nrow();
//    Int row = idoffset;
    Double f, df;
    MirFreqSetup *fs = fm.fsetup;
    fs->id = row;

    // set the id for this setup.  This ID does not yet correspond to
    // any ID in the MS; however, it will eventually correspond to 
    // a specific correlator mode record to be used to store the 
    // specific correlator parameters.  Regardless, the id is used
    // as a flag to indicate whether this setup has been flushed 
    // to disk yet, where id<0 means "not yet".
//      if (fs->id < 0) {
//  	fs->id = 0;
//      } else {
//  	fs->id++;
//      }

    // first predict the output spectral window IDs (needed, for among
    // other reasons, to get associations right)
#if 1
    IDIndex nfidx(fs->id);
    fs->nfidx = nfidx;
#else
    fs->nfidx = IDIndex(fs->id);                        // PJT mismatch type
#endif
    j = min(fs->nspect, (int)winsel_p.nelements());
    for(i=0; i < j; i++) {
        if (winsel_p(i)) fs->nfidx.add(i);
    }
#if 1
    IDIndex wfidx(fs->id);
    fs->wfidx = wfidx;
#else    
    fs->wfidx = IDIndex(fs->nfidx.next());              // PJT mismatch type
#endif
    j = min(fs->nspect+2, (int)widesel_p.nelements());
    for(i=0; i < j; i++) {
        if (widesel_p(i)) fs->wfidx.add(i);
    }

    // Setup the conversion of topocentric frequencies to LSR.  This a 
    // requires a frame that specifies the epoch, observatory position, and 
    // direction of the source.  Note that if the epoch is not B1950,
    // J2000 is assumed.
    MDirection::Types dirtype = 
  	(nearAbs(fm.source->epoch,1950.0,0.01)) ? MDirection::B1950 
 	                                        : MDirection::J2000;
    MEpoch ep(Quantity(time, "s"), MEpoch::UTC);
    MPosition obspos(MVPosition(fm.arrayXYZ), MPosition::ITRF);
    MDirection dir(Quantity(fm.source->ra, "rad"), 
		   Quantity(fm.source->dec, "rad"), 
		   dirtype);
    MeasFrame frame(ep, obspos, dir);
    MFrequency::Convert tolsr(MFrequency::TOPO, 
			      MFrequency::Ref(MFrequency::LSRD, frame));

    // get the rest frequencies already stored for the current source
    Int nlines = (src.numLines())(fm.source->row);
    Vector<Double> rfs(nlines);
    if (nlines > 0) {
        src.restFrequency().get(fm.source->row, rfs, True);
    } 
    if (fs->nspect > 0) rfs.resize(nlines+fs->nspect, True);

    // handle narrow band channels
    for(i=0; i < fs->nspect; i++) {
	if (! winsel_p(i)) continue;

	Vector<Double> chanFreq(fs->nschan[i]),
            resolution(fs->nschan[i], abs(fs->sdf[i])*1e9);   // in Hz

	// convert the topocentric frequencies to LSR before loading
	f = tolsr(fs->sfreq[i]*1e9).getValue().getValue();
        df = tolsr(fs->sdf[i]*1e9).getValue().getValue();
	if (Debug(4) || (Debug(3) && i==0)) {
	    cout << "apparent observatory velocity for spwid=" << i
		 << ": " << (1 - f/(fs->sfreq[i]*1e9))*2.9979e5
		 << " km/s" << endl;
	}
        for(j=0; j < fs->nschan[i]; j++) {
            chanFreq(j) = f;
            f += df;
        }            

	spwtbl.addRow();

	msSpW.numChan().put(row,fs->nschan[i]);
	msSpW.name().put(row,emptyName);
	msSpW.refFrequency().put(row, fs->freq*1e9);            // ???
        msSpW.chanFreq().put(row, chanFreq);
	msSpW.chanWidth().put(row, resolution);
        msSpW.measFreqRef().put(row, MFrequency::LSRD);
	msSpW.effectiveBW().put(row, resolution);
	msSpW.resolution().put(row, resolution);
	msSpW.totalBandwidth().put(row, abs(fs->nschan[i]*fs->sdf[i]*1e9));

//	if (fm.telescope == fm.BIMA) {
	if (wideconv_p == BIMA || wideconv_p == MIRIAD) {

            // lower sideband data have negative widths;
            // (i < fs->mode) would also be an effective test (for BIMA)
	    msSpW.netSideband().put(row, (fs->sdf[i] < 0) ? -1 : 1);
        } else {
	    msSpW.netSideband().put(row, 0);
        }

        // associations
        if (wideconv_p == BIMA) {
	    assoc = 0;
	    j = i+fs->mode;     // data from other sideband
	    if (j >= fs->nspect) j -= fs->nspect;
	    if (fs->mode > 0 && j < (int)winsel_p.nelements() && winsel_p(j)) 
                assoc++;
	    if (i+2 < (int)widesel_p.nelements() && widesel_p(i+2)) assoc++;
            Vector<String> nature(assoc);
            Vector<Int> id(assoc);
	    if (assoc > 0) {
		assoc = 0;
		if (fs->mode > 0 && 
                    j < (int)winsel_p.nelements() && winsel_p(j)) 
                {
		    nature(assoc) = "OTHER-SIDEBAND";
		    id(assoc) = fm.fsetup->nfidx(j);
		    assoc++;
		}
		if (i+2 < (int)widesel_p.nelements() && widesel_p(i+2)) { 
		    nature(assoc) = "CHANNEL-ZERO";
		    id(assoc) = fm.fsetup->wfidx(i+2);
		}
	    }
            msSpW.assocSpwId().put(row, id);
            msSpW.assocNature().put(row, nature);
	}
	else {
            Vector<String> nature(0);
            Vector<Int> id(0);
            msSpW.assocSpwId().put(row, id);
            msSpW.assocNature().put(row, nature);
	}

	msSpW.freqGroup().put(row,1);
	msSpW.freqGroupName().put(row,"MULTI-CHANNEL-DATA");

        // get a transition id for the rest frequency
        f = fs->rfreq[i] * 1e9;
        for(tid=0; tid < nlines && rfs(tid)!=f; tid++);
        if (tid == nlines) { 
            rfs(nlines++) = f;
	    if (verbose()) log_p << LogIO::NORMAL << "Adding rest freq=" 
				 << f << " Hz" << LogIO::POST;
        }

        // now add a record to the doppler table
        did = doptbl.nrow();
        doptbl.addRow();
        dop.dopplerId().put(did, did);
        dop.sourceId().put(did, fm.source->id);
        dop.transitionId().put(did, tid);
        dop.velDefMeas().put(did, MDoppler(Quantity(0),MDoppler::RADIO));

        // link the SPECTRAL_WINDOW record to the DOPPLER record
        msSpW.dopplerId().put(row, did);
	row++;
    }

    // update the transitions for the current source
    rfs.resize(nlines, True);
    src.numLines().put(fm.source->row, nlines);
    src.restFrequency().put(fm.source->row, rfs);

    // handle sideband and window averages
    for(i=0; i < fs->nwide; i++) {
	if (! widesel_p(i)) continue;

	// convert the topocentric frequencies to LSR before loading
        Vector<Double> fq(1, tolsr(fs->wfreq[i]*1e9).getValue().getValue());
        Vector<Double> wd(1, fs->wwidth[i]*1e9);

	spwtbl.addRow();

	msSpW.numChan().put(row, 1);
	msSpW.name().put(row, emptyName);
	msSpW.refFrequency().put(row,fs->freq*1e9);            // ???
	msSpW.chanFreq().put(row,fq);            // ???
	msSpW.chanWidth().put(row, wd);
        msSpW.measFreqRef().put(row, MFrequency::LSRD);
	msSpW.effectiveBW().put(row, wd);
	msSpW.resolution().put(row, wd);
	msSpW.totalBandwidth().put(row, fs->wwidth[i]*1e9);

//	if (fm.telescope == fm.BIMA) {
	if (i < 2 && (wideconv_p == BIMA || wideconv_p == MIRIAD)) {

            // sideband averages
            msSpW.netSideband().put(row, (i == 0) ? -1 : 1);
            msSpW.freqGroup().put(row,2);
            msSpW.freqGroupName().put(row,"SIDEBAND-AVG");

            // associations
            if ((int)widesel_p.nelements() > 1 && widesel_p(1-i)) {
                Vector<String> nature(1);
                Vector<Int> id(1);
                nature(0) = "OTHER-SIDEBAND";
                id(0) = fm.fsetup->wfidx(1-i);
                msSpW.assocSpwId().put(row, id);
                msSpW.assocNature().put(row, nature);
            }
            else {
                Vector<String> nature(0);
                Vector<Int> id(0);
                msSpW.assocSpwId().put(row, id);
                msSpW.assocNature().put(row, nature);
            }
        }
        else if (i >= 2 && wideconv_p == BIMA) {

            // window averages
            msSpW.netSideband().put(row, (i-2 < fs->mode) ? -1 : 1);
            msSpW.freqGroup().put(row,3);
            msSpW.freqGroupName().put(row,"MULTI-CHANNEL-AVG");

		// associations
            assoc = 0;
            j = i+fs->mode;   // average from other sideband
            if (j >= fs->nspect+2) j -= fs->nspect;
            if (fs->mode > 0 && 
                j < (int)widesel_p.nelements() && widesel_p(j)) assoc++;
            if (i-2 < (int)winsel_p.nelements() && winsel_p(i-2)) assoc++;
            Vector<String> nature(assoc);
            Vector<Int> id(assoc);
            if (assoc > 0) {
                assoc = 0;
                if (fs->mode > 0 && 
                    j < (int)widesel_p.nelements() && widesel_p(j)) 
                {
                    nature(assoc) = "OTHER-SIDEBAND";
                    id(assoc) = fm.fsetup->wfidx(j);
                    assoc++;
                }
                if (i-2 < (int)winsel_p.nelements() && winsel_p(i-2)) {
                    nature(assoc) = "SUB-BAND";
                    id(assoc) = fm.fsetup->nfidx(i-2);
                }
            }
            msSpW.assocSpwId().put(row, id);
            msSpW.assocNature().put(row, nature);
        }
        else {
	    msSpW.netSideband().put(row, 0);
	    msSpW.freqGroup().put(row,4);
	    msSpW.freqGroupName().put(row,"SINGLE-CHANNEL-DATA");

            Vector<String> nature(0);
            Vector<Int> id(0);
            msSpW.assocSpwId().put(row, id);
            msSpW.assocNature().put(row, nature);
	}

        // add "null" doppler id
        msSpW.dopplerId().put(row, -1);
	row++;
    }

    if (Debug(3)) cout << "leaving addCorrelatorSetup()" << endl;
}

uInt MirFiller::addSource(FillMetadata &fm, double time) {
    // add the current source to the SOURCE subtable
    if (Debug(3)) cout << FILLERNAME << "::addSource()" << endl;

    MSSourceColumns& src = fm.msc->source();
    MSSource& srctbl = fm.ms->source();

    // determine the new source id.  row = first row the source appears in.
    uInt row = srctbl.nrow();
    if (fm.source->row < 0) fm.source->row = row;
    if (fm.source->id < 0) fm.source->id = MirSource::nextID();
    srctbl.addRow();

    // set a default name if necessary
    String name = fm.source->name;
    if (name.length() == 0) {
        name = String::toString(row);
        name.prepend("SOURCE_");
    }

    // set the direction
    Vector<Double> dir(2);
    dir(0) = fm.source->ra;
    dir(1) = fm.source->dec;

/*
    if (fm.source->isSolSysObj()) {
      srcpos = MDirection(MVDirection(0.0, 0.0), 
	 static_cast<MDirection::Types>(fm.source->solsys+MDirection::MERCURY));
    }
*/

    src.sourceId().put(row, fm.source->id);
    src.time().put(row, time);
    src.interval().put(row, -1);
    src.spectralWindowId().put(row, -1);
    src.name().put(row, name);
    src.code().put(row, "UNKNOWN");
    src.direction().put(row, dir);
    src.properMotion().put(row, Vector<Double>(2, 0.0));

    // now handle transitions.  
    if (fm.source->row < static_cast<Int>(row)) {
        // this source name has already been written once to this table;
        // copy over the transition information
        Int nlines = (src.numLines())(fm.source->row);
        Vector<Double> rfs(nlines);
        src.restFrequency().get(fm.source->row, rfs, True);
        src.numLines().put(row, nlines);
        src.restFrequency().put(row, rfs);
    }
    else {
	// addCorrelatorSetup() will add in the rest frequencies later.  
        src.numLines().put(row, 0);
        src.restFrequency().put(row, Vector<Double>(0));
    }

    if (Debug(3)) cout << "leaving addSource()" << endl;
    return row;
}

void MirFiller::flushFeedRecords(FillMetadata& fm, Double time) {
    // add the records to the FEED subtable for the current set of antennas
    // and polarizations.
    if (Debug(3)) cout << FILLERNAME << "::flushFeedRecords()" << endl;
    Int i;

    if (fm.arrayAnt < 0 || fm.ms->nrow() == 0) {
	if (Debug(3)) cout << "quickly exiting flushFeedRecords()" << endl;
	return;  // no new data, yet.
    }
    Double intv = fm.lasttime - fm.feedtime + fm.inttime;
    Double timecent = fm.feedtime + intv/2;

    if (fm.polrecp == NULL) {
	log_p << LogIO::WARN
	      << "Reciever polarization types unknown; assuming Y."
	      << LogIO::POST;
	fm.polrecp = new MirPolRecps();
	fm.polrecp->addPol(Stokes::YY);
    }

    // With the BIMA telescope system, the number of antennas written into
    // the antpos variable is controled by the capacity of the correlator.
    // Thus, it will tend to claim there are more antennas than really are.
    // Avoid adding those non-existent antennas to this table.
    Int nants=fm.nants;
    if (fm.telescope == fm.BIMA) nants = fm.bimaAntCount();

    // get polarization types
    Int nrecp = fm.polrecp->getPolCount();
    Vector<String> types(nrecp);
    for(i=0; i < Int(types.nelements()); i++) {
	types(i) = fm.polrecp->toString(i);
    }

    Matrix<Double> boff(2, nrecp, 0);
    Matrix<Complex> resp(nrecp, nrecp, Complex(0.0, 0.0));
    resp.diagonal() = Complex(1.0, 0.0);
    MPosition pos;
    Vector<Double> recang(nrecp, 0.0);

    MSFeed& feedtbl = fm.ms->feed();
    MSFeedColumns& msfc = fm.msc->feed();
    Int row = feedtbl.nrow();
    for(i=1; i <= nants; i++) {
	feedtbl.addRow();
	msfc.antennaId().put(row, fm.getAntennaID(i));
	msfc.feedId().put(row, 0);
	msfc.spectralWindowId().put(row, -1);
	msfc.time().put(row, timecent);
	msfc.interval().put(row, intv);
	msfc.numReceptors().put(row, nrecp);
	msfc.beamId().put(row, -1);
	msfc.beamOffset().put(row, boff);
	msfc.polarizationType().put(row, types);
	msfc.polResponse().put(row, resp);
	msfc.positionMeas().put(row, pos);
	msfc.receptorAngle().put(row, recang);
	row++;
    }
    fm.feedtime = time;

    if (Debug(3)) cout << "leaving flushFeedRecords()" << endl;
}

void MirFiller::addPolarization(FillMetadata& fm, Bool addCurrentOnly) {
    // add a POLARIZATION record for the current polarization type
    if (Debug(3)) cout << FILLERNAME << "::addPolarization()" << endl;
    if (fm.pol == NULL) {
	if (Debug(1)) cout << "premature exit from addPolarization()" << endl;
	return;
    }

    MSPolarization &poltbl = fm.ms->polarization();
    MSPolarizationColumns &pc = fm.msc->polarization();

    Int ncorr=1, nrec=1;
    if (joinpol_p) {
	ncorr = fm.pol->getCorrCount();
	nrec = 1;
    } else {
	ncorr = 1;
	nrec = (addCurrentOnly) ? 1 : fm.pol->getCorrCount();
    }
    
    Vector<Int> type(ncorr);
    Matrix<Int> prod(2,ncorr);

    Int i;
    if (joinpol_p || addCurrentOnly) {

	// only one entry will be entered in the polarization table
	if (joinpol_p) {

	    // all polarization cross-correllations will go into a single 
	    // polarization setup record; this is appropriate for telescopes
	    // that observe multiple polarizations simultaneously, and for 
	    // data that approximate this through time averaging.
	    ConstListIter<MirPolCorr> li(fm.pol->getCorrs());
	    for(i=0, li.toStart(); ! li.atEnd(); ++li, ++i) {
		type(i) = li.getRight().getAipsType();
		prod(0,i) = fm.polrecp->find(li.getRight().getFirstPolType());
		prod(1,i) = fm.polrecp->find(li.getRight().getSecondPolType());
		li.getRight().setID(poltbl.nrow());
	    }
	} 
	else {

	    // A single polarization correlation will be recorded as a 
	    // seperate polarization setup.  This is appropriate for 
	    // telescopes that can only observe one polarization at a time.  
	    type(0) = fm.pol->getCurrentAipsType();
	    prod(0,0) = 
                fm.polrecp->find(fm.pol->getCurrent().getFirstPolType()); 
	    prod(1,0) = 
                fm.polrecp->find(fm.pol->getCurrent().getSecondPolType());

	    fm.pol->getCurrent().setID(poltbl.nrow());
	}

	Int row = poltbl.nrow();
	poltbl.addRow();

	pc.numCorr().put(row, ncorr);
	pc.corrType().put(row, type);
	pc.corrProduct().put(row, prod);
	pc.flagRow().put(row, False);
    }
    else {

	// Each polarization correlation will be recorded as a seperate 
	// polarization setup.  This is appropriate for telescopes that 
	// can only observe one polarization at a time.  
	Int row, i;
        ConstListIter<MirPolCorr> li(fm.pol->getCorrs());
        for(i=0, li.toStart(); ! li.atEnd(); ++li, ++i) {
	    type(0) = li.getRight().getAipsType();
	    prod(0,0) = 0;
	    prod(1,0) = 0;

	    row = poltbl.nrow();
	    poltbl.addRow();

	    pc.numCorr().put(row, 1);
	    pc.corrType().put(row, type);
	    pc.corrProduct().put(row, prod);
	    pc.flagRow().put(row, False);
            li.getRight().setID(i);
	}
    }

    if (Debug(3)) cout << "leaving addPolarization()" << endl;
}

void MirFiller::addDataDesc(FillMetadata &fm) {
    // add DATA_DESCRIPTION records for the current polarization
    // and frequency setup
    if (Debug(3)) cout << FILLERNAME << "::addDataDesc()" << endl;

    Int i, j, lim;

    if (fm.fsetup == NULL) {
	if (Debug(3)) cout << "premature exiting of addDataDesc()" << endl;
	return;
    }
    MSDataDescription &ddtbl = fm.ms->dataDescription();
    MSDataDescColumns &dd = fm.msc->dataDescription();

    Int row = ddtbl.nrow();
    lim = fm.fsetup->wfidx.next();
    i = (fm.pol != NULL) ? fm.pol->getCurrent().getID() : 0;
    for(j=fm.fsetup->nfidx.first(); j < lim; ++j) {
	ddtbl.addRow();
	dd.spectralWindowId().put(row, j);
	dd.polarizationId().put(row, i);  
	dd.flagRow().put(row, False);
#if 0
	// PJT 2011, no more ddid
        DataDescComponents ddid(j, i);
        if (fm.ddids.isDefined(ddid)) fm.ddids.remove(ddid);
	fm.ddids.define(ddid, row);
#endif
	row++;
    }

    if (Debug(3)) cout << "leaving addDataDesc()" << endl;
}

void MirFiller::initState(FillMetadata &fm) {
    // initialize the STATE table
    // 
    MSState &tbl = fm.ms->state();
    MSStateColumns &st = fm.msc->state();
    Int row = tbl.nrow();
    tbl.addRow();
    st.sig().put(row, False);
    st.ref().put(row, False);
    st.cal().put(row, 0);
    st.load().put(row, 0);
    st.subScan().put(row, 0);
    st.obsMode().put(row, "UNKNOWN");
    st.flagRow().put(row, False);
}

void MirFiller::copyHistoryFrom(FillMetadata &fm, String tablename) {
    // copy the history from a given HISTORY table
    // 
    // currently not supported
    throw AipsError("Copying History from external table not yet supported");
}

void MirFiller::flushHistory(FillMetadata &fm) {
    // write out the miriad history into the HISTORY table
    // 
    // this implementation assumes that it cannot parse the input 
    // history records.
    String line;
    MSHistory& tbl = fm.ms->history();
    MSHistoryColumns& his = fm.msc->history();
    Int row = tbl.nrow();
    MirHisReader *mirhis = rdr_p.openHistory();

    while (mirhis->readLine(line)) {
	tbl.addRow(); 
	his.observationId().put(row,-1);
	his.time().put(row, fm.starttime);
	his.priority().put(row,"NORMAL");
	his.origin().put(row,"unknown");
	his.application().put(row,FILLERNAME);
	his.message().put(row,line);
	row++;
    }
    delete mirhis;

    Time date;
    MEpoch now(MVEpoch(date.modifiedJulianDay()), MEpoch::Ref(MEpoch::UTC));

    tbl.addRow();
    his.observationId().put(row,-1);
    his.timeMeas().put(row, now);
    his.priority().put(row,"NORMAL");
    his.origin().put(row, String(FILLERNAME)+"::flushHistory");
    his.application().put(row,FILLERNAME);
    his.message().put(row,
		      String("Converted from miriad format into AIPS++ MS2"));
}

void MirFiller::addProcessor(FillMetadata &fm) {
    // add a processor record for the current frequency setup
    //
    // this implementation currently does not write out correllator mode
    // data
    MSProcessor& tbl = fm.ms->processor();
    MSProcessorColumns& proc = fm.msc->processor();
    Int row = tbl.nrow();

    tbl.addRow();
    if (fm.telescope == fm.BIMA || fm.telescope == fm.VLA) {
	proc.type().put(row, "CORRELATOR");
    } else {
	proc.type().put(row, "UNKNOWN");
    }
    proc.subType().put(row, fm.telescope);
    proc.flagRow().put(row, False);
}

void MirFiller::flushPointingRecords(FillMetadata &fm, Double time) {
    // add the records to the POINTING subtable for the current set of antennas
    // 
    // no records are filled out at this time
}

void MirFiller::addHistoryMessage(FillMetadata &fm, String priority, 
				  String origin, String msg) 
{
    // add a filler message to the HISTORY table

    MSHistory& tbl = fm.ms->history();
    MSHistoryColumns& his = fm.msc->history();
    Int row = tbl.nrow();

//    MEpoch now(MVEpoch(Quantity(Time().modifiedJulianDay(), "d")),
//               MEpoch::Ref(MEpoch::UTC));
    Time date;
    MEpoch now(MVEpoch(date.modifiedJulianDay()), MEpoch::Ref(MEpoch::UTC));

    tbl.addRow();
    his.observationId().put(row,-1);
    his.timeMeas().put(row, now);
    his.priority().put(row,priority);
    his.origin().put(row, String(FILLERNAME)+"::"+origin);
    his.application().put(row,FILLERNAME);
    his.message().put(row, msg);
}

void MirFiller::updateIntTime(FillMetadata &fm, Double time) { 
    // no special processing necessary
}

void MirFiller::updateObservatory(FillMetadata &fm, Double time) {

    // flush the Feed records for the current set of antennae (which
    // should get updated next upon return to the reader).  We need to
    // do this now because the polarization list will get updated.
    if (fm.ms != NULL && fm.antpos.nelements() > 0) flushFeedRecords(fm, time);

    // reset the collection of polarizations
    if (fm.polrecp != NULL) {
	delete fm.polrecp;
	fm.polrecp = NULL;
    }
    fm.polrecp = new MirPolRecps();
//    if (fm.telescope == fm.BIMA) {  
//        // pre-add the known BIMA polarizations so that they always
//	// appear in the same order in the FEED table (for user convenience).
//	fm.polrecp->addPol(Stokes::XX);
//	fm.polrecp->addPol(Stokes::RR);
//	fm.polrecp->addPol(Stokes::LL);
//    }

    // add a new processor record (describing the backend) 
    if (fm.ms != NULL) addProcessor(fm);
}

void MirFiller::updateArray(FillMetadata &fm, Double time) {
    if (fm.ms == NULL) return;

    // flush Feed records for the last array if necessary.  If the 
    // "feed time" is set to the current time, then assume that it 
    // has already been done.
    if (time != fm.feedtime) {
	flushFeedRecords(fm, time);
    }

    addAntennaPositions(fm, time);
}

void MirFiller::updateSource(FillMetadata &fm, Double time) { 
    if (fm.ms != NULL) addSource(fm, time);
}

void MirFiller::updateSourceModel(FillMetadata &fm, Double time) {
    // Added by dgoscha -- NCSA to handle updates to the SOURCE_MODEL
    // column and write out the component list.

    // This function assumes that the source has already been added to 
    // the SOURCE table.
    AlwaysAssert(fm.source->row >= 0, AipsError);

    // We don't want to update the model unless the specified time interval
    // (default 8 hr) has passed since the model was last updated.
    // modeltime is the time the last model was written; it is initialized
    // to time-updmodelint_p so that the model is written the first time.
    if (time-updmodelint_p >= fm.modeltime) {

	if (verbose()) 
	    log_p << LogIO::NORMAL << "Updating source model for " 
		  << fm.source->name << LogIO::POST;
	fm.modeltime = time;

	MSSourceColumns& src = fm.msc->source();
	uInt row = static_cast<uInt>(fm.source->row);

	// Check and see if there is an existing Record in the SOURCE_MODEL
	// column.  If not, we write out the model record there.  If so, we
	// have to create a new row to hold the new model record.
	TableRecord existingSourceModelRecord;
	src.sourceModel().get(row, existingSourceModelRecord);
	if (existingSourceModelRecord.nfields() != 0) { 
	    // a model is already there
	    row = addSource(fm, time);
	}

	// This should actually write out the component list.
	ComponentList model;
	SkyComponent planetComponent;
	DiskShape defaultDisk;
	SpectralIndex siModel;
		
	// Convert the brightness temperature to flux.  The conversion used is:
	// S_nu[Jy] = 5.67e-7 (T_b[K]) (nu[GHz])^2 
	//                                        (plmaj[arcsec]*plmin[arcsec])
	Double S_0=5.67e-7 * fm.pltb * (fm.freq*fm.freq) * fm.plmaj * fm.plmin;
	siModel.setRefFrequency(MFrequency(Quantity(fm.freq, "GHz")));

	// Set the spectral index to 2 --->  S = S_0(nu/nu_0)^2
	siModel.setIndex(2.0);
		
	// Grab position.  This is done earlier in addField(), so may be best
	// to promote the variables in that function to private class memeber
	// functions so we don't have to get them again here.
	// MEpoch ep(Quantity(time, "s"), MEpoch::UTC);
	MDirection::Types epoch =
	    (nearAbs(fm.source->epoch,1950.0,0.01)) ? MDirection::B1950
	                                            : MDirection::J2000;
	MDirection position (Quantity(fm.source->ra, "rad"),
			     Quantity(fm.source->dec, "rad"),
			     epoch);
	defaultDisk.setRefDirection (position);
		
	// A quick note about the setWidth function -- as written below, the
	// arguments that are passed in (plmaj, plmin, plangle) are converted 
	// to radians when written.  If you view the table when these are 
	// written the values will be in rad, not the units given here.
	defaultDisk.setWidth(Quantity(fm.plmaj,"arcsec"),
			     Quantity(fm.plmin,"arcsec"),
			     Quantity(fm.plangle,"deg"));
	planetComponent.setShape(defaultDisk);
	planetComponent.flux().setValue(S_0);
	planetComponent.setSpectrum(siModel);
	model.add(planetComponent);
	
	// Write it out
	String modelName = String(fm.outname) + "/SOURCE/MODEL_" + 
	                                     String::toString(row);
	model.rename(Path(modelName));
		
	//update SOURCE_MODEL column in the SOURCE subtable
	// the SOURCE_MODEL column contains a table record that points to the 
	// name of the Component List containing the source model parameters.
	// The naming scheme for models is SOURCE_MODEL_<rownum> where 
	// <rownum> is the row in the SOURCE subtable for that source

	// Need to construct the table record
	RecordDesc modelDescription;
	modelDescription.addField("model", TpTable);
	TableRecord sourceModelRecord(modelDescription);
	sourceModelRecord.defineTable(RecordFieldId("model"), 
				      Table(modelName));

	src.sourceModel().put(row, sourceModelRecord);
    }
}

void MirFiller::updateField(FillMetadata &fm, Double time) { 
    if (fm.ms != NULL && fm.field->id < 0) addField(fm, time);
}

void MirFiller::updateFreqSetup(FillMetadata &fm, Double time) { 

    // update size of the data-loading buffers
    if (fm.ms != NULL) {                          // i.e. we are filling
	fm.buf.resizeForNspect(fm.fsetup->nspect);
	for(Int i=0; i < fm.fsetup->nspect; i++) 
	    fm.buf.resizeForSpWin(i, fm.fsetup->nschan[i]);
	fm.buf.resizeForSpWin(fm.fsetup->nspect, 1);
    }

    if (fm.fsetup->id < 0) {     // update subtables
	addCorrelatorSetup(fm, time); 
	// data desc now added after polarization setup is added
	// addDataDesc(fm);
    }
}

void MirFiller::updatePolSetup(FillMetadata &fm, Double time) { 
    // handle a new polarization correlation type here only when we are 
    // loading polarizations on-the-fly (i.e. dataset was not prescanned) 
    // as single-polarization setups.
    if (fm.polotf) {
	addPolarization(fm, True);
	if (fm.getDataDescID() < 0) addDataDesc(fm);
    }
}

void MirFiller::updateTsys(FillMetadata &fm, Double time) { 
    if (fm.nsystemp.nelements() <= 0 && fm.wsystemp.nelements() <= 0) return;

    MSSysCal& tbl = fm.ms->sysCal();
    MSSysCalColumns& syscal = fm.msc->sysCal();
    ScalarColumn<Float> jyperk(tbl, "BIMA_JYPERK");
    Int row = tbl.nrow();

    Int winoff = fm.fsetup->id;
    Int feedid = fm.ms->feed().nrow();  // Note: feed record not written yet.
    Int nants = fm.nants;
    if (fm.telescope == fm.BIMA) nants = fm.bimaAntCount();

    Vector<Float> wtemps, ntemps;

    Int i,s,w;
    for(i=1; i <= nants; i++) {
	ntemps = fm.nsystemp.row(i-1);
	if (allEQ(ntemps, Float(-1.0))) continue;
	ntemps(ntemps == Float(-1.0)) = 0.0;

	for(s=0, w=0; s < fm.fsetup->nspect; s++) {
	    if (! winsel_p(s)) continue;

	    tbl.addRow();
	    syscal.tsys().put(row, ntemps(Slice(s)));
	    syscal.antennaId().put(row, fm.getAntennaID(i));
	    syscal.feedId().put(row, feedid);
	    syscal.spectralWindowId().put(row, w+winoff);
	    syscal.time().put(row, time);
	    syscal.interval().put(row, -1);
	    syscal.tsysFlag().put(row, False);
	    jyperk.put(row, fm.jyperk);
	    row++;
	    w++;
	}

	wtemps = fm.wsystemp.row(i-1);
	if (allEQ(wtemps, Float(-1.0))) continue;
	wtemps(wtemps == Float(-1.0)) = 0.0;

	for(s=0; s < fm.fsetup->nwide; s++) {
	    if (! widesel_p(s)) continue;

	    tbl.addRow();
	    syscal.tsys().put(row, wtemps(Slice(s)));
	    syscal.antennaId().put(row, fm.getAntennaID(i));
	    syscal.feedId().put(row, feedid);
	    syscal.spectralWindowId().put(row, w+winoff);
	    syscal.time().put(row, time);
	    syscal.interval().put(row, -1);
	    syscal.tsysFlag().put(row, False);
	    jyperk.put(row, fm.jyperk);
	    row++;
	    w++;
	}
    }
}

void MirFiller::addTsysRecords(FillMetadata &fm, Double time, 
			       Cube<Float>& tsys) 
{
    MSSysCal& tbl = fm.ms->sysCal();
    MSSysCalColumns& syscal = fm.msc->sysCal();
    ScalarColumn<Float> jyperk(tbl, "BIMA_JYPERK");
    Int row = tbl.nrow();

    Int nants = fm.nants;
    Int nspw = nwinsel_p+nwidesel_p;
    if (fm.telescope == fm.BIMA) nants = fm.bimaAntCount();

    Int i,j;
    for(i=1; i <= nants; i++) {
	for(j=0; j < nspw; j++) {
            Vector<Float> temps = tsys.xyPlane(j).column(i-1);
            if (allEQ(temps, Float(-1.0))) continue;
            temps(temps == Float(-1.0)) = 0.0;

	    tbl.addRow();
	    syscal.antennaId().put(row, fm.getAntennaID(i));
	    syscal.feedId().put(row, 0);
	    syscal.spectralWindowId().put(row, j);
	    syscal.time().put(row, time);
	    syscal.interval().put(row, -1);
	    syscal.tsys().put(row, temps);
	    syscal.tsysFlag().put(row, False);
	    jyperk.put(row, fm.jyperk);
	    row++;
	}
    }
}

void MirFiller::flushMovingFields(FillMetadata& fm, Double etime) {

    if (!fm.movingsrc) return;

    PtrHolder<Table> movingTbl;
    Table *mtbl = 0;

    // look for moving objects
    ConstListIter<MirSource*> li(rdr_p.getSourceList());
    for(; ! li.atEnd(); ++li) {
	MirSource *src = li.getRight();
	if (src->id > -1 && src->isMoving()) {
	    Vector<Double> time;
	    Vector<Double> ra;
	    Vector<Double> dec;
	    src->getMotion(time, ra, dec);

	    Matrix<Double> dirvals;
	    if (fitskymotion(dirvals, time, ra, dec)) {
		MSFieldColumns& msfc = fm.msc->field();
		Matrix<Double> use(dirvals.shape());
		for(ListIter<MirField*> f(src->flds); ! f.atEnd(); ++f) {
		    MirField *fld = f.getRight();
		    if (fld->id < 0) continue;
		    use = dirvals;

		    // this is only accurate for small ddec
		    use(0,0) += fld->dra/cos(use(1,0));
		    use(1,0) += fld->ddec;

		    msfc.time().put(fld->id, min(time));
		    msfc.numPoly().put(fld->id, use.ncolumn()-1);
		    msfc.delayDir().put(fld->id, use);
		    msfc.phaseDir().put(fld->id, use);
		    msfc.referenceDir().put(fld->id, use);
		}
	    }

	    if (movfield_p) {

		// load the moving field data into our custom subtable 
		// called BIMA_MOVING_OBJECTS
		if (! movingTbl.ptr()) {

		    // gotta create the table
		    TableDesc td("BIMA_MOVING_OBJECTS", TableDesc::Scratch);
		    td.comment() = 
	       "Raw Positions of moving objects loaded from a MIRIAD dataset";
		    td.addColumn(ScalarColumnDesc<Int>("SOURCE_ID"));
		    td.addColumn(ScalarColumnDesc<Int>("FIELD_ID"));
		    td.addColumn(ScalarColumnDesc<Double>("TIME"));

		    TableMeasValueDesc tmeasVal(td, "TIME");
		    TableMeasRefDesc tmeasRef = TableMeasRefDesc(MEpoch::UTC);
		    Vector<Unit> tu(1, Unit("s"));
		    TableMeasDesc<MEpoch> tmeasCol(tmeasVal, tmeasRef, tu);
		    tmeasCol.write(td);

		    td.addColumn(ArrayColumnDesc<Double>("DIRECTION",
			             "Direction to source", IPosition(1,2), 
				     ColumnDesc::FixedShape));

		    TableMeasValueDesc dmeasVal(td, "DIRECTION");
		    TableMeasRefDesc dmeasRef = 
			TableMeasRefDesc(MDirection::J2000);
		    TableMeasDesc<MDirection> dmeasCol(dmeasVal, dmeasRef);
		    dmeasCol.write(td);

		    SetupNewTable snt(fm.outname+"/BIMA_MOVING_OBJECTS",
				      td, Table::New);
		    StandardStMan stman;
		    snt.bindAll(stman);
		    movingTbl.set(new Table(snt));
		    mtbl = movingTbl.ptr();
		    fm.ms->rwKeywordSet().defineTable("BIMA_MOVING_OBJECTS", 
						      *mtbl);
		}

		MDirection::Types epochRef = MDirection::J2000;
		if (nearAbs(src->epoch,1950.0,0.01)) 
		    epochRef = MDirection::B1950;

		uInt row = mtbl->nrow();
		ScalarColumn<Int> srcid(*mtbl, "SOURCE_ID");
		ScalarColumn<Int> fldid(*mtbl, "FIELD_ID");
		ScalarColumn<Double> timec(*mtbl, "TIME");
		MDirection::ScalarColumn dir(*mtbl, "DIRECTION");
		for(ListIter<MirField*> f(src->flds); ! f.atEnd(); ++f) {
		    Double fra, fdec;
		    MirField *fld = f.getRight();
		    mtbl->addRow(time.nelements());
		    for(uInt i=0; i < time.nelements(); ++i) {
			fdec = dec(i) + fld->ddec;
			fra = ra(i) + fld->dra/cos(dec(i));
			MDirection dirm(MVDirection(fra, fdec),
					MDirection::Ref(epochRef));

			srcid.put(row, src->id);
			fldid.put(row, f.getRight()->id);
			timec.put(row, time(i));
			dir.put(row, dirm);
			row++;
		    }
		}
	    }
	}
    }
}

Bool MirFiller::fitskymotion(Matrix<Double> &dirfit, 
			     const Vector<Double> &time, 
			     const Vector<Double> &ra, 
			     const Vector<Double> &dec) 
{
    Int order = time.nelements() - 1;
    if (order > planetfit_p) order = planetfit_p;
    if (order < 0) return False;
    dirfit.resize(2, order+1);
    dirfit = 0.0;

    Vector<double> utime(time.shape());
    utime = time;
    utime -= utime(0);

//      cout << "time data: " << utime << endl;
//      cout << "RA data: " << ra << endl;
//      cout << "Dec data: " << dec << endl;

    LinearFitSVD<Double> fitter;
    Polynomial<AutoDiff<Double> > pfunc(order);
    fitter.setFunction(pfunc);

    dirfit.row(0) = fitter.fit(utime, ra);
    dirfit.row(1) = fitter.fit(utime, dec);

//      cout << "RA fit: " << dirfit.row(0) << endl;
//      cout << "Dec fit: " << dirfit.row(1) << endl;

    return True;
}

