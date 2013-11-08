//# MSTransformIterator.h: This file contains the implementation of the MSTransformIterator class.
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

#include <mstransform/MSTransform/MSTransformIterator.h>

namespace casa
{

// -----------------------------------------------------------------------
//
// -----------------------------------------------------------------------
MSTransformIterator::MSTransformIterator(	vi::VisibilityIterator2 * vi,
											vi::ViImplementation2 * inputVii,
											MSTransformManager *manager):		TransformingVi2 (vi, inputVii)
{

	// TODO (jagonzal): Do we need a copy of the manager here?
	manager_p = manager;

	// Get reference to output (transformed) MS in order to access the SubTables
	transformedMS_p = manager_p->getOutputMs();

	// Create VisBuffer implementation only accessible from this iterator
	buffer_p = new MSTransformBufferImpl(manager_p);

	return;
}

// -----------------------------------------------------------------------
//
// -----------------------------------------------------------------------
MSTransformIterator::~MSTransformIterator()
{
	if (buffer_p != NULL)
	{
		delete buffer_p;
	}

	if (manager_p != NULL)
	{
		String outputMSName = manager_p->getOutputMsName();
		manager_p->close();

		// ~MSTransformManager => ~VisibilityIterator2 => ~VisibilityIteratorImpl2
		delete manager_p;

		Table::relinquishAutoLocks(True);
		Table::deleteTable(outputMSName,True);
	}

	// Now the parent class destructor (~TransformingVi2) is called, which deletes the inner
	// ViImplementation2 object. However it has been already deleted by the MSTRansformManager
	// destructor, so we set it to null here to avoid problems.
	inputVii_p = NULL;

	return;
}

// -----------------------------------------------------------------------
//
// -----------------------------------------------------------------------
void MSTransformIterator::originChunks ()
{
	buffer_p->resetState();
	buffer_p->setRowIdOffset(0);
	getVii()->originChunks();
	return;
}

// -----------------------------------------------------------------------
//
// -----------------------------------------------------------------------
Bool MSTransformIterator::moreChunks () const
{
	buffer_p->resetState();
	Bool res = getVii()->moreChunks();
	return res;
}

// -----------------------------------------------------------------------
//
// -----------------------------------------------------------------------
void MSTransformIterator::nextChunk ()
{
	buffer_p->resetState();
	getVii()->nextChunk();
	return;
}

// -----------------------------------------------------------------------
//
// -----------------------------------------------------------------------
void MSTransformIterator::origin ()
{
	buffer_p->resetState();
	getVii()->origin();
	return;
}

// -----------------------------------------------------------------------
//
// -----------------------------------------------------------------------
Bool MSTransformIterator::more () const
{
	buffer_p->resetState();
	Bool res = getVii()->more();
	if (res)
	{
		manager_p->setupBufferTransformations(manager_p->getVisBuffer());
	}
	return res;
}

// -----------------------------------------------------------------------
//
// -----------------------------------------------------------------------
void MSTransformIterator::next ()
{
	buffer_p->resetState();
	buffer_p->shiftRowIdOffset(manager_p->nRowsToAdd_p);
	getVii()->next();
	return;
}


} //# NAMESPACE CASA - END


