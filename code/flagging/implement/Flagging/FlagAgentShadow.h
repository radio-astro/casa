//# FlagAgentShadow.h: This file contains the interface definition of the FlagAgentShadow class.
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

#ifndef FlagAgentShadow_H_
#define FlagAgentShadow_H_

#include <flagging/Flagging/FlagAgentBase.h>

namespace casa { //# NAMESPACE CASA - BEGIN

class FlagAgentShadow : public FlagAgentBase {

public:

	FlagAgentShadow(FlagDataHandler *dh, Record config, Bool writePrivateFlagCube = false);
	~FlagAgentShadow();

protected:

	// Compute flags afor a given mapped visibility point
	void computeRowFlags(VisBuffer &visBuffer, FlagMapper &flags, uInt row);

	// Parse configuration parameters
	void setAgentParameters(Record config);

private:

	/// Input parameters ///
	Double antennaDiameter_p;
};


} //# NAMESPACE CASA - END

#endif /* FlagAgentShadow_H_ */

