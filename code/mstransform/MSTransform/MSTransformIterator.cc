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
	manager_p->setupBufferTransformations(manager_p->getVisBuffer());
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

// -----------------------------------------------------------------------
//
// -----------------------------------------------------------------------
void MSTransformIterator::writeFlag (const Cube<Bool> & flag)
{
	// Get transformed shape
	IPosition transformedShape = manager_p->getShape();

	// Check that user is providing a cube with the right output shape
	if (flag.shape() != transformedShape)
	{
		manager_p->logger_p << LogIO::WARN << LogOrigin("MSTransformIterator", __FUNCTION__)
				<< "Shape of input cube " << flag.shape()
				<< " does not match transformed shape " << transformedShape
				<< LogIO::POST;

		return;
	}

	// Expand channel dimension
	if (manager_p->channelAverage_p)
	{
		// Get original shape
		IPosition inputShape = manager_p->getVisBuffer()->getShape();
		size_t nCorr = inputShape(0);
		size_t nChan = inputShape(1);
		size_t nRows = inputShape(2);

		// Create an un-flagged flag cube matching the input shape
		Cube<Bool> propagatedInputFlag(inputShape,False);

		// Map input-output channel
		uInt binCounts = 0;
		uInt transformedIndex = 0;
		Vector<uInt> inputOutputChan(nChan);
		uInt spw = manager_p->getVisBuffer()->spectralWindows()[0];
		uInt chanbin = manager_p->freqbinMap_p[spw];
		for (size_t chan_i =0;chan_i<nChan;chan_i++)
		{
			binCounts += 1;

			if (binCounts > chanbin)
			{
				binCounts = 1;
				transformedIndex += 1;
			}

			inputOutputChan(chan_i) = transformedIndex;
		}

		// Propagate chan-avg flags
		for (size_t row_i =0;row_i<nRows;row_i++)
		{
			for (size_t chan_i =0;chan_i<nChan;chan_i++)
			{
				for (size_t corr_i =0;corr_i<nCorr;corr_i++)
				{
					propagatedInputFlag(corr_i,chan_i,row_i) = flag(corr_i,inputOutputChan(chan_i),row_i);
				}
			}
		}

		manager_p->getVisIter()->writeFlag(propagatedInputFlag);

	}
	else
	{
		manager_p->getVisIter()->writeFlag(flag);
	}

	manager_p->selectedInputMs_p->flush();

	return;
}

// -----------------------------------------------------------------------
//
// -----------------------------------------------------------------------
void MSTransformIterator::writeFlagRow (const Vector<Bool> & rowflags)
{
	manager_p->getVisIter()->writeFlag(rowflags);

	return;
}


} //# NAMESPACE CASA - END


