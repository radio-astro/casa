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
#include <images/Annotations/AnnRegion.h>

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
		const Quantity& beginFreq,
		const Quantity& endFreq,
		const String& freqRefFrameString,
		const String& dopplerString,
		const Quantity& restfreq,
		const Vector<Stokes::StokesTypes> stokes,
		const Bool annotationOnly
	);

	Vector<MDirection> getCorners() const;

private:
	Vector<MDirection> _corners;
	Vector<Quantity> _origXPos, _origYPos;
};

}

#endif /* ASCIIPOLYGONREGION_H_ */
