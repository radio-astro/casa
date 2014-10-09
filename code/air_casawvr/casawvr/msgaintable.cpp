/**
   Bojan Nikolic <b.nikolic@mrao.cam.ac.uk>, <bojan@bnikolic.co.uk>
   Initial version January 2010.
   Maintained by ESO since 2013. 
   
   This file is part of LibAIR and is licensed under GNU Public
   License Version 2
   
   \file msgaintable.cpp

   
*/

#include "msgaintable.hpp"
#include "msspec.hpp"

#include <synthesis/CalTables/NewCalTable.h>
#include <synthesis/CalTables/CalTable.h>
#include <synthesis/CalTables/SolvableVJTable.h>
#include <synthesis/CalTables/CalSet.h>
#include <synthesis/CalTables/CalHistRecord.h>

#include "../src/libair_main.hpp"
#include "../src/apps/arraygains.hpp"
#include "../src/dispersion.hpp"

// These are used to turn the directory where the data files live into
// a string
#define bnstringer1(x) #x
#define bnstringer2(x) bnstringer1(x)

namespace LibAIR2 {

  // Write a NewCalTable
  void writeNewGainTbl(const ArrayGains &g,
		       const char *fnameout,
		       const MSSpec &s,
		       std::set<size_t> reverse,
		       bool disperse,
		       const std::string &msname,
		       const std::string &invocation,
		       const std::set<int> &interpImpossibleAnts)
  {

    using namespace casa;

    const size_t nAnt=g.nAnt;
    const size_t ntimes=g.g_time().size();
    const size_t nspw=s.spws.size();
    
    Vector<Int> nChan(nspw, 1);
    Vector<Int> nTime(nspw, ntimes);

    const double deltat=g.g_time()[1]-g.g_time()[0];

    DispersionTab dispt;
    if (disperse)
    {
      // Only bother loading the table if dispersion is going to be
      // applied
      std::string dispname;
      const char *dispdir=getenv("WVRGCAL_DISPDIR");
      if (dispdir)
      {
	dispname=std::string(dispdir)+"/libair-ddefault.csv";	
      }
      else
      {
	dispname=std::string(bnstringer2(DISPTABLEDIR))+"/libair/libair-ddefault.csv";
      }
      loadCSV(dispname.c_str(), 
	      dispt);
    }

    // Make the empty NewCaltable
    NewCalTable ct("wvrgcal",            // temporary name for mem table
		   VisCalEnum::COMPLEX,
		   "T Jones",
		   msname,
		   True);                // enforce single-chan

    // Workspace
    Cube<Complex> cpar(1,1,nAnt);                   // filled below
    Cube<Bool>    flag(1,1,nAnt);                   // filled below
    Cube<Float>   err(1,1,nAnt);  err.set(0.0);     // All zero
    Cube<Float>   snr(1,1,nAnt);  snr.set(3.0);     // All 3

    for(size_t i=0; i<ntimes; ++i)
    {
      for(size_t spw=0; spw<nspw; ++spw)
      {
	const size_t nch=s.spws[spw].chf.size();

	// The minus sign here is required to match ALMA convention
	double path_to_phase=-2 * M_PI * s.spws[spw].chf[nch/2] / 3e8;
	if (reverse.count(spw))
	  {
	    path_to_phase *= -1;
	  }
	if (disperse)
	  {
	    const double dispf=(1+dispt(s.spws[spw].chf[nch/2]));
	    path_to_phase *= dispf;
	  }
	
	
	// Generate antenna-based complex factors
	cpar.set(Complex(0.0));
	flag.set(False); // i.e. all unflagged
	for (size_t j=0; j<nAnt; ++j)
	{

	  if(interpImpossibleAnts.count(j)==0 && g.g_path()[i][j]!=0)
	  {
	    const double phase=g.g_path()[i][j]*path_to_phase;
	    cpar(0,0,j)=std::complex<float>(cos(phase),sin(phase));
	  }
	  else // there is no useful WVR data for this antenna
	  {
            cpar(0,0,j)=std::complex<float>(1.,0.);
	    flag(0,0,j)=True; 
	  }

	}

	// fill this time/spw into the table
	ct.fillAntBasedMainRows(nAnt,           // the number of rows added this call
				g.g_time()[i],  // timestamp
				deltat,         // interval
				g.g_field()[i], // field id
				spw,            // spw id
				-1,             // scan number (non-specific)
				Vector<Int>(),  // antenna1 will be auto-generated: [0,1,2,...]
				-1,             // refant (non-specific)
				cpar,           // the complex gain parameters
				flag,           // flag
				err,            // err
				snr);           // snr

      }
    }


    // Add history info (before flushing to disk)
    ct.addHistoryMessage(std::string("Produced with libAIR version: ") + LibAIR2::version(),
			 invocation);

    // Flush to disk
    ct.writeToDisk(fnameout);
    
  }

  void addCalHistory(const char *fnameout,
		     const std::string &invocation)

  {
    casa::CalHistoryRecord h;
    h.defineCalNotes(std::string("Produced with libAIR version: ") + LibAIR2::version());
    h.defineCalParms(invocation);

    casa::CalTable c(fnameout,
		     casa::Table::Update);
    size_t j= c.nRowHistory() +1;
    c.addRowHistory();
    c.putRowHistory(j, h);
  }

}


