
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
 * File SubscanTable.cpp
 */
#include <ConversionException.h>
#include <DuplicateKey.h>
#include <OutOfBoundsException.h>

using asdm::ConversionException;
using asdm::DuplicateKey;
using asdm::OutOfBoundsException;

#include <ASDM.h>
#include <SubscanTable.h>
#include <SubscanRow.h>
#include <Parser.h>

using asdm::ASDM;
using asdm::SubscanTable;
using asdm::SubscanRow;
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

	string SubscanTable::tableName = "Subscan";
	const vector<string> SubscanTable::attributesNames = initAttributesNames();
		

	/**
	 * The list of field names that make up key key.
	 * (Initialization is in the constructor.)
	 */
	vector<string> SubscanTable::key;

	/**
	 * Return the list of field names that make up key key
	 * as an array of strings.
	 */	
	vector<string> SubscanTable::getKeyName() {
		return key;
	}


	SubscanTable::SubscanTable(ASDM &c) : container(c) {

	
		key.push_back("execBlockId");
	
		key.push_back("scanNumber");
	
		key.push_back("subscanNumber");
	


		// Define a default entity.
		entity.setEntityId(EntityId("uid://X0/X0/X0"));
		entity.setEntityIdEncrypted("na");
		entity.setEntityTypeName("SubscanTable");
		entity.setEntityVersion("1");
		entity.setInstanceVersion("1");
		
		// Archive XML
		archiveAsBin = false;
		
		// File XML
		fileAsBin = false;
	}
	
/**
 * A destructor for SubscanTable.
 */
	SubscanTable::~SubscanTable() {
		for (unsigned int i = 0; i < privateRows.size(); i++) 
			delete(privateRows.at(i));
	}

	/**
	 * Container to which this table belongs.
	 */
	ASDM &SubscanTable::getContainer() const {
		return container;
	}

	/**
	 * Return the number of rows in the table.
	 */
	unsigned int SubscanTable::size() {
		return privateRows.size();
	}
	
	/**
	 * Return the name of this table.
	 */
	string SubscanTable::getName() const {
		return tableName;
	}
	
	/**
	 * Build the vector of attributes names.
	 */
	vector<string> SubscanTable::initAttributesNames() {
		vector<string> attributesNames;

		attributesNames.push_back("execBlockId");

		attributesNames.push_back("scanNumber");

		attributesNames.push_back("subscanNumber");


		attributesNames.push_back("startTime");

		attributesNames.push_back("endTime");

		attributesNames.push_back("fieldName");

		attributesNames.push_back("subscanIntent");

		attributesNames.push_back("numberIntegration");

		attributesNames.push_back("numberSubintegration");

		attributesNames.push_back("flagRow");


		attributesNames.push_back("subscanMode");

		attributesNames.push_back("correlatorCalibration");

		return attributesNames;
	}
	
	/**
	 * Return the names of the attributes.
	 */
	const vector<string>& SubscanTable::getAttributesNames() { return attributesNames; }

	/**
	 * Return this table's Entity.
	 */
	Entity SubscanTable::getEntity() const {
		return entity;
	}

	/**
	 * Set this table's Entity.
	 */
	void SubscanTable::setEntity(Entity e) {
		this->entity = e; 
	}
	
	//
	// ====> Row creation.
	//
	
	/**
	 * Create a new row.
	 */
	SubscanRow *SubscanTable::newRow() {
		return new SubscanRow (*this);
	}
	

	/**
	 * Create a new row initialized to the specified values.
	 * @return a pointer on the created and initialized row.
	
 	 * @param execBlockId 
	
 	 * @param scanNumber 
	
 	 * @param subscanNumber 
	
 	 * @param startTime 
	
 	 * @param endTime 
	
 	 * @param fieldName 
	
 	 * @param subscanIntent 
	
 	 * @param numberIntegration 
	
 	 * @param numberSubintegration 
	
 	 * @param flagRow 
	
     */
	SubscanRow* SubscanTable::newRow(Tag execBlockId, int scanNumber, int subscanNumber, ArrayTime startTime, ArrayTime endTime, string fieldName, SubscanIntentMod::SubscanIntent subscanIntent, int numberIntegration, vector<int > numberSubintegration, bool flagRow){
		SubscanRow *row = new SubscanRow(*this);
			
		row->setExecBlockId(execBlockId);
			
		row->setScanNumber(scanNumber);
			
		row->setSubscanNumber(subscanNumber);
			
		row->setStartTime(startTime);
			
		row->setEndTime(endTime);
			
		row->setFieldName(fieldName);
			
		row->setSubscanIntent(subscanIntent);
			
		row->setNumberIntegration(numberIntegration);
			
		row->setNumberSubintegration(numberSubintegration);
			
		row->setFlagRow(flagRow);
	
		return row;		
	}	
	


SubscanRow* SubscanTable::newRow(SubscanRow* row) {
	return new SubscanRow(*this, *row);
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
	SubscanRow* SubscanTable::add(SubscanRow* x) {
		
		if (getRowByKey(
						x->getExecBlockId()
						,
						x->getScanNumber()
						,
						x->getSubscanNumber()
						))
			//throw DuplicateKey(x.getExecBlockId() + "|" + x.getScanNumber() + "|" + x.getSubscanNumber(),"Subscan");
			throw DuplicateKey("Duplicate key exception in ","SubscanTable");
		
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
	SubscanRow*  SubscanTable::checkAndAdd(SubscanRow* x)  {
		
		
		if (getRowByKey(
	
			x->getExecBlockId()
	,
			x->getScanNumber()
	,
			x->getSubscanNumber()
			
		)) throw DuplicateKey("Duplicate key exception in ", "SubscanTable");
		
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
	 * @return Alls rows as an array of SubscanRow
	 */
	vector<SubscanRow *> SubscanTable::get() {
		return privateRows;
		// return row;
	}

	
/*
 ** Returns a SubscanRow* given a key.
 ** @return a pointer to the row having the key whose values are passed as parameters, or 0 if
 ** no row exists for that key.
 **
 */
 	SubscanRow* SubscanTable::getRowByKey(Tag execBlockId, int scanNumber, int subscanNumber)  {
	SubscanRow* aRow = 0;
	for (unsigned int i = 0; i < row.size(); i++) {
		aRow = row.at(i);
		
			
				if (aRow->execBlockId != execBlockId) continue;
			
		
			
				if (aRow->scanNumber != scanNumber) continue;
			
		
			
				if (aRow->subscanNumber != subscanNumber) continue;
			
		
		return aRow;
	}
	return 0;		
}
	

	
/**
 * Look up the table for a row whose all attributes 
 * are equal to the corresponding parameters of the method.
 * @return a pointer on this row if any, 0 otherwise.
 *
			
 * @param execBlockId.
 	 		
 * @param scanNumber.
 	 		
 * @param subscanNumber.
 	 		
 * @param startTime.
 	 		
 * @param endTime.
 	 		
 * @param fieldName.
 	 		
 * @param subscanIntent.
 	 		
 * @param numberIntegration.
 	 		
 * @param numberSubintegration.
 	 		
 * @param flagRow.
 	 		 
 */
SubscanRow* SubscanTable::lookup(Tag execBlockId, int scanNumber, int subscanNumber, ArrayTime startTime, ArrayTime endTime, string fieldName, SubscanIntentMod::SubscanIntent subscanIntent, int numberIntegration, vector<int > numberSubintegration, bool flagRow) {
		SubscanRow* aRow;
		for (unsigned int i = 0; i < size(); i++) {
			aRow = row.at(i); 
			if (aRow->compareNoAutoInc(execBlockId, scanNumber, subscanNumber, startTime, endTime, fieldName, subscanIntent, numberIntegration, numberSubintegration, flagRow)) return aRow;
		}			
		return 0;	
} 
	
 	 	

	




#ifndef WITHOUT_ACS
	// Conversion Methods

	SubscanTableIDL *SubscanTable::toIDL() {
		SubscanTableIDL *x = new SubscanTableIDL ();
		unsigned int nrow = size();
		x->row.length(nrow);
		vector<SubscanRow*> v = get();
		for (unsigned int i = 0; i < nrow; ++i) {
			x->row[i] = *(v[i]->toIDL());
		}
		return x;
	}
#endif
	
#ifndef WITHOUT_ACS
	void SubscanTable::fromIDL(SubscanTableIDL x) {
		unsigned int nrow = x.row.length();
		for (unsigned int i = 0; i < nrow; ++i) {
			SubscanRow *tmp = newRow();
			tmp->setFromIDL(x.row[i]);
			// checkAndAdd(tmp);
			add(tmp);
		}
	}
#endif

	
	string SubscanTable::toXML()  {
		string buf;

		buf.append("<?xml version=\"1.0\" encoding=\"ISO-8859-1\"?> ");
		buf.append("<SubscanTable xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" xmlns:sbscn=\"http://Alma/XASDM/SubscanTable\" xsi:schemaLocation=\"http://Alma/XASDM/SubscanTable http://almaobservatory.org/XML/XASDM/2/SubscanTable.xsd\" schemaVersion=\"2\" schemaRevision=\"1.54\">\n");
	
		buf.append(entity.toXML());
		string s = container.getEntity().toXML();
		// Change the "Entity" tag to "ContainerEntity".
		buf.append("<Container" + s.substr(1,s.length() - 1)+" ");
		vector<SubscanRow*> v = get();
		for (unsigned int i = 0; i < v.size(); ++i) {
			try {
				buf.append(v[i]->toXML());
			} catch (NoSuchRow e) {
			}
			buf.append("  ");
		}		
		buf.append("</SubscanTable> ");
		return buf;
	}

	
	void SubscanTable::fromXML(string xmlDoc)  {
		Parser xml(xmlDoc);
		if (!xml.isStr("<SubscanTable")) 
			error();
		// cout << "Parsing a SubscanTable" << endl;
		string s = xml.getElement("<Entity","/>");
		if (s.length() == 0) 
			error();
		Entity e;
		e.setFromXML(s);
		if (e.getEntityTypeName() != "SubscanTable")
			error();
		setEntity(e);
		// Skip the container's entity; but, it has to be there.
		s = xml.getElement("<ContainerEntity","/>");
		if (s.length() == 0) 
			error();

		// Get each row in the table.
		s = xml.getElementContent("<row>","</row>");
		SubscanRow *row;
		while (s.length() != 0) {
			// cout << "Parsing a SubscanRow" << endl; 
			row = newRow();
			row->setFromXML(s);
			try {
				checkAndAdd(row);
			} catch (DuplicateKey e1) {
				throw ConversionException(e1.getMessage(),"SubscanTable");
			} 
			catch (UniquenessViolationException e1) {
				throw ConversionException(e1.getMessage(),"SubscanTable");	
			}
			catch (...) {
				// cout << "Unexpected error in SubscanTable::checkAndAdd called from SubscanTable::fromXML " << endl;
			}
			s = xml.getElementContent("<row>","</row>");
		}
		if (!xml.isStr("</SubscanTable>")) 
			error();
			
		archiveAsBin = false;
		fileAsBin = false;
		
	}

	
	void SubscanTable::error()  {
		throw ConversionException("Invalid xml document","Subscan");
	}
	
	
	string SubscanTable::MIMEXMLPart(const asdm::ByteOrder* byteOrder) {
		string UID = getEntity().getEntityId().toString();
		string withoutUID = UID.substr(6);
		string containerUID = getContainer().getEntity().getEntityId().toString();
		ostringstream oss;
		oss << "<?xml version='1.0'  encoding='ISO-8859-1'?>";
		oss << "\n";
		oss << "<SubscanTable xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" xmlns:sbscn=\"http://Alma/XASDM/SubscanTable\" xsi:schemaLocation=\"http://Alma/XASDM/SubscanTable http://almaobservatory.org/XML/XASDM/2/SubscanTable.xsd\" schemaVersion=\"2\" schemaRevision=\"1.54\">\n";
		oss<< "<Entity entityId='"<<UID<<"' entityIdEncrypted='na' entityTypeName='SubscanTable' schemaVersion='1' documentVersion='1'/>\n";
		oss<< "<ContainerEntity entityId='"<<containerUID<<"' entityIdEncrypted='na' entityTypeName='ASDM' schemaVersion='1' documentVersion='1'/>\n";
		oss << "<BulkStoreRef file_id='"<<withoutUID<<"' byteOrder='"<<byteOrder->toString()<<"' />\n";
		oss << "<Attributes>\n";

		oss << "<execBlockId/>\n"; 
		oss << "<scanNumber/>\n"; 
		oss << "<subscanNumber/>\n"; 
		oss << "<startTime/>\n"; 
		oss << "<endTime/>\n"; 
		oss << "<fieldName/>\n"; 
		oss << "<subscanIntent/>\n"; 
		oss << "<numberIntegration/>\n"; 
		oss << "<numberSubintegration/>\n"; 
		oss << "<flagRow/>\n"; 

		oss << "<subscanMode/>\n"; 
		oss << "<correlatorCalibration/>\n"; 
		oss << "</Attributes>\n";		
		oss << "</SubscanTable>\n";

		return oss.str();				
	}
	
	string SubscanTable::toMIME(const asdm::ByteOrder* byteOrder) {
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

	
	void SubscanTable::setFromMIME(const string & mimeMsg) {
    string xmlPartMIMEHeader = "Content-ID: <header.xml>\n\n";
    
    string binPartMIMEHeader = "--MIME_boundary\nContent-Type: binary/octet-stream\nContent-ID: <content.bin>\n\n";
    
    // Detect the XML header.
    string::size_type loc0 = mimeMsg.find(xmlPartMIMEHeader, 0);
    if ( loc0 == string::npos) {
      throw ConversionException("Failed to detect the beginning of the XML header", "Subscan");
    }
    loc0 += xmlPartMIMEHeader.size();
    
    // Look for the string announcing the binary part.
    string::size_type loc1 = mimeMsg.find( binPartMIMEHeader, loc0 );
    
    if ( loc1 == string::npos ) {
      throw ConversionException("Failed to detect the beginning of the binary part", "Subscan");
    }
    
    //
    // Extract the xmlHeader and analyze it to find out what is the byte order and the sequence
    // of attribute names.
    //
    string xmlHeader = mimeMsg.substr(loc0, loc1-loc0);
    xmlDoc *doc;
    doc = xmlReadMemory(xmlHeader.data(), xmlHeader.size(), "BinaryTableHeader.xml", NULL, XML_PARSE_NOBLANKS);
    if ( doc == NULL ) 
      throw ConversionException("Failed to parse the xmlHeader into a DOM structure.", "Subscan");
    
   // This vector will be filled by the names of  all the attributes of the table
   // in the order in which they are expected to be found in the binary representation.
   //
    vector<string> attributesSeq;
      
    xmlNode* root_element = xmlDocGetRootElement(doc);
    if ( root_element == NULL || root_element->type != XML_ELEMENT_NODE )
      throw ConversionException("Failed to parse the xmlHeader into a DOM structure.", "Subscan");
    
    const ByteOrder* byteOrder;
    if ( string("ASDMBinaryTable").compare((const char*) root_element->name) == 0) {
      // Then it's an "old fashioned" MIME file for tables.
      // Just try to deserialize it with Big_Endian for the bytes ordering.
      byteOrder = asdm::ByteOrder::Big_Endian;
      
 	 //
    // Let's consider a  default order for the sequence of attributes.
    //
     
    attributesSeq.push_back("execBlockId") ; 
     
    attributesSeq.push_back("scanNumber") ; 
     
    attributesSeq.push_back("subscanNumber") ; 
     
    attributesSeq.push_back("startTime") ; 
     
    attributesSeq.push_back("endTime") ; 
     
    attributesSeq.push_back("fieldName") ; 
     
    attributesSeq.push_back("subscanIntent") ; 
     
    attributesSeq.push_back("numberIntegration") ; 
     
    attributesSeq.push_back("numberSubintegration") ; 
     
    attributesSeq.push_back("flagRow") ; 
    
     
    attributesSeq.push_back("subscanMode") ; 
     
    attributesSeq.push_back("correlatorCalibration") ; 
              
     }
    else if (string("SubscanTable").compare((const char*) root_element->name) == 0) {
      // It's a new (and correct) MIME file for tables.
      //
      // 1st )  Look for a BulkStoreRef element with an attribute byteOrder.
      //
      xmlNode* bulkStoreRef = 0;
      xmlNode* child = root_element->children;
      
      // Skip the two first children (Entity and ContainerEntity).
      bulkStoreRef = (child ==  0) ? 0 : ( (child->next) == 0 ? 0 : child->next->next );
      
      if ( bulkStoreRef == 0 || (bulkStoreRef->type != XML_ELEMENT_NODE)  || (string("BulkStoreRef").compare((const char*) bulkStoreRef->name) != 0))
      	throw ConversionException ("Could not find the element '/SubscanTable/BulkStoreRef'. Invalid XML header '"+ xmlHeader + "'.", "Subscan");
      	
      // We found BulkStoreRef, now look for its attribute byteOrder.
      _xmlAttr* byteOrderAttr = 0;
      for (struct _xmlAttr* attr = bulkStoreRef->properties; attr; attr = attr->next) 
	  if (string("byteOrder").compare((const char*) attr->name) == 0) {
	   byteOrderAttr = attr;
	   break;
	 }
      
      if (byteOrderAttr == 0) 
	     throw ConversionException("Could not find the element '/SubscanTable/BulkStoreRef/@byteOrder'. Invalid XML header '" + xmlHeader +"'.", "Subscan");
      
      string byteOrderValue = string((const char*) byteOrderAttr->children->content);
      if (!(byteOrder = asdm::ByteOrder::fromString(byteOrderValue)))
		throw ConversionException("No valid value retrieved for the element '/SubscanTable/BulkStoreRef/@byteOrder'. Invalid XML header '" + xmlHeader + "'.", "Subscan");
		
	 //
	 // 2nd) Look for the Attributes element and grab the names of the elements it contains.
	 //
	 xmlNode* attributes = bulkStoreRef->next;
     if ( attributes == 0 || (attributes->type != XML_ELEMENT_NODE)  || (string("Attributes").compare((const char*) attributes->name) != 0))	 
       	throw ConversionException ("Could not find the element '/SubscanTable/Attributes'. Invalid XML header '"+ xmlHeader + "'.", "Subscan");
 
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
	SubscanRow* aRow = SubscanRow::fromBin(eiss, *this, attributesSeq);
	checkAndAdd(aRow);
      }
    }
    catch (DuplicateKey e) {
      throw ConversionException("Error while writing binary data , the message was "
				+ e.getMessage(), "Subscan");
    }
    catch (TagFormatException e) {
      throw ConversionException("Error while reading binary data , the message was "
				+ e.getMessage(), "Subscan");
    }
    archiveAsBin = true;
    fileAsBin = true;
	}

	
	void SubscanTable::toFile(string directory) {
		if (!directoryExists(directory.c_str()) &&
			!createPath(directory.c_str())) {
			throw ConversionException("Could not create directory " , directory);
		}

		string fileName = directory + "/Subscan.xml";
		ofstream tableout(fileName.c_str(),ios::out|ios::trunc);
		if (tableout.rdstate() == ostream::failbit)
			throw ConversionException("Could not open file " + fileName + " to write ", "Subscan");
		if (fileAsBin) 
			tableout << MIMEXMLPart();
		else
			tableout << toXML() << endl;
		tableout.close();
		if (tableout.rdstate() == ostream::failbit)
			throw ConversionException("Could not close file " + fileName, "Subscan");

		if (fileAsBin) {
			// write the bin serialized
			string fileName = directory + "/Subscan.bin";
			ofstream tableout(fileName.c_str(),ios::out|ios::trunc);
			if (tableout.rdstate() == ostream::failbit)
				throw ConversionException("Could not open file " + fileName + " to write ", "Subscan");
			tableout << toMIME() << endl;
			tableout.close();
			if (tableout.rdstate() == ostream::failbit)
				throw ConversionException("Could not close file " + fileName, "Subscan");
		}
	}

	
	void SubscanTable::setFromFile(const string& directory) {		
    if (boost::filesystem::exists(boost::filesystem::path(uniqSlashes(directory + "/Subscan.xml"))))
      setFromXMLFile(directory);
    else if (boost::filesystem::exists(boost::filesystem::path(uniqSlashes(directory + "/Subscan.bin"))))
      setFromMIMEFile(directory);
    else
      throw ConversionException("No file found for the Subscan table", "Subscan");
	}			

	
  void SubscanTable::setFromMIMEFile(const string& directory) {
    string tablePath ;
    
    tablePath = directory + "/Subscan.bin";
    ifstream tablefile(tablePath.c_str(), ios::in|ios::binary);
    if (!tablefile.is_open()) { 
      throw ConversionException("Could not open file " + tablePath, "Subscan");
    }
    // Read in a stringstream.
    stringstream ss; ss << tablefile.rdbuf();
    
    if (tablefile.rdstate() == istream::failbit || tablefile.rdstate() == istream::badbit) {
      throw ConversionException("Error reading file " + tablePath,"Subscan");
    }
    
    // And close.
    tablefile.close();
    if (tablefile.rdstate() == istream::failbit)
      throw ConversionException("Could not close file " + tablePath,"Subscan");
    
    setFromMIME(ss.str());
  }	

	
void SubscanTable::setFromXMLFile(const string& directory) {
    string tablePath ;
    
    tablePath = directory + "/Subscan.xml";
    ifstream tablefile(tablePath.c_str(), ios::in|ios::binary);
    if (!tablefile.is_open()) { 
      throw ConversionException("Could not open file " + tablePath, "Subscan");
    }
      // Read in a stringstream.
    stringstream ss;
    ss << tablefile.rdbuf();
    
    if  (tablefile.rdstate() == istream::failbit || tablefile.rdstate() == istream::badbit) {
      throw ConversionException("Error reading file '" + tablePath + "'", "Subscan");
    }
    
    // And close
    tablefile.close();
    if (tablefile.rdstate() == istream::failbit)
      throw ConversionException("Could not close file '" + tablePath + "'", "Subscan");

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
 
