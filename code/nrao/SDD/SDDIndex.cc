//# SDDIndex.cc: defines SDDIndexRep and SDDIndex, classes containing SDDFile index information
//# Copyright (C) 1999,2001
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

//# Includes

#include <nrao/SDD/SDDIndex.h>
#include <casa/Arrays/Vector.h>
#include <casa/BasicMath/Math.h>
#include <casa/BasicSL/Constants.h>
#include <casa/Exceptions/Error.h>

Block<String> SDDIndexRep::posCodeMap_p;
Block<String> SDDIndexRep::obsModeMap_p;

SDDIndexRep::SDDIndexRep()
    : isLine_p(True), hasChanged_p(False), coordSys_p(0), mode_p(0),
      firstRecord_p(0), nRecords_p(0), scan_p(0), subScan_p(0),
      record_p(0), intPhase_p(0),
      hCoord_p(0), vCoord_p(0), freqRes_p(0), slewRate_p(0), lst_p(0),
      restFreq_p(0), intTime_p(0),
      source_p(""), utDate_p(0)
{
    init_maps();
}

SDDIndexRep::SDDIndexRep(const SDDIndexRep& other)
{
    *this = other;
}

SDDIndexRep& SDDIndexRep::operator=(const SDDIndexRep& other)
{
    if (this != &other) {
	isLine_p = other.isLine_p;
	firstRecord_p = other.firstRecord_p;
	nRecords_p = other.nRecords_p;
	scan_p = other.scan_p; 
	subScan_p = other.subScan_p;
	record_p = other.record_p;
	intPhase_p = other.intPhase_p;
	hCoord_p = other.hCoord_p;
	vCoord_p = other.vCoord_p; 
	freqRes_p = other.freqRes_p;
	slewRate_p = other.slewRate_p;
	lst_p = other.lst_p;
	restFreq_p = other.restFreq_p;
	intTime_p = other.intTime_p;
	coordSys_p = other.coordSys_p;
	source_p = other.source_p;
	mode_p = other.mode_p;
	utDate_p = other.utDate_p;
	hasChanged_p = True;
    }
    return *this;
}

Bool SDDIndexRep::operator==(const SDDIndexRep& other) const
{
    Bool result = True;
    if (this != &other) {
	result = ( isLine_p == other.isLine_p &&
			 firstRecord_p == other.firstRecord_p &&
			 nRecords_p == other.nRecords_p &&
			 scan_p == other.scan_p &&
			 subScan_p == other.subScan_p &&
			 record_p == other.record_p &&
			 intPhase_p == other.intPhase_p &&
			 hCoord_p == other.hCoord_p &&
			 vCoord_p == other.vCoord_p && 
			 lst_p == other.lst_p &&
			 coordSys_p == other.coordSys_p &&
			 source_p == other.source_p &&
			 mode_p == other.mode_p &&
			 utDate_p == other.utDate_p);
	if (result && isLine_p) {
	    result = ( result && 
			     freqRes_p == other.freqRes_p &&
			     restFreq_p == other.restFreq_p);
	} else {
	    result = ( result &&
			     slewRate_p == other.slewRate_p &&
			     intTime_p == other.intTime_p);
	}
    }
    return result;
}

Bool SDDIndexRep::operator!=(const SDDIndexRep& other) const
{
    return ((! (*this == other)));
}
    
void SDDIndexRep::setcoordSys(const String& new_coordSys)
{
    // search through posCodeMap_p for any matches
    // but we know that any string that isn't 8 chars won't match
    if (new_coordSys.length() != 8) {
	coordSys_p = 0;
    } else {
	for (uShort i = 0;i<posCodeMap_p.nelements();i++)
	    if (new_coordSys == posCodeMap_p[i]) {
		coordSys_p = i;
		break;
	    }
	if (posCodeMap_p[coordSys_p] != new_coordSys) coordSys_p = 0;
    }
    hasChanged_p = True;
}

void SDDIndexRep::setcoordSys(uShort new_coordCode)
{
    if (new_coordCode >= posCodeMap_p.nelements()) coordSys_p = 0;
    else coordSys_p = new_coordCode;
    hasChanged_p = True;
}

void SDDIndexRep::setsource(const String& new_source)
{
    // find last non-blank character in new_source out to a 
    // max of 16 characters
    uInt length = min(uInt(16), new_source.length());
    uInt size;
    for (size=length;size>0;size--) {
	if (new_source[size-1] != ' ') break;
    }
    source_p = String(new_source.chars(), size);
    hasChanged_p = True;
}

void SDDIndexRep::setscan(uInt newScan)
{
    if (newScan > 99999) scan_p = 0;
    else scan_p = newScan;
    hasChanged_p = True;
}

void SDDIndexRep::setsubScan(uInt newSubScan)
{
    if (newSubScan < 1 || newSubScan > 99) {
	subScan_p = 0;
	scan_p = 0;
    } else
	subScan_p = newSubScan;
    hasChanged_p = True;
}

void SDDIndexRep::setfreqResolution(Float new_freqRes)
{
    setLine();
    freqRes_p = new_freqRes;
    hasChanged_p = True;
}

void SDDIndexRep::setslewRate(Float new_slewRate)
{
    setCont();
    slewRate_p = new_slewRate;
    hasChanged_p = True;
}

void SDDIndexRep::setrestFrequency(Double new_restFreq)
{
    setLine();
    restFreq_p = new_restFreq;
    hasChanged_p = True;
}


void SDDIndexRep::setintTimePerPoint(Double new_intTime)
{
    setCont();
    intTime_p = new_intTime;
}

void SDDIndexRep::setmode(const String& new_mode)
{
    // search through obsModeMap_p for any matches
    // but we know that any string that isn't 4 chars won't match
    if (new_mode.length() != 4) {
	mode_p = 0;
    } else {
	for (uShort i = 0;i<obsModeMap_p.nelements();i++)
	    if (new_mode == obsModeMap_p[i]) {
		mode_p = i;
		break;
	    }
	if (obsModeMap_p[mode_p] != new_mode) mode_p = 0;
    }
    hasChanged_p = True;
}

void SDDIndexRep::setmode(uShort new_modeCode)
{
    if (new_modeCode >= obsModeMap_p.nelements()) mode_p = 0;
    else mode_p = new_modeCode;
    hasChanged_p = True;
}

void SDDIndexRep::setrecord(Int new_record)
{
    if (new_record < 0) {
	intPhase_p = -1;
	record_p = -1;
    } else {
	record_p = new_record;
    }
    hasChanged_p = True;
}

void SDDIndexRep::setintPhase(Int new_intPhase)
{
    if (new_intPhase < 0) {
	intPhase_p = -1;
	record_p = -1;
    } else {
	intPhase_p = new_intPhase;
    }
    hasChanged_p = True;
}

void SDDIndexRep::setLine()
{
    isLine_p = True;
    slewRate_p = std::numeric_limits<double>::quiet_NaN();
    intTime_p = std::numeric_limits<double>::quiet_NaN();
    hasChanged_p = True;
}

void SDDIndexRep::setCont()
{
    isLine_p = False;
    freqRes_p = std::numeric_limits<double>::quiet_NaN();
    restFreq_p = std::numeric_limits<double>::quiet_NaN();
    hasChanged_p = True;
}

void SDDIndexRep::init_maps()
{
    // if one has the correct size, they must be initialized
    if (posCodeMap_p.nelements() == 15) return;
	
    // first, the short one is the POSCODE vector
    posCodeMap_p.resize(15);
    posCodeMap_p[0] = "UNKNOWN ";    // unknown coordinate system
    posCodeMap_p[1] = "GALACTIC";    // (LII, BII)
    posCodeMap_p[2] = "1950RADC";    // (Ra,Dec) at equinox=1950
    posCodeMap_p[3] = "EPOCRADC";    // (Ra,Dec) at equinox=date
    posCodeMap_p[4] = "MEANRADC";    // (Ra,Dec) mean position at start of scan
    posCodeMap_p[5] = "APPRADC ";    // (Ra,Dec) apparent at start of scan
    posCodeMap_p[6] = "APPHADC ";    // (Ha,Dec) apparent at start of scan
    posCodeMap_p[7] = "1950ECL ";    // Ecliptic coords at equinox=1950
    posCodeMap_p[8] = "EPOCECL ";    // Ecliptic coords at equinox=date
    posCodeMap_p[9] = "MEANECL ";    // Mean ecliptic position at start of scan
    posCodeMap_p[10]= "APPECL  ";    // Apparent ecliptic at start of scan
    posCodeMap_p[11]= "AZEL    ";    // (Azimuth, Elevation)
    posCodeMap_p[12]= "USERDEF ";    // User defined coordinate system
    posCodeMap_p[13]= "2000RADC";    // (Ra,Dec) at equinox=2000
    posCodeMap_p[14]= "INDRADC ";    // (Ra,Dec) indicated

    // now, the OBSMODE vector
    obsModeMap_p.resize(49);
    obsModeMap_p[0] = "    ";       // No mode present, or unknown mode
    obsModeMap_p[1] = "PS  ";       // Position Switched
    obsModeMap_p[2] = "APS ";       // Absolute Position Switched
    obsModeMap_p[3] = "FS  ";       // Frequency Switched
    obsModeMap_p[4] = "BSP ";       // Beam Switched + Position Switched
    obsModeMap_p[5] = "TPON";       // Total Power - ON
    obsModeMap_p[6] = "TPOF";       // Total Power - OFF
    obsModeMap_p[7] = "ATP ";       // Absolute Total Power
    obsModeMap_p[8] = "PSM ";       // Position Switched Map
    obsModeMap_p[9] = "APSM";       // Absolute Position Switched Map
    obsModeMap_p[10]= "FSM ";       // Frequency Switched Map
    obsModeMap_p[11]= "TPMO";       // Total Power Map - ON
    obsModeMap_p[12]= "TPMF";       // Total Power Map - OFF
    obsModeMap_p[13]= "DRF ";       // Drift Map
    obsModeMap_p[14]= "PCAL";       // Position Calibrate
    obsModeMap_p[15]= "BCAL";       // Beam Calibrate
    obsModeMap_p[16]= "BLNK";       // Blanking
    obsModeMap_p[17]= "SEQ ";       // Sequence
    obsModeMap_p[18]= "FIVE";       // Five Point
    obsModeMap_p[19]= "MAP ";       // Continuum Map
    obsModeMap_p[20]= "FOC ";       // Focalize
    obsModeMap_p[21]= "NSFC";       // North-South Focalize
    obsModeMap_p[22]= "TTIP";       // Total Power Tip
    obsModeMap_p[23]= "STIP";       // Switched Power Tip
    obsModeMap_p[24]= "DON ";       // Continuum On
    obsModeMap_p[25]= "CAL ";       // Calibration
    obsModeMap_p[26]= "FSPS";       // Frequency Switched + Position Switched
    obsModeMap_p[27]= "BSPS";       // Beam Switched + Position Switched
    obsModeMap_p[28]= "ZERO";       // Zero check
    obsModeMap_p[29]= "TLPW";       // Total Power
    obsModeMap_p[30]= "FQSW";       // Frequency Switched
    obsModeMap_p[31]= "NOCL";       // No Calibration
    obsModeMap_p[32]= "PLCL";       // Pulse Calibration
    obsModeMap_p[33]= "ONOF";       // Continuum On/Off Scan
    obsModeMap_p[34]= "BMSW";       // Nutation
    obsModeMap_p[35]= "PSSW";       // Position Switched, Tucson, Old
    obsModeMap_p[36]= "DRFT";       // Continuum Drift Scans, Tucson
    obsModeMap_p[37]= "OTF ";       // On-The-Fly, Tucson
    obsModeMap_p[38]= "SON ";       // ? Tucson
    obsModeMap_p[39]= "SOF ";       // ? Tucson
    obsModeMap_p[40]= "QK5 ";       // ? Tucson
    obsModeMap_p[41]= "QK5A";       // ? Tucson
    obsModeMap_p[42]= "PSS1";       // PS flip or PS-1 Mode, Tucson 
    obsModeMap_p[43]= "VLBI";       // VLBI
    obsModeMap_p[44]= "PZC ";       // ? Tucson
    obsModeMap_p[45]= "CPZM";       // ? Tucson
    obsModeMap_p[46]= "PSPZ";       // Position Switched, polarization, Tucson
    obsModeMap_p[47]= "CPZ1";       // ? Tucson
    obsModeMap_p[48]= "CPZ2";       // ? Tucson
}
   

SDDIndex::SDDIndex(const SDDBootStrap& bs)
    : index_p(0), bootstrap_p(bs)
{
    attach(bs);
}

SDDIndex::SDDIndex(const SDDIndex& other)
    : index_p(0), bootstrap_p(other.bootstrap_p)
{
    if (!(index_p = new Block<SDDIndexRep>(*other.index_p))) {
	throw(AllocError("SDDIndex::SDDIndex(const SDDIndex& other) - "
			 "failed to create Block<SDDIndexRep>", 
			 other.index_p->nelements()));
    }
}

SDDIndex::~SDDIndex()
{
    delete index_p;
}

SDDIndex& SDDIndex::operator=(const SDDIndex& other)
{
    if (this != &other) {
	*index_p = *(other.index_p);
	bootstrap_p = other.bootstrap_p;
    }
    return *this;
}

Bool SDDIndex::fullUpdate()
{
    Bool tmp;
    if (tmp = bootstrap_p.hasChanged()) {
	// hasChanged will only return True if there is a file attached
	// get the bootstrap in sync
	bootstrap_p.sync();
	// seek past the first record, this is probably not necessary
	(*bootstrap_p.theStream()).seekg(bootstrap_p.bytesPerRecord());
	// read 1 record, readEntry takes care of any further reads
	sddBlock.read(*(bootstrap_p.theStream()));
	currentRecord_p = 1;
	for (uInt i=0;i<bootstrap_p.maxEntryUsed();i++) 
	    readEntry((*index_p)[i], i);
    }
    return tmp;
}

Bool SDDIndex::incrementalUpdate()
{
    Bool ok = True;
    if (bootstrap_p.hasChanged()) {
	// hasChanged will only return True if there is a file attached
	// verify that the last 8 entries we know about are in sync
	uInt lastEntry = bootstrap_p.maxEntryUsed() - 1;
	uInt startEntry = max(Int(0), Int(lastEntry - 7));
	// get bootstrap in sync
	bootstrap_p.sync();
	SDDIndexRep rep;
	for (uInt i=startEntry;i<=lastEntry;i++) {
	    readEntry(rep, i);
	    if (rep != (*index_p)[i]) {
		ok = False;
		break;
	    }
	}
	if (ok) {
	    // resize Block if necessary, this is probably expensive
	    if (index_p->nelements() < (bootstrap_p.maxEntries())) {
		index_p->resize(bootstrap_p.maxEntries(), False, True);
	    }
	    // this forces a re-read of the current record
	    currentRecord_p = 0;
	    for (uInt i=lastEntry+1;i<bootstrap_p.maxEntryUsed();i++)
		readEntry((*index_p)[i], i);
	}
    }
    return ok;
}
	

void SDDIndex::readEntry(SDDIndexRep& rep, uInt entry)
{
    uInt sddRecord = (entry/bootstrap_p.entriesPerRecord() + 1);
    if (sddRecord != currentRecord_p) {
	// need to fill buffer, sanity checks on appropriateness occur elsewhere
	// first, seek to appropriate record
	(*bootstrap_p.theStream()).seekg(sddRecord * 
					bootstrap_p.bytesPerRecord());
	// then read
	sddBlock.read(*(bootstrap_p.theStream()));
	currentRecord_p = sddRecord;
    }
    uInt offset = entry % bootstrap_p.entriesPerRecord();
    // end fill it in, first the stuff that depends on version
    if (bootstrap_p.version() == SDDBootStrap::CURRENT) {
	rep.setFirstRecord(valueRef(NEW_FIRST, offset)-1);
	rep.setnRecords(valueRef(NEW_LAST, offset) - rep.firstRecord());
	rep.setcoordSys(valueRef(NEW_POSCODE, offset));
    } else {
	rep.setFirstRecord(valueRef(OLD_FIRST, offset)-1);
	rep.setnRecords(valueRef(OLD_LAST, offset) - rep.firstRecord());
	rep.setcoordSys(valueRef(OLD_POSCODE, offset));
    }
    // and everything else
    uShort obsMode = valueRef(OBSMODE, offset);
    rep.setmode(obsMode % 256);
    if (obsMode >= 512) rep.setLine();
    else rep.setCont();
    
    Float scanNumber = valueRef(SCAN_NUMBER, offset);
    rep.setscan(uInt(scanNumber + 0.5));
    rep.setsubScan(uInt((scanNumber - rep.scan())*100.0 + 0.5));
    Short recordPhase = valueRef(RECORD_PHASE, offset);
    if (recordPhase > 0) {
	rep.setrecord(recordPhase/64);
	rep.setintPhase(recordPhase % 64);
    } else {
	rep.setrecord(-1);
	rep.setintPhase(-1);
    }
    rep.sethCoord(valueRef(HCOORD, offset));
    rep.setvCoord(valueRef(VCOORD, offset));
    rep.setlst(valueRef(LST, offset));
    rep.setsource(&(valueRef(SOURCE, offset, 0)));
    Float utdate = valueRef(UTDATE, offset);
    uInt year = uInt(utdate);
    utdate = (utdate - year)*100.0;
    uInt month = uInt(utdate);
    utdate = (utdate - month)*100.0;
    uInt day = uInt(utdate+0.5);
    rep.setutDate(Time(year, month, day));
    if (rep.isLine()) {
	rep.setfreqResolution(valueRef(FRES_SLEWRATE, offset));
	rep.setrestFrequency(valueRef(RESTF_INTTIME, offset));
    } else {
	rep.setslewRate(valueRef(FRES_SLEWRATE, offset));
	rep.setintTimePerPoint(valueRef(RESTF_INTTIME, offset));
    }
    rep.clearChangeFlag();
}

void SDDIndex::attach(const SDDBootStrap& bs)
{
    // delete any current index, writing out anything that has changed (write
    // makes all the necessary checks)
    if (index_p) {
	delete index_p;
	write();
    }

    bootstrap_p = bs;
    needsFlushed_p = False;
    // create an empty index using the size indicated in the bootstrap
    if (!(index_p = new Block<SDDIndexRep>(bs.maxEntries()))) {
	throw(AllocError("SDDIndex::SDDIndex(const SDDBootStrap& bs) - "
	    "failed to create Block<SDDIndexRep>", (bs.maxEntries())));
    }
    // specifically clear each entry
    for (uInt i=0;i<nrEntry();i++) clear(i);

    // changing NIndexRec in bootstrap_p forces a true full update
    // this value is ultimately lost, it mearly forces this bootstrap to
    // differ from the one on disk. A value of zero should never really be on disk.
    bootstrap_p.setNIndexRec(0);
    fullUpdate();
}

void SDDIndex::write()
{
    write(0,index_p->nelements());
}

void SDDIndex::write(uInt firstEntry, uInt nEntries)
{
    // only if we are attached to a file
    if (bootstrap_p.theStream().null()) return;

    // and only if they have really changed
    for (uInt entry=firstEntry; entry < (firstEntry+nEntries); entry++) {
	SDDIndexRep& rep = (*index_p)[entry];
	if (rep.hasChanged()) {
	    writeEntry(rep, entry);
	    rep.clearChangeFlag();
	    if ((entry+1) > bootstrap_p.maxEntryUsed())
		bootstrap_p.setMaxEntryUsed(entry+1);
	}
    }
    // write out the buffer if it needs to be done
    if (needsFlushed_p) {
	// seek to the start of this block
	(*bootstrap_p.theStream()).seekg(currentRecord_p * 
				    bootstrap_p.bytesPerRecord());
	sddBlock.write(*(bootstrap_p.theStream()));
	// and the bootstrap if that needs to be written
	if (bootstrap_p.hasChanged())
	    bootstrap_p.write();
    }
    needsFlushed_p = False;
}

void SDDIndex::writeEntry(SDDIndexRep& rep, uInt entry)
{
    uInt sddRecord = (entry/bootstrap_p.entriesPerRecord()+1);
    if (sddRecord != currentRecord_p) {
	// need to write to buffer not at this location
	// first, if we need to write out this buffer
	if (needsFlushed_p) {
	    	(*bootstrap_p.theStream()).seekg(currentRecord_p * 
				    bootstrap_p.bytesPerRecord());
		sddBlock.write(*(bootstrap_p.theStream()));
	}
 	// seek to the start of the desired block
	(*bootstrap_p.theStream()).seekg(sddRecord * 
					 bootstrap_p.bytesPerRecord());
	// then read it in so that we get things that may not be changing
	sddBlock.read(*(bootstrap_p.theStream()));
	currentRecord_p = sddRecord;
    }
    uInt offset = entry % bootstrap_p.entriesPerRecord();
    // end fill it in to the block, first the stuff that depends on version
    if (bootstrap_p.version() == SDDBootStrap::CURRENT) {
	valueRef(NEW_FIRST, offset) = rep.firstRecord() + 1;
	valueRef(NEW_LAST, offset)  = rep.firstRecord() + rep.nRecords();
	valueRef(NEW_POSCODE, offset) = rep.coordCode();
    } else {
	valueRef(OLD_FIRST, offset) = rep.firstRecord() + 1;
	valueRef(OLD_LAST, offset) = rep.firstRecord() + rep.nRecords();
	valueRef(OLD_POSCODE, offset) = rep.coordCode();
    }
    // and everything else
    uShort obsMode = rep.modeCode();
    if (rep.isLine()) {
	obsMode += 512;
    } else {
	obsMode += 256;
    }
    valueRef(OBSMODE, offset) = obsMode;

    valueRef(SCAN_NUMBER, offset) = 
	Float(rep.scan()) + Float(rep.subScan())/100.0;
    if (rep.record() > 0) {
	valueRef(RECORD_PHASE, offset) = rep.record()*64 + rep.intPhase();
    } else {
	valueRef(RECORD_PHASE, offset) = -1;
    }
    valueRef(HCOORD, offset) = rep.hCoord();
    valueRef(VCOORD, offset) = rep.vCoord();
    valueRef(LST, offset) = rep.lst();
    String src = rep.source();
    uInt i;
    for (i=0;i<src.length();i++) {
	if (src[i] == '\0') {
	    valueRef(SOURCE, offset, i) = ' ';
	} else {
	    valueRef(SOURCE, offset, i) = src[i];
	}
    }
    for (;i<16;i++) valueRef(SOURCE, offset, i) = ' ';
    Time utdate = rep.utDate();
    valueRef(UTDATE, offset) = utdate.year() + 
	(utdate.month() + utdate.dayOfMonth()/100.0)/100.0;
    if (rep.isLine()) {
	valueRef(FRES_SLEWRATE, offset) = rep.freqResolution();
	valueRef(RESTF_INTTIME, offset) = rep.restFrequency();
    } else {
	valueRef(FRES_SLEWRATE, offset) = rep.slewRate();
	valueRef(RESTF_INTTIME, offset) = rep.intTimePerPoint();
    }
    rep.clearChangeFlag();
    needsFlushed_p = True;
}

SDDIndexRep& SDDIndex::operator()(uInt entry)
{
    return (*index_p)[entry];
}

const SDDIndexRep& SDDIndex::operator()(uInt entry) const
{
    return (*index_p)[entry];
}

Fallible<uInt> SDDIndex::maxEntryInUse() const
{
    Fallible<uInt> tmp;
    for (uInt i=index_p->nelements(); i>0; i--) {
	if (inUse(i-1)) {
	    tmp = i;
	    break;
	}
    }
    return tmp;
}

uInt SDDIndex::nrEntry() const
{
    return bootstrap_p.maxEntries();
}

Bool SDDIndex::inUse(uInt entry) const
{
    return ( (((*index_p)[entry]).scan()));
}


void SDDIndex::clear(uInt entry)
{
    SDDIndexRep& rep = (*index_p)[entry];
    rep.setLine();
    rep.setFirstRecord(0);
    rep.setnRecords(0);
    rep.sethCoord(0.0);
    rep.setvCoord(0.0);
    rep.setcoordSys(0);
    rep.setsource("");
    rep.setscan(0);
    rep.setsubScan(0);
    rep.setfreqResolution(0.0);
    rep.setrestFrequency(0.0);
    rep.setlst(0.0);
    rep.setutDate(Time(0,0,0));
    rep.setmode(0);
    rep.setrecord(-1);
    rep.setintPhase(-1);
}

void SDDIndex::setVersion(SDDBootStrap::SDD_VERSION newVersion)
{
    bootstrap_p.setVersion(newVersion);
}

Short& SDDIndex::valueRef(ShortFields field, uInt entry)
{
    return sddBlock.asShort(field + 
			    entry*bootstrap_p.bytesPerEntry()/sizeof(Short));
}

const Short& SDDIndex::valueRef(ShortFields field, uInt entry) const
{
    return sddBlock.asShort(field + 
			    entry*bootstrap_p.bytesPerEntry()/sizeof(Short));
}

Int& SDDIndex::valueRef(IntFields field, uInt entry)
{
    return sddBlock.asInt(field + 
			  entry*bootstrap_p.bytesPerEntry()/sizeof(Int));
}

const Int& SDDIndex::valueRef(IntFields field, uInt entry) const
{
    return sddBlock.asInt(field + 
			  entry*bootstrap_p.bytesPerEntry()/sizeof(Int));
}

float& SDDIndex::valueRef(FloatFields field, uInt entry)
{
    return sddBlock.asfloat(field + 
			    entry*bootstrap_p.bytesPerEntry()/sizeof(Float));
}

const float& SDDIndex::valueRef(FloatFields field, uInt entry) const
{
    return sddBlock.asfloat(field + 
			    entry*bootstrap_p.bytesPerEntry()/sizeof(Float));
}

double& SDDIndex::valueRef(DoubleFields field, uInt entry)
{
    return sddBlock.asdouble(field + 
			     entry*bootstrap_p.bytesPerEntry()/sizeof(Double));
}

const double& SDDIndex::valueRef(DoubleFields field, uInt entry) const
{
    return sddBlock.asdouble(field + 
			     entry*bootstrap_p.bytesPerEntry()/sizeof(Double));
}

Char& SDDIndex::valueRef(CharFields field, uInt entry, uInt position)
{
    return sddBlock[field + 
		    entry*bootstrap_p.bytesPerEntry() + position];
}

const Char& SDDIndex::valueRef(CharFields field, uInt entry, uInt position) const
{
    return sddBlock[field + 
		    entry*bootstrap_p.bytesPerEntry() + position];
}

