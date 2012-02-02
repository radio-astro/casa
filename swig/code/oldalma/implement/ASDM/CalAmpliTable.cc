
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
 * File CalAmpliTable.cpp
 */
#include <ConversionException.h>
#include <DuplicateKey.h>
#include <OutOfBoundsException.h>

using asdm::ConversionException;
using asdm::DuplicateKey;
using asdm::OutOfBoundsException;

#include <ASDM.h>
#include <CalAmpliTable.h>
#include <CalAmpliRow.h>
#include <Parser.h>

using asdm::ASDM;
using asdm::CalAmpliTable;
using asdm::CalAmpliRow;
using asdm::Parser;

#include <iostream>
#include <sstream>
#include <set>
using namespace std;

#include <Misc.h>
using namespace asdm;


namespace asdm {

	string CalAmpliTable::tableName = "CalAmpli";
	

	/**
	 * The list of field names that make up key key.
	 * (Initialization is in the constructor.)
	 */
	vector<string> CalAmpliTable::key;

	/**
	 * Return the list of field names that make up key key
	 * as an array of strings.
	 */	
	vector<string> CalAmpliTable::getKeyName() {
		return key;
	}


	CalAmpliTable::CalAmpliTable(ASDM &c) : container(c) {

	
		key.push_back("calDataId");
	
		key.push_back("calReductionId");
	
		key.push_back("antennaName");
	


		// Define a default entity.
		entity.setEntityId(EntityId("uid://X0/X0/X0"));
		entity.setEntityIdEncrypted("na");
		entity.setEntityTypeName("CalAmpliTable");
		entity.setEntityVersion("1");
		entity.setInstanceVersion("1");
		
		// Archive XML
		archiveAsBin = false;
		
		// File XML
		fileAsBin = false;
	}
	
/**
 * A destructor for CalAmpliTable.
 */
 
	CalAmpliTable::~CalAmpliTable() {
		for (unsigned int i = 0; i < privateRows.size(); i++) 
			delete(privateRows.at(i));
	}

	/**
	 * Container to which this table belongs.
	 */
	ASDM &CalAmpliTable::getContainer() const {
		return container;
	}

	/**
	 * Return the number of rows in the table.
	 */

	unsigned int CalAmpliTable::size() {
		return row.size();
	}	
	
	
	/**
	 * Return the name of this table.
	 */
	string CalAmpliTable::getName() const {
		return tableName;
	}

	/**
	 * Return this table's Entity.
	 */
	Entity CalAmpliTable::getEntity() const {
		return entity;
	}

	/**
	 * Set this table's Entity.
	 */
	void CalAmpliTable::setEntity(Entity e) {
		this->entity = e; 
	}
	
	//
	// ====> Row creation.
	//
	
	/**
	 * Create a new row.
	 */
	CalAmpliRow *CalAmpliTable::newRow() {
		return new CalAmpliRow (*this);
	}
	
	CalAmpliRow *CalAmpliTable::newRowEmpty() {
		return newRow ();
	}


	/**
	 * Create a new row initialized to the specified values.
	 * @return a pointer on the created and initialized row.
	
 	 * @param calDataId. 
	
 	 * @param calReductionId. 
	
 	 * @param antennaName. 
	
 	 * @param numAPC. 
	
 	 * @param numReceptor. 
	
 	 * @param receiverBand. 
	
 	 * @param atmPhaseCorrections. 
	
 	 * @param polarizationTypes. 
	
 	 * @param startValidTime. 
	
 	 * @param endValidTime. 
	
 	 * @param frequencyRange. 
	
     */
	CalAmpliRow* CalAmpliTable::newRow(Tag calDataId, Tag calReductionId, string antennaName, int numAPC, int numReceptor, ReceiverBandMod::ReceiverBand receiverBand, vector<AtmPhaseCorrectionMod::AtmPhaseCorrection > atmPhaseCorrections, vector<PolarizationTypeMod::PolarizationType > polarizationTypes, ArrayTime startValidTime, ArrayTime endValidTime, vector<Frequency > frequencyRange){
		CalAmpliRow *row = new CalAmpliRow(*this);
			
		row->setCalDataId(calDataId);
			
		row->setCalReductionId(calReductionId);
			
		row->setAntennaName(antennaName);
			
		row->setNumAPC(numAPC);
			
		row->setNumReceptor(numReceptor);
			
		row->setReceiverBand(receiverBand);
			
		row->setAtmPhaseCorrections(atmPhaseCorrections);
			
		row->setPolarizationTypes(polarizationTypes);
			
		row->setStartValidTime(startValidTime);
			
		row->setEndValidTime(endValidTime);
			
		row->setFrequencyRange(frequencyRange);
	
		return row;		
	}	

	CalAmpliRow* CalAmpliTable::newRowFull(Tag calDataId, Tag calReductionId, string antennaName, int numAPC, int numReceptor, ReceiverBandMod::ReceiverBand receiverBand, vector<AtmPhaseCorrectionMod::AtmPhaseCorrection > atmPhaseCorrections, vector<PolarizationTypeMod::PolarizationType > polarizationTypes, ArrayTime startValidTime, ArrayTime endValidTime, vector<Frequency > frequencyRange)	{
		CalAmpliRow *row = new CalAmpliRow(*this);
			
		row->setCalDataId(calDataId);
			
		row->setCalReductionId(calReductionId);
			
		row->setAntennaName(antennaName);
			
		row->setNumAPC(numAPC);
			
		row->setNumReceptor(numReceptor);
			
		row->setReceiverBand(receiverBand);
			
		row->setAtmPhaseCorrections(atmPhaseCorrections);
			
		row->setPolarizationTypes(polarizationTypes);
			
		row->setStartValidTime(startValidTime);
			
		row->setEndValidTime(endValidTime);
			
		row->setFrequencyRange(frequencyRange);
	
		return row;				
	}
	


CalAmpliRow* CalAmpliTable::newRow(CalAmpliRow* row) {
	return new CalAmpliRow(*this, *row);
}

CalAmpliRow* CalAmpliTable::newRowCopy(CalAmpliRow* row) {
	return new CalAmpliRow(*this, *row);
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
	CalAmpliRow* CalAmpliTable::add(CalAmpliRow* x) {
		
		if (getRowByKey(
						x->getCalDataId()
						,
						x->getCalReductionId()
						,
						x->getAntennaName()
						))
			//throw DuplicateKey(x.getCalDataId() + "|" + x.getCalReductionId() + "|" + x.getAntennaName(),"CalAmpli");
			throw DuplicateKey("Duplicate key exception in ","CalAmpliTable");
		
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
	CalAmpliRow*  CalAmpliTable::checkAndAdd(CalAmpliRow* x) throw (DuplicateKey) {
		
		
		if (getRowByKey(
	
			x->getCalDataId()
	,
			x->getCalReductionId()
	,
			x->getAntennaName()
			
		)) throw DuplicateKey("Duplicate key exception in ", "CalAmpliTable");
		
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
	 * @return Alls rows as an array of CalAmpliRow
	 */
	vector<CalAmpliRow *> CalAmpliTable::get() {
		return privateRows;
		// return row;
	}

	
/*
 ** Returns a CalAmpliRow* given a key.
 ** @return a pointer to the row having the key whose values are passed as parameters, or 0 if
 ** no row exists for that key.
 **
 */
 	CalAmpliRow* CalAmpliTable::getRowByKey(Tag calDataId, Tag calReductionId, string antennaName)  {
	CalAmpliRow* aRow = 0;
	for (unsigned int i = 0; i < row.size(); i++) {
		aRow = row.at(i);
		
			
				if (aRow->calDataId != calDataId) continue;
			
		
			
				if (aRow->calReductionId != calReductionId) continue;
			
		
			
				if (aRow->antennaName != antennaName) continue;
			
		
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
 	 		
 * @param antennaName.
 	 		
 * @param numAPC.
 	 		
 * @param numReceptor.
 	 		
 * @param receiverBand.
 	 		
 * @param atmPhaseCorrections.
 	 		
 * @param polarizationTypes.
 	 		
 * @param startValidTime.
 	 		
 * @param endValidTime.
 	 		
 * @param frequencyRange.
 	 		 
 */
CalAmpliRow* CalAmpliTable::lookup(Tag calDataId, Tag calReductionId, string antennaName, int numAPC, int numReceptor, ReceiverBandMod::ReceiverBand receiverBand, vector<AtmPhaseCorrectionMod::AtmPhaseCorrection > atmPhaseCorrections, vector<PolarizationTypeMod::PolarizationType > polarizationTypes, ArrayTime startValidTime, ArrayTime endValidTime, vector<Frequency > frequencyRange) {
		CalAmpliRow* aRow;
		for (unsigned int i = 0; i < size(); i++) {
			aRow = row.at(i); 
			if (aRow->compareNoAutoInc(calDataId, calReductionId, antennaName, numAPC, numReceptor, receiverBand, atmPhaseCorrections, polarizationTypes, startValidTime, endValidTime, frequencyRange)) return aRow;
		}			
		return 0;	
} 
	
 	 	

	





#ifndef WITHOUT_ACS
	// Conversion Methods

	CalAmpliTableIDL *CalAmpliTable::toIDL() {
		CalAmpliTableIDL *x = new CalAmpliTableIDL ();
		unsigned int nrow = size();
		x->row.length(nrow);
		vector<CalAmpliRow*> v = get();
		for (unsigned int i = 0; i < nrow; ++i) {
			x->row[i] = *(v[i]->toIDL());
		}
		return x;
	}
#endif
	
#ifndef WITHOUT_ACS
	void CalAmpliTable::fromIDL(CalAmpliTableIDL x) throw(DuplicateKey,ConversionException) {
		unsigned int nrow = x.row.length();
		for (unsigned int i = 0; i < nrow; ++i) {
			CalAmpliRow *tmp = newRow();
			tmp->setFromIDL(x.row[i]);
			// checkAndAdd(tmp);
			add(tmp);
		}
	}
#endif

	char *CalAmpliTable::toFITS() const throw(ConversionException) {
		throw ConversionException("Not implemented","CalAmpli");
	}

	void CalAmpliTable::fromFITS(char *fits) throw(ConversionException) {
		throw ConversionException("Not implemented","CalAmpli");
	}

	string CalAmpliTable::toVOTable() const throw(ConversionException) {
		throw ConversionException("Not implemented","CalAmpli");
	}

	void CalAmpliTable::fromVOTable(string vo) throw(ConversionException) {
		throw ConversionException("Not implemented","CalAmpli");
	}

	string CalAmpliTable::toXML()  throw(ConversionException) {
		string buf;
		buf.append("<?xml version=\"1.0\" encoding=\"ISO-8859-1\"?> ");
//		buf.append("<CalAmpliTable xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" xsi:noNamespaceSchemaLocation=\"../../idl/CalAmpliTable.xsd\"> ");
		buf.append("<?xml-stylesheet type=\"text/xsl\" href=\"../asdm2html/table2html.xsl\"?> ");		
		buf.append("<CalAmpliTable> ");
		buf.append(entity.toXML());
		string s = container.getEntity().toXML();
		// Change the "Entity" tag to "ContainerEntity".
		buf.append("<Container" + s.substr(1,s.length() - 1)+" ");
		vector<CalAmpliRow*> v = get();
		for (unsigned int i = 0; i < v.size(); ++i) {
			try {
				buf.append(v[i]->toXML());
			} catch (NoSuchRow e) {
			}
			buf.append("  ");
		}		
		buf.append("</CalAmpliTable> ");
		return buf;
	}
	
	void CalAmpliTable::fromXML(string xmlDoc) throw(ConversionException) {
		Parser xml(xmlDoc);
		if (!xml.isStr("<CalAmpliTable")) 
			error();
		// cout << "Parsing a CalAmpliTable" << endl;
		string s = xml.getElement("<Entity","/>");
		if (s.length() == 0) 
			error();
		Entity e;
		e.setFromXML(s);
		if (e.getEntityTypeName() != "CalAmpliTable")
			error();
		setEntity(e);
		// Skip the container's entity; but, it has to be there.
		s = xml.getElement("<ContainerEntity","/>");
		if (s.length() == 0) 
			error();

		// Get each row in the table.
		s = xml.getElementContent("<row>","</row>");
		CalAmpliRow *row;
		while (s.length() != 0) {
			// cout << "Parsing a CalAmpliRow" << endl; 
			row = newRow();
			row->setFromXML(s);
			try {
				checkAndAdd(row);
			} catch (DuplicateKey e1) {
				throw ConversionException(e1.getMessage(),"CalAmpliTable");
			} 
			catch (UniquenessViolationException e1) {
				throw ConversionException(e1.getMessage(),"CalAmpliTable");	
			}
			catch (...) {
				// cout << "Unexpected error in CalAmpliTable::checkAndAdd called from CalAmpliTable::fromXML " << endl;
			}
			s = xml.getElementContent("<row>","</row>");
		}
		if (!xml.isStr("</CalAmpliTable>")) 
			error();
	}

	void CalAmpliTable::error() throw(ConversionException) {
		throw ConversionException("Invalid xml document","CalAmpli");
	}
	
	string CalAmpliTable::toMIME() {
	 // To be implemented
		return "";
	}
	
	void CalAmpliTable::setFromMIME(const string & mimeMsg) {
		// To be implemented
		;
	}
	
	
	void CalAmpliTable::toFile(string directory) {
		if (!directoryExists(directory.c_str()) &&
			!createPath(directory.c_str())) {
			throw ConversionException("Could not create directory " , directory);
		}
		
		if (fileAsBin) {
			// write the bin serialized
			string fileName = directory + "/CalAmpli.bin";
			ofstream tableout(fileName.c_str(),ios::out|ios::trunc);
			if (tableout.rdstate() == ostream::failbit)
				throw ConversionException("Could not open file " + fileName + " to write ", "CalAmpli");
			tableout << toMIME() << endl;
			tableout.close();
			if (tableout.rdstate() == ostream::failbit)
				throw ConversionException("Could not close file " + fileName, "CalAmpli");
		}
		else {
			// write the XML
			string fileName = directory + "/CalAmpli.xml";
			ofstream tableout(fileName.c_str(),ios::out|ios::trunc);
			if (tableout.rdstate() == ostream::failbit)
				throw ConversionException("Could not open file " + fileName + " to write ", "CalAmpli");
			tableout << toXML() << endl;
			tableout.close();
			if (tableout.rdstate() == ostream::failbit)
				throw ConversionException("Could not close file " + fileName, "CalAmpli");
		}
	}
	
	void CalAmpliTable::setFromFile(const string& directory) {
		string tablename;
		if (fileAsBin)
			tablename = directory + "/CalAmpli.bin";
		else
			tablename = directory + "/CalAmpli.xml";
			
		// Determine the file size.
		ifstream::pos_type size;
		ifstream tablefile(tablename.c_str(), ios::in|ios::binary|ios::ate);

 		if (tablefile.is_open()) { 
  				size = tablefile.tellg(); 
  		}
		else {
				throw ConversionException("Could not open file " + tablename, "CalAmpli");
		}
		
		// Re position to the beginning.
		tablefile.seekg(0);
		
		// Read in a stringstream.
		stringstream ss;
		ss << tablefile.rdbuf();

		if (tablefile.rdstate() == istream::failbit || tablefile.rdstate() == istream::badbit) {
			throw ConversionException("Error reading file " + tablename,"CalAmpli");
		}

		// And close
		tablefile.close();
		if (tablefile.rdstate() == istream::failbit)
			throw ConversionException("Could not close file " + tablename,"CalAmpli");
					
		// And parse the content with the appropriate method
		if (fileAsBin) 
			setFromMIME(ss.str());
		else
			fromXML(ss.str());	
	}			
			
	
	

	
} // End namespace asdm
 
