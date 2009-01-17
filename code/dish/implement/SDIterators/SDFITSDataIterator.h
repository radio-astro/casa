//# SDFITSDataIterator: iterates through an SDFITS DATA field
//# Copyright (C) 2000,2002
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

#ifndef DISH_SDFITSDATAITERATOR_H
#define DISH_SDFITSDATAITERATOR_H

#include <casa/Arrays/IPosition.h>
#include <casa/Arrays/Matrix.h>
#include <coordinates/Coordinates/DirectionCoordinate.h>
#include <casa/Arrays/Vector.h>
#include <casa/Containers/RecordField.h>
#include <lattices/Lattices/ArrayLattice.h>
#include <lattices/Lattices/LatticeIterator.h>
#include <measures/Measures/MDirection.h>
#include <measures/Measures/MDoppler.h>
#include <measures/Measures/MEpoch.h>
#include <measures/Measures/MFrequency.h>
#include <casa/Quanta/MVTime.h>
#include <casa/Utilities/DataType.h>
#include <casa/Utilities/Regex.h>
#include <casa/BasicSL/String.h>

namespace casa { //# NAMESPACE CASA - BEGIN

//# Forward Declarations
class Record;

// <summary>
// Steps through an SDFITS DATA cell, serving up chunks appropriate for an MS.
// </summary>

// <use visibility=local>

// <reviewed reviewer="" date="yyyy/mm/dd" tests="" demos="">
// </reviewed>

// <prerequisite>
//   <li> SDFITS
//   <li> MS
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
//   <li> full use of field units for all handled keywords
//   <li> use appropriate defaults when not exactly standard SDFITS
//   <li> better error handling, warnings and errors
//   <li> issue warnings just once for each setup
//   <li> better behavior of class members when things go bad
//   <li> handled data-like fields as well as DATA (TMATXnnn=T fields)
// </todo>

class SDFITSDataIterator
{
public:
    SDFITSDataIterator();

    ~SDFITSDataIterator();

    // initialize everything for this row, the cursor is reset at the start
    Bool setrow(const Record &row, String &errMsg);

    // the DATA pointed to by the current cursor, in the order
    // expected for use by the MS - nstokes, nfreq
    const Matrix<Float>& floatData() {return floatData_p;}

    // the vector of frequencies associated with floatData, in Hz
    const Vector<Double>& frequencies() {return freqs_p;}

    // from original axis description, the frequency axis value at
    // the reference channel
    Double freqAxisAtRefPix() { return refFreq_p;}

    // from original axis description, cdelt
    Double freqAxisDelt() {return fdelt_p;}

    // the frequency reference type
    MFrequency::Types freqRefType() {return freqRefType_p;}

    // the velocity definition
    MDoppler::Types dopplerType() {return dopplerType_p;}

    // the stokes values 
    const Vector<Int>& stokes() {return stokes_p;}

    // the direction at the current cursor location
    const MDirection& direction() {return direction_p;}

    // the time at the current cursor location,  this will also
    // take into account other time-related keywords/columns in
    // the fits table
    const MEpoch& time() {return time_p;}

    // The exposure
    const MVTime& exposure() {return exposure_p;}

    // The time range, in the same Measure reference frame as time()
    const Vector<Double> &timeRange() {return timeRange_p;}

    // the beam ID at the current cursor location.  This defaults
    // to a value of 1 if no BEAM axis is present.
    Int beamId() {return beamId_p;}

    // is there a BEAM axis
    Bool hasBeamAxis() {return beamAxis_p>=0;}

    // the receiver ID at the current cursor location.  This defaults
    // to a value of 1 if no RECEIVER axis is present.
    Int receiverId() {return receiverId_p;}
    
    // Is there a RECEIVER axis
    Bool hasReceiverAxis() {return receiverAxis_p>=0;}

    // Move the cursor to the next location
    void next();

    // Move the cursor back to the origin
    void origin() {iter_p->reset(); setValues();}

    // At the end of the iterator
    Bool atEnd() {return iter_p->atEnd();}

    // which columns are handled here
    Vector<Bool> &handledColumns() {return handledCols_p;}
private:
    // Regex's to identify axis types
    Regex freqRegex_p, velRegex_p, longRegex_p, latRegex_p, timeRegex_p, stokesRegex_p,
	beamRegex_p, receiverRegex_p;
    // Data pointers, at least one of these is always set, preferably
    // the Float one.  If the Double one is used, a warning is issued
    // once about the loss of precision.
    RORecordFieldPtr<Array<Float> > fdataPtr_p;
    RORecordFieldPtr<Array<Int> > idataPtr_p;
    RORecordFieldPtr<Array<Short> > sdataPtr_p;
    RORecordFieldPtr<Array<Double> > ddataPtr_p;

    // axis descriptors
    Vector<Double> crval_p, crpix_p, cdelt_p, crota_p;
    Vector<String> ctype_p, cunit_p;

    // axis pointers, these may change with each row
    Int freqAxis_p, stokesAxis_p, longAxis_p, latAxis_p, timeAxis_p,
	beamAxis_p, receiverAxis_p;

    // field IDs, these do not change with each row
    // Ids for things which are inherently vectors
    Vector<Int> ctypeIds_p, crvalIds_p, crpixIds_p, cdeltIds_p,
	crotaIds_p, cunitIds_p;

    // Ids for floating point values - we don't know exactly what
    // type might have been used in the original SDFITS
    Int obsfreqId_p, lstId_p, restfreqId_p, equinoxId_p, timeId_p, exposureId_p;
    DataType equinoxType_p;

    // field PTRs for string fields, these may not all be attached
    RORecordFieldPtr<String> dateObsField_p, timesysField_p, veldefField_p;

    // The cursor shape
    IPosition cursorShape_p;

    // The lattice iterator
    RO_LatticeIterator<Float> *iter_p;

    // The array lattice
    ArrayLattice<Float> *arrLattice_p;

    // The date obs value and possible TIME field value
    MVTime dateObs_p, timeFieldVal_p;
    // And the TIMESYS frame
    MEpoch::Types timesys_p;

    // the direction coordinate for this row
    DirectionCoordinate dirCoord_p;

    Bool needsToBeTransposed_p;

    Matrix<Float> floatData_p;
    Vector<Double> freqs_p;
    MFrequency::Types freqRefType_p;
    MDoppler::Types dopplerType_p;
    Vector<Int> stokes_p;
    MDirection direction_p;
    MEpoch time_p;
    Int beamId_p;
    Int receiverId_p;

    Double refFreq_p, fdelt_p;

    // handled columns
    Vector<Bool> handledCols_p;

    // The time range
    Vector<Double> timeRange_p;

    // the exposure
    MVTime exposure_p;

    // initialize the internals which depend on the row description
    Bool init(const Record &row, String &errMsg);

    // set the values after the cursor has moved or been reset
    void setValues();

    // utility function to get the fieldNumber for an indexed keyword, eg. CTYPE2
    Int indexedFieldNumber(const Record &row, const String &field, Int index);

    // undefined and inaccessible
    SDFITSDataIterator(const SDFITSDataIterator &);
    SDFITSDataIterator &operator=(const SDFITSDataIterator &);
};



} //# NAMESPACE CASA - END

#endif


