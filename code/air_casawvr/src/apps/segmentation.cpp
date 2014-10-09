/**
   Bojan Nikolic <b.nikolic@mrao.cam.ac.uk>, <bojan@bnikolic.co.uk>
   Initial version March 2010.
   Maintained by ESO since 2013.

   This file is part of LibAIR and is licensed under GNU Public
   License Version 2

   \file segmentation.cpp

*/

#include "segmentation.hpp"

namespace LibAIR2 {

  void fieldSegments(const std::vector<double> &time,
		     const std::vector<int> &fieldID,
		     std::vector<std::pair<double, double> > &res)
  {
    res.clear();
    std::pair<double, double> cp;
    cp.first=time[0];
    int cf=fieldID[0];
    for(size_t i=1; i<fieldID.size(); ++i)
    {
      if (fieldID[i]!=cf)
      {
	cp.second=time[i-1];
	res.push_back(cp);
	cp.first=time[i];
	cf=fieldID[i];
      }
    }
    cp.second=time[fieldID.size()-1];
    res.push_back(cp);
  }

  bool areTied(const std::vector<std::set<size_t> > &tied,
	       size_t i,
	       size_t j)
  {
    for(size_t k=0; k<tied.size(); ++k)
    {
      if (tied[k].count(i) && tied[k].count(j))
      {
	// i and j are in the same set, therefore not a transition
	return true;
      }
    }
    return false;
  }
  void fieldSegmentsTied(const std::vector<double> &time,
			 const std::vector<int> &fieldID,
			 const std::vector<std::set<size_t> > &tied,
			 std::vector<std::pair<double, double> > &res)
  {
    res.clear();
    std::pair<double, double> cp;
    cp.first=time[0];
    int cf=fieldID[0];
    for(size_t i=1; i<fieldID.size(); ++i)
    {
      if (fieldID[i]!=cf)
      {
	if (not areTied(tied, fieldID[i], cf))
	{
	  cp.second=time[i-1];
	  res.push_back(cp);
	  cp.first=time[i];
	}
	cf=fieldID[i];
      }
    }
    cp.second=time[fieldID.size()-1];
    res.push_back(cp);
  }

  void fieldTimes(const std::vector<double> &time,
		  const std::vector<int> &fieldID,
		  const std::vector<size_t> &spw,
		  const std::set<size_t>  &fieldselect,
		  size_t spwselect,
		  std::vector<std::pair<double, double> > &res)
  {
    res.resize(0);
    size_t b=0;
    size_t l=0;
    bool infield=false;
    for (size_t i=0; i<time.size(); ++i)
    {
      // Skip all rows which are not in our SPW
      if (spw[i] !=spwselect)
	continue;

      if ( (not infield) && fieldselect.count(fieldID[i]))
      {
	infield=true;
	b=i;
      }
      
      if (infield && ( (not fieldselect.count(fieldID[i]))  || (i==time.size()-1)))
      {
	infield=false;
	res.push_back(std::pair<double, double>(time[b], time[l]));
      }
      // Keep the time of last iteration
      l=i;
    }
  }

}
