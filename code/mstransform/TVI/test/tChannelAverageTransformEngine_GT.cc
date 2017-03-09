//# tCalibraterSolve_GT:  test Calibrater-conducted solving
//# Copyright (C) 1995,1999,2000,2001,2016
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
#include <casacore/casa/OS/EnvVar.h>
#include <casa/iostream.h>
#include <mstransform/TVI/ChannelAverageTVI.h>
#include <mstransform/TVI/UtilsTVI.h>
#include <casa/Arrays/ArrayMath.h>
#include <casa/Arrays/ArrayLogical.h>
#include <casa/Arrays/VectorIter.h>
#include <casa/iomanip.h>
#include <gtest/gtest.h>


#ifdef _OPENMP
 #include <omp.h>
#endif

using namespace std;
using namespace casa;
using namespace casacore;
using namespace casa::vi;

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}




// Duplicate of essential part of FreqAxisTVI::transformFreqAxis2
//   to enable comparison
template <class T>
void transformFreqAxis2(const IPosition &inputShape,
			FreqAxisTransformEngine2<T> &transformer) {

  casacore::uInt nRows = inputShape(2);
  casacore::uInt nCorrs = inputShape(0);
  for (casacore::uInt row=0; row < nRows; row++) {
    transformer.setRowIndex(row);  // Calls Cube<T>::xyPlane
    for (casacore::uInt corr=0; corr < nCorrs; corr++) {
      transformer.setCorrIndex(corr);  // Calls Matrix<T>::row
      transformer.transform();
    }
  }
  return;
}


TEST(ChanAveTransEngine,test1) {

  Double Tvis1(0.0),Tvis2(0.0),Tmod1(0.0),Tmod2(0.0);
  Double Twt1(0.0),Twt2(0.0),Tfl1(0.0),Tfl2(0.0);

  Int N(500);
  Int nCor(4),nChan(64),nRow(351);

  IPosition ish(3,nCor,nChan,nRow);

  // Input data
  Cube<Complex> ivis(ish,Complex(1.0f));
  Cube<Float> iwt(ish,1.0f);
  Cube<Bool> ifl(ish,false);

  uInt width=nChan; 
  Int nChanOut=Int(ceil(Float(nChan)/Float(width)));


  cout << "nCor=" << nCor << endl
       << "nChan=" << nChan << endl
       << "nRow=" << nRow << endl
       << "width=" << width << endl;

  cout << "Averaging " << nCor*nRow*N 
       << " spectra with nChan=" << nChan 
       << " to nChanOut=" << nChanOut << endl;

  Double time0(0.0);
  for (Int i=0;i<N;++i)  {

  {
      
    // Visibility accumulation
    DataCubeMap iDCM;
    DataCubeHolder<Complex> ivisH(ivis); iDCM.add(MS::DATA,ivisH);
    DataCubeHolder<Float>   iwtH(iwt);   iDCM.add(MS::WEIGHT_SPECTRUM,iwtH);
    DataCubeHolder<Bool>    iflH(ifl);   iDCM.add(MS::FLAG,iflH);

    WeightedChannelAverageKernel<Complex> kernel;

    // Old way
    Cube<Complex> ovis1(nCor,nChanOut,nRow);
    {
      DataCubeMap oDCM;
      DataCubeHolder<Complex> ovisH(ovis1); oDCM.add(MS::DATA,ovisH);
      ChannelAverageTransformEngine<Complex>tr(&kernel,
					       &iDCM,
					       &oDCM,
					       width);
      // Do the work
#ifdef _OPENMP 
      time0=omp_get_wtime();
#endif
      transformFreqAxis2(ish,tr);
#ifdef _OPENMP 
      Tvis1+=omp_get_wtime()-time0;
#endif

    }

    // New way
    Cube<Complex> ovis2(nCor,nChanOut,nRow);
    {

      DataCubeMap oDCM;
      DataCubeHolder<Complex> ovisH(ovis2); oDCM.add(MS::DATA,ovisH);
      ChannelAverageTransformEngine<Complex>tr(&kernel,
					       &iDCM,
					       &oDCM,
					       width);
      // Make it so
#ifdef _OPENMP 
      time0=omp_get_wtime();
#endif
      tr.transformAll();
#ifdef _OPENMP 
      Tvis2+=omp_get_wtime()-time0;
#endif

      //cout << "ovis2=" << ovis2.xyPlane(0) << endl;

    }
    ASSERT_TRUE(allEQ(ovis1,ovis2));


  }
  {
      
    // MODEL Visibility accumulation
    DataCubeMap iDCM;
    DataCubeHolder<Complex> ivisH(ivis); iDCM.add(MS::DATA,ivisH);
    DataCubeHolder<Float>   iwtH(iwt);   iDCM.add(MS::WEIGHT_SPECTRUM,iwtH);
    DataCubeHolder<Bool>    iflH(ifl);   iDCM.add(MS::FLAG,iflH);

    FlaggedChannelAverageKernel<Complex> kernel;

    // Old way
    Cube<Complex> ovis1(nCor,nChanOut,nRow);
    {
      DataCubeMap oDCM;
      DataCubeHolder<Complex> ovisH(ovis1); oDCM.add(MS::DATA,ovisH);
      ChannelAverageTransformEngine<Complex>tr(&kernel,
					       &iDCM,
					       &oDCM,
					       width);
      // Do the work
#ifdef _OPENMP 
      time0=omp_get_wtime();
#endif
      transformFreqAxis2(ish,tr);
#ifdef _OPENMP 
      Tmod1+=omp_get_wtime()-time0;
#endif

    }

    // New way
    Cube<Complex> ovis2(nCor,nChanOut,nRow);
    {

      DataCubeMap oDCM;
      DataCubeHolder<Complex> ovisH(ovis2); oDCM.add(MS::DATA,ovisH);
      ChannelAverageTransformEngine<Complex>tr(&kernel,
					       &iDCM,
					       &oDCM,
					       width);
      // Make it so
#ifdef _OPENMP 
      time0=omp_get_wtime();
#endif
      tr.transformAll();
#ifdef _OPENMP 
      Tmod2+=omp_get_wtime()-time0;
#endif

      //cout << "ovis2=" << ovis2.xyPlane(0) << endl;

    }
    ASSERT_TRUE(allEQ(ovis1,ovis2));


  }

  // Weight accumulation
  {

    DataCubeMap iDCM;
    DataCubeHolder<Float> iwtH(iwt); iDCM.add(MS::DATA,iwtH);
    DataCubeHolder<Bool>  iflH(ifl); iDCM.add(MS::FLAG,iflH);

    ChannelAccumulationKernel<Float> kernel;

    // Old way
    Cube<Float> owt1(nCor,nChanOut,nRow);
    {
      DataCubeMap oDCM;
      DataCubeHolder<Float> owtH(owt1); oDCM.add(MS::DATA,owtH);
      
      ChannelAverageTransformEngine<Float>tr(&kernel,
					       &iDCM,
					       &oDCM,
					       width);
      // Do the work
#ifdef _OPENMP 
      time0=omp_get_wtime();
#endif
      transformFreqAxis2(ish,tr);
#ifdef _OPENMP 
      Twt1+=omp_get_wtime()-time0;
#endif

    }

    // New way
    Cube<Float> owt2(nCor,nChanOut,nRow);
    {
      DataCubeMap oDCM;
      DataCubeHolder<Float> owtH(owt2); oDCM.add(MS::DATA,owtH);
      
      
      ChannelAverageTransformEngine<Float>tr(&kernel,
					     &iDCM,
					     &oDCM,
					     width);
      // Do the work
#ifdef _OPENMP 
      time0=omp_get_wtime();
#endif
      tr.transformAll();
#ifdef _OPENMP 
      Twt2+=omp_get_wtime()-time0;
#endif

    }
    ASSERT_TRUE(allEQ(owt1,owt2));


  }

  // Flag accumulation
  {


    DataCubeMap iDCM;
    DataCubeHolder<Bool> iflH(ifl); iDCM.add(MS::FLAG,iflH);

    LogicalANDKernel<Bool> kernel;

    // Old way
    Cube<Bool> ofl1(nCor,nChanOut,nRow);
    {
      DataCubeMap oDCM;
      DataCubeHolder<Bool> oflH(ofl1); oDCM.add(MS::FLAG,oflH);
      
      ChannelAverageTransformEngine<Bool>tr(&kernel,
					    &iDCM,
					    &oDCM,
					    width);
#ifdef _OPENMP 
      time0=omp_get_wtime();
#endif
      transformFreqAxis2(ish,tr);
#ifdef _OPENMP 
      Tfl1+=omp_get_wtime()-time0;
#endif

    }

    // New way
    Cube<Bool> ofl2(nCor,nChanOut,nRow);
    {
      DataCubeMap oDCM;
      DataCubeHolder<Bool> oflH(ofl2); oDCM.add(MS::FLAG,oflH);
      
      ChannelAverageTransformEngine<Bool>tr(&kernel,
					    &iDCM,
					    &oDCM,
					    width);
#ifdef _OPENMP 
      time0=omp_get_wtime();
#endif
      tr.transformAll();
#ifdef _OPENMP 
      Tfl2+=omp_get_wtime()-time0;
#endif

    }
    ASSERT_TRUE(allEQ(ofl1,ofl2));


  }
  } // N iterations

#ifdef _OPENMP
  cout << "Timings (s):" << endl
       << "  Vis:   Old: " << Tvis1 << "  New:" << Tvis2 
       << "  ratio=" << Tvis1/Tvis2 << endl
       << "  Mod:   Old: " << Tmod1 << "  New:" << Tmod2 
       << "  ratio=" << Tmod1/Tmod2 << endl
       << "  Wt:    Old: " << Twt1  << "  New:" << Twt2 
       << "  ratio=" << Twt1/Twt2 << endl
       << "  Flag:  Old: " << Tfl1  << "  New:" << Tfl2 
       << "  ratio=" << Tfl1/Tfl2 << endl
       << "  Total: Old: " << Tvis1+Tmod1+Twt1+Tfl1  
       << "  New:" << Tvis2+Tmod2+Twt2+Tfl2 
       << "  ratio=" << (Tvis1+Tmod1+Twt1+Tfl1)/(Tvis2+Tmod2+Twt2+Tfl2)
       << endl;
#endif

}
