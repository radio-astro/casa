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

#include <singledish/MathUtils.h>
#include <singledish/SDFitTable.h>

using namespace casa;
using namespace asap;

SDFitTable::SDFitTable(const SDFitTable& other) {  
  if (other.length() > 0) {
    this->nfits_ = other.nfits_;
    this->pars_ = other.pars_;
    this->mask_ = other.mask_;
    this->funcs_ = other.funcs_;
    this->comps_ = other.comps_;
    this->frameinfo_ = other.frameinfo_;
  } else {
    this->nfits_ = 0;
  }
}


void SDFitTable::addSTLFit(const std::vector<double>& p, 
		      const std::vector<bool>& m,
		      const std::vector<std::string>& f, 
		      const std::vector<Int>& c,
		      const std::vector<std::string>& fi)
{
  pars_.push_back(p);
  mask_.push_back(m);
  funcs_.push_back(f);
  comps_.push_back(c);
  frameinfo_.push_back(fi);
  nfits_++;
}

void SDFitTable::addFit(const Vector<Double>& p, 
		   const Vector<Bool>& m,
		   const Vector<String>& f, 
		   const Vector<Int>& c,
		   const Vector<String>& fi)
{
  std::vector<double> p1;
  p.tovector(p1);
  pars_.push_back(p1);
  std::vector<bool> m1;
  m.tovector(m1);
  mask_.push_back(m1);
  std::vector<string> f1;
  f1 = mathutil::tovectorstring(f);
  funcs_.push_back(f1);
  std::vector<int> c1;
  c.tovector(c1);
  comps_.push_back(c1);
  std::vector<string> fi1;
  fi1 = mathutil::tovectorstring(fi);
  frameinfo_.push_back(fi1);
  nfits_++;
}

std::vector<double> SDFitTable::getSTLParameters(int which) const
{
  if (which >= nfits_) 
    return std::vector<double>();  
  return pars_[which];
}

Vector<Double> SDFitTable::getParameters(uInt which) const{
  if (int(which) >= nfits_) 
    return Vector<Double>(std::vector<double>());  
  return Vector<Double>(pars_[which]);
}

std::vector<bool> SDFitTable::getSTLParameterMask(int which) const
{
  if (which >= nfits_) 
    return  std::vector<bool>();
  return mask_[which];
}

Vector<Bool> SDFitTable::getParameterMask(uInt which) const
{
  if (which >= nfits_)
    return  Vector<Bool>(std::vector<bool>());  
  return Vector<Bool>(mask_[which]);
}

std::vector<std::string> SDFitTable::getSTLFunctions(int which) const
{
  if (which >= nfits_) 
    return std::vector<std::string>();  
  return funcs_[which];
}

Vector<String> SDFitTable::getFunctions(uInt which) const
{
  if (int(which) >= nfits_) 
    return mathutil::toVectorString(std::vector<std::string>());
  return mathutil::toVectorString(funcs_[which]);
}

std::vector<int> SDFitTable::getSTLComponents(int which) const
{
  if (which >= nfits_) 
    return std::vector<int>();  
  return comps_[which];
}

Vector<Int> SDFitTable::getComponents(uInt which) const
{
  if (int(which) >= nfits_) 
    return Vector<Int>(std::vector<int>());  
  return Vector<Int>(comps_[which]);
}

std::vector<std::string> SDFitTable::getSTLFrameInfo(int which) const
{
  if (which >= nfits_) 
    return std::vector<std::string>();  
  return frameinfo_[which];
}

Vector<String> SDFitTable::getFrameInfo(uInt which) const
{
  if (int(which) >= nfits_) 
    return mathutil::toVectorString(std::vector<std::string>());
  return mathutil::toVectorString(frameinfo_[which]);
}

