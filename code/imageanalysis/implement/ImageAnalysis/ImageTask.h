//# tSubImage.cc: Test program for class SubImage
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
//# $Id: tSubImage.cc 20567 2009-04-09 23:12:39Z gervandiepen $

#ifndef IMAGEANALYSIS_IMAGETASK_H
#define IMAGEANALYSIS_IMAGETASK_H

#include <imageanalysis/ImageAnalysis/ImageInputProcessor.h>
#include <memory>

#include <casa/namespace.h>

namespace casa {

class ImageTask {
    // <summary>
    // Virtual base class for image tasking.
    // </summary>

    // <reviewed reviewer="" date="" tests="" demos="">
    // </reviewed>

    // <prerequisite>
    // </prerequisite>

    // <etymology>
    // Image tasking
    // </etymology>

    // <synopsis>
    // Virtual base class for image tasking.
    // </synopsis>

public:

    // destructor
    virtual ~ImageTask();

    virtual String getClass() const = 0;

    inline void setStretch(const Bool stretch) { _stretch = stretch;}

protected:

	// if <src>outname</src> is empty, no image will be written
 	// if <src>overwrite</src> is True, if image already exists it will be removed
  	// if <src>overwrite</src> is False, if image already exists exception will be thrown

   	ImageTask(
   		const ImageInterface<Float> *const &image,
    	const String& region, const Record *const &regionPtr,
    	const String& box, const String& chanInp,
    	const String& stokes, const String& maskInp,
        const String& outname, const Bool overwrite
    );

   	virtual CasacRegionManager::StokesControl _getStokesControl() const = 0;

    virtual vector<ImageInputProcessor::OutputStruct> _getOutputStruct();

    // does the lion's share of constructing the object, ie checks validity of
    // inputs, etc.

    virtual void _construct();

    inline const ImageInterface<Float>* _getImage() const {return _image;}

    inline const String& _getMask() const {return _mask;}

    inline const Record* _getRegion() const {return &_regionRecord;}

    inline const String& _getStokes() const {return _stokesString;}

    inline const String& _getChans() const {return _chan;}

    inline const String& _getOutname() const {return _outname; }

    virtual vector<Coordinate::Type> _getNecessaryCoordinates() const = 0;

    void _removeExistingOutfileIfNecessary() const;

    String _summaryHeader() const;

    inline const std::auto_ptr<LogIO>& _getLog() const {
    	return _log;
    }

    inline Bool _getStretch() const {return _stretch;}

private:
    const ImageInterface<Float> *const _image;
    std::auto_ptr<LogIO> _log;
    const Record *const _regionPtr;
    Record _regionRecord;
    String _region, _box, _chan, _stokesString, _mask, _outname;
    Bool _overwrite, _stretch;





};
}

#endif
