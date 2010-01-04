
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
 * File CalPositionTable.cpp
 */
#include <ConversionException.h>
#include <DuplicateKey.h>
#include <OutOfBoundsException.h>

using asdm::ConversionException;
using asdm::DuplicateKey;
using asdm::OutOfBoundsException;

#include <ASDM.h>
#include <CalPositionTable.h>
#include <CalPositionRow.h>
#include <Parser.h>

using asdm::ASDM;
using asdm::CalPositionTable;
using asdm::CalPositionRow;
using asdm::Parser;

#include <iostream>
#include <sstream>
#include <set>
using namespace std;

#include <Misc.h>
using namespace asdm;


namespace asdm {

	string CalPositionTable::tableName = "CalPosition";
	

	/**
	 * The list of field names that make up key key.
	 * (Initialization is in the constructor.)
	 */
	vector<string> CalPositionTable::key;

	/**
	 * Return the list of field names that make up key key
	 * as an array of strings.
	 */	
	vector<string> CalPositionTable::getKeyName() {
		return key;
	}


	CalPositionTable::CalPositionTable(ASDM &c) : container(c) {

	
		key.push_back("antennaName");
	
		key.push_back("atmPhaseCorrection");
	
		key.push_back("calDataId");
	
		key.push_back("calReductionId");
	


		// Define a default entity.
		entity.setEntityId(EntityId("uid://X0/X0/X0"));
		entity.setEntityIdEncrypted("na");
		entity.setEntityTypeName("CalPositionTable");
		entity.setEntityVersion("1");
		entity.setInstanceVersion("1");
		
		// Archive XML
		archiveAsBin = false;
		
		// File XML
		fileAsBin = false;
	}
	
/**
 * A destructor for CalPositionTable.
 */
 
	CalPositionTable::~CalPositionTable() {
		for (unsigned int i = 0; i < privateRows.size(); i++) 
			delete(privateRows.at(i));
	}

	/**
	 * Container to which this table belongs.
	 */
	ASDM &CalPositionTable::getContainer() const {
		return container;
	}

	/**
	 * Return the number of rows in the table.
	 */

	unsigned int CalPositionTable::size() {
		return row.size();
	}	
	
	
	/**
	 * Return the name of this table.
	 */
	string CalPositionTable::getName() const {
		return tableName;
	}

	/**
	 * Return this table's Entity.
	 */
	Entity CalPositionTable::getEntity() const {
		return entity;
	}

	/**
	 * Set this table's Entity.
	 */
	void CalPositionTable::setEntity(Entity e) {
		this->entity = e; 
	}
	
	//
	// ====> Row creation.
	//
	
	/**
	 * Create a new row.
	 */
	CalPositionRow *CalPositionTable::newRow() {
		return new CalPositionRow (*this);
	}
	
	CalPositionRow *CalPositionTable::newRowEmpty() {
		return newRow ();
	}


	/**
	 * Create a new row initialized to the specified values.
	 * @return a pointer on the created and initialized row.
	
 	 * @param antennaName. 
	
 	 * @param atmPhaseCorrection. 
	
 	 * @param calDataId. 
	
 	 * @param calReductionId. 
	
 	 * @param startValidTime. 
	
 	 * @param endValidTime. 
	
 	 * @param antennaPosition. 
	
 	 * @param stationName. 
	
 	 * @param stationPosition. 
	
 	 * @param positionMethod. 
	
 	 * @param receiverBand. 
	
 	 * @param numAntenna. 
	
 	 * @param refAntennaNames. 
	
 	 * @param axesOffset. 
	
 	 * @param axesOffsetErr. 
	
 	 * @param axesOffsetFixed. 
	
 	 * @param positionOffset. 
	
 	 * @param positionErr. 
	
 	 * @param reducedChiSquared. 
	
     */
	CalPositionRow* CalPositionTable::newRow(string antennaName, AtmPhaseCorrectionMod::AtmPhaseCorrection atmPhaseCorrection, Tag calDataId, Tag calReductionId, ArrayTime startValidTime, ArrayTime endValidTime, vector<Length > antennaPosition, string stationName, vector<Length > stationPosition, PositionMethodMod::PositionMethod positionMethod, ReceiverBandMod::ReceiverBand receiverBand, int numAntenna, vector<string > refAntennaNames, Length axesOffset, Length axesOffsetErr, bool axesOffsetFixed, vector<Length > positionOffset, vector<Length > positionErr, double reducedChiSquared){
		CalPositionRow *row = new CalPositionRow(*this);
			
		row->setAntennaName(antennaName);
			
		row->setAtmPhaseCorrection(atmPhaseCorrection);
			
		row->setCalDataId(calDataId);
			
		row->setCalReductionId(calReductionId);
			
		row->setStartValidTime(startValidTime);
			
		row->setEndValidTime(endValidTime);
			
		row->setAntennaPosition(antennaPosition);
			
		row->setStationName(stationName);
			
		row->setStationPosition(stationPosition);
			
		row->setPositionMethod(positionMethod);
			
		row->setReceiverBand(receiverBand);
			
		row->setNumAntenna(numAntenna);
			
		row->setRefAntennaNames(refAntennaNames);
			
		row->setAxesOffset(axesOffset);
			
		row->setAxesOffsetErr(axesOffsetErr);
			
		row->setAxesOffsetFixed(axesOffsetFixed);
			
		row->setPositionOffset(positionOffset);
			
		row->setPositionErr(positionErr);
			
		row->setReducedChiSquared(reducedChiSquared);
	
		return row;		
	}	

	CalPositionRow* CalPositionTable::newRowFull(string antennaName, AtmPhaseCorrectionMod::AtmPhaseCorrection atmPhaseCorrection, Tag calDataId, Tag calReductionId, ArrayTime startValidTime, ArrayTime endValidTime, vector<Length > antennaPosition, string stationName, vector<Length > stationPosition, PositionMethodMod::PositionMethod positionMethod, ReceiverBandMod::ReceiverBand receiverBand, int numAntenna, vector<string > refAntennaNames, Length axesOffset, Length axesOffsetErr, bool axesOffsetFixed, vector<Length > positionOffset, vector<Length > positionErr, double reducedChiSquared)	{
		CalPositionRow *row = new CalPositionRow(*this);
			
		row->setAntennaName(antennaName);
			
		row->setAtmPhaseCorrection(atmPhaseCorrection);
			
		row->setCalDataId(calDataId);
			
		row->setCalReductionId(calReductionId);
			
		row->setStartValidTime(startValidTime);
			
		row->setEndValidTime(endValidTime);
			
		row->setAntennaPosition(antennaPosition);
			
		row->setStationName(stationName);
			
		row->setStationPosition(stationPosition);
			
		row->setPositionMethod(positionMethod);
			
		row->setReceiverBand(receiverBand);
			
		row->setNumAntenna(numAntenna);
			
		row->setRefAntennaNames(refAntennaNames);
			
		row->setAxesOffset(axesOffset);
			
		row->setAxesOffsetErr(axesOffsetErr);
			
		row->setAxesOffsetFixed(axesOffsetFixed);
			
		row->setPositionOffset(positionOffset);
			
		row->setPositionErr(positionErr);
			
		row->setReducedChiSquared(reducedChiSquared);
	
		return row;				
	}
	


CalPositionRow* CalPositionTable::newRow(CalPositionRow* row) {
	return new CalPositionRow(*this, *row);
}

CalPositionRow* CalPositionTable::newRowCopy(CalPositionRow* row) {
	return new CalPositionRow(*this, *row);
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
	CalPositionRow* CalPositionTable::add(CalPositionRow* x) {
		
		if (getRowByKey(
						x->getAntennaName()
						,
						x->getAtmPhaseCorrection()
						,
						x->getCalDataId()
						,
						x->getCalReductionId()
						))
			//throw DuplicateKey(x.getAntennaName() + "|" + x.getAtmPhaseCorrection() + "|" + x.getCalDataId() + "|" + x.getCalReductionId(),"CalPosition");
			throw DuplicateKey("Duplicate key exception in ","CalPositionTable");
		
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
	CalPositionRow*  CalPositionTable::checkAndAdd(CalPositionRow* x)  {
		
		
		if (getRowByKey(
	
			x->getAntennaName()
	,
			x->getAtmPhaseCorrection()
	,
			x->getCalDataId()
	,
			x->getCalReductionId()
			
		)) throw DuplicateKey("Duplicate key exception in ", "CalPositionTable");
		
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
	 * @return Alls rows as an array of CalPositionRow
	 */
	vector<CalPositionRow *> CalPositionTable::get() {
		return privateRows;
		// return row;
	}

	
/*
 ** Returns a CalPositionRow* given a key.
 ** @return a pointer to the row having the key whose values are passed as parameters, or 0 if
 ** no row exists for that key.
 **
 */
 	CalPositionRow* CalPositionTable::getRowByKey(string antennaName, AtmPhaseCorrectionMod::AtmPhaseCorrection atmPhaseCorrection, Tag calDataId, Tag calReductionId)  {
	CalPositionRow* aRow = 0;
	for (unsigned int i = 0; i < row.size(); i++) {
		aRow = row.at(i);
		
			
				if (aRow->antennaName != antennaName) continue;
			
		
			
				if (aRow->atmPhaseCorrection != atmPhaseCorrection) continue;
			
		
			
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
 	 		
 * @param atmPhaseCorrection.
 	 		
 * @param calDataId.
 	 		
 * @param calReductionId.
 	 		
 * @param startValidTime.
 	 		
 * @param endValidTime.
 	 		
 * @param antennaPosition.
 	 		
 * @param stationName.
 	 		
 * @param stationPosition.
 	 		
 * @param positionMethod.
 	 		
 * @param receiverBand.
 	 		
 * @param numAntenna.
 	 		
 * @param refAntennaNames.
 	 		
 * @param axesOffset.
 	 		
 * @param axesOffsetErr.
 	 		
 * @param axesOffsetFixed.
 	 		
 * @param positionOffset.
 	 		
 * @param positionErr.
 	 		
 * @param reducedChiSquared.
 	 		 
 */
CalPositionRow* CalPositionTable::lookup(string antennaName, AtmPhaseCorrectionMod::AtmPhaseCorrection atmPhaseCorrection, Tag calDataId, Tag calReductionId, ArrayTime startValidTime, ArrayTime endValidTime, vector<Length > antennaPosition, string stationName, vector<Length > stationPosition, PositionMethodMod::PositionMethod positionMethod, ReceiverBandMod::ReceiverBand receiverBand, int numAntenna, vector<string > refAntennaNames, Length axesOffset, Length axesOffsetErr, bool axesOffsetFixed, vector<Length > positionOffset, vector<Length > positionErr, double reducedChiSquared) {
		CalPositionRow* aRow;
		for (unsigned int i = 0; i < size(); i++) {
			aRow = row.at(i); 
			if (aRow->compareNoAutoInc(antennaName, atmPhaseCorrection, calDataId, calReductionId, startValidTime, endValidTime, antennaPosition, stationName, stationPosition, positionMethod, receiverBand, numAntenna, refAntennaNames, axesOffset, axesOffsetErr, axesOffsetFixed, positionOffset, positionErr, reducedChiSquared)) return aRow;
		}			
		return 0;	
} 
	
 	 	

	




#ifndef WITHOUT_ACS
	// Conversion Methods

	CalPositionTableIDL *CalPositionTable::toIDL() {
		CalPositionTableIDL *x = new CalPositionTableIDL ();
		unsigned int nrow = size();
		x->row.length(nrow);
		vector<CalPositionRow*> v = get();
		for (unsigned int i = 0; i < nrow; ++i) {
			x->row[i] = *(v[i]->toIDL());
		}
		return x;
	}
#endif
	
#ifndef WITHOUT_ACS
	void CalPositionTable::fromIDL(CalPositionTableIDL x) {
		unsigned int nrow = x.row.length();
		for (unsigned int i = 0; i < nrow; ++i) {
			CalPositionRow *tmp = newRow();
			tmp->setFromIDL(x.row[i]);
			// checkAndAdd(tmp);
			add(tmp);
		}
	}
#endif

	char *CalPositionTable::toFITS() const  {
		throw ConversionException("Not implemented","CalPosition");
	}

	void CalPositionTable::fromFITS(char *fits)  {
		throw ConversionException("Not implemented","CalPosition");
	}

	string CalPositionTable::toVOTable() const {
		throw ConversionException("Not implemented","CalPosition");
	}

	void CalPositionTable::fromVOTable(string vo) {
		throw ConversionException("Not implemented","CalPosition");
	}

	
	string CalPositionTable::toXML()  {
		string buf;
		buf.append("<?xml version=\"1.0\" encoding=\"ISO-8859-1\"?> ");
		buf.append("<CalPositionTable xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" xmlns=\"http://Alma/XASDM/CalPositionTable\" xsi:schemaLocation=\"http://Alma/XASDM/CalPositionTable http://almaobservatory.org/XML/XASDM/2/CalPositionTable.xsd\"> ");	
		buf.append(entity.toXML());
		string s = container.getEntity().toXML();
		// Change the "Entity" tag to "ContainerEntity".
		buf.append("<Container" + s.substr(1,s.length() - 1)+" ");
		vector<CalPositionRow*> v = get();
		for (unsigned int i = 0; i < v.size(); ++i) {
			try {
				buf.append(v[i]->toXML());
			} catch (NoSuchRow e) {
			}
			buf.append("  ");
		}		
		buf.append("</CalPositionTable> ");
		return buf;
	}

	
	void CalPositionTable::fromXML(string xmlDoc)  {
		Parser xml(xmlDoc);
		if (!xml.isStr("<CalPositionTable")) 
			error();
		// cout << "Parsing a CalPositionTable" << endl;
		string s = xml.getElement("<Entity","/>");
		if (s.length() == 0) 
			error();
		Entity e;
		e.setFromXML(s);
		if (e.getEntityTypeName() != "CalPositionTable")
			error();
		setEntity(e);
		// Skip the container's entity; but, it has to be there.
		s = xml.getElement("<ContainerEntity","/>");
		if (s.length() == 0) 
			error();

		// Get each row in the table.
		s = xml.getElementContent("<row>","</row>");
		CalPositionRow *row;
		while (s.length() != 0) {
			// cout << "Parsing a CalPositionRow" << endl; 
			row = newRow();
			row->setFromXML(s);
			try {
				checkAndAdd(row);
			} catch (DuplicateKey e1) {
				throw ConversionException(e1.getMessage(),"CalPositionTable");
			} 
			catch (UniquenessViolationException e1) {
				throw ConversionException(e1.getMessage(),"CalPositionTable");	
			}
			catch (...) {
				// cout << "Unexpected error in CalPositionTable::checkAndAdd called from CalPositionTable::fromXML " << endl;
			}
			s = xml.getElementContent("<row>","</row>");
		}
		if (!xml.isStr("</CalPositionTable>")) 
			error();
	}

	
	void CalPositionTable::error()  {
		throw ConversionException("Invalid xml document","CalPosition");
	}
	
	
	string CalPositionTable::toMIME() {
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

	
	void CalPositionTable::setFromMIME(const string & mimeMsg) {
		// cout << "Entering setFromMIME" << endl;
	 	string terminator = "Content-Type: binary/octet-stream\nContent-ID: <content.bin>\n\n";
	 	
	 	// Look for the string announcing the binary part.
	 	string::size_type loc = mimeMsg.find( terminator, 0 );
	 	
	 	if ( loc == string::npos ) {
	 		throw ConversionException("Failed to detect the beginning of the binary part", "CalPosition");
	 	}
	
	 	// Create an EndianISStream from the substring containing the binary part.
	 	EndianISStream eiss(mimeMsg.substr(loc+terminator.size()));
	 	
	 	entity = Entity::fromBin(eiss);
	 	
	 	// We do nothing with that but we have to read it.
	 	Entity containerEntity = Entity::fromBin(eiss);
	 		 	
	 	int numRows = eiss.readInt();
	 	try {
	 		for (int i = 0; i < numRows; i++) {
	 			CalPositionRow* aRow = CalPositionRow::fromBin(eiss, *this);
	 			checkAndAdd(aRow);
	 		}
	 	}
	 	catch (DuplicateKey e) {
	 		throw ConversionException("Error while writing binary data , the message was "
	 					+ e.getMessage(), "CalPosition");
	 	}
		catch (TagFormatException e) {
			throw ConversionException("Error while reading binary data , the message was "
					+ e.getMessage(), "CalPosition");
		} 		 	
	}

	
	void CalPositionTable::toFile(string directory) {
		if (!directoryExists(directory.c_str()) &&
			!createPath(directory.c_str())) {
			throw ConversionException("Could not create directory " , directory);
		}
		
		if (fileAsBin) {
			// write the bin serialized
			string fileName = directory + "/CalPosition.bin";
			ofstream tableout(fileName.c_str(),ios::out|ios::trunc);
			if (tableout.rdstate() == ostream::failbit)
				throw ConversionException("Could not open file " + fileName + " to write ", "CalPosition");
			tableout << toMIME() << endl;
			tableout.close();
			if (tableout.rdstate() == ostream::failbit)
				throw ConversionException("Could not close file " + fileName, "CalPosition");
		}
		else {
			// write the XML
			string fileName = directory + "/CalPosition.xml";
			ofstream tableout(fileName.c_str(),ios::out|ios::trunc);
			if (tableout.rdstate() == ostream::failbit)
				throw ConversionException("Could not open file " + fileName + " to write ", "CalPosition");
			tableout << toXML() << endl;
			tableout.close();
			if (tableout.rdstate() == ostream::failbit)
				throw ConversionException("Could not close file " + fileName, "CalPosition");
		}
	}

	
	void CalPositionTable::setFromFile(const string& directory) {
		string tablename;
		if (fileAsBin)
			tablename = directory + "/CalPosition.bin";
		else
			tablename = directory + "/CalPosition.xml";
			
		// Determine the file size.
		ifstream::pos_type size;
		ifstream tablefile(tablename.c_str(), ios::in|ios::binary|ios::ate);

 		if (tablefile.is_open()) { 
  				size = tablefile.tellg(); 
  		}
		else {
				throw ConversionException("Could not open file " + tablename, "CalPosition");
		}
		
		// Re position to the beginning.
		tablefile.seekg(0);
		
		// Read in a stringstream.
		stringstream ss;
		ss << tablefile.rdbuf();

		if (tablefile.rdstate() == istream::failbit || tablefile.rdstate() == istream::badbit) {
			throw ConversionException("Error reading file " + tablename,"CalPosition");
		}

		// And close
		tablefile.close();
		if (tablefile.rdstate() == istream::failbit)
			throw ConversionException("Could not close file " + tablename,"CalPosition");
					
		// And parse the content with the appropriate method
		if (fileAsBin) 
			setFromMIME(ss.str());
		else
			fromXML(ss.str());	
	}			

	

	

			
	
	

	
} // End namespace asdm
 
