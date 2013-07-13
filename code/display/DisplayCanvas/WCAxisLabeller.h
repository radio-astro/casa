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
		virtual Bool draw(const WCRefreshEvent &ev) = 0;

		// Invalidate any cached drawings
		virtual void invalidate() { };

		// install the default options for this DisplayData
		virtual void setDefaultOptions();

		// apply options stored in rec to the DisplayData; return value
		// True means a refresh is needed.  Any fields added to the
		// updatedOptions argument are options which have changed in
		// some way due to the setting of other options - ie. they
		// are context sensitive.
		virtual Bool setOptions(const Record &rec, Record &updatedOptions);

		// retrieve the current and default options and parameter types.
		virtual Record getOptions() const;

		// set/return whether labelling is on or off.  This is a global
		// switch.  If True is returned from the set function, then a
		// refresh is required.
		// <group>
		virtual Bool setAxisLabelSwitch(const Bool labelswitch = False);
		virtual Bool axisLabelSwitch() const {
			return itsOptionsAxisLabelSwitch;
		}
		// </group>

		// set/return the title text.  If True is returned from the set
		// function then a refresh is required.
		// <group>
		//virtual Bool setTitleText(const String text = String(""));
		virtual void setSubstituteTitleText( const String substituteImageName);
		virtual String titleText() const {
			return itsOptionsTitleText;
		}
		virtual String displayedTitleText() const {
			String actualText = substituteTitleText;
			if ( actualText.length() == 0 ){
				actualText = itsOptionsTitleText;
			}
			return actualText;
		}
		// </group>

		// set/return the color of the title text.  If True is returned from
		// the set function, then a refresh should be called.
		// <group>
		virtual Bool setTitleTextColor(const String color = String("foreground"));
		virtual String titleTextColor() const {
			return itsOptionsTitleTextColor;
		}
		// </group>

		// set/return the X and Y label text.  If True is returned from
		// the set functions, then a refresh is needed.
		// <group>
		virtual Bool setXAxisText(const String text = String(""));
		virtual Bool setYAxisText(const String text = String(""));
		virtual Bool unsetXAxisText();
		virtual Bool unsetYAxisText();
		virtual String xAxisText() const;
		virtual String yAxisText() const;
		virtual Bool isXAxisTextUnset() const {
			return itsOptionsXAxisTextUnset;
		}
		virtual Bool isYAxisTextUnset() const {
			return itsOptionsYAxisTextUnset;
		}
		// </group>

		// set/return the color of the label text.  If True is returned
		// from the set function, then a refresh is needed.
		// <group>
		virtual Bool setXAxisTextColor(const String color = String("foreground"));
		virtual Bool setYAxisTextColor(const String color = String("foreground"));
		virtual String xAxisTextColor() const {
			return itsOptionsXAxisTextColor;
		}
		virtual String yAxisTextColor() const {
			return itsOptionsYAxisTextColor;
		}
		// </group>

		// set/return what type of grid is marked in each direction.
		// If True is returned from the set functions, a refresh is needed.
		// <group>
		virtual Bool setXGridType(const String type = String("None"));
		virtual Bool setYGridType(const String type = String("None"));
		virtual String xGridType() const {
			return itsOptionsXGridType;
		}
		virtual String yGridType() const {
			return itsOptionsYGridType;
		}
		// </group>

		// set/return the color of the grid lines per direction.  If True
		// is returned from the set functions, a refresh is needed.
		// <group>
		virtual Bool setXGridColor(const String color = String("foreground"));
		virtual Bool setYGridColor(const String color = String("foreground"));
		virtual String xGridColor() const {
			return itsOptionsXGridColor;
		}
		virtual String yGridColor() const {
			return itsOptionsYGridColor;
		}
		// </group>

		// set/return the tick length in millimetres.  If True is returned
		// from the set function, then a refresh is probably needed.
		// <group>
		virtual Bool setTickLength(const Float length = 4.0);
		virtual Float tickLength() const {
			return itsOptionsTickLength;
		}
		// </group>

		// set/return the position of World label strings
		// <group>
		virtual Bool setLabelPosition(const String position = String("Auto"));
		virtual String labelPosition() const {
			return itsOptionsLabelPos;
		}
		// </group>


		// set/return whether there is a plot outline or not.  If True is
		// returned from the set function, then a refresh is probably needed.
		// <group>
		virtual Bool setPlotOutline(const Bool outline = True);
		virtual Bool plotOutline() const {
			return itsOptionsPlotOutline;
		}
		// </group>

		// set/return the color used for the plot outline.  If True is
		// returned from the set function, then a refresh should be
		// enacted.
		// <group>
		virtual Bool setPlotOutlineColor(const String color = String("foreground"));
		virtual String plotOutlineColor() const {
			return itsOptionsPlotOutlineColor;
		}
		// </group>

		// set/return the char size for the plot graphics
		// <group>
		virtual Bool setCharSize(const Float size = 1.2);
		virtual Float charSize() const {
			return itsOptionsCharSize;
		}
		// </group>

		// set/return the char font for the plot graphics
		// <group>
		virtual Bool setCharFont(const String font = "normal");
		virtual String charFont() const {
			return itsOptionsCharFont;
		}
		// </group>

		// set/return the line width for the plot graphics.
		// <group>
		/*
		virtual Bool setLineWidth(const Int width = 0);
		virtual Int lineWidth() const
		  { return itsOptionsLineWidth; }
		*/
		virtual Bool setLineWidth(const Float width = 0.0);
		virtual Float lineWidth() const {
			return itsOptionsLineWidth;
		}
		// </group>

		static const String LABEL_CHAR_SIZE;
		static const String PLOT_TITLE;

	protected:
		Bool titleChanged;

	private:

		// global switch on or off
		Bool itsOptionsAxisLabelSwitch;

		// display title text
		String itsOptionsTitleText;

		// title color
		String itsOptionsTitleTextColor;

		// display axes label text
		String itsOptionsXAxisText, itsOptionsYAxisText;

		// are the above unset
		Bool itsOptionsXAxisTextUnset, itsOptionsYAxisTextUnset;

		// label text color
		String itsOptionsXAxisTextColor, itsOptionsYAxisTextColor;

		// coordinate grid?
		String itsOptionsXGridType, itsOptionsYGridType;

		// coordinate grid colors
		String itsOptionsXGridColor, itsOptionsYGridColor;

		// tick mark length in mm
		Float itsOptionsTickLength;

		// Label position
		String itsOptionsLabelPos;

		// plot outline?
		Bool itsOptionsPlotOutline;

		// plot outline color
		String itsOptionsPlotOutlineColor;

		// plot character size
		Float itsOptionsCharSize;

		// plot character font
		String itsOptionsCharFont;

		// plot line width
		//Int itsOptionsLineWidth;
		Float itsOptionsLineWidth;

		// defaults for on/off switch and character size
		// (from .aipsrc, if they exist there).
		Bool itsDefaultSwitch;
		Float itsDefaultCharSize;
		String substituteTitleText;

	};


} //# NAMESPACE CASA - END

#endif
