//# MeasurementEquations.h: The MeasurementEquations module
//# Copyright (C) 1996,1997,1999
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
//# Correspondence concerning AIPS++ should be adressed as follows:
//#        Internet email: aips2-request@nrao.edu.
//#        Postal address: AIPS++ Project Office
//#                        National Radio Astronomy Observatory
//#                        520 Edgemont Road
//#                        Charlottesville, VA 22903-2475 USA
//#
//#
//# $Id$

#ifndef SYNTHESIS_MEASUREMENTEQUATIONS_H
#define SYNTHESIS_MEASUREMENTEQUATIONS_H

#include <synthesis/MeasurementEquations/VisEquation.h>
#include <synthesis/MeasurementEquations/SkyEquation.h>
#include <msvis/MSVis/VisibilityIterator.h>
#include <msvis/MSVis/VisSet.h>

namespace casa { //# NAMESPACE CASA - BEGIN

//<module>

// <summary> 
// MeasurementEquations express how synthesis measurements are made
// </summary>

// <use visibility=export>

// <reviewed reviewer="" date="" tests="" demos="">

// <prerequisite>
//   <li> <linkto module="MeasurementSets">MeasurementSets</linkto>
// </prerequisite>
//
// <etymology>
// Measurement Equation describes a model for measurements from a
// generic radio telescope. 
// </etymology>
//
// <synopsis> 
// Synthesis processing in AIPS++ is implemented using the
// measurement equation due to Hamaker, Bregman and Sault and later
// extended by Noordam, and Cornwell.  The HBS measurement equation
// expresses the measured visibility in terms of Jones matrices and
// other components. A Jones matrix is a two by two complex matrix
// that describes how two orthogonal senses of polarization are
// affected by an element in a measurement system. In the HBS
// formulation of synthesis processing, the antenna is described by a
// compound Jones matrix formed from a product each element of which
// represents a different physical effect:
//
// J = G D C E P T F K
//
// The measured visibility from a radio telescope is then 
// given by:
//
// V_i,j=X_i,j (M_i,j integral directProduct(J_i, Conjugate(J_j)) S I + A_i,j)
//
// where the elements in the equation are
//
// <ul>
// <li> M is the 4 by 4 complex closure error matrix
// <li> K is the Fourier phase Factor Jones matrix
// <li> G is the antenna gain Jones matrix
// <li> D is the polarization leakage Jones matrix
// <li> C is the configuration Jones matrix
// <li> E is the electric field pattern Jones matrix
// <li> P is the receptor position angle Jones matrix
// <li> T is the atmospheric gain Jones matrix
// <li> F is the Faraday rotation Jones matrix
// <li> A is the complex additive component
// <li> X is the non-linear correlator function
// <li> S is the Stokes conversion matrix
// <li> I is the (real) sky brightness 4-vector
// </ul>
//
// and the integral is over time, frequency, sky position.  The direct
// product of two 2 by 2 matrices gives a 4 by 4 matrix in which every
// possible product of the 2 by 2 matrix elements appears.
//
// Manipulation of the equation in this form is possible but is much
// too expensive for most uses so we break it down into two parts and
// also use a special machine for the Fourier summation. This loses
// some generality but makes the use of the HBS measurement equation
// feasible.
//
// The split is such that the class
// <linkto class="VisEquation ">VisEquation </linkto>
// expresses the visibility-plane part of the ME:
//
// J = G D C P 
//
// where the visibility is
// V_i,j=X_i,j (M_i,j integral directProduct(J_i, Conjugate(J_j)) Vsky_i,j 
// + A_i,j)
// and the integral is over time, frequency.
//
// and the class <linkto class="SkyEquation ">SkyEquation </linkto>
// expresses the sky-plane part of the ME:
//
// J = E T F K
//
// Vsky_i,j=integral directProduct(J_i, Conjugate(J_j)) S I and the
// integral is over time, frequency, sky position.
// 
// The last integral (over K) amounts to a Fourier transform and so
// the SkyEquation is given FT machines to use for this purpose. Note
// that we have chosen to move the position of the parallactic angle
// term. This has been done for expediency but will lead to some
// difficulties in difficult cases.
//
// The terms G, D, C, etc, are represented by the classes in the
// module <linkto
// module="MeasurementComponents">MeasurementComponents</linkto>.
// This classes can typically do two basic things: apply a correction
// to a <linkto class="VisBuffer">VisBuffer</linkto> (which is a
// holder of a chunk of visibility data), and solve for its own
// internal parameters. Solution is accomplished using gradients of
// chi-squared obtained via standard services of the
// MeasurementEquation.
//
// The SkyBrightness is modelled by a special type of
// MeasurementComponent called a <linkto class="SkyModel">SkyModel</linkto>.  This has an interface to the
// SkyEquation via a <linkto class="PagedImage">PagedImage</linkto>.
//
// Another type of <linkto module="MeasurementComponents">MeasurementComponent</linkto> is the
// Fourier transform machine <linkto
// class="FTMachine">FTMachine</linkto> which is used for performing
// forward and inverse Fourier transforms. The class <linkto
// class="GridFT">GridFT</linkto> implements a straightforward grid
// and degrid FFT-based Fourier transform.  We anticipate that other
// FTMachines will be needed for e.g.  wide-field imaging.
//
// Visibility Data is held in a 
// <linkto module="MeasurementSets">MeasurementSet</linkto>.
// To expedite processing, we use a 
// <linkto class="VisibilityIterator">VisibilityIterator</linkto>
// (found in msvis/)
// to iterate through the MeasurementSet as needed. Setting up the
// iterator is relatively expensive so we store the iterator
// in a <linkto class="VisSet">VisSet</linkto> (also found in this
// module). Thus one should
// construct a <linkto class="VisSet">VisSet</linkto> and then
// use the iterator method to retrieve the iterator. Once
// one has a VisibilityIterator, it can be used to access the
// actual visibility data in chunk by using the 
// <linkto class="VisBuffer">VisBuffer</linkto> (also in this
// module).
// This scheme may seem baroque but it is needed to cut down on
// superfluous otherhead of various types.
// </synopsis> 
//
// <example>
// <srcblock>
//      // Create a VisSet from a MeasurementSet on disk
//      VisSet vs("3c84.MS");
//
//      // Now make an FTMachine with a 2048 by 2048
//      // complex pixel cache of 16 by 16 tiles,
//      // using Spheriodal Function gridding
//      GridFT ft(2048*2048, 16, "SF")
//
//      // Create an ImageSkyModel from an image on disk
//      PagedImage<Float> im("3c84.modelImage"));
//      ImageSkyModel ism(im);
//
//      // For the imaging, we need a SkyEquation and
//      // an FTMachine
//      SkyEquation se(vs, ft);
//
//      // Predict the visibility set for the model
//      se.predict();
//
//      // Make a VisEquation
//      VisEquation ve(vs);
//
//      // Solve for calibration of G matrix every 5 minutes
//      GJones gj(vs, 5*60);
//      ve.solve(gj);
//
//      // Solve for calibration of D matrix every 12 hours
//      DJones dj(vs, 12*60*60);
//      ve.solve(dj);
//
//      // Now use the SkyEquaton to make a Clean Image
//      HogbomCleanImageSkyModel csm(ism);
//      if (se.solve(csm)) {
//        Image<StokesVector> cleanImage=csm.getImage();
//        cleanImage.setName("3c84.cleanImage");
//      }
//
// </srcblock>
// </example>
//
// <motivation>
// MeasurementEquations are needed to encapsulate the measurement
// equations for linear systems, and both single dish and synthesis 
// observations. The idea is
// that the structure of many calibration and imaging algorithms is much
// the same for many different types of telescope.  MeasurementEquations
// are part of a framework of classes that are designed for synthesis and
// single dish imaging. The others are the <linkto
// module=MeasurementComponents>MeasurementComponents</linkto>.
//</motivation>
//
// <todo asof="">
// <li> Implement SkyJones
// <li> Multiple images in SkyModel/SkyEquation
// <li> Derive special SkyEquation for SD
// <li> Move LinearEquation and LinearModel into separate modules
// </todo>

// </module>

} //# NAMESPACE CASA - END

#endif
