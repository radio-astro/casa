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

#ifndef ANNOTATIONS_ANNPOLYGON_H
#define ANNOTATIONS_ANNPOLYGON_H

#include <casa/aips.h>
#include <imageanalysis/Annotations/AnnRegion.h>

namespace casa {

// <summary>
// This class represents a single polygon (in position coordinates) annotation specified
// in an ascii region file as proposed in CAS-2285
// </summary>
// <author>Dave Mehringer</author>
// <use visibility=export>
// <reviewed reviewer="" date="yyyy/mm/dd" tests="" demos="">
// </reviewed>
// <prerequisite>

// </prerequisite>

// <etymology>
// Holds the specification of a polygon annotation.
// </etymology>

// <synopsis>
// This class represents a polygon annotation.
// </synopsis>


class AnnPolygon: public AnnRegion {

public:

	AnnPolygon(
		const Vector<Quantity>& xPositions,
		const Vector<Quantity>& yPositions,
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
	// xPositions and yPositions
	// must be in the same frame as the csys direction coordinate.
	// is a region (not just an annotation), although this value can be changed after
	// construction.
	AnnPolygon(
		const Vector<Quantity>& xPositions,
		const Vector<Quantity>& yPositions,
		const CoordinateSystem& csys,
		const IPosition& imShape,
		const Vector<Stokes::StokesTypes>& stokes
	);

	// implicit copy constructor and destructor are fine

	AnnPolygon& operator=(const AnnPolygon& other);

	// get the vertices converted to the coordinate system used at construction.
	Vector<MDirection> getCorners() const;

	// get the world coordinates of the polygon vertices
	void worldVertices(vector<Quantity>& x, vector<Quantity>& y) const;

	// get the pixel coordinates of the polygon vertices
	void pixelVertices(vector<Double>& x, vector<Double>& y) const;


	virtual ostream& print(ostream &os) const;

protected:
	// for rectangle subclasses
	AnnPolygon(
		AnnotationBase::Type shape,
		const Quantity& blcx,
		const Quantity& blcy,
		const Quantity& trcx,
		const Quantity& trcy,
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
	// blcx, blcy, trcx, and trcy
	// must be in the same frame as the csys direction coordinate.
	// is a region (not just an annotation), although this value can be changed after
	// construction.
	AnnPolygon(
		AnnotationBase::Type shape,
		const Quantity& blcx,
		const Quantity& blcy,
		const Quantity& trcx,
		const Quantity& trcy,
		const CoordinateSystem& csys,
		const IPosition& imShape,
		const Vector<Stokes::StokesTypes>& stokes
	);

	// For centered rectangles
	AnnPolygon(
		AnnotationBase::Type shape,
		const Quantity& centerx,
		const Quantity& centery,
		const Quantity& widthx,
		const Quantity& widthy,
		const Quantity& positionAngle,
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
		// centerx and centery
		// must be in the same frame as the csys direction coordinate.
		// is a region (not just an annotation), although this value can be changed after
		// construction.
		AnnPolygon(
			AnnotationBase::Type shape,
			const Quantity& centerx,
			const Quantity& centery,
			const Quantity& widthx,
			const Quantity& widthy,
			const Quantity& positionAngle,
			const CoordinateSystem& csys,
			const IPosition& imShape,
			const Vector<Stokes::StokesTypes>& stokes
		);

private:
	Vector<Quantity> _origXPos, _origYPos;

	void _init();

	void _initCorners(
		const Quantity& blcx,
		const Quantity& blcy,
		const Quantity& trcx,
		const Quantity& trcy
	);

	void _initCorners(
		const MDirection& blc,
		const MDirection& corner2,
		const MDirection& trc,
		const MDirection& corner4
	);

	void _initCenterRectCorners(
		const Quantity& centerx,
		const Quantity& centery,
		const Quantity& widthx,
		const Quantity& widthy,
		const Quantity& positionAngle
	);

	void _doCorners(const Quantity& widthx, const Quantity widthy);

};

}

#endif /* ASCIIPOLYGONREGION_H_ */
