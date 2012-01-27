
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
 * File CalPrimaryBeamTable.cpp
 */
#include <ConversionException.h>
#include <DuplicateKey.h>
#include <OutOfBoundsException.h>

using asdm::ConversionException;
using asdm::DuplicateKey;
using asdm::OutOfBoundsException;

#include <ASDM.h>
#include <CalPrimaryBeamTable.h>
#include <CalPrimaryBeamRow.h>
#include <Parser.h>

using asdm::ASDM;
using asdm::CalPrimaryBeamTable;
using asdm::CalPrimaryBeamRow;
using asdm::Parser;

#include <iostream>
#include <sstream>
#include <set>
using namespace std;

#include <Misc.h>
using namespace asdm;


namespace asdm {

	string CalPrimaryBeamTable::tableName = "CalPrimaryBeam";
	

	/**
	 * The list of field names that make up key key.
	 * (Initialization is in the constructor.)
	 */
	vector<string> CalPrimaryBeamTable::key;

	/**
	 * Return the list of field names that make up key key
	 * as an array of strings.
	 */	
	vector<string> CalPrimaryBeamTable::getKeyName() {
		return key;
	}


	CalPrimaryBeamTable::CalPrimaryBeamTable(ASDM &c) : container(c) {

	
		key.push_back("calDataId");
	
		key.push_back("calReductionId");
	
		key.push_back("antennaName");
	


		// Define a default entity.
		entity.setEntityId(EntityId("uid://X0/X0/X0"));
		entity.setEntityIdEncrypted("na");
		entity.setEntityTypeName("CalPrimaryBeamTable");
		entity.setEntityVersion("1");
		entity.setInstanceVersion("1");
		
		// Archive XML
		archiveAsBin = false;
		
		// File XML
		fileAsBin = false;
	}
	
/**
 * A destructor for CalPrimaryBeamTable.
 */
 
	CalPrimaryBeamTable::~CalPrimaryBeamTable() {
		for (unsigned int i = 0; i < privateRows.size(); i++) 
			delete(privateRows.at(i));
	}

	/**
	 * Container to which this table belongs.
	 */
	ASDM &CalPrimaryBeamTable::getContainer() const {
		return container;
	}

	/**
	 * Return the number of rows in the table.
	 */

	unsigned int CalPrimaryBeamTable::size() {
		return row.size();
	}	
	
	
	/**
	 * Return the name of this table.
	 */
	string CalPrimaryBeamTable::getName() const {
		return tableName;
	}

	/**
	 * Return this table's Entity.
	 */
	Entity CalPrimaryBeamTable::getEntity() const {
		return entity;
	}

	/**
	 * Set this table's Entity.
	 */
	void CalPrimaryBeamTable::setEntity(Entity e) {
		this->entity = e; 
	}
	
	//
	// ====> Row creation.
	//
	
	/**
	 * Create a new row.
	 */
	CalPrimaryBeamRow *CalPrimaryBeamTable::newRow() {
		return new CalPrimaryBeamRow (*this);
	}
	
	CalPrimaryBeamRow *CalPrimaryBeamTable::newRowEmpty() {
		return newRow ();
	}


	/**
	 * Create a new row initialized to the specified values.
	 * @return a pointer on the created and initialized row.
	
 	 * @param calDataId. 
	
 	 * @param calReductionId. 
	
 	 * @param antennaName. 
	
 	 * @param antennaMake. 
	
 	 * @param numReceptor. 
	
 	 * @param polarizationTypes. 
	
 	 * @param startValidTime. 
	
 	 * @param endValidTime. 
	
 	 * @param frequencyRange. 
	
 	 * @param receiverBand. 
	
 	 * @param beamMapUID. 
	
 	 * @param relativeAmplitudeRms. 
	
     */
	CalPrimaryBeamRow* CalPrimaryBeamTable::newRow(Tag calDataId, Tag calReductionId, string antennaName, AntennaMakeMod::AntennaMake antennaMake, int numReceptor, vector<PolarizationTypeMod::PolarizationType > polarizationTypes, ArrayTime startValidTime, ArrayTime endValidTime, vector<Frequency > frequencyRange, ReceiverBandMod::ReceiverBand receiverBand, EntityRef beamMapUID, float relativeAmplitudeRms){
		CalPrimaryBeamRow *row = new CalPrimaryBeamRow(*this);
			
		row->setCalDataId(calDataId);
			
		row->setCalReductionId(calReductionId);
			
		row->setAntennaName(antennaName);
			
		row->setAntennaMake(antennaMake);
			
		row->setNumReceptor(numReceptor);
			
		row->setPolarizationTypes(polarizationTypes);
			
		row->setStartValidTime(startValidTime);
			
		row->setEndValidTime(endValidTime);
			
		row->setFrequencyRange(frequencyRange);
			
		row->setReceiverBand(receiverBand);
			
		row->setBeamMapUID(beamMapUID);
			
		row->setRelativeAmplitudeRms(relativeAmplitudeRms);
	
		return row;		
	}	

	CalPrimaryBeamRow* CalPrimaryBeamTable::newRowFull(Tag calDataId, Tag calReductionId, string antennaName, AntennaMakeMod::AntennaMake antennaMake, int numReceptor, vector<PolarizationTypeMod::PolarizationType > polarizationTypes, ArrayTime startValidTime, ArrayTime endValidTime, vector<Frequency > frequencyRange, ReceiverBandMod::ReceiverBand receiverBand, EntityRef beamMapUID, float relativeAmplitudeRms)	{
		CalPrimaryBeamRow *row = new CalPrimaryBeamRow(*this);
			
		row->setCalDataId(calDataId);
			
		row->setCalReductionId(calReductionId);
			
		row->setAntennaName(antennaName);
			
		row->setAntennaMake(antennaMake);
			
		row->setNumReceptor(numReceptor);
			
		row->setPolarizationTypes(polarizationTypes);
			
		row->setStartValidTime(startValidTime);
			
		row->setEndValidTime(endValidTime);
			
		row->setFrequencyRange(frequencyRange);
			
		row->setReceiverBand(receiverBand);
			
		row->setBeamMapUID(beamMapUID);
			
		row->setRelativeAmplitudeRms(relativeAmplitudeRms);
	
		return row;				
	}
	


CalPrimaryBeamRow* CalPrimaryBeamTable::newRow(CalPrimaryBeamRow* row) {
	return new CalPrimaryBeamRow(*this, *row);
}

CalPrimaryBeamRow* CalPrimaryBeamTable::newRowCopy(CalPrimaryBeamRow* row) {
	return new CalPrimaryBeamRow(*this, *row);
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
	CalPrimaryBeamRow* CalPrimaryBeamTable::add(CalPrimaryBeamRow* x) {
		
		if (getRowByKey(
						x->getCalDataId()
						,
						x->getCalReductionId()
						,
						x->getAntennaName()
						))
			//throw DuplicateKey(x.getCalDataId() + "|" + x.getCalReductionId() + "|" + x.getAntennaName(),"CalPrimaryBeam");
			throw DuplicateKey("Duplicate key exception in ","CalPrimaryBeamTable");
		
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
	CalPrimaryBeamRow*  CalPrimaryBeamTable::checkAndAdd(CalPrimaryBeamRow* x) throw (DuplicateKey) {
		
		
		if (getRowByKey(
	
			x->getCalDataId()
	,
			x->getCalReductionId()
	,
			x->getAntennaName()
			
		)) throw DuplicateKey("Duplicate key exception in ", "CalPrimaryBeamTable");
		
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
	 * @return Alls rows as an array of CalPrimaryBeamRow
	 */
	vector<CalPrimaryBeamRow *> CalPrimaryBeamTable::get() {
		return privateRows;
		// return row;
	}

	
/*
 ** Returns a CalPrimaryBeamRow* given a key.
 ** @return a pointer to the row having the key whose values are passed as parameters, or 0 if
 ** no row exists for that key.
 **
 */
 	CalPrimaryBeamRow* CalPrimaryBeamTable::getRowByKey(Tag calDataId, Tag calReductionId, string antennaName)  {
	CalPrimaryBeamRow* aRow = 0;
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
 	 		
 * @param antennaMake.
 	 		
 * @param numReceptor.
 	 		
 * @param polarizationTypes.
 	 		
 * @param startValidTime.
 	 		
 * @param endValidTime.
 	 		
 * @param frequencyRange.
 	 		
 * @param receiverBand.
 	 		
 * @param beamMapUID.
 	 		
 * @param relativeAmplitudeRms.
 	 		 
 */
CalPrimaryBeamRow* CalPrimaryBeamTable::lookup(Tag calDataId, Tag calReductionId, string antennaName, AntennaMakeMod::AntennaMake antennaMake, int numReceptor, vector<PolarizationTypeMod::PolarizationType > polarizationTypes, ArrayTime startValidTime, ArrayTime endValidTime, vector<Frequency > frequencyRange, ReceiverBandMod::ReceiverBand receiverBand, EntityRef beamMapUID, float relativeAmplitudeRms) {
		CalPrimaryBeamRow* aRow;
		for (unsigned int i = 0; i < size(); i++) {
			aRow = row.at(i); 
			if (aRow->compareNoAutoInc(calDataId, calReductionId, antennaName, antennaMake, numReceptor, polarizationTypes, startValidTime, endValidTime, frequencyRange, receiverBand, beamMapUID, relativeAmplitudeRms)) return aRow;
		}			
		return 0;	
} 
	
 	 	

	





#ifndef WITHOUT_ACS
	// Conversion Methods

	CalPrimaryBeamTableIDL *CalPrimaryBeamTable::toIDL() {
		CalPrimaryBeamTableIDL *x = new CalPrimaryBeamTableIDL ();
		unsigned int nrow = size();
		x->row.length(nrow);
		vector<CalPrimaryBeamRow*> v = get();
		for (unsigned int i = 0; i < nrow; ++i) {
			x->row[i] = *(v[i]->toIDL());
		}
		return x;
	}
#endif
	
#ifndef WITHOUT_ACS
	void CalPrimaryBeamTable::fromIDL(CalPrimaryBeamTableIDL x) throw(DuplicateKey,ConversionException) {
		unsigned int nrow = x.row.length();
		for (unsigned int i = 0; i < nrow; ++i) {
			CalPrimaryBeamRow *tmp = newRow();
			tmp->setFromIDL(x.row[i]);
			// checkAndAdd(tmp);
			add(tmp);
		}
	}
#endif

	char *CalPrimaryBeamTable::toFITS() const throw(ConversionException) {
		throw ConversionException("Not implemented","CalPrimaryBeam");
	}

	void CalPrimaryBeamTable::fromFITS(char *fits) throw(ConversionException) {
		throw ConversionException("Not implemented","CalPrimaryBeam");
	}

	string CalPrimaryBeamTable::toVOTable() const throw(ConversionException) {
		throw ConversionException("Not implemented","CalPrimaryBeam");
	}

	void CalPrimaryBeamTable::fromVOTable(string vo) throw(ConversionException) {
		throw ConversionException("Not implemented","CalPrimaryBeam");
	}

	string CalPrimaryBeamTable::toXML()  throw(ConversionException) {
		string buf;
		buf.append("<?xml version=\"1.0\" encoding=\"ISO-8859-1\"?> ");
//		buf.append("<CalPrimaryBeamTable xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" xsi:noNamespaceSchemaLocation=\"../../idl/CalPrimaryBeamTable.xsd\"> ");
		buf.append("<?xml-stylesheet type=\"text/xsl\" href=\"../asdm2html/table2html.xsl\"?> ");		
		buf.append("<CalPrimaryBeamTable> ");
		buf.append(entity.toXML());
		string s = container.getEntity().toXML();
		// Change the "Entity" tag to "ContainerEntity".
		buf.append("<Container" + s.substr(1,s.length() - 1)+" ");
		vector<CalPrimaryBeamRow*> v = get();
		for (unsigned int i = 0; i < v.size(); ++i) {
			try {
				buf.append(v[i]->toXML());
			} catch (NoSuchRow e) {
			}
			buf.append("  ");
		}		
		buf.append("</CalPrimaryBeamTable> ");
		return buf;
	}
	
	void CalPrimaryBeamTable::fromXML(string xmlDoc) throw(ConversionException) {
		Parser xml(xmlDoc);
		if (!xml.isStr("<CalPrimaryBeamTable")) 
			error();
		// cout << "Parsing a CalPrimaryBeamTable" << endl;
		string s = xml.getElement("<Entity","/>");
		if (s.length() == 0) 
			error();
		Entity e;
		e.setFromXML(s);
		if (e.getEntityTypeName() != "CalPrimaryBeamTable")
			error();
		setEntity(e);
		// Skip the container's entity; but, it has to be there.
		s = xml.getElement("<ContainerEntity","/>");
		if (s.length() == 0) 
			error();

		// Get each row in the table.
		s = xml.getElementContent("<row>","</row>");
		CalPrimaryBeamRow *row;
		while (s.length() != 0) {
			// cout << "Parsing a CalPrimaryBeamRow" << endl; 
			row = newRow();
			row->setFromXML(s);
			try {
				checkAndAdd(row);
			} catch (DuplicateKey e1) {
				throw ConversionException(e1.getMessage(),"CalPrimaryBeamTable");
			} 
			catch (UniquenessViolationException e1) {
				throw ConversionException(e1.getMessage(),"CalPrimaryBeamTable");	
			}
			catch (...) {
				// cout << "Unexpected error in CalPrimaryBeamTable::checkAndAdd called from CalPrimaryBeamTable::fromXML " << endl;
			}
			s = xml.getElementContent("<row>","</row>");
		}
		if (!xml.isStr("</CalPrimaryBeamTable>")) 
			error();
	}

	void CalPrimaryBeamTable::error() throw(ConversionException) {
		throw ConversionException("Invalid xml document","CalPrimaryBeam");
	}
	
	string CalPrimaryBeamTable::toMIME() {
	 // To be implemented
		return "";
	}
	
	void CalPrimaryBeamTable::setFromMIME(const string & mimeMsg) {
		// To be implemented
		;
	}
	
	
	void CalPrimaryBeamTable::toFile(string directory) {
		if (!directoryExists(directory.c_str()) &&
			!createPath(directory.c_str())) {
			throw ConversionException("Could not create directory " , directory);
		}
		
		if (fileAsBin) {
			// write the bin serialized
			string fileName = directory + "/CalPrimaryBeam.bin";
			ofstream tableout(fileName.c_str(),ios::out|ios::trunc);
			if (tableout.rdstate() == ostream::failbit)
				throw ConversionException("Could not open file " + fileName + " to write ", "CalPrimaryBeam");
			tableout << toMIME() << endl;
			tableout.close();
			if (tableout.rdstate() == ostream::failbit)
				throw ConversionException("Could not close file " + fileName, "CalPrimaryBeam");
		}
		else {
			// write the XML
			string fileName = directory + "/CalPrimaryBeam.xml";
			ofstream tableout(fileName.c_str(),ios::out|ios::trunc);
			if (tableout.rdstate() == ostream::failbit)
				throw ConversionException("Could not open file " + fileName + " to write ", "CalPrimaryBeam");
			tableout << toXML() << endl;
			tableout.close();
			if (tableout.rdstate() == ostream::failbit)
				throw ConversionException("Could not close file " + fileName, "CalPrimaryBeam");
		}
	}
	
	void CalPrimaryBeamTable::setFromFile(const string& directory) {
		string tablename;
		if (fileAsBin)
			tablename = directory + "/CalPrimaryBeam.bin";
		else
			tablename = directory + "/CalPrimaryBeam.xml";
			
		// Determine the file size.
		ifstream::pos_type size;
		ifstream tablefile(tablename.c_str(), ios::in|ios::binary|ios::ate);

 		if (tablefile.is_open()) { 
  				size = tablefile.tellg(); 
  		}
		else {
				throw ConversionException("Could not open file " + tablename, "CalPrimaryBeam");
		}
		
		// Re position to the beginning.
		tablefile.seekg(0);
		
		// Read in a stringstream.
		stringstream ss;
		ss << tablefile.rdbuf();

		if (tablefile.rdstate() == istream::failbit || tablefile.rdstate() == istream::badbit) {
			throw ConversionException("Error reading file " + tablename,"CalPrimaryBeam");
		}

		// And close
		tablefile.close();
		if (tablefile.rdstate() == istream::failbit)
			throw ConversionException("Could not close file " + tablename,"CalPrimaryBeam");
					
		// And parse the content with the appropriate method
		if (fileAsBin) 
			setFromMIME(ss.str());
		else
			fromXML(ss.str());	
	}			
			
	
	

	
} // End namespace asdm
 
