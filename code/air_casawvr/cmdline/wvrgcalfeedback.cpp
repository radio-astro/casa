/**
   Bojan Nikolic <b.nikolic@mrao.cam.ac.uk>, <bojan@bnikolic.co.uk>
   Initial version December 2010. 
   
   This file is part of LibAIR and is licensed under GNU Public
   License Version 2
   
   \file wvrgcalfeedback.hpp

   Feedback to the user 

*/

#include <boost/foreach.hpp>
#include <boost/format.hpp>

#include "almawvr/libair_main.hpp"
#include "wvrgcalfeedback.hpp"


namespace LibAIR {

  void fatalMsg(const std::string &m)
  {
    std::cout<<"FATAL: "<<m
	     <<std::endl;
  }

  void errorMsg(const std::string &m)
  {
    std::cout<<"ERR: "<<m
	     <<std::endl;
  }

  void warnMsg(const std::string &m)
  {
    std::cout<<"WARN: "<<m
	     <<std::endl;
  }

  void printBanner(std::ostream& /*os*/)
  {
    std::cout<<std::endl
	     <<"WVRGCAL  -- Version "
	     <<LibAIR::version()
	     <<std::endl
	     <<std::endl
	     <<"Developed by Bojan Nikolic at the University of Cambridge as part of EU FP6 ALMA Enhancement"
	     <<std::endl
	     <<"GPLv2 License -- you have a right to the source code (see http://www.mrao.cam.ac.uk/~bn204/alma)"
	     <<std::endl
	     <<std::endl;
    
  }

  void printNoSolution(std::ostream &os)
  {
    os<<std::endl
      <<"Error: Some solution for phase correction coefficients are invalid"<<std::endl
      <<"       This normally happens because WVR data were corrupted is some way:"<<std::endl
      <<"       e.g.: there was some shadowing, the calibration arm was in the WVR beam, or other issues"
      <<std::endl<<std::endl
      <<"       Aborting the calibration process and not writing a gain cal table"
      <<std::endl;
  }

  void printUsedStates(const std::set<size_t> &useID)
  {
    std::cout<<std::endl
	     <<"Good state IDs: ";
    BOOST_FOREACH(const size_t &x, useID)
      std::cout<<x<<", ";
    std::cout<<std::endl;
    
  }


  std::ostream & boolYesNo(std::ostream &os, bool b)
  {
    if (b)
      os<<"Yes";
    else
      os<<"No";
    return os;
  }

  std::ostream & operator<<(std::ostream &os, const AntennaInfo &ai)
  {
    os<<ai.no<<"\t"<<ai.name<<"\t";
    boolYesNo(os, ai.haswvr);
    os<<"\t";
    boolYesNo(os,ai.flag);
    os<<boost::format("\t%4.3g") % (ai.pathRMS/1e-6)
      <<"\t"
      <<boost::format("\t%4.3g") % (ai.pathDisc/1e-6);
    return os;
  }

  AntITable::AntITable(const aname_t &names,
		       const std::set<int> &flag,
                       const std::set<int> &nowvr,                       
		       const std::vector<double> &rms,
		       const std::vector<double> &disc,
		       const std::set<int> &interpolImpossibleAnts)
  {
    for(aname_t::const_iterator i=names.begin(); i !=names.end(); ++i)
    {
      AntennaInfo x;
      x.no= i->left;
      x.name= i->right;
      x.haswvr=(not (nowvr.count(x.no)));
      x.flag=flag.count(x.no);
      x.pathRMS=rms[x.no];
      x.pathDisc=disc[x.no];
      if((x.flag>0) && (interpolImpossibleAnts.count(x.no)>0))
      {
	x.pathRMS = 0.;
	x.pathDisc = 0.;
      }
      push_back(x);
    }
  }

  std::ostream & operator<<(std::ostream &os, const AntITable &at)
  {
    os<<"     Antenna/WVR information:                     "<<std::endl
      <<"-----------------------------------------------------------------------"<<std::endl;
    os<<"#"<<"\t"<<"Name"<<"\t"<<"WVR?"<<"\t"<<"Flag?"<<"\t"<<"RMS (um)" << "\t"<<"Disc (um)" 
      <<std::endl;
    BOOST_FOREACH(const AntennaInfo &x, at)
      os<<x<<std::endl;
    return os;
  }

  void printStatTimes(std::ostream &os,
		      const std::vector<double> &time,
		      const std::vector<std::pair<double, double> > &tmask)
  {
    double tbase=time[0];
    os<<"Times used for the statistics calculation (in seconds from first astro datum) "<<std::endl
      <<"-----------------------------------------------------------------------------"<<std::endl;
    for(size_t i=0; i<tmask.size(); ++i)
    {
      os<<"("<<tmask[i].first-tbase<<", "<<tmask[i].second-tbase<<")"
	<<std::endl;
    }

  }

}


