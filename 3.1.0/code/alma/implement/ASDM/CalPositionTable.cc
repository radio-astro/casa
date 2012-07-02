
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
 * File CalPositionTable.cpp
 */
#include <ConversionException.h>
#include <DuplicateKey.h>
#include <OutOfBoundsException.h>

using asdm::ConversionException;
using asdm::DuplicateKey;
using asdm::OutOfBoundsException;

#include <ASDM.h>
#include <CalPositionTable.h>
#include <CalPositionRow.h>
#include <Parser.h>

using asdm::ASDM;
using asdm::CalPositionTable;
using asdm::CalPositionRow;
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

	string CalPositionTable::tableName = "CalPosition";
	const vector<string> CalPositionTable::attributesNames = initAttributesNames();
		

	/**
	 * The list of field names that make up key key.
	 * (Initialization is in the constructor.)
	 */
	vector<string> CalPositionTable::key;

	/**
	 * Return the list of field names that make up key key
	 * as an array of strings.
	 */	
	vector<string> CalPositionTable::getKeyName() {
		return key;
	}


	CalPositionTable::CalPositionTable(ASDM &c) : container(c) {

	
		key.push_back("antennaName");
	
		key.push_back("atmPhaseCorrection");
	
		key.push_back("calDataId");
	
		key.push_back("calReductionId");
	


		// Define a default entity.
		entity.setEntityId(EntityId("uid://X0/X0/X0"));
		entity.setEntityIdEncrypted("na");
		entity.setEntityTypeName("CalPositionTable");
		entity.setEntityVersion("1");
		entity.setInstanceVersion("1");
		
		// Archive XML
		archiveAsBin = false;
		
		// File XML
		fileAsBin = false;
	}
	
/**
 * A destructor for CalPositionTable.
 */
	CalPositionTable::~CalPositionTable() {
		for (unsigned int i = 0; i < privateRows.size(); i++) 
			delete(privateRows.at(i));
	}

	/**
	 * Container to which this table belongs.
	 */
	ASDM &CalPositionTable::getContainer() const {
		return container;
	}

	/**
	 * Return the number of rows in the table.
	 */
	unsigned int CalPositionTable::size() {
		return privateRows.size();
	}
	
	/**
	 * Return the name of this table.
	 */
	string CalPositionTable::getName() const {
		return tableName;
	}
	
	/**
	 * Build the vector of attributes names.
	 */
	vector<string> CalPositionTable::initAttributesNames() {
		vector<string> attributesNames;

		attributesNames.push_back("antennaName");

		attributesNames.push_back("atmPhaseCorrection");

		attributesNames.push_back("calDataId");

		attributesNames.push_back("calReductionId");


		attributesNames.push_back("startValidTime");

		attributesNames.push_back("endValidTime");

		attributesNames.push_back("antennaPosition");

		attributesNames.push_back("stationName");

		attributesNames.push_back("stationPosition");

		attributesNames.push_back("positionMethod");

		attributesNames.push_back("receiverBand");

		attributesNames.push_back("numAntenna");

		attributesNames.push_back("refAntennaNames");

		attributesNames.push_back("axesOffset");

		attributesNames.push_back("axesOffsetErr");

		attributesNames.push_back("axesOffsetFixed");

		attributesNames.push_back("positionOffset");

		attributesNames.push_back("positionErr");

		attributesNames.push_back("reducedChiSquared");


		attributesNames.push_back("delayRms");

		attributesNames.push_back("phaseRms");

		return attributesNames;
	}
	
	/**
	 * Return the names of the attributes.
	 */
	const vector<string>& CalPositionTable::getAttributesNames() { return attributesNames; }

	/**
	 * Return this table's Entity.
	 */
	Entity CalPositionTable::getEntity() const {
		return entity;
	}

	/**
	 * Set this table's Entity.
	 */
	void CalPositionTable::setEntity(Entity e) {
		this->entity = e; 
	}
	
	//
	// ====> Row creation.
	//
	
	/**
	 * Create a new row.
	 */
	CalPositionRow *CalPositionTable::newRow() {
		return new CalPositionRow (*this);
	}
	

	/**
	 * Create a new row initialized to the specified values.
	 * @return a pointer on the created and initialized row.
	
 	 * @param antennaName 
	
 	 * @param atmPhaseCorrection 
	
 	 * @param calDataId 
	
 	 * @param calReductionId 
	
 	 * @param startValidTime 
	
 	 * @param endValidTime 
	
 	 * @param antennaPosition 
	
 	 * @param stationName 
	
 	 * @param stationPosition 
	
 	 * @param positionMethod 
	
 	 * @param receiverBand 
	
 	 * @param numAntenna 
	
 	 * @param refAntennaNames 
	
 	 * @param axesOffset 
	
 	 * @param axesOffsetErr 
	
 	 * @param axesOffsetFixed 
	
 	 * @param positionOffset 
	
 	 * @param positionErr 
	
 	 * @param reducedChiSquared 
	
     */
	CalPositionRow* CalPositionTable::newRow(string antennaName, AtmPhaseCorrectionMod::AtmPhaseCorrection atmPhaseCorrection, Tag calDataId, Tag calReductionId, ArrayTime startValidTime, ArrayTime endValidTime, vector<Length > antennaPosition, string stationName, vector<Length > stationPosition, PositionMethodMod::PositionMethod positionMethod, ReceiverBandMod::ReceiverBand receiverBand, int numAntenna, vector<string > refAntennaNames, Length axesOffset, Length axesOffsetErr, bool axesOffsetFixed, vector<Length > positionOffset, vector<Length > positionErr, double reducedChiSquared){
		CalPositionRow *row = new CalPositionRow(*this);
			
		row->setAntennaName(antennaName);
			
		row->setAtmPhaseCorrection(atmPhaseCorrection);
			
		row->setCalDataId(calDataId);
			
		row->setCalReductionId(calReductionId);
			
		row->setStartValidTime(startValidTime);
			
		row->setEndValidTime(endValidTime);
			
		row->setAntennaPosition(antennaPosition);
			
		row->setStationName(stationName);
			
		row->setStationPosition(stationPosition);
			
		row->setPositionMethod(positionMethod);
			
		row->setReceiverBand(receiverBand);
			
		row->setNumAntenna(numAntenna);
			
		row->setRefAntennaNames(refAntennaNames);
			
		row->setAxesOffset(axesOffset);
			
		row->setAxesOffsetErr(axesOffsetErr);
			
		row->setAxesOffsetFixed(axesOffsetFixed);
			
		row->setPositionOffset(positionOffset);
			
		row->setPositionErr(positionErr);
			
		row->setReducedChiSquared(reducedChiSquared);
	
		return row;		
	}	
	


CalPositionRow* CalPositionTable::newRow(CalPositionRow* row) {
	return new CalPositionRow(*this, *row);
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
	CalPositionRow* CalPositionTable::add(CalPositionRow* x) {
		
		if (getRowByKey(
						x->getAntennaName()
						,
						x->getAtmPhaseCorrection()
						,
						x->getCalDataId()
						,
						x->getCalReductionId()
						))
			//throw DuplicateKey(x.getAntennaName() + "|" + x.getAtmPhaseCorrection() + "|" + x.getCalDataId() + "|" + x.getCalReductionId(),"CalPosition");
			throw DuplicateKey("Duplicate key exception in ","CalPositionTable");
		
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
	CalPositionRow*  CalPositionTable::checkAndAdd(CalPositionRow* x)  {
		
		
		if (getRowByKey(
	
			x->getAntennaName()
	,
			x->getAtmPhaseCorrection()
	,
			x->getCalDataId()
	,
			x->getCalReductionId()
			
		)) throw DuplicateKey("Duplicate key exception in ", "CalPositionTable");
		
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
	 * @return Alls rows as an array of CalPositionRow
	 */
	vector<CalPositionRow *> CalPositionTable::get() {
		return privateRows;
		// return row;
	}

	
/*
 ** Returns a CalPositionRow* given a key.
 ** @return a pointer to the row having the key whose values are passed as parameters, or 0 if
 ** no row exists for that key.
 **
 */
 	CalPositionRow* CalPositionTable::getRowByKey(string antennaName, AtmPhaseCorrectionMod::AtmPhaseCorrection atmPhaseCorrection, Tag calDataId, Tag calReductionId)  {
	CalPositionRow* aRow = 0;
	for (unsigned int i = 0; i < row.size(); i++) {
		aRow = row.at(i);
		
			
				if (aRow->antennaName != antennaName) continue;
			
		
			
				if (aRow->atmPhaseCorrection != atmPhaseCorrection) continue;
			
		
			
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
			
 * @param antennaName.
 	 		
 * @param atmPhaseCorrection.
 	 		
 * @param calDataId.
 	 		
 * @param calReductionId.
 	 		
 * @param startValidTime.
 	 		
 * @param endValidTime.
 	 		
 * @param antennaPosition.
 	 		
 * @param stationName.
 	 		
 * @param stationPosition.
 	 		
 * @param positionMethod.
 	 		
 * @param receiverBand.
 	 		
 * @param numAntenna.
 	 		
 * @param refAntennaNames.
 	 		
 * @param axesOffset.
 	 		
 * @param axesOffsetErr.
 	 		
 * @param axesOffsetFixed.
 	 		
 * @param positionOffset.
 	 		
 * @param positionErr.
 	 		
 * @param reducedChiSquared.
 	 		 
 */
CalPositionRow* CalPositionTable::lookup(string antennaName, AtmPhaseCorrectionMod::AtmPhaseCorrection atmPhaseCorrection, Tag calDataId, Tag calReductionId, ArrayTime startValidTime, ArrayTime endValidTime, vector<Length > antennaPosition, string stationName, vector<Length > stationPosition, PositionMethodMod::PositionMethod positionMethod, ReceiverBandMod::ReceiverBand receiverBand, int numAntenna, vector<string > refAntennaNames, Length axesOffset, Length axesOffsetErr, bool axesOffsetFixed, vector<Length > positionOffset, vector<Length > positionErr, double reducedChiSquared) {
		CalPositionRow* aRow;
		for (unsigned int i = 0; i < size(); i++) {
			aRow = row.at(i); 
			if (aRow->compareNoAutoInc(antennaName, atmPhaseCorrection, calDataId, calReductionId, startValidTime, endValidTime, antennaPosition, stationName, stationPosition, positionMethod, receiverBand, numAntenna, refAntennaNames, axesOffset, axesOffsetErr, axesOffsetFixed, positionOffset, positionErr, reducedChiSquared)) return aRow;
		}			
		return 0;	
} 
	
 	 	

	




#ifndef WITHOUT_ACS
	// Conversion Methods

	CalPositionTableIDL *CalPositionTable::toIDL() {
		CalPositionTableIDL *x = new CalPositionTableIDL ();
		unsigned int nrow = size();
		x->row.length(nrow);
		vector<CalPositionRow*> v = get();
		for (unsigned int i = 0; i < nrow; ++i) {
			x->row[i] = *(v[i]->toIDL());
		}
		return x;
	}
#endif
	
#ifndef WITHOUT_ACS
	void CalPositionTable::fromIDL(CalPositionTableIDL x) {
		unsigned int nrow = x.row.length();
		for (unsigned int i = 0; i < nrow; ++i) {
			CalPositionRow *tmp = newRow();
			tmp->setFromIDL(x.row[i]);
			// checkAndAdd(tmp);
			add(tmp);
		}
	}
#endif

	
	string CalPositionTable::toXML()  {
		string buf;

		buf.append("<?xml version=\"1.0\" encoding=\"ISO-8859-1\"?> ");
		buf.append("<CalPositionTable xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" xmlns:clposn=\"http://Alma/XASDM/CalPositionTable\" xsi:schemaLocation=\"http://Alma/XASDM/CalPositionTable http://almaobservatory.org/XML/XASDM/2/CalPositionTable.xsd\" schemaVersion=\"2\" schemaRevision=\"1.54\">\n");
	
		buf.append(entity.toXML());
		string s = container.getEntity().toXML();
		// Change the "Entity" tag to "ContainerEntity".
		buf.append("<Container" + s.substr(1,s.length() - 1)+" ");
		vector<CalPositionRow*> v = get();
		for (unsigned int i = 0; i < v.size(); ++i) {
			try {
				buf.append(v[i]->toXML());
			} catch (NoSuchRow e) {
			}
			buf.append("  ");
		}		
		buf.append("</CalPositionTable> ");
		return buf;
	}

	
	void CalPositionTable::fromXML(string xmlDoc)  {
		Parser xml(xmlDoc);
		if (!xml.isStr("<CalPositionTable")) 
			error();
		// cout << "Parsing a CalPositionTable" << endl;
		string s = xml.getElement("<Entity","/>");
		if (s.length() == 0) 
			error();
		Entity e;
		e.setFromXML(s);
		if (e.getEntityTypeName() != "CalPositionTable")
			error();
		setEntity(e);
		// Skip the container's entity; but, it has to be there.
		s = xml.getElement("<ContainerEntity","/>");
		if (s.length() == 0) 
			error();

		// Get each row in the table.
		s = xml.getElementContent("<row>","</row>");
		CalPositionRow *row;
		while (s.length() != 0) {
			// cout << "Parsing a CalPositionRow" << endl; 
			row = newRow();
			row->setFromXML(s);
			try {
				checkAndAdd(row);
			} catch (DuplicateKey e1) {
				throw ConversionException(e1.getMessage(),"CalPositionTable");
			} 
			catch (UniquenessViolationException e1) {
				throw ConversionException(e1.getMessage(),"CalPositionTable");	
			}
			catch (...) {
				// cout << "Unexpected error in CalPositionTable::checkAndAdd called from CalPositionTable::fromXML " << endl;
			}
			s = xml.getElementContent("<row>","</row>");
		}
		if (!xml.isStr("</CalPositionTable>")) 
			error();
			
		archiveAsBin = false;
		fileAsBin = false;
		
	}

	
	void CalPositionTable::error()  {
		throw ConversionException("Invalid xml document","CalPosition");
	}
	
	
	string CalPositionTable::MIMEXMLPart(const asdm::ByteOrder* byteOrder) {
		string UID = getEntity().getEntityId().toString();
		string withoutUID = UID.substr(6);
		string containerUID = getContainer().getEntity().getEntityId().toString();
		ostringstream oss;
		oss << "<?xml version='1.0'  encoding='ISO-8859-1'?>";
		oss << "\n";
		oss << "<CalPositionTable xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" xmlns:clposn=\"http://Alma/XASDM/CalPositionTable\" xsi:schemaLocation=\"http://Alma/XASDM/CalPositionTable http://almaobservatory.org/XML/XASDM/2/CalPositionTable.xsd\" schemaVersion=\"2\" schemaRevision=\"1.54\">\n";
		oss<< "<Entity entityId='"<<UID<<"' entityIdEncrypted='na' entityTypeName='CalPositionTable' schemaVersion='1' documentVersion='1'/>\n";
		oss<< "<ContainerEntity entityId='"<<containerUID<<"' entityIdEncrypted='na' entityTypeName='ASDM' schemaVersion='1' documentVersion='1'/>\n";
		oss << "<BulkStoreRef file_id='"<<withoutUID<<"' byteOrder='"<<byteOrder->toString()<<"' />\n";
		oss << "<Attributes>\n";

		oss << "<antennaName/>\n"; 
		oss << "<atmPhaseCorrection/>\n"; 
		oss << "<calDataId/>\n"; 
		oss << "<calReductionId/>\n"; 
		oss << "<startValidTime/>\n"; 
		oss << "<endValidTime/>\n"; 
		oss << "<antennaPosition/>\n"; 
		oss << "<stationName/>\n"; 
		oss << "<stationPosition/>\n"; 
		oss << "<positionMethod/>\n"; 
		oss << "<receiverBand/>\n"; 
		oss << "<numAntenna/>\n"; 
		oss << "<refAntennaNames/>\n"; 
		oss << "<axesOffset/>\n"; 
		oss << "<axesOffsetErr/>\n"; 
		oss << "<axesOffsetFixed/>\n"; 
		oss << "<positionOffset/>\n"; 
		oss << "<positionErr/>\n"; 
		oss << "<reducedChiSquared/>\n"; 

		oss << "<delayRms/>\n"; 
		oss << "<phaseRms/>\n"; 
		oss << "</Attributes>\n";		
		oss << "</CalPositionTable>\n";

		return oss.str();				
	}
	
	string CalPositionTable::toMIME(const asdm::ByteOrder* byteOrder) {
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

	
	void CalPositionTable::setFromMIME(const string & mimeMsg) {
    string xmlPartMIMEHeader = "Content-ID: <header.xml>\n\n";
    
    string binPartMIMEHeader = "--MIME_boundary\nContent-Type: binary/octet-stream\nContent-ID: <content.bin>\n\n";
    
    // Detect the XML header.
    string::size_type loc0 = mimeMsg.find(xmlPartMIMEHeader, 0);
    if ( loc0 == string::npos) {
      throw ConversionException("Failed to detect the beginning of the XML header", "CalPosition");
    }
    loc0 += xmlPartMIMEHeader.size();
    
    // Look for the string announcing the binary part.
    string::size_type loc1 = mimeMsg.find( binPartMIMEHeader, loc0 );
    
    if ( loc1 == string::npos ) {
      throw ConversionException("Failed to detect the beginning of the binary part", "CalPosition");
    }
    
    //
    // Extract the xmlHeader and analyze it to find out what is the byte order and the sequence
    // of attribute names.
    //
    string xmlHeader = mimeMsg.substr(loc0, loc1-loc0);
    xmlDoc *doc;
    doc = xmlReadMemory(xmlHeader.data(), xmlHeader.size(), "BinaryTableHeader.xml", NULL, XML_PARSE_NOBLANKS);
    if ( doc == NULL ) 
      throw ConversionException("Failed to parse the xmlHeader into a DOM structure.", "CalPosition");
    
   // This vector will be filled by the names of  all the attributes of the table
   // in the order in which they are expected to be found in the binary representation.
   //
    vector<string> attributesSeq;
      
    xmlNode* root_element = xmlDocGetRootElement(doc);
    if ( root_element == NULL || root_element->type != XML_ELEMENT_NODE )
      throw ConversionException("Failed to parse the xmlHeader into a DOM structure.", "CalPosition");
    
    const ByteOrder* byteOrder;
    if ( string("ASDMBinaryTable").compare((const char*) root_element->name) == 0) {
      // Then it's an "old fashioned" MIME file for tables.
      // Just try to deserialize it with Big_Endian for the bytes ordering.
      byteOrder = asdm::ByteOrder::Big_Endian;
      
 	 //
    // Let's consider a  default order for the sequence of attributes.
    //
     
    attributesSeq.push_back("antennaName") ; 
     
    attributesSeq.push_back("atmPhaseCorrection") ; 
     
    attributesSeq.push_back("calDataId") ; 
     
    attributesSeq.push_back("calReductionId") ; 
     
    attributesSeq.push_back("startValidTime") ; 
     
    attributesSeq.push_back("endValidTime") ; 
     
    attributesSeq.push_back("antennaPosition") ; 
     
    attributesSeq.push_back("stationName") ; 
     
    attributesSeq.push_back("stationPosition") ; 
     
    attributesSeq.push_back("positionMethod") ; 
     
    attributesSeq.push_back("receiverBand") ; 
     
    attributesSeq.push_back("numAntenna") ; 
     
    attributesSeq.push_back("refAntennaNames") ; 
     
    attributesSeq.push_back("axesOffset") ; 
     
    attributesSeq.push_back("axesOffsetErr") ; 
     
    attributesSeq.push_back("axesOffsetFixed") ; 
     
    attributesSeq.push_back("positionOffset") ; 
     
    attributesSeq.push_back("positionErr") ; 
     
    attributesSeq.push_back("reducedChiSquared") ; 
    
     
    attributesSeq.push_back("delayRms") ; 
     
    attributesSeq.push_back("phaseRms") ; 
              
     }
    else if (string("CalPositionTable").compare((const char*) root_element->name) == 0) {
      // It's a new (and correct) MIME file for tables.
      //
      // 1st )  Look for a BulkStoreRef element with an attribute byteOrder.
      //
      xmlNode* bulkStoreRef = 0;
      xmlNode* child = root_element->children;
      
      // Skip the two first children (Entity and ContainerEntity).
      bulkStoreRef = (child ==  0) ? 0 : ( (child->next) == 0 ? 0 : child->next->next );
      
      if ( bulkStoreRef == 0 || (bulkStoreRef->type != XML_ELEMENT_NODE)  || (string("BulkStoreRef").compare((const char*) bulkStoreRef->name) != 0))
      	throw ConversionException ("Could not find the element '/CalPositionTable/BulkStoreRef'. Invalid XML header '"+ xmlHeader + "'.", "CalPosition");
      	
      // We found BulkStoreRef, now look for its attribute byteOrder.
      _xmlAttr* byteOrderAttr = 0;
      for (struct _xmlAttr* attr = bulkStoreRef->properties; attr; attr = attr->next) 
	  if (string("byteOrder").compare((const char*) attr->name) == 0) {
	   byteOrderAttr = attr;
	   break;
	 }
      
      if (byteOrderAttr == 0) 
	     throw ConversionException("Could not find the element '/CalPositionTable/BulkStoreRef/@byteOrder'. Invalid XML header '" + xmlHeader +"'.", "CalPosition");
      
      string byteOrderValue = string((const char*) byteOrderAttr->children->content);
      if (!(byteOrder = asdm::ByteOrder::fromString(byteOrderValue)))
		throw ConversionException("No valid value retrieved for the element '/CalPositionTable/BulkStoreRef/@byteOrder'. Invalid XML header '" + xmlHeader + "'.", "CalPosition");
		
	 //
	 // 2nd) Look for the Attributes element and grab the names of the elements it contains.
	 //
	 xmlNode* attributes = bulkStoreRef->next;
     if ( attributes == 0 || (attributes->type != XML_ELEMENT_NODE)  || (string("Attributes").compare((const char*) attributes->name) != 0))	 
       	throw ConversionException ("Could not find the element '/CalPositionTable/Attributes'. Invalid XML header '"+ xmlHeader + "'.", "CalPosition");
 
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
	CalPositionRow* aRow = CalPositionRow::fromBin(eiss, *this, attributesSeq);
	checkAndAdd(aRow);
      }
    }
    catch (DuplicateKey e) {
      throw ConversionException("Error while writing binary data , the message was "
				+ e.getMessage(), "CalPosition");
    }
    catch (TagFormatException e) {
      throw ConversionException("Error while reading binary data , the message was "
				+ e.getMessage(), "CalPosition");
    }
    archiveAsBin = true;
    fileAsBin = true;
	}

	
	void CalPositionTable::toFile(string directory) {
		if (!directoryExists(directory.c_str()) &&
			!createPath(directory.c_str())) {
			throw ConversionException("Could not create directory " , directory);
		}

		string fileName = directory + "/CalPosition.xml";
		ofstream tableout(fileName.c_str(),ios::out|ios::trunc);
		if (tableout.rdstate() == ostream::failbit)
			throw ConversionException("Could not open file " + fileName + " to write ", "CalPosition");
		if (fileAsBin) 
			tableout << MIMEXMLPart();
		else
			tableout << toXML() << endl;
		tableout.close();
		if (tableout.rdstate() == ostream::failbit)
			throw ConversionException("Could not close file " + fileName, "CalPosition");

		if (fileAsBin) {
			// write the bin serialized
			string fileName = directory + "/CalPosition.bin";
			ofstream tableout(fileName.c_str(),ios::out|ios::trunc);
			if (tableout.rdstate() == ostream::failbit)
				throw ConversionException("Could not open file " + fileName + " to write ", "CalPosition");
			tableout << toMIME() << endl;
			tableout.close();
			if (tableout.rdstate() == ostream::failbit)
				throw ConversionException("Could not close file " + fileName, "CalPosition");
		}
	}

	
	void CalPositionTable::setFromFile(const string& directory) {		
    if (boost::filesystem::exists(boost::filesystem::path(uniqSlashes(directory + "/CalPosition.xml"))))
      setFromXMLFile(directory);
    else if (boost::filesystem::exists(boost::filesystem::path(uniqSlashes(directory + "/CalPosition.bin"))))
      setFromMIMEFile(directory);
    else
      throw ConversionException("No file found for the CalPosition table", "CalPosition");
	}			

	
  void CalPositionTable::setFromMIMEFile(const string& directory) {
    string tablePath ;
    
    tablePath = directory + "/CalPosition.bin";
    ifstream tablefile(tablePath.c_str(), ios::in|ios::binary);
    if (!tablefile.is_open()) { 
      throw ConversionException("Could not open file " + tablePath, "CalPosition");
    }
    // Read in a stringstream.
    stringstream ss; ss << tablefile.rdbuf();
    
    if (tablefile.rdstate() == istream::failbit || tablefile.rdstate() == istream::badbit) {
      throw ConversionException("Error reading file " + tablePath,"CalPosition");
    }
    
    // And close.
    tablefile.close();
    if (tablefile.rdstate() == istream::failbit)
      throw ConversionException("Could not close file " + tablePath,"CalPosition");
    
    setFromMIME(ss.str());
  }	

	
void CalPositionTable::setFromXMLFile(const string& directory) {
    string tablePath ;
    
    tablePath = directory + "/CalPosition.xml";
    ifstream tablefile(tablePath.c_str(), ios::in|ios::binary);
    if (!tablefile.is_open()) { 
      throw ConversionException("Could not open file " + tablePath, "CalPosition");
    }
      // Read in a stringstream.
    stringstream ss;
    ss << tablefile.rdbuf();
    
    if  (tablefile.rdstate() == istream::failbit || tablefile.rdstate() == istream::badbit) {
      throw ConversionException("Error reading file '" + tablePath + "'", "CalPosition");
    }
    
    // And close
    tablefile.close();
    if (tablefile.rdstate() == istream::failbit)
      throw ConversionException("Could not close file '" + tablePath + "'", "CalPosition");

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
 
