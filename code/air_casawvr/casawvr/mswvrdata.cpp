/**
   Bojan Nikolic <b.nikolic@mrao.cam.ac.uk>, <bojan@bnikolic.co.uk>
   Initial version January 2010.
   Maintained by ESO since 2013. 
   
   This file is part of LibAIR and is licensed under GNU Public
   License Version 2
   
   \file mswvrdata.cpp

*/

#include <set>
#include <memory>
#include <iostream>
#include <iomanip>

#include "mswvrdata.hpp"
#include "msspec.hpp"
#include "msutils.hpp"
#include "casawvr_errs.hpp"

#include <casacore/ms/MeasurementSets/MeasurementSet.h>
#include <casacore/ms/MeasurementSets/MSProcessor.h>
#include <casacore/ms/MeasurementSets/MSColumns.h>
#include <casacore/casa/Utilities/GenSort.h>
#include <casacore/ms/MSOper/MSDerivedValues.h>

#include <casa/Arrays/Vector.h>
#include <casa/Quanta/MVTime.h>

#include "../src/apps/arraydata.hpp"


namespace LibAIR2 {

  SPWSet
  WVRSPWIDs(const casacore::MeasurementSet &ms)
  {
    const casacore::MSSpectralWindow & specTable(ms.spectralWindow());
    // Not using these in present algorithm
    //const casacore::MSProcessor  & proc(ms.processor());

    casacore::ROScalarColumn<casacore::Int> nc(specTable,
				       casacore::MSSpectralWindow::columnName(casacore::MSSpectralWindow::NUM_CHAN));
    
    SPWSet res;
    for(size_t i=0; i<specTable.nrow(); ++i)
    {
      if (nc(i)==4)
	res.insert(i);
    }
    return res;
  }

  std::set<size_t>
  WVRDataDescIDs(const casacore::MeasurementSet &ms,
		 const std::vector<int> &wvrspws)
  {
    SPWSet ssfull=WVRSPWIDs(ms);
    std::map<size_t, size_t> ddmap=SPWDataDescMap(ms);
    std::set<size_t> res;

    SPWSet ss;
    if(wvrspws.size()==0){
      ss = ssfull;
    }
    else{
      for(size_t i=0; i<wvrspws.size(); i++){
	if(ssfull.count(wvrspws[i])){
	  ss.insert(wvrspws[i]);
	}
      }
    }

    for(SPWSet::const_iterator si=ss.begin();
	si!=ss.end();
	++si)
    {
      if (ddmap.count(*si))
      {
	res.insert(ddmap[*si]);
      }
    }
    return res;
  }

  size_t nWVRSPWIDs(const casacore::MeasurementSet &ms)
  {
    SPWSet s=WVRSPWIDs(ms);
    return s.size();
  }

  AntSet
  WVRAntennas(const casacore::MeasurementSet &ms,
	      const std::vector<int> &wvrspws)
  {
    AntSet res=WVRAntennasFeedTab(ms, wvrspws);
    if (res.size() == 0)
    {
      res=WVRAntennasMainTab(ms, wvrspws);
    }
    return res;
  }
  
  AntSet
  WVRAntennasFeedTab(const casacore::MeasurementSet &ms,
		     const std::vector<int> &wvrspws)
  {
    const casacore::MSFeed &feedtable=ms.feed();

    casacore::ROScalarColumn<casacore::Int> ant(feedtable,
					casacore::MSFeed::columnName(casacore::MSFeed::ANTENNA_ID));

    casacore::ROScalarColumn<casacore::Int> fspw(feedtable,
					 casacore::MSFeed::columnName(casacore::MSFeed::SPECTRAL_WINDOW_ID));

    const size_t nfeeds=feedtable.nrow();
    AntSet res;
    for (size_t i=0; i<nfeeds; ++i){
      for (size_t j=0; j<wvrspws.size(); j++){
	if(fspw(i)==wvrspws[j]){
	    res.insert(ant(i));
	}
      }
    }

    return res;
    
  }

  AntSet
  WVRAntennasMainTab(const casacore::MeasurementSet &ms,
		     const std::vector<int> &wvrspws)
  {
    std::set<size_t> dsc_ids=WVRDataDescIDs(ms, wvrspws);

    casacore::ROScalarColumn<casacore::Int> c_desc_id(ms,
					  casacore::MS::columnName(casacore::MS::DATA_DESC_ID));
    casacore::ROScalarColumn<casacore::Int> a1(ms,
				       casacore::MS::columnName(casacore::MS::ANTENNA1));

    AntSet res;
    const size_t nrows=c_desc_id.nrow();    
    for(size_t i=0; i<nrows; ++i)
    {
      if (dsc_ids.count(c_desc_id(i))>0)
      {
	res.insert(a1(i));
      }
    }
    return res;
  }

  void WVRAddFlaggedAnts(const casacore::MeasurementSet &ms,
			 LibAIR2::AntSet &flaggedAnts)
  {
        // add the antennas flagged in the ANTENNA table to the set
    casacore::ROScalarColumn<casacore::Bool> antflagrow(ms.antenna(),
						casacore::MSAntenna::columnName(casacore::MSAntenna::FLAG_ROW));
    const size_t nants=ms.antenna().nrow();
    for(size_t i=0; i<nants; i++)
    {
      if(antflagrow(i)==casacore::True) // i.e. flagged
      {
	flaggedAnts.insert(i);
      }
    }
  }


  void WVRTimeStatePoints(const casacore::MeasurementSet &ms,
			  std::vector<double> &times,
			  std::vector<size_t> &states,
			  std::vector<size_t> &field,
			  std::vector<size_t> &source,
			  const std::vector<int> &wvrspws,
			  const std::vector<size_t> &sortedI)
  {
    std::set<size_t> dsc_ids=WVRDataDescIDs(ms, wvrspws);
    size_t dsc_id = *dsc_ids.begin();

    casacore::ROScalarColumn<casacore::Double> c_times(ms,
					       casacore::MS::columnName(casacore::MS::TIME));
    casacore::ROScalarColumn<casacore::Int> c_states(ms,
					     casacore::MS::columnName(casacore::MS::STATE_ID));
    casacore::ROScalarColumn<casacore::Int> c_field(ms,
					    casacore::MS::columnName(casacore::MS::FIELD_ID));

    casacore::ROScalarColumn<casacore::Int> c_desc_id(ms,
					      casacore::MS::columnName(casacore::MS::DATA_DESC_ID));
    casacore::ROScalarColumn<casacore::Int> a1(ms,
				       casacore::MS::columnName(casacore::MS::ANTENNA1));

    casacore::ROArrayColumn<casacore::Bool> c_flags(ms,
					    casacore::MS::columnName(casacore::MS::FLAG));

    std::map<size_t, size_t> srcmap=getFieldSrcMap(ms);

    times.resize(0);
    states.resize(0);

    double prev_time=0.;

    const size_t nrows=c_desc_id.nrow();    
    for(size_t ii=0; ii<nrows; ++ii)
    {

      size_t i = sortedI[ii];

      if (c_times(i)>prev_time and  // only one entry per time stamp
	  c_desc_id(i)==(int)dsc_id 
	  )
      {
	prev_time = c_times(i);

	bool haveUnflaggedWvrData=false;
	size_t iii=ii;
	while(iii<nrows && c_times(sortedI[iii])==prev_time)
	{
	  // while in this timestamp, check if there is unflagged WVR data
	  if(c_desc_id(sortedI[iii])==(int)dsc_id and 
	     casacore::allEQ(casacore::False, c_flags(sortedI[iii]))) // i.e. not flagged
	  {
	    haveUnflaggedWvrData=true;
	    break;
	  }
	  iii++;
	}
	if(haveUnflaggedWvrData)
	{
	  times.push_back(c_times(i));
	  states.push_back(c_states(i));
	  field.push_back(c_field(i));
#if __GNUC__ <= 4 and __GNUC_MINOR__ < 1
	  source.push_back(srcmap[(c_field(i))]);
#else
	  source.push_back(srcmap.at(c_field(i)));
#endif
	}
      }
    }
  }

  void loadPointing(const casacore::MeasurementSet &ms,
		    std::vector<double> &time,
		    std::vector<double> &az,
		    std::vector<double> &el)
  {
    const casacore::MSPointing &ptable=ms.pointing();
    const casacore::ROMSPointingColumns ptablecols(ptable);
    const casacore::ROArrayColumn<casacore::Double> &dir=ptablecols.direction();
    const casacore::ROScalarColumn<casacore::Double> &ptime=ptablecols.time();

    const size_t n=ptime.nrow();
    if(n==0){
      throw LibAIR2::MSInputDataError("Didn't find any POINTING data points");
    }

    time.resize(n); 
    az.resize(n); 
    el.resize(n);
    for(size_t i=0; i<n; ++i)
    {
      time[i]=ptime(i);
      casacore::Array<casacore::Double> a;
      dir.get(i, a,
	      casacore::True);
      az[i]=a(casacore::IPosition(2,0,0));
      el[i]=a(casacore::IPosition(2,1,0));
    }
  }

  /** Get the nearest pointing record to each WVR observation
   */
  bool WVRNearestPointing(const casacore::MeasurementSet &ms,
			  const std::vector<double> &time,
			  std::vector<double> &az,
			  std::vector<double> &el)
  {

    std::vector<double> ptime, paz, pel;
    bool rval = true;
    try{
      loadPointing(ms,
		   ptime,
		   paz,
		   pel);
    }
    catch(const std::runtime_error rE){
      std::cerr << std::endl << "WARNING: problem while accessing POINTING table:"
		<< std::endl << "         LibAIR2::WVRNearestPointing: " << rE.what() << std::endl;
      std::cout << std::endl << "WARNING: problem while accessing POINTING table:"
		<< std::endl << "         LibAIR2::WVRNearestPointing: " << rE.what() << std::endl;
      rval = false;
    }

    size_t wrows=time.size();
    size_t prows=ptime.size();
    
    az.resize(wrows);  
    el.resize(wrows);

    size_t pi=0;

    for (size_t wi=0; wi<wrows; ++wi)
    {
      while(pi<(prows-1) and  ptime[pi]<time[wi])
	++pi;
      az[wi]=paz[pi];
      el[wi]=pel[pi];
    }

    return rval;

  }

  /** Calculate the AZ and EL for each WVR observation based on the field table
   */
  void WVRFieldAZEl(const casacore::MeasurementSet &ms,
		       const std::vector<double> &time,
		       const std::vector<size_t> &fields,
		       std::vector<double> &az,
		       std::vector<double> &el)
  {

    casacore::MSDerivedValues msd;
    casacore::MEpoch etime;
    casacore::MDirection azel;


    msd.setMeasurementSet(ms);
    msd.setAntenna(0); // use antenna 0 as reference position

    size_t wrows=time.size();
    
    az.resize(wrows);  
    el.resize(wrows);

    for (size_t wi=0; wi<wrows; ++wi)
    {
      etime.set(casacore::MVEpoch(casacore::Quantity(time[wi], "s")));
      msd.setEpoch(etime);
      msd.setFieldCenter(fields[wi]);
      azel = msd.azel();
      az[wi]=azel.getAngle().getValue()[0];
      el[wi]=azel.getAngle().getValue()[1];
    }

    return;

  }
			  

  InterpArrayData *loadWVRData(const casacore::MeasurementSet &ms, const std::vector<int>& wvrspws,
			       std::vector<size_t>& sortedI,
			       std::set<int>& flaggedantsInMain,
			       double requiredUnflaggedFraction,
			       bool usepointing,
			       std::string offsetstable)
  {
    bool haveOffsets=false;
    casacore::Vector<casacore::Double> offsetTime;
    casacore::Vector<casacore::Int> offsetAnts;
    casacore::Matrix<casacore::Double> offsets;
    if(offsetstable!=""){
      try{
	casacore::Table offsettab(offsetstable);
	casacore::ScalarColumn<casacore::Double> timecol(offsettab, "TIME");
	casacore::ScalarColumn<casacore::Int> antcol(offsettab, "ANTENNA");
	casacore::ArrayColumn<casacore::Double> offsetcol(offsettab, "OFFSETS");
	if(offsettab.nrow()>0){
	  timecol.getColumn(offsetTime, casacore::True);
	  antcol.getColumn(offsetAnts, casacore::True);
	  offsetcol.getColumn(offsets, casacore::True);
	  haveOffsets=true;
	  //for(size_t i=0; i<offsetAnts.size(); i++){
	  //  std::cout << std::setprecision(15) << offsetTime[i] << " " << offsetAnts[i] << " " << offsets.column(i) << std::endl;
	  //}
	}
	else{
	  throw LibAIR2::MSInputDataError("Provided Offsets Table is empty");
	}
      }
      catch(std::exception rE){
	std::cerr << "ERROR reading offsets table " << offsetstable << std::endl;
	throw rE;
      }
    }

    std::set<size_t> dsc_ids=WVRDataDescIDs(ms, wvrspws);
    AntSet wvrants=WVRAntennas(ms, wvrspws);
    const size_t nAnts=ms.antenna().nrow();

    if(requiredUnflaggedFraction<0.){
      std::cout << "WARNING: Negative required fraction of unflagged data points. Will assume 0." << std::endl;
      requiredUnflaggedFraction=0.;
    }
    if(requiredUnflaggedFraction>1.){
      std::cout << "WARNING: Required fraction of unflagged data points > 1. Will assume 1." << std::endl;
      requiredUnflaggedFraction=1.;
    }

    casacore::ROScalarColumn<casacore::Double> maintime(ms,
						casacore::MS::columnName(casacore::MS::TIME));

    const size_t nrows=maintime.nrow();

    sortedI.resize(nrows);
    flaggedantsInMain.clear();

    {
      casacore::Vector<casacore::uInt> sortedIV(nrows);
      casacore::Vector<casacore::Double> mainTimesV = maintime.getColumn();
      casacore::GenSortIndirect<casacore::Double>::sort(sortedIV,mainTimesV);
      for(casacore::uInt i=0; i<nrows; i++){ // necessary for type conversion
	sortedI[i] = (size_t) sortedIV(i); 
      }
    }

    std::vector<double> times, az, el;
    std::vector<size_t> states, fields, source;
    WVRTimeStatePoints(ms,
		       times,
		       states,
		       fields,
		       source,
		       wvrspws,
		       sortedI); 

    if (times.size() == 0){
      throw LibAIR2::MSInputDataError("Didn't find any WVR data points");
    }
    
    if(usepointing && !WVRNearestPointing(ms, times, az, el)){
      std::cout << "Could not get antenna pointing information from POINTING table." << std::endl;
      std::cerr << "Could not get antenna pointing information from POINTING table." << std::endl;
      usepointing=false;
    }
    if(!usepointing){
      std::cout << "Deriving antenna pointing information from FIELD table ..."	<< std::endl;
      std::cerr << "Deriving antenna pointing information from FIELD table ..."	<< std::endl;      
      WVRFieldAZEl(ms, times, fields, az, el);
    }

    std::auto_ptr<InterpArrayData> 
      res(new InterpArrayData(times, 
			      el,
			      az,
			      states,
			      fields,
			      source,
			      nAnts));

    // This holds how far we've filled in for each of the antennas

    int counter = -1;

    std::vector<size_t> nunflagged(nAnts, 0);
    std::vector<size_t> ntotal(nAnts, 0);

    casacore::ROArrayColumn<casacore::Complex> indata(ms, casacore::MS::columnName(casacore::MS::DATA));
    casacore::ROScalarColumn<casacore::Int> indsc_id(ms, casacore::MS::columnName(casacore::MS::DATA_DESC_ID));
    casacore::ROScalarColumn<casacore::Int> a1(ms, casacore::MS::columnName(casacore::MS::ANTENNA1));
    casacore::ROScalarColumn<casacore::Int> a2(ms, casacore::MS::columnName(casacore::MS::ANTENNA2));

    casacore::ROArrayColumn<casacore::Bool> inflags(ms, casacore::MS::columnName(casacore::MS::FLAG));

    casacore::ROScalarColumn<casacore::Double> c_times(ms, casacore::MS::columnName(casacore::MS::TIME));

    std::vector<size_t> offsetSortedI;
    size_t nOffRows=0;

    if(haveOffsets){ // prepare offset application
      for(casacore::uInt i=0; i<nrows; i++){
	if (a1(i) == a2(i) and 
	    dsc_ids.count(indsc_id(i)) > 0)
	  {
	    ++nOffRows;
	  }
      }

      if(offsetTime.size() != nOffRows){
	std::cout << "offsetTime.size() nOffRows nrows " << offsetTime.size() << " " 
		  <<  nOffRows << " " << nrows << std::endl;
	throw LibAIR2::MSInputDataError("Provided Offsets Table number of rows does not match the number of WVR data rows in MS.");
      }
      offsetSortedI.resize(nOffRows);

      casacore::Vector<casacore::uInt> offsetSortedIV(nOffRows);
      casacore::GenSortIndirect<casacore::Double>::sort(offsetSortedIV,offsetTime);
      for(casacore::uInt i=0; i<nOffRows; i++){ // necessary for type conversion
	offsetSortedI[i] = (size_t) offsetSortedIV(i); 
      }
    }

    //size_t offsetIndex=0;
    size_t offI=0;
    size_t offRow=0;
    double prevtime=0;

    for(size_t ii=0; ii<nrows; ++ii)
    {
      size_t i = sortedI[ii];

      if (a1(i) == a2(i) and 
	  dsc_ids.count(indsc_id(i)) > 0)
      {

	if(haveOffsets){
	  offI = offsetSortedI[offRow];
	  ++offRow;
	}

	int newtimestamp = 0;
	if(c_times(i)>prevtime)
	{
	  prevtime = c_times(i); 
	  newtimestamp = 1;
	}
	
	if(c_times(i)==times[counter+newtimestamp]) // there is data for this timestamp
	{

	  if(newtimestamp==1)
	  {
	    ++counter;
	  }

	  ++(ntotal[a1(i)]);

	  casacore::Array<casacore::Bool> fl;
	  inflags.get(i, fl, ::casacore::True);
 
	  if(casacore::allEQ(casacore::False, inflags(i))) // i.e. not flagged
	  {
	    casacore::Array<std::complex<float> > a;
	    indata.get(i,a, casacore::True);
	    bool tobsbad=false;
	    bool matchingOffset=true;
	    
	    if(haveOffsets){
	      if(offsetTime(offI) != c_times(i)){
		std::cerr << "WARNING: time disagreement: j i (offsetTime(j) - c_times(i)) "
			  << offI << " " << i << " " << offsetTime(offI) - c_times(i)
			  << std::endl;
		matchingOffset=false;
	      }
	      if(offsetAnts(offI) != a1(i)){
		std::cerr << "WARNING: antenna mismatch: j i offsetAnts(j) a1(i) "
			  << offI << " " << i << " " << offsetAnts(offI) << " " << a1(i)
			  << std::endl;
		matchingOffset=false;
	      }
	    }

	    for(size_t k=0; k<4; ++k)
            {
	      casacore::Double rdata = a(casacore::IPosition(2,k,0)).real();
		
	      if(haveOffsets && matchingOffset){
		rdata -= offsets(k, offI);
	      }

	      if(2.7<rdata and rdata<300.){
		res->set(counter,
			 a1(i),
			 k,
			 rdata);
	      }
	      else{
		tobsbad=true;
	      }
	    }
	    if(tobsbad){ // TObs outside permitted range
	      for(size_t k=0; k<4; ++k){
		res->set(counter, a1(i), k, 0.);
	      }
	    }
	    else{ 
	      nunflagged[a1(i)]++;
	    }
	  }
	  else{ // flagged
	    for(size_t k=0; k<4; ++k){
	      res->set(counter, a1(i), k, 0.);
	    }
	  }
	} // end if c_times ...
      }
    } // end for ii

    bool allFlagged=true;
    for(AntSet::iterator it=wvrants.begin(); it != wvrants.end(); it++)
    {
      if(nunflagged[*it]>0)
      {
	allFlagged=false;
	break;
      }
    }
    if(!allFlagged)
    {
      allFlagged = true;
      for(AntSet::iterator it=wvrants.begin(); it != wvrants.end(); it++)
      {
	if(nunflagged[*it]==0)
	{
	  std::cout << "All WVR data points for antenna " << *it << " are flagged." << std::endl;
	  flaggedantsInMain.insert(*it);
	}
	else if(nunflagged[*it]<requiredUnflaggedFraction * ntotal[*it])
	{
	  std::cout << "The fraction of good (unflagged) WVR data points for antenna " << *it
		    << " is " << nunflagged[*it] << " out of " << ntotal[*it]
		    << ". This is below the required " << requiredUnflaggedFraction*100.
		    << "%. Antenna will be flagged." << std::endl;
	  flaggedantsInMain.insert(*it);
	}
	else{
	  allFlagged=false;
	}
      }
      if(allFlagged)
      {
	throw LibAIR2::MSInputDataError("All antennas needed to be flagged.");
      }
    }
    else
    {
      throw LibAIR2::MSInputDataError("All WVR data points are flagged.");
    }

    return res.release();
    
  }
  


}


