//# BJonesMBuf.h: BJones calibration main table buffer
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

#ifndef CALIBRATION_BJONESMBUF_H
#define CALIBRATION_BJONESMBUF_H

#include <measures/Measures/MFrequency.h>
#include <synthesis/CalTables/SolvableVJMBuf.h>
#include <synthesis/CalTables/BJonesMCol.h>
#include <synthesis/CalTables/BJonesTable.h>

namespace casa { //# NAMESPACE CASA - BEGIN

// <summary> 
// BJonesMBuf: BJones calibration main table buffer
// </summary>

// <use visibility=export>

// <reviewed reviewer="" date="" tests="" demos="">

// <prerequisite>
//<li><linkto class="CalMainBuffer">CalMainBuffer</linkto> module
//<li><linkto class="SolvableVisJonesMBuf">SolvableVisJonesMBuf</linkto> module
// </prerequisite>
//
// <etymology>
// From "BJones matrix", "cal main table" and "buffer"
// </etymology>
//
// <synopsis>
// The BJonesMBuf class holds a buffer, optionally connected to a 
// BJones calibration main table iterator (of base type CalIterBase). 
// The BJones matrix contains bandpass correction terms.
// </synopsis>
//
// <example>
// <srcblock>
// </srcblock>
// </example>
//
// <motivation>
// Encapsulate BJones calibration main table data buffers.
// </motivation>
//
// <todo asof="01/08/01">
// (i) Deal with non-standard columns.
// </todo>

class BJonesMBuf : public SolvableVisJonesMBuf
{
 public:
  // Default constructor. No connection to an underlying
  // calibration table iterator in this case.
  BJonesMBuf();

  // Construct from a set of calibration buffer indices and
  // their specified values. Non-index columns will be set 
  // to default values, and there is no connection to an 
  // underlying calibration table iterator in this case.
  BJonesMBuf (const Vector<Int>& calIndices, 
	      const Block<Vector<Int> >& indexValues);

  // Construct from a calibration table iterator. The calibration
  // buffer will remain synchronized with the iterator.
  BJonesMBuf (CalIterBase& calIter);

 protected:

 private:
};

// <summary> 
// BJonesPolyMBuf: BJonesPoly calibration main table buffer
// </summary>

// <use visibility=export>

// <reviewed reviewer="" date="" tests="" demos="">

// <prerequisite>
//<li><linkto class="CalMainBuffer">CalMainBuffer</linkto> module
//<li><linkto class="SolvableVisJonesMBuf">SolvableVisJonesMBuf</linkto> module
// </prerequisite>
//
// <etymology>
// From "BJonesPoly matrix", "cal main table" and "buffer"
// </etymology>
//
// <synopsis>
// The BJonesPolyMBuf class holds a buffer, optionally connected to a 
// BJonesPoly calibration main table iterator (of base type CalIterBase). 
// The BJonesPoly matrix contains bandpass correction terms.
// </synopsis>
//
// <example>
// <srcblock>
// </srcblock>
// </example>
//
// <motivation>
// Encapsulate BJonesPoly calibration main table data buffers.
// </motivation>
//
// <todo asof="01/08/01">
// (i) Deal with non-standard columns.
// </todo>

class BJonesPolyMBuf : public BJonesMBuf
{
 public:
  // Default constructor. No connection to an underlying
  // calibration table iterator in this case.
  BJonesPolyMBuf();

  // Construct from a set of calibration buffer indices and
  // their specified values. Non-index columns will be set 
  // to default values, and there is no connection to an 
  // underlying calibration table iterator in this case.
  BJonesPolyMBuf (const Vector<Int>& calIndices, 
		  const Block<Vector<Int> >& indexValues);

  // Construct from a calibration table iterator. The calibration
  // buffer will remain synchronized with the iterator.
  BJonesPolyMBuf (CalIterBase& calIter);

  // Write the current buffer at the end of a specified cal table
  virtual Int append (CalTable& calTable);

  // Maximum number of rows in the calibration buffer
  virtual Int nRow();

  // Update the parametrized solution for a given antenna id.
  virtual Bool putAntGain (const Int& antennaId, const String& sFreqGrpName,
			   const String& sPolyType, 
			   const Complex& sScaleFactor, 
			   const Vector<Double>& sValidDomain,
			   const Int& sNPolyAmp, const Int& sNPolyPhase, 
			   const Vector<Double>& sPolyCoeffAmp,
			   const Vector<Double>& sPolyCoeffPhase,
			   const String& sPhaseUnits, 
			   const Complex& sSideBandRef, 
			   const MFrequency& sRefFreq, const Int& sRefAnt);

  // Data field accessors
  Vector<String>& polyType();
  Vector<String>& polyMode();
  Vector<Complex>& scaleFactor();
  Array<Double>& validDomain();
  Vector<Int>& nPolyAmp();
  Vector<Int>& nPolyPhase();
  Array<Double>& polyCoeffAmp();
  Array<Double>& polyCoeffPhase();
  Vector<String>& phaseUnits();
  Vector<Complex>& sideBandRef();

 protected:
  // Factory method to create a columns accessor object of the 
  // appropriate type
  virtual BJonesPolyMCol* newCalMainCol (CalTable& calTable) {
    return new BJonesPolyMCol(dynamic_cast<BJonesPolyTable&>(calTable));};
  
  // Access to the columns accessor object
  virtual BJonesPolyMCol* calMainCol() 
    {return dynamic_cast<BJonesPolyMCol*>(CalMainBuffer::calMainCol());};

  // Invalidate the current cache.
  virtual void invalidate();

  // <group>
  // Fill the BJonesPoly cal buffer attribute columns in an empty 
  // cal buffer, to extend the inherited method fillAttributes. The
  // cal indices, specified as enums from class MSCalEnums, are excluded
  // as non-attribute columns.
  //
  // Use a visibility buffer to define the attribute values (NYI)
  virtual void fillAttributes(const Vector<Int>& /*calIndices*/,
			      const VisBuffer& /*vb*/) {};
  //
  // Set default attribute values
  virtual void fillAttributes(const Vector<Int>& calIndices);
  // </group>

 private:
  // Buffer fields
  Vector<String> polyType_p;
  Vector<String> polyMode_p;
  Vector<Complex> scaleFactor_p;
  Array<Double> validDomain_p;
  Vector<Int> nPolyAmp_p;
  Vector<Int> nPolyPhase_p;
  Array<Double> polyCoeffAmp_p;
  Array<Double> polyCoeffPhase_p;
  Vector<String> phaseUnits_p;
  Vector<Complex> sideBandRef_p;

  // Buffer field status flags
  Bool polyTypeOK_p;
  Bool polyModeOK_p;
  Bool scaleFactorOK_p;
  Bool validDomainOK_p;
  Bool nPolyAmpOK_p;
  Bool nPolyPhaseOK_p;
  Bool polyCoeffAmpOK_p;
  Bool polyCoeffPhaseOK_p;
  Bool phaseUnitsOK_p;
  Bool sideBandRefOK_p;
};


} //# NAMESPACE CASA - END

#endif
   
  



