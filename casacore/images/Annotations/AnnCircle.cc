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


#include <images/Annotations/AnnCircle.h>

#include <images/Regions/WCEllipsoid.h>

namespace casa {

AnnCircle::AnnCircle(
	const Quantity& xcenter,
	const Quantity& ycenter,
	const Quantity& radius,
	const String& dirRefFrameString,
	const CoordinateSystem& csys,
	const Quantity& beginFreq,
	const Quantity& endFreq,
	const String& freqRefFrameString,
	const String& dopplerString,
	const Quantity& restfreq,
	const Vector<Stokes::StokesTypes> stokes,
	const Bool annotationOnly
) : AnnRegion(
		CIRCLE, dirRefFrameString, csys, beginFreq,
		endFreq, freqRefFrameString, dopplerString,
		restfreq, stokes, annotationOnly
), _inputCenter(Vector<Quantity>(2)), _inputRadius(radius) {

	_convertedRadius = _lengthToAngle(_inputRadius, _directionAxes[0]);
	_inputCenter[0] = xcenter;
	_inputCenter[1] = ycenter;

	_checkAndConvertDirections(String(__FUNCTION__), _inputCenter);

	Vector<Double> coords = _convertedDirections[0].getAngle("rad").getValue();

	Vector<Quantity> center(2);

	center[0] = Quantity(coords[0], "rad");
	center[1] = Quantity(coords[1], "rad");

	WCEllipsoid circle(
		center, _convertedRadius, _directionAxes, _csys,
		RegionType::Abs
	);
	_extend(circle);

	ostringstream os;
	os << "circle [[" << xcenter << ", "
		<< ycenter << "], " << radius << "]";
	_stringRep += os.str();
}

MDirection AnnCircle::getCenter() const {
	return _convertedDirections[0];
}

Quantity AnnCircle::getRadius() const {
	return _convertedRadius;
}

}
