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

#ifndef IMAGES_ASCIIANNOTATIONFILEPARSER_H
#define IMAGES_ASCIIANNOTATIONFILEPARSER_H

#include <casa/aips.h>
#include <casa/Arrays/Vector.h>
#include <casa/Containers/Record.h>
#include <casa/Logging/LogIO.h>
#include <casa/OS/RegularFile.h>
#include <casa/Utilities/Regex.h>
#include <imageanalysis/Annotations/AnnotationBase.h>
#include <imageanalysis/IO/AsciiAnnotationFileLine.h>


#include <coordinates/Coordinates/CoordinateSystem.h>

namespace casa {

// <summary>
// Parse and store regions and annotations from an ascii region file
// </summary>
// <author>Dave Mehringer</author>
// <use visibility=export>
// <reviewed reviewer="" date="yyyy/mm/dd" tests="" demos="">
// </reviewed>
// <prerequisite>

// </prerequisite>

// <etymology>
// This is a class designed to parse and store regions and annotations from an ascii region file
// </etymology>

// <synopsis>
// This class is for parsing and storing regions and annotations from an ascii region file .
// See the region file format proposal attached to CAS-2285 (https://bugs.nrao.edu/browse/CAS-2285)
// </synopsis>
// <note>
// This class will create AnnotationBase pointers via new(). It is assumed the caller will
// make use of these pointers so they are not deleted upon deletion of the object. It is
// the caller's responsibility to delete them. To do so, call getLines() and loop through
// the returned Vector of AsciiRegionLines. For objects of type AsciiRegionLines::ANNOTATION,
// get the pointer and delete it.

class RegionTextParser {

public:

	static const Int CURRENT_VERSION;
	static const Regex MAGIC;

	// because of nonstandard access patterns, be careful when using ParamValue and ParamSet
	// outside this class. These should probably be made into full fledged classes at some
	// point.
	struct ParamValue {
		Double doubleVal;
		Int intVal;
		String stringVal;
		Bool boolVal;
		AnnotationBase::LineStyle lineStyleVal;
		AnnotationBase::FontStyle fontStyleVal;
		Vector<MFrequency> freqRange;
		Vector<Stokes::StokesTypes> stokes;
		AnnotationBase::RGB color;
		vector<Int> intVec;
	};

	typedef std::map<AnnotationBase::Keyword, ParamValue> ParamSet;

	// <group>
	// differentiating between the filename and simple text constructors
	RegionTextParser(
		const String& filename, const CoordinateSystem& csys,
		const IPosition& imShape,
		const Int requireAtLeastThisVersion
	);

	RegionTextParser(
		const CoordinateSystem& csys, const IPosition& imShape, const String& text
	);
	//</group>

	~RegionTextParser();

	Int getFileVersion() const;

	Vector<AsciiAnnotationFileLine> getLines() const;

	// get the parameter set from a line of <src>text</src>. <src>preamble</src> is prepended to exception messages.
	static ParamSet getParamSet(
		Bool& spectralParmsUpdated,
		LogIO& log, const String& text, const String& preamble,
		const CoordinateSystem& csys
	);

private:

	const static String sOnePair;
	const static String bTwoPair;
	const static String sNPair;
	const static Regex startOnePair;
	const static Regex startNPair;

	CoordinateSystem _csys;
	auto_ptr<LogIO> _log;
	ParamSet _currentGlobals;
	Vector<AsciiAnnotationFileLine> _lines;
	Vector<AnnotationBase::Keyword> _globalKeysToApply;
	Int _fileVersion;
	IPosition _imShape;
	uInt _regions;

	RegionTextParser() {}

	RegionTextParser& operator=(const RegionTextParser&);

	void _parse(const String& contents, const String& fileDesc);

	Array<String> _extractTwoPairs(uInt& end, const String& string) const;

	// extract s1 and s2 from a string of the form "[s1, s2]"
	static Vector<String> _extractSinglePair(const String& string);

	void _addLine(const AsciiAnnotationFileLine& line);

	AnnotationBase::Type _getAnnotationType(
		Vector<Quantity>& qDirs,
		vector<Quantity>& qunatities,
		String& textString,
		String& consumeMe, const String& preamble
	) const;

	ParamSet _getCurrentParamSet(
		Bool& spectralParmsUpdated, ParamSet& newParams,
		String& consumeMe, const String& preamble
	) const;

	void _createAnnotation(
		const AnnotationBase::Type annType,
		//const Vector<MDirection> dirs,
		const Vector<Quantity>& qDirs,
		const Vector<Quantity>& qFreqs,
		const vector<Quantity>& quantities,
		const String& textString,
		const ParamSet& currentParamSet,
		const Bool annOnly, const Bool isDifference,
		const String& preamble
	);

	Vector<Quantity> _quantitiesFromFrequencyString(
		const String& freqString,
		const String& preamble
	) const;

	static String _doLabel(String& consumeMe, const String& logPreamble);

	static String _getKeyValue(String& consumeMe, const String& preamble);

	Vector<Quantity> _extractQuantityPairAndSingleQuantity(
		String& consumeMe, const String& preamble
	) const;

	Vector<Quantity> _extractNQuantityPairs(
			String& consumeMe, const String& preamble
	) const;

	Vector<Quantity> _extractTwoQuantityPairs(
		String& consumeMe, const String& preamble
	) const;

	Vector<Quantity> _extractSingleQuantityPair(
		const String& pair, const String& preamble
	) const;

	void _setInitialGlobals();

	static Vector<Stokes::StokesTypes> _stokesFromString(
		const String& stokes, const String& preamble
	);

	Vector<Quantity> _extractTwoQuantityPairsAndSingleQuantity(
		String& consumeMe, const String& preamble
	) const;

	void _extractQuantityPairAndString(
		Vector<Quantity>& quantities, String& string,
		String& consumeMe, const String& preamble,
		const Bool requireQuotesAroundString
	) const;

	Vector<Quantity> _extractQuantitiesFromPair(
		const String& pair, const String& preamble
	) const;

	void _determineVersion(
		const String& chunk, const String& filename,
		const Int requireAtLeastThisVersion
	);


};
}

#endif
