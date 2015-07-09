//# TBHelp.cc: Widget for displaying and updating the browser help files.
//# Copyright (C) 2005
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
//# $Id: $
#include <casaqt/QtBrowser/TBHelp.qo.h>
#include <casaqt/QtBrowser/TBConstants.h>

#include <stdlib.h>
#include <fstream>
#include <sys/stat.h>
#include <unistd.h>
#include <time.h>
#include <vector>
#include <sstream>
#include <iostream>

#include <xercesc/dom/DOM.hpp>
#include <xercesc/parsers/XercesDOMParser.hpp>
#include <xercesc/util/PlatformUtils.hpp>
#include <stdcasa/xerces.h>

#include <sstream>
#include <QFile>
#include <QtCore>

using namespace xercesc;
using namespace casa::xerces;

namespace casa {

////////////////////////
// TBHELP DEFINITIONS //
////////////////////////

// Constructors/Destructors //

TBHelp::TBHelp() {
	setupUi(this);

	QFile file (":/help/index.html");

	if (!file.exists()) {
	    cout << "Couldn't locate help file";
			return;
	}

	QFileInfo fileInfo(file.fileName());
	QString filename(fileInfo.fileName());

	if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
		cout << "Couldn't open help file";
		return;
    }

	QTextStream in(&file);
    QString helpIndex = in.readAll();

	browser->setHtml(helpIndex.toUtf8().data());
	rebuildButton->close();

}

TBHelp::~TBHelp() { }

// Public Methods //

bool TBHelp::rebuild() {
	// make sure helpdir is defined
	if(helpdir.empty()) {
		helpdir = TBConstants::aipspath();
		if(helpdir.empty()) return false;
		helpdir += TBConstants::HELP_DIR;
	}

	// make sure helpdir and xml file exist
	String xml = helpdir + TBConstants::HELP_XML;
	fstream fin;
	fin.open(helpdir.c_str(), ios::in);
	bool open = fin.is_open();
	fin.close();
	fin.open(xml.c_str(), ios::in);
	open = open && fin.is_open();
	fin.close();
	if(!open) return false;

	String header1 = "<html>\n<head>\n<title>CASA Table Browser Help :: ";
	String header2 = "</title>\n<link type=\"text/css\" rel=\"stylesheet\"";
	header2 += "href=\"style.css\">\n</head>\n<body><a name=\"_top\"></a>\n";
	String greeting = "Welcome to the CASA table browser help!";
	String spacing = "\n\n<br />&nbsp;<br />\n\n";
	String footer = "</body>\n</html>";

	// Parse the XML
	XercesDOMParser* parser = new XercesDOMParser();
	parser->parse(xml.c_str());

	// Process and write the document tree
	DOMElement* e = parser->getDocument()->getDocumentElement();
	char* chst = XMLString::transcode(e->getTagName());
	String name(chst);
	XMLString::release(&chst);
	String lname = name; TBConstants::toLower(lname);

	if(lname == TBConstants::HELP_XML_HELP) {
		DOMImplementation* impl=DOMImplementationRegistry::getDOMImplementation(
				XMLString::transcode("LS"));
		Writer* writer = create_writer(impl);
		ofstream index;
		index.open((helpdir + TBConstants::HELP_INDEX).c_str());
		index << header1 << "Home" << header2;


		index << greeting << spacing;


		DOMNodeList* nodes = e->getElementsByTagName(XMLString::transcode(
				TBConstants::HELP_XML_CATEGORY.c_str()));
		for(unsigned int i = 0; i < nodes->getLength(); i++) {
			e = (DOMElement*)nodes->item(i);
			chst = XMLString::transcode(e->getAttribute(
					XMLString::transcode(TBConstants::HELP_XML_NAME.c_str())));
			name = String(chst);
			XMLString::release(&chst);
			lname = name; TBConstants::toLower(lname);
			for(unsigned int i = 0; i < lname.length(); i++)
				if(TBConstants::isWS(lname[i])) lname[i] = '_';

			ofstream file;
			file.open((helpdir + lname + ".html").c_str());
			file << header1 << name << header2;
			file << "<h1>" << name << "</h1>\n";

			file << "<div class=\"toc\"><a href=\"index.html\">Home</a> :: ";
			file << name << "\n<ul style=\"margin-left: -15px\">";

			index << "<h2><a href=\"" << lname << ".html\">";
			index << name << "</a></h2>\n";

			stringstream ss;
			String filename = lname + ".html";
			DOMNodeList* groups = e->getElementsByTagName(
					XMLString::transcode(
							TBConstants::HELP_XML_GROUP.c_str()));
			for(unsigned int j = 0; j < groups->getLength(); j++) {
				e = (DOMElement*)groups->item(j);
				chst = XMLString::transcode(e->getAttribute(
						XMLString::transcode(TBConstants::HELP_XML_NAME.c_str())));
				name = String(chst);
				XMLString::release(&chst);
				lname = name; TBConstants::toLower(lname);
				for(unsigned int i = 0; i < lname.length(); i++)
					if(TBConstants::isWS(lname[i])) lname[i] = '_';
				if(lname == "index") lname = "_index";

				index << "<h3><a href=\"" << filename << "#" << lname << "\">";
				index << name << "</a></h3><ul>\n";

				file << "<li><a href=\"#" << lname << "\">" << name << "</a>";

				ss <<"<div class=\"highlighted\"><a name=\"" << lname << "\">";
				ss << "</a><h2>" << name << "</h2></div>\n";

				// items
				String taskname = lname;
				DOMNodeList* items = e->getElementsByTagName(
						XMLString::transcode(
								TBConstants::HELP_XML_ITEM.c_str()));
				if(items->getLength() > 0)
					file << "<ul style=\"margin-left: -15px\">\n";
				for(unsigned int k = 0; k < items->getLength(); k++) {
					e = (DOMElement*)items->item(k);
					chst = XMLString::transcode(e->getAttribute(
							XMLString::transcode(
									TBConstants::HELP_XML_NAME.c_str())));
					name = String(chst);
					XMLString::release(&chst);
					if(name[0] == '_') name.erase(0);

					stringstream ss2;
					DOMNodeList* t = e->getChildNodes();
					for(unsigned int m = 0; m < t->getLength(); m++) {
						DOMNode* n = t->item(m);
						XMLCh* ch = write_to_string(writer,n);
						if(ch != NULL) {
							chst = XMLString::transcode(ch);
							ss2 << chst;
							XMLString::release(&chst);
						}
					}

					String text = ss2.str();

					//XMLString::transcode(e->getTextContent());
					index << "<li><a href=\"" << filename << "#" << taskname;
					index << k << "\">" << name << "</a></li>\n";

					ss << "<div class=\"highlighted2\">";
					ss << "<a name=\"" << taskname << k << "\"></a><h3>";
					ss << name << "</h3></div>\n" << text << spacing;
					ss << "[ <a href=\"#_top\">top</a> || <a href=\"";
					ss << "index.html\">home</a> ]" << spacing;

					file << "<li><a href=\"#" << taskname << k << "\">";
					file << name << "</a></li>\n";
				}
				if(items->getLength() > 0) file << "</ul>\n";
				file << "</li>\n";

				if(j < groups->getLength() - 1) ss << spacing;
				index << "</ul>";
			}
			index << spacing;

			file << "\n</div><div>\n\n" << ss.str() << "</div>" << footer;
			file.close();
		}

		index << footer;
		index.close();
		writer->release();
	}

	delete parser;
	return true;
}

// Private Slots //

void TBHelp::rebuildClicked() {
	TBConstants::dprint(TBConstants::DEBUG_HIGH,
			"Rebuilding help HTML files...");
	bool rb = rebuild();
	if(rb)
		TBConstants::dprint(TBConstants::DEBUG_HIGH, "...rebuild succeeded!");
	else
		TBConstants::dprint(TBConstants::DEBUG_HIGH, "...rebuild failed!");

	if(rb && !helpdir.empty()) {
		String url = "file://" + helpdir + TBConstants::HELP_INDEX;
		browser->reload();
	} else
		QMessageBox::critical(this, "Rebuild Error", "Error during rebuild.");
}

}
