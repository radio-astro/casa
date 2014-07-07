//# tVisModelData.cc: Tests the Synthesis model data serving
//# Copyright (C) 2011
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

#include <casa/Arrays/ArrayMath.h>
#include <components/ComponentModels/ComponentList.h>
#include <components/ComponentModels/ComponentShape.h>
#include <components/ComponentModels/Flux.h>
#include <tables/Tables/ExprNode.h>
#include <measures/Measures/MeasTable.h>

#include <synthesis/TransformMachines/VisModelData.h>
#include <synthesis/TransformMachines/FTMachine.h>
#include <synthesis/TransformMachines/GridFT.h>
#include <msvis/MSVis/VisibilityIterator.h>
#include <msvis/MSVis/VisBuffer.h>
#include <casa/OS/Timer.h>
#include <casa/namespace.h>



int
main(int argc, char **argv){


  if (argc<2) {
    cout <<"Usage: tVisModelData ms-table-name  model-image-name"<<endl;
    exit(1);
  }
  try{
    
    MeasurementSet myms(argv[1],Table::Update);
   
    cerr << "ISWritable " << myms.isWritable() << endl;
    
    TableExprNode condition;
    Vector<Int> field(1); field(0)=1; //field(1)=1;
    condition=myms.col("FIELD_ID").in(field);
    MeasurementSet mssel(myms(condition));

    Block<Int> sort(0); // create empty block with sortColumns
    VisibilityIterator vi(mssel,sort);
    VisBuffer vb(vi);
    

    MDirection myDir=vi.msColumns().field().phaseDirMeas(0);
    ComponentList cl;
    SkyComponent otherPoint(ComponentType::POINT);
    otherPoint.flux() = Flux<Double>(0.00001, 0.0, 0.0, 0.00000);
    otherPoint.shape().setRefDirection(myDir);
    cl.add(otherPoint);
    Record container;
    String err;
    cl.toRecord(err, container);
    Record clrec;
    clrec.define("type", "componentlist");
    clrec.define("fields", field);
    clrec.define("spws", Vector<Int>(1, 0));
    clrec.defineRecord("container", container);
    Record outRec;
    outRec.define("numcl", 1);
    outRec.defineRecord("cl_0", clrec);
    VisModelData vm;
    //vm.addModel(outRec, Vector<Int>(1, 0), vb);
    Vector<Int>spws(4);
    indgen(spws);
    vm.putModel(myms, container, field, spws, Vector<Int>(1,0), Vector<Int>(1,63), Vector<Int>(1,1), True, False);
    vm.clearModel(myms, "1", "2");
    if(argc>2){
      PagedImage<Float> modim(argv[2]);
      TempImage<Complex> cmod(modim.shape(), modim.coordinates());
      StokesImageUtil::From(cmod, modim);
      MPosition loc;
      MeasTable::Observatory(loc, vi.msColumns().observation().telescopeName()(0));
      GridFT ftm(1000000, 16, "SF", loc, 1.0, False, False);
      ftm.initializeToVis(cmod, vb);
      Record elrec;
      String err;
      ftm.toRecord(err, elrec, True);
      Record ftrec;
      ftrec.define("type", "ftmachine");
      ftrec.define("fields", Vector<Int>(1, 0));
      ftrec.define("spws", Vector<Int>(1, 0));
      ftrec.defineRecord("container", elrec);
      Record outRec1;
      outRec1.define("numft", 1);
      outRec1.defineRecord("ft_0", ftrec);
      cerr << "Error string for Record " << err << endl;
     
		 
    } 


    
    Timer tm;
    tm.mark();
    vi.origin();
    for (vi.originChunks();vi.moreChunks(); vi.nextChunk()){
      for (vi.origin(); vi.more(); vi++){
	
	//vm.getModelVis(vb);
	cerr << "field " << vb.fieldId() << "  spw " << vb.spectralWindow() << " stddev " << stddev(vb.modelVisCube()) << "   mean " << mean(vb.modelVisCube()) <<" max " <<  max(amplitude(vb.modelVisCube())) << "  min " << min(vb.modelVisCube()) << endl;
	//vb.visCube();
      }
    }
    tm.show("End of iter");
    //VisModelData::clearModel(myms);
 



    } catch (AipsError x) {
    cout << "Caught exception " << endl;
    cout << x.getMesg() << endl;
    return(1);
  }














  cout << "Done" << endl;
  exit(0);

}
