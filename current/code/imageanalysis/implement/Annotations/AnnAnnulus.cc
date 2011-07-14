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
	const Quantity& beginFreq,
	const Quantity& endFreq,
	const String& freqRefFrameString,
	const String& dopplerString,
	const Quantity& restfreq,
	const Vector<Stokes::StokesTypes> stokes,
	const Bool annotationOnly
) : AnnRegion(
		ANNULUS, dirRefFrameString, csys, beginFreq,
		endFreq, freqRefFrameString, dopplerString,
		restfreq, stokes, annotationOnly
	), _convertedRadii(Vector<Quantity>(2)),
	_xcenter(xcenter), _ycenter(ycenter),
	_innerRadius(innerRadius), _outerRadius(outerRadius) {

	_convertedRadii[0] = _lengthToAngle(innerRadius, _directionAxes[0]);
	_convertedRadii[1] = _lengthToAngle(outerRadius, _directionAxes[0]);

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
	inputCenter[0] = xcenter;
	inputCenter[1] = ycenter;

	_checkAndConvertDirections(String(__FUNCTION__), inputCenter);

	Vector<Double> coords = _convertedDirections[0].getAngle("rad").getValue();

	Vector<Quantity> qCenter(2);
	qCenter[0] = Quantity(coords[0], "rad");
	qCenter[1] = Quantity(coords[1], "rad");

	WCEllipsoid inner(qCenter, innerRadius, _directionAxes, _csys, RegionType::Abs);
	WCEllipsoid outer(qCenter, outerRadius, _directionAxes, _csys, RegionType::Abs);
	WCDifference annulus(outer, inner);
	_extend(annulus);

}

MDirection AnnAnnulus::getCenter() const {
	return _convertedDirections[0];
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

}
