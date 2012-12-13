//#---------------------------------------------------------------------------
//# SDMath.h: A collection of single dish mathematical operations
//#---------------------------------------------------------------------------
//# Copyright (C) 2004
//# ATNF
//#
//# This program is free software; you can redistribute it and/or modify it
//# under the terms of the GNU General Public License as published by the Free
//# Software Foundation; either version 2 of the License, or (at your option)
//# any later version.
//#
//# This program is distributed in the hope that it will be useful, but
//# WITHOUT ANY WARRANTY; without even the implied warranty of
//# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General
//# Public License for more details.
//#
//# You should have received a copy of the GNU General Public License along
//# with this program; if not, write to the Free Software Foundation, Inc.,
//# 675 Massachusetts Ave, Cambridge, MA 02139, USA.
//#
//# Correspondence concerning this software should be addressed as follows:
//#        Internet email: Malte.Marquarding@csiro.au
//#        Postal address: Malte Marquarding,
//#                        Australia Telescope National Facility,
//#                        P.O. Box 76,
//#                        Epping, NSW, 2121,
//#                        AUSTRALIA
//#
//# $Id:
//#---------------------------------------------------------------------------
#ifndef SDMATH_H
#define SDMATH_H

#include <string>
#include <vector>
#include <casa/aips.h>
#include <casa/Utilities/CountedPtr.h>
#include <coordinates/Coordinates/FrequencyAligner.h>

#include <singledish/SDDefs.h>
#include <singledish/SDLog.h>

class casa::Table;
class casa::MEpoch;
class casa::MPosition;
//template<class T> class casa::PtrBlock;
//template<class T> class casa::Matrix;
//template<class T> class casa::ROScalarColumn;
//template<class T> class casa::ROArrayColumn;

namespace asap {

class SDMemTable;
class SDDataDesc;
  
class SDMath : private SDLog {

public:
  
  // Default constructor 
  SDMath();
  
  // Copy Constructor (copy semantics)
  SDMath(const SDMath& other);
  
  // Assignment  (copy semantics)
  SDMath &operator=(const SDMath& other);
  
  // Destructor
  ~SDMath();
  
  // Binary Table operators. op=ADD, SUB, MUL, DIV, QUOTIENT
  casa::CountedPtr<SDMemTable> binaryOperate(const casa::CountedPtr<SDMemTable>& left,
					     const casa::CountedPtr<SDMemTable>& right,
					     const casa::String& op, casa::Bool preserve,
					     casa::Bool tSys);
    
    // Average in time
    casa::CountedPtr<SDMemTable> average(const std::vector<casa::CountedPtr<SDMemTable> >& in,
					 const casa::Vector<casa::Bool>& mask,
					 casa::Bool scanAverage, 
					 const casa::String& weightStr,
					 casa::Bool align=casa::False);
    
  // Statistics. If row<0, all rows are done otherwise, just the 
  // specified row.
  std::vector<float> statistic(const casa::CountedPtr<SDMemTable>& in, 
			       const casa::Vector<casa::Bool>& mask, 
			       const casa::String& which, casa::Int row) const;
			       
// Bin up spectra
   SDMemTable* bin(const SDMemTable& in, casa::Int width);

  // Resample spectra
   SDMemTable* resample(const SDMemTable& in, const casa::String& method, 
                        casa::Float factor);

// Smooth
  SDMemTable* smooth(const SDMemTable& in, const casa::String& kernel,
		      casa::Float width, casa::Bool doAll);

// Flux conversion between Jansky and Kelvin
   SDMemTable* convertFlux(const SDMemTable& in, casa::Float D, 
			   casa::Float etaAp, 
			   casa::Float JyPerK, casa::Bool doAll);

// Gain-elevation correction
   SDMemTable* gainElevation(const SDMemTable& in, 
			     const casa::Vector<casa::Float>& coeffs,
			     const casa::String& fileName,
			     const casa::String& method, 
			     casa::Bool doAll);

// Frequency Alignment
   SDMemTable* frequencyAlignment(const SDMemTable& in, const casa::String& refTime,
                                   const casa::String& method, casa::Bool perFreqID);

// Opacity correction
   SDMemTable* opacity(const SDMemTable& in, casa::Float tau, casa::Bool doAll);

// Simple unary mathematical operations.  what=0 (mul) or 1 (add)
   SDMemTable* unaryOperate(const SDMemTable& in, casa::Float offset, 
                            casa::Bool doAll, casa::uInt what, casa::Bool tSys);

  // Average polarizations.
  SDMemTable* averagePol(const SDMemTable& in, 
			 const casa::Vector<casa::Bool>& mask,
			 const casa::String& wtStr);

  SDMemTable* frequencySwitch(const SDMemTable& in);
  

// Rotate XY phase. Value in degrees.
   void rotateXYPhase(SDMemTable& in, casa::Float value, casa::Bool doAll);

// Rotate Q & U by operating on the raw correlations.Value in degrees.
   void rotateLinPolPhase(SDMemTable& in, casa::Float value, casa::Bool doAll);

  
 private:

// Weighting type for time averaging

  enum WeightType {NONE=0,VAR,TSYS,TINT,TINTSYS,nWeightTypes};

// Function to use accumulate data during time averaging

  void accumulate(casa::Double& timeSum, casa::Double& intSum, 
		  casa::Int& nAccum,
		  casa::MaskedArray<casa::Float>& sum, 
		  casa::Array<casa::Float>& sumSq,
		  casa::Array<casa::Float>& nPts, 
		  casa::Array<casa::Float>& tSysSum,
		  casa::Array<casa::Float>& tSysSqSum,
		  const casa::Array<casa::Float>& tSys,  
		  const casa::Array<casa::Float>& nInc,
		  const casa::Vector<casa::Bool>& mask, 
		  casa::Double time, casa::Double interval,
		  const std::vector<casa::CountedPtr<SDMemTable> >& in,
		  casa::uInt iTab, casa::uInt iRow, 
		  casa::uInt axis, casa::uInt nAxesSub,
		  casa::Bool useMask, WeightType wtType);

// Work out conversion factor for converting Jy<->K per IF per row and apply
    void convertBrightnessUnits(SDMemTable* pTabOut, const SDMemTable& in, 
                                casa::Bool toKelvin, casa::Float sFac, casa::Bool doAll);

// Convert weight string to enum value

   void convertWeightString(WeightType& wt, const casa::String& weightStr,
                             casa::Bool listType);;

// Convert interpolation type string
//   void convertInterpString(casa::Int& type, const casa::String& interp);
   void convertInterpString(casa::InterpolateArray1D<casa::Double,casa::Float>::InterpolationMethod& method,  
                             const casa::String& interp);

// Scale data with values from an ascii Table
   void scaleFromAsciiTable(SDMemTable* pTabOut, const SDMemTable& in, 
                              const casa::String& fileName,
                              const casa::String& col0, const casa::String& col1,
                              const casa::String& methodStr, casa::Bool doAll,
                              const casa::Vector<casa::Float>& xOut, casa::Bool doTSys);

// Scale data with values from a Table
   void scaleFromTable(SDMemTable* pTabOut, const SDMemTable& in, const casa::Table& tTable,
                         const casa::String& col0, const casa::String& col1,
                         const casa::String& methodStr, casa::Bool doAll,
                         const casa::Vector<casa::Float>& xOut, casa::Bool doTSys);

// Scale data and optionally TSys by values in a Vector
   void scaleByVector(SDMemTable* pTabOut, const SDMemTable& in,
                       casa::Bool doAll, const casa::Vector<casa::Float>& factor,
                       casa::Bool doTSys);

// Convert time String to Epoch
   casa::MEpoch epochFromString(const casa::String& str, casa::MEpoch::Types timeRef);

// Function to fill Scan Container when averaging in time

  void fillSDC(SDContainer& sc, const casa::Array<casa::Bool>& mask,
                const casa::Array<casa::Float>& data,
                const casa::Array<casa::Float>& tSys,
                casa::Int scanID, casa::Double timeStamp,
                casa::Double interval, const casa::String& sourceName,
                const casa::Vector<casa::uInt>& freqID);

// Format EPoch
   casa::String formatEpoch(const casa::MEpoch& epoch) const; 

// Align in Frequency
   SDMemTable* frequencyAlign(const SDMemTable& in,
                              casa::MFrequency::Types system,
                              const casa::String& timeRef,
                              const casa::String& method, 
			       casa::Bool perIF);

// Generate frequency aligners
   void generateFrequencyAligners(casa::PtrBlock<casa::FrequencyAligner<casa::Float>* >& a,
                                   const SDDataDesc& dDesc,
                                   const SDMemTable& in, casa::uInt nChan,
                                   casa::MFrequency::Types system,
                                   const casa::MPosition& refPos,
                                   const casa::MEpoch& refEpoch, 
				   casa::Bool perFreqID);

// Generate data description table(combines source and freqID);
   void generateDataDescTable(casa::Matrix<casa::uInt>& ddIdx,
                               SDDataDesc& dDesc,
                               casa::uInt nIF,
                               const SDMemTable& in,
                               const casa::Table& tabIn,
                               const casa::ROScalarColumn<casa::String>& srcCol,
                               const casa::ROArrayColumn<casa::uInt>& fqIDCol,
                               casa::Bool perFreqID);

// Get row range from SDMemTable state
  casa::Vector<casa::uInt> getRowRange(const SDMemTable& in) const;

// Is row in the row range ?
  casa::Bool rowInRange(casa::uInt i, const casa::Vector<casa::uInt>& range) const;

// Set slice to cursor or all axes
    void setCursorSlice(casa::IPosition& start, casa::IPosition& end, 
			casa::Bool doAll, const SDMemTable& in) const;

// Function to normalize data when averaging in time

  void normalize(casa::MaskedArray<casa::Float>& data,
                  const casa::Array<casa::Float>& sumSq,
                  const casa::Array<casa::Float>& tSysSumSq,
                  const casa::Array<casa::Float>& nPts,
                  casa::Double intSum,
		 WeightType wtType, casa::Int axis, casa::Int nAxes);

// Put the data and mask into the SDContainer
   void putDataInSDC(SDContainer& sc, const casa::Array<casa::Float>& data,
		     const casa::Array<casa::Bool>& mask);

// Read ascii file into a Table

  casa::Table readAsciiFile(const casa::String& fileName) const;

}; // class

} // namespace

#endif
