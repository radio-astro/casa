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


#include <imageanalysis/Annotations/AnnCircle.h>

#include <images/Regions/WCEllipsoid.h>

namespace casa {

AnnCircle::AnnCircle(
	const Quantity& xcenter,
	const Quantity& ycenter,
	const Quantity& radius,
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
		CIRCLE, dirRefFrameString, csys, imShape, beginFreq,
		endFreq, freqRefFrameString, dopplerString,
		restfreq, stokes, annotationOnly
), _inputCenter(AnnotationBase::Direction(1)), _inputRadius(radius) {
	_init(xcenter, ycenter);
}

AnnCircle::AnnCircle(
	const Quantity& xcenter,
	const Quantity& ycenter,
	const Quantity& radius,
	const CoordinateSystem& csys,
	const IPosition& imShape,
	const Vector<Stokes::StokesTypes>& stokes
) : AnnRegion(CIRCLE, csys, imShape, stokes),
	_inputCenter(AnnotationBase::Direction(1)), _inputRadius(radius) {
	_init(xcenter, ycenter);
}

AnnCircle& AnnCircle::operator= (
	const AnnCircle& other
) {
    if (this == &other) {
    	return *this;
    }
    AnnRegion::operator=(other);
    _inputCenter.resize(other._inputCenter.nelements());
    _inputCenter = other._inputCenter;
    _inputRadius = other._inputRadius;
    _convertedRadius = other._convertedRadius;
    return *this;
}


MDirection AnnCircle::getCenter() const {
	return _getConvertedDirections()[0];
}

Quantity AnnCircle::getRadius() const {
	return _convertedRadius;
}

ostream& AnnCircle::print(ostream &os) const {
	_printPrefix(os);
	os << "circle [[" << _inputCenter[0].first << ", "
		<< _inputCenter[0].second << "], " << _inputRadius << "]";
	_printPairs(os);
	return os;
}

void AnnCircle::worldBoundingBox(
	vector<Quantity>& blc, vector<Quantity>& trc
) const {
	const CoordinateSystem csys = getCsys();
	Vector<Double> inc = csys.increment();
	IPosition dirAxes = _getDirectionAxes();
	Int xdir = inc[dirAxes[0]] >= 0 ? 1 : -1;
	Int ydir = inc[dirAxes[1]] >= 0 ? 1 : -1;
	String xUnit = csys.worldAxisUnits()[0];
	String yUnit = csys.worldAxisUnits()[0];
	Quantum<Vector<Double> > centerCoord = _getConvertedDirections()[0].getAngle("rad");

	Quantity blcx = Quantity(centerCoord.getValue()[0], centerCoord.getUnit())
		- (Quantity(xdir,"")*_convertedRadius);
	blcx.convert(xUnit);
	Quantity blcy = Quantity(centerCoord.getValue()[1], centerCoord.getUnit())
		- Quantity(ydir,"")*_convertedRadius;
	blcy.convert(yUnit);
	Quantity trcx = Quantity(centerCoord.getValue()[0], centerCoord.getUnit())
		+ Quantity(xdir,"")*_convertedRadius;
	trcx.convert(xUnit);
	Quantity trcy = Quantity(centerCoord.getValue()[1], centerCoord.getUnit())
		+ Quantity(ydir,"")*_convertedRadius;
	trcy.convert(yUnit);
	blc.resize(2);
	trc.resize(2);
	blc[0] = blcx;
	blc[1] = blcy;
	trc[0] = trcx;
	trc[1] = trcy;
}


void AnnCircle::_init(const Quantity& xcenter, const Quantity& ycenter) {
	_convertedRadius = _lengthToAngle(_inputRadius, _getDirectionAxes()[0]);
	_inputCenter[0].first = xcenter;
	_inputCenter[0].second = ycenter;

	_checkAndConvertDirections(String(__FUNCTION__), _inputCenter);

	Vector<Double> coords = _getConvertedDirections()[0].getAngle("rad").getValue();

	Vector<Quantity> center(2);

	center[0] = Quantity(coords[0], "rad");
	center[1] = Quantity(coords[1], "rad");

	WCEllipsoid circle(
		center, _convertedRadius, _getDirectionAxes(),
		getCsys(), RegionType::Abs
	);
	_setDirectionRegion(circle);
	_extend();
}

}
