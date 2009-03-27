//# Galaxy.cc: most simple implementation of n-body galaxy simulation
//# Copyright (C) 1994,1995,1996,1997,1998,1999,2000
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

#include <casa/aips.h>
#include <casa/Arrays/Vector.h>
#include <casa/Arrays/Matrix.h>
#include <casa/Arrays/ArrayMath.h>
#include <casa/Arrays/MatrixMath.h>
#include <casa/Containers/List.h>
#include <display/Display/PixelCanvas.h>


#include <display/DisplayDatas/Galaxy.h>


namespace casa { //# NAMESPACE CASA - BEGIN

Star::Star(Vector<Double>& position, Vector<Double>& velocity, 
           Int xSize, Int ySize) :
#if 1
  itsPosition(position.copy()),
  oldPosition(3,0),
  itsOffset(3,0),
  itsPlotPosition(3,0),
  itsVelocity(velocity.copy()),
#endif
  itsXSize(xSize), 
  itsYSize(ySize),
  itsDrawList(0), 
  itsPixelCanvas(0), 
  plotMode(Galaxy::POSITION),
  changedPlotMode(True)
{
#if 0
  itsPosition.resize(0);
  itsPosition = position;

  oldPosition.resize(3);
  oldPosition = 0;
  
  itsVelocity.resize(0);
  itsVelocity = velocity;
  
  itsOffset.resize(3);
  itsOffset = 0;

  itsPlotPosition.resize(3);
  itsPlotPosition = 0;
#endif
  
};

Star::~Star() 
{
  if (itsPixelCanvas != 0) {
    itsPixelCanvas->deleteList(itsDrawList);
  }
}

void Star::draw(PixelCanvas *pCanvas) 
{
  // we assume that the drawmode has been set

  if (itsPixelCanvas != 0) {    
    if ((pCanvas != itsPixelCanvas) || (changedPlotMode) ) {
      // something changed, so delete DrawList
      itsPixelCanvas->deleteList(itsDrawList);
      // and store PixelCanvas
      itsPixelCanvas = pCanvas;
      // create new DrawList
      itsDrawList = pCanvas->newList();
      // draw the star
      pCanvas->drawPoint(itsPlotPosition(0), itsPlotPosition(1));
      // end drawlist
      pCanvas->endList();
      // end execute the drawing
      pCanvas->drawList(itsDrawList);
    } else {
      pCanvas->drawList(itsDrawList);
      pCanvas->translateList(itsDrawList, itsOffset(0), itsOffset(1));    
      pCanvas->drawList(itsDrawList);
    }
  } else {
    // used for first time plot, store PixelCanvas
    itsPixelCanvas = pCanvas;
    // create new drawlist
    itsDrawList = pCanvas->newList();
    // draw the star
    pCanvas->drawPoint(itsPlotPosition(0), itsPlotPosition(1));
    // close the drawlist
    pCanvas->endList();
    // and execute draw
    pCanvas->drawList(itsDrawList);
  }
  changedPlotMode = False;
  
}

    
void Star::applyForce(List<void *>& galaxyList, Double timeStep, 
                      Double /* dampingFactor */) 
{
  Galaxy *galaxy;
    // create iterator and temps
  ListIter<void *> iter(&galaxyList);
  Vector<Double> relPosition(3);
  Vector<Double> force(3);
  Double galForce;
  Double distance;
  Int i;
  
  force = 0.0;
  
  // loop through List
  while (!iter.atEnd()) {
    // get next galaxy from List
    galaxy = (Galaxy *)iter.getRight();
    // get galaxy position relative to this Star
    relPosition = galaxy->getPosition();
    distance = 0.0;
    
    for (i = 0; i < 3; i++) {
      relPosition(i) -= itsPosition(i);
    // get distance
      distance += relPosition(i)*relPosition(i);
    }

    distance = sqrt(distance);
    
    if (distance > 1.0) {
      // get the force for this distance
      galForce = galaxy->force(distance);
      for (i = 0; i < 3; i++) {
        // normalize relative position
        relPosition(i) /= distance;
        // and add to total force
        force(i) += relPosition(i)*galForce;
      }
    }
    
    
    iter++;
  }

  for (i = 0; i < 3; i++) {
    itsVelocity(i) = itsVelocity(i) + timeStep*force(i);
    itsPosition(i) = itsPosition(i) + timeStep*itsVelocity(i);
  }

  switch (plotMode) {
    default :
    case Galaxy::POSITION : {
      if (changedPlotMode) {
        oldPosition(0) = Int(itsPosition(0)+itsXSize/2);
        itsPlotPosition(0) = oldPosition(0);
        oldPosition(1) = Int(itsPosition(1)+itsYSize/2);
        itsPlotPosition(1) = oldPosition(1);
      } else {
        itsPlotPosition(0) = Int(itsPosition(0)+itsXSize/2);
        itsOffset(0) = itsPlotPosition(0) - oldPosition(0);
        if (itsOffset(0) != 0) {
          oldPosition(0) = itsPlotPosition(0);
        }
        itsPlotPosition(1) = Int(itsPosition(1)+itsYSize/2);
        itsOffset(1) = itsPlotPosition(1) - oldPosition(1);
        if (itsOffset(1) != 0) {
          oldPosition(1) = itsPlotPosition(1);
        } 
      }
      break;
    }
    case Galaxy::VELOCITY : {
      if (changedPlotMode) {
        oldPosition(0) = Int(itsPosition(0)+itsXSize/2);
        itsPlotPosition(0) = oldPosition(0);
        oldPosition(1) = Int(itsYSize/4*itsVelocity(2) + itsYSize/2);
        itsPlotPosition(1) = oldPosition(1);
      } else {
        itsPlotPosition(0) = Int(itsPosition(0) +itsXSize/2  );
        itsOffset(0) = itsPlotPosition(0)  - oldPosition(0);
        if (itsOffset(0) != 0) {
          oldPosition(0) = itsPlotPosition(0);
        } 
        itsPlotPosition(1) = Int(itsYSize/4*itsVelocity(2) + itsYSize/2);
        itsOffset(1) = itsPlotPosition(1) - oldPosition(1);
        if (itsOffset(1) != 0) {
          oldPosition(1) = itsPlotPosition(1);
        } 
      }
      break;
    }
  }  // of switch
  
}

void Star::setScale(Int xSize, Int ySize) 
{
  itsXSize = xSize;
  itsYSize = ySize;
  changedPlotMode = True;
}

    
// Rotate the position and velocity using a giuven rotation Matrix  
void Star::rotate(Matrix<Double>& rotMatrix) 
{
  itsPosition = product(rotMatrix, itsPosition);
  itsVelocity = product(rotMatrix, itsVelocity);
}

// Set the plot mode
void Star::setPlotMode(Galaxy::PLOT_MODE newPlotMode) 
{
  plotMode = newPlotMode;
  changedPlotMode = True;
}


Galaxy::Galaxy(Vector<Double>& position, Vector<Double>& velocity, 
               Double mass, Double size, uInt numStars, uInt numRings, 
               Double inclination, Double positionAngle, Int xSize, Int ySize) : 
  itsMass(mass),  
  itsXSize(xSize), 
  itsYSize(ySize),
  itsDrawList(0), 
  itsPixelCanvas(0), 
  plotMode(Galaxy::POSITION),
  changedPlotMode(True)
{
  itsPosition.resize(0);
  itsPosition = position;

  itsVelocity.resize(0);
  itsVelocity = velocity;
 
  newPosition.resize(0);
  newPosition = position;

  newVelocity.resize(0);
  newVelocity = velocity;

  oldPosition.resize(3);
  oldPosition = 0;

  itsOffset.resize(3);
  itsOffset = 0;

  itsPlotPosition.resize(3);
  itsPlotPosition = 0;

  itsSize = 5.0;
  
  itsStarListIter = new ListIter<void *>(&itsStarList); 

  Star *star;
  Double ringSize;
  Int starsInRing;
  Double angle = 0.0;
  Double angleStep = 1.0;
  Vector<Double> starPos(3);
  Vector<Double> starVel(3);
  Matrix<Double> rotMatrix(3,3);
  Matrix<Double> incMatrix(3,3);
  Matrix<Double> paMatrix(3,3);
  
  incMatrix = Rot3D(0, inclination);
  paMatrix = Rot3D(2, positionAngle);
  
  for (uInt ring = 0; ring < numRings; ring++) {
    ringSize = (ring+3)*size;
    starsInRing = (ring+1)*numStars;
    
    for (Int inRing = 0; inRing < starsInRing; inRing++) {
      starPos = 0.0;
      starPos(0) = ringSize;
      rotMatrix = Rot3D(2, angle);
      angle += angleStep;
      starPos = product(rotMatrix, starPos);
      starPos = product(incMatrix, starPos);
      starPos = product(paMatrix, starPos);
      
      starPos += itsPosition;
      
      starVel = 0.0;
      starVel(1) = sqrt(ringSize*force(ringSize));
      starVel = product(rotMatrix, starVel);
      starVel = product(incMatrix, starVel);
      starVel = product(paMatrix, starVel);
      starVel += itsVelocity;
      
      star = new Star(starPos, starVel, xSize, ySize);
      itsStarListIter->addRight((void *) star);
    }
  }
}



Galaxy::~Galaxy() 
{
  if (itsPixelCanvas != 0) {
    itsPixelCanvas->deleteList(itsDrawList);
  }

  Star *star;
  
  itsStarListIter->toStart();
  while ( !itsStarListIter->atEnd() ) {
    star = (Star *) itsStarListIter->getRight();
    delete star;
    (*itsStarListIter)++;
  }
  delete itsStarListIter;
}



    
void Galaxy::computeStep(List<void *>& galaxyList, Double timeStep, 
                   Double dampingFactor) 
{
    // create iterator and temps
  ListIter<void *> iter(&galaxyList);
  Vector<Double> relPosition(3);
  Vector<Double> force(3);
  Double galForce;
  Double distance;
  Galaxy *galaxy;
    
  force = 0.0;
  
  // loop through List
  while (!iter.atEnd()) {
    // get next galaxy from List
    galaxy = (Galaxy *)iter.getRight();

    // get galaxy position relative to this Star
    relPosition = (galaxy->getPosition())-itsPosition;

    // get distance
    distance = norm(relPosition);

    // normalize relative position
    if (distance > 1.0) {

      relPosition /= distance;
      // get the force for this distance

      galForce = galaxy->force(distance);
      // and add to total force

      force += relPosition*galForce;
    }

    
    iter++;
  }

  
  newVelocity = itsVelocity + timeStep*force;
  newVelocity *= dampingFactor;
  newPosition = itsPosition + timeStep*newVelocity;
  
}

void Galaxy::update() 
{
  itsPosition = newPosition;
  itsVelocity = newVelocity;
  
  switch (plotMode) {
    default :
    case Galaxy::POSITION : {
      if (changedPlotMode) {
        oldPosition(0) = Int(itsPosition(0)+itsXSize/2);
        itsPlotPosition(0) = oldPosition(0);
        oldPosition(1) = Int(itsPosition(1)+itsYSize/2);
        itsPlotPosition(1) = oldPosition(1);
      } else {
        itsPlotPosition(0) = Int(itsPosition(0)+itsXSize/2);
        itsOffset(0) = itsPlotPosition(0) - oldPosition(0);
        if (itsOffset(0) != 0) {
          oldPosition(0) = itsPlotPosition(0);
        }
        
        itsPlotPosition(1) = Int(itsPosition(1)+itsYSize/2);
        itsOffset(1) = itsPlotPosition(1) - oldPosition(1);
        if (itsOffset(1) != 0) {
          oldPosition(1) = itsPlotPosition(1);
        } 
      }
      break;
    }
    case Galaxy::VELOCITY : {
      if (changedPlotMode) {
        oldPosition(0) = Int(itsPosition(0)+itsXSize/2);
        itsPlotPosition(0) = oldPosition(0);
        oldPosition(1) = Int(itsYSize/4*itsVelocity(2) + itsYSize/2);
        itsPlotPosition(1) = oldPosition(1);
      } else {
        itsPlotPosition(0) = Int(itsPosition(0) + itsXSize/2);
        itsOffset(0) = itsPlotPosition(0)  - oldPosition(0);
        if (itsOffset(0) != 0) {
          oldPosition(0) = itsPlotPosition(0);
        } 
        itsPlotPosition(1) = Int(itsYSize/4*itsVelocity(2) + itsYSize/2);
        itsOffset(1) = itsPlotPosition(1) - oldPosition(1);
        if (itsOffset(1) != 0) {
          oldPosition(1) = itsPlotPosition(1);
        } 
      }
      break;
    }
  }  // of switch

}

  
void Galaxy::applyForceToStars(List<void *>& galaxyList, Double timeStep,
                               Double dampingFactor)
{
  Star *star;

  itsStarListIter->toStart();
  while ( !itsStarListIter->atEnd() ) {
    star = (Star *) itsStarListIter->getRight();
    star->applyForce(galaxyList, timeStep, dampingFactor);
    (*itsStarListIter)++;
  }

  
}

// Rotate the position and velocity using a given rotation Matrix  
void Galaxy::rotate(Matrix<Double>& rotMatrix) 
{
  itsPosition = product(rotMatrix, itsPosition);
  itsVelocity = product(rotMatrix, itsVelocity);

  Star *star;

  itsStarListIter->toStart();
  while ( !itsStarListIter->atEnd() ) {
    star = (Star *) itsStarListIter->getRight();
    star->rotate(rotMatrix);
    (*itsStarListIter)++;
  }
}

// Set the plot mode
void Galaxy::setPlotMode(Galaxy::PLOT_MODE newPlotMode) 
{
  plotMode = newPlotMode;
  changedPlotMode = True;

  Star *star;
  
  itsStarListIter->toStart();
  while ( !itsStarListIter->atEnd() ) {
    star = (Star *) itsStarListIter->getRight();
    star->setPlotMode(newPlotMode);
    (*itsStarListIter)++;
  }
}

void Galaxy::draw(PixelCanvas *pCanvas) 
{

  // we assume that the drawmode has been set

  if (itsPixelCanvas != 0) {    
    if ((pCanvas != itsPixelCanvas) || (changedPlotMode) ) {
      // something changed, so delete DrawList
      itsPixelCanvas->deleteList(itsDrawList);
      // and store PixelCanvas
      itsPixelCanvas = pCanvas;
      // create new DrawList
      itsDrawList = pCanvas->newList();
      // draw the star
      pCanvas->drawLine(itsPlotPosition(0)-5, itsPlotPosition(1), 
                        itsPlotPosition(0)+5, itsPlotPosition(1));
      pCanvas->drawLine(itsPlotPosition(0), itsPlotPosition(1)-5, 
                        itsPlotPosition(0), itsPlotPosition(1)+5);
      // end drawlist
      pCanvas->endList();
      // end execute the drawing
      pCanvas->drawList(itsDrawList);
    } else {
      pCanvas->drawList(itsDrawList);
      pCanvas->translateList(itsDrawList, itsOffset(0), itsOffset(1));    
      pCanvas->drawList(itsDrawList);
    }
  } else {
    // used for first time plot, store PixelCanvas
    itsPixelCanvas = pCanvas;
    // create new drawlist
    itsDrawList = pCanvas->newList();
    // draw the star
    pCanvas->drawLine(itsPlotPosition(0)-5, itsPlotPosition(1), 
                      itsPlotPosition(0)+5, itsPlotPosition(1));
    pCanvas->drawLine(itsPlotPosition(0), itsPlotPosition(1)-5, 
                      itsPlotPosition(0), itsPlotPosition(1)+5);

    // close the drawlist
    pCanvas->endList();
    // and execute draw
    pCanvas->drawList(itsDrawList);
  }
  changedPlotMode = False;

  Star *star;
  
  itsStarListIter->toStart();
  while ( !itsStarListIter->atEnd() ) {
    star = (Star *) itsStarListIter->getRight();
    star->draw(pCanvas);
    (*itsStarListIter)++;
  }
}


Double Galaxy::force(Double distance) 
{

  //return itsMass/(distance*distance+itsSize*itsSize);
  return itsMass/(distance+itsSize);
}


Double Galaxy::getMass() 
{
  return itsMass;
}

Vector<Double>& Galaxy::getPosition() 
{
  return itsPosition;
}

Vector<Double>& Galaxy::getVelocity() 
{
  return itsVelocity;
}

void Galaxy::setScale(Int xSize, Int ySize) 
{
  itsXSize = xSize;
  itsYSize = ySize;
  changedPlotMode = True;

  Star *star;
  
  itsStarListIter->toStart();
  while ( !itsStarListIter->atEnd() ) {
    star = (Star *) itsStarListIter->getRight();
    star->setScale(xSize, ySize);
    (*itsStarListIter)++;
  }
}

  

} //# NAMESPACE CASA - END

