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
#include <casa/OS/Timer.h>
#include <casa/Logging/LogIO.h>
#include <components/ComponentModels/ComponentList.h>
#include <ms/MeasurementSets/MSSelection.h>
#include <ms/MeasurementSets/MSSelectionTools.h>

#include <synthesis/MSVis/VisBuffer.h>
#include <synthesis/TransformMachines/VisModelData.h>
#include <synthesis/TransformMachines/FTMachine.h>
#include <synthesis/TransformMachines/SimpleComponentFTMachine.h>
#include <synthesis/TransformMachines/GridFT.h>
#include <synthesis/TransformMachines/rGridFT.h>
#include <synthesis/TransformMachines/MosaicFT.h>
#include <synthesis/TransformMachines/WProjectFT.h>
#include <synthesis/TransformMachines/MultiTermFT.h>
#include <synthesis/TransformMachines/SetJyGridFT.h>

namespace casa { //# NAMESPACE CASA - BEGIN


VisModelData::VisModelData(): clholder_p(0), ftholder_p(0), flatholder_p(0){
  
  cft_p=new SimpleComponentFTMachine();
  }

  VisModelData::~VisModelData(){


  }

void VisModelData::listModel(const MeasurementSet& thems){
 
  Table newTab(thems);

  ROMSColumns msc(thems);
  Vector<String> fldnames=msc.field().name().getColumn();
  Vector<Int> fields=msc.fieldId().getColumn();
  const Sort::Order order=Sort::Ascending;
  const Int option=Sort::HeapSort | Sort::NoDuplicates;
  Int nfields=GenSort<Int>::sort (fields, order, option);

  LogIO logio;
  if (nfields>0) {

    logio << "MS Header field records:"
	  << LogIO::POST;

    Int nlis(0);
    for (Int k=0; k< nfields; ++k){
      if(newTab.rwKeywordSet().isDefined("definedmodel_field_"+String::toString(fields[k])))
	{
	  String elkey=newTab.rwKeywordSet().asString("definedmodel_field_"+String::toString(fields[k]));
	  if(newTab.rwKeywordSet().isDefined(elkey))
	    logio << " " << fldnames[fields[k]] << " (id = " << fields[k] << ")" << LogIO::POST;
	  ++nlis;
	}
    }
    if (nlis==0)
      logio << " None." << LogIO::POST;
  }
    
}
void VisModelData::clearModel(const MeasurementSet& thems){
  
  Table newTab(thems);
  if(!newTab.isWritable())
    return;

  LogIO logio;
  logio << "Clearing all model records in MS header."
	  << LogIO::POST;

  ROMSColumns msc(thems);
  Vector<Int> fields=msc.fieldId().getColumn();
  Vector<String> fldnames=msc.field().name().getColumn();
  const Sort::Order order=Sort::Ascending;
  const Int option=Sort::HeapSort | Sort::NoDuplicates;
  Int nfields=GenSort<Int>::sort (fields, order, option);
  for (Int k=0; k< nfields; ++k){
    if(newTab.rwKeywordSet().isDefined("definedmodel_field_"+String::toString(fields[k])))

      {
	logio << " " << fldnames[fields[k]] << " (id = " << fields[k] << ") deleted." << LogIO::POST;

	String elkey=newTab.rwKeywordSet().asString("definedmodel_field_"+String::toString(fields[k]));
	if(newTab.rwKeywordSet().isDefined(elkey))
	  newTab.rwKeywordSet().removeField(elkey);
	newTab.rwKeywordSet().removeField("definedmodel_field_"+String::toString(fields[k]));
      }
  }
  



}
  void VisModelData::clearModel(const MeasurementSet& thems, const String field, const String specwindows){
 
  Table newTab(thems);
  if(!newTab.isWritable())
    return;

  ROMSColumns msc(thems);
  Vector<String> fldnames=msc.field().name().getColumn();
  Int nfields=0;
  Vector<Int> fields(0);
  {
    // Parse field specification
    MSSelection mss;
    mss.setFieldExpr(field);
    TableExprNode ten=mss.toTableExprNode(&thems);
    fields=mss.getFieldList();
    nfields=fields.nelements();
  }


  if (nfields==0)
    // Call the method that deletes them all
    VisModelData::clearModel(thems);
  else {

    //Now we have the two cases the whole field or specific spws
    // only delete the specified ones
    Vector<Int> spws(0);
    Int nspws=0;
    {
      // Parse field specification
      MSSelection mss;
      mss.setFieldExpr(field);
      mss.setSpwExpr(specwindows);
      TableExprNode ten=mss.toTableExprNode(&thems);
      spws=mss.getSpwList();
      nspws=spws.nelements();
    }

   
    LogIO logio;
    logio << "Clearing model records in MS header for selected fields." 
	  << LogIO::POST;
    
    for (Int k=0; k< nfields; ++k){
      if(newTab.rwKeywordSet().isDefined("definedmodel_field_"+String::toString(fields[k])))
	
	{
	  
	  String elkey=newTab.rwKeywordSet().asString("definedmodel_field_"+String::toString(fields[k]));
	  if(nspws==0){
	    logio << " " << fldnames[fields[k]] << " (id = " << fields[k] << ") deleted." << LogIO::POST;
	    
	    if(newTab.rwKeywordSet().isDefined(elkey))
	      newTab.rwKeywordSet().removeField(elkey);
	    newTab.rwKeywordSet().removeField("definedmodel_field_"+String::toString(fields[k]));
	  }
	  else{
	    if(newTab.rwKeywordSet().isDefined(elkey)){
	      Record conteneur=newTab.rwKeywordSet().asRecord(elkey);
	      removeSpw(conteneur, spws);
	      newTab.rwKeywordSet().removeField(elkey);
	      newTab.rwKeywordSet().defineRecord(elkey, conteneur);
	      
	      /*
	      Vector<Int> defspws=conteneur.asArrayInt("spws");
	      Vector<Int> newdefspw(defspws.nelements(), -1);
	      Int counter=0;
	      for(uInt k=0; k < defspws.nelements(); ++k){
		for (Int j=0; j < nspws; ++j){
		  if(defspws[k] != spws[j]){
		    newdefspw[counter]=defspws[k];
		    ++counter;
		  }
		}
	      }
	      if(counter==0){
		//Now we have to remove this ftm or cft
		newTab.rwKeywordSet().removeField(elkey);
	      }
	      else{
		conteneur.define("spws", newdefspw);
		updatespw(conteneur, 
	      }
	      */
	    }
	      
	  }
	    
	}
      else
	logio << " " << fldnames[fields[k]] << " (id = " << fields[k] << ") not found." << LogIO::POST;
    }
    
  }
  
  

  }

  Bool VisModelData::removeSpw(Record& therec, const Vector<Int>& spws){
     Int numft=0;
     Int numcl=0;
     Vector<String> numtype(2);
     Vector<String> keyval(2);
     numtype[0]=String("numft");
     numtype[1]=String("numcl");
     keyval[0]="ft_";
     keyval[1]="cl_";
     
     for (Int j=0; j< 2; ++j){
       
       if(therec.isDefined(numtype[j])){
	 numft=therec.asInt(numtype[j]);
	 Vector<String> toberemoved(numft);
	 Int numrem=0;
	 for (Int k=0; k < numft; ++k){
	   RecordInterface& ftrec=therec.asrwRecord(keyval[j]+String::toString(k));
	   if(!removeSpwFromMachineRec(ftrec, spws)){
	     toberemoved[numrem]=String(keyval[j]+String::toString(k));
	     ++numrem;
	   }
	 }
	 if(numrem > 0){
	   for (Int k=0; k < numrem; ++k)
	     removeFTFromRec(therec, toberemoved[k]);
	 }
       }

     }
     return True;
  }

  Bool VisModelData::removeFTFromRec(Record& therec, const String& keyval){

    String *splitkey=new String[2];
    Int nsep=split(keyval, splitkey, 2, String("_"));
    if (nsep <1 || !therec.isDefined(keyval)) 
      return False;
    String eltype=splitkey[0];
    //Int modInd=String::toInt(splitkey[1]);
    Int numcomp= (eltype==String("ft")) ? therec.asInt("numft"): therec.asInt("numcl");
    therec.removeField(keyval);
    
    numcomp=numcomp-1;
    if(eltype=="ft")
      therec.define("numft", numcomp);
    else
      therec.define("numcl", numcomp);
    
    eltype=eltype+String("_");
    Int id=0;
    for(uInt k=0; k < therec.nfields(); ++k){
      if(therec.name(k).contains(eltype)){
	therec.renameField(eltype+String::toString(id), k);
	++id;
      }
    }

    delete [] splitkey;
    return True;
  } 

  Bool VisModelData::removeSpwFromMachineRec(RecordInterface& ftclrec, const Vector<Int>& spws){
    Vector<Int> defspws=ftclrec.asArrayInt("spws");
    Vector<Int> newdefspw(defspws.nelements(), -1);
    Int counter=0;
    for(uInt k=0; k < defspws.nelements(); ++k){
      for (uInt j=0; j < spws.nelements(); ++j){
	if(defspws[k] == spws[j]){
	  defspws[k]=-1;
	  ++counter;
	}
      }
    }
    if(defspws.nelements() == uInt(counter)){
      //Now we have to remove this ft or cl model

      return False;

    }    
    newdefspw.resize(defspws.nelements()-counter);
    counter=0;
    for (uInt k=0; k < defspws.nelements(); ++k){
      if(defspws[k] != -1){
	newdefspw[counter]=defspws[k];
	++counter;
      }
    }
    
    ftclrec.define("spws", newdefspw);
    return True;
  }

  Bool VisModelData::addToRec(Record& therec, const Vector<Int>& spws){

    Int numft=0;
    Int numcl=0;
    Vector<Bool> hasSpw(spws.nelements(), False);
    if(therec.isDefined("numft")){
      numft=therec.asInt("numft");
      Vector<Int> ft_toremove(numft, 0);
      for(Int k=0; k < numft; ++k){
	const Record& ftrec=therec.asRecord("ft_"+String::toString(k));
	const Vector<Int>& ftspws=ftrec.asArrayInt("spws");
	for (uInt i=0; i<spws.nelements(); ++i){
	  for (uInt j=0; j<ftspws.nelements(); ++j){
	    if(spws[i]==ftspws[j]){
	      hasSpw[i]=True;	   
	      ft_toremove[k]=1;
	    }
	  }	
	}
      }
      if(sum(ft_toremove) >0){
	for(Int k=0; k < numft; ++k){
	  if(ft_toremove[k]==1)
	    therec.removeField("ft_"+String::toString(k));
	}
	numft=numft-sum(ft_toremove);
	therec.define("numft", numft);
	Int id=0;
	for(uInt k=0; k < therec.nfields(); ++k){
	  if(therec.name(k).contains("ft_")){
	    therec.renameField("ft_"+String::toString(id), k);
	    ++id;
	  }
	}
      }
    }
    if(therec.isDefined("numcl")){
      numcl=therec.asInt("numcl");
      Vector<Int> cl_toremove(numcl, 0);
      for(Int k=0; k < numcl; ++k){
	const Record& clrec=therec.asRecord("cl_"+String::toString(k));
	const Vector<Int>& clspws=clrec.asArrayInt("spws");
	for (uInt i=0; i<spws.nelements(); ++i){
	  for (uInt j=0; j<clspws.nelements(); ++j){
	    if(spws[i]==clspws[j]){
	      hasSpw[i]=True;	    
	      cl_toremove[k]=1;
	    }
	  }	
	}
      }
      if(sum(cl_toremove) >0){
	for(Int k=0; k < numcl; ++k){
	  if(cl_toremove[k]==1)
	    therec.removeField("cl_"+String::toString(k));
	}
	numcl=numcl-sum(cl_toremove);
	therec.define("numcl", numcl);
	Int id=0;
	for(uInt k=0; k < therec.nfields(); ++k){
	  if(therec.name(k).contains("cl_")){
	    therec.renameField("cl_"+String::toString(id), k);
	    ++id;
	  }
	}
      }
    }
    return (!allTrue(hasSpw) || ((numft+numcl)>0));
  }

void VisModelData::putModel(const MeasurementSet& thems, const RecordInterface& rec, const Vector<Int>& validfieldids, const Vector<Int>& spws, const Vector<Int>& starts, const Vector<Int>& nchan,  const Vector<Int>& incr, Bool iscomponentlist, Bool incremental){

    //A field can have multiple FTmachines and ComponentList associated with it 
    //For example having many flanking images for the model
    //For componentlist it may have multiple componentlist ...for different spw
  //Timer tim;
  //tim.mark();
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
    Record outRec; 
    Bool addtorec=False;
    Table newTab(thems);
    if(newTab.rwKeywordSet().isDefined(elkey)){
      outRec=newTab.rwKeywordSet().asRecord(elkey); 
      //if incremental no need to check & remove what is in the record
      if(!incremental)
	addtorec=addToRec(outRec, spws);
    }
    incremental=incremental || addtorec;
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
    
    // tim.show("Time taken to save record ");
 
}





  void VisModelData::addModel(const Record& rec,  const Vector<Int>& /*msids*/, const VisBuffer& vb){
    


    Int indexft=-1;
    if(rec.isDefined("numft")){
      Int numft=rec.asInt("numft");
      if(numft >0){
	for(Int ftk=0; ftk < numft; ++ftk){
	  Record ftrec(rec.asRecord("ft_"+String::toString(ftk)));
	  Vector<Int>fields;
	  Vector<Int> spws;
	  ftrec.get("fields", fields);
	  ftrec.get("spws", spws);
	  if(anyEQ(spws, vb.spectralWindow())){
	    indexft=ftholder_p.nelements();
	    ftholder_p.resize(indexft+1, False, True);
	    ftholder_p[indexft].resize(1);
	    ftholder_p[indexft][0]=NEW_FT(ftrec.asRecord("container"));
	    ftholder_p[indexft][0]->initMaps(vb);
	    
	    for( uInt fi=0; fi < fields.nelements(); ++fi){
	      for(uInt spi=0; spi < spws.nelements(); ++spi){
		Int indx=-1;
		Int ftindx=-1;
		if(hasModel(vb.msId(), fields[fi], spws[spi]) && (ftindex_p(spws[spi], fields[fi], vb.msId()) >= 0 )){
		 
		  indx=ftindex_p(spws[spi], fields[fi], vb.msId());
		  ftindx=ftholder_p[indx].nelements();
		  Bool alreadyAdded=False;
		  for (Int kk=1; kk < ftindx; ++kk){
		    alreadyAdded= alreadyAdded || (ftholder_p[indexft][0]==ftholder_p[indx][kk]);
		  }
		  if(!alreadyAdded){
		    ftholder_p[indx].resize(ftindx+1, True);
		    ftholder_p[indx][ftindx]=ftholder_p[indexft][0];
		  }
		}
		else{
		  ftindex_p(spws[spi], fields[fi], vb.msId())=indexft;
		}
	      }
	    }
	  }
	  else{
	    if(hasModel(vb.msId(), vb.fieldId(), vb.spectralWindow()) < 0)
	      ftindex_p(vb.spectralWindow(), vb.fieldId(), vb.msId())=-2;
	  }

	  
	}
      }	      
    }
    Int indexcl=-1;
    if(rec.isDefined("numcl")){
      Int numcl=rec.asInt("numcl");
      if(numcl >0){
	for(Int clk=0; clk < numcl; ++clk){
	  Vector<Int>fields;
	  Vector<Int> spws;
	  Record clrec(rec.asRecord("cl_"+String::toString(clk)));
	  clrec.get("fields", fields);
	  clrec.get("spws", spws);
	  if(anyEQ(spws, vb.spectralWindow())){
	    indexcl=clholder_p.nelements();
	    clholder_p.resize(indexcl+1, False, True);
	    clholder_p[indexcl].resize(1);
	    clholder_p[indexcl][0]=new ComponentList();
	    String err;
	    if(!((clholder_p[indexcl][0])->fromRecord(err, clrec.asRecord("container"))))
	      throw(AipsError("Component model failed to load for field "+String::toString(fields)));
	    for( uInt fi=0; fi < fields.nelements(); ++fi){
	      for(uInt spi=0; spi < spws.nelements(); ++spi){
		Int indx=-1;
		Int clindx=-1;
		if(hasModel(vb.msId(), fields[fi], spws[spi]) && (clindex_p(spws[spi], fields[fi], vb.msId()) >= 0 )){
		  indx=clindex_p(spws[spi], fields[fi], vb.msId());
		  clindx=clholder_p[indx].nelements();
		  Bool alreadyAdded=False;
		  for (Int kk=1; kk < clindx; ++kk){
		    alreadyAdded= alreadyAdded || (clholder_p[indexcl][0]==clholder_p[indx][kk]);
		  } 
		  if(!alreadyAdded){
		    clholder_p[indx].resize(clindx+1, True);
		    clholder_p[indx][clindx]=clholder_p[indexcl][0];
		  }
		}
		else{
		  clindex_p(spws[spi], fields[fi], vb.msId())=indexcl;
		}
	      }
	    }
	  }
	  else{
	    if(hasModel(vb.msId(), vb.fieldId(), vb.spectralWindow()) < 0)
	      clindex_p(vb.spectralWindow(), vb.fieldId(), vb.msId())=-2;
	  }

	}
      }
    }


  }

  FTMachine* VisModelData::NEW_FT(const Record& ftrec){
    String name=ftrec.asString("name");
    if(name=="GridFT")
      return new GridFT(ftrec);
    if(name=="rGridFT")
      return new rGridFT(ftrec);
    if(name=="WProjectFT")
      return new WProjectFT(ftrec);
    if(name=="MultiTermFT")
      return new MultiTermFT(ftrec);
    if(name=="MosaicFT")
      return new MosaicFT(ftrec);
    if(name=="SetJyGridFT")
      return new SetJyGridFT(ftrec);
    return NULL;
  }

  Int VisModelData::hasModel(Int msid, Int field, Int spw){

    IPosition oldcubeShape=ftindex_p.shape();
    if(oldcubeShape(0) <(spw+1) || oldcubeShape(1) < (field+1) || oldcubeShape(2) < (msid+1)){
      Cube<Int> newind(max((spw+1), oldcubeShape(0)), max((field+1),oldcubeShape(1)) , max((msid+1), oldcubeShape(2)));
      newind.set(-1);
      newind(IPosition(3, 0,0,0), (oldcubeShape-1))=ftindex_p;
      ftindex_p.assign(newind);
      newind.set(-1);
      newind(IPosition(3, 0,0,0), (oldcubeShape-1))=clindex_p;
      clindex_p.assign(newind);
    }

    if( (clindex_p(spw, field, msid) + ftindex_p(spw, field, msid)) < -2)
      return -2;
    else if( (clindex_p(spw, field, msid) ==-1)  &&  (ftindex_p(spw, field, msid) ==-1))
      return -1;
    return 1;


  }

   void VisModelData::initializeToVis(){
    

  }
  Bool VisModelData::getModelVis(VisBuffer& vb){

    Vector<CountedPtr<ComponentList> >cl=getCL(vb.msId(), vb.fieldId(), vb.spectralWindow());
    Vector<CountedPtr<FTMachine> > ft=getFT(vb.msId(), vb.fieldId(), vb.spectralWindow());
    //Fill the buffer with 0.0; also prevents reading from disk if MODEL_DATA exists
    ///Oh boy this is really dangerous...
    //nCorr etc are public..who know who changed these values before reaching here.
    Cube<Complex> mod(vb.nCorr(), vb.nChannel(), vb.nRow(), Complex(0.0));
    vb.setModelVisCube(mod);
    Bool incremental=False;
    if( cl.nelements()>0){
      // cerr << "In cft " << cl.nelements() << endl;
      for (uInt k=0; k < cl.nelements(); ++k)
	if(!cl[k].null()){
	  cft_p->get(vb, *(cl[k]), -1); 
      //cerr << "max " << max(vb.modelVisCube()) << endl;
	  incremental=True;
	}
    }
    if(ft.nelements()>0){
      Cube<Complex> tmpModel;
      if(incremental || ft.nelements() >1)
	tmpModel.assign(vb.modelVisCube());
      Bool allnull=True;
      for (uInt k=0; k < ft.nelements(); ++k){
	if(!ft[k].null()){
	  if(k >0) vb.setModelVisCube(Cube<Complex> (vb.nCorr(), vb.nChannel(), vb.nRow(), Complex(0.0)));
    
	  ft[k]->get(vb, -1);
	  if(ft.nelements()>1 || incremental){
	    tmpModel+=vb.modelVisCube();
	  }
	  allnull=False;
	}
      }
      //cerr << "min max after ft " << min(vb.modelVisCube()) << max(vb.modelVisCube()) << endl;
      if(!allnull){
	if(ft.nelements()>1 || incremental)
	  vb.modelVisCube()=tmpModel;
	incremental=True;
      }      
    }
    if(!incremental){
      //No model was set so....
      ///Set the Model to 1.0 for parallel hand and 0.0 for x-hand
      
      vb.modelVisCube().set(Complex(1.0));
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
    //cerr << "indx " << indx << "   " << clholder_p[indx].nelements() <<  " spw " << spwId << endl;
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

