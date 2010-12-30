// -*- C++ -*-
//# VisibilityResampler.cc: Implementation of the VisibilityResampler class
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
#include <synthesis/MeasurementComponents/VisibilityResampler.h>
#include <fstream>

namespace casa{
  //
  //-----------------------------------------------------------------------------------
  //
  VisibilityResampler& VisibilityResampler::operator=(const VisibilityResampler& other)
  {
    SETVEC(uvwScale_p, other.uvwScale_p);
    SETVEC(offset_p, other.offset_p);
    SETVEC(dphase_p, other.dphase_p);
    SETVEC(chanMap_p, other.chanMap_p);
    SETVEC(polMap_p, other.polMap_p);

    convFuncStore_p = other.convFuncStore_p;

    return *this;
  }
  //
  //-----------------------------------------------------------------------------------
  // Re-sample the griddedData on the VisBuffer (a.k.a gridding)
  //
  // Template instantiations for re-sampling onto a double precision
  // or single precision grid.
  //
  template
  void VisibilityResampler::DataToGridImpl_p(Array<DComplex>& grid, VBStore& vbs, 
					     const Bool& dopsf,  Matrix<Double>& sumwt);
  template
  void VisibilityResampler::DataToGridImpl_p(Array<Complex>& grid, VBStore& vbs, 
  					     const Bool& dopsf,  Matrix<Double>& sumwt);

  template void VisibilityResampler::addTo4DArray(DComplex* store,const Int* iPos, Complex& val, Double& wt);
  template void VisibilityResampler::addTo4DArray(Complex* store,const Int* iPos, Complex& val, Double& wt);
  //
  //-----------------------------------------------------------------------------------
  // Template implementation for DataToGrid
  //
  template <class T>
  void VisibilityResampler::DataToGridImpl_p(Array<T>& grid,  VBStore& vbs, const Bool& dopsf,
					     Matrix<Double>& sumwt)
  {
    Int nDataChan, nDataPol, nGridPol, nGridChan, nx, ny;
    Int achan, apol, rbeg, rend;
    Vector<Float> sampling(2);
    Vector<Int> support(2),loc(2), off(2), iloc(2);
    Vector<Double> pos(2);

    //    IPosition grdpos(4);
    Vector<Int> igrdpos(4);

    Double norm=0, wt;
    Complex phasor, nvalue;

    rbeg = 0;
    rend = vbs.nRow;
    
    nx       = grid.shape()[0]; ny        = grid.shape()[1];
    nGridPol = grid.shape()[2]; nGridChan = grid.shape()[3];

    nDataPol  = vbs.flagCube.shape()[0];
    nDataChan = vbs.flagCube.shape()[1];

    sampling[0] = sampling[1] = convFuncStore_p.sampling[0];
    support(0) = convFuncStore_p.xSupport[0];
    support(1) = convFuncStore_p.ySupport[0];

    Bool Dummy;
    T *gridStore = grid.getStorage(Dummy);
    const Int *iPosPtr = igrdpos.getStorage(Dummy);
    Double *convFunc=(*(convFuncStore_p.rdata)).getStorage(Dummy);
    Double *freq=vbs.freq.getStorage(Dummy);
    Bool *rowFlag=vbs.rowFlag.getStorage(Dummy);

    cacheAxisIncrements(nx,ny,nGridPol, nGridChan);

    for(Int irow=rbeg; irow< rend; irow++){          // For all rows
      
      if(!rowFlag[irow]){                        // If the row is not flagged
	
	for(Int ichan=0; ichan< nDataChan; ichan++){ // For all channels
	  
	  if (vbs.imagingWeight(ichan,irow)!=0.0) {  // If weights are not zero
	    achan=chanMap_p[ichan];
	    
	    if((achan>=0) && (achan<nGridChan)) {   // If selected channels are valid
	      
	      sgrid(pos,loc,off, phasor, irow, 
		    vbs.uvw, dphase_p[irow], vbs.freq[ichan], 
		    uvwScale_p, offset_p, sampling);

	      if (onGrid(nx, ny, loc, support)) {   // If the data co-ords. are with-in the grid
		
		for(Int ipol=0; ipol< nDataPol; ipol++) { // For all polarizations
		  if((!vbs.flagCube(ipol,ichan,irow))){   // If the pol. & chan. specific
							  // flags are not raised
		    apol=polMap_p(ipol);
		    if ((apol>=0) && (apol<nGridPol)) {
		      igrdpos[2]=apol; igrdpos[3]=achan;
		      
		      norm=0.0;

		      if(dopsf)  nvalue=Complex(vbs.imagingWeight(ichan,irow));
		      else	 nvalue=vbs.imagingWeight(ichan,irow)*
				   (vbs.visCube(ipol,ichan,irow)*phasor);
		      for(Int iy=-support[1]; iy <= support[1]; iy++) 
			{
			  iloc(1)=abs((int)(sampling[1]*iy+off[1]));
			  igrdpos[1]=loc[1]+iy;
			  for(Int ix=-support[0]; ix <= support[0]; ix++) 
			    {
			      iloc[0]=abs((int)(sampling[0]*ix+off[0]));
			      wt = convFunc[iloc[0]]*convFunc[iloc[1]];
			      
			      igrdpos[0]=loc[0]+ix;
			      // grid(grdpos) += nvalue*wt;

			      // The following uses raw index on the 4D grid
			      addTo4DArray(gridStore,iPosPtr,nvalue,wt);
			      norm+=wt;
			    }
			}
		      sumwt(apol,achan)+=vbs.imagingWeight(ichan,irow)*norm;
		    }
		  }
		}
	      }
	    }
	  }
	}
      }
    }
    
  }
  //
  //-----------------------------------------------------------------------------------
  // Re-sample VisBuffer to a regular grid (griddedData) (a.k.a. de-gridding)
  //
  void VisibilityResampler::GridToData(VBStore& vbs, const Array<Complex>& grid)
  //  void VisibilityResampler::GridToData(VBStore& vbs, Array<Complex>& grid)
  {
    Int nDataChan, nDataPol, nGridPol, nGridChan, nx, ny;
    Int achan, apol, rbeg, rend;
    Vector<Float> sampling(2);
    Vector<Int> support(2),loc(2), off(2), iloc(2);
    Vector<Double> pos(2);

    IPosition grdpos(4);

    Double norm=0, wt;
    Complex phasor, nvalue;

    rbeg=0;
    rend=vbs.nRow;
    nx       = grid.shape()[0]; ny        = grid.shape()[1];
    nGridPol = grid.shape()[2]; nGridChan = grid.shape()[3];

    nDataPol  = vbs.flagCube.shape()[0];
    nDataChan = vbs.flagCube.shape()[1];

    sampling[0] = sampling[1] = convFuncStore_p.sampling[0];
    support(0) = convFuncStore_p.xSupport[0];
    support(1) = convFuncStore_p.ySupport[0];

    Bool Dummy;
    const Complex *gridStore = grid.getStorage(Dummy);
    Vector<Int> igrdpos(4);
    const Int *iPosPtr = igrdpos.getStorage(Dummy);
    Double *convFunc=(*(convFuncStore_p.rdata)).getStorage(Dummy);
    Double *freq=vbs.freq.getStorage(Dummy);
    Bool *rowFlag=vbs.rowFlag.getStorage(Dummy);

    cacheAxisIncrements(nx,ny,nGridPol, nGridChan);

    for(Int irow=rbeg; irow<rend; irow++) {
      if(!rowFlag[irow]) {

	for (Int ichan=0; ichan < nDataChan; ichan++) {
	  achan=chanMap_p[ichan];

	  if((achan>=0) && (achan<nGridChan)) {
	    sgrid(pos,loc,off,phasor,irow,vbs.uvw,
		  dphase_p[irow],vbs.freq[ichan],
		  uvwScale_p,offset_p,sampling);

	    if (onGrid(nx, ny, loc, support)) {
	      for(Int ipol=0; ipol < nDataPol; ipol++) {

		if(!vbs.flagCube(ipol,ichan,irow)) { 
		  apol=polMap_p[ipol];
		  
		  if((apol>=0) && (apol<nGridPol)) {
		    igrdpos[2]=apol; igrdpos[3]=achan;
		    nvalue=0.0;
		    norm=0.0;

		    for(Int iy=-support[1]; iy <= support[1]; iy++) 
		      {
			iloc(1)=abs((Int)(sampling[1]*iy+off[1]));
			igrdpos[1]=loc[1]+iy;
			
			for(Int ix=-support[0]; ix <= support[0]; ix++) 
			  {
			    iloc(0)=abs((Int)(sampling[0]*ix+off[0]));
			    igrdpos[0]=loc[0]+ix;
			    
			    wt=convFunc[iloc[1]]*convFunc[iloc[0]];
			    norm+=wt;
			    //			    nvalue+=wt*grid(grdpos);
			    // The following uses raw index on the 4D grid
			    nvalue+=wt*getFrom4DArray(gridStore,iPosPtr);
			  }
		      }
		    vbs.visCube(ipol,ichan,irow)=(nvalue*conj(phasor))/norm;
		  }
		}
	      }
	    }
	  }
	}
      }
    }
  }
  //
  //-----------------------------------------------------------------------------------
  //
  void VisibilityResampler::sgrid(Vector<Double>& pos, Vector<Int>& loc, Vector<Int>& off, 
				  Complex& phasor, const Int& irow,
				  const Matrix<Double>& uvw, const Double& dphase, 
				  const Double& freq, const Vector<Double>& scale, 
				  const Vector<Double>& offset,
				  const Vector<Float>& sampling)
  {
    Double phase;
    Int ndim=pos.shape()(0);

    for(Int idim=0;idim<ndim;idim++)
      {
	pos[idim]=scale[idim]*uvw(idim,irow)*freq/C::c+offset[idim];
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
