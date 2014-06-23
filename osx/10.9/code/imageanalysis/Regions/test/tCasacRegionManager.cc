//# tCasacRegionManager.cc:  test the regions in the Regionhandler classes
//# Copyright (C) 2000,2001
//# Associated Universities, Inc. Washington DC, USA.
//#
//# This program is free software; you can redistribute it and/or modify it
//# under the terms of the GNU General Public License as published by the Free
//# Software Foundation; either version 2 of the License, or(at your option)
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
//# $Id: tRegionHandler.cc 20600 2009-05-11 09:33:40Z gervandiepen $

#include <imageanalysis/Regions/CasacRegionManager.h>

#include <casa/OS/EnvVar.h>

#include <images/Images/FITSImage.h>

#include <iostream>
#include <casa/namespace.h>

void writeTestString(const String& test) {
    cout << "\n" << "*** " << test << " ***" << endl;
}

Vector<Double> recToVec(const Record& rec) {
	uInt nfields = rec.nfields();
	Vector<Double> vec(nfields);
	vec[0] = rec.asRecord("*1").asDouble("value");
	vec[1] = rec.asRecord("*2").asDouble("value");
	if (nfields >= 3) {
		vec[2] = rec.asRecord("*3").asDouble("value");
		if (nfields >= 4) {
			vec[3] = rec.asRecord("*4").asDouble("value");
		}
	}
	return vec;
}

void compVecs(Vector<Double>& got, Vector<Double>& exp) {
	Double epsilon = 1e-8;
	for (uInt i=0; i<got.size(); i++) {
		Double fracDiff = fabs((got[i]-exp[i])/exp[i]);
		AlwaysAssert(fracDiff < epsilon, AipsError);
	}
}

int main() {
	String *parts = new String[2];
	split(EnvironmentVariable::get("CASAPATH"), parts, 2, String(" "));
	String datadir = parts[0] + "/data/regression/unittest/imageanalysis/Regions/";
	delete [] parts;

	  const ImageInterface<Float> *myImage = new FITSImage(datadir + "imregion.fits");
	  const ImageInterface<Float> *myImageNoSpec = new FITSImage(datadir + "imregion_nospec.fits");
	  const ImageInterface<Float> *myImageDirOnly = new FITSImage(datadir + "imregion_dironly.fits");

	  String test, diagnostics, stokes, chans, box;
	  uInt nSelectedChannels;
	  Vector<uInt> chanEndPoints, polEndPoints;
	  CasacRegionManager::StokesControl stokesControl;
	  Record regRec;
	  CasacRegionManager rm(myImage->coordinates());
	  IPosition imShape = myImage->shape();
	  cout << "*** imShape " << imShape << endl;
	  Double box1 = 1.24795026;
	  Double box2 = 0.782552901;
	  Double box3 = 1.24794616;
	  Double box4 = 0.782555814;
	  Double box5 = 1.24794206;
	  Double box6 = 0.782558727;
	  Double box7 = 1.24793797;
	  Double box8 = 0.782561641;
	  Double chan0 = 4.73510000e+09;
	  Double chan4 = 6.33510000e+09;
	  Double chan15 = 1.07351000e+10;
	  Double chan19 = 1.23351000e+10;
	  try {
		  {
			  diagnostics = "";
			  nSelectedChannels = 0;
			  stokes = "";
			  chans = "";
			  stokesControl = CasacRegionManager::USE_ALL_STOKES;
			  box = "";
			  writeTestString("Test non-existent region file throws exception");
			  try {
				  regRec = rm.fromBCS(
						  diagnostics, nSelectedChannels, stokes,
						  0, datadir + "adfjasfksa", chans, stokesControl, box,
						  imShape
				  );
				  AlwaysAssert(False, AipsError)
			  }
			  catch (AipsError) {}
		  }
		  {
			  diagnostics = "";
			  nSelectedChannels = 0;
			  stokes = "";
			  chans = "";
			  stokesControl = CasacRegionManager::USE_ALL_STOKES;
			  box = "";
			  writeTestString("Test bogus region file throws exception");
			  try {
				  regRec = rm.fromBCS(
						  diagnostics, nSelectedChannels, stokes,
						  0, datadir + "bogus.rgn", chans, stokesControl, box,
						  imShape
				  );
				  AlwaysAssert(False, AipsError)
			  }
			  catch (AipsError x) {}
		  }
		  {
			  diagnostics = "";
			  nSelectedChannels = 0;
			  stokes = "";
			  chans = "";
			  stokesControl = CasacRegionManager::USE_ALL_STOKES;
			  box = "";
			  writeTestString("Test bogus text region file throws exception");
			  try {
				  regRec = rm.fromBCS(
						  diagnostics, nSelectedChannels, stokes,
						  0, datadir + "bogus1.rgn", chans, stokesControl, box,
						  imShape
				  );
				  AlwaysAssert(False, AipsError)
			  }
			  catch (AipsError) {}
		  }
		  {
			  diagnostics = "";
			  nSelectedChannels = 0;
			  stokes = "";
			  chans = "";
			  stokesControl = CasacRegionManager::USE_ALL_STOKES;
			  box = "";
			  writeTestString("Test default gives region of entire image");
			  regRec = rm.fromBCS(
					  diagnostics, nSelectedChannels, stokes,
					  0, "", chans, stokesControl, box,
					  imShape
			  );
			  AlwaysAssert(nSelectedChannels == 20, AipsError);
			  Vector<Double> gotblc = recToVec(regRec.asRecord("blc"));
			  Vector<Double> expblc(4);
			  expblc[0] = 1.24795230;
			  expblc[1] = 0.782549990;
			  expblc[2] = 4.73510000e+09;
			  expblc[3] = 1.0;
			  compVecs(gotblc, expblc);
			  Vector<Double> gottrc = recToVec(regRec.asRecord("trc"));
			  Vector<Double> exptrc(4);

			  exptrc[0] = 1.24791339;
			  exptrc[1] = 0.782577665;
			  exptrc[2] = 1.23351000e+10;
			  exptrc[3] = 4.0;
			  compVecs(gottrc, exptrc);
		  }
		  {
			  diagnostics = "";
			  nSelectedChannels = 0;
			  stokes = "";
			  chans = "";
			  stokesControl = CasacRegionManager::USE_ALL_STOKES;
			  box = "";
			  writeTestString("Test valid region text file");
			  regRec = rm.fromBCS(
			      diagnostics, nSelectedChannels, stokes,
			      0, datadir + "goodfile1.txt", chans, stokesControl, box,
			      imShape
			  );
			  AlwaysAssert(regRec.asRecord("regions").asInt("nr") == 2, AipsError);
		  }
		  {
			  diagnostics = "";
			  nSelectedChannels = 0;
			  stokes = "Q";
			  chans = "";
			  stokesControl = CasacRegionManager::USE_ALL_STOKES;
			  box = "";
			  writeTestString("Test setting a single stokes");
			  regRec = rm.fromBCS(
				  diagnostics, nSelectedChannels, stokes,
				  0, "", chans, stokesControl, box,
				  imShape
			  );
			  AlwaysAssert(nSelectedChannels == 20, AipsError);
			  Vector<Double> gotblc = recToVec(regRec.asRecord("blc"));
			  Vector<Double> expblc(4);
			  expblc[0] = 1.24795230;
			  expblc[1] = 0.782549990;
			  expblc[2] = 4.73510000e+09;
			  expblc[3] = 2.0;
			  compVecs(gotblc, expblc);
			  Vector<Double> gottrc = recToVec(regRec.asRecord("trc"));
			  Vector<Double> exptrc(4);

			  exptrc[0] = 1.24791339;
			  exptrc[1] = 0.782577665;
			  exptrc[2] = 1.23351000e+10;
			  exptrc[3] = 2.0;
			  compVecs(gottrc, exptrc);
		  }
		  {
			  diagnostics = "";
			  nSelectedChannels = 0;
			  stokes = "QU";
			  chans = "";
			  stokesControl = CasacRegionManager::USE_ALL_STOKES;
			  box = "";
			  writeTestString("Test setting a contiguous stokes");
			  regRec = rm.fromBCS(
					  diagnostics, nSelectedChannels, stokes,
					  0, "", chans, stokesControl, box,
					  imShape
			  );
			  AlwaysAssert(nSelectedChannels == 20, AipsError);
			  Vector<Double> gotblc = recToVec(regRec.asRecord("blc"));
			  Vector<Double> expblc(4);
			  expblc[0] = 1.24795230;
			  expblc[1] = 0.782549990;
			  expblc[2] = 4.73510000e+09;
			  expblc[3] = 2.0;
			  compVecs(gotblc, expblc);
			  Vector<Double> gottrc = recToVec(regRec.asRecord("trc"));
			  Vector<Double> exptrc(4);

			  exptrc[0] = 1.24791339;
			  exptrc[1] = 0.782577665;
			  exptrc[2] = 1.23351000e+10;
			  exptrc[3] = 3.0;
			  compVecs(gottrc, exptrc);
		  }
		  {
			  diagnostics = "";
			  nSelectedChannels = 0;
			  stokes = "";
			  stokesControl = CasacRegionManager::USE_ALL_STOKES;
			  box = "";
			  writeTestString("Test setting a single channel");
			  for (uInt i=0; i<3; i++) {
				  chans = i == 0
					  ? "5"
					  : i == 1
					      ? "range=[5pix, 5pix]"
					      : "range=[6.73510000e+09Hz, 6.7351 GHz]";
				  regRec = rm.fromBCS(
				      diagnostics, nSelectedChannels, stokes,
					      0, "", chans, stokesControl, box,
					      imShape
				  );
				  cout << "*** nsel " << nSelectedChannels << endl;
				  AlwaysAssert(nSelectedChannels == 1, AipsError);
				  Vector<Double> gotblc = recToVec(regRec.asRecord("blc"));
				  Vector<Double> expblc(4);
				  expblc[0] = 1.24795230;
				  expblc[1] = 0.782549990;
				  expblc[2] = 6.73510000e+09;
				  expblc[3] = 1.0;
				  compVecs(gotblc, expblc);
				  Vector<Double> gottrc = recToVec(regRec.asRecord("trc"));
				  Vector<Double> exptrc(4);

				  exptrc[0] = 1.24791339;
				  exptrc[1] = 0.782577665;
				  exptrc[2] = 6.73510000e+09;
				  exptrc[3] = 4.0;
				  compVecs(gottrc, exptrc);
			  }
		  }
		  {
			  diagnostics = "";
			  nSelectedChannels = 0;
			  stokes = "";
			  stokesControl = CasacRegionManager::USE_ALL_STOKES;
			  box = "";
			  writeTestString("Test setting multiple continuous channels");
			  for (uInt i=0; i<5; i++) {
				  switch (i) {
				  case 0:
					  chans = "5~10";
					  break;
				  case 1:
					  chans = "5, 6, 7, 8, 9,10";
					  break;
				  case 2:
					  chans = "range=[5pix, 10pix]";
					  break;
				  case 3:
					  chans = "range=[6.73510000e+09Hz, 8.73510GHz]";
					  break;
				  case 4:
					  chans = "range=[5channel, 10channel]";
					  break;

				  }
				  regRec = rm.fromBCS(
					  diagnostics, nSelectedChannels, stokes,
					  0, "", chans, stokesControl, box,
					  imShape
				  );
				  AlwaysAssert(nSelectedChannels == 6, AipsError);
				  Vector<Double> gotblc = recToVec(regRec.asRecord("blc"));
				  Vector<Double> expblc(4);
				  expblc[0] = 1.24795230;
				  expblc[1] = 0.782549990;
				  expblc[2] = 6.73510000e+09;
				  expblc[3] = 1.0;
				  compVecs(gotblc, expblc);
				  Vector<Double> gottrc = recToVec(regRec.asRecord("trc"));
				  Vector<Double> exptrc(4);

				  exptrc[0] = 1.24791339;
				  exptrc[1] = 0.782577665;
				  exptrc[2] = 8.73510000e+09;
				  exptrc[3] = 4.0;
				  compVecs(gottrc, exptrc);
			  }
		  }
		  {
			  diagnostics = "";
			  nSelectedChannels = 0;
			  stokes = "";
			  chans = "";
			  stokesControl = CasacRegionManager::USE_ALL_STOKES;
			  box = "4,5,8,9";
			  writeTestString("Test setting box");
			  regRec = rm.fromBCS(
					  diagnostics, nSelectedChannels, stokes,
					  0, "", chans, stokesControl, box,
					  imShape
			  );
			  AlwaysAssert(nSelectedChannels == 20, AipsError);
			  Vector<Double> gotblc = recToVec(regRec.asRecord("blc"));
			  Vector<Double> expblc(4);
			  expblc[0] = 1.24794411;
			  expblc[1] = 0.782557271;
			  expblc[2] = 4.73510000e+09;
			  expblc[3] = 1.0;
			  compVecs(gotblc, expblc);
			  Vector<Double> gottrc = recToVec(regRec.asRecord("trc"));
			  Vector<Double> exptrc(4);

			  exptrc[0] = 1.24793592;
			  exptrc[1] = 0.782563097;
			  exptrc[2] = 1.23351000e+10;
			  exptrc[3] = 4;
			  compVecs(gottrc, exptrc);
		  }
		  {
			  diagnostics = "";
			  nSelectedChannels = 0;
			  stokes = "";
			  chans = "";
			  stokesControl = CasacRegionManager::USE_FIRST_STOKES;
			  box = "";
			  writeTestString("Test using first stokes");
			  regRec = rm.fromBCS(
					  diagnostics, nSelectedChannels, stokes,
					  0, "", chans, stokesControl, box,
					  imShape
			  );
			  AlwaysAssert(nSelectedChannels == 20, AipsError);
			  Vector<Double> gotblc = recToVec(regRec.asRecord("blc"));
			  Vector<Double> expblc(4);
			  expblc[0] = 1.24795230;
			  expblc[1] = 0.782549990;
			  expblc[2] = 4.73510000e+09;
			  expblc[3] = 1.0;
			  compVecs(gotblc, expblc);
			  Vector<Double> gottrc = recToVec(regRec.asRecord("trc"));
			  Vector<Double> exptrc(4);

			  exptrc[0] = 1.24791339;
			  exptrc[1] = 0.782577665;
			  exptrc[2] = 1.23351000e+10;
			  exptrc[3] = 1.0;
			  compVecs(gottrc, exptrc);
		  }
		  {
			  diagnostics = "";
			  nSelectedChannels = 0;
			  stokes = "";
			  chans = "";
			  stokesControl = CasacRegionManager::USE_ALL_STOKES;
			  box = "1,2,3,4,5,6,7,8,9,10,11,12";
			  writeTestString("Test setting multiple boxes");
			  regRec = rm.fromBCS(
					  diagnostics, nSelectedChannels, stokes,
					  0, "", chans, stokesControl, box,
					  imShape
			  );
			  AlwaysAssert(nSelectedChannels == 20, AipsError);
			  Vector<Double> gotblc = recToVec(regRec.asRecord("regions").asRecord("*2").asRecord("blc"));
			  Vector<Double> expblc(4);
			  expblc[0] = 1.24793387;
			  expblc[1] = 0.782564554;
			  expblc[2] = 4.73510000e+09;
			  expblc[3] = 1.0;
			  compVecs(gotblc, expblc);
			  Vector<Double> gottrc = recToVec(regRec.asRecord("regions").asRecord("*2").asRecord("trc"));
			  Vector<Double> exptrc(4);

			  exptrc[0] = 1.24792978;
			  exptrc[1] = 0.782567467;
			  exptrc[2] = 1.23351000e+10;
			  exptrc[3] = 4.0;
			  compVecs(gottrc, exptrc);

			  gotblc = recToVec(regRec.asRecord("regions").asRecord("*1").asRecord("regions").asRecord("*1").asRecord("blc"));
			  expblc[0] = box1;
			  expblc[1] = box2;
			  compVecs(gotblc, expblc);
			  gottrc = recToVec(regRec.asRecord("regions").asRecord("*1").asRecord("regions").asRecord("*1").asRecord("trc"));
			  exptrc[0] = box3;
			  exptrc[1] = box4;
			  compVecs(gottrc, exptrc);

			  gotblc = recToVec(regRec.asRecord("regions").asRecord("*1").asRecord("regions").asRecord("*2").asRecord("blc"));
			  expblc[0] = box5;
			  expblc[1] = box6;
			  compVecs(gotblc, expblc);
			  gottrc = recToVec(regRec.asRecord("regions").asRecord("*1").asRecord("regions").asRecord("*2").asRecord("trc"));
			  exptrc[0] = box7;
			  exptrc[1] = box8;
			  compVecs(gottrc, exptrc);
		  }
		  {
			  diagnostics = "";
			  nSelectedChannels = 0;
			  stokes = "IUV";
			  chans = "";
			  stokesControl = CasacRegionManager::USE_ALL_STOKES;
			  box = "";
			  writeTestString("Test setting multiple stokes ranges");
			  regRec = rm.fromBCS(
					  diagnostics, nSelectedChannels, stokes,
					  0, "", chans, stokesControl, box,
					  imShape
			  );
			  AlwaysAssert(nSelectedChannels == 20, AipsError);
			  Vector<Double> gotblc = recToVec(regRec.asRecord("regions").asRecord("*1").asRecord("blc"));
			  Vector<Double> expblc(4);
			  expblc[0] = 1.24795230;
			  expblc[1] = 0.782549990;
			  expblc[2] = 4.73510000e+09;
			  expblc[3] = 1.0;
			  compVecs(gotblc, expblc);
			  Vector<Double> gottrc = recToVec(regRec.asRecord("regions").asRecord("*1").asRecord("trc"));
			  Vector<Double> exptrc(4);

			  exptrc[0] = 1.24791339;
			  exptrc[1] = 0.782577665;
			  exptrc[2] = 1.23351000e+10;
			  exptrc[3] = 1.0;
			  compVecs(gottrc, exptrc);

			  gotblc = recToVec(regRec.asRecord("regions").asRecord("*2").asRecord("blc"));
			  expblc[0] = 1.24795230;
			  expblc[1] = 0.782549990;
			  expblc[2] = 4.73510000e+09;
			  expblc[3] = 3.0;
			  compVecs(gotblc, expblc);
			  gottrc = recToVec(regRec.asRecord("regions").asRecord("*2").asRecord("trc"));
			  exptrc[0] = 1.24791339;
			  exptrc[1] = 0.782577665;
			  exptrc[2] = 1.23351000e+10;
			  exptrc[3] = 4.0;
			  compVecs(gottrc, exptrc);
		  }
		  {
			  diagnostics = "";
			  nSelectedChannels = 0;
			  stokes = "";
			  chans = "<5,>=15";
			  stokesControl = CasacRegionManager::USE_ALL_STOKES;
			  box = "";
			  writeTestString("Test multiple channel ranges");
			  regRec = rm.fromBCS(
					  diagnostics, nSelectedChannels, stokes,
					  0, "", chans, stokesControl, box,
					  imShape
			  );
			  AlwaysAssert(nSelectedChannels == 10, AipsError);
			  Vector<Double> gotblc = recToVec(regRec.asRecord("regions").asRecord("*1").asRecord("blc"));
			  Vector<Double> expblc(4);
			  expblc[0] = 1.24795230;
			  expblc[1] = 0.782549990;
			  expblc[2] = 4.73510000e+09;
			  expblc[3] = 1.0;
			  compVecs(gotblc, expblc);
			  Vector<Double> gottrc = recToVec(regRec.asRecord("regions").asRecord("*1").asRecord("trc"));
			  Vector<Double> exptrc(4);
			  exptrc[0] = 1.24791339;
			  exptrc[1] = 0.782577665;
			  exptrc[2] = 6.33510000e+09;
			  exptrc[3] = 4.0;
			  compVecs(gottrc, exptrc);

			  gotblc = recToVec(regRec.asRecord("regions").asRecord("*2").asRecord("blc"));
			  expblc[0] = 1.24795230;
			  expblc[1] = 0.782549990;
			  expblc[2] = 1.07351000e+10;
			  expblc[3] = 1.0;
			  compVecs(gotblc, expblc);
			  gottrc = recToVec(regRec.asRecord("regions").asRecord("*2").asRecord("trc"));
			  exptrc[0] = 1.24791339;
			  exptrc[1] = 0.782577665;
			  exptrc[2] = 1.23351000e+10;
			  exptrc[3] = 4.0;
			  compVecs(gottrc, exptrc);
		  }
		  {
			  diagnostics = "";
			  nSelectedChannels = 0;
			  stokes = "IQV";
			  chans = "<5,>=15";
			  stokesControl = CasacRegionManager::USE_ALL_STOKES;
			  box = "1,2,3,4,5,6,7,8";
			  writeTestString("Test multiple channel ranges, multiple stokes ranges, and multiple boxes");
			  regRec = rm.fromBCS(
					  diagnostics, nSelectedChannels, stokes,
					  0, "", chans, stokesControl, box,
					  imShape
			  );
			  AlwaysAssert(nSelectedChannels == 10, AipsError);
			  // box="5,6,7,8", chans="15~19", stokes="V"
			  Vector<Double> gotblc = recToVec(regRec.asRecord("regions").asRecord("*2").asRecord("blc"));
			  Vector<Double> expblc(4);
			  expblc[0] = box5;
			  expblc[1] = box6;
			  expblc[2] = chan15;
			  expblc[3] = 4.0;
			  compVecs(gotblc, expblc);
			  Vector<Double> gottrc = recToVec(regRec.asRecord("regions").asRecord("*2").asRecord("trc"));
			  Vector<Double> exptrc(4);
			  exptrc[0] = box7;
			  exptrc[1] = box8;
			  exptrc[2] = chan19;
			  exptrc[3] = 4.0;
			  compVecs(gottrc, exptrc);

			  // box="5,6,7,8", chans="0~4", stokes="V"
			  gotblc = recToVec(
					  regRec.asRecord("regions").asRecord("*1").asRecord("regions").asRecord("*2")
					  .asRecord("blc")
			  );
			  expblc[0] = box5;
			  expblc[1] = box6;
			  expblc[2] = chan0;
			  expblc[3] = 4.0;
			  compVecs(gotblc, expblc);
			  gottrc = recToVec(
					  regRec.asRecord("regions").asRecord("*1").asRecord("regions").asRecord("*2")
					  .asRecord("trc")
			  );
			  exptrc[0] = box7;
			  exptrc[1] = box8;
			  exptrc[2] = chan4;
			  exptrc[3] = 4.0;
			  compVecs(gottrc, exptrc);

			  // box="5,6,7,8", chans="15-19", stokes="IQ"
			  gotblc = recToVec(
					  regRec.asRecord("regions").asRecord("*1").asRecord("regions").asRecord("*1")
					  .asRecord("regions").asRecord("*2").asRecord("blc")
			  );
			  expblc[0] = box5;
			  expblc[1] = box6;
			  expblc[2] = chan15;
			  expblc[3] = 1.0;
			  compVecs(gotblc, expblc);
			  gottrc = recToVec(
					  regRec.asRecord("regions").asRecord("*1").asRecord("regions").asRecord("*1")
					  .asRecord("regions").asRecord("*2").asRecord("trc")
			  );
			  exptrc[0] = box7;
			  exptrc[1] = box8;
			  exptrc[2] = chan19;
			  exptrc[3] = 2.0;
			  compVecs(gottrc, exptrc);

			  // box="5,6,7,8", chans="0~4", stokes="IQ"
			  gotblc = recToVec(
					  regRec.asRecord("regions").asRecord("*1").asRecord("regions").asRecord("*1")
					  .asRecord("regions").asRecord("*1").asRecord("regions").asRecord("*2").asRecord("blc")
			  );
			  expblc[0] = box5;
			  expblc[1] = box6;
			  expblc[2] = chan0;
			  expblc[3] = 1.0;
			  compVecs(gotblc, expblc);
			  gottrc = recToVec(
					  regRec.asRecord("regions").asRecord("*1").asRecord("regions").asRecord("*1")
					  .asRecord("regions").asRecord("*1").asRecord("regions").asRecord("*2").asRecord("trc")
			  );
			  exptrc[0] = box7;
			  exptrc[1] = box8;
			  exptrc[2] = chan4;
			  exptrc[3] = 2.0;
			  compVecs(gottrc, exptrc);

			  // box="1,2,3,4", chans="15-19", stokes="V"
			  gotblc = recToVec(
					  regRec.asRecord("regions").asRecord("*1").asRecord("regions").asRecord("*1")
					  .asRecord("regions").asRecord("*1").asRecord("regions")
					  .asRecord("*1").asRecord("regions").asRecord("*2").asRecord("blc")
			  );
			  expblc[0] = box1;
			  expblc[1] = box2;
			  expblc[2] = chan15;
			  expblc[3] = 4.0;
			  compVecs(gotblc, expblc);
			  gottrc = recToVec(
					  regRec.asRecord("regions").asRecord("*1").asRecord("regions").asRecord("*1")
					  .asRecord("regions").asRecord("*1").asRecord("regions").asRecord("*1")
					  .asRecord("regions").asRecord("*2").asRecord("trc")
			  );
			  exptrc[0] = box3;
			  exptrc[1] = box4;
			  exptrc[2] = chan19;
			  exptrc[3] = 4.0;
			  compVecs(gottrc, exptrc);

			  // box="1,2,3,4", chans="0-4", stokes="V"
			  gotblc = recToVec(
					  regRec.asRecord("regions").asRecord("*1").asRecord("regions").asRecord("*1")
					  .asRecord("regions").asRecord("*1").asRecord("regions")
					  .asRecord("*1").asRecord("regions").asRecord("*1").asRecord("regions").
					  asRecord("*2").asRecord("blc")
			  );
			  expblc[0] = box1;
			  expblc[1] = box2;
			  expblc[2] = chan0;
			  expblc[3] = 4.0;
			  compVecs(gotblc, expblc);
			  gottrc = recToVec(
					  regRec.asRecord("regions").asRecord("*1").asRecord("regions").asRecord("*1")
					  .asRecord("regions").asRecord("*1").asRecord("regions").asRecord("*1")
					  .asRecord("regions").asRecord("*1").asRecord("regions").
					  asRecord("*2").asRecord("trc")
			  );
			  exptrc[0] = box3;
			  exptrc[1] = box4;
			  exptrc[2] = chan4;
			  exptrc[3] = 4.0;
			  compVecs(gottrc, exptrc);

			  // box="1,2,3,4", chans="15-19", stokes="IQ"
			  gotblc = recToVec(
					  regRec.asRecord("regions").asRecord("*1").asRecord("regions").asRecord("*1")
					  .asRecord("regions").asRecord("*1").asRecord("regions").asRecord("*1").asRecord("regions")
					  .asRecord("*1").asRecord("regions").asRecord("*1").asRecord("regions")
					  .asRecord("*2").asRecord("blc")
			  );
			  expblc[0] = box1;
			  expblc[1] = box2;
			  expblc[2] = chan15;
			  expblc[3] = 1.0;
			  compVecs(gotblc, expblc);
			  gottrc = recToVec(
					  regRec.asRecord("regions").asRecord("*1").asRecord("regions").asRecord("*1")
					  .asRecord("regions").asRecord("*1").asRecord("regions").asRecord("*1")
					  .asRecord("regions").asRecord("*1").asRecord("regions").asRecord("*1").asRecord("regions")
					  .asRecord("*2").asRecord("trc")
			  );
			  exptrc[0] = box3;
			  exptrc[1] = box4;
			  exptrc[2] = chan19;
			  exptrc[3] = 2.0;
			  compVecs(gottrc, exptrc);

			  // box="1,2,3,4", chans="0-4", stokes="IQ"
			  gotblc = recToVec(
					  regRec.asRecord("regions").asRecord("*1").asRecord("regions").asRecord("*1")
					  .asRecord("regions").asRecord("*1").asRecord("regions").asRecord("*1").asRecord("regions")
					  .asRecord("*1").asRecord("regions").asRecord("*1").asRecord("regions").asRecord("*1").asRecord("blc")
			  );
			  expblc[0] = box1;
			  expblc[1] = box2;
			  expblc[2] = chan0;
			  expblc[3] = 1.0;
			  compVecs(gotblc, expblc);
			  gottrc = recToVec(
					  regRec.asRecord("regions").asRecord("*1").asRecord("regions").asRecord("*1")
					  .asRecord("regions").asRecord("*1").asRecord("regions").asRecord("*1").asRecord("regions")
					  .asRecord("*1").asRecord("regions").asRecord("*1").asRecord("regions").asRecord("*1").asRecord("trc")
			  );
			  exptrc[0] = box3;
			  exptrc[1] = box4;
			  exptrc[2] = chan4;
			  exptrc[3] = 2.0;
			  compVecs(gottrc, exptrc);
		  }
		  {
			  CasacRegionManager rm(myImageNoSpec->coordinates());
			  IPosition imShape = myImageNoSpec->shape();
			  diagnostics = "";
			  nSelectedChannels = 0;
			  stokes = "IQV";
			  chans = "";
			  stokesControl = CasacRegionManager::USE_ALL_STOKES;
			  box = "1,2,3,4,5,6,7,8";
			  writeTestString("Test multiple stokes ranges, and multiple boxes on image with no spectral axis");
			  regRec = rm.fromBCS(
					  diagnostics, nSelectedChannels, stokes,
					  0, "", chans, stokesControl, box,
					  imShape
			  );
			  AlwaysAssert(nSelectedChannels == 0, AipsError);

			  // box="5,6,7,8", stokes="V"
			  Vector<Double> gotblc = recToVec(regRec.asRecord("regions").asRecord("*2").asRecord("blc"));
			  Vector<Double> expblc(3);
			  expblc[0] = box5;
			  expblc[1] = box6;
			  expblc[2] = 4.0;
			  compVecs(gotblc, expblc);
			  Vector<Double> gottrc = recToVec(regRec.asRecord("regions").asRecord("*2").asRecord("trc"));
			  Vector<Double> exptrc(3);
			  exptrc[0] = box7;
			  exptrc[1] = box8;
			  exptrc[2] = 4.0;
			  compVecs(gottrc, exptrc);

			  // box="5,6,7,8", stokes="IQ"
			  gotblc = recToVec(regRec.asRecord("regions").asRecord("*1").asRecord("regions").asRecord("*2")
					  .asRecord("blc"));
			  expblc(3);
			  expblc[0] = box5;
			  expblc[1] = box6;
			  expblc[2] = 1.0;
			  compVecs(gotblc, expblc);
			  gottrc = recToVec(regRec.asRecord("regions").asRecord("*1").asRecord("regions").asRecord("*2")
					  .asRecord("trc"));
			  exptrc[0] = box7;
			  exptrc[1] = box8;
			  exptrc[2] = 2.0;
			  compVecs(gottrc, exptrc);

			  // box="1,2,3,4", stokes="V"
			  gotblc = recToVec(regRec.asRecord("regions").asRecord("*1").asRecord("regions").asRecord("*1")
					  .asRecord("regions").asRecord("*2").asRecord("blc"));
			  expblc(3);
			  expblc[0] = box1;
			  expblc[1] = box2;
			  expblc[2] = 4.0;
			  compVecs(gotblc, expblc);
			  gottrc = recToVec(regRec.asRecord("regions").asRecord("*1").asRecord("regions").asRecord("*1")
					  .asRecord("regions").asRecord("*2").asRecord("trc"));
			  exptrc[0] = box3;
			  exptrc[1] = box4;
			  exptrc[2] = 4.0;
			  compVecs(gottrc, exptrc);

			  // box="1,2,3,4", stokes="IQ"
			  gotblc = recToVec(regRec.asRecord("regions").asRecord("*1").asRecord("regions").asRecord("*1")
					  .asRecord("regions").asRecord("*1").asRecord("blc"));
			  expblc(3);
			  expblc[0] = box1;
			  expblc[1] = box2;
			  expblc[2] = 1.0;
			  compVecs(gotblc, expblc);
			  gottrc = recToVec(regRec.asRecord("regions").asRecord("*1").asRecord("regions").asRecord("*1")
					  .asRecord("regions").asRecord("*1").asRecord("trc"));
			  exptrc[0] = box3;
			  exptrc[1] = box4;
			  exptrc[2] = 2.0;
			  compVecs(gottrc, exptrc);
		  }
		  {
			  CasacRegionManager rm(myImageDirOnly->coordinates());
			  IPosition imShape = myImageDirOnly->shape();
			  String imname = myImageDirOnly->name();
			  diagnostics = "";
			  nSelectedChannels = 0;
			  stokes = "";
			  chans = "";
			  stokesControl = CasacRegionManager::USE_ALL_STOKES;
			  box = "1,2,3,4,5,6,7,8";
			  writeTestString("Test multiple boxes on image with direction coordinate only");
			  regRec = rm.fromBCS(
					  diagnostics, nSelectedChannels, stokes,
					  0, "", chans, stokesControl, box,
					  imShape
			  );
			  AlwaysAssert(nSelectedChannels == 0, AipsError);
			  // box="5,6,7,8"
			  Vector<Double> gotblc = recToVec(regRec.asRecord("regions").asRecord("*2").asRecord("blc"));
			  Vector<Double> expblc(2);
			  expblc[0] = box5;
			  expblc[1] = box6;
			  compVecs(gotblc, expblc);
			  Vector<Double> gottrc = recToVec(regRec.asRecord("regions").asRecord("*2").asRecord("trc"));
			  Vector<Double> exptrc(2);
			  exptrc[0] = box7;
			  exptrc[1] = box8;
			  compVecs(gottrc, exptrc);
			  // box="1,2,3,4"
			  gotblc = recToVec(regRec.asRecord("regions").asRecord("*1").asRecord("blc"));
			  expblc[0] = box1;
			  expblc[1] = box2;
			  compVecs(gotblc, expblc);
			  gottrc = recToVec(regRec.asRecord("regions").asRecord("*2").asRecord("trc"));
			  exptrc[0] = box7;
			  exptrc[1] = box8;
			  compVecs(gottrc, exptrc);
		  }
		  {
			  TempImage<Float> ti(IPosition(4, 40,40,2,2), CoordinateUtil::defaultCoords4D());
			  CasacRegionManager rm(ti.coordinates());
			  diagnostics = "";
			  nSelectedChannels = 0;
			  stokes = "";
			  chans = "";
			  stokesControl = CasacRegionManager::USE_ALL_STOKES;
			  box = "";
			  String region = "box[[1pix, 1pix], [4pix, 4pix]]";
			  writeTestString("Test CAS-3603 fix");
			  regRec = rm.fromBCS(
			      diagnostics, nSelectedChannels, stokes,
			      0, region, chans, stokesControl, box,
			      ti.shape()
			  );
			  // the verification is just to make sure this call doesn't segfault
		  }
		  {
			  writeTestString("Test setSpectralRanges throws exception if range is outside image");
			  uInt nSelChans = 0;
			  Bool thrown = True;
			  try {
				  vector<uInt> range = rm.setSpectralRanges("range=[300pix, 400pix]", nSelChans, imShape);
				  thrown = False;
			  }
			  catch (const AipsError& x) {
				  // caught as expected
			  }
			  AlwaysAssert(thrown == True, AipsError);

		  }
		  {
			  writeTestString("Test setSpectralRanges does the right thing when region record supplied.");
			  uInt nSelectedChannels;
			  String regionName = "";
			  String box = "";
			  regRec = rm.fromBCS(
			      diagnostics, nSelectedChannels, stokes,
			      0, regionName, String("5~10,15,18"),
			      CasacRegionManager::USE_ALL_STOKES, box,
			      imShape
			  );
			  vector<uInt> x = rm.setSpectralRanges(nSelectedChannels, &regRec, imShape);
			  AlwaysAssert(nSelectedChannels == 8, AipsError);
			  AlwaysAssert(x.size() == 6, AipsError);
			  AlwaysAssert(x[0] == 5, AipsError);
			  AlwaysAssert(x[1] == 10, AipsError);
			  AlwaysAssert(x[2] == 15, AipsError);
			  AlwaysAssert(x[3] == 15, AipsError);
			  AlwaysAssert(x[4] == 18, AipsError);
			  AlwaysAssert(x[5] == 18, AipsError);

		  }
	  }
	  catch (const AipsError& x) {
		  cerr << "Unexpected exception: " << x.getMesg() << endl;
		  return 1;
	  }
	  cout << "ok" << endl;
	  return 0;
}
