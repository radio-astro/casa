//#---------------------------------------------------------------------------
//# SDFitTable.h: A wrapper for fit parameters
//#---------------------------------------------------------------------------
//# Copyright (C) 2004
//# ATNF
//#
//# This program is free software; you can redistribute it and/or modify it
//# under the terms of the GNU General Public License as published by the Free
//# Software Foundation; either version 2 of the License, or (at your option)
//# any later version.
//#
//# This program is distributed in the hope that it will be useful, but
//# WITHOUT ANY WARRANTY; without even the implied warranty of
//# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General
//# Public License for more details.
//#
//# You should have received a copy of the GNU General Public License along
//# with this program; if not, write to the Free Software Foundation, Inc.,
//# 675 Massachusetts Ave, Cambridge, MA 02139, USA.
//#
//# Correspondence concerning this software should be addressed as follows:
//#        Internet email: Malte.Marquarding@csiro.au
//#        Postal address: Malte Marquarding,
//#                        Australia Telescope National Facility,
//#                        P.O. Box 76,
//#                        Epping, NSW, 2121,
//#                        AUSTRALIA
//#
//# $Id:
//#---------------------------------------------------------------------------
#ifndef SDFITTABLE_H
#define SDFITTABLE_H

#include <casa/aips.h>
#include <casa/Arrays/Vector.h>
#include <casa/BasicSL/String.h>
#include <vector>
#include <string>

namespace asap {

class SDFitTable {
public:
  SDFitTable() : nfits_(0) {;};
  ~SDFitTable() {;};
  SDFitTable(const SDFitTable& other);

  void addFit(const casa::Vector<casa::Double>& p, 
	      const casa::Vector<casa::Bool>& m,
	      const casa::Vector<casa::String>& f, 
	      const casa::Vector<casa::Int>& c,
	      const casa::Vector<casa::String>& fi);
  
  void addSTLFit(const std::vector<double>& p, 
		 const std::vector<bool>& m,
		 const std::vector<std::string>& f, 
		 const std::vector<int>& c,
		 const std::vector<std::string>& fi);
  
  int STLlength() const { return nfits_; };
  
  casa::uInt length() const { return casa::uInt(nfits_); };
  
  std::vector<double> getSTLParameters(int which) const;
  std::vector<bool> getSTLParameterMask(int which) const;
  std::vector<std::string> getSTLFunctions(int which) const;
  std::vector<int> getSTLComponents(int which) const;
  std::vector<std::string> getSTLFrameInfo(int which) const;

  casa::Vector<casa::Double> getParameters(casa::uInt which) const;
  casa::Vector<casa::Bool> getParameterMask(casa::uInt which) const;
  casa::Vector<casa::String> getFunctions(casa::uInt which) const;
  casa::Vector<casa::Int> getComponents(casa::uInt which) const;
  casa::Vector<casa::String> getFrameInfo(casa::uInt which) const;


private:
  int nfits_;
  std::vector<std::vector<double> > pars_;
  std::vector<std::vector<bool> > mask_;
  std::vector<std::vector<std::string> > funcs_;
  std::vector<std::vector<int> > comps_;
  std::vector<std::vector<std::string> > frameinfo_;
};

}//

#endif
