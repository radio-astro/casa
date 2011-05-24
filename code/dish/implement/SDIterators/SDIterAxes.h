//# SDAxesIter: Single dish axes information for an SDIterator.
//# Copyright (C) 1998,1999,2000,2001
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
//#
//# $Id$

#ifndef DISH_SDITERAXES_H
#define DISH_SDITERAXES_H


#include <casa/aips.h>

#include <casa/Arrays/Vector.h>
#include <casa/Containers/Block.h>
#include <casa/Quanta/Quantum.h>
#include <casa/Quanta/Unit.h>
#include <tables/Tables/TableColumn.h>
#include <casa/Utilities/Regex.h>

namespace casa { //# NAMESPACE CASA - BEGIN

//# Forward Declarations
class Table;
class String;
class Regex;
class IPosition;
class MEpoch;

// <summary>
// </summary>

// <use visibility=export>

// <reviewed reviewer="" date="yyyy/mm/dd" tests="" demos="">
// </reviewed>

// <prerequisite>
//   <li> SDRecord
//   <li> SDCalc design
// </prerequisite>
//
// <etymology>
// </etymology>
//
// <synopsis>
// </synopsis>
//
// <example>
// </example>
//
// <motivation>
// </motivation>
//
// <thrown>
//    <li>
//    <li>
// </thrown>
//
// <todo asof="yyyy/mm/dd">
//   <li>
// </todo>

class SDIterAxes
{
public:
    // eventually this should be able to construct one from
    // an SDIterator, I think, or possibly have it be a base
    // class with one of these for the different types of
    // input data (MS, SDFITS table, SDFITS file, etc)
    // But for now, just do this for SDFITS table.  
    // Construct one from a table, which should be made
    // via fits2table from an SDFITS file.
    SDIterAxes(const Table& fitsTable);

    ~SDIterAxes();

    // return the shape of the indicated row number
    IPosition shape(Int rownr) const {return dataColumn_p->shape(rownr);}

    // vectors of the various axis information at a given row
    // these are only valid out to how ever many elements there
    // are in shape for that row.  They have a length equal to
    // the maximum number of elements in shape for any rows
    // in the input table.
    // <group>
    const Vector<String> &ctype(Int rownr);
    const Vector<Double> &crval(Int rownr);
    const Vector<Double> &crpix(Int rownr);
    const Vector<Double> &cdelt(Int rownr);
    const Vector<Double> &crota(Int rownr);
    const Vector<String> &cunit(Int rownr);
    // </group>

    // Set the axis information
    // <group>
    Bool setctype(Int rownr, const Vector<String> &ctypes, Bool &added);
    Bool setcrval(Int rownr, const Vector<Double> &crvals, Bool &added);
    Bool setcrpix(Int rownr, const Vector<Double> &crpixs, Bool &added);
    Bool setcdelt(Int rownr, const Vector<Double> &cdelts, Bool &added);
    Bool setcrota(Int rownr, const Vector<Double> &crotas, Bool &added);
    Bool setcunit(Int rownr, const Vector<String> &cunits, Bool &added);
    // </group>

    // identifies which axis corresponds to one of the known
    // SDFITS types, the first three are required, the 
    // rest are not.  If any axis is not present, a value of
    // -1 is returned.  
    // <group>
    Int freqAxis(Int rownr) const {axisCacheInit(rownr); return freqAxis_p;}
    Int longAxis(Int rownr) const {axisCacheInit(rownr); return longAxis_p;}
    Int latAxis(Int rownr) const {axisCacheInit(rownr); return latAxis_p;}
    Int timeAxis(Int rownr) const {axisCacheInit(rownr); return timeAxis_p;}
    Int stokesAxis(Int rownr) const {axisCacheInit(rownr); return stokesAxis_p;}
    Int beamAxis(Int rownr) const {axisCacheInit(rownr); return beamAxis_p;}
    Int rcvrAxis(Int rownr) const {axisCacheInit(rownr); return rcvrAxis_p;}
    // </group>

    // max number of axes in this table, although if a set
    // function gets more axes then there are available, the
    // necessary columns will be added to the table
    uInt maxaxes() const {return crvalTypes_p.nelements();}
    
    // return the true axis data type for the numerical axis 
    // information columns.
    // <group>
    const Vector<Int> &crvalTypes() const {return crvalTypes_p;}
    const Vector<Int> &crpixTypes() const {return crpixTypes_p;}
    const Vector<Int> &crotaTypes() const {return crotaTypes_p;}
    const Vector<Int> &cdeltTypes() const {return cdeltTypes_p;}
    // </group>

    // return the axis column names for each type of column
    // There are maxaxes() elements here.  If any element is
    // blank, there is no column for that axis and type.
    // <group>
    const Vector<String> &crvalColNames() const {return crvalColNames_p;}
    const Vector<String> &crpixColNames() const {return crpixColNames_p;}
    const Vector<String> &cdeltColNames() const {return cdeltColNames_p;}
    const Vector<String> &ctypeColNames() const {return ctypeColNames_p;}
    const Vector<String> &cunitColNames() const {return cunitColNames_p;}
    // </group>

    // The time for the given row number
    MEpoch time(Int rownr);    

    // check to see if this row looks ok.  The only check at
    // this point is to see that all of the required axes are
    // present.  This returns False if any are not present.
    Bool ok(Int rownr) const;

    // reset this class to the indicated table - this may be the
    // same table as before.  This is necessary if the number of
    // axes has changed in an output table.  The constructor also
    // uses this.
    void reset(const Table& fitsTable);

    // The number of SDRecords for a given row.
    // An SDRecord can hold the freq axis plus the stokes axis
    uInt nsdrecords(Int rownr) const;
    // The number of SDRecords in this iterator (i.e. the sums from
    // each row).
    uInt nsdrecords() const;
private:
    Table *tab_p;
    Bool isWritable_;
    Bool outColsTypeSet_;
    PtrBlock<ROTableColumn *> ctypeCols_p, crvalCols_p, crpixCols_p,
	cdeltCols_p, crotaCols_p, cunitCols_p;
    PtrBlock<TableColumn *> rwctypeCols_p, rwcrvalCols_p, rwcrpixCols_p,
	rwcdeltCols_p, rwcrotaCols_p, rwcunitCols_p;

    ROTableColumn *dataColumn_p;

    Vector<String> ctypes_p, cunits_p, crvalColUnits_p;
    Vector<Double> crvals_p, crpixs_p, cdelts_p, crotas_p;
    Vector<Int> crvalTypes_p, crpixTypes_p, cdeltTypes_p, crotaTypes_p;

    Vector<String> ctypeColNames_p, crvalColNames_p, crpixColNames_p, cdeltColNames_p, 
	cunitColNames_p, crotaColNames_p;

    ROTableColumn timeCol_p, dateObsCol_p;
    Unit timeUnit_p;

    // cache the Regex used here
    Regex freqAxisRegex_p, longAxisRegex_p, latAxisRegex_p, timeAxisRegex_p,
	stokesAxisRegex_p, beamAxisRegex_p, rcvrAxisRegex_p;

    // which axes for the cached row
    mutable Int cachedRow_p;
    mutable Int freqAxis_p, longAxis_p, latAxis_p, timeAxis_p, stokesAxis_p,
	beamAxis_p, rcvrAxis_p;

    // very often, DATE-OBS won't change in the file, cache it here
    String dateObs_p;
    Quantity dateObsQuant_p;

    // clear things
    void cleanup();

    void doubleVector(Int rownr, Vector<Double> &result,
		      const PtrBlock<ROTableColumn *> &colsPtr,
		      Double defaultValue);
    void stringVector(Int rownr, Vector<String> &result,
		      const PtrBlock<ROTableColumn *> &colsPtr,
		      String defaultValue);
    void stringVector(Int rownr, Vector<String> &result,
		      const PtrBlock<ROTableColumn *> &colsPtr,
		      Vector<String> &defaultValues);
    Bool setDoubleCols(Int rownr, const Vector<Double> &vals,
		       const String &baseName,
		       Vector<Int> &types,
		       PtrBlock<ROTableColumn *> &rocolsPtr,
		       PtrBlock<TableColumn *> &rwcolsPtr,
		       Vector<String> &colNames,
		       Bool &added,
		       Double defaultValue);
    Bool setStringCols(Int rownr, const Vector<String> &vals,
		       const String &baseName,
		       PtrBlock<ROTableColumn *> &rocolsPtr,
		       PtrBlock<TableColumn *> &rwcolsPtr,
		       Vector<String> &colNames,
		       Bool &added,
		       String defaultValue);
    void setAxisCache(Int rownr) const;
    void axisCacheInit(Int rownr) const {if (rownr != cachedRow_p) setAxisCache(rownr);}
    void resize(Int naxes);
    // unavailable
    SDIterAxes();
    SDIterAxes(const SDIterAxes &other);
    SDIterAxes& operator=(const SDIterAxes &other);
};


} //# NAMESPACE CASA - END

#endif
