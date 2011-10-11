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

#ifndef ANNOTATIONS_ANNELLIPSE_H
#define ANNOTATIONS_ANNELLIPSE_H

#include <imageanalysis/Annotations/AnnRegion.h>


#include <casa/aips.h>

namespace casa {

// <summary>
// This class represents an annotation of an elliptical (in position coordinates) region specified
// in an ascii region file as proposed in CAS-2285. It is specified by its center position
// major and minor axes, and position angle.
// </summary>
// <author>Dave Mehringer</author>
// <use visibility=export>
// <reviewed reviewer="" date="yyyy/mm/dd" tests="" demos="">
// </reviewed>
// <prerequisite>

// </prerequisite>

// <etymology>
// Holds the specification of a an annotation of an annular region as specified in ASCII format.
// Specified by center position, major and minor axes, and position angle.
// </etymology>

// <synopsis>
// This class represents an annotation of an annular region in coordinate space specified by
// center, major and minor axes, and position angle
// </synopsis>


class AnnEllipse: public AnnRegion {

public:

	// <src>positionAngle</src> is defined as the angle between north and the
	// ellipse major axis.
	// <group>

	AnnEllipse(
		const Quantity& xcenter, const Quantity& ycenter,
		const Quantity& majorAxis,
		const Quantity& minorAxis, const Quantity& positionAngle,
		const String& dirRefFrameString,
		const CoordinateSystem& csys,
		const IPosition& imShape,
		const Quantity& beginFreq,
		const Quantity& endFreq,
		const String& freqRefFrameString,
		const String& dopplerString,
		const Quantity& restfreq,
		const Vector<Stokes::StokesTypes> stokes,
		const Bool annotationOnly
	);

	// Simplified constructor.
	// all frequencies are used (these can be set after construction).
	// xcenter and ycenter
	// must be in the same frame as the csys direction coordinate.
	// is a region (not just an annotation), although this value can be changed after
	// construction.
	AnnEllipse(
		const Quantity& xcenter, const Quantity& ycenter,
		const Quantity& majorAxis,
		const Quantity& minorAxis, const Quantity& positionAngle,
		const CoordinateSystem& csys,
		const IPosition& imShape,
		const Vector<Stokes::StokesTypes>& stokes
	);


	// implicit copy constructor and destructor are fine

	//</group>

	AnnEllipse& operator=(const AnnEllipse& other);

	// Get the center position, tranformed to the reference
	// from of the coordinate system if necessary
	MDirection getCenter() const;

	// get major axis. The quantity will have units
	// of angular measure
	Quantity getMajorAxis() const;

	// get minor axis. The quantity will have units
	// of angular measure
	Quantity getMinorAxis() const;

	// get position angle. The quantity will have units
	// of angular measure
	Quantity getPositionAngle() const;

	void worldBoundingBox(vector<Quantity>& blc, vector<Quantity>& trc) const;

	virtual ostream& print(ostream &os) const;


private:
	Vector<Quantity> _inputCenter;
	Quantity _inputMajorAxis, _inputMinorAxis, _inputPositionAngle,
		_convertedMajorAxis, _convertedMinorAxis;

	void _init(	const Quantity& xcenter, const Quantity& ycenter);
};

}

#endif /* ASCIIELLIPSEREGION_H_ */
