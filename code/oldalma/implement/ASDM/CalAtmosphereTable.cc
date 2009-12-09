
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
 * File CalAtmosphereTable.cpp
 */
#include <ConversionException.h>
#include <DuplicateKey.h>
#include <OutOfBoundsException.h>

using asdm::ConversionException;
using asdm::DuplicateKey;
using asdm::OutOfBoundsException;

#include <ASDM.h>
#include <CalAtmosphereTable.h>
#include <CalAtmosphereRow.h>
#include <Parser.h>

using asdm::ASDM;
using asdm::CalAtmosphereTable;
using asdm::CalAtmosphereRow;
using asdm::Parser;

#include <iostream>
#include <sstream>
#include <set>
using namespace std;

#include <Misc.h>
using namespace asdm;


namespace asdm {

	string CalAtmosphereTable::tableName = "CalAtmosphere";
	

	/**
	 * The list of field names that make up key key.
	 * (Initialization is in the constructor.)
	 */
	vector<string> CalAtmosphereTable::key;

	/**
	 * Return the list of field names that make up key key
	 * as an array of strings.
	 */	
	vector<string> CalAtmosphereTable::getKeyName() {
		return key;
	}


	CalAtmosphereTable::CalAtmosphereTable(ASDM &c) : container(c) {

	
		key.push_back("calDataId");
	
		key.push_back("calReductionId");
	
		key.push_back("antennaName");
	


		// Define a default entity.
		entity.setEntityId(EntityId("uid://X0/X0/X0"));
		entity.setEntityIdEncrypted("na");
		entity.setEntityTypeName("CalAtmosphereTable");
		entity.setEntityVersion("1");
		entity.setInstanceVersion("1");
		
		// Archive XML
		archiveAsBin = false;
		
		// File XML
		fileAsBin = false;
	}
	
/**
 * A destructor for CalAtmosphereTable.
 */
 
	CalAtmosphereTable::~CalAtmosphereTable() {
		for (unsigned int i = 0; i < privateRows.size(); i++) 
			delete(privateRows.at(i));
	}

	/**
	 * Container to which this table belongs.
	 */
	ASDM &CalAtmosphereTable::getContainer() const {
		return container;
	}

	/**
	 * Return the number of rows in the table.
	 */

	unsigned int CalAtmosphereTable::size() {
		return row.size();
	}	
	
	
	/**
	 * Return the name of this table.
	 */
	string CalAtmosphereTable::getName() const {
		return tableName;
	}

	/**
	 * Return this table's Entity.
	 */
	Entity CalAtmosphereTable::getEntity() const {
		return entity;
	}

	/**
	 * Set this table's Entity.
	 */
	void CalAtmosphereTable::setEntity(Entity e) {
		this->entity = e; 
	}
	
	//
	// ====> Row creation.
	//
	
	/**
	 * Create a new row.
	 */
	CalAtmosphereRow *CalAtmosphereTable::newRow() {
		return new CalAtmosphereRow (*this);
	}
	
	CalAtmosphereRow *CalAtmosphereTable::newRowEmpty() {
		return newRow ();
	}


	/**
	 * Create a new row initialized to the specified values.
	 * @return a pointer on the created and initialized row.
	
 	 * @param calDataId. 
	
 	 * @param calReductionId. 
	
 	 * @param antennaName. 
	
 	 * @param numReceptor. 
	
 	 * @param numFreq. 
	
 	 * @param endValidTime. 
	
 	 * @param startValidTime. 
	
 	 * @param polarizationTypes. 
	
 	 * @param frequencyRange. 
	
 	 * @param frequencySpectrum. 
	
 	 * @param syscalType. 
	
 	 * @param tSysSpectrum. 
	
 	 * @param tRecSpectrum. 
	
 	 * @param tAtmSpectrum. 
	
 	 * @param tauSpectrum. 
	
 	 * @param sbGainSpectrum. 
	
 	 * @param forwardEffSpectrum. 
	
 	 * @param groundPressure. 
	
 	 * @param groundTemperature. 
	
 	 * @param groundRelHumidity. 
	
 	 * @param subType. 
	
 	 * @param receiverBand. 
	
     */
	CalAtmosphereRow* CalAtmosphereTable::newRow(Tag calDataId, Tag calReductionId, string antennaName, int numReceptor, int numFreq, ArrayTime endValidTime, ArrayTime startValidTime, vector<PolarizationTypeMod::PolarizationType > polarizationTypes, vector<Frequency > frequencyRange, vector<Frequency > frequencySpectrum, SyscalMethodMod::SyscalMethod syscalType, vector<vector<Temperature > > tSysSpectrum, vector<vector<Temperature > > tRecSpectrum, vector<vector<Temperature > > tAtmSpectrum, vector<vector<float > > tauSpectrum, vector<vector<float > > sbGainSpectrum, vector<vector<float > > forwardEffSpectrum, Pressure groundPressure, Temperature groundTemperature, Humidity groundRelHumidity, string subType, ReceiverBandMod::ReceiverBand receiverBand){
		CalAtmosphereRow *row = new CalAtmosphereRow(*this);
			
		row->setCalDataId(calDataId);
			
		row->setCalReductionId(calReductionId);
			
		row->setAntennaName(antennaName);
			
		row->setNumReceptor(numReceptor);
			
		row->setNumFreq(numFreq);
			
		row->setEndValidTime(endValidTime);
			
		row->setStartValidTime(startValidTime);
			
		row->setPolarizationTypes(polarizationTypes);
			
		row->setFrequencyRange(frequencyRange);
			
		row->setFrequencySpectrum(frequencySpectrum);
			
		row->setSyscalType(syscalType);
			
		row->setTSysSpectrum(tSysSpectrum);
			
		row->setTRecSpectrum(tRecSpectrum);
			
		row->setTAtmSpectrum(tAtmSpectrum);
			
		row->setTauSpectrum(tauSpectrum);
			
		row->setSbGainSpectrum(sbGainSpectrum);
			
		row->setForwardEffSpectrum(forwardEffSpectrum);
			
		row->setGroundPressure(groundPressure);
			
		row->setGroundTemperature(groundTemperature);
			
		row->setGroundRelHumidity(groundRelHumidity);
			
		row->setSubType(subType);
			
		row->setReceiverBand(receiverBand);
	
		return row;		
	}	

	CalAtmosphereRow* CalAtmosphereTable::newRowFull(Tag calDataId, Tag calReductionId, string antennaName, int numReceptor, int numFreq, ArrayTime endValidTime, ArrayTime startValidTime, vector<PolarizationTypeMod::PolarizationType > polarizationTypes, vector<Frequency > frequencyRange, vector<Frequency > frequencySpectrum, SyscalMethodMod::SyscalMethod syscalType, vector<vector<Temperature > > tSysSpectrum, vector<vector<Temperature > > tRecSpectrum, vector<vector<Temperature > > tAtmSpectrum, vector<vector<float > > tauSpectrum, vector<vector<float > > sbGainSpectrum, vector<vector<float > > forwardEffSpectrum, Pressure groundPressure, Temperature groundTemperature, Humidity groundRelHumidity, string subType, ReceiverBandMod::ReceiverBand receiverBand)	{
		CalAtmosphereRow *row = new CalAtmosphereRow(*this);
			
		row->setCalDataId(calDataId);
			
		row->setCalReductionId(calReductionId);
			
		row->setAntennaName(antennaName);
			
		row->setNumReceptor(numReceptor);
			
		row->setNumFreq(numFreq);
			
		row->setEndValidTime(endValidTime);
			
		row->setStartValidTime(startValidTime);
			
		row->setPolarizationTypes(polarizationTypes);
			
		row->setFrequencyRange(frequencyRange);
			
		row->setFrequencySpectrum(frequencySpectrum);
			
		row->setSyscalType(syscalType);
			
		row->setTSysSpectrum(tSysSpectrum);
			
		row->setTRecSpectrum(tRecSpectrum);
			
		row->setTAtmSpectrum(tAtmSpectrum);
			
		row->setTauSpectrum(tauSpectrum);
			
		row->setSbGainSpectrum(sbGainSpectrum);
			
		row->setForwardEffSpectrum(forwardEffSpectrum);
			
		row->setGroundPressure(groundPressure);
			
		row->setGroundTemperature(groundTemperature);
			
		row->setGroundRelHumidity(groundRelHumidity);
			
		row->setSubType(subType);
			
		row->setReceiverBand(receiverBand);
	
		return row;				
	}
	


CalAtmosphereRow* CalAtmosphereTable::newRow(CalAtmosphereRow* row) {
	return new CalAtmosphereRow(*this, *row);
}

CalAtmosphereRow* CalAtmosphereTable::newRowCopy(CalAtmosphereRow* row) {
	return new CalAtmosphereRow(*this, *row);
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
	CalAtmosphereRow* CalAtmosphereTable::add(CalAtmosphereRow* x) {
		
		if (getRowByKey(
						x->getCalDataId()
						,
						x->getCalReductionId()
						,
						x->getAntennaName()
						))
			//throw DuplicateKey(x.getCalDataId() + "|" + x.getCalReductionId() + "|" + x.getAntennaName(),"CalAtmosphere");
			throw DuplicateKey("Duplicate key exception in ","CalAtmosphereTable");
		
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
	CalAtmosphereRow*  CalAtmosphereTable::checkAndAdd(CalAtmosphereRow* x) throw (DuplicateKey) {
		
		
		if (getRowByKey(
	
			x->getCalDataId()
	,
			x->getCalReductionId()
	,
			x->getAntennaName()
			
		)) throw DuplicateKey("Duplicate key exception in ", "CalAtmosphereTable");
		
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
	 * @return Alls rows as an array of CalAtmosphereRow
	 */
	vector<CalAtmosphereRow *> CalAtmosphereTable::get() {
		return privateRows;
		// return row;
	}

	
/*
 ** Returns a CalAtmosphereRow* given a key.
 ** @return a pointer to the row having the key whose values are passed as parameters, or 0 if
 ** no row exists for that key.
 **
 */
 	CalAtmosphereRow* CalAtmosphereTable::getRowByKey(Tag calDataId, Tag calReductionId, string antennaName)  {
	CalAtmosphereRow* aRow = 0;
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
 	 		
 * @param numReceptor.
 	 		
 * @param numFreq.
 	 		
 * @param endValidTime.
 	 		
 * @param startValidTime.
 	 		
 * @param polarizationTypes.
 	 		
 * @param frequencyRange.
 	 		
 * @param frequencySpectrum.
 	 		
 * @param syscalType.
 	 		
 * @param tSysSpectrum.
 	 		
 * @param tRecSpectrum.
 	 		
 * @param tAtmSpectrum.
 	 		
 * @param tauSpectrum.
 	 		
 * @param sbGainSpectrum.
 	 		
 * @param forwardEffSpectrum.
 	 		
 * @param groundPressure.
 	 		
 * @param groundTemperature.
 	 		
 * @param groundRelHumidity.
 	 		
 * @param subType.
 	 		
 * @param receiverBand.
 	 		 
 */
CalAtmosphereRow* CalAtmosphereTable::lookup(Tag calDataId, Tag calReductionId, string antennaName, int numReceptor, int numFreq, ArrayTime endValidTime, ArrayTime startValidTime, vector<PolarizationTypeMod::PolarizationType > polarizationTypes, vector<Frequency > frequencyRange, vector<Frequency > frequencySpectrum, SyscalMethodMod::SyscalMethod syscalType, vector<vector<Temperature > > tSysSpectrum, vector<vector<Temperature > > tRecSpectrum, vector<vector<Temperature > > tAtmSpectrum, vector<vector<float > > tauSpectrum, vector<vector<float > > sbGainSpectrum, vector<vector<float > > forwardEffSpectrum, Pressure groundPressure, Temperature groundTemperature, Humidity groundRelHumidity, string subType, ReceiverBandMod::ReceiverBand receiverBand) {
		CalAtmosphereRow* aRow;
		for (unsigned int i = 0; i < size(); i++) {
			aRow = row.at(i); 
			if (aRow->compareNoAutoInc(calDataId, calReductionId, antennaName, numReceptor, numFreq, endValidTime, startValidTime, polarizationTypes, frequencyRange, frequencySpectrum, syscalType, tSysSpectrum, tRecSpectrum, tAtmSpectrum, tauSpectrum, sbGainSpectrum, forwardEffSpectrum, groundPressure, groundTemperature, groundRelHumidity, subType, receiverBand)) return aRow;
		}			
		return 0;	
} 
	
 	 	

	





#ifndef WITHOUT_ACS
	// Conversion Methods

	CalAtmosphereTableIDL *CalAtmosphereTable::toIDL() {
		CalAtmosphereTableIDL *x = new CalAtmosphereTableIDL ();
		unsigned int nrow = size();
		x->row.length(nrow);
		vector<CalAtmosphereRow*> v = get();
		for (unsigned int i = 0; i < nrow; ++i) {
			x->row[i] = *(v[i]->toIDL());
		}
		return x;
	}
#endif
	
#ifndef WITHOUT_ACS
	void CalAtmosphereTable::fromIDL(CalAtmosphereTableIDL x) throw(DuplicateKey,ConversionException) {
		unsigned int nrow = x.row.length();
		for (unsigned int i = 0; i < nrow; ++i) {
			CalAtmosphereRow *tmp = newRow();
			tmp->setFromIDL(x.row[i]);
			// checkAndAdd(tmp);
			add(tmp);
		}
	}
#endif

	char *CalAtmosphereTable::toFITS() const throw(ConversionException) {
		throw ConversionException("Not implemented","CalAtmosphere");
	}

	void CalAtmosphereTable::fromFITS(char *fits) throw(ConversionException) {
		throw ConversionException("Not implemented","CalAtmosphere");
	}

	string CalAtmosphereTable::toVOTable() const throw(ConversionException) {
		throw ConversionException("Not implemented","CalAtmosphere");
	}

	void CalAtmosphereTable::fromVOTable(string vo) throw(ConversionException) {
		throw ConversionException("Not implemented","CalAtmosphere");
	}

	string CalAtmosphereTable::toXML()  throw(ConversionException) {
		string buf;
		buf.append("<?xml version=\"1.0\" encoding=\"ISO-8859-1\"?> ");
//		buf.append("<CalAtmosphereTable xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" xsi:noNamespaceSchemaLocation=\"../../idl/CalAtmosphereTable.xsd\"> ");
		buf.append("<?xml-stylesheet type=\"text/xsl\" href=\"../asdm2html/table2html.xsl\"?> ");		
		buf.append("<CalAtmosphereTable> ");
		buf.append(entity.toXML());
		string s = container.getEntity().toXML();
		// Change the "Entity" tag to "ContainerEntity".
		buf.append("<Container" + s.substr(1,s.length() - 1)+" ");
		vector<CalAtmosphereRow*> v = get();
		for (unsigned int i = 0; i < v.size(); ++i) {
			try {
				buf.append(v[i]->toXML());
			} catch (NoSuchRow e) {
			}
			buf.append("  ");
		}		
		buf.append("</CalAtmosphereTable> ");
		return buf;
	}
	
	void CalAtmosphereTable::fromXML(string xmlDoc) throw(ConversionException) {
		Parser xml(xmlDoc);
		if (!xml.isStr("<CalAtmosphereTable")) 
			error();
		// cout << "Parsing a CalAtmosphereTable" << endl;
		string s = xml.getElement("<Entity","/>");
		if (s.length() == 0) 
			error();
		Entity e;
		e.setFromXML(s);
		if (e.getEntityTypeName() != "CalAtmosphereTable")
			error();
		setEntity(e);
		// Skip the container's entity; but, it has to be there.
		s = xml.getElement("<ContainerEntity","/>");
		if (s.length() == 0) 
			error();

		// Get each row in the table.
		s = xml.getElementContent("<row>","</row>");
		CalAtmosphereRow *row;
		while (s.length() != 0) {
			// cout << "Parsing a CalAtmosphereRow" << endl; 
			row = newRow();
			row->setFromXML(s);
			try {
				checkAndAdd(row);
			} catch (DuplicateKey e1) {
				throw ConversionException(e1.getMessage(),"CalAtmosphereTable");
			} 
			catch (UniquenessViolationException e1) {
				throw ConversionException(e1.getMessage(),"CalAtmosphereTable");	
			}
			catch (...) {
				// cout << "Unexpected error in CalAtmosphereTable::checkAndAdd called from CalAtmosphereTable::fromXML " << endl;
			}
			s = xml.getElementContent("<row>","</row>");
		}
		if (!xml.isStr("</CalAtmosphereTable>")) 
			error();
	}

	void CalAtmosphereTable::error() throw(ConversionException) {
		throw ConversionException("Invalid xml document","CalAtmosphere");
	}
	
	string CalAtmosphereTable::toMIME() {
	 // To be implemented
		return "";
	}
	
	void CalAtmosphereTable::setFromMIME(const string & mimeMsg) {
		// To be implemented
		;
	}
	
	
	void CalAtmosphereTable::toFile(string directory) {
		if (!directoryExists(directory.c_str()) &&
			!createPath(directory.c_str())) {
			throw ConversionException("Could not create directory " , directory);
		}
		
		if (fileAsBin) {
			// write the bin serialized
			string fileName = directory + "/CalAtmosphere.bin";
			ofstream tableout(fileName.c_str(),ios::out|ios::trunc);
			if (tableout.rdstate() == ostream::failbit)
				throw ConversionException("Could not open file " + fileName + " to write ", "CalAtmosphere");
			tableout << toMIME() << endl;
			tableout.close();
			if (tableout.rdstate() == ostream::failbit)
				throw ConversionException("Could not close file " + fileName, "CalAtmosphere");
		}
		else {
			// write the XML
			string fileName = directory + "/CalAtmosphere.xml";
			ofstream tableout(fileName.c_str(),ios::out|ios::trunc);
			if (tableout.rdstate() == ostream::failbit)
				throw ConversionException("Could not open file " + fileName + " to write ", "CalAtmosphere");
			tableout << toXML() << endl;
			tableout.close();
			if (tableout.rdstate() == ostream::failbit)
				throw ConversionException("Could not close file " + fileName, "CalAtmosphere");
		}
	}
	
	void CalAtmosphereTable::setFromFile(const string& directory) {
		string tablename;
		if (fileAsBin)
			tablename = directory + "/CalAtmosphere.bin";
		else
			tablename = directory + "/CalAtmosphere.xml";
			
		// Determine the file size.
		ifstream::pos_type size;
		ifstream tablefile(tablename.c_str(), ios::in|ios::binary|ios::ate);

 		if (tablefile.is_open()) { 
  				size = tablefile.tellg(); 
  		}
		else {
				throw ConversionException("Could not open file " + tablename, "CalAtmosphere");
		}
		
		// Re position to the beginning.
		tablefile.seekg(0);
		
		// Read in a stringstream.
		stringstream ss;
		ss << tablefile.rdbuf();

		if (tablefile.rdstate() == istream::failbit || tablefile.rdstate() == istream::badbit) {
			throw ConversionException("Error reading file " + tablename,"CalAtmosphere");
		}

		// And close
		tablefile.close();
		if (tablefile.rdstate() == istream::failbit)
			throw ConversionException("Could not close file " + tablename,"CalAtmosphere");
					
		// And parse the content with the appropriate method
		if (fileAsBin) 
			setFromMIME(ss.str());
		else
			fromXML(ss.str());	
	}			
			
	
	

	
} // End namespace asdm
 
