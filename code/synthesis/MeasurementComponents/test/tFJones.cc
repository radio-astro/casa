//# tFJones: test ionosphere term
//# Copyright (C) 2013
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

//#include <casa/Arrays/Array.h>
//#include <casa/Arrays/ArrayMath.h>
#include <casa/Exceptions/Error.h>


#include <measures/Measures.h>
//#include <measures/Measures/EarthField.h>
#include <measures/Measures/EarthMagneticMachine.h>
#include <casa/Quanta/MVTime.h>
#include <measures/Measures/MDirection.h>
#include <measures/Measures/MPosition.h>
#include <measures/Measures/MEpoch.h>
#include <measures/Measures/MeasTable.h>

#include <casa/iostream.h>
#include <casa/BasicMath/Math.h>
#include <casa/namespace.h>

//#include <synthesis/MeasurementComponents/FJones.h>

// <summary>
// Test program for FJones-related classes
// </summary>

// Control verbosity
#define FJONES_TEST_VERBOSE False

void testEMM (Bool verbose=False) {

  {

    // What time is it?
    MEpoch when(MVEpoch(MVTime(2015,1,1,0.0).day()));

    cout << "when = " << when << endl;

    // Where are we?
    MPosition where;
    MeasTable::Observatory(where,"VLA");  // the VLA position

    cout << "where = " << where << endl;

    // Make a frame containing when/where
    MeasFrame frame(when,where);

    // Make the field machine from a direction reference, altitude, 
    //   and frame
    MDirection::Ref MDref(MDirection::J2000);
    EarthMagneticMachine fm(MDref, 
			    Quantity(350., "km"),
			    frame);

    MDirection toward;
    MeasTable::Source(toward,"J133108.2+303032");  // 3C286
    //    MeasTable::Source(toward,"J132527.6-430108");  // southish

    cout << "toward = " << toward << endl;


    //  Loop over times
    Double time=when.getValue().getTime("s").getValue();  // time in seconds
    MVDirection towardV=toward.getValue();
    for (Int i=0;i<25;++i) {
      frame.resetEpoch(Quantity(time,"s"));
      fm.calculate(towardV);
      cout << MVTime(time/86400.0).string(MVTime::YMD,7)
	   << " LOS Field=" << fm.getLOSField("G") << endl;
      time+=3600.;
    }

  }
}

void testEMM2 (Bool verbose=False) {

  // (more like FJones implementation...)

  {

    cout << endl << "testEMM2 (like FJones implementation):" << endl;


    // Set generic frame, emm
    MeasFrame mframe;
    EarthMagneticMachine *emm;
    emm=NULL;

    Quantity ionhgt(350.,"km");

    // What time is it?
    MEpoch when(MVEpoch(MVTime(2015,1,1,0.0).day()));
    Double time=when.getValue().getTime("s").getValue();  // time in seconds
    mframe.set(when);

    // Set position in the frame
    MPosition where;
    MeasTable::Observatory(where,"VLA");  // the VLA position
    mframe.set(where);


    // the direction we will calculate for
    MDirection toward;
    MeasTable::Source(toward,"J133108.2+303032");  // 3C286
    MDirection::Ref mdref=toward.getRef();

    
    if (!emm)
      emm = new EarthMagneticMachine(mdref,ionhgt,mframe);

    //  Loop over times
    MVDirection towardV=toward.getValue();
    for (Int i=0;i<25;++i) {
      when.set(Quantity(time,"s"));
      mframe.set(when);
      emm->calculate(towardV);
      cout << MVTime(time/86400.0).string(MVTime::YMD,7)
	   << " LOS Field=" << emm->getLOSField("G") << endl;
      time+=3600.;
    }


    if (emm)
      delete emm;
    
  }
}


int main ()
{
  try {

    testEMM(FJONES_TEST_VERBOSE);
    testEMM2(FJONES_TEST_VERBOSE);
    
  } catch (AipsError x) {
    cout << "Unexpected exception: " << x.getMesg() << endl;
    exit(1);
  } catch (...) {
    cout << "Unexpected unknown exception" << endl;
    exit(1);
  }
  cout << "OK" << endl;
  exit(0);
};
