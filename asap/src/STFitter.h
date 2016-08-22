//#---------------------------------------------------------------------------
//# Fitter.h: A Fitter class for spectra
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
//# $Id: STFitter.h 2666 2012-10-15 04:52:38Z MalteMarquarding $
//#---------------------------------------------------------------------------
#ifndef STFITTER_H
#define STFITTER_H

#include <string>
#include <vector>

#include <casa/Arrays/Vector.h>
#include <casa/Containers/Block.h>
#include <scimath/Functionals/Function.h>
#include <scimath/Functionals/CompoundFunction.h>
#include <scimath/Fitting/GenericL2Fit.h>


#include "STFitEntry.h"


namespace asap {

class Fitter {
public:
  Fitter();
  virtual ~Fitter();
  // allowed "gauss" and "poly". ncomp is either numvber of gaussions
  // or order of the polynomial
  bool setExpression(const std::string& expr, int ncomp=1);
  bool setData(std::vector<float> absc, std::vector<float> spec,
	       std::vector<bool> mask);
  bool setParameters(std::vector<float> params);
  bool setFixedParameters(std::vector<bool> fixed);
  void addConstraint(const std::vector<float>& constraint);

  std::vector<float> getResidual() const;
  std::vector<float> getFit() const;
  std::vector<float> getParameters() const;
  std::vector<bool> getFixedParameters() const;

  std::vector<float> getEstimate() const;
  std::vector<float> getErrors() const;
  float getChisquared() const;
  void reset();
  bool fit();
  // Fit via linear method
  bool lfit();
  bool computeEstimate();

  std::vector<float> evaluate(int whichComp) const;

  STFitEntry getFitEntry() const;

private:
  void clear();
  void applyConstraints(casa::GenericL2Fit<casa::Float>& fitter);
  casa::Vector<casa::Float> x_;
  casa::Vector<casa::Float> y_;
  casa::Vector<casa::Bool> m_;
  casa::PtrBlock<casa::Function<casa::Float>* > funcs_;
  std::vector<std::string> funcnames_;
  std::vector<int> funccomponents_;
  
  //Bool estimateSet_;
  casa::Float chisquared_;
  casa::Vector<casa::Float> parameters_;
  casa::Vector<casa::Bool> fixedpar_;
  std::vector<std::vector<float> > constraints_;

  casa::Vector<casa::Float> error_;
  casa::Vector<casa::Float> thefit_;
  casa::Vector<casa::Float> residual_;
  casa::Vector<casa::Float> estimate_;
};

} // namespace

#endif
