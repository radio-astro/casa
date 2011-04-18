//# SimpleComponentFTMachine.cc: Implementation of SimpleComponentFTMachine class
//# Copyright (C) 1997,1998,1999,2000,2001
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

#include <synthesis/MeasurementComponents/SimpleComponentFTMachine.h>
#include <scimath/Mathematics/RigidVector.h>
#include <components/ComponentModels/ComponentShape.h>
#include <components/ComponentModels/ComponentList.h>
#include <components/ComponentModels/ComponentType.h>
#include <components/ComponentModels/Flux.h>
#include <components/ComponentModels/SkyComponent.h>
#include <components/ComponentModels/SpectralModel.h>
#include <msvis/MSVis/VisBuffer.h>
#include <casa/Arrays/ArrayMath.h>
#include <casa/Arrays/Cube.h>
#include <casa/Arrays/Matrix.h>
#include <casa/Arrays/Vector.h>
#include <casa/Arrays/IPosition.h>
#include <casa/BasicSL/Complex.h>
#include <casa/BasicSL/Constants.h>

namespace casa { //# NAMESPACE CASA - BEGIN

void SimpleComponentFTMachine::get(VisBuffer& vb, SkyComponent& component,
				   Int row)
{
  // If row is -1 then we pass through all rows
  uInt startRow, endRow;
  if (row < 0) {
    startRow = 0;
    endRow = vb.nRow() - 1;
  } else {
    startRow = endRow = row;
  }
  const uInt nRow = endRow - startRow + 1;
   
  component.flux().convertUnit(Unit("Jy"));
   // Rotate the uvw
  Matrix<Double> uvw(3, nRow);
  Vector<Double> dphase(nRow);
  {
    const Vector<RigidVector<Double,3> >& uvwBuff = vb.uvw();
    for (uInt i = startRow, n = 0; i <= endRow; i++, n++) {
      const RigidVector<Double,3>& uvwValue = uvwBuff(i);
      //NEGATING THE UV to be consitent with the GridFT::get
      for (Int idim = 0; idim < 2; idim++) {
	uvw(idim, n) = -uvwValue(idim);
      }
      uvw(2, n) = uvwValue(2);
    }
    rotateUVW(uvw, dphase, vb, component.shape().refDirection());
    dphase *= -C::_2pi;
  }
  const uInt npol=vb.visCube().shape()(0);
  const uInt nChan=vb.visCube().shape()(1);
  Cube<Complex> modelData;
  modelData.reference(vb.modelVisCube());
  Vector<Complex> visibility(4);
  Double phase;
  Double phaseMult;
  const Vector<Double>& frequency = vb.frequency();
  const Vector<Double> invLambda = frequency/C::c;
   
  // Find the offsets in polarization. 
  Vector<Int> corrType = vb.corrType().copy();
  {
    Int startPol = corrType(0);
    if((startPol > 4) && (startPol < 9)) {
      component.flux().convertPol(ComponentType::CIRCULAR);
      corrType -= 5;
    }
    else if((startPol > 8) && (startPol < 13)) {
      component.flux().convertPol(ComponentType::LINEAR);
      corrType -= 9;
    }
    else {
      component.flux().convertPol(ComponentType::STOKES);
      corrType -= startPol;
    }
  }

  Cube<DComplex> dVis(4, nChan, nRow);
  component.visibility(dVis, uvw, frequency);

  // Loop over all rows
  for (uInt r = 0; r < nRow; r++) {
    phaseMult = dphase(r);
    for (uInt chn = 0; chn < nChan; chn++) {
      phase = phaseMult * invLambda(chn);
      Complex phasor(cos(phase), sin(phase));
      for (uInt pol=0; pol < npol; pol++) {
	const DComplex& val = dVis(corrType(pol), chn, r);
	modelData(pol, chn, r) = Complex(val.real(), val.imag()) * conj(phasor);
      }
    }
  }
}



void SimpleComponentFTMachine::get(VisBuffer& vb, const ComponentList& compList,
				   Int row)
{
  // If row is -1 then we pass through all rows
  uInt startRow, endRow;
  if (row < 0) {
    startRow = 0;
    endRow = vb.nRow() - 1;
  } else {
    startRow = endRow = row;
  }
  const uInt nRow = endRow - startRow + 1;
   
   // Rotate the uvw
  Matrix<Double> uvw0(3, nRow);
  Matrix<Double> uvw(3, nRow); 
  Vector<Double> dphase(nRow);

  const Vector<RigidVector<Double,3> >& uvwBuff = vb.uvw();
  for (uInt i = startRow, n = 0; i <= endRow; i++, n++) {
    const RigidVector<Double,3>& uvwValue = uvwBuff(i);
    //NEGATING the UV to be consitent with GridFT::get
    for (Int idim = 0; idim < 2; idim++) {
      uvw(idim, n) = -uvwValue(idim);
    }
    uvw(2, n) = uvwValue(2);
  }

  uInt ncomponents=compList.nelements();
  const uInt npol=vb.visCube().shape()(0);
  const uInt nChan=vb.visCube().shape()(1);
  Cube<Complex> modelData;
  modelData.reference(vb.modelVisCube());
  modelData=0.0;

  Vector<Complex> visibility(4);
  Double phase;
  Double phaseMult;
  const Vector<Double>& frequency = vb.frequency();
  const Vector<Double> invLambda = frequency/C::c;
  // Find the offsets in polarization. 
  Vector<Int> corrTypeL = vb.corrType().copy();
  Vector<Int> corrTypeC = vb.corrType().copy();
  Vector<Int> corrType = vb.corrType().copy();
  corrTypeL -= 9;
  corrTypeC -= 5;
  Cube<DComplex> dVis(4, nChan, nRow);
  
  for (uInt icomp=0;icomp<ncomponents;icomp++) {
    SkyComponent component=compList.component(icomp).copy();
    component.flux().convertUnit(Unit("Jy"));
    if(vb.polFrame()==MSIter::Linear) {
      component.flux().convertPol(ComponentType::LINEAR);
      corrType = corrTypeL;
    } else {
      component.flux().convertPol(ComponentType::CIRCULAR);
      corrType = corrTypeC;
    }
    
    uvw = uvw0.copy();
    rotateUVW(uvw, dphase, vb, component.shape().refDirection());
    dphase *= -C::_2pi;

    component.visibility(dVis, uvw, frequency);
  
    // Loop over all rows
    for (uInt r = 0; r < nRow; r++) {
      phaseMult = dphase(r);
      for (uInt chn = 0; chn < nChan; chn++) {
	phase = phaseMult * invLambda(chn);	
	Complex phasor(cos(phase), sin(phase));
	for (uInt pol=0; pol < npol; pol++) {
	  const DComplex& val = dVis(corrType(pol), chn, r);
	  modelData(pol, chn, r) += Complex(val.real(), val.imag()) * conj(phasor);
	}
      }
    }
  }
}



} //# NAMESPACE CASA - END

