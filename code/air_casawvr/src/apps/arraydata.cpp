/**
   Bojan Nikolic <b.nikolic@mrao.cam.ac.uk>, <bojan@bnikolic.co.uk>
   Initial version January 2010.
   Maintained by ESO since 2013.

   This file is part of LibAIR and is licensed under GNU Public
   License Version 2

   \file arraydata.hpp

   Structure to hold data from all WVRs  in an array
*/

#include <cmath>

#include "arraydata.hpp"

namespace LibAIR2 {


  InterpArrayData::InterpArrayData(const std::vector<double> &time, 
				   const std::vector<double> &el, 
				   const std::vector<double> &az, 
				   const std::vector<size_t> &state, 
				   const std::vector<size_t> &field, 
				   const std::vector<size_t> &source, 
				   size_t nAnts):
    time(time),
    el(el),
    az(az),
    state(state),
    field(field),
    source(source),
    wvrdata(boost::extents[time.size()][nAnts][4]),
    nAnts(nAnts)
  {
  }

  void InterpArrayData::offsetTime(double dt)
  {
    for(size_t i=0; i<time.size(); ++i)
      time[i] += dt;
  }

  void interpBadAnt(InterpArrayData &d,
		    size_t a,
		    const AntSet &aset)
  {
    size_t n=aset.size();
    const InterpArrayData::wvrdata_t &data(d.g_wvrdata());
    for(size_t i=0; i<d.g_time().size(); ++i)
    {
      for(size_t k=0; k < 4; ++k)
      {
	double p=0;
	for(AntSet::const_iterator j=aset.begin();
	    j!=aset.end();
	    ++j)
	{
	  p+=data[i][*j][k];
	}
	d.set(i, a, k, p/n);
      }
    }
  }

  void interpBadAntW(InterpArrayData &d,
		     size_t a,
		     const AntSetWeight &aset)
  {
    const InterpArrayData::wvrdata_t &data(d.g_wvrdata());
    for(size_t i=0; i<d.g_time().size(); ++i)
    {
      for(size_t k=0; k < 4; ++k)
      {
	double p=0;
	for(AntSetWeight::const_iterator j=aset.begin();
	    j!=aset.end();
	    ++j)
	{
	  p+=data[i][j->second][k]*j->first;
	}
	d.set(i, a, k, p);
      }
    }
  }

  InterpArrayData *filterState(InterpArrayData &d,
			       const std::set<size_t>& states)
  {
    std::vector<double> time;
    std::vector<double> el;
    std::vector<double> az;
    std::vector<size_t> state;
    std::vector<size_t> field;
    std::vector<size_t> src;
    for(size_t i=0; i<d.g_state().size(); ++i)
    {
      if(states.count(d.g_state()[i]))
      {
	time.push_back(d.g_time()[i]);
	el.push_back(d.g_el()[i]);
	az.push_back(d.g_az()[i]);
	state.push_back(d.g_state()[i]);
	field.push_back(d.g_field()[i]);
	src.push_back(d.g_source()[i]);
      }
    }
    std::auto_ptr<InterpArrayData> 
      res(new InterpArrayData(time, 
			      el,
			      az,
			      state,
			      field,
			      src,
			      d.nAnts));
    size_t n=0;
    for(size_t i=0; i<d.g_state().size(); ++i)
      if(states.count(d.g_state()[i]))
      {
	for (size_t j=0; j<d.nAnts; ++j)
	{
	  for(size_t k=0; k<4; ++k)
	  {
	    res->set(n, j, k,
		     d.g_wvrdata()[i][j][k]);
	  }
	}
	++n;
      }
    return res.release();
  }

  void smoothWVR(InterpArrayData &d,
		 size_t nsample)
  {
    if (nsample<2) 
      return;

    const size_t N=d.g_time().size();
    const size_t delt=(nsample)/2;
    const bool even=((nsample%2)==0);

    size_t starti=0;
    while (starti < N )
    {
      // Figure out where the end of this source/state observation is
      size_t src=d.g_source()[starti];
      size_t state=d.g_state()[starti];
      size_t endi=starti;
      while(endi<N and d.g_source()[endi] == src and d.g_state()[endi] == state)
	++endi;

      //go through each wvr channel
      for(size_t k=0; k<4; ++k)
      {
	//go through each wvr.
	for (size_t l=0; l<d.nAnts; ++l)
	{
	  //go through each sample in time
	  for(size_t i=starti; i<endi; ++i)
	  {
	    // Check if we are on edge
	    if (i< starti+delt or i>endi - delt-1)
	      continue;
	    
	    double sum=0;
	    // Go through elements to average over for new sample i
	    for(size_t j=i-delt; j<i+delt+1; ++j)
	      {
		if ( even and (j==i-delt or j==i+delt))
		  {
		    sum=sum+(d.g_wvrdata()[j][l][k]*0.5);
		  }
		else
		  {
		    sum=sum+(d.g_wvrdata()[j][l][k]);
		  }
	      }
	    d.set(i, l, k, sum/(float(nsample)));
	      
	  }
	}
      }
      starti=endi;
    }
  }
    
}


