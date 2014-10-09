/**
   Bojan Nikolic <b.nikolic@mrao.cam.ac.uk>, <bojan@bnikolic.co.uk>
   Initial version September 2010.
   Maintained by ESO since 2013.

   This file is part of LibAIR and is licensed under GNU Public
   License Version 2

   \file antennautils.cpp

*/

#include "antennautils.hpp"


namespace LibAIR2 {
  AntSetD antsDist(const antpos_t &pos,
		   size_t i,
		   const AntSet &flag)
  {
    AntSetD dist;
    for (size_t j=0; j<pos.size1(); ++j)
    {
      if (j==i or flag.count(j))
	continue;
      double d=std::sqrt(std::pow(pos(j,0)-pos(i,0),2)+
			 std::pow(pos(j,1)-pos(i,1),2)+
			 std::pow(pos(j,2)-pos(i,2),2));
      dist.insert(std::make_pair<double, size_t>(d, j));
    }
    return dist;
  }

  AntSet nearestAnts(const antpos_t &pos,
		     size_t i,
		     const AntSet &flag,
		     size_t n)
  {
    AntSetD dist=antsDist(pos, i, flag);
    AntSet res;
    AntSetD::const_iterator s=dist.begin();
    for (size_t j=0; j<n; ++j)
    {
      res.insert(s->second);
      ++s;
    }
    return res;
  }

  AntSetWeight linNearestAnt(const antpos_t &pos,
			     size_t i,
			     const AntSet &flag,
			     size_t n)
  {
    AntSetD dist=antsDist(pos, i, flag);
    AntSetWeight res;
    
    double total=0;
    AntSetD::const_iterator s=dist.begin();
    for (size_t j=0; j<n; ++j)
    {
      total+=s->first;
      ++s;
    }

    s=dist.begin();
    for (size_t j=0; j<n; ++j)
    {
      res.insert(std::make_pair<double, size_t>(s->first/total, s->second));
      ++s;
    }

    return res;

  }

}


