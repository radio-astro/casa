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

