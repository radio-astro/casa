//# Animator.h: movie control for one or more WorldCanvasHolders
//# Copyright (C) 1996,1997,1999,2000
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

#ifndef TRIALDISPLAY_ANIMATOR_H
#define TRIALDISPLAY_ANIMATOR_H

//# aips includes
#include <casa/aips.h>
#include <casa/Containers/List.h>

//# display library includes
#include <display/DisplayEvents/WCRefreshEH.h>

namespace casa { //# NAMESPACE CASA - BEGIN

//# forwards
	class WorldCanvasHolder;
	class Animator;

// <summary>
// WorldCanvas refresh event handler for Animator class.
// </summary>
//
// <synopsis>
// This class is a simple implementation of a WCRefreshEH which
// passes WorldCanvas refresh events on to an Animator object.
// </synopsis>

	class AnimatorRefEH : public WCRefreshEH {
	public:
		AnimatorRefEH(Animator *animator);
		virtual ~AnimatorRefEH() {};
		virtual void operator()(const WCRefreshEvent &ev);
	private:
		Animator *itsAnimator;
	};

// <summary>
// Animation controller for WorldCanvasHolders.
// </summary>
// <use visibility=export>
//
// <reviewed reviewer="" date="yyyy/mm/dd" tests="" demos="">
// </reviewed>
//
// <prerequisite>
//   <li> Attribute
//   <li> AttributeBuffer
//   <li> WorldCanvasHolder
//   <li> DisplayData
// </prerequisite>
//
// <etymology>
// An Animator animates animations
// </etymology>
//
// <synopsis>
//  TBW
// </synopsis>
// <example>
//
// First example is making a simple movie of all the
// channels in a data cube
//
// <srcblock>
// // Create a PixelCanvas for X11 to draw on
// X11PixelCanvas(parent, xpcctbl, width, height) ;
// // and a World Coordinate  interface for this PixelCanvas
// WorldCanvas  wCnvs(&pixelCnvs);
//
// // Create a WorldCanvasHolder
// WorldCanvasHolder wCnvsHldr(&wCnvs);
//
// // Create an ImageDisplayData object of the data cube to display the
// // channels (assuming 3rd axis is velocity in this cube)
// ImageDisplayData cube(myDataSet, 0, 1, 2);
// // and register this with the WorldCanvasHolder
// wCnvsHldr.addDisplayData(&cube);
//
// // Create an Animator for the WorldCanvasHolder and register the
// // WorldCanvasHolder
// Animator animator;
// animator.addWorldCanvasHolder(&wCnvsHldr);
//
// // start with first channel
// animator.setMin(0.0);
// // last channel of cube (assuming it has 30 channels)
// animator.setMax(29.0);
// // go in steps of one channel
// animator.setStep(1.0);
// // only one channel at a time, so tolerance must be less than 1.0
// anomator.setTolerance(0.1);
// // display new channel every 0.2 sec
// animator.setUpdateInterval(0.2);
// // we want a normal boring movie
// animator.setNextMode(animator::NEXT_FORWARD);
// // and we define the channel (ie index)
// animator.setMatchMode(Animator::MATCH_INDEX);
// // set the Update method
// animator.setUpdateMode(Animator::UPDATE_DIRECT);
// // and start the movie
// animator.startMovie();
//       .
//       .
//       .
// // After a while we want to change to Rock & Roll mode:
// animator.setMovieMode(Animator::NEXT_ROCKandROLL);
//       .
//       .
//       .
// // and after another while we stop
// animator.stopMovie();
// </srcblock>
// Second example is that we want to blink between channel 20 of the previous
// cube and an optical image
// <srcblock>
// // Create an ImageDataDisplay of the image and register with
// // the WorldCanvasHolder
// ImageDataDisplay image(myImage, 0, 2);
// wCnvsHldr.addDisplayData(&image);
//
// // Create a List to contain the AttributeBuffers
// List<void *> buffList;
// // and an iterator for this list
// ListIter<void *> it(&buffList);
//
// // Create AttributeBuffers and fill them
// AttributeBuffer atBuff1;
// // This is to mark the cube. The name and value of the Attribute or
// // Attributes that can be used is arbitrary, as long as the different
// // AttributeBuffers in the List select different datasets (even that is
// // not really necessary, but you will be blinking between the same frames
// // and not many people will get excited about that....)
// atBuff1.add("AjaxWordtKampioen", "yes");
// it.addRight((void *) &attBuff1);
//
// // the buffer for the image
// AttributeBuffer atBuff2;
// atBuff2.add("AjaxWordtKampioen", "of course");
// at.addRight((void *) &attBuff2);
//
// // and set this list on the Animator
// animator.setBlinkList(&buffList);
//
// // Set the same attributes on the DisplayDatas so the Attribute matching
// // mechanism between the WorldCanvasHolder and the DisplayDatas will select
// // the right data at the right time automatically
// cube.addAttribute(attBuff1);
// image.addAttribute(attBuff2);
//
// // Tell animator to use channel 20
// animator.gotoCoord(20.0);
// // The Animator should write "zIndex" to select channel 20
// animator.setMatchMode(Animator::MATCH_INDEX);
// // Because we set the UpdateMode to UPDATE_BLINK, the zIndex does
// // not change each time the timer goes off.
// animator.setUpdateMode(Animator::UPDATE_BLINK);
// // and the blinking can start:
// animator.startMovie();
//       .
//       .
//       .
// // After a while we want to blink between channel 19 and the optical image.
// // this is done by setting NextMode to NEXT_BACKWARD and invoke nextCoord().
// // Of course we could also have used only animator.prevCoord(), but just
// // to show the principle...
// animator.setNextMode(Animator::NEXT_BACKWARD);
// animator.nextCoord();

//
// // We can add a third data set, set "AjaxWordtKampioen" to "always" for that data,
// // and the blinking is between 3 datasets:
//
// // stop the blinking
// animator.stopMovie();
//
// // Create DisplayData from data
// ImageDataDisplay image2(mySecondImage, 0, 2);
//
// // Add to WorldCanvasHolder
// wCnvsHldr.addDisplayData(&image2);
//
// //Setup AttributeBuffer
// AttributeBuffer atBuff3;
// atBuff3.add("AjaxWordtKampioen", "always");
//
// // add to data
// image2.addAttribute(attBuff3);
//
// // and add to List
// at.addRight((void *) &attBuff3);
//
// // set the updated list on the animator (strictly speaking not neseccary
// // here, because animator already has the address of this List, but...)
// animator.setBlinkList(&buffList);
//
// //and go!!!
// animator.startMovie();
// </srcblock>
// The third example is to run movies, selected on world coordinates on two
// WorldCanvasHolders in synch:
// <srcblock>
// // Create a PixelCanvas for X11 to draw on
// X11PixelCanvas(parent, xpcctbl, width, height) ;
// // and two  WorldCanvases side by side on the same PixelCanvas
// WorldCanvas wCnvs1(&pixelCanvas, 0.0, 0.25, 0.5, 0.5);
// WorldCanvas wCnvs2(&pixelCanvas, 0.5, 0.25, 0.5, 0.5);
//
// // A WorldCanvasHolder for each WorldCanvas
// WorldCanvasHolder wCnvsHldr1(&wCnvs1);
// WorldCanvasHolder wCnvsHldr2(&wCnvs2);
//
// // We have an HI data cube of an object
// ImageDisplayData HIcube(HIdata, 0, 1, 2);
// // that we display on the first WorldCanvas
// wCnvsHldr1.addDisplayData(HIcube);
//
// // and we have a CO cube of the same object that we display on the second
// // WorldCanvas
// ImageDisplayData COcube(COdata, 0, 1, 2);
// wCnvsHldr2.addDisplayData(COcube);
//
// // Create an animator and register the two WorldCanvasHolders
// Animator animator;
// animator.addWorldCanvasHolder(wCnvsHldr1);
// animator.addWorldCanvasHolder(wCnvsHldr2);
//
// // Set the movie parameters:
// // Select on world coordinate (ie velocity)
// animator.setMatchMode(Animator::MATCH_WORLD);
// // start and end velocities
// animator.setMinAndMax(1200.0, 1400.0);
// // and the tolerance to 10.0 and the step to 20.0
// animator.setTolerance(10.0):
// animator.setStep(20.0);
//
// // Set speed of movies and the mode
// animator.setUpdateInterval(0.2);
// animator.setNextMode(Animator::ROCKandROLL);
//
// // Now, on the first canvas a movie will be played of the HI data, and on
// // the second canvas a movie of the CO data. Because the selection is done
// // on world coordinate, the two movies display the data of the same
// // velocity (within the tolerance of 10.0), in steps of 20.0, synchronized,
// // regardless of the channel separation of the two datasets:
// animator.startMovie();
//
// </srcblock>
// </example>
//
//
// <motivation> To allow for easy control of movies, possibly synchonous on
// more than one WorldCanvas, as well as lay the basis for the userinterface
// for this, a central class is needed that controls sequences of DisplayData.
// </motivation>
//
// <todo>
//   <li> make Animator know about Units
// </todo>

	class Animator {

	public:

		// Defines the way the Animator calculates the Z coordinate of the next
		// frame in the sequence. This defines the behaviour of the member
		// nextCoord().
		enum NextMode {
		    // Movie in forward direction. Step is added, if the Z coordinate is larger
		    // than maximum then display minimum, increment again until maximum, etc.
		    // This is the default.
		    NEXT_FORWARD,
		    // Movie in backward direction. Step is subtracted, if Z coordinate is
		    // smaller than mimnimum then display maximum, decrement until minimum,
		    // etc etc
		    NEXT_BACKWARD,
		    // Movie goes up and down the sequence. Step is added until the Z
		    // coordinate is larger than maximum, then step is subtracted until the Z
		    // coordinate is smaller than minimum, then step is added, and so on, and
		    // so on
		    NEXT_ROCKANDROLL
		};

		// Defines wheter the sequence is defined using the index in the sequence or
		// by the world coordinate of the 'Z-axis' (the "zValue" or "zIndex" used by the
		// WorldCanvasHolders and the DisplayDatas), or only by the AttributeBuffers
		enum MatchMode {
		    // Sequence is defined by writing Attribute "zIndex" with the value of the
		    // Z coordinate to WorldCanvasHolders, plus the ones from the List of
		    // AttributeBuffers.
		    // This is the default
		    MATCH_INDEX,
		    // Sequence is defined by writing Attribute "zValue"with the value of the
		    // Z coordinate to WorldCanvasHolders, plus the ones from the List of
		    // AttributeBuffers
		    MATCH_WORLD,
		    // Only the Attributes from the List of AttributeBuffers are written
		    MATCH_LIST_ONLY
		};

		// Decides whether the Z coordinate should be changed according to the
		// NextMode before each update or not
		enum UpdateMode {
		    // Do change the Z coordinate before each update. Use this for 'normal
		    // movies'.
		    // This is the default.
		    UPDATE_DIRECT,
		    // Do not change Z coordinate, but rely on the AttributeBuffers to change
		    // what is displayed. Use this for blinking.
		    UPDATE_BLINK
		};



		// Constructor
		Animator();

		//Destrutor
		virtual ~Animator();

		// Go to next Z coordinate in sequence
		virtual void nextCoord();

		// Go to previous Z coordinate in sequence
		virtual void prevCoord();

		// Go to  Z coordinate zCoord
		virtual void gotoCoord(Double zCoord);

		// Set increment in the Z coordinate for the movie
		// <group>
		virtual void setStep(uInt zIncrement);
		virtual void setStep(Double zIncrement);
		// </group>

		// Set the tolerance in the Z coordinate
		// <group>
		virtual void setTolerance(uInt tolerance);
		virtual void setTolerance(Double tolerance);
		// </group>

		// Set the minimum and maximum Z coordinate for the movie
		virtual void setMinAndMaxCoord(Double zMin, Double zMax);

		// Set whether "zIndex", "zValue" or none of the two should be written
		// additionally to the AttributeBuffers.
		virtual void setMatchMode(Animator::MatchMode match);


		// Set the way the increments are done, ie. it defines the action of
		// nextCoord() (options NEXT_FORWARD, NEXT_BACKWARD, UPDATE_ROCKANDROLL)
		virtual void setNextMode(Animator::NextMode  mode);


		// Set whether an increment should be done before each update (UPDATE_DIRECT or
		// UPDATE_BLINK)
		virtual void setUpdateMode(Animator::UpdateMode mode);

		// Set update interval of movie in milliseconds
		virtual void setUpdateInterval(Double interval);

		// Set the list of additional Attributes that the Animator  places on the
		// WorldCanvasHolders before each update.
		virtual void setBlinkRestrictions(List<void *> *attBuffers);

		// Remove the List with AttributeBuffers
		virtual void clearBlinkRestrictions();

		// Stop and start movie
		// <group>
		virtual void startMovie();
		virtual void stopMovie();
		// </group>

		// Return the length of the movie. In UpdateMode UPDATE_DIRECT this is the
		// number of frames that follow from the minimum and maximum Z coordiante
		// and the step. In UPDATE_BLINK mode this is the length of the List of
		// AttributeBuffers set on the Animator using setBlinkAttributes
		virtual uInt getMovieLength();

		// Return the current position in the movie.  This is really a bad
		// thing, but needed in the interim for the viewer.
		virtual Int getCurrentPosition();

		// Reset the Animator. This will set the minimum coordiante to 0, the
		// maximum to the number of elements registered with the WorldCanvasHolders,
		// the step to 1.0 and the update mode to MATCH_INDEX
		virtual void reset();

		// Add a WorldCanvasHolder to the list controlled by this Animator
		virtual void addWorldCanvasHolder(WorldCanvasHolder *newHolder);

		// Remove a WorldCanvasHolder from the list controlled by this Animator
		virtual void removeWorldCanvasHolder(WorldCanvasHolder& holder);

		// Refresh event handler - just used to see if resetCoordinates was
		// set.  If so, then we should partially reset the animator.
		virtual void operator()(const WCRefreshEvent& ev);

	private:

		// List of WorldCanvasHolders
		List<void *> holderList;

		// List of the AttributeBuffers
		List<void *> *attBufList;

		// parameters of movies
		Double minCoord;
		Double maxCoord;

		// Increment for computing currentCoord. Always postive
		Double movieStep;

		// State variables
		// Tolerance for coordinate Restriction
		Double coordTolerance;
		// Current Z coordiante
		Double currentCoord;
		// the MatchMode
		Animator::MatchMode matchMode;
		// the NextMode
		Animator::NextMode nextMode;
		// The UpdateMode
		Animator::UpdateMode updateMode;
		// and the interval of the timer
		Double updateInterval;

		// The number of the AttributeBuffer to use. Computed by computeNewCoord()
		// Has to be Int, not uInt! (because I sometimes subtract 1!)
		Int numberInList;

		// In which direction the movie is currently going
		Int movieDirection;

		// Compute, based on the UPDATE_MODE and NEXT_MODE, the Z coordinate of the
		// new frame to display (ie. the new currentCoord). If the updateMode ==
		// UPDATE_BLINK, nothing happens. If the updateMode == UPDATE_DIRECT, the
		// value of currentCoord is incremented or decremented with the absolute
		// value of movieStep, depending in whether the next_Mode is NEXT_FORWARD,
		// NEXT_BACKWARD or NEXT_ROCKANDROLL and whether the new value goes outside
		// the bounds set by minCoord and maxCoord.
		void computeNextCoord(Int addOrSubtract);

		// Helper routine for computeNewCoord(). If addOrSubtract > 0, 1 is added to
		// number, if addOrSubtract < 0, 1 is subtracted
		void increment(Int& number, Int addOrSubtract);

		// Helper routine for computeNewCoord(). The inverse of <src>increment(Int&,
		// Int)</src>
		void decrement(Int& number, Int addOrSubtract);

		// Helper routine for computeNewCoord(). If addOrSubtract > 0, movieStep is added to
		// number, if addOrSubtract < 0, movieStep is subtracted
		void increment(Double& number, Int addOrSubtract);

		// Helper routine for computeNewCoord(). The inverse of <src>increment(Double&,
		// Int)</src>
		void decrement(Double& number, Int addOrSubtract);


		// Write the necessary Attributes to all the WorldCanvasHolders. The content
		// of one of the registered AttributeBuffer is always written. If the
		// updateMode == UPDATE_DIRECT the n-th AttributeBuffer is written, where n
		// is the sequence number of the frame, based on minCoord and movieStep. If
		// the updateMode == UPDATE_BLINK the Animator just loops through the list
		// of AttributeBuffers based on numberInList.
		// If MatchMode == MATCH_INDEX also an Attribute is written with name
		// "zIndex" and with the value of currentCoord.
		// If MatchMode == MATCH_COORD also an Attribute is written with name
		// "zValue" and with the value of currentCoord.
		void writeRestrictions();

		// Invoke refresh() on all the WorldCanvasHolders registered with the
		// Animator
		void refresh();

		// Return the number of AttributeBuffers in the List of AttributeBuffer
		Int listLen();

		// refresh event handler
		AnimatorRefEH *itsAnimatorRefEH;

	};


} //# NAMESPACE CASA - END

#endif
