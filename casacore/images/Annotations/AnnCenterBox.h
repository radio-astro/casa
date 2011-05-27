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

#ifndef REGIONS_ANNCENTERBOX_H
#define REGIONS_ANNCENTERBOX_H

#include <casa/aips.h>
#include <images/Annotations/AnnRegion.h>

namespace casa {

// <summary>
// This class represents a annotation for rectangular (in position coordinates) region specified
// in an ascii region file as proposed in CAS-2285. It is specified by its center position
// and side widths.
// </summary>
// <author>Dave Mehringer</author>
// <use visibility=export>
// <reviewed reviewer="" date="yyyy/mm/dd" tests="" demos="">
// </reviewed>
// <prerequisite>

// </prerequisite>

// <etymology>
// Holds the specification of an annotation of a rectangular region as specified in ASCII format.
// Specified by center position and widths of sides.
// </etymology>

// <synopsis>
// This class represents an annotation of rectangular region in coordinate space specified by
// center and widths of sides.
// </synopsis>


class AnnCenterBox: public AnnRegion {

public:

	AnnCenterBox(
		const Quantity& centerx,
		const Quantity& centery,
		const Quantity& xwidth,
		const Quantity& ywidth,
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

	MDirection getCenter() const;

	Vector<Quantity> getWidths() const;

	// get the blc and trc direction coords for the box which
	// have been converted to the coordinate frame specified
	// when the object was constructed.
	Vector<MDirection> getCorners() const;

private:
	//Vector<Quantity> _center;
	Vector<Quantity> _widths;
	Vector<MDirection> _corners;

	void _doCorners();
};

}

#endif /* ASCIICENTERBOXREGION_H_ */
