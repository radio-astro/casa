//# GJonesMCol.h: GJones cal_main table column access
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

#ifndef CALIBRATION_GJONESMCOL_H
#define CALIBRATION_GJONESMCOL_H

#include <calibration/CalTables/SolvableVJMCol.h>
#include <calibration/CalTables/GJonesTable.h>

namespace casa { //# NAMESPACE CASA - BEGIN

// <summary> 
// ROGJonesPolyMCol: Read-only GJonesPoly cal_main column access
// </summary>

// <use visibility=export>

// <reviewed reviewer="" date="" tests="" demos="">

// <prerequisite>
//  <li> <linkto class="ROCalMainColumns">ROCalMainColumns</linkto> module
// </prerequisite>
//
// <etymology>
// From "read-only","GJonesPoly","cal main" and "columns".
// </etymology>
//
// <synopsis>
// The ROGJonesPolyMCol class allows read-only access to columns in the
// GJonesPoly main calibration table. GJonesPoly matrices are used to 
// store electronic gain information in the Measurement Equation formalism 
// in polynomial form over time.
// </synopsis>
//
// <example>
// <srcblock>
// </srcblock>
// </example>
//
// <motivation>
// Encapsulate read-only access to GJonesPoly calibration table columns.
// </motivation>
//
// <todo asof="02/02/01">
// (i) Deal with non-standard columns.
// (ii) Migrate GJonesMCol to this file.
// </todo>

class ROGJonesPolyMCol : public ROGJonesMCol
{
 public:
  // Construct from a calibration table
  ROGJonesPolyMCol (const GJonesPolyTable& gjpTable);

  // Default destructor
  virtual ~ROGJonesPolyMCol() {};

  // Read-only column accessors
  const ROScalarColumn<String>& polyType() const {return polyType_p;};
  const ROScalarColumn<String>& polyMode() const {return polyMode_p;};
  const ROScalarColumn<Complex>& scaleFactor() const {return scaleFactor_p;};
  const ROScalarColumn<Int>& nPolyAmp() const {return nPolyAmp_p;};
  const ROScalarColumn<Int>& nPolyPhase() const {return nPolyPhase_p;};
  const ROArrayColumn<Double>& polyCoeffAmp() const {return polyCoeffAmp_p;};
  const ROArrayColumn<Double>& polyCoeffPhase() const 
    {return polyCoeffPhase_p;};
  const ROScalarColumn<String>& phaseUnits() const {return phaseUnits_p;};

 protected:
  // Prohibit public use of the null constructor, which
  // does not produce a usable object.
  ROGJonesPolyMCol() {};

 private:
  // Prohibit copy constructor and assignment operator 
  ROGJonesPolyMCol (const ROGJonesPolyMCol&);
  ROGJonesPolyMCol& operator= (const ROGJonesPolyMCol&);

  // Private column accessors
  ROScalarColumn<String> polyType_p;
  ROScalarColumn<String> polyMode_p;
  ROScalarColumn<Complex> scaleFactor_p;
  ROScalarColumn<Int> nPolyAmp_p;
  ROScalarColumn<Int> nPolyPhase_p;
  ROArrayColumn<Double> polyCoeffAmp_p;
  ROArrayColumn<Double> polyCoeffPhase_p;
  ROScalarColumn<String> phaseUnits_p;
};

// <summary> 
// GJonesPolyMCol: RW GJonesPoly cal_main column access
// </summary>

// <use visibility=export>

// <reviewed reviewer="" date="" tests="" demos="">

// <prerequisite>
//   <li> <linkto class="CalMainColumns">CalMainColumns</linkto> module
// </prerequisite>
//
// <etymology>
// From "GJonesPoly","cal main table" and "columns".
// </etymology>
//
// <synopsis>
// The GJonesPolyMCol class allows read-only access to columns in the
// GJonesPoly main calibration table. GJonesPoly matrices are used to 
// store electronic gain information in the Measurement Equation formalism 
// in polynomial form over time.
// </synopsis>
//
// <example>
// <srcblock>
// </srcblock>
// </example>
//
// <motivation>
// Encapsulate read-write access to GJonesPoly calibration table columns.
// </motivation>
//
// <todo asof="01/07/01">
// (i) Deal with non-standard columns.
// </todo>

class GJonesPolyMCol : public GJonesMCol
{
 public:
  // Construct from a calibration table
  GJonesPolyMCol (GJonesPolyTable& gjpTable);

  // Default destructor
  virtual ~GJonesPolyMCol() {};

  // Read-write column accessors
  ScalarColumn<String>& polyType() {return polyType_p;};
  ScalarColumn<String>& polyMode() {return polyMode_p;};
  ScalarColumn<Complex>& scaleFactor() {return scaleFactor_p;};
  ScalarColumn<Int>& nPolyAmp() {return nPolyAmp_p;};
  ScalarColumn<Int>& nPolyPhase() {return nPolyPhase_p;};
  ArrayColumn<Double>& polyCoeffAmp() {return polyCoeffAmp_p;};
  ArrayColumn<Double>& polyCoeffPhase() {return polyCoeffPhase_p;};
  ScalarColumn<String>& phaseUnits() {return phaseUnits_p;};

 protected:
  // Prohibit public use of the null constructor, which
  // does not produce a usable object.
  GJonesPolyMCol() {};

 private:
  // Prohibit copy constructor and assignment operator 
  GJonesPolyMCol (const GJonesPolyMCol&);
  GJonesPolyMCol& operator= (const GJonesPolyMCol&);

  // Private column accessors
  ScalarColumn<String> polyType_p;
  ScalarColumn<String> polyMode_p;
  ScalarColumn<Complex> scaleFactor_p;
  ScalarColumn<Int> nPolyAmp_p;
  ScalarColumn<Int> nPolyPhase_p;
  ArrayColumn<Double> polyCoeffAmp_p;
  ArrayColumn<Double> polyCoeffPhase_p;
  ScalarColumn<String> phaseUnits_p;
};

// <summary> 
// ROGJonesSplineMCol: Read-only GJonesSpline cal_main column access
// </summary>

// <use visibility=export>

// <reviewed reviewer="" date="" tests="" demos="">

// <prerequisite>
//  <li> <linkto class="ROCalMainColumns">ROCalMainColumns</linkto> module
// </prerequisite>
//
// <etymology>
// From "read-only","GJonesSpline","cal main" and "columns".
// </etymology>
//
// <synopsis>
// The ROGJonesSplineMCol class allows read-only access to columns in the
// GJonesSpline main calibration table. GJonesSpline matrices are used to 
// store electronic gain information in the Measurement Equation formalism 
// in spline polynomial form over time.
// </synopsis>
//
// <example>
// <srcblock>
// </srcblock>
// </example>
//
// <motivation>
// Encapsulate read-only access to GJonesSpline calibration table columns.
// </motivation>
//
// <todo asof="02/02/01">
// (i) Deal with non-standard columns.
// (ii) Migrate GJonesMCol to this file.
// </todo>

class ROGJonesSplineMCol : public ROGJonesPolyMCol
{
 public:
  // Construct from a calibration table
  ROGJonesSplineMCol (const GJonesSplineTable& gjsTable);

  // Default destructor
  virtual ~ROGJonesSplineMCol() {};

  // Read-only column accessors
  const ROScalarColumn<Int>& nKnotsAmp() const {return nKnotsAmp_p;};
  const ROScalarColumn<Int>& nKnotsPhase() const {return nKnotsPhase_p;};
  const ROArrayColumn<Double>& splineKnotsAmp() const 
    {return splineKnotsAmp_p;};
  const ROArrayColumn<Double>& splineKnotsPhase() const
    {return splineKnotsPhase_p;};

 protected:
  // Prohibit public use of the null constructor, which
  // does not produce a usable object.
  ROGJonesSplineMCol() {};

 private:
  // Prohibit copy constructor and assignment operator 
  ROGJonesSplineMCol (const ROGJonesSplineMCol&);
  ROGJonesSplineMCol& operator= (const ROGJonesSplineMCol&);

  // Private column accessors
  ROScalarColumn<Int> nKnotsAmp_p;
  ROScalarColumn<Int> nKnotsPhase_p;
  ROArrayColumn<Double> splineKnotsAmp_p;
  ROArrayColumn<Double> splineKnotsPhase_p;
};

// <summary> 
// GJonesSplineMCol: RW GJonesSpline cal_main column access
// </summary>

// <use visibility=export>

// <reviewed reviewer="" date="" tests="" demos="">

// <prerequisite>
//   <li> <linkto class="CalMainColumns">CalMainColumns</linkto> module
// </prerequisite>
//
// <etymology>
// From "GJonesSpline","cal main table" and "columns".
// </etymology>
//
// <synopsis>
// The GJonesSplineMCol class allows read-only access to columns in the
// GJonesSpline main calibration table. GJonesSpline matrices are used to 
// store electronic gain information in the Measurement Equation formalism 
// in spline polynomial form over time.
// </synopsis>
//
// <example>
// <srcblock>
// </srcblock>
// </example>
//
// <motivation>
// Encapsulate read-write access to GJonesSpline calibration table columns.
// </motivation>
//
// <todo asof="01/07/01">
// (i) Deal with non-standard columns.
// </todo>

class GJonesSplineMCol : public GJonesPolyMCol
{
 public:
  // Construct from a calibration table
  GJonesSplineMCol (GJonesSplineTable& gjsTable);

  // Default destructor
  virtual ~GJonesSplineMCol() {};

  // Read-write column accessors
  ScalarColumn<Int>& nKnotsAmp() {return nKnotsAmp_p;};
  ScalarColumn<Int>& nKnotsPhase() {return nKnotsPhase_p;};
  ArrayColumn<Double>& splineKnotsAmp() {return splineKnotsAmp_p;};
  ArrayColumn<Double>& splineKnotsPhase() {return splineKnotsPhase_p;};

 protected:
  // Prohibit public use of the null constructor, which
  // does not produce a usable object.
  GJonesSplineMCol() {};

 private:
  // Prohibit copy constructor and assignment operator 
  GJonesSplineMCol (const GJonesSplineMCol&);
  GJonesSplineMCol& operator= (const GJonesSplineMCol&);

  // Private column accessors
  ScalarColumn<Int> nKnotsAmp_p;
  ScalarColumn<Int> nKnotsPhase_p;
  ArrayColumn<Double> splineKnotsAmp_p;
  ArrayColumn<Double> splineKnotsPhase_p;
};


} //# NAMESPACE CASA - END

#endif
   
  



