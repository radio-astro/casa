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
     VisImagingWeight(ROVisibilityIterator& vi, const String& rmode, const Quantity& noise,
                               const Double robust, const Int nx, const Int ny,
                               const Quantity& cellx, const Quantity& celly,
                               const Int uBox, const Int vBox);
     virtual ~VisImagingWeight();


     // reference semantically = operator
     VisImagingWeight& operator=(const VisImagingWeight& imwgt);



     // Function to calculate the  uniform style weights, include Brigg's for example
     // imagingWeight should be sized by (nchan, row) already
     virtual void weightUniform(Matrix<Float>& imagingWeight, const Matrix<Bool>& flag, const Matrix<Double>& uvw,
                                const Vector<Double>& frequency, const Vector<Float>& weight) const;

     //Natural weighting scheme
     //imagingWeight should be sized by (nchan, row) already
     virtual void weightNatural(Matrix<Float>& imagingWeight, const Matrix<Bool>& flag,
                                const Vector<Float>& weight) const;

     //Radial weighting
     //imagingWeight should be sized by (nchan, row) already
     virtual void weightRadial(Matrix<Float>& imagingWeight, const Matrix<Bool>& flag,
                               const Matrix<Double>& uvw, const Vector<Double>& frequency,
                               const Vector<Float>& weight) const;

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
			 const Vector<Double>& frequency, const Vector<Float>& weight) const;


    private:

     Matrix<Float> gwt_p;
     String wgtType_p;
     Float uscale_p, vscale_p;
     Float f2_p, d2_p;
     Int uorigin_p, vorigin_p;
     Int nx_p, ny_p;
     Bool doFilter_p;
     Double cospa_p;
     Double sinpa_p;
     Double rbmaj_p;
     Double rbmin_p;


 };
} //# NAMESPACE CASA - END
#endif // VISIMAGINGWEIGHT_H
