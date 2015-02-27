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


//-----------------------------------------------------------------------------
// CONFIGURATION SHOULD BE SET HERE


// JUSTO:  Set which MS you want to use here:

// MS to use:
//String msname="ngc5921.ms";      // This does have CORRECTED_DATA column
String msname="ngc5921_noCD.ms";   // This does NOT have CORRECTED_DATA column

//String msname="Four_ants_3C286.ms";  // (This has CORRECTED_DATA, and has more iterations)


//  JUSTO:  This datacol variable is used below when setting up the MSTransformIteratorFactory...

// Set which datacol(s) is arranged for the MSTransformIteratorFactory configuration
//String datacol("ALL");      //  this works ok (despite MSTransformDataHandler?)
String datacol("CORRECTED");  //  this is trapped in MSTransformDataHandler if no physical CORRECTED_DATA column

      //  NB: Strictly, we need to set "datacolumn" to "ALL" above to make sure both 
      //      DATA and CORRECTED are both properly averaged (because we ask for both
      //      from the VB2 below).    However, the point of using only 
      //      CORRECTED is demonstrate a typical case for plotms (only CORRECTED)
      //      and to show that there are still traps in MSTransformDataHandler
      //      that forbid this if the physical column does not exist. 


// Simple factor to calibrate with
Float calfactor(100.0);

// Averaging timescale (where relevant)
Double avetime(5000.0);  // non-trivial
//Double avetime(0.1);  // trivial  (averaging code will be exercised, but no reduction in volume)

// Should summary of each VB be printed out?
Bool verbose(True);

// Control which tests are done
Bool doCalVi2synth(False);  // Ordinary CalibratingVi2 test via synthesis interfaces
Bool doCalVi2msvis(False);  // Ordinary CalibratingVi2 test via msvis interfaces (incl. static init)
Bool doLayeredVi2(True);    // LayeredVi2Factory test, w/ cal and averaging
Bool doMSTransVi2(True);    // MSTransformIteratorFactory test, w/ cal and averaging

// END OF CONFIGURATION
//-----------------------------------------------------------------------------

// A function to test VB contents, etc.
Bool testVB(VisBuffer2* vb, Float calfactor, Bool verbose) {

  // A is corr/data/calfactor which should = 1.0
  Cube<Float> A=amplitude(vb->visCubeCorrected()/vb->visCube())/calfactor;
  A(vb->flagCube())=1.0;  // make sure flagged samples are 1.0
  Float Am=mean(A);
  Float Ae=stddev(A);
  
  // W is  wt(corr)/wt(data)*calfactor^2, which should =1.0
  Matrix<Float> W=vb->weight()*square(vb->sigma())*calfactor*calfactor;
  //  W(W==0.0)=1.0;  // make any zeros 1.0
  Float Wm=mean(W);
  Float We=stddev(W);
  
  if (verbose) {
    // Report on the attached vb
    cout << " nR="<< vb->nRows()
	 << " nCh="<< vb->nChannels()
	 << " nC="<< vb->nCorrelations()
	 << " fldId="<< vb->fieldId()(0)
	 << " scan#="<< vb->scan()(0)
	 << " spwId="<< vb->spectralWindows()(0) << " (DDID="<<vb->dataDescriptionIds()(0)<<")"
	 << " time="<<MVTime(vb->time()(0)/C::day).string(MVTime::YMD,7)
      	 << " nfl="<< ntrue(vb->flagCube())
	 << " Amp="<< max(amplitude(vb->visCube()))
	 << "->"<< max(amplitude(vb->visCubeCorrected()))
	 << " <A>="<< Am << "+/-" << Ae
	 << " W="<< max(1.0f/square(vb->sigma()))
	 << "->"<< max(vb->weight())
	 << " <W>="<< Wm << "+/-" << We
	 << endl;

  }

  return True;  // will add actual tests later
}

Bool doIteration(VisibilityIterator2* vi, Float calfactor, Bool verbose) {

  vi::VisBuffer2 *vb = vi->getVisBuffer();

  Int iChunk(0);
  for(vi->originChunks();vi->moreChunks();++iChunk,vi->nextChunk()) {
    Int iIter(0);
    for (vi->origin();vi->more();++iIter,vi->next()) {
      //cout << "Chunk="<<iChunk<<" Iter="<<iIter<<endl;
      testVB(vb,calfactor,verbose);
    }
  }
  return True;
}


int main() {

  Timer timer;
  try {

    cout << endl <<  "Global Config:" << endl;
    cout << " msname    = " << msname << endl;
    cout << " calfactor = " << calfactor << endl;
    cout << " avetime   = " << avetime << endl;
    cout << " datacol   = " << datacol << endl;
    cout << " verbose   = " << boolalpha << verbose << endl << endl;

    // make the MS object for use below
    MeasurementSet ms(msname, Table::Update);

    // make some sort columns (this matches MSTransform default, apparently)
    Block<int> sort(5);
    Int icol(0);
    sort[icol++] = MS::ARRAY_ID;
    sort[icol++] = MS::SCAN_NUMBER;
    sort[icol++] = MS::FIELD_ID;
    sort[icol++] = MS::DATA_DESC_ID;
    sort[icol++] = MS::TIME;
    SortColumns sc(sort);

    // Now do each test

    //-----------------------------------------------------------
    // Basic use of the CalibratingVi2 (alone, via synthesis)
    if (doCalVi2synth)
    {
      String title="CalibratingVi2 via synthesis";
      cout << endl << "Testing "+title+"------------------------------" << endl;

      timer.mark();

      // Arrange iteration
      IteratingParameters iterpar(0.0,sc);

      // Arrange calibration
      CalibratingParameters calpar(calfactor);

      // Construct the VI2
      vi::CalibratingVi2Factory cf(&ms,calpar,iterpar);
      vi::VisibilityIterator2 *cVi = new vi::VisibilityIterator2(cf);

      timer.show(" "+title+" setup: "); 
      timer.mark();

      // Drive the Iteration
      doIteration(cVi,calfactor,verbose);

      // Cleanup
      delete cVi;
      timer.show(" "+title+" iteration: "); 

    }


    //-----------------------------------------------------------
    // Basic use of the CalibratingVi2 (alone, via msvis interfaces)
    if (doCalVi2msvis)
    {
      String title="CalibratingVi2 via msvis interfaces (static init)";
      cout << endl << "Testing "+title+"------------------------------" << endl;

      timer.mark();

      // Arrange iteration
      IteratingParameters iterpar(0.0,sc);

      // Arrange calibration (by Record, since we use msvis interfaces)
      Record calrec;
      calrec.define("calfactor",calfactor);

      // Construct the VI2
      CalibratingVi2FactoryI* CViFI = casa::vi::CalibratingVi2FactoryI::generate(&ms,calrec,iterpar);
      vi::VisibilityIterator2 *cVi = new vi::VisibilityIterator2(*CViFI);
      delete CViFI;

      timer.show(" "+title+" setup: "); 
      timer.mark();

      // Drive the Iteration
      doIteration(cVi,calfactor,verbose);

      // Cleanup
      delete cVi;
      timer.show(" "+title+" iteration: "); 

    }

    //-----------------------------------------------------------
    // Use of LayeredVi2Factory to combine calibration and time averaging 
    if (doLayeredVi2) 
    {

      String title="LayeredVi2Factory with cal+ave";
      cout << endl << "Testing "+title+"------------------------------" << endl;

      timer.mark();

      // Arrange iteration
      IteratingParameters iterpar(0.0,sc);
						
      // Arrange calibration (by Record since we use msvis interfaces)
      Record calrec;
      calrec.define("calfactor",calfactor);

      // Arrange for averaging
      vi::AveragingOptions aveopt(vi::AveragingOptions::AverageCorrected|
				  vi::AveragingOptions::CorrectedFlagWeightAvgFromWEIGHT|
				  vi::AveragingOptions::AverageObserved|
				  vi::AveragingOptions::ObservedFlagWeightAvgFromSIGMA);
      vi::AveragingParameters avepar(avetime,0.0,SortColumns(),aveopt); 

      // We can set up with many combinations
      //vi::LayeredVi2Factory lf(&ms,&iterpar,Record(),NULL);    // only plain iteration
      //vi::LayeredVi2Factory lf(&ms,&iterpar,calrec,NULL);      // only calibration
      //vi::LayeredVi2Factory lf(&ms,&iterpar,Record(),&avepar); // only averaging
      vi::LayeredVi2Factory lf(&ms,&iterpar,calrec,&avepar);     // cal _and_ averaging

      // Create the VI2
      vi::VisibilityIterator2 *lVi = new vi::VisibilityIterator2(lf);
      // Needed?  // lVi->setWeightScaling(vi::WeightScaling::generateUnityWeightScaling());

      timer.show(" "+title+" setup: "); 
      timer.mark();

      // Drive the Iteration
      doIteration(lVi,calfactor,verbose);

      // Cleanup
      delete lVi;
      timer.show(" "+title+" iteration: "); 
    }

    //-----------------------------------------------------------
    //  Use MSTranformIteratorFactory w/ calibration and averaging
    if (doMSTransVi2)
    {
      String title="MSTransformIteratorFactory with cal+ave";
      cout << endl << "Testing "+title+"------------------------------" << endl;

      cout << "(Setup is same as 'Testing LayeredVi2Factory with cal+ave')" << endl;


      timer.mark();

      // Arrange calibration (by Record since we use mstransform interfaces)
      Record calrec;
      calrec.define("calfactor",calfactor);

      // The configuration Record for MSTransformIteratorFactory
      Record config;
      config.define("inputms",msname);
      config.define("datacolumn",datacol);  //  from above
      config.define("timeaverage",True);
      config.define("timebin",String::toString(avetime)+"s"); 
      config.defineRecord("callib",calrec);

      // Construct the VI2
      MSTransformIteratorFactory mstf(config);
      vi::VisibilityIterator2 *mstVi = new vi::VisibilityIterator2(mstf);
      
      timer.show(" "+title+" setup: "); 
      timer.mark();

      // Drive the Iteration
      doIteration(mstVi,calfactor,verbose);

      // Cleanup
      delete mstVi;
      timer.show(" "+title+" iteration: "); 

    }

  } 
  catch (const AipsError &x) {
    cerr << "***Exception:" << endl;
    cerr << x.getMesg() << endl;
  }
  
  //  timer.show("Done.  "); cout<<endl<<endl<<endl;
  return 0;  
}


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

