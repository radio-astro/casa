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
  Galaxy(Vector<Double>& position, Vector<Double>& velocity, Double mass,
         Double size, uInt numStars, uInt numRings, Double inclination, 
         Double positionAngle, Int xSize, Int ySize);

  // Destructor
  ~Galaxy();

  // Compute a new position, applying the force of the Galaxies in the List
  // but do not update current position and velocity
  void computeStep(List<void *>& galaxyList, Double timeStep, 
                  Double dampingFactor = 1.0);

  // update position and velocity to new value
  void update();
  
  // Compute a new position for the Stars in this Galaxy
  void applyForceToStars(List<void *>& galaxyList, Double timeStep,
                         Double dampingFactor);
  
  // Rotate position and velocity
  void rotate(Matrix<Double>& rotMatrix);

  // Draw the Galaxy on the PixelCanvas
  void draw(PixelCanvas *pixelCanvas);

  // magnitude of the force for this galaxy
  Double force(Double distance);
  
  // Return the Mass of the Galaxy
  Double getMass();

  // Return the position of the Galaxy
  Vector<Double>& getPosition();
  
  // Return the Velocity of the Galaxy
  Vector<Double>& getVelocity();

  // Set the plot mode
  void setPlotMode(Galaxy::PLOT_MODE plotMode);

  void setScale(Int xSize, Int ySize)   ;

 private:

  // The mass of the Galaxy
  Double itsMass;  

  // its size
  Double itsSize;

  // The position of the Galaxy
  Vector<Double> itsPosition;

  // The velocity of the Galaxy
  Vector<Double> itsVelocity;


  // The new computed position of the Galaxy
  Vector<Double> newPosition;

  // The new computed velocity of the Galaxy
  Vector<Double> newVelocity;

  // The list of Stars in this Galaxy
  List<void *>   itsStarList;
  // and an iterator for this List
  
  ListIter<void *> *itsStarListIter;
  
  Vector<Int> oldPosition;
  // Last offset applied to itsPosition
  Vector<Int> itsOffset;
  // Last offset applied to itsPosition
  Vector<Int> itsPlotPosition;

  Int itsXSize;
  Int itsYSize;
  
  // Number of the draw list for the current PixelCanvas
  uInt           itsDrawList;

  PixelCanvas    *itsPixelCanvas;

  Galaxy::PLOT_MODE plotMode;
  
  Bool changedPlotMode;
};

// <summary>
// Simple class which represents a star as a point mass.
// </summary>

class Star {

 public:

  // Constructor
  Star(Vector<Double>& position, Vector<Double>& velocity, 
       Int xSize, Int ySize);

  // Destructor
  ~Star();

  // Draw the current position on the PixelCanvas
  void draw(PixelCanvas *pixelCanvas);

  // Compute new position by applying the forces of the Galaxies in the List
  void applyForce(List<void *>& galaxyList, Double timeStep, 
                  Double dampingFactor);

  // Rotate position and velocity
  void rotate(Matrix<Double>& rotMatrix);

  // Set the plot mode
  void setPlotMode(Galaxy::PLOT_MODE newPlotMode);
  
  void setScale(Int xSize, Int ySize)   ;

 private:

  // Position of the Star
  Vector<Double> itsPosition;
  // The last drawn position of the Star 
  Vector<Int> oldPosition;
  // Last offset applied to itsPosition
  Vector<Int> itsOffset;
  // Last offset applied to itsPosition
  Vector<Int> itsPlotPosition;

  // Velocity of the Star
  Vector<Double> itsVelocity;

  Int itsXSize;
  Int itsYSize;
  
  // Number of the draw list for the current PixelCanvas
  uInt           itsDrawList;

  // Temp to check if we are drawing on a new PixelCanvas
  PixelCanvas    *itsPixelCanvas;

  Galaxy::PLOT_MODE plotMode;
  
  Bool changedPlotMode;
};


} //# NAMESPACE CASA - END

#endif
