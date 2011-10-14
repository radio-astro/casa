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

#ifndef ANNOTATIONS_REGIONTEXTLIST_H
#define ANNOTATIONS_REGIONTEXTLIST_H

#include <casa/aips.h>
#include <casa/Arrays/Vector.h>
#include <coordinates/Coordinates/CoordinateSystem.h>
#include <imageanalysis/IO/AsciiAnnotationFileLine.h>
#include <imageanalysis/IO/RegionTextParser.h>
#include <images/Regions/WCRegion.h>
#include <images/Regions/WCUnion.h>

namespace casa {

// <summary>
// An ordered list of annotations and comments representing an ascii region file.
// </summary>
// <author>Dave Mehringer</author>
// <use visibility=export>
// <reviewed reviewer="" date="yyyy/mm/dd" tests="" demos="">
// </reviewed>
// <prerequisite>

// </prerequisite>

// <etymology>
// An order list of annotations and comments representing an ascii region file.
// </etymology>

// <synopsis>
// A list of regions and annotations and comments representing an ascii region file.
// See the region file format proposal attached to CAS-2285 (https://bugs.nrao.edu/browse/CAS-2285)
// </synopsis>

class RegionTextList {

public:

	// <group>
	// create an empty list which can be appended to. This constructor
	// is used for constructing an annotation list on the fly, possibly
	// to be written to a file when complete. Do not use this constructor
	// if you want to determine the final composite region.
	RegionTextList(const Bool deletePointersOnDestruct=True);

	// create an empty list which can be appended to. This constructor
	// is used for constructing an annotation list on the fly, possibly
	// to be written to a file when complete. It can be used to determine
	// the composite region as well but it is the caller's responsibility
	// to ensure the regions added to this object are constructed
	// in a consistent manner (eg using the same coordinate system).
	// <src>shape</src> is the image shape and is only used if
	// the first region is a difference; in that case, the all pixels in entire
	// shape are set to good initially.
	RegionTextList(
		const CoordinateSystem& csys,
		const IPosition shape,
		const Bool deletePointersOnDestruct=True
	);

	// create a list by reading it from a file.
	// An exception is thrown if the file is not in the correct
	// format or does not exist. The coordinate system is used for
	// setting defaults and reference frames to be used.
	// <src>shape</src> is the image shape and is only used if
	// the first region is a difference; in that case, the all pixels in entire
	// shape are set to good initially.
	RegionTextList(
		const String& filename, const CoordinateSystem& csys,
		const IPosition shape,
		const Int requireAtLeastThisVersion=RegionTextParser::CURRENT_VERSION,
		const Bool deletePointersOnDestruct=True
	);

	// create a list by reading it from a text string.
	// An exception is thrown if the text is not in the correct
	// format. The coordinate system is used for
	// setting defaults and reference frames to be used.
	// <src>shape</src> is the image shape and is only used if
	// the first region is a difference; in that case, the all pixels in entire
	// shape are set to good initially.
	RegionTextList(
		const CoordinateSystem& csys, const String& text,
		const IPosition shape,
		const Bool deletePointersOnDestruct=True
	);
	//</group>

	~RegionTextList();

	// add a line to the end of the list
	void addLine(const AsciiAnnotationFileLine& line);

	// number of lines in the list
	uInt nLines() const;

	// get the line at the specified index
	AsciiAnnotationFileLine lineAt(const uInt i) const;

	// get all lines in the list
	inline const Vector<AsciiAnnotationFileLine>& getLines() const {
		return _lines;
	}

	ostream& print(ostream& os) const;

	// get the composite region.
	WCRegion* getRegion() const;

	// get the composite region as a region record.
	Record regionAsRecord() const;

private:
	Vector<AsciiAnnotationFileLine> _lines;
	Bool _deletePointersOnDestruct;
	PtrBlock<WCRegion *> _regions;
	CoordinateSystem _csys;
	IPosition _shape;
	Bool _canGetRegion;

};

inline ostream &operator<<(ostream& os, const RegionTextList& list) {
	return list.print(os);
};

}

#endif /* IMAGES_ASCIIREGIONFILE_H_ */
