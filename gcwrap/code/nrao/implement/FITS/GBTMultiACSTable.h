//# GBTMultiACSTable.h: GBT multi-bank backend table for the ACS.
//# Copyright (C) 2003
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

#ifndef NRAO_GBTMULTIACSTABLE_H
#define NRAO_GBTMULTIACSTABLE_H

#include <nrao/FITS/GBTBackendTable.h>
#include <nrao/FITS/GBTScanLogReader.h>
#include <nrao/FITS/GBTACSTable.h>

#include <casa/Arrays/Vector.h>
#include <casa/Containers/Block.h>
#include <casa/Containers/SimOrdMap.h>
#include <tables/Tables/Table.h>
#include <tables/Tables/TableRow.h>
#include <casa/BasicSL/String.h>

#include <casa/namespace.h>
//# forward includes
namespace casa { //# NAMESPACE CASA - BEGIN
class Record;
class RecordDesc;
class TableRecord;
template <class T> class Array;
} //# NAMESPACE CASA - END

class GBTStateTable;


// <summary>
// A GBTMultiBackendTable for the ACS
// </summary>
//
// <use visibility=export>
//
// <reviewed reviewer="" date="yyyy/mm/dd" tests="" demos="">
// </reviewed>
//
// <prerequisite>
// </prerequisite>
//
// <etymology>
// </etymology>
//
// <synopsis>
// GBTMultiACStTable is a GBTACSTable holding
// data from multiple banks.  It makes it look as if it all
// came from the same data source.  In order for this
// to work.  The ACT_STATE and STATE tables must be the
// same in all banks and the integration time in each
// bank must be the same.
// </synopsis>
//
// <example>
// </example>
//
// <motivation>
// </motivation>

class GBTMultiACSTable : public GBTBackendTable
{
public:
     // Construct it from a vector of file names
    GBTMultiACSTable(const Vector<String> &fileNames,
		     GBTACSTable::VanVleckCorr vanVleckCorr=GBTACSTable::DefaultVanVleck,
		     GBTACSTable::Smoothing smoothing = GBTACSTable::DefaultSmoothing, 
		     Int vvsize=65, Bool useDCBias = False, Double dcbias=0.0,
		     Int minbiasfactor=-1, Bool fixlags = False, String fixlagslog="",
		     Double sigmaFactor=6.0, Int spikeStart=200);

    ~GBTMultiACSTable();

    // Attach this GBTMultiACSTable to a new set of file names.
    // If fileNames are the same as the
    // currently opened filse, if there are any, then this just
    // closes and reopens the FITS files, setting things right
    // back to the next row in each table, if there are any.
    virtual Bool reopen(const Vector<String> &fileNames, Bool resync=True);

    // reopen if there's just one file
    virtual Bool reopen(const String &fileName, Bool resync) 
    {return reopen(Vector<String>(1,fileName), resync);}

    // This is necessary to avoid hiding the FITSTabular version
    virtual Bool reopen(const String &fileName) {return reopen(fileName,True);}

    // Return the type of this backend
    GBTScanLogReader::BACKENDS type() const {return GBTScanLogReader::ACS;}

    // Returns the name of the file serving up the default bank
    virtual const String &name() const {return defBank().name();}

    virtual const Vector<String> &names() const {return itsFiles;}

    // Is everything okay.
    virtual Bool isValid() const {return itsValid;}

    // The number of elements along the STATE axis
    virtual uInt nstate() const {return defBank().nstate();}

    // The number of elements along the frequency axis (default to 1
    // for non-spectral line data) for a specific sampler.
    virtual uInt nchan(uInt whichSampler) const;

    // The bandwidth of the indicated sampler.  If this is < 0 then that
    // signals that it is not known here and it should be fetched from 
    // the IF BANDWDTH column.
    virtual Double bw(uInt whichSampler) const;

    // The center IF for the indicated sampler.  If this is < 0 then that
    // signals that it is not known here and it should be fetched from
    // the IF CENTER_IF column.
    virtual Double centerIF(uInt whichSampler) const;

    // Indicates whether frequency increases with channel number for the
    // given sampler.
    virtual Bool increases(uInt whichSampler) const;

    // The keywords from the FITS data tables merged into a single record.
    virtual const TableRecord &keywords() const {return defBank().keywords();}

    // advance to the next row in all tables
    virtual void next();

    // return the current row in the default underlying table
    // It is here because of inheritance but it should not be used.
    virtual const Record &currentRow() const {return defBank().currentRow();}

    // The unhandled keywords
    virtual const Record &unhandledKeywords() {return defBank().unhandledKeywords();}

    // Mark a keyword as handled;
    virtual void handleKeyword(const String &kwname) {defBank().handleKeyword(kwname);}

    virtual const RecordDesc &description() const {return defBank().description();}
    virtual const Record &units() const {return defBank().units();}
    virtual const Record &displayFormats() const {return defBank().displayFormats();}
    virtual const Record &nulls() const {return defBank().nulls();}

    // Return the unhandled fields in the current row of the current table
    virtual const Record &unhandledFields() {return defBank().unhandledFields();}

    // Mark a field in the current row of all tables as being handled.
    virtual void handleField(const String &fieldname) {defBank().handleField(fieldname);}

    // The ancilliary tables
    // <group>
    virtual const Table &sampler() const {return itsSampler;}
    virtual const GBTStateTable &state() const {return defBank().state();}
    virtual const Table &actState() const {return defBank().actState();}
    virtual const Table &port() const {return itsPort;}
    // </group>

    // Total number of rows in each bank (should be the same)
    virtual uInt nrow() const {return defBank().nrow();}

    // This specific row (relative to total from all banks)
    virtual uInt rownr() const {return defBank().rownr();}

    // FITSVER - should be the same for all banks
    virtual const String &fitsVers() const {return defBank().fitsVers();}

    // Leading integer (before decimal) of FITSVER
    virtual Int baseVersion() const {return defBank().baseVersion();}

    // Trailing integer(after decimal) of FITSVER
    virtual Int deviceVersion() const {return defBank().deviceVersion();}

    // get the raw data at the current row.
    virtual const Array<Float> rawdata(uInt whichSampler);

    // get the data for the current row, applying the indicated vanVleck 
    // correction and smoothing for the Schwab correction, use the indicated 
    // table size. If useDCBias is True, an attempt is made to use the 
    // DCBias (average of the autocorrelations over the last 5% of the lags) in
    // the van vleck correction if the Schwab correction is specified.
    virtual const Array<Float> data(uInt whichSampler);

    // get the zero channel for the current row, vanVleck correction is
    // applied.  It is only done once for each row, independent of whether
    // data or zeroChannel are called first.
    virtual const Array<Float> zeroChannel(uInt whichSampler);

    // is the data in the current row bad.  It is only done once for each row,
    // independent of whether data, zeroChannel, or badData are called first.
    virtual const Array<Bool> badData(uInt whichSampler);
			     
    // Do the switching signals in ACT_STATE follow the
    // documentation?  Prior to FITSVER 1.2 they had the
    // opposite sense.
    virtual Bool switchOK() const {return defBank().switchOK();}

    // return the number of sampler levels for the indicate sampler
    virtual Int nlevels(uInt whichSampler) const;

    // The value of the INTEGRAT field in the current row for
    // the given sampler and state
    virtual Float integrat(uInt whichSampler, uInt whichState) const;

    // The DMJD value from the current row
    virtual Double dmjd() {return defBank().dmjd();}

    // The value of the TIME-MID field in the current row
    virtual Double timeMid() {return defBank().timeMid();}

    // A string comprising the banks to be filled
    virtual String banks() {return itsBankNames;}
private:
    Bool itsValid;

    GBTACSTable::VanVleckCorr itsVVCorr;
    GBTACSTable::Smoothing itsSmoothing;
    Int itsVVSize, itsMinbiasfactor, itsSpikeStart;
    Bool itsUseDCBias, itsFixlags;
    Double itsDCBias, itsSigmaFactor;

    Vector<String> itsFiles;

    Table itsSampler, itsPort;
    TableRow itsSamplerRow, itsPortRow;

    //# The individual banks, there should never be more than 4
    Block<GBTACSTable *> itsBanks;

    //# the index of the 0 sampler in that bloc
    Block<uInt> itsSampler0;

    uInt itsNbanks;

    String itsBankNames;

    //# maps from requested sampler to index into bank blocks
    SimpleOrderedMap<uInt, uInt> itsBankMap;

    String itsFixLagsLog;

    Bool init(Bool resync=False);

    GBTACSTable &defBank() const {return *(itsBanks[0]);}

    // Undefined and inaccessible.
    GBTMultiACSTable();
    GBTMultiACSTable(const GBTMultiACSTable &);
    GBTMultiACSTable &operator=(const GBTMultiACSTable &);
};


#endif
