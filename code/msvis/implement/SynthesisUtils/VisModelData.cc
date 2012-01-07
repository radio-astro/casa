//# VisImagingWeight.cc: imaging weight calculation for a give buffer
//# Copyright (C) 2011
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
//# Correspondence concerning AIPS++ should be adressed as follows:
//#        Internet email: aips2-request@nrao.edu.
//#        Postal address: AIPS++ Project Office
//#                        National Radio Astronomy Observatory
//#                        520 Edgemont Road
//#                        Charlottesville, VA 22903-2475 USA
//#
//#
//# $Id$


#include <casa/Utilities/CountedPtr.h>
#include <casa/Arrays/ArrayMath.h>
#include <casa/Arrays/Vector.h>
#include <components/ComponentModels/ComponentList.h>

#include <msvis/MSVis/VisBuffer.h>
#include <msvis/SynthesisUtils/VisModelData.h>
#include <msvis/SynthesisUtils/FTMachine.h>
#include <msvis/SynthesisUtils/SimpleComponentFTMachine.h>
#include <msvis/SynthesisUtils/GridFT.h>
#include <msvis/SynthesisUtils/WProjectFT.h>

namespace casa { //# NAMESPACE CASA - BEGIN


VisModelData::VisModelData(): clholder_p(0), ftholder_p(0), flatholder_p(0){
  
  cft_p=new SimpleComponentFTMachine();
  }

  VisModelData::~VisModelData(){


  }

void VisModelData::putModel(const MeasurementSet& thems, const RecordInterface& rec, const Vector<Int>& validfieldids, const Vector<Int>& spws, const Vector<Int>& starts, const Vector<Int>& nchan,  const Vector<Int>& incr, Bool iscomponentlist, Bool incremental){

    //A field can have multiple FTmachines and ComponentList associated with it 
    //For example having many flanking images for the model
    //For componentlist it may have multiple componentlist ...for different spw


    Int counter=0;
    Record modrec;
    modrec.define("fields", validfieldids);
    modrec.define("spws", spws);
    modrec.define("start", starts);
    modrec.define("nchan", nchan);
    modrec.define("incr", incr);
    modrec.defineRecord("container", rec);
    String elkey="model";
    for (uInt k=0; k < validfieldids.nelements();  ++k){
      elkey=elkey+"_"+String::toString(validfieldids[k]);
    }
    Table newTab(thems);
    Record outRec;
    if(incremental){
      if(newTab.rwKeywordSet().isDefined(elkey))
	outRec=newTab.rwKeywordSet().asRecord(elkey); 
    }
    if(iscomponentlist){
      modrec.define("type", "componentlist");
      if(outRec.isDefined("numcl"))
	counter=incremental ? outRec.asInt("numcl") : 0;
            
    }
    else{
      modrec.define("type", "ftmachine");
      if(outRec.isDefined("numft"))
	counter=incremental ? outRec.asInt("numft") : 0;
    }
    iscomponentlist ? outRec.define("numcl", counter+1) : outRec.define("numft", counter+1); 
  
    for (uInt k=0; k < validfieldids.nelements();  ++k){
      newTab.rwKeywordSet().define("definedmodel_field_"+String::toString(validfieldids[k]), elkey);
      
    }
    iscomponentlist ? outRec.defineRecord("cl_"+String::toString(counter), modrec):
      outRec.defineRecord("ft_"+String::toString(counter), modrec);
    if(newTab.rwKeywordSet().isDefined(elkey))
	newTab.rwKeywordSet().removeField(elkey);
    newTab.rwKeywordSet().defineRecord(elkey, outRec); 
  }




  /*  VisModelData::VisModelData(const Record& ftmachine, const Vector<Int>& validfieldids, const Vector<Int>& msIds): clholder_p(0), flatholder_p(0), cft_p(NULL){

    addFTMachine(ftmachine, validfieldids, msIds); 

  }
  
  void VisModelData::addFTMachine(const Record& ftrec, const Vector<Int>& validfieldIds, const Vector<Int>& msIds){
    if(max(validfieldIds)*(max(msIds)+1) > (Int(ftholder_p.nelements())-1)){
      uInt oldMaxInd=ftholder_p.nelements()-1;
      ftholder_p.resize(max(validfieldIds)*(max(msIds)+1)+1, False, True);
      for (uInt k=oldMaxInd+1; k < ftholder_p.nelements(); ++k)
	ftholder_p[k]=NULL;
      CountedPtr<FTMachine> ftp;
      String ftname=ftrec.asString("name");
      if(ftname=="GridFT"){
	ftp=new GridFT(ftrec);
      }
      for (uInt nms=0; nms < msIds.nelements(); ++nms){
	for (uInt nfield=0; nfield < validfieldIds.nelements(); ++ nfield){
	  uInt indx=nfield*(nms+1);
	  ftholder_p[indx]=ftp;
	}
      }
      
     

    }

  }

  */

  void VisModelData::addModel(const Record& rec,  const Vector<Int>& msids){
    Vector<Int>fields;
    Vector<Int> spws;

   

    Int indexft=-1;
    if(rec.isDefined("numft")){
      Int numft=rec.asInt("numft");
      if(numft >0){
	indexft=ftholder_p.nelements();
	ftholder_p.resize(indexft+1, False, True);
	ftholder_p[indexft].resize(numft);
	for(Int ftk=0; ftk < numft; ++ftk){
	  Record ftrec(rec.asRecord("ft_"+String::toString(ftk)));
	  ftrec.get("fields", fields);
	  ftrec.get("spws", spws);
	  ftholder_p[indexft][ftk]=NEW_FT(ftrec.asRecord("container"));
	} 
      }	      
    }
    Int indexcl=-1;
    if(rec.isDefined("numcl")){
      Int numcl=rec.asInt("numcl");
      if(numcl >0){
	indexcl=clholder_p.nelements();
	clholder_p.resize(indexcl+1, False, True);
	clholder_p[indexcl].resize(numcl);
	for(Int clk=0; clk < numcl; ++clk){
	  Record clrec(rec.asRecord("cl_"+String::toString(clk)));
	  clrec.get("fields", fields);
	  clrec.get("spws", spws);
	  String err;
	  clholder_p[indexcl][clk]=new ComponentList();
	  if(!((clholder_p[indexcl][clk])->fromRecord(err, clrec.asRecord("container"))))
	    throw(AipsError("Component model failed to load for field "+String::toString(fields)));
	} 
      }	      
    }

    //cerr << "fields " << fields << "  spws " << spws << endl;

    //make sure indexes are with size
    hasModel(max(msids), max(fields), max(spws));


    for (uInt msi=0; msi < msids.nelements() ; ++msi){
      for (uInt fi=0; fi < fields.nelements(); ++fi){
	for(uInt spi=0; spi < spws.nelements(); ++spi){
	  ftindex_p(spi, fi, msi)=indexft;	  
	  clindex_p(spi, fi, msi)=indexcl;
	}
      }
    }

  }

  FTMachine* VisModelData::NEW_FT(const Record& ftrec){
    String name=ftrec.asString("name");
    if(name=="GridFT")
      return new GridFT(ftrec);
    if(name=="WProjectFT")
      return new WProjectFT(ftrec);
    return NULL;
  }

  Bool VisModelData::hasModel(Int msid, Int field, Int spw){

    IPosition oldcubeShape=ftindex_p.shape();
    if(oldcubeShape(0) <(spw+1) || oldcubeShape(1) < (field+1) || oldcubeShape(2) < (msid+1)){
      Cube<Int> newind((spw+1), (field+1), (msid+1));
      newind.set(-1);
      newind(IPosition(3, 0,0,0), (oldcubeShape-1))=ftindex_p;
      ftindex_p.assign(newind);
      newind.set(-1);
      newind(IPosition(3, 0,0,0), (oldcubeShape-1))=clindex_p;
      clindex_p.assign(newind);
    }

    if( (clindex_p(spw, field, msid) <0)  &&  (ftindex_p(spw, field, msid) <0))
      return False;
    return True;


  }

  /* Bool VisModelData::hasFT(Int msid, Int fieldid){
    
    uInt indx=fieldid*(msid+1);
    if( (indx >= ftholder_p.nelements()) || ftholder_p[indx].null()){
      return False;
    }
    return True;


  }
  Bool VisModelData::hasCL(Int msid, Int fieldid){
    
    uInt indx=fieldid*(msid+1);
    if( (indx >= clholder_p.nelements()) || clholder_p[indx].null()){
      return False;
    }
    return True;


  }
  */
  /*
  void VisModelData::addCompFTMachine(const ComponentList& cl, const Vector<Int>& validfieldIds, const Vector<Int>& msIds){

    cerr << "Max field ids " << max(validfieldIds)*(max(msIds)+1) << " nelements " << (clholder_p.nelements()-1) << endl;
    if((max(validfieldIds)*(max(msIds)+1) > (Int(clholder_p.nelements())-1))){
      Int oldMaxInd=Int(clholder_p.nelements())-1;
      clholder_p.resize(max(validfieldIds)*(max(msIds)+1)+1, False, True);
      for (Int k=oldMaxInd+1; k < Int(clholder_p.nelements()); ++k){
	clholder_p[k]=NULL;
	cerr <<"Doing the NULL thing " << endl;
      }
      for (uInt nms=0; nms < msIds.nelements(); ++nms){
	for (uInt nfield=0; nfield < validfieldIds.nelements(); ++ nfield){
	  uInt indx=nfield*(nms+1);
	  cerr << "doing the indx " << indx  << endl;
	  clholder_p[indx]=new ComponentList(cl);
	}
      }
    }
    //for now use SimpleComponentFT always
    if(clholder_p.nelements() > 0){
      cft_p=new SimpleComponentFTMachine();
      cerr << "Do the ComponentFT Machine " << endl;
    }
  }
  */
  void VisModelData::initializeToVis(){


  }
  Bool VisModelData::getModelVis(VisBuffer& vb){

    Vector<CountedPtr<ComponentList> >cl=getCL(vb.msId(), vb.fieldId(), vb.spectralWindow());
    Vector<CountedPtr<FTMachine> > ft=getFT(vb.msId(), vb.fieldId(), vb.spectralWindow());
    //Fill the buffer with 0.0; also prevents reading from disk if MODEL_DATA exists
    vb.setModelVisCube(Complex(0.0,0.0));
    Bool incremental=False;
    if( cl.nelements()>0){
      //cerr << "In cft" << endl;
      for (uInt k=0; k < cl.nelements(); ++k)
	cft_p->get(vb, *(cl[k]), -1); 
      //cerr << "max " << max(vb.modelVisCube()) << endl;
      incremental=True;
    }
    if(ft.nelements()>0){
      Cube<Complex> tmpModel;
      if(incremental || ft.nelements() >1)
	tmpModel.assign(vb.modelVisCube());
      for (uInt k=0; k < ft.nelements(); ++k){
	ft[k]->get(vb, -1);
	if(ft.nelements()>1 || incremental){
	  tmpModel+=vb.modelVisCube();
	}
      }
      //cerr << "min max after ft " << min(vb.modelVisCube()) << max(vb.modelVisCube()) << endl; 
      if(ft.nelements()>1 || incremental)
	vb.modelVisCube()=tmpModel;
      incremental=True;      
    }
    if(!incremental){
      //No model was set so....
      ///Set the Model to 1.0 for parallel hand and 0.0 for x-hand
      vb.setModelVisCube(Complex(1.0,0.0));
      Vector<Int> corrType=vb.corrType();
      uInt nCorr = corrType.nelements();
      for (uInt i=0; i<nCorr; i++) {
	  if(corrType[i]==Stokes::RL || corrType[i]==Stokes::LR ||
	     corrType[i]==Stokes::XY || corrType[i]==Stokes::YX){
	    vb.modelVisCube().yzPlane(i)=0.0;
	  }
      }
    }
    
    return True;
    
  }


  Vector<CountedPtr<ComponentList> > VisModelData::getCL(const Int msId, const Int fieldId, const Int spwId){
    if(!hasModel(msId, fieldId, spwId))
      return Vector<CountedPtr<ComponentList> >(0);
    Int indx=clindex_p(spwId, fieldId, msId);
    //cerr << "indx " << indx << endl;
    if(indx <0)
      return Vector<CountedPtr<ComponentList> >(0);
    return clholder_p[indx];
	

  }

  Vector<CountedPtr<FTMachine> >VisModelData::getFT(const Int msId, const Int fieldId, Int spwId){

    if(!hasModel(msId, fieldId, spwId))
      return Vector<CountedPtr<FTMachine> >(0);
    Int indx=ftindex_p(spwId, fieldId, msId);
    //cerr << "indx " << indx << endl;
    if(indx <0)
      return Vector<CountedPtr<FTMachine> >(0);
    return ftholder_p[indx];
  }




}//# NAMESPACE CASA - END

