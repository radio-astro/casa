//# SDDFile.cc: defines SDDFile, which access unipops SDD data files
//# Copyright (C) 1999,2001,2002
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

#include <nrao/SDD/SDDFile.h>
#include <nrao/SDD/SDDBootStrap.h>
#include <nrao/SDD/SDDIndex.h>
#include <nrao/SDD/SDDHeader.h>
#include <casa/BasicSL/String.h>
#include <casa/Exceptions/Error.h>
#include <casa/Arrays/Vector.h>
#include <casa/BasicMath/Math.h>
#include <casa/BasicSL/Constants.h>

#include <casa/fstream.h>

SDDFile::SDDFile()
    : boot_p(0), index_p(0), header_p(0), currentHeader_p(-1),
      currentOffset_p(-1), option_p(Scratch), fileName_p(0)
{
    // just use the default constructors
    if ((boot_p = new SDDBootStrap) == 0) {
	throw(AllocError("Unable to create default SDDBootStrap",0));
    }
    // set default size on index 
    boot_p->setNIndexRec(128);
    if ((index_p = new SDDIndex(*boot_p)) == 0) {
	throw(AllocError("Unable to create empty SDDIndex", 0));
    }
    if ((header_p = new SDDHeader) == 0) {
	throw(AllocError("Unable to create default SDDData", 0));
    }
    if ((fileName_p = new String) == 0) {
	throw(AllocError("Unable to create empty string",0));
    }
    nvectors_p.resize(boot_p->maxEntries());
    init_nvectors();
    resetCache();
}

SDDFile::SDDFile(const String& fileName, SDDFile::FileOption option)
    : boot_p(0), index_p(0), header_p(0), currentHeader_p(-1),
      currentOffset_p(-1), option_p(option), fileName_p(0)
{
    if ((fileName_p = new String(fileName)) == 0) {
	throw(AllocError("Unable to create fileName String",1));
    }

    file_p = new fstream(fileName.chars(), static_cast<ios::openmode>(fileMode(option)));
    if (file_p.null()) {
	throw(AllocError("Unable to create fstream",1));
    }
    // check on the status of the open
    if (!(*file_p)) {
	throw(AipsError("There was a problem opening " +
			fileName));
    }
    if ((header_p = new SDDHeader) == 0) {
	throw(AllocError("Unable to create default SDDData", 1));
    }

    if (option == Old || option == Update) {
	// initialize from the file
	if ((boot_p = new SDDBootStrap(file_p)) == 0) {
	    throw(AllocError("Unable to create bootstrap",1));
	}
	if (!(*file_p)) {
	    throw (AipsError("There was a problem reading the bootstrap"));
	}
	if ((index_p = new SDDIndex(*boot_p)) == 0) {
	    throw(AllocError("Unable to create index",1));
	}
	if (!(*file_p)) {
	    throw (AipsError("There was a problem reading the index"));
	}
        nvectors_p.resize(boot_p->maxEntries());
        init_nvectors();
        resetCache();
	// and load up the first data block if one is indicated in the index
	for (uInt i=0;i<boot_p->maxEntryUsed();i++) {
	    if (inUse(i)) {
		loadLocation(i);
		break;
	    }
	}
    } else {
	// just use the default constructors
	if ((boot_p = new SDDBootStrap) == 0) {
	    throw(AllocError("Unable to create default SDDBootStrap",0));
	}
	// set default size on index 
	boot_p->setNIndexRec(128);
	if ((index_p = new SDDIndex(*boot_p)) == 0) {
	    throw(AllocError("Unable to create empty SDDIndex", 0));
	}
	// write out BootStrap and Index
	boot_p->write();
	index_p->write();
        nvectors_p.resize(boot_p->maxEntries());
        init_nvectors();
        resetCache();
    }
}

SDDFile::SDDFile(const SDDFile& other)
    : boot_p(0), index_p(0), header_p(0), 
      data_p(other.data_p), time_p(other.time_p), 
      raOffset_p(other.raOffset_p), decOffset_p(other.decOffset_p), 
      az_p(other.az_p), el_p(other.el_p), file_p(other.file_p), 
      nvectors_p(other.nvectors_p),
      currentHeader_p(other.currentHeader_p), 
      currentOffset_p(other.currentOffset_p),
      option_p(other.option_p), fileName_p(0)
{
    // just use the default constructors
    if ((boot_p = new SDDBootStrap(*other.boot_p)) == 0) {
	throw(AllocError("Unable to create SDDBootStrap",1));
    }
    if ((index_p = new SDDIndex(*other.index_p)) == 0) {
	throw(AllocError("Unable to create SDDIndex", 1));
    }
    if ((header_p = new SDDHeader(*other.header_p)) == 0) {
	throw(AllocError("Unable to create SDDData", 1));
    }
    if ((fileName_p = new String) == 0) {
	throw(AllocError("Unable to create empty string",0));
    }
}

SDDFile::~SDDFile()
{
    delete boot_p;
    delete index_p;
    delete header_p;
    delete fileName_p;
}

SDDFile& SDDFile::operator=(const SDDFile& other)
{
    if (this == &other) return *this;

    // use assignment operators
    *boot_p = *other.boot_p;
    *index_p = *other.index_p;
    *header_p = *other.header_p;
    data_p = other.data_p;
    time_p = other.time_p;
    raOffset_p = other.raOffset_p;
    decOffset_p = other.decOffset_p;
    az_p = other.az_p;
    el_p = other.el_p;
    currentHeader_p = other.currentHeader_p;
    currentOffset_p = other.currentOffset_p;
    option_p = other.option_p;
    nvectors_p = other.nvectors_p;
    *fileName_p = *other.fileName_p;
    file_p = other.file_p;
    return *this;
}

void SDDFile::attach(const String& fileName, FileOption option)
{
    option_p = option;
    *fileName_p = fileName;
    if ((file_p->rdbuf())->is_open()) 
	file_p->close();
    file_p->open(fileName_p->chars(), static_cast<ios::openmode>(fileMode(option_p)));
    if (!file_p->good()) {
	throw(AipsError("SDDFile::attach(const String&, FileOption "
			"error opening file"));
    }
    init_nvectors();
    resetCache();
    // load the first thing in the index
    uInt entry = 0;
    while (entry < boot_p->maxEntryUsed()) {
	if (inUse(entry)) {
	    loadLocation(entry);
	    break;
	}
	entry++;
    }
}

SDDHeader& SDDFile::header(uInt location)
{
    if (currentHeader_p != Int(location)) {
	if (inUse(location)) {
	    loadLocation(location);
	} else {
	    resetCache();
	}
    }
    return *header_p;
}

SDDHeader& SDDFile::header(uInt scan, uInt subScan)
{
    // find it in the index
    uInt i;
    for (i=0;i<boot_p->maxEntryUsed();i++) {
	if (inUse(i) &&
	    (*index_p)(i).scan() == scan &&
	    (*index_p)(i).subScan() == subScan)
	    break;
    }
    if (i < uInt(index_p->maxEntryInUse())) {
	// one was found
	loadLocation(i);
    } else {
	// nothing found, empty everything
	resetCache();
    }
    return *header_p;
}
  

void SDDFile::seek(uInt record)
{
    // non-const seeks both put and get
    uInt bytes = record * boot_p->bytesPerRecord();
    file_p->seekg(bytes);
}

uInt SDDFile::fileMode(SDDFile::FileOption option) const
{
    uInt iosOpt;
    switch (option) {
    case Old:
	iosOpt = ios::in; 
	break;
    case New:
	iosOpt = ios::in | ios::out;
	break;
    case NewNoReplace:
        // the ios::noreplace option is non-standard so something else will
        // need to be done at a different level to catch this problem
        //
	// iosOpt = ios::in | ios::out | ios::noreplace;
	iosOpt = ios::in | ios::out;
	break;
    case Scratch:
	iosOpt = ios::in | ios::out | ios::trunc;
	break;
    case Update:
	iosOpt = ios::in | ios::out | ios::app;
	break;
    case Delete:
	iosOpt = ios::in | ios::out | ios::trunc;
	break;
    default:
	// this should never happen
	throw(AipsError("Invalid FileOption!"));
    }
    return iosOpt;
}

Bool SDDFile::getData(Array<Float>& data, uInt location, uInt which)
{
    Bool tmp = False;
    if (Int(location) != currentHeader_p && inUse(location)) {
	loadLocation(location);
    }
    if (Int(which) != currentOffset_p && inUse(location)) {
	if (which < header_p->nvectors()) {
	    // seek appropriately
	    uInt absoluteOffset = 
		((*index_p)(location).firstRecord()*boot_p->bytesPerRecord()) +
		uInt(header_p->get(SDDHeader::HEADLEN)) +
		which * uInt(header_p->get(SDDHeader::NOINT))*4;
	    (*file_p).seekg(absoluteOffset);
	    // load it up
	    Bool deleteIt;
	    Float* storage = data_p.getStorage(deleteIt);
	    (*file_p).read((char *)storage, 
			   uInt(header_p->get(SDDHeader::NOINT)*4));
	    data_p.putStorage(storage, deleteIt);
	    currentOffset_p = which;
	    tmp = True;
	}
    } else if (inUse(location)) {
	tmp = True;
    }
    if (tmp) {
 	if (data.nelements() != data_p.nelements()) 
	    data.resize(data_p.shape());
	data = data_p;
    }
    return tmp;
}

Float SDDFile::time(uInt location, uInt which)
{
    // time in seconds of UT
    if (Int(location )!= currentHeader_p) loadLocation(location);
    Float value = header_p->get(SDDHeader::UT) * 3600.0;
    if (nvectors(location) > 1)
	value += (time_p(which) - time_p(0)) / 1000.0;
    return value;
}

Float SDDFile::raOffset(uInt location, uInt which)
{
    Float value = 0;
    if (nvectors(location) > 1) {
	if (Int(location) != currentHeader_p) loadLocation(location);
	value = raOffset_p(which)/cosdec_p;
    }
    return value;
}

Float SDDFile::decOffset(uInt location, uInt which)
{
    Float value = 0;
    if (nvectors(location) > 1) {
	if (Int(location) != currentHeader_p) loadLocation(location);
	value = decOffset_p(which);
    }
    return value;
}

Float SDDFile::az(uInt location, uInt which)
{
    if (Int(location) != currentHeader_p) loadLocation(location);
    Float value = header_p->get(SDDHeader::AZ);
    if (nvectors(location) > 1) value = az_p(which);
    return value;
}

Float SDDFile::el(uInt location, uInt which)
{
    if (Int(location) != currentHeader_p) loadLocation(location);
    Float value = header_p->get(SDDHeader::EL);
    if (nvectors(location) > 1) value = el_p(which);
    return value;
}

void SDDFile::init_nvectors()
{
    // loop through each one
    for (uInt i=0;i<boot_p->maxEntries();i++) {
	if (inUse(i)) {
	    nvectors_p[i] = header(i).nvectors();
	} else {
	    nvectors_p[i] = 0;
	}
    }
}

void SDDFile::loadLocation(uInt location) 
{
    static Double radPerDeg = C::pi/180.0;

    if (inUse(location)) {
	seek(((*index_p)(location)).firstRecord());
	header_p->fill(*file_p);
	if (!(*file_p)) {
	    throw(AipsError("There was a problem reading from the desired location"));
	}
	currentHeader_p = location;
	// the first vector must be next, no seek necessary
	// resize the vector
	data_p.resize(uInt(header_p->get(SDDHeader::NOINT)));
	Bool deleteIt;
	Float* storage = data_p.getStorage(deleteIt);
	(*file_p).read((char *)storage, data_p.nelements() * sizeof(Float));
	data_p.putStorage(storage, deleteIt);
	currentOffset_p = 0;
	// for OTF data, advance to the associated arrays and locd them
	if (header_p->isOTF()) {
	    // The RA offset stored with the data need to be corrected by the
	    // cos(EPOCDEC) to get the offset in RA at the equator
	    cosdec_p = cos(header_p->get(SDDHeader::EPOCDEC) * radPerDeg);
	    // resize the OTF vectors
	    time_p.resize(header_p->nvectors());
	    raOffset_p.resize(header_p->nvectors());
	    decOffset_p.resize(header_p->nvectors());
	    az_p.resize(header_p->nvectors());
	    el_p.resize(header_p->nvectors());
	    // How many bytes to advance
	    uInt offset = (header_p->nvectors()-1) * 
		uInt(header_p->get(SDDHeader::NOINT)) * sizeof(Float);
	    // and advance relative to current location
	    (*file_p).seekg(offset, ios::cur);
	    // and read them in in order
	    uInt nbytes = header_p->nvectors() * sizeof(Float);
	    storage = time_p.getStorage(deleteIt);
	    (*file_p).read((char *)storage, nbytes);
	    time_p.putStorage(storage, deleteIt);
	    storage = raOffset_p.getStorage(deleteIt);
	    (*file_p).read((char *)storage, nbytes);
	    raOffset_p.putStorage(storage, deleteIt);
	    storage = decOffset_p.getStorage(deleteIt);
	    (*file_p).read((char *)storage, nbytes);
	    decOffset_p.putStorage(storage, deleteIt);
	    storage = az_p.getStorage(deleteIt);
	    (*file_p).read((char *)storage, nbytes);
	    az_p.putStorage(storage, deleteIt);
	    storage = el_p.getStorage(deleteIt);
	    (*file_p).read((char *)storage, nbytes);
	    el_p.putStorage(storage, deleteIt);
	} 
    } else {
	resetCache();
    }
}

void SDDFile::resetCache() 
{
    header_p->empty();
    currentOffset_p = -1;
    currentHeader_p = -1;
}

Bool SDDFile::fullUpdate()
{
    Bool updated = False;
    // do a full update on the index
    if (updated = index_p->fullUpdate()) {
	// bring things back in to sync
	*boot_p = index_p->bootstrap();
	// remember where we are
	uInt currentLocation = currentHeader_p;
	uInt which = currentOffset_p;
	// reset the cache of number of vectors at each location
	resetCache();
	init_nvectors();
	// see if where we were is still there on disk
	if (!getData(data_p, currentLocation, which)) {
	    // this means that for some reason the current stuff is no longer valid
	    // Get the first location in use element 
	    for (uInt i=0;i<boot_p->maxEntryUsed();i++) {
		if (inUse(i)) {
		    loadLocation(i);
		    break;
		}
	    }
	}
    }
    return updated;
}

Bool SDDFile::incrementalUpdate()
{
    Bool ok = True;
    // remember where we are now
    uInt old_maxEntryUsed = boot_p->maxEntryUsed();
    // do an incremental update on the index
    if (ok = index_p->incrementalUpdate()) {
	 // bring things back in to sync - we know the bootstrap has changed
	*boot_p = index_p->bootstrap();
	// update the cache of vectors
	// loop through each new one
	for (uInt i=old_maxEntryUsed;i<boot_p->maxEntryUsed();i++) {
	    if (inUse(i)) {
		nvectors_p[i] = header(i).nvectors();
	    }
	}
    }
    return ok;
}
	    
