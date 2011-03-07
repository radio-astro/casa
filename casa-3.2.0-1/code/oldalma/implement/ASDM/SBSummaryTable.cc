
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
 * File SBSummaryTable.cpp
 */
#include <ConversionException.h>
#include <DuplicateKey.h>
#include <OutOfBoundsException.h>

using asdm::ConversionException;
using asdm::DuplicateKey;
using asdm::OutOfBoundsException;

#include <ASDM.h>
#include <SBSummaryTable.h>
#include <SBSummaryRow.h>
#include <Parser.h>

using asdm::ASDM;
using asdm::SBSummaryTable;
using asdm::SBSummaryRow;
using asdm::Parser;

#include <iostream>
#include <sstream>
#include <set>
using namespace std;

#include <Misc.h>
using namespace asdm;


namespace asdm {

	string SBSummaryTable::tableName = "SBSummary";
	

	/**
	 * The list of field names that make up key key.
	 * (Initialization is in the constructor.)
	 */
	vector<string> SBSummaryTable::key;

	/**
	 * Return the list of field names that make up key key
	 * as an array of strings.
	 */	
	vector<string> SBSummaryTable::getKeyName() {
		return key;
	}


	SBSummaryTable::SBSummaryTable(ASDM &c) : container(c) {

	
		key.push_back("sbId");
	


		// Define a default entity.
		entity.setEntityId(EntityId("uid://X0/X0/X0"));
		entity.setEntityIdEncrypted("na");
		entity.setEntityTypeName("SBSummaryTable");
		entity.setEntityVersion("1");
		entity.setInstanceVersion("1");
		
		// Archive XML
		archiveAsBin = false;
		
		// File XML
		fileAsBin = false;
	}
	
/**
 * A destructor for SBSummaryTable.
 */
 
	SBSummaryTable::~SBSummaryTable() {
		for (unsigned int i = 0; i < privateRows.size(); i++) 
			delete(privateRows.at(i));
	}

	/**
	 * Container to which this table belongs.
	 */
	ASDM &SBSummaryTable::getContainer() const {
		return container;
	}

	/**
	 * Return the number of rows in the table.
	 */

	unsigned int SBSummaryTable::size() {
		return row.size();
	}	
	
	
	/**
	 * Return the name of this table.
	 */
	string SBSummaryTable::getName() const {
		return tableName;
	}

	/**
	 * Return this table's Entity.
	 */
	Entity SBSummaryTable::getEntity() const {
		return entity;
	}

	/**
	 * Set this table's Entity.
	 */
	void SBSummaryTable::setEntity(Entity e) {
		this->entity = e; 
	}
	
	//
	// ====> Row creation.
	//
	
	/**
	 * Create a new row.
	 */
	SBSummaryRow *SBSummaryTable::newRow() {
		return new SBSummaryRow (*this);
	}
	
	SBSummaryRow *SBSummaryTable::newRowEmpty() {
		return newRow ();
	}


	/**
	 * Create a new row initialized to the specified values.
	 * @return a pointer on the created and initialized row.
	
 	 * @param sbId. 
	
 	 * @param projectId. 
	
 	 * @param obsUnitSetId. 
	
 	 * @param sbIntent. 
	
 	 * @param sbType. 
	
 	 * @param sbDuration. 
	
 	 * @param numScan. 
	
 	 * @param scanIntent. 
	
 	 * @param numberRepeats. 
	
 	 * @param weatherConstraint. 
	
 	 * @param scienceGoal. 
	
 	 * @param raCenter. 
	
 	 * @param decCenter. 
	
 	 * @param frequency. 
	
 	 * @param frequencyBand. 
	
 	 * @param observingMode. 
	
     */
	SBSummaryRow* SBSummaryTable::newRow(EntityRef sbId, EntityRef projectId, EntityRef obsUnitSetId, string sbIntent, SBTypeMod::SBType sbType, Interval sbDuration, int numScan, vector<string > scanIntent, int numberRepeats, vector<string > weatherConstraint, vector<string > scienceGoal, double raCenter, double decCenter, double frequency, string frequencyBand, vector<string > observingMode){
		SBSummaryRow *row = new SBSummaryRow(*this);
			
		row->setSbId(sbId);
			
		row->setProjectId(projectId);
			
		row->setObsUnitSetId(obsUnitSetId);
			
		row->setSbIntent(sbIntent);
			
		row->setSbType(sbType);
			
		row->setSbDuration(sbDuration);
			
		row->setNumScan(numScan);
			
		row->setScanIntent(scanIntent);
			
		row->setNumberRepeats(numberRepeats);
			
		row->setWeatherConstraint(weatherConstraint);
			
		row->setScienceGoal(scienceGoal);
			
		row->setRaCenter(raCenter);
			
		row->setDecCenter(decCenter);
			
		row->setFrequency(frequency);
			
		row->setFrequencyBand(frequencyBand);
			
		row->setObservingMode(observingMode);
	
		return row;		
	}	

	SBSummaryRow* SBSummaryTable::newRowFull(EntityRef sbId, EntityRef projectId, EntityRef obsUnitSetId, string sbIntent, SBTypeMod::SBType sbType, Interval sbDuration, int numScan, vector<string > scanIntent, int numberRepeats, vector<string > weatherConstraint, vector<string > scienceGoal, double raCenter, double decCenter, double frequency, string frequencyBand, vector<string > observingMode)	{
		SBSummaryRow *row = new SBSummaryRow(*this);
			
		row->setSbId(sbId);
			
		row->setProjectId(projectId);
			
		row->setObsUnitSetId(obsUnitSetId);
			
		row->setSbIntent(sbIntent);
			
		row->setSbType(sbType);
			
		row->setSbDuration(sbDuration);
			
		row->setNumScan(numScan);
			
		row->setScanIntent(scanIntent);
			
		row->setNumberRepeats(numberRepeats);
			
		row->setWeatherConstraint(weatherConstraint);
			
		row->setScienceGoal(scienceGoal);
			
		row->setRaCenter(raCenter);
			
		row->setDecCenter(decCenter);
			
		row->setFrequency(frequency);
			
		row->setFrequencyBand(frequencyBand);
			
		row->setObservingMode(observingMode);
	
		return row;				
	}
	


SBSummaryRow* SBSummaryTable::newRow(SBSummaryRow* row) {
	return new SBSummaryRow(*this, *row);
}

SBSummaryRow* SBSummaryTable::newRowCopy(SBSummaryRow* row) {
	return new SBSummaryRow(*this, *row);
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
	SBSummaryRow* SBSummaryTable::add(SBSummaryRow* x) {
		
		if (getRowByKey(
						x->getSbId()
						))
			//throw DuplicateKey(x.getSbId(),"SBSummary");
			throw DuplicateKey("Duplicate key exception in ","SBSummaryTable");
		
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
	SBSummaryRow*  SBSummaryTable::checkAndAdd(SBSummaryRow* x) throw (DuplicateKey) {
		
		
		if (getRowByKey(
	
			x->getSbId()
			
		)) throw DuplicateKey("Duplicate key exception in ", "SBSummaryTable");
		
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
	 * @return Alls rows as an array of SBSummaryRow
	 */
	vector<SBSummaryRow *> SBSummaryTable::get() {
		return privateRows;
		// return row;
	}

	
/*
 ** Returns a SBSummaryRow* given a key.
 ** @return a pointer to the row having the key whose values are passed as parameters, or 0 if
 ** no row exists for that key.
 **
 */
 	SBSummaryRow* SBSummaryTable::getRowByKey(EntityRef sbId)  {
	SBSummaryRow* aRow = 0;
	for (unsigned int i = 0; i < row.size(); i++) {
		aRow = row.at(i);
		
			
				if (aRow->sbId != sbId) continue;
			
		
		return aRow;
	}
	return 0;		
}
	

	
/**
 * Look up the table for a row whose all attributes 
 * are equal to the corresponding parameters of the method.
 * @return a pointer on this row if any, 0 otherwise.
 *
			
 * @param sbId.
 	 		
 * @param projectId.
 	 		
 * @param obsUnitSetId.
 	 		
 * @param sbIntent.
 	 		
 * @param sbType.
 	 		
 * @param sbDuration.
 	 		
 * @param numScan.
 	 		
 * @param scanIntent.
 	 		
 * @param numberRepeats.
 	 		
 * @param weatherConstraint.
 	 		
 * @param scienceGoal.
 	 		
 * @param raCenter.
 	 		
 * @param decCenter.
 	 		
 * @param frequency.
 	 		
 * @param frequencyBand.
 	 		
 * @param observingMode.
 	 		 
 */
SBSummaryRow* SBSummaryTable::lookup(EntityRef sbId, EntityRef projectId, EntityRef obsUnitSetId, string sbIntent, SBTypeMod::SBType sbType, Interval sbDuration, int numScan, vector<string > scanIntent, int numberRepeats, vector<string > weatherConstraint, vector<string > scienceGoal, double raCenter, double decCenter, double frequency, string frequencyBand, vector<string > observingMode) {
		SBSummaryRow* aRow;
		for (unsigned int i = 0; i < size(); i++) {
			aRow = row.at(i); 
			if (aRow->compareNoAutoInc(sbId, projectId, obsUnitSetId, sbIntent, sbType, sbDuration, numScan, scanIntent, numberRepeats, weatherConstraint, scienceGoal, raCenter, decCenter, frequency, frequencyBand, observingMode)) return aRow;
		}			
		return 0;	
} 
	
 	 	

	





#ifndef WITHOUT_ACS
	// Conversion Methods

	SBSummaryTableIDL *SBSummaryTable::toIDL() {
		SBSummaryTableIDL *x = new SBSummaryTableIDL ();
		unsigned int nrow = size();
		x->row.length(nrow);
		vector<SBSummaryRow*> v = get();
		for (unsigned int i = 0; i < nrow; ++i) {
			x->row[i] = *(v[i]->toIDL());
		}
		return x;
	}
#endif
	
#ifndef WITHOUT_ACS
	void SBSummaryTable::fromIDL(SBSummaryTableIDL x) throw(DuplicateKey,ConversionException) {
		unsigned int nrow = x.row.length();
		for (unsigned int i = 0; i < nrow; ++i) {
			SBSummaryRow *tmp = newRow();
			tmp->setFromIDL(x.row[i]);
			// checkAndAdd(tmp);
			add(tmp);
		}
	}
#endif

	char *SBSummaryTable::toFITS() const throw(ConversionException) {
		throw ConversionException("Not implemented","SBSummary");
	}

	void SBSummaryTable::fromFITS(char *fits) throw(ConversionException) {
		throw ConversionException("Not implemented","SBSummary");
	}

	string SBSummaryTable::toVOTable() const throw(ConversionException) {
		throw ConversionException("Not implemented","SBSummary");
	}

	void SBSummaryTable::fromVOTable(string vo) throw(ConversionException) {
		throw ConversionException("Not implemented","SBSummary");
	}

	string SBSummaryTable::toXML()  throw(ConversionException) {
		string buf;
		buf.append("<?xml version=\"1.0\" encoding=\"ISO-8859-1\"?> ");
//		buf.append("<SBSummaryTable xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" xsi:noNamespaceSchemaLocation=\"../../idl/SBSummaryTable.xsd\"> ");
		buf.append("<?xml-stylesheet type=\"text/xsl\" href=\"../asdm2html/table2html.xsl\"?> ");		
		buf.append("<SBSummaryTable> ");
		buf.append(entity.toXML());
		string s = container.getEntity().toXML();
		// Change the "Entity" tag to "ContainerEntity".
		buf.append("<Container" + s.substr(1,s.length() - 1)+" ");
		vector<SBSummaryRow*> v = get();
		for (unsigned int i = 0; i < v.size(); ++i) {
			try {
				buf.append(v[i]->toXML());
			} catch (NoSuchRow e) {
			}
			buf.append("  ");
		}		
		buf.append("</SBSummaryTable> ");
		return buf;
	}
	
	void SBSummaryTable::fromXML(string xmlDoc) throw(ConversionException) {
		Parser xml(xmlDoc);
		if (!xml.isStr("<SBSummaryTable")) 
			error();
		// cout << "Parsing a SBSummaryTable" << endl;
		string s = xml.getElement("<Entity","/>");
		if (s.length() == 0) 
			error();
		Entity e;
		e.setFromXML(s);
		if (e.getEntityTypeName() != "SBSummaryTable")
			error();
		setEntity(e);
		// Skip the container's entity; but, it has to be there.
		s = xml.getElement("<ContainerEntity","/>");
		if (s.length() == 0) 
			error();

		// Get each row in the table.
		s = xml.getElementContent("<row>","</row>");
		SBSummaryRow *row;
		while (s.length() != 0) {
			// cout << "Parsing a SBSummaryRow" << endl; 
			row = newRow();
			row->setFromXML(s);
			try {
				checkAndAdd(row);
			} catch (DuplicateKey e1) {
				throw ConversionException(e1.getMessage(),"SBSummaryTable");
			} 
			catch (UniquenessViolationException e1) {
				throw ConversionException(e1.getMessage(),"SBSummaryTable");	
			}
			catch (...) {
				// cout << "Unexpected error in SBSummaryTable::checkAndAdd called from SBSummaryTable::fromXML " << endl;
			}
			s = xml.getElementContent("<row>","</row>");
		}
		if (!xml.isStr("</SBSummaryTable>")) 
			error();
	}

	void SBSummaryTable::error() throw(ConversionException) {
		throw ConversionException("Invalid xml document","SBSummary");
	}
	
	string SBSummaryTable::toMIME() {
	 // To be implemented
		return "";
	}
	
	void SBSummaryTable::setFromMIME(const string & mimeMsg) {
		// To be implemented
		;
	}
	
	
	void SBSummaryTable::toFile(string directory) {
		if (!directoryExists(directory.c_str()) &&
			!createPath(directory.c_str())) {
			throw ConversionException("Could not create directory " , directory);
		}
		
		if (fileAsBin) {
			// write the bin serialized
			string fileName = directory + "/SBSummary.bin";
			ofstream tableout(fileName.c_str(),ios::out|ios::trunc);
			if (tableout.rdstate() == ostream::failbit)
				throw ConversionException("Could not open file " + fileName + " to write ", "SBSummary");
			tableout << toMIME() << endl;
			tableout.close();
			if (tableout.rdstate() == ostream::failbit)
				throw ConversionException("Could not close file " + fileName, "SBSummary");
		}
		else {
			// write the XML
			string fileName = directory + "/SBSummary.xml";
			ofstream tableout(fileName.c_str(),ios::out|ios::trunc);
			if (tableout.rdstate() == ostream::failbit)
				throw ConversionException("Could not open file " + fileName + " to write ", "SBSummary");
			tableout << toXML() << endl;
			tableout.close();
			if (tableout.rdstate() == ostream::failbit)
				throw ConversionException("Could not close file " + fileName, "SBSummary");
		}
	}
	
	void SBSummaryTable::setFromFile(const string& directory) {
		string tablename;
		if (fileAsBin)
			tablename = directory + "/SBSummary.bin";
		else
			tablename = directory + "/SBSummary.xml";
			
		// Determine the file size.
		ifstream::pos_type size;
		ifstream tablefile(tablename.c_str(), ios::in|ios::binary|ios::ate);

 		if (tablefile.is_open()) { 
  				size = tablefile.tellg(); 
  		}
		else {
				throw ConversionException("Could not open file " + tablename, "SBSummary");
		}
		
		// Re position to the beginning.
		tablefile.seekg(0);
		
		// Read in a stringstream.
		stringstream ss;
		ss << tablefile.rdbuf();

		if (tablefile.rdstate() == istream::failbit || tablefile.rdstate() == istream::badbit) {
			throw ConversionException("Error reading file " + tablename,"SBSummary");
		}

		// And close
		tablefile.close();
		if (tablefile.rdstate() == istream::failbit)
			throw ConversionException("Could not close file " + tablename,"SBSummary");
					
		// And parse the content with the appropriate method
		if (fileAsBin) 
			setFromMIME(ss.str());
		else
			fromXML(ss.str());	
	}			
			
	
	

	
} // End namespace asdm
 
