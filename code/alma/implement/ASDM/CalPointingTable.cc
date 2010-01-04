
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

	
		key.push_back("antennaName");
	
		key.push_back("receiverBand");
	
		key.push_back("calDataId");
	
		key.push_back("calReductionId");
	


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
	
 	 * @param antennaName. 
	
 	 * @param receiverBand. 
	
 	 * @param calDataId. 
	
 	 * @param calReductionId. 
	
 	 * @param startValidTime. 
	
 	 * @param endValidTime. 
	
 	 * @param ambientTemperature. 
	
 	 * @param antennaMake. 
	
 	 * @param atmPhaseCorrection. 
	
 	 * @param direction. 
	
 	 * @param frequencyRange. 
	
 	 * @param pointingModelMode. 
	
 	 * @param pointingMethod. 
	
 	 * @param numReceptor. 
	
 	 * @param polarizationTypes. 
	
 	 * @param collOffsetRelative. 
	
 	 * @param collOffsetAbsolute. 
	
 	 * @param collError. 
	
 	 * @param collOffsetTied. 
	
 	 * @param reducedChiSquared. 
	
     */
	CalPointingRow* CalPointingTable::newRow(string antennaName, ReceiverBandMod::ReceiverBand receiverBand, Tag calDataId, Tag calReductionId, ArrayTime startValidTime, ArrayTime endValidTime, Temperature ambientTemperature, AntennaMakeMod::AntennaMake antennaMake, AtmPhaseCorrectionMod::AtmPhaseCorrection atmPhaseCorrection, vector<Angle > direction, vector<Frequency > frequencyRange, PointingModelModeMod::PointingModelMode pointingModelMode, PointingMethodMod::PointingMethod pointingMethod, int numReceptor, vector<PolarizationTypeMod::PolarizationType > polarizationTypes, vector<vector<Angle > > collOffsetRelative, vector<vector<Angle > > collOffsetAbsolute, vector<vector<Angle > > collError, vector<vector<bool > > collOffsetTied, vector<double > reducedChiSquared){
		CalPointingRow *row = new CalPointingRow(*this);
			
		row->setAntennaName(antennaName);
			
		row->setReceiverBand(receiverBand);
			
		row->setCalDataId(calDataId);
			
		row->setCalReductionId(calReductionId);
			
		row->setStartValidTime(startValidTime);
			
		row->setEndValidTime(endValidTime);
			
		row->setAmbientTemperature(ambientTemperature);
			
		row->setAntennaMake(antennaMake);
			
		row->setAtmPhaseCorrection(atmPhaseCorrection);
			
		row->setDirection(direction);
			
		row->setFrequencyRange(frequencyRange);
			
		row->setPointingModelMode(pointingModelMode);
			
		row->setPointingMethod(pointingMethod);
			
		row->setNumReceptor(numReceptor);
			
		row->setPolarizationTypes(polarizationTypes);
			
		row->setCollOffsetRelative(collOffsetRelative);
			
		row->setCollOffsetAbsolute(collOffsetAbsolute);
			
		row->setCollError(collError);
			
		row->setCollOffsetTied(collOffsetTied);
			
		row->setReducedChiSquared(reducedChiSquared);
	
		return row;		
	}	

	CalPointingRow* CalPointingTable::newRowFull(string antennaName, ReceiverBandMod::ReceiverBand receiverBand, Tag calDataId, Tag calReductionId, ArrayTime startValidTime, ArrayTime endValidTime, Temperature ambientTemperature, AntennaMakeMod::AntennaMake antennaMake, AtmPhaseCorrectionMod::AtmPhaseCorrection atmPhaseCorrection, vector<Angle > direction, vector<Frequency > frequencyRange, PointingModelModeMod::PointingModelMode pointingModelMode, PointingMethodMod::PointingMethod pointingMethod, int numReceptor, vector<PolarizationTypeMod::PolarizationType > polarizationTypes, vector<vector<Angle > > collOffsetRelative, vector<vector<Angle > > collOffsetAbsolute, vector<vector<Angle > > collError, vector<vector<bool > > collOffsetTied, vector<double > reducedChiSquared)	{
		CalPointingRow *row = new CalPointingRow(*this);
			
		row->setAntennaName(antennaName);
			
		row->setReceiverBand(receiverBand);
			
		row->setCalDataId(calDataId);
			
		row->setCalReductionId(calReductionId);
			
		row->setStartValidTime(startValidTime);
			
		row->setEndValidTime(endValidTime);
			
		row->setAmbientTemperature(ambientTemperature);
			
		row->setAntennaMake(antennaMake);
			
		row->setAtmPhaseCorrection(atmPhaseCorrection);
			
		row->setDirection(direction);
			
		row->setFrequencyRange(frequencyRange);
			
		row->setPointingModelMode(pointingModelMode);
			
		row->setPointingMethod(pointingMethod);
			
		row->setNumReceptor(numReceptor);
			
		row->setPolarizationTypes(polarizationTypes);
			
		row->setCollOffsetRelative(collOffsetRelative);
			
		row->setCollOffsetAbsolute(collOffsetAbsolute);
			
		row->setCollError(collError);
			
		row->setCollOffsetTied(collOffsetTied);
			
		row->setReducedChiSquared(reducedChiSquared);
	
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
						x->getAntennaName()
						,
						x->getReceiverBand()
						,
						x->getCalDataId()
						,
						x->getCalReductionId()
						))
			//throw DuplicateKey(x.getAntennaName() + "|" + x.getReceiverBand() + "|" + x.getCalDataId() + "|" + x.getCalReductionId(),"CalPointing");
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
	 * @throws DuplicateKey
	 
	 */
	CalPointingRow*  CalPointingTable::checkAndAdd(CalPointingRow* x)  {
		
		
		if (getRowByKey(
	
			x->getAntennaName()
	,
			x->getReceiverBand()
	,
			x->getCalDataId()
	,
			x->getCalReductionId()
			
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
 	CalPointingRow* CalPointingTable::getRowByKey(string antennaName, ReceiverBandMod::ReceiverBand receiverBand, Tag calDataId, Tag calReductionId)  {
	CalPointingRow* aRow = 0;
	for (unsigned int i = 0; i < row.size(); i++) {
		aRow = row.at(i);
		
			
				if (aRow->antennaName != antennaName) continue;
			
		
			
				if (aRow->receiverBand != receiverBand) continue;
			
		
			
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
			
 * @param antennaName.
 	 		
 * @param receiverBand.
 	 		
 * @param calDataId.
 	 		
 * @param calReductionId.
 	 		
 * @param startValidTime.
 	 		
 * @param endValidTime.
 	 		
 * @param ambientTemperature.
 	 		
 * @param antennaMake.
 	 		
 * @param atmPhaseCorrection.
 	 		
 * @param direction.
 	 		
 * @param frequencyRange.
 	 		
 * @param pointingModelMode.
 	 		
 * @param pointingMethod.
 	 		
 * @param numReceptor.
 	 		
 * @param polarizationTypes.
 	 		
 * @param collOffsetRelative.
 	 		
 * @param collOffsetAbsolute.
 	 		
 * @param collError.
 	 		
 * @param collOffsetTied.
 	 		
 * @param reducedChiSquared.
 	 		 
 */
CalPointingRow* CalPointingTable::lookup(string antennaName, ReceiverBandMod::ReceiverBand receiverBand, Tag calDataId, Tag calReductionId, ArrayTime startValidTime, ArrayTime endValidTime, Temperature ambientTemperature, AntennaMakeMod::AntennaMake antennaMake, AtmPhaseCorrectionMod::AtmPhaseCorrection atmPhaseCorrection, vector<Angle > direction, vector<Frequency > frequencyRange, PointingModelModeMod::PointingModelMode pointingModelMode, PointingMethodMod::PointingMethod pointingMethod, int numReceptor, vector<PolarizationTypeMod::PolarizationType > polarizationTypes, vector<vector<Angle > > collOffsetRelative, vector<vector<Angle > > collOffsetAbsolute, vector<vector<Angle > > collError, vector<vector<bool > > collOffsetTied, vector<double > reducedChiSquared) {
		CalPointingRow* aRow;
		for (unsigned int i = 0; i < size(); i++) {
			aRow = row.at(i); 
			if (aRow->compareNoAutoInc(antennaName, receiverBand, calDataId, calReductionId, startValidTime, endValidTime, ambientTemperature, antennaMake, atmPhaseCorrection, direction, frequencyRange, pointingModelMode, pointingMethod, numReceptor, polarizationTypes, collOffsetRelative, collOffsetAbsolute, collError, collOffsetTied, reducedChiSquared)) return aRow;
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
	void CalPointingTable::fromIDL(CalPointingTableIDL x) {
		unsigned int nrow = x.row.length();
		for (unsigned int i = 0; i < nrow; ++i) {
			CalPointingRow *tmp = newRow();
			tmp->setFromIDL(x.row[i]);
			// checkAndAdd(tmp);
			add(tmp);
		}
	}
#endif

	char *CalPointingTable::toFITS() const  {
		throw ConversionException("Not implemented","CalPointing");
	}

	void CalPointingTable::fromFITS(char *fits)  {
		throw ConversionException("Not implemented","CalPointing");
	}

	string CalPointingTable::toVOTable() const {
		throw ConversionException("Not implemented","CalPointing");
	}

	void CalPointingTable::fromVOTable(string vo) {
		throw ConversionException("Not implemented","CalPointing");
	}

	
	string CalPointingTable::toXML()  {
		string buf;
		buf.append("<?xml version=\"1.0\" encoding=\"ISO-8859-1\"?> ");
		buf.append("<CalPointingTable xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" xmlns=\"http://Alma/XASDM/CalPointingTable\" xsi:schemaLocation=\"http://Alma/XASDM/CalPointingTable http://almaobservatory.org/XML/XASDM/2/CalPointingTable.xsd\"> ");	
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

	
	void CalPointingTable::fromXML(string xmlDoc)  {
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

	
	void CalPointingTable::error()  {
		throw ConversionException("Invalid xml document","CalPointing");
	}
	
	
	string CalPointingTable::toMIME() {
		EndianOSStream eoss;
		
		string UID = getEntity().getEntityId().toString();
		string execBlockUID = getContainer().getEntity().getEntityId().toString();
		
		// The MIME Header
		eoss <<"MIME-Version: 1.0";
		eoss << "\n";
		eoss << "Content-Type: Multipart/Related; boundary='MIME_boundary'; type='text/xml'; start= '<header.xml>'";
		eoss <<"\n";
		eoss <<"Content-Description: Correlator";
		eoss <<"\n";
		eoss <<"alma-uid:" << UID;
		eoss <<"\n";
		eoss <<"\n";		
		
		// The MIME XML part header.
		eoss <<"--MIME_boundary";
		eoss <<"\n";
		eoss <<"Content-Type: text/xml; charset='ISO-8859-1'";
		eoss <<"\n";
		eoss <<"Content-Transfer-Encoding: 8bit";
		eoss <<"\n";
		eoss <<"Content-ID: <header.xml>";
		eoss <<"\n";
		eoss <<"\n";
		
		// The MIME XML part content.
		eoss << "<?xml version='1.0'  encoding='ISO-8859-1'?>";
		eoss << "\n";
		eoss<< "<ASDMBinaryTable  xmlns:xsi='http://www.w3.org/2001/XMLSchema-instance'  xsi:noNamespaceSchemaLocation='ASDMBinaryTable.xsd' ID='None'  version='1.0'>\n";
		eoss << "<ExecBlockUID>\n";
		eoss << execBlockUID  << "\n";
		eoss << "</ExecBlockUID>\n";
		eoss << "</ASDMBinaryTable>\n";		

		// The MIME binary part header
		eoss <<"--MIME_boundary";
		eoss <<"\n";
		eoss <<"Content-Type: binary/octet-stream";
		eoss <<"\n";
		eoss <<"Content-ID: <content.bin>";
		eoss <<"\n";
		eoss <<"\n";	
		
		// The MIME binary content
		entity.toBin(eoss);
		container.getEntity().toBin(eoss);
		eoss.writeInt((int) privateRows.size());
		for (unsigned int i = 0; i < privateRows.size(); i++) {
			privateRows.at(i)->toBin(eoss);	
		}
		
		// The closing MIME boundary
		eoss << "\n--MIME_boundary--";
		eoss << "\n";
		
		return eoss.str();	
	}

	
	void CalPointingTable::setFromMIME(const string & mimeMsg) {
		// cout << "Entering setFromMIME" << endl;
	 	string terminator = "Content-Type: binary/octet-stream\nContent-ID: <content.bin>\n\n";
	 	
	 	// Look for the string announcing the binary part.
	 	string::size_type loc = mimeMsg.find( terminator, 0 );
	 	
	 	if ( loc == string::npos ) {
	 		throw ConversionException("Failed to detect the beginning of the binary part", "CalPointing");
	 	}
	
	 	// Create an EndianISStream from the substring containing the binary part.
	 	EndianISStream eiss(mimeMsg.substr(loc+terminator.size()));
	 	
	 	entity = Entity::fromBin(eiss);
	 	
	 	// We do nothing with that but we have to read it.
	 	Entity containerEntity = Entity::fromBin(eiss);
	 		 	
	 	int numRows = eiss.readInt();
	 	try {
	 		for (int i = 0; i < numRows; i++) {
	 			CalPointingRow* aRow = CalPointingRow::fromBin(eiss, *this);
	 			checkAndAdd(aRow);
	 		}
	 	}
	 	catch (DuplicateKey e) {
	 		throw ConversionException("Error while writing binary data , the message was "
	 					+ e.getMessage(), "CalPointing");
	 	}
		catch (TagFormatException e) {
			throw ConversionException("Error while reading binary data , the message was "
					+ e.getMessage(), "CalPointing");
		} 		 	
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
 
