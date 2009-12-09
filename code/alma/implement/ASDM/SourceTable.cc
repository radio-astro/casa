
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
 * File SourceTable.cpp
 */
#include <ConversionException.h>
#include <DuplicateKey.h>
#include <OutOfBoundsException.h>

using asdm::ConversionException;
using asdm::DuplicateKey;
using asdm::OutOfBoundsException;

#include <ASDM.h>
#include <SourceTable.h>
#include <SourceRow.h>
#include <Parser.h>

using asdm::ASDM;
using asdm::SourceTable;
using asdm::SourceRow;
using asdm::Parser;

#include <iostream>
#include <sstream>
#include <set>
using namespace std;

#include <Misc.h>
using namespace asdm;


namespace asdm {

	string SourceTable::tableName = "Source";
	

	/**
	 * The list of field names that make up key key.
	 * (Initialization is in the constructor.)
	 */
	vector<string> SourceTable::key;

	/**
	 * Return the list of field names that make up key key
	 * as an array of strings.
	 */	
	vector<string> SourceTable::getKeyName() {
		return key;
	}


	SourceTable::SourceTable(ASDM &c) : container(c) {

	
		key.push_back("sourceId");
	
		key.push_back("timeInterval");
	
		key.push_back("spectralWindowId");
	


		// Define a default entity.
		entity.setEntityId(EntityId("uid://X0/X0/X0"));
		entity.setEntityIdEncrypted("na");
		entity.setEntityTypeName("SourceTable");
		entity.setEntityVersion("1");
		entity.setInstanceVersion("1");
		
		// Archive XML
		archiveAsBin = false;
		
		// File XML
		fileAsBin = false;
	}
	
/**
 * A destructor for SourceTable.
 */
 
	SourceTable::~SourceTable() {
		for (unsigned int i = 0; i < privateRows.size(); i++) 
			delete(privateRows.at(i));
	}

	/**
	 * Container to which this table belongs.
	 */
	ASDM &SourceTable::getContainer() const {
		return container;
	}

	/**
	 * Return the number of rows in the table.
	 */
	
	
	unsigned SourceTable::size() {
/*
		int result = 0	;
		map<string, ID_TIME_ROWS >::iterator mapIter = context.begin();
		ID_TIME_ROWS::iterator planeIter;
		vector<SourceRow*>::iterator rowIter; 
		for (mapIter=context.begin(); mapIter!=context.end(); mapIter++)
			for (planeIter=((*mapIter).second).begin(); planeIter != ((*mapIter).second).end(); planeIter++)
				result += (*planeIter).size();
		return result;
*/
	   return privateRows.size();
	}
	
	
	
	/**
	 * Return the name of this table.
	 */
	string SourceTable::getName() const {
		return tableName;
	}

	/**
	 * Return this table's Entity.
	 */
	Entity SourceTable::getEntity() const {
		return entity;
	}

	/**
	 * Set this table's Entity.
	 */
	void SourceTable::setEntity(Entity e) {
		this->entity = e; 
	}
	
	//
	// ====> Row creation.
	//
	
	/**
	 * Create a new row.
	 */
	SourceRow *SourceTable::newRow() {
		return new SourceRow (*this);
	}
	
	SourceRow *SourceTable::newRowEmpty() {
		return newRow ();
	}


	/**
	 * Create a new row initialized to the specified values.
	 * @return a pointer on the created and initialized row.
	
 	 * @param timeInterval. 
	
 	 * @param spectralWindowId. 
	
 	 * @param code. 
	
 	 * @param direction. 
	
 	 * @param properMotion. 
	
 	 * @param sourceName. 
	
     */
	SourceRow* SourceTable::newRow(ArrayTimeInterval timeInterval, Tag spectralWindowId, string code, vector<Angle > direction, vector<AngularRate > properMotion, string sourceName){
		SourceRow *row = new SourceRow(*this);
			
		row->setTimeInterval(timeInterval);
			
		row->setSpectralWindowId(spectralWindowId);
			
		row->setCode(code);
			
		row->setDirection(direction);
			
		row->setProperMotion(properMotion);
			
		row->setSourceName(sourceName);
	
		return row;		
	}	

	SourceRow* SourceTable::newRowFull(ArrayTimeInterval timeInterval, Tag spectralWindowId, string code, vector<Angle > direction, vector<AngularRate > properMotion, string sourceName)	{
		SourceRow *row = new SourceRow(*this);
			
		row->setTimeInterval(timeInterval);
			
		row->setSpectralWindowId(spectralWindowId);
			
		row->setCode(code);
			
		row->setDirection(direction);
			
		row->setProperMotion(properMotion);
			
		row->setSourceName(sourceName);
	
		return row;				
	}
	


SourceRow* SourceTable::newRow(SourceRow* row) {
	return new SourceRow(*this, *row);
}

SourceRow* SourceTable::newRowCopy(SourceRow* row) {
	return new SourceRow(*this, *row);
}

	//
	// Append a row to its table.
	//

	
	
	
		
			

	/** 
	 * Returns a string built by concatenating the ascii representation of the
	 * parameters values suffixed with a "_" character.
	 */
	 string SourceTable::Key(Tag spectralWindowId) {
	 	ostringstream ostrstr;
	 		ostrstr  
			
				<< spectralWindowId.toString()  << "_"
			
			;
		return ostrstr.str();	 	
	 }

	/**
	 * Append a row to a SourceTable which has simply 
	 * 1) an autoincrementable attribute  (sourceId) 
	 * 2) a temporal attribute (timeInterval) in its key section.
	 * 3) other attributes in the key section (defining a so called context).
	 * If there is already a row in the table whose key section non including is equal to x's one and
	 * whose value section is equal to x's one then return this row, otherwise add x to the collection
	 * of rows.
	 */
	SourceRow* SourceTable::add(SourceRow* x) {
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
						
						 (context[k][i][j]->getCode() == x->getCode())
						 && 

						 (context[k][i][j]->getDirection() == x->getDirection())
						 && 

						 (context[k][i][j]->getProperMotion() == x->getProperMotion())
						 && 

						 (context[k][i][j]->getSourceName() == x->getSourceName())
						
						) {
							// cout << "A row equal to x has been found, I return it " << endl;
							return context[k][i][j];
						}
						
						// Otherwise we must autoincrement sourceId and
						// insert a new SourceRow with this autoincremented value.
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
		
		
			x->setSourceId(insertionId);
		
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
	SourceRow*  SourceTable::checkAndAdd(SourceRow* x) {
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
					
						 && (context[k][i][j]->getCode() == x->getCode())
					

						 && (context[k][i][j]->getDirection() == x->getDirection())
					

						 && (context[k][i][j]->getProperMotion() == x->getProperMotion())
					

						 && (context[k][i][j]->getSourceName() == x->getSourceName())
					
					)
						throw UniquenessViolationException("Uniqueness violation exception in table SourceTable");			
		}


		// Good, now it's time to insert the row x, possibly triggering a DuplicateKey exception.	
		
		ID_TIME_ROWS dummyPlane;

		// Determine the integer representation of the identifier of the row (x) to be inserted. 
		int id = 
				x->getSourceId();
				
	
		if (context.find(k) != context.end()) {
			if (id >= (int) context[k].size()) 
				context[k].resize(id+1);
			else {
				// This sourceId 's value has already rows for this context.
				// Check that there is not yet a row with the same time. (simply check start time)
				// If there is such a row then trigger a Duplicate Key Exception.
				for (unsigned int j = 0; j < context[k][id].size(); j++)
					if (context[k][id][j]->getTimeInterval().getStart().equals(startTime))
						throw DuplicateKey("Duplicate key exception in ", "SourceTable"); 
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
	 * @return Alls rows as an array of SourceRow
	 */
	vector<SourceRow *> SourceTable::get()  {
		return privateRows;
	/*	
		vector<SourceRow *> v;
		
		map<string, ID_TIME_ROWS >::iterator mapIter = context.begin();
		ID_TIME_ROWS::iterator planeIter;
		vector<SourceRow*>::iterator rowIter; 
		for (mapIter=context.begin(); mapIter!=context.end(); mapIter++)
			for (planeIter=((*mapIter).second).begin(); planeIter != ((*mapIter).second).end(); planeIter++)
				for (rowIter=(*planeIter).begin(); rowIter != (*planeIter).end(); rowIter++)
					v.push_back(*rowIter);
		return v;
	*/
	}	
	


	
		
		
			
			 
/*
 ** Returns a SourceRow* given a key.
 ** @return a pointer to the row having the key whose values are passed as parameters, or 0 if
 ** no row exists for that key.
 **
 */
 	SourceRow* SourceTable::getRowByKey(int sourceId, ArrayTimeInterval timeInterval, Tag spectralWindowId)  {	
		ArrayTime start = timeInterval.getStart();
		
		map<string, ID_TIME_ROWS >::iterator mapIter;
		if ((mapIter = context.find(Key(spectralWindowId))) != context.end()) {
			
			int id = sourceId;
			
			if (id < (int) ((*mapIter).second).size()) {
				vector <SourceRow*>::iterator rowIter;
				for (rowIter = ((*mapIter).second)[id].begin(); rowIter != ((*mapIter).second)[id].end(); rowIter++) {
					if ((*rowIter)->getTimeInterval().contains(timeInterval))
						return *rowIter; 
				}
			}
		}
		return 0;
	}
/*
 * Returns a vector of pointers on rows whose key element sourceId 
 * is equal to the parameter sourceId.
 * @return a vector of vector <SourceRow *>. A returned vector of size 0 means that no row has been found.
 * @param sourceId int contains the value of
 * the autoincrementable attribute that is looked up in the table.
 */
 vector <SourceRow *>  SourceTable::getRowBySourceId(int sourceId) {
	vector<SourceRow *> list;
	map<string, ID_TIME_ROWS >::iterator mapIter;
	
	for (mapIter=context.begin(); mapIter!=context.end(); mapIter++) {
		int maxId = ((*mapIter).second).size();
		if (sourceId < maxId) {
			vector<SourceRow *>::iterator rowIter;
			for (rowIter=((*mapIter).second)[sourceId].begin(); 
			     rowIter!=((*mapIter).second)[sourceId].end(); rowIter++)
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
			
 * @param <<ASDMAttribute>> timeInterval.
 	 		
 * @param <<ExtrinsicAttribute>> spectralWindowId.
 	 		
 * @param <<ASDMAttribute>> code.
 	 		
 * @param <<ArrayAttribute>> direction.
 	 		
 * @param <<ArrayAttribute>> properMotion.
 	 		
 * @param <<ASDMAttribute>> sourceName.
 	 		 
 */
SourceRow* SourceTable::lookup(ArrayTimeInterval timeInterval, Tag spectralWindowId, string code, vector<Angle > direction, vector<AngularRate > properMotion, string sourceName) {		
		using asdm::ArrayTimeInterval;
		map<string, ID_TIME_ROWS >::iterator mapIter;
		string k = Key(spectralWindowId);
		if ((mapIter = context.find(k)) != context.end()) {
			ID_TIME_ROWS::iterator planeIter;
			for (planeIter = context[k].begin(); planeIter != context[k].end(); planeIter++)  {
				vector <SourceRow*>::iterator rowIter;
				for (rowIter = (*planeIter).begin(); rowIter != (*planeIter).end(); rowIter++) {
					if ((*rowIter)->getTimeInterval().contains(timeInterval)
					    && (*rowIter)->compareRequiredValue(code, direction, properMotion, sourceName)) {
						return *rowIter;
					} 
				}
			}
		}				
		return 0;	
} 
		
	




#ifndef WITHOUT_ACS
	// Conversion Methods

	SourceTableIDL *SourceTable::toIDL() {
		SourceTableIDL *x = new SourceTableIDL ();
		unsigned int nrow = size();
		x->row.length(nrow);
		vector<SourceRow*> v = get();
		for (unsigned int i = 0; i < nrow; ++i) {
			x->row[i] = *(v[i]->toIDL());
		}
		return x;
	}
#endif
	
#ifndef WITHOUT_ACS
	void SourceTable::fromIDL(SourceTableIDL x) {
		unsigned int nrow = x.row.length();
		for (unsigned int i = 0; i < nrow; ++i) {
			SourceRow *tmp = newRow();
			tmp->setFromIDL(x.row[i]);
			// checkAndAdd(tmp);
			add(tmp);
		}
	}
#endif

	char *SourceTable::toFITS() const  {
		throw ConversionException("Not implemented","Source");
	}

	void SourceTable::fromFITS(char *fits)  {
		throw ConversionException("Not implemented","Source");
	}

	string SourceTable::toVOTable() const {
		throw ConversionException("Not implemented","Source");
	}

	void SourceTable::fromVOTable(string vo) {
		throw ConversionException("Not implemented","Source");
	}

	
	string SourceTable::toXML()  {
		string buf;
		buf.append("<?xml version=\"1.0\" encoding=\"ISO-8859-1\"?> ");
		buf.append("<SourceTable xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" xmlns=\"http://Alma/XASDM/SourceTable\" xsi:schemaLocation=\"http://Alma/XASDM/SourceTable http://almaobservatory.org/XML/XASDM/2/SourceTable.xsd\"> ");	
		buf.append(entity.toXML());
		string s = container.getEntity().toXML();
		// Change the "Entity" tag to "ContainerEntity".
		buf.append("<Container" + s.substr(1,s.length() - 1)+" ");
		vector<SourceRow*> v = get();
		for (unsigned int i = 0; i < v.size(); ++i) {
			try {
				buf.append(v[i]->toXML());
			} catch (NoSuchRow e) {
			}
			buf.append("  ");
		}		
		buf.append("</SourceTable> ");
		return buf;
	}

	
	void SourceTable::fromXML(string xmlDoc)  {
		Parser xml(xmlDoc);
		if (!xml.isStr("<SourceTable")) 
			error();
		// cout << "Parsing a SourceTable" << endl;
		string s = xml.getElement("<Entity","/>");
		if (s.length() == 0) 
			error();
		Entity e;
		e.setFromXML(s);
		if (e.getEntityTypeName() != "SourceTable")
			error();
		setEntity(e);
		// Skip the container's entity; but, it has to be there.
		s = xml.getElement("<ContainerEntity","/>");
		if (s.length() == 0) 
			error();

		// Get each row in the table.
		s = xml.getElementContent("<row>","</row>");
		SourceRow *row;
		while (s.length() != 0) {
			// cout << "Parsing a SourceRow" << endl; 
			row = newRow();
			row->setFromXML(s);
			try {
				checkAndAdd(row);
			} catch (DuplicateKey e1) {
				throw ConversionException(e1.getMessage(),"SourceTable");
			} 
			catch (UniquenessViolationException e1) {
				throw ConversionException(e1.getMessage(),"SourceTable");	
			}
			catch (...) {
				// cout << "Unexpected error in SourceTable::checkAndAdd called from SourceTable::fromXML " << endl;
			}
			s = xml.getElementContent("<row>","</row>");
		}
		if (!xml.isStr("</SourceTable>")) 
			error();
	}

	
	void SourceTable::error()  {
		throw ConversionException("Invalid xml document","Source");
	}
	
	
	string SourceTable::toMIME() {
		EndianOSStream eoss;
		
		string UID = getEntity().getEntityId().toString();
		string execBlockUID = getContainer().getEntity().getEntityId().toString();
		
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
		eoss << "<?xml version='1.0'  encoding='ISO-8859-1'?>";
		eoss << "\n";
		eoss<< "<ASDMBinaryTable  xmlns:xsi='http://www.w3.org/2001/XMLSchema-instance'  xsi:noNamespaceSchemaLocation='ASDMBinaryTable.xsd' ID='None'  version='1.0'>\n";
		eoss << "<ExecBlockUID>\n";
		eoss << execBlockUID  << "\n";
		eoss << "</ExecBlockUID>\n";
		eoss << "</ASDMBinaryTable>\n";		

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

	
	void SourceTable::setFromMIME(const string & mimeMsg) {
		// cout << "Entering setFromMIME" << endl;
	 	string terminator = "Content-Type: binary/octet-stream\nContent-ID: <content.bin>\n\n";
	 	
	 	// Look for the string announcing the binary part.
	 	string::size_type loc = mimeMsg.find( terminator, 0 );
	 	
	 	if ( loc == string::npos ) {
	 		throw ConversionException("Failed to detect the beginning of the binary part", "Source");
	 	}
	
	 	// Create an EndianISStream from the substring containing the binary part.
	 	EndianISStream eiss(mimeMsg.substr(loc+terminator.size()));
	 	
	 	entity = Entity::fromBin(eiss);
	 	
	 	// We do nothing with that but we have to read it.
	 	Entity containerEntity = Entity::fromBin(eiss);
	 		 	
	 	int numRows = eiss.readInt();
	 	try {
	 		for (int i = 0; i < numRows; i++) {
	 			SourceRow* aRow = SourceRow::fromBin(eiss, *this);
	 			checkAndAdd(aRow);
	 		}
	 	}
	 	catch (DuplicateKey e) {
	 		throw ConversionException("Error while writing binary data , the message was "
	 					+ e.getMessage(), "Source");
	 	}
		catch (TagFormatException e) {
			throw ConversionException("Error while reading binary data , the message was "
					+ e.getMessage(), "Source");
		} 		 	
	}

	
	void SourceTable::toFile(string directory) {
		if (!directoryExists(directory.c_str()) &&
			!createPath(directory.c_str())) {
			throw ConversionException("Could not create directory " , directory);
		}
		
		if (fileAsBin) {
			// write the bin serialized
			string fileName = directory + "/Source.bin";
			ofstream tableout(fileName.c_str(),ios::out|ios::trunc);
			if (tableout.rdstate() == ostream::failbit)
				throw ConversionException("Could not open file " + fileName + " to write ", "Source");
			tableout << toMIME() << endl;
			tableout.close();
			if (tableout.rdstate() == ostream::failbit)
				throw ConversionException("Could not close file " + fileName, "Source");
		}
		else {
			// write the XML
			string fileName = directory + "/Source.xml";
			ofstream tableout(fileName.c_str(),ios::out|ios::trunc);
			if (tableout.rdstate() == ostream::failbit)
				throw ConversionException("Could not open file " + fileName + " to write ", "Source");
			tableout << toXML() << endl;
			tableout.close();
			if (tableout.rdstate() == ostream::failbit)
				throw ConversionException("Could not close file " + fileName, "Source");
		}
	}

	
	void SourceTable::setFromFile(const string& directory) {
		string tablename;
		if (fileAsBin)
			tablename = directory + "/Source.bin";
		else
			tablename = directory + "/Source.xml";
			
		// Determine the file size.
		ifstream::pos_type size;
		ifstream tablefile(tablename.c_str(), ios::in|ios::binary|ios::ate);

 		if (tablefile.is_open()) { 
  				size = tablefile.tellg(); 
  		}
		else {
				throw ConversionException("Could not open file " + tablename, "Source");
		}
		
		// Re position to the beginning.
		tablefile.seekg(0);
		
		// Read in a stringstream.
		stringstream ss;
		ss << tablefile.rdbuf();

		if (tablefile.rdstate() == istream::failbit || tablefile.rdstate() == istream::badbit) {
			throw ConversionException("Error reading file " + tablename,"Source");
		}

		// And close
		tablefile.close();
		if (tablefile.rdstate() == istream::failbit)
			throw ConversionException("Could not close file " + tablename,"Source");
					
		// And parse the content with the appropriate method
		if (fileAsBin) 
			setFromMIME(ss.str());
		else
			fromXML(ss.str());	
	}			

	

	

			
	
		
		
	/**
	 * Insert a SourceRow* in a vector of SourceRow* so that it's ordered by ascending start time.
	 *
	 * @param SourceRow* x . The pointer to be inserted.
	 * @param vector <SourceRow*>& row. A reference to the vector where to insert x.
	 *
	 */
	 SourceRow* SourceTable::insertByStartTime(SourceRow* x, vector<SourceRow*>& row) {
				
		vector <SourceRow*>::iterator theIterator;
		
		ArrayTime start = x->timeInterval.getStart();

    	// Is the row vector empty ?
    	if (row.size() == 0) {
    		row.push_back(x);
    		privateRows.push_back(x);
    		x->isAdded();
    		return x;
    	}
    	
    	// Optimization for the case of insertion by ascending time.
    	SourceRow* last = *(row.end()-1);
        
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
    	SourceRow* first = *(row.begin());
        
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
					throw DuplicateKey("DuplicateKey exception in ", "SourceTable");	
			}
			else if (start == row[k1]->timeInterval.getStart()) {
				if (row[k1]->equalByRequiredValue(x))
					return row[k1];
				else
					throw DuplicateKey("DuplicateKey exception in ", "SourceTable");	
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
 
