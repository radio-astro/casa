
/*
 * ALMA - Atacama Large Millimeter Array
 * (c) European Southern Observatory, 2002
 * (c) Associated Universities Inc., 2002
 * Copyright by ESO (in the framework of the ALMA collaboration),
 * Copyright by AUI (in the framework of the ALMA collaboration),
 * All rights reserved.
 * 
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY, without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307  USA
 *
 * Warning!
 *  -------------------------------------------------------------------- 
 * | This is generated code!  Do not modify this file.                  |
 * | If you do, all changes will be lost when the file is re-generated. |
 *  --------------------------------------------------------------------
 *
 * File CalFluxTable.cpp
 */
#include <ConversionException.h>
#include <DuplicateKey.h>
#include <OutOfBoundsException.h>

using asdm::ConversionException;
using asdm::DuplicateKey;
using asdm::OutOfBoundsException;

#include <ASDM.h>
#include <CalFluxTable.h>
#include <CalFluxRow.h>
#include <Parser.h>

using asdm::ASDM;
using asdm::CalFluxTable;
using asdm::CalFluxRow;
using asdm::Parser;

#include <iostream>
#include <sstream>
#include <set>
using namespace std;

#include <Misc.h>
using namespace asdm;

#include <libxml/parser.h>
#include <libxml/tree.h>

#include "boost/filesystem/operations.hpp"


namespace asdm {

	string CalFluxTable::tableName = "CalFlux";
	const vector<string> CalFluxTable::attributesNames = initAttributesNames();
		

	/**
	 * The list of field names that make up key key.
	 * (Initialization is in the constructor.)
	 */
	vector<string> CalFluxTable::key;

	/**
	 * Return the list of field names that make up key key
	 * as an array of strings.
	 */	
	vector<string> CalFluxTable::getKeyName() {
		return key;
	}


	CalFluxTable::CalFluxTable(ASDM &c) : container(c) {

	
		key.push_back("sourceName");
	
		key.push_back("calDataId");
	
		key.push_back("calReductionId");
	


		// Define a default entity.
		entity.setEntityId(EntityId("uid://X0/X0/X0"));
		entity.setEntityIdEncrypted("na");
		entity.setEntityTypeName("CalFluxTable");
		entity.setEntityVersion("1");
		entity.setInstanceVersion("1");
		
		// Archive XML
		archiveAsBin = false;
		
		// File XML
		fileAsBin = false;
	}
	
/**
 * A destructor for CalFluxTable.
 */
	CalFluxTable::~CalFluxTable() {
		for (unsigned int i = 0; i < privateRows.size(); i++) 
			delete(privateRows.at(i));
	}

	/**
	 * Container to which this table belongs.
	 */
	ASDM &CalFluxTable::getContainer() const {
		return container;
	}

	/**
	 * Return the number of rows in the table.
	 */
	unsigned int CalFluxTable::size() {
		return privateRows.size();
	}
	
	/**
	 * Return the name of this table.
	 */
	string CalFluxTable::getName() const {
		return tableName;
	}
	
	/**
	 * Build the vector of attributes names.
	 */
	vector<string> CalFluxTable::initAttributesNames() {
		vector<string> attributesNames;

		attributesNames.push_back("sourceName");

		attributesNames.push_back("calDataId");

		attributesNames.push_back("calReductionId");


		attributesNames.push_back("startValidTime");

		attributesNames.push_back("endValidTime");

		attributesNames.push_back("numFrequencyRanges");

		attributesNames.push_back("numStokes");

		attributesNames.push_back("frequencyRanges");

		attributesNames.push_back("fluxMethod");

		attributesNames.push_back("flux");

		attributesNames.push_back("fluxError");

		attributesNames.push_back("stokes");


		attributesNames.push_back("direction");

		attributesNames.push_back("directionCode");

		attributesNames.push_back("directionEquinox");

		attributesNames.push_back("PA");

		attributesNames.push_back("PAError");

		attributesNames.push_back("size");

		attributesNames.push_back("sizeError");

		attributesNames.push_back("sourceModel");

		return attributesNames;
	}
	
	/**
	 * Return the names of the attributes.
	 */
	const vector<string>& CalFluxTable::getAttributesNames() { return attributesNames; }

	/**
	 * Return this table's Entity.
	 */
	Entity CalFluxTable::getEntity() const {
		return entity;
	}

	/**
	 * Set this table's Entity.
	 */
	void CalFluxTable::setEntity(Entity e) {
		this->entity = e; 
	}
	
	//
	// ====> Row creation.
	//
	
	/**
	 * Create a new row.
	 */
	CalFluxRow *CalFluxTable::newRow() {
		return new CalFluxRow (*this);
	}
	

	/**
	 * Create a new row initialized to the specified values.
	 * @return a pointer on the created and initialized row.
	
 	 * @param sourceName 
	
 	 * @param calDataId 
	
 	 * @param calReductionId 
	
 	 * @param startValidTime 
	
 	 * @param endValidTime 
	
 	 * @param numFrequencyRanges 
	
 	 * @param numStokes 
	
 	 * @param frequencyRanges 
	
 	 * @param fluxMethod 
	
 	 * @param flux 
	
 	 * @param fluxError 
	
 	 * @param stokes 
	
     */
	CalFluxRow* CalFluxTable::newRow(string sourceName, Tag calDataId, Tag calReductionId, ArrayTime startValidTime, ArrayTime endValidTime, int numFrequencyRanges, int numStokes, vector<vector<Frequency > > frequencyRanges, FluxCalibrationMethodMod::FluxCalibrationMethod fluxMethod, vector<vector<double > > flux, vector<vector<double > > fluxError, vector<StokesParameterMod::StokesParameter > stokes){
		CalFluxRow *row = new CalFluxRow(*this);
			
		row->setSourceName(sourceName);
			
		row->setCalDataId(calDataId);
			
		row->setCalReductionId(calReductionId);
			
		row->setStartValidTime(startValidTime);
			
		row->setEndValidTime(endValidTime);
			
		row->setNumFrequencyRanges(numFrequencyRanges);
			
		row->setNumStokes(numStokes);
			
		row->setFrequencyRanges(frequencyRanges);
			
		row->setFluxMethod(fluxMethod);
			
		row->setFlux(flux);
			
		row->setFluxError(fluxError);
			
		row->setStokes(stokes);
	
		return row;		
	}	
	


CalFluxRow* CalFluxTable::newRow(CalFluxRow* row) {
	return new CalFluxRow(*this, *row);
}

	//
	// Append a row to its table.
	//

	
	 
	/**
	 * Add a row.
	 * @throws DuplicateKey Thrown if the new row has a key that is already in the table.
	 * @param x A pointer to the row to be added.
	 * @return x
	 */
	CalFluxRow* CalFluxTable::add(CalFluxRow* x) {
		
		if (getRowByKey(
						x->getSourceName()
						,
						x->getCalDataId()
						,
						x->getCalReductionId()
						))
			//throw DuplicateKey(x.getSourceName() + "|" + x.getCalDataId() + "|" + x.getCalReductionId(),"CalFlux");
			throw DuplicateKey("Duplicate key exception in ","CalFluxTable");
		
		row.push_back(x);
		privateRows.push_back(x);
		x->isAdded(true);
		return x;
	}

		





	// 
	// A private method to append a row to its table, used by input conversion
	// methods.
	//

	
	/**
	 * If this table has an autoincrementable attribute then check if *x verifies the rule of uniqueness and throw exception if not.
	 * Check if *x verifies the key uniqueness rule and throw an exception if not.
	 * Append x to its table.
	 * @param x a pointer on the row to be appended.
	 * @returns a pointer on x.
	 * @throws DuplicateKey
	 
	 */
	CalFluxRow*  CalFluxTable::checkAndAdd(CalFluxRow* x)  {
		
		
		if (getRowByKey(
	
			x->getSourceName()
	,
			x->getCalDataId()
	,
			x->getCalReductionId()
			
		)) throw DuplicateKey("Duplicate key exception in ", "CalFluxTable");
		
		row.push_back(x);
		privateRows.push_back(x);
		x->isAdded(true);
		return x;	
	}	







	

	//
	// ====> Methods returning rows.
	//	
	/**
	 * Get all rows.
	 * @return Alls rows as an array of CalFluxRow
	 */
	vector<CalFluxRow *> CalFluxTable::get() {
		return privateRows;
		// return row;
	}

	
/*
 ** Returns a CalFluxRow* given a key.
 ** @return a pointer to the row having the key whose values are passed as parameters, or 0 if
 ** no row exists for that key.
 **
 */
 	CalFluxRow* CalFluxTable::getRowByKey(string sourceName, Tag calDataId, Tag calReductionId)  {
	CalFluxRow* aRow = 0;
	for (unsigned int i = 0; i < row.size(); i++) {
		aRow = row.at(i);
		
			
				if (aRow->sourceName != sourceName) continue;
			
		
			
				if (aRow->calDataId != calDataId) continue;
			
		
			
				if (aRow->calReductionId != calReductionId) continue;
			
		
		return aRow;
	}
	return 0;		
}
	

	
/**
 * Look up the table for a row whose all attributes 
 * are equal to the corresponding parameters of the method.
 * @return a pointer on this row if any, 0 otherwise.
 *
			
 * @param sourceName.
 	 		
 * @param calDataId.
 	 		
 * @param calReductionId.
 	 		
 * @param startValidTime.
 	 		
 * @param endValidTime.
 	 		
 * @param numFrequencyRanges.
 	 		
 * @param numStokes.
 	 		
 * @param frequencyRanges.
 	 		
 * @param fluxMethod.
 	 		
 * @param flux.
 	 		
 * @param fluxError.
 	 		
 * @param stokes.
 	 		 
 */
CalFluxRow* CalFluxTable::lookup(string sourceName, Tag calDataId, Tag calReductionId, ArrayTime startValidTime, ArrayTime endValidTime, int numFrequencyRanges, int numStokes, vector<vector<Frequency > > frequencyRanges, FluxCalibrationMethodMod::FluxCalibrationMethod fluxMethod, vector<vector<double > > flux, vector<vector<double > > fluxError, vector<StokesParameterMod::StokesParameter > stokes) {
		CalFluxRow* aRow;
		for (unsigned int i = 0; i < size(); i++) {
			aRow = row.at(i); 
			if (aRow->compareNoAutoInc(sourceName, calDataId, calReductionId, startValidTime, endValidTime, numFrequencyRanges, numStokes, frequencyRanges, fluxMethod, flux, fluxError, stokes)) return aRow;
		}			
		return 0;	
} 
	
 	 	

	




#ifndef WITHOUT_ACS
	// Conversion Methods

	CalFluxTableIDL *CalFluxTable::toIDL() {
		CalFluxTableIDL *x = new CalFluxTableIDL ();
		unsigned int nrow = size();
		x->row.length(nrow);
		vector<CalFluxRow*> v = get();
		for (unsigned int i = 0; i < nrow; ++i) {
			x->row[i] = *(v[i]->toIDL());
		}
		return x;
	}
#endif
	
#ifndef WITHOUT_ACS
	void CalFluxTable::fromIDL(CalFluxTableIDL x) {
		unsigned int nrow = x.row.length();
		for (unsigned int i = 0; i < nrow; ++i) {
			CalFluxRow *tmp = newRow();
			tmp->setFromIDL(x.row[i]);
			// checkAndAdd(tmp);
			add(tmp);
		}
	}
#endif

	
	string CalFluxTable::toXML()  {
		string buf;

		buf.append("<?xml version=\"1.0\" encoding=\"ISO-8859-1\"?> ");
		buf.append("<CalFluxTable xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" xmlns:clflx=\"http://Alma/XASDM/CalFluxTable\" xsi:schemaLocation=\"http://Alma/XASDM/CalFluxTable http://almaobservatory.org/XML/XASDM/2/CalFluxTable.xsd\" schemaVersion=\"2\" schemaRevision=\"1.54\">\n");
	
		buf.append(entity.toXML());
		string s = container.getEntity().toXML();
		// Change the "Entity" tag to "ContainerEntity".
		buf.append("<Container" + s.substr(1,s.length() - 1)+" ");
		vector<CalFluxRow*> v = get();
		for (unsigned int i = 0; i < v.size(); ++i) {
			try {
				buf.append(v[i]->toXML());
			} catch (NoSuchRow e) {
			}
			buf.append("  ");
		}		
		buf.append("</CalFluxTable> ");
		return buf;
	}

	
	void CalFluxTable::fromXML(string xmlDoc)  {
		Parser xml(xmlDoc);
		if (!xml.isStr("<CalFluxTable")) 
			error();
		// cout << "Parsing a CalFluxTable" << endl;
		string s = xml.getElement("<Entity","/>");
		if (s.length() == 0) 
			error();
		Entity e;
		e.setFromXML(s);
		if (e.getEntityTypeName() != "CalFluxTable")
			error();
		setEntity(e);
		// Skip the container's entity; but, it has to be there.
		s = xml.getElement("<ContainerEntity","/>");
		if (s.length() == 0) 
			error();

		// Get each row in the table.
		s = xml.getElementContent("<row>","</row>");
		CalFluxRow *row;
		while (s.length() != 0) {
			// cout << "Parsing a CalFluxRow" << endl; 
			row = newRow();
			row->setFromXML(s);
			try {
				checkAndAdd(row);
			} catch (DuplicateKey e1) {
				throw ConversionException(e1.getMessage(),"CalFluxTable");
			} 
			catch (UniquenessViolationException e1) {
				throw ConversionException(e1.getMessage(),"CalFluxTable");	
			}
			catch (...) {
				// cout << "Unexpected error in CalFluxTable::checkAndAdd called from CalFluxTable::fromXML " << endl;
			}
			s = xml.getElementContent("<row>","</row>");
		}
		if (!xml.isStr("</CalFluxTable>")) 
			error();
			
		archiveAsBin = false;
		fileAsBin = false;
		
	}

	
	void CalFluxTable::error()  {
		throw ConversionException("Invalid xml document","CalFlux");
	}
	
	
	string CalFluxTable::MIMEXMLPart(const asdm::ByteOrder* byteOrder) {
		string UID = getEntity().getEntityId().toString();
		string withoutUID = UID.substr(6);
		string containerUID = getContainer().getEntity().getEntityId().toString();
		ostringstream oss;
		oss << "<?xml version='1.0'  encoding='ISO-8859-1'?>";
		oss << "\n";
		oss << "<CalFluxTable xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" xmlns:clflx=\"http://Alma/XASDM/CalFluxTable\" xsi:schemaLocation=\"http://Alma/XASDM/CalFluxTable http://almaobservatory.org/XML/XASDM/2/CalFluxTable.xsd\" schemaVersion=\"2\" schemaRevision=\"1.54\">\n";
		oss<< "<Entity entityId='"<<UID<<"' entityIdEncrypted='na' entityTypeName='CalFluxTable' schemaVersion='1' documentVersion='1'/>\n";
		oss<< "<ContainerEntity entityId='"<<containerUID<<"' entityIdEncrypted='na' entityTypeName='ASDM' schemaVersion='1' documentVersion='1'/>\n";
		oss << "<BulkStoreRef file_id='"<<withoutUID<<"' byteOrder='"<<byteOrder->toString()<<"' />\n";
		oss << "<Attributes>\n";

		oss << "<sourceName/>\n"; 
		oss << "<calDataId/>\n"; 
		oss << "<calReductionId/>\n"; 
		oss << "<startValidTime/>\n"; 
		oss << "<endValidTime/>\n"; 
		oss << "<numFrequencyRanges/>\n"; 
		oss << "<numStokes/>\n"; 
		oss << "<frequencyRanges/>\n"; 
		oss << "<fluxMethod/>\n"; 
		oss << "<flux/>\n"; 
		oss << "<fluxError/>\n"; 
		oss << "<stokes/>\n"; 

		oss << "<direction/>\n"; 
		oss << "<directionCode/>\n"; 
		oss << "<directionEquinox/>\n"; 
		oss << "<PA/>\n"; 
		oss << "<PAError/>\n"; 
		oss << "<size/>\n"; 
		oss << "<sizeError/>\n"; 
		oss << "<sourceModel/>\n"; 
		oss << "</Attributes>\n";		
		oss << "</CalFluxTable>\n";

		return oss.str();				
	}
	
	string CalFluxTable::toMIME(const asdm::ByteOrder* byteOrder) {
		EndianOSStream eoss(byteOrder);
		
		string UID = getEntity().getEntityId().toString();
		
		// The MIME Header
		eoss <<"MIME-Version: 1.0";
		eoss << "\n";
		eoss << "Content-Type: Multipart/Related; boundary='MIME_boundary'; type='text/xml'; start= '<header.xml>'";
		eoss <<"\n";
		eoss <<"Content-Description: Correlator";
		eoss <<"\n";
		eoss <<"alma-uid:" << UID;
		eoss <<"\n";
		eoss <<"\n";		
		
		// The MIME XML part header.
		eoss <<"--MIME_boundary";
		eoss <<"\n";
		eoss <<"Content-Type: text/xml; charset='ISO-8859-1'";
		eoss <<"\n";
		eoss <<"Content-Transfer-Encoding: 8bit";
		eoss <<"\n";
		eoss <<"Content-ID: <header.xml>";
		eoss <<"\n";
		eoss <<"\n";
		
		// The MIME XML part content.
		eoss << MIMEXMLPart(byteOrder);

		// The MIME binary part header
		eoss <<"--MIME_boundary";
		eoss <<"\n";
		eoss <<"Content-Type: binary/octet-stream";
		eoss <<"\n";
		eoss <<"Content-ID: <content.bin>";
		eoss <<"\n";
		eoss <<"\n";	
		
		// The MIME binary content
		entity.toBin(eoss);
		container.getEntity().toBin(eoss);
		eoss.writeInt((int) privateRows.size());
		for (unsigned int i = 0; i < privateRows.size(); i++) {
			privateRows.at(i)->toBin(eoss);	
		}
		
		// The closing MIME boundary
		eoss << "\n--MIME_boundary--";
		eoss << "\n";
		
		return eoss.str();	
	}

	
	void CalFluxTable::setFromMIME(const string & mimeMsg) {
    string xmlPartMIMEHeader = "Content-ID: <header.xml>\n\n";
    
    string binPartMIMEHeader = "--MIME_boundary\nContent-Type: binary/octet-stream\nContent-ID: <content.bin>\n\n";
    
    // Detect the XML header.
    string::size_type loc0 = mimeMsg.find(xmlPartMIMEHeader, 0);
    if ( loc0 == string::npos) {
      throw ConversionException("Failed to detect the beginning of the XML header", "CalFlux");
    }
    loc0 += xmlPartMIMEHeader.size();
    
    // Look for the string announcing the binary part.
    string::size_type loc1 = mimeMsg.find( binPartMIMEHeader, loc0 );
    
    if ( loc1 == string::npos ) {
      throw ConversionException("Failed to detect the beginning of the binary part", "CalFlux");
    }
    
    //
    // Extract the xmlHeader and analyze it to find out what is the byte order and the sequence
    // of attribute names.
    //
    string xmlHeader = mimeMsg.substr(loc0, loc1-loc0);
    xmlDoc *doc;
    doc = xmlReadMemory(xmlHeader.data(), xmlHeader.size(), "BinaryTableHeader.xml", NULL, XML_PARSE_NOBLANKS);
    if ( doc == NULL ) 
      throw ConversionException("Failed to parse the xmlHeader into a DOM structure.", "CalFlux");
    
   // This vector will be filled by the names of  all the attributes of the table
   // in the order in which they are expected to be found in the binary representation.
   //
    vector<string> attributesSeq;
      
    xmlNode* root_element = xmlDocGetRootElement(doc);
    if ( root_element == NULL || root_element->type != XML_ELEMENT_NODE )
      throw ConversionException("Failed to parse the xmlHeader into a DOM structure.", "CalFlux");
    
    const ByteOrder* byteOrder;
    if ( string("ASDMBinaryTable").compare((const char*) root_element->name) == 0) {
      // Then it's an "old fashioned" MIME file for tables.
      // Just try to deserialize it with Big_Endian for the bytes ordering.
      byteOrder = asdm::ByteOrder::Big_Endian;
      
 	 //
    // Let's consider a  default order for the sequence of attributes.
    //
     
    attributesSeq.push_back("sourceName") ; 
     
    attributesSeq.push_back("calDataId") ; 
     
    attributesSeq.push_back("calReductionId") ; 
     
    attributesSeq.push_back("startValidTime") ; 
     
    attributesSeq.push_back("endValidTime") ; 
     
    attributesSeq.push_back("numFrequencyRanges") ; 
     
    attributesSeq.push_back("numStokes") ; 
     
    attributesSeq.push_back("frequencyRanges") ; 
     
    attributesSeq.push_back("fluxMethod") ; 
     
    attributesSeq.push_back("flux") ; 
     
    attributesSeq.push_back("fluxError") ; 
     
    attributesSeq.push_back("stokes") ; 
    
     
    attributesSeq.push_back("direction") ; 
     
    attributesSeq.push_back("directionCode") ; 
     
    attributesSeq.push_back("directionEquinox") ; 
     
    attributesSeq.push_back("PA") ; 
     
    attributesSeq.push_back("PAError") ; 
     
    attributesSeq.push_back("size") ; 
     
    attributesSeq.push_back("sizeError") ; 
     
    attributesSeq.push_back("sourceModel") ; 
              
     }
    else if (string("CalFluxTable").compare((const char*) root_element->name) == 0) {
      // It's a new (and correct) MIME file for tables.
      //
      // 1st )  Look for a BulkStoreRef element with an attribute byteOrder.
      //
      xmlNode* bulkStoreRef = 0;
      xmlNode* child = root_element->children;
      
      // Skip the two first children (Entity and ContainerEntity).
      bulkStoreRef = (child ==  0) ? 0 : ( (child->next) == 0 ? 0 : child->next->next );
      
      if ( bulkStoreRef == 0 || (bulkStoreRef->type != XML_ELEMENT_NODE)  || (string("BulkStoreRef").compare((const char*) bulkStoreRef->name) != 0))
      	throw ConversionException ("Could not find the element '/CalFluxTable/BulkStoreRef'. Invalid XML header '"+ xmlHeader + "'.", "CalFlux");
      	
      // We found BulkStoreRef, now look for its attribute byteOrder.
      _xmlAttr* byteOrderAttr = 0;
      for (struct _xmlAttr* attr = bulkStoreRef->properties; attr; attr = attr->next) 
	  if (string("byteOrder").compare((const char*) attr->name) == 0) {
	   byteOrderAttr = attr;
	   break;
	 }
      
      if (byteOrderAttr == 0) 
	     throw ConversionException("Could not find the element '/CalFluxTable/BulkStoreRef/@byteOrder'. Invalid XML header '" + xmlHeader +"'.", "CalFlux");
      
      string byteOrderValue = string((const char*) byteOrderAttr->children->content);
      if (!(byteOrder = asdm::ByteOrder::fromString(byteOrderValue)))
		throw ConversionException("No valid value retrieved for the element '/CalFluxTable/BulkStoreRef/@byteOrder'. Invalid XML header '" + xmlHeader + "'.", "CalFlux");
		
	 //
	 // 2nd) Look for the Attributes element and grab the names of the elements it contains.
	 //
	 xmlNode* attributes = bulkStoreRef->next;
     if ( attributes == 0 || (attributes->type != XML_ELEMENT_NODE)  || (string("Attributes").compare((const char*) attributes->name) != 0))	 
       	throw ConversionException ("Could not find the element '/CalFluxTable/Attributes'. Invalid XML header '"+ xmlHeader + "'.", "CalFlux");
 
 	xmlNode* childOfAttributes = attributes->children;
 	
 	while ( childOfAttributes != 0 && (childOfAttributes->type == XML_ELEMENT_NODE) ) {
 		attributesSeq.push_back(string((const char*) childOfAttributes->name));
 		childOfAttributes = childOfAttributes->next;
    }
    }
    // Create an EndianISStream from the substring containing the binary part.
    EndianISStream eiss(mimeMsg.substr(loc1+binPartMIMEHeader.size()), byteOrder);
    
    entity = Entity::fromBin(eiss);
    
    // We do nothing with that but we have to read it.
    Entity containerEntity = Entity::fromBin(eiss);
    
    int numRows = eiss.readInt();
    try {
      for (int i = 0; i < numRows; i++) {
	CalFluxRow* aRow = CalFluxRow::fromBin(eiss, *this, attributesSeq);
	checkAndAdd(aRow);
      }
    }
    catch (DuplicateKey e) {
      throw ConversionException("Error while writing binary data , the message was "
				+ e.getMessage(), "CalFlux");
    }
    catch (TagFormatException e) {
      throw ConversionException("Error while reading binary data , the message was "
				+ e.getMessage(), "CalFlux");
    }
    archiveAsBin = true;
    fileAsBin = true;
	}

	
	void CalFluxTable::toFile(string directory) {
		if (!directoryExists(directory.c_str()) &&
			!createPath(directory.c_str())) {
			throw ConversionException("Could not create directory " , directory);
		}

		string fileName = directory + "/CalFlux.xml";
		ofstream tableout(fileName.c_str(),ios::out|ios::trunc);
		if (tableout.rdstate() == ostream::failbit)
			throw ConversionException("Could not open file " + fileName + " to write ", "CalFlux");
		if (fileAsBin) 
			tableout << MIMEXMLPart();
		else
			tableout << toXML() << endl;
		tableout.close();
		if (tableout.rdstate() == ostream::failbit)
			throw ConversionException("Could not close file " + fileName, "CalFlux");

		if (fileAsBin) {
			// write the bin serialized
			string fileName = directory + "/CalFlux.bin";
			ofstream tableout(fileName.c_str(),ios::out|ios::trunc);
			if (tableout.rdstate() == ostream::failbit)
				throw ConversionException("Could not open file " + fileName + " to write ", "CalFlux");
			tableout << toMIME() << endl;
			tableout.close();
			if (tableout.rdstate() == ostream::failbit)
				throw ConversionException("Could not close file " + fileName, "CalFlux");
		}
	}

	
	void CalFluxTable::setFromFile(const string& directory) {		
    if (boost::filesystem::exists(boost::filesystem::path(uniqSlashes(directory + "/CalFlux.xml"))))
      setFromXMLFile(directory);
    else if (boost::filesystem::exists(boost::filesystem::path(uniqSlashes(directory + "/CalFlux.bin"))))
      setFromMIMEFile(directory);
    else
      throw ConversionException("No file found for the CalFlux table", "CalFlux");
	}			

	
  void CalFluxTable::setFromMIMEFile(const string& directory) {
    string tablePath ;
    
    tablePath = directory + "/CalFlux.bin";
    ifstream tablefile(tablePath.c_str(), ios::in|ios::binary);
    if (!tablefile.is_open()) { 
      throw ConversionException("Could not open file " + tablePath, "CalFlux");
    }
    // Read in a stringstream.
    stringstream ss; ss << tablefile.rdbuf();
    
    if (tablefile.rdstate() == istream::failbit || tablefile.rdstate() == istream::badbit) {
      throw ConversionException("Error reading file " + tablePath,"CalFlux");
    }
    
    // And close.
    tablefile.close();
    if (tablefile.rdstate() == istream::failbit)
      throw ConversionException("Could not close file " + tablePath,"CalFlux");
    
    setFromMIME(ss.str());
  }	

	
void CalFluxTable::setFromXMLFile(const string& directory) {
    string tablePath ;
    
    tablePath = directory + "/CalFlux.xml";
    ifstream tablefile(tablePath.c_str(), ios::in|ios::binary);
    if (!tablefile.is_open()) { 
      throw ConversionException("Could not open file " + tablePath, "CalFlux");
    }
      // Read in a stringstream.
    stringstream ss;
    ss << tablefile.rdbuf();
    
    if  (tablefile.rdstate() == istream::failbit || tablefile.rdstate() == istream::badbit) {
      throw ConversionException("Error reading file '" + tablePath + "'", "CalFlux");
    }
    
    // And close
    tablefile.close();
    if (tablefile.rdstate() == istream::failbit)
      throw ConversionException("Could not close file '" + tablePath + "'", "CalFlux");

    // Let's make a string out of the stringstream content and empty the stringstream.
    string xmlDocument = ss.str(); ss.str("");

    // Let's make a very primitive check to decide
    // whether the XML content represents the table
    // or refers to it via a <BulkStoreRef element.
    if (xmlDocument.find("<BulkStoreRef") != string::npos)
      setFromMIMEFile(directory);
    else
      fromXML(xmlDocument);
  }

	

	

			
	
	

	
} // End namespace asdm
 
