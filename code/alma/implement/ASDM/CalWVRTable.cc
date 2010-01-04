
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
 * File CalWVRTable.cpp
 */
#include <ConversionException.h>
#include <DuplicateKey.h>
#include <OutOfBoundsException.h>

using asdm::ConversionException;
using asdm::DuplicateKey;
using asdm::OutOfBoundsException;

#include <ASDM.h>
#include <CalWVRTable.h>
#include <CalWVRRow.h>
#include <Parser.h>

using asdm::ASDM;
using asdm::CalWVRTable;
using asdm::CalWVRRow;
using asdm::Parser;

#include <iostream>
#include <sstream>
#include <set>
using namespace std;

#include <Misc.h>
using namespace asdm;


namespace asdm {

	string CalWVRTable::tableName = "CalWVR";
	

	/**
	 * The list of field names that make up key key.
	 * (Initialization is in the constructor.)
	 */
	vector<string> CalWVRTable::key;

	/**
	 * Return the list of field names that make up key key
	 * as an array of strings.
	 */	
	vector<string> CalWVRTable::getKeyName() {
		return key;
	}


	CalWVRTable::CalWVRTable(ASDM &c) : container(c) {

	
		key.push_back("antennaName");
	
		key.push_back("calDataId");
	
		key.push_back("calReductionId");
	


		// Define a default entity.
		entity.setEntityId(EntityId("uid://X0/X0/X0"));
		entity.setEntityIdEncrypted("na");
		entity.setEntityTypeName("CalWVRTable");
		entity.setEntityVersion("1");
		entity.setInstanceVersion("1");
		
		// Archive XML
		archiveAsBin = false;
		
		// File XML
		fileAsBin = false;
	}
	
/**
 * A destructor for CalWVRTable.
 */
 
	CalWVRTable::~CalWVRTable() {
		for (unsigned int i = 0; i < privateRows.size(); i++) 
			delete(privateRows.at(i));
	}

	/**
	 * Container to which this table belongs.
	 */
	ASDM &CalWVRTable::getContainer() const {
		return container;
	}

	/**
	 * Return the number of rows in the table.
	 */

	unsigned int CalWVRTable::size() {
		return row.size();
	}	
	
	
	/**
	 * Return the name of this table.
	 */
	string CalWVRTable::getName() const {
		return tableName;
	}

	/**
	 * Return this table's Entity.
	 */
	Entity CalWVRTable::getEntity() const {
		return entity;
	}

	/**
	 * Set this table's Entity.
	 */
	void CalWVRTable::setEntity(Entity e) {
		this->entity = e; 
	}
	
	//
	// ====> Row creation.
	//
	
	/**
	 * Create a new row.
	 */
	CalWVRRow *CalWVRTable::newRow() {
		return new CalWVRRow (*this);
	}
	
	CalWVRRow *CalWVRTable::newRowEmpty() {
		return newRow ();
	}


	/**
	 * Create a new row initialized to the specified values.
	 * @return a pointer on the created and initialized row.
	
 	 * @param antennaName. 
	
 	 * @param calDataId. 
	
 	 * @param calReductionId. 
	
 	 * @param startValidTime. 
	
 	 * @param endValidTime. 
	
 	 * @param wvrMethod. 
	
 	 * @param numInputAntennas. 
	
 	 * @param inputAntennaNames. 
	
 	 * @param numChan. 
	
 	 * @param chanFreq. 
	
 	 * @param chanWidth. 
	
 	 * @param refTemp. 
	
 	 * @param numPoly. 
	
 	 * @param pathCoeff. 
	
 	 * @param polyFreqLimits. 
	
 	 * @param wetPath. 
	
 	 * @param dryPath. 
	
 	 * @param water. 
	
     */
	CalWVRRow* CalWVRTable::newRow(string antennaName, Tag calDataId, Tag calReductionId, ArrayTime startValidTime, ArrayTime endValidTime, WVRMethodMod::WVRMethod wvrMethod, int numInputAntennas, vector<string > inputAntennaNames, int numChan, vector<Frequency > chanFreq, vector<Frequency > chanWidth, vector<vector<Temperature > > refTemp, int numPoly, vector<vector<vector<float > > > pathCoeff, vector<Frequency > polyFreqLimits, vector<float > wetPath, vector<float > dryPath, Length water){
		CalWVRRow *row = new CalWVRRow(*this);
			
		row->setAntennaName(antennaName);
			
		row->setCalDataId(calDataId);
			
		row->setCalReductionId(calReductionId);
			
		row->setStartValidTime(startValidTime);
			
		row->setEndValidTime(endValidTime);
			
		row->setWvrMethod(wvrMethod);
			
		row->setNumInputAntennas(numInputAntennas);
			
		row->setInputAntennaNames(inputAntennaNames);
			
		row->setNumChan(numChan);
			
		row->setChanFreq(chanFreq);
			
		row->setChanWidth(chanWidth);
			
		row->setRefTemp(refTemp);
			
		row->setNumPoly(numPoly);
			
		row->setPathCoeff(pathCoeff);
			
		row->setPolyFreqLimits(polyFreqLimits);
			
		row->setWetPath(wetPath);
			
		row->setDryPath(dryPath);
			
		row->setWater(water);
	
		return row;		
	}	

	CalWVRRow* CalWVRTable::newRowFull(string antennaName, Tag calDataId, Tag calReductionId, ArrayTime startValidTime, ArrayTime endValidTime, WVRMethodMod::WVRMethod wvrMethod, int numInputAntennas, vector<string > inputAntennaNames, int numChan, vector<Frequency > chanFreq, vector<Frequency > chanWidth, vector<vector<Temperature > > refTemp, int numPoly, vector<vector<vector<float > > > pathCoeff, vector<Frequency > polyFreqLimits, vector<float > wetPath, vector<float > dryPath, Length water)	{
		CalWVRRow *row = new CalWVRRow(*this);
			
		row->setAntennaName(antennaName);
			
		row->setCalDataId(calDataId);
			
		row->setCalReductionId(calReductionId);
			
		row->setStartValidTime(startValidTime);
			
		row->setEndValidTime(endValidTime);
			
		row->setWvrMethod(wvrMethod);
			
		row->setNumInputAntennas(numInputAntennas);
			
		row->setInputAntennaNames(inputAntennaNames);
			
		row->setNumChan(numChan);
			
		row->setChanFreq(chanFreq);
			
		row->setChanWidth(chanWidth);
			
		row->setRefTemp(refTemp);
			
		row->setNumPoly(numPoly);
			
		row->setPathCoeff(pathCoeff);
			
		row->setPolyFreqLimits(polyFreqLimits);
			
		row->setWetPath(wetPath);
			
		row->setDryPath(dryPath);
			
		row->setWater(water);
	
		return row;				
	}
	


CalWVRRow* CalWVRTable::newRow(CalWVRRow* row) {
	return new CalWVRRow(*this, *row);
}

CalWVRRow* CalWVRTable::newRowCopy(CalWVRRow* row) {
	return new CalWVRRow(*this, *row);
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
	CalWVRRow* CalWVRTable::add(CalWVRRow* x) {
		
		if (getRowByKey(
						x->getAntennaName()
						,
						x->getCalDataId()
						,
						x->getCalReductionId()
						))
			//throw DuplicateKey(x.getAntennaName() + "|" + x.getCalDataId() + "|" + x.getCalReductionId(),"CalWVR");
			throw DuplicateKey("Duplicate key exception in ","CalWVRTable");
		
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
	CalWVRRow*  CalWVRTable::checkAndAdd(CalWVRRow* x)  {
		
		
		if (getRowByKey(
	
			x->getAntennaName()
	,
			x->getCalDataId()
	,
			x->getCalReductionId()
			
		)) throw DuplicateKey("Duplicate key exception in ", "CalWVRTable");
		
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
	 * @return Alls rows as an array of CalWVRRow
	 */
	vector<CalWVRRow *> CalWVRTable::get() {
		return privateRows;
		// return row;
	}

	
/*
 ** Returns a CalWVRRow* given a key.
 ** @return a pointer to the row having the key whose values are passed as parameters, or 0 if
 ** no row exists for that key.
 **
 */
 	CalWVRRow* CalWVRTable::getRowByKey(string antennaName, Tag calDataId, Tag calReductionId)  {
	CalWVRRow* aRow = 0;
	for (unsigned int i = 0; i < row.size(); i++) {
		aRow = row.at(i);
		
			
				if (aRow->antennaName != antennaName) continue;
			
		
			
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
 	 		
 * @param calDataId.
 	 		
 * @param calReductionId.
 	 		
 * @param startValidTime.
 	 		
 * @param endValidTime.
 	 		
 * @param wvrMethod.
 	 		
 * @param numInputAntennas.
 	 		
 * @param inputAntennaNames.
 	 		
 * @param numChan.
 	 		
 * @param chanFreq.
 	 		
 * @param chanWidth.
 	 		
 * @param refTemp.
 	 		
 * @param numPoly.
 	 		
 * @param pathCoeff.
 	 		
 * @param polyFreqLimits.
 	 		
 * @param wetPath.
 	 		
 * @param dryPath.
 	 		
 * @param water.
 	 		 
 */
CalWVRRow* CalWVRTable::lookup(string antennaName, Tag calDataId, Tag calReductionId, ArrayTime startValidTime, ArrayTime endValidTime, WVRMethodMod::WVRMethod wvrMethod, int numInputAntennas, vector<string > inputAntennaNames, int numChan, vector<Frequency > chanFreq, vector<Frequency > chanWidth, vector<vector<Temperature > > refTemp, int numPoly, vector<vector<vector<float > > > pathCoeff, vector<Frequency > polyFreqLimits, vector<float > wetPath, vector<float > dryPath, Length water) {
		CalWVRRow* aRow;
		for (unsigned int i = 0; i < size(); i++) {
			aRow = row.at(i); 
			if (aRow->compareNoAutoInc(antennaName, calDataId, calReductionId, startValidTime, endValidTime, wvrMethod, numInputAntennas, inputAntennaNames, numChan, chanFreq, chanWidth, refTemp, numPoly, pathCoeff, polyFreqLimits, wetPath, dryPath, water)) return aRow;
		}			
		return 0;	
} 
	
 	 	

	




#ifndef WITHOUT_ACS
	// Conversion Methods

	CalWVRTableIDL *CalWVRTable::toIDL() {
		CalWVRTableIDL *x = new CalWVRTableIDL ();
		unsigned int nrow = size();
		x->row.length(nrow);
		vector<CalWVRRow*> v = get();
		for (unsigned int i = 0; i < nrow; ++i) {
			x->row[i] = *(v[i]->toIDL());
		}
		return x;
	}
#endif
	
#ifndef WITHOUT_ACS
	void CalWVRTable::fromIDL(CalWVRTableIDL x) {
		unsigned int nrow = x.row.length();
		for (unsigned int i = 0; i < nrow; ++i) {
			CalWVRRow *tmp = newRow();
			tmp->setFromIDL(x.row[i]);
			// checkAndAdd(tmp);
			add(tmp);
		}
	}
#endif

	char *CalWVRTable::toFITS() const  {
		throw ConversionException("Not implemented","CalWVR");
	}

	void CalWVRTable::fromFITS(char *fits)  {
		throw ConversionException("Not implemented","CalWVR");
	}

	string CalWVRTable::toVOTable() const {
		throw ConversionException("Not implemented","CalWVR");
	}

	void CalWVRTable::fromVOTable(string vo) {
		throw ConversionException("Not implemented","CalWVR");
	}

	
	string CalWVRTable::toXML()  {
		string buf;
		buf.append("<?xml version=\"1.0\" encoding=\"ISO-8859-1\"?> ");
		buf.append("<CalWVRTable xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" xmlns=\"http://Alma/XASDM/CalWVRTable\" xsi:schemaLocation=\"http://Alma/XASDM/CalWVRTable http://almaobservatory.org/XML/XASDM/2/CalWVRTable.xsd\"> ");	
		buf.append(entity.toXML());
		string s = container.getEntity().toXML();
		// Change the "Entity" tag to "ContainerEntity".
		buf.append("<Container" + s.substr(1,s.length() - 1)+" ");
		vector<CalWVRRow*> v = get();
		for (unsigned int i = 0; i < v.size(); ++i) {
			try {
				buf.append(v[i]->toXML());
			} catch (NoSuchRow e) {
			}
			buf.append("  ");
		}		
		buf.append("</CalWVRTable> ");
		return buf;
	}

	
	void CalWVRTable::fromXML(string xmlDoc)  {
		Parser xml(xmlDoc);
		if (!xml.isStr("<CalWVRTable")) 
			error();
		// cout << "Parsing a CalWVRTable" << endl;
		string s = xml.getElement("<Entity","/>");
		if (s.length() == 0) 
			error();
		Entity e;
		e.setFromXML(s);
		if (e.getEntityTypeName() != "CalWVRTable")
			error();
		setEntity(e);
		// Skip the container's entity; but, it has to be there.
		s = xml.getElement("<ContainerEntity","/>");
		if (s.length() == 0) 
			error();

		// Get each row in the table.
		s = xml.getElementContent("<row>","</row>");
		CalWVRRow *row;
		while (s.length() != 0) {
			// cout << "Parsing a CalWVRRow" << endl; 
			row = newRow();
			row->setFromXML(s);
			try {
				checkAndAdd(row);
			} catch (DuplicateKey e1) {
				throw ConversionException(e1.getMessage(),"CalWVRTable");
			} 
			catch (UniquenessViolationException e1) {
				throw ConversionException(e1.getMessage(),"CalWVRTable");	
			}
			catch (...) {
				// cout << "Unexpected error in CalWVRTable::checkAndAdd called from CalWVRTable::fromXML " << endl;
			}
			s = xml.getElementContent("<row>","</row>");
		}
		if (!xml.isStr("</CalWVRTable>")) 
			error();
	}

	
	void CalWVRTable::error()  {
		throw ConversionException("Invalid xml document","CalWVR");
	}
	
	
	string CalWVRTable::toMIME() {
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

	
	void CalWVRTable::setFromMIME(const string & mimeMsg) {
		// cout << "Entering setFromMIME" << endl;
	 	string terminator = "Content-Type: binary/octet-stream\nContent-ID: <content.bin>\n\n";
	 	
	 	// Look for the string announcing the binary part.
	 	string::size_type loc = mimeMsg.find( terminator, 0 );
	 	
	 	if ( loc == string::npos ) {
	 		throw ConversionException("Failed to detect the beginning of the binary part", "CalWVR");
	 	}
	
	 	// Create an EndianISStream from the substring containing the binary part.
	 	EndianISStream eiss(mimeMsg.substr(loc+terminator.size()));
	 	
	 	entity = Entity::fromBin(eiss);
	 	
	 	// We do nothing with that but we have to read it.
	 	Entity containerEntity = Entity::fromBin(eiss);
	 		 	
	 	int numRows = eiss.readInt();
	 	try {
	 		for (int i = 0; i < numRows; i++) {
	 			CalWVRRow* aRow = CalWVRRow::fromBin(eiss, *this);
	 			checkAndAdd(aRow);
	 		}
	 	}
	 	catch (DuplicateKey e) {
	 		throw ConversionException("Error while writing binary data , the message was "
	 					+ e.getMessage(), "CalWVR");
	 	}
		catch (TagFormatException e) {
			throw ConversionException("Error while reading binary data , the message was "
					+ e.getMessage(), "CalWVR");
		} 		 	
	}

	
	void CalWVRTable::toFile(string directory) {
		if (!directoryExists(directory.c_str()) &&
			!createPath(directory.c_str())) {
			throw ConversionException("Could not create directory " , directory);
		}
		
		if (fileAsBin) {
			// write the bin serialized
			string fileName = directory + "/CalWVR.bin";
			ofstream tableout(fileName.c_str(),ios::out|ios::trunc);
			if (tableout.rdstate() == ostream::failbit)
				throw ConversionException("Could not open file " + fileName + " to write ", "CalWVR");
			tableout << toMIME() << endl;
			tableout.close();
			if (tableout.rdstate() == ostream::failbit)
				throw ConversionException("Could not close file " + fileName, "CalWVR");
		}
		else {
			// write the XML
			string fileName = directory + "/CalWVR.xml";
			ofstream tableout(fileName.c_str(),ios::out|ios::trunc);
			if (tableout.rdstate() == ostream::failbit)
				throw ConversionException("Could not open file " + fileName + " to write ", "CalWVR");
			tableout << toXML() << endl;
			tableout.close();
			if (tableout.rdstate() == ostream::failbit)
				throw ConversionException("Could not close file " + fileName, "CalWVR");
		}
	}

	
	void CalWVRTable::setFromFile(const string& directory) {
		string tablename;
		if (fileAsBin)
			tablename = directory + "/CalWVR.bin";
		else
			tablename = directory + "/CalWVR.xml";
			
		// Determine the file size.
		ifstream::pos_type size;
		ifstream tablefile(tablename.c_str(), ios::in|ios::binary|ios::ate);

 		if (tablefile.is_open()) { 
  				size = tablefile.tellg(); 
  		}
		else {
				throw ConversionException("Could not open file " + tablename, "CalWVR");
		}
		
		// Re position to the beginning.
		tablefile.seekg(0);
		
		// Read in a stringstream.
		stringstream ss;
		ss << tablefile.rdbuf();

		if (tablefile.rdstate() == istream::failbit || tablefile.rdstate() == istream::badbit) {
			throw ConversionException("Error reading file " + tablename,"CalWVR");
		}

		// And close
		tablefile.close();
		if (tablefile.rdstate() == istream::failbit)
			throw ConversionException("Could not close file " + tablename,"CalWVR");
					
		// And parse the content with the appropriate method
		if (fileAsBin) 
			setFromMIME(ss.str());
		else
			fromXML(ss.str());	
	}			

	

	

			
	
	

	
} // End namespace asdm
 
