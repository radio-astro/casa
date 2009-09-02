
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
 * File FreqOffsetTable.cpp
 */
#include <ConversionException.h>
#include <DuplicateKey.h>
#include <OutOfBoundsException.h>

using asdm::ConversionException;
using asdm::DuplicateKey;
using asdm::OutOfBoundsException;

#include <ASDM.h>
#include <FreqOffsetTable.h>
#include <FreqOffsetRow.h>
#include <Parser.h>

using asdm::ASDM;
using asdm::FreqOffsetTable;
using asdm::FreqOffsetRow;
using asdm::Parser;

#include <iostream>
#include <sstream>
#include <set>
using namespace std;

#include <Misc.h>
using namespace asdm;


namespace asdm {

	string FreqOffsetTable::tableName = "FreqOffset";
	

	/**
	 * The list of field names that make up key key.
	 * (Initialization is in the constructor.)
	 */
	vector<string> FreqOffsetTable::key;

	/**
	 * Return the list of field names that make up key key
	 * as an array of strings.
	 */	
	vector<string> FreqOffsetTable::getKeyName() {
		return key;
	}


	FreqOffsetTable::FreqOffsetTable(ASDM &c) : container(c) {

	
		key.push_back("antennaId");
	
		key.push_back("feedId");
	
		key.push_back("spectralWindowId");
	
		key.push_back("timeInterval");
	


		// Define a default entity.
		entity.setEntityId(EntityId("uid://X0/X0/X0"));
		entity.setEntityIdEncrypted("na");
		entity.setEntityTypeName("FreqOffsetTable");
		entity.setEntityVersion("1");
		entity.setInstanceVersion("1");
		
		// Archive XML
		archiveAsBin = false;
		
		// File XML
		fileAsBin = false;
	}
	
/**
 * A destructor for FreqOffsetTable.
 */
 
	FreqOffsetTable::~FreqOffsetTable() {
		for (unsigned int i = 0; i < privateRows.size(); i++) 
			delete(privateRows.at(i));
	}

	/**
	 * Container to which this table belongs.
	 */
	ASDM &FreqOffsetTable::getContainer() const {
		return container;
	}

	/**
	 * Return the number of rows in the table.
	 */
	
	
		
	unsigned int FreqOffsetTable::size() {
		int result = 0;
		
		map<string, TIME_ROWS >::iterator mapIter;
		for (mapIter=context.begin(); mapIter!=context.end(); mapIter++) 
			result += ((*mapIter).second).size();
			
		return result;
	}	
		
	
	
	
	/**
	 * Return the name of this table.
	 */
	string FreqOffsetTable::getName() const {
		return tableName;
	}

	/**
	 * Return this table's Entity.
	 */
	Entity FreqOffsetTable::getEntity() const {
		return entity;
	}

	/**
	 * Set this table's Entity.
	 */
	void FreqOffsetTable::setEntity(Entity e) {
		this->entity = e; 
	}
	
	//
	// ====> Row creation.
	//
	
	/**
	 * Create a new row.
	 */
	FreqOffsetRow *FreqOffsetTable::newRow() {
		return new FreqOffsetRow (*this);
	}
	
	FreqOffsetRow *FreqOffsetTable::newRowEmpty() {
		return newRow ();
	}


	/**
	 * Create a new row initialized to the specified values.
	 * @return a pointer on the created and initialized row.
	
 	 * @param antennaId. 
	
 	 * @param feedId. 
	
 	 * @param spectralWindowId. 
	
 	 * @param timeInterval. 
	
 	 * @param offset. 
	
     */
	FreqOffsetRow* FreqOffsetTable::newRow(Tag antennaId, int feedId, Tag spectralWindowId, ArrayTimeInterval timeInterval, Frequency offset){
		FreqOffsetRow *row = new FreqOffsetRow(*this);
			
		row->setAntennaId(antennaId);
			
		row->setFeedId(feedId);
			
		row->setSpectralWindowId(spectralWindowId);
			
		row->setTimeInterval(timeInterval);
			
		row->setOffset(offset);
	
		return row;		
	}	

	FreqOffsetRow* FreqOffsetTable::newRowFull(Tag antennaId, int feedId, Tag spectralWindowId, ArrayTimeInterval timeInterval, Frequency offset)	{
		FreqOffsetRow *row = new FreqOffsetRow(*this);
			
		row->setAntennaId(antennaId);
			
		row->setFeedId(feedId);
			
		row->setSpectralWindowId(spectralWindowId);
			
		row->setTimeInterval(timeInterval);
			
		row->setOffset(offset);
	
		return row;				
	}
	


FreqOffsetRow* FreqOffsetTable::newRow(FreqOffsetRow* row) {
	return new FreqOffsetRow(*this, *row);
}

FreqOffsetRow* FreqOffsetTable::newRowCopy(FreqOffsetRow* row) {
	return new FreqOffsetRow(*this, *row);
}

	//
	// Append a row to its table.
	//

	
	
		
		
	/** 
	 * Returns a string built by concatenating the ascii representation of the
	 * parameters values suffixed with a "_" character.
	 */
	 string FreqOffsetTable::Key(Tag antennaId, int feedId, Tag spectralWindowId) {
	 	ostringstream ostrstr;
	 		ostrstr  
			
				<<  antennaId.toString()  << "_"
			
				<<   feedId  << "_"
			
				<<  spectralWindowId.toString()  << "_"
			
			;
		return ostrstr.str();	 	
	 }
	 
			
			
	FreqOffsetRow* FreqOffsetTable::add(FreqOffsetRow* x) {
		ArrayTime startTime = x->getTimeInterval().getStart();

		/*
	 	 * Is there already a context for this combination of not temporal 
	 	 * attributes ?
	 	 */
		string k = Key(
						x->getAntennaId()
					   ,
						x->getFeedId()
					   ,
						x->getSpectralWindowId()
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

	
	
		
		
			
			
			
			
	FreqOffsetRow*  FreqOffsetTable::checkAndAdd(FreqOffsetRow* x) throw (DuplicateKey) {
		string keystr = Key( 
						x->getAntennaId() 
					   , 
						x->getFeedId() 
					   , 
						x->getSpectralWindowId() 
					   ); 
		if (context.find(keystr) == context.end()) {
			vector<FreqOffsetRow *> v;
			context[keystr] = v;
		}
		
		vector<FreqOffsetRow*>& found = context.find(keystr)->second;
		return insertByStartTime(x, found);			
	}
			
					
		







	

	
	
		
	/**
	 * Get all rows.
	 * @return Alls rows as an array of FreqOffsetRow
	 */
	 vector<FreqOffsetRow *> FreqOffsetTable::get() {
	    return privateRows;
	    
	 /*
	 	vector<FreqOffsetRow *> v;
	 	map<string, TIME_ROWS>::iterator mapIter;
	 	vector<FreqOffsetRow *>::iterator rowIter;
	 	
	 	for (mapIter=context.begin(); mapIter!=context.end(); mapIter++) {
	 		for (rowIter=((*mapIter).second).begin(); rowIter!=((*mapIter).second).end(); rowIter++) 
	 			v.push_back(*rowIter); 
	 	}
	 	
	 	return v;
	 */
	 }
	 
	 vector<FreqOffsetRow *> *FreqOffsetTable::getByContext(Tag antennaId, int feedId, Tag spectralWindowId) {
	  	string k = Key(antennaId, feedId, spectralWindowId);
 
	    if (context.find(k) == context.end()) return 0;
 	   else return &(context[k]);		
	}		
		
	


	
		
		
			
			
			
/*
 ** Returns a FreqOffsetRow* given a key.
 ** @return a pointer to the row having the key whose values are passed as parameters, or 0 if
 ** no row exists for that key.
 **
 */
 				
				
	FreqOffsetRow* FreqOffsetTable::getRowByKey(Tag antennaId, int feedId, Tag spectralWindowId, ArrayTimeInterval timeInterval)  {
 		string keystr = Key(antennaId, feedId, spectralWindowId);
 		vector<FreqOffsetRow *> row;
 		
 		if ( context.find(keystr)  == context.end()) return 0;
 		
 		row = context[keystr];
 		
 		// Is the vector empty...impossible in principle !
 		if (row.size() == 0) return 0;
 		
 		// Only one element in the vector
 		if (row.size() == 1) {
 			FreqOffsetRow* r = row.at(0);
 			if ( r->getTimeInterval().contains(timeInterval.getStart()))
 				return r;
 			else
 				return 0;
 		}
 		
 		// Optimizations
 		FreqOffsetRow* last = row.at(row.size()-1);
 		if (timeInterval.getStart().get() >= (last->getTimeInterval().getStart().get()+last->getTimeInterval().getDuration().get())) return 0;
 		
 		FreqOffsetRow* first = row.at(0);
 		if (timeInterval.getStart().get() < first->getTimeInterval().getStart().get()) return 0;
 		
 		
 		// More than one row 
 		// Let's use a dichotomy method for the general case..	
 		int k0 = 0;
 		int k1 = row.size() - 1;
 		FreqOffsetRow* r = 0;
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

	FreqOffsetTableIDL *FreqOffsetTable::toIDL() {
		FreqOffsetTableIDL *x = new FreqOffsetTableIDL ();
		unsigned int nrow = size();
		x->row.length(nrow);
		vector<FreqOffsetRow*> v = get();
		for (unsigned int i = 0; i < nrow; ++i) {
			x->row[i] = *(v[i]->toIDL());
		}
		return x;
	}
#endif
	
#ifndef WITHOUT_ACS
	void FreqOffsetTable::fromIDL(FreqOffsetTableIDL x) throw(DuplicateKey,ConversionException) {
		unsigned int nrow = x.row.length();
		for (unsigned int i = 0; i < nrow; ++i) {
			FreqOffsetRow *tmp = newRow();
			tmp->setFromIDL(x.row[i]);
			// checkAndAdd(tmp);
			add(tmp);
		}
	}
#endif

	char *FreqOffsetTable::toFITS() const throw(ConversionException) {
		throw ConversionException("Not implemented","FreqOffset");
	}

	void FreqOffsetTable::fromFITS(char *fits) throw(ConversionException) {
		throw ConversionException("Not implemented","FreqOffset");
	}

	string FreqOffsetTable::toVOTable() const throw(ConversionException) {
		throw ConversionException("Not implemented","FreqOffset");
	}

	void FreqOffsetTable::fromVOTable(string vo) throw(ConversionException) {
		throw ConversionException("Not implemented","FreqOffset");
	}

	string FreqOffsetTable::toXML()  throw(ConversionException) {
		string buf;
		buf.append("<?xml version=\"1.0\" encoding=\"ISO-8859-1\"?> ");
//		buf.append("<FreqOffsetTable xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" xsi:noNamespaceSchemaLocation=\"../../idl/FreqOffsetTable.xsd\"> ");
		buf.append("<?xml-stylesheet type=\"text/xsl\" href=\"../asdm2html/table2html.xsl\"?> ");		
		buf.append("<FreqOffsetTable> ");
		buf.append(entity.toXML());
		string s = container.getEntity().toXML();
		// Change the "Entity" tag to "ContainerEntity".
		buf.append("<Container" + s.substr(1,s.length() - 1)+" ");
		vector<FreqOffsetRow*> v = get();
		for (unsigned int i = 0; i < v.size(); ++i) {
			try {
				buf.append(v[i]->toXML());
			} catch (NoSuchRow e) {
			}
			buf.append("  ");
		}		
		buf.append("</FreqOffsetTable> ");
		return buf;
	}
	
	void FreqOffsetTable::fromXML(string xmlDoc) throw(ConversionException) {
		Parser xml(xmlDoc);
		if (!xml.isStr("<FreqOffsetTable")) 
			error();
		// cout << "Parsing a FreqOffsetTable" << endl;
		string s = xml.getElement("<Entity","/>");
		if (s.length() == 0) 
			error();
		Entity e;
		e.setFromXML(s);
		if (e.getEntityTypeName() != "FreqOffsetTable")
			error();
		setEntity(e);
		// Skip the container's entity; but, it has to be there.
		s = xml.getElement("<ContainerEntity","/>");
		if (s.length() == 0) 
			error();

		// Get each row in the table.
		s = xml.getElementContent("<row>","</row>");
		FreqOffsetRow *row;
		while (s.length() != 0) {
			// cout << "Parsing a FreqOffsetRow" << endl; 
			row = newRow();
			row->setFromXML(s);
			try {
				checkAndAdd(row);
			} catch (DuplicateKey e1) {
				throw ConversionException(e1.getMessage(),"FreqOffsetTable");
			} 
			catch (UniquenessViolationException e1) {
				throw ConversionException(e1.getMessage(),"FreqOffsetTable");	
			}
			catch (...) {
				// cout << "Unexpected error in FreqOffsetTable::checkAndAdd called from FreqOffsetTable::fromXML " << endl;
			}
			s = xml.getElementContent("<row>","</row>");
		}
		if (!xml.isStr("</FreqOffsetTable>")) 
			error();
	}

	void FreqOffsetTable::error() throw(ConversionException) {
		throw ConversionException("Invalid xml document","FreqOffset");
	}
	
	string FreqOffsetTable::toMIME() {
	 // To be implemented
		return "";
	}
	
	void FreqOffsetTable::setFromMIME(const string & mimeMsg) {
		// To be implemented
		;
	}
	
	
	void FreqOffsetTable::toFile(string directory) {
		if (!directoryExists(directory.c_str()) &&
			!createPath(directory.c_str())) {
			throw ConversionException("Could not create directory " , directory);
		}
		
		if (fileAsBin) {
			// write the bin serialized
			string fileName = directory + "/FreqOffset.bin";
			ofstream tableout(fileName.c_str(),ios::out|ios::trunc);
			if (tableout.rdstate() == ostream::failbit)
				throw ConversionException("Could not open file " + fileName + " to write ", "FreqOffset");
			tableout << toMIME() << endl;
			tableout.close();
			if (tableout.rdstate() == ostream::failbit)
				throw ConversionException("Could not close file " + fileName, "FreqOffset");
		}
		else {
			// write the XML
			string fileName = directory + "/FreqOffset.xml";
			ofstream tableout(fileName.c_str(),ios::out|ios::trunc);
			if (tableout.rdstate() == ostream::failbit)
				throw ConversionException("Could not open file " + fileName + " to write ", "FreqOffset");
			tableout << toXML() << endl;
			tableout.close();
			if (tableout.rdstate() == ostream::failbit)
				throw ConversionException("Could not close file " + fileName, "FreqOffset");
		}
	}
	
	void FreqOffsetTable::setFromFile(const string& directory) {
		string tablename;
		if (fileAsBin)
			tablename = directory + "/FreqOffset.bin";
		else
			tablename = directory + "/FreqOffset.xml";
			
		// Determine the file size.
		ifstream::pos_type size;
		ifstream tablefile(tablename.c_str(), ios::in|ios::binary|ios::ate);

 		if (tablefile.is_open()) { 
  				size = tablefile.tellg(); 
  		}
		else {
				throw ConversionException("Could not open file " + tablename, "FreqOffset");
		}
		
		// Re position to the beginning.
		tablefile.seekg(0);
		
		// Read in a stringstream.
		stringstream ss;
		ss << tablefile.rdbuf();

		if (tablefile.rdstate() == istream::failbit || tablefile.rdstate() == istream::badbit) {
			throw ConversionException("Error reading file " + tablename,"FreqOffset");
		}

		// And close
		tablefile.close();
		if (tablefile.rdstate() == istream::failbit)
			throw ConversionException("Could not close file " + tablename,"FreqOffset");
					
		// And parse the content with the appropriate method
		if (fileAsBin) 
			setFromMIME(ss.str());
		else
			fromXML(ss.str());	
	}			
			
	
		
		
	/**
	 * Insert a FreqOffsetRow* in a vector of FreqOffsetRow* so that it's ordered by ascending start time.
	 *
	 * @param FreqOffsetRow* x . The pointer to be inserted.
	 * @param vector <FreqOffsetRow*>& row. A reference to the vector where to insert x.
	 *
	 */
	 FreqOffsetRow* FreqOffsetTable::insertByStartTime(FreqOffsetRow* x, vector<FreqOffsetRow*>& row) {
				
		vector <FreqOffsetRow*>::iterator theIterator;
		
		ArrayTime start = x->timeInterval.getStart();

    	// Is the row vector empty ?
    	if (row.size() == 0) {
    		row.push_back(x);
    		privateRows.push_back(x);
    		x->isAdded();
    		return x;
    	}
    	
    	// Optimization for the case of insertion by ascending time.
    	FreqOffsetRow* last = *(row.end()-1);
        
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
    	FreqOffsetRow* first = *(row.begin());
        
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
					throw DuplicateKey("DuplicateKey exception in ", "FreqOffsetTable");	
			}
			else if (start == row[k1]->timeInterval.getStart()) {
				if (row[k1]->equalByRequiredValue(x))
					return row[k1];
				else
					throw DuplicateKey("DuplicateKey exception in ", "FreqOffsetTable");	
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
 
