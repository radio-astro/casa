
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
 * File FocusModelTable.cpp
 */
#include <ConversionException.h>
#include <DuplicateKey.h>
#include <OutOfBoundsException.h>

using asdm::ConversionException;
using asdm::DuplicateKey;
using asdm::OutOfBoundsException;

#include <ASDM.h>
#include <FocusModelTable.h>
#include <FocusModelRow.h>
#include <Parser.h>

using asdm::ASDM;
using asdm::FocusModelTable;
using asdm::FocusModelRow;
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

	string FocusModelTable::tableName = "FocusModel";
	const vector<string> FocusModelTable::attributesNames = initAttributesNames();
		

	/**
	 * The list of field names that make up key key.
	 * (Initialization is in the constructor.)
	 */
	vector<string> FocusModelTable::key;

	/**
	 * Return the list of field names that make up key key
	 * as an array of strings.
	 */	
	vector<string> FocusModelTable::getKeyName() {
		return key;
	}


	FocusModelTable::FocusModelTable(ASDM &c) : container(c) {

	
		key.push_back("antennaId");
	
		key.push_back("focusModelId");
	


		// Define a default entity.
		entity.setEntityId(EntityId("uid://X0/X0/X0"));
		entity.setEntityIdEncrypted("na");
		entity.setEntityTypeName("FocusModelTable");
		entity.setEntityVersion("1");
		entity.setInstanceVersion("1");
		
		// Archive XML
		archiveAsBin = false;
		
		// File XML
		fileAsBin = false;
	}
	
/**
 * A destructor for FocusModelTable.
 */
	FocusModelTable::~FocusModelTable() {
		for (unsigned int i = 0; i < privateRows.size(); i++) 
			delete(privateRows.at(i));
	}

	/**
	 * Container to which this table belongs.
	 */
	ASDM &FocusModelTable::getContainer() const {
		return container;
	}

	/**
	 * Return the number of rows in the table.
	 */
	unsigned int FocusModelTable::size() {
		return privateRows.size();
	}
	
	/**
	 * Return the name of this table.
	 */
	string FocusModelTable::getName() const {
		return tableName;
	}
	
	/**
	 * Build the vector of attributes names.
	 */
	vector<string> FocusModelTable::initAttributesNames() {
		vector<string> attributesNames;

		attributesNames.push_back("antennaId");

		attributesNames.push_back("focusModelId");


		attributesNames.push_back("polarizationType");

		attributesNames.push_back("receiverBand");

		attributesNames.push_back("numCoeff");

		attributesNames.push_back("coeffName");

		attributesNames.push_back("coeffFormula");

		attributesNames.push_back("coeffVal");

		attributesNames.push_back("assocNature");

		attributesNames.push_back("assocFocusModelId");


		return attributesNames;
	}
	
	/**
	 * Return the names of the attributes.
	 */
	const vector<string>& FocusModelTable::getAttributesNames() { return attributesNames; }

	/**
	 * Return this table's Entity.
	 */
	Entity FocusModelTable::getEntity() const {
		return entity;
	}

	/**
	 * Set this table's Entity.
	 */
	void FocusModelTable::setEntity(Entity e) {
		this->entity = e; 
	}
	
	//
	// ====> Row creation.
	//
	
	/**
	 * Create a new row.
	 */
	FocusModelRow *FocusModelTable::newRow() {
		return new FocusModelRow (*this);
	}
	

	/**
	 * Create a new row initialized to the specified values.
	 * @return a pointer on the created and initialized row.
	
 	 * @param antennaId 
	
 	 * @param polarizationType 
	
 	 * @param receiverBand 
	
 	 * @param numCoeff 
	
 	 * @param coeffName 
	
 	 * @param coeffFormula 
	
 	 * @param coeffVal 
	
 	 * @param assocNature 
	
 	 * @param assocFocusModelId 
	
     */
	FocusModelRow* FocusModelTable::newRow(Tag antennaId, PolarizationTypeMod::PolarizationType polarizationType, ReceiverBandMod::ReceiverBand receiverBand, int numCoeff, vector<string > coeffName, vector<string > coeffFormula, vector<float > coeffVal, string assocNature, int assocFocusModelId){
		FocusModelRow *row = new FocusModelRow(*this);
			
		row->setAntennaId(antennaId);
			
		row->setPolarizationType(polarizationType);
			
		row->setReceiverBand(receiverBand);
			
		row->setNumCoeff(numCoeff);
			
		row->setCoeffName(coeffName);
			
		row->setCoeffFormula(coeffFormula);
			
		row->setCoeffVal(coeffVal);
			
		row->setAssocNature(assocNature);
			
		row->setAssocFocusModelId(assocFocusModelId);
	
		return row;		
	}	
	


FocusModelRow* FocusModelTable::newRow(FocusModelRow* row) {
	return new FocusModelRow(*this, *row);
}

	//
	// Append a row to its table.
	//

	
	 
	
	/** 
 	 * Look up the table for a row whose noautoincrementable attributes are matching their
 	 * homologues in *x.  If a row is found  this row else autoincrement  *x.focusModelId, 
 	 * add x to its table and returns x.
 	 *  
 	 * @returns a pointer on a FocusModelRow.
 	 * @param x. A pointer on the row to be added.
 	 */ 
 		
			
	FocusModelRow* FocusModelTable::add(FocusModelRow* x) {
		FocusModelRow* aRow = lookup(
				
			x->getAntennaId()
				,
			x->getPolarizationType()
				,
			x->getReceiverBand()
				,
			x->getNumCoeff()
				,
			x->getCoeffName()
				,
			x->getCoeffFormula()
				,
			x->getCoeffVal()
				,
			x->getAssocNature()
				,
			x->getAssocFocusModelId()
				
		);
		if (aRow) return aRow;

		// Autoincrementation algorithm. We use the hashtable.
		ostringstream noAutoIncIdsEntry;
		noAutoIncIdsEntry
			
				
		<< x->antennaId.toString() << "_"
					
			
		;
		
		map<string, int>::iterator iter;
		if ((iter=noAutoIncIds.find(noAutoIncIdsEntry.str())) == noAutoIncIds.end()) {
			// There is not yet a combination of the non autoinc attributes values in the hashtable

			
			// Initialize  focusModelId to 0.
			x->focusModelId = 0;		
			//x->setFocusModelId(0);
			
			// Record it in the map.		
			noAutoIncIds.insert(make_pair(noAutoIncIdsEntry.str(), 0));			
		} 
		else {
			// There is already a combination of the non autoinc attributes values in the hashtable
			// Increment its value.
			int n = iter->second + 1; 
			
			// Initialize  focusModelId to n.		
			//x->setFocusModelId(n);
			x->focusModelId = n;
			
			// Record it in the map.		
			noAutoIncIds.erase(iter);
			noAutoIncIds.insert(make_pair(noAutoIncIdsEntry.str(), n));				
		}	
		
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
	FocusModelRow*  FocusModelTable::checkAndAdd(FocusModelRow* x)  {
	 
		 
		if (lookup(
			
			x->getAntennaId()
		,
			x->getPolarizationType()
		,
			x->getReceiverBand()
		,
			x->getNumCoeff()
		,
			x->getCoeffName()
		,
			x->getCoeffFormula()
		,
			x->getCoeffVal()
		,
			x->getAssocNature()
		,
			x->getAssocFocusModelId()
		
		)) throw UniquenessViolationException("Uniqueness violation exception in table FocusModelTable");
		
		
		
		if (getRowByKey(
	
			x->getAntennaId()
	,
			x->getFocusModelId()
			
		)) throw DuplicateKey("Duplicate key exception in ", "FocusModelTable");
		
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
	 * @return Alls rows as an array of FocusModelRow
	 */
	vector<FocusModelRow *> FocusModelTable::get() {
		return privateRows;
		// return row;
	}

	
/*
 ** Returns a FocusModelRow* given a key.
 ** @return a pointer to the row having the key whose values are passed as parameters, or 0 if
 ** no row exists for that key.
 **
 */
 	FocusModelRow* FocusModelTable::getRowByKey(Tag antennaId, int focusModelId)  {
	FocusModelRow* aRow = 0;
	for (unsigned int i = 0; i < row.size(); i++) {
		aRow = row.at(i);
		
			
				if (aRow->antennaId != antennaId) continue;
			
		
			
				if (aRow->focusModelId != focusModelId) continue;
			
		
		return aRow;
	}
	return 0;		
}
	

	
/**
 * Look up the table for a row whose all attributes  except the autoincrementable one 
 * are equal to the corresponding parameters of the method.
 * @return a pointer on this row if any, 0 otherwise.
 *
			
 * @param antennaId.
 	 		
 * @param polarizationType.
 	 		
 * @param receiverBand.
 	 		
 * @param numCoeff.
 	 		
 * @param coeffName.
 	 		
 * @param coeffFormula.
 	 		
 * @param coeffVal.
 	 		
 * @param assocNature.
 	 		
 * @param assocFocusModelId.
 	 		 
 */
FocusModelRow* FocusModelTable::lookup(Tag antennaId, PolarizationTypeMod::PolarizationType polarizationType, ReceiverBandMod::ReceiverBand receiverBand, int numCoeff, vector<string > coeffName, vector<string > coeffFormula, vector<float > coeffVal, string assocNature, int assocFocusModelId) {
		FocusModelRow* aRow;
		for (unsigned int i = 0; i < size(); i++) {
			aRow = row.at(i); 
			if (aRow->compareNoAutoInc(antennaId, polarizationType, receiverBand, numCoeff, coeffName, coeffFormula, coeffVal, assocNature, assocFocusModelId)) return aRow;
		}			
		return 0;	
} 
	
 	 	

	 	
/*
 * Returns a vector of pointers on rows whose key element focusModelId 
 * is equal to the parameter focusModelId.
 * @return a vector of vector <FocusModelRow *>. A returned vector of size 0 means that no row has been found.
 * @param focusModelId int contains the value of
 * the autoincrementable attribute that is looked up in the table.
 */
 vector <FocusModelRow *>  FocusModelTable::getRowByFocusModelId(int focusModelId) {
	vector<FocusModelRow *> list;
	for (unsigned int i = 0; i < row.size(); ++i) {
		FocusModelRow &x = *row[i];
					
		if (x.focusModelId == focusModelId)
			
		list.push_back(row[i]);
	}
	//if (list.size() == 0) throw  NoSuchRow("","FocusModel");
	return list;	
 }
	




#ifndef WITHOUT_ACS
	// Conversion Methods

	FocusModelTableIDL *FocusModelTable::toIDL() {
		FocusModelTableIDL *x = new FocusModelTableIDL ();
		unsigned int nrow = size();
		x->row.length(nrow);
		vector<FocusModelRow*> v = get();
		for (unsigned int i = 0; i < nrow; ++i) {
			x->row[i] = *(v[i]->toIDL());
		}
		return x;
	}
#endif
	
#ifndef WITHOUT_ACS
	void FocusModelTable::fromIDL(FocusModelTableIDL x) {
		unsigned int nrow = x.row.length();
		for (unsigned int i = 0; i < nrow; ++i) {
			FocusModelRow *tmp = newRow();
			tmp->setFromIDL(x.row[i]);
			// checkAndAdd(tmp);
			add(tmp);
		}
	}
#endif

	
	string FocusModelTable::toXML()  {
		string buf;

		buf.append("<?xml version=\"1.0\" encoding=\"ISO-8859-1\"?> ");
		buf.append("<FocusModelTable xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" xmlns:focsm=\"http://Alma/XASDM/FocusModelTable\" xsi:schemaLocation=\"http://Alma/XASDM/FocusModelTable http://almaobservatory.org/XML/XASDM/2/FocusModelTable.xsd\" schemaVersion=\"2\" schemaRevision=\"1.54\">\n");
	
		buf.append(entity.toXML());
		string s = container.getEntity().toXML();
		// Change the "Entity" tag to "ContainerEntity".
		buf.append("<Container" + s.substr(1,s.length() - 1)+" ");
		vector<FocusModelRow*> v = get();
		for (unsigned int i = 0; i < v.size(); ++i) {
			try {
				buf.append(v[i]->toXML());
			} catch (NoSuchRow e) {
			}
			buf.append("  ");
		}		
		buf.append("</FocusModelTable> ");
		return buf;
	}

	
	void FocusModelTable::fromXML(string xmlDoc)  {
		Parser xml(xmlDoc);
		if (!xml.isStr("<FocusModelTable")) 
			error();
		// cout << "Parsing a FocusModelTable" << endl;
		string s = xml.getElement("<Entity","/>");
		if (s.length() == 0) 
			error();
		Entity e;
		e.setFromXML(s);
		if (e.getEntityTypeName() != "FocusModelTable")
			error();
		setEntity(e);
		// Skip the container's entity; but, it has to be there.
		s = xml.getElement("<ContainerEntity","/>");
		if (s.length() == 0) 
			error();

		// Get each row in the table.
		s = xml.getElementContent("<row>","</row>");
		FocusModelRow *row;
		while (s.length() != 0) {
			// cout << "Parsing a FocusModelRow" << endl; 
			row = newRow();
			row->setFromXML(s);
			try {
				checkAndAdd(row);
			} catch (DuplicateKey e1) {
				throw ConversionException(e1.getMessage(),"FocusModelTable");
			} 
			catch (UniquenessViolationException e1) {
				throw ConversionException(e1.getMessage(),"FocusModelTable");	
			}
			catch (...) {
				// cout << "Unexpected error in FocusModelTable::checkAndAdd called from FocusModelTable::fromXML " << endl;
			}
			s = xml.getElementContent("<row>","</row>");
		}
		if (!xml.isStr("</FocusModelTable>")) 
			error();
			
		archiveAsBin = false;
		fileAsBin = false;
		
	}

	
	void FocusModelTable::error()  {
		throw ConversionException("Invalid xml document","FocusModel");
	}
	
	
	string FocusModelTable::MIMEXMLPart(const asdm::ByteOrder* byteOrder) {
		string UID = getEntity().getEntityId().toString();
		string withoutUID = UID.substr(6);
		string containerUID = getContainer().getEntity().getEntityId().toString();
		ostringstream oss;
		oss << "<?xml version='1.0'  encoding='ISO-8859-1'?>";
		oss << "\n";
		oss << "<FocusModelTable xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" xmlns:focsm=\"http://Alma/XASDM/FocusModelTable\" xsi:schemaLocation=\"http://Alma/XASDM/FocusModelTable http://almaobservatory.org/XML/XASDM/2/FocusModelTable.xsd\" schemaVersion=\"2\" schemaRevision=\"1.54\">\n";
		oss<< "<Entity entityId='"<<UID<<"' entityIdEncrypted='na' entityTypeName='FocusModelTable' schemaVersion='1' documentVersion='1'/>\n";
		oss<< "<ContainerEntity entityId='"<<containerUID<<"' entityIdEncrypted='na' entityTypeName='ASDM' schemaVersion='1' documentVersion='1'/>\n";
		oss << "<BulkStoreRef file_id='"<<withoutUID<<"' byteOrder='"<<byteOrder->toString()<<"' />\n";
		oss << "<Attributes>\n";

		oss << "<antennaId/>\n"; 
		oss << "<focusModelId/>\n"; 
		oss << "<polarizationType/>\n"; 
		oss << "<receiverBand/>\n"; 
		oss << "<numCoeff/>\n"; 
		oss << "<coeffName/>\n"; 
		oss << "<coeffFormula/>\n"; 
		oss << "<coeffVal/>\n"; 
		oss << "<assocNature/>\n"; 
		oss << "<assocFocusModelId/>\n"; 

		oss << "</Attributes>\n";		
		oss << "</FocusModelTable>\n";

		return oss.str();				
	}
	
	string FocusModelTable::toMIME(const asdm::ByteOrder* byteOrder) {
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

	
	void FocusModelTable::setFromMIME(const string & mimeMsg) {
    string xmlPartMIMEHeader = "Content-ID: <header.xml>\n\n";
    
    string binPartMIMEHeader = "--MIME_boundary\nContent-Type: binary/octet-stream\nContent-ID: <content.bin>\n\n";
    
    // Detect the XML header.
    string::size_type loc0 = mimeMsg.find(xmlPartMIMEHeader, 0);
    if ( loc0 == string::npos) {
      throw ConversionException("Failed to detect the beginning of the XML header", "FocusModel");
    }
    loc0 += xmlPartMIMEHeader.size();
    
    // Look for the string announcing the binary part.
    string::size_type loc1 = mimeMsg.find( binPartMIMEHeader, loc0 );
    
    if ( loc1 == string::npos ) {
      throw ConversionException("Failed to detect the beginning of the binary part", "FocusModel");
    }
    
    //
    // Extract the xmlHeader and analyze it to find out what is the byte order and the sequence
    // of attribute names.
    //
    string xmlHeader = mimeMsg.substr(loc0, loc1-loc0);
    xmlDoc *doc;
    doc = xmlReadMemory(xmlHeader.data(), xmlHeader.size(), "BinaryTableHeader.xml", NULL, XML_PARSE_NOBLANKS);
    if ( doc == NULL ) 
      throw ConversionException("Failed to parse the xmlHeader into a DOM structure.", "FocusModel");
    
   // This vector will be filled by the names of  all the attributes of the table
   // in the order in which they are expected to be found in the binary representation.
   //
    vector<string> attributesSeq;
      
    xmlNode* root_element = xmlDocGetRootElement(doc);
    if ( root_element == NULL || root_element->type != XML_ELEMENT_NODE )
      throw ConversionException("Failed to parse the xmlHeader into a DOM structure.", "FocusModel");
    
    const ByteOrder* byteOrder;
    if ( string("ASDMBinaryTable").compare((const char*) root_element->name) == 0) {
      // Then it's an "old fashioned" MIME file for tables.
      // Just try to deserialize it with Big_Endian for the bytes ordering.
      byteOrder = asdm::ByteOrder::Big_Endian;
      
 	 //
    // Let's consider a  default order for the sequence of attributes.
    //
     
    attributesSeq.push_back("antennaId") ; 
     
    attributesSeq.push_back("focusModelId") ; 
     
    attributesSeq.push_back("polarizationType") ; 
     
    attributesSeq.push_back("receiverBand") ; 
     
    attributesSeq.push_back("numCoeff") ; 
     
    attributesSeq.push_back("coeffName") ; 
     
    attributesSeq.push_back("coeffFormula") ; 
     
    attributesSeq.push_back("coeffVal") ; 
     
    attributesSeq.push_back("assocNature") ; 
     
    attributesSeq.push_back("assocFocusModelId") ; 
    
              
     }
    else if (string("FocusModelTable").compare((const char*) root_element->name) == 0) {
      // It's a new (and correct) MIME file for tables.
      //
      // 1st )  Look for a BulkStoreRef element with an attribute byteOrder.
      //
      xmlNode* bulkStoreRef = 0;
      xmlNode* child = root_element->children;
      
      // Skip the two first children (Entity and ContainerEntity).
      bulkStoreRef = (child ==  0) ? 0 : ( (child->next) == 0 ? 0 : child->next->next );
      
      if ( bulkStoreRef == 0 || (bulkStoreRef->type != XML_ELEMENT_NODE)  || (string("BulkStoreRef").compare((const char*) bulkStoreRef->name) != 0))
      	throw ConversionException ("Could not find the element '/FocusModelTable/BulkStoreRef'. Invalid XML header '"+ xmlHeader + "'.", "FocusModel");
      	
      // We found BulkStoreRef, now look for its attribute byteOrder.
      _xmlAttr* byteOrderAttr = 0;
      for (struct _xmlAttr* attr = bulkStoreRef->properties; attr; attr = attr->next) 
	  if (string("byteOrder").compare((const char*) attr->name) == 0) {
	   byteOrderAttr = attr;
	   break;
	 }
      
      if (byteOrderAttr == 0) 
	     throw ConversionException("Could not find the element '/FocusModelTable/BulkStoreRef/@byteOrder'. Invalid XML header '" + xmlHeader +"'.", "FocusModel");
      
      string byteOrderValue = string((const char*) byteOrderAttr->children->content);
      if (!(byteOrder = asdm::ByteOrder::fromString(byteOrderValue)))
		throw ConversionException("No valid value retrieved for the element '/FocusModelTable/BulkStoreRef/@byteOrder'. Invalid XML header '" + xmlHeader + "'.", "FocusModel");
		
	 //
	 // 2nd) Look for the Attributes element and grab the names of the elements it contains.
	 //
	 xmlNode* attributes = bulkStoreRef->next;
     if ( attributes == 0 || (attributes->type != XML_ELEMENT_NODE)  || (string("Attributes").compare((const char*) attributes->name) != 0))	 
       	throw ConversionException ("Could not find the element '/FocusModelTable/Attributes'. Invalid XML header '"+ xmlHeader + "'.", "FocusModel");
 
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
	FocusModelRow* aRow = FocusModelRow::fromBin(eiss, *this, attributesSeq);
	checkAndAdd(aRow);
      }
    }
    catch (DuplicateKey e) {
      throw ConversionException("Error while writing binary data , the message was "
				+ e.getMessage(), "FocusModel");
    }
    catch (TagFormatException e) {
      throw ConversionException("Error while reading binary data , the message was "
				+ e.getMessage(), "FocusModel");
    }
    archiveAsBin = true;
    fileAsBin = true;
	}

	
	void FocusModelTable::toFile(string directory) {
		if (!directoryExists(directory.c_str()) &&
			!createPath(directory.c_str())) {
			throw ConversionException("Could not create directory " , directory);
		}

		string fileName = directory + "/FocusModel.xml";
		ofstream tableout(fileName.c_str(),ios::out|ios::trunc);
		if (tableout.rdstate() == ostream::failbit)
			throw ConversionException("Could not open file " + fileName + " to write ", "FocusModel");
		if (fileAsBin) 
			tableout << MIMEXMLPart();
		else
			tableout << toXML() << endl;
		tableout.close();
		if (tableout.rdstate() == ostream::failbit)
			throw ConversionException("Could not close file " + fileName, "FocusModel");

		if (fileAsBin) {
			// write the bin serialized
			string fileName = directory + "/FocusModel.bin";
			ofstream tableout(fileName.c_str(),ios::out|ios::trunc);
			if (tableout.rdstate() == ostream::failbit)
				throw ConversionException("Could not open file " + fileName + " to write ", "FocusModel");
			tableout << toMIME() << endl;
			tableout.close();
			if (tableout.rdstate() == ostream::failbit)
				throw ConversionException("Could not close file " + fileName, "FocusModel");
		}
	}

	
	void FocusModelTable::setFromFile(const string& directory) {		
    if (boost::filesystem::exists(boost::filesystem::path(uniqSlashes(directory + "/FocusModel.xml"))))
      setFromXMLFile(directory);
    else if (boost::filesystem::exists(boost::filesystem::path(uniqSlashes(directory + "/FocusModel.bin"))))
      setFromMIMEFile(directory);
    else
      throw ConversionException("No file found for the FocusModel table", "FocusModel");
	}			

	
  void FocusModelTable::setFromMIMEFile(const string& directory) {
    string tablePath ;
    
    tablePath = directory + "/FocusModel.bin";
    ifstream tablefile(tablePath.c_str(), ios::in|ios::binary);
    if (!tablefile.is_open()) { 
      throw ConversionException("Could not open file " + tablePath, "FocusModel");
    }
    // Read in a stringstream.
    stringstream ss; ss << tablefile.rdbuf();
    
    if (tablefile.rdstate() == istream::failbit || tablefile.rdstate() == istream::badbit) {
      throw ConversionException("Error reading file " + tablePath,"FocusModel");
    }
    
    // And close.
    tablefile.close();
    if (tablefile.rdstate() == istream::failbit)
      throw ConversionException("Could not close file " + tablePath,"FocusModel");
    
    setFromMIME(ss.str());
  }	

	
void FocusModelTable::setFromXMLFile(const string& directory) {
    string tablePath ;
    
    tablePath = directory + "/FocusModel.xml";
    ifstream tablefile(tablePath.c_str(), ios::in|ios::binary);
    if (!tablefile.is_open()) { 
      throw ConversionException("Could not open file " + tablePath, "FocusModel");
    }
      // Read in a stringstream.
    stringstream ss;
    ss << tablefile.rdbuf();
    
    if  (tablefile.rdstate() == istream::failbit || tablefile.rdstate() == istream::badbit) {
      throw ConversionException("Error reading file '" + tablePath + "'", "FocusModel");
    }
    
    // And close
    tablefile.close();
    if (tablefile.rdstate() == istream::failbit)
      throw ConversionException("Could not close file '" + tablePath + "'", "FocusModel");

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

	

	

			
	
	

	
	void FocusModelTable::autoIncrement(string key, FocusModelRow* x) {
		map<string, int>::iterator iter;
		if ((iter=noAutoIncIds.find(key)) == noAutoIncIds.end()) {
			// There is not yet a combination of the non autoinc attributes values in the hashtable
			
			// Initialize  focusModelId to 0.		
			x->setFocusModelId(0);
			
			// Record it in the map.		
			noAutoIncIds.insert(make_pair(key, 0));			
		} 
		else {
			// There is already a combination of the non autoinc attributes values in the hashtable
			// Increment its value.
			int n = iter->second + 1; 
			
			// Initialize  focusModelId to n.		
			x->setFocusModelId(n);
			
			// Record it in the map.		
			noAutoIncIds.insert(make_pair(key, n));				
		}		
	}
	
} // End namespace asdm
 
