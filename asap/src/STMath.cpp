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

#include "MathUtils.h"
#include "RowAccumulator.h"
#include "STAttr.h"
#include "STMath.h"
#include "STSelector.h"

using namespace casa;

using namespace asap;

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
  if ( avmode == "SCAN" && in.size() != 1 )
    throw(AipsError("Can't perform 'SCAN' averaging on multiple tables.\n"
                    "Use merge first."));
  WeightType wtype = stringToWeight(weight);

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
  while (!iter.pastEnd()) {
    Table subt = iter.table();
    // copy the first row of this selection into the new table
    tout.addRow();
    TableCopy::copyRows(tout, subt, outrowCount, 0, 1);
    // re-index to 0
    if ( avmode != "SCAN" && avmode != "SOURCE" ) {
      scanColOut.put(outrowCount, uInt(0));
    }
    ++outrowCount;
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
    cout << rowstodelete << endl;
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
  }
  return out;
}

CountedPtr< Scantable > STMath::getScantable(const CountedPtr< Scantable >& in,
                                             bool droprows)
{
  if (insitu_) return in;
  else {
    // clone
    Scantable* tabp = new Scantable(*in, Bool(droprows));
    return CountedPtr<Scantable>(tabp);
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
  Table ons = tin(tin.col("SRCTYPE") == Int(0));
  Table offs = tin(tin.col("SRCTYPE") == Int(1));
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
  std::vector<int> scan1, scan2, beams;
  std::vector< vector<int> > scanpair;
  std::vector<string> calstate;
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
  calstate.push_back("*calon");
  calstate.push_back("*[^calon]");
  CountedPtr< Scantable > ws = getScantable(s, false);
  uInt l=0;
  while ( l < sctables.size() ) {
    for (uInt i=0; i < 2; i++) {
      for (uInt j=0; j < 2; j++) {
        for (uInt k=0; k < 2; k++) {
          sel.reset();
          sel.setScans(scanpair[i]);
          sel.setName(calstate[k]);
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

  //split the data
  sel.setName("*_fs");
  ws->setSelection(sel);
  sig = getScantable(ws,false);
  sel.reset();
  sel.setName("*_fs_calon");
  ws->setSelection(sel);
  sigwcal = getScantable(ws,false);
  sel.reset();
  sel.setName("*_fsr");
  ws->setSelection(sel);
  ref = getScantable(ws,false);
  sel.reset();
  sel.setName("*_fsr_calon");
  ws->setSelection(sel);
  refwcal = getScantable(ws,false);

  calsig = dototalpower(sigwcal, sig, tcal=tcal);
  calref = dototalpower(refwcal, ref, tcal=tcal);

  out1=dosigref(calsig,calref,smoothref,tsysv,tau); 
  out2=dosigref(calref,calsig,smoothref,tsysv,tau); 

  Table& tabout1=out1->table();
  Table& tabout2=out2->table();
  ROScalarColumn<uInt> freqidCol1(tabout1, "FREQ_ID");
  ROScalarColumn<uInt> freqidCol2(tabout2, "FREQ_ID");
  ROArrayColumn<Float> specCol(tabout2, "SPECTRA");
  Vector<Float> spec; specCol.get(0, spec);
  uInt nchan = spec.nelements();
  uInt freqid1; freqidCol1.get(0,freqid1);
  uInt freqid2; freqidCol2.get(0,freqid2);
  Double rp1, rp2, rv1, rv2, inc1, inc2;
  out1->frequencies().getEntry(rp1, rv1, inc1, freqid1);
  out2->frequencies().getEntry(rp2, rv2, inc2, freqid2);
  if (rp1==rp2) {
    Double foffset = rv1 - rv2;
    uInt choffset = static_cast<uInt>(foffset/abs(inc2));
    if (choffset >= nchan) {
      cerr<<"out-band frequency switching, no folding"<<endl;
      nofold = True;
    }
  }
 
  if (nofold) {
    std::vector< CountedPtr< Scantable > > tabs; 
    tabs.push_back(out1);
    tabs.push_back(out2);
    out = merge(tabs);
  }
  else { //folding is not implemented yet
    out = out1;
  }
    
  return out;
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
  for ( uInt i=0; i<tab.nrow(); ++i) {
    Float zdist = Float(C::pi_2) - elev(i);
    Float factor = exp(tau/cos(zdist));
    MaskedArray<Float> ma = maskedArray(specCol(i), flagCol(i));
    ma *= factor;
    specCol.put(i, ma.getArray());
    flagCol.put(i, flagsFromMA(ma));
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
      // log message: "ignoring scantable i, as it isn't
      // conformant with the other(s)"
      cerr << "oh oh" << endl;
      ++it;
      continue;
    }
    out->appendToHistoryTable((*it)->history());
    const Table& tab = (*it)->table();
    TableIterator scanit(tab, "SCANNO");
    while (!scanit.pastEnd()) {
      TableIterator freqit(scanit.table(), "FREQ_ID");
      while ( !freqit.pastEnd() ) {
        Table thetab = freqit.table();
        uInt nrow = tout.nrow();
        //tout.addRow(thetab.nrow());
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
  // test if user frame is different to base frame
  if ( in->frequencies().getFrameString(true)
       == in->frequencies().getFrameString(false) ) {
    throw(AipsError("Can't convert as no output frame has been set"
                    " (use set_freqframe) or it is aligned already."));
  }
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
    MDirection direction = dirCol(0);
    uInt nchan = sCol(0).nelements();
    while ( !fiter.pastEnd() ) {
      Table ftab = fiter.table();
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
      uInt id = out->frequencies().addEntry(sc2.referencePixel()[0],
                                            sc2.referenceValue()[0],
                                            sc2.increment()[0]);
      TableVector<uInt> tvec(ftab, "FREQ_ID");
      tvec = id;
      // create the "global" abcissa for alignment with same FREQ_ID
      Vector<Double> abc(nchan);
      Double w;
      for (uInt i=0; i<nchan; i++) {
        sC.toWorld(w,Double(i));
        abc[i] = w;
      }
      // cache abcissa for same time stamps, so iterate over those
      TableIterator timeiter(ftab, "TIME");
      while ( !timeiter.pastEnd() ) {
        Table tab = timeiter.table();
        ArrayColumn<Float> specCol(tab, "SPECTRA");
        ArrayColumn<uChar> flagCol(tab, "FLAGTRA");
        MEpoch::ROScalarColumn timeCol(tab, "TIME");
        // use align abcissa cache after the first row
        bool first = true;
        // these rows should be just be POLNO
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
      for (int k=0; k < flag.nelements(); ++k ) {
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
  if ( avmode == "SCAN" && in.size() != 1 )
    throw(AipsError("Can't perform 'SCAN' averaging on multiple tables.\n"
                    "Use merge first."));
  
  CountedPtr<Scantable> out ;     // processed result 
  if ( compel ) {
    std::vector< CountedPtr<Scantable> > newin ; // input for average process
    uInt insize = in.size() ;    // number of scantables 

    // TEST: do normal average in each table before IF grouping
    cout << "Do preliminary averaging" << endl ;
    vector< CountedPtr<Scantable> > tmpin( insize ) ;
    for ( uInt itable = 0 ; itable < insize ; itable++ ) {
      vector< CountedPtr<Scantable> > v( 1, in[itable] ) ;
      tmpin[itable] = average( v, mask, weight, avmode ) ;
    }

    // warning
    cout << "Average spectra with different spectral resolution" << endl ;
    cout << endl ;

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
    //cout << "Check IF settings in each table" << endl ;
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
	    //cout << "itable = " << itable << ": IF " << id << " is included in the list" << endl ;
	    vector<double> abcissa = tmpin[itable]->getAbcissa( irow ) ;
	    freqid[itable].push_back( id ) ;
	    iffreq[itable].push_back( abcissa[0] - 0.5 * ( abcissa[1] - abcissa[0] ) ) ;
	    iffreq[itable].push_back( abcissa[abcissa.size()-1] + 0.5 * ( abcissa[1] - abcissa[0] ) ) ;
	  }
	}
      }
    }

    // debug
    //cout << "IF settings summary:" << endl ;
    //for ( uInt i = 0 ; i < freqid.size() ; i++ ) {
    //cout << "   Table" << i << endl ;
    //for ( uInt j = 0 ; j < freqid[i].size() ; j++ ) {
    //cout << "      id = " << freqid[i][j] << " (min,max) = (" << iffreq[i][2*j] << "," << iffreq[i][2*j+1] << ")" << endl ;
    //}
    //}
    //cout << endl ;

    // IF grouping based on their frequency coverage
    //cout << "IF grouping based on their frequency coverage" << endl ;

    // IF group 
    // ifgrp[numgrp][nummember*2]
    // ifgrp = [table00, freqrow00, table01, freqrow01, ...],
    //         [table11, freqrow11, table11, freqrow11, ...],
    //         ...
    // ifgfreq[numgrp*2] 
    // ifgfreq = [min0, max0, min1, max1, ...]
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

    // freqgrp[numgrp][nummember]
    // freqgrp = [ifgrp00, ifgrp01, ifgrp02, ...],
    //           [ifgrp10, ifgrp11, ifgrp12, ...],
    //           ...
    // freqrange[numgrp*2]
    // freqrange = [min0, max0, min1, max1, ...]
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
    cout << "IF Group summary: " << endl ;
    cout << "   GROUP_ID [FREQ_MIN, FREQ_MAX]: (TABLE_ID, FREQ_ID)" << endl ;
    for ( uInt i = 0 ; i < ifgrp.size() ; i++ ) {
      cout << "   GROUP " << setw( 2 ) << i << " [" << ifgfreq[2*i] << "," << ifgfreq[2*i+1] << "]: " ;
      for ( uInt j = 0 ; j < ifgrp[i].size()/2 ; j++ ) {
	cout << "(" << ifgrp[i][2*j] << "," << ifgrp[i][2*j+1] << ") " ; 
      }
      cout << endl ;
    }
    cout << endl ;
    
    // print frequency group
    cout << "Frequency Group summary: " << endl ;
    cout << "   GROUP_ID [FREQ_MIN, FREQ_MAX]: IF_GROUP_ID" << endl ;
    for ( uInt i = 0 ; i < freqgrp.size() ; i++ ) {
      cout << "   GROUP " << setw( 2 ) << i << " [" << ifgfreq[2*freqgrp[i][0]] << "," << ifgfreq[2*freqgrp[i][freqgrp[i].size()-1]+1] << "]: " ;
      for ( uInt j = 0 ; j < freqgrp[i].size() ; j++ ) {
	cout << freqgrp[i][j] << " " ;
      }
      cout << endl ;
    }
    cout << endl ;

    // membership check
    // groups[numtable][numIF][nummembership]
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
    //for ( uInt i = 0 ; i < insize ; i++ ) {
    //cout << "Table " << i << endl ;
    //for ( uInt j = 0 ; j < groups[i].size() ; j++ ) {
    //cout << "   FREQ_ID " <<  setw( 2 ) << freqid[i][j] << ": " ;
    //for ( uInt k = 0 ; k < groups[i][j].size() ; k++ ) {
    //cout << setw( 2 ) << groups[i][j][k] << " " ;
    //}
    //cout << endl ;
    //}
    //}

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
      //cout << "Table " << setw(2) << itable << ": " ;
      for ( uInt ifrow = 0 ; ifrow < groups[itable].size() ; ifrow++ ) {
	addrow[itable].push_back( groups[itable][ifrow].size()-1 ) ;
	//cout << addrow[itable][ifrow] << " " ;
      }
      addtable[itable] = *max_element( addrow[itable].begin(), addrow[itable].end() ) ;
      //cout << "(" << addtable[itable] << ")" << endl ;
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
	//cout << "taql = " << taql << endl ;
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
    //cout << "IF Group summary: " << endl ;
    //cout << "   GROUP_ID [FREQ_MIN, FREQ_MAX]: (TABLE_ID, FREQ_ID)" << endl ;
    //for ( uInt i = 0 ; i < ifgrp.size() ; i++ ) {
    //cout << "   GROUP " << setw( 2 ) << i << " [" << ifgfreq[2*i] << "," << ifgfreq[2*i+1] << "]: " ;
    //for ( uInt j = 0 ; j < ifgrp[i].size()/2 ; j++ ) {
    //cout << "(" << ifgrp[i][2*j] << "," << ifgrp[i][2*j+1] << ") " ; 
    //}
    //cout << endl ;
    //}
    //cout << endl ;

    // reset SCANNO and IF: IF number is reset by the result of sortation 
    cout << "All scan number is set to 0" << endl ;
    //cout << "All IF number is set to IF group index" << endl ;
    cout << endl ;
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
    //cout << "Pick common frequency range and align resolution" << endl ;
    for ( uInt itable = 0 ; itable < insize ; itable++ ) {
      uInt rows = newin[itable]->nrow() ;
      int nminchan = -1 ;
      int nmaxchan = -1 ;
      vector<uInt> freqIdUpdate ; 
      for ( uInt irow = 0 ; irow < rows ; irow++ ) {
	uInt ifno = ifNoVec[itable][irow] ;  // IFNO is reset by group index
	double minfreq = ifgfreq[2*ifno] ;
	double maxfreq = ifgfreq[2*ifno+1] ;
	//cout << "frequency range: [" << minfreq << "," << maxfreq << "]" << endl ;
	vector<double> abcissa = newin[itable]->getAbcissa( irow ) ;
	int nchan = abcissa.size() ;
	double resol = abcissa[1] - abcissa[0] ;
	//cout << "abcissa range  : [" << abcissa[0] << "," << abcissa[nchan-1] << "]" << endl ;
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
	//cout << "channel range (" << irow << "): [" << nminchan << "," << nmaxchan << "]" << endl ;
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
    //cout << "Align spectral resolution" << endl ;
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
	    //cout << "GROUP " << igrp << " (" << tableid << "," << rowid << "): nchan = " << nchan << " (minchan = " << minchan << ")" << endl ; 
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
	cout << "GROUP " << igrp << endl ;
	cout << "   Channel number is adjusted to " << minchan << endl ;
	cout << "   Corresponding frequency resolution is " << maxdnu << "Hz" << endl ;
	for ( uInt imem = 0 ; imem < ifgrp[igrp].size()/2 ; imem++ ) {
	  uInt tableid = ifgrp[igrp][2*imem] ;
	  uInt rowid = ifgrp[igrp][2*imem+1] ;
	  freqIDCol.attach( newin[tableid]->table(), "FREQ_ID" ) ;
	  //cout << "tableid = " << tableid << " rowid = " << rowid << ": " << endl ;
	  //cout << "   regridChannel applied to " ;
	  if ( tableid != reftable ) 
	    refreqid = newin[tableid]->frequencies().addEntry( refpixref, refvalref, refinc ) ;
	  for ( uInt irow = 0 ; irow < newin[tableid]->table().nrow() ; irow++ ) {
	    uInt tfreqid = freqIdVec[tableid][irow] ;
	    if ( tfreqid == rowid ) {	  
	      //cout << irow << " " ;
	      newin[tableid]->regridChannel( minchan, maxdnu, irow ) ;
	      freqIDCol.put( irow, refreqid ) ;
	      freqIdVec[tableid][irow] = refreqid ;
	    }
	  }
	  //cout << endl ;
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
    cout << endl ;

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
    cout << "Combine spectra based on frequency grouping" << endl ;
    cout << "IFNO is renumbered as frequency group ID (see above)" << endl ;
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
    //cout << "taql = " << taql << endl ;
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
    Table &tab = tmpout->table() ;
    TableIterator iter( tab, "POLNO" ) ;
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
      uInt polno = polnos( 0 ) ;
      //cout << "POLNO iteration: " << polno << endl ;
      for ( uInt igrp = 0 ; igrp < freqgrp.size() ; igrp++ ) {
	sizes[igrp].resize( freqgrp[igrp].size() ) ;
	for ( uInt imem = 0 ; imem < freqgrp[igrp].size() ; imem++ ) {
	  for ( uInt irow = 0 ; irow < iter.table().nrow() ; irow++ ) {
	    uInt ifno = ifnoCol( irow ) ;
	    if ( ifno == freqgrp[igrp][imem] ) {
	      Vector<Float> spec = specCols( irow ) ;
	      Vector<uChar> flag = flagCols( irow ) ;
	      vector<Float> svec ;
	      spec.tovector( svec ) ;
	      vector<uChar> fvec ;
	      flag.tovector( fvec ) ;
	      //cout << "spec.size() = " << svec.size() << " fvec.size() = " << fvec.size() << endl ;
	      specout[igrp].insert( specout[igrp].end(), svec.begin(), svec.end() ) ;
	      flagout[igrp].insert( flagout[igrp].end(), fvec.begin(), fvec.end() ) ;
	      //cout << "specout[" << igrp << "].size() = " << specout[igrp].size() << endl ;
	      sizes[igrp][imem] = spec.nelements() ;
	    }
	  }
	}
	for ( uInt irow = 0 ; irow < out->table().nrow() ; irow++ ) {
	  uInt ifout = ifnoColOut( irow ) ;
	  uInt polout = polnoColOut( irow ) ;
	  if ( ifout == gmemid[igrp] && polout == polno ) {
	    // set SPECTRA and FRAGTRA
	    Vector<Float> newspec( specout[igrp] ) ;
	    Vector<uChar> newflag( flagout[igrp] ) ;
	    specColOut.put( irow, newspec ) ;
	    flagColOut.put( irow, newflag ) ;
	    // IFNO renumbering
	    ifnoColOut.put( irow, igrp ) ;
	  }
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
	  //cout << "freqgrp " << igrp << " freqidOut = " << freqidOut << endl ;
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
