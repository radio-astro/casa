
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
 * File CalDelayTable.cpp
 */
#include <ConversionException.h>
#include <DuplicateKey.h>
#include <OutOfBoundsException.h>

using asdm::ConversionException;
using asdm::DuplicateKey;
using asdm::OutOfBoundsException;

#include <ASDM.h>
#include <CalDelayTable.h>
#include <CalDelayRow.h>
#include <Parser.h>

using asdm::ASDM;
using asdm::CalDelayTable;
using asdm::CalDelayRow;
using asdm::Parser;

#include <iostream>
#include <sstream>
#include <set>
using namespace std;

#include <Misc.h>
using namespace asdm;


namespace asdm {

	string CalDelayTable::tableName = "CalDelay";
	

	/**
	 * The list of field names that make up key key.
	 * (Initialization is in the constructor.)
	 */
	vector<string> CalDelayTable::key;

	/**
	 * Return the list of field names that make up key key
	 * as an array of strings.
	 */	
	vector<string> CalDelayTable::getKeyName() {
		return key;
	}


	CalDelayTable::CalDelayTable(ASDM &c) : container(c) {

	
		key.push_back("antennaName");
	
		key.push_back("atmPhaseCorrection");
	
		key.push_back("basebandName");
	
		key.push_back("receiverBand");
	
		key.push_back("calDataId");
	
		key.push_back("calReductionId");
	


		// Define a default entity.
		entity.setEntityId(EntityId("uid://X0/X0/X0"));
		entity.setEntityIdEncrypted("na");
		entity.setEntityTypeName("CalDelayTable");
		entity.setEntityVersion("1");
		entity.setInstanceVersion("1");
		
		// Archive XML
		archiveAsBin = false;
		
		// File XML
		fileAsBin = false;
	}
	
/**
 * A destructor for CalDelayTable.
 */
 
	CalDelayTable::~CalDelayTable() {
		for (unsigned int i = 0; i < privateRows.size(); i++) 
			delete(privateRows.at(i));
	}

	/**
	 * Container to which this table belongs.
	 */
	ASDM &CalDelayTable::getContainer() const {
		return container;
	}

	/**
	 * Return the number of rows in the table.
	 */

	unsigned int CalDelayTable::size() {
		return row.size();
	}	
	
	
	/**
	 * Return the name of this table.
	 */
	string CalDelayTable::getName() const {
		return tableName;
	}

	/**
	 * Return this table's Entity.
	 */
	Entity CalDelayTable::getEntity() const {
		return entity;
	}

	/**
	 * Set this table's Entity.
	 */
	void CalDelayTable::setEntity(Entity e) {
		this->entity = e; 
	}
	
	//
	// ====> Row creation.
	//
	
	/**
	 * Create a new row.
	 */
	CalDelayRow *CalDelayTable::newRow() {
		return new CalDelayRow (*this);
	}
	
	CalDelayRow *CalDelayTable::newRowEmpty() {
		return newRow ();
	}


	/**
	 * Create a new row initialized to the specified values.
	 * @return a pointer on the created and initialized row.
	
 	 * @param antennaName. 
	
 	 * @param atmPhaseCorrection. 
	
 	 * @param basebandName. 
	
 	 * @param receiverBand. 
	
 	 * @param calDataId. 
	
 	 * @param calReductionId. 
	
 	 * @param startValidTime. 
	
 	 * @param endValidTime. 
	
 	 * @param refAntennaName. 
	
 	 * @param numReceptor. 
	
 	 * @param delayError. 
	
 	 * @param delayOffset. 
	
 	 * @param polarizationTypes. 
	
 	 * @param reducedChiSquared. 
	
     */
	CalDelayRow* CalDelayTable::newRow(string antennaName, AtmPhaseCorrectionMod::AtmPhaseCorrection atmPhaseCorrection, BasebandNameMod::BasebandName basebandName, ReceiverBandMod::ReceiverBand receiverBand, Tag calDataId, Tag calReductionId, ArrayTime startValidTime, ArrayTime endValidTime, string refAntennaName, int numReceptor, vector<double > delayError, vector<double > delayOffset, vector<PolarizationTypeMod::PolarizationType > polarizationTypes, vector<double > reducedChiSquared){
		CalDelayRow *row = new CalDelayRow(*this);
			
		row->setAntennaName(antennaName);
			
		row->setAtmPhaseCorrection(atmPhaseCorrection);
			
		row->setBasebandName(basebandName);
			
		row->setReceiverBand(receiverBand);
			
		row->setCalDataId(calDataId);
			
		row->setCalReductionId(calReductionId);
			
		row->setStartValidTime(startValidTime);
			
		row->setEndValidTime(endValidTime);
			
		row->setRefAntennaName(refAntennaName);
			
		row->setNumReceptor(numReceptor);
			
		row->setDelayError(delayError);
			
		row->setDelayOffset(delayOffset);
			
		row->setPolarizationTypes(polarizationTypes);
			
		row->setReducedChiSquared(reducedChiSquared);
	
		return row;		
	}	

	CalDelayRow* CalDelayTable::newRowFull(string antennaName, AtmPhaseCorrectionMod::AtmPhaseCorrection atmPhaseCorrection, BasebandNameMod::BasebandName basebandName, ReceiverBandMod::ReceiverBand receiverBand, Tag calDataId, Tag calReductionId, ArrayTime startValidTime, ArrayTime endValidTime, string refAntennaName, int numReceptor, vector<double > delayError, vector<double > delayOffset, vector<PolarizationTypeMod::PolarizationType > polarizationTypes, vector<double > reducedChiSquared)	{
		CalDelayRow *row = new CalDelayRow(*this);
			
		row->setAntennaName(antennaName);
			
		row->setAtmPhaseCorrection(atmPhaseCorrection);
			
		row->setBasebandName(basebandName);
			
		row->setReceiverBand(receiverBand);
			
		row->setCalDataId(calDataId);
			
		row->setCalReductionId(calReductionId);
			
		row->setStartValidTime(startValidTime);
			
		row->setEndValidTime(endValidTime);
			
		row->setRefAntennaName(refAntennaName);
			
		row->setNumReceptor(numReceptor);
			
		row->setDelayError(delayError);
			
		row->setDelayOffset(delayOffset);
			
		row->setPolarizationTypes(polarizationTypes);
			
		row->setReducedChiSquared(reducedChiSquared);
	
		return row;				
	}
	


CalDelayRow* CalDelayTable::newRow(CalDelayRow* row) {
	return new CalDelayRow(*this, *row);
}

CalDelayRow* CalDelayTable::newRowCopy(CalDelayRow* row) {
	return new CalDelayRow(*this, *row);
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
	CalDelayRow* CalDelayTable::add(CalDelayRow* x) {
		
		if (getRowByKey(
						x->getAntennaName()
						,
						x->getAtmPhaseCorrection()
						,
						x->getBasebandName()
						,
						x->getReceiverBand()
						,
						x->getCalDataId()
						,
						x->getCalReductionId()
						))
			//throw DuplicateKey(x.getAntennaName() + "|" + x.getAtmPhaseCorrection() + "|" + x.getBasebandName() + "|" + x.getReceiverBand() + "|" + x.getCalDataId() + "|" + x.getCalReductionId(),"CalDelay");
			throw DuplicateKey("Duplicate key exception in ","CalDelayTable");
		
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
	CalDelayRow*  CalDelayTable::checkAndAdd(CalDelayRow* x)  {
		
		
		if (getRowByKey(
	
			x->getAntennaName()
	,
			x->getAtmPhaseCorrection()
	,
			x->getBasebandName()
	,
			x->getReceiverBand()
	,
			x->getCalDataId()
	,
			x->getCalReductionId()
			
		)) throw DuplicateKey("Duplicate key exception in ", "CalDelayTable");
		
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
	 * @return Alls rows as an array of CalDelayRow
	 */
	vector<CalDelayRow *> CalDelayTable::get() {
		return privateRows;
		// return row;
	}

	
/*
 ** Returns a CalDelayRow* given a key.
 ** @return a pointer to the row having the key whose values are passed as parameters, or 0 if
 ** no row exists for that key.
 **
 */
 	CalDelayRow* CalDelayTable::getRowByKey(string antennaName, AtmPhaseCorrectionMod::AtmPhaseCorrection atmPhaseCorrection, BasebandNameMod::BasebandName basebandName, ReceiverBandMod::ReceiverBand receiverBand, Tag calDataId, Tag calReductionId)  {
	CalDelayRow* aRow = 0;
	for (unsigned int i = 0; i < row.size(); i++) {
		aRow = row.at(i);
		
			
				if (aRow->antennaName != antennaName) continue;
			
		
			
				if (aRow->atmPhaseCorrection != atmPhaseCorrection) continue;
			
		
			
				if (aRow->basebandName != basebandName) continue;
			
		
			
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
 	 		
 * @param atmPhaseCorrection.
 	 		
 * @param basebandName.
 	 		
 * @param receiverBand.
 	 		
 * @param calDataId.
 	 		
 * @param calReductionId.
 	 		
 * @param startValidTime.
 	 		
 * @param endValidTime.
 	 		
 * @param refAntennaName.
 	 		
 * @param numReceptor.
 	 		
 * @param delayError.
 	 		
 * @param delayOffset.
 	 		
 * @param polarizationTypes.
 	 		
 * @param reducedChiSquared.
 	 		 
 */
CalDelayRow* CalDelayTable::lookup(string antennaName, AtmPhaseCorrectionMod::AtmPhaseCorrection atmPhaseCorrection, BasebandNameMod::BasebandName basebandName, ReceiverBandMod::ReceiverBand receiverBand, Tag calDataId, Tag calReductionId, ArrayTime startValidTime, ArrayTime endValidTime, string refAntennaName, int numReceptor, vector<double > delayError, vector<double > delayOffset, vector<PolarizationTypeMod::PolarizationType > polarizationTypes, vector<double > reducedChiSquared) {
		CalDelayRow* aRow;
		for (unsigned int i = 0; i < size(); i++) {
			aRow = row.at(i); 
			if (aRow->compareNoAutoInc(antennaName, atmPhaseCorrection, basebandName, receiverBand, calDataId, calReductionId, startValidTime, endValidTime, refAntennaName, numReceptor, delayError, delayOffset, polarizationTypes, reducedChiSquared)) return aRow;
		}			
		return 0;	
} 
	
 	 	

	




#ifndef WITHOUT_ACS
	// Conversion Methods

	CalDelayTableIDL *CalDelayTable::toIDL() {
		CalDelayTableIDL *x = new CalDelayTableIDL ();
		unsigned int nrow = size();
		x->row.length(nrow);
		vector<CalDelayRow*> v = get();
		for (unsigned int i = 0; i < nrow; ++i) {
			x->row[i] = *(v[i]->toIDL());
		}
		return x;
	}
#endif
	
#ifndef WITHOUT_ACS
	void CalDelayTable::fromIDL(CalDelayTableIDL x) {
		unsigned int nrow = x.row.length();
		for (unsigned int i = 0; i < nrow; ++i) {
			CalDelayRow *tmp = newRow();
			tmp->setFromIDL(x.row[i]);
			// checkAndAdd(tmp);
			add(tmp);
		}
	}
#endif

	char *CalDelayTable::toFITS() const  {
		throw ConversionException("Not implemented","CalDelay");
	}

	void CalDelayTable::fromFITS(char *fits)  {
		throw ConversionException("Not implemented","CalDelay");
	}

	string CalDelayTable::toVOTable() const {
		throw ConversionException("Not implemented","CalDelay");
	}

	void CalDelayTable::fromVOTable(string vo) {
		throw ConversionException("Not implemented","CalDelay");
	}

	
	string CalDelayTable::toXML()  {
		string buf;
		buf.append("<?xml version=\"1.0\" encoding=\"ISO-8859-1\"?> ");
		buf.append("<CalDelayTable xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" xmlns=\"http://Alma/XASDM/CalDelayTable\" xsi:schemaLocation=\"http://Alma/XASDM/CalDelayTable http://almaobservatory.org/XML/XASDM/2/CalDelayTable.xsd\"> ");	
		buf.append(entity.toXML());
		string s = container.getEntity().toXML();
		// Change the "Entity" tag to "ContainerEntity".
		buf.append("<Container" + s.substr(1,s.length() - 1)+" ");
		vector<CalDelayRow*> v = get();
		for (unsigned int i = 0; i < v.size(); ++i) {
			try {
				buf.append(v[i]->toXML());
			} catch (NoSuchRow e) {
			}
			buf.append("  ");
		}		
		buf.append("</CalDelayTable> ");
		return buf;
	}

	
	void CalDelayTable::fromXML(string xmlDoc)  {
		Parser xml(xmlDoc);
		if (!xml.isStr("<CalDelayTable")) 
			error();
		// cout << "Parsing a CalDelayTable" << endl;
		string s = xml.getElement("<Entity","/>");
		if (s.length() == 0) 
			error();
		Entity e;
		e.setFromXML(s);
		if (e.getEntityTypeName() != "CalDelayTable")
			error();
		setEntity(e);
		// Skip the container's entity; but, it has to be there.
		s = xml.getElement("<ContainerEntity","/>");
		if (s.length() == 0) 
			error();

		// Get each row in the table.
		s = xml.getElementContent("<row>","</row>");
		CalDelayRow *row;
		while (s.length() != 0) {
			// cout << "Parsing a CalDelayRow" << endl; 
			row = newRow();
			row->setFromXML(s);
			try {
				checkAndAdd(row);
			} catch (DuplicateKey e1) {
				throw ConversionException(e1.getMessage(),"CalDelayTable");
			} 
			catch (UniquenessViolationException e1) {
				throw ConversionException(e1.getMessage(),"CalDelayTable");	
			}
			catch (...) {
				// cout << "Unexpected error in CalDelayTable::checkAndAdd called from CalDelayTable::fromXML " << endl;
			}
			s = xml.getElementContent("<row>","</row>");
		}
		if (!xml.isStr("</CalDelayTable>")) 
			error();
	}

	
	void CalDelayTable::error()  {
		throw ConversionException("Invalid xml document","CalDelay");
	}
	
	
	string CalDelayTable::toMIME() {
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

	
	void CalDelayTable::setFromMIME(const string & mimeMsg) {
		// cout << "Entering setFromMIME" << endl;
	 	string terminator = "Content-Type: binary/octet-stream\nContent-ID: <content.bin>\n\n";
	 	
	 	// Look for the string announcing the binary part.
	 	string::size_type loc = mimeMsg.find( terminator, 0 );
	 	
	 	if ( loc == string::npos ) {
	 		throw ConversionException("Failed to detect the beginning of the binary part", "CalDelay");
	 	}
	
	 	// Create an EndianISStream from the substring containing the binary part.
	 	EndianISStream eiss(mimeMsg.substr(loc+terminator.size()));
	 	
	 	entity = Entity::fromBin(eiss);
	 	
	 	// We do nothing with that but we have to read it.
	 	Entity containerEntity = Entity::fromBin(eiss);
	 		 	
	 	int numRows = eiss.readInt();
	 	try {
	 		for (int i = 0; i < numRows; i++) {
	 			CalDelayRow* aRow = CalDelayRow::fromBin(eiss, *this);
	 			checkAndAdd(aRow);
	 		}
	 	}
	 	catch (DuplicateKey e) {
	 		throw ConversionException("Error while writing binary data , the message was "
	 					+ e.getMessage(), "CalDelay");
	 	}
		catch (TagFormatException e) {
			throw ConversionException("Error while reading binary data , the message was "
					+ e.getMessage(), "CalDelay");
		} 		 	
	}

	
	void CalDelayTable::toFile(string directory) {
		if (!directoryExists(directory.c_str()) &&
			!createPath(directory.c_str())) {
			throw ConversionException("Could not create directory " , directory);
		}
		
		if (fileAsBin) {
			// write the bin serialized
			string fileName = directory + "/CalDelay.bin";
			ofstream tableout(fileName.c_str(),ios::out|ios::trunc);
			if (tableout.rdstate() == ostream::failbit)
				throw ConversionException("Could not open file " + fileName + " to write ", "CalDelay");
			tableout << toMIME() << endl;
			tableout.close();
			if (tableout.rdstate() == ostream::failbit)
				throw ConversionException("Could not close file " + fileName, "CalDelay");
		}
		else {
			// write the XML
			string fileName = directory + "/CalDelay.xml";
			ofstream tableout(fileName.c_str(),ios::out|ios::trunc);
			if (tableout.rdstate() == ostream::failbit)
				throw ConversionException("Could not open file " + fileName + " to write ", "CalDelay");
			tableout << toXML() << endl;
			tableout.close();
			if (tableout.rdstate() == ostream::failbit)
				throw ConversionException("Could not close file " + fileName, "CalDelay");
		}
	}

	
	void CalDelayTable::setFromFile(const string& directory) {
		string tablename;
		if (fileAsBin)
			tablename = directory + "/CalDelay.bin";
		else
			tablename = directory + "/CalDelay.xml";
			
		// Determine the file size.
		ifstream::pos_type size;
		ifstream tablefile(tablename.c_str(), ios::in|ios::binary|ios::ate);

 		if (tablefile.is_open()) { 
  				size = tablefile.tellg(); 
  		}
		else {
				throw ConversionException("Could not open file " + tablename, "CalDelay");
		}
		
		// Re position to the beginning.
		tablefile.seekg(0);
		
		// Read in a stringstream.
		stringstream ss;
		ss << tablefile.rdbuf();

		if (tablefile.rdstate() == istream::failbit || tablefile.rdstate() == istream::badbit) {
			throw ConversionException("Error reading file " + tablename,"CalDelay");
		}

		// And close
		tablefile.close();
		if (tablefile.rdstate() == istream::failbit)
			throw ConversionException("Could not close file " + tablename,"CalDelay");
					
		// And parse the content with the appropriate method
		if (fileAsBin) 
			setFromMIME(ss.str());
		else
			fromXML(ss.str());	
	}			

	

	

			
	
	

	
} // End namespace asdm
 
