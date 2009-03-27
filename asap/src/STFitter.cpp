//#---------------------------------------------------------------------------
//# Fitter.cc: A Fitter class for spectra
//#--------------------------------------------------------------------------
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
//# $Id: STFitter.cpp 1387 2007-07-26 16:00:22Z TakTsutsumi $
//#---------------------------------------------------------------------------
#include <casa/aips.h>
#include <casa/Arrays/ArrayMath.h>
#include <casa/Arrays/ArrayLogical.h>
#include <scimath/Fitting.h>
#include <scimath/Fitting/LinearFit.h>
#include <scimath/Functionals/CompiledFunction.h>
#include <scimath/Functionals/CompoundFunction.h>
#include <scimath/Functionals/Gaussian1D.h>
#include <scimath/Functionals/Polynomial.h>
#include <scimath/Mathematics/AutoDiff.h>
#include <scimath/Mathematics/AutoDiffMath.h>
#include <scimath/Fitting/NonLinearFitLM.h>
#include <components/SpectralComponents/SpectralEstimate.h>

#include "STFitter.h"

using namespace asap;
using namespace casa;

Fitter::Fitter()
{
}

Fitter::~Fitter()
{
  reset();
}

void Fitter::clear()
{
  for (uInt i=0;i< funcs_.nelements();++i) {
    delete funcs_[i]; funcs_[i] = 0;
  }
  funcs_.resize(0,True);
  parameters_.resize();
  fixedpar_.resize();
  error_.resize();
  thefit_.resize();
  estimate_.resize();
  chisquared_ = 0.0;
}

void Fitter::reset()
{
  clear();
  x_.resize();
  y_.resize();
  m_.resize();
}


bool Fitter::computeEstimate() {
  if (x_.nelements() == 0 || y_.nelements() == 0)
    throw (AipsError("No x/y data specified."));

  if (dynamic_cast<Gaussian1D<Float>* >(funcs_[0]) == 0)
    return false;
  uInt n = funcs_.nelements();
  SpectralEstimate estimator(n);
  estimator.setQ(5);
  Int mn,mx;
  mn = 0;
  mx = m_.nelements()-1;
  for (uInt i=0; i<m_.nelements();++i) {
    if (m_[i]) {
      mn = i;
      break;
    }
  }
  for (uInt j=m_.nelements()-1; j>=0;--j) {
    if (m_[j]) {
      mx = j;
      break;
    }
  }
  //mn = 0+x_.nelements()/10;
  //mx = x_.nelements()-x_.nelements()/10;
  estimator.setRegion(mn,mx);
  //estimator.setWindowing(True);
  SpectralList listGauss = estimator.estimate(x_, y_);
  parameters_.resize(n*3);
  Gaussian1D<Float>* g = 0;
  for (uInt i=0; i<n;i++) {
    g = dynamic_cast<Gaussian1D<Float>* >(funcs_[i]);
    if (g) {
      (*g)[0] = listGauss[i].getAmpl();
      (*g)[1] = listGauss[i].getCenter();
      (*g)[2] = listGauss[i].getFWHM();
    }
  }
  estimate_.resize();
  listGauss.evaluate(estimate_,x_);
  return true;
}

std::vector<float> Fitter::getEstimate() const
{
  if (estimate_.nelements() == 0)
    throw (AipsError("No estimate set."));
  std::vector<float> stlout;
  estimate_.tovector(stlout);
  return stlout;
}


bool Fitter::setExpression(const std::string& expr, int ncomp)
{
  clear();
  if (expr == "gauss") {
    if (ncomp < 1) throw (AipsError("Need at least one gaussian to fit."));
    funcs_.resize(ncomp);
    for (Int k=0; k<ncomp; ++k) {
      funcs_[k] = new Gaussian1D<Float>();
    }
  } else if (expr == "poly") {
    funcs_.resize(1);
    funcs_[0] = new Polynomial<Float>(ncomp);
  } else {
    cerr << " compiled functions not yet implemented" << endl;
    //funcs_.resize(1);
    //funcs_[0] = new CompiledFunction<Float>();
    //funcs_[0]->setFunction(String(expr));
    return false;
  }
  return true;
}

bool Fitter::setData(std::vector<float> absc, std::vector<float> spec,
                       std::vector<bool> mask)
{
    x_.resize();
    y_.resize();
    m_.resize();
    // convert std::vector to casa Vector
    Vector<Float> tmpx(absc);
    Vector<Float> tmpy(spec);
    Vector<Bool> tmpm(mask);
    AlwaysAssert(tmpx.nelements() == tmpy.nelements(), AipsError);
    x_ = tmpx;
    y_ = tmpy;
    m_ = tmpm;
    return true;
}

std::vector<float> Fitter::getResidual() const
{
    if (residual_.nelements() == 0)
        throw (AipsError("Function not yet fitted."));
    std::vector<float> stlout;
    residual_.tovector(stlout);
    return stlout;
}

std::vector<float> Fitter::getFit() const
{
    Vector<Float> out = thefit_;
    std::vector<float> stlout;
    out.tovector(stlout);
    return stlout;

}

std::vector<float> Fitter::getErrors() const
{
    Vector<Float> out = error_;
    std::vector<float> stlout;
    out.tovector(stlout);
    return stlout;
}

bool Fitter::setParameters(std::vector<float> params)
{
    Vector<Float> tmppar(params);
    if (funcs_.nelements() == 0)
        throw (AipsError("Function not yet set."));
    if (parameters_.nelements() > 0 && tmppar.nelements() != parameters_.nelements())
        throw (AipsError("Number of parameters inconsistent with function."));
    if (parameters_.nelements() == 0) {
        parameters_.resize(tmppar.nelements());
        if (tmppar.nelements() != fixedpar_.nelements()) {
            fixedpar_.resize(tmppar.nelements());
            fixedpar_ = False;
        }
    }
    if (dynamic_cast<Gaussian1D<Float>* >(funcs_[0]) != 0) {
        uInt count = 0;
        for (uInt j=0; j < funcs_.nelements(); ++j) {
            for (uInt i=0; i < funcs_[j]->nparameters(); ++i) {
                (funcs_[j]->parameters())[i] = tmppar[count];
                parameters_[count] = tmppar[count];
                ++count;
            }
        }
    } else if (dynamic_cast<Polynomial<Float>* >(funcs_[0]) != 0) {
        for (uInt i=0; i < funcs_[0]->nparameters(); ++i) {
            parameters_[i] = tmppar[i];
            (funcs_[0]->parameters())[i] =  tmppar[i];
        }
    }
    // reset
    if (params.size() == 0) {
        parameters_.resize();
        fixedpar_.resize();
    }
    return true;
}

bool Fitter::setFixedParameters(std::vector<bool> fixed)
{
    if (funcs_.nelements() == 0)
        throw (AipsError("Function not yet set."));
    if (fixedpar_.nelements() > 0 && fixed.size() != fixedpar_.nelements())
        throw (AipsError("Number of mask elements inconsistent with function."));
    if (fixedpar_.nelements() == 0) {
        fixedpar_.resize(parameters_.nelements());
        fixedpar_ = False;
    }
    if (dynamic_cast<Gaussian1D<Float>* >(funcs_[0]) != 0) {
        uInt count = 0;
        for (uInt j=0; j < funcs_.nelements(); ++j) {
            for (uInt i=0; i < funcs_[j]->nparameters(); ++i) {
                funcs_[j]->mask(i) = !fixed[count];
                fixedpar_[count] = fixed[count];
                ++count;
            }
        }
    } else if (dynamic_cast<Polynomial<Float>* >(funcs_[0]) != 0) {
        for (uInt i=0; i < funcs_[0]->nparameters(); ++i) {
            fixedpar_[i] = fixed[i];
            funcs_[0]->mask(i) =  !fixed[i];
        }
    }
    return true;
}

std::vector<float> Fitter::getParameters() const {
    Vector<Float> out = parameters_;
    std::vector<float> stlout;
    out.tovector(stlout);
    return stlout;
}

std::vector<bool> Fitter::getFixedParameters() const {
  Vector<Bool> out(parameters_.nelements());
  if (fixedpar_.nelements() == 0) {
    return std::vector<bool>();
    //throw (AipsError("No parameter mask set."));
  } else {
    out = fixedpar_;
  }
  std::vector<bool> stlout;
  out.tovector(stlout);
  return stlout;
}

float Fitter::getChisquared() const {
    return chisquared_;
}

bool Fitter::fit() {
  NonLinearFitLM<Float> fitter;
  CompoundFunction<Float> func;

  uInt n = funcs_.nelements();
  for (uInt i=0; i<n; ++i) {
    func.addFunction(*funcs_[i]);
  }

  fitter.setFunction(func);
  fitter.setMaxIter(50+n*10);
  // Convergence criterium
  fitter.setCriteria(0.001);

  // Fit
  Vector<Float> sigma(x_.nelements());
  sigma = 1.0;

  parameters_.resize();
  parameters_ = fitter.fit(x_, y_, sigma, &m_);
  if ( !fitter.converged() ) {
     return false;
  }
  std::vector<float> ps;
  parameters_.tovector(ps);
  setParameters(ps);

  error_.resize();
  error_ = fitter.errors();

  chisquared_ = fitter.getChi2();

  residual_.resize();
  residual_ =  y_;
  fitter.residual(residual_,x_);
  // use fitter.residual(model=True) to get the model
  thefit_.resize(x_.nelements());
  fitter.residual(thefit_,x_,True);
  return true;
}

bool Fitter::lfit() {
  LinearFit<Float> fitter;
  CompoundFunction<Float> func;

  uInt n = funcs_.nelements();
  for (uInt i=0; i<n; ++i) {
    func.addFunction(*funcs_[i]);
  }

  fitter.setFunction(func);
  //fitter.setMaxIter(50+n*10);
  // Convergence criterium
  //fitter.setCriteria(0.001);

  // Fit
  Vector<Float> sigma(x_.nelements());
  sigma = 1.0;

  parameters_.resize();
  parameters_ = fitter.fit(x_, y_, sigma, &m_);
  std::vector<float> ps;
  parameters_.tovector(ps);
  setParameters(ps);

  error_.resize();
  error_ = fitter.errors();

  chisquared_ = fitter.getChi2();

  residual_.resize();
  residual_ =  y_;
  fitter.residual(residual_,x_);
  // use fitter.residual(model=True) to get the model
  thefit_.resize(x_.nelements());
  fitter.residual(thefit_,x_,True);
  return true;
}

std::vector<float> Fitter::evaluate(int whichComp) const
{
  std::vector<float> stlout;
  uInt idx = uInt(whichComp);
  Float y;
  if ( idx < funcs_.nelements() ) {
    for (uInt i=0; i<x_.nelements(); ++i) {
      y = (*funcs_[idx])(x_[i]);
      stlout.push_back(float(y));
    }
  }
  return stlout;
}

