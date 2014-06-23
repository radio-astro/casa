//# Copyright (C) 1998,1999,2000,2001,2003
//# Associated Universities, Inc. Washington DC, USA.
//#
//# This program is free software; you can redistribute it and/or modify it
//# under the terms of the GNU General Public License as published by the Free
//# Software Foundation; either version 2 of the License, or (at your option)
//# any later version.
//#
//# This program is distributed in the hope that it will be useful, but WITHOUT
//# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
//# FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
//# more details.
//#
//# You should have received a copy of the GNU General Public License along
//# with this program; if not, write to the Free Software Foundation, Inc.,
//# 675 Massachusetts Ave, Cambridge, MA 02139, USA.
//#
//# Correspondence concerning AIPS++ should be addressed as follows:
//#        Internet email: aips2-request@nrao.edu.
//#        Postal address: AIPS++ Project Office
//#                        National Radio Astronomy Observatory
//#                        520 Edgemont Road
//#                        Charlottesville, VA 22903-2475 USA
//#

#ifndef IMAGES_IMAGEINPUTPROCESSOR_H
#define IMAGES_IMAGEINPUTPROCESSOR_H

#include <images/Images/ImageInterface.h>

#include <imageanalysis/ImageTypedefs.h>
#include <imageanalysis/IO/OutputDestinationChecker.h>
#include <imageanalysis/Regions/CasacRegionManager.h>

#include <casa/namespace.h>

#include <tr1/memory>

namespace casa {

class ImageInputProcessor {
	// <summary>
	// Collection of methods for processing inputs to image analysis applications
	// </summary>

	// <reviewed reviewer="" date="" tests="" demos="">
	// </reviewed>

	// <prerequisite>
	// </prerequisite>

	// <etymology>
	// Processes inputs to image analysis apps.
	// </etymology>

	// <synopsis>
	// Collection of methods for processing inputs to image analysis applications
	// </synopsis>

public:

	//constructor
	ImageInputProcessor();

	//Destructor
	~ImageInputProcessor();

	// Process the inputs. Output parameters are the pointer to the
	// opened <src>image</src>, the specified region as a record (<src>
	// regionRecord</src>, and a <src>diagnostics</src> String describing
	// how the region was chosen. If provided, <src>regionPtr</src> should
	// be a pointer to a record created by a RegionManager method.
	// <src>stokesControl</src> indicates default
	// stokes range to use if <src>stokes</src> is blank. In this case <src>stokes</src>
	// will be set the the value of stokes that will be used. If
	// <src>allowMultipleBoxes</src> is False, an exception will be thrown if
	// the inputs specify multiple n-dimensional rectangles. This should usually
	// be set to false if the caller can only deal with a single n-dimensional
	// rectangular region.
    template<class T> void process(
    	SPIIT image, Record& regionRecord,
    	String& diagnostics,
    	std::vector<OutputDestinationChecker::OutputStruct> *const outputStruct,
    	String& stokes,	const String& imagename,
    	const Record* regionPtr, const String& regionName,
    	const String& box, const String& chans,
    	const CasacRegionManager::StokesControl& stokesControl,
    	const Bool& allowMultipleBoxes,
    	const std::vector<Coordinate::Type> *const &requiredCoordinateTypes,
    	Bool verbose=True
    );

	// Process the inputs. Use this version if the associated image already exists.
    // Output parameters the specified region as a record (<src>
	// regionRecord</src>, and a <src>diagnostics</src> String describing
	// how the region was chosen. If provided, <src>regionPtr</src> should
	// be a pointer to a record created by a RegionManager method.
    // <src>stokesControl</src> indicates default
	// stokes range to use if <src>stokes</src> is blank. In this case <src>stokes</src>
	// will be set the the value of stokes that will be used. If
	// <src>allowMultipleBoxes</src> is False, an exception will be thrown if
	// the inputs specify multiple n-dimensional rectangles. This should usually
	// be set to false if the caller can only deal with a single n-dimensional
	// rectangular region.
    template<class T> void process(
    	Record& regionRecord,
    	String& diagnostics,
    	std::vector<OutputDestinationChecker::OutputStruct> *const outputStruct,
    	String& stokes,
    	SPCIIT image,
    	const Record* regionPtr,
    	const String& regionName, const String& box,
    	const String& chans,
    	const CasacRegionManager::StokesControl& stokesControl,
    	const Bool& allowMultipleBoxes,
    	const std::vector<Coordinate::Type> *const &requiredCoordinateTypes,
    	Bool verbose=True
    );

    // Get the number of channels that have been selected. The process() method must
    // be called prior to calling this method or an exception is thrown.
    uInt nSelectedChannels() const;

private:
    LogIO *_log;
    Bool _processHasRun;
    uInt _nSelectedChannels;

    template<class T> void _process(
    	Record& regionRecord, String& diagnostics,
    	std::vector<OutputDestinationChecker::OutputStruct>* outputStruct,
    	String& stokes, SPCIIT image,
    	const Record *const &regionPtr,
    	const String& regionName, const String& box,
    	const String& chans, const CasacRegionManager::StokesControl& stokesControl,
        const Bool& allowMultipleBoxes,
    	const std::vector<Coordinate::Type> *const &requiredCoordinateTypes, Bool verbose
    );

    // set region given a pointer to a region record.
    void _setRegion(
    	Record& regionRecord, String& diagnostics,
    	const Record *const regionPtr
    ) const;

    template<class T> void _setRegion(Record& regionRecord, String& diagnostics,
    	const ImageInterface<T> *const image, const String& regionName
    ) const;

    String _stokesFromRecord(
    	const Record& region, const CoordinateSystem& csys
    ) const;

    String _pairsToString(const std::vector<uInt>& pairs) const;

};
}

#ifndef AIPS_NO_TEMPLATE_SRC
#include <imageanalysis/ImageAnalysis/ImageInputProcessor2.tcc>
#endif

#endif /* IMAGES_IMAGEINPUTPROCESSOR_H */
