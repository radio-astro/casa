
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
 * File CalBandpassTable.cpp
 */
#include <ConversionException.h>
#include <DuplicateKey.h>
#include <OutOfBoundsException.h>

using asdm::ConversionException;
using asdm::DuplicateKey;
using asdm::OutOfBoundsException;

#include <ASDM.h>
#include <CalBandpassTable.h>
#include <CalBandpassRow.h>
#include <Parser.h>

using asdm::ASDM;
using asdm::CalBandpassTable;
using asdm::CalBandpassRow;
using asdm::Parser;

#include <iostream>
#include <sstream>
#include <set>
using namespace std;

#include <Misc.h>
using namespace asdm;


namespace asdm {

	string CalBandpassTable::tableName = "CalBandpass";
	

	/**
	 * The list of field names that make up key key.
	 * (Initialization is in the constructor.)
	 */
	vector<string> CalBandpassTable::key;

	/**
	 * Return the list of field names that make up key key
	 * as an array of strings.
	 */	
	vector<string> CalBandpassTable::getKeyName() {
		return key;
	}


	CalBandpassTable::CalBandpassTable(ASDM &c) : container(c) {

	
		key.push_back("basebandName");
	
		key.push_back("sideband");
	
		key.push_back("atmPhaseCorrection");
	
		key.push_back("typeCurve");
	
		key.push_back("receiverBand");
	
		key.push_back("calDataId");
	
		key.push_back("calReductionId");
	


		// Define a default entity.
		entity.setEntityId(EntityId("uid://X0/X0/X0"));
		entity.setEntityIdEncrypted("na");
		entity.setEntityTypeName("CalBandpassTable");
		entity.setEntityVersion("1");
		entity.setInstanceVersion("1");
		
		// Archive XML
		archiveAsBin = false;
		
		// File XML
		fileAsBin = false;
	}
	
/**
 * A destructor for CalBandpassTable.
 */
 
	CalBandpassTable::~CalBandpassTable() {
		for (unsigned int i = 0; i < privateRows.size(); i++) 
			delete(privateRows.at(i));
	}

	/**
	 * Container to which this table belongs.
	 */
	ASDM &CalBandpassTable::getContainer() const {
		return container;
	}

	/**
	 * Return the number of rows in the table.
	 */

	unsigned int CalBandpassTable::size() {
		return row.size();
	}	
	
	
	/**
	 * Return the name of this table.
	 */
	string CalBandpassTable::getName() const {
		return tableName;
	}

	/**
	 * Return this table's Entity.
	 */
	Entity CalBandpassTable::getEntity() const {
		return entity;
	}

	/**
	 * Set this table's Entity.
	 */
	void CalBandpassTable::setEntity(Entity e) {
		this->entity = e; 
	}
	
	//
	// ====> Row creation.
	//
	
	/**
	 * Create a new row.
	 */
	CalBandpassRow *CalBandpassTable::newRow() {
		return new CalBandpassRow (*this);
	}
	
	CalBandpassRow *CalBandpassTable::newRowEmpty() {
		return newRow ();
	}


	/**
	 * Create a new row initialized to the specified values.
	 * @return a pointer on the created and initialized row.
	
 	 * @param basebandName. 
	
 	 * @param sideband. 
	
 	 * @param atmPhaseCorrection. 
	
 	 * @param typeCurve. 
	
 	 * @param receiverBand. 
	
 	 * @param calDataId. 
	
 	 * @param calReductionId. 
	
 	 * @param startValidTime. 
	
 	 * @param endValidTime. 
	
 	 * @param numAntenna. 
	
 	 * @param numPoly. 
	
 	 * @param numReceptor. 
	
 	 * @param antennaNames. 
	
 	 * @param refAntennaName. 
	
 	 * @param freqLimits. 
	
 	 * @param polarizationTypes. 
	
 	 * @param curve. 
	
 	 * @param reducedChiSquared. 
	
     */
	CalBandpassRow* CalBandpassTable::newRow(BasebandNameMod::BasebandName basebandName, NetSidebandMod::NetSideband sideband, AtmPhaseCorrectionMod::AtmPhaseCorrection atmPhaseCorrection, CalCurveTypeMod::CalCurveType typeCurve, ReceiverBandMod::ReceiverBand receiverBand, Tag calDataId, Tag calReductionId, ArrayTime startValidTime, ArrayTime endValidTime, int numAntenna, int numPoly, int numReceptor, vector<string > antennaNames, string refAntennaName, vector<Frequency > freqLimits, vector<PolarizationTypeMod::PolarizationType > polarizationTypes, vector<vector<vector<float > > > curve, vector<double > reducedChiSquared){
		CalBandpassRow *row = new CalBandpassRow(*this);
			
		row->setBasebandName(basebandName);
			
		row->setSideband(sideband);
			
		row->setAtmPhaseCorrection(atmPhaseCorrection);
			
		row->setTypeCurve(typeCurve);
			
		row->setReceiverBand(receiverBand);
			
		row->setCalDataId(calDataId);
			
		row->setCalReductionId(calReductionId);
			
		row->setStartValidTime(startValidTime);
			
		row->setEndValidTime(endValidTime);
			
		row->setNumAntenna(numAntenna);
			
		row->setNumPoly(numPoly);
			
		row->setNumReceptor(numReceptor);
			
		row->setAntennaNames(antennaNames);
			
		row->setRefAntennaName(refAntennaName);
			
		row->setFreqLimits(freqLimits);
			
		row->setPolarizationTypes(polarizationTypes);
			
		row->setCurve(curve);
			
		row->setReducedChiSquared(reducedChiSquared);
	
		return row;		
	}	

	CalBandpassRow* CalBandpassTable::newRowFull(BasebandNameMod::BasebandName basebandName, NetSidebandMod::NetSideband sideband, AtmPhaseCorrectionMod::AtmPhaseCorrection atmPhaseCorrection, CalCurveTypeMod::CalCurveType typeCurve, ReceiverBandMod::ReceiverBand receiverBand, Tag calDataId, Tag calReductionId, ArrayTime startValidTime, ArrayTime endValidTime, int numAntenna, int numPoly, int numReceptor, vector<string > antennaNames, string refAntennaName, vector<Frequency > freqLimits, vector<PolarizationTypeMod::PolarizationType > polarizationTypes, vector<vector<vector<float > > > curve, vector<double > reducedChiSquared)	{
		CalBandpassRow *row = new CalBandpassRow(*this);
			
		row->setBasebandName(basebandName);
			
		row->setSideband(sideband);
			
		row->setAtmPhaseCorrection(atmPhaseCorrection);
			
		row->setTypeCurve(typeCurve);
			
		row->setReceiverBand(receiverBand);
			
		row->setCalDataId(calDataId);
			
		row->setCalReductionId(calReductionId);
			
		row->setStartValidTime(startValidTime);
			
		row->setEndValidTime(endValidTime);
			
		row->setNumAntenna(numAntenna);
			
		row->setNumPoly(numPoly);
			
		row->setNumReceptor(numReceptor);
			
		row->setAntennaNames(antennaNames);
			
		row->setRefAntennaName(refAntennaName);
			
		row->setFreqLimits(freqLimits);
			
		row->setPolarizationTypes(polarizationTypes);
			
		row->setCurve(curve);
			
		row->setReducedChiSquared(reducedChiSquared);
	
		return row;				
	}
	


CalBandpassRow* CalBandpassTable::newRow(CalBandpassRow* row) {
	return new CalBandpassRow(*this, *row);
}

CalBandpassRow* CalBandpassTable::newRowCopy(CalBandpassRow* row) {
	return new CalBandpassRow(*this, *row);
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
	CalBandpassRow* CalBandpassTable::add(CalBandpassRow* x) {
		
		if (getRowByKey(
						x->getBasebandName()
						,
						x->getSideband()
						,
						x->getAtmPhaseCorrection()
						,
						x->getTypeCurve()
						,
						x->getReceiverBand()
						,
						x->getCalDataId()
						,
						x->getCalReductionId()
						))
			//throw DuplicateKey(x.getBasebandName() + "|" + x.getSideband() + "|" + x.getAtmPhaseCorrection() + "|" + x.getTypeCurve() + "|" + x.getReceiverBand() + "|" + x.getCalDataId() + "|" + x.getCalReductionId(),"CalBandpass");
			throw DuplicateKey("Duplicate key exception in ","CalBandpassTable");
		
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
	CalBandpassRow*  CalBandpassTable::checkAndAdd(CalBandpassRow* x)  {
		
		
		if (getRowByKey(
	
			x->getBasebandName()
	,
			x->getSideband()
	,
			x->getAtmPhaseCorrection()
	,
			x->getTypeCurve()
	,
			x->getReceiverBand()
	,
			x->getCalDataId()
	,
			x->getCalReductionId()
			
		)) throw DuplicateKey("Duplicate key exception in ", "CalBandpassTable");
		
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
	 * @return Alls rows as an array of CalBandpassRow
	 */
	vector<CalBandpassRow *> CalBandpassTable::get() {
		return privateRows;
		// return row;
	}

	
/*
 ** Returns a CalBandpassRow* given a key.
 ** @return a pointer to the row having the key whose values are passed as parameters, or 0 if
 ** no row exists for that key.
 **
 */
 	CalBandpassRow* CalBandpassTable::getRowByKey(BasebandNameMod::BasebandName basebandName, NetSidebandMod::NetSideband sideband, AtmPhaseCorrectionMod::AtmPhaseCorrection atmPhaseCorrection, CalCurveTypeMod::CalCurveType typeCurve, ReceiverBandMod::ReceiverBand receiverBand, Tag calDataId, Tag calReductionId)  {
	CalBandpassRow* aRow = 0;
	for (unsigned int i = 0; i < row.size(); i++) {
		aRow = row.at(i);
		
			
				if (aRow->basebandName != basebandName) continue;
			
		
			
				if (aRow->sideband != sideband) continue;
			
		
			
				if (aRow->atmPhaseCorrection != atmPhaseCorrection) continue;
			
		
			
				if (aRow->typeCurve != typeCurve) continue;
			
		
			
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
			
 * @param basebandName.
 	 		
 * @param sideband.
 	 		
 * @param atmPhaseCorrection.
 	 		
 * @param typeCurve.
 	 		
 * @param receiverBand.
 	 		
 * @param calDataId.
 	 		
 * @param calReductionId.
 	 		
 * @param startValidTime.
 	 		
 * @param endValidTime.
 	 		
 * @param numAntenna.
 	 		
 * @param numPoly.
 	 		
 * @param numReceptor.
 	 		
 * @param antennaNames.
 	 		
 * @param refAntennaName.
 	 		
 * @param freqLimits.
 	 		
 * @param polarizationTypes.
 	 		
 * @param curve.
 	 		
 * @param reducedChiSquared.
 	 		 
 */
CalBandpassRow* CalBandpassTable::lookup(BasebandNameMod::BasebandName basebandName, NetSidebandMod::NetSideband sideband, AtmPhaseCorrectionMod::AtmPhaseCorrection atmPhaseCorrection, CalCurveTypeMod::CalCurveType typeCurve, ReceiverBandMod::ReceiverBand receiverBand, Tag calDataId, Tag calReductionId, ArrayTime startValidTime, ArrayTime endValidTime, int numAntenna, int numPoly, int numReceptor, vector<string > antennaNames, string refAntennaName, vector<Frequency > freqLimits, vector<PolarizationTypeMod::PolarizationType > polarizationTypes, vector<vector<vector<float > > > curve, vector<double > reducedChiSquared) {
		CalBandpassRow* aRow;
		for (unsigned int i = 0; i < size(); i++) {
			aRow = row.at(i); 
			if (aRow->compareNoAutoInc(basebandName, sideband, atmPhaseCorrection, typeCurve, receiverBand, calDataId, calReductionId, startValidTime, endValidTime, numAntenna, numPoly, numReceptor, antennaNames, refAntennaName, freqLimits, polarizationTypes, curve, reducedChiSquared)) return aRow;
		}			
		return 0;	
} 
	
 	 	

	




#ifndef WITHOUT_ACS
	// Conversion Methods

	CalBandpassTableIDL *CalBandpassTable::toIDL() {
		CalBandpassTableIDL *x = new CalBandpassTableIDL ();
		unsigned int nrow = size();
		x->row.length(nrow);
		vector<CalBandpassRow*> v = get();
		for (unsigned int i = 0; i < nrow; ++i) {
			x->row[i] = *(v[i]->toIDL());
		}
		return x;
	}
#endif
	
#ifndef WITHOUT_ACS
	void CalBandpassTable::fromIDL(CalBandpassTableIDL x) {
		unsigned int nrow = x.row.length();
		for (unsigned int i = 0; i < nrow; ++i) {
			CalBandpassRow *tmp = newRow();
			tmp->setFromIDL(x.row[i]);
			// checkAndAdd(tmp);
			add(tmp);
		}
	}
#endif

	char *CalBandpassTable::toFITS() const  {
		throw ConversionException("Not implemented","CalBandpass");
	}

	void CalBandpassTable::fromFITS(char *fits)  {
		throw ConversionException("Not implemented","CalBandpass");
	}

	string CalBandpassTable::toVOTable() const {
		throw ConversionException("Not implemented","CalBandpass");
	}

	void CalBandpassTable::fromVOTable(string vo) {
		throw ConversionException("Not implemented","CalBandpass");
	}

	
	string CalBandpassTable::toXML()  {
		string buf;
		buf.append("<?xml version=\"1.0\" encoding=\"ISO-8859-1\"?> ");
		buf.append("<CalBandpassTable xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" xmlns=\"http://Alma/XASDM/CalBandpassTable\" xsi:schemaLocation=\"http://Alma/XASDM/CalBandpassTable http://almaobservatory.org/XML/XASDM/2/CalBandpassTable.xsd\"> ");	
		buf.append(entity.toXML());
		string s = container.getEntity().toXML();
		// Change the "Entity" tag to "ContainerEntity".
		buf.append("<Container" + s.substr(1,s.length() - 1)+" ");
		vector<CalBandpassRow*> v = get();
		for (unsigned int i = 0; i < v.size(); ++i) {
			try {
				buf.append(v[i]->toXML());
			} catch (NoSuchRow e) {
			}
			buf.append("  ");
		}		
		buf.append("</CalBandpassTable> ");
		return buf;
	}

	
	void CalBandpassTable::fromXML(string xmlDoc)  {
		Parser xml(xmlDoc);
		if (!xml.isStr("<CalBandpassTable")) 
			error();
		// cout << "Parsing a CalBandpassTable" << endl;
		string s = xml.getElement("<Entity","/>");
		if (s.length() == 0) 
			error();
		Entity e;
		e.setFromXML(s);
		if (e.getEntityTypeName() != "CalBandpassTable")
			error();
		setEntity(e);
		// Skip the container's entity; but, it has to be there.
		s = xml.getElement("<ContainerEntity","/>");
		if (s.length() == 0) 
			error();

		// Get each row in the table.
		s = xml.getElementContent("<row>","</row>");
		CalBandpassRow *row;
		while (s.length() != 0) {
			// cout << "Parsing a CalBandpassRow" << endl; 
			row = newRow();
			row->setFromXML(s);
			try {
				checkAndAdd(row);
			} catch (DuplicateKey e1) {
				throw ConversionException(e1.getMessage(),"CalBandpassTable");
			} 
			catch (UniquenessViolationException e1) {
				throw ConversionException(e1.getMessage(),"CalBandpassTable");	
			}
			catch (...) {
				// cout << "Unexpected error in CalBandpassTable::checkAndAdd called from CalBandpassTable::fromXML " << endl;
			}
			s = xml.getElementContent("<row>","</row>");
		}
		if (!xml.isStr("</CalBandpassTable>")) 
			error();
	}

	
	void CalBandpassTable::error()  {
		throw ConversionException("Invalid xml document","CalBandpass");
	}
	
	
	string CalBandpassTable::toMIME() {
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

	
	void CalBandpassTable::setFromMIME(const string & mimeMsg) {
		// cout << "Entering setFromMIME" << endl;
	 	string terminator = "Content-Type: binary/octet-stream\nContent-ID: <content.bin>\n\n";
	 	
	 	// Look for the string announcing the binary part.
	 	string::size_type loc = mimeMsg.find( terminator, 0 );
	 	
	 	if ( loc == string::npos ) {
	 		throw ConversionException("Failed to detect the beginning of the binary part", "CalBandpass");
	 	}
	
	 	// Create an EndianISStream from the substring containing the binary part.
	 	EndianISStream eiss(mimeMsg.substr(loc+terminator.size()));
	 	
	 	entity = Entity::fromBin(eiss);
	 	
	 	// We do nothing with that but we have to read it.
	 	Entity containerEntity = Entity::fromBin(eiss);
	 		 	
	 	int numRows = eiss.readInt();
	 	try {
	 		for (int i = 0; i < numRows; i++) {
	 			CalBandpassRow* aRow = CalBandpassRow::fromBin(eiss, *this);
	 			checkAndAdd(aRow);
	 		}
	 	}
	 	catch (DuplicateKey e) {
	 		throw ConversionException("Error while writing binary data , the message was "
	 					+ e.getMessage(), "CalBandpass");
	 	}
		catch (TagFormatException e) {
			throw ConversionException("Error while reading binary data , the message was "
					+ e.getMessage(), "CalBandpass");
		} 		 	
	}

	
	void CalBandpassTable::toFile(string directory) {
		if (!directoryExists(directory.c_str()) &&
			!createPath(directory.c_str())) {
			throw ConversionException("Could not create directory " , directory);
		}
		
		if (fileAsBin) {
			// write the bin serialized
			string fileName = directory + "/CalBandpass.bin";
			ofstream tableout(fileName.c_str(),ios::out|ios::trunc);
			if (tableout.rdstate() == ostream::failbit)
				throw ConversionException("Could not open file " + fileName + " to write ", "CalBandpass");
			tableout << toMIME() << endl;
			tableout.close();
			if (tableout.rdstate() == ostream::failbit)
				throw ConversionException("Could not close file " + fileName, "CalBandpass");
		}
		else {
			// write the XML
			string fileName = directory + "/CalBandpass.xml";
			ofstream tableout(fileName.c_str(),ios::out|ios::trunc);
			if (tableout.rdstate() == ostream::failbit)
				throw ConversionException("Could not open file " + fileName + " to write ", "CalBandpass");
			tableout << toXML() << endl;
			tableout.close();
			if (tableout.rdstate() == ostream::failbit)
				throw ConversionException("Could not close file " + fileName, "CalBandpass");
		}
	}

	
	void CalBandpassTable::setFromFile(const string& directory) {
		string tablename;
		if (fileAsBin)
			tablename = directory + "/CalBandpass.bin";
		else
			tablename = directory + "/CalBandpass.xml";
			
		// Determine the file size.
		ifstream::pos_type size;
		ifstream tablefile(tablename.c_str(), ios::in|ios::binary|ios::ate);

 		if (tablefile.is_open()) { 
  				size = tablefile.tellg(); 
  		}
		else {
				throw ConversionException("Could not open file " + tablename, "CalBandpass");
		}
		
		// Re position to the beginning.
		tablefile.seekg(0);
		
		// Read in a stringstream.
		stringstream ss;
		ss << tablefile.rdbuf();

		if (tablefile.rdstate() == istream::failbit || tablefile.rdstate() == istream::badbit) {
			throw ConversionException("Error reading file " + tablename,"CalBandpass");
		}

		// And close
		tablefile.close();
		if (tablefile.rdstate() == istream::failbit)
			throw ConversionException("Could not close file " + tablename,"CalBandpass");
					
		// And parse the content with the appropriate method
		if (fileAsBin) 
			setFromMIME(ss.str());
		else
			fromXML(ss.str());	
	}			

	

	

			
	
	

	
} // End namespace asdm
 
