//# TJonesMBuf.h: TJones calibration main table buffer
//# Copyright (C) 1996,1997,1998,2001,2002,2003
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

#ifndef CALIBRATION_TJONESMBUF_H
#define CALIBRATION_TJONESMBUF_H

#include <measures/Measures/MFrequency.h>
#include <calibration/CalTables/SolvableVJMBuf.h>
#include <calibration/CalTables/TJonesMCol.h>
#include <calibration/CalTables/TJonesTable.h>

namespace casa { //# NAMESPACE CASA - BEGIN

// <summary> 
// TJonesPolyMBuf: TJonesPoly calibration main table buffer
// </summary>

// <use visibility=export>

// <reviewed reviewer="" date="" tests="" demos="">

// <prerequisite>
//<li><linkto class="CalMainBuffer">CalMainBuffer</linkto> module
//<li><linkto class="SolvableVisJonesMBuf">SolvableVisJonesMBuf</linkto> module
//<li><linkto class="TJonesMBuf">TJonesMBuf</linkto> module
// </prerequisite>
//
// <etymology>
// From "TJonesPoly matrix", "cal main table" and "buffer"
// </etymology>
//
// <synopsis>
// The TJonesPolyMBuf class holds a buffer, optionally connected to a 
// TJonesPoly calibration main table iterator (of base type CalIterBase). 
// The TJonesPoly matrix contains atmospheric gains expressed as
// polynomials over time.
// </synopsis>
//
// <example>
// <srcblock>
// </srcblock>
// </example>
//
// <motivation>
// Encapsulate TJonesPoly calibration main table data buffers.
// </motivation>
//
// <todo asof="01/08/01">
// (i) Deal with non-standard columns.
// </todo>

class TJonesPolyMBuf : public TJonesMBuf
{
 public:
  // Default constructor. No connection to an underlying
  // calibration table iterator in this case.
  TJonesPolyMBuf();

  // Construct from a set of calibration buffer indices and
  // their specified values. Non-index columns will be set 
  // to default values, and there is no connection to an 
  // underlying calibration table iterator in this case.
  TJonesPolyMBuf (const Vector<Int>& calIndices, 
		  const Block<Vector<Int> >& indexValues);

  // Construct from a calibration table iterator. The calibration
  // buffer will remain synchronized with the iterator.
  TJonesPolyMBuf (CalIterBase& calIter);

  // Invalidate the current calibration buffer. This signals 
  // that a re-read is required as the iterator has advanced
  virtual void invalidate();

  // Write the current buffer at the end of a specified cal table
  virtual Int append (CalTable& calTable);

  // Maximum number of rows in the calibration buffer
  virtual Int nRow();

  // Update the parametrized solution in each of a set of buffer rows
  virtual Bool fillMatchingRows (const Vector<Int>& matchingRows,
				 const String& sFreqGrpName,
				 const String& sPolyType, 
				 const String& sPolyMode,
				 const Complex& sScaleFactor,
				 const Int& sNPolyAmp, const Int& sNPolyPhase, 
				 const Vector<Double>& sPolyCoeffAmp,
				 const Vector<Double>& sPolyCoeffPhase,
				 const String& sPhaseUnits, 
				 const MFrequency& sRefFreq, 
				 const Int& sRefAnt);

  // Data field accessors
  virtual Vector<String>& polyType();
  virtual Vector<String>& polyMode();
  virtual Vector<Complex>& scaleFactor();
  virtual Vector<Int>& nPolyAmp();
  virtual Vector<Int>& nPolyPhase();
  virtual Array<Double>& polyCoeffAmp();
  virtual Array<Double>& polyCoeffPhase();
  virtual Vector<String>& phaseUnits();

 protected:
  // Factory method to create a columns accessor object of the 
  // appropriate type
  virtual TJonesPolyMCol* newCalMainCol (CalTable& calTable) {
    return new TJonesPolyMCol(dynamic_cast<TJonesPolyTable&>(calTable));};
  
  // Access to the columns accessor object
  virtual TJonesPolyMCol* calMainCol() 
    {return dynamic_cast<TJonesPolyMCol*>(CalMainBuffer::calMainCol());};

  // <group>
  // Fill the TJonesPoly cal buffer attribute columns in an empty 
  // cal buffer, to extend the inherited method fillAttributes. The
  // cal indices, specified as enums from class MSCalEnums, are excluded
  // as non-attribute columns.
  //
  // Use a visibility buffer to define the attribute values (NYI)
  virtual void fillAttributes(const Vector<Int>& calIndices,
			      const VisBuffer& vb) {};
  //
  // Set default attribute values
  virtual void fillAttributes(const Vector<Int>& calIndices);
  // </group>

 private:
  // Buffer fields
  Vector<String> polyType_p;
  Vector<String> polyMode_p;
  Vector<Complex> scaleFactor_p;
  Vector<Int> nPolyAmp_p;
  Vector<Int> nPolyPhase_p;
  Array<Double> polyCoeffAmp_p;
  Array<Double> polyCoeffPhase_p;
  Vector<String> phaseUnits_p;

  // Buffer field status flags
  Bool polyTypeOK_p;
  Bool polyModeOK_p;
  Bool scaleFactorOK_p;
  Bool nPolyAmpOK_p;
  Bool nPolyPhaseOK_p;
  Bool polyCoeffAmpOK_p;
  Bool polyCoeffPhaseOK_p;
  Bool phaseUnitsOK_p;
};

// <summary> 
// TJonesSplineMBuf: TJonesSpline calibration main table buffer
// </summary>

// <use visibility=export>

// <reviewed reviewer="" date="" tests="" demos="">

// <prerequisite>
//<li><linkto class="CalMainBuffer">CalMainBuffer</linkto> module
//<li><linkto class="SolvableVisJonesMBuf">SolvableVisJonesMBuf</linkto> module
//<li><linkto class="TJonesMBuf">TJonesMBuf</linkto> module
//<li><linkto class="TJonesPolyMBuf">TJonesPolyMBuf</linkto> module
// </prerequisite>
//
// <etymology>
// From "TJonesSpline matrix", "cal main table" and "buffer"
// </etymology>
//
// <synopsis>
// The TJonesSplineMBuf class holds a buffer, optionally connected to a 
// TJonesSpline calibration main table iterator (of base type CalIterBase). 
// The TJonesSpline matrix contains atmospheric gain terms expressed as
// as spline polynomials over time.
// </synopsis>
//
// <example>
// <srcblock>
// </srcblock>
// </example>
//
// <motivation>
// Encapsulate TJonesSpline calibration main table data buffers.
// </motivation>
//
// <todo asof="01/08/01">
// (i) Deal with non-standard columns.
// </todo>

class TJonesSplineMBuf : public TJonesPolyMBuf
{
 public:
  // Default constructor. No connection to an underlying
  // calibration table iterator in this case.
  TJonesSplineMBuf();

  // Construct from a set of calibration buffer indices and
  // their specified values. Non-index columns will be set 
  // to default values, and there is no connection to an 
  // underlying calibration table iterator in this case.
  TJonesSplineMBuf (const Vector<Int>& calIndices, 
		    const Block<Vector<Int> >& indexValues);

  // Construct from a calibration table iterator. The calibration
  // buffer will remain synchronized with the iterator.
  TJonesSplineMBuf (CalIterBase& calIter);

  // Invalidate the current calibration buffer. This signals 
  // that a re-read is required as the iterator has advanced
  virtual void invalidate();

  // Write the current buffer at the end of a specified cal table
  virtual Int append (CalTable& calTable);

  // Maximum number of rows in the calibration buffer
  virtual Int nRow();

  // Update the parametrized solution in each of a set of buffer rows
  virtual Bool fillMatchingRows (const Vector<Int>& matchingRows,
				 const String& sFreqGrpName,
				 const String& sPolyType, 
				 const String& sPolyMode,
				 const Complex& sScaleFactor,
				 const Int& sNPolyAmp, const Int& sNPolyPhase, 
				 const Vector<Double>& sPolyCoeffAmp,
				 const Vector<Double>& sPolyCoeffPhase,
				 const String& sPhaseUnits, 
				 const Int& sNKnotsAmp, 
				 const Int& sNKnotsPhase,
				 const Vector<Double>& sSplineKnotsAmp,
				 const Vector<Double>& sSplineKnotsPhase,
				 const MFrequency& sRefFreq, 
				 const Int& sRefAnt);

  // Data field accessors
  virtual Vector<Int>& nKnotsAmp();
  virtual Vector<Int>& nKnotsPhase();
  virtual Array<Double>& splineKnotsAmp();
  virtual Array<Double>& splineKnotsPhase();

 protected:
  // Factory method to create a columns accessor object of the 
  // appropriate type
  virtual TJonesSplineMCol* newCalMainCol (CalTable& calTable) {
    return new TJonesSplineMCol(dynamic_cast<TJonesSplineTable&>(calTable));};
  
  // Access to the columns accessor object
  virtual TJonesSplineMCol* calMainCol() 
    {return dynamic_cast<TJonesSplineMCol*>(CalMainBuffer::calMainCol());};

  // <group>
  // Fill the TJonesSpline cal buffer attribute columns in an empty 
  // cal buffer, to extend the inherited method fillAttributes. The
  // cal indices, specified as enums from class MSCalEnums, are excluded
  // as non-attribute columns.
  //
  // Use a visibility buffer to define the attribute values (NYI)
  virtual void fillAttributes(const Vector<Int>& calIndices,
			      const VisBuffer& vb) {};
  //
  // Set default attribute values
  virtual void fillAttributes(const Vector<Int>& calIndices);
  // </group>

    private:
  // Buffer fields
  Vector<Int> nKnotsAmp_p;
  Vector<Int> nKnotsPhase_p;
  Array<Double> splineKnotsAmp_p;
  Array<Double> splineKnotsPhase_p;

  // Buffer field status flags
  Bool nKnotsAmpOK_p;
  Bool nKnotsPhaseOK_p;
  Bool splineKnotsAmpOK_p;
  Bool splineKnotsPhaseOK_p;
};


} //# NAMESPACE CASA - END

#endif
   
  



