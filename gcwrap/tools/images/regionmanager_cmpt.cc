/***
 * Framework independent implementation file for regionmaager...
 *
 * Implement the regionmanager component here.
 * 
 * // TODO: WRITE YOUR DESCRIPTION HERE! 
 *
 * @author
 * @version 
 ***/

#include <iostream>
#include <math.h>

#include <regionmanager_cmpt.h>
#include <casa/Arrays/ArrayMath.h>
#include <casa/BasicSL/String.h>
#include <casa/Containers/Record.h>
#include <casa/Exceptions/Error.h>
#include <casa/fstream.h>
#include <casa/Logging/LogFilter.h>
#include <casa/Logging/LogIO.h>
#include <casa/Logging/LogOrigin.h>
#include <casa/OS/Directory.h>
#include <casa/OS/EnvVar.h>
#include <casa/OS/HostInfo.h>
#include <casa/OS/RegularFile.h>
#include <casa/OS/SymLink.h>
#include <casa/Utilities/Assert.h>
#include <coordinates/Coordinates/CoordinateSystem.h>
#include <coordinates/Coordinates/StokesCoordinate.h>
#include <coordinates/Coordinates/CoordinateUtil.h>
#include <coordinates/Coordinates/DirectionCoordinate.h>
#include <images/Regions/ImageRegion.h>
#include <imageanalysis/Regions/CasacRegionManager.h>
#include <imageanalysis/Annotations/RegionTextList.h>
#include <tables/Tables/TableRecord.h>
#include <casa/namespace.h>
#include <images/Regions/WCUnion.h>

using namespace std;

namespace casac {

regionmanager::regionmanager()
   : _log(new LogIO()), _regMan(new CasacRegionManager()) {}

regionmanager::~regionmanager() {}

void regionmanager::setup() {
    if (! _regMan.ptr()) {
    	_regMan.set(new CasacRegionManager());
    }
    if (! _log.ptr()) {
    	_log.set(new LogIO());
    }
}

std::string
regionmanager::absreltype(const int absrelvalue)
{
	setup();
    *_log << LogOrigin("regionmanager", __func__);

    try {
    	return _regMan->absreltype(absrelvalue);
    }
    catch (const AipsError& x) {
	*_log << LogIO::SEVERE << "Exception Reported: "
		<< x.getMesg() << LogIO::POST;
	RETHROW(x);
    }
}

::casac::record*
regionmanager::box(const std::vector<double>& blc, const std::vector<double>& trc, const std::vector<double>& inc, const std::string& absrel, const bool frac, const std::string& comment)
{

	setup();
    *_log << LogOrigin("regionmanager", __func__);

    try {
    	Vector<Double> dblc(blc);
    	Vector<Double> dtrc(trc);
    	Vector<Double> dinc(inc);
    	Bool useAllDefaults = False;

    	if( (dtrc.nelements()==1 && dtrc[0]<0)
                 && (dblc.nelements()==1 && dblc[0]<1) ) {
    		useAllDefaults = True;
    	}
	
	if(dtrc.nelements()==1 && dtrc[0]<0)
	{
	    dtrc.resize();
	}
	if(dblc.nelements()==1 && dblc[0]<0)
	{
	    dblc.resize();
	}
	
	PtrHolder<Record> lebox;
	if ( frac || useAllDefaults ) {
	    if(inc.size()==1 && inc[0]==1)
	    {
		dinc.resize(blc.size());
		dinc.set(1.0);
	    }

	    lebox.set(
	    	_regMan->box(dblc, dtrc, dinc,
	    	String(absrel), frac, String(comment))
	    );
	} else {
	    // ARG ARG ARG!!!
	    // Since image analysis tool doesn't seem to be able to
	    // hand LCBox regions this is temporarly the same as for
	    // fractional boxes.  This requires a lot of reworking
	    // of the Image Analysis, Region Manager, and likely
	    // Imager code to get this working properly.
	    if(inc.size()==1 && inc[0]==1)
	    {
		dinc.resize(blc.size());
		dinc.set(1.0);
	    }

	    lebox.set(
	    	_regMan->box(dblc, dtrc, dinc, String(absrel),
	    	frac, String(comment))
	    );
	}
	

	if (lebox.ptr()){
	    return fromRecord(*lebox);
	}
	return 0;
    } catch (const AipsError& x) {
	*_log << LogIO::SEVERE << "Exception Reported: "
		<< x.getMesg() << LogIO::POST;
	RETHROW(x);
    }
  
}
    

// Compute the complement of a region(s)
// 
// For a set of regions the complement taken on the union
// of the regions.
::casac::record*
regionmanager::complement(
	const ::casac::variant& regions, const std::string& comment
) {
	setup();

    *_log << LogOrigin("regionmanager", __func__);
    
    // If the region parameter is not a record then throw an exception
    try {
    	ThrowIf(
    		regions.type() != ::casac::variant::RECORD,
    		"parameter 'regions' has to be a dictionary containing regions"
    	);

	// Create a single region to find the complement of.
	::casac::variant localvar(regions); //cause its const
	PtrHolder<casa::ImageRegion> unionReg;
	PtrHolder<casa::Record> lesRegions(toRecord((localvar.asRecord())));

	// If the "isRegion" field exists then we assume the record
	// is a region, otherwise we have a set of regions.
	if(lesRegions->fieldNumber("isRegion")==-1 && lesRegions->nfields() > 1 ) {
	    unionReg.set(dounion(lesRegions));
	}
	else {
	    if ( lesRegions->fieldNumber("isRegion")==-1 ) {
		TableRecord theRec;
		theRec.assign( lesRegions->asRecord(casa::RecordFieldId(0) ) );
		unionReg.set(ImageRegion::fromRecord( theRec, "" ));
	   }
	    else {
	    	unionReg.set(ImageRegion::fromRecord( *lesRegions, "" ));
	    }
	}
	
	// And find the complement
	PtrHolder<casa::ImageRegion> leComplementReg;
	if( unionReg.ptr() ){
	    leComplementReg.set(_regMan->doComplement(*unionReg));
	}
	
	casa::Record returnRec;
	returnRec.assign(leComplementReg->toRecord(""));
	if ( comment.length() > 1 ) {
	    returnRec.define("comment", comment );
	}
	else {
	    returnRec.define("comment", "Complement region created with the Region Manger tool"  );
	}
	return fromRecord(returnRec);


    } catch (const AipsError& x) {
	*_log << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
	RETHROW(x);
    }
}
    

::casac::record*
regionmanager::concatenation(
	const ::casac::variant& box, const variant& regions, const std::string& comment
) {
	setup();

    *_log << LogOrigin("regionmanager", __func__);
    
    try {
    	// If the region parameter is not a record then throw an exception
    	ThrowIf(
    		box.type() != ::casac::variant::RECORD,
    		"parameter 'box' has to be a dictionary containing a region record"
    	);

    ThrowIf(
    	regions.type() != ::casac::variant::RECORD,
		"parameter 'regions' has to be a dictionary containing one or more region records"
	);


	// ALgorihtm
	//   1. convert incoming image record to a CASA::Record(s)
	::casac::variant localregs(regions); //cause its const
	PtrHolder<casa::Record> lesRegions(toRecord((localregs.asRecord())));
	
	//   2. convert incoming box record to a TableRecord
	::casac::variant localbox(box); //cause its const
	PtrHolder<casa::Record> boxRec(toRecord((localbox.asRecord())));
	ThrowIf(
		boxRec->fieldNumber("isRegion")==-1,
	    "parameter 'box' has to be a region record. Invalid region record given"
	);
	TableRecord boxTblRec;
	boxTblRec.assign( *boxRec );
	
	//   3. call doConcatenation from in itsRegMgr object
	PtrHolder<ImageRegion> leConcatReg(
		_regMan->doConcatenation( *lesRegions, boxTblRec )
	);
	
	//   4. add comment to the record??
	casa::Record returnRec;
	returnRec.assign(leConcatReg->toRecord(""));
	if ( comment.length() > 1 ) {
	    returnRec.define("comment", comment );
	}
	else {
	    returnRec.define("comment", "Created with Region Mgr Concatenation Function"  );
	}
	return fromRecord(returnRec);
    } catch (const AipsError& x) {
	*_log << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
	RETHROW(x);
    }
}

bool
regionmanager::deletefromtable(const std::string& tablename, const std::string& regionname)
{
	setup();
    *_log << LogOrigin("regionmanager", __func__);

    try{ 
	return _regMan->removeRegionInTable(String(tablename), String(regionname));
    }
    catch (const AipsError& x) {
	*_log << LogIO::SEVERE << "Exception Reported: " << x.getMesg()
		<< LogIO::POST;
	RETHROW(x);
    }
}


// NOTE: The underlying classes support handling a list of
// (SDJ) regions.  We may wish to change this method to do
//       the same.
::casac::record*
regionmanager::difference(const ::casac::record& region1, const ::casac::record& region2, const std::string& comment)
{
	setup();
    *_log << LogOrigin("regionmanager", __func__);
    
    try {
	// Get the incoming records into TableRecords.
	PtrHolder<casa::Record> reg1(toRecord(region1));
	PtrHolder<casa::Record> reg2(toRecord(region2));

	TableRecord rec1;
	rec1.assign(*reg1);

	TableRecord rec2;
	rec2.assign(*reg2);
	*_log << LogIO::DEBUGGING
		<< "RegionManager val 1 " << rec1.asInt("isRegion") 
		<< "\nRegionManager val 2 " << rec2.asInt("isRegion") 
		<< LogIO::POST;
	
	// Now turn them into image regions
	PtrHolder<ImageRegion> imgReg1(ImageRegion::fromRecord(rec1,""));
	PtrHolder<ImageRegion> imgReg2(ImageRegion::fromRecord(rec2,""));
	
	ThrowIf (
		! imgReg1.ptr() || ! imgReg2.ptr(),
		"Unable to convert input to Image Regions"
	);
	PtrHolder<ImageRegion> diffReg(_regMan->doDifference(*imgReg1, *imgReg2));

	ThrowIf(
		! diffReg.ptr(),
		"An error has occured while creating the difference of the two regions"
	);

	casa::Record diffrec;
	diffrec.assign(diffReg->toRecord(""));
	if ( comment.length() > 1 ) {
		diffrec.define("comment", comment );
	}
	else {
		diffrec.define("comment", "Difference of between 2 regions created with the Region Manger tool"  );
	}
	return fromRecord(diffrec);


    } catch (const AipsError& x) {
	*_log << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
	RETHROW(x);
    }

}

bool
regionmanager::done()
{
	_regMan.set(0);
	_log.set(0);
    return true;
}

record*
regionmanager::fromtextfile(
	const string& filename, const vector<int>& shape,
	const record& csys
) {
    	setup();
    *_log << LogOrigin("regionmanager", __func__);
    try {
    	ThrowIf(
    		shape.size() == 1 && shape[0] == 0,
    		"Illegal shape. Please provide a legal image "
    		"shape consistent with the supplied coordinate system"
    	);
    	PtrHolder<CoordinateSystem> ncsys;
    	IPosition myShape(shape);
    	PtrHolder<Record> csysRec(toRecord(csys));

    	if((csysRec->nfields()) > 0){
    	    ThrowIf(
    	    	csysRec->nfields() < 2,
    	    	"Given coordsys parameter is not a valid coordsystem record"
    	    );
    	    ncsys.set((CoordinateSystem::restore(*csysRec, "")));
    	}
    	RegionTextList annList(
    		filename, ncsys.ptr() ? *ncsys : _regMan->getcoordsys(), myShape
    	);
		return fromRecord(annList.regionAsRecord());
    }
    catch (const AipsError& x) {
    	*_log << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
    	RETHROW(x);
    }
}

record*
regionmanager::fromtext(
	const string& text, const vector<int>& shape,
	const record& csys
) {

    	setup();
    *_log << LogOrigin("regionmanager", __func__);
    try {
    	CoordinateSystem coordsys;
    	IPosition myShape(shape);
    	PtrHolder<Record> csysRec(toRecord(csys));
    	if((csysRec->nfields()) != 0){
    	    ThrowIf(
    	    	csysRec->nfields() < 2,
    	    	"Given coordsys parameter is not a valid coordsystem record"
    	    );
    	    PtrHolder<CoordinateSystem> c(
    	    	CoordinateSystem::restore(*csysRec, "")
    	    );
    	    coordsys = CoordinateSystem(*c);
    	}
    	else {
    	    //user has set csys already
    	    coordsys = _regMan->getcoordsys();
    	}
    	RegionTextList annList(coordsys, text, myShape);
		return fromRecord(annList.regionAsRecord());
    }
    catch (const AipsError& x) {
    	*_log << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
    	RETHROW(x);
    }
}

::casac::record*
regionmanager::fromfiletorecord(
	const std::string& filename, const bool,
	const string& regionName
) {

	setup();
    *_log << LogOrigin("regionmanager", __func__);
    try {
    	PtrHolder<Record> leReg(RegionManager::readImageFile(filename, regionName));
    	return fromRecord(*leReg);
    }
    catch (const AipsError& x) {
    	*_log << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
    	RETHROW(x);
    }
}

bool 
regionmanager::tofile(const std::string& filename, const ::casac::record& region){
    setup();
    *_log << LogOrigin("regionmanager", __func__);
    try{
      PtrHolder<casa::Record> leRegion(toRecord(region));
      //the string lolo below does not matter its being ignored it seems.
      //may be it was meant for future use
      return RegionManager::writeImageFile(String(filename), "lolo", *leRegion);
    }
    catch (const AipsError& x) {
      *_log << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
      RETHROW(x);
    }
  }

std::string
regionmanager::fromrecordtotable(const std::string& tablename, 
				 const ::casac::variant& regionname, 
				 const ::casac::record& regionrec, const bool asmask, 
				 const bool)
{
	setup();
    *_log << LogOrigin("regionmanager", __func__);

    try {
	*_log << LogOrigin("regionmanager", "fromrecordtotable");
	PtrHolder<casa::Record> leRec(toRecord(regionrec));
    
	String elname=toCasaString(regionname);
	return (
		_regMan->recordToTable(String(tablename), *leRec, elname, asmask)
	).c_str();

    } catch (const AipsError& x) {
	*_log << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
	RETHROW(x);
    }
}

::casac::record*
regionmanager::fromtabletorecord(
	const std::string& tablename, const ::casac::variant& regionname, const bool
) {
	setup();
    *_log << LogOrigin("regionmanager", __func__);
  try {
    String elname=toCasaString(regionname);
    
    PtrHolder<casa::Record> leRec(_regMan->tableToRecord(String(tablename), elname));
    if(! leRec.ptr()) {
    	leRec.set(new Record());
    }
    return fromRecord(*leRec);
  }
  catch (const AipsError& x) {
    *_log << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
    RETHROW(x);
  }
}

::casac::record*
regionmanager::intersection(const ::casac::variant& regions, const std::string& comment)
{
	setup();
    *_log << LogOrigin("regionmanager", __func__);
    
    try {
	ThrowIf(
		regions.type() != ::casac::variant::RECORD,
		"parameter 'regions' has to be a dictionary containing regions"
	);

	    ::casac::variant localvar(regions); //cause its const
	    PtrHolder<casa::Record> lesRegions(toRecord((localvar.asRecord())));
	    PtrHolder<ImageRegion> intersectReg;
	    ThrowIf(
	    	lesRegions->nfields() < 2,
	    	"need 2 or more regions to make an intersection"
	    );

	    TableRecord rec1;
	    TableRecord rec2;
	    rec1.assign(lesRegions->asRecord(casa::RecordFieldId(0)));
	    rec2.assign(lesRegions->asRecord(casa::RecordFieldId(1)));
	    *_log << LogIO::DEBUGGING
		    << "RegionManager val 1 " << rec1.asInt("isRegion") 
		    << "\nRegionManager val 2 " << rec2.asInt("isRegion") 
		    << LogIO::POST;
            
	    PtrHolder<ImageRegion> reg0(ImageRegion::fromRecord(rec1,""));
	    PtrHolder<ImageRegion> reg1(ImageRegion::fromRecord(rec2,""));

	    if(reg0.ptr() && reg1.ptr()) {
	    	intersectReg.set(_regMan->doIntersection(*reg0, *reg1));
	    }
	    for (uInt k=2; k < (lesRegions->nfields()); ++k){
		rec1.assign(lesRegions->asRecord(casa::RecordFieldId(k)));

		reg0.set(ImageRegion::fromRecord(rec1, ""));
		ImageRegion reg3(*intersectReg);
		intersectReg.set(_regMan->doIntersection(*reg0, reg3));
	    }

	    casa::Record intersectrec;
	    intersectrec.assign(intersectReg->toRecord(""));
	    if ( comment.length() > 1 ) {
	    	intersectrec.define("comment", comment );
	    }
	    else {
	    	intersectrec.define(
	    		"comment", "Intersction of multiple regions created with the Region Manger tool"
	    	);
	    }
	    return fromRecord(intersectrec);
    }
    catch (const AipsError& x) {
    	*_log << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
    	RETHROW(x);
    }
}

bool
regionmanager::ispixelregion(const ::casac::record& region)
{
	setup();
    *_log << LogOrigin("regionmanager", __func__);
    try {
    	PtrHolder<casa::Record> localvar(toRecord(region));
    	TableRecord letblrec;
    	letblrec.assign(*localvar);
    	PtrHolder<ImageRegion> reg(ImageRegion::fromRecord(letblrec, ""));
    	return RegionManager::isPixelRegion(*reg);
    }
    catch (const AipsError& x) {
	*_log << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
	RETHROW(x);
    }
    
}

bool
regionmanager::isworldregion(const ::casac::record& region)
{
	setup();
    *_log << LogOrigin("regionmanager", __func__);


    try {
	PtrHolder<casa::Record> localvar(toRecord(region));
	TableRecord letblrec;
	letblrec.assign(*localvar);
	PtrHolder<ImageRegion> reg(ImageRegion::fromRecord(letblrec, ""));
	return RegionManager::isWorldRegion(*reg);
    } catch (AipsError x) {
	*_log << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
	RETHROW(x);
    }
}

std::vector<std::string>
regionmanager::namesintable(const std::string& tablename)
{
	setup();
    *_log << LogOrigin("regionmanager", __func__);

    std::vector<std::string> retval(1,"");
    try {
	Vector<String>names=_regMan->namesInTable(String(tablename));
	return fromVectorString(names);

    } catch (AipsError x) {
	*_log << LogIO::SEVERE << "Exception Reported: "
		<< x.getMesg() << LogIO::POST;
	RETHROW(x);
    }  
}

bool
regionmanager::setcoordinates(const ::casac::record& csys)
{
	setup();
    *_log << LogOrigin("regionmanager", __func__);

    try {
	PtrHolder<casa::Record> csysRec(toRecord(csys));
	ThrowIf(
		csysRec->nfields() <2,
		"Given coorsys parameter does not appear to be a valid coordsystem record"
	);
	PtrHolder<casa::CoordinateSystem> coordsys(casa::CoordinateSystem::restore(*csysRec, ""));
	ThrowIf(
		! coordsys.ptr(),
		"Could not convert given record to a coordsys"
	);

	_regMan->setcoordsys(*coordsys);
	return true;

    } catch (const AipsError& x) {
	*_log << LogIO::SEVERE << "Exception Reported: "
		<< x.getMesg() << LogIO::POST;
	RETHROW(x);
    }
}

::casac::record*
regionmanager::makeunion(const ::casac::variant& regions, const std::string& comment)
{
	setup();
	*_log << LogOrigin("regionmanager", __func__);
    try {
    	ThrowIf(
    		regions.type() != ::casac::variant::RECORD,
    		"parameter 'regions' has to be a dictionary containing regions"
    	);

    	::casac::variant localvar(regions); //cause its const
    	PtrHolder<casa::Record> lesRegions(toRecord((localvar.asRecord())));
    	PtrHolder<ImageRegion> unionReg(dounion(lesRegions));

    	casa::Record unionrec;
    	unionrec.assign(unionReg->toRecord(""));
    	if ( comment.length() > 1 ) {
    		unionrec.define("comment", comment );
    	}
    	else {
    		unionrec.define("comment", "Created with the region manager's union function." );
    	}
    	return fromRecord(unionrec);
    } catch (const AipsError& x) {
	*_log << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
	RETHROW(x);
    }
}

vector<int> regionmanager::selectedchannels(
	const string& specification, const vector<int>& shape
) {
	*_log << LogOrigin("regionmanager", __func__);
	setup();
	try {
		const CoordinateSystem& csys = _regMan->getcoordsys();

		ThrowIf(
			! csys.hasSpectralAxis(), "Associated coordinate system has no spectral axis"
		);
		uInt nChannels = shape[csys.spectralAxisNumber()];
		uInt nSelectedChannels;
		vector<uInt> ranges = _regMan->setSpectralRanges(
			specification, nSelectedChannels,
			IPosition(shape)
		);
		vector<Int> chans(0);
		vector<uInt>::const_iterator end = ranges.end();
		vector<uInt>::const_iterator iter = ranges.begin();

		while (iter!=end && *iter < nChannels) {
			uInt rBegin = *iter;
			iter++;
			uInt rEnd = *iter;
			iter++;
			for (uInt i=rBegin; i<=rEnd; i++) {
				if (i >= nChannels) {
					break;
				}
				chans.push_back(i);
			}
		}
		AlwaysAssert(chans.size() <= nSelectedChannels, AipsError);
		return chans;
	}
	catch (const AipsError& x) {
		*_log << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
		RETHROW(x);
	}
}

// Implementation courtesy of Honglin (https://bugs.aoc.nrao.edu/browse/CAS-1666, 2009dec16)
casa::ImageRegion* regionmanager::dounion(const PtrHolder<casa::Record>& regions) {
    ThrowIf(
    	regions->nfields() < 2 || regions->fieldNumber("isRegion") != -1,
    	"need 2 or more regions to make a union"
    );

    PtrBlock<const ImageRegion*> unionRegions;
    uInt nreg = regions->nfields();
    unionRegions.resize(nreg);
    for (uInt i = 0; i < nreg; i++) {
        TableRecord trec;
        trec.assign(regions->asRecord(casa::RecordFieldId(i)));
        unionRegions[i] = ImageRegion::fromRecord(trec, "");
    }
    WCUnion leUnion(unionRegions);
    SPtrHolder<ImageRegion> retval(new ImageRegion(leUnion));
    for (uInt i = 0; i < nreg; i++) {
    	delete unionRegions[i];
    }

    return retval.transfer();
}

::casac::record*
regionmanager::wbox(const ::casac::variant& blc, const ::casac::variant& trc, const std::vector<int>& pixelaxes, const ::casac::record& csys, const std::string& absrel, const std::string& comment)
{
	setup();
    *_log << LogOrigin("regionmanager", __func__);
    try {
      casa::Vector<casa::String> losBlc;
	casa::Vector<casa::String> losTrc;
	PtrHolder<casa::Record> leRegion;

	if(blc.type() == ::casac::variant::STRING ){
	  sepCommaEmptyToVectorStrings(losBlc, blc.toString());
	}
	else if(blc.type() == ::casac::variant::STRINGVEC){
	  losBlc=toVectorString(blc.toStringVec());
     	}
	else{
	    ThrowCc("blc has to be a string or vector of strings");
      
	}
	if(trc.type() == ::casac::variant::STRING ){
	  sepCommaEmptyToVectorStrings(losTrc, trc.toString());
	} 
	else if(	trc.type() == ::casac::variant::STRINGVEC){
	  losTrc=toVectorString(trc.toStringVec());
	}
	else{
	    ThrowCc("trc has to be a string or vector of strings");
	}
 
	Vector<Int>pixaxes(pixelaxes);
	PtrHolder<casa::Record> csysRec(toRecord(csys));
	if((csysRec->nfields()) != 0){ 
	    PtrHolder<casa::Record> csysRec(toRecord(csys));
	    ThrowIf(
	    	csysRec->nfields() <2,
	    	"Given coordsys parameter does not appear to be a valid coordsystem record"
	    );
	    PtrHolder<casa::CoordinateSystem> coordsys(casa::CoordinateSystem::restore(*csysRec, ""));
	    leRegion.set(_regMan->wbox(losBlc, losTrc,  pixaxes, *coordsys, String(absrel), String(comment)));
	} else {
	    //user has set csys already
	    leRegion.set(
	    	_regMan->wbox(
	    		losBlc, losTrc, pixaxes, String(absrel), String(comment)
	    	)
	    );
	}

	return fromRecord(*leRegion);
    } catch (const AipsError& x) {
	*_log << LogIO::SEVERE << "Exception Reported: "
		<< x.getMesg() << LogIO::POST;
	RETHROW(x);
    }
}

::casac::record*
regionmanager::wpolygon(const ::casac::variant& x, const ::casac::variant& y, 
			const std::vector<int>& pixelaxes, 
			const ::casac::record& csys, 
			const std::string& absrel, const std::string& comment)
{
	setup();
    *_log << LogOrigin("regionmanager", __func__);

    try {
	casa::Vector<casa::Quantity> losX;
	casa::Vector<casa::Quantity> losY;
	if(x.type() == ::casac::variant::STRING || 
		x.type() == ::casac::variant::STRINGVEC){
	    toCasaVectorQuantity(x, losX);
	}
	else{
	    ThrowCc("x has to be a string or vector of strings");
      
	}
	if(y.type() == ::casac::variant::STRING || 
		y.type() == ::casac::variant::STRINGVEC){
	    toCasaVectorQuantity(y, losY);
	}
	else{
	    throw(AipsError("trc has to be a string or vector of strings")); 
	}

	PtrHolder<ImageRegion> leRegion;

	Vector<Int>pixaxes(pixelaxes);
	PtrHolder<casa::Record> csysRec(toRecord(csys));
	if((csysRec->nfields()) != 0){ 
	    PtrHolder<casa::Record> csysRec(toRecord(csys));
	    ThrowIf(
	    	csysRec->nfields() <2,
	    	"Given coorsys parameter does not appear to be a valid coordsystem record"
	    );
	    PtrHolder<casa::CoordinateSystem> coordsys(casa::CoordinateSystem::restore(*csysRec, ""));
	    leRegion.set(_regMan->wpolygon(losX, losY,  pixaxes, *coordsys,
				   String(absrel)));
	}
	else{
	    //user has set csys already
	    leRegion.set(_regMan->wpolygon(losX, losY, pixaxes, String(absrel)));
	}

	if(leRegion.ptr()){
      
	    PtrHolder<casa::Record> leRec(new casa::Record());
	    leRec->assign(leRegion->toRecord(String("")));
	    leRec->define("comment", comment);
	    return fromRecord(*leRec);
	}
	return 0;
    } catch (const AipsError& x) {
	*_log << LogIO::SEVERE << "Exception Reported: "
		<< x.getMesg() << LogIO::POST;
	RETHROW(x);
    }

}

record* regionmanager::frombcs(
	const ::casac::record& csys, const vector<int>& shape,
	const string& box, const string& chans,
	const string& stokes, const string& stokescontrol,
	const variant& region
) {
	setup();
    *_log << LogOrigin("regionmanager", __func__);

    try {
    	String regionString;
    	PtrHolder<Record> regionPtr;
    	if(region.type() == variant::STRING) {
    		regionString = region.toString();
    	}
    	else if (region.type() == variant::RECORD) {
    		PtrHolder<variant> clone(region.clone());
    		regionPtr.set(toRecord(clone->asRecord()));
    	}
    	else if (region.type() == variant::BOOLVEC) {
    		// default type for variants apparently, nothing to do
    	}
    	else {
    		ThrowCc(
    			"Unrecognized type " + region.typeString()
    			+ " for region, must be either string or region dictionary"
    		);
    	}
    	String myControl = stokescontrol;
    	CasacRegionManager::StokesControl sControl;
    	myControl.upcase();
    	if (myControl.startsWith("A")) {
    		sControl = CasacRegionManager::USE_ALL_STOKES;
    	}
    	else if (myControl.startsWith("F")) {
    		sControl = CasacRegionManager::USE_FIRST_STOKES;
    	}
    	else {
    		ThrowCc("Unsupported value for stokescontrol: " + stokescontrol);
    	}
    	if (! csys.empty()) {
    		PtrHolder<Record> csysRec(toRecord(csys));
    		ThrowIf(
    			csysRec->nfields() < 2,
    			"Given coordsys parameter does not appear "
    			"to be a valid coordsystem record"
    		);
    		PtrHolder<CoordinateSystem> coordsys(
    			CoordinateSystem::restore(*csysRec, "")
    		);
    		ThrowIf(
    			! coordsys.ptr(),
    			"Could not convert given csys record "
    			"to a CoordinateSystem object"
    		);
    		_regMan->setcoordsys(*coordsys);
    	}

    	String diagnostics;
    	uInt nSelectedChannels;
    	IPosition imShape(shape);
    	String myStokes(stokes);
    	String myChans(chans);
    	String myBox(box);
    	return fromRecord(
    		_regMan->fromBCS(
    			diagnostics, nSelectedChannels, myStokes,
    			regionPtr.ptr(), regionString, myChans, sControl, myBox,
    			imShape
    		)
    	);
    }
    catch (const AipsError& x) {
    	*_log << LogIO::SEVERE << "Exception Reported: "
    			<< x.getMesg() << LogIO::POST;
    	RETHROW(x);
    }
}


} // casac namespace

