//# RSFileReaderWriter.cc: Interfaces for classes that read/write shape files.
//# Copyright (C) 2008
//# Associated Universities, Inc. Washington DC, USA.
//#
//# This library is free software; you can redistribute it and/or modify it
//# under the terms of the GNU Library General Public License as published by
//# the Free Software Foundation; either version 2 of the License, or (at your
//# option) any later version.
//#
//# This library is distributed in the hope that it will be useful, but WITHOUT
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
//# $Id$
#include <display/RegionShapes/DS9FileWriter.qo.h>
#include <display/RegionShapes/RSFileReaderWriter.h>
#include <display/RegionShapes/DS9FileReader.h>
#include <display/RegionShapes/XMLFileReaderWriter.h>

namespace casa {

// RFERROR DEFINITIONS //

	RFError::RFError() : m_fatal(false) { }

	RFError::RFError(const String& error, bool isFatal) : m_error(error),
		m_fatal(isFatal) { }

	RFError::~RFError() { }

	bool RFError::isFatal() const {
		return m_fatal;
	}

	const String& RFError::error() const {
		return m_error;
	}

	void RFError::set(const String& error, bool isFatal) {
		m_error = error;
		m_fatal = isFatal;
	}


// RSFILEREADERWRITER DEFINITIONS //

// Non-Static Methods //

	void RSFileReaderWriter::setFile(const String& filename) {
		m_filename = filename;
	}

	const RFError& RSFileReaderWriter::lastError() const {
		return m_lastError;
	}

	void RSFileReaderWriter::setError(const String& error, bool isFatal) const {
		const_cast<RFError&>(m_lastError).set(error, isFatal);
	}


// Static Methods //

	RSFileReaderWriter::SupportedType RSFileReaderWriter::supportedType(String t) {
		t.downcase();
		if(t == "ds9")           return DS9;
		else if(t == "casa-xml") return CASA_XML;

		else return DS9; // default
	}

	String RSFileReaderWriter::supportedType(SupportedType type) {
		switch(type) {
		case DS9:
			return "DS9";
		case CASA_XML:
			return "CASA-XML";

		default:
			return ""; // unknown
		}
	}

	String RSFileReaderWriter::extensionForType(SupportedType type) {
		switch(type) {
		case DS9:
			return "reg";
		case CASA_XML:
			return "xml";

		default:
			return ""; // unknown
		}
	}

	vector<RSFileReaderWriter::SupportedType> RSFileReaderWriter::supportedTypes() {
		vector<SupportedType> v(2);
		v[0] = DS9;
		v[1] = CASA_XML;
		return v;
	}

	vector<String> RSFileReaderWriter::supportedTypesStrings() {
		vector<SupportedType> types = supportedTypes();
		vector<String> v(types.size());
		for(unsigned int i = 0; i < v.size(); i++) v[i] = supportedType(types[i]);
		return v;
	}

	RSFileReader* RSFileReaderWriter::readerForType(SupportedType type) {
		switch(type) {
		case DS9:
			return new DS9FileReader();
		case CASA_XML:
			return new XMLFileReaderWriter();

		default:
			return NULL; // unknown
		}
	}

	RSFileWriter* RSFileReaderWriter::writerForType(SupportedType type) {
		switch(type) {
		case DS9:
			return new DS9FileWriter();
		case CASA_XML:
			return new XMLFileReaderWriter();

		default:
			return NULL; // unknown
		}
	}

	QWidget* RSFileReaderWriter::optionsWidgetForType(SupportedType type) {
		switch(type) {
		case DS9:
			return DS9FileWriter().optionsWidget();
		case CASA_XML:
			return XMLFileReaderWriter().optionsWidget();

		default:
			return NULL; // unknown
		}
	}

}
