/**
   Bojan Nikolic <b.nikolic@mrao.cam.ac.uk>, <bojan@bnikolic.co.uk>
   Initial version January 2010. 
   
   This file is part of LibAIR and is licensed under GNU Public
   License Version 2
   
   \file msgaintable.cpp

   
*/

#include "msgaintable.hpp"

#include <synthesis/CalTables/NewCalTable.h>
#include <synthesis/CalTables/CalTable.h>
#include <synthesis/CalTables/SolvableVJTable.h>
#include <synthesis/CalTables/CalSet.h>
#include <synthesis/CalTables/CalHistRecord.h>

#include "almawvr/libair_main.hpp"
#include "almawvr/arraygains.hpp"
#include "msspec.hpp"
#include "almawvr/dispersion.hpp"

// These are used to turn the directory where the data files live into
// a string
#define bnstringer1(x) #x
#define bnstringer2(x) bnstringer1(x)

namespace LibAIR {


  void writeGainTbl(const ArrayGains &g,
		    const char *fnameout,
		    const MSSpec &s,
		    std::set<size_t> reverse,
		    bool disperse,
		    const std::string &msname)
  {

    using namespace casa;

    const size_t nAnt=g.nAnt;
    const size_t ntimes=g.g_time().size();
    const size_t nspw=s.spws.size();
    
    Vector<Int> nChan(nspw, 1);
    Vector<Int> nTime(nspw, ntimes);

    CalSet<Complex> cs(nspw, 
		       1, 
		       nChan,
		       nAnt,
		       nTime);
    cs.initCalTableDesc("T Jones",
			VisCalEnum::COMPLEX);

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

    for(size_t i=0; i<ntimes; ++i)
    {
      const size_t slot=i;

      for(size_t spw=0; spw<nspw; ++spw)
      {
	const size_t nch=s.spws[spw].chf.size();
	
	//cs.stateId(spw)(slot)=0;
	cs.fieldId(spw)(slot)=g.g_field()[i];
	cs.time(spw)(slot)=g.g_time()[i];
	
	cs.startTime(spw)(slot)=g.g_time()[i]+deltat*(-0.5);
	cs.stopTime(spw)(slot) =g.g_time()[i]+deltat*(0.5);
	
	cs.iFit(spw).column(slot)=1.0;
	cs.iFitwt(spw).column(slot)=1.0;
	cs.fit(spw)(slot)=1.0;
	cs.fitwt(spw)(slot)=1.0;
	
	for (size_t j=0; j<nAnt; ++j)
	{
	  
	  /// This defines the block to fill
	  IPosition blc4(4,
			 // Number of parameters -1 
			 0,
			 // Number of channels -1
			 0,
			 // Number of elements (antennas) -1 
			 j,        
			 slot);
	  
	  // This defines the position to fill
	  IPosition trc4(4,
			 0,
			 0,
			 j,
			 slot);
	  

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
	  
	  const double phase=g.g_path()[i][j]*path_to_phase;
	  
	  cs.par(spw)(blc4,trc4).nonDegenerate(3) = std::complex<float>(cos(phase),
									sin(phase));
	  
	  cs.parOK(spw)(blc4,trc4).nonDegenerate(3)= True;
	  cs.parErr(spw)(blc4,trc4).nonDegenerate(3)= 0;
	  cs.parSNR(spw)(blc4,trc4).nonDegenerate(3)= 3;
	}
	cs.solutionOK(spw)(slot) = True;
      }
    }
  
    
    cs.store(fnameout,
	     "T Jones",
	     False,
	     msname);

  }

  // Write a NewCalTable
  void writeNewGainTbl(const ArrayGains &g,
		       const char *fnameout,
		       const MSSpec &s,
		       std::set<size_t> reverse,
		       bool disperse,
		       const std::string &msname,
		       const std::string &invocation)
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
    Cube<Bool>    flag(1,1,nAnt); flag.set(False);  // All unflagged
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
	for (size_t j=0; j<nAnt; ++j)
	{
	  
	  const double phase=g.g_path()[i][j]*path_to_phase;
	  
	  cpar(0,0,j)=std::complex<float>(cos(phase),sin(phase));

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
    ct.addHistoryMessage(std::string("Produced with libAIR version: ") + LibAIR::version(),
			 invocation);

    // Flush to disk
    ct.writeToDisk(fnameout);
    
  }

  void addCalHistory(const char *fnameout,
		     const std::string &invocation)

  {
    casa::CalHistoryRecord h;
    h.defineCalNotes(std::string("Produced with libAIR version: ") + LibAIR::version());
    h.defineCalParms(invocation);

    casa::CalTable c(fnameout,
		     casa::Table::Update);
    size_t j= c.nRowHistory() +1;
    c.addRowHistory();
    c.putRowHistory(j, h);
  }

}


