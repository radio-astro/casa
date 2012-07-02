//
// C++ Implementation: STPol
//
// Description:
//
//
// Author: Malte Marquarding <asap@atnf.csiro.au>, (C) 2005
//
// Copyright: See COPYING file that comes with this distribution
//
//

#include <casa/Exceptions/Error.h>
#include "STPol.h"

std::map<std::string, std::pair<int, std::string> > asap::STPol::polmap_;
std::map<std::string, std::map<int, std::string> > asap::STPol::labelmap_;

void asap::STPol::initLabelMap()
{
  if ( labelmap_.empty() ) {
    std::map<int, std::string> linears;
    linears[0] = "XX";linears[1] = "YY";
    linears[2] = "Re(XY)";linears[3] = "Imag(XY)";
    STPol::labelmap_["linear"] = linears;
    std::map<int, std::string> circulars;
    circulars[0] = "RR";circulars[1] = "LL";
    circulars[2] = "Re(RL)";circulars[3] = "Imag(RL)";
    STPol::labelmap_["circular"] = circulars;
    std::map<int, std::string> stokes;
    stokes[0]="I";stokes[1]="Q";stokes[2]="U";stokes[3]="V";
    STPol::labelmap_["stokes"] = stokes;
    std::map<int, std::string> linpol;
    linpol[1] = "Plinear";linpol[2] = "Pangle";
    STPol::labelmap_["linpol"] = linpol;
  }
}

std::string asap::STPol::getPolLabel( int index,
                                      const std::string& type)
{
  initLabelMap();
  if ( labelmap_.find(type) ==  labelmap_.end() ) {
    std::string msg = "Illegal polarisation type "+type;
    throw(casa::AipsError(msg));
  } else {
    std::map<int, std::string>  poltype = labelmap_[type];
    if ( poltype.find(index) ==  poltype.end() ) {
      std::string msg = "Illegal polarisation index";
      throw(casa::AipsError(msg));
    } else {
      return poltype[index];
    }
  }
}

void asap::STPol::initPolMap( )
{
  if ( polmap_.empty() ) {
    std::pair<int, std::string> val;
    val.first = 0; val.second = "linear";
    STPol::polmap_["XX"] = val;
    val.first = 1;
    STPol::polmap_["YY"] = val;
    val.first = 2;
    STPol::polmap_["Re(XY)"] = val;
    val.first = 3;
    STPol::polmap_["Imag(XY)"] = val;
    val.first = 0; val.second = "stokes";
    STPol::polmap_["I"] = val;
    val.first = 1;
    STPol::polmap_["Q"] = val;
    val.first = 2;
    STPol::polmap_["U"] = val;
    val.first = 3;
    STPol::polmap_["V"] = val;
    val.first = 1; val.second = "linpol";
    STPol::polmap_["Plinear"] = val;
    val.first = 2;
    STPol::polmap_["Pangle"] = val;
    val.first = 0; val.second = "circular";
    STPol::polmap_["RR"] = val;
    val.first = 1;
    STPol::polmap_["LL"] = val;
    val.first = 2;
    STPol::polmap_["Re(RL)"] = val;
    val.first = 3;
    STPol::polmap_["Imag(RL)"] = val;
  }
}

std::pair<int, std::string> asap::STPol::polFromString( const std::string& key )
{
  initPolMap();
  if ( polmap_.find(key) ==  polmap_.end() ) {
    std::string msg = "Illegal polarisation type "+key;
    throw(casa::AipsError(msg));
  } else {
    return polmap_[key];
  }
}
