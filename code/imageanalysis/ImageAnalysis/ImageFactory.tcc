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
//# $Id: $

#include <imageanalysis/ImageAnalysis/ImageFactory.h>

#include <casa/OS/Directory.h>
#include <casa/OS/RegularFile.h>
#include <casa/OS/SymLink.h>
#include <coordinates/Coordinates/CoordinateUtil.h>
#include <images/Images/PagedImage.h>
#include <images/Images/TempImage.h>
#include <tables/LogTables/NewFile.h>

#include <imageanalysis/ImageAnalysis/ImageHistory.h>
#include <imageanalysis/ImageAnalysis/SubImageFactory.h>

namespace casa {

template <class T> void ImageFactory::remove(SPIIT& image, Bool verbose) {
	ThrowIf(
		! image, "Image cannot be null"
	);
	ThrowIf(
		! image->isPersistent(),
		"This image tool is not associated with a "
		"persistent disk file. It cannot be deleted"
	);
	auto fileName = image->name(False);
	ThrowIf(
		fileName.empty(),
		"Filename is empty or does not exist."
    );
	File f(fileName);
	ThrowIf(
		! f.exists(),
		fileName + " does not exist."
	);

	// Destroy object before deleting image. This is why a reference
	// needs to be passed in.
	image.reset();

	// Now try and blow it away.  If it's open, tabledelete won't delete it.
	String message;
	LogIO log;
	if (Table::canDeleteTable(message, fileName, True)) {
		try {
			Table::deleteTable(fileName, True);
			log << (verbose ? LogIO::NORMAL : LogIO::DEBUG1)
            	<< "deleted table " << fileName << LogIO::POST;
		}
		catch (const AipsError& x) {
			ThrowCc(
				"Failed to delete file " + fileName
				+ " because " + x.getMesg()
			);
		};
	}
	else {
		ThrowCc(
			"Cannot delete file " + fileName
            + " because " + message
		);
	}
}

template <class T> SPIIT ImageFactory::createImage(
    const String& outfile,
    const CoordinateSystem& cSys, const IPosition& shape,
    Bool log, Bool overwrite,
    const vector<std::pair<LogOrigin, String> > *const &msgs
) {
    _checkOutfile(outfile, overwrite);
    Bool blank = outfile.empty();
    ThrowIf(
        shape.nelements() != cSys.nPixelAxes(),
        "Supplied CoordinateSystem and image shape are inconsistent"
    );
    SPIIT image;
    if (blank) {
        image.reset(new TempImage<T>(shape, cSys));
        ThrowIf(! image, "Failed to create TempImage");
    }
    else {
        image.reset(new PagedImage<T>(shape, cSys, outfile));
        ThrowIf(
            ! image,
            "Failed to create PagedImage"
        );
    }
    ostringstream os;
    T *x = 0;
    os << "Created "
       << (blank ? "Temp" : "Paged") << " image "
       << (blank ? "" : "'" + outfile + "'")
       << " of shape " << shape << " with "
       << whatType(x) << " valued pixels.";
    ImageHistory<T> hist(image);
    if (msgs) {
        hist.addHistory(*msgs);
    }
    LogOrigin lor("ImageFactory", __func__);
    hist.addHistory(lor, os.str());
    image->set(0.0);
    if (log) {
        LogIO mylog;
        mylog << LogIO::NORMAL << os.str() << LogIO::POST; 
    }
    return image;
}

template <class T> SPIIT ImageFactory::_fromShape(
	const String& outfile, const Vector<Int>& shapeV,
	const Record& coordinates, Bool linear,
	Bool overwrite, Bool verbose,
    const vector<std::pair<LogOrigin, String> > *const &msgs
) {
	ThrowIf(
		shapeV.nelements() == 0,
		"The shape must have more than zero elements"
	);
	ThrowIf(
		anyTrue(shapeV <= 0),
		"All elements of shape must be positive"
	);

    CoordinateSystem mycsys;
	std::unique_ptr<CoordinateSystem> csysPtr;

	if (coordinates.empty()) {
		mycsys = CoordinateUtil::makeCoordinateSystem(
			shapeV, linear
		);
		_centerRefPix(mycsys, shapeV);
	}
	else {
		csysPtr.reset(
			_makeCoordinateSystem(
				coordinates, shapeV
			)
		);
        mycsys = *csysPtr;
	}
	return createImage<T>(
		outfile, mycsys, shapeV, verbose,
		overwrite, msgs
	);
}

template <class T> SPIIT ImageFactory::imageFromArray(
    const String& outfile, const Array<T>& pixels,
    const Record& csys, Bool linear,
    Bool overwrite, Bool verbose,
    const vector<std::pair<LogOrigin, String> > *const &msgs
) {
	SPIIT myim = _fromShape<T>(
		outfile, pixels.shape().asVector(),
		csys, linear, overwrite, verbose, msgs
	);
	myim->put(pixels);
	return myim;
}

template <class T> SPIIT ImageFactory::_fromRecord(
    const RecordInterface& rec, const String& name
) {
    SPIIT image;
    String err;
    image.reset(new TempImage<T>());
    ThrowIf(
        ! image->fromRecord(err, rec),
        "Error converting image from record: " + err
    );
    if (! name.empty()) {
        image = SubImageFactory<T>::createImage(
            *image, name, Record(), "", False,
            True, False, False
        );
    }
    return image;
}

template <class T> pair<SPIIF, SPIIC> ImageFactory::_rename(
	SPIIT& image, const String& name, const Bool overwrite
) {
	LogIO mylog;
	mylog << LogOrigin(className(), __func__);
	ThrowIf (! image, "Image pointer cannot be null");
	ThrowIf(
		! image->isPersistent(),
		"This image tool is not associated with a "
		"persistent disk file. It cannot be renamed"
	);
	ThrowIf(
		name.empty(), "new file name must be specified"
	);
	String oldName = image->name(False);
	ThrowIf(
		oldName.empty(),
		"Current file name is empty, cannot rename"
	);
	ThrowIf(
		oldName == name,
		"Specified output name is the same as the current image name"
	);

	// Let's see if it exists.  If it doesn't, then the user has deleted it
	File file(oldName);
	if (file.isSymLink()) {
		file = File(SymLink(file).followSymLink());
	}
	ThrowIf(
		! file.exists(), "The image to be renamed no longer exists"
	);
	_checkOutfile(name, overwrite);

	// close image before renaming
	image.reset();

	// Now try and move it
	Bool follow(True);
	if (file.isRegular(follow)) {
		RegularFile(file).move(name, overwrite);
	}
	else if (file.isDirectory(follow)) {
		Directory(file).move(name, overwrite);
	}
	else if (file.isSymLink()) {
		SymLink(file).copy(name, overwrite);
	}
	else {
		ThrowCc("Failed to rename file " + oldName + " to " + name);
	}

	mylog << LogIO::NORMAL << "Successfully renamed file " << oldName
			<< " to " << name << LogIO::POST;

	return fromFile(name);

}


}

