#include <imageanalysis/ImageAnalysis/ImageMaxFitter.h>

#include <imageanalysis/ImageAnalysis/ImageSourceFinder.h>


namespace casa {

template<class T> ImageMaxFitter<T>::ImageMaxFitter(
	SPCIIT image, const Record *const &region
) : ImageTask<T>(
		image, "", region, "", "", "", "", "", False
	) {
	this->_construct();
}

template<class T> Record ImageMaxFitter<T>::fit(
	Bool doPoint, Int width, Bool absFind, Bool list
) const {
	Vector<Double> absPixel;
	AxesSpecifier axesSpec(False);
	String mask;
	auto subImage = SubImageFactory<T>::createSubImageRO(
		*this->_getImage(), *this->_getRegion(), mask,
		this->_getLog().get(), axesSpec
	);
	*this->_getLog() << LogOrigin(getClass(), __func__);
	ImageSourceFinder<T> sf(subImage, nullptr, "");
	sf.setCutoff(0.1);
	sf.setAbsFind(absFind);
	sf.setDoPoint(doPoint);
	sf.setWidth(width);
	//auto sky = sf.findSourceInSky(*this->_getLog(), absPixel, cutoff, absFind, doPoint, width);
	auto sky = sf.findSourceInSky(absPixel);
	// modify to show dropped degenerate axes values???
	if (list) {
		*this->_getLog() << LogIO::NORMAL << "Brightness     = " << sky.flux().value()
			<< " " << sky.flux().unit().getName() << LogIO::POST;
		const auto& cSys = subImage->coordinates();
		*this->_getLog() << "World Axis Units: " << cSys.worldAxisUnits() << LogIO::POST;
		Vector<Double> wPix;
		if (!cSys.toWorld(wPix, absPixel)) {
			*this->_getLog() << LogIO::WARN
				<< "Failed to convert to absolute world coordinates "
				<< cSys.errorMessage() << LogIO::POST;
		}
		else {
			*this->_getLog() << "Absolute world = " << wPix << LogIO::POST;
		}
		Vector<Double> wRel = wPix.copy();
		cSys.makeWorldRelative(wRel);
		*this->_getLog() << "Relative world = " << wRel << LogIO::POST;
		*this->_getLog() << LogIO::NORMAL << "Absolute pixel = " << absPixel << endl;
		Vector<Double> pRel = absPixel.copy();
		cSys.makePixelRelative(pRel);
		*this->_getLog() << "Relative pixel = " << pRel << LogIO::POST;
	}
	ComponentList mycomp;
	mycomp.add(sky);
	String error;
	Record outrec;
	ThrowIf(
		! mycomp.toRecord(error, outrec),
		"Cannot convert SkyComponent to output record"
	);
	return outrec;
}

}
