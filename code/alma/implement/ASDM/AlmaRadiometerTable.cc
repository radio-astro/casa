
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
 * File AlmaRadiometerTable.cpp
 */
#include <ConversionException.h>
#include <DuplicateKey.h>
#include <OutOfBoundsException.h>

using asdm::ConversionException;
using asdm::DuplicateKey;
using asdm::OutOfBoundsException;

#include <ASDM.h>
#include <AlmaRadiometerTable.h>
#include <AlmaRadiometerRow.h>
#include <Parser.h>

using asdm::ASDM;
using asdm::AlmaRadiometerTable;
using asdm::AlmaRadiometerRow;
using asdm::Parser;

#include <iostream>
#include <sstream>
#include <set>
using namespace std;

#include <Misc.h>
using namespace asdm;


namespace asdm {

	string AlmaRadiometerTable::tableName = "AlmaRadiometer";
	

	/**
	 * The list of field names that make up key key.
	 * (Initialization is in the constructor.)
	 */
	vector<string> AlmaRadiometerTable::key;

	/**
	 * Return the list of field names that make up key key
	 * as an array of strings.
	 */	
	vector<string> AlmaRadiometerTable::getKeyName() {
		return key;
	}


	AlmaRadiometerTable::AlmaRadiometerTable(ASDM &c) : container(c) {

	
		key.push_back("almaRadiometerId");
	


		// Define a default entity.
		entity.setEntityId(EntityId("uid://X0/X0/X0"));
		entity.setEntityIdEncrypted("na");
		entity.setEntityTypeName("AlmaRadiometerTable");
		entity.setEntityVersion("1");
		entity.setInstanceVersion("1");
		
		// Archive XML
		archiveAsBin = false;
		
		// File XML
		fileAsBin = false;
	}
	
/**
 * A destructor for AlmaRadiometerTable.
 */
 
	AlmaRadiometerTable::~AlmaRadiometerTable() {
		for (unsigned int i = 0; i < privateRows.size(); i++) 
			delete(privateRows.at(i));
	}

	/**
	 * Container to which this table belongs.
	 */
	ASDM &AlmaRadiometerTable::getContainer() const {
		return container;
	}

	/**
	 * Return the number of rows in the table.
	 */

	unsigned int AlmaRadiometerTable::size() {
		return row.size();
	}	
	
	
	/**
	 * Return the name of this table.
	 */
	string AlmaRadiometerTable::getName() const {
		return tableName;
	}

	/**
	 * Return this table's Entity.
	 */
	Entity AlmaRadiometerTable::getEntity() const {
		return entity;
	}

	/**
	 * Set this table's Entity.
	 */
	void AlmaRadiometerTable::setEntity(Entity e) {
		this->entity = e; 
	}
	
	//
	// ====> Row creation.
	//
	
	/**
	 * Create a new row.
	 */
	AlmaRadiometerRow *AlmaRadiometerTable::newRow() {
		return new AlmaRadiometerRow (*this);
	}
	
	AlmaRadiometerRow *AlmaRadiometerTable::newRowEmpty() {
		return newRow ();
	}



AlmaRadiometerRow* AlmaRadiometerTable::newRow(AlmaRadiometerRow* row) {
	return new AlmaRadiometerRow(*this, *row);
}

AlmaRadiometerRow* AlmaRadiometerTable::newRowCopy(AlmaRadiometerRow* row) {
	return new AlmaRadiometerRow(*this, *row);
}

	//
	// Append a row to its table.
	//

	
	 
	
	/** 
 	 * Look up the table for a row whose noautoincrementable attributes are matching their
 	 * homologues in *x.  If a row is found  this row else autoincrement  *x.almaRadiometerId, 
 	 * add x to its table and returns x.
 	 *  
 	 * @returns a pointer on a AlmaRadiometerRow.
 	 * @param x. A pointer on the row to be added.
 	 */ 
 		
			
	AlmaRadiometerRow* AlmaRadiometerTable::add(AlmaRadiometerRow* x) {
			

			
		// Autoincrement almaRadiometerId
		x->setAlmaRadiometerId(Tag(size(), TagType::AlmaRadiometer));
						
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
	AlmaRadiometerRow*  AlmaRadiometerTable::checkAndAdd(AlmaRadiometerRow* x)  {
	 
		
		
		
		if (getRowByKey(
	
			x->getAlmaRadiometerId()
			
		)) throw DuplicateKey("Duplicate key exception in ", "AlmaRadiometerTable");
		
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
	 * @return Alls rows as an array of AlmaRadiometerRow
	 */
	vector<AlmaRadiometerRow *> AlmaRadiometerTable::get() {
		return privateRows;
		// return row;
	}

	
/*
 ** Returns a AlmaRadiometerRow* given a key.
 ** @return a pointer to the row having the key whose values are passed as parameters, or 0 if
 ** no row exists for that key.
 **
 */
 	AlmaRadiometerRow* AlmaRadiometerTable::getRowByKey(Tag almaRadiometerId)  {
	AlmaRadiometerRow* aRow = 0;
	for (unsigned int i = 0; i < row.size(); i++) {
		aRow = row.at(i);
		
			
				if (aRow->almaRadiometerId != almaRadiometerId) continue;
			
		
		return aRow;
	}
	return 0;		
}
	

	
 	 	

	




#ifndef WITHOUT_ACS
	// Conversion Methods

	AlmaRadiometerTableIDL *AlmaRadiometerTable::toIDL() {
		AlmaRadiometerTableIDL *x = new AlmaRadiometerTableIDL ();
		unsigned int nrow = size();
		x->row.length(nrow);
		vector<AlmaRadiometerRow*> v = get();
		for (unsigned int i = 0; i < nrow; ++i) {
			x->row[i] = *(v[i]->toIDL());
		}
		return x;
	}
#endif
	
#ifndef WITHOUT_ACS
	void AlmaRadiometerTable::fromIDL(AlmaRadiometerTableIDL x) {
		unsigned int nrow = x.row.length();
		for (unsigned int i = 0; i < nrow; ++i) {
			AlmaRadiometerRow *tmp = newRow();
			tmp->setFromIDL(x.row[i]);
			// checkAndAdd(tmp);
			add(tmp);
		}
	}
#endif

	char *AlmaRadiometerTable::toFITS() const  {
		throw ConversionException("Not implemented","AlmaRadiometer");
	}

	void AlmaRadiometerTable::fromFITS(char *fits)  {
		throw ConversionException("Not implemented","AlmaRadiometer");
	}

	string AlmaRadiometerTable::toVOTable() const {
		throw ConversionException("Not implemented","AlmaRadiometer");
	}

	void AlmaRadiometerTable::fromVOTable(string vo) {
		throw ConversionException("Not implemented","AlmaRadiometer");
	}

	
	string AlmaRadiometerTable::toXML()  {
		string buf;
		buf.append("<?xml version=\"1.0\" encoding=\"ISO-8859-1\"?> ");
		buf.append("<AlmaRadiometerTable xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" xmlns=\"http://Alma/XASDM/AlmaRadiometerTable\" xsi:schemaLocation=\"http://Alma/XASDM/AlmaRadiometerTable http://almaobservatory.org/XML/XASDM/2/AlmaRadiometerTable.xsd\"> ");	
		buf.append(entity.toXML());
		string s = container.getEntity().toXML();
		// Change the "Entity" tag to "ContainerEntity".
		buf.append("<Container" + s.substr(1,s.length() - 1)+" ");
		vector<AlmaRadiometerRow*> v = get();
		for (unsigned int i = 0; i < v.size(); ++i) {
			try {
				buf.append(v[i]->toXML());
			} catch (NoSuchRow e) {
			}
			buf.append("  ");
		}		
		buf.append("</AlmaRadiometerTable> ");
		return buf;
	}

	
	void AlmaRadiometerTable::fromXML(string xmlDoc)  {
		Parser xml(xmlDoc);
		if (!xml.isStr("<AlmaRadiometerTable")) 
			error();
		// cout << "Parsing a AlmaRadiometerTable" << endl;
		string s = xml.getElement("<Entity","/>");
		if (s.length() == 0) 
			error();
		Entity e;
		e.setFromXML(s);
		if (e.getEntityTypeName() != "AlmaRadiometerTable")
			error();
		setEntity(e);
		// Skip the container's entity; but, it has to be there.
		s = xml.getElement("<ContainerEntity","/>");
		if (s.length() == 0) 
			error();

		// Get each row in the table.
		s = xml.getElementContent("<row>","</row>");
		AlmaRadiometerRow *row;
		while (s.length() != 0) {
			// cout << "Parsing a AlmaRadiometerRow" << endl; 
			row = newRow();
			row->setFromXML(s);
			try {
				checkAndAdd(row);
			} catch (DuplicateKey e1) {
				throw ConversionException(e1.getMessage(),"AlmaRadiometerTable");
			} 
			catch (UniquenessViolationException e1) {
				throw ConversionException(e1.getMessage(),"AlmaRadiometerTable");	
			}
			catch (...) {
				// cout << "Unexpected error in AlmaRadiometerTable::checkAndAdd called from AlmaRadiometerTable::fromXML " << endl;
			}
			s = xml.getElementContent("<row>","</row>");
		}
		if (!xml.isStr("</AlmaRadiometerTable>")) 
			error();
	}

	
	void AlmaRadiometerTable::error()  {
		throw ConversionException("Invalid xml document","AlmaRadiometer");
	}
	
	
	string AlmaRadiometerTable::toMIME() {
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

	
	void AlmaRadiometerTable::setFromMIME(const string & mimeMsg) {
		// cout << "Entering setFromMIME" << endl;
	 	string terminator = "Content-Type: binary/octet-stream\nContent-ID: <content.bin>\n\n";
	 	
	 	// Look for the string announcing the binary part.
	 	string::size_type loc = mimeMsg.find( terminator, 0 );
	 	
	 	if ( loc == string::npos ) {
	 		throw ConversionException("Failed to detect the beginning of the binary part", "AlmaRadiometer");
	 	}
	
	 	// Create an EndianISStream from the substring containing the binary part.
	 	EndianISStream eiss(mimeMsg.substr(loc+terminator.size()));
	 	
	 	entity = Entity::fromBin(eiss);
	 	
	 	// We do nothing with that but we have to read it.
	 	Entity containerEntity = Entity::fromBin(eiss);
	 		 	
	 	int numRows = eiss.readInt();
	 	try {
	 		for (int i = 0; i < numRows; i++) {
	 			AlmaRadiometerRow* aRow = AlmaRadiometerRow::fromBin(eiss, *this);
	 			checkAndAdd(aRow);
	 		}
	 	}
	 	catch (DuplicateKey e) {
	 		throw ConversionException("Error while writing binary data , the message was "
	 					+ e.getMessage(), "AlmaRadiometer");
	 	}
		catch (TagFormatException e) {
			throw ConversionException("Error while reading binary data , the message was "
					+ e.getMessage(), "AlmaRadiometer");
		} 		 	
	}

	
	void AlmaRadiometerTable::toFile(string directory) {
		if (!directoryExists(directory.c_str()) &&
			!createPath(directory.c_str())) {
			throw ConversionException("Could not create directory " , directory);
		}
		
		if (fileAsBin) {
			// write the bin serialized
			string fileName = directory + "/AlmaRadiometer.bin";
			ofstream tableout(fileName.c_str(),ios::out|ios::trunc);
			if (tableout.rdstate() == ostream::failbit)
				throw ConversionException("Could not open file " + fileName + " to write ", "AlmaRadiometer");
			tableout << toMIME() << endl;
			tableout.close();
			if (tableout.rdstate() == ostream::failbit)
				throw ConversionException("Could not close file " + fileName, "AlmaRadiometer");
		}
		else {
			// write the XML
			string fileName = directory + "/AlmaRadiometer.xml";
			ofstream tableout(fileName.c_str(),ios::out|ios::trunc);
			if (tableout.rdstate() == ostream::failbit)
				throw ConversionException("Could not open file " + fileName + " to write ", "AlmaRadiometer");
			tableout << toXML() << endl;
			tableout.close();
			if (tableout.rdstate() == ostream::failbit)
				throw ConversionException("Could not close file " + fileName, "AlmaRadiometer");
		}
	}

	
	void AlmaRadiometerTable::setFromFile(const string& directory) {
		string tablename;
		if (fileAsBin)
			tablename = directory + "/AlmaRadiometer.bin";
		else
			tablename = directory + "/AlmaRadiometer.xml";
			
		// Determine the file size.
		ifstream::pos_type size;
		ifstream tablefile(tablename.c_str(), ios::in|ios::binary|ios::ate);

 		if (tablefile.is_open()) { 
  				size = tablefile.tellg(); 
  		}
		else {
				throw ConversionException("Could not open file " + tablename, "AlmaRadiometer");
		}
		
		// Re position to the beginning.
		tablefile.seekg(0);
		
		// Read in a stringstream.
		stringstream ss;
		ss << tablefile.rdbuf();

		if (tablefile.rdstate() == istream::failbit || tablefile.rdstate() == istream::badbit) {
			throw ConversionException("Error reading file " + tablename,"AlmaRadiometer");
		}

		// And close
		tablefile.close();
		if (tablefile.rdstate() == istream::failbit)
			throw ConversionException("Could not close file " + tablename,"AlmaRadiometer");
					
		// And parse the content with the appropriate method
		if (fileAsBin) 
			setFromMIME(ss.str());
		else
			fromXML(ss.str());	
	}			

	

	

			
	
	

	
	void AlmaRadiometerTable::autoIncrement(string key, AlmaRadiometerRow* x) {
		map<string, int>::iterator iter;
		if ((iter=noAutoIncIds.find(key)) == noAutoIncIds.end()) {
			// There is not yet a combination of the non autoinc attributes values in the hashtable
			
			// Initialize  almaRadiometerId to Tag(0).
			x->setAlmaRadiometerId(Tag(0,  TagType::AlmaRadiometer));
			
			// Record it in the map.		
			noAutoIncIds.insert(make_pair(key, 0));			
		} 
		else {
			// There is already a combination of the non autoinc attributes values in the hashtable
			// Increment its value.
			int n = iter->second + 1; 
			
			// Initialize  almaRadiometerId to Tag(n).
			x->setAlmaRadiometerId(Tag(n, TagType::AlmaRadiometer));
			
			// Record it in the map.		
			noAutoIncIds.insert(make_pair(key, n));				
		}		
	}
	
} // End namespace asdm
 
