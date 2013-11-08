//#---------------------------------------------------------------------------
//# SDMath.cc: A collection of single dish mathematical operations
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
#include <vector>


#include <casa/aips.h>
#include <casa/iostream.h>
#include <casa/sstream.h>
#include <casa/iomanip.h>
#include <casa/BasicSL/String.h>
#include <casa/Arrays/IPosition.h>
#include <casa/Arrays/Array.h>
#include <casa/Arrays/ArrayIter.h>
#include <casa/Arrays/VectorIter.h>
#include <casa/Arrays/ArrayMath.h>
#include <casa/Arrays/ArrayLogical.h>
#include <casa/Arrays/MaskedArray.h>
#include <casa/Arrays/MaskArrMath.h>
#include <casa/Arrays/MaskArrLogi.h>
#include <casa/Arrays/Matrix.h>
#include <casa/BasicMath/Math.h>
#include <casa/Exceptions.h>
#include <casa/Quanta/Quantum.h>
#include <casa/Quanta/Unit.h>
#include <casa/Quanta/MVEpoch.h>
#include <casa/Quanta/MVTime.h>
#include <casa/Utilities/Assert.h>

#include <coordinates/Coordinates/SpectralCoordinate.h>
#include <coordinates/Coordinates/CoordinateSystem.h>
#include <coordinates/Coordinates/CoordinateUtil.h>
#include <coordinates/Coordinates/FrequencyAligner.h>

#include <lattices/Lattices/LatticeUtilities.h>
#include <lattices/Lattices/RebinLattice.h>

#include <measures/Measures/MEpoch.h>
#include <measures/Measures/MDirection.h>
#include <measures/Measures/MPosition.h>

#include <scimath/Mathematics/VectorKernel.h>
#include <scimath/Mathematics/Convolver.h>
#include <scimath/Mathematics/InterpolateArray1D.h>
#include <scimath/Functionals/Polynomial.h>

#include <tables/Tables/Table.h>
#include <tables/Tables/ScalarColumn.h>
#include <tables/Tables/ArrayColumn.h>
#include <tables/Tables/ReadAsciiTable.h>

#include <singledish/MathUtils.h>
#include <singledish/SDDefs.h>
#include <singledish/SDAttr.h>
#include <singledish/SDContainer.h>
#include <singledish/SDMemTable.h>

#include <singledish/SDMath.h>
#include <singledish/SDPol.h>

using namespace casa;
using namespace asap;


SDMath::SDMath()
{
}

SDMath::SDMath(const SDMath& other)
{

// No state

}

SDMath& SDMath::operator=(const SDMath& other)
{
  if (this != &other) {
// No state
  }
  return *this;
}

SDMath::~SDMath()
{;}


SDMemTable* SDMath::frequencyAlignment(const SDMemTable& in,
                                       const String& refTime,
                                       const String& method,
                                       Bool perFreqID)
{
  // Get frame info from Table
   std::vector<std::string> info = in.getCoordInfo();

   // Parse frequency system
   String systemStr(info[1]);
   String baseSystemStr(info[3]);
   if (baseSystemStr==systemStr) {
     throw(AipsError("You have not set a frequency frame different from the initial - use function set_freqframe"));
   }

   MFrequency::Types freqSystem;
   MFrequency::getType(freqSystem, systemStr);

   return frequencyAlign(in, freqSystem, refTime, method, perFreqID);
}



CountedPtr<SDMemTable>
SDMath::average(const std::vector<CountedPtr<SDMemTable> >& in,
                const Vector<Bool>& mask, Bool scanAv,
                const String& weightStr, Bool alignFreq)
// Weighted averaging of spectra from one or more Tables.
{
  // Convert weight type
  WeightType wtType = NONE;
  convertWeightString(wtType, weightStr, True);

  // Create output Table by cloning from the first table
  SDMemTable* pTabOut = new SDMemTable(*in[0],True);
  if (in.size() > 1) {
    for (uInt i=1; i < in.size(); ++i) {
      pTabOut->appendToHistoryTable(in[i]->getHistoryTable());
    }
  }
  // Setup
  IPosition shp = in[0]->rowAsMaskedArray(0).shape();      // Must not change
  Array<Float> arr(shp);
  Array<Bool> barr(shp);
  const Bool useMask = (mask.nelements() == shp(asap::ChanAxis));

  // Columns from Tables
  ROArrayColumn<Float> tSysCol;
  ROScalarColumn<Double> mjdCol;
  ROScalarColumn<String> srcNameCol;
  ROScalarColumn<Double> intCol;
  ROArrayColumn<uInt> fqIDCol;
  ROScalarColumn<Int> scanIDCol;

  // Create accumulation MaskedArray. We accumulate for each
  // channel,if,pol,beam Note that the mask of the accumulation array
  // will ALWAYS remain ALL True.  The MA is only used so that when
  // data which is masked Bad is added to it, that data does not
  // contribute.

  Array<Float> zero(shp);
  zero=0.0;
  Array<Bool> good(shp);
  good = True;
  MaskedArray<Float> sum(zero,good);

  // Counter arrays
  Array<Float> nPts(shp);             // Number of points
  nPts = 0.0;
  Array<Float> nInc(shp);             // Increment
  nInc = 1.0;

  // Create accumulation Array for variance. We accumulate for each
  // if,pol,beam, but average over channel.  So we need a shape with
  // one less axis dropping channels.
  const uInt nAxesSub = shp.nelements() - 1;
  IPosition shp2(nAxesSub);
  for (uInt i=0,j=0; i<(nAxesSub+1); i++) {
     if (i!=asap::ChanAxis) {
       shp2(j) = shp(i);
       j++;
     }
  }
  Array<Float> sumSq(shp2);
  sumSq = 0.0;
  IPosition pos2(nAxesSub,0);                        // For indexing

  // Time-related accumulators
  Double time;
  Double timeSum = 0.0;
  Double intSum = 0.0;
  Double interval = 0.0;

  // To get the right shape for the Tsys accumulator we need to access
  // a column from the first table.  The shape of this array must not
  // change.  Note however that since the TSysSqSum array is used in a
  // normalization process, and that I ignore the channel axis
  // replication of values for now, it loses a dimension

  Array<Float> tSysSum, tSysSqSum;
  {
    const Table& tabIn = in[0]->table();
    tSysCol.attach(tabIn,"TSYS");
    tSysSum.resize(tSysCol.shape(0));
    tSysSqSum.resize(shp2);
  }
  tSysSum = 0.0;
  tSysSqSum = 0.0;
  Array<Float> tSys;

  // Scan and row tracking
  Int oldScanID = 0;
  Int outScanID = 0;
  Int scanID = 0;
  Int rowStart = 0;
  Int nAccum = 0;
  Int tableStart = 0;

  // Source and FreqID
  String sourceName, oldSourceName, sourceNameStart;
  Vector<uInt> freqID, freqIDStart, oldFreqID;

  // Loop over tables
  Float fac = 1.0;
  const uInt nTables = in.size();
  for (uInt iTab=0; iTab<nTables; iTab++) {

    // Should check that the frequency tables don't change if doing
    // FreqAlignment

    // Attach columns to Table
     const Table& tabIn = in[iTab]->table();
     tSysCol.attach(tabIn, "TSYS");
     mjdCol.attach(tabIn, "TIME");
     srcNameCol.attach(tabIn, "SRCNAME");
     intCol.attach(tabIn, "INTERVAL");
     fqIDCol.attach(tabIn, "FREQID");
     scanIDCol.attach(tabIn, "SCANID");

     // Loop over rows in Table
     const uInt nRows = in[iTab]->nRow();
     for (uInt iRow=0; iRow<nRows; iRow++) {
       // Check conformance
        IPosition shp2 = in[iTab]->rowAsMaskedArray(iRow).shape();
        if (!shp.isEqual(shp2)) {
          delete pTabOut;
           throw (AipsError("Shapes for all rows must be the same"));
        }

        // If we are not doing scan averages, make checks for source
        // and frequency setup and warn if averaging across them
        scanIDCol.getScalar(iRow, scanID);

        // Get quantities from columns
        srcNameCol.getScalar(iRow, sourceName);
        mjdCol.get(iRow, time);
        tSysCol.get(iRow, tSys);
        intCol.get(iRow, interval);
        fqIDCol.get(iRow, freqID);

        // Initialize first source and freqID
        if (iRow==0 && iTab==0) {
          sourceNameStart = sourceName;
          freqIDStart = freqID;
        }

        // If we are doing scan averages, see if we are at the end of
        // an accumulation period (scan).  We must check soutce names
        // too, since we might have two tables with one scan each but
        // different source names; we shouldn't average different
        // sources together
        if (scanAv && ( (scanID != oldScanID)  ||
                        (iRow==0 && iTab>0 && sourceName!=oldSourceName))) {

          // Normalize data in 'sum' accumulation array according to
          // weighting scheme
           normalize(sum, sumSq, tSysSqSum, nPts, intSum, wtType,
                     asap::ChanAxis, nAxesSub);

           // Get ScanContainer for the first row of this averaged Scan
           SDContainer scOut = in[iTab]->getSDContainer(rowStart);

           // Fill scan container. The source and freqID come from the
           // first row of the first table that went into this average
           // ( should be the same for all rows in the scan average)

           Float nR(nAccum);
           fillSDC(scOut, sum.getMask(), sum.getArray(), tSysSum/nR, outScanID,
                   timeSum/nR, intSum, sourceNameStart, freqIDStart);

           // Write container out to Table
           pTabOut->putSDContainer(scOut);

           // Reset accumulators
           sum = 0.0;
           sumSq = 0.0;
           nAccum = 0;

           tSysSum =0.0;
           tSysSqSum =0.0;
           timeSum = 0.0;
           intSum = 0.0;
           nPts = 0.0;

           // Increment
           rowStart = iRow;              // First row for next accumulation
           tableStart = iTab;            // First table for next accumulation
           sourceNameStart = sourceName; // First source name for next
                                         // accumulation
           freqIDStart = freqID;         // First FreqID for next accumulation

           oldScanID = scanID;
           outScanID += 1;               // Scan ID for next
                                         // accumulation period
        }

        // Accumulate
        accumulate(timeSum, intSum, nAccum, sum, sumSq, nPts,
                   tSysSum, tSysSqSum, tSys,
                   nInc, mask, time, interval, in, iTab, iRow, asap::ChanAxis,
                   nAxesSub, useMask, wtType);
        oldSourceName = sourceName;
        oldFreqID = freqID;
     }
  }

  // OK at this point we have accumulation data which is either
  //   - accumulated from all tables into one row
  // or
  //   - accumulated from the last scan average
  //
  // Normalize data in 'sum' accumulation array according to weighting
  // scheme

  normalize(sum, sumSq, tSysSqSum, nPts, intSum, wtType,
            asap::ChanAxis, nAxesSub);

  // Create and fill container.  The container we clone will be from
  // the last Table and the first row that went into the current
  // accumulation.  It probably doesn't matter that much really...
  Float nR(nAccum);
  SDContainer scOut = in[tableStart]->getSDContainer(rowStart);
  fillSDC(scOut, sum.getMask(), sum.getArray(), tSysSum/nR, outScanID,
          timeSum/nR, intSum, sourceNameStart, freqIDStart);
  pTabOut->putSDContainer(scOut);
  pTabOut->resetCursor();

  return CountedPtr<SDMemTable>(pTabOut);
}



CountedPtr<SDMemTable>
SDMath::binaryOperate(const CountedPtr<SDMemTable>& left,
                      const CountedPtr<SDMemTable>& right,
                      const String& op, Bool preserve, Bool doTSys)
{

  // Check operator
  String op2(op);
  op2.upcase();
  uInt what = 0;
  if (op2=="ADD") {
     what = 0;
  } else if (op2=="SUB") {
     what = 1;
  } else if (op2=="MUL") {
     what = 2;
  } else if (op2=="DIV") {
     what = 3;
  } else if (op2=="QUOTIENT") {
     what = 4;
     doTSys = True;
  } else {
    throw( AipsError("Unrecognized operation"));
  }

  // Check rows
  const uInt nRowLeft = left->nRow();
  const uInt nRowRight = right->nRow();
  Bool ok = (nRowRight==1 && nRowLeft>0) ||
            (nRowRight>=1 && nRowLeft==nRowRight);
  if (!ok) {
     throw (AipsError("The right Scan Table can have one row or the same number of rows as the left Scan Table"));
  }

  // Input Tables
  const Table& tLeft = left->table();
  const Table& tRight = right->table();

  // TSys columns
  ROArrayColumn<Float> tSysLeftCol, tSysRightCol;
  if (doTSys) {
    tSysLeftCol.attach(tLeft, "TSYS");
    tSysRightCol.attach(tRight, "TSYS");
  }

  // First row for right
  Array<Float> tSysLeftArr, tSysRightArr;
  if (doTSys) tSysRightCol.get(0, tSysRightArr);
  MaskedArray<Float>* pMRight =
    new MaskedArray<Float>(right->rowAsMaskedArray(0));

  IPosition shpRight = pMRight->shape();

  // Output Table cloned from left
  SDMemTable* pTabOut = new SDMemTable(*left, True);
  pTabOut->appendToHistoryTable(right->getHistoryTable());

  // Loop over rows
  for (uInt i=0; i<nRowLeft; i++) {

    // Get data
    MaskedArray<Float> mLeft(left->rowAsMaskedArray(i));
    IPosition shpLeft = mLeft.shape();
    if (doTSys) tSysLeftCol.get(i, tSysLeftArr);

    if (nRowRight>1) {
      delete pMRight;
      pMRight = new MaskedArray<Float>(right->rowAsMaskedArray(i));
      shpRight = pMRight->shape();
      if (doTSys) tSysRightCol.get(i, tSysRightArr);
    }

    if (!shpRight.isEqual(shpLeft)) {
      delete pTabOut;
      delete pMRight;
      throw(AipsError("left and right scan tables are not conformant"));
    }
    if (doTSys) {
      if (!tSysRightArr.shape().isEqual(tSysRightArr.shape())) {
        delete pTabOut;
        delete pMRight;
        throw(AipsError("left and right Tsys data are not conformant"));
      }
      if (!shpRight.isEqual(tSysRightArr.shape())) {
        delete pTabOut;
        delete pMRight;
        throw(AipsError("left and right scan tables are not conformant"));
      }
    }

    // Make container
     SDContainer sc = left->getSDContainer(i);

     // Operate on data and TSys
     if (what==0) {
        MaskedArray<Float> tmp = mLeft + *pMRight;
        putDataInSDC(sc, tmp.getArray(), tmp.getMask());
        if (doTSys) sc.putTsys(tSysLeftArr+tSysRightArr);
     } else if (what==1) {
        MaskedArray<Float> tmp = mLeft - *pMRight;
        putDataInSDC(sc, tmp.getArray(), tmp.getMask());
        if (doTSys) sc.putTsys(tSysLeftArr-tSysRightArr);
     } else if (what==2) {
        MaskedArray<Float> tmp = mLeft * *pMRight;
        putDataInSDC(sc, tmp.getArray(), tmp.getMask());
        if (doTSys) sc.putTsys(tSysLeftArr*tSysRightArr);
     } else if (what==3) {
        MaskedArray<Float> tmp = mLeft / *pMRight;
        putDataInSDC(sc, tmp.getArray(), tmp.getMask());
        if (doTSys) sc.putTsys(tSysLeftArr/tSysRightArr);
     } else if (what==4) {
       if (preserve) {
         MaskedArray<Float> tmp = (tSysRightArr * mLeft / *pMRight) -
           tSysRightArr;
         putDataInSDC(sc, tmp.getArray(), tmp.getMask());
       } else {
         MaskedArray<Float> tmp = (tSysRightArr * mLeft / *pMRight) -
           tSysLeftArr;
         putDataInSDC(sc, tmp.getArray(), tmp.getMask());
       }
       sc.putTsys(tSysRightArr);
     }

     // Put new row in output Table
     pTabOut->putSDContainer(sc);
  }
  if (pMRight) delete pMRight;
  pTabOut->resetCursor();

  return CountedPtr<SDMemTable>(pTabOut);
}


std::vector<float> SDMath::statistic(const CountedPtr<SDMemTable>& in,
                                     const Vector<Bool>& mask,
                                     const String& which, Int row) const
//
// Perhaps iteration over pol/beam/if should be in here
// and inside the nrow iteration ?
//
{
  const uInt nRow = in->nRow();

// Specify cursor location

  IPosition start, end;
  Bool doAll = False;
  setCursorSlice(start, end, doAll, *in);

// Loop over rows

  const uInt nEl = mask.nelements();
  uInt iStart = 0;
  uInt iEnd = in->nRow()-1;
//
  if (row>=0) {
     iStart = row;
     iEnd = row;
  }
//
  std::vector<float> result(iEnd-iStart+1);
  for (uInt ii=iStart; ii <= iEnd; ++ii) {

// Get row and deconstruct

     MaskedArray<Float> dataIn = (in->rowAsMaskedArray(ii))(start,end);
     Array<Float> v = dataIn.getArray().nonDegenerate();
     Array<Bool>  m = dataIn.getMask().nonDegenerate();

// Access desired piece of data

//     Array<Float> v((arr(start,end)).nonDegenerate());
//     Array<Bool> m((barr(start,end)).nonDegenerate());

// Apply OTF mask

     MaskedArray<Float> tmp;
     if (m.nelements()==nEl) {
       tmp.setData(v,m&&mask);
     } else {
       tmp.setData(v,m);
     }

// Get statistic

     result[ii-iStart] = mathutil::statistics(which, tmp);
  }
//
  return result;
}


SDMemTable* SDMath::bin(const SDMemTable& in, Int width)
{
  SDHeader sh = in.getSDHeader();
  SDMemTable* pTabOut = new SDMemTable(in, True);

// Bin up SpectralCoordinates

  IPosition factors(1);
  factors(0) = width;
  for (uInt j=0; j<in.nCoordinates(); ++j) {
    CoordinateSystem cSys;
    cSys.addCoordinate(in.getSpectralCoordinate(j));
    CoordinateSystem cSysBin =
      CoordinateUtil::makeBinnedCoordinateSystem(factors, cSys, False);
//
    SpectralCoordinate sCBin = cSysBin.spectralCoordinate(0);
    pTabOut->setCoordinate(sCBin, j);
  }

// Use RebinLattice to find shape

  IPosition shapeIn(1,sh.nchan);
  IPosition shapeOut = RebinLattice<Float>::rebinShape(shapeIn, factors);
  sh.nchan = shapeOut(0);
  pTabOut->putSDHeader(sh);

// Loop over rows and bin along channel axis

  for (uInt i=0; i < in.nRow(); ++i) {
    SDContainer sc = in.getSDContainer(i);
//
    Array<Float> tSys(sc.getTsys());                           // Get it out before sc changes shape

// Bin up spectrum

    MaskedArray<Float> marr(in.rowAsMaskedArray(i));
    MaskedArray<Float> marrout;
    LatticeUtilities::bin(marrout, marr, asap::ChanAxis, width);

// Put back the binned data and flags

    IPosition ip2 = marrout.shape();
    sc.resize(ip2);
//
    putDataInSDC(sc, marrout.getArray(), marrout.getMask());

// Bin up Tsys.

    Array<Bool> allGood(tSys.shape(),True);
    MaskedArray<Float> tSysIn(tSys, allGood, True);
//
    MaskedArray<Float> tSysOut;
    LatticeUtilities::bin(tSysOut, tSysIn, asap::ChanAxis, width);
    sc.putTsys(tSysOut.getArray());
//
    pTabOut->putSDContainer(sc);
  }
  return pTabOut;
}

SDMemTable* SDMath::resample(const SDMemTable& in, const String& methodStr,
                             Float width)
//
// Should add the possibility of width being specified in km/s. This means
// that for each freqID (SpectralCoordinate) we will need to convert to an
// average channel width (say at the reference pixel).  Then we would need
// to be careful to make sure each spectrum (of different freqID)
// is the same length.
//
{
   Bool doVel = False;
   if (doVel) {
      for (uInt j=0; j<in.nCoordinates(); ++j) {
         SpectralCoordinate sC = in.getSpectralCoordinate(j);
      }
   }

// Interpolation method

  InterpolateArray1D<Double,Float>::InterpolationMethod interp;
  convertInterpString(interp, methodStr);
  Int interpMethod(interp);

// Make output table

  SDMemTable* pTabOut = new SDMemTable(in, True);

// Resample SpectralCoordinates (one per freqID)

  const uInt nCoord = in.nCoordinates();
  Vector<Float> offset(1,0.0);
  Vector<Float> factors(1,1.0/width);
  Vector<Int> newShape;
  for (uInt j=0; j<in.nCoordinates(); ++j) {
    CoordinateSystem cSys;
    cSys.addCoordinate(in.getSpectralCoordinate(j));
    CoordinateSystem cSys2 = cSys.subImage(offset, factors, newShape);
    SpectralCoordinate sC = cSys2.spectralCoordinate(0);
//
    pTabOut->setCoordinate(sC, j);
  }

// Get header

  SDHeader sh = in.getSDHeader();

// Generate resampling vectors

  const uInt nChanIn = sh.nchan;
  Vector<Float> xIn(nChanIn);
  indgen(xIn);
//
  Int fac =  Int(nChanIn/width);
  Vector<Float> xOut(fac+10);          // 10 to be safe - resize later
  uInt i = 0;
  Float x = 0.0;
  Bool more = True;
  while (more) {
    xOut(i) = x;
//
    i++;
    x += width;
    if (x>nChanIn-1) more = False;
  }
  const uInt nChanOut = i;
  xOut.resize(nChanOut,True);
//
  IPosition shapeIn(in.rowAsMaskedArray(0).shape());
  sh.nchan = nChanOut;
  pTabOut->putSDHeader(sh);

// Loop over rows and resample along channel axis

  Array<Float> valuesOut;
  Array<Bool> maskOut;
  Array<Float> tSysOut;
  Array<Bool> tSysMaskIn(shapeIn,True);
  Array<Bool> tSysMaskOut;
  for (uInt i=0; i < in.nRow(); ++i) {

// Get container

     SDContainer sc = in.getSDContainer(i);

// Get data and Tsys

     const Array<Float>& tSysIn = sc.getTsys();
     const MaskedArray<Float>& dataIn(in.rowAsMaskedArray(i));
     Array<Float> valuesIn = dataIn.getArray();
     Array<Bool> maskIn = dataIn.getMask();

// Interpolate data

     InterpolateArray1D<Float,Float>::interpolate(valuesOut, maskOut, xOut,
                                                  xIn, valuesIn, maskIn,
                                                  interpMethod, True, True);
     sc.resize(valuesOut.shape());
     putDataInSDC(sc, valuesOut, maskOut);

// Interpolate TSys

     InterpolateArray1D<Float,Float>::interpolate(tSysOut, tSysMaskOut, xOut,
                                                  xIn, tSysIn, tSysMaskIn,
                                                  interpMethod, True, True);
    sc.putTsys(tSysOut);

// Put container in output

    pTabOut->putSDContainer(sc);
  }
//
  return pTabOut;
}

SDMemTable* SDMath::unaryOperate(const SDMemTable& in, Float val, Bool doAll,
                                 uInt what, Bool doTSys)
//
// what = 0   Multiply
//        1   Add
{
   SDMemTable* pOut = new SDMemTable(in,False);
   const Table& tOut = pOut->table();
   ArrayColumn<Float> specCol(tOut,"SPECTRA");
   ArrayColumn<Float> tSysCol(tOut,"TSYS");
   Array<Float> tSysArr;

// Get data slice bounds

   IPosition start, end;
   setCursorSlice (start, end, doAll, in);
//
   for (uInt i=0; i<tOut.nrow(); i++) {

// Modify data

      MaskedArray<Float> dataIn(pOut->rowAsMaskedArray(i));
      MaskedArray<Float> dataIn2 = dataIn(start,end);    // Reference
      if (what==0) {
         dataIn2 *= val;
      } else if (what==1) {
         dataIn2 += val;
      }
      specCol.put(i, dataIn.getArray());

// Modify Tsys

      if (doTSys) {
         tSysCol.get(i, tSysArr);
         Array<Float> tSysArr2 = tSysArr(start,end);     // Reference
         if (what==0) {
            tSysArr2 *= val;
         } else if (what==1) {
            tSysArr2 += val;
         }
         tSysCol.put(i, tSysArr);
      }
   }
//
   return pOut;
}

SDMemTable* SDMath::averagePol(const SDMemTable& in, const Vector<Bool>& mask,
                               const String& weightStr)
//
// Average all polarizations together, weighted by variance
//
{
   WeightType wtType = NONE;
   convertWeightString(wtType, weightStr, True);

// Create output Table and reshape number of polarizations

  Bool clear=True;
  SDMemTable* pTabOut = new SDMemTable(in, clear);
  SDHeader header = pTabOut->getSDHeader();
  header.npol = 1;
  pTabOut->putSDHeader(header);
//
  const Table& tabIn = in.table();

// Shape of input and output data

  const IPosition& shapeIn = in.rowAsMaskedArray(0).shape();
  IPosition shapeOut(shapeIn);
  shapeOut(asap::PolAxis) = 1;                          // Average all polarizations
  if (shapeIn(asap::PolAxis)==1) {
    delete  pTabOut;
    throw(AipsError("The input has only one polarisation"));
  }
//
  const uInt nRows = in.nRow();
  const uInt nChan = shapeIn(asap::ChanAxis);
  AlwaysAssert(asap::nAxes==4,AipsError);
  const IPosition vecShapeOut(4,1,1,1,nChan);     // A multi-dim form of a Vector shape
  IPosition start(4), end(4);

// Output arrays

  Array<Float> outData(shapeOut, 0.0);
  Array<Bool> outMask(shapeOut, True);
  const IPosition axes(2, asap::PolAxis, asap::ChanAxis);              // pol-channel plane

// Attach Tsys column if needed

  ROArrayColumn<Float> tSysCol;
  Array<Float> tSys;
  if (wtType==TSYS) {
     tSysCol.attach(tabIn,"TSYS");
  }
//
  const Bool useMask = (mask.nelements() == shapeIn(asap::ChanAxis));

// Loop over rows

   for (uInt iRow=0; iRow<nRows; iRow++) {

// Get data for this row

      MaskedArray<Float> marr(in.rowAsMaskedArray(iRow));
      Array<Float>& arr = marr.getRWArray();
      const Array<Bool>& barr = marr.getMask();

// Get Tsys

      if (wtType==TSYS) {
         tSysCol.get(iRow,tSys);
      }

// Make iterators to iterate by pol-channel planes
// The tSys array is empty unless wtType=TSYS so only
// access the iterator is that is the case

      ReadOnlyArrayIterator<Float> itDataPlane(arr, axes);
      ReadOnlyArrayIterator<Bool> itMaskPlane(barr, axes);
      ReadOnlyArrayIterator<Float>* pItTsysPlane = 0;
      if (wtType==TSYS)
        pItTsysPlane = new ReadOnlyArrayIterator<Float>(tSys, axes);

// Accumulations

      Float fac = 1.0;
      Vector<Float> vecSum(nChan,0.0);

// Iterate through data by pol-channel planes

      while (!itDataPlane.pastEnd()) {

// Iterate through plane by polarization  and accumulate Vectors

        Vector<Float> t1(nChan); t1 = 0.0;
        Vector<Bool> t2(nChan); t2 = True;
        Float tSys = 0.0;
        MaskedArray<Float> vecSum(t1,t2);
        Float norm = 0.0;
        {
           ReadOnlyVectorIterator<Float> itDataVec(itDataPlane.array(), 1);
           ReadOnlyVectorIterator<Bool> itMaskVec(itMaskPlane.array(), 1);
//
           ReadOnlyVectorIterator<Float>* pItTsysVec = 0;
           if (wtType==TSYS) {
              pItTsysVec =
                new ReadOnlyVectorIterator<Float>(pItTsysPlane->array(), 1);
           }
//
           while (!itDataVec.pastEnd()) {

// Create MA of data & mask (optionally including OTF mask) and  get variance for this spectrum

              if (useMask) {
                 const MaskedArray<Float> spec(itDataVec.vector(),
                                               mask&&itMaskVec.vector());
                 if (wtType==VAR) {
                    fac = 1.0 / variance(spec);
                 } else if (wtType==TSYS) {
                    tSys = pItTsysVec->vector()[0];      // Drop pseudo channel dependency
                    fac = 1.0 / tSys / tSys;
                 }
              } else {
                 const MaskedArray<Float> spec(itDataVec.vector(),
                                               itMaskVec.vector());
                 if (wtType==VAR) {
                    fac = 1.0 / variance(spec);
                 } else if (wtType==TSYS) {
                    tSys = pItTsysVec->vector()[0];      // Drop pseudo channel dependency
                    fac = 1.0 / tSys / tSys;
                 }
              }

// Normalize spectrum (without OTF mask) and accumulate

              const MaskedArray<Float> spec(fac*itDataVec.vector(),
                                            itMaskVec.vector());
              vecSum += spec;
              norm += fac;

// Next

              itDataVec.next();
              itMaskVec.next();
              if (wtType==TSYS) pItTsysVec->next();
           }

// Clean up

           if (pItTsysVec) {
              delete pItTsysVec;
              pItTsysVec = 0;
           }
        }

// Normalize summed spectrum

        vecSum /= norm;

// FInd position in input data array.  We are iterating by pol-channel
// plane so all that will change is beam and IF and that's what we want.

        IPosition pos = itDataPlane.pos();

// Write out data. This is a bit messy. We have to reform the Vector
// accumulator into an Array of shape (1,1,1,nChan)

        start = pos;
        end = pos;
        end(asap::ChanAxis) = nChan-1;
        outData(start,end) = vecSum.getArray().reform(vecShapeOut);
        outMask(start,end) = vecSum.getMask().reform(vecShapeOut);

// Step to next beam/IF combination

        itDataPlane.next();
        itMaskPlane.next();
        if (wtType==TSYS) pItTsysPlane->next();
      }

// Generate output container and write it to output table

      SDContainer sc = in.getSDContainer();
      sc.resize(shapeOut);
//
      putDataInSDC(sc, outData, outMask);
      pTabOut->putSDContainer(sc);
//
      if (wtType==TSYS) {
         delete pItTsysPlane;
         pItTsysPlane = 0;
      }
   }

// Set polarization cursor to 0

  pTabOut->setPol(0);
//
  return pTabOut;
}


SDMemTable* SDMath::smooth(const SDMemTable& in,
                           const casa::String& kernelType,
                           casa::Float width, Bool doAll)
//
// Should smooth TSys as well
//
{

  // Number of channels
   const uInt nChan = in.nChan();

   // Generate Kernel
   VectorKernel::KernelTypes type = VectorKernel::toKernelType(kernelType);
   Vector<Float> kernel = VectorKernel::make(type, width, nChan, True, False);

   // Generate Convolver
   IPosition shape(1,nChan);
   Convolver<Float> conv(kernel, shape);

   // New Table
   SDMemTable* pTabOut = new SDMemTable(in,True);

   // Output Vectors
   Vector<Float> valuesOut(nChan);
   Vector<Bool> maskOut(nChan);

   // Get data slice bounds
   IPosition start, end;
   setCursorSlice (start, end, doAll, in);

   // Loop over rows in Table
   for (uInt ri=0; ri < in.nRow(); ++ri) {

     // Get slice of data
      MaskedArray<Float> dataIn = in.rowAsMaskedArray(ri);

      // Deconstruct and get slices which reference these arrays
      Array<Float> valuesIn = dataIn.getArray();
      Array<Bool> maskIn = dataIn.getMask();

      Array<Float> valuesIn2 = valuesIn(start,end);       // ref to valuesIn
      Array<Bool> maskIn2 = maskIn(start,end);

      // Iterate through by spectra
      VectorIterator<Float> itValues(valuesIn2, asap::ChanAxis);
      VectorIterator<Bool> itMask(maskIn2, asap::ChanAxis);
      while (!itValues.pastEnd()) {

        // Smooth
        if (kernelType==VectorKernel::HANNING) {
          mathutil::hanning(valuesOut, maskOut, itValues.vector(),
                            itMask.vector());
          itMask.vector() = maskOut;
        } else {
          mathutil::replaceMaskByZero(itValues.vector(), itMask.vector());
          conv.linearConv(valuesOut, itValues.vector());
        }

        itValues.vector() = valuesOut;
        itValues.next();
        itMask.next();
      }

      // Create and put back
      SDContainer sc = in.getSDContainer(ri);
      putDataInSDC(sc, valuesIn, maskIn);

      pTabOut->putSDContainer(sc);
   }

  return pTabOut;
}



SDMemTable* SDMath::convertFlux(const SDMemTable& in, Float D, Float etaAp,
                                Float JyPerK, Bool doAll)
//
// etaAp = aperture efficiency (-1 means find)
// D     = geometric diameter (m)  (-1 means find)
// JyPerK
//
{
  SDHeader sh = in.getSDHeader();
  SDMemTable* pTabOut = new SDMemTable(in, True);

  // Find out how to convert values into Jy and K (e.g. units might be
  // mJy or mK) Also automatically find out what we are converting to
  // according to the flux unit
  Unit fluxUnit(sh.fluxunit);
  Unit K(String("K"));
  Unit JY(String("Jy"));

  Bool toKelvin = True;
  Double cFac = 1.0;

  if (fluxUnit==JY) {
    pushLog("Converting to K");
    Quantum<Double> t(1.0,fluxUnit);
    Quantum<Double> t2 = t.get(JY);
    cFac = (t2 / t).getValue();               // value to Jy

    toKelvin = True;
    sh.fluxunit = "K";
  } else if (fluxUnit==K) {
    pushLog("Converting to Jy");
    Quantum<Double> t(1.0,fluxUnit);
    Quantum<Double> t2 = t.get(K);
    cFac = (t2 / t).getValue();              // value to K

    toKelvin = False;
    sh.fluxunit = "Jy";
  } else {
    throw(AipsError("Unrecognized brightness units in Table - must be consistent with Jy or K"));
  }

  pTabOut->putSDHeader(sh);

  // Make sure input values are converted to either Jy or K first...
  Float factor = cFac;

  // Select method
  if (JyPerK>0.0) {
    factor *= JyPerK;
    if (toKelvin) factor = 1.0 / JyPerK;
    ostringstream oss;
    oss << "Jy/K = " << JyPerK;
    pushLog(String(oss));
    Vector<Float> factors(in.nRow(), factor);
    scaleByVector(pTabOut, in, doAll, factors, False);
  } else if (etaAp>0.0) {
    Bool throwIt = True;
    Instrument inst = SDAttr::convertInstrument(sh.antennaname, throwIt);
    SDAttr sda;
    if (D < 0) D = sda.diameter(inst);
    Float JyPerK = SDAttr::findJyPerK(etaAp,D);
    ostringstream oss;
    oss << "Jy/K = " << JyPerK;
    pushLog(String(oss));
    factor *= JyPerK;
    if (toKelvin) {
      factor = 1.0 / factor;
    }

    Vector<Float> factors(in.nRow(), factor);
    scaleByVector(pTabOut, in, doAll, factors, False);
  } else {

    // OK now we must deal with automatic look up of values.
    // We must also deal with the fact that the factors need
    // to be computed per IF and may be different and may
    // change per integration.

    pushLog("Looking up conversion factors");
    convertBrightnessUnits (pTabOut, in, toKelvin, cFac, doAll);
  }
  return pTabOut;
}


SDMemTable* SDMath::gainElevation(const SDMemTable& in,
                                  const Vector<Float>& coeffs,
                                  const String& fileName,
                                  const String& methodStr, Bool doAll)
{

  // Get header and clone output table
  SDHeader sh = in.getSDHeader();
  SDMemTable* pTabOut = new SDMemTable(in, True);

  // Get elevation data from SDMemTable and convert to degrees
  const Table& tab = in.table();
  ROScalarColumn<Float> elev(tab, "ELEVATION");
  Vector<Float> x = elev.getColumn();
  x *= Float(180 / C::pi);                        // Degrees

  const uInt nC = coeffs.nelements();
  if (fileName.length()>0 && nC>0) {
    throw(AipsError("You must choose either polynomial coefficients or an ascii file, not both"));
  }

  // Correct
  if (nC>0 || fileName.length()==0) {
    // Find instrument
     Bool throwIt = True;
     Instrument inst = SDAttr::convertInstrument (sh.antennaname, throwIt);

     // Set polynomial
     Polynomial<Float>* pPoly = 0;
     Vector<Float> coeff;
     String msg;
     if (nC>0) {
       pPoly = new Polynomial<Float>(nC);
       coeff = coeffs;
       msg = String("user");
     } else {
       SDAttr sdAttr;
       coeff = sdAttr.gainElevationPoly(inst);
       pPoly = new Polynomial<Float>(3);
       msg = String("built in");
     }

     if (coeff.nelements()>0) {
       pPoly->setCoefficients(coeff);
     } else {
       delete pPoly;
       throw(AipsError("There is no known gain-elevation polynomial known for this instrument"));
     }
     ostringstream oss;
     oss << "Making polynomial correction with " << msg << " coefficients:" << endl;
     oss << "   " <<  coeff;
     pushLog(String(oss));
     const uInt nRow = in.nRow();
     Vector<Float> factor(nRow);
     for (uInt i=0; i<nRow; i++) {
       factor[i] = 1.0 / (*pPoly)(x[i]);
     }
     delete pPoly;
     scaleByVector (pTabOut, in, doAll, factor, True);

  } else {

    // Indicate which columns to read from ascii file
    String col0("ELEVATION");
    String col1("FACTOR");

    // Read and correct

    pushLog("Making correction from ascii Table");
    scaleFromAsciiTable (pTabOut, in, fileName, col0, col1,
                         methodStr, doAll, x, True);
  }

  return pTabOut;
}


SDMemTable* SDMath::opacity(const SDMemTable& in, Float tau, Bool doAll)
{

  // Get header and clone output table

  SDHeader sh = in.getSDHeader();
  SDMemTable* pTabOut = new SDMemTable(in, True);

// Get elevation data from SDMemTable and convert to degrees

  const Table& tab = in.table();
  ROScalarColumn<Float> elev(tab, "ELEVATION");
  Vector<Float> zDist = elev.getColumn();
  zDist = Float(C::pi_2) - zDist;

// Generate correction factor

  const uInt nRow = in.nRow();
  Vector<Float> factor(nRow);
  Vector<Float> factor2(nRow);
  for (uInt i=0; i<nRow; i++) {
     factor[i] = exp(tau)/cos(zDist[i]);
  }

// Correct

  scaleByVector (pTabOut, in, doAll, factor, True);

  return pTabOut;
}


void SDMath::rotateXYPhase(SDMemTable& in, Float value, Bool doAll)
//
// phase in degrees
// assumes linear correlations
//
{
  if (in.nPol() != 4) {
    throw(AipsError("You must have 4 polarizations to run this function"));
  }

   SDHeader sh = in.getSDHeader();
   Instrument inst = SDAttr::convertInstrument(sh.antennaname, False);
   SDAttr sdAtt;
   if (sdAtt.feedPolType(inst) != LINEAR) {
      throw(AipsError("Only linear polarizations are supported"));
   }
//
   const Table& tabIn = in.table();
   ArrayColumn<Float> specCol(tabIn,"SPECTRA");
   IPosition start(asap::nAxes,0);
   IPosition end(asap::nAxes);

// Set cursor slice. Assumes shape the same for all rows

   setCursorSlice (start, end, doAll, in);
   IPosition start3(start);
   start3(asap::PolAxis) = 2;                 // Real(XY)
   IPosition end3(end);
   end3(asap::PolAxis) = 2;
//
   IPosition start4(start);
   start4(asap::PolAxis) = 3;                 // Imag (XY)
   IPosition end4(end);
   end4(asap::PolAxis) = 3;
//
   uInt nRow = in.nRow();
   Array<Float> data;
   for (uInt i=0; i<nRow;++i) {
      specCol.get(i,data);
      IPosition shape = data.shape();

      // Get polarization slice references
      Array<Float> C3 = data(start3,end3);
      Array<Float> C4 = data(start4,end4);

      // Rotate
      SDPolUtil::rotatePhase(C3, C4, value);

      // Put
      specCol.put(i,data);
   }
}


void SDMath::rotateLinPolPhase(SDMemTable& in, Float value, Bool doAll)
//
// phase in degrees
// assumes linear correlations
//
{
   if (in.nPol() != 4) {
      throw(AipsError("You must have 4 polarizations to run this function"));
   }
//
   SDHeader sh = in.getSDHeader();
   Instrument inst = SDAttr::convertInstrument(sh.antennaname, False);
   SDAttr sdAtt;
   if (sdAtt.feedPolType(inst) != LINEAR) {
      throw(AipsError("Only linear polarizations are supported"));
   }
//
   const Table& tabIn = in.table();
   ArrayColumn<Float> specCol(tabIn,"SPECTRA");
   ROArrayColumn<Float> stokesCol(tabIn,"STOKES");
   IPosition start(asap::nAxes,0);
   IPosition end(asap::nAxes);

// Set cursor slice. Assumes shape the same for all rows

   setCursorSlice (start, end, doAll, in);
//
   IPosition start1(start);
   start1(asap::PolAxis) = 0;                // C1 (XX)
   IPosition end1(end);
   end1(asap::PolAxis) = 0;
//
   IPosition start2(start);
   start2(asap::PolAxis) = 1;                 // C2 (YY)
   IPosition end2(end);
   end2(asap::PolAxis) = 1;
//
   IPosition start3(start);
   start3(asap::PolAxis) = 2;                 // C3 ( Real(XY) )
   IPosition end3(end);
   end3(asap::PolAxis) = 2;
//
   IPosition startI(start);
   startI(asap::PolAxis) = 0;                 // I
   IPosition endI(end);
   endI(asap::PolAxis) = 0;
//
   IPosition startQ(start);
   startQ(asap::PolAxis) = 1;                 // Q
   IPosition endQ(end);
   endQ(asap::PolAxis) = 1;
//
   IPosition startU(start);
   startU(asap::PolAxis) = 2;                 // U
   IPosition endU(end);
   endU(asap::PolAxis) = 2;

//
   uInt nRow = in.nRow();
   Array<Float> data, stokes;
   for (uInt i=0; i<nRow;++i) {
      specCol.get(i,data);
      stokesCol.get(i,stokes);
      IPosition shape = data.shape();

// Get linear polarization slice references

      Array<Float> C1 = data(start1,end1);
      Array<Float> C2 = data(start2,end2);
      Array<Float> C3 = data(start3,end3);

// Get STokes slice references

      Array<Float> I = stokes(startI,endI);
      Array<Float> Q = stokes(startQ,endQ);
      Array<Float> U = stokes(startU,endU);

// Rotate

      SDPolUtil::rotateLinPolPhase(C1, C2, C3, I, Q, U, value);

// Put

      specCol.put(i,data);
   }
}

// 'private' functions

void SDMath::convertBrightnessUnits (SDMemTable* pTabOut, const SDMemTable& in,
                                     Bool toKelvin, Float cFac, Bool doAll)
{

// Get header

   SDHeader sh = in.getSDHeader();
   const uInt nChan = sh.nchan;

// Get instrument

   Bool throwIt = True;
   Instrument inst = SDAttr::convertInstrument(sh.antennaname, throwIt);

// Get Diameter (m)

   SDAttr sdAtt;
// Get epoch of first row

   MEpoch dateObs = in.getEpoch(0);

// Generate a Vector of correction factors. One per FreqID

   SDFrequencyTable sdft = in.getSDFreqTable();
   Vector<uInt> freqIDs;
//
   Vector<Float> freqs(sdft.length());
   for (uInt i=0; i<sdft.length(); i++) {
      freqs(i) = (nChan/2 - sdft.referencePixel(i))*sdft.increment(i) + sdft.referenceValue(i);
   }
//
   Vector<Float> JyPerK = sdAtt.JyPerK(inst, dateObs, freqs);
   ostringstream oss;
   oss << "Jy/K = " << JyPerK;
   pushLog(String(oss));
   Vector<Float> factors = cFac * JyPerK;
   if (toKelvin) factors = Float(1.0) / factors;

// Get data slice bounds

   IPosition start, end;
   setCursorSlice (start, end, doAll, in);
   const uInt ifAxis = in.getIF();

// Iteration axes

   IPosition axes(asap::nAxes-1,0);
   for (uInt i=0,j=0; i<asap::nAxes; i++) {
      if (i!=asap::IFAxis) {
         axes(j++) = i;
      }
   }

// Loop over rows and apply correction factor

   Float factor = 1.0;
   const uInt axis = asap::ChanAxis;
   for (uInt i=0; i < in.nRow(); ++i) {

// Get data

      MaskedArray<Float> dataIn = in.rowAsMaskedArray(i);
      Array<Float>& values = dataIn.getRWArray();           // Ref to dataIn
      Array<Float> values2 = values(start,end);             // Ref to values to dataIn

// Get SDCOntainer

      SDContainer sc = in.getSDContainer(i);

// Get FreqIDs

      freqIDs = sc.getFreqMap();

// Now the conversion factor depends only upon frequency
// So we need to iterate through by IF only giving
// us BEAM/POL/CHAN cubes

      ArrayIterator<Float> itIn(values2, axes);
      uInt ax = 0;
      while (!itIn.pastEnd()) {
        itIn.array() *= factors(freqIDs(ax));         // Writes back to dataIn
        itIn.next();
      }

// Write out

      putDataInSDC(sc, dataIn.getArray(), dataIn.getMask());
//
      pTabOut->putSDContainer(sc);
   }
}



SDMemTable* SDMath::frequencyAlign(const SDMemTable& in,
                                   MFrequency::Types freqSystem,
                                   const String& refTime,
                                   const String& methodStr,
                                   Bool perFreqID)
{
// Get Header

   SDHeader sh = in.getSDHeader();
   const uInt nChan = sh.nchan;
   const uInt nRows = in.nRow();
   const uInt nIF = sh.nif;

// Get Table reference

   const Table& tabIn = in.table();

// Get Columns from Table

   ROScalarColumn<Double> mjdCol(tabIn, "TIME");
   ROScalarColumn<String> srcCol(tabIn, "SRCNAME");
   ROArrayColumn<uInt> fqIDCol(tabIn, "FREQID");
   Vector<Double> times = mjdCol.getColumn();

// Generate DataDesc table

   Matrix<uInt> ddIdx;
   SDDataDesc dDesc;
   generateDataDescTable(ddIdx, dDesc, nIF, in, tabIn, srcCol,
                          fqIDCol, perFreqID);

// Get reference Epoch to time of first row or given String

   Unit DAY(String("d"));
   MEpoch::Ref epochRef(in.getTimeReference());
   MEpoch refEpoch;
   if (refTime.length()>0) {
     refEpoch = epochFromString(refTime, in.getTimeReference());
   } else {
     refEpoch = in.getEpoch(0);
   }
   ostringstream oss;
   oss << "Aligned at reference Epoch " << formatEpoch(refEpoch) << " in frame " << MFrequency::showType(freqSystem);
   pushLog(String(oss));

   // Get Reference Position

   MPosition refPos = in.getAntennaPosition();

   // Create FrequencyAligner Block. One FA for each possible
   // source/freqID (perFreqID=True) or source/IF (perFreqID=False)
   // combination

   PtrBlock<FrequencyAligner<Float>* > a(dDesc.length());
   generateFrequencyAligners(a, dDesc, in, nChan, freqSystem, refPos,
                              refEpoch, perFreqID);

   // Generate and fill output Frequency Table.  WHen perFreqID=True,
   // there is one output FreqID for each entry in the SDDataDesc
   // table.  However, in perFreqID=False mode, there may be some
   // degeneracy, so we need a little translation map

   SDFrequencyTable freqTabOut = in.getSDFreqTable();
   freqTabOut.setLength(0);
   Vector<String> units(1);
   units = String("Hz");
   Bool linear=True;
   //
   Vector<uInt> ddFQTrans(dDesc.length(),0);
   for (uInt i=0; i<dDesc.length(); i++) {

     // Get Aligned SC in Hz

     SpectralCoordinate sC = a[i]->alignedSpectralCoordinate(linear);
     sC.setWorldAxisUnits(units);

     // Add FreqID

     uInt idx = freqTabOut.addFrequency(sC.referencePixel()[0],
                                        sC.referenceValue()[0],
                                        sC.increment()[0]);
     // output FreqID = ddFQTrans(ddIdx)
     ddFQTrans(i) = idx;
   }

   // Interpolation method

   InterpolateArray1D<Double,Float>::InterpolationMethod interp;
   convertInterpString(interp, methodStr);

   // New output Table

   //pushLog("Create output table");
   SDMemTable* pTabOut = new SDMemTable(in,True);
   pTabOut->putSDFreqTable(freqTabOut);

   // Loop over rows in Table

   Bool extrapolate=False;
   const IPosition polChanAxes(2, asap::PolAxis, asap::ChanAxis);
   Bool useCachedAbcissa = False;
   Bool first = True;
   Bool ok;
   Vector<Float> yOut;
   Vector<Bool> maskOut;
   Vector<uInt> freqID(nIF);
   uInt ifIdx, faIdx;
   Vector<Double> xIn;
   //
   for (uInt iRow=0; iRow<nRows; ++iRow) {
//      if (iRow%10==0) {
//        ostringstream oss;
//        oss << "Processing row " << iRow;
//        pushLog(String(oss));
//      }

     // Get EPoch

     Quantum<Double> tQ2(times[iRow],DAY);
     MVEpoch mv2(tQ2);
     MEpoch epoch(mv2, epochRef);

     // Get copy of data

     const MaskedArray<Float>& mArrIn(in.rowAsMaskedArray(iRow));
     Array<Float> values = mArrIn.getArray();
     Array<Bool> mask = mArrIn.getMask();

     // For each row, the Frequency abcissa will be the same
     // regardless of polarization.  For all other axes (IF and BEAM)
     // the abcissa will change.  So we iterate through the data by
     // pol-chan planes to mimimize the work.  Probably won't work for
     // multiple beams at this point.

     ArrayIterator<Float> itValuesPlane(values, polChanAxes);
     ArrayIterator<Bool> itMaskPlane(mask, polChanAxes);
     while (!itValuesPlane.pastEnd()) {

       // Find the IF index and then the FA PtrBlock index

       const IPosition& pos = itValuesPlane.pos();
       ifIdx = pos(asap::IFAxis);
       faIdx = ddIdx(iRow,ifIdx);

       // Generate abcissa for perIF.  Could cache this in a Matrix on
       // a per scan basis.  Pretty expensive doing it for every row.

       if (!perFreqID) {
         xIn.resize(nChan);
         uInt fqID = dDesc.secID(ddIdx(iRow,ifIdx));
         SpectralCoordinate sC = in.getSpectralCoordinate(fqID);
           Double w;
           for (uInt i=0; i<nChan; i++) {
             sC.toWorld(w,Double(i));
              xIn[i] = w;
           }
       }

       VectorIterator<Float> itValuesVec(itValuesPlane.array(), 1);
       VectorIterator<Bool> itMaskVec(itMaskPlane.array(), 1);

       // Iterate through the plane by vector and align

        first = True;
        useCachedAbcissa=False;
        while (!itValuesVec.pastEnd()) {
          if (perFreqID) {
            ok = a[faIdx]->align (yOut, maskOut, itValuesVec.vector(),
                                  itMaskVec.vector(), epoch, useCachedAbcissa,
                                  interp, extrapolate);
          } else {
            ok = a[faIdx]->align (yOut, maskOut, xIn, itValuesVec.vector(),
                                  itMaskVec.vector(), epoch, useCachedAbcissa,
                                  interp, extrapolate);
          }
          //
          itValuesVec.vector() = yOut;
          itMaskVec.vector() = maskOut;
          //
          itValuesVec.next();
          itMaskVec.next();
          //
          if (first) {
            useCachedAbcissa = True;
            first = False;
          }
        }
        //
        itValuesPlane.next();
        itMaskPlane.next();
     }

     // Create SDContainer and put back

     SDContainer sc = in.getSDContainer(iRow);
     putDataInSDC(sc, values, mask);

     // Set output FreqIDs

     for (uInt i=0; i<nIF; i++) {
       uInt idx = ddIdx(iRow,i);               // Index into SDDataDesc table
       freqID(i) = ddFQTrans(idx);             // FreqID in output FQ table
     }
     sc.putFreqMap(freqID);
     //
     pTabOut->putSDContainer(sc);
   }

   // Now we must set the base and extra frames to the input frame
   std::vector<string> info = pTabOut->getCoordInfo();
   info[1] = MFrequency::showType(freqSystem);   // Conversion frame
   info[3] = info[1];                            // Base frame
   pTabOut->setCoordInfo(info);

   // Clean up PointerBlock
   for (uInt i=0; i<a.nelements(); i++) delete a[i];

   return pTabOut;
}


SDMemTable* SDMath::frequencySwitch(const SDMemTable& in)
{
  if (in.nIF() != 2) {
    throw(AipsError("nIF != 2 "));
  }
  Bool clear = True;
  SDMemTable* pTabOut = new SDMemTable(in, clear);
  const Table& tabIn = in.table();

  // Shape of input and output data
  const IPosition& shapeIn = in.rowAsMaskedArray(0).shape();

  const uInt nRows = in.nRow();
  AlwaysAssert(asap::nAxes==4,AipsError);

  ROArrayColumn<Float> tSysCol;
  Array<Float> tsys;
  tSysCol.attach(tabIn,"TSYS");

  for (uInt iRow=0; iRow<nRows; iRow++) {
    // Get data for this row
    MaskedArray<Float> marr(in.rowAsMaskedArray(iRow));
    tSysCol.get(iRow, tsys);

    // whole Array for IF 0
    IPosition start(asap::nAxes,0);
    IPosition end = shapeIn-1;
    end(asap::IFAxis) = 0;

    MaskedArray<Float> on = marr(start,end);
    Array<Float> ton = tsys(start,end);
    // Make a copy as "src" is a refrence which is manipulated.
    // oncopy is needed for the inverse quotient
    MaskedArray<Float> oncopy = on.copy();

    // whole Array for IF 1
    start(asap::IFAxis) = 1;
    end(asap::IFAxis) = 1;

    MaskedArray<Float> off = marr(start,end);
    Array<Float> toff = tsys(start,end);

    on /= off; on -= 1.0f;
    on *= ton;
    off /= oncopy; off -= 1.0f;
    off *= toff;

    SDContainer sc = in.getSDContainer(iRow);
    putDataInSDC(sc, marr.getArray(), marr.getMask());
    pTabOut->putSDContainer(sc);
  }
  return pTabOut;
}

void SDMath::fillSDC(SDContainer& sc,
                     const Array<Bool>& mask,
                     const Array<Float>& data,
                     const Array<Float>& tSys,
                     Int scanID, Double timeStamp,
                     Double interval, const String& sourceName,
                     const Vector<uInt>& freqID)
{
// Data and mask

  putDataInSDC(sc, data, mask);

// TSys

  sc.putTsys(tSys);

// Time things

  sc.timestamp = timeStamp;
  sc.interval = interval;
  sc.scanid = scanID;
//
  sc.sourcename = sourceName;
  sc.putFreqMap(freqID);
}

void SDMath::accumulate(Double& timeSum, Double& intSum, Int& nAccum,
                        MaskedArray<Float>& sum, Array<Float>& sumSq,
                        Array<Float>& nPts, Array<Float>& tSysSum,
                        Array<Float>& tSysSqSum,
                        const Array<Float>& tSys, const Array<Float>& nInc,
                        const Vector<Bool>& mask, Double time, Double interval,
                        const std::vector<CountedPtr<SDMemTable> >& in,
                        uInt iTab, uInt iRow, uInt axis,
                        uInt nAxesSub, Bool useMask,
                        WeightType wtType)
{

// Get data

   MaskedArray<Float> dataIn(in[iTab]->rowAsMaskedArray(iRow));
   Array<Float>& valuesIn = dataIn.getRWArray();           // writable reference
   const Array<Bool>& maskIn = dataIn.getMask();          // RO reference
//
   if (wtType==NONE) {
      const MaskedArray<Float> n(nInc,dataIn.getMask());
      nPts += n;                               // Only accumulates where mask==T
   } else if (wtType==TINT) {

// We are weighting the data by integration time.

     valuesIn *= Float(interval);

   } else if (wtType==VAR) {

// We are going to average the data, weighted by the noise for each pol, beam and IF.
// So therefore we need to iterate through by spectrum (axis 3)

      VectorIterator<Float> itData(valuesIn, axis);
      ReadOnlyVectorIterator<Bool> itMask(maskIn, axis);
      Float fac = 1.0;
      IPosition pos(nAxesSub,0);
//
      while (!itData.pastEnd()) {

// Make MaskedArray of Vector, optionally apply OTF mask, and find scaling factor

         if (useMask) {
            MaskedArray<Float> tmp(itData.vector(),mask&&itMask.vector());
            fac = 1.0/variance(tmp);
         } else {
            MaskedArray<Float> tmp(itData.vector(),itMask.vector());
            fac = 1.0/variance(tmp);
         }

// Scale data

         itData.vector() *= fac;     // Writes back into 'dataIn'
//
// Accumulate variance per if/pol/beam averaged over spectrum
// This method to get pos2 from itData.pos() is only valid
// because the spectral axis is the last one (so we can just
// copy the first nAXesSub positions out)

         pos = itData.pos().getFirst(nAxesSub);
         sumSq(pos) += fac;
//
         itData.next();
         itMask.next();
      }
   } else if (wtType==TSYS || wtType==TINTSYS) {

// We are going to average the data, weighted by 1/Tsys**2 for each pol, beam and IF.
// So therefore we need to iterate through by spectrum (axis 3).  Although
// Tsys is stored as a vector of length nChan, the values are replicated.
// We will take a short cut and just use the value from the first channel
// for now.
//
      VectorIterator<Float> itData(valuesIn, axis);
      ReadOnlyVectorIterator<Float> itTSys(tSys, axis);
      IPosition pos(nAxesSub,0);
//
      Float fac = 1.0;
      if (wtType==TINTSYS) fac *= interval;
      while (!itData.pastEnd()) {
         Float t = itTSys.vector()[0];
         fac *= 1.0/t/t;

// Scale data

         itData.vector() *= fac;     // Writes back into 'dataIn'
//
// Accumulate Tsys  per if/pol/beam averaged over spectrum
// This method to get pos2 from itData.pos() is only valid
// because the spectral axis is the last one (so we can just
// copy the first nAXesSub positions out)

         pos = itData.pos().getFirst(nAxesSub);
         tSysSqSum(pos) += fac;
//
         itData.next();
         itTSys.next();
      }
   }

// Accumulate sum of (possibly scaled) data

   sum += dataIn;

// Accumulate Tsys, time, and interval

   tSysSum += tSys;
   timeSum += time;
   intSum += interval;
   nAccum += 1;
}


void SDMath::normalize(MaskedArray<Float>& sum,
                       const Array<Float>& sumSq,
                       const Array<Float>& tSysSqSum,
                       const Array<Float>& nPts,
                       Double intSum,
                       WeightType wtType, Int axis,
                       Int nAxesSub)
{
   IPosition pos2(nAxesSub,0);
//
   if (wtType==NONE) {

// We just average by the number of points accumulated.
// We need to make a MA out of nPts so that no divide by
// zeros occur

      MaskedArray<Float> t(nPts, (nPts>Float(0.0)));
      sum /= t;
   } else if (wtType==TINT) {

// Average by sum of Tint

      sum /= Float(intSum);
   } else if (wtType==VAR) {

// Normalize each spectrum by sum(1/var) where the variance
// is worked out for each spectrum

      Array<Float>& data = sum.getRWArray();
      VectorIterator<Float> itData(data, axis);
      while (!itData.pastEnd()) {
         pos2 = itData.pos().getFirst(nAxesSub);
         itData.vector() /= sumSq(pos2);
         itData.next();
      }
   } else if (wtType==TSYS || wtType==TINTSYS) {

// Normalize each spectrum by sum(1/Tsys**2) (TSYS) or
// sum(Tint/Tsys**2) (TINTSYS) where the pseudo
// replication over channel for Tsys has been dropped.

      Array<Float>& data = sum.getRWArray();
      VectorIterator<Float> itData(data, axis);
      while (!itData.pastEnd()) {
         pos2 = itData.pos().getFirst(nAxesSub);
         itData.vector() /= tSysSqSum(pos2);
         itData.next();
      }
   }
}




void SDMath::setCursorSlice (IPosition& start, IPosition& end, Bool doAll, const SDMemTable& in) const
{
  const uInt nDim = asap::nAxes;
  DebugAssert(nDim==4,AipsError);
//
  start.resize(nDim);
  end.resize(nDim);
  if (doAll) {
     start = 0;
     end(0) = in.nBeam()-1;
     end(1) = in.nIF()-1;
     end(2) = in.nPol()-1;
     end(3) = in.nChan()-1;
  } else {
     start(0) = in.getBeam();
     end(0) = start(0);
//
     start(1) = in.getIF();
     end(1) = start(1);
//
     start(2) = in.getPol();
     end(2) = start(2);
//
     start(3) = 0;
     end(3) = in.nChan()-1;
   }
}


void SDMath::convertWeightString(WeightType& wtType, const String& weightStr,
                                 Bool listType)
{
  String tStr(weightStr);
  tStr.upcase();
  String msg;
  if (tStr.contains(String("NONE"))) {
     wtType = NONE;
     msg = String("Weighting type selected : None");
  } else if (tStr.contains(String("VAR"))) {
     wtType = VAR;
     msg = String("Weighting type selected : Variance");
  } else if (tStr.contains(String("TINTSYS"))) {
       wtType = TINTSYS;
       msg = String("Weighting type selected : Tint&Tsys");
  } else if (tStr.contains(String("TINT"))) {
     wtType = TINT;
     msg = String("Weighting type selected : Tint");
  } else if (tStr.contains(String("TSYS"))) {
     wtType = TSYS;
     msg = String("Weighting type selected : Tsys");
  } else {
     msg = String("Weighting type selected : None");
     throw(AipsError("Unrecognized weighting type"));
  }
//
  if (listType) pushLog(msg);
}


void SDMath::convertInterpString(casa::InterpolateArray1D<Double,Float>::InterpolationMethod& type,
                                 const casa::String& interp)
{
  String tStr(interp);
  tStr.upcase();
  if (tStr.contains(String("NEAR"))) {
     type = InterpolateArray1D<Double,Float>::nearestNeighbour;
  } else if (tStr.contains(String("LIN"))) {
     type = InterpolateArray1D<Double,Float>::linear;
  } else if (tStr.contains(String("CUB"))) {
     type = InterpolateArray1D<Double,Float>::cubic;
  } else if (tStr.contains(String("SPL"))) {
     type = InterpolateArray1D<Double,Float>::spline;
  } else {
    throw(AipsError("Unrecognized interpolation type"));
  }
}

void SDMath::putDataInSDC(SDContainer& sc, const Array<Float>& data,
                          const Array<Bool>& mask)
{
    sc.putSpectrum(data);
//
    Array<uChar> outflags(data.shape());
    convertArray(outflags,!mask);
    sc.putFlags(outflags);
}

Table SDMath::readAsciiFile(const String& fileName) const
{
   String formatString;
   Table tbl = readAsciiTable (formatString, Table::Memory, fileName, "", "", False);
   return tbl;
}



void SDMath::scaleFromAsciiTable(SDMemTable* pTabOut,
                                 const SDMemTable& in, const String& fileName,
                                 const String& col0, const String& col1,
                                 const String& methodStr, Bool doAll,
                                 const Vector<Float>& xOut, Bool doTSys)
{

// Read gain-elevation ascii file data into a Table.

  Table geTable = readAsciiFile (fileName);
//
  scaleFromTable (pTabOut, in, geTable, col0, col1, methodStr, doAll, xOut, doTSys);
}

void SDMath::scaleFromTable(SDMemTable* pTabOut, const SDMemTable& in,
                            const Table& tTable, const String& col0,
                            const String& col1,
                            const String& methodStr, Bool doAll,
                            const Vector<Float>& xOut, Bool doTsys)
{

// Get data from Table

  ROScalarColumn<Float> geElCol(tTable, col0);
  ROScalarColumn<Float> geFacCol(tTable, col1);
  Vector<Float> xIn = geElCol.getColumn();
  Vector<Float> yIn = geFacCol.getColumn();
  Vector<Bool> maskIn(xIn.nelements(),True);

// Interpolate (and extrapolate) with desired method

   InterpolateArray1D<Double,Float>::InterpolationMethod method;
   convertInterpString(method, methodStr);
   Int intMethod(method);
//
   Vector<Float> yOut;
   Vector<Bool> maskOut;
   InterpolateArray1D<Float,Float>::interpolate(yOut, maskOut, xOut,
                                                xIn, yIn, maskIn, intMethod,
                                                True, True);
// Apply

   scaleByVector(pTabOut, in, doAll, Float(1.0)/yOut, doTsys);
}


void SDMath::scaleByVector(SDMemTable* pTabOut, const SDMemTable& in,
                           Bool doAll, const Vector<Float>& factor,
                           Bool doTSys)
{

// Set up data slice

  IPosition start, end;
  setCursorSlice (start, end, doAll, in);

// Get Tsys column

  const Table& tIn = in.table();
  ArrayColumn<Float> tSysCol(tIn, "TSYS");
  Array<Float> tSys;

// Loop over rows and apply correction factor

  const uInt axis = asap::ChanAxis;
  for (uInt i=0; i < in.nRow(); ++i) {

// Get data

     MaskedArray<Float> dataIn(in.rowAsMaskedArray(i));
     MaskedArray<Float> dataIn2 = dataIn(start,end);  // reference to dataIn
//
     if (doTSys) {
        tSysCol.get(i, tSys);
        Array<Float> tSys2 = tSys(start,end) * factor[i];
        tSysCol.put(i, tSys);
     }

// Apply factor

     dataIn2 *= factor[i];

// Write out

     SDContainer sc = in.getSDContainer(i);
     putDataInSDC(sc, dataIn.getArray(), dataIn.getMask());
//
     pTabOut->putSDContainer(sc);
  }
}




void SDMath::generateDataDescTable (Matrix<uInt>& ddIdx,
                                    SDDataDesc& dDesc,
                                    uInt nIF,
                                    const SDMemTable& in,
                                    const Table& tabIn,
                                    const ROScalarColumn<String>& srcCol,
                                    const ROArrayColumn<uInt>& fqIDCol,
                                    Bool perFreqID)
{
   const uInt nRows = tabIn.nrow();
   ddIdx.resize(nRows,nIF);
//
   String srcName;
   Vector<uInt> freqIDs;
   for (uInt iRow=0; iRow<nRows; iRow++) {
      srcCol.get(iRow, srcName);
      fqIDCol.get(iRow, freqIDs);
      const MDirection& dir = in.getDirection(iRow);
//
      if (perFreqID) {

// One entry per source/freqID pair

         for (uInt iIF=0; iIF<nIF; iIF++) {
            ddIdx(iRow,iIF) = dDesc.addEntry(srcName, freqIDs[iIF], dir, 0);
         }
      } else {

// One entry per source/IF pair.  Hang onto the FreqID as well

         for (uInt iIF=0; iIF<nIF; iIF++) {
            ddIdx(iRow,iIF) = dDesc.addEntry(srcName, iIF, dir, freqIDs[iIF]);
         }
      }
   }
}





MEpoch SDMath::epochFromString(const String& str, MEpoch::Types timeRef)
{
   Quantum<Double> qt;
   if (MVTime::read(qt,str)) {
      MVEpoch mv(qt);
      MEpoch me(mv, timeRef);
      return me;
   } else {
      throw(AipsError("Invalid format for Epoch string"));
   }
}


String SDMath::formatEpoch(const MEpoch& epoch)  const
{
   MVTime mvt(epoch.getValue());
   return mvt.string(MVTime::YMD) + String(" (") + epoch.getRefString() + String(")");
}



void SDMath::generateFrequencyAligners(PtrBlock<FrequencyAligner<Float>* >& a,
                                       const SDDataDesc& dDesc,
                                       const SDMemTable& in, uInt nChan,
                                       MFrequency::Types system,
                                       const MPosition& refPos,
                                       const MEpoch& refEpoch,
                                       Bool perFreqID)
{
   for (uInt i=0; i<dDesc.length(); i++) {
      uInt ID = dDesc.ID(i);
      uInt secID = dDesc.secID(i);
      const MDirection& refDir = dDesc.secDir(i);
//
      if (perFreqID) {

// One aligner per source/FreqID pair.

         SpectralCoordinate sC = in.getSpectralCoordinate(ID);
         a[i] = new FrequencyAligner<Float>(sC, nChan, refEpoch, refDir, refPos, system);
      } else {

// One aligner per source/IF pair.  But we still need the FreqID to
// get the right SC.  Hence the messing about with the secondary ID

         SpectralCoordinate sC = in.getSpectralCoordinate(secID);
         a[i] = new FrequencyAligner<Float>(sC, nChan, refEpoch, refDir, refPos, system);
      }
   }
}

Vector<uInt> SDMath::getRowRange(const SDMemTable& in) const
{
   Vector<uInt> range(2);
   range[0] = 0;
   range[1] = in.nRow()-1;
   return range;
}


Bool SDMath::rowInRange(uInt i, const Vector<uInt>& range) const
{
   return (i>=range[0] && i<=range[1]);
}

