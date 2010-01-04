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
#include <casa/Logging.h>
#include <casa/Logging/LogIO.h>
#include <casa/Logging/LogSink.h>
#include <casa/Logging/LogMessage.h>


namespace casa { //# NAMESPACE CASA - BEGIN
  VPManager::VPManager() {

    vplist_p=Record();

  }
  VPManager::~VPManager(){

    vplist_p=Record();
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

    os << LogIO::NORMAL << "Voltage patterns in the main CASA repository:"
       << LogIO::POST;
    String telName;
    for(Int pbtype = static_cast<Int>(PBMath::DEFAULT) + 1;
        pbtype < static_cast<Int>(PBMath::NONE); ++pbtype){
      PBMath::nameCommonPB(static_cast<PBMath::CommonPB>(pbtype), telName);
      os << LogIO::NORMAL << telName << LogIO::POST;
    }
    
    os << LogIO::NORMAL << "\nUser defined voltage patterns:" << LogIO::POST;
    if (vplist_p.nfields() > 0) {
      os << "VP#  Tel    VP Type" << LogIO::POST;
      for (uInt i=0; i < vplist_p.nfields(); ++i){
	TableRecord antRec(vplist_p.asRecord(i));
        os << i << "    " + antRec.asString("telescope")
	  + "    " + antRec.asString("name") << LogIO::POST;
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
    vplist_p.defineRecord(vplist_p.nfields(), rec); 

    return True;


  }

} //# NAMESPACE CASA - END
