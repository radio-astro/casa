/* -*- mode: c++ -*- */
//# ParallelImagerParams.h: Parameters for parallel imaging
//# Copyright (C) 2016
//# Associated Universities, Inc. Washington DC, USA.
//#
//# This library is free software; you can redistribute it and/or modify it
//# under the terms of the GNU Library General Public License as published by
//# the Free Software Foundation; either version 2 of the License, or (at your
//# option) any later version.
//#
//# This library is distributed in the hope that it will be useful, but WITHOUT
//# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
//# FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
//# License for more details.
//#
//# You should have received a copy of the GNU Library General Public License
//# along with this library; if not, write to the Free Software Foundation,
//# Inc., 675 Massachusetts Ave, Cambridge, MA 02139, USA.
//#
//# Correspondence concerning AIPS++ should be addressed as follows:
//#        Internet email: aips2-request@nrao.edu.
//#        Postal address: AIPS++ Project Office
//#                        National Radio Astronomy Observatory
//#                        520 Edgemont Road
//#                        Charlottesville, VA 22903-2475 USA
//#
#ifndef PARALLEL_IMAGER_PARAMS_H_
#define PARALLEL_IMAGER_PARAMS_H_

#include <casa/Containers/Record.h>

namespace casa {

/**
 * Collection of parameters for parallel imaging, categorized roughly by imaging
 * component.
 */
struct ParallelImagerParams {
	ParallelImagerParams(Record sl, Record im, Record gr, Record wt, Record nm,
	                     Record dc, Record it)
		: selection(sl)
		, image(im)
		, grid(gr)
		, weight(wt)
		, normalization(nm)
		, deconvolution(dc)
		, iteration(it) {};

	ParallelImagerParams()
		: selection(Record())
		, image(Record())
		, grid(Record())
		, weight(Record())
		, normalization(Record())
		, deconvolution(Record())
		, iteration(Record()) {};

	Record selection;
	Record image;
	Record grid;
	Record weight;
	Record normalization;
	Record deconvolution;
	Record iteration;
};

} // namespace casa

#endif // PARALLEL_IMAGER_PARAMS_H_
