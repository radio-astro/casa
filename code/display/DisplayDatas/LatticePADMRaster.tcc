//# LatticePADMRaster.cc: Class for drawing axis bound lattice elements as rasters
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

#include <casa/aips.h>
#include <casa/Arrays/Array.h>
#include <casa/Arrays/Matrix.h>
#include <casa/Arrays/Vector.h>
#include <casa/Arrays/IPosition.h>
#include <display/Display/WorldCanvas.h>
#include <display/Display/PixelCanvas.h>
#include <display/Display/Attribute.h>
#include <display/DisplayDatas/LatticeAsRaster.h>
#include <display/DisplayCanvas/WCPowerScaleHandler.h>
#include <display/DisplayDatas/LatticePADMRaster.h>

namespace casa { //# NAMESPACE CASA - BEGIN

// Constructor for the case of multiple slices
	template <class T>
	LatticePADMRaster<T>::LatticePADMRaster(const casacore::uInt xAxis,
	                                        const casacore::uInt yAxis, const casacore::uInt mAxis,
	                                        const casacore::IPosition fixedPos,
	                                        LatticePADisplayData<T> *arDat) :
		LatticePADisplayMethod<T>(xAxis, yAxis, mAxis, fixedPos, arDat) {
	}

// Constructor for a single slice
	template <class T>
	LatticePADMRaster<T>::LatticePADMRaster(const casacore::uInt xAxis,
	                                        const casacore::uInt yAxis,
	                                        LatticePADisplayData<T> *arDat) :
		LatticePADisplayMethod<T>(xAxis, yAxis, arDat) {
	}

// Destructor
	template <class T>
	LatticePADMRaster<T>::~LatticePADMRaster() {
	}

	template <class T>
	casacore::Bool LatticePADMRaster<T>::dataRedrawSelf(WorldCanvas *wc,
	        Display::RefreshReason reason) {

		casacore::Bool opaqueMask = true;
		return  wc->redrawIndexedImage(parentDisplayData(), reason, opaqueMask);
		// (Parent DD passed as key: we cache only one image of this
		// type per LatticeAsRaster DD (for each registered WC...)).

	}

	template <class T>
	bool LatticePADMRaster<T>::initializeColorMatrix( LatticeAsRaster<T>* lar,
			const casacore::IPosition &start,
            const casacore::IPosition &shape,
            const casacore::IPosition &stride,
            casacore::Matrix<T>& datMatrix,
			casacore::Matrix<casacore::Bool>& maskMatrix) const {
		bool initialized = false;
		if ( lar != NULL ){
			try {
				lar->initializeDataMatrix(0, datMatrix, maskMatrix, start, shape, stride);
				initialized = true;
			}
			catch( casacore::AipsError& error ){
				casacore::LogIO os;
				os << casacore::LogIO::WARN << casacore::LogOrigin("LatticePADMRaster","dataDrawSelf", WHERE)
					<< "Could not initialize RGB image:"<<error.getMesg().c_str()<< casacore::LogIO::POST;
			}
		}
		return initialized;
	}


// Actually draw the slice as a raster image
	template <class T>
	casacore::uInt LatticePADMRaster<T>::dataDrawSelf(WorldCanvas *wCanvas,
	                                        const casacore::Vector<casacore::Double> &blc,
	                                        const casacore::Vector<casacore::Double> &trc,
	                                        const casacore::IPosition &start,
	                                        const casacore::IPosition &shape,
	                                        const casacore::IPosition &stride,
	                                        const casacore::Bool usePixelEdges) {
		casacore::uInt drawListNumber = wCanvas->newList();

		LatticeAsRaster<T> *lar = (LatticeAsRaster<T> *)parentDisplayData();

		// set min and max datavalues to help out scalehandler on WC.
		Attribute dmin("dataMin", casacore::Double(lar->itsOptionsDataRange(0)));
		Attribute dmax("dataMax", casacore::Double(lar->itsOptionsDataRange(1)));
		wCanvas->setAttribute(dmin);
		wCanvas->setAttribute(dmax);

		// Set the complex-to-real mode on the WorldCanvas if necessary:
		T t;
		casacore::DataType dtype = casacore::whatType(&t);
		if ((dtype == casacore::TpComplex) || (dtype == casacore::TpDComplex)) {
			wCanvas->setComplexToRealMethod
			(lar->complexMode());
		}

		try {
			wCanvas->setResampleHandler(lar->resampleHandler());
			wCanvas->setDataScaleHandler(lar->itsPowerScaleHandler);
			casacore::Matrix<T> datMatrix;
			casacore::Matrix<casacore::Bool> maskMatrix;
			this->dataGetSlice(datMatrix, maskMatrix, start, shape, stride);
			casacore::Bool useMask = (maskMatrix.nelements() == datMatrix.nelements());
			switch (wCanvas->pixelCanvas()->pcctbl()->colorModel()) {
			case Display::Index: {
				if (lar->itsOptionsColorMode == "colormap") {

					LatticeAsRaster<T>*  larRed = lar->getRasterRed();
					LatticeAsRaster<T>*  larGreen = lar->getRasterGreen();
					LatticeAsRaster<T>*  larBlue = lar->getRasterBlue();
					if ( larRed == NULL && larGreen == NULL && larBlue == NULL ){
						casacore::Bool opaqueMask = true;
						// (per CB/cabal request CAS-531 (1), 7/08.  They don't
						// want to see lower Raster layers 'shine through' the
						// top layer's masked regions (and don't want to have to
						// unregister the lower layers, apparently)).

						if (useMask) wCanvas->drawImage(blc, trc, datMatrix, maskMatrix,
						                                usePixelEdges, lar, opaqueMask);

						else wCanvas->drawImage(blc, trc, datMatrix, usePixelEdges, lar);
						// (caching color-indexed image under parent LAR DD).

					}
					else {
						casacore::Matrix<T> datMatrixRed;
						casacore::Matrix<casacore::Bool> maskMatrixRed;
						bool redOK = initializeColorMatrix(larRed,start,shape,stride,datMatrixRed,maskMatrixRed );


						casacore::Matrix<T> datMatrixGreen;
						casacore::Matrix<casacore::Bool> maskMatrixGreen;
						bool greenOK = initializeColorMatrix(larGreen,start,shape,stride,datMatrixGreen,maskMatrixGreen );

						casacore::Matrix<T> datMatrixBlue;
						casacore::Matrix<casacore::Bool> maskMatrixBlue;
						bool blueOK = initializeColorMatrix(larBlue,start,shape,stride,datMatrixBlue,maskMatrixBlue );
						if ( redOK || blueOK || greenOK ){
							wCanvas->drawImage(blc, trc, datMatrix, datMatrixRed,
									datMatrixGreen, datMatrixBlue, usePixelEdges, lar );
						}
						else {
							casacore::LogIO os;
							os << casacore::LogIO::WARN << casacore::LogOrigin("LatticePADMRaster","dataDrawSelf", WHERE)
								<< "RGB Combination Image was not drawn."<< casacore::LogIO::POST;
						}
					}

				} else {
					casacore::LogIO os;
					os << casacore::LogIO::WARN << casacore::LogOrigin("LatticePADMRaster",
					                               "dataDrawSelf", WHERE)
					   << "Invalid colormap mode for an Indexed PixelCanvas"
					   << casacore::LogIO::POST;
				}
			}
			break;
			case Display::RGB:
				if (useMask) {
					//cerr << "mask available, but being ignored in "
					//    << "LPADMRaster::dataDrawSelf" << endl;
				}
				if (lar->itsOptionsColorMode == "red") {
					wCanvas->drawImage(blc, trc, datMatrix, Display::Red, usePixelEdges);
				} else if (lar->itsOptionsColorMode == "green") {
					wCanvas->drawImage(blc, trc, datMatrix, Display::Green, usePixelEdges);
				} else if (lar->itsOptionsColorMode == "blue") {
					wCanvas->drawImage(blc, trc, datMatrix, Display::Blue, usePixelEdges);
				} else {
					casacore::LogIO os;
					os << casacore::LogIO::WARN << casacore::LogOrigin("LatticePADMRaster",
					                               "dataDrawSelf", WHERE)
					   << "Invalid colormap mode for an RGB PixelCanvas"
					   << casacore::LogIO::POST;
				}
				break;
			case Display::HSV:
				if (useMask) {
					// cerr << "mask available, but being ignored in "
					//    << "LPADMRaster::dataDrawSelf" << endl;
				}
				if (lar->itsOptionsColorMode == "hue") {
					wCanvas->drawImage(blc, trc, datMatrix, Display::Hue, usePixelEdges);
				} else if (lar->itsOptionsColorMode == "saturation") {
					wCanvas->drawImage(blc, trc, datMatrix, Display::Saturation,
					                   usePixelEdges);
				} else if (lar->itsOptionsColorMode == "value") {
					wCanvas->drawImage(blc, trc, datMatrix, Display::Value,
					                   usePixelEdges);
				} else {
					casacore::LogIO os;
					os << casacore::LogIO::WARN << casacore::LogOrigin("LatticePADMRaster",
					                               "dataDrawSelf", WHERE)
					   << "Invalid colormap mode for an HSV PixelCanvas"
					   << casacore::LogIO::POST;
				}
				break;
			default:
				throw(casacore::AipsError("Unknown PixelCanvas ColorModel in "
				                "LatticePADMRaster::dataDrawSelf"));
				break;
			}
			wCanvas->setDataScaleHandler(0);
			wCanvas->setResampleHandler(0);
		} catch (const casacore::AipsError &x) {
			wCanvas->endList();
			if (wCanvas->validList(drawListNumber)) {
				wCanvas->deleteList(drawListNumber);
			}
			throw(casacore::AipsError(x));
		}


		wCanvas->endList();
		return drawListNumber;
	}


} //# NAMESPACE CASA - END

