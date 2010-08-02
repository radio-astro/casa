//# tSmooth.cc:  this tests the Smooth class
//# Copyright (C) 2010 by ESO (in the framework of the ALMA collaboration)
//# Copyright (C) 1996,1997,1998,1999,2002
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
//# Correspondence concerning AIPS++ should be addressed as follows:
//#        Internet email: aips2-request@nrao.edu.
//#        Postal address: AIPS++ Project Office
//#                        National Radio Astronomy Observatory
//#                        520 Edgemont Road
//#                        Charlottesville, VA 22903-2475 USA
//#
//# $Id:  $

#include <casa/aips.h>
#include <casa/Arrays/IPosition.h>
#include <casa/Arrays/Array.h>
#include <casa/Arrays/Vector.h>
#include <casa/Arrays/Matrix.h>
#include <casa/Arrays/ArrayLogical.h>
#include <scimath/Mathematics/Smooth.h>
#include <casa/iostream.h>

#include <casa/namespace.h>
int main() {
  Bool anyFailures = False;
  {
    Bool failed = False;
    // Test with Float Vectors, all flags 0
    
    vector<Float> vyin; 
    vector<Bool> vyinFlags;
    
    Vector<Float> myexp;
    Vector<Float> outv;
    Vector<Bool> outFlags;
    
    uInt vdim = 8;
    
    Float myyin[] = {1,3,1,4,2,6,3,8};
    Bool myflags1[] = {0,0,0,0,0,0,0,0};

    vyin.assign(myyin, myyin+vdim);    
    vyinFlags.assign(myflags1, myflags1+vdim);    

    Vector<Float> yin(vyin);
    Vector<Bool> yinFlags(vyinFlags);
    
    myexp.resize(vdim);
    myexp[0] = vyin[0];
    myexp[vdim-1] = vyin[vdim-1];
    for(uInt i=1; i<vdim-1; i++){
      myexp[i] = 0.25 * vyin[i-1] + 0.5 * vyin[i] + 0.25 * vyin[i+1];
    }
    
    Vector<Bool> myexpflags(vdim,False);
    myexpflags[0] = True;
    myexpflags[7] = True;

    outv.resize(vdim);
    outFlags.resize(vdim);

    Smooth<Float>::hanning(outv, // the output
			   outFlags, // the output mask
			   yin, // the input
			   yinFlags, // the input mask
			   False);  // for flagging: good is not true
    
    if(!allNearAbs(myexp, outv, 1.E-6)){
      for(uInt i = 0; i<vdim; i++){
	cout << i << " in " << yin[i] << endl;
	cout << i << " out " << outv[i] << endl;
	cout << i << " exp " << myexp[i] << endl;
      }
      failed = True;
    }

    if(!allEQ(myexpflags, outFlags)){
      for(uInt i = 0; i<vdim; i++){
	cout << i << " inFlags " << yinFlags[i] << endl;
	cout << i << " outFlags " << outFlags[i] << endl;
	cout << i << " expFlags " << myexpflags[i] << endl;
      }
      failed = True;
    }
    
    if (failed){
      cout << "Failed";
    }
    else{
      cout << "Passed";
    }

    cout << " the Float Vector Hanning Smooth Test, all unflagged"<< endl;
    if (failed){
      anyFailures = True;
    }
  }
  {
    Bool failed = False;
    // Test with Float Vectors, all flags 1
    
    vector<Float> vyin; 
    vector<Bool> vyinFlags;
    
    Vector<Float> myexp;
    Vector<Float> outv;
    Vector<Bool> outFlags;
    
    uInt vdim = 8;
    
    Float myyin[] = {1,3,1,4,2,6,3,8};
    Bool myflags1[] = {1,1,1,1,1,1,1,1};

    vyin.assign(myyin, myyin+vdim);    
    vyinFlags.assign(myflags1, myflags1+vdim);    

    Vector<Float> yin(vyin);
    Vector<Bool> yinFlags(vyinFlags);
    
    myexp.resize(vdim);
    myexp.assign(yin);
    
    outv.resize(vdim);
    outFlags.resize(vdim);

    Vector<Bool> myexpflags(yinFlags);

    Smooth<Float>::hanning(outv, // the output
			   outFlags, // the output mask
			   yin, // the input
			   yinFlags, // the input mask
			   False);  // for flagging: good is not true
    
    if(!allNearAbs(myexp, outv, 1.E-6)){
      for(uInt i = 0; i<vdim; i++){
	cout << i << " in " << yin[i] << endl;
	cout << i << " out " << outv[i] << endl;
	cout << i << " exp " << myexp[i] << endl;
      }
      failed = True;
    }
    
    if(!allEQ(myexpflags, outFlags)){
      for(uInt i = 0; i<vdim; i++){
	cout << i << " inFlags " << yinFlags[i] << endl;
	cout << i << " outFlags " << outFlags[i] << endl;
	cout << i << " expFlags " << myexpflags[i] << endl;
      }
      failed = True;
    }

    if (failed){
      cout << "Failed";
    }
    else{
      cout << "Passed";
    }

    cout << " the Float Vector Hanning Smooth Test, all flagged"<< endl;
    if (failed){
      anyFailures = True;
    }
  }
  {
    Bool failed = False;
    // Test with Float Vectors, mixed flags
    
    vector<Float> vyin; 
    vector<Bool> vyinFlags;
    
    Vector<Float> myexp;
    Vector<Float> outv;
    Vector<Bool> outFlags;
    
    uInt vdim = 8;
    
    Float myyin[]   = {1,3,1,4,2,6,3,8};
    Bool myflags1[] = {1,0,1,1,0,0,0,1};

    vyin.assign(myyin, myyin+vdim);    
    vyinFlags.assign(myflags1, myflags1+vdim);    

    Vector<Float> yin(vyin);
    Vector<Bool> yinFlags(vyinFlags);
    
    myexp.resize(vdim);
    myexp[0] = vyin[0];
    myexp[1] = vyin[1];
    myexp[2] = vyin[2];
    myexp[3] = vyin[3];
    myexp[4] = vyin[4];
    myexp[5] = 0.25 * vyin[5-1] + 0.5 * vyin[5] + 0.25 * vyin[5+1];
    myexp[6] = vyin[6];
    myexp[7] = vyin[7];

    Vector<Bool> myexpflags(vdim);
    myexpflags[0] = yinFlags[0] ||  yinFlags[1];
    myexpflags[1] = yinFlags[0] ||  yinFlags[1] || yinFlags[2];
    myexpflags[2] = yinFlags[1] ||  yinFlags[2] || yinFlags[3];
    myexpflags[3] = yinFlags[2] ||  yinFlags[3] || yinFlags[4];
    myexpflags[4] = yinFlags[3] ||  yinFlags[4] || yinFlags[5];
    myexpflags[5] = yinFlags[4] ||  yinFlags[5] || yinFlags[6];
    myexpflags[6] = yinFlags[5] ||  yinFlags[6] || yinFlags[7];
    myexpflags[7] = yinFlags[6] ||  yinFlags[7];

    outv.resize(vdim);
    outFlags.resize(vdim);

    Smooth<Float>::hanning(outv, // the output
			   outFlags, // the output mask
			   yin, // the input
			   yinFlags, // the input mask
			   False);  // for flagging: good is not true
    
    if(!allNearAbs(myexp, outv, 1.E-6)){
      for(uInt i = 0; i<vdim; i++){
	cout << i << " in " << yin[i] << endl;
	cout << i << " out " << outv[i] << endl;
	cout << i << " exp " << myexp[i] << endl;
      }
      failed = True;
    }
    
    if(!allEQ(myexpflags, outFlags)){
      for(uInt i = 0; i<vdim; i++){
	cout << i << " inFlags " << yinFlags[i] << endl;
	cout << i << " outFlags " << outFlags[i] << endl;
	cout << i << " expFlags " << myexpflags[i] << endl;
      }
      failed = True;
    }
    
    if (failed){
      cout << "Failed";
    }
    else{
      cout << "Passed";
    }

    cout << " the Float Vector Hanning Smooth Test, mixed flags"<< endl;
    if (failed){
      anyFailures = True;
    }
  }
  if (anyFailures) {
    cout << "FAIL" << endl;
    return 1;
  }
  else {
    cout << "OK" << endl;
    return 0;
  }

  /////////////////////////////////////

  {
    Bool failed = False;
    // Test with Complex Vectors, all flags 0
    
    vector<Complex> vyin; 
    vector<Bool> vyinFlags;
    
    Vector<Complex> myexp;
    Vector<Complex> outv;
    Vector<Bool> outFlags;
    
    uInt vdim = 8;
    
    Complex myyin[] = {Complex(1.,1.),
		       Complex(3.,3.),
		       Complex(1.,1.),
		       Complex(4.,4.),
		       Complex(2.,2.),
		       Complex(6.,6.),
		       Complex(3.,3.),
		       Complex(8.,8.)};
    Bool myflags1[] = {0,0,0,0,0,0,0,0};

    vyin.assign(myyin, myyin+vdim);    
    vyinFlags.assign(myflags1, myflags1+vdim);    

    Vector<Complex> yin(vyin);
    Vector<Bool> yinFlags(vyinFlags);
    
    myexp.resize(vdim);
    myexp[0] = vyin[0];
    myexp[vdim-1] = vyin[vdim-1];
    for(uInt i=1; i<vdim-1; i++){
      myexp[i] = Complex(0.25,0.) * vyin[i-1] + Complex(0.5,0.) * vyin[i] + Complex(0.25,0) * vyin[i+1];
    }
    
    Vector<Bool> myexpflags(vdim,False);
    myexpflags[0] = True;
    myexpflags[7] = True;

    outv.resize(vdim);
    outFlags.resize(vdim);

    Smooth<Complex>::hanning(outv, // the output
			     outFlags, // the output mask
			     yin, // the input
			     yinFlags, // the input mask
			     False);  // for flagging: good is not true
    
    if(!allNearAbs(myexp, outv, 1.E-6)){
      for(uInt i = 0; i<vdim; i++){
	cout << i << " in " << yin[i] << endl;
	cout << i << " out " << outv[i] << endl;
	cout << i << " exp " << myexp[i] << endl;
      }
      failed = True;
    }

    if(!allEQ(myexpflags, outFlags)){
      for(uInt i = 0; i<vdim; i++){
	cout << i << " inFlags " << yinFlags[i] << endl;
	cout << i << " outFlags " << outFlags[i] << endl;
	cout << i << " expFlags " << myexpflags[i] << endl;
      }
      failed = True;
    }
    
    if (failed){
      cout << "Failed";
    }
    else{
      cout << "Passed";
    }

    cout << " the Complex Vector Hanning Smooth Test, all unflagged"<< endl;
    if (failed){
      anyFailures = True;
    }
  }
  {
    Bool failed = False;
    // Test with Complex Vectors, mixed flags
    
    vector<Complex> vyin; 
    vector<Bool> vyinFlags;
    
    Vector<Complex> myexp;
    Vector<Complex> outv;
    Vector<Bool> outFlags;
    
    uInt vdim = 8;
    
    Complex myyin[] = {Complex(1.,1.),
		       Complex(3.,3.),
		       Complex(1.,1.),
		       Complex(4.,4.),
		       Complex(2.,2.),
		       Complex(6.,6.),
		       Complex(3.,3.),
		       Complex(8.,8.)};
    Bool myflags1[] = {1,0,1,1,0,0,0,1};

    vyin.assign(myyin, myyin+vdim);    
    vyinFlags.assign(myflags1, myflags1+vdim);    

    Vector<Complex> yin(vyin);
    Vector<Bool> yinFlags(vyinFlags);
    
    myexp.resize(vdim);
    myexp[0] = vyin[0];
    myexp[1] = vyin[1];
    myexp[2] = vyin[2];
    myexp[3] = vyin[3];
    myexp[4] = vyin[4];
    myexp[5] = Complex(0.25,0.) * vyin[5-1] + Complex(0.5,0.) * vyin[5] + Complex(0.25,0) * vyin[5+1];
    myexp[6] = vyin[6];
    myexp[7] = vyin[7];

    Vector<Bool> myexpflags(vdim);
    myexpflags[0] = yinFlags[0] ||  yinFlags[1];
    myexpflags[1] = yinFlags[0] ||  yinFlags[1] || yinFlags[2];
    myexpflags[2] = yinFlags[1] ||  yinFlags[2] || yinFlags[3];
    myexpflags[3] = yinFlags[2] ||  yinFlags[3] || yinFlags[4];
    myexpflags[4] = yinFlags[3] ||  yinFlags[4] || yinFlags[5];
    myexpflags[5] = yinFlags[4] ||  yinFlags[5] || yinFlags[6];
    myexpflags[6] = yinFlags[5] ||  yinFlags[6] || yinFlags[7];
    myexpflags[7] = yinFlags[6] ||  yinFlags[7];

    outv.resize(vdim);
    outFlags.resize(vdim);

    Smooth<Complex>::hanning(outv, // the output
			     outFlags, // the output mask
			     yin, // the input
			     yinFlags, // the input mask
			     False);  // for flagging: good is not true
    
    if(!allNearAbs(myexp, outv, 1.E-6)){
      for(uInt i = 0; i<vdim; i++){
	cout << i << " in " << yin[i] << endl;
	cout << i << " out " << outv[i] << endl;
	cout << i << " exp " << myexp[i] << endl;
      }
      failed = True;
    }
    
    if(!allEQ(myexpflags, outFlags)){
      for(uInt i = 0; i<vdim; i++){
	cout << i << " inFlags " << yinFlags[i] << endl;
	cout << i << " outFlags " << outFlags[i] << endl;
	cout << i << " expFlags " << myexpflags[i] << endl;
      }
      failed = True;
    }
    
    if (failed){
      cout << "Failed";
    }
    else{
      cout << "Passed";
    }

    cout << " the Complex Vector Hanning Smooth Test, mixed flags"<< endl;
    if (failed){
      anyFailures = True;
    }
  }
  if (anyFailures) {
    cout << "FAIL" << endl;
    return 1;
  }
  else {
    cout << "OK" << endl;
    return 0;
  }

}

// End:
