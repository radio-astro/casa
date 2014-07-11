/*
 * MSUVBin.h: This file contains the interface definition of the MSUVBin class.
//#
//#  CASA - Common Astronomy Software Applications (http://casa.nrao.edu/)

 //# Copyright (C) 2014
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

//#        Postal address:
//#                        National Radio Astronomy Observatory
//#                        520 Edgemont Road
//#                        Charlottesville, VA 22903-2475 USA
//#

 *  Created on: Feb 3, 2014
 *      Author: kgolap
 */

#ifndef MSTRANSFORM_MSUVBIN_H
#define MSTRANSFORM_MSUVBIN_H
#include <ms/MeasurementSets/MeasurementSet.h>
#include <coordinates/Coordinates/CoordinateSystem.h>
#include <msvis/MSVis/VisBuffer2.h>
#include <msvis/MSVis/VisBuffer.h>
#include <msvis/MSVis/VisBufferUtil.h>
#include <synthesis/MSVis/VisBuffer.h>
#include <synthesis/MSVis/VisBufferUtil.h>
namespace casa { //# NAMESPACE CASA - BEGIN


class MSUVBin {
public:
	MSUVBin();
	//npol should be only 1, 2 (parallel hands)  or 4
	MSUVBin(const MDirection& phaseCenter, const Int nx,
			const Int ny, const Int nchan, const Int npol, Quantity cellx, Quantity celly, Quantity freqStart, Quantity freqStep);
	//Constructor to bin into an existing gridded ms
	MSUVBin (MeasurementSet& ms);
	//The following can be called multiple times to setup multiple input MS
	virtual Bool selectData(const String& msname, const String& spw = "", const String& field = "",
			const String& baseline = "", const String& scan = "",
			const String& uvrange = "", const String& taql = "",
			const String& subarray = "", const String& correlation = "",
			const String& intent = "", const String& obs = "");
	//void setInputMS(const MeasurementSet& ms);
	//void setInputMS(const Block<const MeasurementSet*> mssPtr);
	void setOutputMS(const String& msname);
	//forceDiskUsage is to avoid using in memory gridding even if there is
	Bool fillOutputMS(const Bool forceDiskUsage=False);
	virtual ~MSUVBin();
	//Helper function for creating MDirection from a string
	static Bool String2MDirection(const String& theString,
					            MDirection& theMeas, const String msname="");

private:
	static Int sepCommaEmptyToVectorStrings(Vector<String>& retStr,
			  const String& str);
	Bool fillSmallOutputMS();
	Bool fillBigOutputMS();
	Int recoverGridInfo(const String& msname);
	void storeGridInfo();
	void createOutputMS(const Int nrrows);
	Int makeUVW(const Double reffreq, Vector<Double>& incr, Vector<Int>& cent, Matrix<Double>&uvw);
	void locateuvw(Matrix<Int>& locuv, const Vector<Double>& increment,
			const Vector<Int>& center, const Matrix<Double>& uvw);
	void gridData(const vi::VisBuffer2& vb, Cube<Complex>& grid,
			Matrix<Float>& wght, Cube<Float>& wghtSpec,
			Cube<Bool>& flag, Vector<Bool>& rowFlag, Matrix<Double>& uvw, Vector<Int>& ant1,
			Vector<Int>& ant2, Vector<Double>& time, const Matrix<Int>& locuv);
	void gridData(const VisBuffer& vb, Cube<Complex>& grid,
			Matrix<Float>& wght, Cube<Float>& wghtSpec,
			Cube<Bool>& flag, Vector<Bool>& rowFlag, Matrix<Double>& uvw, Vector<Int>& ant1,
			Vector<Int>& ant2, Vector<Double>& time, const Matrix<Int>& locuv);
	void inplaceGridData(const vi::VisBuffer2& vb);
	void inplaceLargeBW(const vi::VisBuffer2& vb);
	void inplaceSmallBW(const vi::VisBuffer2& vb);
	void makeCoordsys();
	void weightSync();
	// returns a false if either no channel map or pol map onto grid
	Bool datadescMap(const vi::VisBuffer2& vb, Double& fracbw);
	Bool datadescMap(const VisBuffer& vb);
	Bool saveData(const Cube<Complex>& grid, const Cube<Bool>&flag, const Vector<Bool>& rowFlag,
				const Cube<Float>&wghtSpec, const Matrix<Float>& wght,
				const Matrix<Double>& uvw, const Vector<Int>& ant1,
				const Vector<Int>& ant2, const Vector<Double>& time);
	void fillSubTables();
	void fillFieldTable();
	void copySubtable(const String& tabName, const Table& inTab,
	                    const Bool noRows=False);
	void fillDDTables();
	CoordinateSystem csys_p;
	Int nx_p, ny_p, nchan_p, npol_p;
	Double freqStart_p, freqStep_p;
	Vector<Int> chanMap_p, polMap_p;
	MDirection phaseCenter_p;
	Vector<Double> deltas_p;
	Vector<Int> whichStokes_p;
	Bool existOut_p;
	String outMSName_p;
	CountedPtr<MeasurementSet> outMsPtr_p;
	Block<const MeasurementSet *> mss_p;
	VisBufferUtil vbutil_p;

}; // end class MSUVBin
} //# NAMESPACE CASA - END
#endif /* MSTRANSFORM_MSUVBIN_H */
