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

#ifndef ANNOTATIONS_ANNROTBOX_H
#define ANNOTATIONS_ANNROTBOX_H

#include <casa/aips.h>
#include <imageanalysis/Annotations/AnnRegion.h>

namespace casa {

// <summary>
// This class represents an annotation for rectangular (in position coordinates) region specified
// in an ascii region file as proposed in CAS-2285. It is specified by its center position
// and side widths and a position angle.
// </summary>
// <author>Dave Mehringer</author>
// <use visibility=export>
// <reviewed reviewer="" date="yyyy/mm/dd" tests="" demos="">
// </reviewed>
// <prerequisite>

// </prerequisite>

// <etymology>
// Holds the specification of an annotation for a rectangular region as specified in ASCII format.
// Specified by center position and widths of sides and a position angle
// </etymology>

// <synopsis>
// This class represents an annotation for a rectangular region in coordinate space specified by
// center and widths of sides and a position angle.
// </synopsis>


class AnnRotBox: public AnnRegion {

public:

	// <src>positionAngle</src> is measured in the usual astronomical
	// way; starting at north through east (counterclockwise)
	AnnRotBox(
		const Quantity& xcenter,
		const Quantity& ycenter,
		const Quantity& xwidth,
		const Quantity& ywidth, const Quantity& positionAngle,
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

	// implicit copy constructor and destructor are fine

	AnnRotBox& operator=(const AnnRotBox& other);

	// get the four corners of the box in direction space.
	// These will have been converted to the frame of the
	// coordinate system if necessary. The only thing about
	// the order of the vector that is garaunteed is that
	// adjacent corners will not be diagonally opposite
	// from each other.
	Vector<MDirection> getCorners() const;

	virtual ostream& print(ostream &os) const;

private:
	Vector<Quantity> _inputCenter, _inputWidths;
	Vector<Quantity> _widths;
	Quantity _positionAngle;
	Vector<MDirection> _corners;

	void _doCorners();
};

}

#endif
