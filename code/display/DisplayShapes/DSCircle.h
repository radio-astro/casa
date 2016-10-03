//# DSCircle.h: Circle implementation for "DisplayShapes"
//# Copyright (C) 1998,1999,2000,2001,2002
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
//# $Id:
#ifndef TRIALDISPLAY_DSCIRCLE_H
#define TRIALDISPLAY_DSCIRCLE_H

#include <casa/aips.h>

#include <display/DisplayShapes/DSEllipse.h>
#include <casa/Arrays/Matrix.h>
#include <casa/Arrays/Vector.h>


namespace casa { //# NAMESPACE CASA - BEGIN

// <summary>
// Circle implementation;extends DSEllipse, adding general circle functions
// </summary>
//
// <prerequisite>
// <li> <linkto class="DSEllipse">DSEllipse</linkto>
// <li> <linkto class="DisplayShape">DisplayShape</linkto>
// </prerequisite>
//
// <etymology>
// DSCircle is a method of managing the drawing of a circle onto a PixelCanvas.
// </etymology>
//
// <synopsis>
// DSCircle simply extends from DSEllipse, and adds some general circle
// functionality. It basically replaces the set major and set minor axis
// functions with a setRadius function. Almost all of the functionality
// is contained in DSEllipse.
//
// There are generally two ways to make DisplayShape(s); To create them in
// "one hit" by providing arguments to the constructor, or by using the
// default constructor and then the "setOptions" method. A simple interface
// for all classes inheriting from the
// <linkto class="DisplayShape">DisplayShape</linkto> class is provided by
// <linkto class="DisplayShapeInterface">DisplayShapeInterface</linkto>.
// </synopsis>
//
// <motivation>
// To enable display of circles onto a pixel canvas.
// </motivation>
//
// <example>
// <srcblock>
// </srcblock>
// </example>



	class DSCircle : public DSEllipse {

	public:

		// Constructors and destructors.
		// <group>
		DSCircle();
		DSCircle(const casacore::Float& xPos, const casacore::Float& yPos, const casacore::Float& radius,
		         const casacore::Bool& hasHandles, const casacore::Bool& drawHandles);
		DSCircle(const DSCircle& other);
		virtual ~DSCircle();
		// </group>

		// General DisplayShape functionality
		// <group>
		virtual void rotate(const casacore::Float& /*angle*/) {};
		virtual void setCenter(const casacore::Float& xPos, const casacore::Float& yPos);
		virtual casacore::Bool inObject(const casacore::Float& dX, const casacore::Float& dY);
		virtual void changePoint(const casacore::Vector<casacore::Float>& newPos);
		virtual void changePoint(const casacore::Vector<casacore::Float>& newPos, const casacore::Int nPoint);
		// </group>

		// Circle specific functionality
		// <group>
		virtual void setRadius(const casacore::Float& radius);
		virtual void setMajorAxis(const casacore::Float& /*major*/) {};
		virtual void setMinorAxis(const casacore::Float& /*minor*/) {};
		// </group>

		// Get and set options
		// <group>
		virtual casacore::Bool setOptions(const casacore::Record& settings);
		virtual casacore::Record getOptions();
		// </group>

	private:
		// casacore::Function to set defaults when default constructor called
		virtual void setDefaultOptions();

	};

} //# NAMESPACE CASA - END

#endif






