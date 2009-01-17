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
#include <xmlcasa/images/regionmanager_cmpt.h>
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
#include <images/Images/ImageRegion.h>
#include <images/Images/RegionManager.h>
#include <tables/Tables/TableRecord.h>
#include <casa/namespace.h>
using namespace std;

namespace casac {

regionmanager::regionmanager():
    itsLog(0),
    itsRegMan(0)
{
    try {
	itsLog=NULL;
	itsRegMan=NULL;
	itsIsSetup=False;
	setup();
    } catch (AipsError x) {
	if ( itsLog != NULL )
	    *itsLog << LogIO::SEVERE << "Exception Reported: " 
		    << x.getMesg() << LogIO::POST;
	RETHROW(x);
    }
}

regionmanager::~regionmanager()
{
    // Call done to free memory first.
    done();
}

void 
regionmanager::setup()
{
    if ( itsRegMan == NULL )
	itsRegMan = new casa::RegionManager();

    if ( itsLog == NULL )
	itsLog = new LogIO();

    itsIsSetup=True;
}

std::string
regionmanager::absreltype(const int absrelvalue)
{
    if ( !itsIsSetup )
	setup();

    std::string retval="";
    *itsLog << LogOrigin("regionmanager", "absrelvalue");

    try {
	retval=itsRegMan->absreltype(absrelvalue);
    } catch (AipsError x) {
	*itsLog << LogIO::SEVERE << "Exception Reported: " 
		<< x.getMesg() << LogIO::POST;
	RETHROW(x);
    }
    return retval;
}

::casac::record*
regionmanager::box(const std::vector<double>& blc, const std::vector<double>& trc, const std::vector<double>& inc, const std::string& absrel, const bool frac, const std::string& comment)
{
    if ( !itsIsSetup )
	setup();
    
    ::casac::record* retval=0;
    *itsLog << LogOrigin("regionmanager", "box");

    try {
	Vector<Double> dblc(blc);
	Vector<Double> dtrc(trc);
	Vector<Double> dinc(inc);
	if(inc.size()==1 && inc[0]==1)
	{
	    dinc.resize(blc.size());
	    dinc.set(1.0);
	}

	if(dtrc.nelements()==1 && dtrc[0]<0)
	{
	    dtrc.resize();
	}
	Record* lebox=
	    itsRegMan->box(dblc, dtrc, dinc, String(absrel), frac, String(comment));
    
	retval=fromRecord(*lebox);
	if (lebox !=0){
	    delete lebox;
	}
    } catch (AipsError x) {
	*itsLog << LogIO::SEVERE << "Exception Reported: " 
		<< x.getMesg() << LogIO::POST;
	RETHROW(x);
    }
  
    return retval;
}

// Compute the complement of a region(s)
// 
// For a set of regions the complement taken on the union
// of the regions.
::casac::record*
regionmanager::complement(const ::casac::variant& regions, const std::string& comment)
{
    if ( !itsIsSetup )
	setup();

    ::casac::record* retval=0;
    *itsLog << LogOrigin("regionmanager", "complement");
    
    // If the region parameter is not a record then throw an exception
    if(regions.type() != ::casac::variant::RECORD)
	throw(AipsError("parameter 'regions' has to be a dictionary containing regions")); 

    try {
	// Create a single region to find the complement of.
	::casac::variant localvar(regions); //cause its const
	casa::ImageRegion* unionReg=0;
	casa::Record* lesRegions=toRecord((localvar.asRecord()));

	// If the "isRegion" field exists then we assume the record
	// is a region, otherwise we have a set of regions.
	if(lesRegions->fieldNumber("isRegion")==-1 && lesRegions->nfields() > 1 )
	    unionReg=dounion(lesRegions);
	else {
	    if ( lesRegions->fieldNumber("isRegion")==-1 ) {
		TableRecord theRec;
		theRec.assign( lesRegions->asRecord(casa::RecordFieldId(0) ) );
		unionReg=ImageRegion::fromRecord( theRec, "" );
	   } else
		unionReg=ImageRegion::fromRecord( *lesRegions, "" );
	}
	
	// And find the complement
	casa::ImageRegion* leComplementReg=0;
	if( unionReg != 0 ){
	    leComplementReg=itsRegMan->doComplement(*unionReg);
	    delete unionReg;
	}
	
	casa::Record returnRec;
	returnRec.assign(leComplementReg->toRecord(""));
	if ( comment.length() > 1 )
	    returnRec.define("comment", comment );
	else
	    returnRec.define("comment", "Complement region created with the Region Manger tool"  );
	retval=fromRecord(returnRec); 
	if(leComplementReg !=0)
	    delete leComplementReg;

	return retval;
    } catch (AipsError x) {
	*itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
	RETHROW(x);
    }
}
    

::casac::record*
regionmanager::concatenation(const ::casac::variant& box, const ::casac::variant& regions, const std::string& comment)
{
    if ( !itsIsSetup )
	setup();

    ::casac::record* retval=0;
    *itsLog << LogOrigin("regionmanager", "concatenation");
    
    // If the region parameter is not a record then throw an exception
    if(box.type() != ::casac::variant::RECORD )
	throw(AipsError("parameter 'box' has to be a dictionary containing a region record")); 

    if(regions.type() != ::casac::variant::RECORD )
	throw(AipsError("parameter 'regions' has to be a dictionary containing one or more region records")); 


    try {
	// ALgorihtm
	//   1. convert incoming image record to a CASA::Record(s)
	::casac::variant localregs(regions); //cause its const
	casa::Record* lesRegions=toRecord((localregs.asRecord()));
	
	//   2. convert incoming box record to a TableRecord
	::casac::variant localbox(box); //cause its const
	casa::Record* boxRec=toRecord((localbox.asRecord()));
	if ( boxRec->fieldNumber("isRegion")==-1)
	    throw(AipsError("parameter 'box' has to be a region record. Invalid region record given"));
	TableRecord boxTblRec;
	boxTblRec.assign( *boxRec );
	
	//   3. call doConcatenation from in itsRegMgr object
	ImageRegion* leConcatReg = 0;
	leConcatReg = itsRegMan->doConcatenation( *lesRegions, boxTblRec );
	
	//   4. add comment to the record??
	casa::Record returnRec;
	returnRec.assign(leConcatReg->toRecord(""));
	if ( comment.length() > 1 )
	    returnRec.define("comment", comment );
	else
	    returnRec.define("comment", "Created with Region Mgr Concatenation Function"  );
	retval=fromRecord(returnRec); 

	// Cleanup
	if(leConcatReg !=0)
	    delete leConcatReg;

	return retval;
    } catch (AipsError x) {
	*itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
	RETHROW(x);
    }
}
    


bool
regionmanager::copyregions(const std::string& tableout, const std::string& tablein, const std::string& regionname)
{
    if ( !itsIsSetup )
	setup();
    *itsLog << LogOrigin("regionmanager", "copyregions");

    // TODO : IMPLEMENT ME HERE !
    *itsLog << LogIO::SEVERE 
	    << "Sorry! This feature is not implemented yet."
	    << LogIO::EXCEPTION;
    return false;
}
    

int
regionmanager::dflt()
{
    if ( !itsIsSetup )
	setup();
    *itsLog << LogOrigin("regionmanager", "dflt");
    int retval=false;

    // TODO : IMPLEMENT ME HERE !
    *itsLog << LogIO::SEVERE 
	    << "Sorry! This feature is not implemented yet."
	    << LogIO::EXCEPTION;
  return retval;
}

bool
regionmanager::deletefromtable(const std::string& tablename, const std::string& regionname)
{
    if ( !itsIsSetup )
	setup();
    
    bool retval=False;
    *itsLog << LogOrigin("regionmanager", "deletefromtable");

    try{ 
	retval=itsRegMan->removeRegionInTable(String(tablename), String(regionname));
    }catch (AipsError x) {
	*itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() 
		<< LogIO::POST;
	RETHROW(x);
    }

    return retval;
}


// NOTE: The underlying classes support handling a list of
// (SDJ) regions.  We may wish to change this method to do
//       the same.
::casac::record*
regionmanager::difference(const ::casac::record& region1, const ::casac::record& region2, const std::string& comment)
{
    if ( !itsIsSetup )
	setup();

    casac::record* retval=0;
    *itsLog << LogOrigin("regionmanager", "difference");
    
    try {
	// Get the incoming records into TableRecords.
	casa::Record *reg1 = toRecord(region1);
	casa::Record *reg2 = toRecord(region2);	

	TableRecord rec1;
	rec1.assign(*reg1);

	TableRecord rec2;
	rec2.assign(*reg2);
	*itsLog << LogIO::DEBUGGING
		<< "RegionManager val 1 " << rec1.asInt("isRegion") 
		<< "\nRegionManager val 2 " << rec2.asInt("isRegion") 
		<< LogIO::POST;
	
	// Now turn them into image regions
	ImageRegion *imgReg1=0;
	ImageRegion *imgReg2=0;
	imgReg1=ImageRegion::fromRecord(rec1,"");
	imgReg2=ImageRegion::fromRecord(rec2,"");
	
	ImageRegion *diffReg=0;
	if(imgReg1!=0 && imgReg2!=0)
	    diffReg=itsRegMan->doDifference(*imgReg1, *imgReg2);
	else
	    throw(AipsError("Unable to convert input to Image Regions") );

	if ( diffReg != 0 ) {
	    casa::Record diffrec;
	    diffrec.assign(diffReg->toRecord(""));
	    if ( comment.length() > 1 )
		diffrec.define("comment", comment );
	    else
		diffrec.define("comment", "Difference of between 2 regions created with the Region Manger tool"  );

	    retval=fromRecord(diffrec); 
	    if(diffReg !=0)
		delete diffReg;
	} else 
	    throw(AipsError("An error has occured while creating the difference of the two regions" ) );

    } catch (AipsError x) {
	*itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
	RETHROW(x);
    }

    return retval;
}


bool
regionmanager::done()
{
    if ( itsRegMan != NULL )
    {
	delete itsRegMan;
	itsRegMan = NULL;
    }
    
    // The log must be deleted 2nd, because the region
    // manager uses the same log.
    if ( itsLog != NULL )
    {
	delete itsLog;
	itsLog = NULL;
    }
    
    itsIsSetup=False;
    return true;
}

bool
regionmanager::extension(const ::casac::record& box, const ::casac::record& region, const std::string& comment)
{
    if ( !itsIsSetup )
	setup();
    *itsLog << LogOrigin("regionmanager", "extension");

    
    // TODO : IMPLEMENT ME HERE !
    *itsLog << LogIO::SEVERE 
	    << "Sorry! This feature is not implemented yet."
	    << LogIO::EXCEPTION;
    return false;
}

::casac::record*
regionmanager::extractsimpleregions(const ::casac::record& region)
{
    if ( !itsIsSetup )
	setup();
    *itsLog << LogOrigin("regionmanager", "extractsimpleregions");

    // TODO : IMPLEMENT ME HERE !
    *itsLog << LogIO::SEVERE 
	    << "Sorry! This feature is not implemented yet."
	    << LogIO::EXCEPTION;
    return false;
}

::casac::record*
regionmanager::fromfiletorecord(const std::string& filename, 
	const bool verbose, 
	const std::string& regionname)
{
    if ( !itsIsSetup )
	setup();
    *itsLog << LogOrigin("regionmanager", "fromfiletorecord");

    Record * leReg =
	itsRegMan->readImageFile( String(filename), String(regionname ) );
    return fromRecord( *leReg );
}

    
bool
regionmanager::fromglobaltotable(const std::string& tablename, const bool confirm, const bool verbose, const ::casac::variant& regionname, const ::casac::record& regions)
{
    if ( !itsIsSetup )
	setup();
    *itsLog << LogOrigin("regionmanager", "fromglobaltotable");

    // TODO : IMPLEMENT ME HERE !
    *itsLog << LogIO::SEVERE 
	    << "Sorry! This feature is not implemented yet."
	    << LogIO::EXCEPTION;
    return false;
}

std::string
regionmanager::fromrecordtotable(const std::string& tablename, 
				 const ::casac::variant& regionname, 
				 const ::casac::record& regionrec, 
				 const bool verbose)
{
    if ( !itsIsSetup )
	setup();
    *itsLog << LogOrigin("regionmanager", "fromrecordtotable");

    std::string retval="";
    try {
	*itsLog << LogOrigin("regionmanager", "fromrecordtotable");
	casa::Record *leRec=toRecord(regionrec);
    
	String elname=toCasaString(regionname);
	retval=
	    (itsRegMan->recordToTable(String(tablename), *leRec, elname)).c_str();
	if(leRec)
	    delete leRec;

    } catch (AipsError x) {
	*itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
	RETHROW(x);
    }

    return retval;
}

::casac::record*
regionmanager::fromtabletorecord(const std::string& tablename, const ::casac::variant& regionname, const bool verbose)
{
    if ( !itsIsSetup )
	setup();
    *itsLog << LogOrigin("regionmanager", "fromtabletorecord");

  
  ::casac::record* retval=0;
  try {
    String elname=toCasaString(regionname);
    
    casa::Record *leRec;
    leRec=itsRegMan->tableToRecord(String(tablename), elname);
    if(leRec==0)
      leRec=new Record();
    retval=fromRecord(*leRec);
    if(leRec)
      delete leRec;
    
  }catch (AipsError x) {
    *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
    RETHROW(x);
  }

    return retval;
}

::casac::record*
regionmanager::intersection(const ::casac::variant& regions, const std::string& comment)
{
    if ( !itsIsSetup )
	setup();
    casac::record* retval=0;
    *itsLog << LogOrigin("regionmanager", "intersection");
    
    try {
	if(regions.type() == ::casac::variant::RECORD)
	{
	    ::casac::variant localvar(regions); //cause its const
	    casa::Record* lesRegions=toRecord((localvar.asRecord()));
	    ImageRegion* intersectReg=0;
	    if(lesRegions->nfields()<2){
		throw(AipsError("need 2 or more regions to make an intersection")); 
	    }
	    
	    ImageRegion *reg0=0;
	    ImageRegion *reg1=0;
	    TableRecord rec1;
	    TableRecord rec2;
	    rec1.assign(lesRegions->asRecord(casa::RecordFieldId(0)));
	    rec2.assign(lesRegions->asRecord(casa::RecordFieldId(1)));
	    *itsLog << LogIO::DEBUGGING
		    << "RegionManager val 1 " << rec1.asInt("isRegion") 
		    << "\nRegionManager val 2 " << rec2.asInt("isRegion") 
		    << LogIO::POST;
            
	    reg0=ImageRegion::fromRecord(rec1,"");
	    reg1=ImageRegion::fromRecord(rec2,"");

	    if(reg0!=0 && reg1!=0)
		intersectReg=itsRegMan->doIntersection(*reg0, *reg1);
	    if(reg1 !=0)
		delete reg1;
	    for (uInt k=2; k < (lesRegions->nfields()); ++k){
		rec1.assign(lesRegions->asRecord(casa::RecordFieldId(k)));
		if(reg0 !=0)
		    delete reg0;
		reg0=ImageRegion::fromRecord(rec1, "");
		ImageRegion reg3(*intersectReg);
		if(intersectReg !=0)
		    delete intersectReg;
		intersectReg=itsRegMan->doIntersection(*reg0, reg3);
	    }

	    casa::Record intersectrec;
	    intersectrec.assign(intersectReg->toRecord(""));
	    if ( comment.length() > 1 )
		intersectrec.define("comment", comment );
	    else
		intersectrec.define("comment", "Intersction of multiple regions created with the Region Manger tool"  );

	    retval=fromRecord(intersectrec); 
	    if(intersectReg !=0)
		delete intersectReg;
	} else {
	    throw(AipsError("parameter 'regions' has to be a dictionary containing regions")); 
	}

    } catch (AipsError x) {
	*itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
	RETHROW(x);
    }

    return retval;
}

bool
regionmanager::ispixelregion(const ::casac::record& region)
{
    if ( !itsIsSetup )
	setup();
    *itsLog << LogOrigin("regionmanager", "ispixelregion");

    bool retval=False;

    try {
	casa::Record *localvar=toRecord(region); 
	TableRecord letblrec;
	letblrec.assign(*localvar);
	ImageRegion* reg=0;
	reg=ImageRegion::fromRecord(letblrec, "");
	retval=itsRegMan->isPixelRegion(*reg);
    } catch (AipsError x) {
	*itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
	RETHROW(x);
    }
    
    return retval;
}

bool
regionmanager::isworldregion(const ::casac::record& region)
{
    if ( !itsIsSetup )
	setup();
    *itsLog << LogOrigin("regionmanager", "isworldregion");

    bool retval=False;

    try {
	casa::Record *localvar=toRecord(region); 
	TableRecord letblrec;
	letblrec.assign(*localvar);
	ImageRegion* reg=0;
	reg=ImageRegion::fromRecord(letblrec, "");
	retval=itsRegMan->isWorldRegion(*reg);
    } catch (AipsError x) {
	*itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
	RETHROW(x);
    }
    
    return retval;
}

std::vector<std::string>
regionmanager::namesintable(const std::string& tablename)
{
    if ( !itsIsSetup )
	setup();
    *itsLog << LogOrigin("regionmanager", "namesintable");

    std::vector<std::string> retval(1,"");
    try {
	*itsLog << LogOrigin("regionmanager", "namesintable");
	Vector<String>names=itsRegMan->namesInTable(String(tablename));
	retval=fromVectorString(names);

    } catch (AipsError x) {
	*itsLog << LogIO::SEVERE << "Exception Reported: " 
		<< x.getMesg() << LogIO::POST;
	RETHROW(x);
    }  
    return retval;
}

::casac::record*
regionmanager::pixeltoworldregion(const ::casac::record& csys, const std::vector<int>& shape, const ::casac::record& region)
{
    if ( !itsIsSetup )
	setup();
    *itsLog << LogOrigin("regionmanager", "pixeltoworldregion");

    // TODO : IMPLEMENT ME HERE !
    *itsLog << LogIO::SEVERE 
	    << "Sorry! This feature is not implemented yet."
	    << LogIO::EXCEPTION;
    return false;
}

::casac::record*
regionmanager::quarter(const std::string& comment)
{
    if ( !itsIsSetup )
	setup();
    *itsLog << LogOrigin("regionmanager", "quarter");


    // TODO : IMPLEMENT ME HERE !
    *itsLog << LogIO::SEVERE 
	    << "Sorry! This feature is not implemented yet."
	    << LogIO::EXCEPTION;
    return false;
}

bool
regionmanager::setcoordinates(const ::casac::record& csys)
{
    if ( !itsIsSetup )
	setup();
    *itsLog << LogOrigin("regionmanager", "setcoordinates");

    bool retval=false;
    try {
	casa::Record *csysRec=toRecord(csys);
	if(csysRec->nfields() <2){

	    throw(AipsError("Given coorsys parameter does not appear to be a valid coordsystem record"));
	}
	casa::CoordinateSystem *coordsys=casa::CoordinateSystem::restore(*csysRec, "");
	if(coordsys==0){
	    retval=false;
	    throw(AipsError("Could not convert given record to a coordsys")); 
	}
	itsRegMan->setcoordsys(*coordsys);
	retval=true;


    } catch (AipsError x) {
	*itsLog << LogIO::SEVERE << "Exception Reported: " 
		<< x.getMesg() << LogIO::POST;
	RETHROW(x);
    }

    return retval;
}

::casac::record*
regionmanager::makeunion(const ::casac::variant& regions, const std::string& comment)
{
    if ( !itsIsSetup )
	setup();
    casac::record* retval=0;
    *itsLog << LogOrigin("regionmanager", "makeunion");
  
    if(regions.type() != ::casac::variant::RECORD)
	throw(AipsError("parameter 'regions' has to be a dictionary containing regions")); 
	
    try {
	::casac::variant localvar(regions); //cause its const
	casa::Record* lesRegions=toRecord((localvar.asRecord()));
	ImageRegion* unionReg=0;
	unionReg=dounion(lesRegions);

	casa::Record unionrec;
	unionrec.assign(unionReg->toRecord(""));
	if ( comment.length() > 1 )
	    unionrec.define("comment", comment );
	else
	    unionrec.define("comment", "Created with the region manager's union function." );	
	retval=fromRecord(unionrec); 
	if(unionReg !=0)
	    delete unionReg;
    } catch (AipsError x) {
	*itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
	RETHROW(x);
    }

    return retval;
}

casa::ImageRegion*
regionmanager::dounion(casa::Record*& regions){
    casa::ImageRegion* retval=0;
    
    // If the "isRegion" field is defined then what we have is
    // a single region.  If there are less then 2 regions, then
    // we have nothing to union.  Either way we throw an exeception
    // and let the user know why.
    if(regions->nfields()<2 or regions->fieldNumber("isRegion")!=-1)
	throw(AipsError("need 2 or more regions to make a union")); 

    try {
	ImageRegion *reg0=0;
	ImageRegion *reg1=0;
	TableRecord rec1;
	TableRecord rec2;
	rec1.assign(regions->asRecord(casa::RecordFieldId(0)));
	rec2.assign(regions->asRecord(casa::RecordFieldId(1)));
	*itsLog << LogIO::DEBUGGING
		<< "RegionManager val 1 " << rec1.asInt("isRegion") 
		<< "\nRegionManager val 2 " << rec2.asInt("isRegion") 
		<< LogIO::POST;
            
	reg0=ImageRegion::fromRecord(rec1,"");
	reg1=ImageRegion::fromRecord(rec2,"");

	if(reg0!=0 && reg1!=0)
	    retval=itsRegMan->doUnion(*reg0, *reg1);
	if(reg1 !=0)
	    delete reg1;
	for (uInt k=2; k < (regions->nfields()); ++k){
	    rec1.assign(regions->asRecord(casa::RecordFieldId(k)));
	    if(reg0 !=0)
		delete reg0;
	    reg0=ImageRegion::fromRecord(rec1, "");
	    ImageRegion reg3(*retval);
	    if(retval !=0)
		delete retval;
	    retval=itsRegMan->doUnion(*reg0, reg3);
	}
	
    } catch (AipsError x) {
	*itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
	RETHROW(x);
    }

    return retval;
}

::casac::record*
regionmanager::wbox(const ::casac::variant& blc, const ::casac::variant& trc, const std::vector<int>& pixelaxes, const ::casac::record& csys, const std::string& absrel, const std::string& comment)
{
    if ( !itsIsSetup )
	setup();
    *itsLog << LogOrigin("regionmanager", "wbox");
    
    casac::record* retval=0;

    try {
	casa::Vector<casa::Quantity> losBlc;
	casa::Vector<casa::Quantity> losTrc;
	casa::Record * leRegion;

	if(blc.type() == ::casac::variant::STRING || 
		blc.type() == ::casac::variant::STRINGVEC){
	    toCasaVectorQuantity(blc, losBlc);
	}
	else{
	    throw(AipsError("blc has to be a string or vector of strings")); 
      
	}
	if(trc.type() == ::casac::variant::STRING || 
		trc.type() == ::casac::variant::STRINGVEC){
	    toCasaVectorQuantity(trc, losTrc);
	}
	else{
	    throw(AipsError("trc has to be a string or vector of strings")); 
	}
 
	Vector<Int>pixaxes(pixelaxes);
	casa::Record *csysRec=toRecord(csys);
	if((csysRec->nfields()) != 0){ 
	    casa::Record *csysRec=toRecord(csys);
	    if(csysRec->nfields() <2){	
		throw(AipsError("Given coorsys parameter does not appear to be a valid coordsystem record"));
	    }
	    casa::CoordinateSystem *coordsys=casa::CoordinateSystem::restore(*csysRec, "");
	    leRegion=itsRegMan->wbox(losBlc, losTrc,  pixaxes, *coordsys, String(absrel), String(comment));
	} else {
	    //user has set csys already
	    leRegion=
		itsRegMan->wbox(losBlc, losTrc, pixaxes, String(absrel), String(comment));
	}


	retval=fromRecord(*leRegion);
	if(leRegion !=0)
	    delete leRegion;
	
    } catch (AipsError x) {
	*itsLog << LogIO::SEVERE << "Exception Reported: " 
		<< x.getMesg() << LogIO::POST;
	RETHROW(x);
    }

  return retval;
}

::casac::record*
regionmanager::wpolygon(const ::casac::variant& x, const ::casac::variant& y, 
			const std::vector<int>& pixelaxes, 
			const ::casac::record& csys, 
			const std::string& absrel, const std::string& comment)
{
    if ( !itsIsSetup )
	setup();
    casac::record* retval=0;
    *itsLog << LogOrigin("regionmanager", "wbox");

    try {
	casa::Vector<casa::Quantity> losX;
	casa::Vector<casa::Quantity> losY;
	if(x.type() == ::casac::variant::STRING || 
		x.type() == ::casac::variant::STRINGVEC){
	    toCasaVectorQuantity(x, losX);
	}
	else{
	    throw(AipsError("x has to be a string or vector of strings")); 
      
	}
	if(y.type() == ::casac::variant::STRING || 
		y.type() == ::casac::variant::STRINGVEC){
	    toCasaVectorQuantity(y, losY);
	}
	else{
	    throw(AipsError("trc has to be a string or vector of strings")); 
	}

	ImageRegion * leRegion=0;

	Vector<Int>pixaxes(pixelaxes);
	casa::Record *csysRec=toRecord(csys);
	if((csysRec->nfields()) != 0){ 
	    casa::Record *csysRec=toRecord(csys);
	    if(csysRec->nfields() <2){	
		throw(AipsError("Given coorsys parameter does not appear to be a valid coordsystem record"));
	    }
	    casa::CoordinateSystem *coordsys=casa::CoordinateSystem::restore(*csysRec, "");
	    leRegion=itsRegMan->wpolygon(losX, losY,  pixaxes, *coordsys, 
				   String(absrel));
	}
	else{
	    //user has set csys already
	    leRegion=itsRegMan->wpolygon(losX, losY, pixaxes, String(absrel));
	}

	if(leRegion !=0){
      
	    casa::Record *leRec= new casa::Record();
	    leRec->assign(leRegion->toRecord(String("")));
	    leRec->define("comment", comment);
	    retval=fromRecord(*leRec);
	    delete leRec;
	    delete leRegion;
	}
    } catch (AipsError x) {
	*itsLog << LogIO::SEVERE << "Exception Reported: " 
		<< x.getMesg() << LogIO::POST;
	RETHROW(x);
    }

    return retval;
}

::casac::record*
regionmanager::wmask(const std::string& expr)
{
    if ( !itsIsSetup )
	setup();
    *itsLog << LogOrigin("regionmanager", "wmask");


    // TODO : IMPLEMENT ME HERE !
    *itsLog << LogIO::SEVERE 
	    << "Sorry! This feature is not implemented yet."
	    << LogIO::EXCEPTION;
    return false;
}



} // casac namespace

