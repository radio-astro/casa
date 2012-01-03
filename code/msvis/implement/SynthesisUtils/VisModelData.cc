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
#include <casa/Arrays/Cube.h>
#include <casa/Arrays/Vector.h>
#include <components/ComponentModels/ComponentList.h>

#include <msvis/MSVis/VisBuffer.h>
#include <msvis/SynthesisUtils/VisModelData.h>
#include <msvis/SynthesisUtils/FTMachine.h>
#include <msvis/SynthesisUtils/SimpleComponentFTMachine.h>
#include <msvis/SynthesisUtils/GridFT.h>

namespace casa { //# NAMESPACE CASA - BEGIN

  VisModelData::VisModelData(): clholder_p(0), ftholder_p(0), flatholder_p(0), cft_p(NULL){


  }
  VisModelData::VisModelData(const Record& ftmachine, const Vector<Int>& validfieldids, const Vector<Int>& msIds): clholder_p(0), flatholder_p(0), cft_p(NULL){

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
  void VisModelData::initializeToVis(){


  }
  Bool VisModelData::getModelVis(VisBuffer& vb){

    CountedPtr<ComponentList> cl=getCL(vb.msId(), vb.fieldId());
    CountedPtr<FTMachine> ft=getFT(vb.msId(), vb.fieldId());
    //Fill the buffer with 0.0; also prevents reading from disk if MODEL_DATA exists
    vb.setModelVisCube(Complex(0.0,0.0));
    Bool incremental=False;
    if( !cl.null()){
      cerr << "In cft" << endl;
      cft_p->get(vb, *cl, -1); 
      incremental=True;
    }
    if(!ft.null()){
      //if(incremental)
      //	tmpModel.assign(vb.modelVisCube());
      //FTMachines  does an incremental ..saves on memcopy
      ft->get(vb, -1);
      cerr << "min max after ft " << min(vb.modelVisCube()) << max(vb.modelVisCube()) << endl;
      //if(incremental)
      //  vb.modelVisCube()+=tmpModel;
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


  CountedPtr<ComponentList> VisModelData::getCL(const Int msId, const Int fieldId){
    uInt indx=fieldId *(msId+1);
    cerr << "indx " << indx << " nelements " << clholder_p.nelements()  << endl;
    if (indx >= clholder_p.nelements()){
      CountedPtr<ComponentList> nullptr=NULL;
      return nullptr;
    }   
    cerr << "indx " << indx << endl;
    return clholder_p[indx];
	

  }

  CountedPtr<FTMachine> VisModelData::getFT(const Int msId, const Int fieldId){
    uInt indx=fieldId *(msId+1);
    if (indx >= ftholder_p.nelements()){
      CountedPtr<FTMachine> nullptr=NULL;
      return nullptr;
    }
    return ftholder_p[indx];
    

  }




}//# NAMESPACE CASA - END

