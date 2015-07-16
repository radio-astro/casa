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

	FlagAgentClipping(FlagDataHandler *dh, Record config, Bool writePrivateFlagCube = false, Bool flag = true);
	~FlagAgentClipping();

protected:

	// Common functionality for each visBuffer (don't repeat at the row level)
	// jagonzal: With the new WEIGHT/SIGMA convention it is not longer
	// necessary divide the clipping range between the number of channels
	// void preProcessBuffer(const vi::VisBuffer2 &visBuffer);

	// Compute flags afor a given mapped visibility point
	bool computeInRowFlags(const vi::VisBuffer2 &visBuffer, VisMapper &visibilities,FlagMapper &flags, uInt row);

	// Specialization of the different clipping cases
	bool checkVisForClipOutside(Float visExpression);
	bool checkVisForClipInside(Float visExpression);
	bool checkVisForClipOutsideAndZeros(Float visExpression);
	bool checkVisForClipInsideAndZeros(Float visExpression);
	bool checkVisForNaNs(Float visExpression);
	bool checkVisForNaNsAndZeros(Float visExpression);

	// Parse configuration parameters
	void setAgentParameters(Record config);

private:

	/// Input parameters ///
	Bool clipminmax_p;
	Bool clipoutside_p;
	Bool channelavg_p;
	Bool clipzeros_p;
	Bool weightcol_p;
	Float clipmin_p;
	Float clipmax_p;
	Float original_clipmin_p;
	Float original_clipmax_p;
	Bool timeavg_p;
	Double timebin_p;

	// Specialization for the clipping case
	bool (casa::FlagAgentClipping::*checkVis_p)(Float);

};


} //# NAMESPACE CASA - END

#endif /* FlagAgentClipping_H_ */

