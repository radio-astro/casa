//# MaskedArrayModel.h: this defines MaskedArrayModel
//# Copyright (C) 1996,1997,1999
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
//#
//# $Id$

#ifndef SYNTHESIS_MASKEDARRAYMODEL_H
#define SYNTHESIS_MASKEDARRAYMODEL_H


#include <casa/aips.h>
#include <synthesis/MeasurementEquations/LinearModel.h>
#include <casa/Arrays/Array.h>
#include <casa/Arrays/MaskedArray.h>

namespace casa { //# NAMESPACE CASA - BEGIN

// <summary>
// base class for models with an internal & external representation 
// as a MaskedArray 
// </summary>

// <use visibility=export>

// <reviewed reviewer="" date="yyyy/mm/dd" tests="" demos="">
// </reviewed>

// <prerequisite>
//   <li> MaskedArrays
//   <li> LinearModel
//   <li> LinearModel/LinearEquation paradigm
// </prerequisite>
//
// <synopsis>
// An MaskedArrayModel is a base class for Models that can be
// represented by arrays with a mask (used to denote which values
// are valid) . It is expected that this class will be mainly used
// by inheritence from other classes which will then provide the solve()
// functions necessary to update the model given an equation.
// 
// However this class does not contain any pure virtual functions and hence
// can be used "as is". An example of this is given below. For an example of
// how this class can be used by derived classes see the 
// <linkto class=MaskedHogbomCleanModel>MaskedHogbomCleanModel</linkto> 
// class.
//
// This class makes an internal copy of the Array supplied to it (either
// when constructed or when using the setModel function). If this is found
// to significantly affect performance (execution speed or memory
// requirements) this may be changed to a reference, perhaps using a smart
// pointer like the <linkto class=COWPtr>COWPtr</linkto>
// </synopsis>
//
// <example>
// <srcblock>
// MaskedArrayModel<Float> currentModel(); // Cannot use the model yet!
// {
//   Matrix<Float> bestGuess(32,32);
//    ... put your best guess into the Matrix ...
//   currentModel.setModel(bestGuess); // This does a real copy
// }
// ConvolutionEquation eqn(psf, dirty); // psf, and dirty are arrays defined
//                                      // elsewhere.
// eqn.evaluate(result, currentModel); // Here result is the convolution of
//                                     // of the model with the psf.
// </srcblock>
// </example>
//
// <motivation>
// All the different image plane based clean algorithms have a common
// implementation in that they can use an array to store the current
// model. This class provides a way to abstract this functionality, as well
// as provide a mechanism to implement "clean boxes"  
// </motivation>
//
// <templating arg=T>
// While the template arguement for this class can be just about anything,
// the use of this class with an equation class will significantly restrict
// the possible templates. I have used this class (or derivations of it)
// with the following data types.
//    <li> Float
// </templating>
//
// <thrown>
// This class does not explicitly throw exceptions however the objects used
// by this class may
// </thrown>
//
// <todo asof="1996/05/14">
//   <li> Decide whether to copy the input array by reference or stay with
//   the current scheme.
//   <li> Test the class with STokesVectors
// </todo>

template<class T> class MaskedArrayModel: 
  public LinearModel< MaskedArray<T> > {
public:
    // Default constructor sets the internal arrays to zero size
    MaskedArrayModel();
    // Construct the model from a masked array. The data is copied.
    MaskedArrayModel(const MaskedArray<T> & model);
    // Construct the model from an array. The mask is set to be fully
    // transparent. 
    MaskedArrayModel(const Array<T> & model);

    // These functions return the model, either as a masked array or just
    // the data array itself with the mask removed. 
    // <group>
    void getModel(Array<T>& model) const;
    virtual void getModel(MaskedArray<T>& model) const;
    MaskedArray<T> getModel() const;
    // </group>
    // These functions are analogous to the constructors above and must be
    // called when the default constructor is used. If no mask is specified
    // then it is by default set to totally transparent.  
    // <group>
    void setModel(const Array<T>& model);
    virtual void setModel(const MaskedArray<T>& model);
    // </group>
    
protected:
    MaskedArray<T> theModel;
};


} //# NAMESPACE CASA - END

#ifndef AIPS_NO_TEMPLATE_SRC
#include <synthesis/MeasurementEquations/MaskedArrayModel.tcc>
#endif //# AIPS_NO_TEMPLATE_SRC
#endif
