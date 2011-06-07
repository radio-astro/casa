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

#ifndef IMAGES_ASCIIANNOTATIONLIST_H
#define IMAGES_ASCIIANNOTATIONLIST_H

#include <casa/aips.h>
#include <casa/Arrays/Vector.h>
#include <coordinates/Coordinates/CoordinateSystem.h>
#include <images/IO/AsciiAnnotationFileLine.h>

namespace casa {

// <summary>
// A list of regions and annotations and comments representing an ascii region file.
// </summary>
// <author>Dave Mehringer</author>
// <use visibility=export>
// <reviewed reviewer="" date="yyyy/mm/dd" tests="" demos="">
// </reviewed>
// <prerequisite>

// </prerequisite>

// <etymology>
// A list of regions and annotations and comments representing an ascii region file.
// </etymology>

// <synopsis>
// A list of regions and annotations and comments representing an ascii region file.
// See the region file format proposal attached to CAS-2285 (https://bugs.nrao.edu/browse/CAS-2285)
// </synopsis>

class AsciiAnnotationList {

public:

	// <group>
	// create an empty list.
	AsciiAnnotationList(const Bool deletePointersOnDestruct=True);

	// create a list by reading it from a file.
	// An exception is thrown if the file is not in the correct
	// format or does not exist. The coordinate system is used for
	// setting defaults and reference frames to be used.
	AsciiAnnotationList(
		const String& filename, const CoordinateSystem& csys,
		const Bool deletePointersOnDestruct=True
	);
	//</group>

	~AsciiAnnotationList();

	// add a line to the end of the list
	void addLine(const AsciiAnnotationFileLine& line);

	// number of lines in the list
	uInt nLines() const;

	// get the line at the specified position
	AsciiAnnotationFileLine lineAt(const uInt i) const;

	// get all lines in the list
	Vector<AsciiAnnotationFileLine> getLines() const;

	ostream& print(ostream& os) const;

private:
	Vector<AsciiAnnotationFileLine> _lines;
	Bool _deletePointersOnDestruct;

};

inline ostream &operator<<(ostream& os, const AsciiAnnotationList& list) {
	return list.print(os);
};

}



#endif /* IMAGES_ASCIIREGIONFILE_H_ */
