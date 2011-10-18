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


#include <imageanalysis/Annotations/AnnRotBox.h>

#include <images/Regions/WCPolygon.h>

namespace casa {

AnnRotBox::AnnRotBox(
	const Quantity& xcenter,
	const Quantity& ycenter,
	const Quantity& xwidth,
	const Quantity& ywidth, const Quantity& positionAngle,
	const String& dirRefFrameString,
	const CoordinateSystem& csys, const IPosition& imShape,
const Quantity& beginFreq,
	const Quantity& endFreq, const String& freqRefFrameString,
	const String& dopplerString, const Quantity& restfreq,
	const Vector<Stokes::StokesTypes> stokes,
	const Bool annotationOnly
) : AnnRegion(
		ROTATED_BOX, dirRefFrameString, csys, imShape, beginFreq,
		endFreq, freqRefFrameString, dopplerString, restfreq,
		stokes, annotationOnly
	), _inputCenter(AnnotationBase::Direction(1)),
	_inputWidths(Vector<Quantity>(2)), _widths(Vector<Quantity>(2)),
	_positionAngle(positionAngle), _corners(Vector<MDirection>(4)) {
	_init(xcenter, ycenter, xwidth, ywidth);
}

AnnRotBox::AnnRotBox(
	const Quantity& xcenter,
	const Quantity& ycenter,
	const Quantity& xwidth,
	const Quantity& ywidth, const Quantity& positionAngle,
	const CoordinateSystem& csys, const IPosition& imShape,
	const Vector<Stokes::StokesTypes>& stokes
) : AnnRegion(ROTATED_BOX, csys, imShape, stokes),
	_inputCenter(AnnotationBase::Direction(1)),
	_inputWidths(Vector<Quantity>(2)), _widths(Vector<Quantity>(2)),
	_positionAngle(positionAngle), _corners(Vector<MDirection>(4)) {
	_init(xcenter, ycenter, xwidth, ywidth);
}

AnnRotBox& AnnRotBox::operator= (
	const AnnRotBox& other
) {
    if (this == &other) {
    	return *this;
    }
    AnnRegion::operator=(other);
    _inputCenter.resize(other._inputCenter.nelements());
    _inputCenter = other._inputCenter;
    _inputWidths.resize(other._inputWidths.nelements());
    _inputWidths = other._inputWidths;
    _widths.resize(other._widths.nelements());
    _widths = other._widths;
    _positionAngle = other._positionAngle;
    _corners.resize(other._corners.nelements());
    _corners = other._corners;
    return *this;
}

void AnnRotBox::_doCorners() {
	Quantity realAngle = _positionAngle;

	Vector<Double> inc = getCsys().increment();

	Double xFactor = inc(_getDirectionAxes()[0]) > 0 ? 1.0 : -1.0;
	Double yFactor = inc(_getDirectionAxes()[1]) > 0 ? 1.0 : -1.0;

	// assumes first direction axis is the longitudinal axis
    Vector<Quantity> xShift(2);
    Vector<Quantity> yShift(2);
    for (uInt i=0; i<2; i++) {
    	Double fac = i == 0 ? 1.0 : -1.0;
	    Double angleRad = _positionAngle.getValue("rad");
        xShift[i] = Quantity(fac*0.5*xFactor*cos(angleRad))*_widths[0] + Quantity(0.5*yFactor*sin(angleRad))*_widths[1];
	    yShift[i] = Quantity(-0.5*fac*xFactor*sin(angleRad))*_widths[0] + Quantity(0.5*yFactor*cos(angleRad))*_widths[1];
        _corners[i] = _getConvertedDirections()[0];
        _corners[i].shift(xShift[i], yShift[i]);
        _corners[i+2] = _getConvertedDirections()[0];
        _corners[i+2].shift(Quantity(-1)*xShift[i], Quantity(-1)*yShift[i]);
    }
}

Vector<MDirection> AnnRotBox::getCorners() const {
	return _corners;
}

void AnnRotBox::worldBoundingBox(
	vector<Quantity>& blc, vector<Quantity>& trc
) const {
	const CoordinateSystem csys = getCsys();
	Vector<Double> inc = csys.increment();
	IPosition dirAxes = _getDirectionAxes();
	Int xdir = inc[dirAxes[0]] >= 0 ? 1 : -1;
	Int ydir = inc[dirAxes[1]] >= 0 ? 1 : -1;
	String xUnit = csys.worldAxisUnits()[dirAxes[0]];
	String yUnit = csys.worldAxisUnits()[dirAxes[1]];
	vector<Quantum<Vector<Double> > > coords(_corners.size());
	coords[0] = _corners[0].getAngle("rad");
	Double xmin = coords[0].getValue(xUnit)[0];
	Double xmax = xmin;
	Double ymin = coords[0].getValue(yUnit)[1];
	Double ymax = ymin;

	for (uInt i=1; i<coords.size(); i++) {
		coords[i] = _corners[i].getAngle("rad");
		xmin = min(xmin, coords[i].getValue(xUnit)[0]);
		xmax = max(xmax, coords[i].getValue(xUnit)[0]);
		ymin = min(ymin, coords[i].getValue(yUnit)[1]);
		ymax = max(ymax, coords[i].getValue(yUnit)[1]);
	}

	blc.resize(2);
	trc.resize(2);
	blc[0] = xdir > 0 ? Quantity(xmin, xUnit) : Quantity(xmax, xUnit);
	blc[1] = ydir > 0 ? Quantity(ymin, yUnit) : Quantity(ymax, yUnit);
	trc[0] = xdir > 0 ? Quantity(xmax, xUnit) : Quantity(xmin, xUnit);
	trc[1] = ydir > 0 ? Quantity(ymax, yUnit) : Quantity(ymin, yUnit);
}

void AnnRotBox::worldCorners(vector<Quantity>& x, vector<Quantity>& y) const {
	const CoordinateSystem csys = getCsys();
	const IPosition dirAxes = _getDirectionAxes();
	String xUnit = csys.worldAxisUnits()[dirAxes[0]];
	String yUnit = csys.worldAxisUnits()[dirAxes[1]];
	x.resize(_corners.size());
	y.resize(_corners.size());
	for (uInt i=0; i<_corners.size(); i++) {
		x[i] = Quantity(_corners[i].getAngle(xUnit).getValue(xUnit)[0], xUnit);
		y[i] = Quantity(_corners[i].getAngle(yUnit).getValue(yUnit)[1], yUnit);
	}
}

void AnnRotBox::pixelCorners(vector<Double>& x, vector<Double>& y) const {
	vector<Quantity> xx, xy;
	worldCorners(xx, xy);

	const CoordinateSystem csys = getCsys();
	Vector<Double> world = csys.referenceValue();
	const IPosition dirAxes = _getDirectionAxes();
	String xUnit = csys.worldAxisUnits()[dirAxes[0]];
	String yUnit = csys.worldAxisUnits()[dirAxes[1]];

	x.resize(xx.size());
	y.resize(xx.size());

	for (uInt i=0; i<xx.size(); i++) {
		world[dirAxes[0]] = xx[i].getValue(xUnit);
		world[dirAxes[1]] = xy[i].getValue(yUnit);
		Vector<Double> pixel;
		csys.toPixel(pixel, world);
		x[i] = pixel[dirAxes[0]];
		y[i] = pixel[dirAxes[1]];
	}
}


ostream& AnnRotBox::print(ostream &os) const {
	_printPrefix(os);
	os << "rotbox [[" << _inputCenter[0].first << ", "
		<< _inputCenter[0].second << "], [" << _inputWidths[0]
		<< ", " << _inputWidths[1] << "], "
		<< _positionAngle << "]";
	_printPairs(os);
	return os;
}

void AnnRotBox::_init(
	const Quantity& xcenter, const Quantity& ycenter,
	const Quantity& xwidth, const Quantity& ywidth
) {
	String preamble(String(__FUNCTION__) + ": ");

	if (! xwidth.isConform("rad") && ! xwidth.isConform("pix")) {
		throw AipsError(
			preamble + "x width unit " + xwidth.getUnit() + " is not an angular unit."
		);
	}
	if (! ywidth.isConform("rad") && ! ywidth.isConform("pix")) {
		throw AipsError(
			preamble + "y width unit " + ywidth.getUnit() + " is not an angular unit."
		);
	}
	_inputWidths[0] = xwidth;
	_inputWidths[1] = ywidth;

	_widths[0] = _lengthToAngle(xwidth, _getDirectionAxes()[0]);
	_widths[1] = _lengthToAngle(ywidth, _getDirectionAxes()[1]);

	_inputCenter[0].first = xcenter;
	_inputCenter[0].second = ycenter;
	_checkAndConvertDirections(String(__FUNCTION__), _inputCenter);

	_doCorners();
	Vector<Double> xv(4), yv(4);
	for (uInt i=0; i<4; i++) {
		Vector<Double> coords = _corners[i].getAngle("rad").getValue();
		xv[i] = coords[0];
		yv[i] = coords[1];
	}
	Quantum<Vector<Double> > x(xv, "rad");
	Quantum<Vector<Double> > y(yv, "rad");
	WCPolygon box(x, y, _getDirectionAxes(), getCsys(), RegionType::Abs);
	_setDirectionRegion(box);
	_extend();
}

}
