
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
 * File FieldTable.cpp
 */
#include <ConversionException.h>
#include <DuplicateKey.h>
#include <OutOfBoundsException.h>

using asdm::ConversionException;
using asdm::DuplicateKey;
using asdm::OutOfBoundsException;

#include <ASDM.h>
#include <FieldTable.h>
#include <FieldRow.h>
#include <Parser.h>

using asdm::ASDM;
using asdm::FieldTable;
using asdm::FieldRow;
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

	string FieldTable::tableName = "Field";
	const vector<string> FieldTable::attributesNames = initAttributesNames();
		

	/**
	 * The list of field names that make up key key.
	 * (Initialization is in the constructor.)
	 */
	vector<string> FieldTable::key;

	/**
	 * Return the list of field names that make up key key
	 * as an array of strings.
	 */	
	vector<string> FieldTable::getKeyName() {
		return key;
	}


	FieldTable::FieldTable(ASDM &c) : container(c) {

	
		key.push_back("fieldId");
	


		// Define a default entity.
		entity.setEntityId(EntityId("uid://X0/X0/X0"));
		entity.setEntityIdEncrypted("na");
		entity.setEntityTypeName("FieldTable");
		entity.setEntityVersion("1");
		entity.setInstanceVersion("1");
		
		// Archive XML
		archiveAsBin = false;
		
		// File XML
		fileAsBin = false;
	}
	
/**
 * A destructor for FieldTable.
 */
	FieldTable::~FieldTable() {
		for (unsigned int i = 0; i < privateRows.size(); i++) 
			delete(privateRows.at(i));
	}

	/**
	 * Container to which this table belongs.
	 */
	ASDM &FieldTable::getContainer() const {
		return container;
	}

	/**
	 * Return the number of rows in the table.
	 */
	unsigned int FieldTable::size() {
		return privateRows.size();
	}
	
	/**
	 * Return the name of this table.
	 */
	string FieldTable::getName() const {
		return tableName;
	}
	
	/**
	 * Build the vector of attributes names.
	 */
	vector<string> FieldTable::initAttributesNames() {
		vector<string> attributesNames;

		attributesNames.push_back("fieldId");


		attributesNames.push_back("fieldName");

		attributesNames.push_back("code");

		attributesNames.push_back("numPoly");

		attributesNames.push_back("delayDir");

		attributesNames.push_back("phaseDir");

		attributesNames.push_back("referenceDir");


		attributesNames.push_back("time");

		attributesNames.push_back("directionCode");

		attributesNames.push_back("directionEquinox");

		attributesNames.push_back("assocNature");

		attributesNames.push_back("ephemerisId");

		attributesNames.push_back("sourceId");

		attributesNames.push_back("assocFieldId");

		return attributesNames;
	}
	
	/**
	 * Return the names of the attributes.
	 */
	const vector<string>& FieldTable::getAttributesNames() { return attributesNames; }

	/**
	 * Return this table's Entity.
	 */
	Entity FieldTable::getEntity() const {
		return entity;
	}

	/**
	 * Set this table's Entity.
	 */
	void FieldTable::setEntity(Entity e) {
		this->entity = e; 
	}
	
	//
	// ====> Row creation.
	//
	
	/**
	 * Create a new row.
	 */
	FieldRow *FieldTable::newRow() {
		return new FieldRow (*this);
	}
	

	/**
	 * Create a new row initialized to the specified values.
	 * @return a pointer on the created and initialized row.
	
 	 * @param fieldName 
	
 	 * @param code 
	
 	 * @param numPoly 
	
 	 * @param delayDir 
	
 	 * @param phaseDir 
	
 	 * @param referenceDir 
	
     */
	FieldRow* FieldTable::newRow(string fieldName, string code, int numPoly, vector<vector<Angle > > delayDir, vector<vector<Angle > > phaseDir, vector<vector<Angle > > referenceDir){
		FieldRow *row = new FieldRow(*this);
			
		row->setFieldName(fieldName);
			
		row->setCode(code);
			
		row->setNumPoly(numPoly);
			
		row->setDelayDir(delayDir);
			
		row->setPhaseDir(phaseDir);
			
		row->setReferenceDir(referenceDir);
	
		return row;		
	}	
	


FieldRow* FieldTable::newRow(FieldRow* row) {
	return new FieldRow(*this, *row);
}

	//
	// Append a row to its table.
	//

	
	 
	
	/** 
 	 * Look up the table for a row whose noautoincrementable attributes are matching their
 	 * homologues in *x.  If a row is found  this row else autoincrement  *x.fieldId, 
 	 * add x to its table and returns x.
 	 *  
 	 * @returns a pointer on a FieldRow.
 	 * @param x. A pointer on the row to be added.
 	 */ 
 		
			
	FieldRow* FieldTable::add(FieldRow* x) {
			 
		FieldRow* aRow = lookup(
				
		x->getFieldName()
				,
		x->getCode()
				,
		x->getNumPoly()
				,
		x->getDelayDir()
				,
		x->getPhaseDir()
				,
		x->getReferenceDir()
				
		);
		if (aRow) return aRow;
			

			
		// Autoincrement fieldId
		x->setFieldId(Tag(size(), TagType::Field));
						
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
	 
	 * @throws UniquenessViolationException
	 
	 */
	FieldRow*  FieldTable::checkAndAdd(FieldRow* x)  {
	 
		 
		if (lookup(
			
			x->getFieldName()
		,
			x->getCode()
		,
			x->getNumPoly()
		,
			x->getDelayDir()
		,
			x->getPhaseDir()
		,
			x->getReferenceDir()
		
		)) throw UniquenessViolationException("Uniqueness violation exception in table FieldTable");
		
		
		
		if (getRowByKey(
	
			x->getFieldId()
			
		)) throw DuplicateKey("Duplicate key exception in ", "FieldTable");
		
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
	 * @return Alls rows as an array of FieldRow
	 */
	vector<FieldRow *> FieldTable::get() {
		return privateRows;
		// return row;
	}

	
/*
 ** Returns a FieldRow* given a key.
 ** @return a pointer to the row having the key whose values are passed as parameters, or 0 if
 ** no row exists for that key.
 **
 */
 	FieldRow* FieldTable::getRowByKey(Tag fieldId)  {
	FieldRow* aRow = 0;
	for (unsigned int i = 0; i < row.size(); i++) {
		aRow = row.at(i);
		
			
				if (aRow->fieldId != fieldId) continue;
			
		
		return aRow;
	}
	return 0;		
}
	

	
/**
 * Look up the table for a row whose all attributes  except the autoincrementable one 
 * are equal to the corresponding parameters of the method.
 * @return a pointer on this row if any, 0 otherwise.
 *
			
 * @param fieldName.
 	 		
 * @param code.
 	 		
 * @param numPoly.
 	 		
 * @param delayDir.
 	 		
 * @param phaseDir.
 	 		
 * @param referenceDir.
 	 		 
 */
FieldRow* FieldTable::lookup(string fieldName, string code, int numPoly, vector<vector<Angle > > delayDir, vector<vector<Angle > > phaseDir, vector<vector<Angle > > referenceDir) {
		FieldRow* aRow;
		for (unsigned int i = 0; i < size(); i++) {
			aRow = row.at(i); 
			if (aRow->compareNoAutoInc(fieldName, code, numPoly, delayDir, phaseDir, referenceDir)) return aRow;
		}			
		return 0;	
} 
	
 	 	

	




#ifndef WITHOUT_ACS
	// Conversion Methods

	FieldTableIDL *FieldTable::toIDL() {
		FieldTableIDL *x = new FieldTableIDL ();
		unsigned int nrow = size();
		x->row.length(nrow);
		vector<FieldRow*> v = get();
		for (unsigned int i = 0; i < nrow; ++i) {
			x->row[i] = *(v[i]->toIDL());
		}
		return x;
	}
#endif
	
#ifndef WITHOUT_ACS
	void FieldTable::fromIDL(FieldTableIDL x) {
		unsigned int nrow = x.row.length();
		for (unsigned int i = 0; i < nrow; ++i) {
			FieldRow *tmp = newRow();
			tmp->setFromIDL(x.row[i]);
			// checkAndAdd(tmp);
			add(tmp);
		}
	}
#endif

	
	string FieldTable::toXML()  {
		string buf;

		buf.append("<?xml version=\"1.0\" encoding=\"ISO-8859-1\"?> ");
		buf.append("<FieldTable xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" xmlns:field=\"http://Alma/XASDM/FieldTable\" xsi:schemaLocation=\"http://Alma/XASDM/FieldTable http://almaobservatory.org/XML/XASDM/2/FieldTable.xsd\" schemaVersion=\"2\" schemaRevision=\"1.55\">\n");
	
		buf.append(entity.toXML());
		string s = container.getEntity().toXML();
		// Change the "Entity" tag to "ContainerEntity".
		buf.append("<Container" + s.substr(1,s.length() - 1)+" ");
		vector<FieldRow*> v = get();
		for (unsigned int i = 0; i < v.size(); ++i) {
			try {
				buf.append(v[i]->toXML());
			} catch (NoSuchRow e) {
			}
			buf.append("  ");
		}		
		buf.append("</FieldTable> ");
		return buf;
	}

	
	void FieldTable::fromXML(string xmlDoc)  {
		Parser xml(xmlDoc);
		if (!xml.isStr("<FieldTable")) 
			error();
		// cout << "Parsing a FieldTable" << endl;
		string s = xml.getElement("<Entity","/>");
		if (s.length() == 0) 
			error();
		Entity e;
		e.setFromXML(s);
		if (e.getEntityTypeName() != "FieldTable")
			error();
		setEntity(e);
		// Skip the container's entity; but, it has to be there.
		s = xml.getElement("<ContainerEntity","/>");
		if (s.length() == 0) 
			error();

		// Get each row in the table.
		s = xml.getElementContent("<row>","</row>");
		FieldRow *row;
		while (s.length() != 0) {
			// cout << "Parsing a FieldRow" << endl; 
			row = newRow();
			row->setFromXML(s);
			try {
				checkAndAdd(row);
			} catch (DuplicateKey e1) {
				throw ConversionException(e1.getMessage(),"FieldTable");
			} 
			catch (UniquenessViolationException e1) {
				throw ConversionException(e1.getMessage(),"FieldTable");	
			}
			catch (...) {
				// cout << "Unexpected error in FieldTable::checkAndAdd called from FieldTable::fromXML " << endl;
			}
			s = xml.getElementContent("<row>","</row>");
		}
		if (!xml.isStr("</FieldTable>")) 
			error();
			
		archiveAsBin = false;
		fileAsBin = false;
		
	}

	
	void FieldTable::error()  {
		throw ConversionException("Invalid xml document","Field");
	}
	
	
	string FieldTable::MIMEXMLPart(const asdm::ByteOrder* byteOrder) {
		string UID = getEntity().getEntityId().toString();
		string withoutUID = UID.substr(6);
		string containerUID = getContainer().getEntity().getEntityId().toString();
		ostringstream oss;
		oss << "<?xml version='1.0'  encoding='ISO-8859-1'?>";
		oss << "\n";
		oss << "<FieldTable xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" xmlns:field=\"http://Alma/XASDM/FieldTable\" xsi:schemaLocation=\"http://Alma/XASDM/FieldTable http://almaobservatory.org/XML/XASDM/2/FieldTable.xsd\" schemaVersion=\"2\" schemaRevision=\"1.55\">\n";
		oss<< "<Entity entityId='"<<UID<<"' entityIdEncrypted='na' entityTypeName='FieldTable' schemaVersion='1' documentVersion='1'/>\n";
		oss<< "<ContainerEntity entityId='"<<containerUID<<"' entityIdEncrypted='na' entityTypeName='ASDM' schemaVersion='1' documentVersion='1'/>\n";
		oss << "<BulkStoreRef file_id='"<<withoutUID<<"' byteOrder='"<<byteOrder->toString()<<"' />\n";
		oss << "<Attributes>\n";

		oss << "<fieldId/>\n"; 
		oss << "<fieldName/>\n"; 
		oss << "<code/>\n"; 
		oss << "<numPoly/>\n"; 
		oss << "<delayDir/>\n"; 
		oss << "<phaseDir/>\n"; 
		oss << "<referenceDir/>\n"; 

		oss << "<time/>\n"; 
		oss << "<directionCode/>\n"; 
		oss << "<directionEquinox/>\n"; 
		oss << "<assocNature/>\n"; 
		oss << "<ephemerisId/>\n"; 
		oss << "<sourceId/>\n"; 
		oss << "<assocFieldId/>\n"; 
		oss << "</Attributes>\n";		
		oss << "</FieldTable>\n";

		return oss.str();				
	}
	
	string FieldTable::toMIME(const asdm::ByteOrder* byteOrder) {
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

	
	void FieldTable::setFromMIME(const string & mimeMsg) {
    string xmlPartMIMEHeader = "Content-ID: <header.xml>\n\n";
    
    string binPartMIMEHeader = "--MIME_boundary\nContent-Type: binary/octet-stream\nContent-ID: <content.bin>\n\n";
    
    // Detect the XML header.
    string::size_type loc0 = mimeMsg.find(xmlPartMIMEHeader, 0);
    if ( loc0 == string::npos) {
      throw ConversionException("Failed to detect the beginning of the XML header", "Field");
    }
    loc0 += xmlPartMIMEHeader.size();
    
    // Look for the string announcing the binary part.
    string::size_type loc1 = mimeMsg.find( binPartMIMEHeader, loc0 );
    
    if ( loc1 == string::npos ) {
      throw ConversionException("Failed to detect the beginning of the binary part", "Field");
    }
    
    //
    // Extract the xmlHeader and analyze it to find out what is the byte order and the sequence
    // of attribute names.
    //
    string xmlHeader = mimeMsg.substr(loc0, loc1-loc0);
    xmlDoc *doc;
    doc = xmlReadMemory(xmlHeader.data(), xmlHeader.size(), "BinaryTableHeader.xml", NULL, XML_PARSE_NOBLANKS);
    if ( doc == NULL ) 
      throw ConversionException("Failed to parse the xmlHeader into a DOM structure.", "Field");
    
   // This vector will be filled by the names of  all the attributes of the table
   // in the order in which they are expected to be found in the binary representation.
   //
    vector<string> attributesSeq;
      
    xmlNode* root_element = xmlDocGetRootElement(doc);
    if ( root_element == NULL || root_element->type != XML_ELEMENT_NODE )
      throw ConversionException("Failed to parse the xmlHeader into a DOM structure.", "Field");
    
    const ByteOrder* byteOrder;
    if ( string("ASDMBinaryTable").compare((const char*) root_element->name) == 0) {
      // Then it's an "old fashioned" MIME file for tables.
      // Just try to deserialize it with Big_Endian for the bytes ordering.
      byteOrder = asdm::ByteOrder::Big_Endian;
      
 	 //
    // Let's consider a  default order for the sequence of attributes.
    //
     
    attributesSeq.push_back("fieldId") ; 
     
    attributesSeq.push_back("fieldName") ; 
     
    attributesSeq.push_back("code") ; 
     
    attributesSeq.push_back("numPoly") ; 
     
    attributesSeq.push_back("delayDir") ; 
     
    attributesSeq.push_back("phaseDir") ; 
     
    attributesSeq.push_back("referenceDir") ; 
    
     
    attributesSeq.push_back("time") ; 
     
    attributesSeq.push_back("directionCode") ; 
     
    attributesSeq.push_back("directionEquinox") ; 
     
    attributesSeq.push_back("assocNature") ; 
     
    attributesSeq.push_back("ephemerisId") ; 
     
    attributesSeq.push_back("sourceId") ; 
     
    attributesSeq.push_back("assocFieldId") ; 
              
     }
    else if (string("FieldTable").compare((const char*) root_element->name) == 0) {
      // It's a new (and correct) MIME file for tables.
      //
      // 1st )  Look for a BulkStoreRef element with an attribute byteOrder.
      //
      xmlNode* bulkStoreRef = 0;
      xmlNode* child = root_element->children;
      
      // Skip the two first children (Entity and ContainerEntity).
      bulkStoreRef = (child ==  0) ? 0 : ( (child->next) == 0 ? 0 : child->next->next );
      
      if ( bulkStoreRef == 0 || (bulkStoreRef->type != XML_ELEMENT_NODE)  || (string("BulkStoreRef").compare((const char*) bulkStoreRef->name) != 0))
      	throw ConversionException ("Could not find the element '/FieldTable/BulkStoreRef'. Invalid XML header '"+ xmlHeader + "'.", "Field");
      	
      // We found BulkStoreRef, now look for its attribute byteOrder.
      _xmlAttr* byteOrderAttr = 0;
      for (struct _xmlAttr* attr = bulkStoreRef->properties; attr; attr = attr->next) 
	  if (string("byteOrder").compare((const char*) attr->name) == 0) {
	   byteOrderAttr = attr;
	   break;
	 }
      
      if (byteOrderAttr == 0) 
	     throw ConversionException("Could not find the element '/FieldTable/BulkStoreRef/@byteOrder'. Invalid XML header '" + xmlHeader +"'.", "Field");
      
      string byteOrderValue = string((const char*) byteOrderAttr->children->content);
      if (!(byteOrder = asdm::ByteOrder::fromString(byteOrderValue)))
		throw ConversionException("No valid value retrieved for the element '/FieldTable/BulkStoreRef/@byteOrder'. Invalid XML header '" + xmlHeader + "'.", "Field");
		
	 //
	 // 2nd) Look for the Attributes element and grab the names of the elements it contains.
	 //
	 xmlNode* attributes = bulkStoreRef->next;
     if ( attributes == 0 || (attributes->type != XML_ELEMENT_NODE)  || (string("Attributes").compare((const char*) attributes->name) != 0))	 
       	throw ConversionException ("Could not find the element '/FieldTable/Attributes'. Invalid XML header '"+ xmlHeader + "'.", "Field");
 
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
	FieldRow* aRow = FieldRow::fromBin(eiss, *this, attributesSeq);
	checkAndAdd(aRow);
      }
    }
    catch (DuplicateKey e) {
      throw ConversionException("Error while writing binary data , the message was "
				+ e.getMessage(), "Field");
    }
    catch (TagFormatException e) {
      throw ConversionException("Error while reading binary data , the message was "
				+ e.getMessage(), "Field");
    }
    archiveAsBin = true;
    fileAsBin = true;
	}

	
	void FieldTable::toFile(string directory) {
		if (!directoryExists(directory.c_str()) &&
			!createPath(directory.c_str())) {
			throw ConversionException("Could not create directory " , directory);
		}

		string fileName = directory + "/Field.xml";
		ofstream tableout(fileName.c_str(),ios::out|ios::trunc);
		if (tableout.rdstate() == ostream::failbit)
			throw ConversionException("Could not open file " + fileName + " to write ", "Field");
		if (fileAsBin) 
			tableout << MIMEXMLPart();
		else
			tableout << toXML() << endl;
		tableout.close();
		if (tableout.rdstate() == ostream::failbit)
			throw ConversionException("Could not close file " + fileName, "Field");

		if (fileAsBin) {
			// write the bin serialized
			string fileName = directory + "/Field.bin";
			ofstream tableout(fileName.c_str(),ios::out|ios::trunc);
			if (tableout.rdstate() == ostream::failbit)
				throw ConversionException("Could not open file " + fileName + " to write ", "Field");
			tableout << toMIME() << endl;
			tableout.close();
			if (tableout.rdstate() == ostream::failbit)
				throw ConversionException("Could not close file " + fileName, "Field");
		}
	}

	
	void FieldTable::setFromFile(const string& directory) {		
    if (boost::filesystem::exists(boost::filesystem::path(uniqSlashes(directory + "/Field.xml"))))
      setFromXMLFile(directory);
    else if (boost::filesystem::exists(boost::filesystem::path(uniqSlashes(directory + "/Field.bin"))))
      setFromMIMEFile(directory);
    else
      throw ConversionException("No file found for the Field table", "Field");
	}			

	
  void FieldTable::setFromMIMEFile(const string& directory) {
    string tablePath ;
    
    tablePath = directory + "/Field.bin";
    ifstream tablefile(tablePath.c_str(), ios::in|ios::binary);
    if (!tablefile.is_open()) { 
      throw ConversionException("Could not open file " + tablePath, "Field");
    }
    // Read in a stringstream.
    stringstream ss; ss << tablefile.rdbuf();
    
    if (tablefile.rdstate() == istream::failbit || tablefile.rdstate() == istream::badbit) {
      throw ConversionException("Error reading file " + tablePath,"Field");
    }
    
    // And close.
    tablefile.close();
    if (tablefile.rdstate() == istream::failbit)
      throw ConversionException("Could not close file " + tablePath,"Field");
    
    setFromMIME(ss.str());
  }	

	
void FieldTable::setFromXMLFile(const string& directory) {
    string tablePath ;
    
    tablePath = directory + "/Field.xml";
    ifstream tablefile(tablePath.c_str(), ios::in|ios::binary);
    if (!tablefile.is_open()) { 
      throw ConversionException("Could not open file " + tablePath, "Field");
    }
      // Read in a stringstream.
    stringstream ss;
    ss << tablefile.rdbuf();
    
    if  (tablefile.rdstate() == istream::failbit || tablefile.rdstate() == istream::badbit) {
      throw ConversionException("Error reading file '" + tablePath + "'", "Field");
    }
    
    // And close
    tablefile.close();
    if (tablefile.rdstate() == istream::failbit)
      throw ConversionException("Could not close file '" + tablePath + "'", "Field");

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

	

	

			
	
	

	
	void FieldTable::autoIncrement(string key, FieldRow* x) {
		map<string, int>::iterator iter;
		if ((iter=noAutoIncIds.find(key)) == noAutoIncIds.end()) {
			// There is not yet a combination of the non autoinc attributes values in the hashtable
			
			// Initialize  fieldId to Tag(0).
			x->setFieldId(Tag(0,  TagType::Field));
			
			// Record it in the map.		
			noAutoIncIds.insert(make_pair(key, 0));			
		} 
		else {
			// There is already a combination of the non autoinc attributes values in the hashtable
			// Increment its value.
			int n = iter->second + 1; 
			
			// Initialize  fieldId to Tag(n).
			x->setFieldId(Tag(n, TagType::Field));
			
			// Record it in the map.		
			noAutoIncIds.insert(make_pair(key, n));				
		}		
	}
	
} // End namespace asdm
 
