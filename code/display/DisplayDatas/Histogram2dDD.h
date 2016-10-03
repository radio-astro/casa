//# Histogram2dDD.h: 2d histogram DisplayData
//# Copyright (C) 2000,2001,2002
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
//# $Id$

#ifndef TRIALDISPLAY_HISTOGRAM2DDD_H
#define TRIALDISPLAY_HISTOGRAM2DDD_H

#include <casa/aips.h>
#include <lattices/Lattices/Lattice.h>
#include <lattices/Lattices/MaskedLattice.h>
#include <display/DisplayDatas/ActiveCaching2dDD.h>

namespace casacore{

	template <class T> class ImageInterface;
	template <class T> class LatticeStatistics;
	template <class T> class LatticeHistograms;
}

namespace casa { //# NAMESPACE CASA - BEGIN

	class Histogram2dDM;

// <summary>
// A DisplayData to draw histograms
// </summary>
//
// <use visibility=export>
//
// <reviewed reviewer="" date="yyyy/mm/dd" tests="" demos="">
// </reviewed>
//
// <prerequisite>
//   <li> DisplayData
//   <li> CachingDisplayData
//   <li> ActiveCaching2dDD
// </prerequisite>
//
// <etymology>
// </etymology>
// <synopsis>
//
// </synopsis>

	class Histogram2dDD : public ActiveCaching2dDD {

	public:

		// Constructor taking a pointer to an already constructed
		// ImageInterface.
		Histogram2dDD(const casacore::ImageInterface<casacore::Float> *image);

		// Constructor taking a pointer to an already constructed casacore::Table,
		// and a casacore::String indicating a column name.
		//Histogram2dDD(const casacore::Table *table, const casacore::String column);

		// Constructor taking a pointer to an Array.
		//Histogram2dDD(const casacore::Array<casacore::Float> *array);

		// Destructor.
		virtual ~Histogram2dDD();

		// Return the data unit.
		virtual const casacore::Unit dataUnit();

		// casacore::Format the histogram value at the given world position.
		virtual casacore::String showValue(const casacore::Vector<casacore::Double> &world);

		// Install the default options for this DisplayData.
		virtual void setDefaultOptions();

		// Apply options stored in <src>rec</src> to the DisplayData.  A
		// return value of <src>true</src> means a refresh is needed.
		// <src>recOut</src> contains any fields which were implicitly
		// changed as a result of the call to this function.
		virtual casacore::Bool setOptions(casacore::Record &rec, casacore::Record &recOut);

		// Retrieve the current and default options and parameter types.
		virtual casacore::Record getOptions( bool scrub=false ) const;

		// Return the type of this DisplayData.
		virtual Display::DisplayDataType classType() {
			return Display::CanvasAnnotation;
		}

		// Create a new AxesDisplayMethod for drawing on the given
		// WorldCanvas when the AttributeBuffers are suitably matched to the
		// current state of this DisplayData and of the WorldCanvas/Holder.
		// The tag is a unique number used to identify the age of the newly
		// constructed CachingDisplayMethod.
		virtual CachingDisplayMethod *newDisplayMethod(WorldCanvas *worldCanvas,
		        AttributeBuffer *wchAttributes,
		        AttributeBuffer *ddAttributes,
		        CachingDisplayData *dd);

		// Return the current options of this DisplayData as an
		// AttributeBuffer.
		virtual AttributeBuffer optionsAsAttributes();

		// Provide read-only access to the Lattice.
		const casacore::Lattice<casacore::Float> &lattice() const {
			return *itsMaskedLattice;
		}

		// Provide read-only access to the MaskedLattce.
		const casacore::MaskedLattice<casacore::Float> &maskedLattice() const {
			return *itsMaskedLattice;
		}

	protected:

		// (Required) default constructor.
		Histogram2dDD();

		// (Required) copy constructor.
		Histogram2dDD(const Histogram2dDD &other);

		// (Required) copy assignment.
		void operator=(const Histogram2dDD &other);

	private:

		friend class Histogram2dDM;

		// The base image cloned at construction.
		casacore::ImageInterface<casacore::Float> *itsBaseImage;

		// The masked lattice, effectively referencing one of itsBaseImage,
		// ...
		casacore::MaskedLattice<casacore::Float> *itsMaskedLattice;

		// Object to use for calculating statistics.
		casacore::LatticeStatistics<casacore::Float> *itsLatticeStatistics;

		// Object ot use for calculating histograms.
		casacore::LatticeHistograms<casacore::Float> *itsLatticeHistograms;

	};


} //# NAMESPACE CASA - END

#endif
