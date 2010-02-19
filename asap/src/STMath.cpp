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

#include <casa/iomanip.h>
#include <casa/Exceptions/Error.h>
#include <casa/Containers/Block.h>
#include <casa/BasicSL/String.h>
#include <casa/Arrays/MaskArrLogi.h>
#include <casa/Arrays/MaskArrMath.h>
#include <casa/Arrays/ArrayLogical.h>
#include <casa/Arrays/ArrayMath.h>
#include <casa/Arrays/Slice.h>
#include <casa/Arrays/Slicer.h>
#include <casa/Containers/RecordField.h>
#include <tables/Tables/TableRow.h>
#include <tables/Tables/TableVector.h>
#include <tables/Tables/TabVecMath.h>
#include <tables/Tables/ExprNode.h>
#include <tables/Tables/TableRecord.h>
#include <tables/Tables/TableParse.h>
#include <tables/Tables/ReadAsciiTable.h>
#include <tables/Tables/TableIter.h>
#include <tables/Tables/TableCopy.h>
#include <scimath/Mathematics/FFTServer.h>

#include <lattices/Lattices/LatticeUtilities.h>

#include <coordinates/Coordinates/SpectralCoordinate.h>
#include <coordinates/Coordinates/CoordinateSystem.h>
#include <coordinates/Coordinates/CoordinateUtil.h>
#include <coordinates/Coordinates/FrequencyAligner.h>

#include <scimath/Mathematics/VectorKernel.h>
#include <scimath/Mathematics/Convolver.h>
#include <scimath/Functionals/Polynomial.h>

#include <atnf/PKSIO/SrcType.h>

#include <casa/Logging/LogIO.h>
#include <sstream>

#include "MathUtils.h"
#include "RowAccumulator.h"
#include "STAttr.h"
#include "STMath.h"
#include "STSelector.h"

using namespace casa;

using namespace asap;

// tolerance for direction comparison (rad)
#define TOL_OTF    1.0e-15
#define TOL_POINT  2.9088821e-4  // 1 arcmin

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
  LogIO os( LogOrigin( "STMath", "average()", WHERE ) ) ;
  if ( avmode == "SCAN" && in.size() != 1 )
    throw(AipsError("Can't perform 'SCAN' averaging on multiple tables.\n"
                    "Use merge first."));
  WeightType wtype = stringToWeight(weight);

  // check if OTF observation
  String obstype = in[0]->getHeader().obstype ;
  Double tol = 0.0 ;
  if ( (obstype.find( "OTF" ) != String::npos) || (obstype.find( "OBSERVE_TARGET" ) != String::npos) ) {
    tol = TOL_OTF ;
  }
  else {
    tol = TOL_POINT ;
  }

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

  // set up the output table rows. These are based on the structure of the
  // FIRST scantable in the vector
  const Table& baset = in[0]->table();

  Block<String> cols(3);
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
  TableIterator iter(baset, cols);
//   int count = 0 ;
  while (!iter.pastEnd()) {
    Table subt = iter.table();
//     // copy the first row of this selection into the new table
//     tout.addRow();
//     TableCopy::copyRows(tout, subt, outrowCount, 0, 1);
//     // re-index to 0
//     if ( avmode != "SCAN" && avmode != "SOURCE" ) {
//       scanColOut.put(outrowCount, uInt(0));
//     }
//     ++outrowCount;
    MDirection::ScalarColumn dircol ;
    dircol.attach( subt, "DIRECTION" ) ;
    Int length = subt.nrow() ;
    vector< Vector<Double> > dirs ;
    vector<int> indexes ;
    for ( Int i = 0 ; i < length ; i++ ) {
      Vector<Double> t = dircol(i).getAngle(Unit(String("rad"))).getValue() ;
      //os << << count++ << ": " ;
      //os << "[" << t[0] << "," << t[1] << "]" << LogIO::POST ;
      bool adddir = true ;
      for ( uInt j = 0 ; j < dirs.size() ; j++ ) {
        //if ( allTrue( t == dirs[j] ) ) {
        Double dx = t[0] - dirs[j][0] ;
        Double dy = t[1] - dirs[j][1] ;
        Double dd = sqrt( dx * dx + dy * dy ) ;
        //if ( allNearAbs( t, dirs[j], tol ) ) {
        if ( dd <= tol ) {
          adddir = false ;
          break ;
        }
      }
      if ( adddir ) {
        dirs.push_back( t ) ;
        indexes.push_back( i ) ;
      }
    }
    uInt rowNum = dirs.size() ;
    tout.addRow( rowNum ) ;
    for ( uInt i = 0 ; i < rowNum ; i++ ) {
      TableCopy::copyRows( tout, subt, outrowCount+i, indexes[i], 1 ) ;
      // re-index to 0
      if ( avmode != "SCAN" && avmode != "SOURCE" ) {
        scanColOut.put(outrowCount+i, uInt(0));
      }        
    }
    outrowCount += rowNum ;
    ++iter;
  }
  RowAccumulator acc(wtype);
  Vector<Bool> cmask(mask);
  acc.setUserMask(cmask);
  ROTableRow row(tout);
  ROArrayColumn<Float> specCol, tsysCol;
  ROArrayColumn<uChar> flagCol;
  ROScalarColumn<Double> mjdCol, intCol;
  ROScalarColumn<Int> scanIDCol;

  Vector<uInt> rowstodelete;

  for (uInt i=0; i < tout.nrow(); ++i) {
    for ( int j=0; j < int(in.size()); ++j ) {
      const Table& tin = in[j]->table();
      const TableRecord& rec = row.get(i);
      ROScalarColumn<Double> tmp(tin, "TIME");
      Double td;tmp.get(0,td);
      Table basesubt = tin(tin.col("BEAMNO") == Int(rec.asuInt("BEAMNO"))
                       && tin.col("IFNO") == Int(rec.asuInt("IFNO"))
                       && tin.col("POLNO") == Int(rec.asuInt("POLNO")) );
      Table subt;
      if ( avmode == "SOURCE") {
        subt = basesubt( basesubt.col("SRCNAME") == rec.asString("SRCNAME") );
      } else if (avmode == "SCAN") {
        //subt = basesubt( basesubt.col("SCANNO") == Int(rec.asuInt("SCANNO")) );
        subt = basesubt( basesubt.col("SCANNO") == Int(rec.asuInt("SCANNO"))
                         && basesubt.col("SRCNAME") == rec.asString("SRCNAME") );
      } else {
        subt = basesubt;
      }

      vector<uInt> removeRows ;
      uInt nrsubt = subt.nrow() ;
      for ( uInt irow = 0 ; irow < nrsubt ; irow++ ) {
        //if ( !allTrue((subt.col("DIRECTION").getArrayDouble(TableExprId(irow)))==rec.asArrayDouble("DIRECTION")) ) {
        Vector<Double> x0 = (subt.col("DIRECTION").getArrayDouble(TableExprId(irow))) ;
        Vector<Double> x1 = rec.asArrayDouble("DIRECTION") ;
        double dx = x0[0] - x1[0] ;
        double dy = x0[0] - x1[0] ;
        Double dd = sqrt( dx * dx + dy * dy ) ;
        //if ( !allNearAbs((subt.col("DIRECTION").getArrayDouble(TableExprId(irow))), rec.asArrayDouble("DIRECTION"), tol ) ) {
        if ( dd > tol ) {
          removeRows.push_back( irow ) ;
        }
      }
      if ( removeRows.size() != 0 ) {
        subt.removeRow( removeRows ) ;
      }
      
      if ( nrsubt == removeRows.size() )
        throw(AipsError("Averaging data is empty.")) ;

      specCol.attach(subt,"SPECTRA");
      flagCol.attach(subt,"FLAGTRA");
      tsysCol.attach(subt,"TSYS");
      intCol.attach(subt,"INTERVAL");
      mjdCol.attach(subt,"TIME");
      Vector<Float> spec,tsys;
      Vector<uChar> flag;
      Double inter,time;
      for (uInt k = 0; k < subt.nrow(); ++k ) {
        flagCol.get(k, flag);
        Vector<Bool> bflag(flag.shape());
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
        acc.add(spec, !bflag, tsys, inter, time);
      }
    }
    const Vector<Bool>& msk = acc.getMask();
    if ( allEQ(msk, False) ) {
      uint n = rowstodelete.nelements();
      rowstodelete.resize(n+1, True);
      rowstodelete[n] = i;
      continue;
    }
    //write out
    if (acc.state()) {
      Vector<uChar> flg(msk.shape());
      convertArray(flg, !msk);
      flagColOut.put(i, flg);
      specColOut.put(i, acc.getSpectrum());
      tsysColOut.put(i, acc.getTsys());
      intColOut.put(i, acc.getInterval());
      mjdColOut.put(i, acc.getTime());
      // we should only have one cycle now -> reset it to be 0
      // frequency switched data has different CYCLENO for different IFNO
      // which requires resetting this value
      cycColOut.put(i, uInt(0));
    } else {
      ostringstream oss;
      oss << "For output row="<<i<<", all input rows of data are flagged. no averaging" << endl;
      pushLog(String(oss));
    }
    acc.reset();
  }
  if (rowstodelete.nelements() > 0) {
    //cout << rowstodelete << endl;
    os << rowstodelete << LogIO::POST ;
    tout.removeRow(rowstodelete);
    if (tout.nrow() == 0) {
      throw(AipsError("Can't average fully flagged data."));
    }
  }
  return out;
}

CountedPtr< Scantable >
  STMath::averageChannel( const CountedPtr < Scantable > & in,
                          const std::string & mode,
                          const std::string& avmode )
{
  // check if OTF observation
  String obstype = in->getHeader().obstype ;
  Double tol = 0.0 ;
  if ( obstype.find( "OTF" ) != String::npos ) {
    tol = TOL_OTF ;
  }
  else {
    tol = TOL_POINT ;
  }

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
//     tout.addRow();
//     TableCopy::copyRows(tout, subt, outrowCount, 0, 1);
//     if ( avmode != "SCAN") {
//       scanColOut.put(outrowCount, uInt(0));
//     }
//     Vector<Float> tmp;
//     specCol.get(0, tmp);
//     uInt nchan = tmp.nelements();
//     // have to do channel by channel here as MaskedArrMath
//     // doesn't have partialMedians
//     Vector<uChar> flags = flagCol.getColumn(Slicer(Slice(0)));
//     Vector<Float> outspec(nchan);
//     Vector<uChar> outflag(nchan,0);
//     Vector<Float> outtsys(1);/// @fixme when tsys is channel based
//     for (uInt i=0; i<nchan; ++i) {
//       Vector<Float> specs = specCol.getColumn(Slicer(Slice(i)));
//       MaskedArray<Float> ma = maskedArray(specs,flags);
//       outspec[i] = median(ma);
//       if ( allEQ(ma.getMask(), False) )
//         outflag[i] = userflag;// flag data
//     }
//     outtsys[0] = median(tsysCol.getColumn());
//     specColOut.put(outrowCount, outspec);
//     flagColOut.put(outrowCount, outflag);
//     tsysColOut.put(outrowCount, outtsys);
//     Double intsum = sum(intCol.getColumn());
//     intColOut.put(outrowCount, intsum);
//     ++outrowCount;
//     ++iter;
    MDirection::ScalarColumn dircol ;
    dircol.attach( subt, "DIRECTION" ) ;
    Int length = subt.nrow() ;
    vector< Vector<Double> > dirs ;
    vector<int> indexes ;
    for ( Int i = 0 ; i < length ; i++ ) {
      Vector<Double> t = dircol(i).getAngle(Unit(String("rad"))).getValue() ;
      bool adddir = true ;
      for ( uInt j = 0 ; j < dirs.size() ; j++ ) {
        //if ( allTrue( t == dirs[j] ) ) {
        Double dx = t[0] - dirs[j][0] ;
        Double dy = t[1] - dirs[j][1] ;
        Double dd = sqrt( dx * dx + dy * dy ) ;
        //if ( allNearAbs( t, dirs[j], tol ) ) {
        if ( dd <= tol ) {
          adddir = false ;
          break ;
        }
      }
      if ( adddir ) {
        dirs.push_back( t ) ;
        indexes.push_back( i ) ;
      }
    }
    uInt rowNum = dirs.size() ;
    tout.addRow( rowNum );
    for ( uInt i = 0 ; i < rowNum ; i++ ) {
      TableCopy::copyRows(tout, subt, outrowCount+i, indexes[i], 1) ;
      if ( avmode != "SCAN") {
        //scanColOut.put(outrowCount+i, uInt(0));
      }
    }
    MDirection::ScalarColumn dircolOut ;
    dircolOut.attach( tout, "DIRECTION" ) ;
    for ( uInt irow = 0 ; irow < rowNum ; irow++ ) {
      Vector<Double> t = dircolOut(outrowCount+irow).getAngle(Unit(String("rad"))).getValue() ;
      Vector<Float> tmp;
      specCol.get(0, tmp);
      uInt nchan = tmp.nelements();
      // have to do channel by channel here as MaskedArrMath
      // doesn't have partialMedians
      Vector<uChar> flags = flagCol.getColumn(Slicer(Slice(0)));
      // mask spectra for different DIRECTION
      for ( uInt jrow = 0 ; jrow < subt.nrow() ; jrow++ ) {
        Vector<Double> direction = dircol(jrow).getAngle(Unit(String("rad"))).getValue() ;
        //if ( t[0] != direction[0] || t[1] != direction[1] ) {
        Double dx = t[0] - direction[0] ;
        Double dy = t[1] - direction[1] ;
        Double dd = sqrt( dx * dx + dy * dy ) ;
        //if ( !allNearAbs( t, direction, tol ) ) {
        if ( dd > tol ) {
          flags[jrow] = userflag ;
        }
      }
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
      specColOut.put(outrowCount+irow, outspec);
      flagColOut.put(outrowCount+irow, outflag);
      tsysColOut.put(outrowCount+irow, outtsys);
      Vector<Double> integ = intCol.getColumn() ;
      MaskedArray<Double> mi = maskedArray( integ, flags ) ;
      Double intsum = sum(mi);
      intColOut.put(outrowCount+irow, intsum);
    }
    outrowCount += rowNum ;
    ++iter;
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
  for (uInt i=0; i<tab.nrow(); ++i) {
    Vector<Float> spec;
    Vector<Float> ts;
    specCol.get(i, spec);
    tsysCol.get(i, ts);
    if (mode == "MUL" || mode == "DIV") {
      if (mode == "DIV") val = 1.0/val;
      spec *= val;
      specCol.put(i, spec);
      if ( tsys ) {
        ts *= val;
        tsysCol.put(i, ts);
      }
    } else if ( mode == "ADD"  || mode == "SUB") {
      if (mode == "SUB") val *= -1.0;
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
  for (uInt i=0; i<tab.nrow(); ++i) {
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
  if ( fact.nelements() != in->nrow() ) {
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
  for ( uInt i = 0 ; i < in->nrow() ; i++ ) {
    nchans.push_back( (in->getSpectrum( i )).size() ) ;
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
  Vector<Float> tcalout2;  //debug

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
      String tcalt;
      Float tcalUsed;
      tcalUsed = tcal;
      if ( tcal <= 0.0 ) {
        caloff->tcal().getEntry(tcalt, tcalout, tcalId);
        if (polno<=3) {
          tcalUsed = tcalout[polno];
        }
        else {
          tcalUsed = tcalout[0];
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
    oss<<"Applied smoothing of "<<fsmoothref<<" on the reference."<<endl;
    pushLog(String(oss));
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
        oss << "user specified Tsys = " << tsysv;
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
        oss << ", corrected (for El) tsys= "<<tsysrefscalar;
        pushLog(String(oss));
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
  ostringstream oss1;
  oss1 << msg  << endl;
  pushLog(String(oss1));
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
  ostringstream oss2;
  oss2 << msg  << endl;
  pushLog(String(oss2));
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
  RowAccumulator acc( asap::TINTSYS ) ; 
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
  for (uInt i=0; i < tout.nrow(); ++i ) {
    MaskedArray<Float> main  = maskedArray(specCol(i), flagCol(i));
    MaskedArray<Float> maout;
    LatticeUtilities::bin(maout, main, 0, Int(width));
    /// @todo implement channel based tsys binning
    specCol.put(i, maout.getArray());
    flagCol.put(i, flagsFromMA(maout));
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
    lookup["NONE"]   = asap::NONE;
    lookup["TINT"] = asap::TINT;
    lookup["TINTSYS"]  = asap::TINTSYS;
    lookup["TSYS"]  = asap::TSYS;
    lookup["VAR"]  = asap::VAR;
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
      ppoly = new Polynomial<Float>(nc);
      coeff = coeffs;
      msg = String("user");
    } else {
      STAttr sdAttr;
      coeff = sdAttr.gainElevationPoly(inst);
      ppoly = new Polynomial<Float>(3);
      msg = String("built in");
    }

    if ( coeff.nelements() > 0 ) {
      ppoly->setCoefficients(coeff);
    } else {
      delete ppoly;
      throw(AipsError("There is no known gain-elevation polynomial known for this instrument"));
    }
    ostringstream oss;
    oss << "Making polynomial correction with " << msg << " coefficients:" << endl;
    oss << "   " <<  coeff;
    pushLog(String(oss));
    const uInt nrow = tab.nrow();
    Vector<Float> factor(nrow);
    for ( uInt i=0; i < nrow; ++i ) {
      factor[i] = 1.0 / (*ppoly)(x[i]);
    }
    delete ppoly;
    scaleByVector(tab, factor, true);

  } else {
    // Read and correct
    pushLog("Making correction from ascii Table");
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
  CountedPtr< Scantable > out = getScantable(in, false);
  Table& tab = in->table();
  Unit fluxUnit(tab.keywordSet().asString("FluxUnit"));
  Unit K(String("K"));
  Unit JY(String("Jy"));

  bool tokelvin = true;
  Double cfac = 1.0;

  if ( fluxUnit == JY ) {
    pushLog("Converting to K");
    Quantum<Double> t(1.0,fluxUnit);
    Quantum<Double> t2 = t.get(JY);
    cfac = (t2 / t).getValue();               // value to Jy

    tokelvin = true;
    out->setFluxUnit("K");
  } else if ( fluxUnit == K ) {
    pushLog("Converting to Jy");
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
    ostringstream oss;
    oss << "Jy/K = " << jyperk;
    pushLog(String(oss));
    Vector<Float> factors(tab.nrow(), factor);
    scaleByVector(tab,factors, false);
  } else if ( etaap > 0.0) {
    if (d < 0) {
      Instrument inst =
	STAttr::convertInstrument(tab.keywordSet().asString("AntennaName"), 
				  True);
      STAttr sda;
      d = sda.diameter(inst);
    }
    jyperk = STAttr::findJyPerK(etaap, d);
    ostringstream oss;
    oss << "Jy/K = " << jyperk;
    pushLog(String(oss));
    factor *= jyperk;
    if ( tokelvin ) {
      factor = 1.0 / factor;
    }
    Vector<Float> factors(tab.nrow(), factor);
    scaleByVector(tab, factors, False);
  } else {

    // OK now we must deal with automatic look up of values.
    // We must also deal with the fact that the factors need
    // to be computed per IF and may be different and may
    // change per integration.

    pushLog("Looking up conversion factors");
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
                                         float tau )
{
  CountedPtr< Scantable > out = getScantable(in, false);

  Table tab = out->table();
  ROScalarColumn<Float> elev(tab, "ELEVATION");
  ArrayColumn<Float> specCol(tab, "SPECTRA");
  ArrayColumn<uChar> flagCol(tab, "FLAGTRA");
  ArrayColumn<Float> tsysCol(tab, "TSYS");
  for ( uInt i=0; i<tab.nrow(); ++i) {
    Float zdist = Float(C::pi_2) - elev(i);
    Float factor = exp(tau/cos(zdist));
    MaskedArray<Float> ma = maskedArray(specCol(i), flagCol(i));
    ma *= factor;
    specCol.put(i, ma.getArray());
    flagCol.put(i, flagsFromMA(ma));
    Vector<Float> tsys;
    tsysCol.get(i, tsys);
    tsys *= factor;
    tsysCol.put(i, tsys);
  }
  return out;
}

CountedPtr< Scantable > STMath::smoothOther( const CountedPtr< Scantable >& in,
                                             const std::string& kernel,
                                             float width )
{
  CountedPtr< Scantable > out = getScantable(in, false);
  Table& table = out->table();
  ArrayColumn<Float> specCol(table, "SPECTRA");
  ArrayColumn<uChar> flagCol(table, "FLAGTRA");
  Vector<Float> spec;
  Vector<uChar> flag;
  for ( uInt i=0; i<table.nrow(); ++i) {
    specCol.get(i, spec);
    flagCol.get(i, flag);
    Vector<Bool> mask(flag.nelements());
    convertArray(mask, flag);
    Vector<Float> specout;
    Vector<Bool> maskout;
    if ( kernel == "hanning" ) {
      mathutil::hanning(specout, maskout, spec , !mask);
      convertArray(flag, !maskout);
    } else if (  kernel == "rmedian" ) {
      mathutil::runningMedian(specout, maskout, spec , mask, width);
      convertArray(flag, maskout);
    }
    flagCol.put(i, flag);
    specCol.put(i, specout);
  }
  return out;
}

CountedPtr< Scantable > STMath::smooth( const CountedPtr< Scantable >& in,
                                        const std::string& kernel, float width )
{
  if (kernel == "rmedian"  || kernel == "hanning") {
    return smoothOther(in, kernel, width);
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
    Vector<Float> tmpspec; specCol.get(0, tmpspec);
    uInt nchan = tmpspec.nelements();
    Vector<Float> kvec = VectorKernel::make(type, width, nchan, True, False);
    Convolver<Float> conv(kvec, IPosition(1,nchan));
    Vector<Float> spec;
    Vector<uChar> flag;
    for ( uInt i=0; i<tab.nrow(); ++i) {
      specCol.get(i, spec);
      flagCol.get(i, flag);
      Vector<Bool> mask(flag.nelements());
      convertArray(mask, flag);
      Vector<Float> specout;
      mathutil::replaceMaskByZero(specout, mask);
      conv.linearConv(specout, spec);
      specCol.put(i, specout);
    }
    ++iter;
  }
  return out;
}

CountedPtr< Scantable >
  STMath::merge( const std::vector< CountedPtr < Scantable > >& in )
{
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
  // Renumber SCANNO to be 0-based
  Vector<uInt> scannos = scannocol.getColumn();
  uInt offset = min(scannos);
  scannos -= offset;
  scannocol.putColumn(scannos);
  uInt newscanno = max(scannos)+1;
  ++it;
  while ( it != in.end() ){
    if ( ! (*it)->conformant(*out) ) {
      // non conformant.
      pushLog(String("Warning: Can't merge scantables as header info differs."));
    }
    out->appendToHistoryTable((*it)->history());
    const Table& tab = (*it)->table();
    TableIterator scanit(tab, "SCANNO");
    while (!scanit.pastEnd()) {
      TableIterator freqit(scanit.table(), "FREQ_ID");
      while ( !freqit.pastEnd() ) {
        Table thetab = freqit.table();
        uInt nrow = tout.nrow();
        tout.addRow(thetab.nrow());
        TableCopy::copyRows(tout, thetab, nrow, 0, thetab.nrow());
        ROTableRow row(thetab);
        for ( uInt i=0; i<thetab.nrow(); ++i) {
          uInt k = nrow+i;
          scannocol.put(k, newscanno);
          const TableRecord& rec = row.get(i);
          Double rv,rp,inc;
          (*it)->frequencies().getEntry(rp, rv, inc, rec.asuInt("FREQ_ID"));
          uInt id;
          id = out->frequencies().addEntry(rp, rv, inc);
          freqidcol.put(k,id);
          //String name,fname;Double rf;
          Vector<String> name,fname;Vector<Double> rf;
          (*it)->molecules().getEntry(rf, name, fname, rec.asuInt("MOLECULE_ID"));
          id = out->molecules().addEntry(rf, name, fname);
          molidcol.put(k, id);
          Float frot,fax,ftan,fhand,fmount,fuser, fxy, fxyp;
          (*it)->focus().getEntry(fax, ftan, frot, fhand,
                                  fmount,fuser, fxy, fxyp,
                                  rec.asuInt("FOCUS_ID"));
          id = out->focus().addEntry(fax, ftan, frot, fhand,
                                     fmount,fuser, fxy, fxyp);
          focusidcol.put(k, id);
        }
        ++freqit;
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
    ScalarColumn<Float> parancol(t, "PARANGLE");
    Matrix<Float> pols(speccol.getColumn());
    try {
      stpol->setSpectra(pols);
      Float fang,fhand,parang;
      fang = in->focusTable_.getTotalFeedAngle(focidcol(0));
      fhand = in->focusTable_.getFeedHand(focidcol(0));
      parang = parancol(0);
      /// @todo re-enable this
      // disable total feed angle to support paralactifying Caswell style
      stpol->setPhaseCorrections(parang, -parang, fhand);
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
  //pols->table_.rwKeywordSet().define("POLTYPE", String("stokes"));
  pols->table_.rwKeywordSet().define("POLTYPE", in->getPolType());
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
  ostringstream oss;
  oss << "Aligned at reference Epoch " << epochout
      << " in frame " << MFrequency::showType(system);
  pushLog(String(oss));
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
    MDirection::ROScalarColumn dirCol(t, "DIRECTION");
    TableIterator fiter(t, "FREQ_ID");
    // determine nchan from the first row. This should work as
    // we are iterating over BEAMNO and IFNO    // we should have constant direction

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
    SpectralCoordinate sC = in->frequencies().getSpectralCoordinate(freqidCol(0));
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
      Float fang,fhand,parang;
      fang = in->focusTable_.getTotalFeedAngle(in->mfocusidCol_(row));
      fhand = in->focusTable_.getFeedHand(in->mfocusidCol_(row));
      parang = in->paraCol_(row);
      /// @todo re-enable this
      // disable total feed angle to support paralactifying Caswell style
      stpol->setPhaseCorrections(parang, -parang, fhand);
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

CountedPtr< Scantable >
  asap::STMath::lagFlag( const CountedPtr< Scantable > & in,
                          double frequency, double width )
{
  CountedPtr< Scantable > out = getScantable(in, false);
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
      Int lag0 = Int(spec.nelements()*abs(inc)/(frequency+width)+0.5);
      Int lag1 = Int(spec.nelements()*abs(inc)/(frequency-width)+0.5);
      for (unsigned int k=0; k < flag.nelements(); ++k ) {
        if (flag[k] > 0) {
          spec[k] = 0.0;
        }
      }
      Vector<Complex> lags;
      ffts.fft0(lags, spec);
      Int start =  max(0, lag0);
      Int end =  min(Int(lags.nelements()-1), lag1);
      if (start == end) {
        lags[start] = Complex(0.0);
      } else {
        for (int j=start; j <=end ;++j) {
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
  
  // check if OTF observation
  String obstype = in[0]->getHeader().obstype ;
  Double tol = 0.0 ;
  if ( obstype.find( "OTF" ) != String::npos ) {
    tol = TOL_OTF ;
  }
  else {
    tol = TOL_POINT ;
  }

  CountedPtr<Scantable> out ;     // processed result 
  if ( compel ) {
    std::vector< CountedPtr<Scantable> > newin ; // input for average process
    uInt insize = in.size() ;    // number of input scantables 

    // TEST: do normal average in each table before IF grouping
    os << "Do preliminary averaging" << LogIO::POST ;
    vector< CountedPtr<Scantable> > tmpin( insize ) ;
    for ( uInt itable = 0 ; itable < insize ; itable++ ) {
      vector< CountedPtr<Scantable> > v( 1, in[itable] ) ;
      tmpin[itable] = average( v, mask, weight, avmode ) ;
    }

    // warning
    os << "Average spectra with different spectral resolution" << LogIO::POST ;

    // temporarily set coordinfo
    vector<string> oldinfo( insize ) ;
    for ( uInt itable = 0 ; itable < insize ; itable++ ) {
      vector<string> coordinfo = in[itable]->getCoordInfo() ;
      oldinfo[itable] = coordinfo[0] ;
      coordinfo[0] = "Hz" ;
      tmpin[itable]->setCoordInfo( coordinfo ) ;
    }

    // columns
    ScalarColumn<uInt> freqIDCol ;
    ScalarColumn<uInt> ifnoCol ;
    ScalarColumn<uInt> scannoCol ;


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
      uInt rows = tmpin[itable]->nrow() ;
      uInt freqnrows = tmpin[itable]->frequencies().table().nrow() ;
      for ( uInt irow = 0 ; irow < rows ; irow++ ) {
	if ( freqid[itable].size() == freqnrows ) {
	  break ;
	}
	else {
	  freqIDCol.attach( tmpin[itable]->table(), "FREQ_ID" ) ;
	  ifnoCol.attach( tmpin[itable]->table(), "IFNO" ) ;
	  uInt id = freqIDCol( irow ) ;
	  if ( freqid[itable].size() == 0 || count( freqid[itable].begin(), freqid[itable].end(), id ) == 0 ) {
	    //os << "itable = " << itable << ": IF " << id << " is included in the list" << LogIO::POST ;
	    vector<double> abcissa = tmpin[itable]->getAbcissa( irow ) ;
	    freqid[itable].push_back( id ) ;
	    iffreq[itable].push_back( abcissa[0] - 0.5 * ( abcissa[1] - abcissa[0] ) ) ;
	    iffreq[itable].push_back( abcissa[abcissa.size()-1] + 0.5 * ( abcissa[1] - abcissa[0] ) ) ;
	  }
	}
      }
    }

    // debug
    //os << "IF settings summary:" << endl ;
    //for ( uInt i = 0 ; i < freqid.size() ; i++ ) {
    //os << "   Table" << i << endl ;
    //for ( uInt j = 0 ; j < freqid[i].size() ; j++ ) {
    //os << "      id = " << freqid[i][j] << " (min,max) = (" << iffreq[i][2*j] << "," << iffreq[i][2*j+1] << ")" << endl ;
    //}
    //}
    //os << endl ;
    //os.post() ;

    // IF grouping based on their frequency coverage
    // ifgrp: list of table index and FREQ_ID for all members in each IF group
    // ifgfreq: list of minimum and maximum frequency in each IF group
    // ifgrp[numgrp][nummember*2]
    // ifgrp: [[table00, freqrow00, table01, freqrow01, ...],
    //         [table10, freqrow10, table11, freqrow11, ...],
    //         ...
    //         [tablen0, freqrown0, tablen1, freqrown1, ...]]
    // ifgfreq[numgrp*2] 
    // ifgfreq: [min0_grp0, max0_grp0, min1_grp1, max1_grp1, ...]
    //os << "IF grouping based on their frequency coverage" << LogIO::POST ;
    vector< vector<uInt> > ifgrp ;
    vector<double> ifgfreq ;

    // parameter for IF grouping
    // groupmode = OR    retrieve all region 
    //             AND   only retrieve overlaped region
    //string groupmode = "AND" ;
    string groupmode = "OR" ;
    uInt sizecr = 0 ;
    if ( groupmode == "AND" ) 
      sizecr = 2 ;
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
    for ( vector<double>::iterator i = sortedfreq.begin() ; i != sortedfreq.end()-1 ; i++ ) {
      ifgfreq.push_back( *i ) ;
      ifgfreq.push_back( *(i+1) ) ;
    }
    ifgrp.resize( ifgfreq.size()/2 ) ;
    for ( uInt itable = 0 ; itable < insize ; itable++ ) {
      for ( uInt iif = 0 ; iif < freqid[itable].size() ; iif++ ) {
	double range0 = iffreq[itable][2*iif] ;
	double range1 = iffreq[itable][2*iif+1] ;
	for ( uInt j = 0 ; j < ifgrp.size() ; j++ ) {
	  double fmin = max( range0, ifgfreq[2*j] ) ;
	  double fmax = min( range1, ifgfreq[2*j+1] ) ;
	  if ( fmin < fmax ) {
	    ifgrp[j].push_back( itable ) ;
	    ifgrp[j].push_back( freqid[itable][iif] ) ;
	  }
	}
      }
    }
    vector< vector<uInt> >::iterator fiter = ifgrp.begin() ; 
    vector<double>::iterator giter = ifgfreq.begin() ;
    while( fiter != ifgrp.end() ) {
      if ( fiter->size() <= sizecr ) {
	fiter = ifgrp.erase( fiter ) ;
	giter = ifgfreq.erase( giter ) ;
	giter = ifgfreq.erase( giter ) ;
      }
      else {
	fiter++ ;
	advance( giter, 2 ) ;
      }
    }

    // Grouping continuous IF groups (without frequency gap)
    // freqgrp: list of IF group indexes in each frequency group
    // freqrange: list of minimum and maximum frequency in each frequency group
    // freqgrp[numgrp][nummember]
    // freqgrp: [[ifgrp00, ifgrp01, ifgrp02, ...],
    //           [ifgrp10, ifgrp11, ifgrp12, ...],
    //           ...
    //           [ifgrpn0, ifgrpn1, ifgrpn2, ...]]
    // freqrange[numgrp*2]
    // freqrange: [min_grp0, max_grp0, min_grp1, max_grp1, ...]
    vector< vector<uInt> > freqgrp ;
    double freqrange = 0.0 ;
    uInt grpnum = 0 ;
    for ( uInt i = 0 ; i < ifgrp.size() ; i++ ) {
      // Assumed that ifgfreq was sorted
      if ( grpnum != 0 && freqrange == ifgfreq[2*i] ) {
	freqgrp[grpnum-1].push_back( i ) ;
      }
      else {
	vector<uInt> grp0( 1, i ) ;
	freqgrp.push_back( grp0 ) ;
	grpnum++ ;
      }
      freqrange = ifgfreq[2*i+1] ;
    }
	

    // print IF groups
    ostringstream oss ;
    oss << "IF Group summary: " << endl ;
    oss << "   GROUP_ID [FREQ_MIN, FREQ_MAX]: (TABLE_ID, FREQ_ID)" << endl ;
    for ( uInt i = 0 ; i < ifgrp.size() ; i++ ) {
      oss << "   GROUP " << setw( 2 ) << i << " [" << ifgfreq[2*i] << "," << ifgfreq[2*i+1] << "]: " ;
      for ( uInt j = 0 ; j < ifgrp[i].size()/2 ; j++ ) {
	oss << "(" << ifgrp[i][2*j] << "," << ifgrp[i][2*j+1] << ") " ; 
      }
      oss << endl ;
    }
    oss << endl ;
    os << oss.str() << LogIO::POST ;
    
    // print frequency group
    oss.str("") ;
    oss << "Frequency Group summary: " << endl ;
    oss << "   GROUP_ID [FREQ_MIN, FREQ_MAX]: IF_GROUP_ID" << endl ;
    for ( uInt i = 0 ; i < freqgrp.size() ; i++ ) {
      oss << "   GROUP " << setw( 2 ) << i << " [" << ifgfreq[2*freqgrp[i][0]] << "," << ifgfreq[2*freqgrp[i][freqgrp[i].size()-1]+1] << "]: " ;
      for ( uInt j = 0 ; j < freqgrp[i].size() ; j++ ) {
	oss << freqgrp[i][j] << " " ;
      }
      oss << endl ;
    }
    oss << endl ;
    os << oss.str() << LogIO::POST ;

    // membership check
    // groups: list of IF group indexes whose frequency range overlaps with 
    //         that of each table and IF
    // groups[numtable][numIF][nummembership]
    // groups: [[[grp, grp,...], [grp, grp,...],...],
    //          [[grp, grp,...], [grp, grp,...],...],
    //          ...
    //          [[grp, grp,...], [grp, grp,...],...]] 
    vector< vector< vector<uInt> > > groups( insize ) ;
    for ( uInt i = 0 ; i < insize ; i++ ) {
      groups[i].resize( freqid[i].size() ) ;
    }
    for ( uInt igrp = 0 ; igrp < ifgrp.size() ; igrp++ ) {
      for ( uInt imem = 0 ; imem < ifgrp[igrp].size()/2 ; imem++ ) {
	uInt tableid = ifgrp[igrp][2*imem] ;
	vector<uInt>::iterator iter = find( freqid[tableid].begin(), freqid[tableid].end(), ifgrp[igrp][2*imem+1] ) ;
	if ( iter != freqid[tableid].end() ) {
	  uInt rowid = distance( freqid[tableid].begin(), iter ) ;
	  groups[tableid][rowid].push_back( igrp ) ;
	}
      }
    }

    // print membership
    //oss.str("") ;
    //for ( uInt i = 0 ; i < insize ; i++ ) {
    //oss << "Table " << i << endl ;
    //for ( uInt j = 0 ; j < groups[i].size() ; j++ ) {
    //oss << "   FREQ_ID " <<  setw( 2 ) << freqid[i][j] << ": " ;
    //for ( uInt k = 0 ; k < groups[i][j].size() ; k++ ) {
    //oss << setw( 2 ) << groups[i][j][k] << " " ;
    //}
    //oss << endl ;
    //}
    //}
    //os << oss.str() << LogIO::POST ;

    // set back coordinfo
    for ( uInt itable = 0 ; itable < insize ; itable++ ) {
      vector<string> coordinfo = tmpin[itable]->getCoordInfo() ;
      coordinfo[0] = oldinfo[itable] ;
      tmpin[itable]->setCoordInfo( coordinfo ) ;
    }

    // Create additional table if needed
    bool oldInsitu = insitu_ ;
    setInsitu( false ) ;
    vector< vector<uInt> > addrow( insize ) ;
    vector<uInt> addtable( insize, 0 ) ;
    vector<uInt> newtableids( insize ) ;
    vector<uInt> newifids( insize, 0 ) ;
    for ( uInt itable = 0 ; itable < insize ; itable++ ) {
      //os << "Table " << itable << ": " ;
      for ( uInt ifrow = 0 ; ifrow < groups[itable].size() ; ifrow++ ) {
	addrow[itable].push_back( groups[itable][ifrow].size()-1 ) ;
	//os << addrow[itable][ifrow] << " " ;
      }
      addtable[itable] = *max_element( addrow[itable].begin(), addrow[itable].end() ) ;
      //os << "(" << addtable[itable] << ")" << LogIO::POST ;
    }
    newin.resize( insize ) ;
    copy( tmpin.begin(), tmpin.end(), newin.begin() ) ;
    for ( uInt i = 0 ; i < insize ; i++ ) {
      newtableids[i] = i ;
    }
    for ( uInt itable = 0 ; itable < insize ; itable++ ) {
      for ( uInt iadd = 0 ; iadd < addtable[itable] ; iadd++ ) {
	CountedPtr<Scantable> add = getScantable( newin[itable], false ) ;
	vector<int> freqidlist ;
	for ( uInt i = 0 ; i < groups[itable].size() ; i++ ) {
	  if ( groups[itable][i].size() > iadd + 1 ) {
	    freqidlist.push_back( freqid[itable][i] ) ;
	  }
	}
	stringstream taqlstream ;
	taqlstream << "SELECT FROM $1 WHERE FREQ_ID IN [" ;
	for ( uInt i = 0 ; i < freqidlist.size() ; i++ ) {
	  taqlstream << i ;
	  if ( i < freqidlist.size() - 1 )
	    taqlstream << "," ;
	  else
	    taqlstream << "]" ;
	}
	string taql = taqlstream.str() ;
	//os << "taql = " << taql << LogIO::POST ;
	STSelector selector = STSelector() ;
	selector.setTaQL( taql ) ;
	add->setSelection( selector ) ;
	newin.push_back( add ) ;
	newtableids.push_back( itable ) ;
	newifids.push_back( iadd + 1 ) ;
      }
    }

    // udpate ifgrp
    for ( uInt itable = 0 ; itable < insize ; itable++ ) {
      for ( uInt iadd = 0 ; iadd < addtable[itable] ; iadd++ ) {
	for ( uInt ifrow = 0 ; ifrow < groups[itable].size() ; ifrow++ ) {
	  if ( groups[itable][ifrow].size() > iadd + 1 ) {
	    uInt igrp = groups[itable][ifrow][iadd+1] ;
	    for ( uInt imem = 0 ; imem < ifgrp[igrp].size()/2 ; imem++ ) {
	      if ( ifgrp[igrp][2*imem] == newtableids[iadd+insize] && ifgrp[igrp][2*imem+1] == freqid[newtableids[iadd+insize]][ifrow] ) {
		ifgrp[igrp][2*imem] = insize + iadd ;
	      }
	    }
	  }
	}
      }
    }

    // print IF groups again for debug
    //oss.str( "" ) ;
    //oss << "IF Group summary: " << endl ;
    //oss << "   GROUP_ID [FREQ_MIN, FREQ_MAX]: (TABLE_ID, FREQ_ID)" << endl ;
    //for ( uInt i = 0 ; i < ifgrp.size() ; i++ ) {
    //oss << "   GROUP " << setw( 2 ) << i << " [" << ifgfreq[2*i] << "," << ifgfreq[2*i+1] << "]: " ;
    //for ( uInt j = 0 ; j < ifgrp[i].size()/2 ; j++ ) {
    //oss << "(" << ifgrp[i][2*j] << "," << ifgrp[i][2*j+1] << ") " ; 
    //}
    //oss << endl ;
    //}
    //oss << endl ;
    //os << oss.str() << LogIO::POST ;

    // reset SCANNO and IFNO/FREQ_ID: IF is reset by the result of sortation 
    os << "All scan number is set to 0" << LogIO::POST ;
    //os << "All IF number is set to IF group index" << LogIO::POST ;
    insize = newin.size() ;
    for ( uInt itable = 0 ; itable < insize ; itable++ ) {
      uInt rows = newin[itable]->nrow() ;
      Table &tmpt = newin[itable]->table() ;
      freqIDCol.attach( tmpt, "FREQ_ID" ) ;
      scannoCol.attach( tmpt, "SCANNO" ) ;
      ifnoCol.attach( tmpt, "IFNO" ) ;
      for ( uInt irow=0 ; irow < rows ; irow++ ) {
	scannoCol.put( irow, 0 ) ;
	uInt freqID = freqIDCol( irow ) ;
	vector<uInt>::iterator iter = find( freqid[newtableids[itable]].begin(), freqid[newtableids[itable]].end(), freqID ) ;
	if ( iter != freqid[newtableids[itable]].end() ) {
	  uInt index = distance( freqid[newtableids[itable]].begin(), iter ) ;
	  ifnoCol.put( irow, groups[newtableids[itable]][index][newifids[itable]] ) ;
	}
	else {
	  throw(AipsError("IF grouping was wrong in additional tables.")) ;
	} 
      }
    }
    oldinfo.resize( insize ) ;
    setInsitu( oldInsitu ) ;

    // temporarily set coordinfo
    for ( uInt itable = 0 ; itable < insize ; itable++ ) {
      vector<string> coordinfo = newin[itable]->getCoordInfo() ;
      oldinfo[itable] = coordinfo[0] ;
      coordinfo[0] = "Hz" ;
      newin[itable]->setCoordInfo( coordinfo ) ;
    }

    // save column values in the vector
    vector< vector<uInt> > freqTableIdVec( insize ) ;
    vector< vector<uInt> > freqIdVec( insize ) ;
    vector< vector<uInt> > ifNoVec( insize ) ;
    for ( uInt itable = 0 ; itable < insize ; itable++ ) {
      ScalarColumn<uInt> freqIDs ;
      freqIDs.attach( newin[itable]->frequencies().table(), "ID" ) ;
      ifnoCol.attach( newin[itable]->table(), "IFNO" ) ;
      freqIDCol.attach( newin[itable]->table(), "FREQ_ID" ) ;
      for ( uInt irow = 0 ; irow < newin[itable]->frequencies().table().nrow() ; irow++ ) {
	freqTableIdVec[itable].push_back( freqIDs( irow ) ) ;
      }
      for ( uInt irow = 0 ; irow < newin[itable]->table().nrow() ; irow++ ) {
	freqIdVec[itable].push_back( freqIDCol( irow ) ) ;
	ifNoVec[itable].push_back( ifnoCol( irow ) ) ;
      }
    }

    // reset spectra and flagtra: pick up common part of frequency coverage
    //os << "Pick common frequency range and align resolution" << LogIO::POST ;
    for ( uInt itable = 0 ; itable < insize ; itable++ ) {
      uInt rows = newin[itable]->nrow() ;
      int nminchan = -1 ;
      int nmaxchan = -1 ;
      vector<uInt> freqIdUpdate ; 
      for ( uInt irow = 0 ; irow < rows ; irow++ ) {
	uInt ifno = ifNoVec[itable][irow] ;  // IFNO is reset by group index
	double minfreq = ifgfreq[2*ifno] ;
	double maxfreq = ifgfreq[2*ifno+1] ;
	//os << "frequency range: [" << minfreq << "," << maxfreq << "]" << LogIO::POST ;
	vector<double> abcissa = newin[itable]->getAbcissa( irow ) ;
	int nchan = abcissa.size() ;
	double resol = abcissa[1] - abcissa[0] ;
	//os << "abcissa range  : [" << abcissa[0] << "," << abcissa[nchan-1] << "]" << LogIO::POST ;
	if ( minfreq <= abcissa[0] ) 
	  nminchan = 0 ;
	else {
	  //double cfreq = ( minfreq - abcissa[0] ) / resol ;
	  double cfreq = ( minfreq - abcissa[0] + 0.5 * resol ) / resol ;
	  nminchan = int(cfreq) + ( ( cfreq - int(cfreq) <= 0.5 ) ? 0 : 1 ) ;
	}
	if ( maxfreq >= abcissa[abcissa.size()-1] )
	  nmaxchan = abcissa.size() - 1 ;
	else {
	  //double cfreq = ( abcissa[abcissa.size()-1] - maxfreq ) / resol ;
	  double cfreq = ( abcissa[abcissa.size()-1] - maxfreq + 0.5 * resol ) / resol ;
	  nmaxchan = abcissa.size() - 1 - int(cfreq) - ( ( cfreq - int(cfreq) >= 0.5 ) ? 1 : 0 ) ;
	}
	//os << "channel range (" << irow << "): [" << nminchan << "," << nmaxchan << "]" << LogIO::POST ;
	if ( nmaxchan > nminchan ) {
	  newin[itable]->reshapeSpectrum( nminchan, nmaxchan, irow ) ;
	  int newchan = nmaxchan - nminchan + 1 ;
	  if ( count( freqIdUpdate.begin(), freqIdUpdate.end(), freqIdVec[itable][irow] ) == 0 && newchan < nchan )
	    freqIdUpdate.push_back( freqIdVec[itable][irow] ) ;
	}
	else {
	  throw(AipsError("Failed to pick up common part of frequency range.")) ;
	}
      }
      for ( uInt i = 0 ; i < freqIdUpdate.size() ; i++ ) {
	uInt freqId = freqIdUpdate[i] ;
	Double refpix ;
	Double refval ;
	Double increment ;
	
	// update row
	newin[itable]->frequencies().getEntry( refpix, refval, increment, freqId ) ; 
	refval = refval - ( refpix - nminchan ) * increment ;
	refpix = 0 ;
	newin[itable]->frequencies().setEntry( refpix, refval, increment, freqId ) ;
      }    
    }

    
    // reset spectra and flagtra: align spectral resolution
    //os << "Align spectral resolution" << LogIO::POST ;
    // gmaxdnu: the coarsest frequency resolution in the frequency group
    // gmemid: member index that have a resolution equal to gmaxdnu
    // gmaxdnu[numfreqgrp]
    // gmaxdnu: [dnu0, dnu1, ...]
    // gmemid[numfreqgrp]
    // gmemid: [id0, id1, ...]
    vector<double> gmaxdnu( freqgrp.size(), 0.0 ) ;
    vector<uInt> gmemid( freqgrp.size(), 0 ) ;
    for ( uInt igrp = 0 ; igrp < ifgrp.size() ; igrp++ ) {
      double maxdnu = 0.0 ;       // maximum (coarsest) frequency resolution 
      int minchan = INT_MAX ;     // minimum channel number
      Double refpixref = -1 ;     // reference of 'reference pixel'
      Double refvalref = -1 ;     // reference of 'reference frequency'
      Double refinc = -1 ;        // reference frequency resolution 
      uInt refreqid ;
      uInt reftable = INT_MAX;
      // process only if group member > 1
      if ( ifgrp[igrp].size() > 2 ) {
	// find minchan and maxdnu in each group
	for ( uInt imem = 0 ; imem < ifgrp[igrp].size()/2 ; imem++ ) {
	  uInt tableid = ifgrp[igrp][2*imem] ;
	  uInt rowid = ifgrp[igrp][2*imem+1] ;
	  vector<uInt>::iterator iter = find( freqIdVec[tableid].begin(), freqIdVec[tableid].end(), rowid ) ;
	  if ( iter != freqIdVec[tableid].end() ) {
	    uInt index = distance( freqIdVec[tableid].begin(), iter ) ;
	    vector<double> abcissa = newin[tableid]->getAbcissa( index ) ;
	    int nchan = abcissa.size() ;
	    double dnu = abcissa[1] - abcissa[0] ;
	    //os << "GROUP " << igrp << " (" << tableid << "," << rowid << "): nchan = " << nchan << " (minchan = " << minchan << ")" << LogIO::POST ; 
	    if ( nchan < minchan ) {
	      minchan = nchan ;
	      maxdnu = dnu ;
	      newin[tableid]->frequencies().getEntry( refpixref, refvalref, refinc, rowid ) ;
	      refreqid = rowid ;
	      reftable = tableid ;
	    }
	  }
	}
	// regrid spectra in each group
	os << "GROUP " << igrp << endl ;
	os << "   Channel number is adjusted to " << minchan << endl ;
	os << "   Corresponding frequency resolution is " << maxdnu << "Hz" << LogIO::POST ;
	for ( uInt imem = 0 ; imem < ifgrp[igrp].size()/2 ; imem++ ) {
	  uInt tableid = ifgrp[igrp][2*imem] ;
	  uInt rowid = ifgrp[igrp][2*imem+1] ;
	  freqIDCol.attach( newin[tableid]->table(), "FREQ_ID" ) ;
	  //os << "tableid = " << tableid << " rowid = " << rowid << ": " << LogIO::POST ;
	  //os << "   regridChannel applied to " ;
	  if ( tableid != reftable ) 
	    refreqid = newin[tableid]->frequencies().addEntry( refpixref, refvalref, refinc ) ;
	  for ( uInt irow = 0 ; irow < newin[tableid]->table().nrow() ; irow++ ) {
	    uInt tfreqid = freqIdVec[tableid][irow] ;
	    if ( tfreqid == rowid ) {	  
	      //os << irow << " " ;
	      newin[tableid]->regridChannel( minchan, maxdnu, irow ) ;
	      freqIDCol.put( irow, refreqid ) ;
	      freqIdVec[tableid][irow] = refreqid ;
	    }
	  }
	  //os << LogIO::POST ;
	} 
      }
      else {
	uInt tableid = ifgrp[igrp][0] ;
	uInt rowid = ifgrp[igrp][1] ;
	vector<uInt>::iterator iter = find( freqIdVec[tableid].begin(), freqIdVec[tableid].end(), rowid ) ;
	if ( iter != freqIdVec[tableid].end() ) {
	  uInt index = distance( freqIdVec[tableid].begin(), iter ) ;
	  vector<double> abcissa = newin[tableid]->getAbcissa( index ) ;
	  minchan = abcissa.size() ;
	  maxdnu = abcissa[1] - abcissa[0] ;
	}
      }
      for ( uInt i = 0 ; i < freqgrp.size() ; i++ ) {
	if ( count( freqgrp[i].begin(), freqgrp[i].end(), igrp ) > 0 ) {
	  if ( maxdnu > gmaxdnu[i] ) {
	    gmaxdnu[i] = maxdnu ;
	    gmemid[i] = igrp ;
	  }
	  break ;
	}
      }
    }

    // set back coordinfo
    for ( uInt itable = 0 ; itable < insize ; itable++ ) {
      vector<string> coordinfo = newin[itable]->getCoordInfo() ;
      coordinfo[0] = oldinfo[itable] ;
      newin[itable]->setCoordInfo( coordinfo ) ;
    }      

    // accumulate all rows into the first table 
    // NOTE: assumed in.size() = 1
    vector< CountedPtr<Scantable> > tmp( 1 ) ;
    if ( newin.size() == 1 )
      tmp[0] = newin[0] ;
    else 
      tmp[0] = merge( newin ) ;

    //return tmp[0] ;

    // average
    CountedPtr<Scantable> tmpout = average( tmp, mask, weight, avmode ) ;

    //return tmpout ;

    // combine frequency group
    os << "Combine spectra based on frequency grouping" << LogIO::POST ;
    os << "IFNO is renumbered as frequency group ID (see above)" << LogIO::POST ;
    vector<string> coordinfo = tmpout->getCoordInfo() ;
    oldinfo[0] = coordinfo[0] ;
    coordinfo[0] = "Hz" ;
    tmpout->setCoordInfo( coordinfo ) ;
    // create proformas of output table
    stringstream taqlstream ;
    taqlstream << "SELECT FROM $1 WHERE IFNO IN [" ;
    for ( uInt i = 0 ; i < gmemid.size() ; i++ ) {
      taqlstream << gmemid[i] ;
      if ( i < gmemid.size() - 1 )
	taqlstream << "," ;
      else
	taqlstream << "]" ;
    }
    string taql = taqlstream.str() ;
    //os << "taql = " << taql << LogIO::POST ;
    STSelector selector = STSelector() ;
    selector.setTaQL( taql ) ;
    oldInsitu = insitu_ ;
    setInsitu( false ) ;
    out = getScantable( tmpout, false ) ;
    setInsitu( oldInsitu ) ;
    out->setSelection( selector ) ;
    // regrid rows
    ifnoCol.attach( tmpout->table(), "IFNO" ) ;
    for ( uInt irow = 0 ; irow < tmpout->table().nrow() ; irow++ ) {
      uInt ifno = ifnoCol( irow ) ;
      for ( uInt igrp = 0 ; igrp < freqgrp.size() ; igrp++ ) {
	if ( count( freqgrp[igrp].begin(), freqgrp[igrp].end(), ifno ) > 0 ) {
	  vector<double> abcissa = tmpout->getAbcissa( irow ) ;
	  double bw = ( abcissa[1] - abcissa[0] ) * abcissa.size() ;
	  int nchan = (int)( bw / gmaxdnu[igrp] ) ;
	  tmpout->regridChannel( nchan, gmaxdnu[igrp], irow ) ;
	  break ;
	}
      }
    }
    // combine spectra
    ArrayColumn<Float> specColOut ;
    specColOut.attach( out->table(), "SPECTRA" ) ;
    ArrayColumn<uChar> flagColOut ;
    flagColOut.attach( out->table(), "FLAGTRA" ) ;
    ScalarColumn<uInt> ifnoColOut ;
    ifnoColOut.attach( out->table(), "IFNO" ) ;
    ScalarColumn<uInt> polnoColOut ;
    polnoColOut.attach( out->table(), "POLNO" ) ;
    ScalarColumn<uInt> freqidColOut ;
    freqidColOut.attach( out->table(), "FREQ_ID" ) ;
    MDirection::ScalarColumn dirColOut ;
    dirColOut.attach( out->table(), "DIRECTION" ) ;
    Table &tab = tmpout->table() ;
    Block<String> cols(1);
    cols[0] = String("POLNO") ;
    TableIterator iter( tab, cols ) ;
    bool done = false ;
    vector< vector<uInt> > sizes( freqgrp.size() ) ;
    while( !iter.pastEnd() ) {
      vector< vector<Float> > specout( freqgrp.size() ) ;
      vector< vector<uChar> > flagout( freqgrp.size() ) ;
      ArrayColumn<Float> specCols ;
      specCols.attach( iter.table(), "SPECTRA" ) ;
      ArrayColumn<uChar> flagCols ;
      flagCols.attach( iter.table(), "FLAGTRA" ) ;
      ifnoCol.attach( iter.table(), "IFNO" ) ;
      ScalarColumn<uInt> polnos ;
      polnos.attach( iter.table(), "POLNO" ) ;
      MDirection::ScalarColumn dircol ;
      dircol.attach( iter.table(), "DIRECTION" ) ;
      uInt polno = polnos( 0 ) ;
      //os << "POLNO iteration: " << polno << LogIO::POST ;
//       for ( uInt igrp = 0 ; igrp < freqgrp.size() ; igrp++ ) {
// 	sizes[igrp].resize( freqgrp[igrp].size() ) ;
// 	for ( uInt imem = 0 ; imem < freqgrp[igrp].size() ; imem++ ) {
// 	  for ( uInt irow = 0 ; irow < iter.table().nrow() ; irow++ ) {
// 	    uInt ifno = ifnoCol( irow ) ;
// 	    if ( ifno == freqgrp[igrp][imem] ) {
// 	      Vector<Float> spec = specCols( irow ) ;
// 	      Vector<uChar> flag = flagCols( irow ) ;
// 	      vector<Float> svec ;
// 	      spec.tovector( svec ) ;
// 	      vector<uChar> fvec ;
// 	      flag.tovector( fvec ) ;
// 	      //os << "spec.size() = " << svec.size() << " fvec.size() = " << fvec.size() << LogIO::POST ;
// 	      specout[igrp].insert( specout[igrp].end(), svec.begin(), svec.end() ) ;
// 	      flagout[igrp].insert( flagout[igrp].end(), fvec.begin(), fvec.end() ) ;
// 	      //os << "specout[" << igrp << "].size() = " << specout[igrp].size() << LogIO::POST ;
// 	      sizes[igrp][imem] = spec.nelements() ;
// 	    }
// 	  }
// 	}
// 	for ( uInt irow = 0 ; irow < out->table().nrow() ; irow++ ) {
// 	  uInt ifout = ifnoColOut( irow ) ;
// 	  uInt polout = polnoColOut( irow ) ;
// 	  if ( ifout == gmemid[igrp] && polout == polno ) {
// 	    // set SPECTRA and FRAGTRA
// 	    Vector<Float> newspec( specout[igrp] ) ;
// 	    Vector<uChar> newflag( flagout[igrp] ) ;
// 	    specColOut.put( irow, newspec ) ;
// 	    flagColOut.put( irow, newflag ) ;
// 	    // IFNO renumbering
// 	    ifnoColOut.put( irow, igrp ) ;
// 	  }
// 	} 
//       }
      // get a list of number of channels for each frequency group member
      if ( !done ) {
        for ( uInt igrp = 0 ; igrp < freqgrp.size() ; igrp++ ) {
          sizes[igrp].resize( freqgrp[igrp].size() ) ;
          for ( uInt imem = 0 ; imem < freqgrp[igrp].size() ; imem++ ) {
            for ( uInt irow = 0 ; irow < iter.table().nrow() ; irow++ ) {
              uInt ifno = ifnoCol( irow ) ;
              if ( ifno == freqgrp[igrp][imem] ) {
                Vector<Float> spec = specCols( irow ) ;
                sizes[igrp][imem] = spec.nelements() ;
                break ;
              }                
            }
          }
        }
        done = true ;
      }
      // combine spectra
      for ( uInt irow = 0 ; irow < out->table().nrow() ; irow++ ) {
        uInt polout = polnoColOut( irow ) ;
        if ( polout == polno ) {
          uInt ifout = ifnoColOut( irow ) ;
          Vector<Double> direction = dirColOut(irow).getAngle(Unit(String("rad"))).getValue() ;
          uInt igrp ;
          for ( uInt jgrp = 0 ; jgrp < freqgrp.size() ; jgrp++ ) {
            if ( ifout == gmemid[jgrp] ) {
              igrp = jgrp ;
              break ;
            }
          }
          for ( uInt imem = 0 ; imem < freqgrp[igrp].size() ; imem++ ) {
            for ( uInt jrow = 0 ; jrow < iter.table().nrow() ; jrow++ ) {
              uInt ifno = ifnoCol( jrow ) ;
              Vector<Double> tdir = dircol(jrow).getAngle(Unit(String("rad"))).getValue() ;
              //if ( ifno == freqgrp[igrp][imem] && allTrue( tdir == direction  ) ) {
              Double dx = tdir[0] - direction[0] ;
              Double dy = tdir[1] - direction[1] ;
              Double dd = sqrt( dx * dx + dy * dy ) ;
              //if ( ifno == freqgrp[igrp][imem] && allNearAbs( tdir, direction, tol ) ) {
              if ( ifno == freqgrp[igrp][imem] && dd <= tol ) {
                Vector<Float> spec = specCols( jrow ) ;
                Vector<uChar> flag = flagCols( jrow ) ;
                vector<Float> svec ;
                spec.tovector( svec ) ;
                vector<uChar> fvec ;
                flag.tovector( fvec ) ;
                //os << "spec.size() = " << svec.size() << " fvec.size() = " << fvec.size() << LogIO::POST ;
                specout[igrp].insert( specout[igrp].end(), svec.begin(), svec.end() ) ;
                flagout[igrp].insert( flagout[igrp].end(), fvec.begin(), fvec.end() ) ;
                //os << "specout[" << igrp << "].size() = " << specout[igrp].size() << LogIO::POST ;
              }
            }
          }
          // set SPECTRA and FRAGTRA
          Vector<Float> newspec( specout[igrp] ) ;
          Vector<uChar> newflag( flagout[igrp] ) ;
          specColOut.put( irow, newspec ) ;
          flagColOut.put( irow, newflag ) ;
          // IFNO renumbering
          ifnoColOut.put( irow, igrp ) ; 
        }
      }
      iter++ ;
    }
    // update FREQUENCIES subtable
    vector<bool> updated( freqgrp.size(), false ) ;
    for ( uInt igrp = 0 ; igrp < freqgrp.size() ; igrp++ ) {
      uInt index = 0 ;
      uInt pixShift = 0 ;
      while ( freqgrp[igrp][index] != gmemid[igrp] ) {
	pixShift += sizes[igrp][index++] ;
      }
      for ( uInt irow = 0 ; irow < out->table().nrow() ; irow++ ) {
	if ( ifnoColOut( irow ) == gmemid[igrp] && !updated[igrp] ) {
	  uInt freqidOut = freqidColOut( irow ) ;
	  //os << "freqgrp " << igrp << " freqidOut = " << freqidOut << LogIO::POST ;
	  double refpix ;
	  double refval ;
	  double increm ;
	  out->frequencies().getEntry( refpix, refval, increm, freqidOut ) ;
	  refpix += pixShift ;
	  out->frequencies().setEntry( refpix, refval, increm, freqidOut ) ;
	  updated[igrp] = true ;
	}
      }
    }

    //out = tmpout ;

    coordinfo = tmpout->getCoordInfo() ;
    coordinfo[0] = oldinfo[0] ;
    tmpout->setCoordInfo( coordinfo ) ;
  }
  else {
    // simple average
    out =  average( in, mask, weight, avmode ) ;
  }
  
  return out ;
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

    // sky scan
    STSelector sel = STSelector() ;
    types.push_back( SrcType::SKY ) ;
    sel.setTypes( types ) ;
    s->setSelection( sel ) ;
    vector< CountedPtr<Scantable> > tmp( 1, getScantable( s, false ) ) ;
    CountedPtr<Scantable> asky = average( tmp, masks, "TINT", "SCAN" ) ;
    s->unsetSelection() ;
    sel.reset() ;
    types.clear() ;

    // hot scan
    types.push_back( SrcType::HOT ) ;
    sel.setTypes( types ) ;
    s->setSelection( sel ) ;
    tmp.clear() ;
    tmp.push_back( getScantable( s, false ) ) ;
    CountedPtr<Scantable> ahot = average( tmp, masks, "TINT", "SCAN" ) ;
    s->unsetSelection() ;
    sel.reset() ;
    types.clear() ;
    
    // cold scan
    CountedPtr<Scantable> acold ;
//     types.push_back( SrcType::COLD ) ;
//     sel.setTypes( types ) ;
//     s->setSelection( sel ) ;
//     tmp.clear() ;
//     tmp.push_back( getScantable( s, false ) ) ;
//     CountedPtr<Scantable> acold = average( tmp, masks, "TINT", "SCNAN" ) ;
//     s->unsetSelection() ;
//     sel.reset() ;
//     types.clear() ;

    // off scan
    types.push_back( SrcType::PSOFF ) ;
    sel.setTypes( types ) ;
    s->setSelection( sel ) ;
    tmp.clear() ;
    tmp.push_back( getScantable( s, false ) ) ;
    CountedPtr<Scantable> aoff = average( tmp, masks, "TINT", "SCAN" ) ;
    s->unsetSelection() ;
    sel.reset() ;
    types.clear() ;
    
    // on scan
    bool insitu = insitu_ ;
    insitu_ = false ;
    CountedPtr<Scantable> out = getScantable( s, true ) ;
    insitu_ = insitu ;
    types.push_back( SrcType::PSON ) ;
    sel.setTypes( types ) ;
    s->setSelection( sel ) ;
    TableCopy::copyRows( out->table(), s->table() ) ;
    s->unsetSelection() ;
    sel.reset() ;
    types.clear() ;
    
    // process each on scan
    ArrayColumn<Float> tsysCol ;
    tsysCol.attach( out->table(), "TSYS" ) ;
    for ( int i = 0 ; i < out->nrow() ; i++ ) {
      vector<float> sp = getCalibratedSpectra( out, aoff, asky, ahot, acold, i, antname ) ;
      out->setSpectrum( sp, i ) ;
      string reftime = out->getTime( i ) ;
      vector<int> ii( 1, out->getIF( i ) ) ;
      vector<int> ib( 1, out->getBeam( i ) ) ;
      vector<int> ip( 1, out->getPol( i ) ) ;
      sel.setIFs( ii ) ;
      sel.setBeams( ib ) ;
      sel.setPolarizations( ip ) ;
      asky->setSelection( sel ) ;    
      vector<float> sptsys = getTsysFromTime( reftime, asky, "linear" ) ;
      const Vector<Float> Vtsys( sptsys ) ;
      tsysCol.put( i, Vtsys ) ;
      asky->unsetSelection() ;
      sel.reset() ;
    }

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
    vector<bool> masks = s->getMask( 0 ) ;
    
    // off scan
    STSelector sel = STSelector() ;
    vector<int> types ;
    types.push_back( SrcType::PSOFF ) ;
    sel.setTypes( types ) ;
    s->setSelection( sel ) ;
    // TODO 2010/01/08 TN
    // Grouping by time should be needed before averaging.
    // Each group must have own unique SCANNO (should be renumbered).
    // See PIPELINE/SDCalibration.py
    CountedPtr<Scantable> soff = getScantable( s, false ) ;
    Table ttab = soff->table() ;
    ROScalarColumn<Double> timeCol( ttab, "TIME" ) ;
    uInt nrow = timeCol.nrow() ;
    Vector<Double> timeSep( nrow - 1 ) ;
    for ( uInt i = 0 ; i < nrow - 1 ; i++ ) {
      timeSep[i] = timeCol(i+1) - timeCol(i) ;
    }
    ScalarColumn<Double> intervalCol( ttab, "INTERVAL" ) ;
    Vector<Double> interval = intervalCol.getColumn() ;
    interval /= 86400.0 ;
    ScalarColumn<uInt> scanCol( ttab, "SCANNO" ) ;
    vector<uInt> glist ;
    for ( uInt i = 0 ; i < nrow - 1 ; i++ ) {
      double gap = 2.0 * timeSep[i] / ( interval[i] + interval[i+1] ) ;
      //cout << "gap[" << i << "]=" << setw(5) << gap << endl ;
      if ( gap > 1.1 ) {
        glist.push_back( i ) ;
      }
    }
    Vector<uInt> gaplist( glist ) ;
    //cout << "gaplist = " << gaplist << endl ;
    uInt newid = 0 ;
    for ( uInt i = 0 ; i < nrow ; i++ ) {
      scanCol.put( i, newid ) ;
      if ( i == gaplist[newid] ) {
        newid++ ;
      }
    }
    //cout << "new scancol = " << scanCol.getColumn() << endl ;
    vector< CountedPtr<Scantable> > tmp( 1, soff ) ;
    CountedPtr<Scantable> aoff = average( tmp, masks, "TINT", "SCAN" ) ;
    //cout << "aoff.nrow = " << aoff->nrow() << endl ;
    s->unsetSelection() ;
    sel.reset() ;
    types.clear() ;
    
    // on scan
    bool insitu = insitu_ ;
    insitu_ = false ;
    CountedPtr<Scantable> out = getScantable( s, true ) ;
    insitu_ = insitu ;
    types.push_back( SrcType::PSON ) ;
    sel.setTypes( types ) ;
    s->setSelection( sel ) ;
    TableCopy::copyRows( out->table(), s->table() ) ;
    s->unsetSelection() ;
    sel.reset() ;
    types.clear() ;
    
    // process each on scan
    ArrayColumn<Float> tsysCol ;
    tsysCol.attach( out->table(), "TSYS" ) ;
    for ( int i = 0 ; i < out->nrow() ; i++ ) {
      vector<float> sp = getCalibratedSpectra( out, aoff, i ) ;
      out->setSpectrum( sp, i ) ;
    }

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
  CountedPtr<Scantable> out ;

  if ( antname.find( "APEX" ) != string::npos ) {
    // APEX calibration
    // sky scan
    STSelector sel = STSelector() ;
    types.push_back( SrcType::FLOSKY ) ;
    sel.setTypes( types ) ;
    s->setSelection( sel ) ;
    vector< CountedPtr<Scantable> > tmp( 1, getScantable( s, false ) ) ;
    CountedPtr<Scantable> askylo = average( tmp, masks, "TINT", "SCAN" ) ;
    s->unsetSelection() ;
    sel.reset() ;
    types.clear() ;
    types.push_back( SrcType::FHISKY ) ;
    sel.setTypes( types ) ;
    s->setSelection( sel ) ;
    tmp.clear() ;
    tmp.push_back( getScantable( s, false ) ) ;
    CountedPtr<Scantable> askyhi = average( tmp, masks, "TINT", "SCAN" ) ;
    s->unsetSelection() ;
    sel.reset() ;
    types.clear() ;
    
    // hot scan
    types.push_back( SrcType::FLOHOT ) ;
    sel.setTypes( types ) ;
    s->setSelection( sel ) ;
    tmp.clear() ;
    tmp.push_back( getScantable( s, false ) ) ;
    CountedPtr<Scantable> ahotlo = average( tmp, masks, "TINT", "SCAN" ) ;
    s->unsetSelection() ;
    sel.reset() ;
    types.clear() ;
    types.push_back( SrcType::FHIHOT ) ;
    sel.setTypes( types ) ;
    s->setSelection( sel ) ;
    tmp.clear() ;
    tmp.push_back( getScantable( s, false ) ) ;
    CountedPtr<Scantable> ahothi = average( tmp, masks, "TINT", "SCAN" ) ;
    s->unsetSelection() ;
    sel.reset() ;
    types.clear() ;
    
    // cold scan
    CountedPtr<Scantable> acoldlo, acoldhi ;
//     types.push_back( SrcType::FLOCOLD ) ;
//     sel.setTypes( types ) ;
//     s->setSelection( sel ) ;
//     tmp.clear() ;
//     tmp.push_back( getScantable( s, false ) ) ;
//     CountedPtr<Scantable> acoldlo = average( tmp, masks, "TINT", "SCAN" ) ;
//     s->unsetSelection() ;
//     sel.reset() ;
//     types.clear() ;
//     types.push_back( SrcType::FHICOLD ) ;
//     sel.setTypes( types ) ;
//     s->setSelection( sel ) ;
//     tmp.clear() ;
//     tmp.push_back( getScantable( s, false ) ) ;
//     CountedPtr<Scantable> acoldhi = average( tmp, masks, "TINT", "SCAN" ) ;
//     s->unsetSelection() ;
//     sel.reset() ;
//     types.clear() ;

    // ref scan
    bool insitu = insitu_ ;
    insitu_ = false ;
    sref = getScantable( s, true ) ;
    insitu_ = insitu ;
    types.push_back( SrcType::FSLO ) ;
    sel.setTypes( types ) ;
    s->setSelection( sel ) ;
    TableCopy::copyRows( sref->table(), s->table() ) ;
    s->unsetSelection() ;
    sel.reset() ;
    types.clear() ;
    
    // sig scan
    insitu_ = false ;
    ssig = getScantable( s, true ) ;
    insitu_ = insitu ;
    types.push_back( SrcType::FSHI ) ;
    sel.setTypes( types ) ;
    s->setSelection( sel ) ;
    TableCopy::copyRows( ssig->table(), s->table() ) ;
    s->unsetSelection() ;
    sel.reset() ;  
    types.clear() ;
          
    if ( apexcalmode == 0 ) {
      // APEX fs data without off scan
      // process each sig and ref scan
      ArrayColumn<Float> tsysCollo ;
      tsysCollo.attach( ssig->table(), "TSYS" ) ;
      ArrayColumn<Float> tsysColhi ;
      tsysColhi.attach( sref->table(), "TSYS" ) ;
      for ( int i = 0 ; i < ssig->nrow() ; i++ ) {
        vector< CountedPtr<Scantable> > sky( 2 ) ;
        sky[0] = askylo ;
        sky[1] = askyhi ;
        vector< CountedPtr<Scantable> > hot( 2 ) ;
        hot[0] = ahotlo ;
        hot[1] = ahothi ;
        vector< CountedPtr<Scantable> > cold( 2 ) ;
        //cold[0] = acoldlo ;
        //cold[1] = acoldhi ;
        vector<float> sp = getFSCalibratedSpectra( ssig, sref, sky, hot, cold, i ) ;
        ssig->setSpectrum( sp, i ) ;
        string reftime = ssig->getTime( i ) ;
        vector<int> ii( 1, ssig->getIF( i ) ) ;
        vector<int> ib( 1, ssig->getBeam( i ) ) ;
        vector<int> ip( 1, ssig->getPol( i ) ) ;
        sel.setIFs( ii ) ;
        sel.setBeams( ib ) ;
        sel.setPolarizations( ip ) ;
        askylo->setSelection( sel ) ;
        vector<float> sptsys = getTsysFromTime( reftime, askylo, "linear" ) ;
        const Vector<Float> Vtsyslo( sptsys ) ;
        tsysCollo.put( i, Vtsyslo ) ;
        askylo->unsetSelection() ;
        sel.reset() ;
        sky[0] = askyhi ;
        sky[1] = askylo ;
        hot[0] = ahothi ;
        hot[1] = ahotlo ;
        cold[0] = acoldhi ;
        cold[1] = acoldlo ;
        sp = getFSCalibratedSpectra( sref, ssig, sky, hot, cold, i ) ;
        sref->setSpectrum( sp, i ) ;
        reftime = sref->getTime( i ) ;
        ii[0] = sref->getIF( i )  ;
        ib[0] = sref->getBeam( i ) ;
        ip[0] = sref->getPol( i ) ;
        sel.setIFs( ii ) ;
        sel.setBeams( ib ) ;
        sel.setPolarizations( ip ) ;
        askyhi->setSelection( sel ) ;    
        sptsys = getTsysFromTime( reftime, askyhi, "linear" ) ;
        const Vector<Float> Vtsyshi( sptsys ) ;
        tsysColhi.put( i, Vtsyshi ) ;
        askyhi->unsetSelection() ;
        sel.reset() ;
      }
    }
    else if ( apexcalmode == 1 ) {
      // APEX fs data with off scan
      // off scan
      types.push_back( SrcType::FLOOFF ) ;
      sel.setTypes( types ) ;
      s->setSelection( sel ) ;
      tmp.clear() ;
      tmp.push_back( getScantable( s, false ) ) ;
      CountedPtr<Scantable> aofflo = average( tmp, masks, "TINT", "SCAN" ) ;
      s->unsetSelection() ;
      sel.reset() ;
      types.clear() ;
      types.push_back( SrcType::FHIOFF ) ;
      sel.setTypes( types ) ;
      s->setSelection( sel ) ;
      tmp.clear() ;
      tmp.push_back( getScantable( s, false ) ) ;
      CountedPtr<Scantable> aoffhi = average( tmp, masks, "TINT", "SCAN" ) ;
      s->unsetSelection() ;
      sel.reset() ;
      types.clear() ;
      
      // process each sig and ref scan
      ArrayColumn<Float> tsysCollo ;
      tsysCollo.attach( ssig->table(), "TSYS" ) ;
      ArrayColumn<Float> tsysColhi ;
      tsysColhi.attach( sref->table(), "TSYS" ) ;
      for ( int i = 0 ; i < ssig->nrow() ; i++ ) {
        vector<float> sp = getCalibratedSpectra( ssig, aofflo, askylo, ahotlo, acoldlo, i, antname ) ;
        ssig->setSpectrum( sp, i ) ;
        sp = getCalibratedSpectra( sref, aoffhi, askyhi, ahothi, acoldhi, i, antname ) ; 
        string reftime = ssig->getTime( i ) ;
        vector<int> ii( 1, ssig->getIF( i ) ) ;
        vector<int> ib( 1, ssig->getBeam( i ) ) ;
        vector<int> ip( 1, ssig->getPol( i ) ) ;
        sel.setIFs( ii ) ;
        sel.setBeams( ib ) ;
        sel.setPolarizations( ip ) ;
        askylo->setSelection( sel ) ;
        vector<float> sptsys = getTsysFromTime( reftime, askylo, "linear" ) ;
        const Vector<Float> Vtsyslo( sptsys ) ;
        tsysCollo.put( i, Vtsyslo ) ;
        askylo->unsetSelection() ;
        sel.reset() ;
        sref->setSpectrum( sp, i ) ;
        reftime = sref->getTime( i ) ;
        ii[0] = sref->getIF( i )  ;
        ib[0] = sref->getBeam( i ) ;
        ip[0] = sref->getPol( i ) ;
        sel.setIFs( ii ) ;
        sel.setBeams( ib ) ;
        sel.setPolarizations( ip ) ;
        askyhi->setSelection( sel ) ;    
        sptsys = getTsysFromTime( reftime, askyhi, "linear" ) ;
        const Vector<Float> Vtsyshi( sptsys ) ;
        tsysColhi.put( i, Vtsyshi ) ;
        askyhi->unsetSelection() ;
        sel.reset() ;
      }
    }
  }
  else {
    // non-APEX fs data
    // sky scan
    STSelector sel = STSelector() ;
    types.push_back( SrcType::SKY ) ;
    sel.setTypes( types ) ;
    s->setSelection( sel ) ;
    vector< CountedPtr<Scantable> > tmp( 1, getScantable( s, false ) ) ;
    CountedPtr<Scantable> asky = average( tmp, masks, "TINT", "SCAN" ) ;
    s->unsetSelection() ;
    sel.reset() ;
    types.clear() ;
    
    // hot scan
    types.push_back( SrcType::HOT ) ;
    sel.setTypes( types ) ;
    s->setSelection( sel ) ;
    tmp.clear() ;
    tmp.push_back( getScantable( s, false ) ) ;
    CountedPtr<Scantable> ahot = average( tmp, masks, "TINT", "SCAN" ) ;
    s->unsetSelection() ;
    sel.reset() ;
    types.clear() ;

    // cold scan
    CountedPtr<Scantable> acold ;
//     types.push_back( SrcType::COLD ) ;
//     sel.setTypes( types ) ;
//     s->setSelection( sel ) ;
//     tmp.clear() ;
//     tmp.push_back( getScantable( s, false ) ) ;
//     CountedPtr<Scantable> acold = average( tmp, masks, "TINT", "SCAN" ) ;
//     s->unsetSelection() ;
//     sel.reset() ;
//     types.clear() ;
   
    // ref scan
    bool insitu = insitu_ ;
    insitu_ = false ;
    sref = getScantable( s, true ) ;
    insitu_ = insitu ;
    types.push_back( SrcType::FSOFF ) ;
    sel.setTypes( types ) ;
    s->setSelection( sel ) ;
    TableCopy::copyRows( sref->table(), s->table() ) ;
    s->unsetSelection() ;
    sel.reset() ;
    types.clear() ;
    
    // sig scan
    insitu_ = false ;
    ssig = getScantable( s, true ) ;
    insitu_ = insitu ;
    types.push_back( SrcType::FSON ) ;
    sel.setTypes( types ) ;
    s->setSelection( sel ) ;
    TableCopy::copyRows( ssig->table(), s->table() ) ;
    s->unsetSelection() ;
    sel.reset() ;
    types.clear() ;

    // process each sig and ref scan
    ArrayColumn<Float> tsysColsig ;
    tsysColsig.attach( ssig->table(), "TSYS" ) ;
    ArrayColumn<Float> tsysColref ;
    tsysColref.attach( ssig->table(), "TSYS" ) ;
    for ( int i = 0 ; i < ssig->nrow() ; i++ ) {
      vector<float> sp = getFSCalibratedSpectra( ssig, sref, asky, ahot, acold, i ) ;
      ssig->setSpectrum( sp, i ) ;
      string reftime = ssig->getTime( i ) ;
      vector<int> ii( 1, ssig->getIF( i ) ) ;
      vector<int> ib( 1, ssig->getBeam( i ) ) ;
      vector<int> ip( 1, ssig->getPol( i ) ) ;
      sel.setIFs( ii ) ;
      sel.setBeams( ib ) ;
      sel.setPolarizations( ip ) ;
      asky->setSelection( sel ) ;
      vector<float> sptsys = getTsysFromTime( reftime, asky, "linear" ) ;
      const Vector<Float> Vtsys( sptsys ) ;
      tsysColsig.put( i, Vtsys ) ;
      asky->unsetSelection() ;
      sel.reset() ;
      sp = getFSCalibratedSpectra( sref, ssig, asky, ahot, acold, i ) ;
      sref->setSpectrum( sp, i ) ;
      tsysColref.put( i, Vtsys ) ;
    }
  }

  // do folding if necessary
  Table sigtab = ssig->table() ;
  Table reftab = sref->table() ;
  ScalarColumn<uInt> sigifnoCol ;
  ScalarColumn<uInt> refifnoCol ;
  ScalarColumn<uInt> sigfidCol ;
  ScalarColumn<uInt> reffidCol ;
  Int nchan = (Int)ssig->nchan() ;
  sigifnoCol.attach( sigtab, "IFNO" ) ;
  refifnoCol.attach( reftab, "IFNO" ) ;
  sigfidCol.attach( sigtab, "FREQ_ID" ) ;
  reffidCol.attach( reftab, "FREQ_ID" ) ;
  Vector<uInt> sfids( sigfidCol.getColumn() ) ;
  Vector<uInt> rfids( reffidCol.getColumn() ) ;
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
  CountedPtr<Scantable> out ;

  return out ;
}

vector<float> STMath::getSpectrumFromTime( string reftime, 
                                           CountedPtr<Scantable>& s,
                                           string mode ) 
{
  LogIO os( LogOrigin( "STMath", "getSpectrumFromTime", WHERE ) ) ;
  vector<float> sp ;

  if ( s->nrow() == 0 ) {
    os << LogIO::SEVERE << "No spectra in the input scantable. Return empty spectrum." << LogIO::POST ;
    return sp ;
  }
  else if ( s->nrow() == 1 ) {
    //os << "use row " << 0 << " (scanno = " << s->getScan( 0 ) << ")" << LogIO::POST ;
    return s->getSpectrum( 0 ) ; 
  }
  else {
    vector<int> idx = getRowIdFromTime( reftime, s ) ;
    if ( mode == "before" ) {
      int id = -1 ;
      if ( idx[0] != -1 ) {
        id = idx[0] ;
      }
      else if ( idx[1] != -1 ) {
        os << LogIO::WARN << "Failed to find a scan before reftime. return a spectrum just after the reftime." << LogIO::POST ;
        id = idx[1] ;
      }
      //os << "use row " << id << " (scanno = " << s->getScan( id ) << ")" << LogIO::POST ;
      sp = s->getSpectrum( id ) ;
    }
    else if ( mode == "after" ) {
      int id = -1 ;
      if ( idx[1] != -1 ) {
        id = idx[1] ;
      }
      else if ( idx[0] != -1 ) {
        os << LogIO::WARN << "Failed to find a scan after reftime. return a spectrum just before the reftime." << LogIO::POST ;
        id = idx[1] ;
      }
      //os << "use row " << id << " (scanno = " << s->getScan( id ) << ")" << LogIO::POST ;
      sp = s->getSpectrum( id ) ;
    }
    else if ( mode == "nearest" ) {
      int id = -1 ;
      if ( idx[0] == -1 ) {
        id = idx[1] ;
      }
      else if ( idx[1] == -1 ) {
        id = idx[0] ;
      }
      else if ( idx[0] == idx[1] ) {
        id = idx[0] ;
      }
      else {
        double t0 = getMJD( s->getTime( idx[0] ) ) ;
        double t1 = getMJD( s->getTime( idx[1] ) ) ;
        double tref = getMJD( reftime ) ;
        if ( abs( t0 - tref ) > abs( t1 - tref ) ) {
          id = idx[1] ;
        }
        else {
          id = idx[0] ;
        }
      }
      //os << "use row " << id << " (scanno = " << s->getScan( id ) << ")" << LogIO::POST ;
      sp = s->getSpectrum( id ) ;      
    }
    else if ( mode == "linear" ) {
      if ( idx[0] == -1 ) {
        // use after
        os << LogIO::WARN << "Failed to interpolate. return a spectrum just after the reftime." << LogIO::POST ;
        int id = idx[1] ;
        //os << "use row " << id << " (scanno = " << s->getScan( id ) << ")" << LogIO::POST ;
        sp = s->getSpectrum( id ) ;
      }
      else if ( idx[1] == -1 ) {
        // use before
        os << LogIO::WARN << "Failed to interpolate. return a spectrum just before the reftime." << LogIO::POST ;
        int id = idx[0] ;
        //os << "use row " << id << " (scanno = " << s->getScan( id ) << ")" << LogIO::POST ;
        sp = s->getSpectrum( id ) ;
      }
      else if ( idx[0] == idx[1] ) {
        // use before
        //os << "No need to interporate." << LogIO::POST ;
        int id = idx[0] ;
        //os << "use row " << id << " (scanno = " << s->getScan( id ) << ")" << LogIO::POST ;
        sp = s->getSpectrum( id ) ;
      }
      else {
        // do interpolation
        //os << "interpolate between " << idx[0] << " and " << idx[1] << " (scanno: " << s->getScan( idx[0] ) << ", " << s->getScan( idx[1] ) << ")" << LogIO::POST ;
        double t0 = getMJD( s->getTime( idx[0] ) ) ;
        double t1 = getMJD( s->getTime( idx[1] ) ) ;
        double tref = getMJD( reftime ) ;
        vector<float> sp0 = s->getSpectrum( idx[0] ) ;
        vector<float> sp1 = s->getSpectrum( idx[1] ) ;
        for ( unsigned int i = 0 ; i < sp0.size() ; i++ ) {
          float v = ( sp1[i] - sp0[i] ) / ( t1 - t0 ) * ( tref - t0 ) + sp0[i] ;
          sp.push_back( v ) ;
        }
      }
    }
    else {
      os << LogIO::SEVERE << "Unknown mode" << LogIO::POST ;
    }
    return sp ;
  }
}

double STMath::getMJD( string strtime ) 
{
  if ( strtime.find("/") == string::npos ) {
    // MJD time string
    return atof( strtime.c_str() ) ;
  }
  else {
    // string in YYYY/MM/DD/HH:MM:SS format
    uInt year = atoi( strtime.substr( 0, 4 ).c_str() ) ;
    uInt month = atoi( strtime.substr( 5, 2 ).c_str() ) ;
    uInt day = atoi( strtime.substr( 8, 2 ).c_str() ) ;
    uInt hour = atoi( strtime.substr( 11, 2 ).c_str() ) ;
    uInt minute = atoi( strtime.substr( 14, 2 ).c_str() ) ;
    uInt sec = atoi( strtime.substr( 17, 2 ).c_str() ) ;
    Time t( year, month, day, hour, minute, sec ) ;
    return t.modifiedJulianDay() ;
  }
}

vector<int> STMath::getRowIdFromTime( string reftime, CountedPtr<Scantable> &s )
{
  double reft = getMJD( reftime ) ;
  double dtmin = 1.0e100 ;
  double dtmax = -1.0e100 ;
  vector<double> dt ;
  int just_before = -1 ;
  int just_after = -1 ;
  for ( int i = 0 ; i < s->nrow() ; i++ ) {
    dt.push_back( getMJD( s->getTime( i ) ) - reft ) ; 
  }
  for ( unsigned int i = 0 ; i < dt.size() ; i++ ) {
    if ( dt[i] > 0.0 ) {
      // after reftime
      if ( dt[i] < dtmin ) {
        just_after = i ;
        dtmin = dt[i] ;
      }
    }
    else if ( dt[i] < 0.0 ) {
      // before reftime
      if ( dt[i] > dtmax ) {
        just_before = i ;
        dtmax = dt[i] ;
      }
    }
    else {
      // just a reftime
      just_before = i ;
      just_after = i ;
      dtmax = 0 ;
      dtmin = 0 ;
      break ;
    }
  }

  vector<int> v ;
  v.push_back( just_before ) ;
  v.push_back( just_after ) ;

  return v ;
}

vector<float> STMath::getTcalFromTime( string reftime, 
                                       CountedPtr<Scantable>& s,
                                       string mode ) 
{
  LogIO os( LogOrigin( "STMath", "getTcalFromTime", WHERE ) ) ;
  vector<float> tcal ;
  STTcal tcalTable = s->tcal() ;
  String time ;
  Vector<Float> tcalval ;
  if ( s->nrow() == 0 ) {
    os << LogIO::SEVERE << "No row in the input scantable. Return empty tcal." << LogIO::POST ;
    return tcal ;
  }
  else if ( s->nrow() == 1 ) {
    uInt tcalid = s->getTcalId( 0 ) ;
    //os << "use row " << 0 << " (tcalid = " << tcalid << ")" << LogIO::POST ;
    tcalTable.getEntry( time, tcalval, tcalid ) ;
    tcalval.tovector( tcal ) ;
    return tcal ;
  }
  else {
    vector<int> idx = getRowIdFromTime( reftime, s ) ;
    if ( mode == "before" ) {
      int id = -1 ;
      if ( idx[0] != -1 ) {
        id = idx[0] ;
      }
      else if ( idx[1] != -1 ) {
        os << LogIO::WARN << "Failed to find a scan before reftime. return a spectrum just after the reftime." << LogIO::POST ;
        id = idx[1] ;
      }
      uInt tcalid = s->getTcalId( id ) ;
      //os << "use row " << id << " (tcalid = " << tcalid << ")" << LogIO::POST ;
      tcalTable.getEntry( time, tcalval, tcalid ) ;
      tcalval.tovector( tcal ) ;
    }
    else if ( mode == "after" ) {
      int id = -1 ;
      if ( idx[1] != -1 ) {
        id = idx[1] ;
      }
      else if ( idx[0] != -1 ) {
        os << LogIO::WARN << "Failed to find a scan after reftime. return a spectrum just before the reftime." << LogIO::POST ;
        id = idx[1] ;
      }
      uInt tcalid = s->getTcalId( id ) ;
      //os << "use row " << id << " (tcalid = " << tcalid << ")" << LogIO::POST ;
      tcalTable.getEntry( time, tcalval, tcalid ) ;
      tcalval.tovector( tcal ) ;
    }
    else if ( mode == "nearest" ) {
      int id = -1 ;
      if ( idx[0] == -1 ) {
        id = idx[1] ;
      }
      else if ( idx[1] == -1 ) {
        id = idx[0] ;
      }
      else if ( idx[0] == idx[1] ) {
        id = idx[0] ;
      }
      else {
        double t0 = getMJD( s->getTime( idx[0] ) ) ;
        double t1 = getMJD( s->getTime( idx[1] ) ) ;
        double tref = getMJD( reftime ) ;
        if ( abs( t0 - tref ) > abs( t1 - tref ) ) {
          id = idx[1] ;
        }
        else {
          id = idx[0] ;
        }
      }
      uInt tcalid = s->getTcalId( id ) ;
      //os << "use row " << id << " (tcalid = " << tcalid << ")" << LogIO::POST ;
      tcalTable.getEntry( time, tcalval, tcalid ) ;
      tcalval.tovector( tcal ) ;
    }
    else if ( mode == "linear" ) {
      if ( idx[0] == -1 ) {
        // use after
        os << LogIO::WARN << "Failed to interpolate. return a spectrum just after the reftime." << LogIO::POST ;
        int id = idx[1] ;
        uInt tcalid = s->getTcalId( id ) ;
        //os << "use row " << id << " (tcalid = " << tcalid << ")" << LogIO::POST ;
        tcalTable.getEntry( time, tcalval, tcalid ) ;
        tcalval.tovector( tcal ) ;
      }
      else if ( idx[1] == -1 ) {
        // use before
        os << LogIO::WARN << "Failed to interpolate. return a spectrum just before the reftime." << LogIO::POST ;
        int id = idx[0] ;
        uInt tcalid = s->getTcalId( id ) ;
        //os << "use row " << id << " (tcalid = " << tcalid << ")" << LogIO::POST ;
        tcalTable.getEntry( time, tcalval, tcalid ) ;
        tcalval.tovector( tcal ) ;
      }
      else if ( idx[0] == idx[1] ) {
        // use before
        //os << "No need to interporate." << LogIO::POST ;
        int id = idx[0] ;
        uInt tcalid = s->getTcalId( id ) ;
        //os << "use row " << id << " (tcalid = " << tcalid << ")" << LogIO::POST ;
        tcalTable.getEntry( time, tcalval, tcalid ) ;
        tcalval.tovector( tcal ) ;
      }
      else {
        // do interpolation
        //os << "interpolate between " << idx[0] << " and " << idx[1] << " (scanno: " << s->getScan( idx[0] ) << ", " << s->getScan( idx[1] ) << ")" << LogIO::POST ;
        double t0 = getMJD( s->getTime( idx[0] ) ) ;
        double t1 = getMJD( s->getTime( idx[1] ) ) ;
        double tref = getMJD( reftime ) ;
        vector<float> tcal0 ;
        vector<float> tcal1 ;
        uInt tcalid0 = s->getTcalId( idx[0] ) ;
        uInt tcalid1 = s->getTcalId( idx[1] ) ;
        tcalTable.getEntry( time, tcalval, tcalid0 ) ;
        tcalval.tovector( tcal0 ) ;
        tcalTable.getEntry( time, tcalval, tcalid1 ) ;
        tcalval.tovector( tcal1 ) ;        
        for ( unsigned int i = 0 ; i < tcal0.size() ; i++ ) {
          float v = ( tcal1[i] - tcal0[i] ) / ( t1 - t0 ) * ( tref - t0 ) + tcal0[i] ;
          tcal.push_back( v ) ;
        }
      }
    }
    else {
      os << LogIO::SEVERE << "Unknown mode" << LogIO::POST ;
    }
    return tcal ;
  }
}

vector<float> STMath::getTsysFromTime( string reftime, 
                                       CountedPtr<Scantable>& s,
                                       string mode ) 
{
  LogIO os( LogOrigin( "STMath", "getTsysFromTime", WHERE ) ) ;
  ArrayColumn<Float> tsysCol ;
  tsysCol.attach( s->table(), "TSYS" ) ;
  vector<float> tsys ;
  String time ;
  Vector<Float> tsysval ;
  if ( s->nrow() == 0 ) {
    os << LogIO::SEVERE << "No row in the input scantable. Return empty tsys." << LogIO::POST ;
    return tsys ;
  }
  else if ( s->nrow() == 1 ) {
    //os << "use row " << 0 << LogIO::POST ;
    tsysval = tsysCol( 0 ) ;
    tsysval.tovector( tsys ) ;
    return tsys ;
  }
  else {
    vector<int> idx = getRowIdFromTime( reftime, s ) ;
    if ( mode == "before" ) {
      int id = -1 ;
      if ( idx[0] != -1 ) {
        id = idx[0] ;
      }
      else if ( idx[1] != -1 ) {
        os << LogIO::WARN << "Failed to find a scan before reftime. return a spectrum just after the reftime." << LogIO::POST ;
        id = idx[1] ;
      }
      //os << "use row " << id << LogIO::POST ;
      tsysval = tsysCol( id ) ;
      tsysval.tovector( tsys ) ;
    }
    else if ( mode == "after" ) {
      int id = -1 ;
      if ( idx[1] != -1 ) {
        id = idx[1] ;
      }
      else if ( idx[0] != -1 ) {
        os << LogIO::WARN << "Failed to find a scan after reftime. return a spectrum just before the reftime." << LogIO::POST ;
        id = idx[1] ;
      }
      //os << "use row " << id << LogIO::POST ;
      tsysval = tsysCol( id ) ;
      tsysval.tovector( tsys ) ;
    }
    else if ( mode == "nearest" ) {
      int id = -1 ;
      if ( idx[0] == -1 ) {
        id = idx[1] ;
      }
      else if ( idx[1] == -1 ) {
        id = idx[0] ;
      }
      else if ( idx[0] == idx[1] ) {
        id = idx[0] ;
      }
      else {
        double t0 = getMJD( s->getTime( idx[0] ) ) ;
        double t1 = getMJD( s->getTime( idx[1] ) ) ;
        double tref = getMJD( reftime ) ;
        if ( abs( t0 - tref ) > abs( t1 - tref ) ) {
          id = idx[1] ;
        }
        else {
          id = idx[0] ;
        }
      }
      //os << "use row " << id << LogIO::POST ;
      tsysval = tsysCol( id ) ;
      tsysval.tovector( tsys ) ;
    }
    else if ( mode == "linear" ) {
      if ( idx[0] == -1 ) {
        // use after
        os << LogIO::WARN << "Failed to interpolate. return a spectrum just after the reftime." << LogIO::POST ;
        int id = idx[1] ;
        //os << "use row " << id << LogIO::POST ;
        tsysval = tsysCol( id ) ;
        tsysval.tovector( tsys ) ;
      }
      else if ( idx[1] == -1 ) {
        // use before
        os << LogIO::WARN << "Failed to interpolate. return a spectrum just before the reftime." << LogIO::POST ;
        int id = idx[0] ;
        //os << "use row " << id << LogIO::POST ;
        tsysval = tsysCol( id ) ;
        tsysval.tovector( tsys ) ;
      }
      else if ( idx[0] == idx[1] ) {
        // use before
        //os << "No need to interporate." << LogIO::POST ;
        int id = idx[0] ;
        //os << "use row " << id << LogIO::POST ;
        tsysval = tsysCol( id ) ;
        tsysval.tovector( tsys ) ;
      }
      else {
        // do interpolation
        //os << "interpolate between " << idx[0] << " and " << idx[1] << " (scanno: " << s->getScan( idx[0] ) << ", " << s->getScan( idx[1] ) << ")" << LogIO::POST ;
        double t0 = getMJD( s->getTime( idx[0] ) ) ;
        double t1 = getMJD( s->getTime( idx[1] ) ) ;
        double tref = getMJD( reftime ) ;
        vector<float> tsys0 ;
        vector<float> tsys1 ;
        tsysval = tsysCol( idx[0] ) ;
        tsysval.tovector( tsys0 ) ;
        tsysval = tsysCol( idx[1] ) ;
        tsysval.tovector( tsys1 ) ;        
        for ( unsigned int i = 0 ; i < tsys0.size() ; i++ ) {
          float v = ( tsys1[i] - tsys0[i] ) / ( t1 - t0 ) * ( tref - t0 ) + tsys0[i] ;
          tsys.push_back( v ) ;
        }
      }
    }
    else {
      os << LogIO::SEVERE << "Unknown mode" << LogIO::POST ;
    }
    return tsys ;
  }
}

vector<float> STMath::getCalibratedSpectra( CountedPtr<Scantable>& on,
                                            CountedPtr<Scantable>& off,
                                            CountedPtr<Scantable>& sky,
                                            CountedPtr<Scantable>& hot,
                                            CountedPtr<Scantable>& cold,
                                            int index,
                                            string antname ) 
{
  string reftime = on->getTime( index ) ;
  vector<int> ii( 1, on->getIF( index ) ) ;
  vector<int> ib( 1, on->getBeam( index ) ) ;
  vector<int> ip( 1, on->getPol( index ) ) ;
  vector<int> ic( 1, on->getScan( index ) ) ;
  STSelector sel = STSelector() ;
  sel.setIFs( ii ) ;
  sel.setBeams( ib ) ;
  sel.setPolarizations( ip ) ;
  sky->setSelection( sel ) ;
  hot->setSelection( sel ) ;
  //cold->setSelection( sel ) ;
  off->setSelection( sel ) ;
  vector<float> spsky = getSpectrumFromTime( reftime, sky, "linear" ) ;
  vector<float> sphot = getSpectrumFromTime( reftime, hot, "linear" ) ;
  //vector<float> spcold = getSpectrumFromTime( reftime, cold, "linear" ) ;
  vector<float> spoff = getSpectrumFromTime( reftime, off, "linear" ) ;
  vector<float> spec = on->getSpectrum( index ) ;
  vector<float> tcal = getTcalFromTime( reftime, sky, "linear" ) ;
  vector<float> sp( tcal.size() ) ;
  if ( antname.find( "APEX" ) != string::npos ) {
    // using gain array
    for ( unsigned int j = 0 ; j < tcal.size() ; j++ ) {
      float v = ( ( spec[j] - spoff[j] ) / spoff[j] )
        * ( spsky[j] / ( sphot[j] - spsky[j] ) ) * tcal[j] ;
      sp[j] = v ;
    }
  }
  else {
    // Chopper-Wheel calibration (Ulich & Haas 1976)
    for ( unsigned int j = 0 ; j < tcal.size() ; j++ ) {
      float v = ( spec[j] - spoff[j] ) / ( sphot[j] - spsky[j] ) * tcal[j] ;
      sp[j] = v ;
    }
  }
  sel.reset() ;
  sky->unsetSelection() ;
  hot->unsetSelection() ;
  //cold->unsetSelection() ;
  off->unsetSelection() ;

  return sp ;
}

vector<float> STMath::getCalibratedSpectra( CountedPtr<Scantable>& on,
                                            CountedPtr<Scantable>& off,
                                            int index ) 
{
  string reftime = on->getTime( index ) ;
  vector<int> ii( 1, on->getIF( index ) ) ;
  vector<int> ib( 1, on->getBeam( index ) ) ;
  vector<int> ip( 1, on->getPol( index ) ) ;
  vector<int> ic( 1, on->getScan( index ) ) ;
  STSelector sel = STSelector() ;
  sel.setIFs( ii ) ;
  sel.setBeams( ib ) ;
  sel.setPolarizations( ip ) ;
  off->setSelection( sel ) ;
  vector<float> spoff = getSpectrumFromTime( reftime, off, "linear" ) ;
  vector<float> spec = on->getSpectrum( index ) ;
  //vector<float> tcal = getTcalFromTime( reftime, sky, "linear" ) ;
  //vector<float> tsys = on->getTsysVec( index ) ;
  ArrayColumn<Float> tsysCol( on->table(), "TSYS" ) ;
  Vector<Float> tsys = tsysCol( index ) ;
  vector<float> sp( spec.size() ) ;
  // ALMA Calibration
  // 
  // Ta* = Tsys * ( ON - OFF ) / OFF
  //
  // 2010/01/07 Takeshi Nakazato
  unsigned int tsyssize = tsys.nelements() ;
  unsigned int spsize = sp.size() ;
  for ( unsigned int j = 0 ; j < sp.size() ; j++ ) {
    float tscale = 0.0 ;
    if ( tsyssize == spsize ) 
      tscale = tsys[j] ;
    else 
      tscale = tsys[0] ;
    float v = tscale * ( spec[j] - spoff[j] ) / spoff[j] ;
    sp[j] = v ;
  }
  sel.reset() ;
  off->unsetSelection() ;

  return sp ;
}

vector<float> STMath::getFSCalibratedSpectra( CountedPtr<Scantable>& sig,
                                              CountedPtr<Scantable>& ref,
                                              CountedPtr<Scantable>& sky,
                                              CountedPtr<Scantable>& hot,
                                              CountedPtr<Scantable>& cold,
                                              int index ) 
{
  string reftime = sig->getTime( index ) ;
  vector<int> ii( 1, sig->getIF( index ) ) ;
  vector<int> ib( 1, sig->getBeam( index ) ) ;
  vector<int> ip( 1, sig->getPol( index ) ) ;
  vector<int> ic( 1, sig->getScan( index ) ) ;
  STSelector sel = STSelector() ;
  sel.setIFs( ii ) ;
  sel.setBeams( ib ) ;
  sel.setPolarizations( ip ) ;
  sky->setSelection( sel ) ;
  hot->setSelection( sel ) ;
  //cold->setSelection( sel ) ;
  vector<float> spsky = getSpectrumFromTime( reftime, sky, "linear" ) ;
  vector<float> sphot = getSpectrumFromTime( reftime, hot, "linear" ) ;
  //vector<float> spcold = getSpectrumFromTime( reftime, cold, "linear" ) ;
  vector<float> spref = ref->getSpectrum( index ) ;
  vector<float> spsig = sig->getSpectrum( index ) ;
  vector<float> tcal = getTcalFromTime( reftime, sky, "linear" ) ;
  vector<float> sp( tcal.size() ) ;
  for ( unsigned int j = 0 ; j < tcal.size() ; j++ ) {
    float v = tcal[j] * spsky[j] / ( sphot[j] - spsky[j] ) * ( spsig[j] - spref[j] ) / spref[j] ;
    sp[j] = v ;
  }
  sel.reset() ;
  sky->unsetSelection() ;
  hot->unsetSelection() ;
  //cold->unsetSelection() ;

  return sp ;
}

vector<float> STMath::getFSCalibratedSpectra( CountedPtr<Scantable>& sig,
                                              CountedPtr<Scantable>& ref,
                                              vector< CountedPtr<Scantable> >& sky,
                                              vector< CountedPtr<Scantable> >& hot,
                                              vector< CountedPtr<Scantable> >& cold,
                                              int index ) 
{
  string reftime = sig->getTime( index ) ;
  vector<int> ii( 1, sig->getIF( index ) ) ;
  vector<int> ib( 1, sig->getBeam( index ) ) ;
  vector<int> ip( 1, sig->getPol( index ) ) ;
  vector<int> ic( 1, sig->getScan( index ) ) ;
  STSelector sel = STSelector() ;
  sel.setIFs( ii ) ;
  sel.setBeams( ib ) ;
  sel.setPolarizations( ip ) ;
  sky[0]->setSelection( sel ) ;
  hot[0]->setSelection( sel ) ;
  //cold[0]->setSelection( sel ) ;
  vector<float> spskys = getSpectrumFromTime( reftime, sky[0], "linear" ) ;
  vector<float> sphots = getSpectrumFromTime( reftime, hot[0], "linear" ) ;
  //vector<float> spcolds = getSpectrumFromTime( reftime, cold[0], "linear" ) ;
  vector<float> tcals = getTcalFromTime( reftime, sky[0], "linear" ) ;
  sel.reset() ;
  ii[0] = ref->getIF( index ) ;
  sel.setIFs( ii ) ;
  sel.setBeams( ib ) ;
  sel.setPolarizations( ip ) ;
  sky[1]->setSelection( sel ) ;
  hot[1]->setSelection( sel ) ;
  //cold[1]->setSelection( sel ) ;
  vector<float> spskyr = getSpectrumFromTime( reftime, sky[1], "linear" ) ;
  vector<float> sphotr = getSpectrumFromTime( reftime, hot[1], "linear" ) ;
  //vector<float> spcoldr = getSpectrumFromTime( reftime, cold[1], "linear" ) ;
  vector<float> tcalr = getTcalFromTime( reftime, sky[1], "linear" ) ;  
  vector<float> spref = ref->getSpectrum( index ) ;
  vector<float> spsig = sig->getSpectrum( index ) ;
  vector<float> sp( tcals.size() ) ;
  for ( unsigned int j = 0 ; j < tcals.size() ; j++ ) {
    float v = tcals[j] * spsig[j] / ( sphots[j] - spskys[j] ) - tcalr[j] * spref[j] / ( sphotr[j] - spskyr[j] ) ;
    sp[j] = v ;
  }
  sel.reset() ;
  sky[0]->unsetSelection() ;
  hot[0]->unsetSelection() ;
  //cold[0]->unsetSelection() ;
  sky[1]->unsetSelection() ;
  hot[1]->unsetSelection() ;
  //cold[1]->unsetSelection() ;

  return sp ;
}
