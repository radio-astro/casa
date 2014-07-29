//# MSTransformIterator.h: This file contains the interface definition of the MSTransformManager class.
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

#ifndef MSTransformIterator_H_
#define MSTransformIterator_H_

// Where TransformingVi2 interface is defined
#include <msvis/MSVis/TransformingVi2.h>

// Class containing the actual transformation logic
#include <mstransform/MSTransform/MSTransformManager.h>

// VisBuffer class only accessible from this class
#include <mstransform/MSTransform/MSTransformBufferImpl.h>


namespace casa {

class MSTransformIterator : public vi::TransformingVi2
{

public:

	MSTransformIterator(vi::VisibilityIterator2 * vi, vi::ViImplementation2 * inputVii,MSTransformManager *manager);
	~MSTransformIterator();

    const MeasurementSet & ms () const {return *transformedMS_p;};
	vi::VisBuffer2 * getVisBuffer () {return buffer_p;}

    void originChunks ();
    Bool moreChunks () const;
    void nextChunk ();

    void origin ();
    Bool more () const;
    void next ();

private:
	MSTransformManager *manager_p;
	MSTransformBufferImpl *buffer_p;
	MeasurementSet *transformedMS_p;
};

} //# NAMESPACE CASA - END


#endif /* MSTransformIterator_H_ */

