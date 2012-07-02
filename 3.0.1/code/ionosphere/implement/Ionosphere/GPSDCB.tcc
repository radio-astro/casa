//# GPSDCB.cc: 
//# Copyright (C) 2001,2002
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

#include <casa/stdio.h>
#include <casa/string.h>
#include <errno.h>
#include <ionosphere/Ionosphere/GPSDCB.h>
#include <casa/Arrays/LogiVector.h>
#include <casa/Arrays/ArrayMath.h>
#include <casa/Arrays/ArrayLogical.h>
#include <casa/Arrays/MaskedArray.h>
#include <casa/Arrays/MaskArrMath.h>
#include <casa/Exceptions/Error.h>
#include <casa/System/Aipsrc.h>
#include <tables/Tables.h>
#include <scimath/Mathematics/InterpolateArray1D.h>    
#include <casa/iostream.h>
    
namespace casa { //# NAMESPACE CASA - BEGIN

const String pathvar("gps.tgd.path");
const String CMJD("MJD"),
            CP1P2("P1P2"),
            CP1P2RMS("P1P2RMS"),
            CP1C1("P1C1"),
            CP1C1RMS("P1C1RMS"),
            CSTA_P1P2("STA_P1P2"),
            CSTA_P1P2RMS("STA_P1P2RMS"),
            KIGS_ID("IGS_ID"),
            KMISSING("MISSING_VALUE");
    

static Bool loadColumn( const Table &tab,const String &name,Cube<Float> &cube,uInt zplane )
{
  ROArrayColumn<Float> col(tab,name);
  Array<Float> data( cube.xyPlane(zplane) );
  col.getColumn(data);
  return True;
}

template<class T> void toBlock ( Block<T> &blk,const Array<T> &arr )
{
  Bool del;
  const T *parr = arr.getStorage(del);
  uInt n = arr.nelements();
  T *pblk = new T[n];
  memcpy(pblk,parr,sizeof(T)*n);
  arr.freeStorage(parr,del);
  blk.replaceStorage(n,pblk,True);
}

template void toBlock ( Block<Float> &blk,const Array<Float> &arr );
    
GPSDCB::GPSDCB( const char *tablename ) :
  os(LogOrigin("GPSDCB"))
{
  try {
// setup table name from argument or aipsrc
  String tname;
  if( tablename && tablename[0] )
    tname = tablename;
  else
  {
    if( !Aipsrc::find(tname,pathvar) )
      tname = "/aips++/data/gps/DCB";
  }
// check if the table exisits, and open it
  if( !Table::isReadable(tname) )
  {
    os<<"Table not found: "<<tname<<endl<<
        "Is "<<pathvar<<" set correctly?"<<LogIO::EXCEPTION;
    return;
  }
  Table tab(tname,Table::Old);
// setup internal data storage to read the table
  nrec = tab.nrow();
  if( !nrec )
  {
    os<<"Table "<<tname<<" is empty."<<LogIO::EXCEPTION;
    return;
  }
  const ColumnDesc coldesc( tab.tableDesc().columnDesc(CP1P2) );
  if( !coldesc.isFixedShape() )
    os<<"Table "<<tname<<": "<<CP1P2<<" column must have fixed shape."<<LogIO::EXCEPTION;
  nsvn = coldesc.shape()(0);
  
  missing = tab.keywordSet().asFloat(KMISSING);
  
  mjds.resize(nrec);
  fmjd.resize(nrec);
  dcb.resize(nsvn,nrec,2);
  dcbrms.resize(nsvn,nrec,2);

// load MJDs  
  ROScalarColumn<Int> mjdcol(tab,CMJD);
  mjdcol.getColumn(mjds,True);
  convertArray(fmjd,mjds);
  
// load sattelite data from table
  loadColumn(tab,CP1P2,dcb,P1_P2);
  loadColumn(tab,CP1P2RMS,dcbrms,P1_P2);
  loadColumn(tab,CP1C1,dcb,P1_C1);
  loadColumn(tab,CP1C1RMS,dcbrms,P1_C1);
// load station IDs from table
  ROArrayColumn<Float> stcol(tab,CSTA_P1P2),strmscol(tab,CSTA_P1P2RMS);
  stcol.keywordSet().get(KIGS_ID,stids);
  nsta = stids.nelements();
  if( nsta )
  {
    for( uInt i=0; i<nsta; i++ )
    {
      if( stnums.isDefined(stids(i)) )
        os<<"Table "<<tname<<": duplicate IGS station IDs"<<LogIO::EXCEPTION;
      stnums.define(stids(i),(Int)i);
    }
  }
// now load station data
  stadcb.resize(nsta,nrec);
  starms.resize(nsta,nrec);
  stadcb.set(missing);
  starms.set(missing);
  for( uInt i=0; i<nrec; i++ )
  {
    Vector<Float> d( stcol(i) ),drms( strmscol(i) );
    uInt nd = d.nelements();
    if( nd )
    {
      if( drms.nelements() != nd )
        os<<"Table "<<tname<<": mismatched shapes of "<<CSTA_P1P2<<"/"<<
            CSTA_P1P2RMS<<" at row "<<i<<LogIO::EXCEPTION;
      if( nd > nsta )
        os<<"Table "<<tname<<": too many elements in column "<<CSTA_P1P2<<
            " row "<<i<<LogIO::EXCEPTION;
      Vector<Float> s1( stadcb.column(i)(Slice(0,nd)) ); s1=d;
      Vector<Float> s2( starms.column(i)(Slice(0,nd)) ); s2=drms;
    }
  }
// post successful messages
  os<<"Table "<<tname<<" contains DCBs for "<<nrec<<" dates, "<<
        nsvn<<" SVNs, "<<nsta<<" stations."<<LogIO::POST;
  
// catch exceptions here
  } catch( AipsError err ) {
    os<<"Failed to read in GPS differential code biases table."<<LogIO::EXCEPTION;
  }
}

// -----------------------------------------------------------------------
// getDcb
// Returns the DCB values for a number of MJDs
// Stores the RMS into rms, and also a confidence indicator
// as follows:
// 0  if point matches an MJD for which we have a DCB value
//    (i.e. no interpolation)
// >0 interpolating: distance to nearest neighbour in terms of spacing/2 (<=1 is good)
// <0 extrapolating: distance to nearest neighbour in terms of spacing/2
//    (<-1 is BAD)
// -----------------------------------------------------------------------
Vector<Float> GPSDCB::getDcb ( Vector<Float> &rms,DCBType type,
                               const Vector<Float> &mjd,uInt svn,Bool warn )
{
  uInt isat = svn-1;
// get arrays of relevant dcb values
  const Vector<Float> thisdcb(dcb.xyPlane(type).row(isat));
  const Vector<Float> thisrms(dcbrms.xyPlane(type).row(isat));
// get masked array of valid MJDs for this SVN
  const LogicalVector mask(thisdcb!=missing);
  const MaskedArray<Float> valmjd(fmjd,mask);
  uInt nval = valmjd.nelementsValid();
// no data for SVN -- return empty vector
  if( !nval ) 
    return Vector<Float>();
// resize output arrays
  uInt np = mjd.nelements();
  Vector<Float> result(np);
  rms.resize(np); 
// degenerate case: only one data point for SVN
  const Float MAXDMJD = DCB_SPACING*.55;
  if( nval==1 )
  {
    const IPosition pos0(1,0);
    result = thisdcb(mask).getCompressedArray()(pos0);
    rms    = thisrms(mask).getCompressedArray()(pos0);
    // skip warnings if all MJDs are reasonably close
    if( allNear(mjd,valmjd.getCompressedArray()(pos0),MAXDMJD) )
    {
      String id( type==P1_P2?"P1-P2":"P1-C1" );
      cerr<<"GPSDCB warning: only one "<<id<<" sample for SVN "<<svn<<endl;
      if( warn )
        os<<LogIO::WARN<<id<<" data for SVN "<<svn<<" available at one date only.\n"
            "Use of this estimate may lead to inaccurate GPS TECs.\n"<<LogIO::NORMAL;
    }
    return result;
  }
// interpolate DCB data with cubic spline
  Block<Float> xin,xout;
  toBlock(xin,valmjd.getCompressedArray());
  toBlock(xout,mjd);
  InterpolateArray1D<Float,Float>::interpolate(
      result,xout,xin,Vector<Float>(thisdcb(mask).getCompressedArray()),
      InterpolateArray1D<Float,Float>::spline);
// interpolate RMS data using nearest-neighbour
  InterpolateArray1D<Float,Float>::interpolate(
      rms,xout,xin,thisrms(mask).getCompressedArray(),
      InterpolateArray1D<Float,Float>::nearestNeighbour);
// see if any warnings should be thrown for extrapolations
  Float vmin,vmax;
  minMax(vmin,vmax,valmjd);
  if( anyLT(mjd,vmin-MAXDMJD) || anyGT(mjd,vmax+MAXDMJD) )
  { 
    String id( type==P1_P2?"P1-P2":"P1-C1" );
    cerr<<"GPSDCB warning: extrapolating "<<id<<" for SVN "<<svn<<endl;
    if( warn )
      os<<LogIO::WARN<<"Extrapolating "<<id<<" for SVN "<<svn<<" because some dates\n"
          "are outside the range of available data.\n"<<LogIO::NORMAL;
  }
// check for interpolating inside gaps in the data
  Vector<Float> inner_mjd( mjd(mjd>vmin && mjd<vmax).getCompressedArray() );
  for( uInt i=0; i<inner_mjd.nelements(); i++ )
  {
    if( min(abs(inner_mjd(i)-valmjd)) > MAXDMJD )
    {
      String id( type==P1_P2?"P1-P2":"P1-C1" );
      cerr<<"GPSDCB warning: interpolating within gaps in "<<id<<" for SVN "<<svn<<endl;
      if( warn )
        os<<LogIO::WARN<<"Interpolating within gaps in "<<id<<" data for SVN "<<svn<<endl<<LogIO::NORMAL;
      break;
    }
  }
  return result;
}



} //# NAMESPACE CASA - END

