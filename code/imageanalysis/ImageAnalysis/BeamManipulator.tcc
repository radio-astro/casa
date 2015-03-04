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

#include <imageanalysis/ImageAnalysis/BeamManipulator.h>
#include <imageanalysis/ImageAnalysis/ImageMetaData.h>

namespace casa {

template <class T> BeamManipulator<T>::BeamManipulator(SPIIT image)
	: _image(image), _log() {}

template <class T> void BeamManipulator<T>::remove() {
	ImageInfo ii = _image->imageInfo();

	ii.removeRestoringBeam();
	ThrowIf(
		! _image->setImageInfo(ii),
		"Failed to remove restoring beam"
	);
	if (_log) {
		*_log << LogIO::NORMAL << "Deleted restoring beam(s)"
			<< LogIO::POST;
	}
}

template <class T> void BeamManipulator<T>::rotate(const Quantity& angle) {
	ImageInfo ii = _image->imageInfo();
	ImageBeamSet beams = ii.getBeamSet();
	beams.rotate(angle);
	ii.setBeams(beams);
	_image->setImageInfo(ii);
}

template <class T> void BeamManipulator<T>::setVerbose(Bool v) {
	if (v && ! _log) {
		_log.reset(new LogIO());
	}
	else {
		_log.reset();
	}
}

template <class T> void BeamManipulator<T>::set(const ImageBeamSet& beamSet) {
	ThrowIf(
		beamSet.empty(),
		"Input beam set cannot be empty"
	);
	ImageInfo ii = _image->imageInfo();
	ii.setBeams(beamSet);
	ThrowIf(
		! _image->setImageInfo(ii),
		"Failed to set beams"
	);
	if(_log) {
		*_log << LogOrigin("BeamManipulator", __func__);
		*_log << LogIO::NORMAL << "Set image beam set" << LogIO::POST;
	}
}

template <class T> void BeamManipulator<T>::set(
	const Quantity& major, const Quantity& minor,
	const Quantity& pa, const Record& rec,
	Int channel, Int polarization
) {
	if (_log) {
		*_log << LogOrigin("BeamManipulator", __func__);
	}
	ImageInfo ii = _image->imageInfo();
	Quantity bmajor, bminor, bpa;
	if (rec.nfields() != 0) {
		if (
			rec.isDefined("beams") && rec.isDefined("nChannels")
			&& rec.isDefined("nStokes")
		) {
			ImageMetaData md(_image);
			uInt nChanIm = md.nChannels();
			uInt nStokesIm = md.nStokes();
			uInt nChanBeam = rec.asuInt("nChannels");
			uInt nStokesBeam = rec.asuInt("nStokes");
			if (nChanIm == nChanBeam && nStokesIm == nStokesBeam) {
				if (ii.hasBeam()) {
					if (_log) {
						*_log << LogIO::WARN << "Overwriting existing beam(s)" << LogIO::POST;
					}
					ii.removeRestoringBeam();
				}
			}
		}
		else {
			String error;
			// instantiating this object will do implicit consistency checks
			// on the passed-in record
			GaussianBeam beam = GaussianBeam::fromRecord(rec);

			bmajor = beam.getMajor();
			bminor = beam.getMinor();
			bpa = beam.getPA(True);
		}
	}
	else {
		bmajor = major;
		bminor = minor;
		bpa = pa;
	}
	if (bmajor.getValue() == 0 || bminor.getValue() == 0) {
		GaussianBeam currentBeam = ii.restoringBeam(channel, polarization);
		if (! currentBeam.isNull()) {
			bmajor = major.getValue() == 0 ? currentBeam.getMajor() : major;
			bminor = minor.getValue() == 0 ? currentBeam.getMinor() : minor;
			bpa = pa.isConform("rad") ? pa : Quantity(0, "deg");
		}
		else {
			ThrowIf(
				ii.hasMultipleBeams(),
				"This image does not have a corresponding per plane "
				"restoring beam that can be "
				"used to set missing input parameters"
			);
			ThrowCc(
				"This image does not have a restoring beam that can be "
				"used to set missing input parameters"
			);
		}
	}
	if (ii.hasMultipleBeams()) {
		if (channel < 0 && polarization < 0) {
			if (_log) {
				*_log << LogIO::WARN << "This image has per plane beams"
					<< "but no plane (channel/polarization) was specified. All beams will be set "
					<< "equal to the specified beam." << LogIO::POST;
			}
			ImageMetaData md(_image);
			ii.setAllBeams(
				md.nChannels(), md.nStokes(),
				GaussianBeam(bmajor, bminor, bpa)
			);
		}
		else {
			ii.setBeam(channel, polarization, bmajor, bminor, bpa);
		}
	}
	else if (channel >= 0 || polarization >= 0) {
		if (ii.restoringBeam().isNull()) {
			if (_log) {
				*_log << LogIO::NORMAL << "This image currently has no beams of any kind. "
					<< "Since channel and/or polarization were specified, "
					<< "a set of per plane beams, each equal to the specified beam, "
					<< "will be created." << LogIO::POST;
			}
			ImageMetaData md(_image);
			ii.setAllBeams(
				md.nChannels(), md.nStokes(),
				GaussianBeam(bmajor, bminor, bpa)
			);
		}
		else {
			ThrowCc(
				"Channel and/or polarization has "
				"been specified, but this image has a single (global restoring "
				"beam. This beam will not be altered. If you really want to modify "
				"the global beam, rerun setting both channel and "
				"polarization less than zero"
			);
		}
	}
	else {
		if (_log) {
			*_log << LogIO::NORMAL
				<< "Setting (global) restoring beam." << LogIO::POST;
		}
		ii.setRestoringBeam(GaussianBeam(bmajor, bminor, bpa));
	}
	ThrowIf(
		! _image->setImageInfo(ii),
		"Failed to set restoring beam"
	);
	if (_log) {
		*_log << LogIO::NORMAL << "Beam parameters:"
			<< "  Major          : " << bmajor.getValue() << " " << bmajor.getUnit() << endl
			<< "  Minor          : " << bminor.getValue() << " " << bminor.getUnit() << endl
			<< "  Position Angle : " << bpa.getValue() << " " << bpa.getUnit() << endl
			<< LogIO::POST;
	}
}

}

