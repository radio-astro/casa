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


#include <images/Annotations/AnnCenterBox.h>

namespace casa {

AnnCenterBox::AnnCenterBox(
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
) :  AnnRegion(
		CENTER_BOX, dirRefFrameString, csys, beginFreq,
		endFreq, freqRefFrameString, dopplerString,
		restfreq, stokes, annotationOnly
	), _widths(Vector<Quantity>(2)),
	_corners(Vector<MDirection>(2)){
	if (! xwidth.isConform("rad") && ! xwidth.isConform("pix")) {
		throw AipsError(
			"x width unit " + xwidth.getUnit() + " is not an angular unit."
		);
	}
	if (! ywidth.isConform("rad") && ! ywidth.isConform("pix")) {
		throw AipsError(
			"y width unit " + ywidth.getUnit() + " is not an angular unit."
		);
	}
	_widths[0] = _lengthToAngle(xwidth, _directionAxes[0]);
	_widths[1] = _lengthToAngle(ywidth, _directionAxes[1]);

	Vector<Quantity> center(2);
	center[0] = centerx;
	center[1] = centery;
	_checkAndConvertDirections(String(__FUNCTION__), center);

	_doCorners();
	MDirection blc = _corners[0];
	MDirection trc = _corners[1];

	// FIXME refactor this so it can be shared by RectBox
	Vector<Double> blcValues = blc.getAngle().getValue();
	Vector<Quantity> qblc(2);

	Vector<Double> trcValues = trc.getAngle().getValue();
	Vector<Quantity> qtrc(2);
	Vector<Int> absrel(2,(Int)RegionType::Abs);
	for (uInt i=0; i<qtrc.size(); i++) {
		qblc[i] = Quantity(blcValues[i], "rad");
		qtrc[i] = Quantity(trcValues[i], "rad");
	}

	WCBox box(qblc, qtrc, _directionAxes, _csys, absrel);
	_extend(box);
}

MDirection AnnCenterBox::getCenter() const {
	return _convertedDirections[0];
}

Vector<Quantity> AnnCenterBox::getWidths() const {
	return _widths;
}

void AnnCenterBox::_doCorners() {

	Vector<Double> inc = _csys.increment();

	Double xFactor = inc(_directionAxes[0]) > 0 ? 1.0 : -1.0;
	Double yFactor = inc(_directionAxes[1]) > 0 ? 1.0 : -1.0;

	Quantity xShift = Quantity(0.5*xFactor)*_widths[0];
	Quantity yShift = Quantity(0.5*yFactor)*_widths[1];

	MDirection blc = _convertedDirections[0];
	blc.shift(Quantity(-1)*xShift, Quantity(-1)*yShift);
	MDirection trc = _convertedDirections[0];
	trc.shift(xShift, yShift);
	_corners[0] = blc;
	_corners[1] = trc;
}

Vector<MDirection> AnnCenterBox::getCorners() const {
	return _corners;
}

}
