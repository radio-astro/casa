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

//////////////////////////////////////////////////////////////////////////
// FreqAxisTVITest class
//////////////////////////////////////////////////////////////////////////

// -----------------------------------------------------------------------
//
// -----------------------------------------------------------------------
FreqAxisTVITest::FreqAxisTVITest():
		autoMode_p(True), testResult_p(True)
{

}

// -----------------------------------------------------------------------
//
// -----------------------------------------------------------------------
FreqAxisTVITest::FreqAxisTVITest(Record configuration):
		autoMode_p(False), testResult_p(True)
{
	configuration.get (configuration.fieldNumber ("inputms"), inpFile_p);
	testFile_p = inpFile_p + String(".test");
	referenceFile_p = inpFile_p + String(".ref");
}

// -----------------------------------------------------------------------
//
// -----------------------------------------------------------------------
FreqAxisTVITest::~FreqAxisTVITest()
{
	TearDown();

	return;
}

// -----------------------------------------------------------------------
//
// -----------------------------------------------------------------------
void FreqAxisTVITest::init(Record &configuration)
{
	initTestConfiguration(configuration);
	initReferenceConfiguration(configuration);
}

// -----------------------------------------------------------------------
//
// -----------------------------------------------------------------------
void FreqAxisTVITest::SetUp()
{
	// Generate test file
	generateTestFile();

	// Generate reference file
	generateReferenceFile();

	return;
}

// -----------------------------------------------------------------------
//
// -----------------------------------------------------------------------
void FreqAxisTVITest::TearDown()
{
	String rm_command;

	rm_command = String ("rm -rf ") + testFile_p;
	system(rm_command.c_str());

	rm_command = String ("rm -rf ") + referenceFile_p;
	system(rm_command.c_str());

	return;
}


//////////////////////////////////////////////////////////////////////////
// Convenience methods
//////////////////////////////////////////////////////////////////////////

// -----------------------------------------------------------------------
//
// -----------------------------------------------------------------------
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
			ret = False;
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

// -----------------------------------------------------------------------
//
// -----------------------------------------------------------------------
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

// -----------------------------------------------------------------------
//
// -----------------------------------------------------------------------
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

// -----------------------------------------------------------------------
//
// -----------------------------------------------------------------------
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
	try
	{
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

				if (columns.contains(VisBufferComponent2::NRows) and testTVI.existsColumn(VisBufferComponent2::NRows))
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

				if (columns.contains(VisBufferComponent2::NChannels) and testTVI.existsColumn(VisBufferComponent2::NChannels))
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

				if (columns.contains(VisBufferComponent2::NCorrelations) and testTVI.existsColumn(VisBufferComponent2::NCorrelations))
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

				if (columns.contains(VisBufferComponent2::FlagRow) and testTVI.existsColumn(VisBufferComponent2::FlagRow))
				{
					columnName = VisBufferComponents2::name(VisBufferComponent2::FlagRow);
					res = compareVector(columnName.c_str(),testVb->flagRow(),refVb->flagRow(),tolerance);
					if (not res) keepIterating = False;
				}

				if (columns.contains(VisBufferComponent2::FlagCube) and testTVI.existsColumn(VisBufferComponent2::FlagCube))
				{
					columnName = VisBufferComponents2::name(VisBufferComponent2::FlagCube);
					res = compareCube(columnName.c_str(),testVb->flagCube(),refVb->flagCube(),tolerance);
					if (not res) keepIterating = False;
				}

				if (columns.contains(VisBufferComponent2::VisibilityCubeObserved) and testTVI.existsColumn(VisBufferComponent2::VisibilityCubeObserved))
				{
					columnName = VisBufferComponents2::name(VisBufferComponent2::VisibilityCubeObserved);
					res = compareCube(columnName.c_str(),testVb->visCube(),refVb->visCube(),tolerance);
					if (not res) keepIterating = False;
				}

				if (columns.contains(VisBufferComponent2::VisibilityCubeCorrected) and testTVI.existsColumn(VisBufferComponent2::VisibilityCubeCorrected))
				{
					columnName = VisBufferComponents2::name(VisBufferComponent2::VisibilityCubeCorrected);
					res = compareCube(columnName.c_str(),testVb->visCubeCorrected(),refVb->visCubeCorrected(),tolerance);
					if (not res) keepIterating = False;
				}

				if (columns.contains(VisBufferComponent2::VisibilityCubeModel) and testTVI.existsColumn(VisBufferComponent2::VisibilityCubeModel))
				{
					columnName = VisBufferComponents2::name(VisBufferComponent2::VisibilityCubeModel);
					res = compareCube(columnName.c_str(),testVb->visCubeModel(),refVb->visCubeModel(),tolerance);
					if (not res) keepIterating = False;
				}

				if (columns.contains(VisBufferComponent2::VisibilityCubeFloat) and testTVI.existsColumn(VisBufferComponent2::VisibilityCubeFloat))
				{
					columnName = VisBufferComponents2::name(VisBufferComponent2::VisibilityCubeFloat);
					res = compareCube(columnName.c_str(),testVb->visCubeFloat(),refVb->visCubeFloat(),tolerance);
					if (not res) keepIterating = False;
				}

				if (columns.contains(VisBufferComponent2::WeightSpectrum) and testTVI.existsColumn(VisBufferComponent2::WeightSpectrum))
				{
					columnName = VisBufferComponents2::name(VisBufferComponent2::WeightSpectrum);
					res = compareCube(columnName.c_str(),testVb->weightSpectrum(),refVb->weightSpectrum(),tolerance);
					if (not res) keepIterating = False;
				}

				if (columns.contains(VisBufferComponent2::SigmaSpectrum) and testTVI.existsColumn(VisBufferComponent2::SigmaSpectrum))
				{
					columnName = VisBufferComponents2::name(VisBufferComponent2::SigmaSpectrum);
					res = compareCube(columnName.c_str(),testVb->sigmaSpectrum(),refVb->sigmaSpectrum(),tolerance);
					if (not res) keepIterating = False;
				}

				if (columns.contains(VisBufferComponent2::Weight) and testTVI.existsColumn(VisBufferComponent2::Weight))
				{
					columnName = VisBufferComponents2::name(VisBufferComponent2::Weight);
					res = compareMatrix(columnName.c_str(),testVb->weight(),refVb->weight(),tolerance);
					if (not res) keepIterating = False;
				}

				if (columns.contains(VisBufferComponent2::Sigma) and testTVI.existsColumn(VisBufferComponent2::Sigma))
				{
					columnName = VisBufferComponents2::name(VisBufferComponent2::Sigma);
					res = compareMatrix(columnName.c_str(),testVb->sigma(),refVb->sigma(),tolerance);
					if (not res) keepIterating = False;
				}

				if (columns.contains(VisBufferComponent2::Frequencies))
				{
					columnName = VisBufferComponents2::name(VisBufferComponent2::Frequencies);
					res = compareVector(columnName.c_str(),testVb->getFrequencies(0),refVb->getFrequencies(0),tolerance);
					if (not res) keepIterating = False;
				}

				refTVI.next();
				testTVI.next();
			}

			refTVI.nextChunk();
			testTVI.nextChunk();
		}
	}
	catch (AipsError &ex)
	{
		LogIO logger;
		logger 	<< LogIO::SEVERE
					<< "Exception comparing visibility iterators: " << ex.getMesg() << endl
					<< "Stack Trace: " << ex.getStackTrace()
					<< LogIO::POST;
		keepIterating = False;
	}


	cout << RESET << endl;

	return keepIterating;
}

// -----------------------------------------------------------------------
//
// -----------------------------------------------------------------------
Bool copyTestFile(String &path,String &filename,String &outfilename)
{
	Bool ret = True;

	if (path.size() > 0)
	{
		char* pathChar = getenv ("CASAPATH");
		if (pathChar != NULL)
		{
			// Get base path
			String pathStr(pathChar);
			String res[2];
			casa::split(pathChar,res,2,String(" "));

			// Generate full qualified filename
			String fullfilename(res[0]);
			fullfilename += path + "/" + filename;

			// Remove any previously existing copy
			String rm_command = String ("rm -rf ") + outfilename;
			system(rm_command.c_str());

			// Make a copy of the file in the working directory
			String cp_command = String ("cp -r ") + fullfilename + String(" ") + outfilename;
			Int ret1 = system(cp_command.c_str());

			// Check that copy command was successful
			if (ret1 != 0)
			{
				cout << RED;
				cout << "TEST FILE NOT FOUND: " << fullfilename << endl;
				cout << RESET;

				ret = False;
			}
		}
		else
		{
			cout << RED;
			cout << "CASAPATH ENVIRONMENTAL VARIABLE NOT DEFINED" << endl;
			cout << RESET;

			ret = False;
		}
	}
	else
	{
		// Remove any previously existing copy
		String rm_command = String ("rm -rf ") + outfilename;
		system(rm_command.c_str());

		// Make a copy of the file in the working directory
		String cp_command = String ("cp -r ") + filename + String(" ") + outfilename;
		Int ret1 = system(cp_command.c_str());

		// Check that copy command was successful
		if (ret1 != 0)
		{
			cout << RED;
			cout << "TEST FILE NOT FOUND: " << filename << endl;
			cout << RESET;

			ret = False;
		}
	}

	return ret;
}

// -----------------------------------------------------------------------
//
// -----------------------------------------------------------------------
void flagEachOtherChannel(VisibilityIterator2 &vi)
{
	// Declare working variables
	Int chunk = 0,buffer = 0;

	// Get VisBuffer
	VisBuffer2 *vb = vi.getVisBuffer();

	// Propagate flags
	vi.originChunks();
	while (vi.moreChunks())
	{
		chunk += 1;
		buffer = 0;

		vi.origin();
		vi.origin();

		while (vi.more())
		{
			buffer += 1;

			// Initialize flag cube
			IPosition shape = vb->getShape();
			Cube<Bool> flagCube(shape,False);

			// Switch each other buffer the sign of the flag of the first block of channels
			Bool firstChanBlockFlag = buffer % 2? True:False;

			// Fill flag cube alternating flags per blocks channels
			size_t nCorr = shape(0);
			size_t nChan = shape(1);
			size_t nRows = shape(2);
			for (size_t row_i =0;row_i<nRows;row_i++)
			{
				// Row completely flagged
				if (row_i % 2)
				{
					flagCube.xyPlane(row_i) = True;
				}
				else
				{
					for (size_t chan_i =0;chan_i<nChan;chan_i++)
					{
						// Set the flags in each other block of channels
						Bool chanBlockFlag = chan_i % 2? firstChanBlockFlag:!firstChanBlockFlag;

						for (size_t corr_i =0;corr_i<nCorr;corr_i++)
						{
							flagCube(corr_i,chan_i,row_i) = chanBlockFlag;
						}
					}
				}
			}

			vi.writeFlag(flagCube);
			vi.next();
		}

		vi.nextChunk();
	}

	return;
}

} //# NAMESPACE VI - END

} //# NAMESPACE CASA - END
