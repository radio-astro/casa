//# tPBMath1DEVLA.cc: Tests the Synthesis model data serving
//# Copyright (C) 2016
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

#include <synthesis/TransformMachines/PBMath1DEVLA.h>
#include <synthesis/TransformMachines/PBMath1DPoly.h>
#include <casa/namespace.h>
#include <casa/OS/Directory.h>
#include <casa/Utilities/Regex.h>
#include <images/Images/PagedImage.h>
#include <coordinates/Coordinates/CoordinateUtil.h>
#include <casacore/casa/Logging/LogIO.h>
#include <iomanip>
using namespace casa;
Int main(){
  CoordinateSystem cs;
  CoordinateUtil::addDirAxes(cs);
  CoordinateUtil::addIAxis(cs);
  CoordinateUtil::addFreqAxis(cs);
  LogIO os;
  IPosition d1, d2;
  Vector<Double> incr=cs.increment();
  cerr << "incr " << incr << endl;
  incr[0] /=60.0;
  incr[1] /=60.0;
  cs.setIncrement(incr);
  cs.list(os, MDoppler::RADIO, d1, d2);
  MDirection thedir(Quantity(0.0, "deg"), Quantity(0.0, "deg"), MDirection::J2000);
  Vector<Double> pixworld=cs.referenceValue();
  PagedImage<Complex> im(IPosition(4, 1000, 1000, 1, 1), cs, "GULU");
  PBMath1DEVLA pbm(Quantity(0.7, "deg"), True, 1.425e9);
  std::vector<double> freqs={1.e9, 2.e9, 4.e9, 8.e9, 16.e9, 32.e9, 48.e9};
  pbm.summary(10);
  std::vector<Complex> refval={Complex(0.998942792416,0), Complex(0.995834052563,0), Complex(0.984491288662,0), Complex(0.939067363739,0), Complex(0.770893454552,0), Complex(0.316897362471,0), Complex(0.0408035330474,0)};
  for (unsigned int k=0; k< freqs.size(); ++k){
    im.set(Complex(1.0));
    
    
   
    /*std::vector<Double> coeff={ 1.000, -1.462e-3, 8.23e-7, -1.83e-10}; 
      BeamSquint squint(MDirection(Quantity(0.0, "'"),
      Quantity(0.0, "'"),
      MDirection::Ref(MDirection::AZEL)),
      Quantity(1.0, "GHz"));
      PBMath1DPoly pbm(coeff, Quantity(1.0, "deg"), Quantity(1.0, "GHz"), False,  squint, True);*/
    pixworld[3]=freqs[k];
    cs.setReferenceValue(pixworld);
    im.setCoordinateInfo(cs);
    pbm.applyPB(im, im, thedir);
    im.flush();
    cerr << std::setprecision(12) << "Value at (0, 50) for " << freqs[k] << "  " << im.getAt(IPosition(4,0,50,0,0)) << endl; 
    AlwaysAssertExit(near(refval[k], im.getAt(IPosition(4, 0, 50, 0, 0)), 1.0e-5));
   
  }


cout << "OK" << endl;
exit(0);
}
