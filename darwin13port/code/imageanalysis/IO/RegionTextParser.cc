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

#include <imageanalysis/IO/RegionTextParser.h>

#include <casa/IO/RegularFileIO.h>
#include <coordinates/Coordinates/DirectionCoordinate.h>
#include <coordinates/Coordinates/SpectralCoordinate.h>
#include <imageanalysis/Annotations/AnnAnnulus.h>
#include <imageanalysis/Annotations/AnnCenterBox.h>
#include <imageanalysis/Annotations/AnnCircle.h>
#include <imageanalysis/Annotations/AnnEllipse.h>
#include <imageanalysis/Annotations/AnnLine.h>
#include <imageanalysis/Annotations/AnnPolygon.h>
#include <imageanalysis/Annotations/AnnRectBox.h>
#include <imageanalysis/Annotations/AnnRotBox.h>
#include <imageanalysis/Annotations/AnnSymbol.h>
#include <imageanalysis/Annotations/AnnText.h>
#include <imageanalysis/Annotations/AnnVector.h>

#include <measures/Measures/MCDirection.h>
#include <measures/Measures/MDirection.h>
#include <measures/Measures/VelocityMachine.h>

#define _ORIGIN "RegionTextParser::" + String(__FUNCTION__) + ": "

namespace casa {

const Int RegionTextParser::CURRENT_VERSION = 0;
const Regex RegionTextParser::MAGIC("^#CRTF");

const String RegionTextParser::sOnePair = "[[:space:]]*\\[[^\\[,]+,[^\\[,]+\\][[:space:]]*";
const String RegionTextParser::bTwoPair = "\\[" + sOnePair
		+ "," + sOnePair;
// explicit onePair at the end because that one should not be followed by a comma
const String RegionTextParser::sNPair = "\\[(" + sOnePair
		+ ",)+" + sOnePair + "\\]";
const Regex RegionTextParser::startOnePair("^" + sOnePair);
const Regex RegionTextParser::startNPair("^" + sNPair);

RegionTextParser::RegionTextParser(
	const String& filename, const CoordinateSystem& csys,
	const IPosition& imShape,
	const Int requireAtLeastThisVersion
) : _csys(csys),
	_log(new LogIO()),
	_currentGlobals(ParamSet()),
	_lines(Vector<AsciiAnnotationFileLine>(0)),
	_globalKeysToApply(Vector<AnnotationBase::Keyword>(0)),
	_fileVersion(-1), _imShape(imShape), _regions(0) {
	RegularFile file(filename);
	if (! file.exists()) {
		throw AipsError(
			_ORIGIN + "File "
			+ filename + " does not exist."
		);
	}
	if (! file.isReadable()) {
		throw AipsError(
			_ORIGIN + "File "
			+ filename + " is not readable."
		);
	}
	if (! _csys.hasDirectionCoordinate()) {
		throw AipsError(
			_ORIGIN
			+ "Coordinate system does not have a direction coordintate"
		);
	}
	_setInitialGlobals();
	RegularFileIO fileIO(file);
	Int bufSize = 4096;
	char *buffer = new char[bufSize];
	int nRead;
	String contents;
	while ((nRead = fileIO.read(bufSize, buffer, False)) == bufSize) {
		String chunk(buffer, bufSize);
		if (_fileVersion < 0) {
			_determineVersion(chunk, filename, requireAtLeastThisVersion);
		}
		contents += chunk;
	}
	// get the last chunk
	String chunk(buffer, nRead);
	if (_fileVersion < 0) {
		_determineVersion(chunk, filename, requireAtLeastThisVersion);
	}
	contents += chunk;
	delete [] buffer;
	_parse(contents, filename);
}

RegionTextParser::RegionTextParser(
	const CoordinateSystem& csys, const IPosition& imShape,
	const String& text
) : _csys(csys),
	_log(new LogIO()),
	_currentGlobals(ParamSet()),
	_lines(Vector<AsciiAnnotationFileLine>(0)),
	_globalKeysToApply(Vector<AnnotationBase::Keyword>(0)),
	_fileVersion(-1), _imShape(imShape), _regions(0) {
	if (! _csys.hasDirectionCoordinate()) {
		throw AipsError(
			_ORIGIN
			+ "Coordinate system has no direction coordintate"
		);
	}
	_setInitialGlobals();
	_parse(text, "");
}

RegionTextParser::~RegionTextParser() {}

Int RegionTextParser::getFileVersion() const {
	if (_fileVersion < 0) {
		*_log << _ORIGIN << "File version not associated with simple text strings"
			<< LogIO::EXCEPTION;
	}
	return _fileVersion;
}

Vector<AsciiAnnotationFileLine> RegionTextParser::getLines() const {
	return _lines;
}

void RegionTextParser::_determineVersion(
	const String& chunk, const String& filename,
	const Int requireAtLeastThisVersion
) {
	*_log << LogOrigin("RegionTextParser", __FUNCTION__);
	if (_fileVersion >= 0) {
		// already determined
		return;
	}
	if (! chunk.contains(MAGIC)) {
		*_log << _ORIGIN << "File " << filename
			<< " does not contain CASA region text file magic value"
			<< LogIO::EXCEPTION;
	}
	Regex version = Regex(MAGIC.regexp() + "v[0-9]+");
	if (chunk.contains(version)) {
		String vString = chunk.substr(6);
		Bool done = False;
		Int count = 1;
		Int oldVersion = -2000;
		while (! done) {
			try {
				_fileVersion = String::toInt(vString.substr(0, count));
				count++;
				if (_fileVersion == oldVersion) {
					done = True;
				}
				else {
					oldVersion = _fileVersion;
				}
			}
			catch (AipsError) {
				done = True;
			}
		}
		if (_fileVersion < requireAtLeastThisVersion) {
			*_log << _ORIGIN << "File version " << _fileVersion
				<< " is less than required version "
				<< requireAtLeastThisVersion << LogIO::EXCEPTION;
		}
		if (_fileVersion > CURRENT_VERSION) {
			*_log << _ORIGIN << "File version " << _fileVersion
				<< " is greater than the most recent version of the spec ("
				<< CURRENT_VERSION
				<< "). Did you bring this file with you when you traveled "
				<< "here from the future perhaps? Unfortunately we don't "
				<< "support such possibilities yet." << LogIO::EXCEPTION;
		}
		*_log << LogIO::NORMAL << _ORIGIN << "Found spec version "
			<< _fileVersion << LogIO::POST;
	}
	else {
		*_log << LogIO::WARN << _ORIGIN << "File " << filename
			<< " does not contain a CASA Region Text File spec version. "
			<< "The current spec version, " << CURRENT_VERSION << " will be assumed. "
			<< "WE STRONGLY SUGGEST YOU INCLUDE THE SPEC VERSION IN THIS FILE TO AVOID "
			<< "POSSIBLE FUTURE BACKWARD INCOMPATIBILTY ISSUES. Simply ensure the first line "
			<< "of the file is '#CRTFv" << CURRENT_VERSION << "'" << LogIO::POST;
		_fileVersion = CURRENT_VERSION;
	}
}

void RegionTextParser::_parse(const String& contents, const String& fileDesc) {
	_log->origin(LogOrigin("AsciiRegionFileParser", __FUNCTION__));
	static const Regex startAnn("^ann[[:space:]]+");
	static const Regex startDiff("^-[[:space:]]+");
	static const Regex startGlobal("^global[[:space:]]+");
	AnnotationBase::unitInit();

	Vector<String> lines = stringToVector(contents, '\n');
	uInt lineCount = 0;
	Vector<Quantity> qFreqs(2, Quantity(0));
	for(Vector<String>::iterator iter=lines.begin(); iter!=lines.end(); iter++) {
		lineCount++;
		Bool annOnly = False;
		ostringstream preambleoss;
		preambleoss << fileDesc + " line# " << lineCount << ": ";
		String preamble = preambleoss.str();
		Bool difference = False;
		iter->trim();
		if (
			iter->empty() || iter->startsWith("#")
		) {
			// ignore comments and blank lines
			_addLine(AsciiAnnotationFileLine(*iter));
			continue;
		}
		String consumeMe = *iter;
		// consumeMe.downcase();
		Bool spectralParmsUpdated;
		ParamSet newParams;
		if (consumeMe.contains(startDiff)) {
			difference = True;
			// consume the difference character to allow further processing of string
			consumeMe.del(0, 1);
			consumeMe.trim();
			*_log << LogIO::NORMAL << preamble << "difference found" << LogIO::POST;
		}
		else if(consumeMe.contains(startAnn)) {
			annOnly = True;
			// consume the annotation chars
			consumeMe.del(0, 3);
			consumeMe.trim();
			*_log << LogIO::NORMAL << preamble << "annotation only found" << LogIO::POST;
		}
		else if(consumeMe.contains(startGlobal)) {
			consumeMe.del(0, 6);
			_currentGlobals = _getCurrentParamSet(
				spectralParmsUpdated, newParams,
				consumeMe, preamble
			);
			map<AnnotationBase::Keyword, String> gParms;
			for (
				ParamSet::const_iterator iter=newParams.begin();
				iter != newParams.end(); iter++
			) {
				gParms[iter->first] = iter->second.stringVal;
			}
			_addLine(AsciiAnnotationFileLine(gParms));
			if (_csys.hasSpectralAxis() && spectralParmsUpdated) {
				qFreqs = _quantitiesFromFrequencyString(
					newParams[AnnotationBase::RANGE].stringVal, preamble
				);
			}
			*_log << LogIO::NORMAL << preamble << "global found" << LogIO::POST;
			continue;
		}
		// now look for per-line shapes and annotations
		Vector<Quantity> qDirs;
		vector<Quantity> quantities;
		String textString;
		AnnotationBase::Type annType = _getAnnotationType(
			qDirs, quantities, textString, consumeMe, preamble
		);
		ParamSet currentParamSet = _getCurrentParamSet(
			spectralParmsUpdated, newParams, consumeMe, preamble
		);
		if (
			newParams.find(AnnotationBase::LABEL) == newParams.end()
			|| newParams[AnnotationBase::LABEL].stringVal.empty()
		) {
			if (newParams.find(AnnotationBase::LABELCOLOR) != newParams.end()) {
				*_log << LogIO::WARN << preamble
					<< "Ignoring labelcolor because there is no associated label specified"
					<< LogIO::POST;
			}
			if (newParams.find(AnnotationBase::LABELPOS) != newParams.end()) {
				*_log << LogIO::WARN << preamble
					<< "Ignoring labelpos because there is no associated label specified"
					<< LogIO::POST;
			}
			if (newParams.find(AnnotationBase::LABELOFF) != newParams.end()) {
				*_log << LogIO::WARN << preamble
					<< "Ignoring labeloff because there is no associated label specified"
					<< LogIO::POST;
			}
		}
		else if (newParams.find(AnnotationBase::LABELCOLOR) == newParams.end()) {
			// if a label is specified but no label color is specified, the labelcolor
			// is to be the same as the annotation color
			newParams[AnnotationBase::LABELCOLOR] = newParams[AnnotationBase::COLOR];
		}
		if (_csys.hasSpectralAxis()) {
			if(spectralParmsUpdated) {
				qFreqs = _quantitiesFromFrequencyString(
					currentParamSet[AnnotationBase::RANGE].stringVal, preamble
				);
			}
			else if(
				_currentGlobals.find(AnnotationBase::RANGE)
				== _currentGlobals.end()
				|| _currentGlobals.at(AnnotationBase::RANGE).freqRange.size() == 0
			) {
				// no global frequency range, so use entire freq span
				qFreqs = Vector<Quantity>(2, Quantity(0));
			}
		}
		ParamSet globalsLessLocal = _currentGlobals;
		for (
			ParamSet::const_iterator iter=newParams.begin();
			iter != newParams.end(); iter++
		) {
			AnnotationBase::Keyword key = iter->first;
			if (globalsLessLocal.find(key) != globalsLessLocal.end()) {
				globalsLessLocal.erase(key);
			}
		}
		_globalKeysToApply.resize(globalsLessLocal.size(), False);
		uInt i = 0;
		for (
			ParamSet::const_iterator iter=globalsLessLocal.begin();
			iter != globalsLessLocal.end(); iter++
		) {
			_globalKeysToApply[i] = iter->first;
			i++;
		}
		_createAnnotation(
			annType, qDirs, qFreqs, quantities, textString,
			currentParamSet, annOnly, difference, preamble
		);
	}
	*_log << LogIO::NORMAL << "Combined " << _regions
		<< " image regions (which excludes any annotation regions)" << LogIO::POST;
}

void RegionTextParser::_addLine(const AsciiAnnotationFileLine& line) {
	_lines.resize(_lines.size()+1, True);
	_lines[_lines.size()-1] = line;
}

AnnotationBase::Type RegionTextParser::_getAnnotationType(
	Vector<Quantity>& qDirs,
	vector<Quantity>& quantities,
	String& textString,
	String& consumeMe, const String& preamble
) const {
	const static String sOnePairOneSingle =
		"\\[" + sOnePair + ",[^\\[,]+\\]";
	const static String sOnePairAndText =
		"\\[" + sOnePair + ",[[:space:]]*[\"\'].*[\"\'][[:space:]]*\\]";
	const static String sTwoPair = bTwoPair + "\\]";
	const static Regex startTwoPair("^" + sTwoPair);
	const static Regex startOnePairAndText("^" + sOnePairAndText);
	const static String sTwoPairOneSingle = bTwoPair
			+ ",[[:space:]]*[^\\[,]+[[:space:]]*\\]";
	const static Regex startTwoPairOneSingle("^" + sTwoPairOneSingle);
	const static Regex startOnePairOneSingle("^" + sOnePairOneSingle);
	consumeMe.trim();
	String tmp = consumeMe.through(Regex("[[:alpha:]]+"));
	consumeMe.del(0, (Int)tmp.length());
	consumeMe.trim();
	AnnotationBase::Type annotationType = AnnotationBase::typeFromString(tmp);
	switch(annotationType) {
	case AnnotationBase::RECT_BOX:
		if (! consumeMe.contains(startTwoPair)) {
			*_log << preamble << "Illegal box specification "
				<< consumeMe << LogIO::EXCEPTION;
		}
		qDirs = _extractNQuantityPairs(consumeMe, preamble);

		if (qDirs.size() != 4) {
			throw AipsError(preamble
				+ "rectangle box spec must contain exactly 2 direction pairs but it has "
				+ String::toString(qDirs.size())
			);
		}
		break;

	case AnnotationBase::CENTER_BOX:
		if (! consumeMe.contains(startTwoPair)) {
			*_log << preamble << "Illegal center box specification "
				<< consumeMe << LogIO::EXCEPTION;
		}
		qDirs.resize(2);
		quantities.resize(2);
		{
			Vector<Quantity> qs = _extractNQuantityPairs(consumeMe, preamble);
			qDirs[0] = qs[0];
			qDirs[1] = qs[1];
			quantities[0] = qs[2];
			quantities[1] = qs[3];
		}
		break;
	case AnnotationBase::ROTATED_BOX:
		if (! consumeMe.contains(startTwoPairOneSingle)) {
			*_log << preamble << "Illegal rotated box specification "
				<< consumeMe << LogIO::EXCEPTION;
		}
		qDirs.resize(2);
		quantities.resize(3);
		{
			Vector<Quantity> qs = _extractTwoQuantityPairsAndSingleQuantity(consumeMe, preamble);
			qDirs[0] = qs[0];
			qDirs[1] = qs[1];
			quantities[0] = qs[2];
			quantities[1] = qs[3];
			quantities[2] = qs[4];
		}
		break;

	case AnnotationBase::POLYGON:
		if (! consumeMe.contains(startNPair)) {
			*_log << preamble << "Illegal polygon specification "
				<< consumeMe << LogIO::EXCEPTION;
		}
		{
			Vector<Quantity> qs = _extractNQuantityPairs(
				consumeMe, preamble
			);
			qDirs.resize(qs.size());
			qDirs = qs;
		}
		break;
	case AnnotationBase::CIRCLE:
		if (! consumeMe.contains(startOnePairOneSingle)) {
			*_log << preamble << "Illegal circle specification "
				<< consumeMe << LogIO::EXCEPTION;
		}
		qDirs.resize(2);
		quantities.resize(1);
		{
			Vector<Quantity> qs = _extractQuantityPairAndSingleQuantity(
				consumeMe, preamble
			);
			qDirs[0] = qs[0];
			qDirs[1] = qs[1];
			quantities[0] = qs[2];
		}
		break;
	case AnnotationBase::ANNULUS:
		if (! consumeMe.contains(startTwoPair)) {
			*_log << preamble << "Illegal annulus specification "
				<< consumeMe << LogIO::EXCEPTION;
		}
		qDirs.resize(2);
		quantities.resize(2);
		{
			Vector<Quantity> qs = _extractNQuantityPairs(
				consumeMe, preamble
			);
			qDirs[0] = qs[0];
			qDirs[1] = qs[1];
			quantities[0] = qs[2];
			quantities[1] = qs[3];
		}
		break;
	case AnnotationBase::ELLIPSE:
		if (! consumeMe.contains(startTwoPairOneSingle)) {
			*_log << preamble << "Illegal ellipse specification "
				<< consumeMe << LogIO::EXCEPTION;
		}
		qDirs.resize(2);
		quantities.resize(3);
		{
			Vector<Quantity> qs = _extractTwoQuantityPairsAndSingleQuantity(
				consumeMe, preamble
			);
			qDirs[0] = qs[0];
			qDirs[1] = qs[1];
			quantities[0] = qs[2];
			quantities[1] = qs[3];
			quantities[2] = qs[4];
		}
		break;
	case AnnotationBase::LINE:
		if (! consumeMe.contains(startTwoPair)) {
			*_log << preamble << "Illegal line specification "
				<< consumeMe << LogIO::EXCEPTION;
		}
		qDirs.resize(4);
		qDirs = _extractNQuantityPairs(consumeMe, preamble);
		if (qDirs.size() != 4) {
			throw AipsError(preamble
				+ "line spec must contain exactly 2 direction pairs but it has "
				+ String::toString(qDirs.size())
			);
		}
		break;
	case AnnotationBase::VECTOR:
		if (! consumeMe.contains(startTwoPair)) {
			*_log << preamble << "Illegal vector specification "
				<< consumeMe << LogIO::EXCEPTION;
		}
		qDirs.resize(4);
		qDirs = _extractNQuantityPairs(consumeMe, preamble);
		if (qDirs.size() != 4) {
			throw AipsError(preamble
				+ "line spec must contain exactly 2 direction pairs but it has "
				+ String::toString(qDirs.size())
			);
		}
		break;
	case AnnotationBase::TEXT:
		if (! consumeMe.contains(startOnePairAndText)) {
			*_log << preamble << "Illegal text specification "
				<< consumeMe << LogIO::EXCEPTION;
		}
		qDirs.resize(2);
		_extractQuantityPairAndString(
			qDirs, textString, consumeMe, preamble, True
		);
		break;
	case AnnotationBase::SYMBOL:
		if (! consumeMe.contains(startOnePairOneSingle)) {
			*_log << preamble << "Illegal symbol specification "
				<< consumeMe << LogIO::EXCEPTION;
		}
		qDirs.resize(2);
		_extractQuantityPairAndString(
			qDirs, textString, consumeMe, preamble, False
		);
		textString.trim();
		if (textString.length() > 1) {
			throw AipsError(
				preamble
					+ ": A symbol is defined by a single character. The provided string ("
					+ textString
					+ ") has more than one"
			);
		}
		break;
	default:
		throw AipsError(
			preamble + "Unable to determine annotation type"
		);
	}
	return annotationType;
}

RegionTextParser::ParamSet RegionTextParser::getParamSet(
	Bool& spectralParmsUpdated, LogIO& log,
	const String& text, const String& preamble,
	const CoordinateSystem& csys
) {
	ParamSet parms;
	spectralParmsUpdated = False;
	String consumeMe = text;
	// get key-value pairs on the line
	while (consumeMe.size() > 0) {
		ParamValue paramValue;
		AnnotationBase::Keyword key = AnnotationBase::UNKNOWN_KEYWORD;
		consumeMe.trim();
		consumeMe.ltrim(',');
		consumeMe.trim();
		if (! consumeMe.contains('=')) {
			log << preamble << "Illegal extra characters on line ("
				<< consumeMe << "). Did you forget a '='?"
				<< LogIO::EXCEPTION;
		}
		uInt equalPos = consumeMe.find('=');
		String keyword = consumeMe.substr(0, equalPos);
		keyword.trim();
		keyword.downcase();
		consumeMe.del(0, (Int)equalPos + 1);
		consumeMe.trim();
		if (keyword == "label") {
			key = AnnotationBase::LABEL;
			paramValue.stringVal = _doLabel(consumeMe, preamble);
		}
		else {
			paramValue.stringVal = _getKeyValue(consumeMe, preamble);
			if (keyword == "coord") {
				key = AnnotationBase::COORD;
			}
			else if (keyword == "corr") {
				if (csys.hasPolarizationCoordinate()) {
					key = AnnotationBase::CORR;
					paramValue.stokes = _stokesFromString(
						paramValue.stringVal, preamble
					);
				}
				else {
					log << LogIO::WARN << preamble
						<< "Keyword " << keyword << " specified but will be ignored "
						<< "because the coordinate system has no polarization axis."
						<< LogIO::POST;
				}
			}
			else if (
				keyword == "frame" || keyword == "range"
				|| keyword == "veltype" || keyword == "restfreq"
			) {
				spectralParmsUpdated = True;
				if (! csys.hasSpectralAxis()) {
					spectralParmsUpdated = False;
					log << LogIO::WARN << preamble
						<< "Keyword " << keyword << " specified but will be ignored "
						<< "because the coordinate system has no spectral axis."
						<< LogIO::POST;
				}
				else if (keyword == "frame") {
					key = AnnotationBase::FRAME;
				}
				else if (keyword == "range") {
					key = AnnotationBase::RANGE;
				}
				else if (keyword == "veltype") {
					key = AnnotationBase::VELTYPE;
				}
				else if (keyword == "restfreq") {
					key = AnnotationBase::RESTFREQ;
					Quantity qRestfreq;
					if (! readQuantity(qRestfreq, paramValue.stringVal)) {
						log << preamble << "Could not convert rest frequency "
							<< paramValue.stringVal << " to quantity"
							<< LogIO::EXCEPTION;
					}
				}
			}
			else if (keyword == "linewidth") {
				key = AnnotationBase::LINEWIDTH;
				if (! paramValue.stringVal.matches(Regex("^[1-9]+$"))) {
					log << preamble << "linewidth (" << paramValue.stringVal
						<< ") must be a positive integer but is not." << LogIO::EXCEPTION;
				}
				paramValue.intVal = String::toInt(paramValue.stringVal);
			}
			else if (keyword == "linestyle") {
				key = AnnotationBase::LINESTYLE;
				paramValue.lineStyleVal = AnnotationBase::lineStyleFromString(
					paramValue.stringVal
				);
			}
			else if (keyword == "symsize") {
				key = AnnotationBase::SYMSIZE;
				if (! paramValue.stringVal.matches(Regex("^[1-9]+$"))) {
					log << preamble << "symsize (" << paramValue.stringVal
						<< ") must be a positive integer but is not." << LogIO::EXCEPTION;
				}
				paramValue.intVal = String::toInt(paramValue.stringVal);
			}
			else if (keyword == "symthick") {
				key = AnnotationBase::SYMTHICK;
				if (! paramValue.stringVal.matches(Regex("^[1-9]+$"))) {
					log << preamble << "symthick (" << paramValue.stringVal
						<< ") must be a positive integer but is not." << LogIO::EXCEPTION;
				}
				paramValue.intVal = String::toInt(paramValue.stringVal);
			}
			else if (keyword == "color") {
				key = AnnotationBase::COLOR;
			}
			else if (keyword == "font") {
				key = AnnotationBase::FONT;
			}
			else if (keyword == "fontsize") {
				key = AnnotationBase::FONTSIZE;
				paramValue.intVal = String::toInt(paramValue.stringVal);
			}
			else if (keyword == "fontstyle") {
				key = AnnotationBase::FONTSTYLE;
				paramValue.fontStyleVal = AnnotationBase::fontStyleFromString(
					paramValue.stringVal
				);
			}
			else if (keyword == "usetex") {
				String v = paramValue.stringVal;
				v.downcase();
				key = AnnotationBase::USETEX;
				if (
					v != "true"  && v != "t"
					&& v != "false" && v != "f"
				) {
					log << preamble << "Cannot determine boolean value of usetex"
						<< paramValue.stringVal << LogIO::EXCEPTION;
				}
				paramValue.boolVal = (v == "true" || v == "t");
			}
			else if (keyword == "labelcolor") {
				key = AnnotationBase::LABELCOLOR;
			}
			else if (keyword == "labelpos") {
				key = AnnotationBase::LABELPOS;
			}
			else if (keyword == "labeloff") {
				String v = paramValue.stringVal;
				static const String sInt("[-+]?[0-9]+");
				static const Regex rInt(sInt);
				if (
					! v.contains(
						Regex(
							sInt + "[[:space:]]*,[[:space:]]*" + sInt
						)
					)
				) {
					log << preamble << "Illegal label offset specification \""
						<< v << "\"" << LogIO::EXCEPTION;
				}
				// the brackets have been stripped, add them back to make it easier
				// to parse with a method already in existence
				Vector<String> pair = _extractSinglePair("[" + v + "]");
				paramValue.intVec = vector<Int>();

				for (
					Vector<String>::const_iterator iter=pair.begin();
					iter != pair.end(); iter++
				) {
					if (! iter->matches(rInt)) {
						log << preamble << "Illegal label offset specification, "
							<< *iter << " is not an integer" << LogIO::EXCEPTION;
					}
					paramValue.intVec.push_back(String::toInt(*iter));
				}
				key = AnnotationBase::LABELOFF;
			}
			else {
				log << preamble << "Unrecognized key " << keyword
					<< LogIO::EXCEPTION;
			}
		}
		consumeMe.trim();
		if (key != AnnotationBase::UNKNOWN_KEYWORD) {
			parms[key] = paramValue;
		}
	}
	return parms;
}

RegionTextParser::ParamSet
RegionTextParser::_getCurrentParamSet(
	Bool& spectralParmsUpdated, ParamSet& newParams,
	String& consumeMe, const String& preamble
) const {
	ParamSet currentParams = _currentGlobals;
	newParams = getParamSet(
		spectralParmsUpdated,
		*_log, consumeMe, preamble, _csys
	);
	ParamSet::const_iterator end = newParams.end();
	for (
		ParamSet::const_iterator iter=newParams.begin();
		iter!=end; iter++
	) {
		currentParams[iter->first] = iter->second;
	}
	if (
		currentParams.find(AnnotationBase::RANGE) == currentParams.end()
		&& currentParams.find(AnnotationBase::FRAME) != currentParams.end()
	) {
		*_log << preamble << "Frame specified but frequency range not specified"
			<< LogIO::EXCEPTION;
	}
	if (
		currentParams.find(AnnotationBase::RANGE) == currentParams.end()
		&& currentParams.find(AnnotationBase::RESTFREQ) != currentParams.end()
	) {
		*_log << preamble << "Rest frequency specified but velocity range not specified"
			<< LogIO::EXCEPTION;
	}
	return currentParams;
}

Vector<Quantity> RegionTextParser::_quantitiesFromFrequencyString(
	const String& freqString, const String& preamble
) const {
	// the brackets have been stripped, add them back to make it easier
	// to parse with a method already in existence
	String cString = "[" + freqString + "]";
	if (! cString.contains(startOnePair)) {
		*_log << preamble << "Incorrect spectral range specification ("
			<< freqString << ")" << LogIO::EXCEPTION;
	}
	return _extractSingleQuantityPair(
		cString, preamble
	);
}

void RegionTextParser::_createAnnotation(
	const AnnotationBase::Type annType,
	const Vector<Quantity>& qDirs,
	const Vector<Quantity>& qFreqs,
	const vector<Quantity>& quantities,
	const String& textString,
	const ParamSet& currentParamSet,
	const Bool annOnly, const Bool isDifference,
	const String& preamble
) {
	AnnotationBase *annotation = 0;
	Vector<Stokes::StokesTypes> stokes(0);
	if (
		currentParamSet.find(AnnotationBase::CORR) != currentParamSet.end()
		&& _csys.hasPolarizationCoordinate()
	) {
		stokes.resize(currentParamSet.at(AnnotationBase::CORR).stokes.size());
		stokes = currentParamSet.at(AnnotationBase::CORR).stokes;
	}
	String dirRefFrame = currentParamSet.at(AnnotationBase::COORD).stringVal;
	String freqRefFrame = currentParamSet.find(AnnotationBase::FRAME) == currentParamSet.end()
		? "" : currentParamSet.at(AnnotationBase::FRAME).stringVal;
	String doppler = currentParamSet.find(AnnotationBase::VELTYPE) == currentParamSet.end()
		? "" :	currentParamSet.at(AnnotationBase::VELTYPE).stringVal;
	Quantity restfreq;
	if (
		currentParamSet.find(AnnotationBase::RESTFREQ) != currentParamSet.end()
		&& ! readQuantity(
			restfreq, currentParamSet.at(AnnotationBase::RESTFREQ).stringVal
		)
	) {
		*_log << preamble << "restfreq value "
			<< currentParamSet.at(AnnotationBase::RESTFREQ).stringVal << " is not "
			<< "a valid quantity." << LogIO::EXCEPTION;
	}
	try {
	switch (annType) {
		case AnnotationBase::RECT_BOX:
			annotation = new AnnRectBox(
				qDirs[0], qDirs[1], qDirs[2], qDirs[3],
				dirRefFrame, _csys, _imShape, qFreqs[0], qFreqs[1],
				freqRefFrame, doppler, restfreq, stokes,
				annOnly
			);
			break;
		case AnnotationBase::CENTER_BOX:
			annotation = new AnnCenterBox(
				qDirs[0], qDirs[1], quantities[0], quantities[1],
				dirRefFrame, _csys, _imShape, qFreqs[0], qFreqs[1],
				freqRefFrame, doppler, restfreq, stokes,
				annOnly
			);
			break;
		case AnnotationBase::ROTATED_BOX:
			annotation = new AnnRotBox(
				qDirs[0], qDirs[1], quantities[0], quantities[1],
				quantities[2], dirRefFrame, _csys, _imShape, qFreqs[0], qFreqs[1],
				freqRefFrame, doppler, restfreq,  stokes, annOnly
			);
			break;
		case AnnotationBase::POLYGON:
			{
				Vector<Quantity> x(qDirs.size()/2);
				Vector<Quantity> y(qDirs.size()/2);
				for (uInt i=0; i<x.size(); i++) {
					x[i] = qDirs[2*i];
					y[i] = qDirs[2*i + 1];
				}
				annotation = new AnnPolygon(
					x, y, dirRefFrame,  _csys, _imShape, qFreqs[0], qFreqs[1],
					freqRefFrame, doppler, restfreq,  stokes, annOnly
				);
			}
			break;
		case AnnotationBase::CIRCLE:
			if (
				quantities[0].getUnit() == "pix"
				&& ! _csys.directionCoordinate().hasSquarePixels()
			) {
				// radius specified in pixels and pixels are not square, use
				// an AnnEllipse
				annotation = new AnnEllipse(
					qDirs[0], qDirs[1], quantities[0], quantities[0], Quantity(0, "deg"),
					dirRefFrame,  _csys, _imShape, qFreqs[0], qFreqs[1],
					freqRefFrame, doppler, restfreq,  stokes, annOnly
				);
			}
			else {
				annotation = new AnnCircle(
					qDirs[0], qDirs[1], quantities[0],
					dirRefFrame,  _csys, _imShape, qFreqs[0], qFreqs[1],
					freqRefFrame, doppler, restfreq,  stokes, annOnly
				);
			}
			break;
		case AnnotationBase::ANNULUS:
			annotation = new AnnAnnulus(
				qDirs[0], qDirs[1], quantities[0], quantities[1],
				dirRefFrame,  _csys, _imShape, qFreqs[0], qFreqs[1],
				freqRefFrame, doppler, restfreq,  stokes, annOnly
			);
			break;
		case AnnotationBase::ELLIPSE:
			annotation = new AnnEllipse(
				qDirs[0], qDirs[1], quantities[0], quantities[1], quantities[2],
				dirRefFrame,  _csys, _imShape, qFreqs[0], qFreqs[1],
				freqRefFrame, doppler, restfreq,  stokes, annOnly
			);
			break;
		case AnnotationBase::LINE:
			annotation = new AnnLine(
				qDirs[0], qDirs[1], qDirs[2],
				qDirs[3], dirRefFrame,  _csys,
				qFreqs[0], qFreqs[1],
				freqRefFrame, doppler, restfreq,  stokes
			);
			break;
		case AnnotationBase::VECTOR:
			annotation = new AnnVector(
				qDirs[0], qDirs[1], qDirs[2],
				qDirs[3], dirRefFrame,  _csys,
				qFreqs[0], qFreqs[1],
				freqRefFrame, doppler, restfreq,  stokes
			);
			break;
		case AnnotationBase::TEXT:
			annotation = new AnnText(
				qDirs[0], qDirs[1], dirRefFrame,
				_csys, textString, qFreqs[0], qFreqs[1],
				freqRefFrame, doppler, restfreq,  stokes
			);
			break;
		case AnnotationBase::SYMBOL:
			annotation = new AnnSymbol(
				qDirs[0], qDirs[1], dirRefFrame,
				_csys, textString.firstchar(),
				qFreqs[0], qFreqs[1], freqRefFrame,
				doppler, restfreq,  stokes
			);
			break;
		default:
			throw AipsError(
				preamble + "Logic error. Unhandled type "
					+  String::toString(annType) + " in switch statement"
			);
	}
	}
	catch (const WorldToPixelConversionError& x) {
		*_log << LogIO::WARN << preamble
			<< "Error converting one or more world coordinates to pixel coordinates. "
			<< "This could mean, among other things, that (part of) the region or "
			<< "annotation lies far outside the image. This region/annotation will "
			<< "be ignored. The related message is: " << x.getMesg() << LogIO::POST;
		return;
	}
	catch (ToLCRegionConversionError x) {
		*_log << LogIO::WARN << preamble
			<< "Error converting world region to lattice region which probably indicates "
			<< "the region lies outside of the image. This region will be ignored."
			<< "The related message is: " << x.getMesg() << LogIO::POST;
		return;
	}
	catch (AipsError x) {
		*_log << preamble << x.getMesg() << LogIO::EXCEPTION;
	}
	if (annotation->isRegion()) {
		dynamic_cast<AnnRegion *>(annotation)->setDifference(isDifference);
		if (! annOnly) {
			_regions++;
		}
	}
	annotation->setLineWidth(currentParamSet.at(AnnotationBase::LINEWIDTH).intVal);
	annotation->setLineStyle(
		AnnotationBase::lineStyleFromString(
			currentParamSet.at(AnnotationBase::LINESTYLE).stringVal
		)
	);
	annotation->setSymbolSize(currentParamSet.at(AnnotationBase::SYMSIZE).intVal);
	annotation->setSymbolThickness(currentParamSet.at(AnnotationBase::SYMTHICK).intVal);
	annotation->setColor(currentParamSet.at(AnnotationBase::COLOR).stringVal);
	annotation->setFont(currentParamSet.at(AnnotationBase::FONT).stringVal);
	annotation->setFontSize(currentParamSet.at(AnnotationBase::FONTSIZE).intVal);
	annotation->setFontStyle(
		AnnotationBase::fontStyleFromString(
			currentParamSet.at(AnnotationBase::FONTSTYLE).stringVal
		)
	);
	annotation->setUseTex(currentParamSet.at(AnnotationBase::USETEX).boolVal);
	if (
		currentParamSet.find(AnnotationBase::LABEL) != currentParamSet.end()
		&& ! currentParamSet.find(AnnotationBase::LABEL)->second.stringVal.empty()
	) {
		annotation->setLabel(currentParamSet.at(AnnotationBase::LABEL).stringVal);
		annotation->setLabelColor(currentParamSet.at(AnnotationBase::LABELCOLOR).stringVal);
		annotation->setLabelColor(currentParamSet.at(AnnotationBase::LABELCOLOR).stringVal);
		annotation->setLabelPosition(currentParamSet.at(AnnotationBase::LABELPOS).stringVal);
		annotation->setLabelOffset(currentParamSet.at(AnnotationBase::LABELOFF).intVec);
	}
	annotation->setGlobals(_globalKeysToApply);
	AsciiAnnotationFileLine line(annotation);
	_addLine(line);
}

Array<String> RegionTextParser::_extractTwoPairs(uInt& end, const String& string) const {
	end = 0;
	Int firstBegin = string.find('[', 1);
	Int firstEnd = string.find(']', firstBegin);
	String firstPair = string.substr(firstBegin, firstEnd - firstBegin + 1);
	Int secondBegin = string.find('[', firstEnd);
	Int secondEnd = string.find(']', secondBegin);
	String secondPair = string.substr(secondBegin, secondEnd - secondBegin + 1);
	Vector<String> first = _extractSinglePair(firstPair);
	Vector<String> second = _extractSinglePair(secondPair);

	end = secondEnd;
	Array<String> ret(IPosition(2, 2, 2));
	ret(IPosition(2, 0, 0)) = first[0];
	ret(IPosition(2, 0, 1)) = first[1];
	ret(IPosition(2, 1, 0)) = second[0];
	ret(IPosition(2, 1, 1)) = second[1];
	return ret;
}

Vector<String> RegionTextParser::_extractSinglePair(const String& string) {
	Char quotes[2];
	quotes[0] = '\'';
	quotes[1] = '"';
	Int firstBegin = string.find('[', 0) + 1;
	Int firstEnd = string.find(',', firstBegin);
	String first = string.substr(firstBegin, firstEnd - firstBegin);
	first.trim();
	first.trim(quotes, 2);
	Int secondBegin = firstEnd + 1;
	Int secondEnd = string.find(']', secondBegin);
	String second = string.substr(secondBegin, secondEnd - secondBegin);
	second.trim();
	second.trim(quotes, 2);
	Vector<String> ret(2);
	ret[0] = first;
	ret[1] = second;
	return ret;
}

String RegionTextParser::_doLabel(
	String& consumeMe, const String& preamble
) {
	Char firstChar = consumeMe.firstchar();
	if (firstChar != '\'' && firstChar != '"') {
		ostringstream oss;
		oss << preamble << "keyword 'label' found but first non-whitespace "
			<< "character after the '=' is not a quote. It must be.";
		throw AipsError(oss.str());
	}
	String::size_type posCloseQuote = consumeMe.find(firstChar, 1);
	if (posCloseQuote == String::npos) {
		ostringstream err;
		err << preamble << "Could not find closing quote ("
			<< String(firstChar) << ") for label";
		throw AipsError(err.str());
	}
	String label = consumeMe.substr(1, posCloseQuote - 1);
	consumeMe.del(0, (Int)posCloseQuote + 1);
	return label;
}

String RegionTextParser::_getKeyValue(
	String& consumeMe, const String& preamble
) {
	String value;
	if (consumeMe.startsWith("[")) {
		if (! consumeMe.contains("]")) {
			ostringstream err;
			err << preamble << "Unmatched open bracket: "
				<< consumeMe;
			throw AipsError(err.str());
		}
		Int closeBracketPos = consumeMe.find("]");
		// don't save the open and close brackets
		value = consumeMe.substr(1, closeBracketPos - 1);
		consumeMe.del(0, closeBracketPos + 1);
	}
	if (consumeMe.contains(",")) {
		Int commaPos = consumeMe.find(",");
		if (value.empty()) {
			value = consumeMe.substr(0, commaPos);
		}
		consumeMe.del(0, commaPos);
	}
	else if (value.empty()) {
		// last key-value pair on the line
		value = consumeMe;
		consumeMe = "";
	}
	consumeMe.trim();
	Char quotes[2];
	quotes[0] = '\'';
	quotes[1] = '"';
	value.trim();
	value.trim(quotes, 2);
	value.trim();
	return value;
}

Vector<Quantity> RegionTextParser::_extractTwoQuantityPairsAndSingleQuantity(
	String& consumeMe, const String& preamble
) const {
	Vector<Quantity> quantities = _extractTwoQuantityPairs(
		consumeMe, preamble
	);
	consumeMe.trim();
	consumeMe.ltrim(',');
	consumeMe.trim();
	Char quotes[2];
	quotes[0] = '\'';
	quotes[1] = '"';

	Int end = consumeMe.find(']', 0);
	String qString = consumeMe.substr(0, end);
	qString.trim();

	qString.trim(quotes, 2);
	quantities.resize(5, True);
	if (! readQuantity(quantities[4], qString)) {
		*_log << preamble + "Could not convert "
			<< qString << " to quantity." << LogIO::EXCEPTION;
	}
	consumeMe.del(0, end + 1);
	return quantities;
}

void RegionTextParser::_extractQuantityPairAndString(
	Vector<Quantity>& quantities, String& string,
	String& consumeMe, const String& preamble,
	const Bool requireQuotesAroundString
) const {
	// erase the left '['
	consumeMe.del(0, 1);
	SubString pairString = consumeMe.through(startOnePair);
	quantities = _extractSingleQuantityPair(pairString, preamble);
	consumeMe.del(0, (Int)pairString.length() + 1);
	consumeMe.trim();
	consumeMe.ltrim(',');
	consumeMe.trim();
	Int end = 0;
	String::size_type startSearchPos = 0;
	if (requireQuotesAroundString) {
		Char quoteChar = consumeMe.firstchar();
		if (quoteChar != '\'' && quoteChar != '"') {
			*_log << preamble
				<< "Quotes around string required but no quotes were found";
		}
		startSearchPos = consumeMe.find(quoteChar, 1);
		if (startSearchPos == String::npos) {
			*_log << preamble
				<< "Quotes required around string but no matching close quote found"
				<< LogIO::EXCEPTION;
		}
	}
	end = consumeMe.find(']', startSearchPos);
	string = consumeMe.substr(0, end);
	consumeMe.del(0, end + 1);
	string.trim();
	Char quotes[2];
	quotes[0] = '\'';
	quotes[1] = '"';
	string.trim(quotes, 2);
	string.trim();
}

Vector<Quantity> RegionTextParser::_extractQuantityPairAndSingleQuantity(
	String& consumeMe, const String& preamble
) const {
	String qString;
	Vector<Quantity> quantities(2);
	_extractQuantityPairAndString(
		quantities, qString, consumeMe, preamble, False
	);
	quantities.resize(3, True);
	if (! readQuantity(quantities[2], qString)) {
		*_log << preamble << "Could not convert "
			<< qString << " to quantity" << LogIO::EXCEPTION;
	}
	return quantities;
}

Vector<Quantity> RegionTextParser::_extractTwoQuantityPairs(
	String& consumeMe, const String& preamble
) const {
	const Regex startbTwoPair("^" + bTwoPair);
	String mySubstring = String(consumeMe).through(startbTwoPair);
	uInt end = 0;
	Array<String> pairs = _extractTwoPairs(end, mySubstring);
	Vector<Quantity> quantities(4);

	for (uInt i=0; i<4; i++) {
		String desc("string " + String::toString(i));
		String value = pairs(IPosition(2, i/2, i%2));
		if (! readQuantity(quantities[i], value)) {
			*_log << preamble << "Could not convert " << desc
				<< " (" << value << ") to quantity." << LogIO::EXCEPTION;
		}
	}
	consumeMe.del(0, (Int)end + 1);
	return quantities;
}

Vector<Quantity> RegionTextParser::_extractNQuantityPairs (
		String& consumeMe, const String& preamble
) const {
	String pairs = consumeMe.through(startNPair);
	consumeMe.del(0, (Int)pairs.length() + 1);
	pairs.trim();
	// remove the left most [
	pairs.del(0, 1);
	pairs.trim();
	Vector<Quantity> qs(0);
	while (pairs.length() > 1) {
		Vector<Quantity> myqs = _extractSingleQuantityPair(pairs, preamble);
		qs.resize(qs.size() + 2, True);
		qs[qs.size() - 2] = myqs[0];
		qs[qs.size() - 1] = myqs[1];
		pairs.del(0, (Int)pairs.find(']', 0) + 1);
		pairs.trim();
		pairs.ltrim(',');
		pairs.trim();
	}
	return qs;
}

Vector<Quantity> RegionTextParser::_extractSingleQuantityPair(
	const String& pairString, const String& preamble
) const {
	String mySubstring = String(pairString).through(sOnePair, 0);
	Vector<String> pair = _extractSinglePair(mySubstring);
	Vector<Quantity> quantities(2);

	for (uInt i=0; i<2; i++) {
		String value = pair[i];
		if (! readQuantity(quantities[i], value)) {
			*_log << preamble << "Could not convert "
				<< " (" << value << ") to quantity." << LogIO::EXCEPTION;
		}
	}
	return quantities;
}

Vector<Stokes::StokesTypes>
RegionTextParser::_stokesFromString(
	const String& stokes, const String& preamble
) {
	Int maxn = Stokes::NumberOfTypes;
	string *res = new string[maxn];
	Int nStokes = split(stokes, res, maxn, ",");
	Vector<Stokes::StokesTypes> myTypes(nStokes);
	for (Int i=0; i<nStokes; i++) {
		String x(res[i]);
		x.trim();
		myTypes[i] = Stokes::type(x);
		if (myTypes[i] == Stokes::Undefined) {
			throw AipsError(preamble + "Unknown correlation type " + x);
		}
	}
	delete [] res;
	return myTypes;
}

void RegionTextParser::_setInitialGlobals() {
	ParamValue coord;
	coord.intVal = _csys.directionCoordinate(
		_csys.findCoordinate(Coordinate::DIRECTION)
	).directionType(False);
	coord.stringVal = MDirection::showType(coord.intVal);
	_currentGlobals[AnnotationBase::COORD] = coord;

	ParamValue range;
	range.freqRange = Vector<MFrequency>(0);
	_currentGlobals[AnnotationBase::RANGE] = range;

	ParamValue corr;
	corr.stokes = Vector<Stokes::StokesTypes>(0);
	_currentGlobals[AnnotationBase::CORR] = corr;

	if (_csys.hasSpectralAxis()) {
		SpectralCoordinate spectral = _csys.spectralCoordinate(
			_csys.findCoordinate(Coordinate::SPECTRAL)
		);

		ParamValue frame;
		frame.intVal = spectral.frequencySystem(False);
		_currentGlobals[AnnotationBase::FRAME] = frame;

		ParamValue veltype;
		veltype.intVal = spectral.velocityDoppler();
		_currentGlobals[AnnotationBase::VELTYPE] = veltype;

		ParamValue restfreq;
		restfreq.stringVal = String::toString(spectral.restFrequency()) + "Hz";
		_currentGlobals[AnnotationBase::RESTFREQ] = restfreq;
	}
	ParamValue linewidth;
	linewidth.intVal = AnnotationBase::DEFAULT_LINEWIDTH;
	_currentGlobals[AnnotationBase::LINEWIDTH] = linewidth;

	ParamValue linestyle;
	linestyle.lineStyleVal = AnnotationBase::DEFAULT_LINESTYLE;
	_currentGlobals[AnnotationBase::LINESTYLE] = linestyle;

	ParamValue symsize;
	symsize.intVal = AnnotationBase::DEFAULT_SYMBOLSIZE;
	_currentGlobals[AnnotationBase::SYMSIZE] = symsize;

	ParamValue symthick;
	symthick.intVal = AnnotationBase::DEFAULT_SYMBOLTHICKNESS;
	_currentGlobals[AnnotationBase::SYMTHICK] = symthick;

	ParamValue color;
	color.color = AnnotationBase::DEFAULT_COLOR;
	color.stringVal = AnnotationBase::colorToString(color.color);
	_currentGlobals[AnnotationBase::COLOR] = color;

	ParamValue font;
	font.stringVal = AnnotationBase::DEFAULT_FONT;
	_currentGlobals[AnnotationBase::FONT] = font;

	ParamValue fontsize;
	fontsize.intVal = AnnotationBase::DEFAULT_FONTSIZE;
	_currentGlobals[AnnotationBase::FONTSIZE] = fontsize;

	ParamValue fontstyle;
	fontstyle.fontStyleVal = AnnotationBase::DEFAULT_FONTSTYLE;
	_currentGlobals[AnnotationBase::FONTSTYLE] = fontstyle;

	ParamValue usetex;
	usetex.boolVal = AnnotationBase::DEFAULT_USETEX;
	_currentGlobals[AnnotationBase::USETEX] = usetex;

	ParamValue labelcolor;
	labelcolor.color = AnnotationBase::DEFAULT_LABELCOLOR;
	labelcolor.stringVal = AnnotationBase::colorToString(labelcolor.color);
	_currentGlobals[AnnotationBase::LABELCOLOR] = labelcolor;

	ParamValue labelpos;
	labelpos.stringVal = AnnotationBase::DEFAULT_LABELPOS;
	_currentGlobals[AnnotationBase::LABELPOS] = labelpos;

	ParamValue labeloff;
	labeloff.intVec = AnnotationBase::DEFAULT_LABELOFF;
	_currentGlobals[AnnotationBase::LABELOFF] = labeloff;

}

}

