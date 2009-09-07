
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
 * File ConfigDescriptionTable.cpp
 */
#include <ConversionException.h>
#include <DuplicateKey.h>
#include <OutOfBoundsException.h>

using asdm::ConversionException;
using asdm::DuplicateKey;
using asdm::OutOfBoundsException;

#include <ASDM.h>
#include <ConfigDescriptionTable.h>
#include <ConfigDescriptionRow.h>
#include <Parser.h>

using asdm::ASDM;
using asdm::ConfigDescriptionTable;
using asdm::ConfigDescriptionRow;
using asdm::Parser;

#include <iostream>
#include <sstream>
#include <set>
using namespace std;

#include <Misc.h>
using namespace asdm;


namespace asdm {

	string ConfigDescriptionTable::tableName = "ConfigDescription";
	

	/**
	 * The list of field names that make up key key.
	 * (Initialization is in the constructor.)
	 */
	vector<string> ConfigDescriptionTable::key;

	/**
	 * Return the list of field names that make up key key
	 * as an array of strings.
	 */	
	vector<string> ConfigDescriptionTable::getKeyName() {
		return key;
	}


	ConfigDescriptionTable::ConfigDescriptionTable(ASDM &c) : container(c) {

	
		key.push_back("configDescriptionId");
	


		// Define a default entity.
		entity.setEntityId(EntityId("uid://X0/X0/X0"));
		entity.setEntityIdEncrypted("na");
		entity.setEntityTypeName("ConfigDescriptionTable");
		entity.setEntityVersion("1");
		entity.setInstanceVersion("1");
		
		// Archive XML
		archiveAsBin = false;
		
		// File XML
		fileAsBin = false;
	}
	
/**
 * A destructor for ConfigDescriptionTable.
 */
 
	ConfigDescriptionTable::~ConfigDescriptionTable() {
		for (unsigned int i = 0; i < privateRows.size(); i++) 
			delete(privateRows.at(i));
	}

	/**
	 * Container to which this table belongs.
	 */
	ASDM &ConfigDescriptionTable::getContainer() const {
		return container;
	}

	/**
	 * Return the number of rows in the table.
	 */

	unsigned int ConfigDescriptionTable::size() {
		return row.size();
	}	
	
	
	/**
	 * Return the name of this table.
	 */
	string ConfigDescriptionTable::getName() const {
		return tableName;
	}

	/**
	 * Return this table's Entity.
	 */
	Entity ConfigDescriptionTable::getEntity() const {
		return entity;
	}

	/**
	 * Set this table's Entity.
	 */
	void ConfigDescriptionTable::setEntity(Entity e) {
		this->entity = e; 
	}
	
	//
	// ====> Row creation.
	//
	
	/**
	 * Create a new row.
	 */
	ConfigDescriptionRow *ConfigDescriptionTable::newRow() {
		return new ConfigDescriptionRow (*this);
	}
	
	ConfigDescriptionRow *ConfigDescriptionTable::newRowEmpty() {
		return newRow ();
	}


	/**
	 * Create a new row initialized to the specified values.
	 * @return a pointer on the created and initialized row.
	
 	 * @param numAntenna. 
	
 	 * @param numDataDescription. 
	
 	 * @param numFeed. 
	
 	 * @param correlationMode. 
	
 	 * @param numAtmPhaseCorrection. 
	
 	 * @param atmPhaseCorrection. 
	
 	 * @param processorType. 
	
 	 * @param spectralType. 
	
 	 * @param antennaId. 
	
 	 * @param feedId. 
	
 	 * @param switchCycleId. 
	
 	 * @param dataDescriptionId. 
	
 	 * @param processorId. 
	
     */
	ConfigDescriptionRow* ConfigDescriptionTable::newRow(int numAntenna, int numDataDescription, int numFeed, CorrelationModeMod::CorrelationMode correlationMode, int numAtmPhaseCorrection, vector<AtmPhaseCorrectionMod::AtmPhaseCorrection > atmPhaseCorrection, ProcessorTypeMod::ProcessorType processorType, SpectralResolutionTypeMod::SpectralResolutionType spectralType, vector<Tag>  antennaId, vector<int>  feedId, vector<Tag>  switchCycleId, vector<Tag>  dataDescriptionId, Tag processorId){
		ConfigDescriptionRow *row = new ConfigDescriptionRow(*this);
			
		row->setNumAntenna(numAntenna);
			
		row->setNumDataDescription(numDataDescription);
			
		row->setNumFeed(numFeed);
			
		row->setCorrelationMode(correlationMode);
			
		row->setNumAtmPhaseCorrection(numAtmPhaseCorrection);
			
		row->setAtmPhaseCorrection(atmPhaseCorrection);
			
		row->setProcessorType(processorType);
			
		row->setSpectralType(spectralType);
			
		row->setAntennaId(antennaId);
			
		row->setFeedId(feedId);
			
		row->setSwitchCycleId(switchCycleId);
			
		row->setDataDescriptionId(dataDescriptionId);
			
		row->setProcessorId(processorId);
	
		return row;		
	}	

	ConfigDescriptionRow* ConfigDescriptionTable::newRowFull(int numAntenna, int numDataDescription, int numFeed, CorrelationModeMod::CorrelationMode correlationMode, int numAtmPhaseCorrection, vector<AtmPhaseCorrectionMod::AtmPhaseCorrection > atmPhaseCorrection, ProcessorTypeMod::ProcessorType processorType, SpectralResolutionTypeMod::SpectralResolutionType spectralType, vector<Tag>  antennaId, vector<int>  feedId, vector<Tag>  switchCycleId, vector<Tag>  dataDescriptionId, Tag processorId)	{
		ConfigDescriptionRow *row = new ConfigDescriptionRow(*this);
			
		row->setNumAntenna(numAntenna);
			
		row->setNumDataDescription(numDataDescription);
			
		row->setNumFeed(numFeed);
			
		row->setCorrelationMode(correlationMode);
			
		row->setNumAtmPhaseCorrection(numAtmPhaseCorrection);
			
		row->setAtmPhaseCorrection(atmPhaseCorrection);
			
		row->setProcessorType(processorType);
			
		row->setSpectralType(spectralType);
			
		row->setAntennaId(antennaId);
			
		row->setFeedId(feedId);
			
		row->setSwitchCycleId(switchCycleId);
			
		row->setDataDescriptionId(dataDescriptionId);
			
		row->setProcessorId(processorId);
	
		return row;				
	}
	


ConfigDescriptionRow* ConfigDescriptionTable::newRow(ConfigDescriptionRow* row) {
	return new ConfigDescriptionRow(*this, *row);
}

ConfigDescriptionRow* ConfigDescriptionTable::newRowCopy(ConfigDescriptionRow* row) {
	return new ConfigDescriptionRow(*this, *row);
}

	//
	// Append a row to its table.
	//

	
	 
	
	/** 
 	 * Look up the table for a row whose noautoincrementable attributes are matching their
 	 * homologues in *x.  If a row is found  this row else autoincrement  *x.configDescriptionId, 
 	 * add x to its table and returns x.
 	 *  
 	 * @returns a pointer on a ConfigDescriptionRow.
 	 * @param x. A pointer on the row to be added.
 	 */ 
 		
			
	ConfigDescriptionRow* ConfigDescriptionTable::add(ConfigDescriptionRow* x) {
			 
		ConfigDescriptionRow* aRow = lookup(
				
		x->getNumAntenna()
				,
		x->getNumDataDescription()
				,
		x->getNumFeed()
				,
		x->getCorrelationMode()
				,
		x->getNumAtmPhaseCorrection()
				,
		x->getAtmPhaseCorrection()
				,
		x->getProcessorType()
				,
		x->getSpectralType()
				,
		x->getAntennaId()
				,
		x->getFeedId()
				,
		x->getSwitchCycleId()
				,
		x->getDataDescriptionId()
				,
		x->getProcessorId()
				
		);
		if (aRow) return aRow;
			

			
		// Autoincrement configDescriptionId
		x->setConfigDescriptionId(Tag(size(), TagType::ConfigDescription));
						
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
	 
	 * @throws UniquenessViolationException
	 
	 */
	ConfigDescriptionRow*  ConfigDescriptionTable::checkAndAdd(ConfigDescriptionRow* x)  {
	 
		 
		if (lookup(
			
			x->getNumAntenna()
		,
			x->getNumDataDescription()
		,
			x->getNumFeed()
		,
			x->getCorrelationMode()
		,
			x->getNumAtmPhaseCorrection()
		,
			x->getAtmPhaseCorrection()
		,
			x->getProcessorType()
		,
			x->getSpectralType()
		,
			x->getAntennaId()
		,
			x->getFeedId()
		,
			x->getSwitchCycleId()
		,
			x->getDataDescriptionId()
		,
			x->getProcessorId()
		
		)) throw UniquenessViolationException("Uniqueness violation exception in table ConfigDescriptionTable");
		
		
		
		if (getRowByKey(
	
			x->getConfigDescriptionId()
			
		)) throw DuplicateKey("Duplicate key exception in ", "ConfigDescriptionTable");
		
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
	 * @return Alls rows as an array of ConfigDescriptionRow
	 */
	vector<ConfigDescriptionRow *> ConfigDescriptionTable::get() {
		return privateRows;
		// return row;
	}

	
/*
 ** Returns a ConfigDescriptionRow* given a key.
 ** @return a pointer to the row having the key whose values are passed as parameters, or 0 if
 ** no row exists for that key.
 **
 */
 	ConfigDescriptionRow* ConfigDescriptionTable::getRowByKey(Tag configDescriptionId)  {
	ConfigDescriptionRow* aRow = 0;
	for (unsigned int i = 0; i < row.size(); i++) {
		aRow = row.at(i);
		
			
				if (aRow->configDescriptionId != configDescriptionId) continue;
			
		
		return aRow;
	}
	return 0;		
}
	

	
/**
 * Look up the table for a row whose all attributes  except the autoincrementable one 
 * are equal to the corresponding parameters of the method.
 * @return a pointer on this row if any, 0 otherwise.
 *
			
 * @param numAntenna.
 	 		
 * @param numDataDescription.
 	 		
 * @param numFeed.
 	 		
 * @param correlationMode.
 	 		
 * @param numAtmPhaseCorrection.
 	 		
 * @param atmPhaseCorrection.
 	 		
 * @param processorType.
 	 		
 * @param spectralType.
 	 		
 * @param antennaId.
 	 		
 * @param feedId.
 	 		
 * @param switchCycleId.
 	 		
 * @param dataDescriptionId.
 	 		
 * @param processorId.
 	 		 
 */
ConfigDescriptionRow* ConfigDescriptionTable::lookup(int numAntenna, int numDataDescription, int numFeed, CorrelationModeMod::CorrelationMode correlationMode, int numAtmPhaseCorrection, vector<AtmPhaseCorrectionMod::AtmPhaseCorrection > atmPhaseCorrection, ProcessorTypeMod::ProcessorType processorType, SpectralResolutionTypeMod::SpectralResolutionType spectralType, vector<Tag>  antennaId, vector<int>  feedId, vector<Tag>  switchCycleId, vector<Tag>  dataDescriptionId, Tag processorId) {
		ConfigDescriptionRow* aRow;
		for (unsigned int i = 0; i < size(); i++) {
			aRow = row.at(i); 
			if (aRow->compareNoAutoInc(numAntenna, numDataDescription, numFeed, correlationMode, numAtmPhaseCorrection, atmPhaseCorrection, processorType, spectralType, antennaId, feedId, switchCycleId, dataDescriptionId, processorId)) return aRow;
		}			
		return 0;	
} 
	
 	 	

	




#ifndef WITHOUT_ACS
	// Conversion Methods

	ConfigDescriptionTableIDL *ConfigDescriptionTable::toIDL() {
		ConfigDescriptionTableIDL *x = new ConfigDescriptionTableIDL ();
		unsigned int nrow = size();
		x->row.length(nrow);
		vector<ConfigDescriptionRow*> v = get();
		for (unsigned int i = 0; i < nrow; ++i) {
			x->row[i] = *(v[i]->toIDL());
		}
		return x;
	}
#endif
	
#ifndef WITHOUT_ACS
	void ConfigDescriptionTable::fromIDL(ConfigDescriptionTableIDL x) {
		unsigned int nrow = x.row.length();
		for (unsigned int i = 0; i < nrow; ++i) {
			ConfigDescriptionRow *tmp = newRow();
			tmp->setFromIDL(x.row[i]);
			// checkAndAdd(tmp);
			add(tmp);
		}
	}
#endif

	char *ConfigDescriptionTable::toFITS() const  {
		throw ConversionException("Not implemented","ConfigDescription");
	}

	void ConfigDescriptionTable::fromFITS(char *fits)  {
		throw ConversionException("Not implemented","ConfigDescription");
	}

	string ConfigDescriptionTable::toVOTable() const {
		throw ConversionException("Not implemented","ConfigDescription");
	}

	void ConfigDescriptionTable::fromVOTable(string vo) {
		throw ConversionException("Not implemented","ConfigDescription");
	}

	
	string ConfigDescriptionTable::toXML()  {
		string buf;
		buf.append("<?xml version=\"1.0\" encoding=\"ISO-8859-1\"?> ");
		buf.append("<ConfigDescriptionTable xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" xmlns=\"http://Alma/XASDM/ConfigDescriptionTable\" xsi:schemaLocation=\"http://Alma/XASDM/ConfigDescriptionTable http://almaobservatory.org/XML/XASDM/2/ConfigDescriptionTable.xsd\"> ");	
		buf.append(entity.toXML());
		string s = container.getEntity().toXML();
		// Change the "Entity" tag to "ContainerEntity".
		buf.append("<Container" + s.substr(1,s.length() - 1)+" ");
		vector<ConfigDescriptionRow*> v = get();
		for (unsigned int i = 0; i < v.size(); ++i) {
			try {
				buf.append(v[i]->toXML());
			} catch (NoSuchRow e) {
			}
			buf.append("  ");
		}		
		buf.append("</ConfigDescriptionTable> ");
		return buf;
	}

	
	void ConfigDescriptionTable::fromXML(string xmlDoc)  {
		Parser xml(xmlDoc);
		if (!xml.isStr("<ConfigDescriptionTable")) 
			error();
		// cout << "Parsing a ConfigDescriptionTable" << endl;
		string s = xml.getElement("<Entity","/>");
		if (s.length() == 0) 
			error();
		Entity e;
		e.setFromXML(s);
		if (e.getEntityTypeName() != "ConfigDescriptionTable")
			error();
		setEntity(e);
		// Skip the container's entity; but, it has to be there.
		s = xml.getElement("<ContainerEntity","/>");
		if (s.length() == 0) 
			error();

		// Get each row in the table.
		s = xml.getElementContent("<row>","</row>");
		ConfigDescriptionRow *row;
		while (s.length() != 0) {
			// cout << "Parsing a ConfigDescriptionRow" << endl; 
			row = newRow();
			row->setFromXML(s);
			try {
				checkAndAdd(row);
			} catch (DuplicateKey e1) {
				throw ConversionException(e1.getMessage(),"ConfigDescriptionTable");
			} 
			catch (UniquenessViolationException e1) {
				throw ConversionException(e1.getMessage(),"ConfigDescriptionTable");	
			}
			catch (...) {
				// cout << "Unexpected error in ConfigDescriptionTable::checkAndAdd called from ConfigDescriptionTable::fromXML " << endl;
			}
			s = xml.getElementContent("<row>","</row>");
		}
		if (!xml.isStr("</ConfigDescriptionTable>")) 
			error();
	}

	
	void ConfigDescriptionTable::error()  {
		throw ConversionException("Invalid xml document","ConfigDescription");
	}
	
	
	string ConfigDescriptionTable::toMIME() {
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

	
	void ConfigDescriptionTable::setFromMIME(const string & mimeMsg) {
		// cout << "Entering setFromMIME" << endl;
	 	string terminator = "Content-Type: binary/octet-stream\nContent-ID: <content.bin>\n\n";
	 	
	 	// Look for the string announcing the binary part.
	 	string::size_type loc = mimeMsg.find( terminator, 0 );
	 	
	 	if ( loc == string::npos ) {
	 		throw ConversionException("Failed to detect the beginning of the binary part", "ConfigDescription");
	 	}
	
	 	// Create an EndianISStream from the substring containing the binary part.
	 	EndianISStream eiss(mimeMsg.substr(loc+terminator.size()));
	 	
	 	entity = Entity::fromBin(eiss);
	 	
	 	// We do nothing with that but we have to read it.
	 	Entity containerEntity = Entity::fromBin(eiss);
	 		 	
	 	int numRows = eiss.readInt();
	 	try {
	 		for (int i = 0; i < numRows; i++) {
	 			ConfigDescriptionRow* aRow = ConfigDescriptionRow::fromBin(eiss, *this);
	 			checkAndAdd(aRow);
	 		}
	 	}
	 	catch (DuplicateKey e) {
	 		throw ConversionException("Error while writing binary data , the message was "
	 					+ e.getMessage(), "ConfigDescription");
	 	}
		catch (TagFormatException e) {
			throw ConversionException("Error while reading binary data , the message was "
					+ e.getMessage(), "ConfigDescription");
		} 		 	
	}

	
	void ConfigDescriptionTable::toFile(string directory) {
		if (!directoryExists(directory.c_str()) &&
			!createPath(directory.c_str())) {
			throw ConversionException("Could not create directory " , directory);
		}
		
		if (fileAsBin) {
			// write the bin serialized
			string fileName = directory + "/ConfigDescription.bin";
			ofstream tableout(fileName.c_str(),ios::out|ios::trunc);
			if (tableout.rdstate() == ostream::failbit)
				throw ConversionException("Could not open file " + fileName + " to write ", "ConfigDescription");
			tableout << toMIME() << endl;
			tableout.close();
			if (tableout.rdstate() == ostream::failbit)
				throw ConversionException("Could not close file " + fileName, "ConfigDescription");
		}
		else {
			// write the XML
			string fileName = directory + "/ConfigDescription.xml";
			ofstream tableout(fileName.c_str(),ios::out|ios::trunc);
			if (tableout.rdstate() == ostream::failbit)
				throw ConversionException("Could not open file " + fileName + " to write ", "ConfigDescription");
			tableout << toXML() << endl;
			tableout.close();
			if (tableout.rdstate() == ostream::failbit)
				throw ConversionException("Could not close file " + fileName, "ConfigDescription");
		}
	}

	
	void ConfigDescriptionTable::setFromFile(const string& directory) {
		string tablename;
		if (fileAsBin)
			tablename = directory + "/ConfigDescription.bin";
		else
			tablename = directory + "/ConfigDescription.xml";
			
		// Determine the file size.
		ifstream::pos_type size;
		ifstream tablefile(tablename.c_str(), ios::in|ios::binary|ios::ate);

 		if (tablefile.is_open()) { 
  				size = tablefile.tellg(); 
  		}
		else {
				throw ConversionException("Could not open file " + tablename, "ConfigDescription");
		}
		
		// Re position to the beginning.
		tablefile.seekg(0);
		
		// Read in a stringstream.
		stringstream ss;
		ss << tablefile.rdbuf();

		if (tablefile.rdstate() == istream::failbit || tablefile.rdstate() == istream::badbit) {
			throw ConversionException("Error reading file " + tablename,"ConfigDescription");
		}

		// And close
		tablefile.close();
		if (tablefile.rdstate() == istream::failbit)
			throw ConversionException("Could not close file " + tablename,"ConfigDescription");
					
		// And parse the content with the appropriate method
		if (fileAsBin) 
			setFromMIME(ss.str());
		else
			fromXML(ss.str());	
	}			

	

	

			
	
	

	
	void ConfigDescriptionTable::autoIncrement(string key, ConfigDescriptionRow* x) {
		map<string, int>::iterator iter;
		if ((iter=noAutoIncIds.find(key)) == noAutoIncIds.end()) {
			// There is not yet a combination of the non autoinc attributes values in the hashtable
			
			// Initialize  configDescriptionId to Tag(0).
			x->setConfigDescriptionId(Tag(0,  TagType::ConfigDescription));
			
			// Record it in the map.		
			noAutoIncIds.insert(make_pair(key, 0));			
		} 
		else {
			// There is already a combination of the non autoinc attributes values in the hashtable
			// Increment its value.
			int n = iter->second + 1; 
			
			// Initialize  configDescriptionId to Tag(n).
			x->setConfigDescriptionId(Tag(n, TagType::ConfigDescription));
			
			// Record it in the map.		
			noAutoIncIds.insert(make_pair(key, n));				
		}		
	}
	
} // End namespace asdm
 
