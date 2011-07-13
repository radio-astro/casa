
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
 * File CalPointingModelTable.cpp
 */
#include <ConversionException.h>
#include <DuplicateKey.h>
#include <OutOfBoundsException.h>

using asdm::ConversionException;
using asdm::DuplicateKey;
using asdm::OutOfBoundsException;

#include <ASDM.h>
#include <CalPointingModelTable.h>
#include <CalPointingModelRow.h>
#include <Parser.h>

using asdm::ASDM;
using asdm::CalPointingModelTable;
using asdm::CalPointingModelRow;
using asdm::Parser;

#include <iostream>
#include <sstream>
#include <set>
using namespace std;

#include <Misc.h>
using namespace asdm;


namespace asdm {

	string CalPointingModelTable::tableName = "CalPointingModel";
	

	/**
	 * The list of field names that make up key key.
	 * (Initialization is in the constructor.)
	 */
	vector<string> CalPointingModelTable::key;

	/**
	 * Return the list of field names that make up key key
	 * as an array of strings.
	 */	
	vector<string> CalPointingModelTable::getKeyName() {
		return key;
	}


	CalPointingModelTable::CalPointingModelTable(ASDM &c) : container(c) {

	
		key.push_back("calDataId");
	
		key.push_back("calReductionId");
	
		key.push_back("antennaName");
	
		key.push_back("receiverBand");
	


		// Define a default entity.
		entity.setEntityId(EntityId("uid://X0/X0/X0"));
		entity.setEntityIdEncrypted("na");
		entity.setEntityTypeName("CalPointingModelTable");
		entity.setEntityVersion("1");
		entity.setInstanceVersion("1");
		
		// Archive XML
		archiveAsBin = false;
		
		// File XML
		fileAsBin = false;
	}
	
/**
 * A destructor for CalPointingModelTable.
 */
 
	CalPointingModelTable::~CalPointingModelTable() {
		for (unsigned int i = 0; i < privateRows.size(); i++) 
			delete(privateRows.at(i));
	}

	/**
	 * Container to which this table belongs.
	 */
	ASDM &CalPointingModelTable::getContainer() const {
		return container;
	}

	/**
	 * Return the number of rows in the table.
	 */

	unsigned int CalPointingModelTable::size() {
		return row.size();
	}	
	
	
	/**
	 * Return the name of this table.
	 */
	string CalPointingModelTable::getName() const {
		return tableName;
	}

	/**
	 * Return this table's Entity.
	 */
	Entity CalPointingModelTable::getEntity() const {
		return entity;
	}

	/**
	 * Set this table's Entity.
	 */
	void CalPointingModelTable::setEntity(Entity e) {
		this->entity = e; 
	}
	
	//
	// ====> Row creation.
	//
	
	/**
	 * Create a new row.
	 */
	CalPointingModelRow *CalPointingModelTable::newRow() {
		return new CalPointingModelRow (*this);
	}
	
	CalPointingModelRow *CalPointingModelTable::newRowEmpty() {
		return newRow ();
	}


	/**
	 * Create a new row initialized to the specified values.
	 * @return a pointer on the created and initialized row.
	
 	 * @param calDataId. 
	
 	 * @param calReductionId. 
	
 	 * @param antennaName. 
	
 	 * @param receiverBand. 
	
 	 * @param antennaMake. 
	
 	 * @param numObs. 
	
 	 * @param numCoeff. 
	
 	 * @param startValidTime. 
	
 	 * @param endValidTime. 
	
 	 * @param numFormula. 
	
 	 * @param azimuthRms. 
	
 	 * @param elevationRms. 
	
 	 * @param skyRms. 
	
 	 * @param coeffName. 
	
 	 * @param coeffVal. 
	
 	 * @param coeffError. 
	
 	 * @param coeffFixed. 
	
 	 * @param coeffFormula. 
	
 	 * @param pointingModelMode. 
	
     */
	CalPointingModelRow* CalPointingModelTable::newRow(Tag calDataId, Tag calReductionId, string antennaName, ReceiverBandMod::ReceiverBand receiverBand, AntennaMakeMod::AntennaMake antennaMake, int numObs, int numCoeff, ArrayTime startValidTime, ArrayTime endValidTime, int numFormula, Angle azimuthRms, Angle elevationRms, Angle skyRms, vector<string > coeffName, vector<float > coeffVal, vector<float > coeffError, vector<bool > coeffFixed, vector<string > coeffFormula, PointingModelModeMod::PointingModelMode pointingModelMode){
		CalPointingModelRow *row = new CalPointingModelRow(*this);
			
		row->setCalDataId(calDataId);
			
		row->setCalReductionId(calReductionId);
			
		row->setAntennaName(antennaName);
			
		row->setReceiverBand(receiverBand);
			
		row->setAntennaMake(antennaMake);
			
		row->setNumObs(numObs);
			
		row->setNumCoeff(numCoeff);
			
		row->setStartValidTime(startValidTime);
			
		row->setEndValidTime(endValidTime);
			
		row->setNumFormula(numFormula);
			
		row->setAzimuthRms(azimuthRms);
			
		row->setElevationRms(elevationRms);
			
		row->setSkyRms(skyRms);
			
		row->setCoeffName(coeffName);
			
		row->setCoeffVal(coeffVal);
			
		row->setCoeffError(coeffError);
			
		row->setCoeffFixed(coeffFixed);
			
		row->setCoeffFormula(coeffFormula);
			
		row->setPointingModelMode(pointingModelMode);
	
		return row;		
	}	

	CalPointingModelRow* CalPointingModelTable::newRowFull(Tag calDataId, Tag calReductionId, string antennaName, ReceiverBandMod::ReceiverBand receiverBand, AntennaMakeMod::AntennaMake antennaMake, int numObs, int numCoeff, ArrayTime startValidTime, ArrayTime endValidTime, int numFormula, Angle azimuthRms, Angle elevationRms, Angle skyRms, vector<string > coeffName, vector<float > coeffVal, vector<float > coeffError, vector<bool > coeffFixed, vector<string > coeffFormula, PointingModelModeMod::PointingModelMode pointingModelMode)	{
		CalPointingModelRow *row = new CalPointingModelRow(*this);
			
		row->setCalDataId(calDataId);
			
		row->setCalReductionId(calReductionId);
			
		row->setAntennaName(antennaName);
			
		row->setReceiverBand(receiverBand);
			
		row->setAntennaMake(antennaMake);
			
		row->setNumObs(numObs);
			
		row->setNumCoeff(numCoeff);
			
		row->setStartValidTime(startValidTime);
			
		row->setEndValidTime(endValidTime);
			
		row->setNumFormula(numFormula);
			
		row->setAzimuthRms(azimuthRms);
			
		row->setElevationRms(elevationRms);
			
		row->setSkyRms(skyRms);
			
		row->setCoeffName(coeffName);
			
		row->setCoeffVal(coeffVal);
			
		row->setCoeffError(coeffError);
			
		row->setCoeffFixed(coeffFixed);
			
		row->setCoeffFormula(coeffFormula);
			
		row->setPointingModelMode(pointingModelMode);
	
		return row;				
	}
	


CalPointingModelRow* CalPointingModelTable::newRow(CalPointingModelRow* row) {
	return new CalPointingModelRow(*this, *row);
}

CalPointingModelRow* CalPointingModelTable::newRowCopy(CalPointingModelRow* row) {
	return new CalPointingModelRow(*this, *row);
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
	CalPointingModelRow* CalPointingModelTable::add(CalPointingModelRow* x) {
		
		if (getRowByKey(
						x->getCalDataId()
						,
						x->getCalReductionId()
						,
						x->getAntennaName()
						,
						x->getReceiverBand()
						))
			//throw DuplicateKey(x.getCalDataId() + "|" + x.getCalReductionId() + "|" + x.getAntennaName() + "|" + x.getReceiverBand(),"CalPointingModel");
			throw DuplicateKey("Duplicate key exception in ","CalPointingModelTable");
		
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
	CalPointingModelRow*  CalPointingModelTable::checkAndAdd(CalPointingModelRow* x) throw (DuplicateKey) {
		
		
		if (getRowByKey(
	
			x->getCalDataId()
	,
			x->getCalReductionId()
	,
			x->getAntennaName()
	,
			x->getReceiverBand()
			
		)) throw DuplicateKey("Duplicate key exception in ", "CalPointingModelTable");
		
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
	 * @return Alls rows as an array of CalPointingModelRow
	 */
	vector<CalPointingModelRow *> CalPointingModelTable::get() {
		return privateRows;
		// return row;
	}

	
/*
 ** Returns a CalPointingModelRow* given a key.
 ** @return a pointer to the row having the key whose values are passed as parameters, or 0 if
 ** no row exists for that key.
 **
 */
 	CalPointingModelRow* CalPointingModelTable::getRowByKey(Tag calDataId, Tag calReductionId, string antennaName, ReceiverBandMod::ReceiverBand receiverBand)  {
	CalPointingModelRow* aRow = 0;
	for (unsigned int i = 0; i < row.size(); i++) {
		aRow = row.at(i);
		
			
				if (aRow->calDataId != calDataId) continue;
			
		
			
				if (aRow->calReductionId != calReductionId) continue;
			
		
			
				if (aRow->antennaName != antennaName) continue;
			
		
			
				if (aRow->receiverBand != receiverBand) continue;
			
		
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
 	 		
 * @param receiverBand.
 	 		
 * @param antennaMake.
 	 		
 * @param numObs.
 	 		
 * @param numCoeff.
 	 		
 * @param startValidTime.
 	 		
 * @param endValidTime.
 	 		
 * @param numFormula.
 	 		
 * @param azimuthRms.
 	 		
 * @param elevationRms.
 	 		
 * @param skyRms.
 	 		
 * @param coeffName.
 	 		
 * @param coeffVal.
 	 		
 * @param coeffError.
 	 		
 * @param coeffFixed.
 	 		
 * @param coeffFormula.
 	 		
 * @param pointingModelMode.
 	 		 
 */
CalPointingModelRow* CalPointingModelTable::lookup(Tag calDataId, Tag calReductionId, string antennaName, ReceiverBandMod::ReceiverBand receiverBand, AntennaMakeMod::AntennaMake antennaMake, int numObs, int numCoeff, ArrayTime startValidTime, ArrayTime endValidTime, int numFormula, Angle azimuthRms, Angle elevationRms, Angle skyRms, vector<string > coeffName, vector<float > coeffVal, vector<float > coeffError, vector<bool > coeffFixed, vector<string > coeffFormula, PointingModelModeMod::PointingModelMode pointingModelMode) {
		CalPointingModelRow* aRow;
		for (unsigned int i = 0; i < size(); i++) {
			aRow = row.at(i); 
			if (aRow->compareNoAutoInc(calDataId, calReductionId, antennaName, receiverBand, antennaMake, numObs, numCoeff, startValidTime, endValidTime, numFormula, azimuthRms, elevationRms, skyRms, coeffName, coeffVal, coeffError, coeffFixed, coeffFormula, pointingModelMode)) return aRow;
		}			
		return 0;	
} 
	
 	 	

	





#ifndef WITHOUT_ACS
	// Conversion Methods

	CalPointingModelTableIDL *CalPointingModelTable::toIDL() {
		CalPointingModelTableIDL *x = new CalPointingModelTableIDL ();
		unsigned int nrow = size();
		x->row.length(nrow);
		vector<CalPointingModelRow*> v = get();
		for (unsigned int i = 0; i < nrow; ++i) {
			x->row[i] = *(v[i]->toIDL());
		}
		return x;
	}
#endif
	
#ifndef WITHOUT_ACS
	void CalPointingModelTable::fromIDL(CalPointingModelTableIDL x) throw(DuplicateKey,ConversionException) {
		unsigned int nrow = x.row.length();
		for (unsigned int i = 0; i < nrow; ++i) {
			CalPointingModelRow *tmp = newRow();
			tmp->setFromIDL(x.row[i]);
			// checkAndAdd(tmp);
			add(tmp);
		}
	}
#endif

	char *CalPointingModelTable::toFITS() const throw(ConversionException) {
		throw ConversionException("Not implemented","CalPointingModel");
	}

	void CalPointingModelTable::fromFITS(char *fits) throw(ConversionException) {
		throw ConversionException("Not implemented","CalPointingModel");
	}

	string CalPointingModelTable::toVOTable() const throw(ConversionException) {
		throw ConversionException("Not implemented","CalPointingModel");
	}

	void CalPointingModelTable::fromVOTable(string vo) throw(ConversionException) {
		throw ConversionException("Not implemented","CalPointingModel");
	}

	string CalPointingModelTable::toXML()  throw(ConversionException) {
		string buf;
		buf.append("<?xml version=\"1.0\" encoding=\"ISO-8859-1\"?> ");
//		buf.append("<CalPointingModelTable xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" xsi:noNamespaceSchemaLocation=\"../../idl/CalPointingModelTable.xsd\"> ");
		buf.append("<?xml-stylesheet type=\"text/xsl\" href=\"../asdm2html/table2html.xsl\"?> ");		
		buf.append("<CalPointingModelTable> ");
		buf.append(entity.toXML());
		string s = container.getEntity().toXML();
		// Change the "Entity" tag to "ContainerEntity".
		buf.append("<Container" + s.substr(1,s.length() - 1)+" ");
		vector<CalPointingModelRow*> v = get();
		for (unsigned int i = 0; i < v.size(); ++i) {
			try {
				buf.append(v[i]->toXML());
			} catch (NoSuchRow e) {
			}
			buf.append("  ");
		}		
		buf.append("</CalPointingModelTable> ");
		return buf;
	}
	
	void CalPointingModelTable::fromXML(string xmlDoc) throw(ConversionException) {
		Parser xml(xmlDoc);
		if (!xml.isStr("<CalPointingModelTable")) 
			error();
		// cout << "Parsing a CalPointingModelTable" << endl;
		string s = xml.getElement("<Entity","/>");
		if (s.length() == 0) 
			error();
		Entity e;
		e.setFromXML(s);
		if (e.getEntityTypeName() != "CalPointingModelTable")
			error();
		setEntity(e);
		// Skip the container's entity; but, it has to be there.
		s = xml.getElement("<ContainerEntity","/>");
		if (s.length() == 0) 
			error();

		// Get each row in the table.
		s = xml.getElementContent("<row>","</row>");
		CalPointingModelRow *row;
		while (s.length() != 0) {
			// cout << "Parsing a CalPointingModelRow" << endl; 
			row = newRow();
			row->setFromXML(s);
			try {
				checkAndAdd(row);
			} catch (DuplicateKey e1) {
				throw ConversionException(e1.getMessage(),"CalPointingModelTable");
			} 
			catch (UniquenessViolationException e1) {
				throw ConversionException(e1.getMessage(),"CalPointingModelTable");	
			}
			catch (...) {
				// cout << "Unexpected error in CalPointingModelTable::checkAndAdd called from CalPointingModelTable::fromXML " << endl;
			}
			s = xml.getElementContent("<row>","</row>");
		}
		if (!xml.isStr("</CalPointingModelTable>")) 
			error();
	}

	void CalPointingModelTable::error() throw(ConversionException) {
		throw ConversionException("Invalid xml document","CalPointingModel");
	}
	
	string CalPointingModelTable::toMIME() {
	 // To be implemented
		return "";
	}
	
	void CalPointingModelTable::setFromMIME(const string & mimeMsg) {
		// To be implemented
		;
	}
	
	
	void CalPointingModelTable::toFile(string directory) {
		if (!directoryExists(directory.c_str()) &&
			!createPath(directory.c_str())) {
			throw ConversionException("Could not create directory " , directory);
		}
		
		if (fileAsBin) {
			// write the bin serialized
			string fileName = directory + "/CalPointingModel.bin";
			ofstream tableout(fileName.c_str(),ios::out|ios::trunc);
			if (tableout.rdstate() == ostream::failbit)
				throw ConversionException("Could not open file " + fileName + " to write ", "CalPointingModel");
			tableout << toMIME() << endl;
			tableout.close();
			if (tableout.rdstate() == ostream::failbit)
				throw ConversionException("Could not close file " + fileName, "CalPointingModel");
		}
		else {
			// write the XML
			string fileName = directory + "/CalPointingModel.xml";
			ofstream tableout(fileName.c_str(),ios::out|ios::trunc);
			if (tableout.rdstate() == ostream::failbit)
				throw ConversionException("Could not open file " + fileName + " to write ", "CalPointingModel");
			tableout << toXML() << endl;
			tableout.close();
			if (tableout.rdstate() == ostream::failbit)
				throw ConversionException("Could not close file " + fileName, "CalPointingModel");
		}
	}
	
	void CalPointingModelTable::setFromFile(const string& directory) {
		string tablename;
		if (fileAsBin)
			tablename = directory + "/CalPointingModel.bin";
		else
			tablename = directory + "/CalPointingModel.xml";
			
		// Determine the file size.
		ifstream::pos_type size;
		ifstream tablefile(tablename.c_str(), ios::in|ios::binary|ios::ate);

 		if (tablefile.is_open()) { 
  				size = tablefile.tellg(); 
  		}
		else {
				throw ConversionException("Could not open file " + tablename, "CalPointingModel");
		}
		
		// Re position to the beginning.
		tablefile.seekg(0);
		
		// Read in a stringstream.
		stringstream ss;
		ss << tablefile.rdbuf();

		if (tablefile.rdstate() == istream::failbit || tablefile.rdstate() == istream::badbit) {
			throw ConversionException("Error reading file " + tablename,"CalPointingModel");
		}

		// And close
		tablefile.close();
		if (tablefile.rdstate() == istream::failbit)
			throw ConversionException("Could not close file " + tablename,"CalPointingModel");
					
		// And parse the content with the appropriate method
		if (fileAsBin) 
			setFromMIME(ss.str());
		else
			fromXML(ss.str());	
	}			
			
	
	

	
} // End namespace asdm
 
