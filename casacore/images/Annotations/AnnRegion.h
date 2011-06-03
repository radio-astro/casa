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

#ifndef REGIONS_ANNREGION_H
#define REGIONS_ANNREGION_H

#include <coordinates/Coordinates/CoordinateSystem.h>
#include <images/Annotations/AnnotationBase.h>
#include <images/Regions/WCBox.h>
#include <images/Regions/ImageRegion.h>
#include <measures/Measures/MDirection.h>
#include <measures/Measures/MFrequency.h>
#include <measures/Measures/Stokes.h>

namespace casa {

// <summary>
// This class represents a annotation referring to a region specified in an ascii region file as proposed
// in CAS-2285
// </summary>
// <author>Dave Mehringer</author>
// <use visibility=export>
// <reviewed reviewer="" date="yyyy/mm/dd" tests="" demos="">
// </reviewed>
// <prerequisite>

// </prerequisite>

// <etymology>
// Holds the specification of an annotation containing a region as specified in ASCII format.
// </etymology>

// <synopsis>
// This class is meant to be a container for all parameters necessary to specify a region per
// the format proposal attached to CAS-2285 (https://bugs.nrao.edu/browse/CAS-2285).

// Conversions of frequency from one reference frame to another are done here. The position of
// the reference pixel in the supplied coordinate system is used when converting frequencies.
// </synopsis>

class AnnRegion: public AnnotationBase {

public:

	virtual ~AnnRegion();

	// is this region an annotation only? ie just for graphical rendering?
	Bool isAnnotationOnly() const;

	Vector<MFrequency> getFrequencyLimits() const;

	Vector<Stokes::StokesTypes> getStokes() const;

	virtual TableRecord asRecord() const;

	virtual ImageRegion asImageRegion() const;

	virtual const WCRegion* getRegion() const;

	// returns True unless overridden.
	virtual Bool isRegion() const;

	void setDifference(const Bool difference);

	Bool isDifference() const;


protected:
	Bool _isAnnotationOnly;
	Vector<MFrequency> _convertedFreqLimits;
	ImageRegion _imageRegion;
	Quantity _beginFreq, _endFreq, _restFreq;
	Vector<Stokes::StokesTypes> _stokes;
	WCRegion *_wcRegion;
	MFrequency::Types _freqRefFrame;
	MDoppler::Types _dopplerType;
	Bool _isDifference;

	// only to be called by subclasses

	// beginFreq and endFreq can both be 0, in which case
	// all the spectral range is used if a spectral axis exists in
	// <src>csys</csys>. If one of <src>beginFreq</src> or <src>endFreq</src>
	// is given, the other must be given. Frequency units can either conform
	// to Hz, m/s, or pix. If <src>beginFreq</src> and <src>endFreq</src> are not
	// specifed or if they are specified in pixel units,
	// <src>freqRefFrame</src>, <src>dopplerString</src>, and
	// <src>restfreq</src> are ignored. If provided, <src>beginFreq</src>
	// and <src>endFreq</src> must conform to the same units.
	AnnRegion(
		const Type shape,
		const String& dirRefFrameString,
		const CoordinateSystem& csys,
		const Quantity& beginFreq,
		const Quantity& endFreq,
		const String& freqRefFrame,
		const String& dopplerString,
		const Quantity& restfreq,
		const Vector<Stokes::StokesTypes> stokes,
		const Bool annotationOnly
	);

	// extend the region over spectral and/or polarization
	// coordinates
	void _extend(const ImageRegion& region);

	void _toRecord(const ImageRegion& region);

	// convert a length in pixels to an angle.
	Quantity _lengthToAngle(
		const Quantity& quantity, const uInt pixelAxis
	) const;


private:

	WCBox _makeExtensionBox(
		const Vector<Quantity>& freqRange,
		const Vector<Stokes::StokesTypes>& stokesRange,
		const IPosition& pixelAxes
	) const;

	void _checkAndConvertFrequencies();


};

}

#endif
