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

#ifndef REGIONS_ANNANNULUS_H
#define REGIONS_ANNANNULUS_H

#include <casa/aips.h>
#include <imageanalysis/Annotations/AnnRegion.h>

namespace casa {

// <summary>
// This class represents an annotation for an annular (in position coordinates) region specified
// in an ascii region file as proposed in CAS-2285. It is specified by its center position
// and inner and outer radii.
// </summary>
// <author>Dave Mehringer</author>
// <use visibility=export>
// <reviewed reviewer="" date="yyyy/mm/dd" tests="" demos="">
// </reviewed>
// <prerequisite>

// </prerequisite>

// <etymology>
// Holds the specification of a annotation for an annular region as specified in ASCII format.
// Specified by center position and inner and outer radii.
// </etymology>

// <synopsis>
// This class represents a annotation for an annular region in specified by
// center and inner and outer radii.
// </synopsis>


class AnnAnnulus: public AnnRegion {

public:

	// avoid using the default constructor, it's behavior is subject to change
	AnnAnnulus();

	AnnAnnulus(
		const Quantity& xcenter,
		const Quantity& ycenter,
		const Quantity& innerRadius,
		const Quantity& outerRadius,
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
	// all frequencies and all polarizations are used (these can be set after construction).
	// xcenter and ycenter
	// must be in the same frame as the csys direction coordinate.
	// is a region (not just an annotation), although this value can be changed after
	// construction.
	AnnAnnulus(
		const Quantity& xcenter,
		const Quantity& ycenter,
		const Quantity& innerRadius,
		const Quantity& outerRadius,
		const CoordinateSystem& csys,
		const IPosition& imShape,
		const Vector<Stokes::StokesTypes>& stokes
	);

	// the default copy constructor and destructor are fine

	AnnAnnulus& operator=(const AnnAnnulus& other);

	Bool operator==(const AnnAnnulus& other);


	// get the center position, converted to the frame
	// of the input coordinate system if necessary
	MDirection getCenter() const;

	// get the radii in angular units. The first
	// value will be the inner radius, the second
	// the outer
	Vector<Quantity> getRadii() const;

	virtual ostream& print(ostream &os) const;

private:
	Vector<Quantity> _convertedRadii;
	Quantity _xcenter, _ycenter, _innerRadius, _outerRadius;

	void _init();
};

}

#endif /* ANNANNULUS_H */
