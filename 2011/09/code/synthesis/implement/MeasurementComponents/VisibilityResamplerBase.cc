// -*- C++ -*-
//# VisibilityResamplerBase.cc: Implementation of the VisibilityResamplerBase class
//# Copyright (C) 1997,1998,1999,2000,2001,2002,2003
//# Associated Universities, Inc. Washington DC, USA.
//#
//# This library is free software; you can redistribute it and/or modify it
//# under the terms of the GNU Library General Public License as published by
//# the Free Software Foundation; either version 2 of the License, or (at your
//# option) any later version.
//#
//# This library is distributed in the hope that it will be useful, but WITHOUT
//# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
//# FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
//# License for more details.
//#
//# You should have received a copy of the GNU Library General Public License
//# along with this library; if not, write to the Free Software Foundation,
//# Inc., 675 Massachusetts Ave, Cambridge, MA 02139, USA.
//#
//# Correspondence concerning AIPS++ should be addressed as follows:
//#        Internet email: aips2-request@nrao.edu.
//#        Postal address: AIPS++ Project Office
//#                        National Radio Astronomy Observatory
//#                        520 Edgemont Road
//#                        Charlottesville, VA 22903-2475 USA
//#
//# $Id$

#include <synthesis/MeasurementComponents/SynthesisError.h>
#include <synthesis/MeasurementComponents/VisibilityResamplerBase.h>
#include <synthesis/MeasurementComponents/Utils.h>
#include <msvis/MSVis/AsynchronousTools.h>
#include <fstream>

namespace casa{
  //
  //-----------------------------------------------------------------------------------
  //
  void VisibilityResamplerBase::copy(const VisibilityResamplerBase& other)
  {
    SynthesisUtils::SETVEC(uvwScale_p, other.uvwScale_p);
    SynthesisUtils::SETVEC(offset_p, other.offset_p);
    SynthesisUtils::SETVEC(dphase_p, other.dphase_p);
    SynthesisUtils::SETVEC(chanMap_p, other.chanMap_p);
    SynthesisUtils::SETVEC(polMap_p, other.polMap_p);

    convFuncStore_p = other.convFuncStore_p;
  }
  //
  //-----------------------------------------------------------------------------------
  //
  VisibilityResamplerBase& VisibilityResamplerBase::operator=(const VisibilityResamplerBase& other)
  {
    copy(other);
    return *this;
  }
  //
  //-----------------------------------------------------------------------------------
  //
  // template void VisibilityResamplerBase::addTo4DArray(DComplex* store,const Int* iPos, Complex& val, Double& wt) __restrict__;
  // template void VisibilityResamplerBase::addTo4DArray(Complex* store,const Int* iPos, Complex& val, Double& wt) __restrict__;
  //
  //-----------------------------------------------------------------------------------
  //
  void VisibilityResamplerBase::sgrid(Int& uvwDim,
				      Double* __restrict__ pos, 
				      Int* __restrict__ loc, 
				      Int* __restrict__ off, 
				      Complex& phasor, const Int& irow,
				      // const Matrix<Double>& __restrict__ uvw, 
				      const Double* __restrict__ uvw, 
				      const Double&  dphase, 
				      const Double&  freq, 
				      const Double* __restrict__ scale, 
				      const Double* __restrict__ offset,
				      const Float* __restrict__ sampling) __restrict__ 
				  // const Vector<Double>& __restrict__ scale, 
				  // const Vector<Double>& __restrict__ offset,
				  // const Vector<Float>& __restrict__ sampling) __restrict__ 
  {
    Double phase;
    Int ndim=2;

    for(Int idim=0;idim<ndim;idim++)
      {
	pos[idim]=scale[idim]*uvw[idim+irow*uvwDim]*freq/C::c+offset[idim];
	loc[idim]=(Int)std::floor(pos[idim]+0.5);
	off[idim]=(Int)std::floor(((loc[idim]-pos[idim])*sampling[idim])+0.5);
      }

    if (dphase != 0.0)
      {
	phase=-2.0*C::pi*dphase*freq/C::c;
	phasor=Complex(cos(phase), sin(phase));
      }
    else
      phasor=Complex(1.0);
  }
};// end namespace casa
