//# VPManager.cc: Implementation of VPManager.h
//# Copyright (C) 1996-2007
//# Associated Universities, Inc. Washington DC, USA.
//#
//# This program is free software; you can redistribute it and/or modify it
//# under the terms of the GNU General Public License as published by the Free
//# Software Foundation; either version 2 of the License, or (at your option)
//# any later version.
//#
//# This program is distributed in the hope that it will be useful, but WITHOUT
//# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
//# FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
//# more details.
//#
//# You should have received a copy of the GNU General Public License along
//# with this program; if not, write to the Free Software Foundation, Inc.,
//# 675 Massachusetts Ave, Cambridge, MA 02139, USA.
//#
//# Correspondence concerning AIPS++ should be addressed as follows:
//#        Internet email: aips2-request@nrao.edu.
//#        Postal address: AIPS++ Project Office
//#                        National Radio Astronomy Observatory
//#                        520 Edgemont Road
//#                        Charlottesville, VA 22903-2475 USA
//#
//# $Id$


#include <casa/Containers/Record.h>
#include <casa/Exceptions/Error.h>
#include <casa/iostream.h>
#include <tables/Tables/ScalarColumn.h>
#include <tables/Tables/ScaColDesc.h>
#include <tables/Tables/ScaRecordColDesc.h>
#include <tables/Tables/Table.h>
#include <tables/Tables/SetupNewTab.h>
#include <tables/Tables/TableParse.h>
#include <tables/Tables/TableDesc.h>
#include <casa/Quanta/QuantumHolder.h>
#include <casa/Quanta/Quantum.h>
#include <measures/Measures/MDirection.h>
#include <measures/Measures/MeasureHolder.h>
#include <synthesis/MeasurementEquations/VPManager.h>
#include <synthesis/MeasurementComponents/PBMathInterface.h>
#include <synthesis/MeasurementComponents/PBMath.h>
#include <synthesis/MeasurementComponents/SynthesisError.h>
#include <synthesis/MeasurementComponents/ALMACalcIlluminationConvFunc.h>
#include <casa/Logging.h>
#include <casa/Logging/LogIO.h>
#include <casa/Logging/LogSink.h>
#include <casa/Logging/LogMessage.h>
#include <casa/OS/Directory.h>
#include <images/Images/PagedImage.h>

namespace casa { //# NAMESPACE CASA - BEGIN

  VPManager* VPManager::instance_p = 0;

  VPManager* VPManager::Instance(){
    if(instance_p==0){
      instance_p = new VPManager();
    }
    return instance_p;
  }

  void VPManager::reset(){
    if(instance_p){
      delete instance_p;
      instance_p = new VPManager(True);
    }
  }    

  VPManager::VPManager(Bool verbose):
    vplist_p(),
    vplistdefaults_p(-1),
    aR_p()
  {

    LogIO os;
    os << LogOrigin("VPManager", "ctor");

    String telName;
    for(Int pbtype = static_cast<Int>(PBMath::DEFAULT) + 1;
	pbtype < static_cast<Int>(PBMath::NONE); ++pbtype){
      PBMath::nameCommonPB(static_cast<PBMath::CommonPB>(pbtype), telName);
      vplistdefaults_p.define(telName,-1);
    }

    // check for available AntennaResponses tables in the Observatories table
    Vector<String> obsName = MeasTable::Observatories();
    for(uInt i=0; i<obsName.size(); i++){

      String telName = obsName(i);

      String antRespPath;
      if(!MeasTable::AntennaResponsesPath(antRespPath, telName)) {
	// unknown observatory
	continue;
      }
      else{ // remember the corresponding telescopes as special vplist entries
	if(!aR_p.init(antRespPath)){
	  if(verbose){
	    os << LogIO::WARN
	       << "Invalid path defined in Observatories table for \"" << telName << "\":" << endl
	       << antRespPath << endl
	       << LogIO::POST;
	  }
	}
	else{
	  // init successful
	  Record rec;
	  rec.define("name", "REFERENCE");
	  rec.define("isVP", PBMathInterface::NONE);
	  rec.define("telescope", telName);
	  rec.define("antresppath", antRespPath);
	  
	  if(verbose){
	    os << LogIO::NORMAL
	       << "Will use " << telName << " antenna responses from table " 
	       << antRespPath << LogIO::POST;
	  }

	  if(vplistdefaults_p.isDefined(telName)){ // there is already a vp for this telName
	    Int ifield = vplistdefaults_p(telName);
	    if(ifield>=0){
	      Record rrec = vplist_p.rwSubRecord(ifield);
	      rrec.define("dopb", False);
	    }
	    vplistdefaults_p.remove(telName);	    
	  }
	  vplistdefaults_p.define(telName,vplist_p.nfields()); 
	  rec.define("dopb", True);
	  
	  vplist_p.defineRecord(vplist_p.nfields(), rec);
	}
      }
    }	
    if(verbose){
      os << LogIO::NORMAL << "VPManager initialized." << LogIO::POST;
    }
    
  }


  Bool VPManager::saveastable(const String& tablename){
    
    if (vplist_p.nfields() == 0)
      return False;
    TableDesc td("vptable", "1", TableDesc::New);
    td.addColumn(ScalarColumnDesc<String>("telescope"));
    td.addColumn(ScalarColumnDesc<Int>("antenna"));
    td.addColumn(ScalarRecordColumnDesc("pbdescription"));
    SetupNewTable aTab(tablename, td, Table::New);

    Table tb(aTab, Table::Plain, vplist_p.nfields());
    ScalarColumn<String> telcol(tb, "telescope");
    ScalarColumn<Int> antcol(tb, "antenna");
    ScalarColumn<TableRecord> pbcol(tb, "pbdescription");
    for (uInt k=0; k < vplist_p.nfields(); ++k){
      TableRecord antRec(vplist_p.asRecord(k));
      String tel=antRec.asString("telescope");
      telcol.put(k, tel);
      antcol.put(k,k);
      pbcol.put(k,antRec);      
	 
    }
    tb.flush();
    return True;
  }

  Bool VPManager::summarizevps(const Bool verbose) {

    LogIO os(LogOrigin("vpmanager", "summarizevps"));

    os << LogIO::NORMAL << "Voltage patterns internally defined in CASA (* = set as default):"
       << LogIO::POST;
    String telName;
    for(Int pbtype = static_cast<Int>(PBMath::DEFAULT) + 1;
        pbtype < static_cast<Int>(PBMath::NONE); ++pbtype){
      PBMath::nameCommonPB(static_cast<PBMath::CommonPB>(pbtype), telName);
      if(vplistdefaults_p.isDefined(telName)
	 && (vplistdefaults_p(telName)==-1)
	 ){
	os << LogIO::NORMAL << "  *  ";
      }
      else{
	os << LogIO::NORMAL << "     ";
      }
      os << telName << LogIO::POST;
    }
    
    os << LogIO::NORMAL << "\nExternally defined voltage patterns (* = set as default):" << LogIO::POST;
    if (vplist_p.nfields() > 0) {
      os << "VP#     Tel         VP Type" << LogIO::POST;
      for (uInt i=0; i < vplist_p.nfields(); ++i){
	TableRecord antRec(vplist_p.asRecord(i));
	String telName = antRec.asString("telescope");
	if(vplistdefaults_p.isDefined(telName)
	   && ((Int)i == vplistdefaults_p(telName))
	   ){
	  os << i << "  *  ";
	}
	else{
	  os << i << "     ";
	}
        os << String(telName+ "             ").resize(13) << antRec.asString("name");
	if(antRec.asString("name")=="REFERENCE"){
	  os << ": " << antRec.asString("antresppath");
	}
	os << LogIO::POST;
        if (verbose) {
	  ostringstream oss;
	  antRec.print(oss);
	  os << oss.str() << LogIO::POST;
        }
      }
    } else {
      os << "\tNone" << LogIO::POST;
    }

    return True;
  }

  Bool VPManager::setcannedpb(const String& tel, 
			      const String& other, 
			      const Bool dopb,
			      const String& commonpb,
			      const Bool dosquint, 
			      const Quantity& paincrement, 
			      const Bool usesymmetricbeam,
			      Record& rec){
    rec = Record();
    rec.define("name", "COMMONPB");
    rec.define("isVP", PBMathInterface::COMMONPB);
    if (tel=="OTHER") {
      rec.define("telescope", other);
    } else {
      rec.define("telescope", tel);
    }
    rec.define("dopb", dopb);
    rec.define("commonpb", commonpb);
    rec.define("dosquint", dosquint);
    String error;
    Record tempholder;
    QuantumHolder(paincrement).toRecord(error, tempholder);
    rec.defineRecord("paincrement", tempholder);
    rec.define("usesymmetricbeam", usesymmetricbeam);
    vplist_p.defineRecord(vplist_p.nfields(), rec);

    if(dopb){
      vplistdefaults_p.define(rec.asString(rec.fieldNumber("telescope")), vplist_p.nfields());
    } 

    return True;
  }

  Bool VPManager::setpbairy(const String& tel, 
			    const String& other, 
			    const Bool dopb, const Quantity& dishdiam, 
			    const Quantity& blockagediam, 
			    const Quantity& maxrad, 
			    const Quantity& reffreq, 
			    MDirection& squintdir, 
			    const Quantity& squintreffreq,const Bool dosquint, 
			    const Quantity& paincrement, 
			    const Bool usesymmetricbeam,
			    Record& rec){

    rec=Record();
    rec.define("name", "AIRY");
    rec.define("isVP", PBMathInterface::AIRY);
    if(tel=="OTHER"){
      rec.define("telescope", other);
    }
    else{
      rec.define("telescope", tel);
    }
    rec.define("dopb", dopb);
    String error;
    Record tempholder; 
    QuantumHolder(dishdiam).toRecord(error, tempholder);
    rec.defineRecord("dishdiam", tempholder);
    QuantumHolder(blockagediam).toRecord(error, tempholder);
    rec.defineRecord("blockagediam", tempholder);
    QuantumHolder(maxrad).toRecord(error, tempholder);
    rec.defineRecord("maxrad", tempholder);
    QuantumHolder(reffreq).toRecord(error, tempholder);
    rec.defineRecord("reffreq", tempholder);
    MeasureHolder(squintdir).toRecord(error, tempholder);
    rec.defineRecord("squintdir", tempholder);
    QuantumHolder(squintreffreq).toRecord(error, tempholder);
    rec.defineRecord("squintreffreq", tempholder);
    rec.define("dosquint", dosquint);
    QuantumHolder(paincrement).toRecord(error, tempholder);
    rec.defineRecord("paincrement", tempholder);
    rec.define("usesymmetricbeam", usesymmetricbeam);

    if(dopb){
      vplistdefaults_p.define(rec.asString(rec.fieldNumber("telescope")), vplist_p.nfields());
    } 

    vplist_p.defineRecord(vplist_p.nfields(), rec); 

    return True;

  }

  Bool VPManager::setpbcospoly(const String& telescope,
			       const String& othertelescope,
			       const Bool dopb, const Vector<Double>& coeff,
			       const Vector<Double>& scale,
			       const Quantity& maxrad,
			       const Quantity& reffreq,
			       const String& isthispb,
			       MDirection& squintdir,
			       const Quantity& squintreffreq,
			       const Bool dosquint,
			       const Quantity& paincrement,
			       const Bool usesymmetricbeam,
			       Record& rec) {

    rec=Record();
    rec.define("name", "COSPOLY");
    rec.define("isVP", PBMathInterface::COSPOLY);
    if(telescope=="OTHER"){
      rec.define("telescope", othertelescope);
    }
    else{
      rec.define("telescope", telescope);
    }
    rec.define("dopb", dopb);
    rec.define("coeff", coeff);
    rec.define("scale", scale);
    String error;
    Record tempholder; 
    QuantumHolder(maxrad).toRecord(error, tempholder);
    rec.defineRecord("maxrad", tempholder);
    QuantumHolder(reffreq).toRecord(error, tempholder);
    rec.defineRecord("reffreq", tempholder);
    if(isthispb=="PB" || isthispb=="pb"){
      rec.define("isthisvp", False);
    }
    else{
      rec.define("isthisvp", True);
    }
    MeasureHolder(squintdir).toRecord(error, tempholder);
    rec.defineRecord("squintdir", tempholder);
    QuantumHolder(squintreffreq).toRecord(error, tempholder);
    rec.defineRecord("squintreffreq", tempholder);
    rec.define("dosquint", dosquint);
    QuantumHolder(paincrement).toRecord(error, tempholder);
    rec.defineRecord("paincrement", tempholder);
    rec.define("usesymmetricbeam", usesymmetricbeam);

    if(dopb){
      vplistdefaults_p.define(rec.asString(rec.fieldNumber("telescope")), vplist_p.nfields());
    } 

    vplist_p.defineRecord(vplist_p.nfields(), rec); 

    return True;

  }

  Bool VPManager::setpbgauss(const String& tel, const String& other, 
			     const Bool dopb,
			     const Quantity& halfwidth, const Quantity maxrad, 
			     const Quantity& reffreq, 
			     const String& isthispb, 
			     MDirection& squintdir, 
			     const Quantity& squintreffreq,
			     const Bool dosquint, 
			     const Quantity& paincrement, 
			     const Bool usesymmetricbeam,
			     Record& rec){
    rec=Record();
    rec.define("name", "GAUSS");
    rec.define("isVP", PBMathInterface::GAUSS);
    if(tel=="OTHER"){
      rec.define("telescope", other);
    }
    else{
      rec.define("telescope", tel);
    }
    rec.define("dopb", dopb);
    Quantity hpw=halfwidth;
    if(isthispb=="PB" || isthispb=="pb"){
      hpw.setValue(hpw.getValue()/2.0);
    }
    String error;
    Record tempholder; 
    QuantumHolder(hpw).toRecord(error, tempholder);
    rec.defineRecord("halfwidth", tempholder);
    
    QuantumHolder(maxrad).toRecord(error, tempholder);
    rec.defineRecord("maxrad", tempholder);
    QuantumHolder(reffreq).toRecord(error, tempholder);
    rec.defineRecord("reffreq", tempholder);
    if(isthispb=="PB" || isthispb=="pb"){
      rec.define("isthisvp", False);
    }
    else{
      rec.define("isthisvp", True);
    }
    MeasureHolder(squintdir).toRecord(error, tempholder);
    rec.defineRecord("squintdir", tempholder);
    QuantumHolder(squintreffreq).toRecord(error, tempholder);
    rec.defineRecord("squintreffreq", tempholder);
    rec.define("dosquint", dosquint);
    QuantumHolder(paincrement).toRecord(error, tempholder);
    rec.defineRecord("paincrement", tempholder);
    rec.define("usesymmetricbeam", usesymmetricbeam);

    if(dopb){
      vplistdefaults_p.define(rec.asString(rec.fieldNumber("telescope")), vplist_p.nfields());
    } 

    vplist_p.defineRecord(vplist_p.nfields(), rec); 

    return True;

  }

  Bool VPManager::setpbinvpoly(const String& telescope,
			       const String& othertelescope,
			       const Bool dopb, const Vector<Double>& coeff,
			       const Quantity& maxrad,
			       const Quantity& reffreq,
			       const String& isthispb,
			       MDirection& squintdir,
			       const Quantity& squintreffreq,
			       const Bool dosquint,
			       const Quantity& paincrement,
			       const Bool usesymmetricbeam,
			       Record& rec) {

    rec=Record();
    rec.define("name", "IPOLY");
    rec.define("isVP", PBMathInterface::IPOLY);
    if(telescope=="OTHER"){
      rec.define("telescope", othertelescope);
    }
    else{
      rec.define("telescope", telescope);
    }
    rec.define("dopb", dopb);
    rec.define("coeff", coeff);
    String error;
    Record tempholder; 
    QuantumHolder(maxrad).toRecord(error, tempholder);
    rec.defineRecord("maxrad", tempholder);
    QuantumHolder(reffreq).toRecord(error, tempholder);
    rec.defineRecord("reffreq", tempholder);
    if(isthispb=="PB" || isthispb=="pb"){
      rec.define("isthisvp", False);
    }
    else{
      rec.define("isthisvp", True);
    }
    MeasureHolder(squintdir).toRecord(error, tempholder);
    rec.defineRecord("squintdir", tempholder);
    QuantumHolder(squintreffreq).toRecord(error, tempholder);
    rec.defineRecord("squintreffreq", tempholder);
    rec.define("dosquint", dosquint);
    QuantumHolder(paincrement).toRecord(error, tempholder);
    rec.defineRecord("paincrement", tempholder);
    rec.define("usesymmetricbeam", usesymmetricbeam);

    if(dopb){
      vplistdefaults_p.define(rec.asString(rec.fieldNumber("telescope")), vplist_p.nfields());
    } 

    vplist_p.defineRecord(vplist_p.nfields(), rec); 

    return True;


  }

  Bool VPManager::setpbnumeric(const String& telescope,
			       const String& othertelescope,
			       const Bool dopb, const Vector<Double>& vect,
			       const Quantity& maxrad,
			       const Quantity& reffreq,
			       const String& isthispb,
			       MDirection& squintdir,
			       const Quantity& squintreffreq,
			       const Bool dosquint,
			       const Quantity& paincrement,
			       const Bool usesymmetricbeam,
			       Record &rec) {

    rec=Record();
    rec.define("name", "NUMERIC");
    rec.define("isVP", PBMathInterface::NUMERIC);
    if(telescope=="OTHER"){
      rec.define("telescope", othertelescope);
    }
    else{
      rec.define("telescope", telescope);
    }
    rec.define("dopb", dopb);
    rec.define("vect", vect);
    String error;
    Record tempholder; 
    QuantumHolder(maxrad).toRecord(error, tempholder);
    rec.defineRecord("maxrad", tempholder);
    QuantumHolder(reffreq).toRecord(error, tempholder);
    rec.defineRecord("reffreq", tempholder);
    if(isthispb=="PB" || isthispb=="pb"){
      rec.define("isthisvp", False);
    }
    else{
      rec.define("isthisvp", True);
    }
    MeasureHolder(squintdir).toRecord(error, tempholder);
    rec.defineRecord("squintdir", tempholder);
    QuantumHolder(squintreffreq).toRecord(error, tempholder);
    rec.defineRecord("squintreffreq", tempholder);
    rec.define("dosquint", dosquint);
    QuantumHolder(paincrement).toRecord(error, tempholder);
    rec.defineRecord("paincrement", tempholder);
    rec.define("usesymmetricbeam", usesymmetricbeam);

    if(dopb){
      vplistdefaults_p.define(rec.asString(rec.fieldNumber("telescope")), vplist_p.nfields());
    } 

    vplist_p.defineRecord(vplist_p.nfields(), rec); 

    return True;

  }

  Bool VPManager::setpbimage(const String& tel, 
			     const String& other, 
			     const Bool dopb, const String& realimage, 
			     const String& imagimage,
			     Record& rec){
    rec=Record();
    rec.define("name", "IMAGE");
    rec.define("isVP", PBMathInterface::IMAGE);
    if(tel=="OTHER"){
      rec.define("telescope", other);
    }
    else{
      rec.define("telescope", tel);
    }
    rec.define("dopb", dopb);
    rec.define("realimage", realimage);
    rec.define("imagimage", imagimage);

    if(dopb){
      vplistdefaults_p.define(rec.asString(rec.fieldNumber("telescope")), vplist_p.nfields());
    } 

    vplist_p.defineRecord(vplist_p.nfields(), rec); 
    
    return True;
    
  }

  Bool VPManager::setpbpoly(const String& telescope,
			    const String& othertelescope,
			    const Bool dopb, const Vector<Double>& coeff,
			    const Quantity& maxrad,
			    const Quantity& reffreq,
			    const String& isthispb,
			    MDirection& squintdir,
			    const Quantity& squintreffreq, const Bool dosquint,
			    const Quantity& paincrement,
			    const Bool usesymmetricbeam,
			    Record &rec) {

    rec=Record();
    rec.define("name", "POLY");
    rec.define("isVP", PBMathInterface::POLY);
    if(telescope=="OTHER"){
      rec.define("telescope", othertelescope);
    }
    else{
      rec.define("telescope", telescope);
    }
    rec.define("dopb", dopb);
    rec.define("coeff", coeff);
    String error;
    Record tempholder; 
    QuantumHolder(maxrad).toRecord(error, tempholder);
    rec.defineRecord("maxrad", tempholder);
    QuantumHolder(reffreq).toRecord(error, tempholder);
    rec.defineRecord("reffreq", tempholder);
    if(isthispb=="PB" || isthispb=="pb"){
      rec.define("isthisvp", False);
    }
    else{
      rec.define("isthisvp", True);
    }
    MeasureHolder(squintdir).toRecord(error, tempholder);
    rec.defineRecord("squintdir", tempholder);
    QuantumHolder(squintreffreq).toRecord(error, tempholder);
    rec.defineRecord("squintreffreq", tempholder);
    rec.define("dosquint", dosquint);
    QuantumHolder(paincrement).toRecord(error, tempholder);
    rec.defineRecord("paincrement", tempholder);
    rec.define("usesymmetricbeam", usesymmetricbeam);

    if(dopb){
      vplistdefaults_p.define(rec.asString(rec.fieldNumber("telescope")), vplist_p.nfields());
    } 

    vplist_p.defineRecord(vplist_p.nfields(), rec); 

    return True;

  }

  Bool VPManager::setpbantresptable(const String& telescope, const String& othertelescope,
				    const Bool dopb, const String& tablepath){

    Record rec;
    rec.define("name", "REFERENCE");
    rec.define("isVP", PBMathInterface::NONE);
    if(telescope=="OTHER"){
      rec.define("telescope", othertelescope);
    }
    else{
      rec.define("telescope", telescope);
    }
    rec.define("dopb", dopb);
    rec.define("antresppath", tablepath);
    
    if(dopb){
      vplistdefaults_p.define(rec.asString(rec.fieldNumber("telescope")), vplist_p.nfields());
    } 

    vplist_p.defineRecord(vplist_p.nfields(), rec); 

    return True;

  }
  

  Bool VPManager::setuserdefault(const Int vplistfield, // (-1 means reset to standard default)
				 const String& telescope,
				 const String& antennatype){     

    LogIO os;
    os <<  LogOrigin("VPManager", "setuserdefault");

    if((vplistfield < -1) || ((Int)(vplist_p.nfields()) <= vplistfield)){
      os << LogIO::SEVERE << " entry " << vplistfield << " does not exist in VP list."
	 << LogIO::POST;
      return False;
    }

    String antennaDesc = antennaDescription(telescope, antennatype);

    if(vplistfield>=0){
      const Record rec = vplist_p.subRecord(vplistfield);
      // check if this is a valid VP for this telescope
      String telName;
      const Int telFieldNumber=rec.fieldNumber("telescope");
      if (telFieldNumber!=-1){
	telName = rec.asString(telFieldNumber);
	if(telFromAntDesc(telName)!=telescope){
	  os << LogIO::SEVERE << " entry " << vplistfield << " does not point ot a valid VP for " << telescope
	     << LogIO::POST;
	  return False;
	}
      }
      Record srec = vplist_p.rwSubRecord(vplistfield);
      srec.define("dopb", True);
    }
    // unset set an existing default 
    if(vplistdefaults_p.isDefined(antennaDesc)){ 
      vplistdefaults_p.remove(antennaDesc);
    }
    vplistdefaults_p.define(antennaDesc,vplistfield);

    return True;

  }

  Bool VPManager::getuserdefault(Int& vplistfield,
				 const String& telescope,
				 const String& antennatype){

    String antennaDesc = antennaDescription(telescope, antennatype);

    if(!vplistdefaults_p.isDefined(antennaDesc)){
      return False;
    }
    vplistfield = vplistdefaults_p(antennaDesc);
    return True;

  }

  // return number of voltage patterns satisfying the given constraints
  Int VPManager::numvps(const String& telescope,
			const MEpoch& obstime,
			const MFrequency& freq, 
			const String& antennatype, // default: any
			const MDirection& obsdirection // default: Zenith
			){
    LogIO os;
    os << LogOrigin("VPManager", "numvps");

    Int rval=0;

    if(telescope.empty()){ // return number of keys in vplistdefaults_p
      rval = vplistdefaults_p.ndefined();
    }
    else{ // we have a telescope
      if(vplistdefaults_p.isDefined(telescope)){ // found global entry
	Int ifield = vplistdefaults_p(telescope);
	if(ifield==-1){ // internally defined PB does not distinguish antenna types
	  rval = 1;
	}
	else{ // externally defined PB
	  TableRecord antRec(vplist_p.asRecord(ifield));
	  String thename = antRec.asString("name");
	  if(thename=="REFERENCE"){ // points to an AntennaResponses table
	    // query the antenna responses
	    String antRespPath = antRec.asString("antresppath");
	    if(!aR_p.isInit(antRespPath) // don't reread if not necessary 
	       && !aR_p.init(antRespPath)){
	      os << LogIO::SEVERE
		 << "Invalid path defined in vpmanager for \"" << telescope << "\":" << endl
		 << antRespPath << endl
		 << LogIO::POST;
	    }
	    else{ // init successful
	      Vector<String> antTypes;
	      if(aR_p.getAntennaTypes(antTypes,
				      telescope, // (the observatory name, e.g. "ALMA" or "ACA")
				      obstime,
				      freq,
				      AntennaResponses::ANY, // the requested function type
				      obsdirection)){ // success
	      }
	      if(antennatype.empty()){ // antenna type was not given
		rval = antTypes.size();
	      }
	      else{
		for(uInt i=0; i<antTypes.size(); i++){
		  if(antTypes(i)==antennatype){
		    rval = 1;
		    break;
		  }
		}
	      }
	    }
	  }
	  else{ // we have a PBMath response
	    rval = 1;
	  }
	} // end if (ifield==-1)
      }
      else if(antennatype.empty()){ // no global entry and antenna type not given: need to count specific entries
	rval = 0;
	for(uInt i=0; i<vplistdefaults_p.ndefined(); i++){
	  String aDesc = vplistdefaults_p.getKey(i);
	  if(telescope == telFromAntDesc(aDesc)){
	    rval++;
	  }
	}
      }
      else{ // no global entry and antenna type given: need to look for specific entry
	String antDesc = antennaDescription(telescope,antennatype);
	if(vplistdefaults_p.isDefined(antDesc)){
	  rval = 1;
	}
      } 
    } // endif(telescope.empty())

    return rval;

  }

    // get the voltage pattern satisfying the given constraints
  Bool VPManager::getvp(Record &rec,
			const String& telescope,
			const MEpoch& obstime,
			const MFrequency& freq, 
			const String& antennatype, // default: "" 
			const MDirection& obsdirection){ // default is the Zenith

    LogIO os;
    os << LogOrigin("VPManager", "getvp");
    
    rec = Record();
    Int rval=False;

    String antDesc = antennaDescription(telescope, antennatype);

    Int ifield = -2;
    if(vplistdefaults_p.isDefined(telescope)){ // found global entry
      ifield = vplistdefaults_p(telescope);
    }
    else if(vplistdefaults_p.isDefined(antDesc)){
      ifield = vplistdefaults_p(antDesc);
    }

    if(ifield==-1){ // internally defined PB does not distinguish antenna types
	
      rec.define("name", "COMMONPB");
      rec.define("isVP", PBMathInterface::COMMONPB);
      rec.define("telescope", telescope);
      rec.define("dopb", True);
      rec.define("commonpb", telescope);
      rec.define("dosquint", False);
      String error;
      Record tempholder;
      QuantumHolder(Quantity(10.,"deg")).toRecord(error, tempholder);
      rec.defineRecord("paincrement", tempholder);
      rec.define("usesymmetricbeam", False);
	
      rval = True;
	
    }
    else if(ifield>=0){ // externally defined PB
      TableRecord antRec(vplist_p.asRecord(ifield));
      String thename = antRec.asString("name");
      if(thename=="REFERENCE"){ // points to an AntennaResponses table

	// query the antenna responses
	String antRespPath = antRec.asString("antresppath");
	if(!aR_p.isInit(antRespPath) // don't reread if not necessary 
	   && !aR_p.init(antRespPath)){
	  os << LogIO::SEVERE
	     << "Invalid path defined in vpmanager for \"" << telescope << "\":" << endl
	     << antRespPath << endl
	     << LogIO::POST;
	  return False;
	}
	// init successful
	String functionImageName;
	uInt funcChannel;
	MFrequency nomFreq;
	AntennaResponses::FuncTypes fType;
	MVAngle rotAngOffset;
	
	if(!aR_p.getImageName(functionImageName, // the path to the image
			      funcChannel, // the channel to use in the image  
			      nomFreq, // nominal frequency of the image (in the given channel)
			      fType, // the function type of the image
			      rotAngOffset, // the response rotation angle offset
			      /////////////////////
			      telescope,
			      obstime,
			      freq,
			      AntennaResponses::ANY, // the requested function type
			      antennatype,
			      obsdirection)
	   ){
	  rec = Record();
	  return False;
	}
	
	// getImageName was successful
	
	// construct record
	rec = Record();
	Unit uHz("Hz");
	switch(fType){
	case AntennaResponses::AIF: // complex aperture illumination function
	  os << LogIO::WARN << "Responses type AIF provided for " << telescope << " in " << endl
	     << antRespPath << endl
	     << " not yet supported."
	     << LogIO::POST;
	  rval = False;
	  break;
	case AntennaResponses::EFP: // complex electric field pattern
	  rec.define("name", "IMAGE");
	  rec.define("isVP", PBMathInterface::IMAGE);
	  rec.define("telescope", telescope);
	  rec.define("dopb", True);
	  rec.define("compleximage", functionImageName);
	  rec.define("channel", funcChannel);
	  rec.define("reffreq", nomFreq.get(uHz).getValue());
	  rval = True;
	  break;
	case AntennaResponses::VP: // real voltage pattern
	  rec.define("name", "IMAGE");
	  rec.define("isVP", PBMathInterface::IMAGE);
	  rec.define("telescope", telescope);
	  rec.define("dopb", True);
	  rec.define("realimage", functionImageName);
	  rec.define("channel", funcChannel);
	  rec.define("reffreq", nomFreq.get(uHz).getValue());
	  rval = True;
	  break;
	case AntennaResponses::VPMAN: // the function is available in casa via the vp manager, i.e. use COMMONPB
	  // same as if ifield == -1
	  rec.define("name", "COMMONPB");
	  rec.define("isVP", PBMathInterface::COMMONPB);
	  rec.define("telescope", telescope);
	  rec.define("dopb", True);
	  rec.define("commonpb", telescope);
	  rec.define("dosquint", False);
	  {
	    String error;
	    Record tempholder;
	    QuantumHolder(Quantity(10.,"deg")).toRecord(error, tempholder);
	    rec.defineRecord("paincrement", tempholder);
	  }
	  rec.define("usesymmetricbeam", False);
	  rval = True;
	  break;
	case AntennaResponses::INTERNAL: // the function is generated using the BeamCalc class
	  {
	    String antRayPath = functionImageName;
	    String beamCalcedImagePath = "./BeamCalcTmpImage_"+telescope+"_"+antennatype+"_"
	      +String::toString(freq.get(Unit("MHz")).getValue())+"MHz";
	  
	    // calculate the beam
	    
	    if(!(telescope=="ALMA" || telescope=="ACA")){
	      os << LogIO::WARN << "Responses type INTERNAL provided for \"" << telescope << " in " << endl
		 << antRespPath << endl
		 << " not yet supported."
		 << LogIO::POST;
	      rval = False;
	    }
	    else{ // telescope=="ALMA" || telescope=="ACA"
	      try{
		// handle preexisting beam image
		Directory f(beamCalcedImagePath);
		if(f.exists()){
		  os << LogIO::NORMAL << "Will re-use VP image \"" << beamCalcedImagePath << "\"" << LogIO::POST;
		  //f.removeRecursive();
		}
		else{
		  CoordinateSystem coordsys;
	      
		  Double refFreq = freq.get(uHz).getValue();

		  // DirectionCoordinate
		  Matrix<Double> xform(2,2);                                    
		  xform = 0.0; xform.diagonal() = 1.0;                          
		  DirectionCoordinate dirCoords(MDirection::AZELGEO,                  
						Projection(Projection::SIN),        
						0.0, 0.0,
						0.5*C::pi/180.0/3600.0 * 5E11/refFreq, 
						0.5*C::pi/180.0/3600.0 * 5E11/refFreq,        
						xform,                              
						127.5, 127.5);  // (256-1)/2.
		  Vector<String> units(2); 
		  //units = "deg";                       
		  //dirCoords.setWorldAxisUnits(units);                               
		  
		  // StokesCoordinate
		  Vector<Int> stoks(4);
		  stoks(0) = Stokes::XX;
		  stoks(1) = Stokes::XY;
		  stoks(2) = Stokes::YX;
		  stoks(3) = Stokes::YY;
		  StokesCoordinate stokesCoords(stoks);	
		  
		  // SpectralCoordinate
		  SpectralCoordinate spectralCoords(MFrequency::castType(freq.myType()),           
						    refFreq,                 
						    1.0E+3, // dummy increment                  
						    0,                             
						    refFreq);          
		  units.resize(1);
		  units = "Hz";
		  spectralCoords.setWorldAxisUnits(units);
		  
		  coordsys.addCoordinate(dirCoords);
		  coordsys.addCoordinate(stokesCoords);
		  coordsys.addCoordinate(spectralCoords);
		  
		  TiledShape ts(IPosition(4,256,256,4,1));
		  PagedImage<Complex> im(ts, coordsys, beamCalcedImagePath);
		  im.set(Complex(1.0,1.0));
		  ALMACalcIlluminationConvFunc almaPB;
		  Long cachesize=(HostInfo::memoryTotal(True)/8)*1024;
		  almaPB.setMaximumCacheSize(cachesize);
		  almaPB.applyPB(im, telescope, obstime, antennatype, antennatype, freq.get(uHz));
		} // endif exists
	      } catch (AipsError x) {
		os << LogIO::SEVERE
		   << "BeamCalc failed."
		   << LogIO::POST;
		return False;
	      }
	    
	      // construct record
	      rec.define("name", "IMAGE");
	      rec.define("isVP", PBMathInterface::IMAGE);
	      rec.define("telescope", telescope);
	      rec.define("dopb", True);
	      rec.define("realimage", beamCalcedImagePath);
	      rec.define("channel", 0);
	      rec.define("antennatype", antennatype);
	      rec.define("reffreq", freq.get(uHz).getValue());
	      rval = True;
	    }
	  }
	  break;
	case AntennaResponses::NA: // not available
	default:
	  rval = False;
	  break;
	} // end switch(ftype)
      } 
      else{ // we have a PBMath response
      
	rec = vplist_p.subRecord(ifield);
	rval = True;
	
      } // end if internally defined
    }
    // else{} // ifield < -1, i.e. no VP available

    return rval;

  }


} //# NAMESPACE CASA - END
