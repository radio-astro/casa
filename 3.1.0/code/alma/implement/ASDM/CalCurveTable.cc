
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
 * File CalCurveTable.cpp
 */
#include <ConversionException.h>
#include <DuplicateKey.h>
#include <OutOfBoundsException.h>

using asdm::ConversionException;
using asdm::DuplicateKey;
using asdm::OutOfBoundsException;

#include <ASDM.h>
#include <CalCurveTable.h>
#include <CalCurveRow.h>
#include <Parser.h>

using asdm::ASDM;
using asdm::CalCurveTable;
using asdm::CalCurveRow;
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

	string CalCurveTable::tableName = "CalCurve";
	const vector<string> CalCurveTable::attributesNames = initAttributesNames();
		

	/**
	 * The list of field names that make up key key.
	 * (Initialization is in the constructor.)
	 */
	vector<string> CalCurveTable::key;

	/**
	 * Return the list of field names that make up key key
	 * as an array of strings.
	 */	
	vector<string> CalCurveTable::getKeyName() {
		return key;
	}


	CalCurveTable::CalCurveTable(ASDM &c) : container(c) {

	
		key.push_back("atmPhaseCorrection");
	
		key.push_back("typeCurve");
	
		key.push_back("receiverBand");
	
		key.push_back("calDataId");
	
		key.push_back("calReductionId");
	


		// Define a default entity.
		entity.setEntityId(EntityId("uid://X0/X0/X0"));
		entity.setEntityIdEncrypted("na");
		entity.setEntityTypeName("CalCurveTable");
		entity.setEntityVersion("1");
		entity.setInstanceVersion("1");
		
		// Archive XML
		archiveAsBin = false;
		
		// File XML
		fileAsBin = false;
	}
	
/**
 * A destructor for CalCurveTable.
 */
	CalCurveTable::~CalCurveTable() {
		for (unsigned int i = 0; i < privateRows.size(); i++) 
			delete(privateRows.at(i));
	}

	/**
	 * Container to which this table belongs.
	 */
	ASDM &CalCurveTable::getContainer() const {
		return container;
	}

	/**
	 * Return the number of rows in the table.
	 */
	unsigned int CalCurveTable::size() {
		return privateRows.size();
	}
	
	/**
	 * Return the name of this table.
	 */
	string CalCurveTable::getName() const {
		return tableName;
	}
	
	/**
	 * Build the vector of attributes names.
	 */
	vector<string> CalCurveTable::initAttributesNames() {
		vector<string> attributesNames;

		attributesNames.push_back("atmPhaseCorrection");

		attributesNames.push_back("typeCurve");

		attributesNames.push_back("receiverBand");

		attributesNames.push_back("calDataId");

		attributesNames.push_back("calReductionId");


		attributesNames.push_back("startValidTime");

		attributesNames.push_back("endValidTime");

		attributesNames.push_back("frequencyRange");

		attributesNames.push_back("numAntenna");

		attributesNames.push_back("numPoly");

		attributesNames.push_back("numReceptor");

		attributesNames.push_back("antennaNames");

		attributesNames.push_back("refAntennaName");

		attributesNames.push_back("polarizationTypes");

		attributesNames.push_back("curve");

		attributesNames.push_back("reducedChiSquared");


		attributesNames.push_back("numBaseline");

		attributesNames.push_back("rms");

		return attributesNames;
	}
	
	/**
	 * Return the names of the attributes.
	 */
	const vector<string>& CalCurveTable::getAttributesNames() { return attributesNames; }

	/**
	 * Return this table's Entity.
	 */
	Entity CalCurveTable::getEntity() const {
		return entity;
	}

	/**
	 * Set this table's Entity.
	 */
	void CalCurveTable::setEntity(Entity e) {
		this->entity = e; 
	}
	
	//
	// ====> Row creation.
	//
	
	/**
	 * Create a new row.
	 */
	CalCurveRow *CalCurveTable::newRow() {
		return new CalCurveRow (*this);
	}
	

	/**
	 * Create a new row initialized to the specified values.
	 * @return a pointer on the created and initialized row.
	
 	 * @param atmPhaseCorrection 
	
 	 * @param typeCurve 
	
 	 * @param receiverBand 
	
 	 * @param calDataId 
	
 	 * @param calReductionId 
	
 	 * @param startValidTime 
	
 	 * @param endValidTime 
	
 	 * @param frequencyRange 
	
 	 * @param numAntenna 
	
 	 * @param numPoly 
	
 	 * @param numReceptor 
	
 	 * @param antennaNames 
	
 	 * @param refAntennaName 
	
 	 * @param polarizationTypes 
	
 	 * @param curve 
	
 	 * @param reducedChiSquared 
	
     */
	CalCurveRow* CalCurveTable::newRow(AtmPhaseCorrectionMod::AtmPhaseCorrection atmPhaseCorrection, CalCurveTypeMod::CalCurveType typeCurve, ReceiverBandMod::ReceiverBand receiverBand, Tag calDataId, Tag calReductionId, ArrayTime startValidTime, ArrayTime endValidTime, vector<Frequency > frequencyRange, int numAntenna, int numPoly, int numReceptor, vector<string > antennaNames, string refAntennaName, vector<PolarizationTypeMod::PolarizationType > polarizationTypes, vector<vector<vector<float > > > curve, vector<double > reducedChiSquared){
		CalCurveRow *row = new CalCurveRow(*this);
			
		row->setAtmPhaseCorrection(atmPhaseCorrection);
			
		row->setTypeCurve(typeCurve);
			
		row->setReceiverBand(receiverBand);
			
		row->setCalDataId(calDataId);
			
		row->setCalReductionId(calReductionId);
			
		row->setStartValidTime(startValidTime);
			
		row->setEndValidTime(endValidTime);
			
		row->setFrequencyRange(frequencyRange);
			
		row->setNumAntenna(numAntenna);
			
		row->setNumPoly(numPoly);
			
		row->setNumReceptor(numReceptor);
			
		row->setAntennaNames(antennaNames);
			
		row->setRefAntennaName(refAntennaName);
			
		row->setPolarizationTypes(polarizationTypes);
			
		row->setCurve(curve);
			
		row->setReducedChiSquared(reducedChiSquared);
	
		return row;		
	}	
	


CalCurveRow* CalCurveTable::newRow(CalCurveRow* row) {
	return new CalCurveRow(*this, *row);
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
	CalCurveRow* CalCurveTable::add(CalCurveRow* x) {
		
		if (getRowByKey(
						x->getAtmPhaseCorrection()
						,
						x->getTypeCurve()
						,
						x->getReceiverBand()
						,
						x->getCalDataId()
						,
						x->getCalReductionId()
						))
			//throw DuplicateKey(x.getAtmPhaseCorrection() + "|" + x.getTypeCurve() + "|" + x.getReceiverBand() + "|" + x.getCalDataId() + "|" + x.getCalReductionId(),"CalCurve");
			throw DuplicateKey("Duplicate key exception in ","CalCurveTable");
		
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
	CalCurveRow*  CalCurveTable::checkAndAdd(CalCurveRow* x)  {
		
		
		if (getRowByKey(
	
			x->getAtmPhaseCorrection()
	,
			x->getTypeCurve()
	,
			x->getReceiverBand()
	,
			x->getCalDataId()
	,
			x->getCalReductionId()
			
		)) throw DuplicateKey("Duplicate key exception in ", "CalCurveTable");
		
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
	 * @return Alls rows as an array of CalCurveRow
	 */
	vector<CalCurveRow *> CalCurveTable::get() {
		return privateRows;
		// return row;
	}

	
/*
 ** Returns a CalCurveRow* given a key.
 ** @return a pointer to the row having the key whose values are passed as parameters, or 0 if
 ** no row exists for that key.
 **
 */
 	CalCurveRow* CalCurveTable::getRowByKey(AtmPhaseCorrectionMod::AtmPhaseCorrection atmPhaseCorrection, CalCurveTypeMod::CalCurveType typeCurve, ReceiverBandMod::ReceiverBand receiverBand, Tag calDataId, Tag calReductionId)  {
	CalCurveRow* aRow = 0;
	for (unsigned int i = 0; i < row.size(); i++) {
		aRow = row.at(i);
		
			
				if (aRow->atmPhaseCorrection != atmPhaseCorrection) continue;
			
		
			
				if (aRow->typeCurve != typeCurve) continue;
			
		
			
				if (aRow->receiverBand != receiverBand) continue;
			
		
			
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
			
 * @param atmPhaseCorrection.
 	 		
 * @param typeCurve.
 	 		
 * @param receiverBand.
 	 		
 * @param calDataId.
 	 		
 * @param calReductionId.
 	 		
 * @param startValidTime.
 	 		
 * @param endValidTime.
 	 		
 * @param frequencyRange.
 	 		
 * @param numAntenna.
 	 		
 * @param numPoly.
 	 		
 * @param numReceptor.
 	 		
 * @param antennaNames.
 	 		
 * @param refAntennaName.
 	 		
 * @param polarizationTypes.
 	 		
 * @param curve.
 	 		
 * @param reducedChiSquared.
 	 		 
 */
CalCurveRow* CalCurveTable::lookup(AtmPhaseCorrectionMod::AtmPhaseCorrection atmPhaseCorrection, CalCurveTypeMod::CalCurveType typeCurve, ReceiverBandMod::ReceiverBand receiverBand, Tag calDataId, Tag calReductionId, ArrayTime startValidTime, ArrayTime endValidTime, vector<Frequency > frequencyRange, int numAntenna, int numPoly, int numReceptor, vector<string > antennaNames, string refAntennaName, vector<PolarizationTypeMod::PolarizationType > polarizationTypes, vector<vector<vector<float > > > curve, vector<double > reducedChiSquared) {
		CalCurveRow* aRow;
		for (unsigned int i = 0; i < size(); i++) {
			aRow = row.at(i); 
			if (aRow->compareNoAutoInc(atmPhaseCorrection, typeCurve, receiverBand, calDataId, calReductionId, startValidTime, endValidTime, frequencyRange, numAntenna, numPoly, numReceptor, antennaNames, refAntennaName, polarizationTypes, curve, reducedChiSquared)) return aRow;
		}			
		return 0;	
} 
	
 	 	

	




#ifndef WITHOUT_ACS
	// Conversion Methods

	CalCurveTableIDL *CalCurveTable::toIDL() {
		CalCurveTableIDL *x = new CalCurveTableIDL ();
		unsigned int nrow = size();
		x->row.length(nrow);
		vector<CalCurveRow*> v = get();
		for (unsigned int i = 0; i < nrow; ++i) {
			x->row[i] = *(v[i]->toIDL());
		}
		return x;
	}
#endif
	
#ifndef WITHOUT_ACS
	void CalCurveTable::fromIDL(CalCurveTableIDL x) {
		unsigned int nrow = x.row.length();
		for (unsigned int i = 0; i < nrow; ++i) {
			CalCurveRow *tmp = newRow();
			tmp->setFromIDL(x.row[i]);
			// checkAndAdd(tmp);
			add(tmp);
		}
	}
#endif

	
	string CalCurveTable::toXML()  {
		string buf;

		buf.append("<?xml version=\"1.0\" encoding=\"ISO-8859-1\"?> ");
		buf.append("<CalCurveTable xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" xmlns:clcrv=\"http://Alma/XASDM/CalCurveTable\" xsi:schemaLocation=\"http://Alma/XASDM/CalCurveTable http://almaobservatory.org/XML/XASDM/2/CalCurveTable.xsd\" schemaVersion=\"2\" schemaRevision=\"1.54\">\n");
	
		buf.append(entity.toXML());
		string s = container.getEntity().toXML();
		// Change the "Entity" tag to "ContainerEntity".
		buf.append("<Container" + s.substr(1,s.length() - 1)+" ");
		vector<CalCurveRow*> v = get();
		for (unsigned int i = 0; i < v.size(); ++i) {
			try {
				buf.append(v[i]->toXML());
			} catch (NoSuchRow e) {
			}
			buf.append("  ");
		}		
		buf.append("</CalCurveTable> ");
		return buf;
	}

	
	void CalCurveTable::fromXML(string xmlDoc)  {
		Parser xml(xmlDoc);
		if (!xml.isStr("<CalCurveTable")) 
			error();
		// cout << "Parsing a CalCurveTable" << endl;
		string s = xml.getElement("<Entity","/>");
		if (s.length() == 0) 
			error();
		Entity e;
		e.setFromXML(s);
		if (e.getEntityTypeName() != "CalCurveTable")
			error();
		setEntity(e);
		// Skip the container's entity; but, it has to be there.
		s = xml.getElement("<ContainerEntity","/>");
		if (s.length() == 0) 
			error();

		// Get each row in the table.
		s = xml.getElementContent("<row>","</row>");
		CalCurveRow *row;
		while (s.length() != 0) {
			// cout << "Parsing a CalCurveRow" << endl; 
			row = newRow();
			row->setFromXML(s);
			try {
				checkAndAdd(row);
			} catch (DuplicateKey e1) {
				throw ConversionException(e1.getMessage(),"CalCurveTable");
			} 
			catch (UniquenessViolationException e1) {
				throw ConversionException(e1.getMessage(),"CalCurveTable");	
			}
			catch (...) {
				// cout << "Unexpected error in CalCurveTable::checkAndAdd called from CalCurveTable::fromXML " << endl;
			}
			s = xml.getElementContent("<row>","</row>");
		}
		if (!xml.isStr("</CalCurveTable>")) 
			error();
			
		archiveAsBin = false;
		fileAsBin = false;
		
	}

	
	void CalCurveTable::error()  {
		throw ConversionException("Invalid xml document","CalCurve");
	}
	
	
	string CalCurveTable::MIMEXMLPart(const asdm::ByteOrder* byteOrder) {
		string UID = getEntity().getEntityId().toString();
		string withoutUID = UID.substr(6);
		string containerUID = getContainer().getEntity().getEntityId().toString();
		ostringstream oss;
		oss << "<?xml version='1.0'  encoding='ISO-8859-1'?>";
		oss << "\n";
		oss << "<CalCurveTable xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" xmlns:clcrv=\"http://Alma/XASDM/CalCurveTable\" xsi:schemaLocation=\"http://Alma/XASDM/CalCurveTable http://almaobservatory.org/XML/XASDM/2/CalCurveTable.xsd\" schemaVersion=\"2\" schemaRevision=\"1.54\">\n";
		oss<< "<Entity entityId='"<<UID<<"' entityIdEncrypted='na' entityTypeName='CalCurveTable' schemaVersion='1' documentVersion='1'/>\n";
		oss<< "<ContainerEntity entityId='"<<containerUID<<"' entityIdEncrypted='na' entityTypeName='ASDM' schemaVersion='1' documentVersion='1'/>\n";
		oss << "<BulkStoreRef file_id='"<<withoutUID<<"' byteOrder='"<<byteOrder->toString()<<"' />\n";
		oss << "<Attributes>\n";

		oss << "<atmPhaseCorrection/>\n"; 
		oss << "<typeCurve/>\n"; 
		oss << "<receiverBand/>\n"; 
		oss << "<calDataId/>\n"; 
		oss << "<calReductionId/>\n"; 
		oss << "<startValidTime/>\n"; 
		oss << "<endValidTime/>\n"; 
		oss << "<frequencyRange/>\n"; 
		oss << "<numAntenna/>\n"; 
		oss << "<numPoly/>\n"; 
		oss << "<numReceptor/>\n"; 
		oss << "<antennaNames/>\n"; 
		oss << "<refAntennaName/>\n"; 
		oss << "<polarizationTypes/>\n"; 
		oss << "<curve/>\n"; 
		oss << "<reducedChiSquared/>\n"; 

		oss << "<numBaseline/>\n"; 
		oss << "<rms/>\n"; 
		oss << "</Attributes>\n";		
		oss << "</CalCurveTable>\n";

		return oss.str();				
	}
	
	string CalCurveTable::toMIME(const asdm::ByteOrder* byteOrder) {
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

	
	void CalCurveTable::setFromMIME(const string & mimeMsg) {
    string xmlPartMIMEHeader = "Content-ID: <header.xml>\n\n";
    
    string binPartMIMEHeader = "--MIME_boundary\nContent-Type: binary/octet-stream\nContent-ID: <content.bin>\n\n";
    
    // Detect the XML header.
    string::size_type loc0 = mimeMsg.find(xmlPartMIMEHeader, 0);
    if ( loc0 == string::npos) {
      throw ConversionException("Failed to detect the beginning of the XML header", "CalCurve");
    }
    loc0 += xmlPartMIMEHeader.size();
    
    // Look for the string announcing the binary part.
    string::size_type loc1 = mimeMsg.find( binPartMIMEHeader, loc0 );
    
    if ( loc1 == string::npos ) {
      throw ConversionException("Failed to detect the beginning of the binary part", "CalCurve");
    }
    
    //
    // Extract the xmlHeader and analyze it to find out what is the byte order and the sequence
    // of attribute names.
    //
    string xmlHeader = mimeMsg.substr(loc0, loc1-loc0);
    xmlDoc *doc;
    doc = xmlReadMemory(xmlHeader.data(), xmlHeader.size(), "BinaryTableHeader.xml", NULL, XML_PARSE_NOBLANKS);
    if ( doc == NULL ) 
      throw ConversionException("Failed to parse the xmlHeader into a DOM structure.", "CalCurve");
    
   // This vector will be filled by the names of  all the attributes of the table
   // in the order in which they are expected to be found in the binary representation.
   //
    vector<string> attributesSeq;
      
    xmlNode* root_element = xmlDocGetRootElement(doc);
    if ( root_element == NULL || root_element->type != XML_ELEMENT_NODE )
      throw ConversionException("Failed to parse the xmlHeader into a DOM structure.", "CalCurve");
    
    const ByteOrder* byteOrder;
    if ( string("ASDMBinaryTable").compare((const char*) root_element->name) == 0) {
      // Then it's an "old fashioned" MIME file for tables.
      // Just try to deserialize it with Big_Endian for the bytes ordering.
      byteOrder = asdm::ByteOrder::Big_Endian;
      
 	 //
    // Let's consider a  default order for the sequence of attributes.
    //
     
    attributesSeq.push_back("atmPhaseCorrection") ; 
     
    attributesSeq.push_back("typeCurve") ; 
     
    attributesSeq.push_back("receiverBand") ; 
     
    attributesSeq.push_back("calDataId") ; 
     
    attributesSeq.push_back("calReductionId") ; 
     
    attributesSeq.push_back("startValidTime") ; 
     
    attributesSeq.push_back("endValidTime") ; 
     
    attributesSeq.push_back("frequencyRange") ; 
     
    attributesSeq.push_back("numAntenna") ; 
     
    attributesSeq.push_back("numPoly") ; 
     
    attributesSeq.push_back("numReceptor") ; 
     
    attributesSeq.push_back("antennaNames") ; 
     
    attributesSeq.push_back("refAntennaName") ; 
     
    attributesSeq.push_back("polarizationTypes") ; 
     
    attributesSeq.push_back("curve") ; 
     
    attributesSeq.push_back("reducedChiSquared") ; 
    
     
    attributesSeq.push_back("numBaseline") ; 
     
    attributesSeq.push_back("rms") ; 
              
     }
    else if (string("CalCurveTable").compare((const char*) root_element->name) == 0) {
      // It's a new (and correct) MIME file for tables.
      //
      // 1st )  Look for a BulkStoreRef element with an attribute byteOrder.
      //
      xmlNode* bulkStoreRef = 0;
      xmlNode* child = root_element->children;
      
      // Skip the two first children (Entity and ContainerEntity).
      bulkStoreRef = (child ==  0) ? 0 : ( (child->next) == 0 ? 0 : child->next->next );
      
      if ( bulkStoreRef == 0 || (bulkStoreRef->type != XML_ELEMENT_NODE)  || (string("BulkStoreRef").compare((const char*) bulkStoreRef->name) != 0))
      	throw ConversionException ("Could not find the element '/CalCurveTable/BulkStoreRef'. Invalid XML header '"+ xmlHeader + "'.", "CalCurve");
      	
      // We found BulkStoreRef, now look for its attribute byteOrder.
      _xmlAttr* byteOrderAttr = 0;
      for (struct _xmlAttr* attr = bulkStoreRef->properties; attr; attr = attr->next) 
	  if (string("byteOrder").compare((const char*) attr->name) == 0) {
	   byteOrderAttr = attr;
	   break;
	 }
      
      if (byteOrderAttr == 0) 
	     throw ConversionException("Could not find the element '/CalCurveTable/BulkStoreRef/@byteOrder'. Invalid XML header '" + xmlHeader +"'.", "CalCurve");
      
      string byteOrderValue = string((const char*) byteOrderAttr->children->content);
      if (!(byteOrder = asdm::ByteOrder::fromString(byteOrderValue)))
		throw ConversionException("No valid value retrieved for the element '/CalCurveTable/BulkStoreRef/@byteOrder'. Invalid XML header '" + xmlHeader + "'.", "CalCurve");
		
	 //
	 // 2nd) Look for the Attributes element and grab the names of the elements it contains.
	 //
	 xmlNode* attributes = bulkStoreRef->next;
     if ( attributes == 0 || (attributes->type != XML_ELEMENT_NODE)  || (string("Attributes").compare((const char*) attributes->name) != 0))	 
       	throw ConversionException ("Could not find the element '/CalCurveTable/Attributes'. Invalid XML header '"+ xmlHeader + "'.", "CalCurve");
 
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
	CalCurveRow* aRow = CalCurveRow::fromBin(eiss, *this, attributesSeq);
	checkAndAdd(aRow);
      }
    }
    catch (DuplicateKey e) {
      throw ConversionException("Error while writing binary data , the message was "
				+ e.getMessage(), "CalCurve");
    }
    catch (TagFormatException e) {
      throw ConversionException("Error while reading binary data , the message was "
				+ e.getMessage(), "CalCurve");
    }
    archiveAsBin = true;
    fileAsBin = true;
	}

	
	void CalCurveTable::toFile(string directory) {
		if (!directoryExists(directory.c_str()) &&
			!createPath(directory.c_str())) {
			throw ConversionException("Could not create directory " , directory);
		}

		string fileName = directory + "/CalCurve.xml";
		ofstream tableout(fileName.c_str(),ios::out|ios::trunc);
		if (tableout.rdstate() == ostream::failbit)
			throw ConversionException("Could not open file " + fileName + " to write ", "CalCurve");
		if (fileAsBin) 
			tableout << MIMEXMLPart();
		else
			tableout << toXML() << endl;
		tableout.close();
		if (tableout.rdstate() == ostream::failbit)
			throw ConversionException("Could not close file " + fileName, "CalCurve");

		if (fileAsBin) {
			// write the bin serialized
			string fileName = directory + "/CalCurve.bin";
			ofstream tableout(fileName.c_str(),ios::out|ios::trunc);
			if (tableout.rdstate() == ostream::failbit)
				throw ConversionException("Could not open file " + fileName + " to write ", "CalCurve");
			tableout << toMIME() << endl;
			tableout.close();
			if (tableout.rdstate() == ostream::failbit)
				throw ConversionException("Could not close file " + fileName, "CalCurve");
		}
	}

	
	void CalCurveTable::setFromFile(const string& directory) {		
    if (boost::filesystem::exists(boost::filesystem::path(uniqSlashes(directory + "/CalCurve.xml"))))
      setFromXMLFile(directory);
    else if (boost::filesystem::exists(boost::filesystem::path(uniqSlashes(directory + "/CalCurve.bin"))))
      setFromMIMEFile(directory);
    else
      throw ConversionException("No file found for the CalCurve table", "CalCurve");
	}			

	
  void CalCurveTable::setFromMIMEFile(const string& directory) {
    string tablePath ;
    
    tablePath = directory + "/CalCurve.bin";
    ifstream tablefile(tablePath.c_str(), ios::in|ios::binary);
    if (!tablefile.is_open()) { 
      throw ConversionException("Could not open file " + tablePath, "CalCurve");
    }
    // Read in a stringstream.
    stringstream ss; ss << tablefile.rdbuf();
    
    if (tablefile.rdstate() == istream::failbit || tablefile.rdstate() == istream::badbit) {
      throw ConversionException("Error reading file " + tablePath,"CalCurve");
    }
    
    // And close.
    tablefile.close();
    if (tablefile.rdstate() == istream::failbit)
      throw ConversionException("Could not close file " + tablePath,"CalCurve");
    
    setFromMIME(ss.str());
  }	

	
void CalCurveTable::setFromXMLFile(const string& directory) {
    string tablePath ;
    
    tablePath = directory + "/CalCurve.xml";
    ifstream tablefile(tablePath.c_str(), ios::in|ios::binary);
    if (!tablefile.is_open()) { 
      throw ConversionException("Could not open file " + tablePath, "CalCurve");
    }
      // Read in a stringstream.
    stringstream ss;
    ss << tablefile.rdbuf();
    
    if  (tablefile.rdstate() == istream::failbit || tablefile.rdstate() == istream::badbit) {
      throw ConversionException("Error reading file '" + tablePath + "'", "CalCurve");
    }
    
    // And close
    tablefile.close();
    if (tablefile.rdstate() == istream::failbit)
      throw ConversionException("Could not close file '" + tablePath + "'", "CalCurve");

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
 
