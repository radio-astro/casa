
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
 * File GainTrackingTable.cpp
 */
#include <ConversionException.h>
#include <DuplicateKey.h>
#include <OutOfBoundsException.h>

using asdm::ConversionException;
using asdm::DuplicateKey;
using asdm::OutOfBoundsException;

#include <ASDM.h>
#include <GainTrackingTable.h>
#include <GainTrackingRow.h>
#include <Parser.h>

using asdm::ASDM;
using asdm::GainTrackingTable;
using asdm::GainTrackingRow;
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

	string GainTrackingTable::tableName = "GainTracking";
	const vector<string> GainTrackingTable::attributesNames = initAttributesNames();
		

	/**
	 * The list of field names that make up key key.
	 * (Initialization is in the constructor.)
	 */
	vector<string> GainTrackingTable::key;

	/**
	 * Return the list of field names that make up key key
	 * as an array of strings.
	 */	
	vector<string> GainTrackingTable::getKeyName() {
		return key;
	}


	GainTrackingTable::GainTrackingTable(ASDM &c) : container(c) {

	
		key.push_back("antennaId");
	
		key.push_back("spectralWindowId");
	
		key.push_back("timeInterval");
	
		key.push_back("feedId");
	


		// Define a default entity.
		entity.setEntityId(EntityId("uid://X0/X0/X0"));
		entity.setEntityIdEncrypted("na");
		entity.setEntityTypeName("GainTrackingTable");
		entity.setEntityVersion("1");
		entity.setInstanceVersion("1");
		
		// Archive XML
		archiveAsBin = false;
		
		// File XML
		fileAsBin = false;
	}
	
/**
 * A destructor for GainTrackingTable.
 */
	GainTrackingTable::~GainTrackingTable() {
		for (unsigned int i = 0; i < privateRows.size(); i++) 
			delete(privateRows.at(i));
	}

	/**
	 * Container to which this table belongs.
	 */
	ASDM &GainTrackingTable::getContainer() const {
		return container;
	}

	/**
	 * Return the number of rows in the table.
	 */
	unsigned int GainTrackingTable::size() {
		return privateRows.size();
	}
	
	/**
	 * Return the name of this table.
	 */
	string GainTrackingTable::getName() const {
		return tableName;
	}
	
	/**
	 * Build the vector of attributes names.
	 */
	vector<string> GainTrackingTable::initAttributesNames() {
		vector<string> attributesNames;

		attributesNames.push_back("antennaId");

		attributesNames.push_back("spectralWindowId");

		attributesNames.push_back("timeInterval");

		attributesNames.push_back("feedId");


		attributesNames.push_back("attenuator");

		attributesNames.push_back("numLO");

		attributesNames.push_back("numReceptor");

		attributesNames.push_back("cableDelay");

		attributesNames.push_back("crossPolarizationDelay");

		attributesNames.push_back("loPropagationDelay");

		attributesNames.push_back("polarizationTypes");

		attributesNames.push_back("receiverDelay");


		attributesNames.push_back("delayOffset");

		attributesNames.push_back("freqOffset");

		attributesNames.push_back("phaseOffset");

		attributesNames.push_back("samplingLevel");

		attributesNames.push_back("numAttFreq");

		attributesNames.push_back("attFreq");

		attributesNames.push_back("attSpectrum");

		return attributesNames;
	}
	
	/**
	 * Return the names of the attributes.
	 */
	const vector<string>& GainTrackingTable::getAttributesNames() { return attributesNames; }

	/**
	 * Return this table's Entity.
	 */
	Entity GainTrackingTable::getEntity() const {
		return entity;
	}

	/**
	 * Set this table's Entity.
	 */
	void GainTrackingTable::setEntity(Entity e) {
		this->entity = e; 
	}
	
	//
	// ====> Row creation.
	//
	
	/**
	 * Create a new row.
	 */
	GainTrackingRow *GainTrackingTable::newRow() {
		return new GainTrackingRow (*this);
	}
	

	/**
	 * Create a new row initialized to the specified values.
	 * @return a pointer on the created and initialized row.
	
 	 * @param antennaId 
	
 	 * @param spectralWindowId 
	
 	 * @param timeInterval 
	
 	 * @param feedId 
	
 	 * @param attenuator 
	
 	 * @param numLO 
	
 	 * @param numReceptor 
	
 	 * @param cableDelay 
	
 	 * @param crossPolarizationDelay 
	
 	 * @param loPropagationDelay 
	
 	 * @param polarizationTypes 
	
 	 * @param receiverDelay 
	
     */
	GainTrackingRow* GainTrackingTable::newRow(Tag antennaId, Tag spectralWindowId, ArrayTimeInterval timeInterval, int feedId, float attenuator, int numLO, int numReceptor, vector<double > cableDelay, double crossPolarizationDelay, vector<double > loPropagationDelay, vector<PolarizationTypeMod::PolarizationType > polarizationTypes, vector<double > receiverDelay){
		GainTrackingRow *row = new GainTrackingRow(*this);
			
		row->setAntennaId(antennaId);
			
		row->setSpectralWindowId(spectralWindowId);
			
		row->setTimeInterval(timeInterval);
			
		row->setFeedId(feedId);
			
		row->setAttenuator(attenuator);
			
		row->setNumLO(numLO);
			
		row->setNumReceptor(numReceptor);
			
		row->setCableDelay(cableDelay);
			
		row->setCrossPolarizationDelay(crossPolarizationDelay);
			
		row->setLoPropagationDelay(loPropagationDelay);
			
		row->setPolarizationTypes(polarizationTypes);
			
		row->setReceiverDelay(receiverDelay);
	
		return row;		
	}	
	


GainTrackingRow* GainTrackingTable::newRow(GainTrackingRow* row) {
	return new GainTrackingRow(*this, *row);
}

	//
	// Append a row to its table.
	//

	
	
		
		
	/** 
	 * Returns a string built by concatenating the ascii representation of the
	 * parameters values suffixed with a "_" character.
	 */
	 string GainTrackingTable::Key(Tag antennaId, Tag spectralWindowId, int feedId) {
	 	ostringstream ostrstr;
	 		ostrstr  
			
				<<  antennaId.toString()  << "_"
			
				<<  spectralWindowId.toString()  << "_"
			
				<<   feedId  << "_"
			
			;
		return ostrstr.str();	 	
	 }
	 
			
			
	GainTrackingRow* GainTrackingTable::add(GainTrackingRow* x) {
		ArrayTime startTime = x->getTimeInterval().getStart();

		/*
	 	 * Is there already a context for this combination of not temporal 
	 	 * attributes ?
	 	 */
		string k = Key(
						x->getAntennaId()
					   ,
						x->getSpectralWindowId()
					   ,
						x->getFeedId()
					   );
 
		if (context.find(k) == context.end()) { 
			// There is not yet a context ...
			// Create and initialize an entry in the context map for this combination....
			TIME_ROWS v;
			context[k] = v;			
		}
		
		return insertByStartTime(x, context[k]);
	}
			
		
	




	// 
	// A private method to append a row to its table, used by input conversion
	// methods.
	//

	
	
		
		
			
			
			
			
	GainTrackingRow*  GainTrackingTable::checkAndAdd(GainTrackingRow* x) {
		string keystr = Key( 
						x->getAntennaId() 
					   , 
						x->getSpectralWindowId() 
					   , 
						x->getFeedId() 
					   ); 
		if (context.find(keystr) == context.end()) {
			vector<GainTrackingRow *> v;
			context[keystr] = v;
		}
		
		vector<GainTrackingRow*>& found = context.find(keystr)->second;
		return insertByStartTime(x, found);			
	}
			
					
		







	

	
	
		
	/**
	 * Get all rows.
	 * @return Alls rows as an array of GainTrackingRow
	 */
	 vector<GainTrackingRow *> GainTrackingTable::get() {
	    return privateRows;
	    
	 /*
	 	vector<GainTrackingRow *> v;
	 	map<string, TIME_ROWS>::iterator mapIter;
	 	vector<GainTrackingRow *>::iterator rowIter;
	 	
	 	for (mapIter=context.begin(); mapIter!=context.end(); mapIter++) {
	 		for (rowIter=((*mapIter).second).begin(); rowIter!=((*mapIter).second).end(); rowIter++) 
	 			v.push_back(*rowIter); 
	 	}
	 	
	 	return v;
	 */
	 }
	 
	 vector<GainTrackingRow *> *GainTrackingTable::getByContext(Tag antennaId, Tag spectralWindowId, int feedId) {
	  	string k = Key(antennaId, spectralWindowId, feedId);
 
	    if (context.find(k) == context.end()) return 0;
 	   else return &(context[k]);		
	}		
		
	


	
		
		
			
			
			
/*
 ** Returns a GainTrackingRow* given a key.
 ** @return a pointer to the row having the key whose values are passed as parameters, or 0 if
 ** no row exists for that key.
 **
 */
 				
				
	GainTrackingRow* GainTrackingTable::getRowByKey(Tag antennaId, Tag spectralWindowId, ArrayTimeInterval timeInterval, int feedId)  {
 		string keystr = Key(antennaId, spectralWindowId, feedId);
 		vector<GainTrackingRow *> row;
 		
 		if ( context.find(keystr)  == context.end()) return 0;
 		
 		row = context[keystr];
 		
 		// Is the vector empty...impossible in principle !
 		if (row.size() == 0) return 0;
 		
 		// Only one element in the vector
 		if (row.size() == 1) {
 			GainTrackingRow* r = row.at(0);
 			if ( r->getTimeInterval().contains(timeInterval.getStart()))
 				return r;
 			else
 				return 0;
 		}
 		
 		// Optimizations
 		GainTrackingRow* last = row.at(row.size()-1);
 		if (timeInterval.getStart().get() >= (last->getTimeInterval().getStart().get()+last->getTimeInterval().getDuration().get())) return 0;
 		
 		GainTrackingRow* first = row.at(0);
 		if (timeInterval.getStart().get() < first->getTimeInterval().getStart().get()) return 0;
 		
 		
 		// More than one row 
 		// Let's use a dichotomy method for the general case..	
 		int k0 = 0;
 		int k1 = row.size() - 1;
 		GainTrackingRow* r = 0;
 		while (k0!=k1) {
 		
 			// Is the start time contained in the time interval of row #k0
 			r = row.at(k0);
 			if (r->getTimeInterval().contains(timeInterval.getStart())) return r;
 			
 			// Is the start contained in the time interval of row #k1
 			r = row.at(k1);
			if (r->getTimeInterval().contains(timeInterval.getStart())) return r;
			
			// Are the rows #k0 and #k1 consecutive
			// Then we know for sure that there is no row containing the start of timeInterval
			if (k1==(k0+1)) return 0;
			
			// Proceed to the next step of dichotomy.
			r = row.at((k0+k1)/2);
			if ( timeInterval.getStart().get() <= r->getTimeInterval().getStart().get())
				k1 = (k0 + k1) / 2;
			else
				k0 = (k0 + k1) / 2;
		}
		return 0;	
	}
							
			
		
		
		
	




#ifndef WITHOUT_ACS
	// Conversion Methods

	GainTrackingTableIDL *GainTrackingTable::toIDL() {
		GainTrackingTableIDL *x = new GainTrackingTableIDL ();
		unsigned int nrow = size();
		x->row.length(nrow);
		vector<GainTrackingRow*> v = get();
		for (unsigned int i = 0; i < nrow; ++i) {
			x->row[i] = *(v[i]->toIDL());
		}
		return x;
	}
#endif
	
#ifndef WITHOUT_ACS
	void GainTrackingTable::fromIDL(GainTrackingTableIDL x) {
		unsigned int nrow = x.row.length();
		for (unsigned int i = 0; i < nrow; ++i) {
			GainTrackingRow *tmp = newRow();
			tmp->setFromIDL(x.row[i]);
			// checkAndAdd(tmp);
			add(tmp);
		}
	}
#endif

	
	string GainTrackingTable::toXML()  {
		string buf;

		buf.append("<?xml version=\"1.0\" encoding=\"ISO-8859-1\"?> ");
		buf.append("<GainTrackingTable xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" xmlns:gntrk=\"http://Alma/XASDM/GainTrackingTable\" xsi:schemaLocation=\"http://Alma/XASDM/GainTrackingTable http://almaobservatory.org/XML/XASDM/2/GainTrackingTable.xsd\" schemaVersion=\"2\" schemaRevision=\"1.54\">\n");
	
		buf.append(entity.toXML());
		string s = container.getEntity().toXML();
		// Change the "Entity" tag to "ContainerEntity".
		buf.append("<Container" + s.substr(1,s.length() - 1)+" ");
		vector<GainTrackingRow*> v = get();
		for (unsigned int i = 0; i < v.size(); ++i) {
			try {
				buf.append(v[i]->toXML());
			} catch (NoSuchRow e) {
			}
			buf.append("  ");
		}		
		buf.append("</GainTrackingTable> ");
		return buf;
	}

	
	void GainTrackingTable::fromXML(string xmlDoc)  {
		Parser xml(xmlDoc);
		if (!xml.isStr("<GainTrackingTable")) 
			error();
		// cout << "Parsing a GainTrackingTable" << endl;
		string s = xml.getElement("<Entity","/>");
		if (s.length() == 0) 
			error();
		Entity e;
		e.setFromXML(s);
		if (e.getEntityTypeName() != "GainTrackingTable")
			error();
		setEntity(e);
		// Skip the container's entity; but, it has to be there.
		s = xml.getElement("<ContainerEntity","/>");
		if (s.length() == 0) 
			error();

		// Get each row in the table.
		s = xml.getElementContent("<row>","</row>");
		GainTrackingRow *row;
		while (s.length() != 0) {
			// cout << "Parsing a GainTrackingRow" << endl; 
			row = newRow();
			row->setFromXML(s);
			try {
				checkAndAdd(row);
			} catch (DuplicateKey e1) {
				throw ConversionException(e1.getMessage(),"GainTrackingTable");
			} 
			catch (UniquenessViolationException e1) {
				throw ConversionException(e1.getMessage(),"GainTrackingTable");	
			}
			catch (...) {
				// cout << "Unexpected error in GainTrackingTable::checkAndAdd called from GainTrackingTable::fromXML " << endl;
			}
			s = xml.getElementContent("<row>","</row>");
		}
		if (!xml.isStr("</GainTrackingTable>")) 
			error();
			
		archiveAsBin = false;
		fileAsBin = false;
		
	}

	
	void GainTrackingTable::error()  {
		throw ConversionException("Invalid xml document","GainTracking");
	}
	
	
	string GainTrackingTable::MIMEXMLPart(const asdm::ByteOrder* byteOrder) {
		string UID = getEntity().getEntityId().toString();
		string withoutUID = UID.substr(6);
		string containerUID = getContainer().getEntity().getEntityId().toString();
		ostringstream oss;
		oss << "<?xml version='1.0'  encoding='ISO-8859-1'?>";
		oss << "\n";
		oss << "<GainTrackingTable xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" xmlns:gntrk=\"http://Alma/XASDM/GainTrackingTable\" xsi:schemaLocation=\"http://Alma/XASDM/GainTrackingTable http://almaobservatory.org/XML/XASDM/2/GainTrackingTable.xsd\" schemaVersion=\"2\" schemaRevision=\"1.54\">\n";
		oss<< "<Entity entityId='"<<UID<<"' entityIdEncrypted='na' entityTypeName='GainTrackingTable' schemaVersion='1' documentVersion='1'/>\n";
		oss<< "<ContainerEntity entityId='"<<containerUID<<"' entityIdEncrypted='na' entityTypeName='ASDM' schemaVersion='1' documentVersion='1'/>\n";
		oss << "<BulkStoreRef file_id='"<<withoutUID<<"' byteOrder='"<<byteOrder->toString()<<"' />\n";
		oss << "<Attributes>\n";

		oss << "<antennaId/>\n"; 
		oss << "<spectralWindowId/>\n"; 
		oss << "<timeInterval/>\n"; 
		oss << "<feedId/>\n"; 
		oss << "<attenuator/>\n"; 
		oss << "<numLO/>\n"; 
		oss << "<numReceptor/>\n"; 
		oss << "<cableDelay/>\n"; 
		oss << "<crossPolarizationDelay/>\n"; 
		oss << "<loPropagationDelay/>\n"; 
		oss << "<polarizationTypes/>\n"; 
		oss << "<receiverDelay/>\n"; 

		oss << "<delayOffset/>\n"; 
		oss << "<freqOffset/>\n"; 
		oss << "<phaseOffset/>\n"; 
		oss << "<samplingLevel/>\n"; 
		oss << "<numAttFreq/>\n"; 
		oss << "<attFreq/>\n"; 
		oss << "<attSpectrum/>\n"; 
		oss << "</Attributes>\n";		
		oss << "</GainTrackingTable>\n";

		return oss.str();				
	}
	
	string GainTrackingTable::toMIME(const asdm::ByteOrder* byteOrder) {
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

	
	void GainTrackingTable::setFromMIME(const string & mimeMsg) {
    string xmlPartMIMEHeader = "Content-ID: <header.xml>\n\n";
    
    string binPartMIMEHeader = "--MIME_boundary\nContent-Type: binary/octet-stream\nContent-ID: <content.bin>\n\n";
    
    // Detect the XML header.
    string::size_type loc0 = mimeMsg.find(xmlPartMIMEHeader, 0);
    if ( loc0 == string::npos) {
      throw ConversionException("Failed to detect the beginning of the XML header", "GainTracking");
    }
    loc0 += xmlPartMIMEHeader.size();
    
    // Look for the string announcing the binary part.
    string::size_type loc1 = mimeMsg.find( binPartMIMEHeader, loc0 );
    
    if ( loc1 == string::npos ) {
      throw ConversionException("Failed to detect the beginning of the binary part", "GainTracking");
    }
    
    //
    // Extract the xmlHeader and analyze it to find out what is the byte order and the sequence
    // of attribute names.
    //
    string xmlHeader = mimeMsg.substr(loc0, loc1-loc0);
    xmlDoc *doc;
    doc = xmlReadMemory(xmlHeader.data(), xmlHeader.size(), "BinaryTableHeader.xml", NULL, XML_PARSE_NOBLANKS);
    if ( doc == NULL ) 
      throw ConversionException("Failed to parse the xmlHeader into a DOM structure.", "GainTracking");
    
   // This vector will be filled by the names of  all the attributes of the table
   // in the order in which they are expected to be found in the binary representation.
   //
    vector<string> attributesSeq;
      
    xmlNode* root_element = xmlDocGetRootElement(doc);
    if ( root_element == NULL || root_element->type != XML_ELEMENT_NODE )
      throw ConversionException("Failed to parse the xmlHeader into a DOM structure.", "GainTracking");
    
    const ByteOrder* byteOrder;
    if ( string("ASDMBinaryTable").compare((const char*) root_element->name) == 0) {
      // Then it's an "old fashioned" MIME file for tables.
      // Just try to deserialize it with Big_Endian for the bytes ordering.
      byteOrder = asdm::ByteOrder::Big_Endian;
      
 	 //
    // Let's consider a  default order for the sequence of attributes.
    //
     
    attributesSeq.push_back("antennaId") ; 
     
    attributesSeq.push_back("spectralWindowId") ; 
     
    attributesSeq.push_back("timeInterval") ; 
     
    attributesSeq.push_back("feedId") ; 
     
    attributesSeq.push_back("attenuator") ; 
     
    attributesSeq.push_back("numLO") ; 
     
    attributesSeq.push_back("numReceptor") ; 
     
    attributesSeq.push_back("cableDelay") ; 
     
    attributesSeq.push_back("crossPolarizationDelay") ; 
     
    attributesSeq.push_back("loPropagationDelay") ; 
     
    attributesSeq.push_back("polarizationTypes") ; 
     
    attributesSeq.push_back("receiverDelay") ; 
    
     
    attributesSeq.push_back("delayOffset") ; 
     
    attributesSeq.push_back("freqOffset") ; 
     
    attributesSeq.push_back("phaseOffset") ; 
     
    attributesSeq.push_back("samplingLevel") ; 
     
    attributesSeq.push_back("numAttFreq") ; 
     
    attributesSeq.push_back("attFreq") ; 
     
    attributesSeq.push_back("attSpectrum") ; 
              
     }
    else if (string("GainTrackingTable").compare((const char*) root_element->name) == 0) {
      // It's a new (and correct) MIME file for tables.
      //
      // 1st )  Look for a BulkStoreRef element with an attribute byteOrder.
      //
      xmlNode* bulkStoreRef = 0;
      xmlNode* child = root_element->children;
      
      // Skip the two first children (Entity and ContainerEntity).
      bulkStoreRef = (child ==  0) ? 0 : ( (child->next) == 0 ? 0 : child->next->next );
      
      if ( bulkStoreRef == 0 || (bulkStoreRef->type != XML_ELEMENT_NODE)  || (string("BulkStoreRef").compare((const char*) bulkStoreRef->name) != 0))
      	throw ConversionException ("Could not find the element '/GainTrackingTable/BulkStoreRef'. Invalid XML header '"+ xmlHeader + "'.", "GainTracking");
      	
      // We found BulkStoreRef, now look for its attribute byteOrder.
      _xmlAttr* byteOrderAttr = 0;
      for (struct _xmlAttr* attr = bulkStoreRef->properties; attr; attr = attr->next) 
	  if (string("byteOrder").compare((const char*) attr->name) == 0) {
	   byteOrderAttr = attr;
	   break;
	 }
      
      if (byteOrderAttr == 0) 
	     throw ConversionException("Could not find the element '/GainTrackingTable/BulkStoreRef/@byteOrder'. Invalid XML header '" + xmlHeader +"'.", "GainTracking");
      
      string byteOrderValue = string((const char*) byteOrderAttr->children->content);
      if (!(byteOrder = asdm::ByteOrder::fromString(byteOrderValue)))
		throw ConversionException("No valid value retrieved for the element '/GainTrackingTable/BulkStoreRef/@byteOrder'. Invalid XML header '" + xmlHeader + "'.", "GainTracking");
		
	 //
	 // 2nd) Look for the Attributes element and grab the names of the elements it contains.
	 //
	 xmlNode* attributes = bulkStoreRef->next;
     if ( attributes == 0 || (attributes->type != XML_ELEMENT_NODE)  || (string("Attributes").compare((const char*) attributes->name) != 0))	 
       	throw ConversionException ("Could not find the element '/GainTrackingTable/Attributes'. Invalid XML header '"+ xmlHeader + "'.", "GainTracking");
 
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
	GainTrackingRow* aRow = GainTrackingRow::fromBin(eiss, *this, attributesSeq);
	checkAndAdd(aRow);
      }
    }
    catch (DuplicateKey e) {
      throw ConversionException("Error while writing binary data , the message was "
				+ e.getMessage(), "GainTracking");
    }
    catch (TagFormatException e) {
      throw ConversionException("Error while reading binary data , the message was "
				+ e.getMessage(), "GainTracking");
    }
    archiveAsBin = true;
    fileAsBin = true;
	}

	
	void GainTrackingTable::toFile(string directory) {
		if (!directoryExists(directory.c_str()) &&
			!createPath(directory.c_str())) {
			throw ConversionException("Could not create directory " , directory);
		}

		string fileName = directory + "/GainTracking.xml";
		ofstream tableout(fileName.c_str(),ios::out|ios::trunc);
		if (tableout.rdstate() == ostream::failbit)
			throw ConversionException("Could not open file " + fileName + " to write ", "GainTracking");
		if (fileAsBin) 
			tableout << MIMEXMLPart();
		else
			tableout << toXML() << endl;
		tableout.close();
		if (tableout.rdstate() == ostream::failbit)
			throw ConversionException("Could not close file " + fileName, "GainTracking");

		if (fileAsBin) {
			// write the bin serialized
			string fileName = directory + "/GainTracking.bin";
			ofstream tableout(fileName.c_str(),ios::out|ios::trunc);
			if (tableout.rdstate() == ostream::failbit)
				throw ConversionException("Could not open file " + fileName + " to write ", "GainTracking");
			tableout << toMIME() << endl;
			tableout.close();
			if (tableout.rdstate() == ostream::failbit)
				throw ConversionException("Could not close file " + fileName, "GainTracking");
		}
	}

	
	void GainTrackingTable::setFromFile(const string& directory) {		
    if (boost::filesystem::exists(boost::filesystem::path(uniqSlashes(directory + "/GainTracking.xml"))))
      setFromXMLFile(directory);
    else if (boost::filesystem::exists(boost::filesystem::path(uniqSlashes(directory + "/GainTracking.bin"))))
      setFromMIMEFile(directory);
    else
      throw ConversionException("No file found for the GainTracking table", "GainTracking");
	}			

	
  void GainTrackingTable::setFromMIMEFile(const string& directory) {
    string tablePath ;
    
    tablePath = directory + "/GainTracking.bin";
    ifstream tablefile(tablePath.c_str(), ios::in|ios::binary);
    if (!tablefile.is_open()) { 
      throw ConversionException("Could not open file " + tablePath, "GainTracking");
    }
    // Read in a stringstream.
    stringstream ss; ss << tablefile.rdbuf();
    
    if (tablefile.rdstate() == istream::failbit || tablefile.rdstate() == istream::badbit) {
      throw ConversionException("Error reading file " + tablePath,"GainTracking");
    }
    
    // And close.
    tablefile.close();
    if (tablefile.rdstate() == istream::failbit)
      throw ConversionException("Could not close file " + tablePath,"GainTracking");
    
    setFromMIME(ss.str());
  }	

	
void GainTrackingTable::setFromXMLFile(const string& directory) {
    string tablePath ;
    
    tablePath = directory + "/GainTracking.xml";
    ifstream tablefile(tablePath.c_str(), ios::in|ios::binary);
    if (!tablefile.is_open()) { 
      throw ConversionException("Could not open file " + tablePath, "GainTracking");
    }
      // Read in a stringstream.
    stringstream ss;
    ss << tablefile.rdbuf();
    
    if  (tablefile.rdstate() == istream::failbit || tablefile.rdstate() == istream::badbit) {
      throw ConversionException("Error reading file '" + tablePath + "'", "GainTracking");
    }
    
    // And close
    tablefile.close();
    if (tablefile.rdstate() == istream::failbit)
      throw ConversionException("Could not close file '" + tablePath + "'", "GainTracking");

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

	

	

			
	
		
		
	/**
	 * Insert a GainTrackingRow* in a vector of GainTrackingRow* so that it's ordered by ascending start time.
	 *
	 * @param GainTrackingRow* x . The pointer to be inserted.
	 * @param vector <GainTrackingRow*>& row. A reference to the vector where to insert x.
	 *
	 */
	 GainTrackingRow* GainTrackingTable::insertByStartTime(GainTrackingRow* x, vector<GainTrackingRow*>& row) {
				
		vector <GainTrackingRow*>::iterator theIterator;
		
		ArrayTime start = x->timeInterval.getStart();

    	// Is the row vector empty ?
    	if (row.size() == 0) {
    		row.push_back(x);
    		privateRows.push_back(x);
    		x->isAdded(true);
    		return x;
    	}
    	
    	// Optimization for the case of insertion by ascending time.
    	GainTrackingRow* last = *(row.end()-1);
        
    	if ( start > last->timeInterval.getStart() ) {
 	    	//
	    	// Modify the duration of last if and only if the start time of x
	    	// is located strictly before the end time of last.
	    	//
	  		if ( start < (last->timeInterval.getStart() + last->timeInterval.getDuration()))   		
    			last->timeInterval.setDuration(start - last->timeInterval.getStart());
    		row.push_back(x);
    		privateRows.push_back(x);
    		x->isAdded(true);
    		return x;
    	}
    	
    	// Optimization for the case of insertion by descending time.
    	GainTrackingRow* first = *(row.begin());
        
    	if ( start < first->timeInterval.getStart() ) {
			//
	  		// Modify the duration of x if and only if the start time of first
	  		// is located strictly before the end time of x.
	  		//
	  		if ( first->timeInterval.getStart() < (start + x->timeInterval.getDuration()) )	  		
    			x->timeInterval.setDuration(first->timeInterval.getStart() - start);
    		row.insert(row.begin(), x);
    		privateRows.push_back(x);
    		x->isAdded(true);
    		return x;
    	}
    	
    	// Case where x has to be inserted inside row; let's use a dichotomy
    	// method to find the insertion index.
		unsigned int k0 = 0;
		unsigned int k1 = row.size() - 1;
	
		while (k0 != (k1 - 1)) {
			if (start == row[k0]->timeInterval.getStart()) {
				if (row[k0]->equalByRequiredValue(x))
					return row[k0];
				else
					throw DuplicateKey("DuplicateKey exception in ", "GainTrackingTable");	
			}
			else if (start == row[k1]->timeInterval.getStart()) {
				if (row[k1]->equalByRequiredValue(x))
					return row[k1];
				else
					throw DuplicateKey("DuplicateKey exception in ", "GainTrackingTable");	
			}
			else {
				if (start <= row[(k0+k1)/2]->timeInterval.getStart())
					k1 = (k0 + k1) / 2;
				else
					k0 = (k0 + k1) / 2;				
			} 	
		}

		if (start == row[k0]->timeInterval.getStart()) {
			if (row[k0]->equalByRequiredValue(x))
				return row[k0];
			else
				throw DuplicateKey("DuplicateKey exception in ", "GainTrackingTable");	
		}
		else if (start == row[k1]->timeInterval.getStart()) {
			if (row[k1]->equalByRequiredValue(x))
				return row[k1];
			else
				throw DuplicateKey("DuplicateKey exception in ", "GainTrackingTable");	
		}	

		row[k0]->timeInterval.setDuration(start-row[k0]->timeInterval.getStart());
		x->timeInterval.setDuration(row[k0+1]->timeInterval.getStart() - start);
		row.insert(row.begin()+(k0+1), x);
		privateRows.push_back(x);
   		x->isAdded(true);
		return x;   
    } 
    	
	
	

	
} // End namespace asdm
 
