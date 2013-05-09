//# AxesDisplayData.h: axis labelling for registering on WorldCanvasHolders
//# Copyright (C) 1999,2000,2001,2002,2004
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

#ifndef TRIALDISPLAY_AXESDISPLAYDATA_H
#define TRIALDISPLAY_AXESDISPLAYDATA_H

#include <casa/aips.h>
#include <display/Display/DParameterRange.h>
#include <display/Display/DParameterString.h>
#include <display/Display/DParameterChoice.h>
#include <display/Display/DParameterColorChoice.h>
#include <display/DisplayDatas/PassiveCachingDD.h>

#include <iostream>
using namespace std;

namespace casa { //# NAMESPACE CASA - BEGIN

	class WorldCanvas;
	class WorldCanvasHolder;
	template <class T> class Vector;
	class String;
	class AttributeBuffer;

// <summary>
// Interface and simple implementation of axis labelling.
// </summary>
//
// <synopsis>
// This class adds to the interface defined by CachingDisplayData to
// provide the necessary infrastructure for drawing axis labels on
// WorldCanvases.  This class serves as a basic implementation which
// can (and probably should) be over-written in derived classes.
// </synopsis>

	class AxesDisplayData : public PassiveCachingDD {

	public:

		// Constructor.
		AxesDisplayData();

		// Destructor.
		virtual ~AxesDisplayData();

		// Install the default options for this DisplayData.
		virtual void setDefaultOptions();

		// Apply options stored in <src>rec</src> to the DisplayData.  A
		// return value of <src>True</src> means a refresh is needed.
		// <src>recOut</src> contains any fields which were implicitly
		// changed as a result of the call to this function.
		virtual Bool setOptions(Record &rec, Record &recOut);

		// Retrieve the current and default options and parameter types.
		virtual Record getOptions();

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

		// Get the title text for labelling.
		virtual String titleText() const {
			return itsParamTitleText->value();
		}

		// Get the X and Y axis text Strings for labelling.
		// a WC can be supplied in order to retrieve default titles
		// from the WC CS.
		// <group>
		virtual String xAxisText(const WorldCanvas* wc=0) const;
		virtual String yAxisText(const WorldCanvas* wc=0) const;
		// </group>

		// Get what type of grid should be marked in each direction.
		// <group>
		virtual String xGridType() const {
			return itsParamXGridType->value();
		}
		virtual String yGridType() const {
			return itsParamYGridType->value();
		}
		// </group>

		// Get the color to use for the title text, the X axis text, and the
		// Y axis text labels.
		// <group>
		virtual String titleTextColor() const {
			return itsParamTitleColor->value();
		}
		virtual String xAxisColor() const {
			return itsParamXAxisColor->value();
		}
		virtual String yAxisColor() const {
			return itsParamYAxisColor->value();
		}
		// </group>

		// Get the line width for labelling.
		virtual Float lineWidth() const {
			return itsParamLineWidth->value();
		}

		// Get the character font for labelling.
		virtual String charFont() const {
			return itsParamCharacterFont->value();
		}

		// Get the character size for labelling.
		virtual Float charSize() const {
			return itsParamCharacterSize->value();
		}

		// Get the color of the plot outline.
		virtual String outlineColor() const {
			return itsParamOutlineColor->value();
		}
		static const float AXIS_LABEL_DEFAULT_CHAR_SIZE;
	protected:

		// (Required) copy constructor.
		AxesDisplayData(const AxesDisplayData &other);

		// (Required) copy assignment.
		void operator=(const AxesDisplayData &other);

	private:

		// display title text
		DParameterString *itsParamTitleText;

		// Store for X axis label text String.
		DParameterString *itsParamXAxisText;

		// Store for Y axis label text String.
		DParameterString *itsParamYAxisText;

		// coordinate grid?
		DParameterChoice *itsParamXGridType, *itsParamYGridType;

		// title color
		DParameterColorChoice *itsParamTitleColor;
		//String itsOptionsTitleTextColor;

		// label text color
		DParameterColorChoice *itsParamXAxisColor, *itsParamYAxisColor;
		//String itsOptionsXAxisColor, itsOptionsYAxisColor;

		// plot line width
		DParameterRange<Float> *itsParamLineWidth;

		// plot character font
		DParameterChoice *itsParamCharacterFont;

		// plot character size
		DParameterRange<Float> *itsParamCharacterSize;

		// plot outline color
		DParameterColorChoice *itsParamOutlineColor;


	};


} //# NAMESPACE CASA - END

#endif

