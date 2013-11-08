//# MSTransformIteratorFactory.h: This file contains the implementation of the MSTransformIteratorFactory class.
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

namespace casa
{

// -----------------------------------------------------------------------
//
// -----------------------------------------------------------------------
MSTransformIteratorFactory::MSTransformIteratorFactory(Record &configuration)
{
	setConfiguration(configuration);
	eligibleSubTables_p = MrsEligibility::defaultEligible();
	return;
}

// -----------------------------------------------------------------------
//
// -----------------------------------------------------------------------
MSTransformIteratorFactory::MSTransformIteratorFactory(Record &configuration, MrsEligibility &eligibleSubTables)
{
	setConfiguration(configuration);
	eligibleSubTables_p = eligibleSubTables;
	return;
}

// -----------------------------------------------------------------------
//
// -----------------------------------------------------------------------
void MSTransformIteratorFactory::setConfiguration(Record &configuration)
{
	configuration_p = configuration;
	configuration_p.define("buffermode",True);
	configuration_p.define("outputms",File::newUniqueName("").absoluteName());
	return;
}

// -----------------------------------------------------------------------
//
// -----------------------------------------------------------------------
MSTransformIteratorFactory::~MSTransformIteratorFactory()
{
	return;
}

// -----------------------------------------------------------------------
// InpVi is the VisibilityIterator in which the implementation provided is to be plugged
// -----------------------------------------------------------------------
vi::ViImplementation2 * MSTransformIteratorFactory::createVi(vi::VisibilityIterator2 * inpVi) const
{
	// Create MSTransformManager
	MSTransformManager *manager =  new MSTransformManager(configuration_p);
	manager->open();
	manager->setup();

	// Enable memory resident SubTables in transformed MS
	manager->getOutputMs()->setMemoryResidentSubtables(eligibleSubTables_p);

	// Create output VisibilityIterator
	// NOTE: InpVi is not really necessary downstream. We only have to provide an implementation
	MSTransformIterator *outputVI = new MSTransformIterator(NULL,manager->getVisIter()->getImpl(),manager);

	return outputVI;
}

} //# NAMESPACE CASA - END


