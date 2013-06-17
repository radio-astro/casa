//# WorldAxesDD.h: world axis labelling DisplayData
//# Copyright (C) 2000,2001,2003,2004
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

#ifndef TRIALDISPLAY_WORLDAXESDD_H
#define TRIALDISPLAY_WORLDAXESDD_H

#include <casa/aips.h>
#include <display/DisplayDatas/AxesDisplayData.h>

namespace casa { //# NAMESPACE CASA - BEGIN

	class DParameterChoice;

// <summary>
// A DisplayData to setup a World coordinate plot
// </summary>
//
// <use visibility=export>
//
// <reviewed reviewer="" date="yyyy/mm/dd" tests="" demos="">
// </reviewed>
//
// <prerequisite>
//   <li> DisplayData
//   <li> AxesDisplayData
// </prerequisite>
//
// <etymology>
// </etymology>
// <synopsis>
//
// </synopsis>

	class WorldAxesDD : public AxesDisplayData {

	public:

		// Constructor.
		WorldAxesDD();

		// Destructor.
		virtual ~WorldAxesDD();

		// Install the default options for this DisplayData.
		virtual void setDefaultOptions();

		// Apply options stored in <src>rec</src> to the DisplayData.  A
		// return value of <src>True</src> means a refresh is needed.
		// <src>recOut</src> contains any fields which were implicitly
		// changed as a result of the call to this function.
		virtual Bool setOptions(Record& rec, Record& recOut);

		// Retrieve the current and default options and parameter types.
		virtual Record getOptions();

		// Create a new AxesDisplayMethod for drawing on the given
		// WorldCanvas when the AttributeBuffers are suitably matched to the
		// current state of this DisplayData and of the WorldCanvas/Holder.
		// The tag is a unique number used to identify the age of the newly
		// constructed CachingDisplayMethod.
		virtual CachingDisplayMethod* newDisplayMethod(WorldCanvas* worldCanvas,
		        AttributeBuffer* wchAttributes,
		        AttributeBuffer* ddAttributes,
		        CachingDisplayData* dd);

		// Return the current options of this DisplayData as an
		// AttributeBuffer.
		virtual AttributeBuffer optionsAsAttributes();

		virtual String spectralUnit() const {
			return itsParamSpectralUnit->value();
		}

		virtual String velocityType() const {
			return itsParamVelocityType->value();
		}

		virtual String axisText(const WorldCanvas* wc, const uInt axisNo) const;
		virtual String xAxisText(const WorldCanvas* wc=0) const;
		virtual String yAxisText(const WorldCanvas* wc=0) const;

		// Pure virtual function from DisplayData...
		String dataType() const {
			return "label";
		}

		std::string errorMessage( ) const { return ""; }

	protected:

		// (Required) copy constructor.
		WorldAxesDD(const WorldAxesDD& other);

		// (Required) copy assignment.
		void operator=(const WorldAxesDD& other);

	private:

		// Install the default options for this DisplayData.
		void installDefaultOptions();

		// Set Spectral preference (options)
		DParameterChoice* itsParamSpectralUnit;
		DParameterChoice* itsParamVelocityType;

	};


} //# NAMESPACE CASA - END

#endif
