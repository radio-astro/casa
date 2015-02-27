/**
   Bojan Nikolic <b.nikolic@mrao.cam.ac.uk>, <bojan@bnikolic.co.uk>
   Initial version December 2010.
   Maintained by ESO since 2013. 
   
   This file is part of LibAIR and is licensed under GNU Public
   License Version 2
   
   \file wvrgcalfeedback.hpp

   Feedback to the user 

*/
#ifndef _LIBAIR_CMDLINE_WVRGCALFEEDBACK__
#define _LIBAIR_CMDLINE_WVRGCALFEEDBACK__

#include <string>
#include <list>
#include <iostream>
#include <set>

#include "../casawvr/msantdata.hpp"


namespace LibAIR2 {

  void fatalMsg(const std::string &m);
  void errorMsg(const std::string &m);
  void warnMsg(const std::string &m);

  /// Print the wvrgcal banner
  void printBanner(std::ostream &os);
  
  /// Infor user about states_ids that we will take into accout
  void printUsedStates(const std::set<size_t> &useID);

  struct AntennaInfo {
    size_t no;
    std::string name;
    bool haswvr;
    bool flag;
    double pathRMS;
    double pathDisc;
  };
  
  std::ostream & boolYesNo(std::ostream &os, bool b);

  std::ostream & operator<<(std::ostream &os, const AntennaInfo &ai);

  struct AntITable:
    public std::list<AntennaInfo>
  {

  public:
    AntITable(const aname_t &names,
	      const LibAIR2::AntSet &flag,
	      const LibAIR2::AntSet &nowvr,
	      const std::vector<double> &rms,
	      const std::vector<double> &disc,
	      const LibAIR2::AntSet &interpolImpossibleAnts=LibAIR2::AntSet()
	      );
    
  };

  std::ostream & operator<<(std::ostream &os, 
			    const AntITable &at);

  /// Info about times used for statistics
  void printStatTimes(std::ostream &os,
		      const std::vector<double> &time,
		      const std::vector<std::pair<double, double> > &tmask);
		      

}

#endif
