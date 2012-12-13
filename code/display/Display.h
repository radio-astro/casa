//# Display.h: Display module header file
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

#ifndef TRIALDISPLAY_DISPLAY_H
#define TRIALDISPLAY_DISPLAY_H

//#include <graphics/X11/X11Util.h>
#include <display/Display/DisplayEnums.h>
#include <display/Display/PixelCanvasColorTable.h>
#include <display/Display/PixelCanvas.h>
#include <display/Display/WorldCanvas.h>

// Go ahead and include X11 stuff for now
//#include <display/Display/X11PixelCanvasColorTable.h>
//#include <display/Display/X11PixelCanvas.h>

// Include Simple{World,Pixel}CanvasApp stuff
#include <display/Display/SimplePixelCanvasApp.h>
#include <display/Display/SimpleWorldCanvasApp.h>

namespace casa { //# NAMESPACE CASA - BEGIN

//
// <module>
//
// <summary>A module providing graphical display classes for use with AIPS++ </summary>
//
// <prerequisite>
// <li> <linkto class="Vector">Vector</linkto>
// <li> <linkto class="Matrix">Matrix</linkto>
// </prerequisite>
//
// <reviewed reviewer="None yet" date="yyyy/mm/dd" demos="">
// </reviewed>
//
// <etymology>
// Module provides classes for an Image Display Library
// </etymology>
//
// <synopsis> The purpose of the Display library is twofold: 1) to provide a
// set of tools for the AIPS++ programmer which make it easier to create
// drawing windows and use those drawing windows as output devices for
// graphical primitives, including images, and 2) to provide a set of higher-
// level classes to display datasets in a number of ways (images, contours, ...)
// as well as classes to help build an application that needs to display
// several datasets.
//
// A design goal is to have Display Library applications portable to
// another graphics system by augmenting the Library to provide an 
// interface to that graphics system.
//
// <h1><center><Display Library Contents></center></h1>
//
// <ol>
//  <li> <a href="#Overview">Display Library Overview</a>
//  <li> <a href="#PixelCanvas">PixelCanvas</a>
//   <ol>
//    <li> <a href="#PCDrawingCommands">Drawing Commands</a>
//    <li> <a href="#PCEvents">Event Handling</a>
//    <li> <a href="#PCCaching">Caching Mechanism</a>
//    <li> <a href="#PCColormaps">Colormap System</a>
//   </ol>
//  <li> <a href="#WorldCanvas">WorldCanvas</a>
//   <ol>
//    <li> <a href="#WCDrawingCommands">Drawing Commands</a>
//    <li> <a href="#WCEvents">Event Handling</a>
//    <li> <a href="#WCHandlers">Other Handlers</a>
//   </ol> 
//  <li> <a href="#WorldCanvasHolder">WorldCanvasHolder</a>
//  <li> <a href="#DisplayData">DisplayData</a>
//  <li> <a href="#Zoomer">Zoomer</a>
//  <li> <a href="#Animator">Animator</a>
// </ol>
//
// <hr>
//
// <h2><a name="Overview">Display Library Overview</a></h2>
// 
// The Display library is a set of tools for the AIPS++ programmer which make
// it easier to create drawing windows and use those drawing windows as output
// devices for graphical primitives, including images. To some extent, the
// Display Library consists of two parts. One part is a set of device
// indepentent classes that give the basic functionality for data display. The
// classes of the first group are quite generic and can basically be used for
// any application that needs an interactive display window and these classes
// not tied to AIPS++ data sets or AIPS++ coordinate systems. They make only
// very few assumptions for what they will be used.  The second part is more
// concerned with transforming data sets into objects that are understood by
// the first layer, as well as classes that should make it easier to build a
// full application that displays data. The classes of the second group can be
// quite data specific.
// 
// The main classes from the first group are the PixelCanvas, the WorldCanvas
// and the ColorMap classes. The PixelCanvas conceptually corresponds to the
// window on the screen. It works only in 'screen' units, ie. screen pixel
// coordiantes and screen data values. The PixelCanvas is, to some extent, the
// interface between Aips++ and the display hardware and graphics environment
// (e.g. X11). The WorldCanvas is mostly an interface layer between the 'real
// world' and the PixelCanvas. Its main responsibility is to transform drawing
// instruction that are specified in world coordinates and world values to the
// corresponding instructions of the PixelCanvas specified in pixel
// coordinates and pixel values. The PixelCanvas and the WorldCanvas are
// intended to be relatively 'raw' devices. With this we mean that both
// classes have basically no knowledge of the classes that use these Canvases
// to display data. In effect, The PixelCanvas does not know what a
// WorldCanvas is. The reason to make the canvases relatively ignorant, is
// that by putting in as few assumptions as possible in the canvases, they
// (hopefully) can be used in a very wide range of applications. For example,
// it is quite easy to make a 2D colormap editor (an application that is quite
// different from e.g. displaying a contour map) based on the PixelCanvas or
// on the WorldCanvas. To achieve that only a few assumption have to be built
// in in the canvases, the canvases communicate to other classes via event
// handlers that these other classes have to register with the canvases. Also
// the PixelCanvas communicates with the WorldCanvas thourg such event
// handlers. For example, if the window corresponding to the Pixelcanvas is
// resized by the user, this generates a refresh event and the Pixelcanvas
// calls the refresh event handlers registered with the PixelCanvas. When a
// WorldCanvas is created, the first thing the WorldCanvas does is to install
// a number of event handlers on the PixelCanvas. This means that if e.g. the
// refresh event occurs on the PixelCanvas, the WorldCanvas is notified and
// takes the necessary action. This scheme of communication through event
// handlers is used throughout the Display Library. The classes that generate
// events do not have to know what classes consume these events, the only
// thing that is defined is the interface (and content) of the event. This
// means that one could build a class on top of the PixelCanvas that is quite
// different from the WorldCanvas.
//
// To keep the WorldCanvas as generic as possible, the coordinate system the
// WorldCanvas uses to transform from screen pixels to world coordinates and
// back is not stored in the WorldCnvas but is also defined through an event
// handler (a CoordinateHandler). If the WorldCanvas has to do a coordinate
// transformation, it asks the CoordinateHandler to do this. In many cases,
// this CoordinateHandler will use a standard Aips++ CoordinateSystem to do
// the transformation, but this is not a requirement, and the programmer can
// implement any tranformation required, as long as it satisfies the interface
// of the CoordinateHandler.
//
// The features of the first group include
//
// <ul>
// <li> Multi-layer programming library (PixelCanvas, WorldCanvas, Application)
// <li> PixelCanvas Interface which abstracts the underlying graphics library.
// <li> Colormap system allows applications to function independent of the
//      number of available colors.
// <li> System color resources can be changed while running.
// <li> Drawing commands in world or pixel coordinates and values.
// <li> Events represented in world and pixel coordinates.
// </ul>
//
// The classes of the second group have a quite different purpose. They are
// much more concerned with building an application that needs to display one
// or more aips++ datasets in a number of ways. 
//
// To make the link between the relatively generic display classes
// (PixelCanvas, WorldCanvas etc) and classes quite specific for displaying
// Aips++ datasets, an intermediate class is required. This class is called
// the WorldCanvasHolder. The main role of the WorldCanvasHolder is to catch
// the events that occur on a WorldCanvas and distribute these events to the
// data that is being displayed on the Worldcanvas. Another important role of
// the WorldCanvasHolder is that it is the class that is used to control what
// is actually displayed on the WorldCanvas.  One important design requirement
// for the Display Library was that, from the programmer point of view, it
// should be easy to display more than one dataset in a window at the same
// time. A standard example is a contourmap on top of a greyscale image.
// Consequently, one can register more than one display object with the
// WorldCanvasHolder so that they are displayed at the same time. 
//
// One important aspect that the WorldCanvasHolder takes care of is the
// sizeControl of the WorldCanvas. If a refresh event happens on the
// WorldCanvas, before invoking the refresh event handlers installed on the
// WorldCanvas, the WorldCanvas invokes a sizeControl event. The meaning of
// this event is to ask the object that draws on the WorldCanvas (e.g. the
// WorldCanvasHolder) to check if the state of the WorldCanvas is ok so that
// it can be drawn on, and if it is not ok, that the state of the WorldCanvas
// be modified. For example, if an image is drwan as a pseudeocolor image and
// the window (or WorldCanvas) is much larger that the data array, one wants
// to expand the data array so that it fills more or less the window. One way
// of doing this is to do this by pixelreplication. But this means that there
// are requirements on the size of the area that the WorldCanvas uses for
// drawing the image (it has to be a integer multiple of the size of the data
// array. This explains the name sizeControl). It is the responsibility of the
// objects that draw on the WorldCanvas, to get these size right. What in
// practice happens is that the WorldCanvasHolder asks the display objects to
// sort this out and set the correct size on the WorldCanvas. Once this is
// sorted out, the WorldCanvas is in the coreect state and things can be
// drawn. The name sizeControl is not entirely correct. The obvious use of
// this mechanism is as in the example described above, but the problem is
// more general: before display objects can draw on the WorldCanvas they have
// to be sure that the WorldCanvas is in the correct state and possibly they
// have to modify the attributes of the WorldCanvas. Because more than one
// display object can draw on the WorldCanvas, this has to be done before the
// actual drawing occurs (because the display objects do not know if there are
// more than one display objects drawing on the WorldCanvas). 
//
// Another requirement for the Display Library was that no distinction should
// be made based on how data is displayed. For example, displaying an image as
// a pseudocolor image on the screen should, from the Display Library point of
// view, be the same operation as displaying that image as a contourmap.  This
// means that the display object is quite an 'abstract' class. Display Objects
// in the Display Library are called DisplayData (or better: are derived from
// DisplayData). The role of the DisplayData is, using a certain algorithm, to
// transform data into one or more draw instructions for the WorldCanvas. An
// example is ImageDisplayData that takes an Aips++ dataset and display its
// contents as images, Another example would be ContourDisplayData that draws
// images as contourmaps. Also here, the communication to the DisplayData goes
// via calling event handlers. Depending on what happens (e.g. a refresh
// event, or a position event), the WorldCanvasHolder (who catches these
// events from the WorldCanvas), invokes member functions of the DisplayData
// registered with the WorldCanvasHolder (so to a large extent, a DisplayData
// is an event handler on the WorldCanvasHolder). For example, if a refresh
// event happens, the WorldCanvasHolder will ask the DisplayData to draw what
// they should draw. If a programmer wants to implement a new way of
// displaying data, what this programmer has to do is to implement a new class
// derived from DisplayData that computes whatever it has to compute (say a
// volume rendering of a datacube) and transform this into draw instructions
// for the WorldCanvas (in this example, just call WorldCanvas.drawImage() on
// the result of the volume rendering). A class derived from DisplayData does
// not need to have an Aips++ dataset, but one could for example image a
// DisplayData class that reads positions from a catalog and plots these
// positions on the WorldCanvas. 
//
// Because everything in the Display Library is event driven, it is also easy
// to make applications that require more than one display window. The
// programmer only has to decide how to link the various windows by installng
// the appropriate event handlers on the various classes. An example of how
// this can be done is the Zoomer class. This class is a high-level class that
// catches position events on the WorldCanvases (possibly more than one) that
// are registered with the Zoomer. If the right event happens on one of the
// WorldCanvases (say, the user is rubberbanding a rectangle), the Zoomer sets
// the correct attributes on all the WorldCanvases registered with the Zoomer
// (meaning: it defines the area to which should be zoomed in) and invokes a
// refresh event on all these WorldCanvases. This means that all the
// WorldCanvases will zoom in synch. 
//
// Another design requirement of the Display Library was that it should be
// easy to display movies, and the DisplayData are build with sequences in
// mind (as is clear from their interface). The programmer is free to define
// what a sequence really means, but it is probably best to keep the structure
// of the sequence in a DisplayData logical to at least some extent. But there
// is no real requirement on the structure of the sequence.  Movies can be
// controlled using the Animator class. The easy way is to use indices, but
// there is a generic way of defining movies using restrictions (see later
// what these mean). So a sequence does not have to correspond to a 'physical'
// sequence in one datastructure (channels in a cube for example), but can be
// made of representations of different datasets (e.g. blinking), or more
// exotic combinations of different datasets displayed in different form
// (e.g. blink between a greyscale and a contourmap, if such a thing would be
// useful). This system is very flexible and there are no real limits to what
// a movie really means.
//
// An important concept in the Display Library is that of Attributes (and
// their use as restrictions). Attributes are name-value pairs. Many classes
// of the Display Library have a buffer where they can store these Attributes.
// They can be used for various things (see AttributeBuffer for a number of
// examples). One applications is that a uniform userinterface can be defined
// for changing attributes of a class (meaning internal members), but since
// Attributes can have an arbitrary name, they can be used to place (almost)
// any kind of information on classes <em> at run time</em>. This provides a
// mechanism of distributing information in a display aplication, while at
// compile time it is not yet defined what that information is (name, type,
// etc).
//
// An important application of Attributes is their use as
// <em>restrictions</em>: they are used to select which data is actually
// displayed. DisplayData classes, like ImageDisplayData, are supposed to have
// defined, for each distinct representation that they can draw, one or more
// Attributes, specifically defined to select what data is displayed. These
// Attributes are in a separate (or several separate) AttributeBuffer, called
// the <em>restriction buffer</em>. Also the WorldCanvasHolder has such a
// restriction buffer whose content should be controlled by the application
// programmer.  If a refresh happens, the WorldCanvasHolder, after the
// sizeControl step, asks each DisplayData that is registered with the
// WorldCanvasHolder to draw itself, by calling the refreshEH() member of each
// DisplayData. The first thing that a DisplayData should do it to see if the
// restrictions that are placed on the WorldCanvasHolder are compatible with
// this DisplayData and/or check with which element of the DisplayData the
// restriction buffer WorldCanvasHolder is compatible. If the DisplayData has
// compatible data, it should draw this data. An example may make this process
// clearer. An ImageDisplayData is a class derived from DisplayData that
// displays the 2D slices from a n-dimensional dataset on a canvas (for
// example the channels from a datacube). An ImageDisplayData has two general
// restrictions (meaning they apply to all channels): the names of the axes
// ("xAxisName" = "Ra" and "yAxisName" = "Dec"). Since an ImageDisplayData
// (possibly) consists of a number of channels, additional restrictions exist,
// but these have a different value for each element (ie. channel). In the
// case of an ImageDisplayData, each 2D subset has defined "zIndex" and
// "zValue", the first has the value of the pixelcoordinate of the 2D image
// (in our example channel number), the second has the value of the
// WorldCoordinate of centre of the 2D image (in our example the velocity of
// the channel). So to display a channel, a programmer has to set 3
// restrictions on the WorldCanvasHolder: "xAxisName", "yAxisName" to select
// channels from a datacube and set these to "Ra" and "Dec" (or whatever is in
// the header of the data), plus a restriction to select the actual channel,
// e.g. set "zIndex" to 20 to select channel 20, or "zValue" to 1200.0, to
// select the channel corresponding to velocity 1200.0. To display a position
// velocity image, one would have to specify e.g. "xAxisName" = "Dec" and
// "yAxisName" = "Velocity" and set "zValue" to 5.23443311 (the Ra of the
// slice you want to look at). 
//
// To determine if restrictionbuffers match, one can simply use the member
// matches() of an AttributeBuffer. The logic of matching restrictions is
// perhaps a bit distorted: restrictions (or Attributes in general) of
// different names <em> always match</em>. So if a DisplayData has a
// restriction called "A" and the WorldCanvasHolder specifies "B", the
// DisplayData should draw itself. Restrictions (and Attributes) can have some
// tolerance. In our example we could have specified "zValue" to be 1200.0,
// plus or minus 5.0. This obviously can be used e.g. to match the channels of
// two data cubes that are on a different velocity grid. The Animator class,
// who can be used to make movies, completely relies on this restriction
// mechanism. 
//
//
//
// <hr>
//
// <h2><a name="PixelCanvas">The PixelCanvas</a></h2>
//
// The PixelCanvas display library defines an interface to an underlying
// 2D graphics library.
//
// The design of the PixelCanvas emphasizes the following features:
//
// <ul>
// <li> Straightforward drawing interface using simple Aips++ array types
// <li> Enhanced event handling - Event classes can be derived from to
//      add extra functionality.
// <li> Advanced caching system - User can create sequences of drawing commands
//      for later recall.  Sequences can be compacted and stored into
//      native structures for increased drawing performance.
// <li> Ability to register and use several colormaps in the same window at
//      the same time.
// <li> Minimize the dependency on the underlying graphics system, in
//      this case, X11.
// </ul>
//
//
// <h3><a name="PCDrawingCommands">PixelCanvas Drawing Commands</a></h3>
//
// PixelCanvas drawing commands accept simple AIPS++ objects.  Presently the
// drawing commands accept points as Vector<t>'s, where t can be
// any scalar type but Bool.   Bool images are not supported because a Bool 
// cannot represent a color index.  Complex values are not supported because
// there are many ways of creating a single scalar value from complex
// values. 
//
//
// <h3><a name="PCEvents">PixelCanvas Event Handling</a></h3>
//
// There are 3 kinds of events the PixelCanvas reports
//
// <ol>
// <li> refresh event - sent when the canvas must be redrawn.
// <li> position event - sent when a button or key is pressed.
// <li> motion event - sent when the mouse moves over the cursor.
// </ol>
//
// Applications handle 
// <linkto class=PixelCanvas>PixelCanvas</linkto> events by creating 
// <linkto class=PixelCanvas>PixelCanvas</linkto> event handlers
// that must be derived from the appropriate event class, either
// <linkto class=PCRefreshEH>PCRefreshEH</linkto>, 
// <linkto class=PCPositionEH>PCPositionEH</linkto>, or
// <linkto class=PCMotionEH>PCMotionEH</linkto>.  The () operator
// must be overridden and implemented by responding to the
// information contained in the 
// <linkto class=PCRefreshEvent>PCRefreshEvent</linkto>, 
// <linkto class=PCPositionEvent>PCPositionEvent</linkto>, or
// <linkto class=PCMotionEvent>PCMotionEvent</linkto>, as appropriate.
// 
//
// <h3><a name="PCCaching">PixelCanvas Caching Mechanism</a></h3>
//
// There is a system for creating sequences of commands.  It works
// be turning on caching, performing drawing commands, then shutting
// it off.  An id is returned to the user to recall the stored 
// sequence.  This gives the user control over what is to be cached
// while still abstracting the business of caching.
// 
// The cache system improves drawing speed by storing drawing data
// in native library formats.
//
// Data stored in native formats means that, for the X11PixelCanvas, 
//
// <ul>
// <li> images are stored in terms of pixels as XImages, 
// <li> coordinate positions are stored how X likes them (upper left is 0,0), 
// <li> lines are stored as XLines or XSegments, as appropriate 
// <li> Points are stored as XPoints.
// <li> Pixmap Text is stored as drawing strings
// <li> Stroked Text is stored as XSegments or XLines
// </ul>
//
// It is the responsibility of the user to rebuild display lists
// when necessary.  Normally this means when the canvas changes
// size or colormap distribution (if colormaps are in use).
//
//
// <h3><a name="PCColormaps">PixelCanvas Colormap System</a></h3>
//
// The PixelCanvas colormap system design was one of the most difficult
// design problems we faced in building this system.  The design goals
// were as follows:
//
// <ol>
// <li> Wanted to be able to pretend we had multiple simultaneous
//      colormaps available in some display window.
// <li> Wanted to allow for but not demand that multiple windows
//      share the same hardware colormap.
// <li> Wanted to distribute the same colormap to different windows
//      potentially displaying on different terminals.
// <li> Wanted to allow for customizing the colormaps, and allow for
//      data-dependent colormaps.
// <li> Wanted to be able to resize the available colormap while
//      a Display library application was running.
// </ol>
// 
// The constraints above have driven us to the following scheme:
//
// <ol>
// <li> Create something called a 
// <linkto class="PixelCanvasColorTable">PixelCanvasColorTable</linkto>
// and let it define a table of available colors for one or more
// <linkto class="PixelCanvas">PixelCanvas</linkto>es and be responsible
// for interfacing to the underlying graphics system's color allocation
// facilities.  Through the 
// <linkto class="PixelCanvasColorTable">PixelCanvasColorTable</linkto>
// one can abstract away the underlying graphic library's tedious and
// cumbersome constructs and functions required to manage color resources
// and visuals and provide a high-level interface based on concepts
// that are better understood.  This interface is implemented as the
// constructors for the classes derived from the
// <linkto class="PixelCanvasColorTable">PixelCanvasColorTable</linkto>
// class.
//
// <li> Define a <linkto class=Colormap>Colormap</linkto> to mean not a 
// table of colors, but rather a function that takes a floating-point
// parameter in [0.0,1.0] and returns an RGB triple.  This function can
// then be used to fill some arbitrary number of consecutive colorcells.
// Allow for <linkto class=Colormap>Colormap</linkto>s to be of a fixed
// size if needed (then they are called 'rigid')
// and design them for derivation so custom colormaps can be created
// and used in this system.
//
// <li> Define a <linkto class=ColormapManager>ColormapManager</linkto> to be
// responsible for dynamically partitioning its
// <linkto class="PixelCanvasColorTable">PixelCanvasColorTable</linkto>
// into chunks, one chunk for each registered 
// <linkto class=Colormap>Colormap</linkto>, allowing for rigid
// <linkto class=Colormap>Colormap</linkto>s (that must be for some reason
// of a particular size) and weights to give greater color resolution
// to certain maps.  Make it responsible for filling in the color based
// on its distribution of <linkto class=Colormap>Colormap</linkto>s
// in the <linkto class="PixelCanvasColorTable">PixelCanvasColorTable</linkto>.
//
// <li> Allow for <linkto class="PixelCanvasColorTable">PixelCanvasColorTable</linkto>s
// to be shared by more than one <linkto class="PixelCanvas">PixelCanvas</linkto>,
// and allow <linkto class="PixelCanvas">PixelCanvas</linkto>es to share
// <linkto class=Colormap>Colormap</linkto>s. 
//
// <li> Try to abstract away all this mess by placing gateways to necessary
// functionality in the <linkto class="PixelCanvas">PixelCanvas</linkto>
// itself.  For example images are drawn on the
// <linkto class="PixelCanvas">PixelCanvas</linkto> through the following
// process:
//
// <ol>
// <li> register a <linkto class=Colormap>Colormap</linkto> with the
// <linkto class="PixelCanvas">PixelCanvas</linkto> (needed only once).
// <li> set the active colormap to the one used in step 1.
// <li> create an image whose values range from 0 to N-1, where N is the size
//      of the active colormap 
//     (<linkto class="PixelCanvas">PixelCanvas</linkto>::getColormapSize()).
// <li> call <linkto class="PixelCanvas">PixelCanvas</linkto>::mapToColor().
// to convert to color values.
// <li> call <linkto class="PixelCanvas">PixelCanvas</linkto>drawImage() 
// with the Matrix of color values
// </ol>
//
// The color image can be reused as long as the distribution of colors
// on the
// <linkto class="PixelCanvasColorTable">PixelCanvasColorTable</linkto>
// doesn't change.  Any such change will trigger a refresh event with the
// reason Display::ColorTableChange on the affected
// <linkto class="PixelCanvas">PixelCanvas</linkto>. 
// This process can also be used to draw other primitives that each have
// a value associated with it.
// Several colormaps can be switched between by setting the active
// colormap and querying the range, and using the mapToColor function.
//
// <li> Implement a function that resizes the 
// <linkto class="PixelCanvasColorTable">PixelCanvasColorTable</linkto>,
// forcing a redistribution of colormaps on affected
// <linkto class="PixelCanvas">PixelCanvas</linkto>es followed by a
// refresh event.  This ultimately causes a reallocation of writable
// color cells on the underlying graphics library and can be
// used to increase or decrease the number of color cells available to
// the <linkto class="PixelCanvasColorTable">PixelCanvasColorTable</linkto>
// and hence to the application.  This mechanism can be used to recover
// allocated color cells from the system map released by other applications
// when they exit, or to balance color resources between two or more
// applications.
//
// </ol>
//
// <hr>
//
// <h2><a name="WorldCanvas">The WorldCanvas</a></h2>
//
// The <linkto class=WorldCanvas>WorldCanvas</linkto> is intended to
// serve as a world-coordinate plotting canvas
//
//
// <h3><a name="WCDrawingCommands">WorldCanvas DrawingCommands</a></h3>
//
// The <linkto class=WorldCanvas>WorldCanvas</linkto> drawing commands
// expect to be given world coordinate values for position information.
//
//
// <h3><a name="WCEvents">WorldCanvas Event Handling</a></h3>
//
// The <linkto class=WorldCanvas>WorldCanvas</linkto> event handling
// is similar to the <linkto class=PixelCanvas>PixelCanvas</linkto>
// event handling, but the <linkto class=WorldCanvas>WorldCanvas</linkto>
// events contain extra information which includes the world coordinate
// and linear coordinate position of the mouse pointer.  The same
// basic three events are available:
//
// <ol>
// <li> refresh event - sent when the canvas must be redrawn.
// <li> position event - sent when a button or key is pressed.
// <li> motion event - sent when the mouse moves over the cursor.
// </ol>
//
// Applications handle 
// <linkto class=WorldCanvas>WorldCanvas</linkto> events by creating 
// <linkto class=WorldCanvas>WorldCanvas</linkto> event handlers
// that must be derived from the appropriate event class, either
// <linkto class=WCRefreshEH>WCRefreshEH</linkto>, 
// <linkto class=WCPositionEH>WCPositionEH</linkto>, or
// <linkto class=WCMotionEH>WCMotionEH</linkto>.  The () operator
// must be overridden and implemented by responding to the
// information contained in the 
// <linkto class=WCRefreshEvent>WCRefreshEvent</linkto>, 
// <linkto class=WCPositionEvent>WCPositionEvent</linkto>, or
// <linkto class=WCMotionEvent>WCMotionEvent</linkto>, as appropriate.
// 
//
// <h3><a name="WCHandlers">Other WorldCanvas Handlers</a></h3>
//
// Other handlers can be registered with the WorldCanvas to customize:
// <ul>
// <li> World Coordinate transformations
// <li> Linear resampling algorithm
// <li> Data scaling methods
// <li> Size control function
// </ul>
//
//
// <h2><a name="WorldCanvasHolder">The WorldCanvasHolder</a></h2>
//
//  The WorldCanvasHolder is a user of a WorldCanvas. It installs handlers on
//  the WorldCanvas for each event a WorldCanvas can generate. The main role
//  of the WorldCanvasHolder is to allow to have more than one DisplayData
//  object draw on a WorldCanvas (e.g contours on top of an image). A number
//  of DisplayDatas can be registered with a WorldCanvasHolder, and the
//  WorldCanvasHolder passes WorldCanvas events to these DisplayData.
//  To control what is displayed on a WorldCanvas, the programmer can put
//  restrictions on a WorldCanvasHolder. Only those DisplayData do actually
//  draw whose restrictions match those of the WorldCanvasHolder. See the
//  example given above, or have a look at the doc of the Animator. 
//
// <h2><a name="DisplayData">The DisplayData Interface</a></h2>
//
// A DisplayData is the class that transforms data (in whatever form: an
// image, a cube, a catalog or whatever) into drawing instructions for the
// WorldCanvas. A DisplayData is the 'workhorse' of the display libary: here
// is defined in what way on can represent data. If one want to add a new way
// of displaying data to aips++, they only thing that has to be done is to
// write a new class, derived from DisplayData, that computes this new
// representation and draws it. For example, if one would want to add volume
// rendering to aips++, one would have to write a class that computes this
// volume rendering (e.g. using some hot-gas algorithm), and draw the result
// of this on a WorldCanvas (ie. simply WorldCanvas.drawImage() on the
// result). DisplayDatas are registered witha WorldCanvasHolder and the
// restriction mechanism is used to select what is being
// displayed. DisplayDatas are also responsible for defining the
// WorldCoordinate system of the WorldCanvas (they have an interface that can
// be called (indirectly) by the WorldCanvas to do the transformations), as
// wel las they have to insure that the state of the WorldCanvas is ok (the
// sizeControl, see above). An example of a DisplayData is the
// ImageDisplayData class, that draws pseudocolor images from a data set.
//
// <h2><a name="Zoomer">The Zoomer</a></h2>
//
// Zooming is done by setying the linear coordinates of the WorldCanvas to
// define the zoom area (using a set function of the WorldCanvas) and force a
// refresh of the WorldCanvas. The Zoomer class defines the userinterface for
// zooming. One can register one (or more) WorldCanvas with a Zoomer. This
// Zoomer installs  a position event handler on the WorldCanvas that listens
// to certain key- and mouse events. The Zoomer defines a default
// userinterface, but this can be re-defined. A Zoomer can also handle more
// than one WorldCanvas, so it is easy to let different WorldCanvases zoom in synch.
//
// <h2><a name="Animator">The Animator</a></h2>
//
// The role of the Animator is to give an easy way of controling what is
// displayed on one or more WorldCanvases. By specifying how an Animator
// controls what is displayed (by using indices, world coordinates or
// restrictions), the Animator sets the necessary restrictions on the
// WorldCanvasHolders that are registered with the Animator, and forces a
// refresh on all of them. All WorldCanvasHolder registered on an Animator
// move in synch. 
//
//
//

// <hr>
//
// <motivation>
// Need to provide some usefull tools to assist in developing graphical
// C++ applications, addressing the problems encountered in trying to
// use other graphical libraries.
// </motivation>
//
// <hr>
// This section is intended as information about what tasks remain to be
// accomplished for the Display library.
// 
// The following list illustrates work remaining at the PixelCanvas level:
//
// <todo asof="1997/10/06">
// <li> <b>PixelCanvas</b>: change/add PixelCanvas interface to use Matrices for multiple
//      vector graphic primitives, and use those interfaces at the
//      WorldCanvas level.
// <li> compact sequences of drawPoint and drawLine into drawPoints and
//      drawLines, respectively.
// <li> add circle, arc, rounded rectangle drawing primitives.
// <li> implement rotated text perhaps at this level or maybe at the WorldCanvas level
// <li> More testing of the drawing primitives.
// <li> Correct bug related to clipwindow location when XCS (X coordinate syste) y
//      value is less than zero.
// <li> Check function visibility for X11PixelCanvasColorTable
// <li> WorldCanvas Coordinate routines.
// <li> Check all HTML docs for problems.
// </todo>
//
// The following is the todo list for the WorldCanvas and related classes
//
// <todo asof="1997/10/06">
//
// <li> mapToColor and mapToColor3 functions that redirect to pixelCanvas
// <li> decide what drawImage interface higher classes require, and implement
//      those in terms of the PixelCanvas interface.
// <li> test multiple WorldCanvases on a single PixelCanvas.
// <li> WCSplineResampleHandler class to handle bicubic interpolation of arrays ?
// <li> WCFunctionalScaleHandler class to take a functional as a scale function ?
// </todo>
//
// The following is the todo list for the application level classes:
// <todo asof="1997/10/06">
// <li> Write simple display app with colormap editor.
// <li> Demonstrate use of RonAndRenzo colormap implementation.
// </todo>
//
// <hr>
//
// <b>The following section is intended for a programmer who wants to 
// improve the Display Library</b>
//
// <h3>Caching System Optimization</h3>
//
// An advanced optimization to the 
// <linkto class="PixelCanvas">PixelCanvas</linkto>
// caching system would be to compact sequences 
// of drawing commands.  This would mean that a sequence received
// by the PixelCanvas would be transformed into a more efficient sequence
// of drawing commands that produces the same result.  Obviously, sequences 
// of the same commands can often be combined into a single command.  
// But consider for example a drawn raster image I followed by a set of
// vectors V on  top of the image.  This sequence can be represented by 
// { I V }.
//
// We can partition V into 2 sets: Vin (contains vectors drawn inside the 
// image) and Vout (vectors that have portions drawn outside the image). 
// A new image I' can be created from I by drawing I onto a pixmap, then
// painting the vectors Vin on top, and storing the resultant image I'.
//
// The cached sequence is then { I' Vout }, and I and V can be discarded  
// This will result in faster drawing times and reduced memory usage if 
// Vin has significant size because the vectors in Vin are not drawn explicitly.
//
// Similar results can be achieved with drawn text and points, so that some
// sequence { I V P T } can be replaced with { I' Vout Pout Tout }.  Also
// overlapping images can be stored to avoid drawing the overlapping region
// more than once.
//
// It is not clear whether this would be worthwhile to pursue, so it has not
// been implemented.
//
// </synopsis>
//
// </module>
//


} //# NAMESPACE CASA - END

#endif

// ---- End of module Display ----
