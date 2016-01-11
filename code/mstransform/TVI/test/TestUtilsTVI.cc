//# TestUtilsTVI.cc:  This file contains the implementation of the ChannelAverageTVI class.
//#
//#  CASA - Common Astronomy Software Applications (http://casa.nrao.edu/)
//#  Copyright (C) Associated Universities, Inc. Washington DC, USA 2011, All rights reserved.
//#  Copyright (C) European Southern Observatory, 2011, All rights reserved.
//#
//#  This library is free software; you can redistribute it and/or
//#  modify it under the terms of the GNU Lesser General Public
//#  License as published by the Free software Foundation; either
//#  version 2.1 of the License, or (at your option) any later version.
//#
//#  This library is distributed in the hope that it will be useful,
//#  but WITHOUT ANY WARRANTY, without even the implied warranty of
//#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
//#  Lesser General Public License for more details.
//#
//#  You should have received a copy of the GNU Lesser General Public
//#  License along with this library; if not, write to the Free Software
//#  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
//#  MA 02111-1307  USA
//# $Id: $

#include <mstransform/TVI/test/TestUtilsTVI.h>

namespace casa { //# NAMESPACE CASA - BEGIN

namespace vi { //# NAMESPACE VI - BEGIN

template <class T> Bool compareVector(	const Char* column,
										const Vector<T> &inp,
										const Vector<T> &ref,
										Float tolerance)
{
	// Check matching shape
	if (inp.size() != ref.size())
	{
		cout << RED;
		cout << column << " test and reference vectors don't have the same size"
				<< " test size=" << inp.size()
				<< " reference size=" << ref.size()
				<< endl;
		return False;
	}

	// Compare values
	Bool ret = True;
	for (uInt index=0;index < inp.size(); index++)
	{
		if (abs(inp(index) - ref(index)) > tolerance )
		{
			cout << RED;
			cout << column << " does not match in position="
					<< index
					<< " test=" << inp(index)
					<< " reference=" << ref(index)
					<< endl;
		}
	}

	if (ret)
	{
		cout << GREEN;
		cout 	<< "=>" << column << " match" << endl;
	}

	return ret;
}

template <class T> Bool compareMatrix(	const Char* column,
										const Matrix<T> &inp,
										const Matrix<T> &ref,
										Float tolerance)
{
	// Check matching shape
	if (inp.shape() != ref.shape())
	{
		cout << RED;
		cout << column << " test and reference matrix don't have the same shape"
				<< " test (row,col)="
				<< "("<< inp.shape()(1) << "," << inp.shape()(0) << ")"
				<< " reference (row,col)="
				<< "("<< ref.shape()(1) << "," << ref.shape()(0) << ")"
				<< endl;
		return False;
	}

	// Compare values
	Bool ret = True;
	const IPosition &shape = inp.shape();
	for (uInt row=0;row < shape(1) and ret; row++)
	{
		for (uInt col=0;col < shape(0) and ret; col++)
		{
			if (abs(inp(col,row) - ref(col,row)) > tolerance )
			{
				ret = False;
				cout << RED;
				cout << column << " does not match in position (row,col)="
						<< "("<< row << "," << col << ")"
						<< " test=" << inp(col,row)
						<< " reference=" << ref(col,row)
						<< endl;
			}
		}
	}

	if (ret)
	{
		cout << GREEN;
		cout 	<< "=>" << column << " match" << endl;
	}

	return ret;
}

template <class T> Bool compareCube(const Char* column,
									const Cube<T> &inp,
									const Cube<T> &ref,
									Float tolerance)
{
	// Check matching shape
	if (inp.shape() != ref.shape())
	{
		cout << RED;
		cout << column << " test and reference cubes don't have the same shape"
				<< " test (row,chan,corr)="
				<< "("<< inp.shape()(2) << "," << inp.shape()(1) << "," << inp.shape()(0) << ")"
				<< " reference (row,chan,corr)="
				<< "("<< ref.shape()(2) << "," << ref.shape()(1) << "," << ref.shape()(0) << ")"
				<< endl;
		return False;
	}

	// Compare values
	Bool ret = True;
	const IPosition &shape = inp.shape();
	for (uInt row=0;row < shape(2) and ret; row++)
	{
		for (uInt chan=0;chan < shape(1) and ret; chan++)
		{
			for (uInt corr=0;corr < shape(0) and ret; corr++)
			{
				if (abs(inp(corr,chan,row) - ref(corr,chan,row)) > tolerance )
				{
					ret = False;
					cout << RED;
					cout << column << " does not match in position (row,chan,corr)="
							<< "("<< row << "," << chan << "," << corr << ")"
							<< " test=" << inp(corr,chan,row)
							<< " reference=" << ref(corr,chan,row)
							<< endl;
				}
			}
		}
	}

	if (ret)
	{
		cout << GREEN;
		cout 	<< "=>" << column << " match" << endl;
	}

	return ret;
}

Bool compareVisibilityIterators(VisibilityIterator2 &testTVI,
								VisibilityIterator2 &refTVI,
								VisBufferComponents2 &columns,
								Float tolerance)
{
	// Declare working variables
	String columnName;
	Int chunk = 0,buffer = 0;
	Bool res, keepIterating = True;

	// Get VisBuffers
	VisBuffer2 *refVb = refTVI.getVisBuffer();
	VisBuffer2 *testVb = testTVI.getVisBuffer();

	// Compare selected columns
	refTVI.originChunks();
	testTVI.originChunks();
	while (refTVI.moreChunks() and testTVI.moreChunks() and keepIterating)
	{
		chunk += 1;
		buffer = 0;

		refTVI.origin();
		testTVI.origin();

		while (refTVI.more() and testTVI.more() and keepIterating)
		{
			buffer += 1;
			cout << BLUE;
			cout << " COMPARING CHUNK " << chunk << " BUFFER " << buffer << endl;

			if (columns.contains(NRows) and testTVI.existsColumn(NRows))
			{
				res = testVb->nRows() == refVb->nRows();
				if (not res)
				{
					keepIterating = False;
					cout << RED;
					cout << "Number of rows does not match "
							<< " test=" << testVb->nRows()
							<< " reference=" << refVb->nRows()
							<< endl;
				}
			}

			if (columns.contains(NChannels) and testTVI.existsColumn(NChannels))
			{
				res = testVb->nChannels() == refVb->nChannels();
				if (not res)
				{
					keepIterating = False;
					cout << RED;
					cout << "Number of channels does not match "
							<< " test=" << testVb->nChannels()
							<< " reference=" << refVb->nChannels()
							<< endl;
				}
			}

			if (columns.contains(NCorrelations) and testTVI.existsColumn(NCorrelations))
			{
				res = testVb->nCorrelations() == refVb->nCorrelations();
				if (not res)
				{
					keepIterating = False;
					cout << RED;
					cout << "Number of correlations does not match "
							<< " test=" << testVb->nCorrelations()
							<< " reference=" << refVb->nCorrelations()
							<< endl;
				}
			}

			if (columns.contains(FlagRow) and testTVI.existsColumn(FlagRow))
			{
				columnName = VisBufferComponents2::name(FlagRow);
				res = compareVector(columnName.c_str(),testVb->flagRow(),refVb->flagRow(),tolerance);
				if (not res) keepIterating = False;
			}

			if (columns.contains(FlagCube) and testTVI.existsColumn(FlagCube))
			{
				columnName = VisBufferComponents2::name(FlagCube);
				res = compareCube(columnName.c_str(),testVb->flagCube(),refVb->flagCube(),tolerance);
				if (not res) keepIterating = False;
			}

			if (columns.contains(VisibilityCubeObserved) and testTVI.existsColumn(VisibilityCubeObserved))
			{
				columnName = VisBufferComponents2::name(VisibilityCubeObserved);
				res = compareCube(columnName.c_str(),testVb->visCube(),refVb->visCube(),tolerance);
				if (not res) keepIterating = False;
			}

			if (columns.contains(VisibilityCubeCorrected) and testTVI.existsColumn(VisibilityCubeCorrected))
			{
				columnName = VisBufferComponents2::name(VisibilityCubeCorrected);
				res = compareCube(columnName.c_str(),testVb->visCubeCorrected(),refVb->visCubeCorrected(),tolerance);
				if (not res) keepIterating = False;
			}

			if (columns.contains(VisibilityCubeModel) and testTVI.existsColumn(VisibilityCubeModel))
			{
				columnName = VisBufferComponents2::name(VisibilityCubeModel);
				res = compareCube(columnName.c_str(),testVb->visCubeModel(),refVb->visCubeModel(),tolerance);
				if (not res) keepIterating = False;
			}

			if (columns.contains(VisibilityCubeFloat) and testTVI.existsColumn(VisibilityCubeFloat))
			{
				columnName = VisBufferComponents2::name(VisibilityCubeFloat);
				res = compareCube(columnName.c_str(),testVb->visCubeFloat(),refVb->visCubeFloat(),tolerance);
				if (not res) keepIterating = False;
			}

			if (columns.contains(WeightSpectrum) and testTVI.existsColumn(WeightSpectrum))
			{
				columnName = VisBufferComponents2::name(WeightSpectrum);
				res = compareCube(columnName.c_str(),testVb->weightSpectrum(),refVb->weightSpectrum(),tolerance);
				if (not res) keepIterating = False;
			}

			if (columns.contains(SigmaSpectrum) and testTVI.existsColumn(SigmaSpectrum))
			{
				columnName = VisBufferComponents2::name(SigmaSpectrum);
				res = compareCube(columnName.c_str(),testVb->sigmaSpectrum(),refVb->sigmaSpectrum(),tolerance);
				if (not res) keepIterating = False;
			}

			if (columns.contains(Weight) and testTVI.existsColumn(Weight))
			{
				columnName = VisBufferComponents2::name(Weight);
				res = compareMatrix(columnName.c_str(),testVb->weight(),refVb->weight(),tolerance);
				if (not res) keepIterating = False;
			}

			if (columns.contains(Sigma) and testTVI.existsColumn(Sigma))
			{
				columnName = VisBufferComponents2::name(Sigma);
				res = compareMatrix(columnName.c_str(),testVb->sigma(),refVb->sigma(),tolerance);
				if (not res) keepIterating = False;
			}

			refTVI.next();
			testTVI.next();
		}

		refTVI.nextChunk();
		testTVI.nextChunk();
	}

	cout << RESET << endl;

	return keepIterating;
}


} //# NAMESPACE VI - END

} //# NAMESPACE CASA - END
