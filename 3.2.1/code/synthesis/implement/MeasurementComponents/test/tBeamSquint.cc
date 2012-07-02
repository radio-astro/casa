//# tBeamSquint.cc: This program tests BeamSquint class
//# Copyright (C) 1998,1999,2000,2002
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

//# Includes
#include <casa/aips.h>
#include <synthesis/MeasurementComponents/BeamSquint.h>
#include <coordinates/Coordinates.h>
#include <casa/Arrays/Matrix.h>
#include <casa/Exceptions/Error.h>
#include <casa/iostream.h>

#include <casa/namespace.h>
int main()
{
  try {
    cout << "Testing BeamSquint" << endl;
    cout << "--------------------------------------" << endl;

    BeamSquint nullBS;
    
    BeamSquint oneBS((MDirection( Quantity(1.0, "deg"), Quantity(1.0, "deg"),
				  MDirection::Ref(MDirection::AZEL))),
		      Quantity(1.0, "GHz") );
    BeamSquint otherBS(oneBS);
    MDirection otherMD = otherBS.scale( Quantity(2.0, "GHz") );
    MDirection oneMD = oneBS.scale( Quantity(1.4, "GHz") );

    cout << "oneMD = " << oneMD.getAngle("deg") << endl;
    cout << "otherMD = " << otherMD.getAngle("deg") << endl;

    

    Matrix<Double> xform(2,2);                                    
    xform = 0.0; xform.diagonal() = 1.0;                          
    /*    DirectionCoordinate dirCoords(MDirection::J2000,                  
                                  Projection(Projection::SIN),        
                                  135*C::pi/180.0, 60*C::pi/180.0,    
                                  -1*C::pi/180.0/3600, 1*C::pi/180/3600,        
                                  xform,                              
                                  128, 128);  
    MDirection oldPointing(Quantity(135.0,"deg"), Quantity(60.0,"deg"), 
			   MDirection::Ref(MDirection::J2000));
			   */
    DirectionCoordinate dirCoords(MDirection::J2000,                  
                                  Projection(Projection::SIN),        
                                  0.0, 0.0,    
                                  -1*C::pi/180.0/3600, 1*C::pi/180/3600,        
                                  xform,                              
                                  128, 128);  
    MDirection oldPointing(Quantity(0.0,"deg"), Quantity(0.0,"deg"), 
			   MDirection::Ref(MDirection::J2000));


    MDirection newPointing;

    oneBS.getPointingDirection(oldPointing,
			       Quantity(0.0, "deg"),Quantity(1.0, "GHz"),
			       BeamSquint::NONE, newPointing);	
    cout << "non-squinted pointing: " << newPointing.getAngle("deg") << endl;

    oneBS.getPointingDirection(oldPointing,
			       Quantity(0.0, "deg"),Quantity(1.0, "GHz"),
			       BeamSquint::RR, newPointing);	
    cout << " RR-squinted pointing: " << newPointing.getAngle("deg") << endl;
    cout << " original    pointing: " << oldPointing.getAngle("deg") << endl;
    oneBS.getPointingDirection(oldPointing,
			       Quantity(0.1, "rad"),Quantity(1.0, "GHz"),
			       BeamSquint::RR, newPointing);	
    cout << " RR-  rot .1 pointing: " << newPointing.getAngle("deg") << endl;
    cout << " original    pointing: " << oldPointing.getAngle("deg") << endl;
    oneBS.getPointingDirection(oldPointing,
			       Quantity(C::pi/4, "rad"),Quantity(1.0, "GHz"),
			       BeamSquint::LL, newPointing);	
    cout << " RR-rot pi/4 pointing: " << newPointing.getAngle("deg") << endl;
    cout << " original    pointing: " << oldPointing.getAngle("deg") << endl;
    oneBS.getPointingDirection(oldPointing,
			       Quantity(C::pi/2, "rad"),Quantity(1.0, "GHz"),
			       BeamSquint::RR, newPointing);	
    cout << " RR-rot pi/2 pointing: " << newPointing.getAngle("deg") << endl;
    cout << " original    pointing: " << oldPointing.getAngle("deg") << endl;
    oneBS.getPointingDirection(oldPointing,
			       Quantity(90.0, "deg"),Quantity(1.0, "GHz"),
			       BeamSquint::LL, newPointing);	
    cout << " LL-squinted, rotated: " << newPointing.getAngle("deg") << endl;
    cout << " original    pointing: " << oldPointing.getAngle("deg") << endl;
    oneBS.getPointingDirection(oldPointing,
			       Quantity(90.0, "deg"),Quantity(.5, "GHz"),
			       BeamSquint::LL, newPointing);	
    cout << " LL-rotated, 1/2*freq: " << newPointing.getAngle("deg") << endl;
    cout << " original    pointing: " << oldPointing.getAngle("deg") << endl;
  

  } catch (AipsError x) {
    cout << x.getMesg() << endl;
  } 
  
  exit(0);
}
