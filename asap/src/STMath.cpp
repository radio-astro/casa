//
// C++ Implementation: STMath
//
// Description:
//
//
// Author: Malte Marquarding <asap@atnf.csiro.au>, (C) 2006
//
// Copyright: See COPYING file that comes with this distribution
//
//

#include <sstream>

#include <casa/iomanip.h>
#include <casa/Arrays/MaskArrLogi.h>
#include <casa/Arrays/MaskArrMath.h>
#include <casa/Arrays/ArrayLogical.h>
#include <casa/Arrays/ArrayMath.h>
#include <casa/Arrays/Slice.h>
#include <casa/Arrays/Slicer.h>
#include <casa/BasicSL/String.h>
#include <casa/Containers/Block.h>
#include <casa/Containers/RecordField.h>
#include <casa/Exceptions/Error.h>
#include <casa/Logging/LogIO.h>
#include <casa/Quanta/Quantum.h>

#include <coordinates/Coordinates/CoordinateSystem.h>
#include <coordinates/Coordinates/CoordinateUtil.h>
#include <coordinates/Coordinates/FrequencyAligner.h>
#include <coordinates/Coordinates/SpectralCoordinate.h>

#include <lattices/Lattices/LatticeUtilities.h>

#include <scimath/Functionals/Polynomial.h>
#include <scimath/Mathematics/Convolver.h>
#include <scimath/Mathematics/VectorKernel.h>

#include <tables/Tables/ExprNode.h>
#include <tables/Tables/ReadAsciiTable.h>
#include <tables/Tables/TableCopy.h>
#include <tables/Tables/TableIter.h>
#include <tables/Tables/TableParse.h>
#include <tables/Tables/TableRecord.h>
#include <tables/Tables/TableRow.h>
#include <tables/Tables/TableVector.h>
#include <tables/Tables/TabVecMath.h>

#include <atnf/PKSIO/SrcType.h>

#include "RowAccumulator.h"
#include "STAttr.h"
#include "STMath.h"
#include "STSelector.h"
#include "Accelerator.h"
#include "STIdxIter.h"

#include "CalibrationHelper.h"

using namespace casa;
using namespace asap;

// 2012/02/17 TN
// Since STGrid is implemented, average doesn't consider direction 
// when accumulating
// tolerance for direction comparison (rad)
// #define TOL_OTF    1.0e-15
// #define TOL_POINT  2.9088821e-4  // 1 arcmin

STMath::STMath(bool insitu) :
  insitu_(insitu)
{
}


STMath::~STMath()
{
}

CountedPtr<Scantable>
STMath::average( const std::vector<CountedPtr<Scantable> >& in,
                 const std::vector<bool>& mask,
                 const std::string& weight,
                 const std::string& avmode)
{
//    double t0, t1 ;
//    t0 = mathutil::gettimeofday_sec() ;

  LogIO os( LogOrigin( "STMath", "average()", WHERE ) ) ;
  if ( avmode == "SCAN" && in.size() != 1 )
    throw(AipsError("Can't perform 'SCAN' averaging on multiple tables.\n"
                    "Use merge first."));
  WeightType wtype = stringToWeight(weight);

  // 2012/02/17 TN
  // Since STGrid is implemented, average doesn't consider direction 
  // when accumulating
  // check if OTF observation
//   String obstype = in[0]->getHeader().obstype ;
//   Double tol = 0.0 ;
//   if ( (obstype.find( "OTF" ) != String::npos) || (obstype.find( "OBSERVE_TARGET" ) != String::npos) ) {
//     tol = TOL_OTF ;
//   }
//   else {
//     tol = TOL_POINT ;
//   }

  // output
  // clone as this is non insitu
  bool insitu = insitu_;
  setInsitu(false);
  CountedPtr< Scantable > out = getScantable(in[0], true);
  setInsitu(insitu);
  std::vector<CountedPtr<Scantable> >::const_iterator stit = in.begin();
  ++stit;
  while ( stit != in.end() ) {
    out->appendToHistoryTable((*stit)->history());
    ++stit;
  }

  Table& tout = out->table();

  /// @todo check if all scantables are conformant

  ArrayColumn<Float> specColOut(tout,"SPECTRA");
  ArrayColumn<uChar> flagColOut(tout,"FLAGTRA");
  ArrayColumn<Float> tsysColOut(tout,"TSYS");
  ScalarColumn<Double> mjdColOut(tout,"TIME");
  ScalarColumn<Double> intColOut(tout,"INTERVAL");
  ScalarColumn<uInt> cycColOut(tout,"CYCLENO");
  ScalarColumn<uInt> scanColOut(tout,"SCANNO");
  ScalarColumn<uInt> flagRowColOut(tout,"FLAGROW");

  // set up the output table rows. These are based on the structure of the
  // FIRST scantable in the vector
  const Table& baset = in[0]->table();

  RowAccumulator acc(wtype);
  Vector<Bool> cmask(mask);
  acc.setUserMask(cmask);
//   ROTableRow row(tout);
  ROArrayColumn<Float> specCol, tsysCol;
  ROArrayColumn<uChar> flagCol;
  ROScalarColumn<Double> mjdCol, intCol;
  ROScalarColumn<Int> scanIDCol;
  ROScalarColumn<uInt> flagRowCol;

  //Vector<uInt> rowstodelete;
  Block<uInt> rowstodelB( in[0]->nrow() ) ;
  uInt nrowdel = 0 ;

//   Block<String> cols(3);
  vector<string> cols(3) ;
  cols[0] = String("BEAMNO");
  cols[1] = String("IFNO");
  cols[2] = String("POLNO");
  if ( avmode == "SOURCE" ) {
    cols.resize(4);
    cols[3] = String("SRCNAME");
  }
  if ( avmode == "SCAN"  && in.size() == 1) {
    //cols.resize(4);
    //cols[3] = String("SCANNO");
    cols.resize(5);
    cols[3] = String("SRCNAME");
    cols[4] = String("SCANNO");
  }
  uInt outrowCount = 0;
  // use STIdxIter2 instead of TableIterator
  STIdxIter2 iter( in[0], cols ) ;
//   double t2 = 0 ;
//   double t3 = 0 ;
//   double t4 = 0 ;
//   double t5 = 0 ;
//   TableIterator iter(baset, cols);
//   int count = 0 ;
  while (!iter.pastEnd()) {
    Vector<uInt> rows = iter.getRows( SHARE ) ;
    if ( rows.nelements() == 0 ) {
      iter.next() ;
      continue ;
    }
    Record current = iter.currentValue() ;
    //Table subt = iter.table();
    // copy the first row of this selection into the new table
    tout.addRow();
//     t4 = mathutil::gettimeofday_sec() ;
    // skip to copy SPECTRA, FLAGTRA, and TSYS since those heavy columns are 
    // overwritten in the following process
    copyRows( tout, baset, outrowCount, rows[0], 1, False, False, False ) ;
//     t5 += mathutil::gettimeofday_sec() - t4 ;
    // re-index to 0
    if ( avmode != "SCAN" && avmode != "SOURCE" ) {
      scanColOut.put(outrowCount, uInt(0));
    }

    // 2012/02/17 TN
    // Since STGrid is implemented, average doesn't consider direction 
    // when accumulating
//     MDirection::ScalarColumn dircol ;
//     dircol.attach( subt, "DIRECTION" ) ;
//     Int length = subt.nrow() ;
//     vector< Vector<Double> > dirs ;
//     vector<int> indexes ;
//     for ( Int i = 0 ; i < length ; i++ ) {
//       Vector<Double> t = dircol(i).getAngle(Unit(String("rad"))).getValue() ;
//       //os << << count++ << ": " ;
//       //os << "[" << t[0] << "," << t[1] << "]" << LogIO::POST ;
//       bool adddir = true ;
//       for ( uInt j = 0 ; j < dirs.size() ; j++ ) {
//         //if ( allTrue( t == dirs[j] ) ) {
//         Double dx = t[0] - dirs[j][0] ;
//         Double dy = t[1] - dirs[j][1] ;
//         Double dd = sqrt( dx * dx + dy * dy ) ;
//         //if ( allNearAbs( t, dirs[j], tol ) ) {
//         if ( dd <= tol ) {
//           adddir = false ;
//           break ;
//         }
//       }
//       if ( adddir ) {
//         dirs.push_back( t ) ;
//         indexes.push_back( i ) ;
//       }
//     }
//     uInt rowNum = dirs.size() ;
//     tout.addRow( rowNum ) ;
//     for ( uInt i = 0 ; i < rowNum ; i++ ) {
//       TableCopy::copyRows( tout, subt, outrowCount+i, indexes[i], 1 ) ;
//       // re-index to 0
//       if ( avmode != "SCAN" && avmode != "SOURCE" ) {
//         scanColOut.put(outrowCount+i, uInt(0));
//       }        
//     }
//     outrowCount += rowNum ;

    // merge loop 
    uInt i = outrowCount ;
    // in[0] is already selected by iterator
    specCol.attach(baset,"SPECTRA");
    flagCol.attach(baset,"FLAGTRA");
    tsysCol.attach(baset,"TSYS");
    intCol.attach(baset,"INTERVAL");
    mjdCol.attach(baset,"TIME");
    flagRowCol.attach(baset,"FLAGROW");
    Vector<Float> spec,tsys;
    Vector<uChar> flag;
    Double inter,time;
    uInt flagRow;

    for (uInt l = 0; l < rows.nelements(); ++l ) {
      uInt k = rows[l] ;
      flagCol.get(k, flag);
      Vector<Bool> bflag(flag.shape());
      flagRowCol.get(k, flagRow);
      if (flagRow > 0)
	bflag = true;
      else
	convertArray(bflag, flag);
      /*                                                                                                   
        if ( allEQ(bflag, True) ) {                                                                          
        continue;//don't accumulate                                                                          
        }                                                                                                    
      */
      specCol.get(k, spec);
      tsysCol.get(k, tsys);
      intCol.get(k, inter);
      mjdCol.get(k, time);
      // spectrum has to be added last to enable weighting by the other values                             
//       t2 = mathutil::gettimeofday_sec() ;
      acc.add(spec, !bflag, tsys, inter, time);
//       t3 += mathutil::gettimeofday_sec() - t2 ;
      
    }


    // in[0] is already selected by TableIterator so that index is 
    // started from 1
    for ( int j=1; j < int(in.size()); ++j ) {
      const Table& tin = in[j]->table();
      //const TableRecord& rec = row.get(i);
      ROScalarColumn<Double> tmp(tin, "TIME");
      Double td;tmp.get(0,td);

#if 1
      static char const*const colNames1[] = { "IFNO", "BEAMNO", "POLNO" };
      //uInt const values1[] = { rec.asuInt("IFNO"), rec.asuInt("BEAMNO"), rec.asuInt("POLNO") };
      uInt const values1[] = { current.asuInt("IFNO"), current.asuInt("BEAMNO"), current.asuInt("POLNO") };
      SingleTypeEqPredicate<uInt, 3> myPred(tin, colNames1, values1);
      CustomTableExprNodeRep myNodeRep(tin, myPred);
      myNodeRep.link(); // to avoid automatic delete when myExpr is destructed.
      CustomTableExprNode myExpr(myNodeRep);
      Table basesubt = tin(myExpr);
#else
//       Table basesubt = tin( tin.col("BEAMNO") == Int(rec.asuInt("BEAMNO"))
//                          && tin.col("IFNO") == Int(rec.asuInt("IFNO"))
//                          && tin.col("POLNO") == Int(rec.asuInt("POLNO")) );
      Table basesubt = tin( tin.col("BEAMNO") == current.asuInt("BEAMNO")
			    && tin.col("IFNO") == current.asuInt("IFNO")
			    && tin.col("POLNO") == current.asuInt("POLNO") );
#endif
      Table subt;
      if ( avmode == "SOURCE") {
//         subt = basesubt( basesubt.col("SRCNAME") == rec.asString("SRCNAME"));
        subt = basesubt( basesubt.col("SRCNAME") == current.asString("SRCNAME") );

      } else if (avmode == "SCAN") {
//         subt = basesubt( basesubt.col("SRCNAME") == rec.asString("SRCNAME") 
// 		      && basesubt.col("SCANNO") == Int(rec.asuInt("SCANNO")) );
        subt = basesubt( basesubt.col("SRCNAME") == current.asString("SRCNAME")
			 && basesubt.col("SCANNO") == current.asuInt("SCANNO") );
      } else {
        subt = basesubt;
      }

      // 2012/02/17 TN
      // Since STGrid is implemented, average doesn't consider direction 
      // when accumulating
//       vector<uInt> removeRows ;
//       uInt nrsubt = subt.nrow() ;
//       for ( uInt irow = 0 ; irow < nrsubt ; irow++ ) {
//         //if ( !allTrue((subt.col("DIRECTION").getArrayDouble(TableExprId(irow)))==rec.asArrayDouble("DIRECTION")) ) {
//         Vector<Double> x0 = (subt.col("DIRECTION").getArrayDouble(TableExprId(irow))) ;
//         Vector<Double> x1 = rec.asArrayDouble("DIRECTION") ;
//         double dx = x0[0] - x1[0];
//         double dy = x0[1] - x1[1];
//         Double dd = sqrt( dx * dx + dy * dy ) ;
//         //if ( !allNearAbs((subt.col("DIRECTION").getArrayDouble(TableExprId(irow))), rec.asArrayDouble("DIRECTION"), tol ) ) {
//         if ( dd > tol ) {
//           removeRows.push_back( irow ) ;
//         }
//       }
//       if ( removeRows.size() != 0 ) {
//         subt.removeRow( removeRows ) ;
//       }
      
//       if ( nrsubt == removeRows.size() )
//         throw(AipsError("Averaging data is empty.")) ;

      specCol.attach(subt,"SPECTRA");
      flagCol.attach(subt,"FLAGTRA");
      tsysCol.attach(subt,"TSYS");
      intCol.attach(subt,"INTERVAL");
      mjdCol.attach(subt,"TIME");
      flagRowCol.attach(subt,"FLAGROW");
      for (uInt k = 0; k < subt.nrow(); ++k ) {
        flagCol.get(k, flag);
        Vector<Bool> bflag(flag.shape());
	flagRowCol.get(k, flagRow);
	if (flagRow > 0)
	  bflag = true;
	else
	  convertArray(bflag, flag);
	/*
        if ( allEQ(bflag, True) ) {
	continue;//don't accumulate
        }
	*/
        specCol.get(k, spec);
        //tsysCol.get(k, tsys);
        tsys.assign( tsysCol(k) );
        intCol.get(k, inter);
        mjdCol.get(k, time);
        // spectrum has to be added last to enable weighting by the other values
//         t2 = mathutil::gettimeofday_sec() ;
        acc.add(spec, !bflag, tsys, inter, time);
//         t3 += mathutil::gettimeofday_sec() - t2 ;
      }

    }
    const Vector<Bool>& msk = acc.getMask();
    if ( allEQ(msk, False) ) {
      rowstodelB[nrowdel] = i ;
      nrowdel++ ;
      continue;
    }
    //write out
    if (acc.state()) {
      // If there exists a channel at which all the input spectra are masked, 
      // spec has 'nan' values for that channel and it may affect the following 
      // processes. To avoid this, replacing 'nan' values in spec with 
      // weighted-mean of all spectra in the following line. 
      // (done for CAS-2776, 2011/04/07 by Wataru Kawasaki)
      acc.replaceNaN();

      Vector<uChar> flg(msk.shape());
      convertArray(flg, !msk);
      for (uInt k = 0; k < flg.nelements(); ++k) {
	uChar userFlag = 1 << 7;
	if (msk[k]==True) userFlag = 0 << 7;
	flg(k) = userFlag;
      }

      flagColOut.put(i, flg);
      specColOut.put(i, acc.getSpectrum());
      tsysColOut.put(i, acc.getTsys());
      intColOut.put(i, acc.getInterval());
      mjdColOut.put(i, acc.getTime());
      // we should only have one cycle now -> reset it to be 0
      // frequency switched data has different CYCLENO for different IFNO
      // which requires resetting this value
      cycColOut.put(i, uInt(0));
      // completely flagged rows are removed anyway
      flagRowColOut.put(i, uInt(0));
    } else {
      os << "For output row="<<i<<", all input rows of data are flagged. no averaging" << LogIO::POST;
    }
    acc.reset();

    // merge with while loop for preparing out table
    ++outrowCount;
//     ++iter ;
    iter.next() ;
  }

  if ( nrowdel > 0 ) {
    Vector<uInt> rowstodelete( IPosition(1,nrowdel), rowstodelB.storage(), SHARE ) ;
    os << rowstodelete << LogIO::POST ;
    tout.removeRow(rowstodelete);
    if (tout.nrow() == 0) {
      throw(AipsError("Can't average fully flagged data."));
    }
  }

//    t1 = mathutil::gettimeofday_sec() ;
//    cout << "elapsed time for average(): " << t1-t0 << " sec" << endl ;
//    cout << "   elapsed time for acc.add(): " << t3 << " sec" << endl ;
//    cout << "   elapsed time for copyRows(): " << t5 << " sec" << endl ;

  return out;
}

CountedPtr< Scantable >
STMath::averageChannel( const CountedPtr < Scantable > & in,
                          const std::string & mode,
                          const std::string& avmode )
{
  (void) mode; // currently unused
  // 2012/02/17 TN
  // Since STGrid is implemented, average doesn't consider direction 
  // when accumulating
  // check if OTF observation
//   String obstype = in->getHeader().obstype ;
//   Double tol = 0.0 ;
//   if ( obstype.find( "OTF" ) != String::npos ) {
//     tol = TOL_OTF ;
//   }
//   else {
//     tol = TOL_POINT ;
//   }

  // clone as this is non insitu
  bool insitu = insitu_;
  setInsitu(false);
  CountedPtr< Scantable > out = getScantable(in, true);
  setInsitu(insitu);
  Table& tout = out->table();
  ArrayColumn<Float> specColOut(tout,"SPECTRA");
  ArrayColumn<uChar> flagColOut(tout,"FLAGTRA");
  ArrayColumn<Float> tsysColOut(tout,"TSYS");
  ScalarColumn<uInt> scanColOut(tout,"SCANNO");
  ScalarColumn<Double> intColOut(tout, "INTERVAL");
  Table tmp = in->table().sort("BEAMNO");
  Block<String> cols(3);
  cols[0] = String("BEAMNO");
  cols[1] = String("IFNO");
  cols[2] = String("POLNO");
  if ( avmode == "SCAN") {
    cols.resize(4);
    cols[3] = String("SCANNO");
  }
  uInt outrowCount = 0;
  uChar userflag = 1 << 7;
  TableIterator iter(tmp, cols);
  while (!iter.pastEnd()) {
    Table subt = iter.table();
    ROArrayColumn<Float> specCol, tsysCol;
    ROArrayColumn<uChar> flagCol;
    ROScalarColumn<Double> intCol(subt, "INTERVAL");
    specCol.attach(subt,"SPECTRA");
    flagCol.attach(subt,"FLAGTRA");
    tsysCol.attach(subt,"TSYS");

    tout.addRow();
    TableCopy::copyRows(tout, subt, outrowCount, 0, 1);
    if ( avmode != "SCAN") {
      scanColOut.put(outrowCount, uInt(0));
    }
    Vector<Float> tmp;
    specCol.get(0, tmp);
    uInt nchan = tmp.nelements();
    // have to do channel by channel here as MaskedArrMath
    // doesn't have partialMedians
    Vector<uChar> flags = flagCol.getColumn(Slicer(Slice(0)));
    Vector<Float> outspec(nchan);
    Vector<uChar> outflag(nchan,0);
    Vector<Float> outtsys(1);/// @fixme when tsys is channel based
    for (uInt i=0; i<nchan; ++i) {
      Vector<Float> specs = specCol.getColumn(Slicer(Slice(i)));
      MaskedArray<Float> ma = maskedArray(specs,flags);
      outspec[i] = median(ma);
      if ( allEQ(ma.getMask(), False) )
        outflag[i] = userflag;// flag data
    }
    outtsys[0] = median(tsysCol.getColumn());
    specColOut.put(outrowCount, outspec);
    flagColOut.put(outrowCount, outflag);
    tsysColOut.put(outrowCount, outtsys);
    Double intsum = sum(intCol.getColumn());
    intColOut.put(outrowCount, intsum);
    ++outrowCount;
    ++iter;

    // 2012/02/17 TN
    // Since STGrid is implemented, average doesn't consider direction 
    // when accumulating
//     MDirection::ScalarColumn dircol ;
//     dircol.attach( subt, "DIRECTION" ) ;
//     Int length = subt.nrow() ;
//     vector< Vector<Double> > dirs ;
//     vector<int> indexes ;
//     // Handle MX mode averaging
//     if (in->nbeam() > 1 ) {      
//       length = 1;
//     }
//     for ( Int i = 0 ; i < length ; i++ ) {
//       Vector<Double> t = dircol(i).getAngle(Unit(String("rad"))).getValue() ;
//       bool adddir = true ;
//       for ( uInt j = 0 ; j < dirs.size() ; j++ ) {
//         //if ( allTrue( t == dirs[j] ) ) {
//         Double dx = t[0] - dirs[j][0] ;
//         Double dy = t[1] - dirs[j][1] ;
//         Double dd = sqrt( dx * dx + dy * dy ) ;
//         //if ( allNearAbs( t, dirs[j], tol ) ) {
//         if ( dd <= tol ) {
//           adddir = false ;
//           break ;
//         }
//       }
//       if ( adddir ) {
//         dirs.push_back( t ) ;
//         indexes.push_back( i ) ;
//       }
//     }
//     uInt rowNum = dirs.size() ;
//     tout.addRow( rowNum );
//     for ( uInt i = 0 ; i < rowNum ; i++ ) {
//       TableCopy::copyRows(tout, subt, outrowCount+i, indexes[i], 1) ;
//       // Handle MX mode averaging
//       if ( avmode != "SCAN") {
//         scanColOut.put(outrowCount+i, uInt(0));
//       }
//     }
//     MDirection::ScalarColumn dircolOut ;
//     dircolOut.attach( tout, "DIRECTION" ) ;
//     for ( uInt irow = 0 ; irow < rowNum ; irow++ ) {
//       Vector<Double> t = \
// 	dircolOut(outrowCount+irow).getAngle(Unit(String("rad"))).getValue() ;
//       Vector<Float> tmp;
//       specCol.get(0, tmp);
//       uInt nchan = tmp.nelements();
//       // have to do channel by channel here as MaskedArrMath
//       // doesn't have partialMedians
//       Vector<uChar> flags = flagCol.getColumn(Slicer(Slice(0)));
//       // mask spectra for different DIRECTION
//       for ( uInt jrow = 0 ; jrow < subt.nrow() ; jrow++ ) {
//         Vector<Double> direction = \
// 	  dircol(jrow).getAngle(Unit(String("rad"))).getValue() ;
//         //if ( t[0] != direction[0] || t[1] != direction[1] ) {
//         Double dx = t[0] - direction[0];
//         Double dy = t[1] - direction[1];
//         Double dd = sqrt(dx*dx + dy*dy);
//         //if ( !allNearAbs( t, direction, tol ) ) {
//         if ( dd > tol &&  in->nbeam() < 2 ) {
//           flags[jrow] = userflag ;
//         }
//       }
//       Vector<Float> outspec(nchan);
//       Vector<uChar> outflag(nchan,0);
//       Vector<Float> outtsys(1);/// @fixme when tsys is channel based
//       for (uInt i=0; i<nchan; ++i) {
//         Vector<Float> specs = specCol.getColumn(Slicer(Slice(i)));
//         MaskedArray<Float> ma = maskedArray(specs,flags);
//         outspec[i] = median(ma);
//         if ( allEQ(ma.getMask(), False) )
//           outflag[i] = userflag;// flag data
//       }
//       outtsys[0] = median(tsysCol.getColumn());
//       specColOut.put(outrowCount+irow, outspec);
//       flagColOut.put(outrowCount+irow, outflag);
//       tsysColOut.put(outrowCount+irow, outtsys);
//       Vector<Double> integ = intCol.getColumn() ;
//       MaskedArray<Double> mi = maskedArray( integ, flags ) ;
//       Double intsum = sum(mi);
//       intColOut.put(outrowCount+irow, intsum);
//     }
//     outrowCount += rowNum ;
//     ++iter;
  }
  return out;
}

CountedPtr< Scantable > STMath::getScantable(const CountedPtr< Scantable >& in,
                                             bool droprows)
{
  if (insitu_) {
    return in;
  }
  else {
    // clone
    return CountedPtr<Scantable>(new Scantable(*in, Bool(droprows)));
  }
}

CountedPtr< Scantable > STMath::unaryOperate( const CountedPtr< Scantable >& in,
                                              float val,
                                              const std::string& mode,
                                              bool tsys )
{
  CountedPtr< Scantable > out = getScantable(in, false);
  Table& tab = out->table();
  ArrayColumn<Float> specCol(tab,"SPECTRA");
  ArrayColumn<Float> tsysCol(tab,"TSYS");
  if (mode=="DIV") val = 1.0/val ;
  else if (mode=="SUB") val *= -1.0 ;
  for (uInt i=0; i<tab.nrow(); ++i) {
    Vector<Float> spec;
    Vector<Float> ts;
    specCol.get(i, spec);
    tsysCol.get(i, ts);
    if (mode == "MUL" || mode == "DIV") {
      //if (mode == "DIV") val = 1.0/val;
      spec *= val;
      specCol.put(i, spec);
      if ( tsys ) {
        ts *= val;
        tsysCol.put(i, ts);
      }
    } else if ( mode == "ADD"  || mode == "SUB") {
      //if (mode == "SUB") val *= -1.0;
      spec += val;
      specCol.put(i, spec);
      if ( tsys ) {
        ts += val;
        tsysCol.put(i, ts);
      }
    }
  }
  return out;
}

CountedPtr< Scantable > STMath::arrayOperate( const CountedPtr< Scantable >& in,
                                              const std::vector<float> val,
                                              const std::string& mode,
                                              const std::string& opmode, 
                                              bool tsys )
{
  CountedPtr< Scantable > out ;
  if ( opmode == "channel" ) {
    out = arrayOperateChannel( in, val, mode, tsys ) ;
  }
  else if ( opmode == "row" ) {
    out = arrayOperateRow( in, val, mode, tsys ) ;
  }
  else {
    throw( AipsError( "Unknown array operation mode." ) ) ;
  }
  return out ;
}

CountedPtr< Scantable > STMath::arrayOperateChannel( const CountedPtr< Scantable >& in,
                                                     const std::vector<float> val,
                                                     const std::string& mode,
                                                     bool tsys )
{
  if ( val.size() == 1 ){
    return unaryOperate( in, val[0], mode, tsys ) ;
  }

  // conformity of SPECTRA and TSYS
  if ( tsys ) {
    TableIterator titer(in->table(), "IFNO");
    while ( !titer.pastEnd() ) {
      ArrayColumn<Float> specCol( in->table(), "SPECTRA" ) ;
      ArrayColumn<Float> tsysCol( in->table(), "TSYS" ) ;
      Array<Float> spec = specCol.getColumn() ;
      Array<Float> ts = tsysCol.getColumn() ;
      if ( !spec.conform( ts ) ) {
        throw( AipsError( "SPECTRA and TSYS must conform in shape if you want to apply operation on Tsys." ) ) ;
      }
      titer.next() ;
    }
  }

  // check if all spectra in the scantable have the same number of channel
  vector<uInt> nchans;
  vector<uInt> ifnos = in->getIFNos() ;
  for ( uInt i = 0 ; i < ifnos.size() ; i++ ) {
    nchans.push_back( in->nchan( ifnos[i] ) ) ;
  }
  Vector<uInt> mchans( nchans ) ;
  if ( anyNE( mchans, mchans[0] ) ) {
    throw( AipsError("All spectra in the input scantable must have the same number of channel for vector operation." ) ) ;
  }

  // check if vector size is equal to nchan
  Vector<Float> fact( val ) ;
  if ( fact.nelements() != mchans[0] ) {
    throw( AipsError("Vector size must be 1 or be same as number of channel.") ) ;
  }

  // check divided by zero
  if ( ( mode == "DIV" ) && anyEQ( fact, (float)0.0 ) ) {
    throw( AipsError("Divided by zero is not recommended." ) ) ;
  }

  CountedPtr< Scantable > out = getScantable(in, false);
  Table& tab = out->table();
  ArrayColumn<Float> specCol(tab,"SPECTRA");
  ArrayColumn<Float> tsysCol(tab,"TSYS");
  if (mode == "DIV") fact = (float)1.0 / fact;
  else if (mode == "SUB") fact *= (float)-1.0 ;
  for (uInt i=0; i<tab.nrow(); ++i) {
    Vector<Float> spec;
    Vector<Float> ts;
    specCol.get(i, spec);
    tsysCol.get(i, ts);
    if (mode == "MUL" || mode == "DIV") {
      //if (mode == "DIV") fact = (float)1.0 / fact;
      spec *= fact;
      specCol.put(i, spec);
      if ( tsys ) {
        ts *= fact;
        tsysCol.put(i, ts);
      }
    } else if ( mode == "ADD"  || mode == "SUB") {
      //if (mode == "SUB") fact *= (float)-1.0 ;
      spec += fact;
      specCol.put(i, spec);
      if ( tsys ) {
        ts += fact;
        tsysCol.put(i, ts);
      }
    }
  }
  return out;
}

CountedPtr< Scantable > STMath::arrayOperateRow( const CountedPtr< Scantable >& in,
                                                 const std::vector<float> val,
                                                 const std::string& mode,
                                                 bool tsys )
{
  if ( val.size() == 1 ) {
    return unaryOperate( in, val[0], mode, tsys ) ;
  }

  // conformity of SPECTRA and TSYS
  if ( tsys ) {
    TableIterator titer(in->table(), "IFNO");
    while ( !titer.pastEnd() ) {
      ArrayColumn<Float> specCol( in->table(), "SPECTRA" ) ;
      ArrayColumn<Float> tsysCol( in->table(), "TSYS" ) ;
      Array<Float> spec = specCol.getColumn() ;
      Array<Float> ts = tsysCol.getColumn() ;
      if ( !spec.conform( ts ) ) {
        throw( AipsError( "SPECTRA and TSYS must conform in shape if you want to apply operation on Tsys." ) ) ;
      }
      titer.next() ;
    }
  }

  // check if vector size is equal to nrow
  Vector<Float> fact( val ) ;
  if (fact.nelements() != uInt(in->nrow())) {
    throw( AipsError("Vector size must be 1 or be same as number of row.") ) ;
  }

  // check divided by zero
  if ( ( mode == "DIV" ) && anyEQ( fact, (float)0.0 ) ) {
    throw( AipsError("Divided by zero is not recommended." ) ) ;
  }

  CountedPtr< Scantable > out = getScantable(in, false);
  Table& tab = out->table();
  ArrayColumn<Float> specCol(tab,"SPECTRA");
  ArrayColumn<Float> tsysCol(tab,"TSYS");
  if (mode == "DIV") fact = (float)1.0 / fact;
  if (mode == "SUB") fact *= (float)-1.0 ;
  for (uInt i=0; i<tab.nrow(); ++i) {
    Vector<Float> spec;
    Vector<Float> ts;
    specCol.get(i, spec);
    tsysCol.get(i, ts);
    if (mode == "MUL" || mode == "DIV") {
      spec *= fact[i];
      specCol.put(i, spec);
      if ( tsys ) {
        ts *= fact[i];
        tsysCol.put(i, ts);
      }
    } else if ( mode == "ADD"  || mode == "SUB") {
      spec += fact[i];
      specCol.put(i, spec);
      if ( tsys ) {
        ts += fact[i];
        tsysCol.put(i, ts);
      }
    }
  }
  return out;
}

CountedPtr< Scantable > STMath::array2dOperate( const CountedPtr< Scantable >& in,
                                                const std::vector< std::vector<float> > val,
                                                const std::string& mode,
                                                bool tsys )
{
  // conformity of SPECTRA and TSYS
  if ( tsys ) {
    TableIterator titer(in->table(), "IFNO");
    while ( !titer.pastEnd() ) {
      ArrayColumn<Float> specCol( in->table(), "SPECTRA" ) ;
      ArrayColumn<Float> tsysCol( in->table(), "TSYS" ) ;
      Array<Float> spec = specCol.getColumn() ;
      Array<Float> ts = tsysCol.getColumn() ;
      if ( !spec.conform( ts ) ) {
        throw( AipsError( "SPECTRA and TSYS must conform in shape if you want to apply operation on Tsys." ) ) ;
      }
      titer.next() ;
    }
  }

  // some checks 
  vector<uInt> nchans;
  for (Int i = 0 ; i < in->nrow() ; i++) {
    nchans.push_back((in->getSpectrum(i)).size());
  }
  //Vector<uInt> mchans( nchans ) ;
  vector< Vector<Float> > facts ;
  for ( uInt i = 0 ; i < nchans.size() ; i++ ) {
    Vector<Float> tmp( val[i] ) ;
    // check divided by zero
    if ( ( mode == "DIV" ) && anyEQ( tmp, (float)0.0 ) ) {
      throw( AipsError("Divided by zero is not recommended." ) ) ;
    }
    // conformity check
    if ( tmp.nelements() != nchans[i] ) {
      stringstream ss ;
      ss << "Row " << i << ": Vector size must be same as number of channel." ;
      throw( AipsError( ss.str() ) ) ;
    }
    facts.push_back( tmp ) ;
  }


  CountedPtr< Scantable > out = getScantable(in, false);
  Table& tab = out->table();
  ArrayColumn<Float> specCol(tab,"SPECTRA");
  ArrayColumn<Float> tsysCol(tab,"TSYS");
  for (uInt i=0; i<tab.nrow(); ++i) {
    Vector<Float> fact = facts[i] ;
    Vector<Float> spec;
    Vector<Float> ts;
    specCol.get(i, spec);
    tsysCol.get(i, ts);
    if (mode == "MUL" || mode == "DIV") {
      if (mode == "DIV") fact = (float)1.0 / fact;
      spec *= fact;
      specCol.put(i, spec);
      if ( tsys ) {
        ts *= fact;
        tsysCol.put(i, ts);
      }
    } else if ( mode == "ADD"  || mode == "SUB") {
      if (mode == "SUB") fact *= (float)-1.0 ;
      spec += fact;
      specCol.put(i, spec);
      if ( tsys ) {
        ts += fact;
        tsysCol.put(i, ts);
      }
    }
  }
  return out;
}

CountedPtr<Scantable> STMath::binaryOperate(const CountedPtr<Scantable>& left,
					    const CountedPtr<Scantable>& right,
					    const std::string& mode)
{
  bool insitu = insitu_;
  if ( ! left->conformant(*right) ) {
    throw(AipsError("'left' and 'right' scantables are not conformant."));
  }
  setInsitu(false);
  CountedPtr< Scantable > out = getScantable(left, false);
  setInsitu(insitu);
  Table& tout = out->table();
  Block<String> coln(5);
  coln[0] = "SCANNO";  coln[1] = "CYCLENO";  coln[2] = "BEAMNO";
  coln[3] = "IFNO";  coln[4] = "POLNO";
  Table tmpl = tout.sort(coln);
  Table tmpr = right->table().sort(coln);
  ArrayColumn<Float> lspecCol(tmpl,"SPECTRA");
  ROArrayColumn<Float> rspecCol(tmpr,"SPECTRA");
  ArrayColumn<uChar> lflagCol(tmpl,"FLAGTRA");
  ROArrayColumn<uChar> rflagCol(tmpr,"FLAGTRA");

  for (uInt i=0; i<tout.nrow(); ++i) {
    Vector<Float> lspecvec, rspecvec;
    Vector<uChar> lflagvec, rflagvec;
    lspecvec = lspecCol(i);    rspecvec = rspecCol(i);
    lflagvec = lflagCol(i);    rflagvec = rflagCol(i);
    MaskedArray<Float> mleft = maskedArray(lspecvec, lflagvec);
    MaskedArray<Float> mright = maskedArray(rspecvec, rflagvec);
    if (mode == "ADD") {
      mleft += mright;
    } else if ( mode == "SUB") {
      mleft -= mright;
    } else if ( mode == "MUL") {
      mleft *= mright;
    } else if ( mode == "DIV") {
      mleft /= mright;
    } else {
      throw(AipsError("Illegal binary operator"));
    }
    lspecCol.put(i, mleft.getArray());
  }
  return out;
}



MaskedArray<Float> STMath::maskedArray( const Vector<Float>& s,
                                        const Vector<uChar>& f)
{
  Vector<Bool> mask;
  mask.resize(f.shape());
  convertArray(mask, f);
  return MaskedArray<Float>(s,!mask);
}

MaskedArray<Double> STMath::maskedArray( const Vector<Double>& s,
                                         const Vector<uChar>& f)
{
  Vector<Bool> mask;
  mask.resize(f.shape());
  convertArray(mask, f);
  return MaskedArray<Double>(s,!mask);
}

Vector<uChar> STMath::flagsFromMA(const MaskedArray<Float>& ma)
{
  const Vector<Bool>& m = ma.getMask();
  Vector<uChar> flags(m.shape());
  convertArray(flags, !m);
  return flags;
}

CountedPtr< Scantable > STMath::autoQuotient( const CountedPtr< Scantable >& in,
                                              const std::string & mode,
                                              bool preserve )
{
  /// @todo make other modes available
  /// modes should be "nearest", "pair"
  // make this operation non insitu
  (void) mode; //currently unused
  const Table& tin = in->table();
  Table ons = tin(tin.col("SRCTYPE") == Int(SrcType::PSON));
  Table offs = tin(tin.col("SRCTYPE") == Int(SrcType::PSOFF));
  if ( offs.nrow() == 0 )
    throw(AipsError("No 'off' scans present."));
  // put all "on" scans into output table

  bool insitu = insitu_;
  setInsitu(false);
  CountedPtr< Scantable > out = getScantable(in, true);
  setInsitu(insitu);
  Table& tout = out->table();

  TableCopy::copyRows(tout, ons);
  TableRow row(tout);
  ROScalarColumn<Double> offtimeCol(offs, "TIME");
  ArrayColumn<Float> outspecCol(tout, "SPECTRA");
  ROArrayColumn<Float> outtsysCol(tout, "TSYS");
  ArrayColumn<uChar> outflagCol(tout, "FLAGTRA");
  for (uInt i=0; i < tout.nrow(); ++i) {
    const TableRecord& rec = row.get(i);
    Double ontime = rec.asDouble("TIME");
    Table presel = offs(offs.col("BEAMNO") == Int(rec.asuInt("BEAMNO"))
			&& offs.col("IFNO") == Int(rec.asuInt("IFNO"))
			&& offs.col("POLNO") == Int(rec.asuInt("POLNO")) );
    ROScalarColumn<Double> offtimeCol(presel, "TIME");

    Double mindeltat = min(abs(offtimeCol.getColumn() - ontime));
    // Timestamp may vary within a cycle ???!!!
    // increase this by 0.01 sec in case of rounding errors...
    // There might be a better way to do this.
    // fix to this fix. TIME is MJD, so 1.0d not 1.0s
    mindeltat += 0.01/24./60./60.;
    Table sel = presel( abs(presel.col("TIME")-ontime) <= mindeltat);

    if ( sel.nrow() < 1 )  {
      throw(AipsError("No closest in time found... This could be a rounding "
                      "issue. Try quotient instead."));
    }
    TableRow offrow(sel);
    const TableRecord& offrec = offrow.get(0);//should only be one row
    RORecordFieldPtr< Array<Float> > specoff(offrec, "SPECTRA");
    RORecordFieldPtr< Array<Float> > tsysoff(offrec, "TSYS");
    RORecordFieldPtr< Array<uChar> > flagoff(offrec, "FLAGTRA");
    /// @fixme this assumes tsys is a scalar not vector
    Float tsysoffscalar = (*tsysoff)(IPosition(1,0));
    Vector<Float> specon, tsyson;
    outtsysCol.get(i, tsyson);
    outspecCol.get(i, specon);
    Vector<uChar> flagon;
    outflagCol.get(i, flagon);
    MaskedArray<Float> mon = maskedArray(specon, flagon);
    MaskedArray<Float> moff = maskedArray(*specoff, *flagoff);
    MaskedArray<Float> quot = (tsysoffscalar * mon / moff);
    if (preserve) {
      quot -= tsysoffscalar;
    } else {
      quot -= tsyson[0];
    }
    outspecCol.put(i, quot.getArray());
    outflagCol.put(i, flagsFromMA(quot));
  }
  // renumber scanno
  TableIterator it(tout, "SCANNO");
  uInt i = 0;
  while ( !it.pastEnd() ) {
    Table t = it.table();
    TableVector<uInt> vec(t, "SCANNO");
    vec = i;
    ++i;
    ++it;
  }
  return out;
}


CountedPtr< Scantable > STMath::quotient( const CountedPtr< Scantable > & on,
                                          const CountedPtr< Scantable > & off,
                                          bool preserve )
{
  bool insitu = insitu_;
  if ( ! on->conformant(*off) ) {
    throw(AipsError("'on' and 'off' scantables are not conformant."));
  }
  setInsitu(false);
  CountedPtr< Scantable > out = getScantable(on, false);
  setInsitu(insitu);
  Table& tout = out->table();
  const Table& toff = off->table();
  TableIterator sit(tout, "SCANNO");
  TableIterator s2it(toff, "SCANNO");
  while ( !sit.pastEnd() ) {
    Table ton = sit.table();
    TableRow row(ton);
    Table t = s2it.table();
    ArrayColumn<Float> outspecCol(ton, "SPECTRA");
    ROArrayColumn<Float> outtsysCol(ton, "TSYS");
    ArrayColumn<uChar> outflagCol(ton, "FLAGTRA");
    for (uInt i=0; i < ton.nrow(); ++i) {
      const TableRecord& rec = row.get(i);
      Table offsel = t( t.col("BEAMNO") == Int(rec.asuInt("BEAMNO"))
                          && t.col("IFNO") == Int(rec.asuInt("IFNO"))
                          && t.col("POLNO") == Int(rec.asuInt("POLNO")) );
      if ( offsel.nrow() == 0 )
        throw AipsError("STMath::quotient: no matching off");
      TableRow offrow(offsel);
      const TableRecord& offrec = offrow.get(0);//should be ncycles - take first
      RORecordFieldPtr< Array<Float> > specoff(offrec, "SPECTRA");
      RORecordFieldPtr< Array<Float> > tsysoff(offrec, "TSYS");
      RORecordFieldPtr< Array<uChar> > flagoff(offrec, "FLAGTRA");
      Float tsysoffscalar = (*tsysoff)(IPosition(1,0));
      Vector<Float> specon, tsyson;
      outtsysCol.get(i, tsyson);
      outspecCol.get(i, specon);
      Vector<uChar> flagon;
      outflagCol.get(i, flagon);
      MaskedArray<Float> mon = maskedArray(specon, flagon);
      MaskedArray<Float> moff = maskedArray(*specoff, *flagoff);
      MaskedArray<Float> quot = (tsysoffscalar * mon / moff);
      if (preserve) {
        quot -= tsysoffscalar;
      } else {
        quot -= tsyson[0];
      }
      outspecCol.put(i, quot.getArray());
      outflagCol.put(i, flagsFromMA(quot));
    }
    ++sit;
    ++s2it;
    // take the first off for each on scan which doesn't have a
    // matching off scan
    // non <= noff:  matching pairs, non > noff matching pairs then first off
    if ( s2it.pastEnd() ) s2it.reset();
  }
  return out;
}

// dototalpower (migration of GBTIDL procedure dototalpower.pro)
// calibrate the CAL on-off pair. It calculate Tsys and average CAL on-off subintegrations
// do it for each cycles in a specific scan.
CountedPtr< Scantable > STMath::dototalpower( const CountedPtr< Scantable >& calon,
                                              const CountedPtr< Scantable >& caloff, Float tcal )
{
  if ( ! calon->conformant(*caloff) ) {
    throw(AipsError("'CAL on' and 'CAL off' scantables are not conformant."));
  }
  setInsitu(false);
  CountedPtr< Scantable > out = getScantable(caloff, false);
  Table& tout = out->table();
  const Table& tcon = calon->table();
  Vector<Float> tcalout;

  std::map<uInt,uInt> tcalIdToRecNoMap;
  const Table& calOffTcalTable = caloff->tcal().table();
  {
    ROScalarColumn<uInt> calOffTcalTable_IDcol(calOffTcalTable, "ID");
    const Vector<uInt> tcalIds(calOffTcalTable_IDcol.getColumn());
    size_t tcalIdsEnd = tcalIds.nelements();
    for (uInt i = 0; i < tcalIdsEnd; i++) {
      tcalIdToRecNoMap[tcalIds[i]] = i;
    }
  }
  ROArrayColumn<Float> calOffTcalTable_TCALcol(calOffTcalTable, "TCAL");

  if ( tout.nrow() != tcon.nrow() ) {
    throw(AipsError("Mismatch in number of rows to form cal on - off pair."));
  }
  // iteration by scanno or cycle no.
  TableIterator sit(tout, "SCANNO");
  TableIterator s2it(tcon, "SCANNO");
  while ( !sit.pastEnd() ) {
    Table toff = sit.table();
    TableRow row(toff);
    Table t = s2it.table();
    ScalarColumn<Double> outintCol(toff, "INTERVAL");
    ArrayColumn<Float> outspecCol(toff, "SPECTRA");
    ArrayColumn<Float> outtsysCol(toff, "TSYS");
    ArrayColumn<uChar> outflagCol(toff, "FLAGTRA");
    ROScalarColumn<uInt> outtcalIdCol(toff, "TCAL_ID");
    ROScalarColumn<uInt> outpolCol(toff, "POLNO");
    ROScalarColumn<Double> onintCol(t, "INTERVAL");
    ROArrayColumn<Float> onspecCol(t, "SPECTRA");
    ROArrayColumn<Float> ontsysCol(t, "TSYS");
    ROArrayColumn<uChar> onflagCol(t, "FLAGTRA");
    //ROScalarColumn<uInt> ontcalIdCol(t, "TCAL_ID");

    for (uInt i=0; i < toff.nrow(); ++i) {
      //skip these checks -> assumes the data order are the same between the cal on off pairs
      //
      Vector<Float> specCalon, specCaloff;
      // to store scalar (mean) tsys
      Vector<Float> tsysout(1);
      uInt tcalId, polno;
      Double offint, onint;
      outpolCol.get(i, polno);
      outspecCol.get(i, specCaloff);
      onspecCol.get(i, specCalon);
      Vector<uChar> flagCaloff, flagCalon;
      outflagCol.get(i, flagCaloff);
      onflagCol.get(i, flagCalon);
      outtcalIdCol.get(i, tcalId);
      outintCol.get(i, offint);
      onintCol.get(i, onint);
      // caluculate mean Tsys
      uInt nchan = specCaloff.nelements();
      // percentage of edge cut off
      uInt pc = 10;
      uInt bchan = nchan/pc;
      uInt echan = nchan-bchan;

      Slicer chansl(IPosition(1,bchan-1), IPosition(1,echan-1), IPosition(1,1),Slicer::endIsLast);
      Vector<Float> testsubsp = specCaloff(chansl);
      MaskedArray<Float> spoff = maskedArray( specCaloff(chansl),flagCaloff(chansl) );
      MaskedArray<Float> spon = maskedArray( specCalon(chansl),flagCalon(chansl) );
      MaskedArray<Float> spdiff = spon-spoff;
      uInt noff = spoff.nelementsValid();
      //uInt non = spon.nelementsValid();
      uInt ndiff = spdiff.nelementsValid();
      Float meantsys;

/**
      Double subspec, subdiff;
      uInt usednchan;
      subspec = 0;
      subdiff = 0;
      usednchan = 0;
      for(uInt k=(bchan-1); k<echan; k++) {
        subspec += specCaloff[k];
        subdiff += static_cast<Double>(specCalon[k]-specCaloff[k]);
        ++usednchan;
      }
**/
      // get tcal if input tcal <= 0
      Float tcalUsed;
      tcalUsed = tcal;
      if ( tcal <= 0.0 ) {
	uInt tcalRecNo = tcalIdToRecNoMap[tcalId];
	calOffTcalTable_TCALcol.get(tcalRecNo, tcalout);
//         if (polno<=3) {
//           tcalUsed = tcalout[polno];
//         }
//         else {
//           tcalUsed = tcalout[0];
//         }
        if ( tcalout.size() == 1 )
          tcalUsed = tcalout[0] ;
        else if ( tcalout.size() == nchan ) 
          tcalUsed = mean(tcalout) ;
        else {
          uInt ipol = polno ;
          if ( ipol > 3 ) ipol = 0 ;
          tcalUsed = tcalout[ipol] ;
        }
      }

      Float meanoff;
      Float meandiff;
      if (noff && ndiff) {
         //Debug
         //if(noff!=ndiff) cerr<<"noff and ndiff is not equal"<<endl;
         //LogIO os( LogOrigin( "STMath", "dototalpower()", WHERE ) ) ; 
         //if(noff!=ndiff) os<<"noff and ndiff is not equal"<<LogIO::POST;
         meanoff = sum(spoff)/noff;
         meandiff = sum(spdiff)/ndiff;
         meantsys= (meanoff/meandiff )*tcalUsed + tcalUsed/2;
      }
      else {
         meantsys=1;
      }

      tsysout[0] = Float(meantsys);
      MaskedArray<Float> mcaloff = maskedArray(specCaloff, flagCaloff);
      MaskedArray<Float> mcalon = maskedArray(specCalon, flagCalon);
      MaskedArray<Float> sig =   Float(0.5) * (mcaloff + mcalon);
      //uInt ncaloff = mcaloff.nelementsValid();
      //uInt ncalon = mcalon.nelementsValid();

      outintCol.put(i, offint+onint);
      outspecCol.put(i, sig.getArray());
      outflagCol.put(i, flagsFromMA(sig));
      outtsysCol.put(i, tsysout);
    }
    ++sit;
    ++s2it;
  }
  return out;
}

//dosigref - migrated from GBT IDL's dosigref.pro, do calibration of position switch
// observatiions.
// input: sig and ref scantables, and an optional boxcar smoothing width(default width=0,
//        no smoothing).
// output: resultant scantable [= (sig-ref/ref)*tsys]
CountedPtr< Scantable > STMath::dosigref( const CountedPtr < Scantable >& sig,
                                          const CountedPtr < Scantable >& ref,
                                          int smoothref,
                                          casa::Float tsysv,
                                          casa::Float tau )
{
  LogIO os( casa::LogOrigin( "STMath", "dosigref()"));
if ( ! ref->conformant(*sig) ) {
    throw(AipsError("'sig' and 'ref' scantables are not conformant."));
  }
  setInsitu(false);
  CountedPtr< Scantable > out = getScantable(sig, false);
  CountedPtr< Scantable > smref;
  if ( smoothref > 1 ) {
    float fsmoothref = static_cast<float>(smoothref);
    std::string inkernel = "boxcar";
    smref = smooth(ref, inkernel, fsmoothref );
    ostringstream oss;
    os <<"Applied smoothing of "<<fsmoothref<<" on the reference."
       << LogIO::POST;
  }
  else {
    smref = ref;
  }
  Table& tout = out->table();
  const Table& tref = smref->table();
  if ( tout.nrow() != tref.nrow() ) {
    throw(AipsError("Mismatch in number of rows to form on-source and reference pair."));
  }
  // iteration by scanno? or cycle no.
  TableIterator sit(tout, "SCANNO");
  TableIterator s2it(tref, "SCANNO");
  while ( !sit.pastEnd() ) {
    Table ton = sit.table();
    Table t = s2it.table();
    ScalarColumn<Double> outintCol(ton, "INTERVAL");
    ArrayColumn<Float> outspecCol(ton, "SPECTRA");
    ArrayColumn<Float> outtsysCol(ton, "TSYS");
    ArrayColumn<uChar> outflagCol(ton, "FLAGTRA");
    ArrayColumn<Float> refspecCol(t, "SPECTRA");
    ROScalarColumn<Double> refintCol(t, "INTERVAL");
    ROArrayColumn<Float> reftsysCol(t, "TSYS");
    ArrayColumn<uChar> refflagCol(t, "FLAGTRA");
    ROScalarColumn<Float> refelevCol(t, "ELEVATION");
    for (uInt i=0; i < ton.nrow(); ++i) {

      Double onint, refint;
      Vector<Float> specon, specref;
      // to store scalar (mean) tsys
      Vector<Float> tsysref;
      outintCol.get(i, onint);
      refintCol.get(i, refint);
      outspecCol.get(i, specon);
      refspecCol.get(i, specref);
      Vector<uChar> flagref, flagon;
      outflagCol.get(i, flagon);
      refflagCol.get(i, flagref);
      reftsysCol.get(i, tsysref);

      Float tsysrefscalar;
      if ( tsysv > 0.0 ) {
        ostringstream oss;
        Float elev;
        refelevCol.get(i, elev);
        os << "user specified Tsys = " << tsysv;
        // do recalc elevation if EL = 0
        if ( elev == 0 ) {
          throw(AipsError("EL=0, elevation data is missing."));
        } else {
          if ( tau <= 0.0 ) {
            throw(AipsError("Valid tau is not supplied."));
          } else {
            tsysrefscalar = tsysv * exp(tau/elev);
          }
        }
        os << ", corrected (for El) tsys= "<<tsysrefscalar;
      }
      else {
        tsysrefscalar = tsysref[0];
      }
      //get quotient spectrum
      MaskedArray<Float> mref = maskedArray(specref, flagref);
      MaskedArray<Float> mon = maskedArray(specon, flagon);
      MaskedArray<Float> specres =   tsysrefscalar*((mon - mref)/mref);
      Double resint = onint*refint*smoothref/(onint+refint*smoothref);

      //Debug
      //cerr<<"Tsys used="<<tsysrefscalar<<endl;
      //LogIO os( LogOrigin( "STMath", "dosigref", WHERE ) ) ;
      //os<<"Tsys used="<<tsysrefscalar<<LogIO::POST;
      // fill the result, replay signal tsys by reference tsys
      outintCol.put(i, resint);
      outspecCol.put(i, specres.getArray());
      outflagCol.put(i, flagsFromMA(specres));
      outtsysCol.put(i, tsysref);
    }
    ++sit;
    ++s2it;
  }
  
  out->setFluxUnit("K");

  return out;
}

CountedPtr< Scantable > STMath::donod(const casa::CountedPtr<Scantable>& s,
                                     const std::vector<int>& scans,
                                     int smoothref,
                                     casa::Float tsysv,
                                     casa::Float tau,
                                     casa::Float tcal )

{
  setInsitu(false);
  LogIO os( casa::LogOrigin( "STMath", "donod()"));
  STSelector sel;
  std::vector<int> scan1, scan2, beams, types;
  std::vector< vector<int> > scanpair;
  //std::vector<string> calstate;
  std::vector<int> calstate;
  String msg;

  CountedPtr< Scantable > s1b1on, s1b1off, s1b2on, s1b2off;
  CountedPtr< Scantable > s2b1on, s2b1off, s2b2on, s2b2off;

  std::vector< CountedPtr< Scantable > > sctables;
  sctables.push_back(s1b1on);
  sctables.push_back(s1b1off);
  sctables.push_back(s1b2on);
  sctables.push_back(s1b2off);
  sctables.push_back(s2b1on);
  sctables.push_back(s2b1off);
  sctables.push_back(s2b2on);
  sctables.push_back(s2b2off);

  //check scanlist
  int n=s->checkScanInfo(scans);
  if (n==1) {
     throw(AipsError("Incorrect scan pairs. "));
  }

  // Assume scans contain only a pair of consecutive scan numbers.
  // It is assumed that first beam, b1,  is on target.
  // There is no check if the first beam is on or not.
  if ( scans.size()==1 ) {
    scan1.push_back(scans[0]);
    scan2.push_back(scans[0]+1);
  } else if ( scans.size()==2 ) {
   scan1.push_back(scans[0]);
   scan2.push_back(scans[1]);
  } else {
    if ( scans.size()%2 == 0 ) {
      for (uInt i=0; i<scans.size(); i++) {
        if (i%2 == 0) {
          scan1.push_back(scans[i]);
        }
        else {
          scan2.push_back(scans[i]);
        }
      }
    } else {
      throw(AipsError("Odd numbers of scans, cannot form pairs."));
    }
  }
  scanpair.push_back(scan1);
  scanpair.push_back(scan2);
  //calstate.push_back("*calon");
  //calstate.push_back("*[^calon]");
  calstate.push_back(SrcType::NODCAL);
  calstate.push_back(SrcType::NOD);
  CountedPtr< Scantable > ws = getScantable(s, false);
  uInt l=0;
  while ( l < sctables.size() ) {
    for (uInt i=0; i < 2; i++) {
      for (uInt j=0; j < 2; j++) {
        for (uInt k=0; k < 2; k++) {
          sel.reset();
          sel.setScans(scanpair[i]);
          //sel.setName(calstate[k]);
          types.clear();
          types.push_back(calstate[k]);
          sel.setTypes(types);
          beams.clear();
          beams.push_back(j);
          sel.setBeams(beams);
          ws->setSelection(sel);
          sctables[l]= getScantable(ws, false);
          l++;
        }
      }
    }
  }

  // replace here by splitData or getData functionality
  CountedPtr< Scantable > sig1;
  CountedPtr< Scantable > ref1;
  CountedPtr< Scantable > sig2;
  CountedPtr< Scantable > ref2;
  CountedPtr< Scantable > calb1;
  CountedPtr< Scantable > calb2;

  msg=String("Processing dototalpower for subset of the data");
  os << msg << LogIO::POST;
  // Debug for IRC CS data
  //float tcal1=7.0;
  //float tcal2=4.0;
  sig1 = dototalpower(sctables[0], sctables[1], tcal=tcal);
  ref1 = dototalpower(sctables[2], sctables[3], tcal=tcal);
  ref2 = dototalpower(sctables[4], sctables[5], tcal=tcal);
  sig2 = dototalpower(sctables[6], sctables[7], tcal=tcal);

  // correction of user-specified tsys for elevation here

  // dosigref calibration
  msg=String("Processing dosigref for subset of the data");
  os << msg  << endl;
  calb1=dosigref(sig1,ref2,smoothref,tsysv,tau);
  calb2=dosigref(sig2,ref1,smoothref,tsysv,tau);

  // iteration by scanno or cycle no.
  Table& tcalb1 = calb1->table();
  Table& tcalb2 = calb2->table();
  TableIterator sit(tcalb1, "SCANNO");
  TableIterator s2it(tcalb2, "SCANNO");
  while ( !sit.pastEnd() ) {
    Table t1 = sit.table();
    Table t2= s2it.table();
    ArrayColumn<Float> outspecCol(t1, "SPECTRA");
    ArrayColumn<Float> outtsysCol(t1, "TSYS");
    ArrayColumn<uChar> outflagCol(t1, "FLAGTRA");
    ScalarColumn<Double> outintCol(t1, "INTERVAL");
    ArrayColumn<Float> t2specCol(t2, "SPECTRA");
    ROArrayColumn<Float> t2tsysCol(t2, "TSYS");
    ArrayColumn<uChar> t2flagCol(t2, "FLAGTRA");
    ROScalarColumn<Double> t2intCol(t2, "INTERVAL");
    for (uInt i=0; i < t1.nrow(); ++i) {
      Vector<Float> spec1, spec2;
      // to store scalar (mean) tsys
      Vector<Float> tsys1, tsys2;
      Vector<uChar> flag1, flag2;
      Double tint1, tint2;
      outspecCol.get(i, spec1);
      t2specCol.get(i, spec2);
      outflagCol.get(i, flag1);
      t2flagCol.get(i, flag2);
      outtsysCol.get(i, tsys1);
      t2tsysCol.get(i, tsys2);
      outintCol.get(i, tint1);
      t2intCol.get(i, tint2);
      // average
      // assume scalar tsys for weights
      Float wt1, wt2, tsyssq1, tsyssq2;
      tsyssq1 = tsys1[0]*tsys1[0];
      tsyssq2 = tsys2[0]*tsys2[0];
      wt1 = Float(tint1)/tsyssq1;
      wt2 = Float(tint2)/tsyssq2;
      Float invsumwt=1/(wt1+wt2);
      MaskedArray<Float> mspec1 = maskedArray(spec1, flag1);
      MaskedArray<Float> mspec2 = maskedArray(spec2, flag2);
      MaskedArray<Float> avspec =  invsumwt * (wt1*mspec1 + wt2*mspec2);
      //Array<Float> avtsys =  Float(0.5) * (tsys1 + tsys2);
      // cerr<< "Tsys1="<<tsys1<<" Tsys2="<<tsys2<<endl;
      // LogIO os( LogOrigin( "STMath", "donod", WHERE ) ) ;
      // os<< "Tsys1="<<tsys1<<" Tsys2="<<tsys2<<LogIO::POST;
      tsys1[0] = sqrt(tsyssq1 + tsyssq2);
      Array<Float> avtsys =  tsys1;

      outspecCol.put(i, avspec.getArray());
      outflagCol.put(i, flagsFromMA(avspec));
      outtsysCol.put(i, avtsys);
    }
    ++sit;
    ++s2it;
  }

  calb1->setFluxUnit("K");
  
  return calb1;
}

//GBTIDL version of frequency switched data calibration
CountedPtr< Scantable > STMath::dofs( const CountedPtr< Scantable >& s,
                                      const std::vector<int>& scans,
                                      int smoothref,
                                      casa::Float tsysv,
                                      casa::Float tau,
                                      casa::Float tcal )
{


  (void) scans; //currently unused
  STSelector sel;
  CountedPtr< Scantable > ws = getScantable(s, false);
  CountedPtr< Scantable > sig, sigwcal, ref, refwcal;
  CountedPtr< Scantable > calsig, calref, out, out1, out2;
  Bool nofold=False;
  vector<int> types ; 

  //split the data
  //sel.setName("*_fs");
  types.push_back( SrcType::FSON ) ;
  sel.setTypes( types ) ;
  ws->setSelection(sel);
  sig = getScantable(ws,false);
  sel.reset();
  types.clear() ;
  //sel.setName("*_fs_calon");
  types.push_back( SrcType::FONCAL ) ;
  sel.setTypes( types ) ;
  ws->setSelection(sel);
  sigwcal = getScantable(ws,false);
  sel.reset();
  types.clear() ;
  //sel.setName("*_fsr");
  types.push_back( SrcType::FSOFF ) ;
  sel.setTypes( types ) ;
  ws->setSelection(sel);
  ref = getScantable(ws,false);
  sel.reset();
  types.clear() ;
  //sel.setName("*_fsr_calon");
  types.push_back( SrcType::FOFFCAL ) ;
  sel.setTypes( types ) ;
  ws->setSelection(sel);
  refwcal = getScantable(ws,false);
  sel.reset() ;
  types.clear() ;

  calsig = dototalpower(sigwcal, sig, tcal=tcal);
  calref = dototalpower(refwcal, ref, tcal=tcal);

  out1=dosigref(calsig,calref,smoothref,tsysv,tau); 
  out2=dosigref(calref,calsig,smoothref,tsysv,tau); 

  Table& tabout1=out1->table();
  Table& tabout2=out2->table();
  ROScalarColumn<uInt> freqidCol1(tabout1, "FREQ_ID");
  ScalarColumn<uInt> freqidCol2(tabout2, "FREQ_ID");
  ROArrayColumn<Float> specCol(tabout2, "SPECTRA");
  Vector<Float> spec; specCol.get(0, spec);
  uInt nchan = spec.nelements();
  uInt freqid1; freqidCol1.get(0,freqid1);
  uInt freqid2; freqidCol2.get(0,freqid2);
  Double rp1, rp2, rv1, rv2, inc1, inc2;
  out1->frequencies().getEntry(rp1, rv1, inc1, freqid1);
  out2->frequencies().getEntry(rp2, rv2, inc2, freqid2);
  //cerr << out1->frequencies().table().nrow() << " " << out2->frequencies().table().nrow() << endl ;
  //LogIO os( LogOrigin( "STMath", "dofs()", WHERE ) ) ;
  //os << out1->frequencies().table().nrow() << " " << out2->frequencies().table().nrow() << LogIO::POST ;
  if (rp1==rp2) {
    Double foffset = rv1 - rv2;
    uInt choffset = static_cast<uInt>(foffset/abs(inc2));
    if (choffset >= nchan) {
      //cerr<<"out-band frequency switching, no folding"<<endl;
      LogIO os( LogOrigin( "STMath", "dofs()", WHERE ) ) ;
      os<<"out-band frequency switching, no folding"<<LogIO::POST;
      nofold = True;
    }
  }

  if (nofold) {
    std::vector< CountedPtr< Scantable > > tabs; 
    tabs.push_back(out1);
    tabs.push_back(out2);
    out = merge(tabs);
  }
  else {
    //out = out1;
    Double choffset = ( rv1 - rv2 ) / inc2 ;
    out = dofold( out1, out2, choffset ) ;
  }
   
  out->setFluxUnit("K");

  return out;
}

CountedPtr<Scantable> STMath::dofold( const CountedPtr<Scantable> &sig,
                                      const CountedPtr<Scantable> &ref,
                                      Double choffset,
                                      Double choffset2 )
{
  LogIO os( LogOrigin( "STMath", "dofold", WHERE ) ) ;
  os << "choffset=" << choffset << " choffset2=" << choffset2 << LogIO::POST ;

  // output scantable
  CountedPtr<Scantable> out = getScantable( sig, false ) ;

  // separate choffset to integer part and decimal part
  Int ioffset = (Int)choffset ;
  Double doffset = choffset - ioffset ;
  Int ioffset2 = (Int)choffset2 ;
  Double doffset2 = choffset2 - ioffset2 ;
  os << "ioffset=" << ioffset << " doffset=" << doffset << LogIO::POST ;
  os << "ioffset2=" << ioffset2 << " doffset2=" << doffset2 << LogIO::POST ;  

  // get column
  ROArrayColumn<Float> specCol1( sig->table(), "SPECTRA" ) ;
  ROArrayColumn<Float> specCol2( ref->table(), "SPECTRA" ) ;
  ROArrayColumn<Float> tsysCol1( sig->table(), "TSYS" ) ;
  ROArrayColumn<Float> tsysCol2( ref->table(), "TSYS" ) ;
  ROArrayColumn<uChar> flagCol1( sig->table(), "FLAGTRA" ) ;
  ROArrayColumn<uChar> flagCol2( ref->table(), "FLAGTRA" ) ;
  ROScalarColumn<Double> mjdCol1( sig->table(), "TIME" ) ;
  ROScalarColumn<Double> mjdCol2( ref->table(), "TIME" ) ;
  ROScalarColumn<Double> intervalCol1( sig->table(), "INTERVAL" ) ;
  ROScalarColumn<Double> intervalCol2( ref->table(), "INTERVAL" ) ;

  // check
  if ( ioffset == 0 ) {
    LogIO os( LogOrigin( "STMath", "dofold()", WHERE ) ) ;
    os << "channel offset is zero, no folding" << LogIO::POST ;
    return out ;
  }
  int nchan = ref->nchan() ;
  if ( abs(ioffset) >= nchan ) {
    LogIO os( LogOrigin( "STMath", "dofold()", WHERE ) ) ;
    os << "out-band frequency switching, no folding" << LogIO::POST ;
    return out ;
  }

  // attach column for output scantable
  ArrayColumn<Float> specColOut( out->table(), "SPECTRA" ) ;
  ArrayColumn<uChar> flagColOut( out->table(), "FLAGTRA" ) ;
  ArrayColumn<Float> tsysColOut( out->table(), "TSYS" ) ;
  ScalarColumn<Double> mjdColOut( out->table(), "TIME" ) ;
  ScalarColumn<Double> intervalColOut( out->table(), "INTERVAL" ) ;
  ScalarColumn<uInt> fidColOut( out->table(), "FREQ_ID" ) ;

  // for each row
  // assume that the data order are same between sig and ref
  RowAccumulator acc( asap::W_TINTSYS ) ; 
  for ( int i = 0 ; i < sig->nrow() ; i++ ) {
    // get values
    Vector<Float> spsig ;
    specCol1.get( i, spsig ) ;
    Vector<Float> spref ;
    specCol2.get( i, spref ) ;
    Vector<Float> tsyssig ;
    tsysCol1.get( i, tsyssig ) ;
    Vector<Float> tsysref ;
    tsysCol2.get( i, tsysref ) ;
    Vector<uChar> flagsig ;
    flagCol1.get( i, flagsig ) ;
    Vector<uChar> flagref ;
    flagCol2.get( i, flagref ) ;
    Double timesig ;
    mjdCol1.get( i, timesig ) ;
    Double timeref ;
    mjdCol2.get( i, timeref ) ;
    Double intsig ;
    intervalCol1.get( i, intsig ) ;
    Double intref ;
    intervalCol2.get( i, intref ) ;

    // shift reference spectra
    int refchan = spref.nelements() ;
    Vector<Float> sspref( spref.nelements() ) ;
    Vector<Float> stsysref( tsysref.nelements() ) ;
    Vector<uChar> sflagref( flagref.nelements() ) ;
    if ( ioffset > 0 ) {
      // SPECTRA and FLAGTRA
      for ( int j = 0 ; j < refchan-ioffset ; j++ ) {
        sspref[j] = spref[j+ioffset] ; 
        sflagref[j] = flagref[j+ioffset] ;
      }
      for ( int j = refchan-ioffset ; j < refchan ; j++ ) {
        sspref[j] = spref[j-refchan+ioffset] ;
        sflagref[j] = flagref[j-refchan+ioffset] ;
      }
      spref = sspref.copy() ;
      flagref = sflagref.copy() ;
      for ( int j = 0 ; j < refchan - 1 ; j++ ) {
        sspref[j] = doffset * spref[j+1] + ( 1.0 - doffset ) * spref[j] ;
        sflagref[j] = flagref[j+1] + flagref[j] ;
      }
      sspref[refchan-1] = doffset * spref[0] + ( 1.0 - doffset ) * spref[refchan-1] ;
      sflagref[refchan-1] = flagref[0] + flagref[refchan-1] ; 

      // TSYS
      if ( spref.nelements() == tsysref.nelements() ) {
        for ( int j = 0 ; j < refchan-ioffset ; j++ ) {
          stsysref[j] = tsysref[j+ioffset] ;
        }
        for ( int j = refchan-ioffset ; j < refchan ; j++ ) {
          stsysref[j] = tsysref[j-refchan+ioffset] ;
        }
        tsysref = stsysref.copy() ;
        for ( int j = 0 ; j < refchan - 1 ; j++ ) {
          stsysref[j] = doffset * tsysref[j+1] + ( 1.0 - doffset ) * tsysref[j] ;
        }
        stsysref[refchan-1] = doffset * tsysref[0] + ( 1.0 - doffset ) * tsysref[refchan-1] ;
      }
    }
    else {
      // SPECTRA and FLAGTRA
      for ( int j = 0 ; j < abs(ioffset) ; j++ ) {
        sspref[j] = spref[refchan+ioffset+j] ;
        sflagref[j] = flagref[refchan+ioffset+j] ;
      }
      for ( int j = abs(ioffset) ; j < refchan ; j++ ) {
        sspref[j] = spref[j+ioffset] ;
        sflagref[j] = flagref[j+ioffset] ;
      }
      spref = sspref.copy() ;
      flagref = sflagref.copy() ;
      sspref[0] = doffset * spref[refchan-1] + ( 1.0 - doffset ) * spref[0] ;
      sflagref[0] = flagref[0] + flagref[refchan-1] ; 
      for ( int j = 1 ; j < refchan ; j++ ) {
        sspref[j] = doffset * spref[j-1] + ( 1.0 - doffset ) * spref[j] ;
        sflagref[j] = flagref[j-1] + flagref[j] ;
      }
      // TSYS
      if ( spref.nelements() == tsysref.nelements() ) {
        for ( int j = 0 ; j < abs(ioffset) ; j++ ) {
          stsysref[j] = tsysref[refchan+ioffset+j] ;
        }
        for ( int j = abs(ioffset) ; j < refchan ; j++ ) {
          stsysref[j] = tsysref[j+ioffset] ;
        }
        tsysref = stsysref.copy() ;
        stsysref[0] = doffset * tsysref[refchan-1] + ( 1.0 - doffset ) * tsysref[0] ;
        for ( int j = 1 ; j < refchan ; j++ ) {
          stsysref[j] = doffset * tsysref[j-1] + ( 1.0 - doffset ) * tsysref[j] ;
        }
      }
    }

    // shift signal spectra if necessary (only for APEX?)
    if ( choffset2 != 0.0 ) {
      int sigchan = spsig.nelements() ;
      Vector<Float> sspsig( spsig.nelements() ) ;
      Vector<Float> stsyssig( tsyssig.nelements() ) ;
      Vector<uChar> sflagsig( flagsig.nelements() ) ;
      if ( ioffset2 > 0 ) {
        // SPECTRA and FLAGTRA
        for ( int j = 0 ; j < sigchan-ioffset2 ; j++ ) {
          sspsig[j] = spsig[j+ioffset2] ; 
          sflagsig[j] = flagsig[j+ioffset2] ;
        }
        for ( int j = sigchan-ioffset2 ; j < sigchan ; j++ ) {
          sspsig[j] = spsig[j-sigchan+ioffset2] ;
          sflagsig[j] = flagsig[j-sigchan+ioffset2] ;
        }
        spsig = sspsig.copy() ;
        flagsig = sflagsig.copy() ;
        for ( int j = 0 ; j < sigchan - 1 ; j++ ) {
          sspsig[j] = doffset2 * spsig[j+1] + ( 1.0 - doffset2 ) * spsig[j] ;
          sflagsig[j] = flagsig[j+1] || flagsig[j] ;
        }
        sspsig[sigchan-1] = doffset2 * spsig[0] + ( 1.0 - doffset2 ) * spsig[sigchan-1] ;
        sflagsig[sigchan-1] = flagsig[0] || flagsig[sigchan-1] ; 
        // TSTS
        if ( spsig.nelements() == tsyssig.nelements() ) {
          for ( int j = 0 ; j < sigchan-ioffset2 ; j++ ) {
            stsyssig[j] = tsyssig[j+ioffset2] ;
          }
          for ( int j = sigchan-ioffset2 ; j < sigchan ; j++ ) {
            stsyssig[j] = tsyssig[j-sigchan+ioffset2] ;
          }
          tsyssig = stsyssig.copy() ;
          for ( int j = 0 ; j < sigchan - 1 ; j++ ) {
            stsyssig[j] = doffset2 * tsyssig[j+1] + ( 1.0 - doffset2 ) * tsyssig[j] ;
          }
          stsyssig[sigchan-1] = doffset2 * tsyssig[0] + ( 1.0 - doffset2 ) * tsyssig[sigchan-1] ;
        }
      }
      else {
        // SPECTRA and FLAGTRA
        for ( int j = 0 ; j < abs(ioffset2) ; j++ ) {
          sspsig[j] = spsig[sigchan+ioffset2+j] ;
          sflagsig[j] = flagsig[sigchan+ioffset2+j] ;
        }
        for ( int j = abs(ioffset2) ; j < sigchan ; j++ ) {
          sspsig[j] = spsig[j+ioffset2] ;
          sflagsig[j] = flagsig[j+ioffset2] ;
        }
        spsig = sspsig.copy() ;
        flagsig = sflagsig.copy() ;
        sspsig[0] = doffset2 * spsig[sigchan-1] + ( 1.0 - doffset2 ) * spsig[0] ;
        sflagsig[0] = flagsig[0] + flagsig[sigchan-1] ; 
        for ( int j = 1 ; j < sigchan ; j++ ) {
          sspsig[j] = doffset2 * spsig[j-1] + ( 1.0 - doffset2 ) * spsig[j] ;
          sflagsig[j] = flagsig[j-1] + flagsig[j] ;
        }
        // TSYS
        if ( spsig.nelements() == tsyssig.nelements() ) {
          for ( int j = 0 ; j < abs(ioffset2) ; j++ ) {
            stsyssig[j] = tsyssig[sigchan+ioffset2+j] ;
          }
          for ( int j = abs(ioffset2) ; j < sigchan ; j++ ) {
            stsyssig[j] = tsyssig[j+ioffset2] ;
          }
          tsyssig = stsyssig.copy() ;
          stsyssig[0] = doffset2 * tsyssig[sigchan-1] + ( 1.0 - doffset2 ) * tsyssig[0] ;
          for ( int j = 1 ; j < sigchan ; j++ ) {
            stsyssig[j] = doffset2 * tsyssig[j-1] + ( 1.0 - doffset2 ) * tsyssig[j] ;
          }
        }
      }
    }

    // folding
    acc.add( spsig, !flagsig, tsyssig, intsig, timesig ) ;
    acc.add( sspref, !sflagref, stsysref, intref, timeref ) ;
    
    // put result
    specColOut.put( i, acc.getSpectrum() ) ;
    const Vector<Bool> &msk = acc.getMask() ;
    Vector<uChar> flg( msk.shape() ) ;
    convertArray( flg, !msk ) ;
    flagColOut.put( i, flg ) ;
    tsysColOut.put( i, acc.getTsys() ) ;
    intervalColOut.put( i, acc.getInterval() ) ;
    mjdColOut.put( i, acc.getTime() ) ;
    // change FREQ_ID to unshifted IF setting (only for APEX?)
    if ( choffset2 != 0.0 ) {
      uInt freqid = fidColOut( 0 ) ; // assume single-IF data
      double refpix, refval, increment ;
      out->frequencies().getEntry( refpix, refval, increment, freqid ) ;
      refval -= choffset * increment ;
      uInt newfreqid = out->frequencies().addEntry( refpix, refval, increment ) ;
      Vector<uInt> freqids = fidColOut.getColumn() ;
      for ( uInt j = 0 ; j < freqids.nelements() ; j++ ) {
        if ( freqids[j] == freqid ) 
          freqids[j] = newfreqid ;
      }
      fidColOut.putColumn( freqids ) ;
    }

    acc.reset() ;
  }

  return out ;
}


CountedPtr< Scantable > STMath::freqSwitch( const CountedPtr< Scantable >& in )
{
  // make copy or reference
  CountedPtr< Scantable > out = getScantable(in, false);
  Table& tout = out->table();
  Block<String> cols(4);
  cols[0] = String("SCANNO");
  cols[1] = String("CYCLENO");
  cols[2] = String("BEAMNO");
  cols[3] = String("POLNO");
  TableIterator iter(tout, cols);
  while (!iter.pastEnd()) {
    Table subt = iter.table();
    // this should leave us with two rows for the two IFs....if not ignore
    if (subt.nrow() != 2 ) {
      continue;
    }
    ArrayColumn<Float> specCol(subt, "SPECTRA");
    ArrayColumn<Float> tsysCol(subt, "TSYS");
    ArrayColumn<uChar> flagCol(subt, "FLAGTRA");
    Vector<Float> onspec,offspec, ontsys, offtsys;
    Vector<uChar> onflag, offflag;
    tsysCol.get(0, ontsys);   tsysCol.get(1, offtsys);
    specCol.get(0, onspec);   specCol.get(1, offspec);
    flagCol.get(0, onflag);   flagCol.get(1, offflag);
    MaskedArray<Float> on  = maskedArray(onspec, onflag);
    MaskedArray<Float> off = maskedArray(offspec, offflag);
    MaskedArray<Float> oncopy = on.copy();

    on /= off; on -= 1.0f;
    on *= ontsys[0];
    off /= oncopy; off -= 1.0f;
    off *= offtsys[0];
    specCol.put(0, on.getArray());
    const Vector<Bool>& m0 = on.getMask();
    Vector<uChar> flags0(m0.shape());
    convertArray(flags0, !m0);
    flagCol.put(0, flags0);

    specCol.put(1, off.getArray());
    const Vector<Bool>& m1 = off.getMask();
    Vector<uChar> flags1(m1.shape());
    convertArray(flags1, !m1);
    flagCol.put(1, flags1);
    ++iter;
  }

  return out;
}

std::vector< float > STMath::statistic( const CountedPtr< Scantable > & in,
                                        const std::vector< bool > & mask,
                                        const std::string& which )
{

  Vector<Bool> m(mask);
  const Table& tab = in->table();
  ROArrayColumn<Float> specCol(tab, "SPECTRA");
  ROArrayColumn<uChar> flagCol(tab, "FLAGTRA");
  std::vector<float> out;
  for (uInt i=0; i < tab.nrow(); ++i ) {
    Vector<Float> spec; specCol.get(i, spec);
    Vector<uChar> flag; flagCol.get(i, flag);
    MaskedArray<Float> ma  = maskedArray(spec, flag);
    float outstat = 0.0;
    if ( spec.nelements() == m.nelements() ) {
      outstat = mathutil::statistics(which, ma(m));
    } else {
      outstat = mathutil::statistics(which, ma);
    }
    out.push_back(outstat);
  }
  return out;
}

std::vector< float > STMath::statisticRow( const CountedPtr< Scantable > & in,
                                        const std::vector< bool > & mask,
					const std::string& which,
					int row )
{

  Vector<Bool> m(mask);
  const Table& tab = in->table();
  ROArrayColumn<Float> specCol(tab, "SPECTRA");
  ROArrayColumn<uChar> flagCol(tab, "FLAGTRA");
  std::vector<float> out;

  Vector<Float> spec; specCol.get(row, spec);
  Vector<uChar> flag; flagCol.get(row, flag);
  MaskedArray<Float> ma  = maskedArray(spec, flag);
  float outstat = 0.0;
  if ( spec.nelements() == m.nelements() ) {
    outstat = mathutil::statistics(which, ma(m));
  } else {
    outstat = mathutil::statistics(which, ma);
  }
  out.push_back(outstat);

  return out;
}

std::vector< int > STMath::minMaxChan( const CountedPtr< Scantable > & in,
                                        const std::vector< bool > & mask,
                                        const std::string& which )
{

  Vector<Bool> m(mask);
  const Table& tab = in->table();
  ROArrayColumn<Float> specCol(tab, "SPECTRA");
  ROArrayColumn<uChar> flagCol(tab, "FLAGTRA");
  std::vector<int> out;
  for (uInt i=0; i < tab.nrow(); ++i ) {
    Vector<Float> spec; specCol.get(i, spec);
    Vector<uChar> flag; flagCol.get(i, flag);
    MaskedArray<Float> ma  = maskedArray(spec, flag);
    if (ma.ndim() != 1) {
      throw (ArrayError(
          "std::vector<int> STMath::minMaxChan("
          "ContedPtr<Scantable> &in, std::vector<bool> &mask, "
          " std::string &which)"
	  " - MaskedArray is not 1D"));
    }
    IPosition outpos(1,0);
    if ( spec.nelements() == m.nelements() ) {
      outpos = mathutil::minMaxPos(which, ma(m));
    } else {
      outpos = mathutil::minMaxPos(which, ma);
    }
    out.push_back(outpos[0]);
  }
  return out;
}

CountedPtr< Scantable > STMath::bin( const CountedPtr< Scantable > & in,
                                     int width )
{
  if ( !in->getSelection().empty() ) throw(AipsError("Can't bin subset of the data."));
  CountedPtr< Scantable > out = getScantable(in, false);
  Table& tout = out->table();
  out->frequencies().rescale(width, "BIN");
  ArrayColumn<Float> specCol(tout, "SPECTRA");
  ArrayColumn<uChar> flagCol(tout, "FLAGTRA");
  ArrayColumn<Float> tsysCol(tout, "TSYS");

  for (uInt i=0; i < tout.nrow(); ++i ) {
    MaskedArray<Float> main  = maskedArray(specCol(i), flagCol(i));
    MaskedArray<Float> maout;
    LatticeUtilities::bin(maout, main, 0, Int(width));
    specCol.put(i, maout.getArray());
    flagCol.put(i, flagsFromMA(maout));
    if (tsysCol(i).nelements() == specCol(i).nelements()) {
      MaskedArray<Float> matsysin = maskedArray(tsysCol(i), flagCol(i));
      MaskedArray<Float> matsysout;
      LatticeUtilities::bin(matsysout, matsysin, 0, Int(width));
      tsysCol.put(i, matsysout.getArray());
    }
    // take only the first binned spectrum's length for the deprecated
    // global header item nChan
    if (i==0) tout.rwKeywordSet().define(String("nChan"),
                                       Int(maout.getArray().nelements()));
  }
  return out;
}

CountedPtr< Scantable > STMath::resample( const CountedPtr< Scantable >& in,
                                          const std::string& method,
                                          float width )
//
// Should add the possibility of width being specified in km/s. This means
// that for each freqID (SpectralCoordinate) we will need to convert to an
// average channel width (say at the reference pixel).  Then we would need
// to be careful to make sure each spectrum (of different freqID)
// is the same length.
//
{
  //InterpolateArray1D<Double,Float>::InterpolationMethod interp;
  Int interpMethod(stringToIMethod(method));

  CountedPtr< Scantable > out = getScantable(in, false);
  Table& tout = out->table();

// Resample SpectralCoordinates (one per freqID)
  out->frequencies().rescale(width, "RESAMPLE");
  TableIterator iter(tout, "IFNO");
  TableRow row(tout);
  while ( !iter.pastEnd() ) {
    Table tab = iter.table();
    ArrayColumn<Float> specCol(tab, "SPECTRA");
    //ArrayColumn<Float> tsysCol(tout, "TSYS");
    ArrayColumn<uChar> flagCol(tab, "FLAGTRA");
    Vector<Float> spec;
    Vector<uChar> flag;
    specCol.get(0,spec); // the number of channels should be constant per IF
    uInt nChanIn = spec.nelements();
    Vector<Float> xIn(nChanIn); indgen(xIn);
    Int fac =  Int(nChanIn/width);
    Vector<Float> xOut(fac+10); // 10 to be safe - resize later
    uInt k = 0;
    Float x = 0.0;
    while (x < Float(nChanIn) ) {
      xOut(k) = x;
      k++;
      x += width;
    }
    uInt nChanOut = k;
    xOut.resize(nChanOut, True);
    // process all rows for this IFNO
    Vector<Float> specOut;
    Vector<Bool> maskOut;
    Vector<uChar> flagOut;
    for (uInt i=0; i < tab.nrow(); ++i) {
      specCol.get(i, spec);
      flagCol.get(i, flag);
      Vector<Bool> mask(flag.nelements());
      convertArray(mask, flag);

      IPosition shapeIn(spec.shape());
      //sh.nchan = nChanOut;
      InterpolateArray1D<Float,Float>::interpolate(specOut, maskOut, xOut,
                                                   xIn, spec, mask,
                                                   interpMethod, True, True);
      /// @todo do the same for channel based Tsys
      flagOut.resize(maskOut.nelements());
      convertArray(flagOut, maskOut);
      specCol.put(i, specOut);
      flagCol.put(i, flagOut);
    }
    ++iter;
  }

  return out;
}

STMath::imethod STMath::stringToIMethod(const std::string& in)
{
  static STMath::imap lookup;

  // initialize the lookup table if necessary
  if ( lookup.empty() ) {
    lookup["nearest"]   = InterpolateArray1D<Double,Float>::nearestNeighbour;
    lookup["linear"] = InterpolateArray1D<Double,Float>::linear;
    lookup["cubic"]  = InterpolateArray1D<Double,Float>::cubic;
    lookup["spline"]  = InterpolateArray1D<Double,Float>::spline;
  }

  STMath::imap::const_iterator iter = lookup.find(in);

  if ( lookup.end() == iter ) {
    std::string message = in;
    message += " is not a valid interpolation mode";
    throw(AipsError(message));
  }
  return iter->second;
}

WeightType STMath::stringToWeight(const std::string& in)
{
  static std::map<std::string, WeightType> lookup;

  // initialize the lookup table if necessary
  if ( lookup.empty() ) {
    lookup["NONE"]   = asap::W_NONE;
    lookup["TINT"] = asap::W_TINT;
    lookup["TINTSYS"]  = asap::W_TINTSYS;
    lookup["TSYS"]  = asap::W_TSYS;
    lookup["VAR"]  = asap::W_VAR;
  }

  std::map<std::string, WeightType>::const_iterator iter = lookup.find(in);

  if ( lookup.end() == iter ) {
    std::string message = in;
    message += " is not a valid weighting mode";
    throw(AipsError(message));
  }
  return iter->second;
}

CountedPtr< Scantable > STMath::gainElevation( const CountedPtr< Scantable >& in,
                                               const vector< float > & coeff,
                                               const std::string & filename,
                                               const std::string& method)
{
  LogIO os( LogOrigin( "STMath", "gainElevation", WHERE ) ) ;
  // Get elevation data from Scantable and convert to degrees
  CountedPtr< Scantable > out = getScantable(in, false);
  Table& tab = out->table();
  ROScalarColumn<Float> elev(tab, "ELEVATION");
  Vector<Float> x = elev.getColumn();
  x *= Float(180 / C::pi);                        // Degrees

  Vector<Float> coeffs(coeff);
  const uInt nc = coeffs.nelements();
  if ( filename.length() > 0 && nc > 0 ) {
    throw(AipsError("You must choose either polynomial coefficients or an ascii file, not both"));
  }

  // Correct
  if ( nc > 0 || filename.length() == 0 ) {
    // Find instrument
    Bool throwit = True;
    Instrument inst =
      STAttr::convertInstrument(tab.keywordSet().asString("AntennaName"),
                                throwit);

    // Set polynomial
    Polynomial<Float>* ppoly = 0;
    Vector<Float> coeff;
    String msg;
    if ( nc > 0 ) {
      ppoly = new Polynomial<Float>(nc-1);
      coeff = coeffs;
      msg = String("user");
    } else {
      STAttr sdAttr;
      coeff = sdAttr.gainElevationPoly(inst);
      ppoly = new Polynomial<Float>(coeff.nelements()-1);
      msg = String("built in");
    }

    if ( coeff.nelements() > 0 ) {
      ppoly->setCoefficients(coeff);
    } else {
      delete ppoly;
      throw(AipsError("There is no known gain-elevation polynomial known for this instrument"));
    }
    os << "Making polynomial correction with " << msg << " coefficients:" << endl;
    os << "   " <<  coeff << LogIO::POST;
    const uInt nrow = tab.nrow();
    Vector<Float> factor(nrow);
    for ( uInt i=0; i < nrow; ++i ) {
      factor[i] = 1.0 / (*ppoly)(x[i]);
    }
    delete ppoly;
    scaleByVector(tab, factor, true);

  } else {
    // Read and correct
    os << "Making correction from ascii Table" << LogIO::POST;
    scaleFromAsciiTable(tab, filename, method, x, true);
  }
  return out;
}

void STMath::scaleFromAsciiTable(Table& in, const std::string& filename,
                                 const std::string& method,
                                 const Vector<Float>& xout, bool dotsys)
{

// Read gain-elevation ascii file data into a Table.

  String formatString;
  Table tbl = readAsciiTable(formatString, Table::Memory, filename, "", "", False);
  scaleFromTable(in, tbl, method, xout, dotsys);
}

void STMath::scaleFromTable(Table& in,
                            const Table& table,
                            const std::string& method,
                            const Vector<Float>& xout, bool dotsys)
{

  ROScalarColumn<Float> geElCol(table, "ELEVATION");
  ROScalarColumn<Float> geFacCol(table, "FACTOR");
  Vector<Float> xin = geElCol.getColumn();
  Vector<Float> yin = geFacCol.getColumn();
  Vector<Bool> maskin(xin.nelements(),True);

  // Interpolate (and extrapolate) with desired method

  InterpolateArray1D<Double,Float>::InterpolationMethod interp = stringToIMethod(method);

   Vector<Float> yout;
   Vector<Bool> maskout;
   InterpolateArray1D<Float,Float>::interpolate(yout, maskout, xout,
                                                xin, yin, maskin, interp,
                                                True, True);

   scaleByVector(in, Float(1.0)/yout, dotsys);
}

void STMath::scaleByVector( Table& in,
                            const Vector< Float >& factor,
                            bool dotsys )
{
  uInt nrow = in.nrow();
  if ( factor.nelements() != nrow ) {
    throw(AipsError("factors.nelements() != table.nelements()"));
  }
  ArrayColumn<Float> specCol(in, "SPECTRA");
  ArrayColumn<uChar> flagCol(in, "FLAGTRA");
  ArrayColumn<Float> tsysCol(in, "TSYS");
  for (uInt i=0; i < nrow; ++i) {
    MaskedArray<Float> ma  = maskedArray(specCol(i), flagCol(i));
    ma *= factor[i];
    specCol.put(i, ma.getArray());
    flagCol.put(i, flagsFromMA(ma));
    if ( dotsys ) {
      Vector<Float> tsys = tsysCol(i);
      tsys *= factor[i];
      tsysCol.put(i,tsys);
    }
  }
}

CountedPtr< Scantable > STMath::convertFlux( const CountedPtr< Scantable >& in,
                                             float d, float etaap,
                                             float jyperk )
{
  LogIO os( LogOrigin( "STMath", "convertFlux", WHERE ) ) ;

  CountedPtr< Scantable > out = getScantable(in, false);
  Table& tab = in->table();
  Table& outtab = out->table();
  Unit fluxUnit(tab.keywordSet().asString("FluxUnit"));
  Unit K(String("K"));
  Unit JY(String("Jy"));

  bool tokelvin = true;
  Double cfac = 1.0;

  if ( fluxUnit == JY ) {
    os << "Converting to K" << LogIO::POST;
    Quantum<Double> t(1.0,fluxUnit);
    Quantum<Double> t2 = t.get(JY);
    cfac = (t2 / t).getValue();               // value to Jy

    tokelvin = true;
    out->setFluxUnit("K");
  } else if ( fluxUnit == K ) {
    os << "Converting to Jy" << LogIO::POST;
    Quantum<Double> t(1.0,fluxUnit);
    Quantum<Double> t2 = t.get(K);
    cfac = (t2 / t).getValue();              // value to K

    tokelvin = false;
    out->setFluxUnit("Jy");
  } else {
    throw(AipsError("Unrecognized brightness units in Table - must be consistent with Jy or K"));
  }
  // Make sure input values are converted to either Jy or K first...
  Float factor = cfac;

  // Select method
  if (jyperk > 0.0) {
    factor *= jyperk;
    if ( tokelvin ) factor = 1.0 / jyperk;
    os << "Jy/K = " << jyperk << LogIO::POST;
    Vector<Float> factors(outtab.nrow(), factor);
    scaleByVector(outtab,factors, false);
  } else if ( etaap > 0.0) {
    if (d < 0) {
      Instrument inst =
	STAttr::convertInstrument(tab.keywordSet().asString("AntennaName"),
				  True);
      STAttr sda;
      d = sda.diameter(inst);
    }
    jyperk = STAttr::findJyPerK(etaap, d);
    os << "Jy/K = " << jyperk << LogIO::POST;
    factor *= jyperk;
    if ( tokelvin ) {
      factor = 1.0 / factor;
    }
    Vector<Float> factors(outtab.nrow(), factor);
    scaleByVector(outtab, factors, False);
  } else {

    // OK now we must deal with automatic look up of values.
    // We must also deal with the fact that the factors need
    // to be computed per IF and may be different and may
    // change per integration.

    os <<"Looking up conversion factors" << LogIO::POST;
    convertBrightnessUnits(out, tokelvin, cfac);
  }

  return out;
}

void STMath::convertBrightnessUnits( CountedPtr<Scantable>& in,
                                     bool tokelvin, float cfac )
{
  Table& table = in->table();
  Instrument inst =
    STAttr::convertInstrument(table.keywordSet().asString("AntennaName"), True);
  TableIterator iter(table, "FREQ_ID");
  STFrequencies stfreqs = in->frequencies();
  STAttr sdAtt;
  while (!iter.pastEnd()) {
    Table tab = iter.table();
    ArrayColumn<Float> specCol(tab, "SPECTRA");
    ArrayColumn<uChar> flagCol(tab, "FLAGTRA");
    ROScalarColumn<uInt> freqidCol(tab, "FREQ_ID");
    MEpoch::ROScalarColumn timeCol(tab, "TIME");

    uInt freqid; freqidCol.get(0, freqid);
    Vector<Float> tmpspec; specCol.get(0, tmpspec);
    // STAttr.JyPerK has a Vector interface... change sometime.
    Vector<Float> freqs(1,stfreqs.getRefFreq(freqid, tmpspec.nelements()));
    for ( uInt i=0; i<tab.nrow(); ++i) {
      Float jyperk = (sdAtt.JyPerK(inst, timeCol(i), freqs))[0];
      Float factor = cfac * jyperk;
      if ( tokelvin ) factor = Float(1.0) / factor;
      MaskedArray<Float> ma  = maskedArray(specCol(i), flagCol(i));
      ma *= factor;
      specCol.put(i, ma.getArray());
      flagCol.put(i, flagsFromMA(ma));
    }
  ++iter;
  }
}

CountedPtr< Scantable > STMath::opacity( const CountedPtr< Scantable > & in,
                                         const std::vector<float>& tau )
{
  CountedPtr< Scantable > out = getScantable(in, false);

  Table outtab = out->table();

  const Int ntau = uInt(tau.size());
  std::vector<float>::const_iterator tauit = tau.begin();
  AlwaysAssert((ntau == 1 || ntau == in->nif() || ntau == in->nif() * in->npol()),
               AipsError);
  TableIterator iiter(outtab, "IFNO");
  while ( !iiter.pastEnd() ) {
    Table itab = iiter.table();
    TableIterator piter(itab, "POLNO");
    while ( !piter.pastEnd() ) {
      Table tab = piter.table();
      ROScalarColumn<Float> elev(tab, "ELEVATION");
      ArrayColumn<Float> specCol(tab, "SPECTRA");
      ArrayColumn<uChar> flagCol(tab, "FLAGTRA");
      ArrayColumn<Float> tsysCol(tab, "TSYS");
      for ( uInt i=0; i<tab.nrow(); ++i) {
        Float zdist = Float(C::pi_2) - elev(i);
        Float factor = exp(*tauit/cos(zdist));
        MaskedArray<Float> ma = maskedArray(specCol(i), flagCol(i));
        ma *= factor;
        specCol.put(i, ma.getArray());
        flagCol.put(i, flagsFromMA(ma));
        Vector<Float> tsys;
        tsysCol.get(i, tsys);
        tsys *= factor;
        tsysCol.put(i, tsys);
      }
      if (ntau == in->nif()*in->npol() ) {
        tauit++;
      }
      piter++;
    }
    if (ntau >= in->nif() ) {
      tauit++;
    }
    iiter++;
  }
  return out;
}

CountedPtr< Scantable > STMath::smoothOther( const CountedPtr< Scantable >& in,
                                             const std::string& kernel,
                                             float width, int order)
{
  CountedPtr< Scantable > out = getScantable(in, false);
  Table table = out->table();

  TableIterator iter(table, "IFNO");
  while (!iter.pastEnd()) {
    Table tab = iter.table();
    ArrayColumn<Float> specCol(tab, "SPECTRA");
    ArrayColumn<uChar> flagCol(tab, "FLAGTRA");
    Vector<Float> spec;
    Vector<uChar> flag;
    for (uInt i = 0; i < tab.nrow(); ++i) {
      specCol.get(i, spec);
      flagCol.get(i, flag);
      Vector<Bool> mask(flag.nelements());
      convertArray(mask, flag);
      Vector<Float> specout;
      Vector<Bool> maskout;
      if (kernel == "hanning") {
	mathutil::hanning(specout, maskout, spec, !mask);
      } else if (kernel == "rmedian") {
	mathutil::runningMedian(specout, maskout, spec , mask, width);
      } else if (kernel == "poly") {
	mathutil::polyfit(specout, maskout, spec, !mask, width, order);
      }

      for (uInt j = 0; j < flag.nelements(); ++j) {
	uChar userFlag = 1 << 7;
	if (maskout[j]==True) userFlag = 0 << 7;
	flag(j) = userFlag;
      }

      flagCol.put(i, flag);
      specCol.put(i, specout);
    }
  ++iter;
  }
  return out;
}

CountedPtr< Scantable > STMath::smooth( const CountedPtr< Scantable >& in,
                                        const std::string& kernel, float width,
                                        int order)
{
  if (kernel == "rmedian"  || kernel == "hanning" || kernel == "poly") {
    return smoothOther(in, kernel, width, order);
  }
  CountedPtr< Scantable > out = getScantable(in, false);
  Table& table = out->table();
  VectorKernel::KernelTypes type = VectorKernel::toKernelType(kernel);
  // same IFNO should have same no of channels
  // this saves overhead
  TableIterator iter(table, "IFNO");
  while (!iter.pastEnd()) {
    Table tab = iter.table();
    ArrayColumn<Float> specCol(tab, "SPECTRA");
    ArrayColumn<uChar> flagCol(tab, "FLAGTRA");
    Vector<Float> spec = specCol( 0 );
    uInt nchan = spec.nelements();
    Vector<Float> kvec = VectorKernel::make(type, width, nchan, True, False);
    Convolver<Float> conv(kvec, IPosition(1,nchan));
    Vector<uChar> flag;
    Vector<Bool> mask(nchan);
    for ( uInt i=0; i<tab.nrow(); ++i) {
      specCol.get(i, spec);
      flagCol.get(i, flag);
      convertArray(mask, flag);
      Vector<Float> specout;
      //mathutil::replaceMaskByZero(specout, mask);
      mathutil::replaceMaskByZero(spec, !mask);
      //std::vector<bool> vmask;
      //(!mask).tovector(vmask);
      //mathutil::doZeroOrderInterpolation(spec, vmask);
      conv.linearConv(specout, spec);
      specCol.put(i, specout);
    }
    ++iter;
  }
  return out;
}

CountedPtr< Scantable >
STMath::merge( const std::vector< CountedPtr < Scantable > >& in,
	       const std::string &freqTol )
{
  Double freqTolInHz = 0.0; // default is 0.0Hz (merge only when exact match)
  if (freqTol.size() > 0) {
    Quantum<Double> freqTolInQuantity;
    if (!Quantum<Double>::read(freqTolInQuantity, freqTol)) {
      throw(AipsError("Failed to convert freqTol string to quantity"));
    }
    if (!freqTolInQuantity.isConform("Hz")) {
      throw(AipsError("Invalid freqTol string"));
    }
    freqTolInHz = freqTolInQuantity.getValue("Hz");
    LogIO os(LogOrigin("STMath", "merge", WHERE));
    os << "frequency tolerance = " << freqTolInHz << "Hz" << LogIO::POST;
  }
  
  if ( in.size() < 2 ) {
    throw(AipsError("Need at least two scantables to perform a merge."));
  }
  std::vector<CountedPtr < Scantable > >::const_iterator it = in.begin();
  bool insitu = insitu_;
  setInsitu(false);
  CountedPtr< Scantable > out = getScantable(*it, false);
  setInsitu(insitu);
  Table& tout = out->table();
  ScalarColumn<uInt> freqidcol(tout,"FREQ_ID"), molidcol(tout, "MOLECULE_ID");
  ScalarColumn<uInt> scannocol(tout,"SCANNO"), focusidcol(tout,"FOCUS_ID");
  ScalarColumn<uInt> ifnocol(tout, "IFNO");
  // Renumber SCANNO to be 0-based
  Vector<uInt> scannos = scannocol.getColumn();
  uInt offset = min(scannos);
  scannos -= offset;
  scannocol.putColumn(scannos);
  uInt newscanno = max(scannos)+1;
  ++it;

  // new IFNO
  uInt ifnoCounter = max(ifnocol.getColumn()) + 1;
  
  // Here we assume that each IFNO has unique MOLECULE_ID
  // molIdMap:
  //    KEY: IFNO
  //    VALUE: MOLECULE_ID
  map<uInt, uInt> molIdMap;
  {
    TableIterator ifit(tout, "IFNO");
    while (!ifit.pastEnd()) {
      ROTableRow row(ifit.table());
      const TableRecord& rec = row.get(0);
      molIdMap[rec.asuInt("IFNO")] = rec.asuInt("MOLECULE_ID");
      ifit.next();
    }
  }
  
  while ( it != in.end() ){
    // Check FREQUENCIES/BASEFRAME
    if ( out->frequencies().getFrame(true) != (*it)->frequencies().getFrame(true) ) {
      throw(AipsError("BASEFRAME is not identical"));
    }
    
    if ( ! (*it)->conformant(*out) ) {
      // non conformant.
      LogIO os( LogOrigin( "STMath", "merge()", WHERE ) ) ;
      os << LogIO::SEVERE << "Can't merge scantables as header informations (any one of AntennaName, Equinox, and FluxUnit) differ." << LogIO::EXCEPTION ;
    }
    out->appendToHistoryTable((*it)->history());
    const Table& tab = (*it)->table();

    Block<String> cols(3);
    cols[0] = String("FREQ_ID");
    cols[1] = String("MOLECULE_ID");
    cols[2] = String("FOCUS_ID");
    
    TableIterator scanit(tab, "SCANNO");
    while (!scanit.pastEnd()) {
      TableIterator subit(scanit.table(), cols);
      while ( !subit.pastEnd() ) {
        uInt nrow = tout.nrow();
        Table thetab = subit.table();
        ROTableRow row(thetab);
	Vector<uInt> thecolvals(thetab.nrow());
	// The selected subset of table should have 
	// the equal FREQ_ID, MOLECULE_ID, and FOCUS_ID values.
	const TableRecord& rec = row.get(0);
	tout.addRow(thetab.nrow());
	TableCopy::copyRows(tout, thetab, nrow, 0, thetab.nrow());

	Slicer slice(IPosition(1, nrow), IPosition(1, thetab.nrow()),
		     Slicer::endIsLength);

	// Set the proper FREQ_ID
	Double rv,rp,inc;
	(*it)->frequencies().getEntry(rp, rv, inc, rec.asuInt("FREQ_ID"));
	uInt id;
	
	// default value is new unique IFNO
	uInt newifno = ifnoCounter;
	Bool isIfMerged = False;
	uInt nchan = rec.asArrayFloat("SPECTRA").shape()[0];
	//id = out->frequencies().addEntry(rp, rv, inc);
	if ( !out->frequencies().match(rp, rv, inc, freqTolInHz, id) ) {
	  // add new entry to FREQUENCIES table
	  id = out->frequencies().addEntry(rp, rv, inc);

	  // increment counter for IFNO
	  ifnoCounter++;
	}
	else {
	  // should renumber IFNO to be same as existing rows that have same FREQ_ID
	  LogIO os(LogOrigin("STMath", "merge", WHERE));
	  Table outFreqIdSelected = tout(tout.col("FREQ_ID") == id);
	  TableIterator _iter(outFreqIdSelected, "IFNO");
	  map<uInt, uInt> nchanMap;
	  while (!_iter.pastEnd()) {
	    const Table _table = _iter.table();
	    ROTableRow _row(_table);
	    const TableRecord &_rec = _row.get(0); 
	    uInt nchan = _rec.asArrayFloat("SPECTRA").shape()[0];
	    if (nchanMap.find(nchan) != nchanMap.end()) {
	      throw(AipsError("There are non-unique IFNOs assigned to spectra that have same FREQ_ID and same nchan. Something wrong."));
	    }
	    nchanMap[nchan] = _rec.asuInt("IFNO");
	    _iter.next();
	  }

	  os << LogIO::DEBUGGING << "nchanMap for " << id << ":" << LogIO::POST;
	  for (map<uInt, uInt>::iterator i = nchanMap.begin(); i != nchanMap.end(); ++i) {
	    os << LogIO::DEBUGGING << "nchanMap[" << i->first << "] = " << i->second << LogIO::POST;
	  }

	  if (nchanMap.find(nchan) == nchanMap.end()) {
	    // increment counter for IFNO
	    ifnoCounter++;
	  }
	  else {
	    // renumber IFNO to be same as existing value that corresponds to nchan
	    newifno = nchanMap[nchan];
	    isIfMerged = True;
	  }
	  os << LogIO::DEBUGGING << "newifno = " << newifno << LogIO::POST;
	}
	thecolvals = id;
	freqidcol.putColumnRange(slice, thecolvals);

	thecolvals = newifno;
	ifnocol.putColumnRange(slice, thecolvals);
	
	// Set the proper MOLECULE_ID
	Vector<String> name,fname;Vector<Double> rf;
	(*it)->molecules().getEntry(rf, name, fname, rec.asuInt("MOLECULE_ID"));
	id = out->molecules().addEntry(rf, name, fname);
	if (molIdMap.find(newifno) == molIdMap.end()) {
	  // add new entry to molIdMap
	  molIdMap[newifno] = id;
	}
	if (isIfMerged) {
	  id = molIdMap[newifno];
	}
	thecolvals = id;
	molidcol.putColumnRange(slice, thecolvals);
	
	// Set the proper FOCUS_ID
	Float fpa,frot,fax,ftan,fhand,fmount,fuser, fxy, fxyp;
	(*it)->focus().getEntry(fpa, fax, ftan, frot, fhand, fmount,fuser,
				fxy, fxyp, rec.asuInt("FOCUS_ID"));
	id = out->focus().addEntry(fpa, fax, ftan, frot, fhand, fmount,fuser,
				   fxy, fxyp);
	thecolvals = id;
	focusidcol.putColumnRange(slice, thecolvals);

	// Set the proper SCANNO
	thecolvals = newscanno;
	scannocol.putColumnRange(slice, thecolvals);

        ++subit;
      }
      ++newscanno;
      ++scanit;
    }
    ++it;
  }
  return out;
}

CountedPtr< Scantable >
  STMath::invertPhase( const CountedPtr < Scantable >& in )
{
  return applyToPol(in, &STPol::invertPhase, Float(0.0));
}

CountedPtr< Scantable >
  STMath::rotateXYPhase( const CountedPtr < Scantable >& in, float phase )
{
   return applyToPol(in, &STPol::rotatePhase, Float(phase));
}

CountedPtr< Scantable >
  STMath::rotateLinPolPhase( const CountedPtr < Scantable >& in, float phase )
{
  return applyToPol(in, &STPol::rotateLinPolPhase, Float(phase));
}

CountedPtr< Scantable > STMath::applyToPol( const CountedPtr<Scantable>& in,
                                             STPol::polOperation fptr,
                                             Float phase )
{
  CountedPtr< Scantable > out = getScantable(in, false);
  Table& tout = out->table();
  Block<String> cols(4);
  cols[0] = String("SCANNO");
  cols[1] = String("BEAMNO");
  cols[2] = String("IFNO");
  cols[3] = String("CYCLENO");
  TableIterator iter(tout, cols);
  CountedPtr<STPol> stpol = STPol::getPolClass(out->factories_,
                                               out->getPolType() );
  while (!iter.pastEnd()) {
    Table t = iter.table();
    ArrayColumn<Float> speccol(t, "SPECTRA");
    ScalarColumn<uInt> focidcol(t, "FOCUS_ID");
    Matrix<Float> pols(speccol.getColumn());
    try {
      stpol->setSpectra(pols);
      Float fang,fhand;
      fang = in->focusTable_.getTotalAngle(focidcol(0));
      fhand = in->focusTable_.getFeedHand(focidcol(0));
      stpol->setPhaseCorrections(fang, fhand);
      // use a member function pointer in STPol.  This only works on
      // the STPol pointer itself, not the Counted Pointer so
      // derefernce it.
      (&(*(stpol))->*fptr)(phase);
      speccol.putColumn(stpol->getSpectra());
    } catch (AipsError& e) {
      //delete stpol;stpol=0;
      throw(e);
    }
    ++iter;
  }
  //delete stpol;stpol=0;
  return out;
}

CountedPtr< Scantable >
  STMath::swapPolarisations( const CountedPtr< Scantable > & in )
{
  CountedPtr< Scantable > out = getScantable(in, false);
  Table& tout = out->table();
  Table t0 = tout(tout.col("POLNO") == 0);
  Table t1 = tout(tout.col("POLNO") == 1);
  if ( t0.nrow() != t1.nrow() )
    throw(AipsError("Inconsistent number of polarisations"));
  ArrayColumn<Float> speccol0(t0, "SPECTRA");
  ArrayColumn<uChar> flagcol0(t0, "FLAGTRA");
  ArrayColumn<Float> speccol1(t1, "SPECTRA");
  ArrayColumn<uChar> flagcol1(t1, "FLAGTRA");
  Matrix<Float> s0 = speccol0.getColumn();
  Matrix<uChar> f0 = flagcol0.getColumn();
  speccol0.putColumn(speccol1.getColumn());
  flagcol0.putColumn(flagcol1.getColumn());
  speccol1.putColumn(s0);
  flagcol1.putColumn(f0);
  return out;
}

CountedPtr< Scantable >
  STMath::averagePolarisations( const CountedPtr< Scantable > & in,
                                const std::vector<bool>& mask,
                                const std::string& weight )
{
  if (in->npol() < 2 )
    throw(AipsError("averagePolarisations can only be applied to two or more"
                    "polarisations"));
  bool insitu = insitu_;
  setInsitu(false);
  CountedPtr< Scantable > pols = getScantable(in, true);
  setInsitu(insitu);
  Table& tout = pols->table();
  std::string taql = "SELECT FROM $1 WHERE POLNO IN [0,1]";
  Table tab = tableCommand(taql, in->table());
  if (tab.nrow() == 0 )
    throw(AipsError("Could not find  any rows with POLNO==0 and POLNO==1"));
  TableCopy::copyRows(tout, tab);
  TableVector<uInt> vec(tout, "POLNO");
  vec = 0;
  pols->table_.rwKeywordSet().define("nPol", Int(1));
  pols->table_.rwKeywordSet().define("POLTYPE", String("stokes"));
  //pols->table_.rwKeywordSet().define("POLTYPE", in->getPolType());
  std::vector<CountedPtr<Scantable> > vpols;
  vpols.push_back(pols);
  CountedPtr< Scantable > out = average(vpols, mask, weight, "SCAN");
  return out;
}

CountedPtr< Scantable >
  STMath::averageBeams( const CountedPtr< Scantable > & in,
                        const std::vector<bool>& mask,
                        const std::string& weight )
{
  bool insitu = insitu_;
  setInsitu(false);
  CountedPtr< Scantable > beams = getScantable(in, false);
  setInsitu(insitu);
  Table& tout = beams->table();
  // give all rows the same BEAMNO
  TableVector<uInt> vec(tout, "BEAMNO");
  vec = 0;
  beams->table_.rwKeywordSet().define("nBeam", Int(1));
  std::vector<CountedPtr<Scantable> > vbeams;
  vbeams.push_back(beams);
  CountedPtr< Scantable > out = average(vbeams, mask, weight, "SCAN");
  return out;
}


CountedPtr< Scantable >
  asap::STMath::frequencyAlign( const CountedPtr< Scantable > & in,
                                const std::string & refTime,
                                const std::string & method)
{
  LogIO os( casa::LogOrigin("STMath", "frequencyAlign()", WHERE));
  // clone as this is not working insitu
  bool insitu = insitu_;
  setInsitu(false);
  CountedPtr< Scantable > out = getScantable(in, false);
  setInsitu(insitu);
  Table& tout = out->table();
  // Get reference Epoch to time of first row or given String
  Unit DAY(String("d"));
  MEpoch::Ref epochRef(in->getTimeReference());
  MEpoch refEpoch;
  if (refTime.length()>0) {
    Quantum<Double> qt;
    if (MVTime::read(qt,refTime)) {
      MVEpoch mv(qt);
      refEpoch = MEpoch(mv, epochRef);
   } else {
      throw(AipsError("Invalid format for Epoch string"));
   }
  } else {
    refEpoch = in->timeCol_(0);
  }
  MPosition refPos = in->getAntennaPosition();

  InterpolateArray1D<Double,Float>::InterpolationMethod interp = stringToIMethod(method);
  /*
  // Comment from MV.
  // the following code has been commented out because different FREQ_IDs have to be aligned together even
  // if the frame doesn't change. So far, lack of this check didn't cause any problems.
  // test if user frame is different to base frame
  if ( in->frequencies().getFrameString(true)
       == in->frequencies().getFrameString(false) ) {
    throw(AipsError("Can't convert as no output frame has been set"
                    " (use set_freqframe) or it is aligned already."));
  }
  */
  MFrequency::Types system = in->frequencies().getFrame();
  MVTime mvt(refEpoch.getValue());
  String epochout = mvt.string(MVTime::YMD) + String(" (") + refEpoch.getRefString() + String(")");
  os << "Aligning at reference Epoch " << epochout
     << " in frame " << MFrequency::showType(system) << LogIO::POST;
  // set up the iterator
  Block<String> cols(4);
  // select by constant direction
  cols[0] = String("SRCNAME");
  cols[1] = String("BEAMNO");
  // select by IF ( no of channels varies over this )
  cols[2] = String("IFNO");
  // select by restfrequency
  cols[3] = String("MOLECULE_ID");
  TableIterator iter(tout, cols);
  while ( !iter.pastEnd() ) {
    Table t = iter.table();
    ROScalarColumn<String> snCol(t, "SRCNAME");
    os << "Aligning to position of source '" << snCol(0) << "'" << LogIO::POST;
    MDirection::ROScalarColumn dirCol(t, "DIRECTION");
    TableIterator fiter(t, "FREQ_ID");
    // determine nchan from the first row. This should work as
    // we are iterating over BEAMNO and IFNO    
    // we should have constant direction

    ROArrayColumn<Float> sCol(t, "SPECTRA");
    const MDirection direction = dirCol(0);
    const uInt nchan = sCol(0).nelements();

    // skip operations if there is nothing to align
    if (fiter.pastEnd()) {
        continue;
    }

    Table ftab = fiter.table();
    // align all frequency ids with respect to the first encountered id
    ScalarColumn<uInt> freqidCol(ftab, "FREQ_ID");
    // get the SpectralCoordinate for the freqid, which we are iterating over
    SpectralCoordinate sC = \
      in->frequencies().getSpectralCoordinate(freqidCol(0));
    FrequencyAligner<Float> fa( sC, nchan, refEpoch,
                                direction, refPos, system );
    // realign the SpectralCoordinate and put into the output Scantable
    Vector<String> units(1);
    units = String("Hz");
    Bool linear=True;
    SpectralCoordinate sc2 = fa.alignedSpectralCoordinate(linear);
    sc2.setWorldAxisUnits(units);
    const uInt id = out->frequencies().addEntry(sc2.referencePixel()[0],
                                                sc2.referenceValue()[0],
                                                sc2.increment()[0]);
    while ( !fiter.pastEnd() ) {
      
      ftab = fiter.table();
      // spectral coordinate for the current FREQ_ID
      ScalarColumn<uInt> freqidCol2(ftab, "FREQ_ID");
      sC = in->frequencies().getSpectralCoordinate(freqidCol2(0));
      // create the "global" abcissa for alignment with same FREQ_ID
      Vector<Double> abc(nchan);
      for (uInt i=0; i<nchan; i++) {
           Double w;
           sC.toWorld(w,Double(i));
           abc[i] = w;
      }
      TableVector<uInt> tvec(ftab, "FREQ_ID");
      // assign new frequency id to all rows
      tvec = id;
      // cache abcissa for same time stamps, so iterate over those
      TableIterator timeiter(ftab, "TIME");
      while ( !timeiter.pastEnd() ) {
        Table tab = timeiter.table();
        ArrayColumn<Float> specCol(tab, "SPECTRA");
        ArrayColumn<uChar> flagCol(tab, "FLAGTRA");
        MEpoch::ROScalarColumn timeCol(tab, "TIME");
        // use align abcissa cache after the first row
        // these rows should be just be POLNO
        bool first = true;
        for (int i=0; i<int(tab.nrow()); ++i) {
          // input values
          Vector<uChar> flag = flagCol(i);
          Vector<Bool> mask(flag.shape());
          Vector<Float> specOut, spec;
          spec  = specCol(i);
          Vector<Bool> maskOut;Vector<uChar> flagOut;
          convertArray(mask, flag);
          // alignment
          Bool ok = fa.align(specOut, maskOut, abc, spec,
                             mask, timeCol(i), !first,
                             interp, False);
	  (void) ok; // unused stop compiler nagging
          // back into scantable
          flagOut.resize(maskOut.nelements());
          convertArray(flagOut, maskOut);
          flagCol.put(i, flagOut);
          specCol.put(i, specOut);
          // start abcissa caching
          first = false;
        }
        // next timestamp
        ++timeiter;
      }
      // next FREQ_ID
      ++fiter;
    }
    // next aligner
    ++iter;
  }
  // set this afterwards to ensure we are doing insitu correctly.
  out->frequencies().setFrame(system, true);
  return out;
}

CountedPtr<Scantable>
  asap::STMath::convertPolarisation( const CountedPtr<Scantable>& in,
                                     const std::string & newtype )
{
  if (in->npol() != 2 && in->npol() != 4)
    throw(AipsError("Can only convert two or four polarisations."));
  if ( in->getPolType() == newtype )
    throw(AipsError("No need to convert."));
  if ( ! in->selector_.empty() )
    throw(AipsError("Can only convert whole scantable. Unset the selection."));
  bool insitu = insitu_;
  setInsitu(false);
  CountedPtr< Scantable > out = getScantable(in, true);
  setInsitu(insitu);
  Table& tout = out->table();
  tout.rwKeywordSet().define("POLTYPE", String(newtype));

  Block<String> cols(4);
  cols[0] = "SCANNO";
  cols[1] = "CYCLENO";
  cols[2] = "BEAMNO";
  cols[3] = "IFNO";
  TableIterator it(in->originalTable_, cols);
  String basetype = in->getPolType();
  STPol* stpol = STPol::getPolClass(in->factories_, basetype);
  try {
    while ( !it.pastEnd() ) {
      Table tab = it.table();
      uInt row = tab.rowNumbers()[0];
      stpol->setSpectra(in->getPolMatrix(row));
      Float fang,fhand;
      fang = in->focusTable_.getTotalAngle(in->mfocusidCol_(row));
      fhand = in->focusTable_.getFeedHand(in->mfocusidCol_(row));
      stpol->setPhaseCorrections(fang, fhand);
      Int npolout = 0;
      for (uInt i=0; i<tab.nrow(); ++i) {
        Vector<Float> outvec = stpol->getSpectrum(i, newtype);
        if ( outvec.nelements() > 0 ) {
          tout.addRow();
          TableCopy::copyRows(tout, tab, tout.nrow()-1, 0, 1);
          ArrayColumn<Float> sCol(tout,"SPECTRA");
          ScalarColumn<uInt> pCol(tout,"POLNO");
          sCol.put(tout.nrow()-1 ,outvec);
          pCol.put(tout.nrow()-1 ,uInt(npolout));
          npolout++;
       }
      }
      tout.rwKeywordSet().define("nPol", npolout);
      ++it;
    }
  } catch (AipsError& e) {
    delete stpol;
    throw(e);
  }
  delete stpol;
  return out;
}

CountedPtr< Scantable >
  asap::STMath::mxExtract( const CountedPtr< Scantable > & in,
                           const std::string & scantype )
{
  bool insitu = insitu_;
  setInsitu(false);
  CountedPtr< Scantable > out = getScantable(in, true);
  setInsitu(insitu);
  Table& tout = out->table();
  std::string taql = "SELECT FROM $1 WHERE BEAMNO != REFBEAMNO";
  if (scantype == "on") {
    taql = "SELECT FROM $1 WHERE BEAMNO == REFBEAMNO";
  }
  Table tab = tableCommand(taql, in->table());
  TableCopy::copyRows(tout, tab);
  if (scantype == "on") {
    // re-index SCANNO to 0
    TableVector<uInt> vec(tout, "SCANNO");
    vec = 0;
  }
  return out;
}

std::vector<float>
  asap::STMath::fft( const casa::CountedPtr< Scantable > & in, 
		     const std::vector<int>& whichrow, 
		     bool getRealImag )
{
  std::vector<float> res;
  Table tab = in->table();
  std::vector<bool> mask;

  if (whichrow.size() < 1) {          // for all rows (by default)
    int nrow = int(tab.nrow());
    for (int i = 0; i < nrow; ++i) {
      res = in->execFFT(i, mask, getRealImag);
    }
  } else {                           // for specified rows
    for (uInt i = 0; i < whichrow.size(); ++i) {
      res = in->execFFT(i, mask, getRealImag);
    }
  }

  return res;
}


CountedPtr<Scantable>
  asap::STMath::lagFlag( const CountedPtr<Scantable>& in,
                         double start, double end,
                         const std::string& mode )
{
  CountedPtr<Scantable> out = getScantable(in, false);
  Table& tout = out->table();
  TableIterator iter(tout, "FREQ_ID");
  FFTServer<Float,Complex> ffts;

  while ( !iter.pastEnd() ) {
    Table tab = iter.table();
    Double rp,rv,inc;
    ROTableRow row(tab);
    const TableRecord& rec = row.get(0);
    uInt freqid = rec.asuInt("FREQ_ID");
    out->frequencies().getEntry(rp, rv, inc, freqid);
    ArrayColumn<Float> specCol(tab, "SPECTRA");
    ArrayColumn<uChar> flagCol(tab, "FLAGTRA");

    for (int i=0; i<int(tab.nrow()); ++i) {
      Vector<Float> spec = specCol(i);
      Vector<uChar> flag = flagCol(i);
      std::vector<bool> mask;
      for (uInt j = 0; j < flag.nelements(); ++j) {
	mask.push_back(!(flag[j]>0));
      }
      mathutil::doZeroOrderInterpolation(spec, mask);

      Vector<Complex> lags;
      ffts.fft0(lags, spec);

      Int lag0(start+0.5);
      Int lag1(end+0.5);
      if (mode == "frequency") {
        lag0 = Int(spec.nelements()*abs(inc)/(start)+0.5);
        lag1 = Int(spec.nelements()*abs(inc)/(end)+0.5);
      }
      Int lstart =  max(0, lag0);
      Int lend   =  min(Int(lags.nelements()-1), lag1);
      if (lstart == lend) {
        lags[lstart] = Complex(0.0);
      } else {
        if (lstart > lend) {
          Int tmp = lend;
          lend = lstart;
          lstart = tmp;
        }
        for (int j=lstart; j <=lend ;++j) {
          lags[j] = Complex(0.0);
        }
      }

      ffts.fft0(spec, lags);

      specCol.put(i, spec);
    }
    ++iter;
  }
  return out;
}

// Averaging spectra with different channel/resolution
CountedPtr<Scantable>
STMath::new_average( const std::vector<CountedPtr<Scantable> >& in,
		     const bool& compel,
		     const std::vector<bool>& mask,
		     const std::string& weight,
		     const std::string& avmode )
  throw ( casa::AipsError )
{
  LogIO os( LogOrigin( "STMath", "new_average()", WHERE ) ) ;
  if ( avmode == "SCAN" && in.size() != 1 )
    throw(AipsError("Can't perform 'SCAN' averaging on multiple tables.\n"
                    "Use merge first."));
  
  CountedPtr<Scantable> out ;     // processed result 
  if ( compel ) {
    std::vector< CountedPtr<Scantable> > newin ; // input for average process
    uInt insize = in.size() ;    // number of input scantables 

    // setup newin
    bool oldInsitu = insitu_ ;
    setInsitu( false ) ;
    newin.resize( insize ) ;
    for ( uInt itable = 0 ; itable < insize ; itable++ ) {
      newin[itable] = getScantable( in[itable], false ) ;
    }
    setInsitu( oldInsitu ) ;

    // warning
    os << "Average spectra with different spectral resolution" << LogIO::POST ;

    // temporarily set coordinfo
    vector<string> oldinfo( insize ) ;
    for ( uInt itable = 0 ; itable < insize ; itable++ ) {
      vector<string> coordinfo = in[itable]->getCoordInfo() ;
      oldinfo[itable] = coordinfo[0] ;
      coordinfo[0] = "Hz" ;
      newin[itable]->setCoordInfo( coordinfo ) ;
    }

    ostringstream oss ;

    // check IF frequency coverage
    // freqid: list of FREQ_ID, which is used, in each table  
    // iffreq: list of minimum and maximum frequency for each FREQ_ID in 
    //         each table
    // freqid[insize][numIF]
    // freqid: [[id00, id01, ...],
    //          [id10, id11, ...],
    //          ...
    //          [idn0, idn1, ...]]
    // iffreq[insize][numIF*2]
    // iffreq: [[min_id00, max_id00, min_id01, max_id01, ...],
    //          [min_id10, max_id10, min_id11, max_id11, ...],
    //          ...
    //          [min_idn0, max_idn0, min_idn1, max_idn1, ...]]
    //os << "Check IF settings in each table" << LogIO::POST ;
    vector< vector<uInt> > freqid( insize );
    vector< vector<double> > iffreq( insize ) ;
    for ( uInt itable = 0 ; itable < insize ; itable++ ) {
      Vector<uInt> freqIds = newin[itable]->mfreqidCol_.getColumn() ;
      vector<uInt> uniqueFreqId = newin[itable]->getNumbers(newin[itable]->mfreqidCol_) ;
      for ( vector<uInt>::iterator i = uniqueFreqId.begin() ;
            i != uniqueFreqId.end() ; i++ ) {
        //os << "itable = " << itable << ": IF " << id << " is included in the list" << LogIO::POST ;
        uInt target = 0 ;
        while ( freqIds[target] != *i )
          target++ ;
        vector<double> abcissa = newin[itable]->getAbcissa( target ) ;
        freqid[itable].push_back( *i ) ;
        double incr = abs( abcissa[1] - abcissa[0] ) ;
        iffreq[itable].push_back( (*min_element(abcissa.begin(),abcissa.end()))-0.5*incr ) ;
        iffreq[itable].push_back( (*max_element(abcissa.begin(),abcissa.end()))+0.5*incr ) ;
      }
    }

    // debug
//     os << "IF settings summary:" << endl ;
//     for ( uInt i = 0 ; i < freqid.size() ; i++ ) {
//       os << "   Table" << i << endl ;
//       for ( uInt j = 0 ; j < freqid[i].size() ; j++ ) {
//         os << "      id = " << freqid[i][j] << " (min,max) = (" << iffreq[i][2*j] << "," << iffreq[i][2*j+1] << ")" << endl ;
//       }
//     }
//     os << endl ;
//     os.post() ;

    // IF grouping based on their frequency coverage
    // ifgrp: number of member in each IF group
    // ifgrp[numgrp]
    // ifgrp: [n0, n1, ...]
    //os << "IF grouping based on their frequency coverage" << LogIO::POST ;

    // parameter for IF grouping
    // groupmode = OR    retrieve all region 
    //             AND   only retrieve overlaped region
    //string groupmode = "AND" ;
    string groupmode = "OR" ;
    uInt sizecr = 0 ;
    if ( groupmode == "AND" ) 
      sizecr = 1 ;
    else if ( groupmode == "OR" ) 
      sizecr = 0 ;

    vector<double> sortedfreq ;
    for ( uInt i = 0 ; i < iffreq.size() ; i++ ) {
      for ( uInt j = 0 ; j < iffreq[i].size() ; j++ ) {
	if ( count( sortedfreq.begin(), sortedfreq.end(), iffreq[i][j] ) == 0 )
	  sortedfreq.push_back( iffreq[i][j] ) ;
      }
    }
    sort( sortedfreq.begin(), sortedfreq.end() ) ;
    vector<uInt> ifgrp( sortedfreq.size()-1, 0 ) ;
    for ( uInt itable = 0 ; itable < insize ; itable++ ) {
      for ( uInt iif = 0 ; iif < freqid[itable].size() ; iif++ ) {
	double range0 = iffreq[itable][2*iif] ;
	double range1 = iffreq[itable][2*iif+1] ;
        for ( uInt j = 0 ; j < sortedfreq.size()-1 ; j++ ) {
          double fmin = max( range0, sortedfreq[j] ) ;
          double fmax = min( range1, sortedfreq[j+1] ) ;
          if ( fmin < fmax ) {
            ifgrp[j]++ ;
          }
        }
      }
    }

    // Grouping continuous IF groups (without frequency gap)
    // freqgrp: list of IF group indexes in each frequency group
    // freqgrp[numgrp][nummember]
    // freqgrp: [[ifgrp00, ifgrp01, ifgrp02, ...],
    //           [ifgrp10, ifgrp11, ifgrp12, ...],
    //           ...
    //           [ifgrpn0, ifgrpn1, ifgrpn2, ...]]
    // grprange[2*numgrp]
    // grprange: [fmin0,fmax0,fmin1,fmax1,...]
    vector< vector<uInt> > freqgrp ;
    vector<double> grprange ;
    vector<uInt> grpedge ;
    for ( uInt igrp = 0 ; igrp < ifgrp.size() ; igrp++ ) {
      if ( ifgrp[igrp] <= sizecr ) {
        grpedge.push_back( igrp ) ;
      }
    }
    grpedge.push_back( ifgrp.size() ) ;
    uInt itmp = 0 ;
    for ( uInt i = 0 ; i < grpedge.size() ; i++ ) {
      int n = grpedge[i] - itmp ;
      if ( n > 0 ) {
        vector<uInt> members( n ) ;
        for ( int j = 0 ; j < n ; j++ ) {
          members[j] = itmp+j ;
        }
        freqgrp.push_back( members ) ;
        grprange.push_back( sortedfreq[itmp] ) ;
        grprange.push_back( sortedfreq[grpedge[i]] ) ;
      }
      itmp += n + 1 ;
    }

    // print frequency group
    oss.str("") ;
    oss << "Frequency Group summary: " << endl ;
    oss << "   GROUP_ID: [FREQ_MIN, FREQ_MAX]" << endl ;
    for ( uInt i = 0 ; i < freqgrp.size() ; i++ ) {
      oss << "   GROUP " << setw( 2 ) << i << ": [" << grprange[2*i] << "," << grprange[2*i+1] << "]" ;
      oss << endl ;
    }
    oss << endl ;
    os << oss.str() << LogIO::POST ;

    // groups: list of frequency group index whose frequency range overlaps 
    //         with that of each table and IF 
    // groups[numtable][numIF]
    // groups: [[grpx, grpy,...],
    //          [grpa, grpb,...],
    //          ...
    //          [grpk, grpm,...]]
    vector< vector<uInt> > groups( insize ) ;
    for ( uInt i = 0 ; i < insize ; i++ ) {
      groups[i].resize( freqid[i].size() ) ;
    }
    for ( uInt itable = 0 ; itable < insize ; itable++ ) {
      for ( uInt ifreq = 0 ; ifreq < freqid[itable].size() ; ifreq++ ) {
        double minf = iffreq[itable][2*ifreq] ;
        uInt groupid ;
        for ( uInt igrp = 0 ; igrp < freqgrp.size() ; igrp++ ) {
          vector<uInt> memberlist = freqgrp[igrp] ;
          if ( (minf >= grprange[2*igrp]) && (minf <= grprange[2*igrp+1]) ) {
            groupid = igrp ;
            break ;
          }
        }
        groups[itable][ifreq] = groupid ;
      }
    }
                                                          

    // print membership
    oss.str("") ;
    for ( uInt i = 0 ; i < insize ; i++ ) {
      oss << "Table " << i << endl ;
      for ( uInt j = 0 ; j < groups[i].size() ; j++ ) {
        oss << "   FREQ_ID " <<  setw( 2 ) << freqid[i][j] << ": " ;
        oss << setw( 2 ) << groups[i][j] ;
        oss << endl ;
      }
    }
    os << oss.str() << LogIO::POST ;

    // reset SCANNO and IFNO/FREQ_ID: IF is reset by the result of sortation 
    //os << "All IF number is set to IF group index" << LogIO::POST ;
    // reset SCANNO only when avmode != "SCAN"
    if ( avmode != "SCAN" ) {
      os << "All scan number is set to 0" << LogIO::POST ;
      for ( uInt itable = 0 ; itable < insize ; itable++ ) {
        uInt nrow = newin[itable]->nrow() ;
        Vector<uInt> resetScan( nrow, 0 ) ;
        newin[itable]->scanCol_.putColumn( resetScan ) ;
      }
    }

    // reset spectra and flagtra: align spectral resolution
    //os << "Align spectral resolution" << LogIO::POST ;
    // gmaxdnu: the coarsest frequency resolution in the frequency group
    // gminfreq: lower frequency edge of the frequency group
    // gnchan: number of channels for the frequency group
    vector<double> gmaxdnu( freqgrp.size(), 0.0 ) ;
    vector<double> gminfreq( freqgrp.size() ) ;
    vector<double> gnchan( freqgrp.size() ) ;
    for ( uInt i = 0 ; i < insize ; i++ ) {
      vector<uInt> members = groups[i] ;
      for ( uInt j = 0 ; j < members.size() ; j++ ) {
        uInt groupid = members[j] ;
        Double rp,rv,ic ;
        newin[i]->frequencies().getEntry( rp, rv, ic, j ) ;
        if ( abs(ic) > abs(gmaxdnu[groupid]) ) 
          gmaxdnu[groupid] = ic ;
      }
    }
    for ( uInt igrp = 0 ; igrp < freqgrp.size() ; igrp++ ) {
      gminfreq[igrp] = grprange[2*igrp] ;
      double maxfreq = grprange[2*igrp+1] ;
      gnchan[igrp] = (int)(abs((maxfreq-gminfreq[igrp])/gmaxdnu[igrp])+0.9) ;
    }
      
    // regrid spectral data and update frequency info
    for ( uInt itable = 0 ; itable < insize ; itable++ ) {
      Vector<uInt> oldFreqId = newin[itable]->mfreqidCol_.getColumn() ;
      Vector<uInt> newFreqId( oldFreqId.nelements() ) ;

      // update MAIN
      for ( uInt irow = 0 ; irow < newin[itable]->nrow() ; irow++ ) {
        uInt groupid = groups[itable][oldFreqId[irow]] ;
        newFreqId[irow] = groupid ;
        newin[itable]->regridChannel( gnchan[groupid], 
                                      gmaxdnu[groupid], 
                                      gminfreq[groupid], 
                                      irow ) ;
      }
      newin[itable]->mfreqidCol_.putColumn( newFreqId ) ;
      newin[itable]->ifCol_.putColumn( newFreqId ) ;

      // update FREQUENCIES
      Table tab = newin[itable]->frequencies().table() ;
      ScalarColumn<uInt> fIdCol( tab, "ID" ) ;
      ScalarColumn<Double> fRefPixCol( tab, "REFPIX" ) ;
      ScalarColumn<Double> fRefValCol( tab, "REFVAL" ) ;
      ScalarColumn<Double> fIncrCol( tab, "INCREMENT" ) ;
      if ( freqgrp.size() > tab.nrow() ) {
        tab.addRow( freqgrp.size()-tab.nrow() ) ;
      }
      for ( uInt irow = 0 ; irow < freqgrp.size() ; irow++ ) {
        Double refval = gminfreq[irow] + 0.5 * abs(gmaxdnu[irow]) ;
        Double refpix = (gmaxdnu[irow] > 0.0) ? 0 : gnchan[irow]-1 ;
        Double increment = gmaxdnu[irow] ;
        fIdCol.put( irow, irow ) ;
        fRefPixCol.put( irow, refpix ) ;
        fRefValCol.put( irow, refval ) ;
        fIncrCol.put( irow, increment ) ;
      }
    }

    // set back coordinfo
    for ( uInt itable = 0 ; itable < insize ; itable++ ) {
      vector<string> coordinfo = newin[itable]->getCoordInfo() ;
      coordinfo[0] = oldinfo[itable] ;
      newin[itable]->setCoordInfo( coordinfo ) ;
    }

    // average
    out = average( newin, mask, weight, avmode ) ;
  }
  else {
    // simple average
    out =  average( in, mask, weight, avmode ) ;
  }
  
  return out;
}

CountedPtr<Scantable> STMath::cwcal( const CountedPtr<Scantable>& s,
                                     const String calmode, 
                                     const String antname )
{
  // frequency switch
  if ( calmode == "fs" ) {
    return cwcalfs( s, antname ) ;
  }
  else {
    vector<bool> masks = s->getMask( 0 ) ;
    vector<int> types ;

    // save original table selection
    Table torg  = s->table_ ;

    // sky scan
    bool insitu = insitu_ ;
    insitu_ = false ;
    // share calibration scans before average with out
    CountedPtr<Scantable> out = getScantable( s, true ) ;
    insitu_ = insitu ;
    out->table_ = s->table_( s->table_.col("SRCTYPE") == (Int)SrcType::SKY ) ;
    out->attach() ;
    CountedPtr<Scantable> asky = averageWithinSession( out, 
                                                       masks,
                                                       "TINT" ) ;
    // hot scan
    out->table_ = s->table_( s->table_.col("SRCTYPE") == (Int)SrcType::HOT ) ;
    out->attach() ;
    CountedPtr<Scantable> ahot = averageWithinSession( out, 
                                                       masks,
                                                       "TINT" ) ;
    // cold scan
    CountedPtr<Scantable> acold ;
//     out->table_ = s->table_( s->table_.col("SRCTYPE") == (Int)SrcType::COLD ) ;
//     out->attach() ;
//     CountedPtr<Scantable> acold = averageWithinSession( out, 
//                                                         masks,
//                                                         "TINT" ) ;

    // off scan
    out->table_ = s->table_( s->table_.col("SRCTYPE") == (Int)SrcType::PSOFF ) ;
    out->attach() ;
    CountedPtr<Scantable> aoff = averageWithinSession( out, 
                                                       masks,
                                                       "TINT" ) ;
    
    // on scan
    s->table_ = s->table_( s->table_.col("SRCTYPE") == (Int)SrcType::PSON ) ;
    s->attach() ;
    out->table_ = out->originalTable_ ;
    out->attach() ;
    out->table().addRow( s->nrow() ) ;
    copyRows( out->table(), s->table(), 0, 0, s->nrow(), False, True, False ) ;
    
    // iterate throgh STIdxIter2
    ChopperWheelCalibrator cal(out, s, aoff, asky, ahot, acold);
    STIdxIter2::Iterate<ChopperWheelCalibrator>(cal, "BEAMNO,POLNO,IFNO");

    s->table_ = torg ;
    s->attach() ;

    // flux unit
    out->setFluxUnit( "K" ) ;

    return out ;
  }
}
  
CountedPtr<Scantable> STMath::almacal( const CountedPtr<Scantable>& s,
                                       const String calmode )
{
  // frequency switch
  if ( calmode == "fs" ) {
    return almacalfs( s ) ;
  }
  else {
//     double t0, t1 ;
//     t0 = mathutil::gettimeofday_sec() ;
    vector<bool> masks = s->getMask( 0 ) ;

    // save original table selection
    Table torg = s->table_ ;

    // off scan
    // TODO 2010/01/08 TN
    // Grouping by time should be needed before averaging.
    // Each group must have own unique SCANNO (should be renumbered).
    // See PIPELINE/SDCalibration.py
    bool insitu = insitu_ ;
    insitu_ = false ;
    // share off scan before average with out
    CountedPtr<Scantable> out = getScantable( s, true ) ;
    out->table_ = s->table_( s->table_.col("SRCTYPE") == (Int)SrcType::PSOFF ) ;
    out->attach() ;
    insitu_ = insitu ;
    CountedPtr<Scantable> aoff = averageWithinSession( out, 
                                                       masks,
                                                       "TINT" ) ;

    // on scan
//     t0 = mathutil::gettimeofday_sec() ;
    s->table_ = s->table_( s->table_.col("SRCTYPE") == (Int)SrcType::PSON ) ;
    s->attach() ;
    out->table_ = out->originalTable_ ;
    out->attach() ;
    out->table().addRow( s->nrow() ) ;
    copyRows( out->table(), s->table(), 0, 0, s->nrow(), False ) ;
//     t1 = mathutil::gettimeofday_sec() ;
//     cout << "elapsed time for preparing output table: " << t1-t0 << " sec" << endl ;

    // process each on scan
//     t0 = mathutil::gettimeofday_sec() ;

    // iterate throgh STIdxIter2
    AlmaCalibrator cal(out, s, aoff);
    STIdxIter2::Iterate<AlmaCalibrator>(cal, "BEAMNO,POLNO,IFNO");

    // finalize
    s->table_ = torg ;
    s->attach() ;

//     t1 = mathutil::gettimeofday_sec() ;
//     cout << "elapsed time for calibration: " << t1-t0 << " sec" << endl ;

    // flux unit
    out->setFluxUnit( "K" ) ;

    return out ;
  }
}

CountedPtr<Scantable> STMath::cwcalfs( const CountedPtr<Scantable>& s,
                                       const String antname )
{
  vector<int> types ;

  // APEX calibration mode
  int apexcalmode = 1 ;
 
  if ( antname.find( "APEX" ) != string::npos ) {
    // check if off scan exists or not
    STSelector sel = STSelector() ;
    //sel.setName( offstr1 ) ;
    types.push_back( SrcType::FLOOFF ) ;
    sel.setTypes( types ) ;
    try {
      s->setSelection( sel ) ;
    }
    catch ( AipsError &e ) {
      apexcalmode = 0 ;
    }
    sel.reset() ;
  }
  s->unsetSelection() ;
  types.clear() ;

  vector<bool> masks = s->getMask( 0 ) ;
  CountedPtr<Scantable> ssig, sref ;
  //CountedPtr<Scantable> out ;
  bool insitu = insitu_ ;
  insitu_ = False ;
  CountedPtr<Scantable> out = getScantable( s, true ) ;
  insitu_ = insitu ;

  if ( antname.find( "APEX" ) != string::npos ) {
    // APEX calibration
    // sky scan
    out->table_ = s->table_( s->table_.col("SRCTYPE") == (Int)SrcType::FLOSKY ) ;
    out->attach() ;
    CountedPtr<Scantable> askylo = averageWithinSession( out, 
                                                         masks,
                                                         "TINT" ) ;
    out->table_ = s->table_( s->table_.col("SRCTYPE") == (Int)SrcType::FHISKY ) ;
    out->attach() ;
    CountedPtr<Scantable> askyhi = averageWithinSession( out, 
                                                         masks,
                                                         "TINT" ) ;
    
    // hot scan
    out->table_ = s->table_( s->table_.col("SRCTYPE") == (Int)SrcType::FLOHOT ) ;
    out->attach() ;
    CountedPtr<Scantable> ahotlo = averageWithinSession( out, 
                                                         masks,
                                                         "TINT" ) ;
    out->table_ = s->table_( s->table_.col("SRCTYPE") == (Int)SrcType::FHIHOT ) ;
    out->attach() ;
    CountedPtr<Scantable> ahothi = averageWithinSession( out, 
                                                         masks,
                                                         "TINT" ) ;
    
    // cold scan
    CountedPtr<Scantable> acoldlo, acoldhi ;
//     out->table_ = s->table_( s->table_.col("SRCTYPE") == (Int)SrcType::FLOCOLD ) ;
//     out->attach() ;
//     CountedPtr<Scantable> acoldlo = averageWithinSession( out, 
//                                                           masks,
//                                                           "TINT" ) ;
//     out->table_ = s->table_( s->table_.col("SRCTYPE") == (Int)SrcType::FHICOLD ) ;
//     out->attach() ;
//     CountedPtr<Scantable> acoldhi = averageWithinSession( out, 
//                                                           masks,
//                                                           "TINT" ) ;

    // ref scan
    insitu_ = false ;
    sref = getScantable( s, true ) ;
    CountedPtr<Scantable> rref = getScantable( s, true ) ;
    insitu_ = insitu ;
    rref->table_ = s->table_( s->table_.col("SRCTYPE") == (Int)SrcType::FSLO ) ;
    rref->attach() ;
    copyRows( sref->table_, rref->table_, 0, 0, rref->nrow(), False, True, False ) ;
    
    // sig scan
    insitu_ = false ;
    ssig = getScantable( s, true ) ;
    CountedPtr<Scantable> rsig = getScantable( s, true ) ;
    insitu_ = insitu ;
    rsig->table_ = s->table_( s->table_.col("SRCTYPE") == (Int)SrcType::FSHI ) ;
    rsig->attach() ;
    copyRows( ssig->table_, rsig->table_, 0, 0, rsig->nrow(), False, True, False ) ;
          
    if ( apexcalmode == 0 ) {
      // using STIdxIter2 
      vector<string> cols( 3 ) ;
      cols[0] = "BEAMNO" ;
      cols[1] = "POLNO" ;
      cols[2] = "IFNO" ;
      STIdxIter2 iter(ssig, cols) ;
      STSelector sel ;
      vector< CountedPtr<Scantable> > on( 2 ) ;
      on[0] = rsig ;
      on[1] = rref ;
      vector< CountedPtr<Scantable> > sky( 2 ) ;
      sky[0] = askylo ;
      sky[1] = askyhi ;
      vector< CountedPtr<Scantable> > hot( 2 ) ;
      hot[0] = ahotlo ;
      hot[1] = ahothi ;
      vector< CountedPtr<Scantable> > cold( 2 ) ;
      while ( !iter.pastEnd() ) {
        Record ids = iter.currentValue() ;
        stringstream ss ;
        ss << "SELECT FROM $1 WHERE "
           << "BEAMNO==" << ids.asuInt("BEAMNO") << "&&"
           << "POLNO==" << ids.asuInt("POLNO") << "&&"
           << "IFNO==" << ids.asuInt("IFNO") ;
        //cout << "TaQL string: " << ss.str() << endl ;
        sel.setTaQL( ss.str() ) ;
        sky[0]->setSelection( sel ) ;
        sky[1]->setSelection( sel ) ;
        hot[0]->setSelection( sel ) ;
        hot[1]->setSelection( sel ) ;
        Vector<uInt> rows = iter.getRows( SHARE ) ;
        calibrateAPEXFS( ssig, sref, on, sky, hot, cold, rows ) ;
        sky[0]->unsetSelection() ;
        sky[1]->unsetSelection() ;
        hot[0]->unsetSelection() ;
        hot[1]->unsetSelection() ;
        sel.reset() ;
        iter.next() ;
      }

    }
    else if ( apexcalmode == 1 ) {
      // APEX fs data with off scan
      // off scan
      out->table_ = s->table_( s->table_.col("SRCTYPE") == (Int)SrcType::FLOOFF ) ;
      out->attach() ;
      CountedPtr<Scantable> aofflo = averageWithinSession( out, 
                                                           masks,
                                                           "TINT" ) ;
      out->table_ = s->table_( s->table_.col("SRCTYPE") == (Int)SrcType::FHIOFF ) ;
      out->attach() ;
      CountedPtr<Scantable> aoffhi = averageWithinSession( out, 
                                                           masks,
                                                           "TINT" ) ;
      
      // process each sig and ref scan
      // iterate throgh STIdxIter2
      ChopperWheelCalibrator cal_sig(ssig, rsig, aofflo, askylo, ahotlo, acoldlo);
      STIdxIter2::Iterate<ChopperWheelCalibrator>(cal_sig, "BEAMNO,POLNO,IFNO");
      ChopperWheelCalibrator cal_ref(sref, rref, aoffhi, askyhi, ahothi, acoldhi);
      STIdxIter2::Iterate<ChopperWheelCalibrator>(cal_ref, "BEAMNO,POLNO,IFNO");
    }
  }
  else {
    // non-APEX fs data
    // sky scan
    out->table_ = s->table_( s->table_.col("SRCTYPE") == (Int)SrcType::SKY ) ;
    out->attach() ;
    CountedPtr<Scantable> asky = averageWithinSession( out, 
                                                       masks,
                                                       "TINT" ) ;
    STSelector sel = STSelector() ;

    // hot scan
    out->table_ = s->table_( s->table_.col("SRCTYPE") == (Int)SrcType::HOT ) ;
    out->attach() ;
    CountedPtr<Scantable> ahot = averageWithinSession( out, 
                                                       masks,
                                                       "TINT" ) ;

    // cold scan
    CountedPtr<Scantable> acold ;
//     out->table_ = s->table_( s->table_.col("SRCTYPE") == (Int)SrcType::COLD ) ;
//     out->attach() ;
//     CountedPtr<Scantable> acold = averageWithinSession( out, 
//                                                         masks,
//                                                         "TINT" ) ;
   
    // ref scan
    bool insitu = insitu_ ;
    insitu_ = false ;
    sref = getScantable( s, true ) ;
    CountedPtr<Scantable> rref = getScantable( s, true ) ;
    insitu_ = insitu ;
    rref->table_ = s->table_( s->table_.col("SRCTYPE") == (Int)SrcType::PSOFF ) ;
    rref->attach() ;
    copyRows( sref->table_, rref->table_, 0, 0, rref->nrow(), False, True, False ) ;
    
    // sig scan
    insitu_ = false ;
    ssig = getScantable( s, true ) ;
    CountedPtr<Scantable> rsig = getScantable( s, true ) ;
    insitu_ = insitu ;
    rsig->table_ = s->table_( s->table_.col("SRCTYPE") == (Int)SrcType::PSON ) ;
    rsig->attach() ;
    copyRows( ssig->table_, rsig->table_, 0, 0, rsig->nrow(), False, True, False ) ;

    // process each sig and ref scan
    vector<string> cols( 3 ) ;
    cols[0] = "BEAMNO" ;
    cols[1] = "POLNO" ;
    cols[2] = "IFNO" ;
    STIdxIter2 iter(ssig, cols);
    while ( !iter.pastEnd() ) {
      Record ids = iter.currentValue() ;
      stringstream ss ;
      ss << "SELECT FROM $1 WHERE "
         << "BEAMNO==" << ids.asuInt("BEAMNO") << "&&"
         << "POLNO==" << ids.asuInt("POLNO") << "&&"
         << "IFNO==" << ids.asuInt("IFNO") ;
      //cout << "TaQL string: " << ss.str() << endl ;
      sel.setTaQL( ss.str() ) ;
      ahot->setSelection( sel ) ;
      asky->setSelection( sel ) ;
      Vector<uInt> rows = iter.getRows( SHARE ) ;
      // out should be an exact copy of s except that SPECTRA column is empty
      calibrateFS( ssig, sref, rsig, rref, asky, ahot, acold, rows ) ; 
      ahot->unsetSelection() ;
      asky->unsetSelection() ;
      sel.reset() ;
      iter.next() ;
    }
  }

  // do folding if necessary
  Table sigtab = ssig->table() ;
  Table reftab = sref->table() ;
  ScalarColumn<uInt> reffidCol ;
  Int nchan = (Int)ssig->nchan() ;
  reffidCol.attach( reftab, "FREQ_ID" ) ;
  Vector<uInt> sfids = ssig->mfreqidCol_.getColumn() ;
  Vector<uInt> rfids = sref->mfreqidCol_.getColumn() ;
  vector<uInt> sfids_unique ;
  vector<uInt> rfids_unique ;
  vector<uInt> sifno_unique ;
  vector<uInt> rifno_unique ;
  for ( uInt i = 0 ; i < sfids.nelements() ; i++ ) {
    if ( count( sfids_unique.begin(), sfids_unique.end(), sfids[i] ) == 0 ) {
      sfids_unique.push_back( sfids[i] ) ;
      sifno_unique.push_back( ssig->getIF( i ) ) ;
    }
    if ( count( rfids_unique.begin(), rfids_unique.end(),  rfids[i] ) == 0 ) {
      rfids_unique.push_back( rfids[i] ) ;
      rifno_unique.push_back( sref->getIF( i ) ) ;
    }
  }
  double refpix_sig, refval_sig, increment_sig ;
  double refpix_ref, refval_ref, increment_ref ;
  vector< CountedPtr<Scantable> > tmp( sfids_unique.size() ) ;
  for ( uInt i = 0 ; i < sfids_unique.size() ; i++ ) {
    ssig->frequencies().getEntry( refpix_sig, refval_sig, increment_sig, sfids_unique[i] ) ;
    sref->frequencies().getEntry( refpix_ref, refval_ref, increment_ref, rfids_unique[i] ) ;
    if ( refpix_sig == refpix_ref ) {
      double foffset = refval_ref - refval_sig ;
      int choffset = static_cast<int>(foffset/increment_sig) ;
      double doffset = foffset / increment_sig ;
      if ( abs(choffset) >= nchan ) {
        LogIO os( LogOrigin( "STMath", "cwcalfs", WHERE ) ) ;
        os << "FREQ_ID=[" << sfids_unique[i] << "," << rfids_unique[i] << "]: out-band frequency switching, no folding" << LogIO::POST ;
        os << "Just return signal data" << LogIO::POST ;
        //std::vector< CountedPtr<Scantable> > tabs ;
        //tabs.push_back( ssig ) ;
        //tabs.push_back( sref ) ;
        //out = merge( tabs ) ;
        tmp[i] = ssig ;
      }
      else {
        STSelector sel = STSelector() ;
        vector<int> v( 1, sifno_unique[i] ) ; 
        sel.setIFs( v ) ;
        ssig->setSelection( sel ) ;
        sel.reset() ;
        v[0] = rifno_unique[i] ;
        sel.setIFs( v ) ;
        sref->setSelection( sel ) ;
        sel.reset() ;
        if ( antname.find( "APEX" ) != string::npos ) {
          tmp[i] = dofold( ssig, sref, 0.5*doffset, -0.5*doffset ) ;
          //tmp[i] = dofold( ssig, sref, doffset ) ;
        }
        else {
          tmp[i] = dofold( ssig, sref, doffset ) ;
        }
        ssig->unsetSelection() ;
        sref->unsetSelection() ;
      }
    }
  }

  if ( tmp.size() > 1 ) {
    out = merge( tmp ) ;
  }
  else {
    out = tmp[0] ;
  }

  // flux unit
  out->setFluxUnit( "K" ) ;

  return out ;
}

CountedPtr<Scantable> STMath::almacalfs( const CountedPtr<Scantable>& s )
{
  (void) s; //currently unused
  CountedPtr<Scantable> out ;

  return out ;
}

void STMath::calibrateAPEXFS( CountedPtr<Scantable> &sig,
                              CountedPtr<Scantable> &ref,
                              const vector< CountedPtr<Scantable> >& on,
                              const vector< CountedPtr<Scantable> >& sky,
                              const vector< CountedPtr<Scantable> >& hot,
                              const vector< CountedPtr<Scantable> >& cold,
                              const Vector<uInt> &rows )
{
  // if rows is empty, just return
  if ( rows.nelements() == 0 )
    return ;
  ROScalarColumn<Double> timeCol( sky[0]->table(), "TIME" ) ;
  Vector<Double> timeSkyS = timeCol.getColumn() ;
  timeCol.attach( sky[1]->table(), "TIME" ) ;
  Vector<Double> timeSkyR = timeCol.getColumn() ;
  timeCol.attach( hot[0]->table(), "TIME" ) ;
  Vector<Double> timeHotS = timeCol.getColumn() ;
  timeCol.attach( hot[1]->table(), "TIME" ) ;
  Vector<Double> timeHotR = timeCol.getColumn() ;
  timeCol.attach( sig->table(), "TIME" ) ;
  ROScalarColumn<Double> timeCol2( ref->table(), "TIME" ) ; 
  ROArrayColumn<Float> arrayFloatCol( sky[0]->table(), "SPECTRA" ) ;
  SpectralData skyspectraS(arrayFloatCol.getColumn());
  arrayFloatCol.attach( sky[1]->table(), "SPECTRA" ) ;
  SpectralData skyspectraR(arrayFloatCol.getColumn());
  arrayFloatCol.attach( hot[0]->table(), "SPECTRA" ) ;
  SpectralData hotspectraS(arrayFloatCol.getColumn());
  arrayFloatCol.attach( hot[1]->table(), "SPECTRA" ) ;
  SpectralData hotspectraR(arrayFloatCol.getColumn());
  TcalData tcaldataS(sky[0]);
  TsysData tsysdataS(sky[0]);
  TcalData tcaldataR(sky[1]);
  TsysData tsysdataR(sky[1]);
  unsigned int spsize = sig->nchan( sig->getIF(rows[0]) ) ;
  Vector<Float> spec( spsize ) ;
  // I know that the data is contiguous
  const uInt *p = rows.data() ;
  vector<int> ids( 2 ) ;
  Block<uInt> flagchan( spsize ) ;
  uInt nflag = 0 ;
  for ( int irow = 0 ; irow < rows.nelements() ; irow++ ) {
    double reftime = timeCol.asdouble(*p) ;
    ids = getRowIdFromTime( reftime, timeSkyS ) ;
    Vector<Float> spskyS = SimpleInterpolationHelper<SpectralData>::GetFromTime(reftime, timeSkyS, ids, skyspectraS, "linear");
    Vector<Float> tcalS = SimpleInterpolationHelper<TcalData>::GetFromTime(reftime, timeSkyS, ids, tcaldataS, "linear");
    Vector<Float> tsysS = SimpleInterpolationHelper<TsysData>::GetFromTime(reftime, timeSkyS, ids, tsysdataS, "linear");
    ids = getRowIdFromTime( reftime, timeHotS ) ;
    Vector<Float> sphotS = SimpleInterpolationHelper<SpectralData>::GetFromTime(reftime, timeHotS, ids, hotspectraS, "linear");
    reftime = timeCol2.asdouble(*p) ;
    ids = getRowIdFromTime( reftime, timeSkyR ) ;
    Vector<Float> spskyR = SimpleInterpolationHelper<SpectralData>::GetFromTime(reftime, timeSkyR, ids, skyspectraR, "linear");
    Vector<Float> tcalR = SimpleInterpolationHelper<TcalData>::GetFromTime(reftime, timeSkyR, ids, tcaldataR, "linear");
    Vector<Float> tsysR = SimpleInterpolationHelper<TsysData>::GetFromTime(reftime, timeSkyR, ids, tsysdataR, "linear");
    ids = getRowIdFromTime( reftime, timeHotR ) ;
    Vector<Float> sphotR = SimpleInterpolationHelper<SpectralData>::GetFromTime(reftime, timeHotR, ids, hotspectraR, "linear");
    Vector<Float> spsig = on[0]->specCol_( *p ) ;
    Vector<Float> spref = on[1]->specCol_( *p ) ;
    for ( unsigned int j = 0 ; j < spsize ; j++ ) {
      if ( (sphotS[j]-spskyS[j]) == 0.0 || (sphotR[j]-spskyR[j]) == 0.0 ) {
        spec[j] = 0.0 ;
        flagchan[nflag++] = j ;
      }
      else {
        spec[j] = tcalS[j] * spsig[j] / ( sphotS[j] - spskyS[j] ) 
          - tcalR[j] * spref[j] / ( sphotR[j] - spskyR[j] ) ;
      }
    }
    sig->specCol_.put( *p, spec ) ;
    sig->tsysCol_.put( *p, tsysS ) ;
    spec *= (Float)-1.0 ;
    ref->specCol_.put( *p, spec ) ;
    ref->tsysCol_.put( *p, tsysR ) ;    
    if ( nflag > 0 ) {
      Vector<uChar> flsig = sig->flagsCol_( *p ) ;
      Vector<uChar> flref = ref->flagsCol_( *p ) ;
      for ( unsigned int j = 0 ; j < nflag ; j++ ) {
        flsig[flagchan[j]] = (uChar)True ;
        flref[flagchan[j]] = (uChar)True ;
      }
      sig->flagsCol_.put( *p, flsig ) ;
      ref->flagsCol_.put( *p, flref ) ;
    }
    nflag = 0 ;
    p++ ;
  }
}

void STMath::calibrateFS( CountedPtr<Scantable> &sig,
                          CountedPtr<Scantable> &ref,
                          const CountedPtr<Scantable>& rsig,
                          const CountedPtr<Scantable>& rref,
                          const CountedPtr<Scantable>& sky,
                          const CountedPtr<Scantable>& hot,
                          const CountedPtr<Scantable>& cold,
                          const Vector<uInt> &rows )
{
  // if rows is empty, just return
  if ( rows.nelements() == 0 )
    return ;
  ROScalarColumn<Double> timeCol( sky->table(), "TIME" ) ;
  Vector<Double> timeSky = timeCol.getColumn() ;
  timeCol.attach( hot->table(), "TIME" ) ;
  Vector<Double> timeHot = timeCol.getColumn() ;
  timeCol.attach( sig->table(), "TIME" ) ;
  ROScalarColumn<Double> timeCol2( ref->table(), "TIME" ) ; 
  ROArrayColumn<Float> arrayFloatCol( sky->table(), "SPECTRA" ) ;
  SpectralData skyspectra(arrayFloatCol.getColumn());
  arrayFloatCol.attach( hot->table(), "SPECTRA" ) ;
  SpectralData hotspectra(arrayFloatCol.getColumn());
  TcalData tcaldata(sky);
  TsysData tsysdata(sky);
  unsigned int spsize = sig->nchan( sig->getIF(rows[0]) ) ;
  Vector<Float> spec( spsize ) ;
  // I know that the data is contiguous
  const uInt *p = rows.data() ;
  vector<int> ids( 2 ) ;
  Block<uInt> flagchan( spsize ) ;
  uInt nflag = 0 ;
  for ( int irow = 0 ; irow < rows.nelements() ; irow++ ) {
    double reftime = timeCol.asdouble(*p) ;
    ids = getRowIdFromTime( reftime, timeSky ) ;
    Vector<Float> spsky = SimpleInterpolationHelper<SpectralData>::GetFromTime(reftime, timeSky, ids, skyspectra, "linear");
    Vector<Float> tcal = SimpleInterpolationHelper<TcalData>::GetFromTime(reftime, timeSky, ids, tcaldata, "linear");
    Vector<Float> tsys = SimpleInterpolationHelper<TsysData>::GetFromTime(reftime, timeSky, ids, tsysdata, "linear");
    ids = getRowIdFromTime( reftime, timeHot ) ;
    Vector<Float> sphot = SimpleInterpolationHelper<SpectralData>::GetFromTime(reftime, timeHot, ids, hotspectra, "linear");
    Vector<Float> spsig = rsig->specCol_( *p ) ;
    Vector<Float> spref = rref->specCol_( *p ) ;
    // using gain array
    for ( unsigned int j = 0 ; j < spsize ; j++ ) {
      if ( spref[j] == 0.0 || (sphot[j]-spsky[j]) == 0.0 ) {
        spec[j] = 0.0 ;
        flagchan[nflag++] = j ;
      }
      else {
        spec[j] = ( ( spsig[j] - spref[j] ) / spref[j] )
          * ( spsky[j] / ( sphot[j] - spsky[j] ) ) * tcal[j] ;
      }
    }
    sig->specCol_.put( *p, spec ) ;
    sig->tsysCol_.put( *p, tsys ) ;
    if ( nflag > 0 ) {
      Vector<uChar> fl = sig->flagsCol_( *p ) ;
      for ( unsigned int j = 0 ; j < nflag ; j++ ) {
        fl[flagchan[j]] = (uChar)True ;
      }
      sig->flagsCol_.put( *p, fl ) ;
    }
    nflag = 0 ;

    reftime = timeCol2.asdouble(*p) ;
    ids = getRowIdFromTime( reftime, timeSky ) ;
    spsky = SimpleInterpolationHelper<SpectralData>::GetFromTime(reftime, timeSky, ids, skyspectra, "linear");
    tcal = SimpleInterpolationHelper<TcalData>::GetFromTime(reftime, timeSky, ids, tcaldata, "linear");
    tsys = SimpleInterpolationHelper<TsysData>::GetFromTime(reftime, timeSky, ids, tsysdata, "linear");
    ids = getRowIdFromTime( reftime, timeHot ) ;
    sphot = SimpleInterpolationHelper<SpectralData>::GetFromTime(reftime, timeHot, ids, hotspectra, "linear");
    // using gain array
    for ( unsigned int j = 0 ; j < spsize ; j++ ) {
      if ( spsig[j] == 0.0 || (sphot[j]-spsky[j]) == 0.0 ) {
        spec[j] = 0.0 ;
        flagchan[nflag++] = j ;
      }
      else {
        spec[j] = ( ( spref[j] - spsig[j] ) / spsig[j] )
          * ( spsky[j] / ( sphot[j] - spsky[j] ) ) * tcal[j] ;
      }
    }
    ref->specCol_.put( *p, spec ) ;
    ref->tsysCol_.put( *p, tsys ) ;    
    if ( nflag > 0 ) {
      Vector<uChar> fl = ref->flagsCol_( *p ) ;
      for ( unsigned int j = 0 ; j < nflag ; j++ ) {
        fl[flagchan[j]] = (uChar)True ;
      }
      ref->flagsCol_.put( *p, fl ) ;
    }
    nflag = 0 ;
    p++ ;
  }
}

void STMath::copyRows( Table &out,
                       const Table &in,
                       uInt startout,
                       uInt startin,
                       uInt nrow,
                       Bool copySpectra,
                       Bool copyFlagtra,
                       Bool copyTsys )
{
  uInt nexclude = 0 ;
  Block<String> excludeColsBlock( 3 ) ;
  if ( !copySpectra ) {
    excludeColsBlock[nexclude] = "SPECTRA" ;
    nexclude++ ;
  }
  if ( !copyFlagtra ) {
    excludeColsBlock[nexclude] = "FLAGTRA" ;
    nexclude++ ;
  }
  if ( !copyTsys ) {
    excludeColsBlock[nexclude] = "TSYS" ;
    nexclude++ ;
  }
  //  if ( nexclude < 3 ) {
  //    excludeCols.resize( nexclude, True ) ;
  //  }
  Vector<String> excludeCols( IPosition(1,nexclude), 
                              excludeColsBlock.storage(),
                              SHARE ) ;
//   cout << "excludeCols=" << excludeCols << endl ;
  TableRow rowout( out, excludeCols, True ) ;
  ROTableRow rowin( in, excludeCols, True ) ;
  uInt rin = startin ;
  uInt rout = startout ;
  for ( uInt i = 0 ; i < nrow ; i++ ) {
    rowin.get( rin ) ;
    rowout.putMatchingFields( rout, rowin.record() ) ;
    rin++ ;
    rout++ ;
  }
}

CountedPtr<Scantable> STMath::averageWithinSession( CountedPtr<Scantable> &s,
                                                    vector<bool> &mask,
                                                    string weight )
{
  // prepare output table
  bool insitu = insitu_ ;
  insitu_ = false ;
  CountedPtr<Scantable> a = getScantable( s, true ) ;
  insitu_ = insitu ;
  Table &atab = a->table() ;
  ScalarColumn<Double> timeColOut( atab, "TIME" ) ;

  if ( s->nrow() == 0 ) 
    return a ;

  // setup RowAccumulator
  WeightType wtype = stringToWeight( weight ) ;
  RowAccumulator acc( wtype ) ;
  Vector<Bool> cmask( mask ) ;
  acc.setUserMask( cmask ) ;

  vector<string> cols( 3 ) ;
  cols[0] = "IFNO" ;
  cols[1] = "POLNO" ;
  cols[2] = "BEAMNO" ;
  STIdxIter2 iter( s, cols ) ;

  Table ttab = s->table() ;
  ROScalarColumn<Double> *timeCol = new ROScalarColumn<Double>( ttab, "TIME" ) ;
  Vector<Double> timeVec = timeCol->getColumn() ;
  delete timeCol ;
  Vector<Double> interval = s->integrCol_.getColumn() ;
  uInt nrow = timeVec.nelements() ;
  uInt outrow = 0 ;

  while( !iter.pastEnd() ) {

    Vector<uInt> rows = iter.getRows( SHARE ) ;
    uInt len = rows.nelements() ;

    if ( len == 0 ) {
      iter.next() ;
      continue ;
    }

    uInt nchan = s->nchan(s->getIF(rows[0])) ;
    Vector<uChar> flag( nchan ) ;
    Vector<Bool> bflag( nchan ) ;
    Vector<Float> spec( nchan ) ;
    Vector<Float> tsys( nchan ) ;

    Vector<Double> timeSep( len-1 ) ; 
    for ( uInt i = 0 ; i < len-1 ; i++ ) {
      timeSep[i] = timeVec[rows[i+1]] - timeVec[rows[i]] ;
    }

    uInt irow ;
    uInt jrow ;
    for ( uInt i = 0 ; i < len-1 ; i++ ) {
      irow = rows[i] ;
      jrow = rows[i+1] ;
      // accumulate data
      s->flagsCol_.get( irow, flag ) ;
      convertArray( bflag, flag ) ;
      s->specCol_.get( irow, spec ) ;
      tsys.assign( s->tsysCol_( irow ) ) ;
      if ( !allEQ(bflag,True) ) 
        acc.add( spec, !bflag, tsys, interval[irow], timeVec[irow] ) ;
      double gap = 2.0 * 86400.0 * timeSep[i] / ( interval[jrow] + interval[irow] ) ;
      //cout << "gap[" << i << "]=" << setw(5) << gap << endl ;
      if ( gap > 1.1 ) {
        //cout << "detected gap between " << i << " and " << i+1 << endl ;
        // put data to output table
        // reset RowAccumulator
        if ( acc.state() ) {
          atab.addRow() ;
          copyRows( atab, ttab, outrow, irow, 1, False, False, False ) ; 
          acc.replaceNaN() ;
          const Vector<Bool> &msk = acc.getMask() ;
          convertArray( flag, !msk ) ;
          for (uInt k = 0; k < nchan; ++k) {
            uChar userFlag = 1 << 7;
            if (msk[k]==True) userFlag = 0 << 7;
            flag(k) = userFlag;
          }
          a->flagsCol_.put( outrow, flag ) ;
          a->specCol_.put( outrow, acc.getSpectrum() ) ;
          a->tsysCol_.put( outrow, acc.getTsys() ) ;
          a->integrCol_.put( outrow, acc.getInterval() ) ;
          timeColOut.put( outrow, acc.getTime() ) ;
          a->cycleCol_.put( outrow, 0 ) ;
        }
        acc.reset() ;
        outrow++ ;
      }
    }

    // accumulate and add last data
    irow = rows[len-1] ;
    s->flagsCol_.get( irow, flag ) ;
    convertArray( bflag, flag ) ;
    s->specCol_.get( irow, spec ) ;
    tsys.assign( s->tsysCol_( irow ) ) ;
    if (!allEQ(bflag,True) ) 
      acc.add( spec, !bflag, tsys, interval[irow], timeVec[irow] ) ;
    if ( acc.state() ) {
      atab.addRow() ;
      copyRows( atab, ttab, outrow, irow, 1, False, False, False ) ;
      acc.replaceNaN() ;
      const Vector<Bool> &msk = acc.getMask() ;
      convertArray( flag, !msk ) ;
      for (uInt k = 0; k < nchan; ++k) {
        uChar userFlag = 1 << 7;
        if (msk[k]==True) userFlag = 0 << 7;
        flag(k) = userFlag;
      }
      a->flagsCol_.put( outrow, flag ) ;
      a->specCol_.put( outrow, acc.getSpectrum() ) ;
      a->tsysCol_.put( outrow, acc.getTsys() ) ;
      a->integrCol_.put( outrow, acc.getInterval() ) ;
      timeColOut.put( outrow, acc.getTime() ) ;
      a->cycleCol_.put( outrow, 0 ) ;
    }
    acc.reset() ;
    outrow++ ;

    iter.next() ;
  }

  return a ;
}
