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


#include <imageanalysis/Annotations/AnnRectBox.h>

#include <coordinates/Coordinates/DirectionCoordinate.h>

namespace casa {

AnnRectBox::AnnRectBox(
	const Quantity& blcx,
	const Quantity& blcy,
	const Quantity& trcx,
	const Quantity& trcy,
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
		RECT_BOX, dirRefFrameString, csys, imShape,
		beginFreq, endFreq, freqRefFrameString,
		dopplerString, restfreq, stokes,
		annotationOnly
	  ), _inputCorners(AnnotationBase::Direction(2)) {
	_init(blcx, blcy, trcx, trcy);
}

AnnRectBox::AnnRectBox(
	const Quantity& blcx,
	const Quantity& blcy,
	const Quantity& trcx,
	const Quantity& trcy,
	const CoordinateSystem& csys,
	const IPosition& imShape,
	const Vector<Stokes::StokesTypes>& stokes
) : AnnRegion(RECT_BOX, csys, imShape, stokes),
	_inputCorners(AnnotationBase::Direction(2)) {
	_init(blcx, blcy, trcx, trcy);
}

AnnRectBox& AnnRectBox::operator= (
	const AnnRectBox& other
) {
    if (this == &other) {
    	return *this;
    }
    AnnRegion::operator=(other);
    _inputCorners.resize(other._inputCorners.shape());
    _inputCorners = other._inputCorners;
    return *this;
}

Vector<MDirection> AnnRectBox::getCorners() const {
	return _getConvertedDirections();
}

ostream& AnnRectBox::print(ostream &os) const {
	_printPrefix(os);
	os << "box [[" << _inputCorners[0].first << ", "
		<< _inputCorners[0].second << "], ["
		<< _inputCorners[1].first << ", "
		<< _inputCorners[1].second << "]]";
	_printPairs(os);
	return os;
}

void AnnRectBox::worldBoundingBox(
	vector<Quantity>& blc, vector<Quantity>& trc
) const {
	Vector<MDirection> corners = _getConvertedDirections();
	Quantum<Vector<Double> > wblc = corners[0].getAngle("rad");
	Quantum<Vector<Double> > wtrc = corners[1].getAngle("rad");
	blc.resize(2);
	trc.resize(2);

	blc[0] = Quantity(wblc.getValue()[0], wblc.getUnit());
	blc[1] = Quantity(wblc.getValue()[1], wblc.getUnit());

	trc[0] = Quantity(wtrc.getValue()[0], wblc.getUnit());
	trc[1] = Quantity(wtrc.getValue()[1], wblc.getUnit());
}
void AnnRectBox::_init(
	const Quantity& blcx, const Quantity& blcy,
	const Quantity& trcx, const Quantity& trcy
) {
	_inputCorners[0].first = blcx;
	_inputCorners[0].second = blcy;
	_inputCorners[1].first = trcx;
	_inputCorners[1].second = trcy;
	_checkAndConvertDirections(String(__FUNCTION__), _inputCorners);
	Vector<Int> absrel(2,(Int)RegionType::Abs);
	Vector<Quantity> qblc(2);
	Vector<Quantity> qtrc(2);
	for (uInt i=0; i<2; i++) {
		qblc[i] = Quantity(
			_getConvertedDirections()[0].getAngle("rad").getValue("rad")[i],
			"rad"
		);
		qtrc[i] = Quantity(
			_getConvertedDirections()[1].getAngle("rad").getValue("rad")[i],
			"rad"
		);
	}
	// we just want a 2-d coordinate system for creating our box. We'll extend
	// it after.
	CoordinateSystem csys;
	csys.addCoordinate(*getCsys().directionCoordinate().clone());
	WCBox box(qblc, qtrc, _getDirectionAxes(), csys, absrel);
	_setDirectionRegion(box);
	_extend();
}


}
