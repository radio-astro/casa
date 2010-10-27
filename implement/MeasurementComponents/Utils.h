// -*- C++ -*-
//# Utils.h: Definition of global functions in Utils.cc
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
#include <casa/aips.h>
#include <casa/Exceptions/Error.h>
#include <msvis/MSVis/VisBuffer.h>
#include <casa/Quanta/Quantum.h>
#include <ms/MeasurementSets/MSColumns.h>
#include <ms/MeasurementSets/MSRange.h>
#include <images/Images/ImageInterface.h>
#include <ms/MeasurementSets/MeasurementSet.h>
#include <casa/Arrays/Array.h>
#include <casa/Logging/LogIO.h>

#ifndef SYNTHESIS_UTILS_H
#define SYNTHESIS_UTILS_H

namespace casa
{
  Int getPhaseCenter(MeasurementSet& ms, MDirection& dir0, Int whichField=-1);
  Bool findMaxAbsLattice(const ImageInterface<Float>& lattice,
			 Float& maxAbs,IPosition& posMaxAbs);
  Bool findMaxAbsLattice(const ImageInterface<Float>& masklat,
			 const Lattice<Float>& lattice,
			 Float& maxAbs,IPosition& posMaxAbs, 
			 Bool flip=False);
  Double getCurrentTimeStamp(const VisBuffer& vb);
  void makeStokesAxis(Int npol_p, Vector<String>& polType, Vector<Int>& whichStokes);
  Double getPA(const VisBuffer& vb);
  void storeImg(String fileName,ImageInterface<Complex>& theImg, Bool writeReIm=False);
  void storeImg(String fileName,ImageInterface<Float>& theImg);
  void storeArrayAsImage(String fileName, const CoordinateSystem& coords, const Array<Complex>& cf);
			 
  Bool isVBNaN(const VisBuffer& vb, String& mesg);
  namespace SynthesisUtils
  {
    void rotateComplexArray(LogIO& logIO, Array<Complex>& inArray, 
			    CoordinateSystem& inCS,
			    Array<Complex>& outArray, 
			    Double dAngleRad, 
			    String interpMathod=String("CUBIC"));
    void findLatticeMax(const Array<Complex>& lattice,
			Vector<Float>& maxAbs,
			Vector<IPosition>& posMaxAbs) ;
    void findLatticeMax(const ImageInterface<Complex>& lattice,
			Vector<Float>& maxAbs,
			Vector<IPosition>& posMaxAbs) ;
    void findLatticeMax(const ImageInterface<Float>& lattice,
			Vector<Float>& maxAbs,
			Vector<IPosition>& posMaxAbs) ;
  }

  void getHADec(MeasurementSet& ms, const VisBuffer& vb, Double &HA, Double& RA, Double& Dec);
  /////////////////////////////////////////////////////////////////////////////
  // 
  // An interface class to detect changes in the VisBuffer
  // Exact meaning of the "change" is defined in the derived classes
  //
  struct IChangeDetector {
     // return True if a change occurs in the given row since the last call of update
     virtual Bool changed(const VisBuffer &vb, Int row) const throw(AipsError) = 0;
     // start looking for a change from the given row of the VisBuffer
     virtual void update(const VisBuffer &vb, Int row) throw(AipsError) = 0;
     
     // reset to the state which exists just after construction
     virtual void reset() throw(AipsError) = 0;

     // some derived methods, which use the abstract virtual function changed(vb,row)
     
     // return True if a change occurs somewhere in the buffer
     Bool changed(const VisBuffer &vb) const throw(AipsError);
     // return True if a change occurs somewhere in the buffer starting from row1
     // up to row2 (row2=-1 means up to the end of the buffer). The row number, 
     // where the change occurs is returned in the row2 parameter
     Bool changedBuffer(const VisBuffer &vb, Int row1, Int &row2) const throw(AipsError);
  protected:
     // a virtual destructor to make the compiler happy
     virtual ~IChangeDetector() throw(AipsError);
  };
  //
  //////////////////////////////////////////////////////////////////////////////

  //////////////////////////////////////////////////////////////////////////////
  //
  // ParAngleChangeDetector - a class to detect a change in the parallactic 
  //                          angle. 
  //
  class ParAngleChangeDetector : public IChangeDetector {
     Double pa_tolerance_p;   // a parallactic angle tolerance. If exeeded, 
                              // the angle is considered to be changed.
     Double last_pa_p;        // last value of the parallactic angle
  public:
     // The default constructor
     ParAngleChangeDetector():pa_tolerance_p(0.0) {};
     // set up the tolerance, which determines how much the position angle should
     // change to report the change by this class
     ParAngleChangeDetector(const Quantity &pa_tolerance) throw(AipsError);

     virtual void setTolerance(const Quantity &pa_tolerance);
     // reset to the state which exists just after construction
     virtual void reset() throw(AipsError);

     // return parallactic angle tolerance
     Quantity getParAngleTolerance() const throw(AipsError);
      
     // implementation of the base class' virtual functions
     
     // return True if a change occurs in the given row since the last call of update
     virtual Bool changed(const VisBuffer &vb, Int row) const throw(AipsError);
     // start looking for a change from the given row of the VisBuffer
     virtual void update(const VisBuffer &vb, Int row) throw(AipsError);
  };

  //
  /////////////////////////////////////////////////////////////////////////////

}
#endif
