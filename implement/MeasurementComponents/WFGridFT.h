//# GridFT.h: Definition for WFGridFT
//# Copyright (C) 1996,1997,1998,1999,2000,2002,2003
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
//# Correspondence concerning AIPS++ should be adressed as follows:
//#        Internet email: aips2-request@nrao.edu.
//#        Postal address: AIPS++ Project Office
//#                        National Radio Astronomy Observatory
//#                        520 Edgemont Road
//#                        Charlottesville, VA 22903-2475 USA
//#
//#
//# $Id$

#ifndef SYNTHESIS_WFGRIDFT_H
#define SYNTHESIS_WFGRIDFT_H

#include <synthesis/MeasurementComponents/GridFT.h>
#include <images/Images/TempImage.h>


namespace casa { //# NAMESPACE CASA - BEGIN

class WFGridFT : public GridFT {
public:

 WFGridFT(Long cachesize, Int tilesize, String convType,
	 MPosition mLocation, MDirection mTangent, Float padding=1.2);

 WFGridFT(const WFGridFT &other);

 ~WFGridFT();

 //Initializing the multiple pointers

 virtual void initializeToMultiVis(PtrBlock<TempImage<Complex> *>& image, 
				   const VisBuffer& vb, Int n) ;
 virtual void initializeToMultiSky(PtrBlock<TempImage<Complex> *>& image,
				   PtrBlock<Matrix<Float> *>& weight, 
				   const VisBuffer& vb, Int  n);
 virtual void multiPut(const VisBuffer& vb, Int row=-1, Bool dopsf=False, 
		       Int n=1);

 virtual void multiGet(VisBuffer& vb, Int row, Int nmaps);

 
 void makeMultiImage(FTMachine::Type type, VisSet& vs,
		     PtrBlock<TempImage<Complex> *>& image,
		     PtrBlock<Matrix<Float> *>& weight, Int n);
 
 ImageInterface<Complex>& getImage(Matrix<Float>&, Bool normalize=True);
 
 void changeMapNumber( Int& num);
 void assignMapNumber( Int& num){ nmaps_p=num;}; 

 void initMapsMulti(const VisBuffer& vb, Int nmaps, Int nth);
 void initMulti(Int nmaps, Int nth);  
 PtrBlock< Matrix<Float>* > sumWeightPtr; 
 PtrBlock< TempImage<Complex> *> imagePtr;  
 Block<MDirection> mImageHolder; 
 Int nmaps_p;
 Int mapNumber_p;  
 PtrBlock<ConvolveGridder<Double, Complex> * > gridderNumber; 
 PtrBlock<Lattice<Complex> * > arrayLatticeNumber; 
 Block<Float> maxAbsDataNumber; 
 PtrBlock< Array<Complex> *> griddedDataNumber;
 Vector<Int> nxPixels, nyPixels;

   
};

} //# NAMESPACE CASA - END

#endif
