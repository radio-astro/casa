
//# RFASelector.cc: this defines RFASelector
//# Copyright (C) 2000,2001,2002
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
#include <casa/Exceptions/Error.h>
#include <casa/Arrays/ArrayMath.h>
#include <casa/Arrays/ArrayLogical.h>
#include <casa/Arrays/MaskedArray.h>
#include <casa/Arrays/MaskArrMath.h>
#include <casa/Quanta/Quantum.h>
#include <casa/Quanta/MVTime.h>
#include <casa/Logging/LogIO.h>
#include <synthesis/MSVis/VisibilityIterator.h>
#include <synthesis/MSVis/VisBuffer.h>
#include <flagging/Flagging/RFASelector.h>
#include <casa/stdio.h>

#include <iomanip>

namespace casa { //# NAMESPACE CASA - BEGIN
        
// -----------------------------------------------------------------------
// reformRange
// Reforms an array of 2N elements into a [2,N] matrix
// -----------------------------------------------------------------------
template<class T> Bool RFASelector::reformRange( Matrix<T> &rng,const Array<T> &arr )
{
  if( arr.ndim()>2 || (arr.nelements()%2) !=0 )
    return False;
  rng = arr.reform(IPosition(2,2,arr.nelements()/2));
  return True;
}

template<class T> Array<T> fieldToArray( const RecordInterface &parm,const String &id );

/*
template<> Array<Int> fieldToArray<Int>( const RecordInterface &parm,const String &id )
{ return parm.toArrayInt(id); }
template<> Array<Double> fieldToArray<Double>( const RecordInterface &parm,const String &id )
{ return parm.toArrayDouble(id); }
template<> Array<String> fieldToArray<String>( const RecordInterface &parm,const String &id )
{ return parm.toArrayString(id); }
*/

// -----------------------------------------------------------------------
// RFASelector::parseRange
// Returns a record field of 2N elements as a [2,N] matrix
// -----------------------------------------------------------------------
template<class T> Bool RFASelector::parseRange( Matrix<T> &rng,const RecordInterface &parm,const String &id )
{
  if( isFieldSet(parm,id) )
  {
    try 
    {
      Array<T> arr( fieldToArray<T>(parm,id) );
      if( !reformRange(rng,arr) )
        throw( AipsError("") );
      return True;
    } 
    catch( AipsError x ) 
    {
      os<<"Illegal \""<<id<<"\" array\n"<<LogIO::EXCEPTION;
    }
  }
  return False;
}

/*
template Bool RFASelector::reformRange<Int>( Matrix<Int>&,const Array<Int>& );
template Bool RFASelector::reformRange<Double>( Matrix<Double>&,const Array<Double>& );
template Bool RFASelector::reformRange<String>( Matrix<String>&,const Array<String>& );
template Bool RFASelector::parseRange<Int>( Matrix<Int>&,const RecordInterface&,const String&);
template Bool RFASelector::parseRange<Double>( Matrix<Double>&,const RecordInterface&,const String&);
template Bool RFASelector::parseRange<String>( Matrix<String>&,const RecordInterface&,const String&);
*/

// -----------------------------------------------------------------------
// RFA_selector::find
// Helper templated method to find an object in an array
// -----------------------------------------------------------------------
template<class T> Bool RFASelector::find( uInt &index,const T &obj,const Vector<T> &arr )
{
  for( uInt i=0; i<arr.nelements(); i++ )
    if( obj == arr(i) )
    {
      index = i;
      return True;
    }
  return False;
}
/*
template Bool RFASelector::find<uInt>(uInt&,const uInt&,const Vector<uInt>&);
template Bool RFASelector::find<Int>(uInt&,const Int&,const Vector<Int>&);
template Bool RFASelector::find<String>(uInt&,const String&,const Vector<String>&);
*/
} //# NAMESPACE CASA - END

