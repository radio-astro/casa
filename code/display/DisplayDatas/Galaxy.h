//# Galaxy.h: Simple classes for galaxies and stars
//# Copyright (C) 1994,1995,1996,1997,1998,1999
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

#ifndef TRIALDISPLAY_GALAXY_H
#define TRIALDISPLAY_GALAXY_H

#include <casa/aips.h>
#include <casa/Arrays/Vector.h>
#include <casa/Arrays/Matrix.h>
#include <casa/Containers/List.h>

namespace casa { //# NAMESPACE CASA - BEGIN

	class PixelCanvas;

// <summary>
// Simple class to represent a Galaxy as a collection of Stars.
// </summary>

	class Galaxy {

	public:

		enum PLOT_MODE {
		    POSITION,
		    VELOCITY
		};

		// Constructor
		Galaxy(casacore::Vector<casacore::Double>& position, casacore::Vector<casacore::Double>& velocity, casacore::Double mass,
		       casacore::Double size, casacore::uInt numStars, casacore::uInt numRings, casacore::Double inclination,
		       casacore::Double positionAngle, casacore::Int xSize, casacore::Int ySize);

		// Destructor
		~Galaxy();

		// Compute a new position, applying the force of the Galaxies in the List
		// but do not update current position and velocity
		void computeStep(casacore::List<void *>& galaxyList, casacore::Double timeStep,
		                 casacore::Double dampingFactor = 1.0);

		// update position and velocity to new value
		void update();

		// Compute a new position for the Stars in this Galaxy
		void applyForceToStars(casacore::List<void *>& galaxyList, casacore::Double timeStep,
		                       casacore::Double dampingFactor);

		// Rotate position and velocity
		void rotate(casacore::Matrix<casacore::Double>& rotMatrix);

		// Draw the Galaxy on the PixelCanvas
		void draw(PixelCanvas *pixelCanvas);

		// magnitude of the force for this galaxy
		casacore::Double force(casacore::Double distance);

		// Return the Mass of the Galaxy
		casacore::Double getMass();

		// Return the position of the Galaxy
		casacore::Vector<casacore::Double>& getPosition();

		// Return the Velocity of the Galaxy
		casacore::Vector<casacore::Double>& getVelocity();

		// Set the plot mode
		void setPlotMode(Galaxy::PLOT_MODE plotMode);

		void setScale(casacore::Int xSize, casacore::Int ySize)   ;

	private:

		// The mass of the Galaxy
		casacore::Double itsMass;

		// its size
		casacore::Double itsSize;

		// The position of the Galaxy
		casacore::Vector<casacore::Double> itsPosition;

		// The velocity of the Galaxy
		casacore::Vector<casacore::Double> itsVelocity;


		// The new computed position of the Galaxy
		casacore::Vector<casacore::Double> newPosition;

		// The new computed velocity of the Galaxy
		casacore::Vector<casacore::Double> newVelocity;

		// The list of Stars in this Galaxy
		casacore::List<void *>   itsStarList;
		// and an iterator for this List

		casacore::ListIter<void *> *itsStarListIter;

		casacore::Vector<casacore::Int> oldPosition;
		// Last offset applied to itsPosition
		casacore::Vector<casacore::Int> itsOffset;
		// Last offset applied to itsPosition
		casacore::Vector<casacore::Int> itsPlotPosition;

		casacore::Int itsXSize;
		casacore::Int itsYSize;

		// Number of the draw list for the current PixelCanvas
		casacore::uInt           itsDrawList;

		PixelCanvas    *itsPixelCanvas;

		Galaxy::PLOT_MODE plotMode;

		casacore::Bool changedPlotMode;
	};

// <summary>
// Simple class which represents a star as a point mass.
// </summary>

	class Star {

	public:

		// Constructor
		Star(casacore::Vector<casacore::Double>& position, casacore::Vector<casacore::Double>& velocity,
		     casacore::Int xSize, casacore::Int ySize);

		// Destructor
		~Star();

		// Draw the current position on the PixelCanvas
		void draw(PixelCanvas *pixelCanvas);

		// Compute new position by applying the forces of the Galaxies in the List
		void applyForce(casacore::List<void *>& galaxyList, casacore::Double timeStep,
		                casacore::Double dampingFactor);

		// Rotate position and velocity
		void rotate(casacore::Matrix<casacore::Double>& rotMatrix);

		// Set the plot mode
		void setPlotMode(Galaxy::PLOT_MODE newPlotMode);

		void setScale(casacore::Int xSize, casacore::Int ySize)   ;

	private:

		// Position of the Star
		casacore::Vector<casacore::Double> itsPosition;
		// The last drawn position of the Star
		casacore::Vector<casacore::Int> oldPosition;
		// Last offset applied to itsPosition
		casacore::Vector<casacore::Int> itsOffset;
		// Last offset applied to itsPosition
		casacore::Vector<casacore::Int> itsPlotPosition;

		// Velocity of the Star
		casacore::Vector<casacore::Double> itsVelocity;

		casacore::Int itsXSize;
		casacore::Int itsYSize;

		// Number of the draw list for the current PixelCanvas
		casacore::uInt           itsDrawList;

		// Temp to check if we are drawing on a new PixelCanvas
		PixelCanvas    *itsPixelCanvas;

		Galaxy::PLOT_MODE plotMode;

		casacore::Bool changedPlotMode;
	};


} //# NAMESPACE CASA - END

#endif
