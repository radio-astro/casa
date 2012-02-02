
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
 * File CalCurveTable.cpp
 */
#include <ConversionException.h>
#include <DuplicateKey.h>
#include <OutOfBoundsException.h>

using asdm::ConversionException;
using asdm::DuplicateKey;
using asdm::OutOfBoundsException;

#include <ASDM.h>
#include <CalCurveTable.h>
#include <CalCurveRow.h>
#include <Parser.h>

using asdm::ASDM;
using asdm::CalCurveTable;
using asdm::CalCurveRow;
using asdm::Parser;

#include <iostream>
#include <sstream>
#include <set>
using namespace std;

#include <Misc.h>
using namespace asdm;


namespace asdm {

	string CalCurveTable::tableName = "CalCurve";
	

	/**
	 * The list of field names that make up key key.
	 * (Initialization is in the constructor.)
	 */
	vector<string> CalCurveTable::key;

	/**
	 * Return the list of field names that make up key key
	 * as an array of strings.
	 */	
	vector<string> CalCurveTable::getKeyName() {
		return key;
	}


	CalCurveTable::CalCurveTable(ASDM &c) : container(c) {

	
		key.push_back("calDataId");
	
		key.push_back("calReductionId");
	


		// Define a default entity.
		entity.setEntityId(EntityId("uid://X0/X0/X0"));
		entity.setEntityIdEncrypted("na");
		entity.setEntityTypeName("CalCurveTable");
		entity.setEntityVersion("1");
		entity.setInstanceVersion("1");
		
		// Archive XML
		archiveAsBin = false;
		
		// File XML
		fileAsBin = false;
	}
	
/**
 * A destructor for CalCurveTable.
 */
 
	CalCurveTable::~CalCurveTable() {
		for (unsigned int i = 0; i < privateRows.size(); i++) 
			delete(privateRows.at(i));
	}

	/**
	 * Container to which this table belongs.
	 */
	ASDM &CalCurveTable::getContainer() const {
		return container;
	}

	/**
	 * Return the number of rows in the table.
	 */

	unsigned int CalCurveTable::size() {
		return row.size();
	}	
	
	
	/**
	 * Return the name of this table.
	 */
	string CalCurveTable::getName() const {
		return tableName;
	}

	/**
	 * Return this table's Entity.
	 */
	Entity CalCurveTable::getEntity() const {
		return entity;
	}

	/**
	 * Set this table's Entity.
	 */
	void CalCurveTable::setEntity(Entity e) {
		this->entity = e; 
	}
	
	//
	// ====> Row creation.
	//
	
	/**
	 * Create a new row.
	 */
	CalCurveRow *CalCurveTable::newRow() {
		return new CalCurveRow (*this);
	}
	
	CalCurveRow *CalCurveTable::newRowEmpty() {
		return newRow ();
	}


	/**
	 * Create a new row initialized to the specified values.
	 * @return a pointer on the created and initialized row.
	
 	 * @param calDataId. 
	
 	 * @param calReductionId. 
	
 	 * @param numAntenna. 
	
 	 * @param numBaseline. 
	
 	 * @param numAPC. 
	
 	 * @param numReceptor. 
	
 	 * @param numPoly. 
	
 	 * @param antennaNames. 
	
 	 * @param refAntennaName. 
	
 	 * @param receiverBand. 
	
 	 * @param atmPhaseCorrections. 
	
 	 * @param polarizationTypes. 
	
 	 * @param startValidTime. 
	
 	 * @param endValidTime. 
	
 	 * @param frequencyRange. 
	
 	 * @param typeCurve. 
	
 	 * @param timeOrigin. 
	
 	 * @param curve. 
	
 	 * @param rms. 
	
     */
	CalCurveRow* CalCurveTable::newRow(Tag calDataId, Tag calReductionId, int numAntenna, int numBaseline, int numAPC, int numReceptor, int numPoly, vector<string > antennaNames, string refAntennaName, ReceiverBandMod::ReceiverBand receiverBand, vector<AtmPhaseCorrectionMod::AtmPhaseCorrection > atmPhaseCorrections, vector<PolarizationTypeMod::PolarizationType > polarizationTypes, ArrayTime startValidTime, ArrayTime endValidTime, vector<Frequency > frequencyRange, CalCurveTypeMod::CalCurveType typeCurve, ArrayTime timeOrigin, vector<vector<vector<vector<float > > > > curve, vector<vector<vector<float > > > rms){
		CalCurveRow *row = new CalCurveRow(*this);
			
		row->setCalDataId(calDataId);
			
		row->setCalReductionId(calReductionId);
			
		row->setNumAntenna(numAntenna);
			
		row->setNumBaseline(numBaseline);
			
		row->setNumAPC(numAPC);
			
		row->setNumReceptor(numReceptor);
			
		row->setNumPoly(numPoly);
			
		row->setAntennaNames(antennaNames);
			
		row->setRefAntennaName(refAntennaName);
			
		row->setReceiverBand(receiverBand);
			
		row->setAtmPhaseCorrections(atmPhaseCorrections);
			
		row->setPolarizationTypes(polarizationTypes);
			
		row->setStartValidTime(startValidTime);
			
		row->setEndValidTime(endValidTime);
			
		row->setFrequencyRange(frequencyRange);
			
		row->setTypeCurve(typeCurve);
			
		row->setTimeOrigin(timeOrigin);
			
		row->setCurve(curve);
			
		row->setRms(rms);
	
		return row;		
	}	

	CalCurveRow* CalCurveTable::newRowFull(Tag calDataId, Tag calReductionId, int numAntenna, int numBaseline, int numAPC, int numReceptor, int numPoly, vector<string > antennaNames, string refAntennaName, ReceiverBandMod::ReceiverBand receiverBand, vector<AtmPhaseCorrectionMod::AtmPhaseCorrection > atmPhaseCorrections, vector<PolarizationTypeMod::PolarizationType > polarizationTypes, ArrayTime startValidTime, ArrayTime endValidTime, vector<Frequency > frequencyRange, CalCurveTypeMod::CalCurveType typeCurve, ArrayTime timeOrigin, vector<vector<vector<vector<float > > > > curve, vector<vector<vector<float > > > rms)	{
		CalCurveRow *row = new CalCurveRow(*this);
			
		row->setCalDataId(calDataId);
			
		row->setCalReductionId(calReductionId);
			
		row->setNumAntenna(numAntenna);
			
		row->setNumBaseline(numBaseline);
			
		row->setNumAPC(numAPC);
			
		row->setNumReceptor(numReceptor);
			
		row->setNumPoly(numPoly);
			
		row->setAntennaNames(antennaNames);
			
		row->setRefAntennaName(refAntennaName);
			
		row->setReceiverBand(receiverBand);
			
		row->setAtmPhaseCorrections(atmPhaseCorrections);
			
		row->setPolarizationTypes(polarizationTypes);
			
		row->setStartValidTime(startValidTime);
			
		row->setEndValidTime(endValidTime);
			
		row->setFrequencyRange(frequencyRange);
			
		row->setTypeCurve(typeCurve);
			
		row->setTimeOrigin(timeOrigin);
			
		row->setCurve(curve);
			
		row->setRms(rms);
	
		return row;				
	}
	


CalCurveRow* CalCurveTable::newRow(CalCurveRow* row) {
	return new CalCurveRow(*this, *row);
}

CalCurveRow* CalCurveTable::newRowCopy(CalCurveRow* row) {
	return new CalCurveRow(*this, *row);
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
	CalCurveRow* CalCurveTable::add(CalCurveRow* x) {
		
		if (getRowByKey(
						x->getCalDataId()
						,
						x->getCalReductionId()
						))
			//throw DuplicateKey(x.getCalDataId() + "|" + x.getCalReductionId(),"CalCurve");
			throw DuplicateKey("Duplicate key exception in ","CalCurveTable");
		
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
	CalCurveRow*  CalCurveTable::checkAndAdd(CalCurveRow* x) throw (DuplicateKey) {
		
		
		if (getRowByKey(
	
			x->getCalDataId()
	,
			x->getCalReductionId()
			
		)) throw DuplicateKey("Duplicate key exception in ", "CalCurveTable");
		
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
	 * @return Alls rows as an array of CalCurveRow
	 */
	vector<CalCurveRow *> CalCurveTable::get() {
		return privateRows;
		// return row;
	}

	
/*
 ** Returns a CalCurveRow* given a key.
 ** @return a pointer to the row having the key whose values are passed as parameters, or 0 if
 ** no row exists for that key.
 **
 */
 	CalCurveRow* CalCurveTable::getRowByKey(Tag calDataId, Tag calReductionId)  {
	CalCurveRow* aRow = 0;
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
 	 		
 * @param numAntenna.
 	 		
 * @param numBaseline.
 	 		
 * @param numAPC.
 	 		
 * @param numReceptor.
 	 		
 * @param numPoly.
 	 		
 * @param antennaNames.
 	 		
 * @param refAntennaName.
 	 		
 * @param receiverBand.
 	 		
 * @param atmPhaseCorrections.
 	 		
 * @param polarizationTypes.
 	 		
 * @param startValidTime.
 	 		
 * @param endValidTime.
 	 		
 * @param frequencyRange.
 	 		
 * @param typeCurve.
 	 		
 * @param timeOrigin.
 	 		
 * @param curve.
 	 		
 * @param rms.
 	 		 
 */
CalCurveRow* CalCurveTable::lookup(Tag calDataId, Tag calReductionId, int numAntenna, int numBaseline, int numAPC, int numReceptor, int numPoly, vector<string > antennaNames, string refAntennaName, ReceiverBandMod::ReceiverBand receiverBand, vector<AtmPhaseCorrectionMod::AtmPhaseCorrection > atmPhaseCorrections, vector<PolarizationTypeMod::PolarizationType > polarizationTypes, ArrayTime startValidTime, ArrayTime endValidTime, vector<Frequency > frequencyRange, CalCurveTypeMod::CalCurveType typeCurve, ArrayTime timeOrigin, vector<vector<vector<vector<float > > > > curve, vector<vector<vector<float > > > rms) {
		CalCurveRow* aRow;
		for (unsigned int i = 0; i < size(); i++) {
			aRow = row.at(i); 
			if (aRow->compareNoAutoInc(calDataId, calReductionId, numAntenna, numBaseline, numAPC, numReceptor, numPoly, antennaNames, refAntennaName, receiverBand, atmPhaseCorrections, polarizationTypes, startValidTime, endValidTime, frequencyRange, typeCurve, timeOrigin, curve, rms)) return aRow;
		}			
		return 0;	
} 
	
 	 	

	





#ifndef WITHOUT_ACS
	// Conversion Methods

	CalCurveTableIDL *CalCurveTable::toIDL() {
		CalCurveTableIDL *x = new CalCurveTableIDL ();
		unsigned int nrow = size();
		x->row.length(nrow);
		vector<CalCurveRow*> v = get();
		for (unsigned int i = 0; i < nrow; ++i) {
			x->row[i] = *(v[i]->toIDL());
		}
		return x;
	}
#endif
	
#ifndef WITHOUT_ACS
	void CalCurveTable::fromIDL(CalCurveTableIDL x) throw(DuplicateKey,ConversionException) {
		unsigned int nrow = x.row.length();
		for (unsigned int i = 0; i < nrow; ++i) {
			CalCurveRow *tmp = newRow();
			tmp->setFromIDL(x.row[i]);
			// checkAndAdd(tmp);
			add(tmp);
		}
	}
#endif

	char *CalCurveTable::toFITS() const throw(ConversionException) {
		throw ConversionException("Not implemented","CalCurve");
	}

	void CalCurveTable::fromFITS(char *fits) throw(ConversionException) {
		throw ConversionException("Not implemented","CalCurve");
	}

	string CalCurveTable::toVOTable() const throw(ConversionException) {
		throw ConversionException("Not implemented","CalCurve");
	}

	void CalCurveTable::fromVOTable(string vo) throw(ConversionException) {
		throw ConversionException("Not implemented","CalCurve");
	}

	string CalCurveTable::toXML()  throw(ConversionException) {
		string buf;
		buf.append("<?xml version=\"1.0\" encoding=\"ISO-8859-1\"?> ");
//		buf.append("<CalCurveTable xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" xsi:noNamespaceSchemaLocation=\"../../idl/CalCurveTable.xsd\"> ");
		buf.append("<?xml-stylesheet type=\"text/xsl\" href=\"../asdm2html/table2html.xsl\"?> ");		
		buf.append("<CalCurveTable> ");
		buf.append(entity.toXML());
		string s = container.getEntity().toXML();
		// Change the "Entity" tag to "ContainerEntity".
		buf.append("<Container" + s.substr(1,s.length() - 1)+" ");
		vector<CalCurveRow*> v = get();
		for (unsigned int i = 0; i < v.size(); ++i) {
			try {
				buf.append(v[i]->toXML());
			} catch (NoSuchRow e) {
			}
			buf.append("  ");
		}		
		buf.append("</CalCurveTable> ");
		return buf;
	}
	
	void CalCurveTable::fromXML(string xmlDoc) throw(ConversionException) {
		Parser xml(xmlDoc);
		if (!xml.isStr("<CalCurveTable")) 
			error();
		// cout << "Parsing a CalCurveTable" << endl;
		string s = xml.getElement("<Entity","/>");
		if (s.length() == 0) 
			error();
		Entity e;
		e.setFromXML(s);
		if (e.getEntityTypeName() != "CalCurveTable")
			error();
		setEntity(e);
		// Skip the container's entity; but, it has to be there.
		s = xml.getElement("<ContainerEntity","/>");
		if (s.length() == 0) 
			error();

		// Get each row in the table.
		s = xml.getElementContent("<row>","</row>");
		CalCurveRow *row;
		while (s.length() != 0) {
			// cout << "Parsing a CalCurveRow" << endl; 
			row = newRow();
			row->setFromXML(s);
			try {
				checkAndAdd(row);
			} catch (DuplicateKey e1) {
				throw ConversionException(e1.getMessage(),"CalCurveTable");
			} 
			catch (UniquenessViolationException e1) {
				throw ConversionException(e1.getMessage(),"CalCurveTable");	
			}
			catch (...) {
				// cout << "Unexpected error in CalCurveTable::checkAndAdd called from CalCurveTable::fromXML " << endl;
			}
			s = xml.getElementContent("<row>","</row>");
		}
		if (!xml.isStr("</CalCurveTable>")) 
			error();
	}

	void CalCurveTable::error() throw(ConversionException) {
		throw ConversionException("Invalid xml document","CalCurve");
	}
	
	string CalCurveTable::toMIME() {
	 // To be implemented
		return "";
	}
	
	void CalCurveTable::setFromMIME(const string & mimeMsg) {
		// To be implemented
		;
	}
	
	
	void CalCurveTable::toFile(string directory) {
		if (!directoryExists(directory.c_str()) &&
			!createPath(directory.c_str())) {
			throw ConversionException("Could not create directory " , directory);
		}
		
		if (fileAsBin) {
			// write the bin serialized
			string fileName = directory + "/CalCurve.bin";
			ofstream tableout(fileName.c_str(),ios::out|ios::trunc);
			if (tableout.rdstate() == ostream::failbit)
				throw ConversionException("Could not open file " + fileName + " to write ", "CalCurve");
			tableout << toMIME() << endl;
			tableout.close();
			if (tableout.rdstate() == ostream::failbit)
				throw ConversionException("Could not close file " + fileName, "CalCurve");
		}
		else {
			// write the XML
			string fileName = directory + "/CalCurve.xml";
			ofstream tableout(fileName.c_str(),ios::out|ios::trunc);
			if (tableout.rdstate() == ostream::failbit)
				throw ConversionException("Could not open file " + fileName + " to write ", "CalCurve");
			tableout << toXML() << endl;
			tableout.close();
			if (tableout.rdstate() == ostream::failbit)
				throw ConversionException("Could not close file " + fileName, "CalCurve");
		}
	}
	
	void CalCurveTable::setFromFile(const string& directory) {
		string tablename;
		if (fileAsBin)
			tablename = directory + "/CalCurve.bin";
		else
			tablename = directory + "/CalCurve.xml";
			
		// Determine the file size.
		ifstream::pos_type size;
		ifstream tablefile(tablename.c_str(), ios::in|ios::binary|ios::ate);

 		if (tablefile.is_open()) { 
  				size = tablefile.tellg(); 
  		}
		else {
				throw ConversionException("Could not open file " + tablename, "CalCurve");
		}
		
		// Re position to the beginning.
		tablefile.seekg(0);
		
		// Read in a stringstream.
		stringstream ss;
		ss << tablefile.rdbuf();

		if (tablefile.rdstate() == istream::failbit || tablefile.rdstate() == istream::badbit) {
			throw ConversionException("Error reading file " + tablename,"CalCurve");
		}

		// And close
		tablefile.close();
		if (tablefile.rdstate() == istream::failbit)
			throw ConversionException("Could not close file " + tablename,"CalCurve");
					
		// And parse the content with the appropriate method
		if (fileAsBin) 
			setFromMIME(ss.str());
		else
			fromXML(ss.str());	
	}			
			
	
	

	
} // End namespace asdm
 
