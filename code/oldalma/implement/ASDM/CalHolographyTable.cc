
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
 * File CalHolographyTable.cpp
 */
#include <ConversionException.h>
#include <DuplicateKey.h>
#include <OutOfBoundsException.h>

using asdm::ConversionException;
using asdm::DuplicateKey;
using asdm::OutOfBoundsException;

#include <ASDM.h>
#include <CalHolographyTable.h>
#include <CalHolographyRow.h>
#include <Parser.h>

using asdm::ASDM;
using asdm::CalHolographyTable;
using asdm::CalHolographyRow;
using asdm::Parser;

#include <iostream>
#include <sstream>
#include <set>
using namespace std;

#include <Misc.h>
using namespace asdm;


namespace asdm {

	string CalHolographyTable::tableName = "CalHolography";
	

	/**
	 * The list of field names that make up key key.
	 * (Initialization is in the constructor.)
	 */
	vector<string> CalHolographyTable::key;

	/**
	 * Return the list of field names that make up key key
	 * as an array of strings.
	 */	
	vector<string> CalHolographyTable::getKeyName() {
		return key;
	}


	CalHolographyTable::CalHolographyTable(ASDM &c) : container(c) {

	
		key.push_back("calDataId");
	
		key.push_back("calReductionId");
	
		key.push_back("antennaName");
	


		// Define a default entity.
		entity.setEntityId(EntityId("uid://X0/X0/X0"));
		entity.setEntityIdEncrypted("na");
		entity.setEntityTypeName("CalHolographyTable");
		entity.setEntityVersion("1");
		entity.setInstanceVersion("1");
		
		// Archive XML
		archiveAsBin = false;
		
		// File XML
		fileAsBin = false;
	}
	
/**
 * A destructor for CalHolographyTable.
 */
 
	CalHolographyTable::~CalHolographyTable() {
		for (unsigned int i = 0; i < privateRows.size(); i++) 
			delete(privateRows.at(i));
	}

	/**
	 * Container to which this table belongs.
	 */
	ASDM &CalHolographyTable::getContainer() const {
		return container;
	}

	/**
	 * Return the number of rows in the table.
	 */

	unsigned int CalHolographyTable::size() {
		return row.size();
	}	
	
	
	/**
	 * Return the name of this table.
	 */
	string CalHolographyTable::getName() const {
		return tableName;
	}

	/**
	 * Return this table's Entity.
	 */
	Entity CalHolographyTable::getEntity() const {
		return entity;
	}

	/**
	 * Set this table's Entity.
	 */
	void CalHolographyTable::setEntity(Entity e) {
		this->entity = e; 
	}
	
	//
	// ====> Row creation.
	//
	
	/**
	 * Create a new row.
	 */
	CalHolographyRow *CalHolographyTable::newRow() {
		return new CalHolographyRow (*this);
	}
	
	CalHolographyRow *CalHolographyTable::newRowEmpty() {
		return newRow ();
	}


	/**
	 * Create a new row initialized to the specified values.
	 * @return a pointer on the created and initialized row.
	
 	 * @param calDataId. 
	
 	 * @param calReductionId. 
	
 	 * @param antennaName. 
	
 	 * @param antennaMake. 
	
 	 * @param numScrew. 
	
 	 * @param numReceptor. 
	
 	 * @param startValidTime. 
	
 	 * @param endValidTime. 
	
 	 * @param focusPosition. 
	
 	 * @param rawRms. 
	
 	 * @param weightedRms. 
	
 	 * @param screwName. 
	
 	 * @param screwMotion. 
	
 	 * @param screwMotionError. 
	
 	 * @param panelModes. 
	
 	 * @param frequencyRange. 
	
 	 * @param beamMapUID. 
	
 	 * @param surfaceMapUID. 
	
 	 * @param polarizationTypes. 
	
 	 * @param receiverBand. 
	
 	 * @param direction. 
	
 	 * @param ambientTemperature. 
	
     */
	CalHolographyRow* CalHolographyTable::newRow(Tag calDataId, Tag calReductionId, string antennaName, AntennaMakeMod::AntennaMake antennaMake, int numScrew, int numReceptor, ArrayTime startValidTime, ArrayTime endValidTime, vector<Length > focusPosition, Length rawRms, Length weightedRms, vector<string > screwName, vector<Length > screwMotion, vector<Length > screwMotionError, int panelModes, vector<Frequency > frequencyRange, EntityRef beamMapUID, EntityRef surfaceMapUID, vector<PolarizationTypeMod::PolarizationType > polarizationTypes, ReceiverBandMod::ReceiverBand receiverBand, vector<Angle > direction, Temperature ambientTemperature){
		CalHolographyRow *row = new CalHolographyRow(*this);
			
		row->setCalDataId(calDataId);
			
		row->setCalReductionId(calReductionId);
			
		row->setAntennaName(antennaName);
			
		row->setAntennaMake(antennaMake);
			
		row->setNumScrew(numScrew);
			
		row->setNumReceptor(numReceptor);
			
		row->setStartValidTime(startValidTime);
			
		row->setEndValidTime(endValidTime);
			
		row->setFocusPosition(focusPosition);
			
		row->setRawRms(rawRms);
			
		row->setWeightedRms(weightedRms);
			
		row->setScrewName(screwName);
			
		row->setScrewMotion(screwMotion);
			
		row->setScrewMotionError(screwMotionError);
			
		row->setPanelModes(panelModes);
			
		row->setFrequencyRange(frequencyRange);
			
		row->setBeamMapUID(beamMapUID);
			
		row->setSurfaceMapUID(surfaceMapUID);
			
		row->setPolarizationTypes(polarizationTypes);
			
		row->setReceiverBand(receiverBand);
			
		row->setDirection(direction);
			
		row->setAmbientTemperature(ambientTemperature);
	
		return row;		
	}	

	CalHolographyRow* CalHolographyTable::newRowFull(Tag calDataId, Tag calReductionId, string antennaName, AntennaMakeMod::AntennaMake antennaMake, int numScrew, int numReceptor, ArrayTime startValidTime, ArrayTime endValidTime, vector<Length > focusPosition, Length rawRms, Length weightedRms, vector<string > screwName, vector<Length > screwMotion, vector<Length > screwMotionError, int panelModes, vector<Frequency > frequencyRange, EntityRef beamMapUID, EntityRef surfaceMapUID, vector<PolarizationTypeMod::PolarizationType > polarizationTypes, ReceiverBandMod::ReceiverBand receiverBand, vector<Angle > direction, Temperature ambientTemperature)	{
		CalHolographyRow *row = new CalHolographyRow(*this);
			
		row->setCalDataId(calDataId);
			
		row->setCalReductionId(calReductionId);
			
		row->setAntennaName(antennaName);
			
		row->setAntennaMake(antennaMake);
			
		row->setNumScrew(numScrew);
			
		row->setNumReceptor(numReceptor);
			
		row->setStartValidTime(startValidTime);
			
		row->setEndValidTime(endValidTime);
			
		row->setFocusPosition(focusPosition);
			
		row->setRawRms(rawRms);
			
		row->setWeightedRms(weightedRms);
			
		row->setScrewName(screwName);
			
		row->setScrewMotion(screwMotion);
			
		row->setScrewMotionError(screwMotionError);
			
		row->setPanelModes(panelModes);
			
		row->setFrequencyRange(frequencyRange);
			
		row->setBeamMapUID(beamMapUID);
			
		row->setSurfaceMapUID(surfaceMapUID);
			
		row->setPolarizationTypes(polarizationTypes);
			
		row->setReceiverBand(receiverBand);
			
		row->setDirection(direction);
			
		row->setAmbientTemperature(ambientTemperature);
	
		return row;				
	}
	


CalHolographyRow* CalHolographyTable::newRow(CalHolographyRow* row) {
	return new CalHolographyRow(*this, *row);
}

CalHolographyRow* CalHolographyTable::newRowCopy(CalHolographyRow* row) {
	return new CalHolographyRow(*this, *row);
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
	CalHolographyRow* CalHolographyTable::add(CalHolographyRow* x) {
		
		if (getRowByKey(
						x->getCalDataId()
						,
						x->getCalReductionId()
						,
						x->getAntennaName()
						))
			//throw DuplicateKey(x.getCalDataId() + "|" + x.getCalReductionId() + "|" + x.getAntennaName(),"CalHolography");
			throw DuplicateKey("Duplicate key exception in ","CalHolographyTable");
		
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
	CalHolographyRow*  CalHolographyTable::checkAndAdd(CalHolographyRow* x) throw (DuplicateKey) {
		
		
		if (getRowByKey(
	
			x->getCalDataId()
	,
			x->getCalReductionId()
	,
			x->getAntennaName()
			
		)) throw DuplicateKey("Duplicate key exception in ", "CalHolographyTable");
		
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
	 * @return Alls rows as an array of CalHolographyRow
	 */
	vector<CalHolographyRow *> CalHolographyTable::get() {
		return privateRows;
		// return row;
	}

	
/*
 ** Returns a CalHolographyRow* given a key.
 ** @return a pointer to the row having the key whose values are passed as parameters, or 0 if
 ** no row exists for that key.
 **
 */
 	CalHolographyRow* CalHolographyTable::getRowByKey(Tag calDataId, Tag calReductionId, string antennaName)  {
	CalHolographyRow* aRow = 0;
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
 	 		
 * @param numScrew.
 	 		
 * @param numReceptor.
 	 		
 * @param startValidTime.
 	 		
 * @param endValidTime.
 	 		
 * @param focusPosition.
 	 		
 * @param rawRms.
 	 		
 * @param weightedRms.
 	 		
 * @param screwName.
 	 		
 * @param screwMotion.
 	 		
 * @param screwMotionError.
 	 		
 * @param panelModes.
 	 		
 * @param frequencyRange.
 	 		
 * @param beamMapUID.
 	 		
 * @param surfaceMapUID.
 	 		
 * @param polarizationTypes.
 	 		
 * @param receiverBand.
 	 		
 * @param direction.
 	 		
 * @param ambientTemperature.
 	 		 
 */
CalHolographyRow* CalHolographyTable::lookup(Tag calDataId, Tag calReductionId, string antennaName, AntennaMakeMod::AntennaMake antennaMake, int numScrew, int numReceptor, ArrayTime startValidTime, ArrayTime endValidTime, vector<Length > focusPosition, Length rawRms, Length weightedRms, vector<string > screwName, vector<Length > screwMotion, vector<Length > screwMotionError, int panelModes, vector<Frequency > frequencyRange, EntityRef beamMapUID, EntityRef surfaceMapUID, vector<PolarizationTypeMod::PolarizationType > polarizationTypes, ReceiverBandMod::ReceiverBand receiverBand, vector<Angle > direction, Temperature ambientTemperature) {
		CalHolographyRow* aRow;
		for (unsigned int i = 0; i < size(); i++) {
			aRow = row.at(i); 
			if (aRow->compareNoAutoInc(calDataId, calReductionId, antennaName, antennaMake, numScrew, numReceptor, startValidTime, endValidTime, focusPosition, rawRms, weightedRms, screwName, screwMotion, screwMotionError, panelModes, frequencyRange, beamMapUID, surfaceMapUID, polarizationTypes, receiverBand, direction, ambientTemperature)) return aRow;
		}			
		return 0;	
} 
	
 	 	

	





#ifndef WITHOUT_ACS
	// Conversion Methods

	CalHolographyTableIDL *CalHolographyTable::toIDL() {
		CalHolographyTableIDL *x = new CalHolographyTableIDL ();
		unsigned int nrow = size();
		x->row.length(nrow);
		vector<CalHolographyRow*> v = get();
		for (unsigned int i = 0; i < nrow; ++i) {
			x->row[i] = *(v[i]->toIDL());
		}
		return x;
	}
#endif
	
#ifndef WITHOUT_ACS
	void CalHolographyTable::fromIDL(CalHolographyTableIDL x) throw(DuplicateKey,ConversionException) {
		unsigned int nrow = x.row.length();
		for (unsigned int i = 0; i < nrow; ++i) {
			CalHolographyRow *tmp = newRow();
			tmp->setFromIDL(x.row[i]);
			// checkAndAdd(tmp);
			add(tmp);
		}
	}
#endif

	char *CalHolographyTable::toFITS() const throw(ConversionException) {
		throw ConversionException("Not implemented","CalHolography");
	}

	void CalHolographyTable::fromFITS(char *fits) throw(ConversionException) {
		throw ConversionException("Not implemented","CalHolography");
	}

	string CalHolographyTable::toVOTable() const throw(ConversionException) {
		throw ConversionException("Not implemented","CalHolography");
	}

	void CalHolographyTable::fromVOTable(string vo) throw(ConversionException) {
		throw ConversionException("Not implemented","CalHolography");
	}

	string CalHolographyTable::toXML()  throw(ConversionException) {
		string buf;
		buf.append("<?xml version=\"1.0\" encoding=\"ISO-8859-1\"?> ");
//		buf.append("<CalHolographyTable xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" xsi:noNamespaceSchemaLocation=\"../../idl/CalHolographyTable.xsd\"> ");
		buf.append("<?xml-stylesheet type=\"text/xsl\" href=\"../asdm2html/table2html.xsl\"?> ");		
		buf.append("<CalHolographyTable> ");
		buf.append(entity.toXML());
		string s = container.getEntity().toXML();
		// Change the "Entity" tag to "ContainerEntity".
		buf.append("<Container" + s.substr(1,s.length() - 1)+" ");
		vector<CalHolographyRow*> v = get();
		for (unsigned int i = 0; i < v.size(); ++i) {
			try {
				buf.append(v[i]->toXML());
			} catch (NoSuchRow e) {
			}
			buf.append("  ");
		}		
		buf.append("</CalHolographyTable> ");
		return buf;
	}
	
	void CalHolographyTable::fromXML(string xmlDoc) throw(ConversionException) {
		Parser xml(xmlDoc);
		if (!xml.isStr("<CalHolographyTable")) 
			error();
		// cout << "Parsing a CalHolographyTable" << endl;
		string s = xml.getElement("<Entity","/>");
		if (s.length() == 0) 
			error();
		Entity e;
		e.setFromXML(s);
		if (e.getEntityTypeName() != "CalHolographyTable")
			error();
		setEntity(e);
		// Skip the container's entity; but, it has to be there.
		s = xml.getElement("<ContainerEntity","/>");
		if (s.length() == 0) 
			error();

		// Get each row in the table.
		s = xml.getElementContent("<row>","</row>");
		CalHolographyRow *row;
		while (s.length() != 0) {
			// cout << "Parsing a CalHolographyRow" << endl; 
			row = newRow();
			row->setFromXML(s);
			try {
				checkAndAdd(row);
			} catch (DuplicateKey e1) {
				throw ConversionException(e1.getMessage(),"CalHolographyTable");
			} 
			catch (UniquenessViolationException e1) {
				throw ConversionException(e1.getMessage(),"CalHolographyTable");	
			}
			catch (...) {
				// cout << "Unexpected error in CalHolographyTable::checkAndAdd called from CalHolographyTable::fromXML " << endl;
			}
			s = xml.getElementContent("<row>","</row>");
		}
		if (!xml.isStr("</CalHolographyTable>")) 
			error();
	}

	void CalHolographyTable::error() throw(ConversionException) {
		throw ConversionException("Invalid xml document","CalHolography");
	}
	
	string CalHolographyTable::toMIME() {
	 // To be implemented
		return "";
	}
	
	void CalHolographyTable::setFromMIME(const string & mimeMsg) {
		// To be implemented
		;
	}
	
	
	void CalHolographyTable::toFile(string directory) {
		if (!directoryExists(directory.c_str()) &&
			!createPath(directory.c_str())) {
			throw ConversionException("Could not create directory " , directory);
		}
		
		if (fileAsBin) {
			// write the bin serialized
			string fileName = directory + "/CalHolography.bin";
			ofstream tableout(fileName.c_str(),ios::out|ios::trunc);
			if (tableout.rdstate() == ostream::failbit)
				throw ConversionException("Could not open file " + fileName + " to write ", "CalHolography");
			tableout << toMIME() << endl;
			tableout.close();
			if (tableout.rdstate() == ostream::failbit)
				throw ConversionException("Could not close file " + fileName, "CalHolography");
		}
		else {
			// write the XML
			string fileName = directory + "/CalHolography.xml";
			ofstream tableout(fileName.c_str(),ios::out|ios::trunc);
			if (tableout.rdstate() == ostream::failbit)
				throw ConversionException("Could not open file " + fileName + " to write ", "CalHolography");
			tableout << toXML() << endl;
			tableout.close();
			if (tableout.rdstate() == ostream::failbit)
				throw ConversionException("Could not close file " + fileName, "CalHolography");
		}
	}
	
	void CalHolographyTable::setFromFile(const string& directory) {
		string tablename;
		if (fileAsBin)
			tablename = directory + "/CalHolography.bin";
		else
			tablename = directory + "/CalHolography.xml";
			
		// Determine the file size.
		ifstream::pos_type size;
		ifstream tablefile(tablename.c_str(), ios::in|ios::binary|ios::ate);

 		if (tablefile.is_open()) { 
  				size = tablefile.tellg(); 
  		}
		else {
				throw ConversionException("Could not open file " + tablename, "CalHolography");
		}
		
		// Re position to the beginning.
		tablefile.seekg(0);
		
		// Read in a stringstream.
		stringstream ss;
		ss << tablefile.rdbuf();

		if (tablefile.rdstate() == istream::failbit || tablefile.rdstate() == istream::badbit) {
			throw ConversionException("Error reading file " + tablename,"CalHolography");
		}

		// And close
		tablefile.close();
		if (tablefile.rdstate() == istream::failbit)
			throw ConversionException("Could not close file " + tablename,"CalHolography");
					
		// And parse the content with the appropriate method
		if (fileAsBin) 
			setFromMIME(ss.str());
		else
			fromXML(ss.str());	
	}			
			
	
	

	
} // End namespace asdm
 
