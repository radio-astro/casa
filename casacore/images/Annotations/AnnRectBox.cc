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


#include <images/Annotations/AnnRectBox.h>

namespace casa {

AnnRectBox::AnnRectBox(
	const Quantity& blcx,
	const Quantity& blcy,
	const Quantity& trcx,
	const Quantity& trcy,
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
		RECT_BOX, dirRefFrameString, csys,
		beginFreq, endFreq, freqRefFrameString,
		dopplerString, restfreq, stokes,
		annotationOnly
	  ), _inputCorners(Matrix<Quantity>(2, 2)) {

	_inputCorners(0, 0) = blcx;
	_inputCorners(1, 0) = blcy;
	_inputCorners(0, 1) = trcx;
	_inputCorners(1, 1) = trcy;
	_checkAndConvertDirections(String(__FUNCTION__), _inputCorners);
	Vector<Int> absrel(2,(Int)RegionType::Abs);

	Vector<Quantity> qblc(2);
	Vector<Quantity> qtrc(2);
	for (uInt i=0; i<2; i++) {
		qblc[i] = Quantity(
			_convertedDirections[0].getAngle("rad").getValue("rad")[i],
			"rad"
		);
		qtrc[i] = Quantity(
			_convertedDirections[1].getAngle("rad").getValue("rad")[i],
			"rad"
		);
	}

	WCBox box(qblc, qtrc, _directionAxes, _csys, absrel);
	_extend(box);

}

Vector<MDirection> AnnRectBox::getCorners() const {
	return _convertedDirections;
}

ostream& AnnRectBox::print(ostream &os) const {
	_printPrefix(os);
	os << "box [[" << _inputCorners(0, 0) << ", "
		<< _inputCorners(1, 0) << "], ["
		<< _inputCorners(0, 1) << ", "
		<< _inputCorners(1, 1) << "]]";
	_printPairs(os);
	return os;
}



}
