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
#include <synthesis/MSVis/VisBuffer.h>
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
  void storeArrayAsImage(String fileName, const CoordinateSystem& coords, const Array<Float>& cf);
			 
  Bool isVBNaN(const VisBuffer& vb, String& mesg);
  namespace SynthesisUtils
  {
    void rotateComplexArray(LogIO& logIO, Array<Complex>& inArray, 
			    CoordinateSystem& inCS,
			    Array<Complex>& outArray, 
			    Double dAngleRad, 
			    String interpMathod=String("CUBIC"),
			    Bool modifyInCS=True);
    void findLatticeMax(const Array<Complex>& lattice,
			Vector<Float>& maxAbs,
			Vector<IPosition>& posMaxAbs) ;
    void findLatticeMax(const ImageInterface<Complex>& lattice,
			Vector<Float>& maxAbs,
			Vector<IPosition>& posMaxAbs) ;
    void findLatticeMax(const ImageInterface<Float>& lattice,
			Vector<Float>& maxAbs,
			Vector<IPosition>& posMaxAbs) ;
    inline  Int nint(const Double& v) {return (Int)std::floor(v+0.5);}
    inline  Int nint(const Float& v) {return (Int)std::floor(v+0.5);}
    inline  Bool near(const Double& d1, const Double& d2, 
		      const Double EPS=1E-6) 
    {
      Bool b1=(fabs(d1-d2) < EPS)?True:False;
      return b1;
    }
    template <class T>
    inline void SETVEC(Vector<T>& lhs, const Vector<T>& rhs)
    {lhs.resize(rhs.shape()); lhs = rhs;};
    template <class T>
    inline void SETVEC(Array<T>& lhs, const Array<T>& rhs)
    {lhs.resize(rhs.shape()); lhs = rhs;};

    template <class T>
    T getenv(const char *name, const T defaultVal);
    Float libreSpheroidal(Float nu);
    Double getRefFreq(const VisBuffer& vb);
    void makeFTCoordSys(const CoordinateSystem& coords,
			const Int& convSize,
			const Vector<Double>& ftRef,
			CoordinateSystem& ftCoords);

    void expandFreqSelection(const Matrix<Double>& freqSelection,
			     Matrix<Double>& expandedFreqList,
			     Matrix<Double>& expandedConjFreqList);

    template <class T>
    void libreConvolver(Array<T>& c1, const Array<T>& c2);
    inline Double conjFreq(const Double& freq, const Double& refFreq) 
    {return sqrt(2*refFreq*refFreq - freq*freq);};

    Double nearestValue(const Vector<Double>& list, const Double& val, Int& index);

    CoordinateSystem makeUVCoords(CoordinateSystem& imageCoordSys,
				  IPosition& shape);

    Vector<Int> mapSpwIDToDDID(const VisBuffer& vb, const Int& spwID);
    Vector<Int> mapSpwIDToPolID(const VisBuffer& vb, const Int& spwID);
    void calcIntersection(const Int blc1[2], const Int trc1[2], const Float blc2[2], const Float trc2[2],
			  Float blc[2], Float trc[2]);
    Bool checkIntersection(const Int blc1[2], const Int trc1[2], const Float blc2[2], const Float trc2[2]);
			  
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
