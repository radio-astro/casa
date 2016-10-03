//# WCAxisLabeller.h: base class for labelling axes on the WorldCanvas
//# Copyright (C) 1999,2000,2001,2002
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

#ifndef TRIALDISPLAY_WCAXISLABELLER_H
#define TRIALDISPLAY_WCAXISLABELLER_H

//# aips includes:
#include <casa/aips.h>
#include <casa/Containers/Record.h>

//# trial includes:

//# display library includes:
#include <display/Utilities/DisplayOptions.h>
#include <display/DisplayEvents/WCRefreshEvent.h>

namespace casa { //# NAMESPACE CASA - BEGIN

//# forwards:

// <summary>
// Abstract class for drawing grids, axes, ticks and tick labels
// on a WorldCanvas
// </summary>
//
// <use visibility=local>
//
// <reviewed reviewer="" date="yyyy/mm/dd" tests="" demos="">
// </reviewed>
//
// <prerequisite>
//   <li> WorldCanvas
// </prerequisite>
//
// <etymology>
// WCAxisLabeller stands for WorldCanvas Axis Labeller
// </etymology>
//
// <synopsis>
// This class forms the base for classes which draw axis grids,
// axes, ticks and tick labels on WorldCanvases.  In its
// present form, it is really a heavily pruned form of the
// DisplayData.
// </synopsis>
//
// <example>
// </example>
//
// <motivation>
// Axis labels are a necessary annotation for meaningful displays
// of data.  This class is motivated by the desire to provide
// (eventually) many different types of axis labelling.
// </motivation>
//
// <todo asof="1999/02/16">
//   <li> add to interface: getOptions, setOptions
//   <li> perhaps convert to a DisplayData?
// </todo>

	class WCAxisLabeller : public DisplayOptions {

	public:

		// Constructor
		WCAxisLabeller();

		// Destructor
		virtual ~WCAxisLabeller();

		// Draw axis labels for the provided WorldCanvas refresh event
		virtual casacore::Bool draw(const WCRefreshEvent &ev) = 0;

		// Invalidate any cached drawings
		virtual void invalidate() { };

		// install the default options for this DisplayData
		virtual void setDefaultOptions();

		// apply options stored in rec to the DisplayData; return value
		// true means a refresh is needed.  Any fields added to the
		// updatedOptions argument are options which have changed in
		// some way due to the setting of other options - ie. they
		// are context sensitive.
		virtual casacore::Bool setOptions(const casacore::Record &rec, casacore::Record &updatedOptions);

		// retrieve the current and default options and parameter types.
		virtual casacore::Record getOptions() const;

		// set/return whether labelling is on or off.  This is a global
		// switch.  If true is returned from the set function, then a
		// refresh is required.
		// <group>
		virtual casacore::Bool setAxisLabelSwitch(const casacore::Bool labelswitch = false);
		virtual casacore::Bool axisLabelSwitch() const {
			return itsOptionsAxisLabelSwitch;
		}
		// </group>

		// set/return the title text.  If true is returned from the set
		// function then a refresh is required.
		// <group>
		//virtual casacore::Bool setTitleText(const casacore::String text = casacore::String(""));
		virtual void setSubstituteTitleText( const casacore::String substituteImageName);
		virtual casacore::String titleText() const {
			return itsOptionsTitleText;
		}
		virtual casacore::String displayedTitleText() const {
			casacore::String actualText = substituteTitleText;
			if ( actualText.length() == 0 ){
				actualText = itsOptionsTitleText;
			}
			return actualText;
		}
		// </group>

		// set/return the color of the title text.  If true is returned from
		// the set function, then a refresh should be called.
		// <group>
		virtual casacore::Bool setTitleTextColor(const casacore::String color = casacore::String("foreground"));
		virtual casacore::String titleTextColor() const {
			return itsOptionsTitleTextColor;
		}
		// </group>

		// set/return the X and Y label text.  If true is returned from
		// the set functions, then a refresh is needed.
		// <group>
		virtual casacore::Bool setXAxisText(const casacore::String text = casacore::String(""));
		virtual casacore::Bool setYAxisText(const casacore::String text = casacore::String(""));
		virtual casacore::Bool unsetXAxisText();
		virtual casacore::Bool unsetYAxisText();
		virtual casacore::String xAxisText() const;
		virtual casacore::String yAxisText() const;
		virtual casacore::Bool isXAxisTextUnset() const {
			return itsOptionsXAxisTextUnset;
		}
		virtual casacore::Bool isYAxisTextUnset() const {
			return itsOptionsYAxisTextUnset;
		}
		// </group>

		// set/return the color of the label text.  If true is returned
		// from the set function, then a refresh is needed.
		// <group>
		virtual casacore::Bool setXAxisTextColor(const casacore::String color = casacore::String("foreground"));
		virtual casacore::Bool setYAxisTextColor(const casacore::String color = casacore::String("foreground"));
		virtual casacore::String xAxisTextColor() const {
			return itsOptionsXAxisTextColor;
		}
		virtual casacore::String yAxisTextColor() const {
			return itsOptionsYAxisTextColor;
		}
		// </group>

		// set/return what type of grid is marked in each direction.
		// If true is returned from the set functions, a refresh is needed.
		// <group>
		virtual casacore::Bool setXGridType(const casacore::String type = casacore::String("None"));
		virtual casacore::Bool setYGridType(const casacore::String type = casacore::String("None"));
		virtual casacore::String xGridType() const {
			return itsOptionsXGridType;
		}
		virtual casacore::String yGridType() const {
			return itsOptionsYGridType;
		}
		// </group>

		// set/return the color of the grid lines per direction.  If true
		// is returned from the set functions, a refresh is needed.
		// <group>
		virtual casacore::Bool setXGridColor(const casacore::String color = casacore::String("foreground"));
		virtual casacore::Bool setYGridColor(const casacore::String color = casacore::String("foreground"));
		virtual casacore::String xGridColor() const {
			return itsOptionsXGridColor;
		}
		virtual casacore::String yGridColor() const {
			return itsOptionsYGridColor;
		}
		// </group>

		// set/return the tick length in millimetres.  If true is returned
		// from the set function, then a refresh is probably needed.
		// <group>
		virtual casacore::Bool setTickLength(const casacore::Float length = 4.0);
		virtual casacore::Float tickLength() const {
			return itsOptionsTickLength;
		}
		// </group>

		// set/return the position of World label strings
		// <group>
		virtual casacore::Bool setLabelPosition(const casacore::String position = casacore::String("Auto"));
		virtual casacore::String labelPosition() const {
			return itsOptionsLabelPos;
		}
		// </group>


		// set/return whether there is a plot outline or not.  If true is
		// returned from the set function, then a refresh is probably needed.
		// <group>
		virtual casacore::Bool setPlotOutline(const casacore::Bool outline = true);
		virtual casacore::Bool plotOutline() const {
			return itsOptionsPlotOutline;
		}
		// </group>

		// set/return the color used for the plot outline.  If true is
		// returned from the set function, then a refresh should be
		// enacted.
		// <group>
		virtual casacore::Bool setPlotOutlineColor(const casacore::String color = casacore::String("foreground"));
		virtual casacore::String plotOutlineColor() const {
			return itsOptionsPlotOutlineColor;
		}
		// </group>

		// set/return the char size for the plot graphics
		// <group>
		virtual casacore::Bool setCharSize(const casacore::Float size = 1.2);
		virtual casacore::Float charSize() const {
			return itsOptionsCharSize;
		}
		// </group>

		// set/return the char font for the plot graphics
		// <group>
		virtual casacore::Bool setCharFont(const casacore::String font = "normal");
		virtual casacore::String charFont() const {
			return itsOptionsCharFont;
		}
		// </group>

		// set/return the line width for the plot graphics.
		// <group>
		/*
		virtual casacore::Bool setLineWidth(const casacore::Int width = 0);
		virtual casacore::Int lineWidth() const
		  { return itsOptionsLineWidth; }
		*/
		virtual casacore::Bool setLineWidth(const casacore::Float width = 0.0);
		virtual casacore::Float lineWidth() const {
			return itsOptionsLineWidth;
		}
		// </group>

		static const casacore::String LABEL_CHAR_SIZE;
		static const casacore::String PLOT_TITLE;

	protected:
		casacore::Bool titleChanged;

	private:

		// global switch on or off
		casacore::Bool itsOptionsAxisLabelSwitch;

		// display title text
		casacore::String itsOptionsTitleText;

		// title color
		casacore::String itsOptionsTitleTextColor;

		// display axes label text
		casacore::String itsOptionsXAxisText, itsOptionsYAxisText;

		// are the above unset
		casacore::Bool itsOptionsXAxisTextUnset, itsOptionsYAxisTextUnset;

		// label text color
		casacore::String itsOptionsXAxisTextColor, itsOptionsYAxisTextColor;

		// coordinate grid?
		casacore::String itsOptionsXGridType, itsOptionsYGridType;

		// coordinate grid colors
		casacore::String itsOptionsXGridColor, itsOptionsYGridColor;

		// tick mark length in mm
		casacore::Float itsOptionsTickLength;

		// Label position
		casacore::String itsOptionsLabelPos;

		// plot outline?
		casacore::Bool itsOptionsPlotOutline;

		// plot outline color
		casacore::String itsOptionsPlotOutlineColor;

		// plot character size
		casacore::Float itsOptionsCharSize;

		// plot character font
		casacore::String itsOptionsCharFont;

		// plot line width
		//casacore::Int itsOptionsLineWidth;
		casacore::Float itsOptionsLineWidth;

		// defaults for on/off switch and character size
		// (from .aipsrc, if they exist there).
		casacore::Bool itsDefaultSwitch;
		casacore::Float itsDefaultCharSize;
		casacore::String substituteTitleText;

	};


} //# NAMESPACE CASA - END

#endif
