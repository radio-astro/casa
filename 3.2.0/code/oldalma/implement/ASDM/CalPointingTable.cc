
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
 * File CalPointingTable.cpp
 */
#include <ConversionException.h>
#include <DuplicateKey.h>
#include <OutOfBoundsException.h>

using asdm::ConversionException;
using asdm::DuplicateKey;
using asdm::OutOfBoundsException;

#include <ASDM.h>
#include <CalPointingTable.h>
#include <CalPointingRow.h>
#include <Parser.h>

using asdm::ASDM;
using asdm::CalPointingTable;
using asdm::CalPointingRow;
using asdm::Parser;

#include <iostream>
#include <sstream>
#include <set>
using namespace std;

#include <Misc.h>
using namespace asdm;


namespace asdm {

	string CalPointingTable::tableName = "CalPointing";
	

	/**
	 * The list of field names that make up key key.
	 * (Initialization is in the constructor.)
	 */
	vector<string> CalPointingTable::key;

	/**
	 * Return the list of field names that make up key key
	 * as an array of strings.
	 */	
	vector<string> CalPointingTable::getKeyName() {
		return key;
	}


	CalPointingTable::CalPointingTable(ASDM &c) : container(c) {

	
		key.push_back("calDataId");
	
		key.push_back("calReductionId");
	
		key.push_back("antennaName");
	


		// Define a default entity.
		entity.setEntityId(EntityId("uid://X0/X0/X0"));
		entity.setEntityIdEncrypted("na");
		entity.setEntityTypeName("CalPointingTable");
		entity.setEntityVersion("1");
		entity.setInstanceVersion("1");
		
		// Archive XML
		archiveAsBin = false;
		
		// File XML
		fileAsBin = false;
	}
	
/**
 * A destructor for CalPointingTable.
 */
 
	CalPointingTable::~CalPointingTable() {
		for (unsigned int i = 0; i < privateRows.size(); i++) 
			delete(privateRows.at(i));
	}

	/**
	 * Container to which this table belongs.
	 */
	ASDM &CalPointingTable::getContainer() const {
		return container;
	}

	/**
	 * Return the number of rows in the table.
	 */

	unsigned int CalPointingTable::size() {
		return row.size();
	}	
	
	
	/**
	 * Return the name of this table.
	 */
	string CalPointingTable::getName() const {
		return tableName;
	}

	/**
	 * Return this table's Entity.
	 */
	Entity CalPointingTable::getEntity() const {
		return entity;
	}

	/**
	 * Set this table's Entity.
	 */
	void CalPointingTable::setEntity(Entity e) {
		this->entity = e; 
	}
	
	//
	// ====> Row creation.
	//
	
	/**
	 * Create a new row.
	 */
	CalPointingRow *CalPointingTable::newRow() {
		return new CalPointingRow (*this);
	}
	
	CalPointingRow *CalPointingTable::newRowEmpty() {
		return newRow ();
	}


	/**
	 * Create a new row initialized to the specified values.
	 * @return a pointer on the created and initialized row.
	
 	 * @param calDataId. 
	
 	 * @param calReductionId. 
	
 	 * @param antennaName. 
	
 	 * @param startValidTime. 
	
 	 * @param endValidTime. 
	
 	 * @param receiverBand. 
	
 	 * @param frequencyRange. 
	
 	 * @param direction. 
	
 	 * @param collOffsetRelative. 
	
 	 * @param collOffsetAbsolute. 
	
 	 * @param collError. 
	
 	 * @param pointingMethod. 
	
 	 * @param mode. 
	
 	 * @param ambientTemperature. 
	
     */
	CalPointingRow* CalPointingTable::newRow(Tag calDataId, Tag calReductionId, string antennaName, ArrayTime startValidTime, ArrayTime endValidTime, ReceiverBandMod::ReceiverBand receiverBand, vector<Frequency > frequencyRange, vector<Angle > direction, vector<Angle > collOffsetRelative, vector<Angle > collOffsetAbsolute, vector<Angle > collError, PointingMethodMod::PointingMethod pointingMethod, PointingModelModeMod::PointingModelMode mode, Temperature ambientTemperature){
		CalPointingRow *row = new CalPointingRow(*this);
			
		row->setCalDataId(calDataId);
			
		row->setCalReductionId(calReductionId);
			
		row->setAntennaName(antennaName);
			
		row->setStartValidTime(startValidTime);
			
		row->setEndValidTime(endValidTime);
			
		row->setReceiverBand(receiverBand);
			
		row->setFrequencyRange(frequencyRange);
			
		row->setDirection(direction);
			
		row->setCollOffsetRelative(collOffsetRelative);
			
		row->setCollOffsetAbsolute(collOffsetAbsolute);
			
		row->setCollError(collError);
			
		row->setPointingMethod(pointingMethod);
			
		row->setMode(mode);
			
		row->setAmbientTemperature(ambientTemperature);
	
		return row;		
	}	

	CalPointingRow* CalPointingTable::newRowFull(Tag calDataId, Tag calReductionId, string antennaName, ArrayTime startValidTime, ArrayTime endValidTime, ReceiverBandMod::ReceiverBand receiverBand, vector<Frequency > frequencyRange, vector<Angle > direction, vector<Angle > collOffsetRelative, vector<Angle > collOffsetAbsolute, vector<Angle > collError, PointingMethodMod::PointingMethod pointingMethod, PointingModelModeMod::PointingModelMode mode, Temperature ambientTemperature)	{
		CalPointingRow *row = new CalPointingRow(*this);
			
		row->setCalDataId(calDataId);
			
		row->setCalReductionId(calReductionId);
			
		row->setAntennaName(antennaName);
			
		row->setStartValidTime(startValidTime);
			
		row->setEndValidTime(endValidTime);
			
		row->setReceiverBand(receiverBand);
			
		row->setFrequencyRange(frequencyRange);
			
		row->setDirection(direction);
			
		row->setCollOffsetRelative(collOffsetRelative);
			
		row->setCollOffsetAbsolute(collOffsetAbsolute);
			
		row->setCollError(collError);
			
		row->setPointingMethod(pointingMethod);
			
		row->setMode(mode);
			
		row->setAmbientTemperature(ambientTemperature);
	
		return row;				
	}
	


CalPointingRow* CalPointingTable::newRow(CalPointingRow* row) {
	return new CalPointingRow(*this, *row);
}

CalPointingRow* CalPointingTable::newRowCopy(CalPointingRow* row) {
	return new CalPointingRow(*this, *row);
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
	CalPointingRow* CalPointingTable::add(CalPointingRow* x) {
		
		if (getRowByKey(
						x->getCalDataId()
						,
						x->getCalReductionId()
						,
						x->getAntennaName()
						))
			//throw DuplicateKey(x.getCalDataId() + "|" + x.getCalReductionId() + "|" + x.getAntennaName(),"CalPointing");
			throw DuplicateKey("Duplicate key exception in ","CalPointingTable");
		
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
	CalPointingRow*  CalPointingTable::checkAndAdd(CalPointingRow* x) throw (DuplicateKey) {
		
		
		if (getRowByKey(
	
			x->getCalDataId()
	,
			x->getCalReductionId()
	,
			x->getAntennaName()
			
		)) throw DuplicateKey("Duplicate key exception in ", "CalPointingTable");
		
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
	 * @return Alls rows as an array of CalPointingRow
	 */
	vector<CalPointingRow *> CalPointingTable::get() {
		return privateRows;
		// return row;
	}

	
/*
 ** Returns a CalPointingRow* given a key.
 ** @return a pointer to the row having the key whose values are passed as parameters, or 0 if
 ** no row exists for that key.
 **
 */
 	CalPointingRow* CalPointingTable::getRowByKey(Tag calDataId, Tag calReductionId, string antennaName)  {
	CalPointingRow* aRow = 0;
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
 	 		
 * @param startValidTime.
 	 		
 * @param endValidTime.
 	 		
 * @param receiverBand.
 	 		
 * @param frequencyRange.
 	 		
 * @param direction.
 	 		
 * @param collOffsetRelative.
 	 		
 * @param collOffsetAbsolute.
 	 		
 * @param collError.
 	 		
 * @param pointingMethod.
 	 		
 * @param mode.
 	 		
 * @param ambientTemperature.
 	 		 
 */
CalPointingRow* CalPointingTable::lookup(Tag calDataId, Tag calReductionId, string antennaName, ArrayTime startValidTime, ArrayTime endValidTime, ReceiverBandMod::ReceiverBand receiverBand, vector<Frequency > frequencyRange, vector<Angle > direction, vector<Angle > collOffsetRelative, vector<Angle > collOffsetAbsolute, vector<Angle > collError, PointingMethodMod::PointingMethod pointingMethod, PointingModelModeMod::PointingModelMode mode, Temperature ambientTemperature) {
		CalPointingRow* aRow;
		for (unsigned int i = 0; i < size(); i++) {
			aRow = row.at(i); 
			if (aRow->compareNoAutoInc(calDataId, calReductionId, antennaName, startValidTime, endValidTime, receiverBand, frequencyRange, direction, collOffsetRelative, collOffsetAbsolute, collError, pointingMethod, mode, ambientTemperature)) return aRow;
		}			
		return 0;	
} 
	
 	 	

	





#ifndef WITHOUT_ACS
	// Conversion Methods

	CalPointingTableIDL *CalPointingTable::toIDL() {
		CalPointingTableIDL *x = new CalPointingTableIDL ();
		unsigned int nrow = size();
		x->row.length(nrow);
		vector<CalPointingRow*> v = get();
		for (unsigned int i = 0; i < nrow; ++i) {
			x->row[i] = *(v[i]->toIDL());
		}
		return x;
	}
#endif
	
#ifndef WITHOUT_ACS
	void CalPointingTable::fromIDL(CalPointingTableIDL x) throw(DuplicateKey,ConversionException) {
		unsigned int nrow = x.row.length();
		for (unsigned int i = 0; i < nrow; ++i) {
			CalPointingRow *tmp = newRow();
			tmp->setFromIDL(x.row[i]);
			// checkAndAdd(tmp);
			add(tmp);
		}
	}
#endif

	char *CalPointingTable::toFITS() const throw(ConversionException) {
		throw ConversionException("Not implemented","CalPointing");
	}

	void CalPointingTable::fromFITS(char *fits) throw(ConversionException) {
		throw ConversionException("Not implemented","CalPointing");
	}

	string CalPointingTable::toVOTable() const throw(ConversionException) {
		throw ConversionException("Not implemented","CalPointing");
	}

	void CalPointingTable::fromVOTable(string vo) throw(ConversionException) {
		throw ConversionException("Not implemented","CalPointing");
	}

	string CalPointingTable::toXML()  throw(ConversionException) {
		string buf;
		buf.append("<?xml version=\"1.0\" encoding=\"ISO-8859-1\"?> ");
//		buf.append("<CalPointingTable xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" xsi:noNamespaceSchemaLocation=\"../../idl/CalPointingTable.xsd\"> ");
		buf.append("<?xml-stylesheet type=\"text/xsl\" href=\"../asdm2html/table2html.xsl\"?> ");		
		buf.append("<CalPointingTable> ");
		buf.append(entity.toXML());
		string s = container.getEntity().toXML();
		// Change the "Entity" tag to "ContainerEntity".
		buf.append("<Container" + s.substr(1,s.length() - 1)+" ");
		vector<CalPointingRow*> v = get();
		for (unsigned int i = 0; i < v.size(); ++i) {
			try {
				buf.append(v[i]->toXML());
			} catch (NoSuchRow e) {
			}
			buf.append("  ");
		}		
		buf.append("</CalPointingTable> ");
		return buf;
	}
	
	void CalPointingTable::fromXML(string xmlDoc) throw(ConversionException) {
		Parser xml(xmlDoc);
		if (!xml.isStr("<CalPointingTable")) 
			error();
		// cout << "Parsing a CalPointingTable" << endl;
		string s = xml.getElement("<Entity","/>");
		if (s.length() == 0) 
			error();
		Entity e;
		e.setFromXML(s);
		if (e.getEntityTypeName() != "CalPointingTable")
			error();
		setEntity(e);
		// Skip the container's entity; but, it has to be there.
		s = xml.getElement("<ContainerEntity","/>");
		if (s.length() == 0) 
			error();

		// Get each row in the table.
		s = xml.getElementContent("<row>","</row>");
		CalPointingRow *row;
		while (s.length() != 0) {
			// cout << "Parsing a CalPointingRow" << endl; 
			row = newRow();
			row->setFromXML(s);
			try {
				checkAndAdd(row);
			} catch (DuplicateKey e1) {
				throw ConversionException(e1.getMessage(),"CalPointingTable");
			} 
			catch (UniquenessViolationException e1) {
				throw ConversionException(e1.getMessage(),"CalPointingTable");	
			}
			catch (...) {
				// cout << "Unexpected error in CalPointingTable::checkAndAdd called from CalPointingTable::fromXML " << endl;
			}
			s = xml.getElementContent("<row>","</row>");
		}
		if (!xml.isStr("</CalPointingTable>")) 
			error();
	}

	void CalPointingTable::error() throw(ConversionException) {
		throw ConversionException("Invalid xml document","CalPointing");
	}
	
	string CalPointingTable::toMIME() {
	 // To be implemented
		return "";
	}
	
	void CalPointingTable::setFromMIME(const string & mimeMsg) {
		// To be implemented
		;
	}
	
	
	void CalPointingTable::toFile(string directory) {
		if (!directoryExists(directory.c_str()) &&
			!createPath(directory.c_str())) {
			throw ConversionException("Could not create directory " , directory);
		}
		
		if (fileAsBin) {
			// write the bin serialized
			string fileName = directory + "/CalPointing.bin";
			ofstream tableout(fileName.c_str(),ios::out|ios::trunc);
			if (tableout.rdstate() == ostream::failbit)
				throw ConversionException("Could not open file " + fileName + " to write ", "CalPointing");
			tableout << toMIME() << endl;
			tableout.close();
			if (tableout.rdstate() == ostream::failbit)
				throw ConversionException("Could not close file " + fileName, "CalPointing");
		}
		else {
			// write the XML
			string fileName = directory + "/CalPointing.xml";
			ofstream tableout(fileName.c_str(),ios::out|ios::trunc);
			if (tableout.rdstate() == ostream::failbit)
				throw ConversionException("Could not open file " + fileName + " to write ", "CalPointing");
			tableout << toXML() << endl;
			tableout.close();
			if (tableout.rdstate() == ostream::failbit)
				throw ConversionException("Could not close file " + fileName, "CalPointing");
		}
	}
	
	void CalPointingTable::setFromFile(const string& directory) {
		string tablename;
		if (fileAsBin)
			tablename = directory + "/CalPointing.bin";
		else
			tablename = directory + "/CalPointing.xml";
			
		// Determine the file size.
		ifstream::pos_type size;
		ifstream tablefile(tablename.c_str(), ios::in|ios::binary|ios::ate);

 		if (tablefile.is_open()) { 
  				size = tablefile.tellg(); 
  		}
		else {
				throw ConversionException("Could not open file " + tablename, "CalPointing");
		}
		
		// Re position to the beginning.
		tablefile.seekg(0);
		
		// Read in a stringstream.
		stringstream ss;
		ss << tablefile.rdbuf();

		if (tablefile.rdstate() == istream::failbit || tablefile.rdstate() == istream::badbit) {
			throw ConversionException("Error reading file " + tablename,"CalPointing");
		}

		// And close
		tablefile.close();
		if (tablefile.rdstate() == istream::failbit)
			throw ConversionException("Could not close file " + tablename,"CalPointing");
					
		// And parse the content with the appropriate method
		if (fileAsBin) 
			setFromMIME(ss.str());
		else
			fromXML(ss.str());	
	}			
			
	
	

	
} // End namespace asdm
 
