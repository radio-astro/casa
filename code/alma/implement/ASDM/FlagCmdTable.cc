
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
 * File FlagCmdTable.cpp
 */
#include <ConversionException.h>
#include <DuplicateKey.h>
#include <OutOfBoundsException.h>

using asdm::ConversionException;
using asdm::DuplicateKey;
using asdm::OutOfBoundsException;

#include <ASDM.h>
#include <FlagCmdTable.h>
#include <FlagCmdRow.h>
#include <Parser.h>

using asdm::ASDM;
using asdm::FlagCmdTable;
using asdm::FlagCmdRow;
using asdm::Parser;

#include <iostream>
#include <sstream>
#include <set>
using namespace std;

#include <Misc.h>
using namespace asdm;


namespace asdm {

	string FlagCmdTable::tableName = "FlagCmd";
	

	/**
	 * The list of field names that make up key key.
	 * (Initialization is in the constructor.)
	 */
	vector<string> FlagCmdTable::key;

	/**
	 * Return the list of field names that make up key key
	 * as an array of strings.
	 */	
	vector<string> FlagCmdTable::getKeyName() {
		return key;
	}


	FlagCmdTable::FlagCmdTable(ASDM &c) : container(c) {

	
		key.push_back("timeInterval");
	


		// Define a default entity.
		entity.setEntityId(EntityId("uid://X0/X0/X0"));
		entity.setEntityIdEncrypted("na");
		entity.setEntityTypeName("FlagCmdTable");
		entity.setEntityVersion("1");
		entity.setInstanceVersion("1");
		
		// Archive XML
		archiveAsBin = false;
		
		// File XML
		fileAsBin = false;
	}
	
/**
 * A destructor for FlagCmdTable.
 */
 
	FlagCmdTable::~FlagCmdTable() {
		for (unsigned int i = 0; i < privateRows.size(); i++) 
			delete(privateRows.at(i));
	}

	/**
	 * Container to which this table belongs.
	 */
	ASDM &FlagCmdTable::getContainer() const {
		return container;
	}

	/**
	 * Return the number of rows in the table.
	 */
	
	
		
	unsigned int FlagCmdTable::size() {
		return row.size();
	}
		
	
	
	
	/**
	 * Return the name of this table.
	 */
	string FlagCmdTable::getName() const {
		return tableName;
	}

	/**
	 * Return this table's Entity.
	 */
	Entity FlagCmdTable::getEntity() const {
		return entity;
	}

	/**
	 * Set this table's Entity.
	 */
	void FlagCmdTable::setEntity(Entity e) {
		this->entity = e; 
	}
	
	//
	// ====> Row creation.
	//
	
	/**
	 * Create a new row.
	 */
	FlagCmdRow *FlagCmdTable::newRow() {
		return new FlagCmdRow (*this);
	}
	
	FlagCmdRow *FlagCmdTable::newRowEmpty() {
		return newRow ();
	}


	/**
	 * Create a new row initialized to the specified values.
	 * @return a pointer on the created and initialized row.
	
 	 * @param timeInterval. 
	
 	 * @param type. 
	
 	 * @param reason. 
	
 	 * @param level. 
	
 	 * @param severity. 
	
 	 * @param applied. 
	
 	 * @param command. 
	
     */
	FlagCmdRow* FlagCmdTable::newRow(ArrayTimeInterval timeInterval, string type, string reason, int level, int severity, bool applied, string command){
		FlagCmdRow *row = new FlagCmdRow(*this);
			
		row->setTimeInterval(timeInterval);
			
		row->setType(type);
			
		row->setReason(reason);
			
		row->setLevel(level);
			
		row->setSeverity(severity);
			
		row->setApplied(applied);
			
		row->setCommand(command);
	
		return row;		
	}	

	FlagCmdRow* FlagCmdTable::newRowFull(ArrayTimeInterval timeInterval, string type, string reason, int level, int severity, bool applied, string command)	{
		FlagCmdRow *row = new FlagCmdRow(*this);
			
		row->setTimeInterval(timeInterval);
			
		row->setType(type);
			
		row->setReason(reason);
			
		row->setLevel(level);
			
		row->setSeverity(severity);
			
		row->setApplied(applied);
			
		row->setCommand(command);
	
		return row;				
	}
	


FlagCmdRow* FlagCmdTable::newRow(FlagCmdRow* row) {
	return new FlagCmdRow(*this, *row);
}

FlagCmdRow* FlagCmdTable::newRowCopy(FlagCmdRow* row) {
	return new FlagCmdRow(*this, *row);
}

	//
	// Append a row to its table.
	//

	
	
		
		  
	FlagCmdRow* FlagCmdTable::add(FlagCmdRow* x) {
		FlagCmdRow* aRow = getRowByKey(
		
			x->getTimeInterval()
		
		);
		// There is a row with x's key section return it.
		if (aRow) throw DuplicateKey("Duplicate key exception in ", "FlagCmdTable");
		
		// Insert the row x in the table in such a way that the vector row is sorted
		// by ascending values on timeInterval.getStart().
		return insertByStartTime(x, row);
	}
		
	




	// 
	// A private method to append a row to its table, used by input conversion
	// methods.
	//

	
	
		
		
			
	FlagCmdRow*  FlagCmdTable::checkAndAdd(FlagCmdRow* x) throw (DuplicateKey) {
		if (getRowByKey(
		
			x->getTimeInterval()
				
		)) throw DuplicateKey("Duplicate key exception in ", "FlagCmdTable");

		return insertByStartTime(x, row);
	}
					
		







	

	
	
		
	/**
	 * Get all rows.
	 * @return Alls rows as an array of FlagCmdRow
	 */
	vector<FlagCmdRow *> FlagCmdTable::get() {
		return privateRows;
		// return row;
	}
		
	


	
		
		
			
			
/*
 ** Returns a FlagCmdRow* given a key.
 ** @return a pointer to the row having the key whose values are passed as parameters, or 0 if
 ** no row exists for that key.
 **
 */
 	FlagCmdRow* FlagCmdTable::getRowByKey(ArrayTimeInterval timeInterval)  {
	FlagCmdRow* aRow = 0;
		for (unsigned int i = 0; i < row.size(); i++) {
			aRow = row.at(i);
			if (aRow->timeInterval.contains(timeInterval.getStart())) return aRow;
		}
		return 0;		
	}
			
		
		
		
	




#ifndef WITHOUT_ACS
	// Conversion Methods

	FlagCmdTableIDL *FlagCmdTable::toIDL() {
		FlagCmdTableIDL *x = new FlagCmdTableIDL ();
		unsigned int nrow = size();
		x->row.length(nrow);
		vector<FlagCmdRow*> v = get();
		for (unsigned int i = 0; i < nrow; ++i) {
			x->row[i] = *(v[i]->toIDL());
		}
		return x;
	}
#endif
	
#ifndef WITHOUT_ACS
	void FlagCmdTable::fromIDL(FlagCmdTableIDL x) throw(DuplicateKey,ConversionException) {
		unsigned int nrow = x.row.length();
		for (unsigned int i = 0; i < nrow; ++i) {
			FlagCmdRow *tmp = newRow();
			tmp->setFromIDL(x.row[i]);
			// checkAndAdd(tmp);
			add(tmp);
		}
	}
#endif

	char *FlagCmdTable::toFITS() const throw(ConversionException) {
		throw ConversionException("Not implemented","FlagCmd");
	}

	void FlagCmdTable::fromFITS(char *fits) throw(ConversionException) {
		throw ConversionException("Not implemented","FlagCmd");
	}

	string FlagCmdTable::toVOTable() const throw(ConversionException) {
		throw ConversionException("Not implemented","FlagCmd");
	}

	void FlagCmdTable::fromVOTable(string vo) throw(ConversionException) {
		throw ConversionException("Not implemented","FlagCmd");
	}

	string FlagCmdTable::toXML()  throw(ConversionException) {
		string buf;
		buf.append("<?xml version=\"1.0\" encoding=\"ISO-8859-1\"?> ");
//		buf.append("<FlagCmdTable xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" xsi:noNamespaceSchemaLocation=\"../../idl/FlagCmdTable.xsd\"> ");
		buf.append("<?xml-stylesheet type=\"text/xsl\" href=\"../asdm2html/table2html.xsl\"?> ");		
		buf.append("<FlagCmdTable> ");
		buf.append(entity.toXML());
		string s = container.getEntity().toXML();
		// Change the "Entity" tag to "ContainerEntity".
		buf.append("<Container" + s.substr(1,s.length() - 1)+" ");
		vector<FlagCmdRow*> v = get();
		for (unsigned int i = 0; i < v.size(); ++i) {
			try {
				buf.append(v[i]->toXML());
			} catch (NoSuchRow e) {
			}
			buf.append("  ");
		}		
		buf.append("</FlagCmdTable> ");
		return buf;
	}
	
	void FlagCmdTable::fromXML(string xmlDoc) throw(ConversionException) {
		Parser xml(xmlDoc);
		if (!xml.isStr("<FlagCmdTable")) 
			error();
		// cout << "Parsing a FlagCmdTable" << endl;
		string s = xml.getElement("<Entity","/>");
		if (s.length() == 0) 
			error();
		Entity e;
		e.setFromXML(s);
		if (e.getEntityTypeName() != "FlagCmdTable")
			error();
		setEntity(e);
		// Skip the container's entity; but, it has to be there.
		s = xml.getElement("<ContainerEntity","/>");
		if (s.length() == 0) 
			error();

		// Get each row in the table.
		s = xml.getElementContent("<row>","</row>");
		FlagCmdRow *row;
		while (s.length() != 0) {
			// cout << "Parsing a FlagCmdRow" << endl; 
			row = newRow();
			row->setFromXML(s);
			try {
				checkAndAdd(row);
			} catch (DuplicateKey e1) {
				throw ConversionException(e1.getMessage(),"FlagCmdTable");
			} 
			catch (UniquenessViolationException e1) {
				throw ConversionException(e1.getMessage(),"FlagCmdTable");	
			}
			catch (...) {
				// cout << "Unexpected error in FlagCmdTable::checkAndAdd called from FlagCmdTable::fromXML " << endl;
			}
			s = xml.getElementContent("<row>","</row>");
		}
		if (!xml.isStr("</FlagCmdTable>")) 
			error();
	}

	void FlagCmdTable::error() throw(ConversionException) {
		throw ConversionException("Invalid xml document","FlagCmd");
	}
	
	string FlagCmdTable::toMIME() {
	 // To be implemented
		return "";
	}
	
	void FlagCmdTable::setFromMIME(const string & mimeMsg) {
		// To be implemented
		;
	}
	
	
	void FlagCmdTable::toFile(string directory) {
		if (!directoryExists(directory.c_str()) &&
			!createPath(directory.c_str())) {
			throw ConversionException("Could not create directory " , directory);
		}
		
		if (fileAsBin) {
			// write the bin serialized
			string fileName = directory + "/FlagCmd.bin";
			ofstream tableout(fileName.c_str(),ios::out|ios::trunc);
			if (tableout.rdstate() == ostream::failbit)
				throw ConversionException("Could not open file " + fileName + " to write ", "FlagCmd");
			tableout << toMIME() << endl;
			tableout.close();
			if (tableout.rdstate() == ostream::failbit)
				throw ConversionException("Could not close file " + fileName, "FlagCmd");
		}
		else {
			// write the XML
			string fileName = directory + "/FlagCmd.xml";
			ofstream tableout(fileName.c_str(),ios::out|ios::trunc);
			if (tableout.rdstate() == ostream::failbit)
				throw ConversionException("Could not open file " + fileName + " to write ", "FlagCmd");
			tableout << toXML() << endl;
			tableout.close();
			if (tableout.rdstate() == ostream::failbit)
				throw ConversionException("Could not close file " + fileName, "FlagCmd");
		}
	}
	
	void FlagCmdTable::setFromFile(const string& directory) {
		string tablename;
		if (fileAsBin)
			tablename = directory + "/FlagCmd.bin";
		else
			tablename = directory + "/FlagCmd.xml";
			
		// Determine the file size.
		ifstream::pos_type size;
		ifstream tablefile(tablename.c_str(), ios::in|ios::binary|ios::ate);

 		if (tablefile.is_open()) { 
  				size = tablefile.tellg(); 
  		}
		else {
				throw ConversionException("Could not open file " + tablename, "FlagCmd");
		}
		
		// Re position to the beginning.
		tablefile.seekg(0);
		
		// Read in a stringstream.
		stringstream ss;
		ss << tablefile.rdbuf();

		if (tablefile.rdstate() == istream::failbit || tablefile.rdstate() == istream::badbit) {
			throw ConversionException("Error reading file " + tablename,"FlagCmd");
		}

		// And close
		tablefile.close();
		if (tablefile.rdstate() == istream::failbit)
			throw ConversionException("Could not close file " + tablename,"FlagCmd");
					
		// And parse the content with the appropriate method
		if (fileAsBin) 
			setFromMIME(ss.str());
		else
			fromXML(ss.str());	
	}			
			
	
		
		
	/**
	 * Insert a FlagCmdRow* in a vector of FlagCmdRow* so that it's ordered by ascending start time.
	 *
	 * @param FlagCmdRow* x . The pointer to be inserted.
	 * @param vector <FlagCmdRow*>& row. A reference to the vector where to insert x.
	 *
	 */
	 FlagCmdRow* FlagCmdTable::insertByStartTime(FlagCmdRow* x, vector<FlagCmdRow*>& row) {
				
		vector <FlagCmdRow*>::iterator theIterator;
		
		ArrayTime start = x->timeInterval.getStart();

    	// Is the row vector empty ?
    	if (row.size() == 0) {
    		row.push_back(x);
    		privateRows.push_back(x);
    		x->isAdded();
    		return x;
    	}
    	
    	// Optimization for the case of insertion by ascending time.
    	FlagCmdRow* last = *(row.end()-1);
        
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
    	FlagCmdRow* first = *(row.begin());
        
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
					throw DuplicateKey("DuplicateKey exception in ", "FlagCmdTable");	
			}
			else if (start == row[k1]->timeInterval.getStart()) {
				if (row[k1]->equalByRequiredValue(x))
					return row[k1];
				else
					throw DuplicateKey("DuplicateKey exception in ", "FlagCmdTable");	
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
 
