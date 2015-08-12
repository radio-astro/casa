//# VisImagingWeight.h: Calculate Imaging Weights for a buffer from weight
//# Copyright (C) 2009
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

#ifndef VISIMAGINGWEIGHT_H
#define VISIMAGINGWEIGHT_H
#include <casa/aips.h>
#include <casa/BasicSL/Complex.h>
#include <casa/Quanta/Quantum.h>
#include <msvis/MSVis/VisibilityIterator2.h>

namespace casa { //# NAMESPACE CASA - BEGIN
//#forward
class ROVisibilityIterator;
template<class T> class Matrix;
template<class T> class Vector;

// <summary>
// Object to hold type of imaging weight scheme to be used on the fly and to provide
// facilities to do that.
// </summary>

// <reviewed reviewer="" date="" tests="" demos="">

// <prerequisite>
// </prerequisite>
//
// <etymology>
// </etymology>
//
// <synopsis>
// </synopsis>
//
// <example>
// <srcblock>
// </srcblock>
// </example>
//
// <motivation>
// </motivation>
//
// <todo asof="">
// </todo>

 class VisImagingWeight {
    public:
     //empty constructor
     VisImagingWeight();
     //Constructor to calculate natural and radial weights
     VisImagingWeight(const String& type);
     //Constructor to calculate uniform weight schemes; include Brigg's and super/uniform
     //If multiField=True, the weight density calcution is done on a per field basis, 
     //else it is all fields combined
     VisImagingWeight(ROVisibilityIterator& vi, const String& rmode, const Quantity& noise,
                               const Double robust, const Int nx, const Int ny,
                               const Quantity& cellx, const Quantity& celly,
		      const Int uBox, const Int vBox, const Bool multiField=False);
     //Constructor for uniform style weighting when the weight density is calculated 
     //elsewhere
     VisImagingWeight(ROVisibilityIterator& vi, Block<Matrix<Float> >& grids, const String& rmode, const Quantity& noise,
                                     const Double robust, const Quantity& cellx, const Quantity& celly,
		      const Bool multiField=False) ;
     //VisibilityIterator2 version of the above....
     // Note the VisibilityIterator can be readonly...thus recommended if you can
     // as that will prevent unnecessary locks
     VisImagingWeight(vi::VisibilityIterator2& vi, const String& rmode, const Quantity& noise,
                               const Double robust, const Int nx, const Int ny,
                               const Quantity& cellx, const Quantity& celly,
		      const Int uBox, const Int vBox, const Bool multiField=False);

     virtual ~VisImagingWeight();


     // reference semantically = operator
     VisImagingWeight& operator=(const VisImagingWeight& imwgt);



     // Function to calculate the  uniform style weights, include Brigg's for example
     // imagingWeight should be sized by (nchan, row) already
     // The fieldid and msid parameters must correspond to what VisBuffer  or VisIter fieldId() and msId() returns
     virtual void weightUniform(Matrix<Float>& imagingWeight, const Matrix<Bool>& flag, const Matrix<Double>& uvw,
                                const Vector<Double>& frequency, const Matrix<Float>& weight, const Int msid, const Int fieldid ) const;

     //Natural weighting scheme
     //imagingWeight should be sized by (nchan, row) already
     virtual void weightNatural(Matrix<Float>& imagingWeight, const Matrix<Bool>& flag,
                                const Matrix<Float>& weight) const;

   /*  unused version?
     //weight as Matrix version
     virtual void weightNatural(Matrix<Float>& imagingWeight, const Matrix<Bool>& flag,
                                const Matrix<Float>& weight) const;

   */
     //Radial weighting
     //imagingWeight should be sized by (nchan, row) already
     virtual void weightRadial(Matrix<Float>& imagingWeight, const Matrix<Bool>& flag,
                               const Matrix<Double>& uvw, const Vector<Double>& frequency,
                               const Matrix<Float>& weight) const;

     //Get the type of weighting this object is on..will return one of "natural", "uniform", "radial"
     virtual String getType() const;
     
     //setting uv filtering
     virtual void setFilter(const String& type, const Quantity& bmaj,
			    const Quantity& bmin, const Quantity& bpa);

     //returns if uv filtering is set
     virtual Bool doFilter() const;

     //do uvfiltering...to be called after imaging weight is calculated
     virtual void filter(Matrix<Float>& imWeight, const Matrix<Bool>& flag, 
			 const Matrix<Double>& uvw,
			 const Vector<Double>& frequency, const Matrix<Float>& weight) const;

     // This is to  get/set uniform style weight density...e.g if the caller wants to
     // add densities from different pieces of data distributed via different 
     // VisibilityIterators
     virtual Bool getWeightDensity (Block<Matrix<Float> >& density);
     virtual void setWeightDensity(const Block<Matrix<Float> >& density);

     // Form corr-indep weight by averaging parallel-hand weights
     void unPolChanWeight(Matrix<Float>& chanRowWt, const Cube<Float>& corrChanRowWt) const;

    private:
     void cube2Matrix(const Cube<Bool>& fcube, Matrix<Bool>& fMat);
     SimpleOrderedMap <String, Int> multiFieldMap_p;
     Block<Matrix<Float> > gwt_p;
     String wgtType_p;
     Float uscale_p, vscale_p;
     Vector<Float> f2_p, d2_p;
     Int uorigin_p, vorigin_p;
     Int nx_p, ny_p;
     Bool doFilter_p;
     Double cospa_p;
     Double sinpa_p;
     Double rbmaj_p;
     Double rbmin_p;
     Double robust_p;
     String rmode_p;
     Quantity noise_p;


 };
} //# NAMESPACE CASA - END
#endif // VISIMAGINGWEIGHT_H
