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


#include <imageanalysis/Annotations/AnnPolygon.h>

#include <images/Regions/WCPolygon.h>

namespace casa {

AnnPolygon::AnnPolygon(
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
) : AnnRegion(
		POLYGON, dirRefFrameString, csys, imShape, beginFreq,
		endFreq, freqRefFrameString, dopplerString,
		restfreq, stokes, annotationOnly
), _origXPos(xPositions), _origYPos(yPositions) {
	_init();
}

AnnPolygon::AnnPolygon(
	const Vector<Quantity>& xPositions,
	const Vector<Quantity>& yPositions,
	const CoordinateSystem& csys,
	const IPosition& imShape,
	const Vector<Stokes::StokesTypes>& stokes
) : AnnRegion(POLYGON, csys, imShape, stokes),
	_origXPos(xPositions), _origYPos(yPositions) {
	_init();
}

AnnPolygon& AnnPolygon::operator= (
	const AnnPolygon& other
) {
    if (this == &other) {
    	return *this;
    }
    AnnRegion::operator=(other);
    _corners.resize(other._corners.nelements());
    _corners = other._corners;
    _origXPos.resize(other._origXPos.nelements());
    _origXPos = other._origXPos;
    _origYPos.resize(other._origYPos.nelements());
    _origYPos = other._origYPos;
    return *this;
}

Vector<MDirection> AnnPolygon::getCorners() const {
	return _getConvertedDirections();
}

ostream& AnnPolygon::print(ostream &os) const {
	_printPrefix(os);
	os << "poly [";
	for (uInt i=0; i<_origXPos.size(); i++) {
		os << "[" << _origXPos[i] << ", " << _origYPos[i] << "]";
		if (i < _origXPos.size()-1) {
			os << ", ";
		}
	}
	os << "]";
	_printPairs(os);
	return os;
}

void AnnPolygon::_init() {
	String preamble(String(__FUNCTION__) + ": ");
	if (_origXPos.size() != _origYPos.size()) {
		throw AipsError(
			preamble + "x and y vectors are not the same length but must be."
		);
	}

	Matrix<Quantity> corners(2, _origXPos.size());
	for (uInt i=0; i<_origXPos.size(); i++) {
		corners(0, i) = _origXPos[i];
		corners(1, i) = _origYPos[i];
	}
	_checkAndConvertDirections(String(__FUNCTION__), corners);
	Vector<Double> xv(_origXPos.size()), yv(_origYPos.size());
	for (uInt i=0; i<xv.size(); i++) {
		Vector<Double> coords = _getConvertedDirections()[i].getAngle("rad").getValue();
		xv[i] = coords[0];
		yv[i] = coords[1];
	}

	Quantum<Vector<Double> > x(xv, "rad");
	Quantum<Vector<Double> > y(yv, "rad");

	WCPolygon wpoly(
		x, y, IPosition(_getDirectionAxes()),
		_getCsys(), RegionType::Abs
	);
	_setDirectionRegion(wpoly);
	_extend();
}


}
