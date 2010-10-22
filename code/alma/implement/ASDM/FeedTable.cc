
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
 * File FeedTable.cpp
 */
#include <ConversionException.h>
#include <DuplicateKey.h>
#include <OutOfBoundsException.h>

using asdm::ConversionException;
using asdm::DuplicateKey;
using asdm::OutOfBoundsException;

#include <ASDM.h>
#include <FeedTable.h>
#include <FeedRow.h>
#include <Parser.h>

using asdm::ASDM;
using asdm::FeedTable;
using asdm::FeedRow;
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

	string FeedTable::tableName = "Feed";
	const vector<string> FeedTable::attributesNames = initAttributesNames();
		

	/**
	 * The list of field names that make up key key.
	 * (Initialization is in the constructor.)
	 */
	vector<string> FeedTable::key;

	/**
	 * Return the list of field names that make up key key
	 * as an array of strings.
	 */	
	vector<string> FeedTable::getKeyName() {
		return key;
	}


	FeedTable::FeedTable(ASDM &c) : container(c) {

	
		key.push_back("antennaId");
	
		key.push_back("spectralWindowId");
	
		key.push_back("timeInterval");
	
		key.push_back("feedId");
	


		// Define a default entity.
		entity.setEntityId(EntityId("uid://X0/X0/X0"));
		entity.setEntityIdEncrypted("na");
		entity.setEntityTypeName("FeedTable");
		entity.setEntityVersion("1");
		entity.setInstanceVersion("1");
		
		// Archive XML
		archiveAsBin = false;
		
		// File XML
		fileAsBin = false;
	}
	
/**
 * A destructor for FeedTable.
 */
	FeedTable::~FeedTable() {
		for (unsigned int i = 0; i < privateRows.size(); i++) 
			delete(privateRows.at(i));
	}

	/**
	 * Container to which this table belongs.
	 */
	ASDM &FeedTable::getContainer() const {
		return container;
	}

	/**
	 * Return the number of rows in the table.
	 */
	unsigned int FeedTable::size() {
		return privateRows.size();
	}
	
	/**
	 * Return the name of this table.
	 */
	string FeedTable::getName() const {
		return tableName;
	}
	
	/**
	 * Build the vector of attributes names.
	 */
	vector<string> FeedTable::initAttributesNames() {
		vector<string> attributesNames;

		attributesNames.push_back("antennaId");

		attributesNames.push_back("spectralWindowId");

		attributesNames.push_back("timeInterval");

		attributesNames.push_back("feedId");


		attributesNames.push_back("numReceptor");

		attributesNames.push_back("beamOffset");

		attributesNames.push_back("focusReference");

		attributesNames.push_back("polarizationTypes");

		attributesNames.push_back("polResponse");

		attributesNames.push_back("receptorAngle");

		attributesNames.push_back("receiverId");


		attributesNames.push_back("feedNum");

		attributesNames.push_back("illumOffset");

		attributesNames.push_back("position");

		attributesNames.push_back("beamId");

		return attributesNames;
	}
	
	/**
	 * Return the names of the attributes.
	 */
	const vector<string>& FeedTable::getAttributesNames() { return attributesNames; }

	/**
	 * Return this table's Entity.
	 */
	Entity FeedTable::getEntity() const {
		return entity;
	}

	/**
	 * Set this table's Entity.
	 */
	void FeedTable::setEntity(Entity e) {
		this->entity = e; 
	}
	
	//
	// ====> Row creation.
	//
	
	/**
	 * Create a new row.
	 */
	FeedRow *FeedTable::newRow() {
		return new FeedRow (*this);
	}
	

	/**
	 * Create a new row initialized to the specified values.
	 * @return a pointer on the created and initialized row.
	
 	 * @param antennaId 
	
 	 * @param spectralWindowId 
	
 	 * @param timeInterval 
	
 	 * @param numReceptor 
	
 	 * @param beamOffset 
	
 	 * @param focusReference 
	
 	 * @param polarizationTypes 
	
 	 * @param polResponse 
	
 	 * @param receptorAngle 
	
 	 * @param receiverId 
	
     */
	FeedRow* FeedTable::newRow(Tag antennaId, Tag spectralWindowId, ArrayTimeInterval timeInterval, int numReceptor, vector<vector<double > > beamOffset, vector<vector<Length > > focusReference, vector<PolarizationTypeMod::PolarizationType > polarizationTypes, vector<vector<Complex > > polResponse, vector<Angle > receptorAngle, vector<int>  receiverId){
		FeedRow *row = new FeedRow(*this);
			
		row->setAntennaId(antennaId);
			
		row->setSpectralWindowId(spectralWindowId);
			
		row->setTimeInterval(timeInterval);
			
		row->setNumReceptor(numReceptor);
			
		row->setBeamOffset(beamOffset);
			
		row->setFocusReference(focusReference);
			
		row->setPolarizationTypes(polarizationTypes);
			
		row->setPolResponse(polResponse);
			
		row->setReceptorAngle(receptorAngle);
			
		row->setReceiverId(receiverId);
	
		return row;		
	}	
	


FeedRow* FeedTable::newRow(FeedRow* row) {
	return new FeedRow(*this, *row);
}

	//
	// Append a row to its table.
	//

	
	
	
		
			

	/** 
	 * Returns a string built by concatenating the ascii representation of the
	 * parameters values suffixed with a "_" character.
	 */
	 string FeedTable::Key(Tag antennaId, Tag spectralWindowId) {
	 	ostringstream ostrstr;
	 		ostrstr  
			
				<< antennaId.toString()  << "_"
			
				<< spectralWindowId.toString()  << "_"
			
			;
		return ostrstr.str();	 	
	 }

	/**
	 * Append a row to a FeedTable which has simply 
	 * 1) an autoincrementable attribute  (feedId) 
	 * 2) a temporal attribute (timeInterval) in its key section.
	 * 3) other attributes in the key section (defining a so called context).
	 * If there is already a row in the table whose key section non including is equal to x's one and
	 * whose value section is equal to x's one then return this row, otherwise add x to the collection
	 * of rows.
	 */
	FeedRow* FeedTable::add(FeedRow* x) {
		// Get the start time of the row to be inserted.
		ArrayTime startTime = x->getTimeInterval().getStart();
		// cout << "Trying to add a new row with start time = " << startTime << endl;
		int insertionId = 0;

		 
		// Determine the entry in the context map from the appropriates attributes.
		string k = Key(
						x->getAntennaId()
					   ,
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
						
						 (context[k][i][j]->getNumReceptor() == x->getNumReceptor())
						 && 

						 (context[k][i][j]->getBeamOffset() == x->getBeamOffset())
						 && 

						 (context[k][i][j]->getFocusReference() == x->getFocusReference())
						 && 

						 (context[k][i][j]->getPolarizationTypes() == x->getPolarizationTypes())
						 && 

						 (context[k][i][j]->getPolResponse() == x->getPolResponse())
						 && 

						 (context[k][i][j]->getReceptorAngle() == x->getReceptorAngle())
						 && 

						 (context[k][i][j]->getReceiverId() == x->getReceiverId())
						
						) {
							// cout << "A row equal to x has been found, I return it " << endl;
							return context[k][i][j];
						}
						
						// Otherwise we must autoincrement feedId and
						// insert a new FeedRow with this autoincremented value.
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
		
		
			x->setFeedId(insertionId);
		
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
	FeedRow*  FeedTable::checkAndAdd(FeedRow* x) {
		ArrayTime startTime = x->getTimeInterval().getStart();		
		
		// Determine the entry in the context map from the appropriate attributes.
		string k = Key(
		                x->getAntennaId()
		               ,
		                x->getSpectralWindowId()
		               );

		// Uniqueness Rule Check
		if (context.find(k) != context.end()) {
			for (unsigned int i = 0;  i < context[k].size(); i++) 
				for (unsigned int j = 0; j < context[k][i].size(); j++)
					if (
						(context[k][i][j]->getTimeInterval().getStart().equals(startTime)) 
					
						 && (context[k][i][j]->getNumReceptor() == x->getNumReceptor())
					

						 && (context[k][i][j]->getBeamOffset() == x->getBeamOffset())
					

						 && (context[k][i][j]->getFocusReference() == x->getFocusReference())
					

						 && (context[k][i][j]->getPolarizationTypes() == x->getPolarizationTypes())
					

						 && (context[k][i][j]->getPolResponse() == x->getPolResponse())
					

						 && (context[k][i][j]->getReceptorAngle() == x->getReceptorAngle())
					

						 && (context[k][i][j]->getReceiverId() == x->getReceiverId())
					
					)
						throw UniquenessViolationException("Uniqueness violation exception in table FeedTable");			
		}


		// Good, now it's time to insert the row x, possibly triggering a DuplicateKey exception.	
		
		ID_TIME_ROWS dummyPlane;

		// Determine the integer representation of the identifier of the row (x) to be inserted. 
		int id = 
				x->getFeedId();
				
	
		if (context.find(k) != context.end()) {
			if (id >= (int) context[k].size()) 
				context[k].resize(id+1);
			else {
				// This feedId 's value has already rows for this context.
				// Check that there is not yet a row with the same time. (simply check start time)
				// If there is such a row then trigger a Duplicate Key Exception.
				for (unsigned int j = 0; j < context[k][id].size(); j++)
					if (context[k][id][j]->getTimeInterval().getStart().equals(startTime))
						throw DuplicateKey("Duplicate key exception in ", "FeedTable"); 
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
	 * @return Alls rows as an array of FeedRow
	 */
	vector<FeedRow *> FeedTable::get()  {
		return privateRows;
	/*	
		vector<FeedRow *> v;
		
		map<string, ID_TIME_ROWS >::iterator mapIter = context.begin();
		ID_TIME_ROWS::iterator planeIter;
		vector<FeedRow*>::iterator rowIter; 
		for (mapIter=context.begin(); mapIter!=context.end(); mapIter++)
			for (planeIter=((*mapIter).second).begin(); planeIter != ((*mapIter).second).end(); planeIter++)
				for (rowIter=(*planeIter).begin(); rowIter != (*planeIter).end(); rowIter++)
					v.push_back(*rowIter);
		return v;
	*/
	}	
	


	
		
		
			
			 
/*
 ** Returns a FeedRow* given a key.
 ** @return a pointer to the row having the key whose values are passed as parameters, or 0 if
 ** no row exists for that key.
 **
 */
 	FeedRow* FeedTable::getRowByKey(Tag antennaId, Tag spectralWindowId, ArrayTimeInterval timeInterval, int feedId)  {	
		ArrayTime start = timeInterval.getStart();
		
		map<string, ID_TIME_ROWS >::iterator mapIter;
		if ((mapIter = context.find(Key(antennaId, spectralWindowId))) != context.end()) {
			
			int id = feedId;
			
			if (id < (int) ((*mapIter).second).size()) {
				vector <FeedRow*>::iterator rowIter;
				for (rowIter = ((*mapIter).second)[id].begin(); rowIter != ((*mapIter).second)[id].end(); rowIter++) {
					if ((*rowIter)->getTimeInterval().contains(timeInterval))
						return *rowIter; 
				}
			}
		}
		return 0;
	}
/*
 * Returns a vector of pointers on rows whose key element feedId 
 * is equal to the parameter feedId.
 * @return a vector of vector <FeedRow *>. A returned vector of size 0 means that no row has been found.
 * @param feedId int contains the value of
 * the autoincrementable attribute that is looked up in the table.
 */
 vector <FeedRow *>  FeedTable::getRowByFeedId(int feedId) {
	vector<FeedRow *> list;
	map<string, ID_TIME_ROWS >::iterator mapIter;
	
	for (mapIter=context.begin(); mapIter!=context.end(); mapIter++) {
		int maxId = ((*mapIter).second).size();
		if (feedId < maxId) {
			vector<FeedRow *>::iterator rowIter;
			for (rowIter=((*mapIter).second)[feedId].begin(); 
			     rowIter!=((*mapIter).second)[feedId].end(); rowIter++)
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
			
 * @param <<ExtrinsicAttribute>> antennaId.
 	 		
 * @param <<ExtrinsicAttribute>> spectralWindowId.
 	 		
 * @param <<ASDMAttribute>> timeInterval.
 	 		
 * @param <<ASDMAttribute>> numReceptor.
 	 		
 * @param <<ArrayAttribute>> beamOffset.
 	 		
 * @param <<ArrayAttribute>> focusReference.
 	 		
 * @param <<ArrayAttribute>> polarizationTypes.
 	 		
 * @param <<ArrayAttribute>> polResponse.
 	 		
 * @param <<ArrayAttribute>> receptorAngle.
 	 		
 * @param <<ExtrinsicAttribute>> receiverId.
 	 		 
 */
FeedRow* FeedTable::lookup(Tag antennaId, Tag spectralWindowId, ArrayTimeInterval timeInterval, int numReceptor, vector<vector<double > > beamOffset, vector<vector<Length > > focusReference, vector<PolarizationTypeMod::PolarizationType > polarizationTypes, vector<vector<Complex > > polResponse, vector<Angle > receptorAngle, vector<int>  receiverId) {		
		using asdm::ArrayTimeInterval;
		map<string, ID_TIME_ROWS >::iterator mapIter;
		string k = Key(antennaId, spectralWindowId);
		if ((mapIter = context.find(k)) != context.end()) {
			ID_TIME_ROWS::iterator planeIter;
			for (planeIter = context[k].begin(); planeIter != context[k].end(); planeIter++)  {
				vector <FeedRow*>::iterator rowIter;
				for (rowIter = (*planeIter).begin(); rowIter != (*planeIter).end(); rowIter++) {
					if ((*rowIter)->getTimeInterval().contains(timeInterval)
					    && (*rowIter)->compareRequiredValue(numReceptor, beamOffset, focusReference, polarizationTypes, polResponse, receptorAngle, receiverId)) {
						return *rowIter;
					} 
				}
			}
		}				
		return 0;	
} 
		
	




#ifndef WITHOUT_ACS
	// Conversion Methods

	FeedTableIDL *FeedTable::toIDL() {
		FeedTableIDL *x = new FeedTableIDL ();
		unsigned int nrow = size();
		x->row.length(nrow);
		vector<FeedRow*> v = get();
		for (unsigned int i = 0; i < nrow; ++i) {
			x->row[i] = *(v[i]->toIDL());
		}
		return x;
	}
#endif
	
#ifndef WITHOUT_ACS
	void FeedTable::fromIDL(FeedTableIDL x) {
		unsigned int nrow = x.row.length();
		for (unsigned int i = 0; i < nrow; ++i) {
			FeedRow *tmp = newRow();
			tmp->setFromIDL(x.row[i]);
			// checkAndAdd(tmp);
			add(tmp);
		}
	}
#endif

	
	string FeedTable::toXML()  {
		string buf;

		buf.append("<?xml version=\"1.0\" encoding=\"ISO-8859-1\"?> ");
		buf.append("<FeedTable xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" xmlns:feed=\"http://Alma/XASDM/FeedTable\" xsi:schemaLocation=\"http://Alma/XASDM/FeedTable http://almaobservatory.org/XML/XASDM/2/FeedTable.xsd\" schemaVersion=\"2\" schemaRevision=\"1.55\">\n");
	
		buf.append(entity.toXML());
		string s = container.getEntity().toXML();
		// Change the "Entity" tag to "ContainerEntity".
		buf.append("<Container" + s.substr(1,s.length() - 1)+" ");
		vector<FeedRow*> v = get();
		for (unsigned int i = 0; i < v.size(); ++i) {
			try {
				buf.append(v[i]->toXML());
			} catch (NoSuchRow e) {
			}
			buf.append("  ");
		}		
		buf.append("</FeedTable> ");
		return buf;
	}

	
	void FeedTable::fromXML(string xmlDoc)  {
		Parser xml(xmlDoc);
		if (!xml.isStr("<FeedTable")) 
			error();
		// cout << "Parsing a FeedTable" << endl;
		string s = xml.getElement("<Entity","/>");
		if (s.length() == 0) 
			error();
		Entity e;
		e.setFromXML(s);
		if (e.getEntityTypeName() != "FeedTable")
			error();
		setEntity(e);
		// Skip the container's entity; but, it has to be there.
		s = xml.getElement("<ContainerEntity","/>");
		if (s.length() == 0) 
			error();

		// Get each row in the table.
		s = xml.getElementContent("<row>","</row>");
		FeedRow *row;
		while (s.length() != 0) {
			// cout << "Parsing a FeedRow" << endl; 
			row = newRow();
			row->setFromXML(s);
			try {
				checkAndAdd(row);
			} catch (DuplicateKey e1) {
				throw ConversionException(e1.getMessage(),"FeedTable");
			} 
			catch (UniquenessViolationException e1) {
				throw ConversionException(e1.getMessage(),"FeedTable");	
			}
			catch (...) {
				// cout << "Unexpected error in FeedTable::checkAndAdd called from FeedTable::fromXML " << endl;
			}
			s = xml.getElementContent("<row>","</row>");
		}
		if (!xml.isStr("</FeedTable>")) 
			error();
			
		archiveAsBin = false;
		fileAsBin = false;
		
	}

	
	void FeedTable::error()  {
		throw ConversionException("Invalid xml document","Feed");
	}
	
	
	string FeedTable::MIMEXMLPart(const asdm::ByteOrder* byteOrder) {
		string UID = getEntity().getEntityId().toString();
		string withoutUID = UID.substr(6);
		string containerUID = getContainer().getEntity().getEntityId().toString();
		ostringstream oss;
		oss << "<?xml version='1.0'  encoding='ISO-8859-1'?>";
		oss << "\n";
		oss << "<FeedTable xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" xmlns:feed=\"http://Alma/XASDM/FeedTable\" xsi:schemaLocation=\"http://Alma/XASDM/FeedTable http://almaobservatory.org/XML/XASDM/2/FeedTable.xsd\" schemaVersion=\"2\" schemaRevision=\"1.55\">\n";
		oss<< "<Entity entityId='"<<UID<<"' entityIdEncrypted='na' entityTypeName='FeedTable' schemaVersion='1' documentVersion='1'/>\n";
		oss<< "<ContainerEntity entityId='"<<containerUID<<"' entityIdEncrypted='na' entityTypeName='ASDM' schemaVersion='1' documentVersion='1'/>\n";
		oss << "<BulkStoreRef file_id='"<<withoutUID<<"' byteOrder='"<<byteOrder->toString()<<"' />\n";
		oss << "<Attributes>\n";

		oss << "<antennaId/>\n"; 
		oss << "<spectralWindowId/>\n"; 
		oss << "<timeInterval/>\n"; 
		oss << "<feedId/>\n"; 
		oss << "<numReceptor/>\n"; 
		oss << "<beamOffset/>\n"; 
		oss << "<focusReference/>\n"; 
		oss << "<polarizationTypes/>\n"; 
		oss << "<polResponse/>\n"; 
		oss << "<receptorAngle/>\n"; 
		oss << "<receiverId/>\n"; 

		oss << "<feedNum/>\n"; 
		oss << "<illumOffset/>\n"; 
		oss << "<position/>\n"; 
		oss << "<beamId/>\n"; 
		oss << "</Attributes>\n";		
		oss << "</FeedTable>\n";

		return oss.str();				
	}
	
	string FeedTable::toMIME(const asdm::ByteOrder* byteOrder) {
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

	
	void FeedTable::setFromMIME(const string & mimeMsg) {
    string xmlPartMIMEHeader = "Content-ID: <header.xml>\n\n";
    
    string binPartMIMEHeader = "--MIME_boundary\nContent-Type: binary/octet-stream\nContent-ID: <content.bin>\n\n";
    
    // Detect the XML header.
    string::size_type loc0 = mimeMsg.find(xmlPartMIMEHeader, 0);
    if ( loc0 == string::npos) {
      throw ConversionException("Failed to detect the beginning of the XML header", "Feed");
    }
    loc0 += xmlPartMIMEHeader.size();
    
    // Look for the string announcing the binary part.
    string::size_type loc1 = mimeMsg.find( binPartMIMEHeader, loc0 );
    
    if ( loc1 == string::npos ) {
      throw ConversionException("Failed to detect the beginning of the binary part", "Feed");
    }
    
    //
    // Extract the xmlHeader and analyze it to find out what is the byte order and the sequence
    // of attribute names.
    //
    string xmlHeader = mimeMsg.substr(loc0, loc1-loc0);
    xmlDoc *doc;
    doc = xmlReadMemory(xmlHeader.data(), xmlHeader.size(), "BinaryTableHeader.xml", NULL, XML_PARSE_NOBLANKS);
    if ( doc == NULL ) 
      throw ConversionException("Failed to parse the xmlHeader into a DOM structure.", "Feed");
    
   // This vector will be filled by the names of  all the attributes of the table
   // in the order in which they are expected to be found in the binary representation.
   //
    vector<string> attributesSeq;
      
    xmlNode* root_element = xmlDocGetRootElement(doc);
    if ( root_element == NULL || root_element->type != XML_ELEMENT_NODE )
      throw ConversionException("Failed to parse the xmlHeader into a DOM structure.", "Feed");
    
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
     
    attributesSeq.push_back("numReceptor") ; 
     
    attributesSeq.push_back("beamOffset") ; 
     
    attributesSeq.push_back("focusReference") ; 
     
    attributesSeq.push_back("polarizationTypes") ; 
     
    attributesSeq.push_back("polResponse") ; 
     
    attributesSeq.push_back("receptorAngle") ; 
     
    attributesSeq.push_back("receiverId") ; 
    
     
    attributesSeq.push_back("feedNum") ; 
     
    attributesSeq.push_back("illumOffset") ; 
     
    attributesSeq.push_back("position") ; 
     
    attributesSeq.push_back("beamId") ; 
              
     }
    else if (string("FeedTable").compare((const char*) root_element->name) == 0) {
      // It's a new (and correct) MIME file for tables.
      //
      // 1st )  Look for a BulkStoreRef element with an attribute byteOrder.
      //
      xmlNode* bulkStoreRef = 0;
      xmlNode* child = root_element->children;
      
      // Skip the two first children (Entity and ContainerEntity).
      bulkStoreRef = (child ==  0) ? 0 : ( (child->next) == 0 ? 0 : child->next->next );
      
      if ( bulkStoreRef == 0 || (bulkStoreRef->type != XML_ELEMENT_NODE)  || (string("BulkStoreRef").compare((const char*) bulkStoreRef->name) != 0))
      	throw ConversionException ("Could not find the element '/FeedTable/BulkStoreRef'. Invalid XML header '"+ xmlHeader + "'.", "Feed");
      	
      // We found BulkStoreRef, now look for its attribute byteOrder.
      _xmlAttr* byteOrderAttr = 0;
      for (struct _xmlAttr* attr = bulkStoreRef->properties; attr; attr = attr->next) 
	  if (string("byteOrder").compare((const char*) attr->name) == 0) {
	   byteOrderAttr = attr;
	   break;
	 }
      
      if (byteOrderAttr == 0) 
	     throw ConversionException("Could not find the element '/FeedTable/BulkStoreRef/@byteOrder'. Invalid XML header '" + xmlHeader +"'.", "Feed");
      
      string byteOrderValue = string((const char*) byteOrderAttr->children->content);
      if (!(byteOrder = asdm::ByteOrder::fromString(byteOrderValue)))
		throw ConversionException("No valid value retrieved for the element '/FeedTable/BulkStoreRef/@byteOrder'. Invalid XML header '" + xmlHeader + "'.", "Feed");
		
	 //
	 // 2nd) Look for the Attributes element and grab the names of the elements it contains.
	 //
	 xmlNode* attributes = bulkStoreRef->next;
     if ( attributes == 0 || (attributes->type != XML_ELEMENT_NODE)  || (string("Attributes").compare((const char*) attributes->name) != 0))	 
       	throw ConversionException ("Could not find the element '/FeedTable/Attributes'. Invalid XML header '"+ xmlHeader + "'.", "Feed");
 
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
	FeedRow* aRow = FeedRow::fromBin(eiss, *this, attributesSeq);
	checkAndAdd(aRow);
      }
    }
    catch (DuplicateKey e) {
      throw ConversionException("Error while writing binary data , the message was "
				+ e.getMessage(), "Feed");
    }
    catch (TagFormatException e) {
      throw ConversionException("Error while reading binary data , the message was "
				+ e.getMessage(), "Feed");
    }
    archiveAsBin = true;
    fileAsBin = true;
	}

	
	void FeedTable::toFile(string directory) {
		if (!directoryExists(directory.c_str()) &&
			!createPath(directory.c_str())) {
			throw ConversionException("Could not create directory " , directory);
		}

		string fileName = directory + "/Feed.xml";
		ofstream tableout(fileName.c_str(),ios::out|ios::trunc);
		if (tableout.rdstate() == ostream::failbit)
			throw ConversionException("Could not open file " + fileName + " to write ", "Feed");
		if (fileAsBin) 
			tableout << MIMEXMLPart();
		else
			tableout << toXML() << endl;
		tableout.close();
		if (tableout.rdstate() == ostream::failbit)
			throw ConversionException("Could not close file " + fileName, "Feed");

		if (fileAsBin) {
			// write the bin serialized
			string fileName = directory + "/Feed.bin";
			ofstream tableout(fileName.c_str(),ios::out|ios::trunc);
			if (tableout.rdstate() == ostream::failbit)
				throw ConversionException("Could not open file " + fileName + " to write ", "Feed");
			tableout << toMIME() << endl;
			tableout.close();
			if (tableout.rdstate() == ostream::failbit)
				throw ConversionException("Could not close file " + fileName, "Feed");
		}
	}

	
	void FeedTable::setFromFile(const string& directory) {		
    if (boost::filesystem::exists(boost::filesystem::path(uniqSlashes(directory + "/Feed.xml"))))
      setFromXMLFile(directory);
    else if (boost::filesystem::exists(boost::filesystem::path(uniqSlashes(directory + "/Feed.bin"))))
      setFromMIMEFile(directory);
    else
      throw ConversionException("No file found for the Feed table", "Feed");
	}			

	
  void FeedTable::setFromMIMEFile(const string& directory) {
    string tablePath ;
    
    tablePath = directory + "/Feed.bin";
    ifstream tablefile(tablePath.c_str(), ios::in|ios::binary);
    if (!tablefile.is_open()) { 
      throw ConversionException("Could not open file " + tablePath, "Feed");
    }
    // Read in a stringstream.
    stringstream ss; ss << tablefile.rdbuf();
    
    if (tablefile.rdstate() == istream::failbit || tablefile.rdstate() == istream::badbit) {
      throw ConversionException("Error reading file " + tablePath,"Feed");
    }
    
    // And close.
    tablefile.close();
    if (tablefile.rdstate() == istream::failbit)
      throw ConversionException("Could not close file " + tablePath,"Feed");
    
    setFromMIME(ss.str());
  }	

	
void FeedTable::setFromXMLFile(const string& directory) {
    string tablePath ;
    
    tablePath = directory + "/Feed.xml";
    ifstream tablefile(tablePath.c_str(), ios::in|ios::binary);
    if (!tablefile.is_open()) { 
      throw ConversionException("Could not open file " + tablePath, "Feed");
    }
      // Read in a stringstream.
    stringstream ss;
    ss << tablefile.rdbuf();
    
    if  (tablefile.rdstate() == istream::failbit || tablefile.rdstate() == istream::badbit) {
      throw ConversionException("Error reading file '" + tablePath + "'", "Feed");
    }
    
    // And close
    tablefile.close();
    if (tablefile.rdstate() == istream::failbit)
      throw ConversionException("Could not close file '" + tablePath + "'", "Feed");

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
	 * Insert a FeedRow* in a vector of FeedRow* so that it's ordered by ascending start time.
	 *
	 * @param FeedRow* x . The pointer to be inserted.
	 * @param vector <FeedRow*>& row. A reference to the vector where to insert x.
	 *
	 */
	 FeedRow* FeedTable::insertByStartTime(FeedRow* x, vector<FeedRow*>& row) {
				
		vector <FeedRow*>::iterator theIterator;
		
		ArrayTime start = x->timeInterval.getStart();

    	// Is the row vector empty ?
    	if (row.size() == 0) {
    		row.push_back(x);
    		privateRows.push_back(x);
    		x->isAdded(true);
    		return x;
    	}
    	
    	// Optimization for the case of insertion by ascending time.
    	FeedRow* last = *(row.end()-1);
        
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
    	FeedRow* first = *(row.begin());
        
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
					throw DuplicateKey("DuplicateKey exception in ", "FeedTable");	
			}
			else if (start == row[k1]->timeInterval.getStart()) {
				if (row[k1]->equalByRequiredValue(x))
					return row[k1];
				else
					throw DuplicateKey("DuplicateKey exception in ", "FeedTable");	
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
				throw DuplicateKey("DuplicateKey exception in ", "FeedTable");	
		}
		else if (start == row[k1]->timeInterval.getStart()) {
			if (row[k1]->equalByRequiredValue(x))
				return row[k1];
			else
				throw DuplicateKey("DuplicateKey exception in ", "FeedTable");	
		}	

		row[k0]->timeInterval.setDuration(start-row[k0]->timeInterval.getStart());
		x->timeInterval.setDuration(row[k0+1]->timeInterval.getStart() - start);
		row.insert(row.begin()+(k0+1), x);
		privateRows.push_back(x);
   		x->isAdded(true);
		return x;   
    } 
    	
	
	

	
} // End namespace asdm
 
