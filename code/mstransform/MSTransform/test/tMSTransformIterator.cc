//# tMSTransformDataHandlerr.cc: This file contains the unit tests of the MsTransformDataHandler class.
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

#include <mstransform/MSTransform/MSTransformIteratorFactory.h>
#include <synthesis/MSVis/VisibilityIterator2.h>
#include <synthesis/MSVis/VisBuffer2.h>

#include <string>
#include <iostream>

using namespace casa;
using namespace std;

#define RESET   "\033[0m"
#define BLACK   "\033[30m"      /* Black */
#define RED     "\033[31m"      /* Red */
#define GREEN   "\033[32m"      /* Green */
#define YELLOW  "\033[33m"      /* Yellow */
#define BLUE    "\033[34m"      /* Blue */
#define MAGENTA "\033[35m"      /* Magenta */
#define CYAN    "\033[36m"      /* Cyan */
#define WHITE   "\033[37m"      /* White */
#define BOLDBLACK   "\033[1m\033[30m"      /* Bold Black */
#define BOLDRED     "\033[1m\033[31m"      /* Bold Red */
#define BOLDGREEN   "\033[1m\033[32m"      /* Bold Green */
#define BOLDYELLOW  "\033[1m\033[33m"      /* Bold Yellow */
#define BOLDBLUE    "\033[1m\033[34m"      /* Bold Blue */
#define BOLDMAGENTA "\033[1m\033[35m"      /* Bold Magenta */
#define BOLDCYAN    "\033[1m\033[36m"      /* Bold Cyan */
#define BOLDWHITE   "\033[1m\033[37m"      /* Bold White */

Record parseConfiguration(int argc, char **argv)
{
	string parameter,value;
	Record configuration;
	Bool autoMode = True;

	for (unsigned short i=0;i<argc-1;i++)
	{
		parameter = string(argv[i]);
		value = string(argv[i+1]);

		if (parameter == string("-vis"))
		{
			configuration.define ("inputms", value);
			autoMode = False;
		}
		else if (parameter == string("-field"))
		{
			configuration.define ("field", value);
		}
		else if (parameter == string("-spw"))
		{
			configuration.define ("spw", value);
		}
		else if (parameter == string("-scan"))
		{
			configuration.define ("scan", value);
		}
		else if (parameter == string("-antenna"))
		{
			configuration.define ("antenna", value);
		}
		else if (parameter == string("-correlation"))
		{
			configuration.define ("correlation", value);
		}
		else if (parameter == string("-timerange"))
		{
			configuration.define ("timerange", value);
		}
		else if (parameter == string("-intent"))
		{
			configuration.define ("intent", value);
		}
		else if (parameter == string("-array"))
		{
			configuration.define ("array", value);
		}
		else if (parameter == string("-uvrange"))
		{
			configuration.define ("uvrange", value);
		}
		else if (parameter == string("-observation"))
		{
			configuration.define ("observation", value);
		}
		else if (parameter == string("-feed"))
		{
			configuration.define ("feed", value);
		}
		else if (parameter == string("-datacolumn"))
		{
			configuration.define ("datacolumn", value);
		}
		else if (parameter == string("-combinespws:"))
		{
			Bool tmp = Bool(atoi(value.c_str()));
			configuration.define ("combinespws", tmp);
		}
		else if (parameter == string("-chanaverage:"))
		{
			Bool tmp = Bool(atoi(value.c_str()));
			configuration.define ("chanaverage", tmp);
		}
		else if (parameter == string("-chanbin:"))
		{
			Int tmp = Int(atoi(value.c_str()));
			configuration.define ("chanbin", tmp);
		}
		else if (parameter == string("-useweights"))
		{
			configuration.define ("useweights", value);
		}
		else if (parameter == string("-hanning:"))
		{
			Bool tmp = Bool(atoi(value.c_str()));
			configuration.define ("hanning", tmp);
		}
		else if (parameter == string("-regridms:"))
		{
			Bool tmp = Bool(atoi(value.c_str()));
			configuration.define ("regridms", tmp);
		}
		else if (parameter == string("-mode"))
		{
			configuration.define ("mode", value);
		}
		else if (parameter == string("-nchan:"))
		{
			Int tmp = Int(atoi(value.c_str()));
			configuration.define ("nchan", tmp);
		}
		else if (parameter == string("-start:"))
		{
			Int tmp = Int(atoi(value.c_str()));
			configuration.define ("start", tmp);
		}
		else if (parameter == string("-width:"))
		{
			Int tmp = Int(atoi(value.c_str()));
			configuration.define ("width", tmp);
		}
		else if (parameter == string("-nspw:"))
		{
			Int tmp = Int(atoi(value.c_str()));
			configuration.define ("nspw", tmp);
		}
		else if (parameter == string("-interpolation"))
		{
			configuration.define ("interpolation", value);
		}
		else if (parameter == string("-phasecenter"))
		{
			configuration.define ("phasecenter", value);
		}
		else if (parameter == string("-restfreq"))
		{
			configuration.define ("restfreq", value);
		}
		else if (parameter == string("-outframe"))
		{
			configuration.define ("outframe", value);
		}
		else if (parameter == string("-veltype"))
		{
			configuration.define ("veltype", value);
		}
		else if (parameter == string("-timeaverage:"))
		{
			Bool tmp = Bool(atoi(value.c_str()));
			configuration.define ("timeaverage", tmp);
		}
		else if (parameter == string("-timebin"))
		{
			configuration.define ("timebin", value);
		}
		else if (parameter == string("-timespan"))
		{
			configuration.define ("timespan", value);
		}
		else if (parameter == string("-maxuvwdistance:"))
		{
			Double tmp = Double(atof(value.c_str()));
			configuration.define ("maxuvwdistance", tmp);
		}
		else if (parameter == string("-minbaselines:"))
		{
			Int tmp = Int(atoi(value.c_str()));
			configuration.define ("minbaselines", tmp);
		}
	}

	if (autoMode)
	{
		char* pathChar = getenv ("CASAPATH");
		if (pathChar != NULL)
		{
			// Get base path
			String pathStr(pathChar);
			string res[2];
			casa::split(pathChar,res,2,string(" "));

			// Generate filename
			string filename(res[0]);
			filename += string("/data/regression/unittest/mstransform/Four_ants_3C286.ms");

			// Check if file exists
			string command = string ("ls ") + filename + string(" &> /dev/null");
			Int exists = system(command.c_str());
			if (exists == 0)
			{
				configuration.define ("inputms", filename);
				configuration.define ("spw", "8,9,10,11");
				configuration.define ("antenna", "1&&2");
				configuration.define ("combinespws", True);
				configuration.define ("regridms", True);
				configuration.define ("mode", "channel");
				configuration.define ("width", "2");
				configuration.define ("timeaverage", True);
				configuration.define ("timebin", "30s");
			}
			else
			{
				cout << RED;
				cout << "AUTOMATIC MODE DOES NOT FIND TESTING FILE: " << filename << endl;
				cout << RESET;
				exit(1);
			}

		}
		else
		{
			cout << RED;
			cout << "AUTOMATIC MODE DOES NOT FIND ENV. VARIABLE CASAPATH" << endl;
			cout << RESET;
			exit(1);
		}
	}

	return configuration;
}

String produceTmpTransformedMSToCompare(Record configuration)
{
	String tmpFileName = File::newUniqueName("").absoluteName();
	configuration.define("outputms",tmpFileName);
	configuration.define("datacolumn","ALL");
	MSTransformManager *manager = new MSTransformManager(configuration);
	manager->open();
	manager->setup();

	vi::VisibilityIterator2 *visIter = manager->getVisIter();
	vi::VisBuffer2 *vb = visIter->getVisBuffer();
	visIter->originChunks();
	while (visIter->moreChunks())
	{
		visIter->origin();
		while (visIter->more())
		{
			manager->fillOutputMs(vb);
			visIter->next();
		}

		visIter->nextChunk();
	}

	manager->close();
	delete manager;
	return tmpFileName;
}

template <class T> Int compareVector(const Vector<T> &inp,const Vector<T> &ref)
{
	Int res = -1;

	for (uInt index=0;index < inp.size(); index++)
	{
		if (inp(index) != ref(index))
		{
			res = index;
			break;
		}
	}

	return res;
}

Bool test_compareTransformedFileWithTransformingBuffer(Record configuration, String tmpFileName)
{
	// Declare tmp variables
	Int chunk = 0,buffer = 0,row = 0;
	Bool keepIterating = True;

	// Open up transformed file
	MeasurementSet ms(tmpFileName,Table::Old);

	// Prepare Iterator
	Block<Int> sortCols(7);
	sortCols[0] = MS::OBSERVATION_ID;
	sortCols[1] = MS::ARRAY_ID;
	sortCols[2] = MS::SCAN_NUMBER;
	sortCols[3] = MS::STATE_ID;
	sortCols[4] = MS::FIELD_ID;
	sortCols[5] = MS::DATA_DESC_ID;
	sortCols[6] = MS::TIME;
	vi::VisibilityIterator2 visIterRef(ms,vi::SortColumns (sortCols,false));
	visIterRef.setRowBlocking(ms.nrow());
	vi::VisBuffer2 *visBufferRef = visIterRef.getVisBuffer();

	// Prepare transforming buffer
	MSTransformIteratorFactory factory(configuration);
	vi::VisibilityIterator2 *visIter = new vi::VisibilityIterator2 (factory);
	vi::VisBuffer2 *visBuffer = visIter->getVisBuffer();

	visIter->originChunks();
	visIterRef.originChunks();
	while (visIter->moreChunks() and visIterRef.moreChunks() and keepIterating)
	{
		chunk += 1;
		visIter->origin();
		visIterRef.origin();
		while (visIter->more() and visIterRef.more() and keepIterating)
		{
			buffer += 1;
			cout << BLUE;
			cout << " COMPARING CHUNK " << chunk << " BUFFER " << buffer << endl;


			// Re-indexable Vectors
			row = compareVector(visBuffer->dataDescriptionIds(),visBufferRef->dataDescriptionIds());
			if (row >= 0)
			{
				cout << RED;
				cout 	<< " dataDescriptionIds does not match in row " << row
						<< " transformBuffer=" << visBuffer->dataDescriptionIds()
						<< " transformFile=" << visBufferRef->dataDescriptionIds() << endl;
				keepIterating = False;
				break;
			}
			else
			{
				cout << GREEN;
				cout << "=>dataDescriptionIds match" << endl;
			}

			row = compareVector(visBuffer->observationId(),visBufferRef->observationId());
			if (row >= 0)
			{
				cout << RED;
				cout << " observationId does not match in row " << row
						<< " transformBuffer=" << visBuffer->observationId()(row)
						<< " transformFile=" << visBufferRef->observationId()(row)<< endl;
				keepIterating = False;
				break;
			}
			else
			{
				cout << GREEN;
				cout << "=>observationId match" << endl;
			}


			row = compareVector(visBuffer->arrayId(),visBufferRef->arrayId());
			if (row >= 0)
			{
				cout << RED;
				cout << " arrayId does not match in row " << row
						<< " transformBuffer=" << visBuffer->arrayId()(row)
						<< " transformFile=" << visBufferRef->arrayId()(row) << endl;
				keepIterating = False;
				break;
			}
			else
			{
				cout << GREEN;
				cout 	<< "=>arrayId match"<< endl;
			}


			row = compareVector(visBuffer->fieldId(),visBufferRef->fieldId());
			if (row >= 0)
			{
				cout << RED;
				cout << " fieldId does not match in row " << row
						<< " transformBuffer=" << visBuffer->fieldId()(row)
						<< " transformFile=" << visBufferRef->fieldId()(row) << endl;
				keepIterating = False;
				break;
			}
			else
			{
				cout << GREEN;
				cout 	<< "=>fieldId match"<< endl;
			}


			row = compareVector(visBuffer->stateId(),visBufferRef->stateId());
			if (row >= 0)
			{
				cout << RED;
				cout << " stateId does not match in row " << row
						<< " transformBuffer=" << visBuffer->stateId()(row)
						<< " transformFile=" << visBufferRef->stateId()(row) << endl;
				keepIterating = False;
				break;
			}
			else
			{
				cout << GREEN;
				cout 	<< "=>stateId match" << endl;
			}


			row = compareVector(visBuffer->antenna1(),visBufferRef->antenna1());
			if (row >= 0)
			{
				cout << RED;
				cout << " antenna1 does not match in row " << row
						<< " transformBuffer=" << visBuffer->antenna1()(row)
						<< " transformFile=" << visBufferRef->antenna1()(row) << endl;
				keepIterating = False;
				break;
			}
			else
			{
				cout << GREEN;
				cout 	<< "=>antenna1 match"<< endl;
			}


			row = compareVector(visBuffer->antenna2(),visBufferRef->antenna2());
			if (row >= 0)
			{
				cout << RED;
				cout << " antenna2 does not match in row " << row
						<< " transformBuffer=" << visBuffer->antenna2()(row)
						<< " transformFile=" << visBufferRef->antenna2()(row) << endl;
				keepIterating = False;
				break;
			}
			else
			{
				cout << GREEN;
				cout 	<< "=>antenna2 match" << endl;
			}

			// Not-Re-indexable Vectors

			row = compareVector(visBuffer->scan(),visBufferRef->scan());
			if (row >= 0)
			{
				cout << RED;
				cout << " scan does not match in row " << row
						<< " transformBuffer=" << visBuffer->scan()(row)
						<< " transformFile=" << visBufferRef->scan()(row)<< endl;
				keepIterating = False;
				break;
			}
			else
			{
				cout << GREEN;
				cout 	<< "=>scan match"<< endl;
			}


			row = compareVector(visBuffer->processorId(),visBufferRef->processorId());
			if (row >= 0)
			{
				cout << RED;
				cout << " processorId does not match in row " << row
						<< " transformBuffer=" << visBuffer->processorId()(row)
						<< " transformFile=" << visBufferRef->processorId()(row) << endl;
				keepIterating = False;
				break;
			}
			else
			{
				cout << GREEN;
				cout 	<< "=>processorId match" << endl;
			}


			row = compareVector(visBuffer->feed1(),visBufferRef->feed1());
			if (row >= 0)
			{
				cout << RED;
				cout << " feed1 does not match in row " << row
						<< " transformBuffer=" << visBuffer->feed1()(row)
						<< " transformFile=" << visBufferRef->feed1()(row) << endl;
				keepIterating = False;
				break;
			}
			else
			{
				cout << GREEN;
				cout 	<< "=>feed1 match" << endl;
			}


			row = compareVector(visBuffer->feed2(),visBufferRef->feed2());
			if (row >= 0)
			{
				cout << RED;
				cout << " feed2 does not match in row " << row
						<< " transformBuffer=" << visBuffer->feed2()(row)
						<< " transformFile=" << visBufferRef->feed2()(row) << endl;
				keepIterating = False;
				break;
			}
			else
			{
				cout << GREEN;
				cout 	<< "=>feed2 match" << endl;
			}


			row = compareVector(visBuffer->time(),visBufferRef->time());
			if (row >= 0)
			{
				cout << RED;
				cout << " time does not match in row " << row
						<< " transformBuffer=" << visBuffer->time()(row)
						<< " transformFile=" << visBufferRef->time()(row) << endl;
				keepIterating = False;
				break;
			}
			else
			{
				cout << GREEN;
				cout 	<< "=>time match" << endl;
			}


			row = compareVector(visBuffer->timeCentroid(),visBufferRef->timeCentroid());
			if (row >= 0)
			{
				cout << RED;
				cout << " timeCentroid does not match in row " << row
						<< " transformBuffer=" << visBuffer->timeCentroid()(row)
						<< " transformFile=" << visBufferRef->timeCentroid()(row) << endl;
				keepIterating = False;
				break;
			}
			else
			{
				cout << GREEN;
				cout 	<< "=>timeCentroid match" << endl;
			}


			row = compareVector(visBuffer->timeInterval(),visBufferRef->timeInterval());
			if (row >= 0)
			{
				cout << RED;
				cout << " timeInterval does not match in row " << row
						<< " transformBuffer=" << visBuffer->timeInterval()(row)
						<< " transformFile=" << visBufferRef->timeInterval()(row) << endl;
				keepIterating = False;
				break;
			}
			else
			{
				cout << GREEN;
				cout 	<< "=>timeInterval match" << endl;
			}


			// Average-able vectors
			row = compareVector(visBuffer->exposure(),visBufferRef->exposure());
			if (row >= 0)
			{
				cout << RED;
				cout << " exposure does not match in row " << row
						<< " transformBuffer=" << visBuffer->exposure()(row)
						<< " transformFile=" << visBufferRef->exposure()(row) << endl;
				keepIterating = False;
				break;
			}
			else
			{
				cout << GREEN;
				cout 	<< "=>exposure match" << endl;
			}


			row = compareVector(visBuffer->flagRow(),visBufferRef->flagRow());
			if (row >= 0)
			{
				cout << RED;
				cout << " flagRow does not match in row "<< row
						<< " transformBuffer=" << visBuffer->flagRow()(row)
						<< " transformFile=" << visBufferRef->flagRow()(row) << endl;
				keepIterating = False;
				break;
			}
			else
			{
				cout << GREEN;
				cout 	<< "=>flagRow match" << endl;
			}


			visIter->next();
			visIterRef.next();
		}

		visIter->nextChunk();
		visIterRef.nextChunk();
	}

	cout << RESET << endl;

	delete visIter;

	return keepIterating;
}

int main(int argc, char **argv)
{
	Record configuration = parseConfiguration(argc, argv);

	String tmpFileName = produceTmpTransformedMSToCompare(configuration);
	Bool result = test_compareTransformedFileWithTransformingBuffer(configuration,tmpFileName);

	if (result)
	{
		Table::deleteTable(tmpFileName,True);
		exit(0);
	}
	else
	{
		exit(1);
	}
}
