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


#include <imageanalysis/Annotations/AnnAnnulus.h>

#include <images/Regions/WCDifference.h>
#include <images/Regions/WCEllipsoid.h>


namespace casa {

AnnAnnulus::AnnAnnulus(
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
) : AnnRegion(
		ANNULUS, dirRefFrameString, csys, imShape, beginFreq,
		endFreq, freqRefFrameString, dopplerString,
		restfreq, stokes, annotationOnly
	), _convertedRadii(Vector<Quantity>(2)),
	_xcenter(xcenter), _ycenter(ycenter),
	_innerRadius(innerRadius), _outerRadius(outerRadius) {
	_init();
}

AnnAnnulus::AnnAnnulus(
	const Quantity& xcenter,
	const Quantity& ycenter,
	const Quantity& innerRadius,
	const Quantity& outerRadius,
	const CoordinateSystem& csys,
	const IPosition& imShape,
	const Vector<Stokes::StokesTypes>& stokes
) : AnnRegion(ANNULUS, csys, imShape, stokes),
_convertedRadii(Vector<Quantity>(2)),
	_xcenter(xcenter), _ycenter(ycenter),
	_innerRadius(innerRadius), _outerRadius(outerRadius) {
	_init();
}

AnnAnnulus& AnnAnnulus::operator= (
	const AnnAnnulus& other
) {
    if (this == &other) {
    	return *this;
    }
    AnnRegion::operator=(other);
	_convertedRadii.resize(other._convertedRadii.nelements());
	_convertedRadii = other._convertedRadii;
	_xcenter = other._xcenter;
	_ycenter = other._ycenter;
	_innerRadius = other._innerRadius;
	_outerRadius = other._outerRadius;
    return *this;
}


MDirection AnnAnnulus::getCenter() const {
	return _getConvertedDirections()[0];
}

Vector<Quantity> AnnAnnulus::getRadii() const {
	return _convertedRadii;
}

ostream& AnnAnnulus::print(ostream &os) const {
	_printPrefix(os);
	os << "annulus [[" << _xcenter << ", " << _ycenter << "], ["
		<< _innerRadius << ", " << _outerRadius << "]] ";
	_printPairs(os);
	return os;
}

void AnnAnnulus::worldBoundingBox(
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
		- (Quantity(xdir,"")*_convertedRadii[1]);
	blcx.convert(xUnit);
	Quantity blcy = Quantity(centerCoord.getValue()[1], centerCoord.getUnit())
		- Quantity(ydir,"")*_convertedRadii[1];
	blcy.convert(yUnit);
	Quantity trcx = Quantity(centerCoord.getValue()[0], centerCoord.getUnit())
		+ Quantity(xdir,"")*_convertedRadii[1];
	trcx.convert(xUnit);
	Quantity trcy = Quantity(centerCoord.getValue()[1], centerCoord.getUnit())
		+ Quantity(ydir,"")*_convertedRadii[1];
	trcy.convert(yUnit);
	blc.resize(2);
	trc.resize(2);
	blc[0] = blcx;
	blc[1] = blcy;
	trc[0] = trcx;
	trc[1] = trcy;
}

void AnnAnnulus::_init() {
	_convertedRadii[0] = _lengthToAngle(_innerRadius, _getDirectionAxes()[0]);
	_convertedRadii[1] = _lengthToAngle(_outerRadius, _getDirectionAxes()[0]);

	if (
		_convertedRadii[0].getValue("rad")
		>= _convertedRadii[1].getValue("rad")
	) {
		throw AipsError(
			String(__FUNCTION__)
			+ "Inner radius must be less than "
			+ "outer radius"
		);
	}

	Vector<Quantity> inputCenter(2);
	inputCenter[0] = _xcenter;
	inputCenter[1] = _ycenter;

	_checkAndConvertDirections(String(__FUNCTION__), inputCenter);

	Vector<Double> coords = _getConvertedDirections()[0].getAngle("rad").getValue();

	Vector<Quantity> qCenter(2);
	qCenter[0] = Quantity(coords[0], "rad");
	qCenter[1] = Quantity(coords[1], "rad");
	WCEllipsoid inner(qCenter, _innerRadius, _getDirectionAxes(), getCsys(), RegionType::Abs);
	WCEllipsoid outer(qCenter, _outerRadius, _getDirectionAxes(), getCsys(), RegionType::Abs);
	WCDifference annulus(outer, inner);
	_setDirectionRegion(annulus);
	_extend();

}

}
