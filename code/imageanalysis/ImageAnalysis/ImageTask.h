#ifndef IMAGEANALYSIS_IMAGETASK_H
#define IMAGEANALYSIS_IMAGETASK_H

#include <casa/Containers/Record.h>

#include <imageanalysis/ImageTypedefs.h>
#include <imageanalysis/IO/OutputDestinationChecker.h>
#include <imageanalysis/Regions/CasacRegionManager.h>

namespace casac {
class variant;
}

namespace casa {
class LogFile;
template <class T> class ArrayLattice;

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

    // tacitly does nothing if <src>lf</src> is the empty string.
    void setLogfile(const String& lf);

    void setLogfileAppend(const Bool a);

    void setRegion(const Record& region);

    void setMask(const String& mask) { _mask = mask; }

    void setVerbosity(Verbosity verbosity) { _verbosity = verbosity; }

    // These messages will appear in the product image history upon the call to
    // _prepareOutputImage(). They will be located immediately after the input
    // image's copied history. The first value in the pair is the log origin.
    // The second is the associated message. If this method is called more than once
    // on the same object, messages from subsequent calls are appended to the
    // end of messages set in prior calls.
    void addHistory(const vector<std::pair<String, String> >& msgs) const;

    void addHistory(const LogOrigin& origin, const String& msg) const;

    void addHistory(const LogOrigin& origin, const vector<String>& msgs) const;

    // This adds standard history messages regarding the task that was run and
    // input parameters used. The vectors must have the same length
    void addHistory(
    	const LogOrigin& origin, const String& taskname,
    	const vector<String>& paramNames, const vector<casac::variant>& paramValues
    ) const;

    // suppress writing the history on _prepareOutputImage() call. Useful for
    // not writing history to intermediate image products.
    void suppressHistoryWriting(Bool b) { _suppressHistory = b; }

    // get the history associated with the task. Does not include the
    // history of the input image.
    vector<std::pair<String, String> > getHistory() {return _newHistory;}

    void setDropDegen(Bool d) { _dropDegen = d; }

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

    inline const SHARED_PTR<LogIO> _getLog() const {return _log;}

    inline void _setSupportsLogfile(const Bool b) { _logfileSupport=b;}

    Bool _hasLogfileSupport() const {return _logfileSupport;}

    inline Bool _getStretch() const {return _stretch;}

    const SHARED_PTR<LogFile> _getLogFile() const;

    Bool _writeLogfile(
    	const String& output, const Bool open=True,
    	const Bool close=True
    );

    Bool _openLogfile();

    void _closeLogfile() const;

    virtual inline Bool _supportsMultipleRegions() const {return False;}

    virtual inline Bool _supportsMultipleBeams() {return True;}

    // If outname != NULL, use the value supplied. If is NULL, use the value of _outname.
    // Create a TempImage or PagedImage depending if outname/_outname is empty or not. Generally meant
    // for the image to be returned to the UI or the final image product that the user will want.
    // values=0 => the pixel values from the image will be used
    // mask=0 => the mask attached to the image, if any will be used, outShape=0 => use image shape, coordsys=0 => use image coordinate
    // system. overwrite is only used if outname != NULL.
    SPIIT _prepareOutputImage(
    	const ImageInterface<T>& image, const Array<T> *const values=0,
    	const ArrayLattice<Bool> *const mask=0,
    	const IPosition *const outShape=0, const CoordinateSystem *const coordsys=0,
    	const String *const outname=0, Bool overwrite=False, Bool dropDegen=False
    ) const;

    SPIIT _prepareOutputImage(
    	const ImageInterface<T>& image, Bool dropDegen
    ) const {
    	return _prepareOutputImage(
    		image, 0, 0, 0, 0, 0, False, dropDegen
    	);
    }

    Verbosity _getVerbosity() const { return _verbosity; }

    Bool _getOverwrite() const { return _overwrite; }

    virtual Bool _mustHaveSquareDirectionPixels() const {return False;}

    Bool _getDropDegen() const { return _dropDegen; }

private:
    const SPCIIT _image;
    SHARED_PTR<LogIO> _log;
    const Record *const _regionPtr;
    Record _regionRecord;
    String _region, _box, _chan, _stokesString, _mask, _outname;
    Bool _overwrite, _stretch, _logfileSupport, _logfileAppend,
    	_suppressHistory, _dropDegen;
	std::unique_ptr<FiledesIO> _logFileIO;
	Verbosity _verbosity;
	SHARED_PTR<LogFile> _logfile;
	mutable vector<std::pair<String, String> > _newHistory;
};

}

#ifndef AIPS_NO_TEMPLATE_SRC
#include <imageanalysis/ImageAnalysis/ImageTask.tcc>
#endif

#endif
