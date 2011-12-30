//# VisModelData.h: Calculate Model Visibilities for a buffer from model images / complist
//# Copyright (C) 2011
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
#ifndef VISMODELDATA_H
#define VISMODELDATA_H
#include <casa/aips.h>
#include <casa/Containers/Record.h>
#include <casa/Containers/Block.h>
#include <msvis/SynthesisUtils/ComponentFTMachine.h>
namespace casa { //# NAMESPACE CASA - BEGIN
//#forward
  class VisBuffer;
  class ComponentList;  
  class FTMachine;
  template <class T> class Vector;
  template <class T> class CountedPtr;
// <summary>
// Object to provide MODEL_DATA visibilities on demand
// 
// </summary>

// <reviewed reviewer="" date="" tests="" demos="">

// <prerequisite>
//  FTMachine and ComponentFTMachine
//
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

class VisModelData {
 public:
  //empty constructor
  VisModelData();
  //From a FTMachine Record
  VisModelData(const Record& ftmachinerec, const Vector<Int>& validfieldids, const Vector<Int>& msIds);
  //Add Image/FTMachine to generate visibilities for
  void addFTMachine(const Record& recordFTMachine, const Vector<Int>& validfieldids, const Vector<Int>& msIds);
  //Add componentlist to generate visibilities for
  void addCompFTMachine(const ComponentList& cl, const Vector<Int>& validfieldids, 
			const Vector<Int>& msIds);
  //For simple model a special case for speed 
  void addFlatModel(const Vector<Double>& value, const Vector<Int>& validfieldids, 
		    const Vector<Int>& msIds);
  //put the model data for this VisBuffer in the modelVisCube
  Bool getModelVis(VisBuffer& vb);
  
  



 private:
  void initializeToVis();
  CountedPtr<ComponentList> getCL(const Int msId, const Int fieldId);
  CountedPtr<FTMachine> getFT(const Int msId, const Int fieldId);
  Block<CountedPtr<ComponentList> > clholder_p;
  Block<CountedPtr<FTMachine> > ftholder_p;
  Block<Vector<Double> > flatholder_p;
  CountedPtr<ComponentFTMachine> cft_p;

};

}//end namespace
#endif // VISMODELDATA_H
