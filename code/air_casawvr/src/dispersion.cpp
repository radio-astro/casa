/**
   Bojan Nikolic <b.nikolic@mrao.cam.ac.uk>, <bojan@bnikolic.co.uk>
   Initial version August 2010.
   Maintained by ESO since 2013.

   This file is part of LibAIR and is licensed under GNU Public
   License Version 2

   \file dispersion.cpp
*/
#include <iostream>
#include <fstream>

#include <boost/tokenizer.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>

#include "dispersion.hpp"

namespace LibAIR2 {

  double DispersionTab::operator() (double fnu)
  {
    std::pair<double, double> b= *lower_bound(fnu);
    if (b.first==fnu)
      return b.second;

    std::pair<double, double> l= *(--lower_bound(fnu));
    std::pair<double, double> u= *upper_bound(fnu);
    
    const double f=(fnu-l.first)/(u.first-l.first);
    return l.second+ f*(u.second-l.second);
    
  }

  typedef boost::tokenizer<boost::escaped_list_separator<char> >  tok_t;

  tok_t tokLine(const std::string &line)
  {
    return tok_t(line,
		 boost::escaped_list_separator<char>( "\\",
						      ",;",
						      "\""));    
  }

  void loadCSV(const char *fname,
	       DispersionTab &dt)
  {
    std::ifstream ifs(fname);
    if (not ifs.good())
    {
      throw std::runtime_error(std::string("Could not open dispersion table ")+fname);
    }
    std::string   scratch;

    while(ifs.good())
    {
      std::getline(ifs, scratch);
      if (scratch.size() < 5)
	continue;

      tok_t tok (tokLine(scratch));

      std::string first  (*tok.begin());
      std::string second (*(++tok.begin()));
      boost::trim(first);
      boost::trim(second);
      try {
	dt.insert(dt.end(),
		  std::pair<double, double>(boost::lexical_cast<double>(first),
					    boost::lexical_cast<double>(second)
					    ));
      }
      catch (const std::bad_cast &bc)
      {
	std::cerr<<"Could not interpret " << first << " and " << second
		 <<std::endl;
      }
    }
  }


}



