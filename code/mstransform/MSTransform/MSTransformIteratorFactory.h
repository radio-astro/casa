//# MSTransformIteratorFactory.h: This file contains the interface definition of the MSTransformManager class.
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

#ifndef MSTransformIteratorFactory_H_
#define MSTransformIteratorFactory_H_

// Where ViFactory interface is defined
#include <synthesis/MSVis/VisibilityIterator2.h>

// Class containing the actual transformation logic
#include <mstransform/MSTransform/MSTransformManager.h>

// Implementation returned by the factory method
#include <mstransform/MSTransform/MSTransformIterator.h>

namespace casa {

// <summary>
// A top level class defining the data handling interface for the flagging module
// </summary>
//
// <use visibility=export>
//
// <prerequisite>
//   <li> <linkto class="VisibilityIterator2:description">VisibilityIterator2</linkto>
//   <li> <linkto class="MSTransformIterator:description">MSTransformIterator</linkto>
//   <li> <linkto class="MSTransformManager:description">MSTransformManager</linkto>
// </prerequisite>
//
// <etymology>
// MSTransformFactory is a class that generates a Visibility Iterator implementation based on mstransform
// </etymology>
//
// <synopsis>
// MSTransformFactory generates a Visibility Iterator implementation (ViImplementation2)
// object that can be plugged into a Visibility Iterator (VisibilityIterator2) object,
// so that the user can access the data using the Visibility Iterator interface.
// </synopsis>
//
// <motivation>
// The idea is that an external application (plotms, imaging) can consume data
// transformed using the mstransform code via a Visibility Iterator interface
// </motivation>
//
// <example>
// External usage is quite simple, and compliant with the 'normal' VI/VB framework.
//
// The user defines a dictionary with the parameters that would be normally passed to test_mstransform
// (therefore I point to task_mstransform help for doubts regarding the parameters)
//
// <srcblock>
// 	Record configuration;
// 	configuration.define ("inputms", filename);
// 	configuration.define ("spw", "8,9,10,11");
// 	configuration.define ("antenna", "1&&2");
// 	configuration.define ("combinespws", True);
// 	configuration.define ("regridms", True);
// 	configuration.define ("mode", "channel");
// 	configuration.define ("width", "2");
// 	configuration.define ("timeaverage", True);
// 	configuration.define ("timebin", "30s");
// </srcblock>
//
// Notice that some parameters don't make sense in the context of a VI/VB interface:
// * outputms
// * tileshape
// * datacolumn
// * realmodelcol
// * usewtspectrum
//
// With this configuration record the factory class  MSTransformIteratorFactory will
// create internally a  MSTransformIterator and return a pointer to a VisibilityIterator2
// object whose implementation  is the newly created  MSTransformIterator.
//
// <srcblock>
// 	MSTransformIteratorFactory factory(configuration);
// 	vi::VisibilityIterator2 *visIter = new vi::VisibilityIterator2 (factory);
// 	vi::VisBuffer2 *visBuffer = visIter->getVisBuffer();
// </srcblock>
//
// Once this is done one can normally iterate and access the transformed data:
//
// <srcblock>
// 	while (visIter->moreChunks())
// 	{
// 		visIter->origin();
//
//		while (visIter->more())
//		{
//
//			Vector<Int> ddi = visBuffer->dataDescriptionIds();
//			Vector<Int> antenna1 = visBuffer->antenna1();
//			Vector<Int> antenna2 = visBuffer->antenna2();
//			// Etc
//
//			visIter->next();
//		}
//
//		visIter->nextChunk();
//	}
// </srcblock>
//
// It is also possible to access the transformed Sub-Tables
// (loaded in memory thanks to the Memory Resident Sub-Tables mechanism):
//
// <srcblock>
// 	MSSpectralWindow transformedSpwTable = visIter->ms().spectralWindow();
// </srcblock>
//
// Notice that it is the responsibility of the application layer to delete the VisibilityIterator2
// pointer returned by the factory method. However the life cycle of the VisBuffer2 object is
// responsibility of the VisibilityIterator2 object.
//
// <srcblock>
// 	delete visIter;
// </srcblock>
//
// </example>

class MSTransformIteratorFactory : public vi::ViFactory
{

public:

	MSTransformIteratorFactory(Record &configuration);
	MSTransformIteratorFactory(Record &configuration, MrsEligibility &eligibleSubTables);
	~MSTransformIteratorFactory();

protected:

	void setConfiguration(Record &configuration);
	vi::ViImplementation2 * createVi (vi::VisibilityIterator2 * inpVi) const;

private:

	Record configuration_p;
	MrsEligibility eligibleSubTables_p;
};

} //# NAMESPACE CASA - END


#endif /* MSTransformIteratorFactory_H_ */

