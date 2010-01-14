
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
 * File ReceiverTable.cpp
 */
#include <ConversionException.h>
#include <DuplicateKey.h>
#include <OutOfBoundsException.h>

using asdm::ConversionException;
using asdm::DuplicateKey;
using asdm::OutOfBoundsException;

#include <ASDM.h>
#include <ReceiverTable.h>
#include <ReceiverRow.h>
#include <Parser.h>

using asdm::ASDM;
using asdm::ReceiverTable;
using asdm::ReceiverRow;
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

	string ReceiverTable::tableName = "Receiver";
	

	/**
	 * The list of field names that make up key key.
	 * (Initialization is in the constructor.)
	 */
	vector<string> ReceiverTable::key;

	/**
	 * Return the list of field names that make up key key
	 * as an array of strings.
	 */	
	vector<string> ReceiverTable::getKeyName() {
		return key;
	}


	ReceiverTable::ReceiverTable(ASDM &c) : container(c) {

	
		key.push_back("receiverId");
	
		key.push_back("spectralWindowId");
	
		key.push_back("timeInterval");
	


		// Define a default entity.
		entity.setEntityId(EntityId("uid://X0/X0/X0"));
		entity.setEntityIdEncrypted("na");
		entity.setEntityTypeName("ReceiverTable");
		entity.setEntityVersion("1");
		entity.setInstanceVersion("1");
		
		// Archive XML
		archiveAsBin = false;
		
		// File XML
		fileAsBin = false;
	}
	
/**
 * A destructor for ReceiverTable.
 */
 
	ReceiverTable::~ReceiverTable() {
		for (unsigned int i = 0; i < privateRows.size(); i++) 
			delete(privateRows.at(i));
	}

	/**
	 * Container to which this table belongs.
	 */
	ASDM &ReceiverTable::getContainer() const {
		return container;
	}

	/**
	 * Return the number of rows in the table.
	 */
	unsigned int ReceiverTable::size() {
		return privateRows.size();
	}
	

	/**
	 * Return the name of this table.
	 */
	string ReceiverTable::getName() const {
		return tableName;
	}

	/**
	 * Return this table's Entity.
	 */
	Entity ReceiverTable::getEntity() const {
		return entity;
	}

	/**
	 * Set this table's Entity.
	 */
	void ReceiverTable::setEntity(Entity e) {
		this->entity = e; 
	}
	
	//
	// ====> Row creation.
	//
	
	/**
	 * Create a new row.
	 */
	ReceiverRow *ReceiverTable::newRow() {
		return new ReceiverRow (*this);
	}
	

	/**
	 * Create a new row initialized to the specified values.
	 * @return a pointer on the created and initialized row.
	
 	 * @param spectralWindowId 
	
 	 * @param timeInterval 
	
 	 * @param name 
	
 	 * @param numLO 
	
 	 * @param frequencyBand 
	
 	 * @param freqLO 
	
 	 * @param receiverSideband 
	
 	 * @param sidebandLO 
	
     */
	ReceiverRow* ReceiverTable::newRow(Tag spectralWindowId, ArrayTimeInterval timeInterval, string name, int numLO, ReceiverBandMod::ReceiverBand frequencyBand, vector<Frequency > freqLO, ReceiverSidebandMod::ReceiverSideband receiverSideband, vector<NetSidebandMod::NetSideband > sidebandLO){
		ReceiverRow *row = new ReceiverRow(*this);
			
		row->setSpectralWindowId(spectralWindowId);
			
		row->setTimeInterval(timeInterval);
			
		row->setName(name);
			
		row->setNumLO(numLO);
			
		row->setFrequencyBand(frequencyBand);
			
		row->setFreqLO(freqLO);
			
		row->setReceiverSideband(receiverSideband);
			
		row->setSidebandLO(sidebandLO);
	
		return row;		
	}	
	


ReceiverRow* ReceiverTable::newRow(ReceiverRow* row) {
	return new ReceiverRow(*this, *row);
}

	//
	// Append a row to its table.
	//

	
	
	
		
			

	/** 
	 * Returns a string built by concatenating the ascii representation of the
	 * parameters values suffixed with a "_" character.
	 */
	 string ReceiverTable::Key(Tag spectralWindowId) {
	 	ostringstream ostrstr;
	 		ostrstr  
			
				<< spectralWindowId.toString()  << "_"
			
			;
		return ostrstr.str();	 	
	 }

	/**
	 * Append a row to a ReceiverTable which has simply 
	 * 1) an autoincrementable attribute  (receiverId) 
	 * 2) a temporal attribute (timeInterval) in its key section.
	 * 3) other attributes in the key section (defining a so called context).
	 * If there is already a row in the table whose key section non including is equal to x's one and
	 * whose value section is equal to x's one then return this row, otherwise add x to the collection
	 * of rows.
	 */
	ReceiverRow* ReceiverTable::add(ReceiverRow* x) {
		// Get the start time of the row to be inserted.
		ArrayTime startTime = x->getTimeInterval().getStart();
		// cout << "Trying to add a new row with start time = " << startTime << endl;
		int insertionId = 0;

		 
		// Determine the entry in the context map from the appropriates attributes.
		string k = Key(
						x->getSpectralWindowId()
					   );
					   

		// Determine the insertion index for the row x, possibly returning a pointer to a row identical to x. 					   
		if (context.find(k) != context.end()) {
			// cout << "The context " << k << " already exists " << endl;
			for (unsigned int i = 0; i < context[k].size(); i++) {
				//cout << "Looking for a same starttime in i = " << i << endl;
				for (unsigned int j=0; j<context[k][i].size(); j++) 
					if (context[k][i][j]->getTimeInterval().getStart().equals(startTime)) {
						if (
						
						 (context[k][i][j]->getName() == x->getName())
						 && 

						 (context[k][i][j]->getNumLO() == x->getNumLO())
						 && 

						 (context[k][i][j]->getFrequencyBand() == x->getFrequencyBand())
						 && 

						 (context[k][i][j]->getFreqLO() == x->getFreqLO())
						 && 

						 (context[k][i][j]->getReceiverSideband() == x->getReceiverSideband())
						 && 

						 (context[k][i][j]->getSidebandLO() == x->getSidebandLO())
						
						) {
							// cout << "A row equal to x has been found, I return it " << endl;
							return context[k][i][j];
						}
						
						// Otherwise we must autoincrement receiverId and
						// insert a new ReceiverRow with this autoincremented value.
						insertionId = i+1;
						break;
						
						// And goto insertion
						// goto done;
					}
			}
			//cout << "No row with the same start time than x, it will be inserted in row with id = 0" << endl;
			// insertionId = 0;
		}
		else { // There is not yet a context ...
			   // Create and initialize an entry in the context map for this combination....
			// cout << "Starting a new context " << k << endl;
			ID_TIME_ROWS vv;
			context[k] = vv;
			insertionId = 0;
		}
		
		
			x->setReceiverId(insertionId);
		
			if (insertionId >= (int) context[k].size()) context[k].resize(insertionId+1);
			return insertByStartTime(x, context[k][insertionId]);
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
	ReceiverRow*  ReceiverTable::checkAndAdd(ReceiverRow* x) {
		ArrayTime startTime = x->getTimeInterval().getStart();		
		
		// Determine the entry in the context map from the appropriate attributes.
		string k = Key(
		                x->getSpectralWindowId()
		               );

		// Uniqueness Rule Check
		if (context.find(k) != context.end()) {
			for (unsigned int i = 0;  i < context[k].size(); i++) 
				for (unsigned int j = 0; j < context[k][i].size(); j++)
					if (
						(context[k][i][j]->getTimeInterval().getStart().equals(startTime)) 
					
						 && (context[k][i][j]->getName() == x->getName())
					

						 && (context[k][i][j]->getNumLO() == x->getNumLO())
					

						 && (context[k][i][j]->getFrequencyBand() == x->getFrequencyBand())
					

						 && (context[k][i][j]->getFreqLO() == x->getFreqLO())
					

						 && (context[k][i][j]->getReceiverSideband() == x->getReceiverSideband())
					

						 && (context[k][i][j]->getSidebandLO() == x->getSidebandLO())
					
					)
						throw UniquenessViolationException("Uniqueness violation exception in table ReceiverTable");			
		}


		// Good, now it's time to insert the row x, possibly triggering a DuplicateKey exception.	
		
		ID_TIME_ROWS dummyPlane;

		// Determine the integer representation of the identifier of the row (x) to be inserted. 
		int id = 
				x->getReceiverId();
				
	
		if (context.find(k) != context.end()) {
			if (id >= (int) context[k].size()) 
				context[k].resize(id+1);
			else {
				// This receiverId 's value has already rows for this context.
				// Check that there is not yet a row with the same time. (simply check start time)
				// If there is such a row then trigger a Duplicate Key Exception.
				for (unsigned int j = 0; j < context[k][id].size(); j++)
					if (context[k][id][j]->getTimeInterval().getStart().equals(startTime))
						throw DuplicateKey("Duplicate key exception in ", "ReceiverTable"); 
			}					
		}
		else {
			context[k] = dummyPlane;
			context[k].resize(id+1);
		}
		return insertByStartTime(x, context[k][id]);
	}
		







	

	
	
	/**
	 * Get all rows.
	 * @return Alls rows as an array of ReceiverRow
	 */
	vector<ReceiverRow *> ReceiverTable::get()  {
		return privateRows;
	/*	
		vector<ReceiverRow *> v;
		
		map<string, ID_TIME_ROWS >::iterator mapIter = context.begin();
		ID_TIME_ROWS::iterator planeIter;
		vector<ReceiverRow*>::iterator rowIter; 
		for (mapIter=context.begin(); mapIter!=context.end(); mapIter++)
			for (planeIter=((*mapIter).second).begin(); planeIter != ((*mapIter).second).end(); planeIter++)
				for (rowIter=(*planeIter).begin(); rowIter != (*planeIter).end(); rowIter++)
					v.push_back(*rowIter);
		return v;
	*/
	}	
	


	
		
		
			
			 
/*
 ** Returns a ReceiverRow* given a key.
 ** @return a pointer to the row having the key whose values are passed as parameters, or 0 if
 ** no row exists for that key.
 **
 */
 	ReceiverRow* ReceiverTable::getRowByKey(int receiverId, Tag spectralWindowId, ArrayTimeInterval timeInterval)  {	
		ArrayTime start = timeInterval.getStart();
		
		map<string, ID_TIME_ROWS >::iterator mapIter;
		if ((mapIter = context.find(Key(spectralWindowId))) != context.end()) {
			
			int id = receiverId;
			
			if (id < (int) ((*mapIter).second).size()) {
				vector <ReceiverRow*>::iterator rowIter;
				for (rowIter = ((*mapIter).second)[id].begin(); rowIter != ((*mapIter).second)[id].end(); rowIter++) {
					if ((*rowIter)->getTimeInterval().contains(timeInterval))
						return *rowIter; 
				}
			}
		}
		return 0;
	}
/*
 * Returns a vector of pointers on rows whose key element receiverId 
 * is equal to the parameter receiverId.
 * @return a vector of vector <ReceiverRow *>. A returned vector of size 0 means that no row has been found.
 * @param receiverId int contains the value of
 * the autoincrementable attribute that is looked up in the table.
 */
 vector <ReceiverRow *>  ReceiverTable::getRowByReceiverId(int receiverId) {
	vector<ReceiverRow *> list;
	map<string, ID_TIME_ROWS >::iterator mapIter;
	
	for (mapIter=context.begin(); mapIter!=context.end(); mapIter++) {
		int maxId = ((*mapIter).second).size();
		if (receiverId < maxId) {
			vector<ReceiverRow *>::iterator rowIter;
			for (rowIter=((*mapIter).second)[receiverId].begin(); 
			     rowIter!=((*mapIter).second)[receiverId].end(); rowIter++)
				list.push_back(*rowIter);
		}
	}
	return list;	
 }			
			
		
		
		
/**
 * Look up the table for a row whose all attributes  except the autoincrementable one 
 * are equal to the corresponding parameters of the method.
 * @return a pointer on this row if any, 0 otherwise.
 *
			
 * @param <<ExtrinsicAttribute>> spectralWindowId.
 	 		
 * @param <<ASDMAttribute>> timeInterval.
 	 		
 * @param <<ASDMAttribute>> name.
 	 		
 * @param <<ASDMAttribute>> numLO.
 	 		
 * @param <<ASDMAttribute>> frequencyBand.
 	 		
 * @param <<ArrayAttribute>> freqLO.
 	 		
 * @param <<ASDMAttribute>> receiverSideband.
 	 		
 * @param <<ArrayAttribute>> sidebandLO.
 	 		 
 */
ReceiverRow* ReceiverTable::lookup(Tag spectralWindowId, ArrayTimeInterval timeInterval, string name, int numLO, ReceiverBandMod::ReceiverBand frequencyBand, vector<Frequency > freqLO, ReceiverSidebandMod::ReceiverSideband receiverSideband, vector<NetSidebandMod::NetSideband > sidebandLO) {		
		using asdm::ArrayTimeInterval;
		map<string, ID_TIME_ROWS >::iterator mapIter;
		string k = Key(spectralWindowId);
		if ((mapIter = context.find(k)) != context.end()) {
			ID_TIME_ROWS::iterator planeIter;
			for (planeIter = context[k].begin(); planeIter != context[k].end(); planeIter++)  {
				vector <ReceiverRow*>::iterator rowIter;
				for (rowIter = (*planeIter).begin(); rowIter != (*planeIter).end(); rowIter++) {
					if ((*rowIter)->getTimeInterval().contains(timeInterval)
					    && (*rowIter)->compareRequiredValue(name, numLO, frequencyBand, freqLO, receiverSideband, sidebandLO)) {
						return *rowIter;
					} 
				}
			}
		}				
		return 0;	
} 
		
	




#ifndef WITHOUT_ACS
	// Conversion Methods

	ReceiverTableIDL *ReceiverTable::toIDL() {
		ReceiverTableIDL *x = new ReceiverTableIDL ();
		unsigned int nrow = size();
		x->row.length(nrow);
		vector<ReceiverRow*> v = get();
		for (unsigned int i = 0; i < nrow; ++i) {
			x->row[i] = *(v[i]->toIDL());
		}
		return x;
	}
#endif
	
#ifndef WITHOUT_ACS
	void ReceiverTable::fromIDL(ReceiverTableIDL x) {
		unsigned int nrow = x.row.length();
		for (unsigned int i = 0; i < nrow; ++i) {
			ReceiverRow *tmp = newRow();
			tmp->setFromIDL(x.row[i]);
			// checkAndAdd(tmp);
			add(tmp);
		}
	}
#endif

	
	string ReceiverTable::toXML()  {
		string buf;

		buf.append("<?xml version=\"1.0\" encoding=\"ISO-8859-1\"?> ");
		buf.append("<ReceiverTable xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" xmlns:rcvr=\"http://Alma/XASDM/ReceiverTable\" xsi:schemaLocation=\"http://Alma/XASDM/ReceiverTable http://almaobservatory.org/XML/XASDM/2/ReceiverTable.xsd\" schemaVersion=\"2\" schemaRevision=\"1.53\">\n");
	
		buf.append(entity.toXML());
		string s = container.getEntity().toXML();
		// Change the "Entity" tag to "ContainerEntity".
		buf.append("<Container" + s.substr(1,s.length() - 1)+" ");
		vector<ReceiverRow*> v = get();
		for (unsigned int i = 0; i < v.size(); ++i) {
			try {
				buf.append(v[i]->toXML());
			} catch (NoSuchRow e) {
			}
			buf.append("  ");
		}		
		buf.append("</ReceiverTable> ");
		return buf;
	}

	
	void ReceiverTable::fromXML(string xmlDoc)  {
		Parser xml(xmlDoc);
		if (!xml.isStr("<ReceiverTable")) 
			error();
		// cout << "Parsing a ReceiverTable" << endl;
		string s = xml.getElement("<Entity","/>");
		if (s.length() == 0) 
			error();
		Entity e;
		e.setFromXML(s);
		if (e.getEntityTypeName() != "ReceiverTable")
			error();
		setEntity(e);
		// Skip the container's entity; but, it has to be there.
		s = xml.getElement("<ContainerEntity","/>");
		if (s.length() == 0) 
			error();

		// Get each row in the table.
		s = xml.getElementContent("<row>","</row>");
		ReceiverRow *row;
		while (s.length() != 0) {
			// cout << "Parsing a ReceiverRow" << endl; 
			row = newRow();
			row->setFromXML(s);
			try {
				checkAndAdd(row);
			} catch (DuplicateKey e1) {
				throw ConversionException(e1.getMessage(),"ReceiverTable");
			} 
			catch (UniquenessViolationException e1) {
				throw ConversionException(e1.getMessage(),"ReceiverTable");	
			}
			catch (...) {
				// cout << "Unexpected error in ReceiverTable::checkAndAdd called from ReceiverTable::fromXML " << endl;
			}
			s = xml.getElementContent("<row>","</row>");
		}
		if (!xml.isStr("</ReceiverTable>")) 
			error();
			
		archiveAsBin = false;
		fileAsBin = false;
		
	}

	
	void ReceiverTable::error()  {
		throw ConversionException("Invalid xml document","Receiver");
	}
	
	
	string ReceiverTable::MIMEXMLPart(const asdm::ByteOrder* byteOrder) {
		string UID = getEntity().getEntityId().toString();
		string withoutUID = UID.substr(6);
		string containerUID = getContainer().getEntity().getEntityId().toString();
		ostringstream oss;
		oss << "<?xml version='1.0'  encoding='ISO-8859-1'?>";
		oss << "\n";
		oss << "<ReceiverTable xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" xmlns:rcvr=\"http://Alma/XASDM/ReceiverTable\" xsi:schemaLocation=\"http://Alma/XASDM/ReceiverTable http://almaobservatory.org/XML/XASDM/2/ReceiverTable.xsd\" schemaVersion=\"2\" schemaRevision=\"1.53\">\n";
		oss<< "<Entity entityId='"<<UID<<"' entityIdEncrypted='na' entityTypeName='ReceiverTable' schemaVersion='1' documentVersion='1'/>\n";
		oss<< "<ContainerEntity entityId='"<<containerUID<<"' entityIdEncrypted='na' entityTypeName='ASDM' schemaVersion='1' documentVersion='1'/>\n";
		oss << "<BulkStoreRef file_id='"<<withoutUID<<"' byteOrder='"<<byteOrder->toString()<<"' />\n";
		oss << "<Attributes>\n";

		oss << "<receiverId/>\n"; 
		oss << "<spectralWindowId/>\n"; 
		oss << "<timeInterval/>\n"; 
		oss << "<name/>\n"; 
		oss << "<numLO/>\n"; 
		oss << "<frequencyBand/>\n"; 
		oss << "<freqLO/>\n"; 
		oss << "<receiverSideband/>\n"; 
		oss << "<sidebandLO/>\n"; 

		oss << "</Attributes>\n";		
		oss << "</ReceiverTable>\n";

		return oss.str();				
	}
	
	string ReceiverTable::toMIME(const asdm::ByteOrder* byteOrder) {
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

	
	void ReceiverTable::setFromMIME(const string & mimeMsg) {
    string xmlPartMIMEHeader = "Content-ID: <header.xml>\n\n";
    
    string binPartMIMEHeader = "--MIME_boundary\nContent-Type: binary/octet-stream\nContent-ID: <content.bin>\n\n";
    
    // Detect the XML header.
    string::size_type loc0 = mimeMsg.find(xmlPartMIMEHeader, 0);
    if ( loc0 == string::npos) {
      throw ConversionException("Failed to detect the beginning of the XML header", "Receiver");
    }
    loc0 += xmlPartMIMEHeader.size();
    
    // Look for the string announcing the binary part.
    string::size_type loc1 = mimeMsg.find( binPartMIMEHeader, loc0 );
    
    if ( loc1 == string::npos ) {
      throw ConversionException("Failed to detect the beginning of the binary part", "Receiver");
    }
    
    //
    // Extract the xmlHeader and analyze it to find out what is the byte order and the sequence
    // of attribute names.
    //
    string xmlHeader = mimeMsg.substr(loc0, loc1-loc0);
    xmlDoc *doc;
    doc = xmlReadMemory(xmlHeader.data(), xmlHeader.size(), "BinaryTableHeader.xml", NULL, XML_PARSE_NOBLANKS);
    if ( doc == NULL ) 
      throw ConversionException("Failed to parse the xmlHeader into a DOM structure.", "Receiver");
    
   // This vector will be filled by the names of  all the attributes of the table
   // in the order in which they are expected to be found in the binary representation.
   //
    vector<string> attributesSeq;
      
    xmlNode* root_element = xmlDocGetRootElement(doc);
    if ( root_element == NULL || root_element->type != XML_ELEMENT_NODE )
      throw ConversionException("Failed to parse the xmlHeader into a DOM structure.", "Receiver");
    
    const ByteOrder* byteOrder;
    if ( string("ASDMBinaryTable").compare((const char*) root_element->name) == 0) {
      // Then it's an "old fashioned" MIME file for tables.
      // Just try to deserialize it with Big_Endian for the bytes ordering.
      byteOrder = asdm::ByteOrder::Big_Endian;
      
 	 //
    // Let's consider a  default order for the sequence of attributes.
    //
     
    attributesSeq.push_back("receiverId") ; 
     
    attributesSeq.push_back("spectralWindowId") ; 
     
    attributesSeq.push_back("timeInterval") ; 
     
    attributesSeq.push_back("name") ; 
     
    attributesSeq.push_back("numLO") ; 
     
    attributesSeq.push_back("frequencyBand") ; 
     
    attributesSeq.push_back("freqLO") ; 
     
    attributesSeq.push_back("receiverSideband") ; 
     
    attributesSeq.push_back("sidebandLO") ; 
    
              
     }
    else if (string("ReceiverTable").compare((const char*) root_element->name) == 0) {
      // It's a new (and correct) MIME file for tables.
      //
      // 1st )  Look for a BulkStoreRef element with an attribute byteOrder.
      //
      xmlNode* bulkStoreRef = 0;
      xmlNode* child = root_element->children;
      
      // Skip the two first children (Entity and ContainerEntity).
      bulkStoreRef = (child ==  0) ? 0 : ( (child->next) == 0 ? 0 : child->next->next );
      
      if ( bulkStoreRef == 0 || (bulkStoreRef->type != XML_ELEMENT_NODE)  || (string("BulkStoreRef").compare((const char*) bulkStoreRef->name) != 0))
      	throw ConversionException ("Could not find the element '/ReceiverTable/BulkStoreRef'. Invalid XML header '"+ xmlHeader + "'.", "Receiver");
      	
      // We found BulkStoreRef, now look for its attribute byteOrder.
      _xmlAttr* byteOrderAttr = 0;
      for (struct _xmlAttr* attr = bulkStoreRef->properties; attr; attr = attr->next) 
	  if (string("byteOrder").compare((const char*) attr->name) == 0) {
	   byteOrderAttr = attr;
	   break;
	 }
      
      if (byteOrderAttr == 0) 
	     throw ConversionException("Could not find the element '/ReceiverTable/BulkStoreRef/@byteOrder'. Invalid XML header '" + xmlHeader +"'.", "Receiver");
      
      string byteOrderValue = string((const char*) byteOrderAttr->children->content);
      if (!(byteOrder = asdm::ByteOrder::fromString(byteOrderValue)))
		throw ConversionException("No valid value retrieved for the element '/ReceiverTable/BulkStoreRef/@byteOrder'. Invalid XML header '" + xmlHeader + "'.", "Receiver");
		
	 //
	 // 2nd) Look for the Attributes element and grab the names of the elements it contains.
	 //
	 xmlNode* attributes = bulkStoreRef->next;
     if ( attributes == 0 || (attributes->type != XML_ELEMENT_NODE)  || (string("Attributes").compare((const char*) attributes->name) != 0))	 
       	throw ConversionException ("Could not find the element '/ReceiverTable/Attributes'. Invalid XML header '"+ xmlHeader + "'.", "Receiver");
 
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
	ReceiverRow* aRow = ReceiverRow::fromBin(eiss, *this, attributesSeq);
	checkAndAdd(aRow);
      }
    }
    catch (DuplicateKey e) {
      throw ConversionException("Error while writing binary data , the message was "
				+ e.getMessage(), "Receiver");
    }
    catch (TagFormatException e) {
      throw ConversionException("Error while reading binary data , the message was "
				+ e.getMessage(), "Receiver");
    }
    archiveAsBin = true;
    fileAsBin = true;
	}

	
	void ReceiverTable::toFile(string directory) {
		if (!directoryExists(directory.c_str()) &&
			!createPath(directory.c_str())) {
			throw ConversionException("Could not create directory " , directory);
		}

		string fileName = directory + "/Receiver.xml";
		ofstream tableout(fileName.c_str(),ios::out|ios::trunc);
		if (tableout.rdstate() == ostream::failbit)
			throw ConversionException("Could not open file " + fileName + " to write ", "Receiver");
		if (fileAsBin) 
			tableout << MIMEXMLPart();
		else
			tableout << toXML() << endl;
		tableout.close();
		if (tableout.rdstate() == ostream::failbit)
			throw ConversionException("Could not close file " + fileName, "Receiver");

		if (fileAsBin) {
			// write the bin serialized
			string fileName = directory + "/Receiver.bin";
			ofstream tableout(fileName.c_str(),ios::out|ios::trunc);
			if (tableout.rdstate() == ostream::failbit)
				throw ConversionException("Could not open file " + fileName + " to write ", "Receiver");
			tableout << toMIME() << endl;
			tableout.close();
			if (tableout.rdstate() == ostream::failbit)
				throw ConversionException("Could not close file " + fileName, "Receiver");
		}
	}

	
	void ReceiverTable::setFromFile(const string& directory) {
    if (boost::filesystem::exists(boost::filesystem::path(directory + "/Receiver.xml")))
      setFromXMLFile(directory);
    else if (boost::filesystem::exists(boost::filesystem::path(directory + "/Receiver.bin")))
      setFromMIMEFile(directory);
    else
      throw ConversionException("No file found for the Receiver table", "Receiver");
	}			

	
  void ReceiverTable::setFromMIMEFile(const string& directory) {
    string tablePath ;
    
    tablePath = directory + "/Receiver.bin";
    ifstream tablefile(tablePath.c_str(), ios::in|ios::binary);
    if (!tablefile.is_open()) { 
      throw ConversionException("Could not open file " + tablePath, "Receiver");
    }
    // Read in a stringstream.
    stringstream ss; ss << tablefile.rdbuf();
    
    if (tablefile.rdstate() == istream::failbit || tablefile.rdstate() == istream::badbit) {
      throw ConversionException("Error reading file " + tablePath,"Receiver");
    }
    
    // And close.
    tablefile.close();
    if (tablefile.rdstate() == istream::failbit)
      throw ConversionException("Could not close file " + tablePath,"Receiver");
    
    setFromMIME(ss.str());
  }	

	
void ReceiverTable::setFromXMLFile(const string& directory) {
    string tablePath ;
    
    tablePath = directory + "/Receiver.xml";
    ifstream tablefile(tablePath.c_str(), ios::in|ios::binary);
    if (!tablefile.is_open()) { 
      throw ConversionException("Could not open file " + tablePath, "Receiver");
    }
      // Read in a stringstream.
    stringstream ss;
    ss << tablefile.rdbuf();
    
    if  (tablefile.rdstate() == istream::failbit || tablefile.rdstate() == istream::badbit) {
      throw ConversionException("Error reading file '" + tablePath + "'", "Receiver");
    }
    
    // And close
    tablefile.close();
    if (tablefile.rdstate() == istream::failbit)
      throw ConversionException("Could not close file '" + tablePath + "'", "Receiver");

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
	 * Insert a ReceiverRow* in a vector of ReceiverRow* so that it's ordered by ascending start time.
	 *
	 * @param ReceiverRow* x . The pointer to be inserted.
	 * @param vector <ReceiverRow*>& row. A reference to the vector where to insert x.
	 *
	 */
	 ReceiverRow* ReceiverTable::insertByStartTime(ReceiverRow* x, vector<ReceiverRow*>& row) {
				
		vector <ReceiverRow*>::iterator theIterator;
		
		ArrayTime start = x->timeInterval.getStart();

    	// Is the row vector empty ?
    	if (row.size() == 0) {
    		row.push_back(x);
    		privateRows.push_back(x);
    		x->isAdded();
    		return x;
    	}
    	
    	// Optimization for the case of insertion by ascending time.
    	ReceiverRow* last = *(row.end()-1);
        
    	if ( start > last->timeInterval.getStart() ) {
 	    	//
	    	// Modify the duration of last if and only if the start time of x
	    	// is located strictly before the end time of last.
	    	//
	  		if ( start < (last->timeInterval.getStart() + last->timeInterval.getDuration()))   		
    			last->timeInterval.setDuration(start - last->timeInterval.getStart());
    		row.push_back(x);
    		privateRows.push_back(x);
    		x->isAdded();
    		return x;
    	}
    	
    	// Optimization for the case of insertion by descending time.
    	ReceiverRow* first = *(row.begin());
        
    	if ( start < first->timeInterval.getStart() ) {
			//
	  		// Modify the duration of x if and only if the start time of first
	  		// is located strictly before the end time of x.
	  		//
	  		if ( first->timeInterval.getStart() < (start + x->timeInterval.getDuration()) )	  		
    			x->timeInterval.setDuration(first->timeInterval.getStart() - start);
    		row.insert(row.begin(), x);
    		privateRows.push_back(x);
    		x->isAdded();
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
					throw DuplicateKey("DuplicateKey exception in ", "ReceiverTable");	
			}
			else if (start == row[k1]->timeInterval.getStart()) {
				if (row[k1]->equalByRequiredValue(x))
					return row[k1];
				else
					throw DuplicateKey("DuplicateKey exception in ", "ReceiverTable");	
			}
			else {
				if (start <= row[(k0+k1)/2]->timeInterval.getStart())
					k1 = (k0 + k1) / 2;
				else
					k0 = (k0 + k1) / 2;				
			} 	
		}
	
		row[k0]->timeInterval.setDuration(start-row[k0]->timeInterval.getStart());
		x->timeInterval.setDuration(row[k0+1]->timeInterval.getStart() - start);
		row.insert(row.begin()+(k0+1), x);
		privateRows.push_back(x);
   		x->isAdded();
		return x;   
    } 
    	
	
	

	
} // End namespace asdm
 
