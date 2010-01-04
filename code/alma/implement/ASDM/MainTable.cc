
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
 * File MainTable.cpp
 */
#include <ConversionException.h>
#include <DuplicateKey.h>
#include <OutOfBoundsException.h>

using asdm::ConversionException;
using asdm::DuplicateKey;
using asdm::OutOfBoundsException;

#include <ASDM.h>
#include <MainTable.h>
#include <MainRow.h>
#include <Parser.h>

using asdm::ASDM;
using asdm::MainTable;
using asdm::MainRow;
using asdm::Parser;

#include <iostream>
#include <sstream>
#include <set>
using namespace std;

#include <Misc.h>
using namespace asdm;


namespace asdm {

	string MainTable::tableName = "Main";
	

	/**
	 * The list of field names that make up key key.
	 * (Initialization is in the constructor.)
	 */
	vector<string> MainTable::key;

	/**
	 * Return the list of field names that make up key key
	 * as an array of strings.
	 */	
	vector<string> MainTable::getKeyName() {
		return key;
	}


	MainTable::MainTable(ASDM &c) : container(c) {

	
		key.push_back("time");
	
		key.push_back("configDescriptionId");
	
		key.push_back("fieldId");
	


		// Define a default entity.
		entity.setEntityId(EntityId("uid://X0/X0/X0"));
		entity.setEntityIdEncrypted("na");
		entity.setEntityTypeName("MainTable");
		entity.setEntityVersion("1");
		entity.setInstanceVersion("1");
		
		// Archive XML
		archiveAsBin = false;
		
		// File XML
		fileAsBin = false;
	}
	
/**
 * A destructor for MainTable.
 */
 
	MainTable::~MainTable() {
		for (unsigned int i = 0; i < privateRows.size(); i++) 
			delete(privateRows.at(i));
	}

	/**
	 * Container to which this table belongs.
	 */
	ASDM &MainTable::getContainer() const {
		return container;
	}

	/**
	 * Return the number of rows in the table.
	 */
	
	
		
	unsigned int MainTable::size() {
		int result = 0;
		
		map<string, TIME_ROWS >::iterator mapIter;
		for (mapIter=context.begin(); mapIter!=context.end(); mapIter++) 
			result += ((*mapIter).second).size();
			
		return result;
	}	
		
	
	
	
	/**
	 * Return the name of this table.
	 */
	string MainTable::getName() const {
		return tableName;
	}

	/**
	 * Return this table's Entity.
	 */
	Entity MainTable::getEntity() const {
		return entity;
	}

	/**
	 * Set this table's Entity.
	 */
	void MainTable::setEntity(Entity e) {
		this->entity = e; 
	}
	
	//
	// ====> Row creation.
	//
	
	/**
	 * Create a new row.
	 */
	MainRow *MainTable::newRow() {
		return new MainRow (*this);
	}
	
	MainRow *MainTable::newRowEmpty() {
		return newRow ();
	}


	/**
	 * Create a new row initialized to the specified values.
	 * @return a pointer on the created and initialized row.
	
 	 * @param time. 
	
 	 * @param configDescriptionId. 
	
 	 * @param fieldId. 
	
 	 * @param numAntenna. 
	
 	 * @param timeSampling. 
	
 	 * @param interval. 
	
 	 * @param numIntegration. 
	
 	 * @param scanNumber. 
	
 	 * @param subscanNumber. 
	
 	 * @param dataSize. 
	
 	 * @param dataOid. 
	
 	 * @param stateId. 
	
 	 * @param execBlockId. 
	
     */
	MainRow* MainTable::newRow(ArrayTime time, Tag configDescriptionId, Tag fieldId, int numAntenna, TimeSamplingMod::TimeSampling timeSampling, Interval interval, int numIntegration, int scanNumber, int subscanNumber, int dataSize, EntityRef dataOid, vector<Tag>  stateId, Tag execBlockId){
		MainRow *row = new MainRow(*this);
			
		row->setTime(time);
			
		row->setConfigDescriptionId(configDescriptionId);
			
		row->setFieldId(fieldId);
			
		row->setNumAntenna(numAntenna);
			
		row->setTimeSampling(timeSampling);
			
		row->setInterval(interval);
			
		row->setNumIntegration(numIntegration);
			
		row->setScanNumber(scanNumber);
			
		row->setSubscanNumber(subscanNumber);
			
		row->setDataSize(dataSize);
			
		row->setDataOid(dataOid);
			
		row->setStateId(stateId);
			
		row->setExecBlockId(execBlockId);
	
		return row;		
	}	

	MainRow* MainTable::newRowFull(ArrayTime time, Tag configDescriptionId, Tag fieldId, int numAntenna, TimeSamplingMod::TimeSampling timeSampling, Interval interval, int numIntegration, int scanNumber, int subscanNumber, int dataSize, EntityRef dataOid, vector<Tag>  stateId, Tag execBlockId)	{
		MainRow *row = new MainRow(*this);
			
		row->setTime(time);
			
		row->setConfigDescriptionId(configDescriptionId);
			
		row->setFieldId(fieldId);
			
		row->setNumAntenna(numAntenna);
			
		row->setTimeSampling(timeSampling);
			
		row->setInterval(interval);
			
		row->setNumIntegration(numIntegration);
			
		row->setScanNumber(scanNumber);
			
		row->setSubscanNumber(subscanNumber);
			
		row->setDataSize(dataSize);
			
		row->setDataOid(dataOid);
			
		row->setStateId(stateId);
			
		row->setExecBlockId(execBlockId);
	
		return row;				
	}
	


MainRow* MainTable::newRow(MainRow* row) {
	return new MainRow(*this, *row);
}

MainRow* MainTable::newRowCopy(MainRow* row) {
	return new MainRow(*this, *row);
}

	//
	// Append a row to its table.
	//

	
	
		
		
	/** 
	 * Returns a string built by concatenating the ascii representation of the
	 * parameters values suffixed with a "_" character.
	 */
	 string MainTable::Key(Tag configDescriptionId, Tag fieldId) {
	 	ostringstream ostrstr;
	 		ostrstr  
			
				<<  configDescriptionId.toString()  << "_"
			
				<<  fieldId.toString()  << "_"
			
			;
		return ostrstr.str();	 	
	 }
	 
			
			
	MainRow* MainTable::add(MainRow* x) {
		string keystr = Key(
						x->getConfigDescriptionId()
					   ,
						x->getFieldId()
					   );
		if (context.find(keystr) == context.end()) {
			vector<MainRow *> v;
			context[keystr] = v;
		}
		return insertByTime(x, context[keystr]);					
	}
			
		
	




	// 
	// A private method to append a row to its table, used by input conversion
	// methods.
	//

	
	
		
		
			
			
			
			
	MainRow*  MainTable::checkAndAdd(MainRow* x) {
		string keystr = Key( 
						x->getConfigDescriptionId() 
					   , 
						x->getFieldId() 
					   ); 
		if (context.find(keystr) == context.end()) {
			vector<MainRow *> v;
			context[keystr] = v;
		}
		
		vector<MainRow*>& found = context.find(keystr)->second;
		return insertByTime(x, found);	
	}				
			
					
		







	

	
	
		
	/**
	 * Get all rows.
	 * @return Alls rows as an array of MainRow
	 */
	 vector<MainRow *> MainTable::get() {
	    return privateRows;
	    
	 /*
	 	vector<MainRow *> v;
	 	map<string, TIME_ROWS>::iterator mapIter;
	 	vector<MainRow *>::iterator rowIter;
	 	
	 	for (mapIter=context.begin(); mapIter!=context.end(); mapIter++) {
	 		for (rowIter=((*mapIter).second).begin(); rowIter!=((*mapIter).second).end(); rowIter++) 
	 			v.push_back(*rowIter); 
	 	}
	 	
	 	return v;
	 */
	 }
	 
	 vector<MainRow *> *MainTable::getByContext(Tag configDescriptionId, Tag fieldId) {
	  	string k = Key(configDescriptionId, fieldId);
 
	    if (context.find(k) == context.end()) return 0;
 	   else return &(context[k]);		
	}		
		
	


	
		
		
			
			
			
/*
 ** Returns a MainRow* given a key.
 ** @return a pointer to the row having the key whose values are passed as parameters, or 0 if
 ** no row exists for that key.
 **
 */
 				
				
 	MainRow* MainTable::getRowByKey(ArrayTime time, Tag configDescriptionId, Tag fieldId)  {
		string keystr = Key(configDescriptionId, fieldId);
 		
 		if (context.find(keystr) == context.end()) return 0;
 		
 		vector<MainRow* > row = context[keystr];
 		
 		// Is the vector empty...impossible in principle !
		if (row.size() == 0) return 0;
		
		// Only one element in the vector
		if (row.size() == 1) {
			if (time.get() == row.at(0)->getTime().get())
				return row.at(0);
			else
				return 0;	
		}
		
		// Optimizations 
		MainRow* last = row.at(row.size()-1);		
		if (time.get() > last->getTime().get()) return 0;
		MainRow* first = row.at(0);
		if (time.get() < first->getTime().get()) return 0;
		
		// More than one row
		// let's use a dichotomy method for the general case..		
		int k0 = 0;
		int k1 = row.size() - 1;    	  
		while (k0 !=  k1 ) {
			if (time.get() == row.at(k0)->getTime().get()) {
				return row.at(k0);
			}
			else if (time.get() == row.at(k1)->getTime().get()) {
				return row.at(k1);
			}
			else {
				if (time.get() <= row.at((k0+k1)/2)->getTime().get())
					k1 = (k0 + k1) / 2;
				else
					k0 = (k0 + k1) / 2;				
			} 	
		}
		return 0; 			
	}								
							
			
		
		
		
	




#ifndef WITHOUT_ACS
	// Conversion Methods

	MainTableIDL *MainTable::toIDL() {
		MainTableIDL *x = new MainTableIDL ();
		unsigned int nrow = size();
		x->row.length(nrow);
		vector<MainRow*> v = get();
		for (unsigned int i = 0; i < nrow; ++i) {
			x->row[i] = *(v[i]->toIDL());
		}
		return x;
	}
#endif
	
#ifndef WITHOUT_ACS
	void MainTable::fromIDL(MainTableIDL x) {
		unsigned int nrow = x.row.length();
		for (unsigned int i = 0; i < nrow; ++i) {
			MainRow *tmp = newRow();
			tmp->setFromIDL(x.row[i]);
			// checkAndAdd(tmp);
			add(tmp);
		}
	}
#endif

	char *MainTable::toFITS() const  {
		throw ConversionException("Not implemented","Main");
	}

	void MainTable::fromFITS(char *fits)  {
		throw ConversionException("Not implemented","Main");
	}

	string MainTable::toVOTable() const {
		throw ConversionException("Not implemented","Main");
	}

	void MainTable::fromVOTable(string vo) {
		throw ConversionException("Not implemented","Main");
	}

	
	string MainTable::toXML()  {
		string buf;
		buf.append("<?xml version=\"1.0\" encoding=\"ISO-8859-1\"?> ");
		buf.append("<MainTable xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" xmlns=\"http://Alma/XASDM/MainTable\" xsi:schemaLocation=\"http://Alma/XASDM/MainTable http://almaobservatory.org/XML/XASDM/2/MainTable.xsd\"> ");	
		buf.append(entity.toXML());
		string s = container.getEntity().toXML();
		// Change the "Entity" tag to "ContainerEntity".
		buf.append("<Container" + s.substr(1,s.length() - 1)+" ");
		vector<MainRow*> v = get();
		for (unsigned int i = 0; i < v.size(); ++i) {
			try {
				buf.append(v[i]->toXML());
			} catch (NoSuchRow e) {
			}
			buf.append("  ");
		}		
		buf.append("</MainTable> ");
		return buf;
	}

	
	void MainTable::fromXML(string xmlDoc)  {
		Parser xml(xmlDoc);
		if (!xml.isStr("<MainTable")) 
			error();
		// cout << "Parsing a MainTable" << endl;
		string s = xml.getElement("<Entity","/>");
		if (s.length() == 0) 
			error();
		Entity e;
		e.setFromXML(s);
		if (e.getEntityTypeName() != "MainTable")
			error();
		setEntity(e);
		// Skip the container's entity; but, it has to be there.
		s = xml.getElement("<ContainerEntity","/>");
		if (s.length() == 0) 
			error();

		// Get each row in the table.
		s = xml.getElementContent("<row>","</row>");
		MainRow *row;
		while (s.length() != 0) {
			// cout << "Parsing a MainRow" << endl; 
			row = newRow();
			row->setFromXML(s);
			try {
				checkAndAdd(row);
			} catch (DuplicateKey e1) {
				throw ConversionException(e1.getMessage(),"MainTable");
			} 
			catch (UniquenessViolationException e1) {
				throw ConversionException(e1.getMessage(),"MainTable");	
			}
			catch (...) {
				// cout << "Unexpected error in MainTable::checkAndAdd called from MainTable::fromXML " << endl;
			}
			s = xml.getElementContent("<row>","</row>");
		}
		if (!xml.isStr("</MainTable>")) 
			error();
	}

	
	void MainTable::error()  {
		throw ConversionException("Invalid xml document","Main");
	}
	
	
	string MainTable::toMIME() {
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

	
	void MainTable::setFromMIME(const string & mimeMsg) {
		// cout << "Entering setFromMIME" << endl;
	 	string terminator = "Content-Type: binary/octet-stream\nContent-ID: <content.bin>\n\n";
	 	
	 	// Look for the string announcing the binary part.
	 	string::size_type loc = mimeMsg.find( terminator, 0 );
	 	
	 	if ( loc == string::npos ) {
	 		throw ConversionException("Failed to detect the beginning of the binary part", "Main");
	 	}
	
	 	// Create an EndianISStream from the substring containing the binary part.
	 	EndianISStream eiss(mimeMsg.substr(loc+terminator.size()));
	 	
	 	entity = Entity::fromBin(eiss);
	 	
	 	// We do nothing with that but we have to read it.
	 	Entity containerEntity = Entity::fromBin(eiss);
	 		 	
	 	int numRows = eiss.readInt();
	 	try {
	 		for (int i = 0; i < numRows; i++) {
	 			MainRow* aRow = MainRow::fromBin(eiss, *this);
	 			checkAndAdd(aRow);
	 		}
	 	}
	 	catch (DuplicateKey e) {
	 		throw ConversionException("Error while writing binary data , the message was "
	 					+ e.getMessage(), "Main");
	 	}
		catch (TagFormatException e) {
			throw ConversionException("Error while reading binary data , the message was "
					+ e.getMessage(), "Main");
		} 		 	
	}

	
	void MainTable::toFile(string directory) {
		if (!directoryExists(directory.c_str()) &&
			!createPath(directory.c_str())) {
			throw ConversionException("Could not create directory " , directory);
		}
		
		if (fileAsBin) {
			// write the bin serialized
			string fileName = directory + "/Main.bin";
			ofstream tableout(fileName.c_str(),ios::out|ios::trunc);
			if (tableout.rdstate() == ostream::failbit)
				throw ConversionException("Could not open file " + fileName + " to write ", "Main");
			tableout << toMIME() << endl;
			tableout.close();
			if (tableout.rdstate() == ostream::failbit)
				throw ConversionException("Could not close file " + fileName, "Main");
		}
		else {
			// write the XML
			string fileName = directory + "/Main.xml";
			ofstream tableout(fileName.c_str(),ios::out|ios::trunc);
			if (tableout.rdstate() == ostream::failbit)
				throw ConversionException("Could not open file " + fileName + " to write ", "Main");
			tableout << toXML() << endl;
			tableout.close();
			if (tableout.rdstate() == ostream::failbit)
				throw ConversionException("Could not close file " + fileName, "Main");
		}
	}

	
	void MainTable::setFromFile(const string& directory) {
		string tablename;
		if (fileAsBin)
			tablename = directory + "/Main.bin";
		else
			tablename = directory + "/Main.xml";
			
		// Determine the file size.
		ifstream::pos_type size;
		ifstream tablefile(tablename.c_str(), ios::in|ios::binary|ios::ate);

 		if (tablefile.is_open()) { 
  				size = tablefile.tellg(); 
  		}
		else {
				throw ConversionException("Could not open file " + tablename, "Main");
		}
		
		// Re position to the beginning.
		tablefile.seekg(0);
		
		// Read in a stringstream.
		stringstream ss;
		ss << tablefile.rdbuf();

		if (tablefile.rdstate() == istream::failbit || tablefile.rdstate() == istream::badbit) {
			throw ConversionException("Error reading file " + tablename,"Main");
		}

		// And close
		tablefile.close();
		if (tablefile.rdstate() == istream::failbit)
			throw ConversionException("Could not close file " + tablename,"Main");
					
		// And parse the content with the appropriate method
		if (fileAsBin) 
			setFromMIME(ss.str());
		else
			fromXML(ss.str());	
	}			

	

	

			
	
		
    	
 	MainRow * MainTable::insertByTime(MainRow* x, vector<MainRow *>&row ) {
		ArrayTime start = x->getTime();
		
		// Is the vector empty ?
		if (row.size() == 0) {
			row.push_back(x);
			privateRows.push_back(x);
			x->isAdded();
			return x;
		}
		
		// Optimization for the case of insertion by ascending time.
		MainRow* last = row.at(row.size()-1);
		
		if (start.get() > last->getTime().get()) {
			row.push_back(x);
			privateRows.push_back(x);
			x->isAdded();
			return x;
		}
		
		// Optimization for the case of insertion by descending time.
		MainRow* first = row.at(0);
		
		if (start.get() < first->getTime().get()) {
			row.insert(row.begin(), x);
			privateRows.push_back(x);
			x->isAdded();
			return x;
		}
		
		// Case where x has to be inserted inside row; let's use a dichotomy
		// method to find the insertion index.		
		int k0 = 0;
		int k1 = row.size() - 1;    	  
		while (k0 != (k1 - 1)) {
			if (start.get() == row.at(k0)->getTime().get()) {
				if (row.at(k0)->equalByRequiredValue(x))
					return row.at(k0);
				else
					throw DuplicateKey("DuplicateKey exception in ", "MainTable");	
			}
			else if (start.get() == row.at(k1)->getTime().get()) {
				if (row.at(k1)->equalByRequiredValue(x))
					return row.at(k1);
				else
					throw  DuplicateKey("DuplicateKey exception in ", "MainTable");	
			}
			else {
				if (start.get() <= row.at((k0+k1)/2)->getTime().get())
					k1 = (k0 + k1) / 2;
				else
					k0 = (k0 + k1) / 2;				
			} 	
		}
		row.insert(row.begin()+(k0+1), x);
		privateRows.push_back(x);
		x->isAdded();
		return x; 						
	}   	
    	
	
	

	
} // End namespace asdm
 
