//# carmafiller:  miriad dataset to MeasurementSet filler
//# Copyright (C) 1997,2000,2001,2002
//# Associated Universities, Inc. Washington DC, USA.
//#
//# This program is free software; you can redistribute it and/or modify
//# it under the terms of the GNU General Public License as published by
//# the Free Software Foundation; either version 2 of the License, or
//# (at your option) any later version.
//#
//# This program is distributed in the hope that it will be useful,
//# but WITHOUT ANY WARRANTY; without even the implied warranty of
//# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//# GNU General Public License for more details.
//# 
//# You should have received a copy of the GNU General Public License
//# along with this program; if not, write to the Free Software
//# Foundation, Inc., 675 Masve, Cambridge, MA 02139, USA.
//#
//# Correspondence concerning AIPS++ should be addressed as follows:
//#        Internet email: aips2-request@nrao.edu.
//#        Postal address: AIPS++ Project Office
//#                        National Radio Astronomy Observatory
//#                        520 Edgemont Road
//#                        Charlottesville, VA 22903-2475 USA
//#
//# $Id: bimafiller.cc,v 1.6 2009/10/22 18:27:54 pteuben Exp $
//

#include <casa/aips.h>
#include <casa/stdio.h>
#include <casa/iostream.h>
#include <casa/OS/File.h>
#include <casa/Utilities/GenSort.h>

#include <casa/Arrays/Cube.h>
#include <casa/Arrays/Matrix.h>
#include <casa/Arrays/Vector.h>
#include <casa/Arrays/ArrayMath.h>
#include <casa/Arrays/ArrayUtil.h>
#include <casa/Arrays/ArrayLogical.h>

#include <casa/Inputs/Input.h>

#include <measures/Measures.h>
#include <measures/Measures/MPosition.h>
#include <measures/Measures/MeasData.h>
#include <measures/Measures/Stokes.h>

#include <tables/Tables.h>
#include <tables/Tables/TableInfo.h>


#include <ms/MeasurementSets.h>              // Measurementset and MSColumns

// Miriad interface
#include <mirlib/maxdimc.h>
#include <mirlib/miriad.h>

#include <casa/namespace.h>

// MIRIAD dataset MeasurementSet filler. Derived from bimafiller.
// This version is CARMA specific, but in the end there is no
// reason why this should be CARMA specific.
//
// For MS2 based files, see also mirfiller.g, developed by Ray
// Plante to test the new SpectralWindow layout.
//
// See also: 
//	http://www.astro.umd.edu/~teuben/casa/bimafiller.html
// for more up to date documentation, as well as a reference to Ray's
// work on mirfiller.
// Despite that this program is written in C++ and uses classes, it is
// really a set of routines manipulating a huge common block.....
//
// Acknowledgement: this program was originally cloned off uvfitsfiller.cc
//                  whose code is now in MSFitsInput.cc
// 
//
// Limitations, Caveats and Remaining ToDo's
//    - only really deals with 1 polarization
//    - does not apply the various (miriad) complex gain corrections
//      (uvcat and friends should be used for that in miriad, it would be
//      silly to duplicate that code here)
//    - copies both the wide, window and narrow band data (or whichever is present)
//      but can subselect via wide=,win=,narrow=
//    - this code won't run if Double!=double or Float!=float or Int!=int
//      On some future 64bit machines this may cause problems?
//    - CARMA type arrays when dishes are not the same size
//    - do not mix arrays from different telescopes (e.g. BIMA and ATCA)
//      but it should handle different configurations (ARRAYs) of the same
//      instrument
//    - handle multiple array configuration datasets that were UVCAT'ed
//      (needs to count antennae up at each array configuration)
//    - true miriad storage manager ?
//    - read and test ATNF data (primarely a MIRIAD-UVFITS conformance test?)
//    - check UV override, is that being handled transparently?
//
//    - spectral windows layout  (code present, fix table access descriptors) 
//      CHECK: what happens if e.g. uvcat does some line= preprocessing
//    - correct restfreq's in source tables that relate to the spectral windows
//      (this code currently coredumps)
//    - make win=,narrow=,wide=0 cause non of them to be written in that selection
//    - add syscal table
//    - add weather table
//    - turn it into a true AIPS++/GUI application

//  History:
//   Spring 1997:   written (cloned off uvfitsfiller)       Peter Teuben
//   July 1997:     Y2K, fixed table-interface		                PJT
//   Dec 1997:      fixed wideband only data (e.g. uvgen)               PJT
//   ???            somebody fixed up this code for some new release    ???
//   May 2000:	    fixed up for various new AIPS++ conventions	        PJT
//                  and added multi-source & field                      PJT
//   Sep 2000:      development now on linux, converted to OldMS        PJT
//   Dec 2000:	    Conversion to MS (MS2)				PJT
//                  typical compile time: (P600/256M/15MBps HD: 36")
//                  typical 3c273 conversion time:  2.6" (5.3->11.3 MB)
//                  Cf. that to "uvio" processing time, which runs at disk I/O
//                  speed (15 MB/s on the beforementioned laptop)
//   Jan-Feb 2001:  window layout, restfreq's and various things to get 
//                  msmultiscale to work; added some syscal support     PJT
//   April 2001:    final cleanup for the 1.5 AIPS++ release -          PJT
//                  [note that from this moment on bimafiller is for
//                   experimental use and deprecated, the new version
//                   will be called 'mirfiller'
//   Oct 2001:      mirlib changed location
//   Sep 2009:      revived in CASA, added various tables for CARMA



// a placeholder for the MIRIAD spectral window configuration
// see also the MIRIAD programmers guide, and its appendix on UV Variables 

typedef struct window {     // AIPS++ defines everything mid-band mid-interval
  // narrow band (plus MAXWIDE space to point into the wide band data too)
  int    nspect;                   // number of valid windows (<=MAXWIN, typically 16)
  int    nschan[MAXWIN+MAXWIDE];   // number of channels in a window
  int    ischan[MAXWIN+MAXWIDE];   // starting channel of a window (1-based)
  double sdf[MAXWIN+MAXWIDE];      // channel separation 
  double sfreq[MAXWIN+MAXWIDE];    // frequency of first channel in window (doppler changes)
  double restfreq[MAXWIN+MAXWIDE]; // rest freq, if appropriate
  char   code[MAXWIN+MAXWIDE];     // code to aips++ identification (N, W or S)
  int    keep[MAXWIN+MAXWIDE];     // keep this window for output (0=false 1=true)

  // wide band (for BIMA actually contains window averages too)
  int    nwide;                    // number of wide & window averages
  float  wfreq[MAXWIDE];           // freq
  float  wwidth[MAXWIDE];          // width


} WINDOW;

//  the maximum number of fields in mosaicing observations, careful, miriad
//  often has a smaller value, like 64 or 128.
#ifndef MAXFIELD
# define MAXFIELD  256
#endif

#ifndef MAXMSG
# define MAXMSG 256
#endif

void cause_coredump(void);


// helper functions

// Convert fits date string of form dd/mm/yy to mjd seconds

Double date2mjd(const String& date)
{
  Int day,month,year;
  
  if (date[2] == '/') {     // old FITS style (dd/mm/yy)
    sscanf(date.chars(),"%2d/%2d/%2d",&day,&month,&year);
    year+=1900;
    if (year<1950) year+=100;   // yuck !
  } else {      // YEAR-2000 (ISO) convention (ccyy-mm-ddThh:mm:ss.sss)
    cerr << "Parsing new Year-2000 notation" << endl;
    sscanf(date.chars(),"%4d-%2d-%2d",&year,&month,&day);
    //sscanf(date,"%4d-%2d-%2dT%2d:%2d:%f",&year,&month,&day,&hour,&min,&sec);
  }
  MVTime mjd_date(year,month,(Double)day);
  return mjd_date.second();
}



// a helper class 

class BimaFiller
{
  // This is an implementation helper class used to store 'local' data
  // during the filling process.
public:
  // Create from a bima dataset (directory)
  BimaFiller(String& infile, Int debug=0, 
	     Bool Qtsys=False,
	     Bool Qsplit=True,
	     Bool Qarrays=False);

  // Standard destructor
  ~BimaFiller();
  
  // Check some of the contents of the data and header read
  void checkInput(Block<Int>& narrow, Block<Int>& wide, Block<Int>& window);

  // Debug output level
  Bool Debug(int level);

  // Set up the MeasurementSet, including StorageManagers and fixed columns.
  // If useTSM is True, the Tiled Storage Manager will be used to store
  // DATA, FLAG and WEIGHT_SPECTRUM
  void setupMeasurementSet(const String& MSFileName, Bool useTSM=True);

  // Fill the Observation and History (formerly ObsLog) tables
  void fillObsTables();

  // Fill the main table by reading in all the visibilities
  void fillMSMainTable();

  // Make an Antenna Table (can be called incrementally now)
  void fillAntennaTable();

  // Make a Syscal Table (can be called incrementally)
  void fillSyscalTable();

  // fill Spectralwindow table 
  void fillSpectralWindowTable();

  // fill Field table 
  void fillFieldTable();

  // fill Source table 
  void fillSourceTable();

  // fill the Feed table with minimal info needed for synthesis processing
  void fillFeedTable();

  // fix up the EPOCH MEASURE_REFERENCE keywords using the value found
  // in the (last) AN table - check if miriad really needs it
  void fixEpochReferences();

  void Tracking(int record);
  void init_window(Block<Int>& narrow, Block<Int>& wide, Block<Int>& window);
  void update_window();
  void Error(char *msg);
  void Warning(char *msg);

private:
  String                 infile_p;     // filename
  Int                    uv_handle_p;  // miriad handle 
  MeasurementSet         ms_p;         // the MS itself
  MSColumns             *msc_p;        // handy pointer to the columns in an MS  
  Int                    debug_p;      // debug level
  Int                    nIF_p;
  String                 array_p, project_p, object_p, telescope_p, observer_p, timsys_p;
  Vector<Int>            nPixel_p, corrType_p, corrIndex_p;
  Matrix<Int>            corrProduct_p;
  Double                 epoch_p;
  MDirection::Types      epochRef_p;
  Int                    nArray_p;      // number of arrays (nAnt_p.nelements())
  Block<Int>             nAnt_p;        // number of antennas per array
  Block<Vector<Double> > receptorAngle_p;
  Vector<Double>         arrayXYZ_p;    // needs to be made with 3 elements
  Vector<String>         source_p;      // list of source names

  // the following variables are for miriad, hence not Double/Int/Float
  // thus the code may have to be fixed on machines where these do not
  // agree ... may need special access code to get those into AIPS++
  // types on 64 bit machines??

  double preamble[5], first_time;
  int    ifield, nfield, npoint;          // both dra/ddec should become Vector's
  float  dra[MAXFIELD];
  float  ddec[MAXFIELD];
  float  dra_p, ddec_p;
  int    pol_p;
  char   message[MAXMSG];


  // The following items more or less follow the uv variables in a dataset
  Int    nants_p, nants_offset_p, nchan_p, nwide_p, npol_p;
  Double antpos[3*MAXANT];
  Double ra_p, dec_p;       // current pointing center RA,DEC at EPOCH 
  Float  inttime_p;
  Double freq_p;            // rest frequency of the primary line
  Int    mount_p;
  Double time_p;            // current MJD time

  // MIRIAD spectral window definition
  WINDOW win;
  Bool   Qtsys_p;   /* tsys weight's */
  Bool   Qsplit_p;  /* split spectral windows */
  Bool   Qarrays_p; /* write separate arrays */

  // Data buffers.... again in MIRIAD format
  
  float  data[2*MAXCHAN], wdata[2*MAXCHAN];	// 2*MAXCHAN since complex !!
  int    flags[MAXCHAN], wflags[MAXCHAN];
  float  systemp[MAXANT*MAXWIDE];
  int    zero_tsys;
};

BimaFiller::BimaFiller(String& infile, Int debug, 
		       Bool Qtsys, Bool Qsplit, Bool Qarrays)
{
  infile_p = infile;
  debug_p = debug;
  nArray_p = 0;
  nfield = 0;           //  # mosaiced fields (using offsets?)
  npoint = 0;           //  # pointings (using independant RA/DEC?)
  Qtsys_p = Qtsys;
  Qsplit_p = Qsplit;
  Qarrays_p = Qarrays;
  zero_tsys = 0;

  if (Debug(1)) cout << "BimaFiller::Bimafiller debug_level=" << debug << endl;
  if (Debug(1)) cout << "Opening miriad dataset " << infile_p << endl;
  if (Debug(1)) cout << "See also "
		     << "http://www.astro.umd.edu/~teuben/casa/bimafiller.html" << endl;
  if (Debug(1)) cout << (Qsplit_p ? "splitting windows" : "pasting windows") << endl;
  if (Debug(1)) cout << (Qtsys_p ?  "tsys weights" : "weights=1") << endl;
  if (Debug(1)) cout << (Qarrays_p ? "split arrays" : "single array forced") << endl;

  if (sizeof(double) != sizeof(Double))
    cout << "Double != double; bimafiller will probably fail" << endl;
  if (sizeof(int) != sizeof(Int))
    cout << "int != Int; bimafiller will probably fail" << endl;

  // open miriad dataset
  uvopen_c(&uv_handle_p, infile_p.chars(), "old");

  // preamble data must be UVW (default miriad is UV)
  uvset_c(uv_handle_p,"preamble","uvw/time/baseline",0,0.0,0.0,0.0);

  // initialize those UV variables that need to be tracked
  Tracking(-1);      
}
 
BimaFiller::~BimaFiller() 
{
  if (Debug(1)) cout << "BimaFiller::~BimaFiller" << endl;
  if (zero_tsys)
    cout << "There were " << zero_tsys << " record with no WEIGHT due to zero TSYS" << endl;

  // most single MIRIAD files are time ordered, so could check for 
  // that, and if so, add SORT_ORDER = 'ASCENDING' and COLUMNS = 'TIME'

  if (Debug(1)) cout << "*** Closing " << infile_p << " ***\n" ;
}

void BimaFiller::Error(char *msg)
{
  throw(AipsError(msg));
}

void BimaFiller::Warning(char *msg)
{
  cout << "### Warning: " << msg <<  endl;
}

Bool BimaFiller::Debug(int level)
{
  Bool ok=False;
  if (level <= debug_p) ok=True;
  return ok;
}

void BimaFiller::checkInput(Block<Int>& narrow, Block<Int>& wide, Block<Int>& window)
{
  Bool ok=True;
  Int i, nread, nwread, vlen, vupd;
  char vtype[10], vdata[64];
  Float epoch;
  Float zero = 0.0;

  if (Debug(1)) cout << "BimaFiller::checkInput" << endl;

  // Let's read one scan and try and derive some basics. If important
  // variables not present, bail out (or else scan on)

  for (;;) {

    uvread_c(uv_handle_p, preamble, data, flags, MAXCHAN, &nread);    
    if (nread <= 0) {
      throw(AipsError("BimaFiller: Bad first uvread: no narrow or wide band data present"));
      ok = False;
      break;
    }
    uvwread_c(uv_handle_p, wdata, wflags, MAXCHAN, &nwread);
    if (Debug(1))
        cout << "1st scan: nread=" << nread << " nwread="<<nwread<<endl;

    // get the initial correllator setup

    init_window(narrow,wide,window);

    // BIMA should store nread + nwread, or handle it as option
    if (win.nspect > 0) {               // narrow band, with possibly wide band also
      nchan_p = nread;
      nwide_p = nwread;
    } else {                            // wide band data only: nread=nwread
      nchan_p = nread;
      nwide_p = 0;
    }

    // get the initial array configuration

    nants_offset_p = 0;
    uvgetvr_c(uv_handle_p,H_INT, "nants", (char *)&nants_p,1);
    uvgetvr_c(uv_handle_p,H_DBLE,"antpos",(char *)antpos,3*nants_p);
    if (Debug(1)) {
      cout << "Found " << nants_p << " antennas (first scan)" << endl;
      for (int i=0; i<nants_p; i++) {
        cout << antpos[i] << " " << 
              antpos[i+nants_p] << " " << 
              antpos[i+nants_p*2] << endl;
      }
    }
    
    // remember systemp is stored systemp[nants][nwin] in C notation
    if (win.nspect > 0) {
      uvgetvr_c(uv_handle_p,H_REAL,"systemp",(char *)systemp,nants_p*win.nspect);
      if (Debug(1)) {
	cout << "Found systemps (first scan)" ;
	for (Int i=0; i<nants_p; i++)  cout << systemp[i] << " ";
	cout << endl;
      }
    } else {
      uvgetvr_c(uv_handle_p,H_REAL,"wsystemp",(char *)systemp,nants_p);
      if (Debug(1)) {
	cout << "Found wsystemps (first scan)" ;
	for (Int i=0; i<nants_p; i++)  cout << systemp[i] << " ";
	cout << endl;
      }    }

    if (win.nspect > 0) {
      uvgetvr_c(uv_handle_p,H_DBLE,"restfreq",(char *)win.restfreq,win.nspect);
      if (Debug(1)) {
	cout << "Found restfreq (first scan)" ;
	for (Int i=0; i<win.nspect; i++)  cout << win.restfreq[i] << " ";
	cout << endl;
      }
    }

    // Note that BIMA coordinates are in nanosec, but actual unused
    // antennas are filled with -999 values (or sometimes 0!)

    uvprobvr_c(uv_handle_p,"project",vtype,&vlen,&vupd);
    if (vupd) {
      uvgetvr_c(uv_handle_p,H_BYTE,"project",vdata,32);
      project_p = vdata;
    } else
      project_p = "unknown";

    if (Debug(1)) cout << "Project=>" << project_p << "<=" << endl;

    uvgetvr_c(uv_handle_p,H_BYTE,"source",vdata,10);
    object_p = vdata;
    if (Debug(0)) {
      cout << "Source=>" << object_p << "<="
           << "ns=" << source_p.nelements() 
	   << endl;
    }

    // TODO: telescope will now change, so this is not a good idea
    uvgetvr_c(uv_handle_p,H_BYTE,"telescop",vdata,10);
    array_p = vdata;
    if (Debug(1)) cout << "Telescope=>" << array_p << "<=" << endl;

    if (array_p=="HATCREEK" || array_p=="BIMA" || array_p=="OVRO" || array_p=="CARMA") {
      mount_p = 0;      // hmm, Wilson never wrote this out....
    } else {
      if (array_p == "VLA")
        mount_p = 1;
      uvrdvr_c(uv_handle_p,H_INT,"mount",(char *)&mount_p, (char *)&mount_p, 1);
      cout << "Warning: " << array_p 
           << " Cannot handle all of this telescope yet" << endl;
      cout << "Assumed mount=" << mount_p << endl;
    }
    
    uvprobvr_c(uv_handle_p,"observer",vtype,&vlen,&vupd);
    if (vupd) {
      uvgetvr_c(uv_handle_p,H_BYTE,"observer",vdata,10);
      observer_p = vdata;
    } else              // BIMA doesn't write out the observer (yet)
      observer_p = "unknown";    

    uvgetvr_c(uv_handle_p,H_REAL,"epoch",(char *)&epoch,1);
    epoch_p = epoch;
    // do this globally, we used to do this in the Field table alone
    epochRef_p=MDirection::J2000;      
    if (nearAbs(epoch_p,1950.0,0.01)) epochRef_p=MDirection::B1950;   

    uvgetvr_c(uv_handle_p,H_INT,"npol", (char *)&npol_p,1);
    uvgetvr_c(uv_handle_p,H_INT,"pol",(char *)&pol_p,1);
    uvgetvr_c(uv_handle_p,H_REAL,"inttime",(char *)&inttime_p,1);
    uvgetvr_c(uv_handle_p,H_DBLE,"freq",(char *)&freq_p,1);
    freq_p *= 1e9;

    // get initial pointing offset (ATCA doesn't use them)

    if (array_p == "HATCREEK") {
#if 0
      // one would expect HatCreek data to have dra/ddec
      // but e.g. uvgen doesn't write them if only 1 pointing.....
      uvgetvr_c(uv_handle_p,H_REAL,"dra", (char *)&dra_p, 1);
      uvgetvr_c(uv_handle_p,H_REAL,"ddec",(char *)&ddec_p, 1);
#else
      uvrdvr_c(uv_handle_p,H_REAL,"dra", (char *)&dra_p, (char *)&zero,1);
      uvrdvr_c(uv_handle_p,H_REAL,"ddec",(char *)&ddec_p,(char *)&zero,1);
#endif
    }

    // and initial source position

    uvgetvr_c(uv_handle_p,H_DBLE,"ra", (char *)&ra_p, 1);
    uvgetvr_c(uv_handle_p,H_DBLE,"dec",(char *)&dec_p,1);

    // check if certain calibration tables are present and warn if so,
    // since we can't (don't want to) deal with them here; miriad
    // programs like uvcat should be used to apply them!

    if (hexists_c(uv_handle_p,"gains")) 
        cout << "Warning: gains table present, but cannot apply them" << endl;
    if (hexists_c(uv_handle_p,"bandpass")) 
        cout << "Warning: bandpass table present, but cannot apply them" << endl;
    if (hexists_c(uv_handle_p,"leakage")) 
        cout << "Warning: leakage table present, but cannot apply them" << endl;


    if (npol_p > 1) {     // read the next npol-1 scans to find the other pols
      for (i=1; i<npol_p; i++) {
        uvread_c(uv_handle_p, preamble, data, flags, MAXCHAN, &nread);
        if (nread <= 0) {
          ok = False;
          break;
        }
        if (i==1) cout << "POL(" << i << ") = " << pol_p << endl;
        uvgetvr_c(uv_handle_p,H_INT,"pol",(char *)&pol_p,1);        // FIX
        cout << "POL(" << i+1 << ") = " << pol_p << endl;
      }
    }
    // only do one scan
    break;
  }
  uvrewind_c(uv_handle_p);

  Int numCorr = 1;                  // 1 polarization ?!?!
  corrType_p.resize(numCorr); 
  for (i=0; i < numCorr; i++) {
    // note: 1-based ref pix
    corrType_p(i)=pol_p;            // 1 pol !!!!
    // convert AIPS-convention Stokes description to aips++ enum
    // CHECK if these are really the right conversions for AIPS++
    if (corrType_p(i)<0) {
      if (corrType_p(i)==-8) corrType_p(i)=Stokes::YX;
      if (corrType_p(i)==-7) corrType_p(i)=Stokes::XY;
      if (corrType_p(i)==-6) corrType_p(i)=Stokes::YY;
      if (corrType_p(i)==-5) corrType_p(i)=Stokes::XX;
      if (corrType_p(i)==-4) corrType_p(i)=Stokes::LR;
      if (corrType_p(i)==-3) corrType_p(i)=Stokes::RL;
      if (corrType_p(i)==-2) corrType_p(i)=Stokes::LL;
      if (corrType_p(i)==-1) corrType_p(i)=Stokes::RR;
    }
  }
  Vector<Int> tmp(numCorr); tmp=corrType_p;
  // Sort the polarizations to standard order
  GenSort<Int>::sort(corrType_p);
  corrIndex_p.resize(numCorr);
  // Get the sort indices to rearrange the data to standard order
  for (i=0;i<numCorr;i++) {
    for (Int j=0;j<numCorr;j++) {
      if (corrType_p(j)==tmp(i)) corrIndex_p(i)=j;
    }
  }

  // Figure out the correlation products from the polarizations
  corrProduct_p.resize(2,numCorr); corrProduct_p=0;
  for (i=0; i<numCorr; i++) {
    Fallible<Int> receptor=Stokes::receptor1(Stokes::type(corrType_p(i)));
    if (receptor.isValid()) corrProduct_p(0,i)=receptor;
    receptor=Stokes::receptor2(Stokes::type(corrType_p(i)));
    if (receptor.isValid()) corrProduct_p(1,i)=receptor;
  }
}

void BimaFiller::setupMeasurementSet(const String& MSFileName, Bool useTSM)
{
  if (Debug(1)) cout << "BimaFiller::setupMeasurementSet" << endl;

  Int nCorr = 1;        // STOKES axis: only one polarization for now, BIMA timeslices
  Int nChan = nchan_p + nwide_p;  // total along FREQ axis (line + wide + window avg)

  if (Qsplit_p) 
    nIF_p = win.nspect + win.nwide;
  else
    nIF_p = 1;

  // Make the MS table
  TableDesc td = MS::requiredTableDesc();

  // In case we don't split windows, 
  // we know that the data is going to be the same shape throughout
  // so we want to have a fixed shape data column 
  // TODO?? could optimize if we do have equal size windows!!
  if (Qsplit_p) {
    MS::addColumnToDesc(td, MS::DATA,2);
    td.removeColumn(MS::columnName(MS::FLAG));
    MS::addColumnToDesc(td, MS::FLAG,2);
  } else {
    MS::addColumnToDesc(td, MS::DATA, IPosition(2,nCorr,nChan), 
		      ColumnDesc::Direct);
    td.removeColumn(MS::columnName(MS::FLAG));
    MS::addColumnToDesc(td, MS::FLAG, IPosition(2,nCorr,nChan), 
		      ColumnDesc::Direct);
  }
#if 0
  // why does the FITS code do this? We don't need it....
  td.removeColumn(MS::columnName(MS::SIGMA));
  MS::addColumnToDesc(td, MS::SIGMA, IPosition(1,nCorr), 
		      ColumnDesc::Direct);
#endif


  // #define OLD_CODE     // define this if you want to try the old method again

#ifdef OLD_CODE
  // OLD
  if (useTSM) {
    td.defineHypercolumn("TiledData",3,
			 stringToVector(MS::columnName(MS::DATA)+","+
					MS::columnName(MS::FLAG)));
  }
#else
  // NEW
  if (useTSM) {    
    td.defineHypercolumn("TiledData",3,
			 stringToVector(MS::columnName(MS::DATA)));
    td.defineHypercolumn("TiledFlag",3,
			 stringToVector(MS::columnName(MS::FLAG)));
    td.defineHypercolumn("TiledUVW",2,
			 stringToVector(MS::columnName(MS::UVW)));
  }
#endif

  if (Debug(1))  cout << "Creating MS=" << MSFileName  << endl;
  SetupNewTable newtab(MSFileName, td, Table::New);
  
  // Set the default Storage Manager to be the Incr one
  IncrementalStMan incrStMan ("ISMData");;
  newtab.bindAll(incrStMan, True);
  // StManAipsIO aipsStMan;  // don't use this anymore
  StandardStMan aipsStMan;  // these are more efficient now


#ifdef OLD_CODE
  // ORIGINAL CODE
  newtab.bindColumn(MS::columnName(MS::ANTENNA2), aipsStMan);
  if (useTSM) {
    // choose a tile size in the channel direction that is <=10
    Int tileSize=(nChan+nChan/10)/(nChan/10+1);
    // make the tile about 32k big
    TiledColumnStMan tiledStMan1("TiledData",
				 IPosition(3,nCorr,tileSize,
					   2000/nCorr/tileSize));
    TiledColumnStMan tiledStMan2("TiledWeight",
				 IPosition(2,tileSize,
					   8000/tileSize));
    // Bind the DATA and FLAG columns to the tiled stman
    newtab.bindColumn(MS::columnName(MS::DATA),tiledStMan1);
    newtab.bindColumn(MS::columnName(MS::FLAG),tiledStMan1);
  }
  // Change some to aipsStMan as they change every row
  newtab.bindColumn(MS::columnName(MS::ANTENNA2),aipsStMan);
  newtab.bindColumn(MS::columnName(MS::UVW),aipsStMan);
  if (!useTSM) {
    newtab.bindColumn(MS::columnName(MS::DATA),aipsStMan);
    newtab.bindColumn(MS::columnName(MS::FLAG),aipsStMan);
  }    
  MeasurementSet ms(newtab);
#else
  //  NEW CODE TO ACCOMODATE VARYING SHAPED COLUMNS 
  if (useTSM) {
    Int tileSize=nChan/10+1;

    TiledShapeStMan tiledStMan1("TiledData",
				 IPosition(3,nCorr,tileSize,
					   16384/nCorr/tileSize));
    TiledShapeStMan tiledStMan1f("TiledFlag",
				 IPosition(3,nCorr,tileSize,
					   16384/nCorr/tileSize));
    TiledShapeStMan tiledStMan2("TiledWeight",
				 IPosition(3,nCorr,tileSize,
					   16384/nCorr/tileSize));
    TiledColumnStMan tiledStMan3("TiledUVW",
				 IPosition(2,3,1024));

    // Bind the DATA and FLAG columns to the tiled stman
    newtab.bindColumn(MS::columnName(MS::DATA),tiledStMan1);
    newtab.bindColumn(MS::columnName(MS::FLAG),tiledStMan1f);
    newtab.bindColumn(MS::columnName(MS::UVW),tiledStMan3);
  } else {
    newtab.bindColumn(MS::columnName(MS::DATA),aipsStMan);
    newtab.bindColumn(MS::columnName(MS::FLAG),aipsStMan);
    newtab.bindColumn(MS::columnName(MS::UVW),aipsStMan);
  }   
  TableLock lock(TableLock::PermanentLocking);
  MeasurementSet ms(newtab,lock);
#endif

  // create all subtables
  // we make new tables with 0 rows
  Table::TableOption option=Table::New;

  // Set up the default subtables for the Bima MS
  ms.createDefaultSubtables(option);

  // Add some optional columns to the required tables
  ms.spectralWindow().addColumn(ArrayColumnDesc<Int>(
MSSpectralWindow::columnName(MSSpectralWindow::ASSOC_SPW_ID),
                MSSpectralWindow::columnStandardComment(
                    MSSpectralWindow::ASSOC_SPW_ID)));

  ms.spectralWindow().addColumn(ArrayColumnDesc<String>(
MSSpectralWindow::columnName(MSSpectralWindow::ASSOC_NATURE),
                MSSpectralWindow::columnStandardComment(
                    MSSpectralWindow::ASSOC_NATURE)));

  ms.spectralWindow().addColumn(ScalarColumnDesc<Int>(
MSSpectralWindow::columnName(MSSpectralWindow::DOPPLER_ID),
                MSSpectralWindow::columnStandardComment(
                    MSSpectralWindow::DOPPLER_ID)));

  // Now setup some optional columns::

  // the SOURCE table, 1 extra optional column needed
  TableDesc sourceDesc = MSSource::requiredTableDesc();
  MSSource::addColumnToDesc(sourceDesc,MSSourceEnums::REST_FREQUENCY,1);
  SetupNewTable sourceSetup(ms.sourceTableName(),sourceDesc,option);
  ms.rwKeywordSet().defineTable(MS::keywordName(MS::SOURCE),
                                     Table(sourceSetup));

  // the DOPPLER table, no optional columns needed
  TableDesc dopplerDesc = MSDoppler::requiredTableDesc();
  SetupNewTable dopplerSetup(ms.dopplerTableName(),dopplerDesc,option);
  ms.rwKeywordSet().defineTable(MS::keywordName(MS::DOPPLER),
                                     Table(dopplerSetup));

  // the SYSCAL table, 1 optional column needed
  TableDesc syscalDesc = MSSysCal::requiredTableDesc();
  MSSysCal::addColumnToDesc(syscalDesc,MSSysCalEnums::TSYS,1);
  SetupNewTable syscalSetup(ms.sysCalTableName(),syscalDesc,option);
  ms.rwKeywordSet().defineTable(MS::keywordName(MS::SYSCAL),
                                     Table(syscalSetup));

  // update the references to the subtable keywords
  ms.initRefs();

  { // Set the TableInfo
    TableInfo& info(ms.tableInfo());
    info.setType(TableInfo::type(TableInfo::MEASUREMENTSET));
    info.setSubType(String("MIRIAD/BIMA"));
    info.readmeAddLine("Made with BimaFiller");
  }                                       

  ms_p=ms;
  msc_p = new MSColumns(ms_p);
} 

void BimaFiller::fillObsTables()
{
  if (Debug(1)) cout << "BimaFiller::fillObsTables" << endl;

  char hline[256];
  Int heof;

  ms_p.observation().addRow();
  MSObservationColumns msObsCol(ms_p.observation());

  msObsCol.telescopeName().put(0,array_p);
  msObsCol.observer().put(0,observer_p);
  msObsCol.project().put(0,project_p);
  if (array_p == "HATCREEK") {
    Vector<String> blog(1);
    blog(0) = "See HISTORY for BIMA observing log";
    msObsCol.log().put(0,blog);
  }

  // should double buffer history, and search for  (e.g.)
  // GPAVER: Executed on: 96SEP12:15:40:48.0
 
  // String date("");
  // if (date=="") date="01/01/00";
  // Double time=date2mjd(date);

  MSHistoryColumns msHisCol(ms_p.history());

  String history;
  Int row=-1;
  hisopen_c(uv_handle_p,"read");
  for (;;) {
    hisread_c(uv_handle_p,hline,256,&heof);
    if (heof) break;
    ms_p.history().addRow(); 
    row++;
    msHisCol.observationId().put(row,0);
    //    msHisCol.time().put(row,time);    // fix the "2000/01/01/24:00:00" bug
    //  nono, better file a report, it appears to be an aips++ problem
    msHisCol.priority().put(row,"NORMAL");
    msHisCol.origin().put(row,"BimaFiller::fillObsTables");
    msHisCol.application().put(row,"bimafiller");
    msHisCol.message().put(row,hline);
  }
  hisclose_c(uv_handle_p);
}

//
// Loop over the visibility data and fill the main table of the MeasurementSet 
// as you find corr/wcorr's
//
void BimaFiller::fillMSMainTable()
{
  if (Debug(1)) cout << "BimaFiller::fillMSMainTable" << endl;

  MSColumns& msc(*msc_p);   // Get access to the MS columns, new way
  Int nCorr = 1;            // # stokes (1 for BIMA for now)
  Int nChan = nchan_p+nwide_p;      // # channels
  Int nCat  = 3;            // # initial flagging categories (fixed at 3)

  Matrix<Complex> vis(nCorr,nChan);
  Vector<Float>   sigma(nCorr);
  Vector<String>  cat(nCat);
  cat(0)="FLAG_CMD";
  cat(1)="ORIGINAL";
  cat(2)="USER";
  msc.flagCategory().rwKeywordSet().define("CATEGORY",cat);
  Cube<Bool> flagCat(nCorr,nChan,nCat,False);  
  Matrix<Bool> flag = flagCat.xyPlane(0); // references flagCat's storage
  Vector<Float> w1(nCorr), w2(nCorr);
  
  nAnt_p.resize(1);
  nAnt_p[0]=0;

  receptorAngle_p.resize(1);
  Int group, i, j, row=-1;
  Double interval;
  Bool lastRowFlag = False;

  if (Debug(1)) cout << "Writing " << nIF_p << " spectral windows" << endl;

  for (group=0; ; group++) {        // loop forever until end-of-file
    int nread, nwread;
    uvread_c(uv_handle_p, preamble, data, flags, MAXCHAN, &nread);

    if (Debug(9)) cout << "UVREAD: " << nread << endl;
    if (nread <= 0) break;          // done with reading miriad data
    if (win.nspect > 0)
        uvwread_c(uv_handle_p, wdata, wflags, MAXCHAN, &nwread);
    else
        nwread=0;
    if (nread != nchan_p) {     // big trouble: data width has changed
      cout << "### Error: Narrow Channel changing from " << nchan_p <<
              " to " << nread << endl;
      break;                    // bail out for now
    }
    if (nwread != nwide_p) {     // big trouble: data width has changed
      cout << "### Error: Wide Channel changing from " << nwide_p <<
              " to " << nwread << endl;
      break;                    // bail out for now
    }

    // append wideband + windows averages to the narrow band data,
    // as well as their flags
    for (i=nread, j=0; j < nwread; j++, i++) {
      data[2*i]   = wdata[2*j];
      data[2*i+1] = wdata[2*j+1];
      flags[2*i]  = wflags[2*j];
    }

    // get time in MJD seconds ; input was in JD
    Double time = (preamble[3] - 2400000.5) * C::day;
    time_p = time;

    if (Debug(3)) {                 // timeline monitoring...
      static Double time0 = -1.0;
      static Double dt0 = -1.0;
#if 0
	// enforce timesteps increasing to test sorting effect 
      if (time0 > 0)
	time = time0 + inttime_p;
      else
	cout << "Warning: faking timesorted data" << endl;
#endif

      MVTime mjd_date(time/C::day);
      mjd_date.setFormat(MVTime::FITS);
      cout << "DATE=" << mjd_date ;
      if (time0 > 0) {
	if (time - time0 < 0) {
	  cout << " BACKWARDS";
	  dt0 = time - time0;
	}
      }
      if (dt0 > 0) {
	if ( (time-time0) > 5*dt0) {
	  cout << " FORWARDS";
	  dt0 = time - time0;
	}
      } else
	dt0 = time-time0;
      time0 = time;
      cout << endl;
    } // debugging 

    interval = inttime_p;
    //msc.interval().put(0,interval);
    //msc.exposure().put(0,interval);

    Float baseline = preamble[4];

    // for miriad, this would always cause a single array dataset,
    // but we need to count the antpos occurences to find out
    // which array configuration we're in.

    if (uvupdate_c(uv_handle_p)) {       // aha, something important changed
        if (Debug(4)) {
            cout << "Record " << group+1 << " uvupdate" << endl;
        }
        Tracking(group);
    } else {
        if (Debug(5)) cout << "Record " << group << endl;
    }


    //  nAnt_p.resize(array+1);
    //  receptorAngle_p.resize(array+1);

    Int ant1 = Int(baseline)/256;              // baseline = 256*A1 + A2
    nAnt_p[nArray_p-1] = max(nAnt_p[nArray_p-1],ant1);   // for MIRIAD, and also 
    Int ant2 = Int(baseline) - ant1*256;       // mostly A1 <= A2
    nAnt_p[nArray_p-1] = max(nAnt_p[nArray_p-1],ant2);
    ant1--; ant2--; // make them 0-based for aips++

    ant1 += nants_offset_p;     // correct for different array offsets
    ant2 += nants_offset_p;


    // should ant1 and ant2 be offset with (nArray_p-1)*nant_p ???
    // in case there are multiple arrays???
    
    Int count = 0;                // index into data[] array
    Vector<Double> uvw(3);
    uvw(0) = -preamble[0] * 1e-9; // convert to seconds
    uvw(1) = -preamble[1] * 1e-9; //  (miriad uses nanosec)
    uvw(2) = -preamble[2] * 1e-9; //
    uvw   *= C::c;                // Convert to meters

    if (group==0 && Debug(1)) {
        cout << "### First record: " << endl;
        cout << "### Preamble: " << preamble[0] << " " <<
                                preamble[1] << " " <<
                                preamble[2] << " nanosec." << endl;
        cout << "### uvw: " << uvw(0) << " " <<
                               uvw(1) << " " <<
                               uvw(2) << " meter." << endl;
    }


    // first construct the data (vis & flag) in a single long array
    // containing all spectral windows
    // In the (optional) loop over all spectral windows, subsets of
    // these arrays will be written out

      for (Int chan=0; chan<nChan; chan++) {
	for (Int pol=0; pol<nCorr; pol++) {

          // miriad uses bl=ant1-ant2, FITS/AIPS use bl=ant2-ant1
          Bool  visFlag =  (flags[count/2] == 0) ? False : True;
	  Float visReal = +data[count]; count++; 
	  Float visImag = -data[count]; count++;
	  Float wt = 1.0;
          if (!visFlag) wt = -wt;

          // check flags array !! need separate counter (count/2)

	  flag(pol,chan) = (wt<=0); 
          vis(pol,chan) = Complex(visReal,visImag);
	} // pol
      } // chan



    for (Int ifno=0; ifno < nIF_p; ifno++) {
      if (Qsplit_p && win.keep[ifno]==0) continue;    
      // IFs go to separate rows in the MS, pol's do not!
      ms_p.addRow(); 
      row++;

      // first fill in values for all the unused columns
      if (row==0) {
	msc.feed1().put(row,0);
	msc.feed2().put(row,0);
	msc.flagRow().put(row,False);
	lastRowFlag = False;
	msc.scanNumber().put(row,0);
	msc.processorId().put(row,-1);
	msc.observationId().put(row,0);
	msc.stateId().put(row,-1);
	if (!Qtsys_p) {
	  Vector<Float> tmp(nCorr); tmp=1.0;
	  msc.weight().put(row,tmp);
	  msc.sigma().put(row,tmp);
	}
      }
      msc.exposure().put(row,interval);
      msc.interval().put(row,interval);
      if (Qsplit_p) {
#if 1
	// the dumb way: e.g. 3" -> 20" for 3c273
	Matrix<Complex> tvis(nCorr,win.nschan[ifno]);
	Cube<Bool> tflagCat(nCorr,win.nschan[ifno],nCat,False);  
	Matrix<Bool> tflag = tflagCat.xyPlane(0); // references flagCat's storage

	Int woffset = win.ischan[ifno]-1;
	Int wsize   = win.nschan[ifno];
	for (Int i=0; i< wsize; i++) {
	  tvis(0,i) = vis(0,i+woffset);
	  tflag(0,i) = flag(0,i+woffset);
	}
#else
	// the 'smart' way,  using IPositions (still 20"....)
	IPosition blc(2,0,0);
	IPosition trc(2,nCorr-1,win.nschan[ifno]-1);
	IPosition offset(2,0,win.ischan[ifno]-1);
	Matrix<Complex> tvis(nCorr,win.nschan[ifno]);
	Cube<Bool> tflagCat(nCorr,win.nschan[ifno],nCat,False);  
	Matrix<Bool> tflag = tflagCat.xyPlane(0); // references flagCat's storage

	tvis(blc,trc) = vis(blc+offset,trc+offset);
	tflag(blc,trc) = flag(blc+offset,trc+offset);
#endif
	msc.data().put(row,tvis);
	msc.flag().put(row,tflag);
	msc.flagCategory().put(row,tflagCat);
      } else {
	msc.data().put(row,vis);
	msc.flag().put(row,flag);
	msc.flagCategory().put(row,flagCat);
      }

      Bool rowFlag = allEQ(flag,True);
      if (rowFlag != lastRowFlag) {
	msc.flagRow().put(row,rowFlag);
	lastRowFlag = rowFlag;
      }

      msc.antenna1().put(row,ant1);
      msc.antenna2().put(row,ant2);
      msc.time().put(row,time);           // BIMA does begin of scan..
      msc.timeCentroid().put(row,time);   // do we really need this ?
      if (Qtsys_p) {	
	// Vector<Float> w1(nCorr), w2(nCorr);
	w2 = 1.0;   // i use this as a 'version' id  to test FC refresh bugs :-)
	if( systemp[ant1] == 0 || systemp[ant2] == 0) {
	  zero_tsys++;
	  w1 = 0.0;
	} else
	  w1 = 1.0/sqrt((double)(systemp[ant1]*systemp[ant2]));
	if (Debug(1)) cout << w1 << " " << w2 << endl;
	msc.weight().put(row,w1);
	msc.sigma().put(row,w2);	
      }
      msc.uvw().put(row,uvw);
      msc.arrayId().put(row,nArray_p-1);

      if (Qsplit_p)
	msc.dataDescId().put(row,ifno);
      else
	msc.dataDescId().put(row,0);
    
      msc.fieldId().put(row,ifield);

    }  // ifNo
  } // loop over all visibilities

  cout << infile_p << ": Processed " << group << " visibilities." << endl;
  cout << "Found " << npoint << " pointings with "
       <<  nfield << " unique mosaiced fields and "
       <<  nArray_p << " arrays." 
       << endl;
  cout << "nAnt_p contains: " << nAnt_p.nelements() << endl;

#if 0
  // fill the receptorAngle with defaults, just in case there is no AN table
  for (Int arr=0; arr<nAnt_p.nelements(); arr++) {
    Vector<Double> angle(2*nAnt_p[arr]); 
    angle=0;
    receptorAngle_p[arr]=angle;
  }
#endif
} // fillMSMainTable

void BimaFiller::fillAntennaTable()
{
  if (Debug(1)) cout << "BimaFiller::fillAntennaTable" << endl;
  Int nAnt=nants_p;

#if 0
  Int array = nArray_p;
  // we don't have 'array' yet, and nAnt_p isnt' big enough....
  if (nAnt_p[array]>MAXANT)
    throw(AipsError("Too many antennas -- should never occur"));
  if (nAnt_p[array]>nants_p)
    throw(AipsError("Not all antennas found in antenna table:"));


  receptorAngle_p[array].resize(2*nAnt);
#endif

  // Vector<Double> arrayXYZ(3);
  arrayXYZ_p.resize(3);
  if (array_p == "HATCREEK" || array_p == "BIMA") {     // Array center:
      arrayXYZ_p(0) = -2523862.04;
      arrayXYZ_p(1) = -4123592.80;
      arrayXYZ_p(2) =  4147750.37;
  } else if (array_p == "ATCA") {
      arrayXYZ_p(0) = -4750915.84;
      arrayXYZ_p(1) =  2792906.18;
      arrayXYZ_p(2) = -3200483.75;
  } else if (array_p == "OVRO") {
      arrayXYZ_p(0) = -2523862.04;     // TODO: change this to CARMA location
      arrayXYZ_p(1) = -4123592.80;
      arrayXYZ_p(2) =  4147750.37;
  } else {
      cout << "Warning: unknown array position for "<<array_p<<endl;
      arrayXYZ_p = 0.0;
  }
  if(Debug(3)) cout << "number of antennas ="<<nAnt<<endl;
  if(Debug(3)) cout << "array ref pos:"<<arrayXYZ_p<<endl;

  String timsys = "TAI";  // assume, for now .... 

  // store the time keywords ; again, miriad doesn't have this (yet)
  // check w/ uvfitsfiller again

  //save value to set time reference frame elsewhere
  timsys_p=timsys;

  // Antenna diamater:
  // Should check the 'antdiam' UV variable, but it doesn't appear to 
  // exist in our BIMA datasets.
  // So, fill in some likely values
  Float diameter=25;                        //# most common size (:-)
  if (array_p=="ATCA")     diameter=22;     //# only at 'low' freq !!
  if (array_p=="HATCREEK") diameter=6;
  if (array_p=="BIMA")     diameter=6;
  if (array_p=="CARMA")    diameter=8;
  if (array_p=="OVRO")     diameter=10;

  if (nAnt == 15 && array_p=="OVRO")
    cout << "CARMA array (6 OVRO, 15 BIMA) assumed" << endl;
  else
    cout << "Ant configuration not supported yet";

  MSAntennaColumns& ant(msc_p->antenna());
  Vector<Double> antXYZ(3);

  // add antenna info to table
  if (nArray_p == 0) {                   // check if needed
    ant.setPositionRef(MPosition::ITRF);
  }
  Int row=ms_p.antenna().nrow()-1;

  if (Debug(2)) cout << "BimaFiller::fillAntennaTable row=" << row+1 
       << " array " << nArray_p+1 << endl;

  for (Int i=0; i<nAnt; i++) {

    ms_p.antenna().addRow(); 
    row++;

    if (i<6)
      ant.dishDiameter().put(row,10.4);  // OVRO
    else if (i<15)
      ant.dishDiameter().put(row,6.1);   // BIMA or HATCREEK
    else
      ant.dishDiameter().put(row,3.5);   // SZA
    
    antXYZ(0) = antpos[i];              //# these are now in nano-sec
    antXYZ(1) = antpos[i+nAnt];
    antXYZ(2) = antpos[i+nAnt*2];
    antXYZ *= 1e-9 * C::c;;             //# and now in meters
    if (Debug(2)) cout << "Ant " << i+1 << ":" << antXYZ << " (m)." << endl;

    String mount;                           // really should consult
    switch (mount_p) {                 	    // the "mount" uv-variable
      case  0: mount="ALT-AZ";      break;
      case  1: mount="EQUATORIAL";  break;
      case  2: mount="X-Y";         break;
      case  3: mount="ORBITING";    break;
      case  4: mount="BIZARRE";     break;
      // case  5: mount="SPACE-HALCA"; break;
      default: mount="UNKNOWN";     break;
    }
    ant.mount().put(row,mount);
    ant.flagRow().put(row,False);
    ant.name().put(row,"ANT" + String::toString(i+1));
    ant.station().put(row,"UNKNOWN");	// station names unknown at HatCreek (miriad really)
    ant.type().put(row,"GROUND-BASED");

    Vector<Double> offsets(3);
    offsets=0.0; 
    // store absolute positions, all offsets 0
    ant.position().put(row,arrayXYZ_p+antXYZ);
    ant.offset().put(row,offsets);

    // store the angle for use in the feed table
//    receptorAngle_p[array](2*i+0)=polangleA(i)*C::degree;
//    receptorAngle_p[array](2*i+1)=polangleB(i)*C::degree;
  }
  // ant.position().rwKeywordSet().define("MEASURE_REFERENCE","ITRF");

  nArray_p++;
  nAnt_p.resize(nArray_p);
  nAnt_p[nArray_p-1] = 0;
  if (Debug(3) && nArray_p > 1)
    cout << "DEBUG0 :: " << nAnt_p[nArray_p-2] << endl;
  
  if (nArray_p > 1) return;

  // now do some things which only need to happen the first time around

  // store these items in non-standard keywords for now
  // 
  String arrnam = "BIMA";   // for now only "support" BIMA data
  ant.name().rwKeywordSet().define("ARRAY_NAME",arrnam);
  ant.position().rwKeywordSet().define("ARRAY_POSITION",arrayXYZ_p);


  // fill the array table entry
  // this assumes there is one AN table for each (sub)array index encountered.

  //PJT ms_p.array().addRow();
  // array is now gone, there is an array_id in the main MS table for 
  // id purposes.  We store the ARRAY_POSITION as a non-standard keyword
  // with the POSITION collumn in the ANTENNA table (see above)
#if 0
  MSArrayColumns arr(ms_p.array());
  arr.name().put(array,array_p);
  arr.position().put(array,arrayXYZ);
  arr.position().rwKeywordSet().define("MEASURE_REFERENCE","ITRF");
#endif
} // fillAntennaTable

void BimaFiller::fillSyscalTable()
{
  if (Debug(1)) cout << "BimaFiller::fillSyscalTable" << endl;

  MSSysCalColumns&     msSys(msc_p->sysCal());
  Vector<Float> Systemp(1);    // should we set both receptors same?
  static Int row = -1;

  if (Debug(1)) 
    for (Int i=0; i<nants_p; i++)
      cout  << "SYSTEMP: " << i << ": " << systemp[i] << endl;

  
  for (Int i=0; i<nants_p; i++) {
    ms_p.sysCal().addRow();
    row++;  // should be a static, since this routine will be called again

    msSys.antennaId().put(row,i);   //  i, or i+nants_offset_p ????
    msSys.feedId().put(row,0);
    msSys.spectralWindowId().put(row,-1);    // all of them for now .....
    msSys.time().put(row,time_p);
    msSys.interval().put(row,-1.0);
    
    Systemp(0) = systemp[i];
    msSys.tsys().put(row,Systemp);
  }
 


  // this may actually be a nasty problem for BIMA datasets that are not
  // timesorted. A temporary table needs to be written with all records,
  // which then needs to be sorted and 'recomputed'
}

void BimaFiller::fillSpectralWindowTable()
{
  if (Debug(1)) cout << "BimaFiller::fillSpectralWindowTable" << endl;

  MSSpWindowColumns&      msSpW(msc_p->spectralWindow());
  MSDataDescColumns&      msDD(msc_p->dataDescription());
  MSPolarizationColumns&  msPol(msc_p->polarization());
  MSDopplerColumns&       msDop(msc_p->doppler());

  Int iFreq = 0;
  Int nChan = nchan_p + nwide_p;
  Int nCorr = 1;            // only 1 polarization 
  Int iChan;
  Int spw=0;
  Int i, j, side;
  Double BW = 0.0;

  MDirection::Types dirtype = epochRef_p;    // MDirection::B1950 or MDirection::J2000;
  MEpoch ep(Quantity(time_p, "s"), MEpoch::UTC);
#if 0
  // ERROR::   type specifier omitted for parameter 
  MPosition obspos(MVPosition(arrayXYZ_p), MPosition::ITRF);
#else
  MVPosition here(arrayXYZ_p);
  MPosition obspos(here, MPosition::ITRF);
#endif
  MDirection dir(Quantity(ra_p, "rad"), Quantity(dec_p, "rad"), dirtype);
  MeasFrame frame(ep, obspos, dir);
  MFrequency::Convert tolsr(MFrequency::TOPO, 
			    MFrequency::Ref(MFrequency::LSRD, frame));

  // fill out the polarization info (only 1 entry allowed for now)
  ms_p.polarization().addRow();
  msPol.numCorr().put(0,nCorr);
  msPol.corrType().put(0,corrType_p);
  msPol.corrProduct().put(0,corrProduct_p);
  msPol.flagRow().put(0,False);

  // fill out doppler table (only 1 entry needed, BIMA data only identify 1 line :-(
  cout << "BimaFiller:: now writing Doppler table " << endl;
  ms_p.doppler().addRow();
  msDop.dopplerId().put(0,0);
  msDop.sourceId().put(0,0);
  msDop.transitionId().put(0,0);
  msDop.velDefMeas().put(0,MDoppler(Quantity(0),MDoppler::RADIO));

  MFrequency::Types freqsys_p = MFrequency::TOPO;    // observer centric

  if (!Qsplit_p) {       // All spectral windows glued in one row

    ms_p.spectralWindow().addRow();
    ms_p.dataDescription().addRow();

    msDD.spectralWindowId().put(spw,spw);
    msDD.polarizationId().put(spw,0);
    msDD.flagRow().put(spw,False);

    msSpW.name().put(spw,"none");
    msSpW.freqGroupName().put(spw,"ALL-OF-THEM");
    msSpW.ifConvChain().put(spw,0);
    msSpW.numChan().put(spw,nChan);
    msSpW.dopplerId().put(spw,0);    // BIMA has only 1 ref freq line

    Vector<Double> chanFreq(nChan), resolution(nChan);

    // note these are sky frequencies for BIMA, and slowly change !!!!
    // need to find a better solution for this... i.e. compute
    // them here on the fly, and pass them on, which would make 
    // the AIPS++ tables prohibitively expensive (large), or teach
    // AIPS++ how to compute them on  the fly if we only give limited
    // info

    iChan = 0;    // counter of channels accross various types of windows

    // narrow band channels first

    for (i=0; i < win.nspect; i++) {          // Narrow band channels
      Double fwin = win.sfreq[i]*1e9;            // miriad uses Ghz, AIPS++ Hz
      if (Debug(1)) cout << "Fwin: OBS=" << fwin/1e9;
      fwin = tolsr(fwin).getValue().getValue();
      if (Debug(1)) cout << " LSR=" << fwin/1e9 << endl;
      for (j=0; j < win.nschan[i]; j++) {
        // could also use iChan if you want to preserve data order ....
        iFreq = win.ischan[i]+j-1;		// 0-based
        chanFreq(iFreq) = fwin + j * win.sdf[i];
        resolution(iFreq) = abs(win.sdf[i]*1e9);  // ?? what if hanning ???
        BW += resolution(iChan);                  // probably not quite correct
        iChan++;
      }
    }

    // wide and window averages come last 

    for (i=0; i < win.nwide; i++) {           // Wide band channels
      Double fwin = win.wfreq[i]*1e9;
      fwin = tolsr(fwin).getValue().getValue();
      chanFreq(iChan) = fwin;
      resolution(iChan) = abs(win.wwidth[i]*1e9);
      BW += resolution(iChan);                // not good if mixing win & wide
      iChan++;
    }

    msSpW.chanFreq().put(spw,chanFreq);
    msSpW.refFrequency().put(spw,freq_p);            // just a reference, not used
    msSpW.resolution().put(spw,resolution);
    msSpW.chanWidth().put(spw,resolution);
    msSpW.effectiveBW().put(spw,resolution);
    msSpW.totalBandwidth().put(spw,BW);
    // set the reference frames for frequency 
    // can also do it implicitly via Measures you give to the freq's
    msSpW.measFreqRef().put(spw,freqsys_p);

  } else {

    for (i=0; i < win.nspect + win.nwide; i++) {
      Int n = win.nschan[i];
      Vector<Double> f(n), w(n);

      ms_p.spectralWindow().addRow();
      ms_p.dataDescription().addRow();

      msDD.spectralWindowId().put(i,i);
      msDD.polarizationId().put(i,0);
      msDD.flagRow().put(i,False);

      msSpW.numChan().put(i,win.nschan[i]);
      BW = 0.0;
      Double fwin = win.sfreq[i]*1e9;
      if (Debug(1)) cout << "Fwin: OBS=" << fwin/1e9;
      fwin = tolsr(fwin).getValue().getValue();
      if (Debug(1)) cout << " LSR=" << fwin/1e9 << endl;
      for (j=0; j < win.nschan[i]; j++) {
	f(j) = fwin + j * win.sdf[i] * 1e9;
	w(j) = abs(win.sdf[i]*1e9);
	BW += w(j);
      }

      msSpW.chanFreq().put(i,f);
      msSpW.refFrequency().put(i,freq_p);            // ???
      msSpW.resolution().put(i,w);
      msSpW.chanWidth().put(i,w);
      msSpW.effectiveBW().put(i,w);
      msSpW.totalBandwidth().put(i,BW);
      msSpW.ifConvChain().put(i,0);
      // can also do it implicitly via Measures you give to the freq's
      msSpW.measFreqRef().put(i,freqsys_p);

      if (win.sdf[i] > 0)      side = 1;
      else if (win.sdf[i] < 0) side = -1;
      else                     side = 0;

      switch (win.code[i]) {
      case 'N':
	msSpW.netSideband().put(i,side);
	msSpW.freqGroup().put(i,1);
	msSpW.freqGroupName().put(i,"MULTI-CHANNEL-DATA");
	break;
      case 'W':
	msSpW.netSideband().put(i,side);
	msSpW.freqGroup().put(i,3);
	msSpW.freqGroupName().put(i,"SIDE-BAND-AVERAGE");
	break;
      case 'S':
	msSpW.netSideband().put(i,side);
	msSpW.freqGroup().put(i,2);
	msSpW.freqGroupName().put(i,"MULTI-CHANNEL-AVG");
	break;
      default:
	throw(AipsError("Bad code for a spectral window"));
	break;
      }
    }
  } 

  // set the reference frames for frequency
  // msSpW.chanFreq().rwKeywordSet().define("MEASURE_REFERENCE","TOPO");

  //PJT msSpW.restFrequency().rwKeywordSet().define("MEASURE_REFERENCE","REST");
  // msSpW.refFrequency().rwKeywordSet().define("MEASURE_REFERENCE","TOPO");
}

void BimaFiller::fillFieldTable()
{
  if (Debug(1)) cout << "BimaFiller::fillFieldTable" << endl;

  // set the DIRECTION MEASURE REFERENCE for appropriate columns
  // but note we're not varying them accross rows
  /// MDirection::Types epochRef=MDirection::J2000;
  /// if (nearAbs(epoch_p,1950.0,0.01)) epochRef=MDirection::B1950;   
  msc_p->setDirectionRef(epochRef_p);

  MSFieldColumns& msField(msc_p->field());

  Vector<Double> radec(2), pm(2);
  Vector<MDirection> radecMeas(1);
  Int fld;
  Double cosdec;

  pm = 0;                       // Proper motion is zero

  if (nfield == 0) {            // if no pointings found, say there is 1
    cout << "Warning: no dra/ddec pointings found, creating 1." << endl;
    nfield = npoint = 1;
    dra[0] = ddec[0] = 0.0;
  } 
  cosdec = cos(dec_p);

  for (fld = 0; fld < nfield; fld++) {

    ms_p.field().addRow();

    msField.sourceId().put(fld,-1);     // source table not yet used
    msField.code().put(fld," ");
    msField.name().put(fld,object_p);   // hmm ??
    msField.numPoly().put(fld,0);

    radec(0) = ra_p + dra[fld]/cosdec;            // RA, in radians
    radec(1) = dec_p + ddec[fld];                 // DEC, in radians

    radecMeas(0).set(MVDirection(radec(0), radec(1)), MDirection::Ref(epochRef_p));

    msField.delayDirMeasCol().put(fld,radecMeas);
    msField.phaseDirMeasCol().put(fld,radecMeas);
    msField.referenceDirMeasCol().put(fld,radecMeas);

    // Need to convert epoch in years to MJD time
    if (nearAbs(epoch_p,2000.0,0.01)) {
      msField.time().put(fld, MeasData::MJD2000*C::day);
      // assume UTC epoch
    } else if (nearAbs(epoch_p,1950.0,0.01)) {
      msField.time().put(fld, MeasData::MJDB1950*C::day);
    } else {
      cerr << " Cannot handle epoch "<< epoch_p <<endl;
    }
  }
}

void BimaFiller::fillSourceTable()
{
  if (Debug(1)) cout << "BimaFiller::fillSourceTable" << endl;
  Int maxSrc = 1;     // call this method for each new source
  Int n = win.nspect;

  MSSourceColumns& msSource(msc_p->source());

  Vector<Double> radec(2);

  //String key("MEASURE_REFERENCE");
  //msSource.restFrequency().rwKeywordSet().define(key,"REST");

  // 
  for (Int src=0; src < maxSrc; src++) {
    ms_p.source().addRow();

    radec(0) = ra_p;
    radec(1) = dec_p;

    msSource.sourceId().put(src,src);
    msSource.name().put(src,object_p);
    //    msSource.spectralWindowId().put(src,-1);     // really valid for all ??
    msSource.spectralWindowId().put(src,0);     // FIX it due to a bug in MS2 code (6feb2001)
    msSource.direction().put(src,radec);
    if (n > 0) {
      Vector<Double> restFreq(n);
      for (Int i=0; i<n; i++)
	restFreq(i) = win.restfreq[i] * 1e9;    // convert from GHz to Hz

      msSource.numLines().put(src,win.nspect);
      msSource.restFrequency().put(src,restFreq);
    }
    msSource.time().put(src,0.0);               // valid for all times
    msSource.interval().put(src,0);             // valid forever

  }
}

void BimaFiller::fillFeedTable() 
{
  if (Debug(1)) cout << "BimaFiller::fillFeedTable" << endl;

  MSFeedColumns msfc(ms_p.feed());

  // find out the POLARIZATION_TYPE
  // In the fits files we handle there can be only a single, uniform type
  // of polarization so the following should work.
  MSPolarizationColumns& msPolC(msc_p->polarization());

  Int numCorr=msPolC.numCorr()(0);
  Vector<String> rec_type(2); rec_type="";
  if (corrType_p(0)>=Stokes::RR && corrType_p(numCorr-1)<=Stokes::LL) {
      rec_type(0)="R"; rec_type(1)="L";
  }
  if (corrType_p(0)>=Stokes::XX && corrType_p(numCorr-1)<=Stokes::YY) {
      rec_type(0)="X"; rec_type(1)="Y";
  }

  Matrix<Complex> polResponse(2,2); 
  polResponse=0.; polResponse(0,0)=polResponse(1,1)=1.;
  Matrix<Double> offset(2,2); offset=0.;
  Vector<Double> position(3); position=0.;
  Vector<Double> ra(2);
  ra = 0.0;

  // fill the feed table
  // will only do UP TO the largest antenna referenced in the dataset
  Int row=-1;
  if (Debug(3)) cout << "DEBUG1 :: " << nAnt_p.nelements() << endl;
  for (Int arr=0; arr< (Int)nAnt_p.nelements(); arr++) {   
    if (Debug(3)) cout << "DEBUG2 :: " << nAnt_p[arr] << endl; 
    for (Int ant=0; ant<nAnt_p[arr]; ant++) {
      ms_p.feed().addRow(); row++;

      msfc.antennaId().put(row,ant);
      msfc.beamId().put(row,-1);
      msfc.feedId().put(row,0);
      msfc.interval().put(row,DBL_MAX);

      // msfc.phasedFeedId().put(row,-1);    // now optional
      msfc.spectralWindowId().put(row,-1); 
      msfc.time().put(row,0.);
      msfc.numReceptors().put(row,2);
      msfc.beamOffset().put(row,offset);
      msfc.polarizationType().put(row,rec_type);
      msfc.polResponse().put(row,polResponse);
      msfc.position().put(row,position);
      // fix these when incremental array building is ok.
      // although for BIMA this would never change ....
      msfc.receptorAngle().put(row,ra);
      // msfc.receptorAngle().put(row,receptorAngle_p[arr](Slice(2*ant,2)));
    }
  }      
}

void BimaFiller::fixEpochReferences() {

  if (Debug(1)) cout << "BimaFiller::fixEpochReferences" << endl;

  if (timsys_p=="IAT") timsys_p="TAI";
  if (timsys_p=="UTC" || timsys_p=="TAI") {
    String key("MEASURE_REFERENCE");
    MSColumns msc(ms_p);
    msc.time().rwKeywordSet().define(key,timsys_p);
    msc.feed().time().rwKeywordSet().define(key,timsys_p);
    msc.field().time().rwKeywordSet().define(key,timsys_p);
    // Fits obslog time is probably local time instead of TAI or UTC
    //PJT msc.obsLog().time().rwKeywordSet().define(key,timsys_p);
  } else {
    if (timsys_p!="")
      cerr << "Unhandled time reference frame: "<<timsys_p<<endl;
  }
}

//
// track some important uv variables to get notified when they change
// 
void BimaFiller::Tracking(int record)
{
  if (Debug(3)) cout << "BimaFiller::Tracking" << endl;

  char vtype[10], vdata[10];
  int vlen, vupd, idat, vupd1, i, j;
  //  float dx, dy;
  //  Float rdat;
  //  Double ddat;

  if (record < 0) {                 // first time around: set variables to track
    uvtrack_c(uv_handle_p,"nschan","u");   // narrow lines
    uvtrack_c(uv_handle_p,"nspect","u");   // window averages
    uvtrack_c(uv_handle_p,"ischan","u");
    uvtrack_c(uv_handle_p,"sdf","u");
    uvtrack_c(uv_handle_p,"sfreq","u");    // changes a lot (doppler)

    uvtrack_c(uv_handle_p,"restfreq","u"); // never really changes....
    uvtrack_c(uv_handle_p,"freq","u");     // never really changes....


    uvtrack_c(uv_handle_p,"nwide","u");
    uvtrack_c(uv_handle_p,"wfreq","u");
    uvtrack_c(uv_handle_p,"wwidth","u");

    uvtrack_c(uv_handle_p,"antpos","u");   // array's
    uvtrack_c(uv_handle_p,"pol","u");      // pol's
    uvtrack_c(uv_handle_p,"dra","u");      // fields
    uvtrack_c(uv_handle_p,"ddec","u");     // fields

    uvtrack_c(uv_handle_p,"inttime","u");

    // weather:
    // uvtrack_c(uv_handle_p,"airtemp","u");
    // uvtrack_c(uv_handle_p,"dewpoint","u");
    // uvtrack_c(uv_handle_p,"relhumid","u");
    // uvtrack_c(uv_handle_p,"winddir","u");
    // uvtrack_c(uv_handle_p,"windmph","u");

    return;
  }

  // here is all the special tracking code...
  uvprobvr_c(uv_handle_p,"source",vtype,&vlen,&vupd);
  if (vupd) {
    uvgetvr_c(uv_handle_p,H_BYTE,"source",vdata,10);
    cout << "Source changed: " << vdata << endl;
    if (source_p.nelements()==0) {
      source_p.resize(1);
      source_p[0] = vdata;
    } else if (source_p.nelements()==1) {
      source_p.resize(2);
      source_p[1] = vdata;
    } else {
      cout << "WARNING: cannot handle > 2 sources" << endl;
    }
  }


  uvprobvr_c(uv_handle_p,"pol",vtype,&vlen,&vupd);
  if (vupd && npol_p==1) {
    uvrdvr_c(uv_handle_p,H_INT,"pol",(char *)&idat, NULL, 1);
    if (idat != pol_p)
        cout << "Warning: polarization changed to " << pol_p << endl;
    pol_p = idat;
  }

  uvprobvr_c(uv_handle_p,"npol",vtype,&vlen,&vupd);
  if (vupd) {
    uvrdvr_c(uv_handle_p,H_INT,"npol",(char *)&idat, NULL, 1);
    if (idat != npol_p)
      throw(AipsError("Cannot handle a changing npol yet"));
  }

  uvprobvr_c(uv_handle_p,"inttime",vtype,&vlen,&vupd);
  if (vupd) {
    uvgetvr_c(uv_handle_p,H_REAL,"inttime",(char *)&inttime_p,1);
  }

  uvprobvr_c(uv_handle_p,"antpos",vtype,&vlen,&vupd);
  if (vupd && record) {
    if (Qarrays_p) 
      nants_offset_p += nants_p;      // increment from size of previous array
    uvgetvr_c(uv_handle_p,H_INT, "nants", (char *)&nants_p,1);
    uvgetvr_c(uv_handle_p,H_DBLE,"antpos",(char *)antpos,3*nants_p);
    if (Debug(2)) {
      cout << "Found " << nants_p << " antennas for array " 
	   << nArray_p << endl;
      for (int i=0; i<nants_p; i++) {
        cout << antpos[i] << " " << 
                antpos[i+nants_p] << " " << 
                antpos[i+nants_p*2] << endl;
      }
    }
    if (Debug(2)) cout << "Warning: antpos changed at record " << record << endl;
    if (Qarrays_p)
      fillAntennaTable();
  }

  if (win.nspect > 0) {
    uvprobvr_c(uv_handle_p,"systemp",vtype,&vlen,&vupd);  
    if (vupd) {
      uvgetvr_c(uv_handle_p,H_REAL,"systemp",(char *)systemp,nants_p*win.nspect);
      if (Debug(3)) {
	cout << "Found systemps (new scan)" ;
	for (Int i=0; i<nants_p; i++)  cout << systemp[i] << " ";
	cout << endl;
      }
    }
  } else {
    uvprobvr_c(uv_handle_p,"wsystemp",vtype,&vlen,&vupd);  
    if (vupd) {
      uvgetvr_c(uv_handle_p,H_REAL,"wsystemp",(char *)systemp,nants_p);
      if (Debug(3)) {
	cout << "Found wsystemps (new scan)" ;
	for (Int i=0; i<nants_p; i++)  cout << systemp[i] << " ";
	cout << endl;
      }
    }
  }

  uvprobvr_c(uv_handle_p,"dra", vtype,&vlen,&vupd);
  uvprobvr_c(uv_handle_p,"ddec",vtype,&vlen,&vupd1);
  if (vupd || vupd1) {
    npoint++;
    uvrdvr_c(uv_handle_p,H_REAL,"dra",(char *)&dra_p, NULL, 1);
    uvrdvr_c(uv_handle_p,H_REAL,"ddec",(char *)&ddec_p, NULL, 1);

    for (i=0, j=-1; i<nfield; i++) { // check if we had this pointing before
        if (dra[i] == dra_p && ddec[i] == ddec_p) {
            ifield = j = i;
            break;
        } 
    }

    if (j<0) {                      // no, we have a new field;
      j = ifield = nfield++;        // j>=0 is new index
      if (Debug(2)) cout << "Adding new field " << ifield 
	    << " at " 
            << dra_p *206264.8062 << " " 
            << ddec_p*206264.8062 << " arcsec." << endl;
      if (nfield >= MAXFIELD) {
	cout << "Cannot handle more than " << MAXFIELD << " fields." << endl;
	exit(1);
      }
      dra[ifield]  = dra_p;
      ddec[ifield] = ddec_p;
    }
    if (Debug(3)) cout << "Warning: pointing " << j 
        << " (dra/ddec) changed at record " << record << " : " 
        << dra_p *206264.8062 << " " 
        << ddec_p*206264.8062 << endl;
  }
}

//
//  this is also a nasty routine. It makes assumptions on
//  a relationship between narrow and window averages
//  which normally exists for BIMA telescope data, but which
//  in principle can be modified by uvcat/uvaver and possibly
//  break this routine...
//  (there has been some talk at the site to write subsets of
//   the full data, which could break this routine)

void BimaFiller::init_window(Block<Int>& narrow, Block<Int>& wide, Block<Int>& window)
{
  if (Debug(1)) cout << "BimaFiller::init_window" << endl;

  char vtype[10];
  int i, j, k, idx, vlen, vupd, nchan, nspect, nwide, cormode;

  uvprobvr_c(uv_handle_p,"nchan",vtype,&vlen,&vupd);
  if (vupd) {
    uvrdvr_c(uv_handle_p,H_INT,"nchan",(char *)&nchan, NULL, 1);
  } else {
    nchan = 0;
    if (Debug(1)) cout << "nchan = 0" << endl;
  }

  uvprobvr_c(uv_handle_p,"nspect",vtype,&vlen,&vupd);
  if (vupd) {
    uvrdvr_c(uv_handle_p,H_INT,"nspect",(char *)&nspect, NULL, 1);
    win.nspect = nspect;
  } else
    win.nspect = nspect = 0;

  uvprobvr_c(uv_handle_p,"nwide",vtype,&vlen,&vupd);
  if (vupd) {
    uvrdvr_c(uv_handle_p,H_INT,"nwide",(char *)&nwide, NULL, 1);
    win.nwide = nwide;
  } else
    win.nwide = nwide = 0;

  uvprobvr_c(uv_handle_p,"cormode",vtype,&vlen,&vupd);
  if (vupd) {
    uvrdvr_c(uv_handle_p,H_INT,"cormode",(char *)&cormode, NULL, 1);
  } else
    cormode = 0;

  if (nspect > 0 && nspect <= MAXWIN) {

    uvprobvr_c(uv_handle_p,"ischan",vtype,&vlen,&vupd);
    if (vupd)
      uvgetvr_c(uv_handle_p,H_INT,"ischan",(char *)win.ischan, nspect);
    else if (nspect==1)
      win.ischan[0] = 1;
    else
      throw(AipsError("missing ischan"));

    uvprobvr_c(uv_handle_p,"nschan",vtype,&vlen,&vupd);
    if (vupd)
      uvgetvr_c(uv_handle_p,H_INT,"nschan",(char *)win.nschan, nspect);
    else if (nspect==1)
      win.nschan[0] = nchan_p;
    else
      throw(AipsError("missing nschan"));

    uvprobvr_c(uv_handle_p,"restfreq",vtype,&vlen,&vupd);
    if (vupd)
      uvgetvr_c(uv_handle_p,H_DBLE,"restfreq",(char *)win.restfreq, nspect);
    else
      throw(AipsError("missing restfreq"));

    uvprobvr_c(uv_handle_p,"sdf",vtype,&vlen,&vupd);
    if (vupd)
      uvgetvr_c(uv_handle_p,H_DBLE,"sdf",(char *)win.sdf, nspect);
    else if (nspect>1)
      throw(AipsError("missing sdf"));

    uvprobvr_c(uv_handle_p,"sfreq",vtype,&vlen,&vupd);
    if (vupd)
      uvgetvr_c(uv_handle_p,H_DBLE,"sfreq",(char *)win.sfreq, nspect);
    else
      throw(AipsError("missing sfreq"));
  }

  if (nwide > 0 && nwide <= MAXWIDE) {
    uvprobvr_c(uv_handle_p,"wfreq",vtype,&vlen,&vupd);
    if (vupd)
      uvgetvr_c(uv_handle_p,H_REAL,"wfreq",(char *)win.wfreq, nwide);
    uvprobvr_c(uv_handle_p,"wwidth",vtype,&vlen,&vupd);
    if (vupd)
      uvgetvr_c(uv_handle_p,H_REAL,"wwidth",(char *)win.wwidth, nwide);
  }

  idx = (nspect > 0 ? nspect : 0);           // idx points into the combined win.xxx[] elements
  for (i=0; i<nspect; i++) {
    win.code[i] = 'N';
    win.keep[i] = 1;
  }
  if (nspect > 0 && narrow[0] > 0) {         // fix up the keep[] array from the narrow= keyword
    for (j=0; j<nspect; j++)
      win.keep[j] = 0;
    for (j=0; j<narrow.nelements(); j++) {
      k = narrow[j]-1;
      if (k >= 0 || k < nspect)
	win.keep[k] = 1;
      else
	cout << "### Warning: bad narrow spectral window id " << k+1 << endl;
    }
  }
  Int side = 0;    // sideband (-1 = LSB    1 = USB)
  for (i=0; i<nwide; i++) {
    if (cormode > 0) {    // special for BIMA: nspect = 2*cormode
       if (i==0 || (i>1 && i<cormode+2))
	 side = -1;
       else
	 side = 1;
    }
    if (Debug(1)) cout << "Processing wide into window defs " << i << " side " << side << endl;
    win.code[idx] = (i<2 ? 'W' : 'S');
    win.keep[idx] = 1;
    win.ischan[idx] = nchan + i + 1;
    win.nschan[idx] = 1;
    win.sfreq[idx] = win.wfreq[i];
    win.sdf[idx] = side * win.wwidth[i];
    win.restfreq[idx] = -1.0;
    idx++;
  }

  if (nwide > 0) {
    idx = nspect;
    if (wide[0] > 0) {
      for (j=0; j<2; j++)
	win.keep[idx+j] = 0;
      for (j=0; j<wide.nelements(); j++) {
	k = wide[j]-1;
	if (k >= 0 || k < 2)
	  win.keep[idx+k] = 1;
	else
	  cout << "### Warning: bad wide band window id " << k+1 << endl;
      }
    }
    idx += 2;
    if (window[0] > 0) {
      for (j=0; j<nwide-2; j++)
	win.keep[idx+j] = 0;
      for (j=0; j<window.nelements(); j++) {
	k = window[j]-1;
	if (k >= 0 || k < nwide-2)
	  win.keep[idx+k] = 1;
	else
	  cout << "### Warning: bad window band window id " << k+1 << endl;
      }
    }
  } // i

  if (Debug(1)) {
    cout << "Layout of spectral windows (init_window): nspect=" << nspect 
	 << " nwide=" << nwide 
	 << "\n";
    cout << "(N=narrow W=wide, S=spectral window averages)" << endl;

    for (i=0; i<nspect+nwide; i++)
      cout << win.code[i] << ": " << i+1  << " " << win.keep[i] << " "
	   << win.nschan[i] << " " << win.ischan[i] << " " 
	   << win.sfreq[i] <<  " " << win.sdf[i] <<  " " << win.restfreq[i]
	   << "\n";

    cout << "narrow: " ;
    for (i=0; i<narrow.nelements(); i++)
	cout << narrow[i] << " ";
    cout << endl;
    cout << "win: " ;
    for (i=0; i<window.nelements(); i++)
	cout << window[i] << " ";
    cout << endl;
    cout << "wide: " ;
    for (i=0; i<wide.nelements(); i++)
	cout << wide[i] << " ";
    cout << endl;


  }
}

void BimaFiller::update_window()
{
  if (Debug(1)) cout << "BimaFiller::update_window" << endl;
  throw(AipsError("Cannot update window configuration yet"));
}




int main(int argc, char **argv)
{
  cout << "BimaFiller is now deprecated, use the carmafiller command" << endl;
  cout << "BimaFiller::START" << endl;
  try {
    
    // Define inputs
    Input inp(1);
    inp.version("3 - BIMA to MS filler (12-oct-2009) PJT)");

    inp.create("vis",     "vis0",    "Name of BIMA dataset name",          "string");    
    inp.create("ms",      "ms0",     "Name of MeasurementSet",             "string");    
    inp.create("useTSM",  "True",    "Use the TiledStorageManager",        "bool");        
    inp.create("scratch", "True",    "Use a scratch file, or scan twice?", "bool");   // ignored
    inp.create("narrow",  "all",     "Which of the narrow band windows",   "string");
    inp.create("win",     "all",     "Which of the window averages",       "string");
    inp.create("wide",    "all",     "Which of the (two) wide bands",      "string");
    inp.create("tsys",    "False",   "Fill WEIGHT from Tsys in data?",     "bool");
    inp.create("split",   "True",    "DEBUG: Split spectral windows?",     "bool");
    inp.create("arrays",  "False",   "DEBUG: Split multiple arrays?",      "bool");

    inp.readArguments(argc, argv);

    String vis(inp.getString("vis"));;
    String ms(inp.getString("ms"));
    if(ms==" ") ms=vis.before('.') + ".ms";
    Bool useTSM  = inp.getBool("useTSM");      // historic: we used to use UseISM
    Bool Qtsys   = inp.getBool("tsys");
    Bool Qsplit  = inp.getBool("split");   // debug 
    Bool Qarrays = inp.getBool("arrays");  // debug
    File t(vis);
    Int i, debug = -1;
    Block<Int> narrow, win, wide;

    for (i=0; i<99; i++)      // hmm, must this be so hard ??
      if  (!inp.debug(i)) {
        debug = i-1;
        break;
      }

    if (!t.isDirectory())
      throw(AipsError("Input file does not appear to be miriad dataset"));

    if (inp.getString("narrow") == "all") {
      narrow.resize(1);
      narrow[0] = -1;
    } else
      narrow = inp.getIntArray("narrow");

    if (inp.getString("win") == "all") {
      win.resize(1);
      win[0] = -1;
    } else
      win = inp.getIntArray("win");

    if (inp.getString("wide") == "all") {
      wide.resize(1);
      wide[0] = -1;
    } else
      wide = inp.getIntArray("wide");

    BimaFiller bf(vis,debug,Qtsys,Qsplit,Qarrays);

    bf.checkInput(narrow,wide,win);
    bf.setupMeasurementSet(ms,useTSM);
    bf.fillObsTables();
    bf.fillAntennaTable();    // put first array in place

    // fill the main table
    bf.fillMSMainTable(); // also: fillAntennaTable()

    // fill remaining tables
    bf.fillSyscalTable();      
    bf.fillSpectralWindowTable();   
    bf.fillFieldTable();
    bf.fillSourceTable();
    bf.fillFeedTable();
    bf.fixEpochReferences();

  } 
  catch (AipsError x) {
      cerr << x.getMesg() << endl;
  } 

  cout << "BimaFiller::END" << endl;
  cout << "BimaFiller is now deprecated, use the carmafiller command" << endl;
  return 1;
}

void cause_coredump(void)
{
  char *p;

  p = 0;
  *p = 0;
}


