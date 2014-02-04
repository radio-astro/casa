#ifndef IMAGEANALYSIS_IMAGETASK_H
#define IMAGEANALYSIS_IMAGETASK_H

#include <imageanalysis/ImageTypedefs.h>
#include <imageanalysis/ImageAnalysis/ImageInputProcessor.h>

#include <casa/IO/FiledesIO.h>

#include <imageanalysis/IO/OutputDestinationChecker.h>
#include <casa/namespace.h>

#include <tr1/memory>

namespace casa {
class LogFile;

template <class T> class ImageTask {

    // <summary>
    // Virtual base class for image analysis tasks.
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

	// verbosity levels
	enum Verbosity {
		QUIET,
		WHISPER,
		LOW,
		NORMAL,
		HIGH,
		NOISY,
		DEAFENING
	};

    virtual ~ImageTask();

    virtual String getClass() const = 0;

    inline void setStretch(const Bool stretch) { _stretch = stretch;}

    void setLogfile(const String& lf);

    void setLogfileAppend(const Bool a);

    void setRegion(const Record& region);

    void setMask(const String& mask) { _mask = mask; }

    void setVerbosity(Verbosity verbosity) { _verbosity = verbosity; }

protected:

	// if <src>outname</src> is empty, no image will be written
 	// if <src>overwrite</src> is True, if image already exists it will be removed
  	// if <src>overwrite</src> is False, if image already exists exception will be thrown

   	ImageTask(
   		const SPCIIT image,
    	const String& region, const Record *const &regionPtr,
    	const String& box, const String& chanInp,
    	const String& stokes, const String& maskInp,
        const String& outname, const Bool overwrite
    );

   	virtual CasacRegionManager::StokesControl _getStokesControl() const = 0;

    virtual std::vector<OutputDestinationChecker::OutputStruct> _getOutputStruct();

    // does the lion's share of constructing the object, ie checks validity of
    // inputs, etc.

    virtual void _construct(Bool verbose=True);

    inline const SPCIIT _getImage() const {return _image;}

    inline const String& _getMask() const {return _mask;}

    inline const Record* _getRegion() const {return &_regionRecord;}

    inline void _setStokes(const String& stokes) { _stokesString = stokes; }

    inline const String& _getStokes() const {return _stokesString;}

    inline const String& _getChans() const {return _chan;}

    inline const String& _getOutname() const {return _outname; }

    // Represents the minimum set of coordinates necessary for the
    // task to function.
    virtual std::vector<Coordinate::Type> _getNecessaryCoordinates() const = 0;

    void _removeExistingOutfileIfNecessary() const;

    static void _removeExistingFileIfNecessary(
    	const String& filename, const Bool overwrite
    );

    String _summaryHeader() const;

    inline const std::tr1::shared_ptr<LogIO> _getLog() const {return _log;}

    inline void _setSupportsLogfile(const Bool b) { _logfileSupport=b;}

    Bool _hasLogfileSupport() const {return _logfileSupport;}

    inline Bool _getStretch() const {return _stretch;}

    const std::tr1::shared_ptr<LogFile> _getLogFile() const;

    Bool _writeLogfile(
    	const String& output, const Bool open=True,
    	const Bool close=True
    );

    Bool _openLogfile();

    void _closeLogfile() const;

    virtual inline Bool _supportsMultipleRegions() {return False;}

    // Create a TempImage or PagedImage depending if _outname is empty or not. Generally meant
    // for the image to be returned to the UI or the final image product that the user will want.
    // values=0 => the pixel values from the image will be used
    // mask=0 => the mask attached to the image, if any will be used, outShape=0 => use image shape, coordsys=0 => use image coordinate
    // system
    SPIIT _prepareOutputImage(
    	const ImageInterface<T>& image, const Array<T> *const values=0,
    	const ArrayLattice<Bool> *const mask=0,
    	const IPosition *const outShape=0, const CoordinateSystem *const coordsys=0
    ) const;

    Verbosity _getVerbosity() const { return _verbosity; }

    Bool _getOverwrite() const { return _overwrite; }

    virtual Bool _mustHaveSquareDirectionPixels() const {return False;}

private:
    const SPCIIT _image;
    std::tr1::shared_ptr<LogIO> _log;
    const Record *const _regionPtr;
    Record _regionRecord;
    String _region, _box, _chan, _stokesString, _mask, _outname;
    Bool _overwrite, _stretch, _logfileSupport, _logfileAppend;
	std::auto_ptr<FiledesIO> _logFileIO;
	Verbosity _verbosity;
	std::tr1::shared_ptr<LogFile> _logfile;
};

}

#ifndef AIPS_NO_TEMPLATE_SRC
#include <imageanalysis/ImageAnalysis/ImageTask.tcc>
#endif

#endif
