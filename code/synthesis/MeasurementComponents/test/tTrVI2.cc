//# tTrVI2.cc: Tests the TransformingVI stuff
//# Copyright (C) 1995,1999,2000,2001
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

#include <casa/aips.h>
#include <casa/Exceptions/Error.h>
#include <casa/iostream.h>
#include <msvis/MSVis/VisibilityIterator2.h>
#include <msvis/MSVis/VisibilityIteratorImpl2.h>
#include <msvis/MSVis/VisBuffer2.h>
#include <msvis/MSVis/IteratingParameters.h>
#include <msvis/MSVis/AveragingVi2Factory.h>
#include <msvis/MSVis/LayeredVi2Factory.h>
#include <mstransform/MSTransform/MSTransformIteratorFactory.h>
#include <synthesis/MeasurementComponents/CalibratingVi2.h>
#include <synthesis/MeasurementComponents/CalibratingVi2Factory.h>
#include <casa/Quanta/MVTime.h>
#include <casa/OS/Timer.h>
#include <casa/iomanip.h>
#include <casa/namespace.h>
using namespace std;
using namespace casa;
using namespace casa::vi;

//int main(int argc, char **argv) {
int main() {

  Bool verbose(True);

  // JUSTO:  Set which MS you want to use here:
  //String msname="ngc5921.ms";            // This does have CORRECTED_DATA column
  String msname="ngc5921_noCD.ms";   // This does NOT have CORRECTED_DATA column

  //String msname="Four_ants_3C286.ms";  // (This has CORRECTED_DATA, and has more iterations)

  Timer timer;
  try {

    timer.mark();
    //MS ms(argv1], Table::Update);
    MeasurementSet ms(msname, Table::Update);

    Block<int> sort(5);
    Int icol(0);
    sort[icol++] = MS::ARRAY_ID;
    sort[icol++] = MS::SCAN_NUMBER;
    sort[icol++] = MS::FIELD_ID;
    sort[icol++] = MS::DATA_DESC_ID;
    sort[icol++] = MS::TIME;

    SortColumns sc(sort);

    //-----------------------------------------------------------
    // Basic use of the CalibratingVi2 (alone, via synthesis)
    if (False)
    {

      timer.mark();
      IteratingParameters iterpar(0.0,sc);
      Float calfactor(1000.0);
      CalibratingParameters calpar(calfactor);

      vi::CalibratingVi2Factory cf(&ms,calpar,iterpar);
      vi::VisibilityIterator2 *cVi = new vi::VisibilityIterator2(cf);
      vi::VisBuffer2 *cvb = cVi->getVisBuffer();

      Int iChunk(0);
      for(cVi->originChunks();cVi->moreChunks();cVi->nextChunk()) {
	Int iIter(0);
	for (cVi->origin();cVi->more();cVi->next()) {	  

	  // ratio should be 1.0, with error ~<float precision
	  Cube<Float> ratio=amplitude(cvb->visCubeCorrected()/cvb->visCube())/calfactor;
	  ratio(cvb->flagCube())=1.0;  // make sure flagged samples are 1.0
	  Float mratio=mean(ratio);
	  Float eratio=stddev(ratio);

	  // Report on the attached vb
	  if (verbose) {
	    cout << " nRows="<< cvb->nRows()
		 << " nChans="<< cvb->nChannels()
		 << " nCorrs="<< cvb->nCorrelations()
		 << " arrId="<< cvb->arrayId()(0)
		 << " fldId="<< cvb->fieldId()(0)
		 << " scan#="<< cvb->scan()(0)
		 << " spwId="<< cvb->spectralWindows()(0) << " (DDID="<<cvb->dataDescriptionIds()(0)<<")"
		 << " time="<<MVTime(cvb->time()(0)/C::day).string(MVTime::YMD,7)
		 << " shape="<< cvb->visCube().shape()
		 << " max(amp(data))="<< max(amplitude(cvb->visCube()))
		 << " max(amp(model))="<< max(amplitude(cvb->visCubeModel()))
		 << " max(amp(corr))="<< max(amplitude(cvb->visCubeCorrected()))
		 << " ratio="<< mratio << "+/-" << eratio
		 << endl;
	  }
	  ++iIter;
	}
	++iChunk;
      }
      delete cVi;
      timer.show(" CVI: "); 

    }


    //-----------------------------------------------------------
    // Basic use of the CalibratingVi2 (alone, via msvis interfaces)
    if (False)
    {

      IteratingParameters iterpar(0.0,sc);

      Float calfactor(10000.0);
      Record calrec;
      calrec.define("calfactor",calfactor);

      CalibratingVi2FactoryI* CViFI = casa::vi::CalibratingVi2FactoryI::generate(&ms,calrec,iterpar);
      vi::VisibilityIterator2 *cVi = new vi::VisibilityIterator2(*CViFI);
      delete CViFI;

      vi::VisBuffer2 *cvb = cVi->getVisBuffer();

      Int iChunk(0);
      for(cVi->originChunks();cVi->moreChunks();cVi->nextChunk()) {
	Int iIter(0);
	for (cVi->origin();cVi->more();cVi->next()) {	  
	  // Report on the attached vb

	  // ratio should be 1.0, with error ~<float precision
	  Cube<Float> ratio=amplitude(cvb->visCubeCorrected()/cvb->visCube())/calfactor;
	  ratio(cvb->flagCube())=1.0;  // make sure flagged samples are 1.0
	  Float mratio=mean(ratio);
	  Float eratio=stddev(ratio);
	  
	  if (verbose) {
	    cout << " nRows="<< cvb->nRows()
		 << " nChans="<< cvb->nChannels()
		 << " nCorrs="<< cvb->nCorrelations()
		 << " arrId="<< cvb->arrayId()(0)
		 << " fldId="<< cvb->fieldId()(0)
		 << " scan#="<< cvb->scan()(0)
		 << " spwId="<< cvb->spectralWindows()(0) << " (DDID="<<cvb->dataDescriptionIds()(0)<<")"
		 << " time="<<MVTime(cvb->time()(0)/C::day).string(MVTime::YMD,7)
		 << " shape="<< cvb->visCube().shape()
		 << " max(amp(data))="<< max(amplitude(cvb->visCube()))
		 << " max(amp(corr))="<< max(amplitude(cvb->visCubeCorrected()))
		 << " ratio="<< mratio << "+/-" << eratio
		 << endl;
	  }
	  ++iIter;
	}
	++iChunk;
      }
      delete cVi;
      timer.show(" CVI by static init: "); 

    }

    //-----------------------------------------------------------
    // Use of LayeredVi2Factory to combine calibration and time averaging 
    if (True)
    {

      cout << endl << "Using LayeredVi2Factory------------------------" << endl;

      timer.mark();

      IteratingParameters iterpar(0.0,sc);

      Float calfactor(10000.0);
      Record calrec;
      calrec.define("calfactor",calfactor);

      vi::AveragingOptions aveopt(vi::AveragingOptions::AverageCorrected|
				  vi::AveragingOptions::CorrectedFlagWeightAvgFromWEIGHT|
				  vi::AveragingOptions::AverageObserved|
				  vi::AveragingOptions::ObservedFlagWeightAvgFromSIGMA);
      vi::AveragingParameters avepar(5000.0,0.0,SortColumns(),aveopt); 

      // We can set up with many combinations
      //vi::LayeredVi2Factory lf(&ms,&iterpar,Record(),NULL);    // only plain iteration
      //vi::LayeredVi2Factory lf(&ms,&iterpar,calrec,NULL);      // only calibration
      //vi::LayeredVi2Factory lf(&ms,&iterpar,Record(),&avepar); // only averaging
      vi::LayeredVi2Factory lf(&ms,&iterpar,calrec,&avepar);     // cal _and_ averaging

      vi::VisibilityIterator2 *lVi = new vi::VisibilityIterator2(lf);
      lVi->setWeightScaling(vi::WeightScaling::generateUnityWeightScaling());

      vi::VisBuffer2 *lvb = lVi->getVisBuffer();

      Int iChunk(0);
      for(lVi->originChunks();lVi->moreChunks();lVi->nextChunk()) {
	Int iIter(0);
	for (lVi->origin();lVi->more();lVi->next()) {
	  //cout << "Chunk="<<iChunk<<" Iter="<<iIter<<endl;

	  // ratio should be 1.0, with error ~<float precision
	  Cube<Float> ratio=amplitude(lvb->visCubeCorrected()/lvb->visCube())/calfactor;
	  ratio(lvb->flagCube())=1.0;  // make sure flagged samples are 1.0
	  Float mratio=mean(ratio);
	  Float eratio=stddev(ratio);

	  // Report on the attached vb
	  if (verbose) {
	    cout << " nRows="<< lvb->nRows()
		 << " nChans="<< lvb->nChannels()
		 << " nCorrs="<< lvb->nCorrelations()
		 << " arrId="<< lvb->arrayId()(0)
		 << " fldId="<< lvb->fieldId()(0)
		 << " scan#="<< lvb->scan()(0)
		 << " spwId="<< lvb->spectralWindows()(0) << " (DDID="<<lvb->dataDescriptionIds()(0)<<")"
		 << " time="<<MVTime(lvb->time()(0)/C::day).string(MVTime::YMD,7)
		 << " shape="<< lvb->visCube().shape()
		 << " max(amp(data))="<< max(amplitude(lvb->visCube()))
		 << " max(amp(corr))="<< max(amplitude(lvb->visCubeCorrected()))
		 << " ratio="<< mratio << "+/-" << eratio
		 << endl;
	  }
	  ++iIter;
	}
	++iChunk;
      }
      delete lVi;
      timer.show(" LVI: "); 
    }

    //-----------------------------------------------------------
    //  Use MSTranformIteratorFactory w/ calibration and averaging
    if (True)
    {
      cout << endl << "Using MSTransformIteratorFactory------------------------" << endl;

      // Use MSTransformIteratorFactory

      Float calfactor(10000.0);
      Record calrec;
      calrec.define("calfactor",calfactor);

      Record config;



      // JUSTO:

      //config.define("datacolumn","ALL");      //  this works ok
      config.define("datacolumn","CORRECTED");  //  this is trapped in MSTransformDataHandler if no physical CORRECTED_DATA column

      //  NB: Strictly, we need to set "datacolumn" to "ALL" above to make sure both 
      //      DATA and CORRECTED are both properly averaged (because we ask for both
      //      from the VB2 below.    .   The point of using only 
      //      CORRECTED is demonstrate a typical case for plotms (only CORRECTED)
      //      and to show that there are still traps in MSTransformDataHandler
      //      that forbid this if the physical column does not exist. 




      config.define("inputms",msname);
      config.define("timeaverage",False);
      config.define("timeaverage",True);
      config.define("timebin","5000.0s");
      config.defineRecord("callib",calrec);
      
      MSTransformIteratorFactory mstf(config);
      vi::VisibilityIterator2 *mstVi = new vi::VisibilityIterator2(mstf);
      
      vi::VisBuffer2 *mstvb = mstVi->getVisBuffer();

      Int iChunk(0);
      for(mstVi->originChunks();mstVi->moreChunks();mstVi->nextChunk()) {
	Int iIter(0);
	for (mstVi->origin();mstVi->more();mstVi->next()) {
	  //	  cout << "Chunk="<<iChunk<<" Iter="<<iIter<<endl;

	  // ratio should be 1.0, with error ~<float precision
	  Cube<Float> ratio=amplitude(mstvb->visCubeCorrected()/mstvb->visCube())/calfactor;
	  ratio(mstvb->flagCube())=1.0;  // make sure flagged samples are 1.0
	  Float mratio=mean(ratio);
	  Float eratio=stddev(ratio);

	  if (verbose) {
	    // Report on the attached vb
	    cout << " nRows="<< mstvb->nRows()
		 << " nChans="<< mstvb->nChannels()
		 << " nCorrs="<< mstvb->nCorrelations()
		 << " arrId="<< mstvb->arrayId()(0)
		 << " fldId="<< mstvb->fieldId()(0)
		 << " scan#="<< mstvb->scan()(0)
		 << " spwId="<< mstvb->spectralWindows()(0) << " (DDID="<<mstvb->dataDescriptionIds()(0)<<")"
		 << " time="<<MVTime(mstvb->time()(0)/C::day).string(MVTime::YMD,7)
		 << " shape="<< mstvb->visCube().shape()
		 << " max(amp(data))="<< max(amplitude(mstvb->visCube()))
		 << " max(amp(corr))="<< max(amplitude(mstvb->visCubeCorrected()))
		 << " ratio="<< mratio << "+/-" << eratio
		 << endl;
	  }
	  ++iIter;
	}
	++iChunk;
      }
      delete mstVi;
      timer.show(" MSTVI: "); 


    }


  } 
  catch (const AipsError &x) {
    cerr << "***Exception:" << endl;
    cerr << x.getMesg() << endl;
  }
  
  //  timer.show("Done.  "); cout<<endl<<endl<<endl;
  return 0;  }


//----------------------------------------------------------------
// Stuff below this line is not used
/*


    //-----------------------------------------------------------
    // this is just ordinary plane iteration
    if (False)  // not really needed in this test
    {

      timer.mark();
      
      VisibilityIterator2 vi(ms,sc,False);
      VisBuffer2* vb = vi.getVisBuffer();
      
      Int iChunk(0);

      for (vi.originChunks();vi.moreChunks();vi.nextChunk()) {
	Int iIter(0);
	for (vi.origin();vi.more();vi.next()) {
	  // Report on the attached vb
	  cout << " nRows="<< vb->nRows()
	       << " nChans="<< vb->nChannels()
	       << " nCorrs="<< vb->nCorrelations()
	       << " arrId="<<vb->arrayId()(0)
	       << " fldId="<<vb->fieldId()(0)
	       << " scan#="<<vb->scan()(0)
	       << " spwId="<<vb->spectralWindows()(0) << " (DDID="<<vb->dataDescriptionIds()(0)<<")"
	       << " time="<<MVTime(vb->time()(0)/C::day).string(MVTime::YMD,7)
	       << " shape="<< vb->visCube().shape()
	       << " max(amp(data))="<< max(amplitude(vb->visCube()))
	       << " max(amp(model))="<< max(amplitude(vb->visCubeModel()))
	       << " max(amp(corr))="<< max(amplitude(vb->visCubeCorrected()))
	       << endl;
	  
	  iIter++;  
	}
	iChunk++;  
      }
      timer.show(" VI: "); 
    }
*/

