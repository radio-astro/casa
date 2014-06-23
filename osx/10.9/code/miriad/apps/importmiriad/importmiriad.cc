//# importmiriad:  (carma) miriad dataset to MeasurementSet conversion
//# Copyright (C) 1997,2000,2001,2002,2013
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
//# $Id: importmiriad.cc,v 1.31 2011/08/10 20:25:39 pteuben Exp $
//

#include <casa/Inputs/Input.h>
#include <casa/OS/File.h>
#include <casa/Utilities/GenSort.h>

#include <casa/Arrays/Cube.h>
#include <casa/Arrays/Matrix.h>
#include <casa/Arrays/Vector.h>
#include <casa/Arrays/ArrayMath.h>
#include <casa/Arrays/ArrayUtil.h>
#include <casa/Arrays/ArrayLogical.h>
#include <casa/Arrays/MatrixMath.h>


#include <measures/Measures.h>
#include <measures/Measures/MPosition.h>
#include <measures/Measures/MeasData.h>
#include <measures/Measures/Stokes.h>

#include <tables/Tables.h>
#include <tables/Tables/TableInfo.h>

#include <ms/MeasurementSets.h> 

#include <mirlib/maxdimc.h>
#include <mirlib/miriad.h>

#include <casa/namespace.h>

// MIRIAD dataset MeasurementSet filler. Derived from carmafiller.
//
// Despite that this program is written in C++ and uses classes, it is
// really a set of routines manipulating a huge common block.....
//
// Acknowledgement: this program was originally cloned off uvfitsfiller.cc
//                  whose code is now in MSFitsInput.cc
//                  Ray Plante wrote the other filler, with which we had
//                  some cross talk
// 
//
// Limitations, Caveats and Remaining ToDo's
//    - does not apply the various (miriad) complex gain corrections
//      (uvcat and friends should be used for that in miriad, it would be
//      silly to duplicate that code here)
//    - this code won't run if Double!=double or Float!=float or Int!=int
//      On some future 64bit machines this may cause problems?
//    - CARMA type arrays when dishes are not the same size
//    - do not mix arrays from different telescopes (e.g. BIMA and ATCA)
//      but it should handle different configurations (ARRAYs) of the same
//      instrument
//    - handle multiple array configuration datasets that were UVCAT'ed
//      (needs to count antennae up at each array configuration)
//    - true miriad storage manager ?
//
//    - check UV override, is that being handled transparently?
//
//    - spectral windows layout  (code present, fix table access descriptors) 
//      CHECK: what happens if e.g. uvcat does some line= preprocessing
//    - correct restfreq's in source tables that relate to the spectral windows
//      (this code currently coredumps)
//    - make win=,narrow=,wide=0 cause non of them to be written in that selection
//    - add syscal table
//    - add weather table
//    - although no more wides are written, there is a way to make a small MS
//      from the wides only by preprocessing in miriad, but conceivably
//      could be done here as well??  -- except the narrow= keyword seems to write
//      files that suffer from the Table array conformance error

//  History:
//   Spring 1997:   written (cloned off uvfitsfiller)          Peter Teuben
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
//                  also renamed from bimafiller to carmafiller
//   Sep 2011:      initial release
//   Mar 2013:      Rename to importmiriad and make it work for ATCA 
//                  CABB data (more channels, windows and 4 pols)       MHW
//   May 2014:      Fix some compiler warnings, add TOPO option         MHW

// a placeholder for the MIRIAD spectral window configuration
// see also the MIRIAD programmers guide, and its appendix on UV Variables 
// for CARMA we have MAXWIN = MAXWIDE (16 normally, 6 in 2006-2009, more later)
// narrow band (plus MAXWIDE space to point into the wide band data too)

typedef struct window {     // CASA defines everything mid-band mid-interval
  int    nspect;                   // number of valid windows (<=MAXWIN, typically 16)
  int    nschan[MAXWIN+MAXWIDE];   // number of channels in a window
  int    ischan[MAXWIN+MAXWIDE];   // starting channel of a window (1-based)
  double sdf[MAXWIN+MAXWIDE];      // channel separation 
  double sfreq[MAXWIN+MAXWIDE];    // frequency of first channel in window (doppler changes)
  double restfreq[MAXWIN+MAXWIDE]; // rest freq, if appropriate
  char   code[MAXWIN+MAXWIDE];     // code to CASA identification (N, W or S; S not used anymore)
  int    keep[MAXWIN+MAXWIDE];     // keep this window for output to MS (0=false 1=true)

  // wide band (for CARMA these are the spectral window averages - i.e. nspect=nwide)
  int    nwide;                    // number of wide band channels
  float  wfreq[MAXWIDE];           // freq
  float  wwidth[MAXWIDE];          // width
  // ifchain for ATCA
  int    chain[MAXWIN+MAXWIDE];    // if conversion chain


} WINDOW;

//  the maximum number of fields in mosaicing observations, careful, miriad
//  often has a smaller value, like 64 or 128.
#ifndef MAXFIELD
# define MAXFIELD  2048
#endif

#ifndef MAXMSG
# define MAXMSG 256
#endif


// helper functions

void show_version_info()
{
  cout << "============================================================\n";
  cout << "Importmiriad - last few updates:\n";
  cout << " Mar 2013 - make it process ATCA/CABB data \n";
  cout << "           ...\n";
  cout << "============================================================\n";
}


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

// apply CARMA line calibration, the 'linecal' method
// check MIRIADs 'uvcal options=linecal' for the other approach

void linecal(int ndata, float *data, float phi1, float phi2)
{
  float x,y,c,s;
  if (ndata <= 0) return;

  c = cos(phi1-phi2);
  s = sin(phi1-phi2);

  for (int i=0; i<ndata*2; i+=2) {
    x = data[i];
    y = data[i+1];
    data[i]   =  c*x - s*y;
    data[i+1] =  s*x + c*y;
#if 0
    if (i==0) cout << "LINECAL: " << c << " " << s << 
		" old: " << x << " " << y <<
		" new: " << data[0] << " " << data[1] << endl;
#endif
  }
}



// a helper class 

class Importmiriad
{
  // This is an implementation helper class used to store 'local' data
  // during the filling process.
public:
  // Create from a miriad (CARMA) dataset (a directory)
  Importmiriad(String& infile, Int debug=0, 
	      Bool Qtsys=False,
	      Bool Qarrays=False,
	      Bool Qlinecal=False);

  // Standard destructor
  ~Importmiriad();
  
  // Check some of the contents of the data and header read
  void checkInput(Block<Int>& narrow, Block<Int>& window);

  // Debug output level
  Bool Debug(int level);

  // Set up the MeasurementSet, including StorageManagers and fixed columns.
  // If useTSM is True, the Tiled Storage Manager will be used to store
  // DATA, FLAG and WEIGHT_SPECTRUM
  void setupMeasurementSet(const String& MSFileName, Bool useTSM);

  // Fill the Observation and History (formerly ObsLog) tables
  void fillObsTables();

  // Fill the main table by reading in all the visibilities
  void fillMSMainTable();

  // Make an Antenna Table (can be called incrementally now)
  void fillAntennaTable();

  // Make a Syscal Table (can be called incrementally)
  void fillSyscalTable();

  // fill Spectralwindow table 
  void fillSpectralWindowTable(String vel);

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
  void init_window(Block<Int>& narrow, Block<Int>& window);
  void update_window();
  void Error(char *msg);
  void Warning(char *msg);
  void show();
  void close();

private:
  String                 infile_p;     // filename
  Int                    uv_handle_p;  // miriad handle 
  MeasurementSet         ms_p;         // the MS itself
  MSColumns             *msc_p;        // handy pointer to the columns in an MS  
  Int                    debug_p;      // debug level
  Int                    nIF_p;
  String                 array_p, 
                         project_p, 
                         object_p, 
                         telescope_p, 
                         observer_p, 
                         version_p,
                         timsys_p;
  Vector<Int>            nPixel_p, corrType_p, corrIndex_p;
  Matrix<Int>            corrProduct_p;
  Double                 epoch_p;
  MDirection::Types      epochRef_p;
  Int                    nArray_p;      // number of arrays (nAnt_p.nelements())
  Block<Int>             nAnt_p;        // number of antennas per array
  Block<Vector<Double> > receptorAngle_p;
  Vector<Double>         arrayXYZ_p;    // needs to be made with 3 elements
  Vector<Double>         ras_p, decs_p; // ra/dec for source list (source_p)
  Vector<String>         source_p,      // list of source names (?? object_p ??)
                         purpose_p;     // purpose of this source 


  // the following variables are for miriad, hence not Double/Int/Float
  // thus the code may have to be fixed on machines where these do not
  // agree ... may need special access code to get those into CASA
  // types on 64 bit machines??

  double preamble[5], first_time;
  int    ifield, nfield, npoint, nsource;     // both dra/ddec should become Vector's
  float  dra[MAXFIELD], ddec[MAXFIELD];       // offset in radians
  double ra[MAXFIELD], dec[MAXFIELD];
  int    field[MAXFIELD];                     // source index
  int    fcount[MAXFIELD], sid_p[MAXFIELD];
  float  dra_p, ddec_p;
  int    pol_p[4];
  char   message[MAXMSG];


  // The following items more or less follow the uv variables in a dataset
  Int    nants_p, nants_offset_p, nchan_p, nwide_p, npol_p;
  Double antpos[3*MAXANT];
  Float  phasem1[MAXANT];
  Double ra_p, dec_p;       // current pointing center RA,DEC at EPOCH 
  Float  inttime_p;
  Double freq_p;            // rest frequency of the primary line
  Int    mount_p;
  Double time_p;            // current MJD time

  // MIRIAD spectral window definition
  WINDOW win;
  Bool   Qtsys_p;    /* tsys weight's */
  Bool   Qarrays_p;  /* write separate arrays */
  Bool   Qlinecal_p; /* do linecal */

  // Data buffers.... again in MIRIAD format
  
  float  data[2*MAXCHAN], wdata[2*MAXCHAN];	// 2*MAXCHAN since (Re,Im) pairs complex numbers
  int    flags[MAXCHAN], wflags[MAXCHAN];
  float  systemp[MAXANT*MAXWIDE];
  int    zero_tsys;

  // Counters
  int    nvis;
};

// ==============================================================================================
Importmiriad::Importmiriad(String& infile, Int debug, 
			 Bool Qtsys, Bool Qarrays, Bool Qlinecal)
{
  infile_p = infile;
  debug_p = debug;
  nArray_p = 0;
  nfield = 0;           //  # mosaiced fields (using offsets?)
  npoint = 0;           //  # pointings (using independant RA/DEC?)
  Qtsys_p = Qtsys;
  Qarrays_p = Qarrays;
  Qlinecal_p = Qlinecal;
  zero_tsys = 0;
  for (int i=0; i<MAXFIELD; i++) fcount[i] = 0;
  for (int i=0; i<MAXANT;   i++) phasem1[i] = 0.0;

  if (Debug(1)) cout << "Importmiriad::Importmiriad debug_level=" << debug << endl;
  if (Debug(1)) cout << "Opening miriad dataset " << infile_p << endl;
  if (Debug(1)) cout << "See also "
		     << "http://www.astro.umd.edu/~teuben/casa/importmiriad.html" << endl;
  if (Debug(1)) cout << (Qtsys_p ?  "tsys weights" : "weights=1") << endl;
  if (Debug(1)) cout << (Qarrays_p ? "split arrays" : "single array forced") << endl;
  if (Debug(1)) cout << (Qlinecal_p ? "linecal applied" : "linecal not applied") << endl;

  if (sizeof(double) != sizeof(Double))
    cout << "Double != double; importmiriad will probably fail" << endl;
  if (sizeof(int) != sizeof(Int))
    cout << "int != Int; importmiriad will probably fail" << endl;

  // open miriad dataset
  uvopen_c(&uv_handle_p, infile_p.chars(), "old");

  // preamble data must be UVW (default miriad is UV)
  uvset_c(uv_handle_p,"preamble","uvw/time/baseline",0,0.0,0.0,0.0);

  // initialize those UV variables that need to be tracked
  Tracking(-1);      
}

// ============================================================================================== 
Importmiriad::~Importmiriad() 
{
  if (Debug(1)) cout << "Importmiriad::~Importmiriad" << endl;
  if (zero_tsys)
    cout << "There were " << zero_tsys << " record with no WEIGHT due to zero TSYS" << endl;

  if (Debug(1))
    for (int i=0; i<nfield; i++)
      cout << "Field " << i << " = " << fcount[i] << " records" << endl;

  // most single MIRIAD files are time ordered, so could check for 
  // that, and if so, add SORT_ORDER = 'ASCENDING' and COLUMNS = 'TIME'

  if (Debug(1)) cout << "*** Closing " << infile_p << " ***\n" ;
}

// ==============================================================================================
void Importmiriad::Error(char *msg)
{
  throw(AipsError(msg));
}

// ==============================================================================================
void Importmiriad::Warning(char *msg)
{
  cout << "### Warning: " << msg <<  endl;
}

// ==============================================================================================
Bool Importmiriad::Debug(int level)
{
  Bool ok=False;
  if (level <= debug_p) ok=True;
  return ok;
}

// ==============================================================================================
void Importmiriad::checkInput(Block<Int>& narrow, Block<Int>& window)
{
  Bool ok=True;
  Int i, nread, nwread, vlen, vupd;
  char vtype[10], vdata[256];
  Float epoch;

  if (Debug(1)) cout << "Importmiriad::checkInput" << endl;

  // Let's read one scan and try and derive some basics. If important
  // variables not present, bail out (or else scan on)

  nvis = 0;
  for (;;) {   // loop forever until happy or EOF

    uvread_c(uv_handle_p, preamble, data, flags, MAXCHAN, &nread);    
    if (nread <= 0) break;
    nvis++;
    uvwread_c(uv_handle_p, wdata, wflags, MAXCHAN, &nwread);

    if (nvis == 1) {
      // get the initial correllator setup
      init_window(narrow,window);

      //  should store nread + nwread, or handle it as option
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
	}
      }

      if (win.nspect > 0) {
	uvgetvr_c(uv_handle_p,H_DBLE,"restfreq",(char *)win.restfreq,win.nspect);
	if (Debug(1)) {
	  cout << "Found restfreq (first scan)" ;
	  for (Int i=0; i<win.nspect; i++)  cout << win.restfreq[i] << " ";
	  cout << endl;
	}
      }

      // Note that MIRIAD coordinates are in nanosec, but actual unused
      // antennas are filled with -999 values (or sometimes 0!)

      uvprobvr_c(uv_handle_p,"project",vtype,&vlen,&vupd);
      if (vupd) {
	uvgetvr_c(uv_handle_p,H_BYTE,"project",vdata,32);
	project_p = vdata;
      } else
	project_p = "unknown";
      if (Debug(1)) cout << "Project=>" << project_p << "<=" << endl;

      uvprobvr_c(uv_handle_p,"version",vtype,&vlen,&vupd);
      if (vupd) {
        uvgetvr_c(uv_handle_p,H_BYTE,"version",vdata,80);
        version_p = vdata;
        if (Debug(1)) cout << "Version=>" << version_p << "<=" << endl;
      }
      uvgetvr_c(uv_handle_p,H_BYTE,"source",vdata,16);
      object_p = vdata;
      
      // TODO: telescope will now change, so this is not a good idea
      uvgetvr_c(uv_handle_p,H_BYTE,"telescop",vdata,16);
      array_p = vdata;

      // array_p = "CARMA";
      if (Debug(1)) cout << "First baseline=>" << array_p << "<=" << endl;
      
      // All CARMA (OVRO,BIMA,SZA) & ATCA have this 
      mount_p = 0;
#if 0
	if (array_p == "VLA")
	  mount_p = 1;
	uvrdvr_c(uv_handle_p,H_INT,"mount",(char *)&mount_p, (char *)&mount_p, 1);
	cout << "Warning: " << array_p 
	     << " Cannot handle all of this telescope yet" << endl;
	cout << "Assumed mount=" << mount_p << endl;
#endif
      
      uvprobvr_c(uv_handle_p,"observer",vtype,&vlen,&vupd);
      if (vupd) {
	uvgetvr_c(uv_handle_p,H_BYTE,"observer",vdata,16);
	observer_p = vdata;
      } else              
	observer_p = "unknown";    
      
      uvgetvr_c(uv_handle_p,H_REAL,"epoch",(char *)&epoch,1);
      epoch_p = epoch;
      // do this globally, we used to do this in the Field table alone
      epochRef_p=MDirection::J2000;      
      if (nearAbs(epoch_p,1950.0,0.01)) epochRef_p=MDirection::B1950;   

      uvgetvr_c(uv_handle_p,H_INT,"npol", (char *)&npol_p,1);
      uvgetvr_c(uv_handle_p,H_INT,"pol",(char *)&pol_p,1);
      uvgetvr_c(uv_handle_p,H_REAL,"inttime",(char *)&inttime_p,1);
      
      uvprobvr_c(uv_handle_p,"freq",vtype,&vlen,&vupd);
      freq_p = 1e9;
      if (vupd) {
        uvgetvr_c(uv_handle_p,H_DBLE,"freq",(char *)&freq_p,1);
        freq_p *= 1e9;
      }

      uvprobvr_c(uv_handle_p,"ifchain",vtype,&vlen,&vupd);
      if(!vupd) {
      	for (i=0; i<MAXWIN+MAXWIDE;i++) win.chain[i]=0;
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
	  if (Debug(1)) { if (i==1) cout << "POL(" << i << ") = " << pol_p[0] << endl;}
	  uvgetvr_c(uv_handle_p,H_INT,"pol",(char *)&pol_p[i],1);        // FIX
	  if (Debug(1)) cout << "POL(" << i+1 << ") = " << pol_p[i] << endl;
	}
      }
      // only do one scan
      break;
    }
  }
  if (nvis == 0) {
    throw(AipsError("Importmiriad: Bad first uvread: no narrow or wide band data present"));
    ok = False;
    return;
  } else
    //cout << "Importmiriad::checkInput: " << nvis << " records found" << endl;
    //cout << "Found " << nvis << " records" << endl;
  uvrewind_c(uv_handle_p);

  Int numCorr = npol_p;
  corrType_p.resize(numCorr); 
  for (i=0; i < numCorr; i++) {
    // note: 1-based ref pix
    corrType_p(i)=pol_p[i];
    // convert AIPS-convention Stokes description to CASA enum
    // CHECK if these are really the right conversions for CASA
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

// ==============================================================================================
void Importmiriad::setupMeasurementSet(const String& MSFileName, Bool useTSM)
{
  if (Debug(1)) cout << "Importmiriad::setupMeasurementSet" << endl;

  Int nCorr =  (array_p=="CARMA" ? 1 : npol_p); // # stokes (1 for CARMA for now)
  Int nChan = nchan_p;  // we are only exporting the narrow channels to the MS

  nIF_p = win.nspect;   // number of spectral windows (for narrow channels only)

  // Make the MS table
  TableDesc td = MS::requiredTableDesc();

  MS::addColumnToDesc(td, MS::DATA,2);
  td.removeColumn(MS::columnName(MS::FLAG));
  MS::addColumnToDesc(td, MS::FLAG,2);

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

  // Set up the default subtables for the MS
  ms.createDefaultSubtables(option);

  // Add some optional columns to the required tables
  ms.spectralWindow().addColumn(ArrayColumnDesc<Int>(
    MSSpectralWindow::columnName(MSSpectralWindow::ASSOC_SPW_ID),
    MSSpectralWindow::columnStandardComment(MSSpectralWindow::ASSOC_SPW_ID)));

  ms.spectralWindow().addColumn(ArrayColumnDesc<String>(
    MSSpectralWindow::columnName(MSSpectralWindow::ASSOC_NATURE),
    MSSpectralWindow::columnStandardComment(MSSpectralWindow::ASSOC_NATURE)));

  ms.spectralWindow().addColumn(ScalarColumnDesc<Int>(
    MSSpectralWindow::columnName(MSSpectralWindow::DOPPLER_ID),
    MSSpectralWindow::columnStandardComment(MSSpectralWindow::DOPPLER_ID)));

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
    info.setSubType(String("MIRIAD/CARMA"));
    info.readmeAddLine("Made with Importmiriad");
  }                                       

  ms_p=ms;
  msc_p = new MSColumns(ms_p);
} // setupMeasurementSet()

// ==============================================================================================
void Importmiriad::fillObsTables()
{
  if (Debug(1)) cout << "Importmiriad::fillObsTables" << endl;

  char hline[256];
  Int heof;

  ms_p.observation().addRow();
  MSObservationColumns msObsCol(ms_p.observation());

  msObsCol.telescopeName().put(0,array_p);
  msObsCol.observer().put(0,observer_p);
  msObsCol.project().put(0,project_p);
  if (array_p == "HATCREEK") {
    Vector<String> blog(1);
    blog(0) = "See HISTORY for CARMA observing log";
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
    msHisCol.origin().put(row,"Importmiriad::fillObsTables");
    msHisCol.application().put(row,"importmiriad");
    msHisCol.message().put(row,hline);
  }
  hisclose_c(uv_handle_p);
} // fillObsTables()

// ==============================================================================================
//
// Loop over the visibility data and fill the main table of the MeasurementSet 
// as you find corr/wcorr's
//
void Importmiriad::fillMSMainTable()
{
  if (Debug(1)) cout << "Importmiriad::fillMSMainTable" << endl;

  MSColumns& msc(*msc_p);           // Get access to the MS columns, new way
  Int nCorr = (array_p=="CARMA" ? 1 : npol_p); // # stokes (1 for CARMA for now)
  Int nChan = nchan_p;              // # channels to be written
  if (Debug(1)) cout << "nCorr = "<<nCorr<<", nChan = "<< nChan <<endl;
  Int nCat  = 3;                    // # initial flagging categories (fixed at 3)
  Int iscan = 0;
  Int ifield_old = 0;

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

  uvrewind_c(uv_handle_p);
  
  nAnt_p.resize(1);
  nAnt_p[0]=0;

  receptorAngle_p.resize(1);
  Int group, row=-1;
  Double interval;
  Bool lastRowFlag = False;

  // cout << "Importmiriad::fillMSMainTable(): using " << nIF_p << " spectral windows" << endl;
  cout << "Found  " << nIF_p << " spectral window" << (nIF_p>1 ? "s":"") << endl;

  if (Debug(1)) cout << "Writing " << nIF_p << " spectral windows" << endl;
  time_p=0;
  for (group=0; ; group++) {        // loop forever until end-of-file
    int nread, nwread;
    uvread_c(uv_handle_p, preamble, data, flags, MAXCHAN, &nread);
    // cout << "UVREAD: " << data[0] << " " << data[1] << endl;

    Float baseline = preamble[4];
    Int ant1 = Int(baseline)/256;              // baseline = 256*A1 + A2
    Int ant2 = Int(baseline) - ant1*256;       // mostly A1 <= A2


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

    // get time in MJD seconds ; input was in JD
    Double time = (preamble[3] - 2400000.5) * C::day;
    if (time>time_p) {
      // new time slot (assuming time sorted data)
      // update tsys data - TODO
    }
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
    } // Debug(3) for timeline monitoring

    interval = inttime_p;
    //msc.interval().put(0,interval);
    //msc.exposure().put(0,interval);


    // for MIRIAD, this would always cause a single array dataset,
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

    // now that phasem1 has been loaded, apply linelength, if needed
    if (Qlinecal_p) {
      linecal(nread,data,phasem1[ant1-1],phasem1[ant2-1]);
      // linecal(nwread,wdata,phasem1[ant1-1],phasem1[ant2-1]);
    }

    //  nAnt_p.resize(array+1);
    //  receptorAngle_p.resize(array+1);

    nAnt_p[nArray_p-1] = max(nAnt_p[nArray_p-1],ant1);   // for MIRIAD, and also 
    nAnt_p[nArray_p-1] = max(nAnt_p[nArray_p-1],ant2);
    ant1--; ant2--;                                      // make them 0-based for CASA

    ant1 += nants_offset_p;     // correct for different array offsets
    ant2 += nants_offset_p;


    // should ant1 and ant2 be offset with (nArray_p-1)*nant_p ???
    // in case there are multiple arrays???
    // TODO: code should just assuming single array
    
    Vector<Double> uvw(3);
    uvw(0) = -preamble[0] * 1e-9; // convert to seconds
    uvw(1) = -preamble[1] * 1e-9; // MIRIAD uses nanosec
    uvw(2) = -preamble[2] * 1e-9; // note - sign (CASA vs. MIRIAD convention)
    uvw   *= C::c;                // Finally convert to meters for CASA

    if (group==0 && Debug(1)) {
        cout << "### First record: " << endl;
        cout << "### Preamble: " << preamble[0] << " " <<
                                preamble[1] << " " <<
                                preamble[2] << " nanosec.(MIRIAD convention)" << endl;
        cout << "### uvw: " << uvw(0) << " " <<
                               uvw(1) << " " <<
                               uvw(2) << " meter. (CASA convention)" << endl;
    }


    // first construct the data (vis & flag) in a single long array
    // containing all spectral windows
    // In the (optional) loop over all spectral windows, subsets of
    // these arrays will be written out

    for (Int i=0; i<nCorr; i++) {
      Int count = 0;                // index into data[] array
      if (i>0) uvread_c(uv_handle_p, preamble, data, flags, MAXCHAN, &nread);
      //if (group==0) {
      //	cout << "pol="<< pol<<", nread="<<nread<<endl;
      //  cout << "data(500)="<< data[1000] <<", "<< data[1001] <<endl;
      //}
      for (Int chan=0; chan<nChan; chan++) {

	// miriad uses bl=ant1-ant2, FITS/AIPS/CASA use bl=ant2-ant1
	// apart from using -(UVW)'s, the visib need to be conjugated as well
	Bool  visFlag =  (flags[count/2] == 0) ? False : True;
	Float visReal = +data[count]; count++; 
	Float visImag = -data[count]; count++;
	Float wt = 1.0;
	if (!visFlag) wt = -wt;
	
	// check flags array !! need separate counter (count/2)
        Int pol = corrIndex_p(i);
	flag(pol,chan) = (wt<=0); 
	vis(pol,chan) = Complex(visReal,visImag);
      } // chan
      //if (group==0) cout << "vis = "<<vis(pol,500)<<endl;
    } // pol


    for (Int ifno=0; ifno < nIF_p; ifno++) {
      if (win.keep[ifno]==0) continue;    
      // IFs go to separate rows in the MS, pol's do not!
      ms_p.addRow(); 
      row++;

      // first fill in values for all the unused columns
      if (row==0) {
	ifield_old = ifield;
	msc.feed1().put(row,0);
	msc.feed2().put(row,0);
	msc.flagRow().put(row,False);
	lastRowFlag = False;
	msc.scanNumber().put(row,iscan);
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
#if 1
      // the dumb way: e.g. 3" -> 20" for 3c273
      Matrix<Complex> tvis(nCorr,win.nschan[ifno]);
      Cube<Bool> tflagCat(nCorr,win.nschan[ifno],nCat,False);  
      Matrix<Bool> tflag = tflagCat.xyPlane(0); // references flagCat's storage
      
      Int woffset = win.ischan[ifno]-1;
      Int wsize   = win.nschan[ifno];
      for (Int j=0; j<nCorr; j++) {
        for (Int i=0; i< wsize; i++) {
          tvis(j,i) = vis(j,i+woffset);
          tflag(j,i) = flag(j,i+woffset);
	}
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
      //if (group==0) cout<<"tvis="<<tvis(0,500)<<", "<<tvis(1,500)<<endl;
      msc.data().put(row,tvis);
      msc.flag().put(row,tflag);
      msc.flagCategory().put(row,tflagCat);

      Bool rowFlag = allEQ(flag,True);
      if (rowFlag != lastRowFlag) {
	msc.flagRow().put(row,rowFlag);
	lastRowFlag = rowFlag;
      }

      msc.antenna1().put(row,ant1);
      msc.antenna2().put(row,ant2);
      msc.time().put(row,time);           // CARMA did begin of scan.., now middle (2009)
      msc.timeCentroid().put(row,time);   // do we really need this ? flagging/blanking ?
      if (Qtsys_p) {	
	// Vector<Float> w1(nCorr), w2(nCorr);
	w2 = 1.0;   // i use this as a 'version' id  to test FC refresh bugs :-)
	if( systemp[ant1] == 0 || systemp[ant2] == 0) {
	  zero_tsys++;
	  w1 = 0.0;
	} else
	  w1 = 1.0/(systemp[ant1]*systemp[ant2]);  // see uvio::uvinfo_variance()
	//if (Debug(1)) cout << w1 << " " << w2 << endl;
	msc.weight().put(row,w1);
	msc.sigma().put(row,w2);	
      }
      msc.uvw().put(row,uvw);
      msc.arrayId().put(row,nArray_p-1);
      msc.dataDescId().put(row,ifno);
      msc.fieldId().put(row,ifield);


#if 1
      // TODO: SCAN_NUMBER needs to be added, they are all 0 now
      if (ifield_old != ifield) 
	iscan++;
      ifield_old = ifield;
      msc.scanNumber().put(row,iscan);
#endif

    }  // ifNo
    fcount[ifield]++;
  } // for(grou) : loop over all visibilities
  show();

  cout << "Processed " << group << " visibilities from " << infile_p  
       << (Qlinecal_p ? " (applying linecal)." : " (raw)." )
       << endl;
  cout << "Found " << npoint << " pointings with "
       <<  nfield << " unique source/fields, "
       <<  source_p.nelements() << " sources and "
       <<  nArray_p << " array"<< (nArray_p>1 ? "s":"")<<"." 
       << endl;
  if (Debug(1))
    cout << "nAnt_p contains: " << nAnt_p.nelements() << endl;

#if 0
  // fill the receptorAngle with defaults, just in case there is no AN table
  for (Int arr=0; arr<nAnt_p.nelements(); arr++) {
    Vector<Double> angle(2*nAnt_p[arr]); 
    angle=0;
    receptorAngle_p[arr]=angle;
  }
#endif
} // fillMSMainTable()

void Importmiriad::fillAntennaTable()
{
  if (Debug(1)) cout << "Importmiriad::fillAntennaTable" << endl;
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

  // === Here's a recipe to get them in ITRF format in casapy ===
  //     But there appears to be some GEODETIC vs. GEOCENTRIC issue 
  // WGS84 
  // b=me.measure(me.observatory('carma'),'itrf')    
  // b2=me.measure(me.observatory('carma'),'wgs84')    
  // lon=me.getvalue(b)['m0']['value'] 
  // lat=me.getvalue(b2)['m1']['value'] 
  // r=me.getvalue(b)['m2']['value'] 
  // x=r*cos(lat)*cos(lon)
  // y=r*cos(lat)*sin(lon)
  // z=r*sin(lat)
  // print 'arrayXYZ_p(0) =',x,';'
  // print 'arrayXYZ_p(1) =',y,';'
  // print 'arrayXYZ_p(2) =',z,';'
  arrayXYZ_p.resize(3);
  if (array_p == "HATCREEK" || array_p == "BIMA") {     // Array center:
    arrayXYZ_p(0) = -2523862.04;
    arrayXYZ_p(1) = -4123592.80;
    arrayXYZ_p(2) =  4147750.37;
  } else if (array_p == "ATCA") {
    arrayXYZ_p(0) = -4750915.837;
    arrayXYZ_p(1) =  2792906.182;
    arrayXYZ_p(2) = -3200483.747;
  } else if (array_p == "OVRO" || array_p == "CARMA") {
    arrayXYZ_p(0) = -2397389.65197;
    arrayXYZ_p(1) = -4482068.56252;
    arrayXYZ_p(2) =  3843528.41479;
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
  // exist in our CARMA datasets.
  // So, fill in some likely values
  Float diameter=25;                        //# most common size (:-)
  if (array_p=="ATCA")     diameter=22;     //# only at 'low' freq !!
  if (array_p=="HATCREEK") diameter=6;
  if (array_p=="BIMA")     diameter=6;
  if (array_p=="CARMA")    diameter=8;
  if (array_p=="OVRO")     diameter=10;

  if (nAnt == 15 && array_p=="OVRO") {
    cout << "CARMA array (6 OVRO, 9 BIMA) assumed" << endl;
    array_p = "CARMA";
  } else  if (nAnt == 23 && array_p=="OVRO") {
    cout << "CARMA array (6 OVRO, 9 BIMA, 8 SZA) assumed" << endl;
    array_p = "CARMA";
  } else  if (array_p=="CARMA") {
    cout << "Hurray, CARMA data; version " << version_p << " with " << nAnt
         << " antennas" << endl;
  } else if (array_p=="ATCA") {
    cout <<"Found ATCA data with " << nAnt << " antennas" << endl;
  } else
    cout << "Ant configuration not supported yet" << endl;

  MSAntennaColumns& ant(msc_p->antenna());
  Vector<Double> antXYZ(3);

  // add antenna info to table
  if (nArray_p == 0) {                   // check if needed
    ant.setPositionRef(MPosition::ITRF);
    //ant.setPositionRef(MPosition::WGS84);
  }
  Int row=ms_p.antenna().nrow()-1;

  if (Debug(2)) cout << "Importmiriad::fillAntennaTable row=" << row+1 
       << " array " << nArray_p+1 << endl;

  for (Int i=0; i<nAnt; i++) {

    ms_p.antenna().addRow(); 
    row++;
    if (array_p=="OVRO" || array_p=="BIMA" || array_p=="HATCREEK" || array_p=="CARMA") {
      if (i<6)
        ant.dishDiameter().put(row,10.4);  // OVRO
      else if (i<15)
        ant.dishDiameter().put(row,6.1);   // BIMA or HATCREEK
      else
        ant.dishDiameter().put(row,3.5);   // SZA
    } else {
      ant.dishDiameter().put(row,diameter); // others
    }
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
    String antName = "C";
    if (array_p=="ATCA") antName="CA0";
    antName += String::toString(i+1);
    ant.name().put(row,antName);
    ant.station().put(row,"ANT" + String::toString(i+1));  // unknown PADs, so for now ANT#
    ant.type().put(row,"GROUND-BASED");

    Vector<Double> offsets(3);
    offsets=0.0;
    // store absolute positions, with all offsets 0

#if 1
    // from MirFiller; but why we're rotating this? To reverse miriad's rotation
    // of y-axis to local East
    Double arrayLong = atan2(arrayXYZ_p(1),arrayXYZ_p(0));
    Matrix<Double> posRot = Rot3D(2,arrayLong);
    antXYZ = product(posRot,antXYZ);
#endif

#if 1
// This doesn't work because miriad calculated the relative positions with
// respect to the first antenna with non zero coordinates, 
// not the array reference position. This makes it impossible to invert exactly
    ant.position().put(row,arrayXYZ_p+antXYZ);
#else
    //test
    ant.position().put(row,arrayXYZ_p);
#endif
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
  String arrnam = array_p;
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

// ==============================================================================================
void Importmiriad::fillSyscalTable()
{
  //if (Debug(1)) cout << "Importmiriad::fillSyscalTable" << endl;

  MSSysCalColumns&     msSys(msc_p->sysCal());
  Vector<Float> Systemp(1);    // should we set both receptors same?
  static Int row = -1;

  //  if (Debug(1)) 
  //   for (Int i=0; i<nants_p; i++)
  //     cout  << "SYSTEMP: " << i << ": " << systemp[i] << endl;

  for (Int j=0; j<win.nspect; j++) {
    for (Int i=0; i<nants_p; i++) {
      ms_p.sysCal().addRow();
      row++;  // should be a static, since this routine will be called again

      msSys.antennaId().put(row,i);   //  i, or i+nants_offset_p ????
      msSys.feedId().put(row,0);
      msSys.spectralWindowId().put(row,j);    // all of them for now .....
      msSys.time().put(row,time_p);
      msSys.interval().put(row,-1.0);
    
      Systemp(0) = systemp[i+j*nants_p];
      msSys.tsys().put(row,Systemp);
    }
  }
 


  // this may actually be a nasty problem for MIRIAD datasets that are not
  // timesorted. A temporary table needs to be written with all records,
  // which then needs to be sorted and 'recomputed'
}

// ==============================================================================================
void Importmiriad::fillSpectralWindowTable(String vel)
{
  if (Debug(1)) cout << "Importmiriad::fillSpectralWindowTable" << endl;

  MSSpWindowColumns&      msSpW(msc_p->spectralWindow());
  MSDataDescColumns&      msDD(msc_p->dataDescription());
  MSPolarizationColumns&  msPol(msc_p->polarization());
  MSDopplerColumns&       msDop(msc_p->doppler());

  Int nCorr = ( array_p=="CARMA" ? 1 : npol_p);            // CARMA wants 1 polarization 
  Int i, j, side;
  Double BW = 0.0;

  MDirection::Types dirtype = epochRef_p;    // MDirection::B1950 or MDirection::J2000;
  MEpoch ep(Quantity(time_p, "s"), MEpoch::UTC);
  // ERROR::   type specifier omitted for parameter  in older AIPS++, now works in CASA
  MPosition obspos(MVPosition(arrayXYZ_p), MPosition::ITRF);
  //MPosition obspos(MVPosition(arrayXYZ_p), MPosition::WGS84);
  MDirection dir(Quantity(ra_p, "rad"), Quantity(dec_p, "rad"), dirtype);
  MeasFrame frame(ep, obspos, dir);
  
  String velsys = vel;
  // Keep previous default, for ATCA leave at TOPO (multi-source data)
  if (array_p!="ATCA" && velsys=="") velsys="LSRK";
  
  Bool convert=True;
  MFrequency::Types freqsys_p;
  if (velsys=="LSRK") {
    freqsys_p = MFrequency::LSRK;        // LSRD vs. LSRK
    if (Debug(1)) cout << "USE_LSRK" << endl;
  } else if (velsys=="LSRD"){
    freqsys_p = MFrequency::LSRD;        // LSRD vs. LSRK
    if (Debug(1)) cout << "USE_LSRD" << endl;
  } else {
    freqsys_p = MFrequency::TOPO;        // use TOPO if unspecified
    convert=False;
  }

  MFrequency::Convert tolsr(MFrequency::TOPO, 
			    MFrequency::Ref(freqsys_p, frame));     // LSRD vs. LSRK
  // fill out the polarization info (only 1 entry allowed for now)
  ms_p.polarization().addRow();
  msPol.numCorr().put(0,nCorr);
  msPol.corrType().put(0,corrType_p);
  msPol.corrProduct().put(0,corrProduct_p);
  msPol.flagRow().put(0,False);

  // fill out doppler table (only 1 entry needed, CARMA data only identify 1 line :-(
  if (Debug(1)) cout << "Importmiriad:: now writing Doppler table " << endl;
  for (i=0; i<win.nspect; i++) {
    ms_p.doppler().addRow();
    msDop.dopplerId().put(i,i);
    msDop.sourceId().put(i,-1);     // how the heck..... for all i guess
    msDop.transitionId().put(i,-1);
    msDop.velDefMeas().put(i,MDoppler(Quantity(0),MDoppler::RADIO));
  }

  // THIS BUG caused that array conformance error, but fixing it
  // one also needed to restart casapy!!! go figure.
  // now only write out spectral windows, not the wides.
  if (Debug(1)) cout << "Array Conformance error check" << endl;
#if 0
  for (i=0; i < win.nspect + win.nwide; i++) 
#else
  for (i=0; i < win.nspect; i++) 
#endif
    {

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
    if (convert) {
      if (Debug(1)) cout << "Fwin: OBS=" << fwin/1e9;
      fwin = tolsr(fwin).getValue().getValue();
      if (Debug(1)) cout << " LSR=" << fwin/1e9 << endl;
    }
    for (j=0; j < win.nschan[i]; j++) {
      f(j) = fwin + j * win.sdf[i] * 1e9;
      w(j) = abs(win.sdf[i]*1e9);
      BW += w(j);
    }

    msSpW.chanFreq().put(i,f);
    if (i<win.nspect)
      msSpW.refFrequency().put(i,win.restfreq[i]*1e9);
    else
      msSpW.refFrequency().put(i,freq_p);            // no reference for wide band???
    
    msSpW.resolution().put(i,w);
    msSpW.chanWidth().put(i,w);
    msSpW.effectiveBW().put(i,w);
    msSpW.totalBandwidth().put(i,BW);
    Int ifchain = win.chain[i];
    msSpW.ifConvChain().put(i,ifchain);
    // can also do it implicitly via Measures you give to the freq's
    msSpW.measFreqRef().put(i,freqsys_p);
    if (i<win.nspect)
      msSpW.dopplerId().put(i,i);    // CARMA has only 1 ref freq line
    else
      msSpW.dopplerId().put(i,-1);    // no ref

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

  // set the reference frames for frequency
  //msSpW.chanFreq().rwKeywordSet().define("MEASURE_REFERENCE","TOPO");

  //PJT msSpW.restFrequency().rwKeywordSet().define("MEASURE_REFERENCE","REST");
  //msSpW.refFrequency().rwKeywordSet().define("MEASURE_REFERENCE","TOPO");
}

// ==============================================================================================
void Importmiriad::fillFieldTable()
{
  if (Debug(1)) cout << "Importmiriad::fillFieldTable" << endl;

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

  if (Debug(1)) cout << "Importmiriad::fillFieldTable() adding " << nfield 
                     << " fields" << endl;

  pm = 0;                       // Proper motion is zero

  if (nfield == 0) {            // if no pointings found, say there is 1
    cout << "Warning: no dra/ddec pointings found, creating 1." << endl;
    nfield = npoint = 1;
    dra[0] = ddec[0] = 0.0;
  } 

  for (fld = 0; fld < nfield; fld++) {
    int sid = sid_p[fld];

    ms_p.field().addRow();

    if (Debug(1))
      cout << "FLD: " << fld << " " << sid << " " << source_p[field[fld]] << endl;

    if (sid > 0) {
      msField.sourceId().put(fld,sid-1); 
      msField.name().put(fld,source_p[field[fld]]);        // this is the source name
    } else {
      // a special test where the central source gets _C appended to the source name
      msField.sourceId().put(fld,-sid-1); 
#if 0
      msField.name().put(fld,source_p[field[fld]]+"_C");   // central source: append _C
#else
      msField.name().put(fld,source_p[field[fld]]);        // or keep them all same name 
#endif
    }

    msField.code().put(fld,purpose_p[field[fld]]);

    msField.numPoly().put(fld,0);

    cosdec = cos(dec[fld]);
    radec(0) = ra[fld]  + dra[fld]/cosdec;           // RA, in radians
    radec(1) = dec[fld] + ddec[fld];                 // DEC, in radians

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

// ==============================================================================================
void Importmiriad::fillSourceTable()
{
  if (Debug(1)) cout << "Importmiriad::fillSourceTable" << endl;
  Int n = win.nspect;
  Int ns = 0;
  Int skip;

  MSSourceColumns& msSource(msc_p->source());

  Vector<Double> radec(2);

  //String key("MEASURE_REFERENCE");
  //msSource.restFrequency().rwKeywordSet().define(key,"REST");

  if (Debug(1)) {
    cout << "Importmiriad::fillSourceTable() querying " << source_p.nelements()
         << " sources" << endl;
    cout << source_p << endl;
  }
  

  // 
  for (uInt src=0; src < source_p.nelements(); src++) {

    skip = 0;
    for (uInt i=0; i<src; i++) {               // loop over sources to avoid duplicates
      if (source_p[i] == source_p[src]) {
	skip=1;
	break;
      }
    }

    if (Debug(1)) cout << "source : " << source_p[src] << " " << skip << endl;

    if (skip) continue;    // if seen before, don't add it again

    // the side effect of this long source_p array with duplicate names
    // is that the source index can be e.g. 0,1,3,4,5 depending on how
    // this reader encountered them first in the scans.

    ms_p.source().addRow();

    radec(0) = ras_p[src];
    radec(1) = decs_p[src];

    msSource.sourceId().put(ns,src);
    msSource.name().put(ns,source_p[src]);
    //    msSource.spectralWindowId().put(src,-1);     // really valid for all ??
    msSource.spectralWindowId().put(ns,0);     // FIX it due to a bug in MS2 code (6feb2001)
    // msSource.spectralWindowId().put(src,-1); // valid for all?
    msSource.direction().put(ns,radec);
    if (n > 0) {
      Int m=n;
      // cout << "TESTING numlines=" << m << endl;
      Vector<Double> restFreq(m);
      for (Int i=0; i<m; i++)
	restFreq(i) = win.restfreq[i] * 1e9;    // convert from GHz to Hz

      msSource.numLines().put(ns,win.nspect);
      msSource.restFrequency().put(ns,restFreq);
    }
    msSource.time().put(ns,0.0);               // valid for all times
    msSource.interval().put(ns,0);             // valid forever

    // TODO?
    // missing position/sysvel/transition in the produced MS/SOURCE sub-table ??

    // listobs complains:
    // No systemic velocity information found in SOURCE table.

    ns++;
  }

  // TODO:  #sources wrong if you take raw miriad before noise taken out
  if (Debug(1)) cout << "Importmiriad::fillSourceTable() added " << ns << " sources" << endl;
}

// ==============================================================================================
void Importmiriad::fillFeedTable() 
{
  if (Debug(1)) cout << "Importmiriad::fillFeedTable" << endl;

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
  if (array_p == "ATCA") {
    ra(0)=C::pi/4;
    // 7mm feed is different; assumes all spectral windows are in same band for now...
    if (win.sfreq[0]>30. && win.sfreq[0]<50.) ra(0)+=C::pi/2;
    ra(1)=ra(0)+C::pi/2;
  }

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
      // although for CARMA this would never change ....
      msfc.receptorAngle().put(row,ra);
      // msfc.receptorAngle().put(row,receptorAngle_p[arr](Slice(2*ant,2)));
    }
  }      
}

// ==============================================================================================
void Importmiriad::fixEpochReferences() {

  if (Debug(1)) cout << "Importmiriad::fixEpochReferences" << endl;

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
// ==============================================================================================
void Importmiriad::Tracking(int record)
{
  if (Debug(3)) cout << "Importmiriad::Tracking" << endl;

  char vtype[10], vdata[10];
  int vlen, vupd, idat, vupd1, vupd2, vupd3, j, k;
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
    uvtrack_c(uv_handle_p,"ifchain","u");  // optional


    uvtrack_c(uv_handle_p,"nwide","u");
    uvtrack_c(uv_handle_p,"wfreq","u");
    uvtrack_c(uv_handle_p,"wwidth","u");

    uvtrack_c(uv_handle_p,"antpos","u");   // array's
    uvtrack_c(uv_handle_p,"pol","u");      // pol's
    uvtrack_c(uv_handle_p,"dra","u");      // fields
    uvtrack_c(uv_handle_p,"ddec","u");     // fields

    uvtrack_c(uv_handle_p,"ra","u");       // source position
    uvtrack_c(uv_handle_p,"dec","u");      // source position

    uvtrack_c(uv_handle_p,"inttime","u");

    if (Qlinecal_p)
      uvtrack_c(uv_handle_p,"phasem1","u");  // linelength meaurements

    // weather:
    // uvtrack_c(uv_handle_p,"airtemp","u");
    // uvtrack_c(uv_handle_p,"dewpoint","u");
    // uvtrack_c(uv_handle_p,"relhumid","u");
    // uvtrack_c(uv_handle_p,"winddir","u");
    // uvtrack_c(uv_handle_p,"windmph","u");

    return;
  }

  // here is all the special tracking code...

  uvprobvr_c(uv_handle_p,"pol",vtype,&vlen,&vupd);
  if (vupd && npol_p==1) {
    uvrdvr_c(uv_handle_p,H_INT,"pol",(char *)&idat, NULL, 1);
    if (idat != pol_p[0])
        cout << "Warning: polarization changed to " << pol_p << endl;
    pol_p[0] = idat;
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

  if (Qlinecal_p) {
    uvprobvr_c(uv_handle_p,"phasem1",vtype,&vlen,&vupd);
    if (vupd) {
      // lets assume (hope?) that nants_p didn't change, it better not.
      uvgetvr_c(uv_handle_p,H_REAL,"phasem1",(char *)phasem1,nants_p);
      // cout << "PHASEM1: " << phasem1[0] << " " << phasem1[1] << " ...\n";
    }
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
      fillSyscalTable();
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

  // Go after a new pointing (where {source,ra,dec} was changed)
  // SOURCE and DRA/DDEC are mixed together they define a row in the FIELD table
  // so we need to build a field index here as well

  uvprobvr_c(uv_handle_p,"source",vtype,&vlen,&vupd);
  if (vupd) {
    uvgetvr_c(uv_handle_p,H_BYTE,"source",vdata,16);
    object_p = vdata;  

    // note: as is, source_p will get repeated values, trim it later  (bug?)
    cout << "Found source: " << object_p << endl;

    source_p.resize(source_p.nelements()+1, True);     // need to copy the old values
    source_p[source_p.nelements()-1] = object_p;

    ras_p.resize(ras_p.nelements()+1, True);     
    decs_p.resize(decs_p.nelements()+1, True);   
    ras_p[ras_p.nelements()-1] = 0.0;                  // if no source at (0,0) offset
    decs_p[decs_p.nelements()-1] = 0.0;                // these would never be initialized  

    uvprobvr_c(uv_handle_p,"purpose",vtype,&vlen,&vupd3);
    purpose_p.resize(purpose_p.nelements()+1, True);   // need to copy the old values
    purpose_p[purpose_p.nelements()-1] = " ";
    if (vupd3) {
      uvgetvr_c(uv_handle_p,H_BYTE,"purpose",vdata,16);
      purpose_p[purpose_p.nelements()-1] = vdata;
    }
  }

  uvprobvr_c(uv_handle_p,"dra", vtype,&vlen,&vupd1);
  uvprobvr_c(uv_handle_p,"ddec",vtype,&vlen,&vupd2);

  if (vupd || vupd1 || vupd2) {    // if either source or offset changed, find FIELD_ID
    npoint++;
    uvgetvr_c(uv_handle_p,H_DBLE,"ra", (char *)&ra_p, 1);
    uvgetvr_c(uv_handle_p,H_DBLE,"dec",(char *)&dec_p,1);
    if (vupd1) uvgetvr_c(uv_handle_p,H_REAL,"dra", (char *)&dra_p,  1);
    if (vupd2) uvgetvr_c(uv_handle_p,H_REAL,"ddec",(char *)&ddec_p, 1);
    uvgetvr_c(uv_handle_p,H_BYTE,"source",vdata,16);
    object_p = vdata;  // also track object name whenever changed

    j=-1;
    for (uInt i=0; i<source_p.nelements(); i++) {    // find first matching source name
      if (source_p[i] == object_p) {
	j = i ;
	break;
      }
    }
    // j should always be >= 0 now, and is the source index
    k=-1;
    for (Int i=0; i<nfield; i++) { // check if we had this pointing/source before 
      if (vupd1 && vupd2 && dra[i] == dra_p && ddec[i] == ddec_p && field[i] == j) {
	k = i;
	break;
      } else if (!vupd1 && !vupd2 && field[i] == j) {
	k = i;
	break;
      }
    }
    // k could be -1, when a new field/source is found
    // else it is >=0 and the index into the field array

    if (Debug(1)) {
      cout << "POINTING: " << npoint 
	   << " source: " << object_p << " [" << j << "," << k << "] "
	   << " dra/ddec: "   << dra_p << " " << ddec_p << endl;
    }
    
    if (k<0) {                             // we have a new source/field combination
      ifield = nfield;
      nfield++;
      if (Debug(2)) cout << "Adding new field " << ifield 
			 << " for " << object_p << " " << source_p[j] 
			 << " at " 
			 << dra_p *206264.8062 << " " 
			 << ddec_p*206264.8062 << " arcsec." << endl;
      if (Debug(1)) show();

      if (nfield >= MAXFIELD) {
	cout << "Cannot handle more than " << MAXFIELD << " fields." << endl;
	exit(1);
      }
      ra[ifield]  = ra_p;
      dec[ifield] = dec_p;
      dra[ifield]  = dra_p;
      ddec[ifield] = ddec_p;
      field[ifield] = j;
      sid_p[ifield] = j + 1;
      if (dra_p == 0.0 && ddec_p==0.0) {   // store ra/dec for SOURCE table as well 
	ras_p[j]  = ra_p;
	decs_p[j] = dec_p;
	sid_p[ifield] = -sid_p[ifield];    // make central one -index for later NAME change 
      }
    } else {
      ifield = k;
    }

    if (Debug(3)) cout << "Warning: pointing " << j 
        << " (dra/ddec) changed at record " << record << " : " 
        << dra_p *206264.8062 << " " 
        << ddec_p*206264.8062 << endl;
  }
} // Tracking()

//
//  this is also a nasty routine. It makes assumptions on
//  a relationship between narrow and window averages
//  which normally exists for CARMA telescope data, but which
//  in principle can be modified by uvcat/uvaver and possibly
//  break this routine...
//  (there has been some talk at the site to write subsets of
//   the full data, which could break this routine)

// ==============================================================================================
void Importmiriad::init_window(Block<Int>& narrow, Block<Int>& window)
{
  if (Debug(1)) cout << "Importmiriad::init_window" << endl;

  char vtype[10];
  Int k, idx, vlen, vupd, nchan, nspect, nwide;

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

    uvprobvr_c(uv_handle_p,"ifchain",vtype,&vlen,&vupd);
    if (vupd)
      uvgetvr_c(uv_handle_p,H_INT,"ifchain",(char *)win.chain, nspect);
  }

  if (nwide > 0 && nwide <= MAXWIDE) {
    uvprobvr_c(uv_handle_p,"wfreq",vtype,&vlen,&vupd);
    if (vupd)
      uvgetvr_c(uv_handle_p,H_REAL,"wfreq",(char *)win.wfreq, nwide);
    uvprobvr_c(uv_handle_p,"wwidth",vtype,&vlen,&vupd);
    if (vupd)
      uvgetvr_c(uv_handle_p,H_REAL,"wwidth",(char *)win.wwidth, nwide);
  }

  if (nwide >0 && nspect != nwide) {
    // don't know how to handle this
    // we could assume the smaller one is the one we should deal with
    // but there's no way to check how select=win() was used...
    // also, if you've used uvcat options=nowide, nwide=0 and nspect non-zero.
    // we really don't care about the wide bands anymore
    if (nwide < nspect)
      throw(AipsError("nspect != nwide"));
    else {
      nwide = nspect;
    }
  }

  for (Int i=0; i<nspect; i++) {
    win.code[i] = 'N';
    win.keep[i] = 1;
  }
  if (nspect > 0 && narrow[0] > 0) {         // fix up the keep[] array from the narrow= keyword
    for (Int j=0; j<nspect; j++)                 // flag all so we don't keep them
      win.keep[j] = 0;
    for (uInt j=0; j<narrow.nelements(); j++) {   // keep the ones listed in the narrow= keyword
      k = narrow[j]-1;
      if (k >= 0 || k < nspect)
	win.keep[k] = 1;
      else
	cout << "### Warning: bad narrow spectral window id " << k+1 << endl;
    }
  }

  idx = (nspect > 0 ? nspect : 0);           // idx points into the combined win.xxx[] elements
  for (Int i=0; i<nwide; i++) {
    Int side = (win.sdf[i] < 0 ? -1 : 1);
    win.code[idx]     = 'S';
    win.keep[idx]     = 1;
    win.ischan[idx]   = nchan + i + 1;
    win.nschan[idx]   = 1;
    win.sfreq[idx]    = win.wfreq[i];
    win.sdf[idx]      = side * win.wwidth[i];
    win.restfreq[idx] = -1.0;  // no meaning
    idx++;
  }

  if (nwide > 0) {
    idx = nspect;
    if (window[0] > 0) {
      for (Int j=0; j<nwide; j++)
	win.keep[idx+j] = 0;
      for (uInt j=0; j<window.nelements(); j++) {
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
    cout << "(N=narrow    W=wide,   S=spectral window averages)" << endl;

    for (Int i=0; i<nspect+nwide; i++)
      cout << win.code[i] << ": " << i+1  << " " << win.keep[i] << " "
	   << win.nschan[i] << " " << win.ischan[i] << " " 
	   << win.sfreq[i] <<  " " << win.sdf[i] <<  " " << win.restfreq[i]
	   << "\n";

    cout << "narrow: " ;
    for (uInt i=0; i<narrow.nelements(); i++)
	cout << narrow[i] << " ";
    cout << endl;
    cout << "win: " ;
    for (uInt i=0; i<window.nelements(); i++)
	cout << window[i] << " ";
    cout << endl;
  }
}

// ==============================================================================================
void Importmiriad::update_window()
{
  if (Debug(1)) cout << "Importmiriad::update_window" << endl;
  throw(AipsError("Cannot update window configuration yet"));
}

// ==============================================================================================
void Importmiriad::show()
{
#if 0
  cout << "Importmiriad::show()" << endl;
  for (int i=0; i<source_p.nelements(); i++)  cout << "SOURCE_P_1: " << source_p[i] << endl;
#endif
}

// ==============================================================================================
void Importmiriad::close()
{
  // does nothing for now
}



// ==============================================================================================
int main(int argc, char **argv)
{
  Bool show_info = False;

  // cout << "Importmiriad::START" << endl;
  try {
    
    // Define inputs
    Input inp(1);
    inp.version("4 - Miriad to MS filler (08-May-2014)");
    inp.create("mirfile",   "",      "Name of Miriad dataset name",       "string");    
    inp.create("vis",      "",       "Name of MeasurementSet",            "string");    
    inp.create("tsys",    "False",   "Fill WEIGHT from Tsys in data?",    "bool");
    inp.create("spw",     "all",     "Which of the spectral windows",     "string");
    inp.create("vel",     "",        "Velocity system: LSRK,LSRD,TOPO?",  "string");
    inp.create("linecal", "False",   "Apply CARMA linecal on the fly?",   "bool");
    inp.create("wide",     "all",    "Which of the window averages",      "string");
    inp.create("arrays",  "False",   "DEBUG: Split multiple arrays?",     "bool");
    inp.create("useTSM",  "True",    "Use the TiledStorageManager",       "bool");        
    inp.readArguments(argc, argv);

    String mirfile(inp.getString("mirfile"));
    if (mirfile=="")
      throw(AipsError("No input file mirfile= given"));
    
    String ms(inp.getString("vis"));
    if(ms=="") ms=mirfile.before('.') + ".ms";

    Bool useTSM  =  inp.getBool("useTSM");      // historic: we used to use UseISM
    Bool Qtsys   =  inp.getBool("tsys");        //
    Bool Qarrays =  inp.getBool("arrays");      // debug
    Bool Qlinecal = inp.getBool("linecal");     // 
    String vel   = inp.getString("vel");       // LSRK, LSRD,TOPO

    File t(mirfile);                                // only used for sanity checks
    Int i, debug = -1;
    Block<Int> narrow, win;

    for (i=0; i<99; i++)      // hmm, must this be so hard ??
      if  (!inp.debug(i)) {
        debug = i-1;
        break;
      }
    if (debug>1) show_info = True;

    if (!t.isDirectory())
      throw(AipsError("Input file does not appear to be miriad dataset"));

    if (inp.getString("spw") == "all") {
      narrow.resize(1);
      narrow[0] = -1;
    } else
      narrow = inp.getIntArray("spw");

    if (inp.getString("wide") == "all") {
      win.resize(1);
      win[0] = -1;
    } else
      win = inp.getIntArray("wide");

    Importmiriad bf(mirfile,debug,Qtsys,Qarrays,Qlinecal);

    bf.checkInput(narrow,win);
    bf.setupMeasurementSet(ms,useTSM);
    bf.fillAntennaTable();    // put first array in place
    bf.fillObsTables();


    // fill the main table
    bf.fillMSMainTable();


    // fill remaining tables
    bf.fillSyscalTable();      
    bf.fillSpectralWindowTable(vel);   
    bf.fillFieldTable();
    bf.fillSourceTable();
    bf.fillFeedTable();
    bf.fixEpochReferences();
    bf.close();
    //cout << "Importmiriad::close()  Created MeasurementSet " << ms << endl;
    cout << "Created MeasurementSet " << ms << endl;
  } 
  catch (AipsError x) {
    cerr << "AiprError" << x.getMesg() << endl;
  } 

  //cout << "Importmiriad::END" << endl;
  if (show_info > 0)  show_version_info();
  return 0;
}
