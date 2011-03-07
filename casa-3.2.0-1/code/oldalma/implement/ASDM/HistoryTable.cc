
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
 * File HistoryTable.cpp
 */
#include <ConversionException.h>
#include <DuplicateKey.h>
#include <OutOfBoundsException.h>

using asdm::ConversionException;
using asdm::DuplicateKey;
using asdm::OutOfBoundsException;

#include <ASDM.h>
#include <HistoryTable.h>
#include <HistoryRow.h>
#include <Parser.h>

using asdm::ASDM;
using asdm::HistoryTable;
using asdm::HistoryRow;
using asdm::Parser;

#include <iostream>
#include <sstream>
#include <set>
using namespace std;

#include <Misc.h>
using namespace asdm;


namespace asdm {

	string HistoryTable::tableName = "History";
	

	/**
	 * The list of field names that make up key key.
	 * (Initialization is in the constructor.)
	 */
	vector<string> HistoryTable::key;

	/**
	 * Return the list of field names that make up key key
	 * as an array of strings.
	 */	
	vector<string> HistoryTable::getKeyName() {
		return key;
	}


	HistoryTable::HistoryTable(ASDM &c) : container(c) {

	
		key.push_back("execBlockId");
	
		key.push_back("time");
	


		// Define a default entity.
		entity.setEntityId(EntityId("uid://X0/X0/X0"));
		entity.setEntityIdEncrypted("na");
		entity.setEntityTypeName("HistoryTable");
		entity.setEntityVersion("1");
		entity.setInstanceVersion("1");
		
		// Archive XML
		archiveAsBin = false;
		
		// File XML
		fileAsBin = false;
	}
	
/**
 * A destructor for HistoryTable.
 */
 
	HistoryTable::~HistoryTable() {
		for (unsigned int i = 0; i < privateRows.size(); i++) 
			delete(privateRows.at(i));
	}

	/**
	 * Container to which this table belongs.
	 */
	ASDM &HistoryTable::getContainer() const {
		return container;
	}

	/**
	 * Return the number of rows in the table.
	 */
	
	
		
	unsigned int HistoryTable::size() {
		int result = 0;
		
		map<string, TIME_ROWS >::iterator mapIter;
		for (mapIter=context.begin(); mapIter!=context.end(); mapIter++) 
			result += ((*mapIter).second).size();
			
		return result;
	}	
		
	
	
	
	/**
	 * Return the name of this table.
	 */
	string HistoryTable::getName() const {
		return tableName;
	}

	/**
	 * Return this table's Entity.
	 */
	Entity HistoryTable::getEntity() const {
		return entity;
	}

	/**
	 * Set this table's Entity.
	 */
	void HistoryTable::setEntity(Entity e) {
		this->entity = e; 
	}
	
	//
	// ====> Row creation.
	//
	
	/**
	 * Create a new row.
	 */
	HistoryRow *HistoryTable::newRow() {
		return new HistoryRow (*this);
	}
	
	HistoryRow *HistoryTable::newRowEmpty() {
		return newRow ();
	}


	/**
	 * Create a new row initialized to the specified values.
	 * @return a pointer on the created and initialized row.
	
 	 * @param execBlockId. 
	
 	 * @param time. 
	
 	 * @param message. 
	
 	 * @param priority. 
	
 	 * @param origin. 
	
 	 * @param objectId. 
	
 	 * @param application. 
	
 	 * @param cliCommand. 
	
 	 * @param appParms. 
	
     */
	HistoryRow* HistoryTable::newRow(Tag execBlockId, ArrayTime time, string message, string priority, string origin, string objectId, string application, string cliCommand, string appParms){
		HistoryRow *row = new HistoryRow(*this);
			
		row->setExecBlockId(execBlockId);
			
		row->setTime(time);
			
		row->setMessage(message);
			
		row->setPriority(priority);
			
		row->setOrigin(origin);
			
		row->setObjectId(objectId);
			
		row->setApplication(application);
			
		row->setCliCommand(cliCommand);
			
		row->setAppParms(appParms);
	
		return row;		
	}	

	HistoryRow* HistoryTable::newRowFull(Tag execBlockId, ArrayTime time, string message, string priority, string origin, string objectId, string application, string cliCommand, string appParms)	{
		HistoryRow *row = new HistoryRow(*this);
			
		row->setExecBlockId(execBlockId);
			
		row->setTime(time);
			
		row->setMessage(message);
			
		row->setPriority(priority);
			
		row->setOrigin(origin);
			
		row->setObjectId(objectId);
			
		row->setApplication(application);
			
		row->setCliCommand(cliCommand);
			
		row->setAppParms(appParms);
	
		return row;				
	}
	


HistoryRow* HistoryTable::newRow(HistoryRow* row) {
	return new HistoryRow(*this, *row);
}

HistoryRow* HistoryTable::newRowCopy(HistoryRow* row) {
	return new HistoryRow(*this, *row);
}

	//
	// Append a row to its table.
	//

	
	
		
		
	/** 
	 * Returns a string built by concatenating the ascii representation of the
	 * parameters values suffixed with a "_" character.
	 */
	 string HistoryTable::Key(Tag execBlockId) {
	 	ostringstream ostrstr;
	 		ostrstr  
			
				<<  execBlockId.toString()  << "_"
			
			;
		return ostrstr.str();	 	
	 }
	 
			
			
	HistoryRow* HistoryTable::add(HistoryRow* x) {
		string keystr = Key(
						x->getExecBlockId()
					   );
		if (context.find(keystr) == context.end()) {
			vector<HistoryRow *> v;
			context[keystr] = v;
		}
		return insertByTime(x, context[keystr]);					
	}
			
		
	




	// 
	// A private method to append a row to its table, used by input conversion
	// methods.
	//

	
	
		
		
			
			
			
			
	HistoryRow*  HistoryTable::checkAndAdd(HistoryRow* x) throw (DuplicateKey) {
		string keystr = Key( 
						x->getExecBlockId() 
					   ); 
		if (context.find(keystr) == context.end()) {
			vector<HistoryRow *> v;
			context[keystr] = v;
		}
		
		vector<HistoryRow*>& found = context.find(keystr)->second;
		return insertByTime(x, found);	
	}				
			
					
		







	

	
	
		
	/**
	 * Get all rows.
	 * @return Alls rows as an array of HistoryRow
	 */
	 vector<HistoryRow *> HistoryTable::get() {
	    return privateRows;
	    
	 /*
	 	vector<HistoryRow *> v;
	 	map<string, TIME_ROWS>::iterator mapIter;
	 	vector<HistoryRow *>::iterator rowIter;
	 	
	 	for (mapIter=context.begin(); mapIter!=context.end(); mapIter++) {
	 		for (rowIter=((*mapIter).second).begin(); rowIter!=((*mapIter).second).end(); rowIter++) 
	 			v.push_back(*rowIter); 
	 	}
	 	
	 	return v;
	 */
	 }
	 
	 vector<HistoryRow *> *HistoryTable::getByContext(Tag execBlockId) {
	  	string k = Key(execBlockId);
 
	    if (context.find(k) == context.end()) return 0;
 	   else return &(context[k]);		
	}		
		
	


	
		
		
			
			
			
/*
 ** Returns a HistoryRow* given a key.
 ** @return a pointer to the row having the key whose values are passed as parameters, or 0 if
 ** no row exists for that key.
 **
 */
 				
				
 	HistoryRow* HistoryTable::getRowByKey(Tag execBlockId, ArrayTime time)  {
		string keystr = Key(execBlockId);
 		
 		if (context.find(keystr) == context.end()) return 0;
 		
 		vector<HistoryRow* > row = context[keystr];
 		
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
		HistoryRow* last = row.at(row.size()-1);		
		if (time.get() > last->getTime().get()) return 0;
		HistoryRow* first = row.at(0);
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

	HistoryTableIDL *HistoryTable::toIDL() {
		HistoryTableIDL *x = new HistoryTableIDL ();
		unsigned int nrow = size();
		x->row.length(nrow);
		vector<HistoryRow*> v = get();
		for (unsigned int i = 0; i < nrow; ++i) {
			x->row[i] = *(v[i]->toIDL());
		}
		return x;
	}
#endif
	
#ifndef WITHOUT_ACS
	void HistoryTable::fromIDL(HistoryTableIDL x) throw(DuplicateKey,ConversionException) {
		unsigned int nrow = x.row.length();
		for (unsigned int i = 0; i < nrow; ++i) {
			HistoryRow *tmp = newRow();
			tmp->setFromIDL(x.row[i]);
			// checkAndAdd(tmp);
			add(tmp);
		}
	}
#endif

	char *HistoryTable::toFITS() const throw(ConversionException) {
		throw ConversionException("Not implemented","History");
	}

	void HistoryTable::fromFITS(char *fits) throw(ConversionException) {
		throw ConversionException("Not implemented","History");
	}

	string HistoryTable::toVOTable() const throw(ConversionException) {
		throw ConversionException("Not implemented","History");
	}

	void HistoryTable::fromVOTable(string vo) throw(ConversionException) {
		throw ConversionException("Not implemented","History");
	}

	string HistoryTable::toXML()  throw(ConversionException) {
		string buf;
		buf.append("<?xml version=\"1.0\" encoding=\"ISO-8859-1\"?> ");
//		buf.append("<HistoryTable xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" xsi:noNamespaceSchemaLocation=\"../../idl/HistoryTable.xsd\"> ");
		buf.append("<?xml-stylesheet type=\"text/xsl\" href=\"../asdm2html/table2html.xsl\"?> ");		
		buf.append("<HistoryTable> ");
		buf.append(entity.toXML());
		string s = container.getEntity().toXML();
		// Change the "Entity" tag to "ContainerEntity".
		buf.append("<Container" + s.substr(1,s.length() - 1)+" ");
		vector<HistoryRow*> v = get();
		for (unsigned int i = 0; i < v.size(); ++i) {
			try {
				buf.append(v[i]->toXML());
			} catch (NoSuchRow e) {
			}
			buf.append("  ");
		}		
		buf.append("</HistoryTable> ");
		return buf;
	}
	
	void HistoryTable::fromXML(string xmlDoc) throw(ConversionException) {
		Parser xml(xmlDoc);
		if (!xml.isStr("<HistoryTable")) 
			error();
		// cout << "Parsing a HistoryTable" << endl;
		string s = xml.getElement("<Entity","/>");
		if (s.length() == 0) 
			error();
		Entity e;
		e.setFromXML(s);
		if (e.getEntityTypeName() != "HistoryTable")
			error();
		setEntity(e);
		// Skip the container's entity; but, it has to be there.
		s = xml.getElement("<ContainerEntity","/>");
		if (s.length() == 0) 
			error();

		// Get each row in the table.
		s = xml.getElementContent("<row>","</row>");
		HistoryRow *row;
		while (s.length() != 0) {
			// cout << "Parsing a HistoryRow" << endl; 
			row = newRow();
			row->setFromXML(s);
			try {
				checkAndAdd(row);
			} catch (DuplicateKey e1) {
				throw ConversionException(e1.getMessage(),"HistoryTable");
			} 
			catch (UniquenessViolationException e1) {
				throw ConversionException(e1.getMessage(),"HistoryTable");	
			}
			catch (...) {
				// cout << "Unexpected error in HistoryTable::checkAndAdd called from HistoryTable::fromXML " << endl;
			}
			s = xml.getElementContent("<row>","</row>");
		}
		if (!xml.isStr("</HistoryTable>")) 
			error();
	}

	void HistoryTable::error() throw(ConversionException) {
		throw ConversionException("Invalid xml document","History");
	}
	
	string HistoryTable::toMIME() {
	 // To be implemented
		return "";
	}
	
	void HistoryTable::setFromMIME(const string & mimeMsg) {
		// To be implemented
		;
	}
	
	
	void HistoryTable::toFile(string directory) {
		if (!directoryExists(directory.c_str()) &&
			!createPath(directory.c_str())) {
			throw ConversionException("Could not create directory " , directory);
		}
		
		if (fileAsBin) {
			// write the bin serialized
			string fileName = directory + "/History.bin";
			ofstream tableout(fileName.c_str(),ios::out|ios::trunc);
			if (tableout.rdstate() == ostream::failbit)
				throw ConversionException("Could not open file " + fileName + " to write ", "History");
			tableout << toMIME() << endl;
			tableout.close();
			if (tableout.rdstate() == ostream::failbit)
				throw ConversionException("Could not close file " + fileName, "History");
		}
		else {
			// write the XML
			string fileName = directory + "/History.xml";
			ofstream tableout(fileName.c_str(),ios::out|ios::trunc);
			if (tableout.rdstate() == ostream::failbit)
				throw ConversionException("Could not open file " + fileName + " to write ", "History");
			tableout << toXML() << endl;
			tableout.close();
			if (tableout.rdstate() == ostream::failbit)
				throw ConversionException("Could not close file " + fileName, "History");
		}
	}
	
	void HistoryTable::setFromFile(const string& directory) {
		string tablename;
		if (fileAsBin)
			tablename = directory + "/History.bin";
		else
			tablename = directory + "/History.xml";
			
		// Determine the file size.
		ifstream::pos_type size;
		ifstream tablefile(tablename.c_str(), ios::in|ios::binary|ios::ate);

 		if (tablefile.is_open()) { 
  				size = tablefile.tellg(); 
  		}
		else {
				throw ConversionException("Could not open file " + tablename, "History");
		}
		
		// Re position to the beginning.
		tablefile.seekg(0);
		
		// Read in a stringstream.
		stringstream ss;
		ss << tablefile.rdbuf();

		if (tablefile.rdstate() == istream::failbit || tablefile.rdstate() == istream::badbit) {
			throw ConversionException("Error reading file " + tablename,"History");
		}

		// And close
		tablefile.close();
		if (tablefile.rdstate() == istream::failbit)
			throw ConversionException("Could not close file " + tablename,"History");
					
		// And parse the content with the appropriate method
		if (fileAsBin) 
			setFromMIME(ss.str());
		else
			fromXML(ss.str());	
	}			
			
	
		
    	
 	HistoryRow * HistoryTable::insertByTime(HistoryRow* x, vector<HistoryRow *>&row ) {
		ArrayTime start = x->getTime();
		
		// Is the vector empty ?
		if (row.size() == 0) {
			row.push_back(x);
			privateRows.push_back(x);
			x->isAdded();
			return x;
		}
		
		// Optimization for the case of insertion by ascending time.
		HistoryRow* last = row.at(row.size()-1);
		
		if (start.get() > last->getTime().get()) {
			row.push_back(x);
			privateRows.push_back(x);
			x->isAdded();
			return x;
		}
		
		// Optimization for the case of insertion by descending time.
		HistoryRow* first = row.at(0);
		
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
					throw new DuplicateKey("DuplicateKey exception in ", "HistoryTable");	
			}
			else if (start.get() == row.at(k1)->getTime().get()) {
				if (row.at(k1)->equalByRequiredValue(x))
					return row.at(k1);
				else
					throw new DuplicateKey("DuplicateKey exception in ", "HistoryTable");	
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
 
