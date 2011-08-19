//# BJonesMCol.h: BJones cal_main table column access
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

#ifndef CALIBRATION_BJONESMCOL_H
#define CALIBRATION_BJONESMCOL_H

#include <calibration/CalTables/SolvableVJMCol.h>
#include <calibration/CalTables/BJonesTable.h>

namespace casa { //# NAMESPACE CASA - BEGIN

// <summary> 
// ROBJonesMCol: Read-only BJones cal_main column access
// </summary>

// <use visibility=export>

// <reviewed reviewer="" date="" tests="" demos="">

// <prerequisite>
//  <li> <linkto class="ROCalMainColumns">ROCalMainColumns</linkto> module
// </prerequisite>
//
// <etymology>
// From "read-only","BJones","cal main" and "columns".
// </etymology>
//
// <synopsis>
// The ROBJonesMCol class allows read-only access to columns in the
// BJones main calibration table. BJones matrices are used to store
// bandpass information in the Measurement Equation formalism.
// </etymology>
//
// <example>
// <srcblock>
// </srcblock>
// </example>
//
// <motivation>
// Encapsulate read-only access to BJones calibration table columns.
// </motivation>
//
// <todo asof="01/07/01">
// (i) Deal with non-standard columns.
// </todo>

class ROBJonesMCol : public ROSolvableVisJonesMCol
{
 public:
  // Construct from a calibration table
  ROBJonesMCol (const BJonesTable& bjTable);

  // Default destructor
  virtual ~ROBJonesMCol() {};

 protected:
  // Prohibit public use of the null constructor, which
  // does not produce a usable object.
  ROBJonesMCol() {};

 private:
  // Prohibit copy constructor and assignment operator 
  ROBJonesMCol (const ROBJonesMCol&);
  ROBJonesMCol& operator= (const ROBJonesMCol&);
};

// <summary> 
// BJonesMCol: RW BJones cal_main column access
// </summary>

// <use visibility=export>

// <reviewed reviewer="" date="" tests="" demos="">

// <prerequisite>
//   <li> <linkto class="CalMainColumns">CalMainColumns</linkto> module
// </prerequisite>
//
// <etymology>
// From "BJones","cal main table" and "columns".
// </etymology>
//
// <synopsis>
// The BJonesMCol class allows read-write access to columns in the
// BJones main calibration table. BJones matrices are used to store
// bandpass information in the Measurement Equation formalism.
// </etymology>
//
// <example>
// <srcblock>
// </srcblock>
// </example>
//
// <motivation>
// Encapsulate read-write access to BJones calibration table columns.
// </motivation>
//
// <todo asof="01/07/01">
// (i) Deal with non-standard columns.
// </todo>

class BJonesMCol : public SolvableVisJonesMCol
{
 public:
  // Construct from a calibration table
  BJonesMCol (BJonesTable& bjTable);

  // Default destructor
  virtual ~BJonesMCol() {};

 protected:
  // Prohibit public use of the null constructor, which
  // does not produce a usable object.
  BJonesMCol() {};

 private:
  // Prohibit copy constructor and assignment operator 
  BJonesMCol (const BJonesMCol&);
  BJonesMCol& operator= (const BJonesMCol&);
};


// <summary> 
// ROBJonesPolyMCol: Read-only BJonesPoly cal_main column access
// </summary>

// <use visibility=export>

// <reviewed reviewer="" date="" tests="" demos="">

// <prerequisite>
//  <li> <linkto class="ROCalMainColumns">ROCalMainColumns</linkto> module
// </prerequisite>
//
// <etymology>
// From "read-only","BJonesPoly","cal main" and "columns".
// </etymology>
//
// <synopsis>
// The ROBJonesPolyMCol class allows read-only access to columns in the
// BJonesPoly main calibration table. BJonesPoly matrices are used to store
// bandpass information in the Measurement Equation formalism in polynomial
// form.
// </etymology>
//
// <example>
// <srcblock>
// </srcblock>
// </example>
//
// <motivation>
// Encapsulate read-only access to BJonesPoly calibration table columns.
// </motivation>
//
// <todo asof="01/07/01">
// (i) Deal with non-standard columns.
// </todo>

class ROBJonesPolyMCol : public ROBJonesMCol
{
 public:
  // Construct from a calibration table
  ROBJonesPolyMCol (const BJonesPolyTable& bjTable);

  // Default destructor
  virtual ~ROBJonesPolyMCol() {};

  // Read-only column accessors
  const ROScalarColumn<String>& polyType() const {return polyType_p;};
  const ROScalarColumn<String>& polyMode() const {return polyMode_p;};
  const ROScalarColumn<Complex>& scaleFactor() const {return scaleFactor_p;};
  const ROArrayColumn<Double>& validDomain() const {return validDomain_p;};
  const ROScalarColumn<Int>& nPolyAmp() const {return nPolyAmp_p;};
  const ROScalarColumn<Int>& nPolyPhase() const {return nPolyPhase_p;};
  const ROArrayColumn<Double>& polyCoeffAmp() const {return polyCoeffAmp_p;};
  const ROArrayColumn<Double>& polyCoeffPhase() const 
    {return polyCoeffPhase_p;};
  const ROScalarColumn<String>& phaseUnits() const {return phaseUnits_p;};
  const ROScalarColumn<Complex>& sideBandRef() const {return sideBandRef_p;};

 protected:
  // Prohibit public use of the null constructor, which
  // does not produce a usable object.
  ROBJonesPolyMCol() {};

 private:
  // Prohibit copy constructor and assignment operator 
  ROBJonesPolyMCol (const ROBJonesPolyMCol&);
  ROBJonesPolyMCol& operator= (const ROBJonesPolyMCol&);

  // Private column accessors
  ROScalarColumn<String> polyType_p;
  ROScalarColumn<String> polyMode_p;
  ROScalarColumn<Complex> scaleFactor_p;
  ROArrayColumn<Double> validDomain_p;
  ROScalarColumn<Int> nPolyAmp_p;
  ROScalarColumn<Int> nPolyPhase_p;
  ROArrayColumn<Double> polyCoeffAmp_p;
  ROArrayColumn<Double> polyCoeffPhase_p;
  ROScalarColumn<String> phaseUnits_p;
  ROScalarColumn<Complex> sideBandRef_p;
};

// <summary> 
// BJonesPolyMCol: RW BJonesPoly cal_main column access
// </summary>

// <use visibility=export>

// <reviewed reviewer="" date="" tests="" demos="">

// <prerequisite>
//   <li> <linkto class="CalMainColumns">CalMainColumns</linkto> module
// </prerequisite>
//
// <etymology>
// From "BJonesPoly","cal main table" and "columns".
// </etymology>
//
// <synopsis>
// The BJonesPolyMCol class allows read-write access to columns in the
// BJonesPoly main calibration table. BJonesPoly matrices are used to store
// bandpass information in the Measurement Equation formalism in polynomial
// form.
// </etymology>
//
// <example>
// <srcblock>
// </srcblock>
// </example>
//
// <motivation>
// Encapsulate read-write access to BJonesPoly calibration table columns.
// </motivation>
//
// <todo asof="01/07/01">
// (i) Deal with non-standard columns.
// </todo>

class BJonesPolyMCol : public BJonesMCol
{
 public:
  // Construct from a calibration table
  BJonesPolyMCol (BJonesPolyTable& bjTable);

  // Default destructor
  virtual ~BJonesPolyMCol() {};

  // Read-write column accessors
  ScalarColumn<String>& polyType() {return polyType_p;};
  ScalarColumn<String>& polyMode() {return polyMode_p;};
  ScalarColumn<Complex>& scaleFactor() {return scaleFactor_p;};
  ArrayColumn<Double>& validDomain() {return validDomain_p;};
  ScalarColumn<Int>& nPolyAmp() {return nPolyAmp_p;};
  ScalarColumn<Int>& nPolyPhase() {return nPolyPhase_p;};
  ArrayColumn<Double>& polyCoeffAmp() {return polyCoeffAmp_p;};
  ArrayColumn<Double>& polyCoeffPhase() {return polyCoeffPhase_p;};
  ScalarColumn<String>& phaseUnits() {return phaseUnits_p;};
  ScalarColumn<Complex>& sideBandRef() {return sideBandRef_p;};

 protected:
  // Prohibit public use of the null constructor, which
  // does not produce a usable object.
  BJonesPolyMCol() {};

 private:
  // Prohibit copy constructor and assignment operator 
  BJonesPolyMCol (const BJonesPolyMCol&);
  BJonesPolyMCol& operator= (const BJonesPolyMCol&);

  // Private column accessors
  ScalarColumn<String> polyType_p;
  ScalarColumn<String> polyMode_p;
  ScalarColumn<Complex> scaleFactor_p;
  ArrayColumn<Double> validDomain_p;
  ScalarColumn<Int> nPolyAmp_p;
  ScalarColumn<Int> nPolyPhase_p;
  ArrayColumn<Double> polyCoeffAmp_p;
  ArrayColumn<Double> polyCoeffPhase_p;
  ScalarColumn<String> phaseUnits_p;
  ScalarColumn<Complex> sideBandRef_p;
};


} //# NAMESPACE CASA - END

#endif
   
  



