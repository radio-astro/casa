/*
 * tMSBin.cc
 *
 *Tests MSBin functionality
//#
//# Copyright (C) 2014
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
 *  Created on: Jan 16, 2014
 *      Author: kgolap
 */
#include <coordinates/Coordinates/CoordinateSystem.h>
#include <coordinates/Coordinates/DirectionCoordinate.h>
#include <coordinates/Coordinates/SpectralCoordinate.h>
#include <coordinates/Coordinates/StokesCoordinate.h>
#include <coordinates/Coordinates/Projection.h>
#include <measures/Measures/MDirection.h>
#include <msvis/MSVis/VisBuffer2.h>
#include <msvis/MSVis/VisibilityIterator2.h>
#include <ms/MeasurementSets/MSMainColumns.h>
#include <mstransform/MSTransform/MSTransformDataHandler.h>
#include <mstransform/MSTransform/MSUVBin.h>
#include <tables/Tables/TableIter.h>
#include <casa/namespace.h>

using namespace std;

Vector<Double> increment_p(2);
Vector<Int> center_p(2);
Vector<Int> npix_p(2);
Vector<Int> chanMap_p;
Vector<Int> polMap_p;

Int makeUVW(const DirectionCoordinate& thedir, Matrix<Double>& uvw, const Int nx, const Int ny, const Double reffreq){
	Vector<Int> shp(2);
	shp(0)=nx; shp(1)=ny;
	Coordinate *ftcoord=thedir.makeFourierCoordinate(Vector<Bool>(2, True), shp);
	increment_p=ftcoord->increment();
	increment_p *= C::c/reffreq;
	center_p(0)=nx/2;
	center_p(1)=ny/2;
	npix_p(0)=nx;
	npix_p(1)=ny;
	uvw.resize(3, nx*ny);
	uvw.set(0.0);
	Vector<Double> px(2);
	Vector<Double> wld(2);
	Int counter=0;
    for (Int k=0; k < ny;++k ){
    	px(1)=Double(k);
    	for(Int j=0; j < nx; ++j){
    		px(0)=Double(j);
    		if(ftcoord->toWorld(wld, px)){
    			uvw(0, k*nx+j)=wld(0)*C::c/reffreq;
    			uvw(1, k*nx+j)=wld(1)*C::c/reffreq;
    			++counter;
    		}
    	}
    }
	return counter;
}

void locateuvw(Matrix<Int>& locuv, const Matrix<Double>& uvw){
	locuv.resize(2, uvw.shape()(1));
	for (Int k=0; k <uvw.shape()(1); ++k){
		for(Int j=0; j < 2; ++j)
			locuv(j,k)=Int(center_p(j)+uvw(j,k)/increment_p(j));
	}


}

void gridData(const vi::VisBuffer2& vb, Cube<Complex>& grid,
		Matrix<Float>& wght, Cube<Float>& wghtSpec,
		Cube<Bool>& flag, Vector<Bool>& rowFlag, Matrix<Double>& uvw){
//all pixel that are touched the flag and flag Row shall be unset and the w be assigned
	//later we'll deal with multiple w for the same uv
	//we need polmap and chanmap;
    Matrix<Int>	locuv;
    //Dang i thought the new vb will return Data or FloatData if correctedData was
    //not there
    Bool hasCorrected=!(ROMSMainColumns(vb.getVi()->ms()).correctedData().isNull());
	locateuvw(locuv, vb.uvw());
	for (Int k=0; k < vb.nRows(); ++k){
		Int newrow=locuv(1,k)*npix_p(0)+locuv(0,k);
		if(rowFlag(newrow) && !(vb.flagRow()(k))){
			rowFlag(newrow)=False;
			uvw(2,newrow)=vb.uvw()(2,k);

		}
		for(Int chan=0; chan < vb.nChannels(); ++chan ){
			for(Int pol=0; pol < vb.nCorrelations(); ++pol){
				if(!vb.flagCube()(pol,chan, k)){
					grid(polMap_p(pol),chanMap_p(chan), newrow)
								+= hasCorrected ? vb.visCubeCorrected()(pol,chan,k):
										vb.visCube()(pol,chan,k);
					flag(polMap_p(pol),chanMap_p(chan), newrow)=False;
					wghtSpec(polMap_p(pol),chanMap_p(chan), newrow)+=vb.weightSpectrum()(pol, chan, k);
				}
			}
		}
		//sum wgtspec along channels for weight
		for (Int pol=0; pol < wght.shape()(0); ++pol){
			wght(pol,newrow)=sum(wghtSpec.xyPlane(k).row(pol));
		}
	}


}

int main(int argc, char **argv) {

	if (argc<2) {
		cout <<"Usage: tMSBin ms-table-name "<<endl;
	    exit(1);
	}
	//////////////
	String msname(argv[1]);
	MeasurementSet myms(msname, Table::Old);

	cerr << "spec type" << ROMSSpWindowColumns(myms.spectralWindow()).measFreqRef().getColumn() << endl;
	//////////////
	MDirection  phasecenter(Quantity(230.5,"deg"), Quantity(5.0667, "deg"), MDirection::J2000);
	//MSUVBin binner(phasecenter, 256,
	//		256, 63, 2, Quantity(15,"arcsec"), Quantity(15,"arcsec"), Quantity(1412.665,"MHz"), Quantity(24.414,"kHz"));
	MSUVBin binner(phasecenter, 512,
				512, 63, 2, Quantity(15.,"arcsec"), Quantity(15.,"arcsec"), Quantity(1412.665,"MHz"), Quantity(24.414,"kHz"));
	binner.selectData(String(argv[1]), "0","0");
	binner.setOutputMS("OutMS.ms");
	binner.fillOutputMS();
//	MeasurementSet myms(argv[1],Table::Old);
	/////////////
/*
	 Block<String> iv0(1);
	 iv0[0] = "ROWID";
	//    iv0[1] = "baseline";
	//    // Create the iterator. This will prepare the first subtable.
	TableIterator iter(myms, iv0);
	Int nr=0;
	Table t;
	while (!iter.pastEnd()) {

	t = iter.table();
	cout << t.nrow() << " ";
	nr++;
		iter.next();
	}
	cout << "nr " << nr << endl;
	*/
	///////////////

/*
	vi::VisibilityIterator2 vi_p(myms, vi::SortColumns(), False);
	vi_p.originChunks();
	vi_p.origin();
	vi::VisBuffer2* vb=vi_p.getVisBuffer();
	// For now input nchan is output nchan
	// one to one mapping
	Int nchan=vb->nChannels();
	Int npol=vb->nCorrelations();
	chanMap_p.resize(nchan);
	polMap_p.resize(npol);
	indgen(chanMap_p);
	indgen(polMap_p);
	//////////
	Vector<Int> tileShape(3);
	tileShape[0]=4; tileShape[1]=200; tileShape[2]=500;
	Int nAnt=vb->nAntennas();
	Int nx=500;
	Int ny=500;
	Matrix<Double> xform(2,2);
	xform=0.0;xform.diagonal()=1.0;
	MDirection phasecenter(Quantity(60,"deg"), Quantity(20, "deg"), MDirection::J2000);
	DirectionCoordinate radec(MDirection::J2000, Projection(Projection::SIN), Quantity(60, "deg"),
			Quantity(20, "deg"), Quantity(1, "arcsec"), Quantity(1, "arcsec"), xform, nx/2.0, ny/2.0);

	//value of middle channel for example
	Double reffreq=1.420e9;
	Matrix<Double> uvw;
    Int nrrows=makeUVW(radec, uvw, nx, ny, reffreq);
    String outms("outPut.ms");
    if(Table::isReadable(outms))
    	Table::deleteTable(outms);
	MeasurementSet *msPtr=MSTransformDataHandler::setupMS(outms, nchan, npol,
			Vector<MS::PredefinedColumns>(1, MS::DATA),
			tileShape);
*/
	/*MeasurementSet *msPtr=MSTransformDataHandler::setupMS("outPut.ms", 8192, 4, "VLA",
				Vector<MS::PredefinedColumns>(1, MS::DATA));
*/

/*	Cube<Complex> grid(npol, nchan, nrrows);
	Matrix<Float> wght(npol, nrrows);
	Cube<Float> wghtSpec(npol, nchan, nrrows);
	Cube<Bool> flag(npol, nchan, nrrows);
	Vector<Bool> rowFlag(nrrows);
	msPtr->addRow(nrrows, True);
	for (vi_p.originChunks(); vi_p.moreChunks(); vi_p.nextChunk()){
		for(vi_p.origin(); vi_p.more(); vi_p.next()){
			gridData(*vb, grid, wght, wghtSpec,flag, rowFlag,uvw);
		}
	}
	//MSTransformDataHandler::createSubtables(*msPtr, Table::New);
	MSTransformDataHandler::addOptionalColumns(myms.spectralWindow(),
				msPtr->spectralWindow());
	///Setup pointing
	{
	SetupNewTable pointingSetup(msPtr->pointingTableName(),
	                              MSPointing::requiredTableDesc(), Table::New);
	  // POINTING can be large, set some sensible defaults for storageMgrs
	  IncrementalStMan ismPointing ("ISMPointing");
	  StandardStMan ssmPointing("SSMPointing", 32768);
	  pointingSetup.bindAll(ismPointing, True);
	  pointingSetup.bindColumn(MSPointing::columnName(MSPointing::DIRECTION),
	                           ssmPointing);
	  pointingSetup.bindColumn(MSPointing::columnName(MSPointing::TARGET),
	                           ssmPointing);
	  pointingSetup.bindColumn(MSPointing::columnName(MSPointing::TIME),
	                           ssmPointing);
	  msPtr->rwKeywordSet().defineTable(MS::keywordName(MS::POINTING),
	                                     Table(pointingSetup));
	  msPtr->initRefs();
	}
	TableCopy::copySubTables(msPtr->pointing(), myms.pointing());
	MSColumns msc(*msPtr);
	msc.uvw().putColumn(uvw);
	msc.data().putColumn(grid);
	msc.weightSpectrum().putColumn(wghtSpec);
	msc.weight().putColumn(wght);
	msc.flag().putColumn(flag);
	msc.flagRow().putColumn(rowFlag);




	delete msPtr;
	
*/



	return 0;
}



