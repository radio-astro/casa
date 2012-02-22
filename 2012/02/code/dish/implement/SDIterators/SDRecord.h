//# SDRecord.h: this defines SDRecord, a Record used for Single Dish data.
//# Copyright (C) 1996,1998,1999,2000,2001,2002
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

#ifndef DISH_SDRECORD_H
#define DISH_SDRECORD_H


//#! Includes go here

#include <casa/aips.h>
#include <casa/Arrays/Array.h>
#include <casa/Arrays/IPosition.h>
#include <casa/Containers/Record.h>
#include <casa/Containers/RecordDesc.h>
#include <casa/Containers/RecordField.h>
#include <casa/Utilities/DataType.h>
#include <casa/BasicSL/String.h>

//# Forward Declarations
#include <casa/iosfwd.h>

namespace casa { //# NAMESPACE CASA - BEGIN

// <summary>
// This class enforces the structure of SDRecord expected by SDRecord consumers.
// </summary>

// <use visibility=export>

// <reviewed reviewer="" date="yyyy/mm/dd" tests="" demos="">
// </reviewed>

// <prerequisite>
//   <li> Record class
//   <li> SDIterator
//   <li> SDCalc design document
// </prerequisite>
//
// <etymology>
// An SDRecord has a Record with a structure particularly suited for 
// single dish data.
// </etymology>
//
// <synopsis>
// An SDRecord is a fixed format Record with the following structure:
//    data TpRecord (fixed)
//        desc TpRecord - things which describe the data
//            chan_freq - Quantum<Vector<Double> > - chan_freq cell values
//            refframe - String - frequency reference frame
//            reffrequency - Double - frequence at value of reference pixel, same units as chan_freq
//            chan_width - Double - channel width at reffrequency, for use in selection, same units as chan_freq
//            restfrequency - Douuble - Hz
//            corr_type - Vector<String> - Stokes values converted to string representation
//            units - String - units of the arr array (the data units)
//        arr (TpArrayFloat || TpArrayComplex)(nchan, nstokes)
//        flag TpArrayBool (nchan, nstokes)
//             when flag(i,j) == True arr(i,j) is bad and should
//             not be used
//        weight TpArrayFloat (nchan, nstokes)
//        sigma TpArrayFloat (nchan, nstokes)
//    header TpRecord (fixed)
//        time - MEpoch as a MeasureHolder
//        scan_number - TpInt
//        source_name - TpString 
//        direction - MDirection as a MeasureHolder
//        refdirection - MDirection as a MeasureHolder
//        veldef - TpString - velocity definition (RADIO, OPTICAL, ...)
//        transition - TpString
//        exposure - Double - seconds
//        duration - DOuble - seconds
//        observer - TpString
//        project - TpString
//        resolution - Double - channel resulution at reffrequency - Hz
//        bandwidth - Double - total bandwidth - Hz
//        tcal - TpArrayFloat(nstokes)  - K
//        trx - TpArrayFloat(nstokes) - K
//        tsys - TpArrayFloat(nstokes) - K
//        telescope - TpString
//        telescope_position - MPosition as MeasureHolder
//        pressure - hPa
//        dewpoint - K
//        tambient - K
//        wind_dir - rad
//        wind_speed - m/s
//        azel - MDirection as MeasureHolder
//    hist TpArrayString - the history of this SDRecord
//    other - TpRecord - the contents of this depend on the data source
//      if the source is a MeaurementSet, this record contains sub-records having 
//           the same names as the subtables found in the MeasurementSet. The contents of 
//           these records will be the appropriate single row from that subtable.
//           Each record has an interp_method attribut which is a String that indicates
//           how that row was filled from the Measurements.  "direct" means that the
//           row was directly indexed and no interpolation was necessary.  "nearest" means
//           that the nearest row in time and falling within the appropriate interval, 
//           as appropriate, was used.  Other interpolation schemes are planned.
//           In addition to the sub-table records, there will also be a "keys" record to
//           hold the keys from the Main table.  Non-standard subtables will be included
//           here IF they have a TIME column and optionally an INTERVAL column or if they
//           have an NS_KEYS keyword which indicates integer key columns.
//       if the source is a table from an SDFITS binary table, any column/keyword not used
//           should be added to an sdfits subrecord in the other record.
//       if the source is an Image, any information not used in data and header should be
//           added to an image subrecord in the other record
// 
// Anything with the above structure is a valid SDRecord
// It is intended to hold data originally from a
// MeasurementSet, a table constructed from an SDFITS table, or from
// an image cube and still be independent from these original data sources.
// As such, the data and header and intended to contain the most comonly
// used standard information about single dish data.  Some data sources,
// most notably the MeasurementSet, will also typically duplicate some of
// the information found in the data and header records in the
// other records.
//
// This class also provides convenient access to the
// required fields.
// </synopsis>
//
// <example>
// </example>
//
// <motivation>
// DISH works on individual rows of a MS and this structure helps that.
// </motivation>
//
// <thrown>
//    <li>
//    <li>
// </thrown>
//
// <todo asof="yyyy/mm/dd">
//   <li> Should there be some indication where an SDRecord came from (appart from anything in the history)
//   <li> Is the overhead in flag, weight, and sigma too much for most cases?
//   <li> Should a CoordinateSystem be used instead of the contents of desc?
// </todo>

class SDRecord : public Record
{
public:

    // This is mostly useful during debugging
    friend ostream& operator<< (ostream& os, const SDRecord& rec);

    // The default SDRecord has the basic structure but indicates
    // an empty data array, a default CoordinateSystem, empty
    // header values and indexes.
    // Primarily useful for assignments except within SDIterator
    // which starts from this and constructs the full record.
    // The type of an SDRecord can be either TpFloat or TpComplex.
    // This determines the data type of the arr field and it must
    // be set when the SDRecord is first constructed.
    SDRecord(DataType dataType = TpFloat);
    // SDRecords are generally constructed within an SDIterator
    // Sets this to an empty SDRecord and raises the error flag
    // if other is not valid.  The data type of this SDRecord will be
    // the same as that of other
    SDRecord(const SDRecord& other);
    SDRecord(const Record& other);
    
    ~SDRecord();

    SDRecord& operator=(const SDRecord& other);

    // the data sub-record
    const Record &data() const {return *itsData;}
    Record &data() {return *itsData;}

    // the desc sub-record
    const Record &desc() const {return *itsDesc;}
    Record &desc() {return *itsDesc;}

    // the header record
    const Record &header() const {return *itsHeader;}
    Record &header() {return *itsHeader;}

    // the history vector
    const Array<String> &hist() const {return *itsHist;}
    Array<String> &hist() {return *itsHist;}

    // the other record
    const Record &other() const {return *itsOther;}
    Record &other() {return *itsOther;}

    // resize all standard array fields to be consistent with the new data shape
    Bool resize(const IPosition &newShape);

    // return the current data shape
    const IPosition& shape() const { return itsShape;}

    // verify that this SDRecord is still valid
    Bool isValid() const { return isValid(*this);}

    // verify that this SDRecord is consistent, a message describing the first failed inconsistency
    // is return in errmsg when the return value is False.
    Bool isConsistent(String &errmsg) const;

    // what type of a data array is this
    DataType arrType() const { return data().dataType("arr");}

    // These methods provide quick access to some fields.
    // <group>
    const RORecordFieldPtr<Array<Float> > &rotsys() const {return itsROTsys;}
    const RORecordFieldPtr<Double> &roduration() const {return itsROTdur;}
    const RORecordFieldPtr<Double> &roexposure() const {return itsROTexp;}
    const RORecordFieldPtr<Double> &rorestfrequency() const {return itsROFrest;}
    const RORecordFieldPtr<String> &roveldef() const {return itsROVeldef;}
    const RORecordFieldPtr<String> &rorefframe() const {return itsRORefFrame;}
    const RORecordFieldPtr<Record> &rochan_freq() const {return itsROChanFreq;}
    const RORecordFieldPtr<Double> &roreffrequency() const {return itsROFref;}
    const RORecordFieldPtr<Array<Bool> > &roflag() const {return itsROFlag;}
    const RORecordFieldPtr<Array<Float> > &roweight() const {return itsROWeight;}
    const RORecordFieldPtr<Array<Float> > &rosigma() const {return itsROSigma;}
    RecordFieldPtr<Array<Float> > &tsys() {return itsTsys;}
    RecordFieldPtr<Double> &duration() {return itsTdur;}
    RecordFieldPtr<Double> &exposure() {return itsTexp;}
    RecordFieldPtr<Double> &restfrequency() {return itsFrest;}
    RecordFieldPtr<String> &veldef() {return itsVeldef;}
    RecordFieldPtr<String> &refframe() {return itsRefFrame;}
    RecordFieldPtr<Record> &chan_freq() {return itsChanFreq;}
    RecordFieldPtr<Double> &reffrequency() {return itsFref;}
    RecordFieldPtr<Array<Bool> > &flag() {return itsFlag;}
    RecordFieldPtr<Array<Float> > &weight() {return itsWeight;}
    RecordFieldPtr<Array<Float> > &sigma() {return itsSigma;}
    // </group>

    // Quick access to the data array.
    // One of farr and carr will be empty, depending on the type of record.
    // <group>
    // arr field when this contains Floats
    const RORecordFieldPtr<Array<Float>  > &rofarr() const {return itsROFarr;}
    RecordFieldPtr<Array<Float>  > &farr() {return itsFarr;}
    // arr field when this contains Complex data
    const RORecordFieldPtr<Array<Complex> > &rocarr() const {return itsROCarr;}
    RecordFieldPtr<Array<Complex> > &carr() {return itsCarr;}
    // </group>

    // verify that a record is a valid SDRecord
    static Bool isValid(const Record& rec) 
    { Bool eqtypes; return (requiredDesc(TpFloat).isSubset(rec.description(), eqtypes) ||
			    requiredDesc(TpComplex).isSubset(rec.description(), eqtypes));}
    
    // return the minimum required description
    static RecordDesc requiredDesc(DataType dataType);

    // used by ostream << operator
    ostream & show(ostream &os) const;

    // this actually does the work, hierachically on each subrecord
    static ostream & showRecord(ostream &os, const Record &rec);

private:
    static RecordDesc itsBasicDesc;
    static RecordDesc itsBasicComplexDesc;

    RecordFieldPtr<Record> itsData;
    RecordFieldPtr<Record> itsDesc;
    RecordFieldPtr<Record> itsHeader;
    RecordFieldPtr<Record> itsOther;

    RecordFieldPtr<Array<Bool> > itsFlag;
    RecordFieldPtr<Array<Float> > itsTsys, itsWeight, itsSigma, itsFarr, itsTcal, itsTrx;
    RecordFieldPtr<Array<Complex> > itsCarr;
    RecordFieldPtr<Double> itsTdur, itsTexp, itsFrest, itsFref;
    RecordFieldPtr<String> itsVeldef, itsRefFrame;
    RecordFieldPtr<Record> itsChanFreq;

    RORecordFieldPtr<Array<Bool> > itsROFlag;
    RORecordFieldPtr<Array<Float> > itsROTsys, itsROWeight, itsROSigma, itsROFarr;
    RORecordFieldPtr<Array<Complex> > itsROCarr;
    RORecordFieldPtr<Double> itsROTdur, itsROTexp, itsROFrest, itsROFref;
    RORecordFieldPtr<String> itsROVeldef, itsRORefFrame;
    RORecordFieldPtr<Record> itsROChanFreq;

    RecordFieldPtr<Array<String> > itsHist, itsCorrType;

    // cache the size
    IPosition itsShape;

    // function to initialize itsBasicDesc and itsBasicComplexDesc
    static void initStatics();

    // initialize the pointers
    void initPointers();
    // set the default values
    void defaultValues();
    // set the default values for variable shaped matrices
    void defaultMatValues();
    // set the default values for variable shaped vectors
    void defaultVecValues();
    // get shape from the arr field
    void cacheShape();

    // function to set all fields to their default values by type
    void clear();
};

inline ostream& SDRecord::show(ostream &os) const
{
    return SDRecord::showRecord(os, *this);
}

inline ostream& operator<< (ostream &os, const SDRecord &rec)
{
    return rec.show(os);
}


} //# NAMESPACE CASA - END

#endif


