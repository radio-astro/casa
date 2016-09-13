//# WedgeDD.h: Color Wedge DisplayData
//# Copyright (C) 2001,2002
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

#ifndef TRIALDISPLAY_WEDGEDD_H
#define TRIALDISPLAY_WEDGEDD_H

#include <casa/aips.h>
#include <display/DisplayDatas/ActiveCaching2dDD.h>
#include <display/Display/DisplayCoordinateSystem.h>
#include <display/DisplayCanvas/WCCSNLAxisLabeller.h>

namespace casacore{

	template <class T> class Matrix;
}

namespace casa { //# NAMESPACE CASA - BEGIN

	class WCPowerScaleHandler;

	class WedgeDM;

// <summary>
// A DisplayData to draw color wedges
// </summary>
//
// <use visibility=export>
//
// <reviewed reviewer="" date="yyyy/mm/dd" tests="" demos="">
// </reviewed>
//
// <prerequisite>
//   <li> DisplayData
//   </li> CachingDisplayData
// </prerequisite>
//
// <etymology>
// </etymology>
//
// <synopsis> This DisplayData can be created by itself. It's main
// task though is to set up a coordinate system form the data range
// and units of another DisplayData. The user can control this through
// options in this DisplayData: "datamin", "datamax, "dataunit" and
// "powercycles".

// </synopsis>
//
// <example>
// <srcBlock>
// </srcBlock>
// </example>
//
// <motivation>
// Users want wedges
//</motivation>
//
// <todo>
// Transposed wedges
// </todo>
//

	class WedgeDD : public ActiveCaching2dDD {

	public:

		WedgeDD( DisplayData *image );

		// Destructor.
		virtual ~WedgeDD();

		// Constructor help function, creates coordinatesystem and fills data Matrix
		virtual void setup();

		// The coordinate sydtem needs to be update when the data range changes
		virtual void updateCsys();

		// Axis labeller, 5th step in the WCHolder refresh cycle
		virtual casacore::Bool labelAxes(const WCRefreshEvent &ev);
		virtual casacore::Bool canLabelAxes() const;
		bool isDisplayable( ) const;

		// Return the data unit.
		virtual const casacore::Unit dataUnit() const;
		const casacore::IPosition dataShape() const {
			return casacore::IPosition( );
		}
		casacore::uInt dataDim() const {
			return 0;
		}
		std::vector<int> displayAxes( ) const {
			return std::vector<int>( );
		}

		// casacore::Format the wedge value at the given world position.
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
			return Display::Raster;
		}
		// Pure virtual function from DisplayData...
		casacore::String dataType() const {
			return "wedge";
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

		// Take actions on removal from WC[H] (notably, deletion of drawlists).
		virtual void notifyUnregister(WorldCanvasHolder& wcHolder,
		                              casacore::Bool ignoreRefresh = false);

		const static casacore::String WEDGE_PREFIX;

		std::string errorMessage( ) const { return ""; }

	protected:



	private:
		// (Required) copy constructor.
		WedgeDD(const WedgeDD &other);

		// (Required) copy assignment.
		void operator=(const WedgeDD &other);
		friend class WedgeDM;
		// casacore::Data minimum and maximum to set up the coordinate system
		casacore::Float itsMin, itsMax;
		// the length of the Vector
		casacore::uInt itsLength;
		// the unit of the input data
		casacore::String itsDataUnit;
		//<group>
		// the power law adjustment for for the scle handler
		casacore::Float itsPowerCycles;
		WCPowerScaleHandler* itsPowerScaleHandler;

		casacore::String itsOptionsMode;
		//</group>
		// the axis labeller
		WCCSNLAxisLabeller itsAxisLabeller;
		// The actual colorbar data
		casacore::Matrix<casacore::Float> itsColorbar;
		// the local coordinate system - gets exported to the parent DD
		DisplayCoordinateSystem itsCoordinateSystem;

		DlHandle<DisplayData> ihandle_;
	};


} //# NAMESPACE CASA - END

#endif
