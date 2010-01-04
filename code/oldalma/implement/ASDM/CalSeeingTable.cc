
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
 * File CalSeeingTable.cpp
 */
#include <ConversionException.h>
#include <DuplicateKey.h>
#include <OutOfBoundsException.h>

using asdm::ConversionException;
using asdm::DuplicateKey;
using asdm::OutOfBoundsException;

#include <ASDM.h>
#include <CalSeeingTable.h>
#include <CalSeeingRow.h>
#include <Parser.h>

using asdm::ASDM;
using asdm::CalSeeingTable;
using asdm::CalSeeingRow;
using asdm::Parser;

#include <iostream>
#include <sstream>
#include <set>
using namespace std;

#include <Misc.h>
using namespace asdm;


namespace asdm {

	string CalSeeingTable::tableName = "CalSeeing";
	

	/**
	 * The list of field names that make up key key.
	 * (Initialization is in the constructor.)
	 */
	vector<string> CalSeeingTable::key;

	/**
	 * Return the list of field names that make up key key
	 * as an array of strings.
	 */	
	vector<string> CalSeeingTable::getKeyName() {
		return key;
	}


	CalSeeingTable::CalSeeingTable(ASDM &c) : container(c) {

	
		key.push_back("calDataId");
	
		key.push_back("calReductionId");
	


		// Define a default entity.
		entity.setEntityId(EntityId("uid://X0/X0/X0"));
		entity.setEntityIdEncrypted("na");
		entity.setEntityTypeName("CalSeeingTable");
		entity.setEntityVersion("1");
		entity.setInstanceVersion("1");
		
		// Archive XML
		archiveAsBin = false;
		
		// File XML
		fileAsBin = false;
	}
	
/**
 * A destructor for CalSeeingTable.
 */
 
	CalSeeingTable::~CalSeeingTable() {
		for (unsigned int i = 0; i < privateRows.size(); i++) 
			delete(privateRows.at(i));
	}

	/**
	 * Container to which this table belongs.
	 */
	ASDM &CalSeeingTable::getContainer() const {
		return container;
	}

	/**
	 * Return the number of rows in the table.
	 */

	unsigned int CalSeeingTable::size() {
		return row.size();
	}	
	
	
	/**
	 * Return the name of this table.
	 */
	string CalSeeingTable::getName() const {
		return tableName;
	}

	/**
	 * Return this table's Entity.
	 */
	Entity CalSeeingTable::getEntity() const {
		return entity;
	}

	/**
	 * Set this table's Entity.
	 */
	void CalSeeingTable::setEntity(Entity e) {
		this->entity = e; 
	}
	
	//
	// ====> Row creation.
	//
	
	/**
	 * Create a new row.
	 */
	CalSeeingRow *CalSeeingTable::newRow() {
		return new CalSeeingRow (*this);
	}
	
	CalSeeingRow *CalSeeingTable::newRowEmpty() {
		return newRow ();
	}


	/**
	 * Create a new row initialized to the specified values.
	 * @return a pointer on the created and initialized row.
	
 	 * @param calDataId. 
	
 	 * @param calReductionId. 
	
 	 * @param numBaseLength. 
	
 	 * @param startValidTime. 
	
 	 * @param endValidTime. 
	
 	 * @param frequencyRange. 
	
 	 * @param baseLength. 
	
 	 * @param corrPhaseRms. 
	
 	 * @param uncorrPhaseRms. 
	
 	 * @param seeing. 
	
 	 * @param seeingFrequency. 
	
 	 * @param seeingFreqBandwidth. 
	
     */
	CalSeeingRow* CalSeeingTable::newRow(Tag calDataId, Tag calReductionId, int numBaseLength, ArrayTime startValidTime, ArrayTime endValidTime, vector<Frequency > frequencyRange, vector<Length > baseLength, vector<Angle > corrPhaseRms, vector<Angle > uncorrPhaseRms, Angle seeing, Frequency seeingFrequency, Frequency seeingFreqBandwidth){
		CalSeeingRow *row = new CalSeeingRow(*this);
			
		row->setCalDataId(calDataId);
			
		row->setCalReductionId(calReductionId);
			
		row->setNumBaseLength(numBaseLength);
			
		row->setStartValidTime(startValidTime);
			
		row->setEndValidTime(endValidTime);
			
		row->setFrequencyRange(frequencyRange);
			
		row->setBaseLength(baseLength);
			
		row->setCorrPhaseRms(corrPhaseRms);
			
		row->setUncorrPhaseRms(uncorrPhaseRms);
			
		row->setSeeing(seeing);
			
		row->setSeeingFrequency(seeingFrequency);
			
		row->setSeeingFreqBandwidth(seeingFreqBandwidth);
	
		return row;		
	}	

	CalSeeingRow* CalSeeingTable::newRowFull(Tag calDataId, Tag calReductionId, int numBaseLength, ArrayTime startValidTime, ArrayTime endValidTime, vector<Frequency > frequencyRange, vector<Length > baseLength, vector<Angle > corrPhaseRms, vector<Angle > uncorrPhaseRms, Angle seeing, Frequency seeingFrequency, Frequency seeingFreqBandwidth)	{
		CalSeeingRow *row = new CalSeeingRow(*this);
			
		row->setCalDataId(calDataId);
			
		row->setCalReductionId(calReductionId);
			
		row->setNumBaseLength(numBaseLength);
			
		row->setStartValidTime(startValidTime);
			
		row->setEndValidTime(endValidTime);
			
		row->setFrequencyRange(frequencyRange);
			
		row->setBaseLength(baseLength);
			
		row->setCorrPhaseRms(corrPhaseRms);
			
		row->setUncorrPhaseRms(uncorrPhaseRms);
			
		row->setSeeing(seeing);
			
		row->setSeeingFrequency(seeingFrequency);
			
		row->setSeeingFreqBandwidth(seeingFreqBandwidth);
	
		return row;				
	}
	


CalSeeingRow* CalSeeingTable::newRow(CalSeeingRow* row) {
	return new CalSeeingRow(*this, *row);
}

CalSeeingRow* CalSeeingTable::newRowCopy(CalSeeingRow* row) {
	return new CalSeeingRow(*this, *row);
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
	CalSeeingRow* CalSeeingTable::add(CalSeeingRow* x) {
		
		if (getRowByKey(
						x->getCalDataId()
						,
						x->getCalReductionId()
						))
			//throw DuplicateKey(x.getCalDataId() + "|" + x.getCalReductionId(),"CalSeeing");
			throw DuplicateKey("Duplicate key exception in ","CalSeeingTable");
		
		row.push_back(x);
		privateRows.push_back(x);
		x->isAdded();
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
	 */
	CalSeeingRow*  CalSeeingTable::checkAndAdd(CalSeeingRow* x) throw (DuplicateKey) {
		
		
		if (getRowByKey(
	
			x->getCalDataId()
	,
			x->getCalReductionId()
			
		)) throw DuplicateKey("Duplicate key exception in ", "CalSeeingTable");
		
		row.push_back(x);
		privateRows.push_back(x);
		x->isAdded();
		return x;	
	}	







	

	//
	// ====> Methods returning rows.
	//	
	/**
	 * Get all rows.
	 * @return Alls rows as an array of CalSeeingRow
	 */
	vector<CalSeeingRow *> CalSeeingTable::get() {
		return privateRows;
		// return row;
	}

	
/*
 ** Returns a CalSeeingRow* given a key.
 ** @return a pointer to the row having the key whose values are passed as parameters, or 0 if
 ** no row exists for that key.
 **
 */
 	CalSeeingRow* CalSeeingTable::getRowByKey(Tag calDataId, Tag calReductionId)  {
	CalSeeingRow* aRow = 0;
	for (unsigned int i = 0; i < row.size(); i++) {
		aRow = row.at(i);
		
			
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
			
 * @param calDataId.
 	 		
 * @param calReductionId.
 	 		
 * @param numBaseLength.
 	 		
 * @param startValidTime.
 	 		
 * @param endValidTime.
 	 		
 * @param frequencyRange.
 	 		
 * @param baseLength.
 	 		
 * @param corrPhaseRms.
 	 		
 * @param uncorrPhaseRms.
 	 		
 * @param seeing.
 	 		
 * @param seeingFrequency.
 	 		
 * @param seeingFreqBandwidth.
 	 		 
 */
CalSeeingRow* CalSeeingTable::lookup(Tag calDataId, Tag calReductionId, int numBaseLength, ArrayTime startValidTime, ArrayTime endValidTime, vector<Frequency > frequencyRange, vector<Length > baseLength, vector<Angle > corrPhaseRms, vector<Angle > uncorrPhaseRms, Angle seeing, Frequency seeingFrequency, Frequency seeingFreqBandwidth) {
		CalSeeingRow* aRow;
		for (unsigned int i = 0; i < size(); i++) {
			aRow = row.at(i); 
			if (aRow->compareNoAutoInc(calDataId, calReductionId, numBaseLength, startValidTime, endValidTime, frequencyRange, baseLength, corrPhaseRms, uncorrPhaseRms, seeing, seeingFrequency, seeingFreqBandwidth)) return aRow;
		}			
		return 0;	
} 
	
 	 	

	





#ifndef WITHOUT_ACS
	// Conversion Methods

	CalSeeingTableIDL *CalSeeingTable::toIDL() {
		CalSeeingTableIDL *x = new CalSeeingTableIDL ();
		unsigned int nrow = size();
		x->row.length(nrow);
		vector<CalSeeingRow*> v = get();
		for (unsigned int i = 0; i < nrow; ++i) {
			x->row[i] = *(v[i]->toIDL());
		}
		return x;
	}
#endif
	
#ifndef WITHOUT_ACS
	void CalSeeingTable::fromIDL(CalSeeingTableIDL x) throw(DuplicateKey,ConversionException) {
		unsigned int nrow = x.row.length();
		for (unsigned int i = 0; i < nrow; ++i) {
			CalSeeingRow *tmp = newRow();
			tmp->setFromIDL(x.row[i]);
			// checkAndAdd(tmp);
			add(tmp);
		}
	}
#endif

	char *CalSeeingTable::toFITS() const throw(ConversionException) {
		throw ConversionException("Not implemented","CalSeeing");
	}

	void CalSeeingTable::fromFITS(char *fits) throw(ConversionException) {
		throw ConversionException("Not implemented","CalSeeing");
	}

	string CalSeeingTable::toVOTable() const throw(ConversionException) {
		throw ConversionException("Not implemented","CalSeeing");
	}

	void CalSeeingTable::fromVOTable(string vo) throw(ConversionException) {
		throw ConversionException("Not implemented","CalSeeing");
	}

	string CalSeeingTable::toXML()  throw(ConversionException) {
		string buf;
		buf.append("<?xml version=\"1.0\" encoding=\"ISO-8859-1\"?> ");
//		buf.append("<CalSeeingTable xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" xsi:noNamespaceSchemaLocation=\"../../idl/CalSeeingTable.xsd\"> ");
		buf.append("<?xml-stylesheet type=\"text/xsl\" href=\"../asdm2html/table2html.xsl\"?> ");		
		buf.append("<CalSeeingTable> ");
		buf.append(entity.toXML());
		string s = container.getEntity().toXML();
		// Change the "Entity" tag to "ContainerEntity".
		buf.append("<Container" + s.substr(1,s.length() - 1)+" ");
		vector<CalSeeingRow*> v = get();
		for (unsigned int i = 0; i < v.size(); ++i) {
			try {
				buf.append(v[i]->toXML());
			} catch (NoSuchRow e) {
			}
			buf.append("  ");
		}		
		buf.append("</CalSeeingTable> ");
		return buf;
	}
	
	void CalSeeingTable::fromXML(string xmlDoc) throw(ConversionException) {
		Parser xml(xmlDoc);
		if (!xml.isStr("<CalSeeingTable")) 
			error();
		// cout << "Parsing a CalSeeingTable" << endl;
		string s = xml.getElement("<Entity","/>");
		if (s.length() == 0) 
			error();
		Entity e;
		e.setFromXML(s);
		if (e.getEntityTypeName() != "CalSeeingTable")
			error();
		setEntity(e);
		// Skip the container's entity; but, it has to be there.
		s = xml.getElement("<ContainerEntity","/>");
		if (s.length() == 0) 
			error();

		// Get each row in the table.
		s = xml.getElementContent("<row>","</row>");
		CalSeeingRow *row;
		while (s.length() != 0) {
			// cout << "Parsing a CalSeeingRow" << endl; 
			row = newRow();
			row->setFromXML(s);
			try {
				checkAndAdd(row);
			} catch (DuplicateKey e1) {
				throw ConversionException(e1.getMessage(),"CalSeeingTable");
			} 
			catch (UniquenessViolationException e1) {
				throw ConversionException(e1.getMessage(),"CalSeeingTable");	
			}
			catch (...) {
				// cout << "Unexpected error in CalSeeingTable::checkAndAdd called from CalSeeingTable::fromXML " << endl;
			}
			s = xml.getElementContent("<row>","</row>");
		}
		if (!xml.isStr("</CalSeeingTable>")) 
			error();
	}

	void CalSeeingTable::error() throw(ConversionException) {
		throw ConversionException("Invalid xml document","CalSeeing");
	}
	
	string CalSeeingTable::toMIME() {
	 // To be implemented
		return "";
	}
	
	void CalSeeingTable::setFromMIME(const string & mimeMsg) {
		// To be implemented
		;
	}
	
	
	void CalSeeingTable::toFile(string directory) {
		if (!directoryExists(directory.c_str()) &&
			!createPath(directory.c_str())) {
			throw ConversionException("Could not create directory " , directory);
		}
		
		if (fileAsBin) {
			// write the bin serialized
			string fileName = directory + "/CalSeeing.bin";
			ofstream tableout(fileName.c_str(),ios::out|ios::trunc);
			if (tableout.rdstate() == ostream::failbit)
				throw ConversionException("Could not open file " + fileName + " to write ", "CalSeeing");
			tableout << toMIME() << endl;
			tableout.close();
			if (tableout.rdstate() == ostream::failbit)
				throw ConversionException("Could not close file " + fileName, "CalSeeing");
		}
		else {
			// write the XML
			string fileName = directory + "/CalSeeing.xml";
			ofstream tableout(fileName.c_str(),ios::out|ios::trunc);
			if (tableout.rdstate() == ostream::failbit)
				throw ConversionException("Could not open file " + fileName + " to write ", "CalSeeing");
			tableout << toXML() << endl;
			tableout.close();
			if (tableout.rdstate() == ostream::failbit)
				throw ConversionException("Could not close file " + fileName, "CalSeeing");
		}
	}
	
	void CalSeeingTable::setFromFile(const string& directory) {
		string tablename;
		if (fileAsBin)
			tablename = directory + "/CalSeeing.bin";
		else
			tablename = directory + "/CalSeeing.xml";
			
		// Determine the file size.
		ifstream::pos_type size;
		ifstream tablefile(tablename.c_str(), ios::in|ios::binary|ios::ate);

 		if (tablefile.is_open()) { 
  				size = tablefile.tellg(); 
  		}
		else {
				throw ConversionException("Could not open file " + tablename, "CalSeeing");
		}
		
		// Re position to the beginning.
		tablefile.seekg(0);
		
		// Read in a stringstream.
		stringstream ss;
		ss << tablefile.rdbuf();

		if (tablefile.rdstate() == istream::failbit || tablefile.rdstate() == istream::badbit) {
			throw ConversionException("Error reading file " + tablename,"CalSeeing");
		}

		// And close
		tablefile.close();
		if (tablefile.rdstate() == istream::failbit)
			throw ConversionException("Could not close file " + tablename,"CalSeeing");
					
		// And parse the content with the appropriate method
		if (fileAsBin) 
			setFromMIME(ss.str());
		else
			fromXML(ss.str());	
	}			
			
	
	

	
} // End namespace asdm
 
