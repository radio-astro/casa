//# FlagAgentClipping.h: This file contains the interface definition of the FlagAgentClipping class.
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

#ifndef FlagAgentClipping_H_
#define FlagAgentClipping_H_

#include <flagging/Flagging/FlagAgentBase.h>

namespace casa { //# NAMESPACE CASA - BEGIN

class FlagAgentClipping : public FlagAgentBase {

public:

	FlagAgentClipping(FlagDataHandler *dh, Record config, Bool writePrivateFlagCube = false);
	~FlagAgentClipping();

protected:

	// Compute flags afor a given mapped visibility point
	void computeInRowFlags(VisMapper &visibilities,FlagMapper &flags,uInt channel, uInt row);

	// Parse configuration parameters
	void setAgentParameters(Record config);

private:

	/// Input parameters ///
	Float clipmin_p;
	Float clipmax_p;

};


} //# NAMESPACE CASA - END

#endif /* FlagAgentClipping_H_ */

