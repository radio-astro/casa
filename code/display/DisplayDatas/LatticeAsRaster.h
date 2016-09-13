//# LatticeAsRaster.h: Class to display lattice objects as rastered images
//# Copyright (C) 1996,1997,1998,1999,2000,2001,2002
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

#ifndef TRIALDISPLAY_LATTICEASRASTER_H
#define TRIALDISPLAY_LATTICEASRASTER_H

#include <casa/aips.h>
#include <images/Images/ImageInterface.h>
#include <casa/Arrays/Array.h>
#include <display/DisplayDatas/LatticePADD.h>

namespace casa { //# NAMESPACE CASA - BEGIN

	template <class T> class Array;
	class IPosition;
	class Record;
	template <class T> class LatticePADMRaster;
	class WCPowerScaleHandler;

// <summary>Class to manage the drawing of raster images of slices from AIPS++ Lattices</summary>
//
// <use visibility=export>
//
// <reviewed reviewer="" date="yyyy/mm/dd" tests="" demos="">
// </reviewed>
//
// <prerequisite>
// <li> <linkto class="LatticePADisplayData">LatticePADisplayData</linkto>
// <li> <linkto class="DisplayData">DisplayData</linkto>
// </prerequisite>
//
// <etymology>
// The purpose of this class is to draw "raster" images of data that
// is "lattice"-based.
// </etymology>
//
// <synopsis>
// This class should be used to display raster images--- ie. filled,
// pseudo-color pixel images where each screen (or device) pixel is
// shaded with a color from a look-up table that is assigned to be
// proportional in some way to the intensity of the data pixel---of
// two-dimensional slices of data extracted from AIPS++ Lattices or
// Arrays having two or more dimensions.  Thus, this is the class to
// use to display standard channel maps of position-velocity cubes, or
// Digitized Sky Survey images, for example.
//
// At construction, any axes in the data can be mapped to the X and Y
// axes of the display device (see the <linkto
// class="PrincipalAxesDD">PrincipalAxesDD</linkto> class).  For data
// with more than two dimensions, a third axis in the dataset can be
// selected for generating a sequence of images along: this is known
// as the "movie" axis.  Animation (see the <linkto
// class="Animator">Animator</linkto> class) will cause different
// slices of the data to be selected from along this axis.  After
// construction, the axis settings of a LatticeAsRaster object can be
// modified at a later stage.
//
// The LatticeAsRaster object supports a number of options which can
// be set or retrieved using the setOptions and getOptions functions.
// These functions simply accept a Record, which can be converted from
// a GlishRecord: this is done in the <linkto
// class="GTkDisplayData">GTkDisplayData</linkto> class.  The options
// for the LatticeAsRaster class are:
// <li> range: a Vector<Double> of two elements, being the minimum and
// maximum data values to map to the minimum and maximum color values.
// <li> power: a Float which specifies the deviation from linear of
// the scaling of the data onto the colormap.  Increasingly negative
// values give increasingly severe logarithmic scaling: this scaling
// effectively allocates more color cells to the lowest data values
// (usually corresponding to the thermal noise and low surface
// brightness emission) in the image.  Increasing positive power
// values give increasingly severe exponential scaling, thereby
// allocating more and more color cells to the highest data values
// (ie. signal and interference)in the image.
// <li> resample: a String which is either "nearest" for nearest
// neighbour pixel resampling, or "bilinear" for bilinear
// interpolation between adjacent groups of four pixels.
// <li> complexmode: this is a String, and is only relevant for
// LatticeAsRaster<Complex> or LatticeAsRaster<DComplex>
// instantantiations.  One of "phase", "real", "imaginary" or
// "magnitude" is appropriate for this option, and indicates how
// complex data values should be translated to real pixel values.
//
// LatticeAsRaster is templated, and can be used to draw Complex or Real
// Images or Arrays.  For Complex data, the default complexmode is
// "magnitude."  LatticeAsRaster objects can be registered on any
// WorldCanvasHolder, but will only draw on WorldCanvases having an
// attribute value of Display::Index for "colormodel."
// </synopsis>
//
// <example>
// A LatticeAsRaster object could be constructed and used as follows:
// <srcblock>
// PagedImage<Float> *pimage = new PagedImage<Float>(String("test.im"));
// DisplayData *dd;
// uInt ndim = pimage->ndim();
// if (ndim < 2) {
//   throw(AipsError(String("Image has less than two dimensions")));
// } else if (ndim == 2) {
//   dd = (DisplayData *)(new LatticeAsRaster<Float>(pimage, 0, 1));
// } else {
//   IPosition fixedPos(ndim);
//   fixedPos = 0;
//   dd = (DisplayData *)(new LatticeAsRaster<Float>(pimage, 0, 1, 2,
//                                                   fixedPos));
// }
// // wcHolder is an existing WorldCanvasHolder *...
// wcHolder->addDisplayData(ddata);
// wcHolder->refresh();
// </srcblock>
// </example>
//
// <motivation>
// Displaying 2-dimensional slices of a lattice-based data volume is
// a standard display requirement for astronomical data visualization
// and presentation.
// </motivation>
//
// <templating arg=T>
// </templating>
//
// <thrown>
// </thrown>
//
// <todo asof="yyyy/mm/dd">
// </todo>

	template <class T> class LatticeAsRaster : public LatticePADisplayData<T> {

	public:

		// Array-based constructors: >2d and 2d.  xAxis and yAxis specify
		// which axis in the array (0-based) should be mapped to X and Y
		// on the display device: ie. 2-d slices of the data to be displayed
		// have these axes.  mAxis specifies the "movie" axis, which is the axis
		// along which different slices are taken.  fixedPos is an IPosition
		// having the same length as the number of dimensions in the array,
		// and indicate the fixed axis values for axes in the data that are
		// not specified as xAxis, yAxis or mAxis.
		// <group>
		LatticeAsRaster(Array<T> *array, const uInt xAxis,
		                const uInt yAxis, const uInt mAxis,
		                const IPosition fixedPos);
		LatticeAsRaster(Array<T> *array, const uInt xAxis,
		                const uInt yAxis);
		// </group>

		// Image-based constructors: >2d and 2d.  xAxis and yAxis specify
		// which axis in the image (0-based) should be mapped to X and Y
		// on the display device: ie. 2-d slices of the data to be displayed
		// have these axes.  mAxis specifies the "movie" axis, which is the axis
		// along which different slices are taken.  fixedPos is an IPosition
		// having the same length as the number of dimensions in the image,
		// and indicate the fixed axis values for axes in the data that are
		// not specified as xAxis, yAxis or mAxis.
		// <group>
		LatticeAsRaster( SHARED_PTR<ImageInterface<T> > image, const uInt xAxis, const uInt yAxis, const uInt mAxis, const IPosition fixedPos, viewer::StatusSink *sink=0 );
		LatticeAsRaster(SHARED_PTR<ImageInterface<T> > image, const uInt xAxis,
		                const uInt yAxis);
		// </group>

		// Destructor
		virtual ~LatticeAsRaster();

		// Create the various elements in the sequence of displayable
		// images.  This is called upon construction as well as whenever
		// the display and/or movie axes are changed via a call to
		// PrincipalAxesDD::setAxes.
		//virtual void setupElements(IPosition fixedPos = IPosition(uInt(2)));
		virtual void setupElements();

		// Install the default options for display.
		virtual void setDefaultOptions();

		// Apply the options stored in the provided Record to the
		// LatticeAsRaster object.  If the return value is True, then
		// some options have changed, and a refresh is needed to update
		// the display.
		virtual Bool setOptions(Record &rec, Record &recOut);

		// Retrieve the currently set options, and their types, default
		// values, and any help text associated with each parameter.  This
		// information can be used to generate form-type graphical user
		// interfaces or command-line interfaces to set the options with
		// prompts.
		virtual Record getOptions( bool scrub=false ) const;
		//static Record getGlobalOption();

		// Return the DisplayData type; used by the WorldCanvasHolder to
		// determine the order of drawing.
		virtual Display::DisplayDataType classType() {
			return Display::Raster;
		}

		// class name
		virtual String className() const {
			return String("LatticeAsRaster");
		}
		const static String HISTOGRAM_RANGE;
		const static String COLOR_MODE;

	private:

		// Storage for the data range parameters
		Vector<Float> itsOptionsDataRange;
		Vector<Float> itsOptionsDataDefault;

		LatticeAsRaster<T>* rasterRed;
		LatticeAsRaster<T>* rasterGreen;
		LatticeAsRaster<T>* rasterBlue;

		//Float itsOptionsPower;

		// what colormode the raster should be drawn in, options are
		// "colormap" for normal pseudo-color rasters, and "red", "green"
		// and "blue" for RGB canvases, and "hue", "saturation" and "value"
		// for HSV canvases.
		String itsOptionsColorMode;

		// pointers to scale and resampling handlers
		WCPowerScaleHandler *itsPowerScaleHandler;

		// allow the corresponding DisplayMethod to access this' private data.
		friend class LatticePADMRaster<T>;
		LatticeAsRaster<T>* getRasterRed();
		LatticeAsRaster<T>* getRasterGreen();
		LatticeAsRaster<T>* getRasterBlue();
		virtual void setDisplayDataRed( DisplayData* dd );
		virtual void setDisplayDataBlue( DisplayData* dd );
		virtual void setDisplayDataGreen( DisplayData* dd );
		void initializeDataMatrix( int index,
					Matrix<T>& datMatrix, Matrix<Bool>& mask, const IPosition& start,
					const IPosition& sliceShape, const IPosition& stride );

		//static bool globalColors;

		//# Make parent members known.
	public:
		using LatticePADisplayData<T>::nelements;
		using LatticePADisplayData<T>::nPixelAxes;
		using LatticePADisplayData<T>::fixedPosition;
		using LatticePADisplayData<T>::displayAxes;
		using LatticePADisplayData<T>::dataShape;
		using LatticePADisplayData<T>::getDataMin;
		using LatticePADisplayData<T>::getDataMax;
		using LatticePADisplayData<T>::readOptionRecord;
		using LatticePADisplayData<T>::setAttribute;
		using LatticePADisplayData<T>::getHist;
		using LatticePADisplayData<T>::getBrightnessUnits;
	protected:
		using LatticePADisplayData<T>::setNumImages;
		using LatticePADisplayData<T>::DDelement;
	};


} //# NAMESPACE CASA - END

#ifndef AIPS_NO_TEMPLATE_SRC
#include <display/DisplayDatas/LatticeAsRaster.tcc>
#endif //# AIPS_NO_TEMPLATE_SRC
#endif
