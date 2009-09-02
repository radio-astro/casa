
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
 * File SourceParameterTable.cpp
 */
#include <ConversionException.h>
#include <DuplicateKey.h>
#include <OutOfBoundsException.h>

using asdm::ConversionException;
using asdm::DuplicateKey;
using asdm::OutOfBoundsException;

#include <ASDM.h>
#include <SourceParameterTable.h>
#include <SourceParameterRow.h>
#include <Parser.h>

using asdm::ASDM;
using asdm::SourceParameterTable;
using asdm::SourceParameterRow;
using asdm::Parser;

#include <iostream>
#include <sstream>
#include <set>
using namespace std;

#include <Misc.h>
using namespace asdm;


namespace asdm {

	string SourceParameterTable::tableName = "SourceParameter";
	

	/**
	 * The list of field names that make up key key.
	 * (Initialization is in the constructor.)
	 */
	vector<string> SourceParameterTable::key;

	/**
	 * Return the list of field names that make up key key
	 * as an array of strings.
	 */	
	vector<string> SourceParameterTable::getKeyName() {
		return key;
	}


	SourceParameterTable::SourceParameterTable(ASDM &c) : container(c) {

	
		key.push_back("sourceParameterId");
	
		key.push_back("sourceId");
	
		key.push_back("timeInterval");
	


		// Define a default entity.
		entity.setEntityId(EntityId("uid://X0/X0/X0"));
		entity.setEntityIdEncrypted("na");
		entity.setEntityTypeName("SourceParameterTable");
		entity.setEntityVersion("1");
		entity.setInstanceVersion("1");
		
		// Archive XML
		archiveAsBin = false;
		
		// File XML
		fileAsBin = false;
	}
	
/**
 * A destructor for SourceParameterTable.
 */
 
	SourceParameterTable::~SourceParameterTable() {
		for (unsigned int i = 0; i < privateRows.size(); i++) 
			delete(privateRows.at(i));
	}

	/**
	 * Container to which this table belongs.
	 */
	ASDM &SourceParameterTable::getContainer() const {
		return container;
	}

	/**
	 * Return the number of rows in the table.
	 */
	
	
	unsigned SourceParameterTable::size() {
/*
		int result = 0	;
		map<string, ID_TIME_ROWS >::iterator mapIter = context.begin();
		ID_TIME_ROWS::iterator planeIter;
		vector<SourceParameterRow*>::iterator rowIter; 
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
	string SourceParameterTable::getName() const {
		return tableName;
	}

	/**
	 * Return this table's Entity.
	 */
	Entity SourceParameterTable::getEntity() const {
		return entity;
	}

	/**
	 * Set this table's Entity.
	 */
	void SourceParameterTable::setEntity(Entity e) {
		this->entity = e; 
	}
	
	//
	// ====> Row creation.
	//
	
	/**
	 * Create a new row.
	 */
	SourceParameterRow *SourceParameterTable::newRow() {
		return new SourceParameterRow (*this);
	}
	
	SourceParameterRow *SourceParameterTable::newRowEmpty() {
		return newRow ();
	}


	/**
	 * Create a new row initialized to the specified values.
	 * @return a pointer on the created and initialized row.
	
 	 * @param sourceId. 
	
 	 * @param timeInterval. 
	
 	 * @param numFreq. 
	
 	 * @param numStokes. 
	
 	 * @param numDep. 
	
 	 * @param stokesParameter. 
	
 	 * @param flux. 
	
 	 * @param frequency. 
	
 	 * @param frequencyInterval. 
	
 	 * @param fluxErr. 
	
     */
	SourceParameterRow* SourceParameterTable::newRow(int sourceId, ArrayTimeInterval timeInterval, int numFreq, int numStokes, int numDep, vector<StokesParameterMod::StokesParameter > stokesParameter, vector<vector<Flux > > flux, vector<Frequency > frequency, vector<Frequency > frequencyInterval, vector<vector<Flux > > fluxErr){
		SourceParameterRow *row = new SourceParameterRow(*this);
			
		row->setSourceId(sourceId);
			
		row->setTimeInterval(timeInterval);
			
		row->setNumFreq(numFreq);
			
		row->setNumStokes(numStokes);
			
		row->setNumDep(numDep);
			
		row->setStokesParameter(stokesParameter);
			
		row->setFlux(flux);
			
		row->setFrequency(frequency);
			
		row->setFrequencyInterval(frequencyInterval);
			
		row->setFluxErr(fluxErr);
	
		return row;		
	}	

	SourceParameterRow* SourceParameterTable::newRowFull(int sourceId, ArrayTimeInterval timeInterval, int numFreq, int numStokes, int numDep, vector<StokesParameterMod::StokesParameter > stokesParameter, vector<vector<Flux > > flux, vector<Frequency > frequency, vector<Frequency > frequencyInterval, vector<vector<Flux > > fluxErr)	{
		SourceParameterRow *row = new SourceParameterRow(*this);
			
		row->setSourceId(sourceId);
			
		row->setTimeInterval(timeInterval);
			
		row->setNumFreq(numFreq);
			
		row->setNumStokes(numStokes);
			
		row->setNumDep(numDep);
			
		row->setStokesParameter(stokesParameter);
			
		row->setFlux(flux);
			
		row->setFrequency(frequency);
			
		row->setFrequencyInterval(frequencyInterval);
			
		row->setFluxErr(fluxErr);
	
		return row;				
	}
	


SourceParameterRow* SourceParameterTable::newRow(SourceParameterRow* row) {
	return new SourceParameterRow(*this, *row);
}

SourceParameterRow* SourceParameterTable::newRowCopy(SourceParameterRow* row) {
	return new SourceParameterRow(*this, *row);
}

	//
	// Append a row to its table.
	//

	
	
	
		
			

	/** 
	 * Returns a string built by concatenating the ascii representation of the
	 * parameters values suffixed with a "_" character.
	 */
	 string SourceParameterTable::Key(int sourceId) {
	 	ostringstream ostrstr;
	 		ostrstr  
			
				<<  sourceId  << "_"
			
			;
		return ostrstr.str();	 	
	 }

	/**
	 * Append a row to a SourceParameterTable which has simply 
	 * 1) an autoincrementable attribute  (sourceParameterId) 
	 * 2) a temporal attribute (timeInterval) in its key section.
	 * 3) other attributes in the key section (defining a so called context).
	 * If there is already a row in the table whose key section non including is equal to x's one and
	 * whose value section is equal to x's one then return this row, otherwise add x to the collection
	 * of rows.
	 */
	SourceParameterRow* SourceParameterTable::add(SourceParameterRow* x) {
		// Get the start time of the row to be inserted.
		ArrayTime startTime = x->getTimeInterval().getStart();
		// cout << "Trying to add a new row with start time = " << startTime << endl;
		int insertionId = 0;

		 
		// Determine the entry in the context map from the appropriates attributes.
		string k = Key(
						x->getSourceId()
					   );
					   

		// Determine the insertion index for the row x, possibly returning a pointer to a row identical to x. 					   
		if (context.find(k) != context.end()) {
			// cout << "The context " << k << " already exists " << endl;
			for (unsigned int i = 0; i < context[k].size(); i++) {
				//cout << "Looking for a same starttime in i = " << i << endl;
				for (unsigned int j=0; j<context[k][i].size(); j++) 
					if (context[k][i][j]->getTimeInterval().getStart().equals(startTime)) {
						if (
						
						 (context[k][i][j]->getNumFreq() == x->getNumFreq())
						 && 

						 (context[k][i][j]->getNumStokes() == x->getNumStokes())
						 && 

						 (context[k][i][j]->getNumDep() == x->getNumDep())
						 && 

						 (context[k][i][j]->getStokesParameter() == x->getStokesParameter())
						 && 

						 (context[k][i][j]->getFlux() == x->getFlux())
						 && 

						 (context[k][i][j]->getFrequency() == x->getFrequency())
						 && 

						 (context[k][i][j]->getFrequencyInterval() == x->getFrequencyInterval())
						 && 

						 (context[k][i][j]->getFluxErr() == x->getFluxErr())
						
						) {
							// cout << "A row equal to x has been found, I return it " << endl;
							return context[k][i][j];
						}
						
						// Otherwise we must autoincrement sourceParameterId and
						// insert a new SourceParameterRow with this autoincremented value.
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
		
		
			x->setSourceParameterId(insertionId);
		
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
	 */
	SourceParameterRow*  SourceParameterTable::checkAndAdd(SourceParameterRow* x) throw (DuplicateKey, UniquenessViolationException) {
		ArrayTime startTime = x->getTimeInterval().getStart();		
		
		// Determine the entry in the context map from the appropriate attributes.
		string k = Key(
		                x->getSourceId()
		               );

		// Uniqueness Rule Check
		if (context.find(k) != context.end()) {
			for (unsigned int i = 0;  i < context[k].size(); i++) 
				for (unsigned int j = 0; j < context[k][i].size(); j++)
					if (
						(context[k][i][j]->getTimeInterval().getStart().equals(startTime)) 
					
						 && (context[k][i][j]->getNumFreq() == x->getNumFreq())
					

						 && (context[k][i][j]->getNumStokes() == x->getNumStokes())
					

						 && (context[k][i][j]->getNumDep() == x->getNumDep())
					

						 && (context[k][i][j]->getStokesParameter() == x->getStokesParameter())
					

						 && (context[k][i][j]->getFlux() == x->getFlux())
					

						 && (context[k][i][j]->getFrequency() == x->getFrequency())
					

						 && (context[k][i][j]->getFrequencyInterval() == x->getFrequencyInterval())
					

						 && (context[k][i][j]->getFluxErr() == x->getFluxErr())
					
					)
						throw UniquenessViolationException("Uniqueness violation exception in table SourceParameterTable");			
		}


		// Good, now it's time to insert the row x, possibly triggering a DuplicateKey exception.	
		
		ID_TIME_ROWS dummyPlane;

		// Determine the integer representation of the identifier of the row (x) to be inserted. 
		int id = 
				x->getSourceParameterId();
				
	
		if (context.find(k) != context.end()) {
			if (id >= (int) context[k].size()) 
				context[k].resize(id+1);
			else {
				// This sourceParameterId 's value has already rows for this context.
				// Check that there is not yet a row with the same time. (simply check start time)
				// If there is such a row then trigger a Duplicate Key Exception.
				for (unsigned int j = 0; j < context[k][id].size(); j++)
					if (context[k][id][j]->getTimeInterval().getStart().equals(startTime))
						throw DuplicateKey("Duplicate key exception in ", "SourceParameterTable"); 
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
	 * @return Alls rows as an array of SourceParameterRow
	 */
	vector<SourceParameterRow *> SourceParameterTable::get()  {
		return privateRows;
	/*	
		vector<SourceParameterRow *> v;
		
		map<string, ID_TIME_ROWS >::iterator mapIter = context.begin();
		ID_TIME_ROWS::iterator planeIter;
		vector<SourceParameterRow*>::iterator rowIter; 
		for (mapIter=context.begin(); mapIter!=context.end(); mapIter++)
			for (planeIter=((*mapIter).second).begin(); planeIter != ((*mapIter).second).end(); planeIter++)
				for (rowIter=(*planeIter).begin(); rowIter != (*planeIter).end(); rowIter++)
					v.push_back(*rowIter);
		return v;
	*/
	}	
	


	
		
		
			
			 
/*
 ** Returns a SourceParameterRow* given a key.
 ** @return a pointer to the row having the key whose values are passed as parameters, or 0 if
 ** no row exists for that key.
 **
 */
 	SourceParameterRow* SourceParameterTable::getRowByKey(int sourceParameterId, int sourceId, ArrayTimeInterval timeInterval)  {	
		ArrayTime start = timeInterval.getStart();
		
		map<string, ID_TIME_ROWS >::iterator mapIter;
		if ((mapIter = context.find(Key(sourceId))) != context.end()) {
			
			int id = sourceParameterId;
			
			if (id < (int) ((*mapIter).second).size()) {
				vector <SourceParameterRow*>::iterator rowIter;
				for (rowIter = ((*mapIter).second)[id].begin(); rowIter != ((*mapIter).second)[id].end(); rowIter++) {
					if ((*rowIter)->getTimeInterval().contains(timeInterval))
						return *rowIter; 
				}
			}
		}
		return 0;
	}
/*
 * Returns a vector of pointers on rows whose key element sourceParameterId 
 * is equal to the parameter sourceParameterId.
 * @return a vector of vector <SourceParameterRow *>. A returned vector of size 0 means that no row has been found.
 * @param sourceParameterId int contains the value of
 * the autoincrementable attribute that is looked up in the table.
 */
 vector <SourceParameterRow *>  SourceParameterTable::getRowBySourceParameterId(int sourceParameterId) {
	vector<SourceParameterRow *> list;
	map<string, ID_TIME_ROWS >::iterator mapIter;
	
	for (mapIter=context.begin(); mapIter!=context.end(); mapIter++) {
		int maxId = ((*mapIter).second).size();
		if (sourceParameterId < maxId) {
			vector<SourceParameterRow *>::iterator rowIter;
			for (rowIter=((*mapIter).second)[sourceParameterId].begin(); 
			     rowIter!=((*mapIter).second)[sourceParameterId].end(); rowIter++)
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
			
 * @param <<ExtrinsicAttribute>> sourceId.
 	 		
 * @param <<ASDMAttribute>> timeInterval.
 	 		
 * @param <<ASDMAttribute>> numFreq.
 	 		
 * @param <<ASDMAttribute>> numStokes.
 	 		
 * @param <<ASDMAttribute>> numDep.
 	 		
 * @param <<ArrayAttribute>> stokesParameter.
 	 		
 * @param <<ArrayAttribute>> flux.
 	 		
 * @param <<ArrayAttribute>> frequency.
 	 		
 * @param <<ArrayAttribute>> frequencyInterval.
 	 		
 * @param <<ArrayAttribute>> fluxErr.
 	 		 
 */
SourceParameterRow* SourceParameterTable::lookup(int sourceId, ArrayTimeInterval timeInterval, int numFreq, int numStokes, int numDep, vector<StokesParameterMod::StokesParameter > stokesParameter, vector<vector<Flux > > flux, vector<Frequency > frequency, vector<Frequency > frequencyInterval, vector<vector<Flux > > fluxErr) {		
		using asdm::ArrayTimeInterval;
		map<string, ID_TIME_ROWS >::iterator mapIter;
		string k = Key(sourceId);
		if ((mapIter = context.find(k)) != context.end()) {
			ID_TIME_ROWS::iterator planeIter;
			for (planeIter = context[k].begin(); planeIter != context[k].end(); planeIter++)  {
				vector <SourceParameterRow*>::iterator rowIter;
				for (rowIter = (*planeIter).begin(); rowIter != (*planeIter).end(); rowIter++) {
					if ((*rowIter)->getTimeInterval().contains(timeInterval)
					    && (*rowIter)->compareRequiredValue(numFreq, numStokes, numDep, stokesParameter, flux, frequency, frequencyInterval, fluxErr)) {
						return *rowIter;
					} 
				}
			}
		}				
		return 0;	
} 
		
	




#ifndef WITHOUT_ACS
	// Conversion Methods

	SourceParameterTableIDL *SourceParameterTable::toIDL() {
		SourceParameterTableIDL *x = new SourceParameterTableIDL ();
		unsigned int nrow = size();
		x->row.length(nrow);
		vector<SourceParameterRow*> v = get();
		for (unsigned int i = 0; i < nrow; ++i) {
			x->row[i] = *(v[i]->toIDL());
		}
		return x;
	}
#endif
	
#ifndef WITHOUT_ACS
	void SourceParameterTable::fromIDL(SourceParameterTableIDL x) throw(DuplicateKey,ConversionException) {
		unsigned int nrow = x.row.length();
		for (unsigned int i = 0; i < nrow; ++i) {
			SourceParameterRow *tmp = newRow();
			tmp->setFromIDL(x.row[i]);
			// checkAndAdd(tmp);
			add(tmp);
		}
	}
#endif

	char *SourceParameterTable::toFITS() const throw(ConversionException) {
		throw ConversionException("Not implemented","SourceParameter");
	}

	void SourceParameterTable::fromFITS(char *fits) throw(ConversionException) {
		throw ConversionException("Not implemented","SourceParameter");
	}

	string SourceParameterTable::toVOTable() const throw(ConversionException) {
		throw ConversionException("Not implemented","SourceParameter");
	}

	void SourceParameterTable::fromVOTable(string vo) throw(ConversionException) {
		throw ConversionException("Not implemented","SourceParameter");
	}

	string SourceParameterTable::toXML()  throw(ConversionException) {
		string buf;
		buf.append("<?xml version=\"1.0\" encoding=\"ISO-8859-1\"?> ");
//		buf.append("<SourceParameterTable xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" xsi:noNamespaceSchemaLocation=\"../../idl/SourceParameterTable.xsd\"> ");
		buf.append("<?xml-stylesheet type=\"text/xsl\" href=\"../asdm2html/table2html.xsl\"?> ");		
		buf.append("<SourceParameterTable> ");
		buf.append(entity.toXML());
		string s = container.getEntity().toXML();
		// Change the "Entity" tag to "ContainerEntity".
		buf.append("<Container" + s.substr(1,s.length() - 1)+" ");
		vector<SourceParameterRow*> v = get();
		for (unsigned int i = 0; i < v.size(); ++i) {
			try {
				buf.append(v[i]->toXML());
			} catch (NoSuchRow e) {
			}
			buf.append("  ");
		}		
		buf.append("</SourceParameterTable> ");
		return buf;
	}
	
	void SourceParameterTable::fromXML(string xmlDoc) throw(ConversionException) {
		Parser xml(xmlDoc);
		if (!xml.isStr("<SourceParameterTable")) 
			error();
		// cout << "Parsing a SourceParameterTable" << endl;
		string s = xml.getElement("<Entity","/>");
		if (s.length() == 0) 
			error();
		Entity e;
		e.setFromXML(s);
		if (e.getEntityTypeName() != "SourceParameterTable")
			error();
		setEntity(e);
		// Skip the container's entity; but, it has to be there.
		s = xml.getElement("<ContainerEntity","/>");
		if (s.length() == 0) 
			error();

		// Get each row in the table.
		s = xml.getElementContent("<row>","</row>");
		SourceParameterRow *row;
		while (s.length() != 0) {
			// cout << "Parsing a SourceParameterRow" << endl; 
			row = newRow();
			row->setFromXML(s);
			try {
				checkAndAdd(row);
			} catch (DuplicateKey e1) {
				throw ConversionException(e1.getMessage(),"SourceParameterTable");
			} 
			catch (UniquenessViolationException e1) {
				throw ConversionException(e1.getMessage(),"SourceParameterTable");	
			}
			catch (...) {
				// cout << "Unexpected error in SourceParameterTable::checkAndAdd called from SourceParameterTable::fromXML " << endl;
			}
			s = xml.getElementContent("<row>","</row>");
		}
		if (!xml.isStr("</SourceParameterTable>")) 
			error();
	}

	void SourceParameterTable::error() throw(ConversionException) {
		throw ConversionException("Invalid xml document","SourceParameter");
	}
	
	string SourceParameterTable::toMIME() {
	 // To be implemented
		return "";
	}
	
	void SourceParameterTable::setFromMIME(const string & mimeMsg) {
		// To be implemented
		;
	}
	
	
	void SourceParameterTable::toFile(string directory) {
		if (!directoryExists(directory.c_str()) &&
			!createPath(directory.c_str())) {
			throw ConversionException("Could not create directory " , directory);
		}
		
		if (fileAsBin) {
			// write the bin serialized
			string fileName = directory + "/SourceParameter.bin";
			ofstream tableout(fileName.c_str(),ios::out|ios::trunc);
			if (tableout.rdstate() == ostream::failbit)
				throw ConversionException("Could not open file " + fileName + " to write ", "SourceParameter");
			tableout << toMIME() << endl;
			tableout.close();
			if (tableout.rdstate() == ostream::failbit)
				throw ConversionException("Could not close file " + fileName, "SourceParameter");
		}
		else {
			// write the XML
			string fileName = directory + "/SourceParameter.xml";
			ofstream tableout(fileName.c_str(),ios::out|ios::trunc);
			if (tableout.rdstate() == ostream::failbit)
				throw ConversionException("Could not open file " + fileName + " to write ", "SourceParameter");
			tableout << toXML() << endl;
			tableout.close();
			if (tableout.rdstate() == ostream::failbit)
				throw ConversionException("Could not close file " + fileName, "SourceParameter");
		}
	}
	
	void SourceParameterTable::setFromFile(const string& directory) {
		string tablename;
		if (fileAsBin)
			tablename = directory + "/SourceParameter.bin";
		else
			tablename = directory + "/SourceParameter.xml";
			
		// Determine the file size.
		ifstream::pos_type size;
		ifstream tablefile(tablename.c_str(), ios::in|ios::binary|ios::ate);

 		if (tablefile.is_open()) { 
  				size = tablefile.tellg(); 
  		}
		else {
				throw ConversionException("Could not open file " + tablename, "SourceParameter");
		}
		
		// Re position to the beginning.
		tablefile.seekg(0);
		
		// Read in a stringstream.
		stringstream ss;
		ss << tablefile.rdbuf();

		if (tablefile.rdstate() == istream::failbit || tablefile.rdstate() == istream::badbit) {
			throw ConversionException("Error reading file " + tablename,"SourceParameter");
		}

		// And close
		tablefile.close();
		if (tablefile.rdstate() == istream::failbit)
			throw ConversionException("Could not close file " + tablename,"SourceParameter");
					
		// And parse the content with the appropriate method
		if (fileAsBin) 
			setFromMIME(ss.str());
		else
			fromXML(ss.str());	
	}			
			
	
		
		
	/**
	 * Insert a SourceParameterRow* in a vector of SourceParameterRow* so that it's ordered by ascending start time.
	 *
	 * @param SourceParameterRow* x . The pointer to be inserted.
	 * @param vector <SourceParameterRow*>& row. A reference to the vector where to insert x.
	 *
	 */
	 SourceParameterRow* SourceParameterTable::insertByStartTime(SourceParameterRow* x, vector<SourceParameterRow*>& row) {
				
		vector <SourceParameterRow*>::iterator theIterator;
		
		ArrayTime start = x->timeInterval.getStart();

    	// Is the row vector empty ?
    	if (row.size() == 0) {
    		row.push_back(x);
    		privateRows.push_back(x);
    		x->isAdded();
    		return x;
    	}
    	
    	// Optimization for the case of insertion by ascending time.
    	SourceParameterRow* last = *(row.end()-1);
        
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
    	SourceParameterRow* first = *(row.begin());
        
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
					throw DuplicateKey("DuplicateKey exception in ", "SourceParameterTable");	
			}
			else if (start == row[k1]->timeInterval.getStart()) {
				if (row[k1]->equalByRequiredValue(x))
					return row[k1];
				else
					throw DuplicateKey("DuplicateKey exception in ", "SourceParameterTable");	
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
 
