//# dInterpolate1D.cc: This program demonstrates the Interpolate1D class
//# Copyright (C) 1996,1999,2001
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

#include <scimath/Functionals/Interpolate1D.h>
#include <scimath/Functionals/ScalarSampledFunctional.h>

#include <casa/Arrays/Array.h>
#include <casa/Arrays/ArrayMath.h>
#include <casa/Arrays/Vector.h>

#include <tables/Tables/TableDesc.h>
#include <tables/Tables/SetupNewTab.h>
#include <tables/Tables/Table.h>
#include <tables/Tables/ScaColDesc.h>
#include <tables/Tables/ScalarColumn.h>

#include <casa/iostream.h>
#include <casa/namespace.h>

// Demonstrate the interpolate class by using it to create a table 
// and then look at the results (using glish) 

int main()
{
  uInt i;
  cout << "Producing a table called dInterpolate1D_tmp.table" << endl;
  // first calculate the "true" data (actually just a sum of Gaussians)
  Vector<Float> x(200), yTrue(200);
  indgen(x, Float(-10), Float(.1));
  yTrue = Float(10) - Float( 5)*exp(-pow( x-Float(3.3)            ,2)) 
                    - Float( 9)*exp(-pow((x+Float( 1))/Float(0.3) ,2))
                    - Float(20)*exp(-pow((x+Float(10))/Float(5.0) ,2));

  // Now calculate the measured data, sampled a x=-5,-4...4 (ten points)
  Vector<Float> xMeas(10), yMeas(10);
  for (i = 0; i < 10; i++){
    xMeas(i) = x(50+i*10);
    yMeas(i) = yTrue(50+i*10);
  }
  // Create the interpolator
  ScalarSampledFunctional<Float> fx(xMeas), fy(yMeas);
  Interpolate1D<Float,Float> interpolate(fx, fy); 
      
  // Now get the estimated data using interpolation
  // First use linear Interpolation
  Vector<Float> yLinear(200); 
  for (i = 0; i < 200; i++)
    yLinear(i) = interpolate(x(i));
  // Now use Nearest Neighbour Interpolation
  Vector<Float> yNearest(200); 
  interpolate.setMethod(Interpolate1D<Float,Float>::nearestNeighbour);
  for (i = 0; i < 200; i++)
    yNearest(i) = interpolate(x(i));
  // Now use Cubic Interpolation
  Vector<Float> yCubic(200); 
  interpolate.setMethod(Interpolate1D<Float,Float>::cubic);
  for (i = 0; i < 200; i++)
    yCubic(i) = interpolate(x(i));
  // And Finally use Cubic Spline Interpolation
  Vector<Float> ySpline(200); 
  interpolate.setMethod(Interpolate1D<Float,Float>::spline);
  for (i = 0; i < 200; i++)
    ySpline(i) = interpolate(x(i));
  
// Now create a table to hold the results (so they can be plotted!)
  TableDesc td("Different interpolation methods", TableDesc::Scratch);
  td.comment() = 
    "This demo shows the how the different interpolation methods compare when used for spectral line fitting";
  td.addColumn(ScalarColumnDesc<Float> ("X"));
  td.addColumn(ScalarColumnDesc<Float> ("True"));
  td.addColumn(ScalarColumnDesc<Float> ("Nearest Neighbour"));
  td.addColumn(ScalarColumnDesc<Float> ("Linear"));
  td.addColumn(ScalarColumnDesc<Float> ("Cubic"));
  td.addColumn(ScalarColumnDesc<Float> ("Spline"));
  // Store the measured data in a table keyword
  // I was going to do this but the way to add keywords seems to have 
  // changed and I cannot find/understand the updated documentation

  SetupNewTable newtab("dInterpolate1D_tmp.table", td, Table::New);
  Table tab(newtab);
  
  ScalarColumn<Float> tX (tab, "X");
  ScalarColumn<Float> tTrue (tab, "True");
  ScalarColumn<Float> tNearest (tab, "Nearest Neighbour");
  ScalarColumn<Float> tLinear (tab, "Linear");
  ScalarColumn<Float> tCubic (tab, "Cubic");
  ScalarColumn<Float> tSpline (tab, "Spline");

  tab.addRow(200);
  tX.putColumn(x);
  tTrue.putColumn(yTrue);
  tNearest.putColumn(yNearest);
  tLinear.putColumn(yLinear);
  tCubic.putColumn(yCubic);
  tSpline.putColumn(ySpline);
  cout << "interpolation.table constructed." << endl
       << "Use glish dInterpolate1D.g to see results" << endl;
}
