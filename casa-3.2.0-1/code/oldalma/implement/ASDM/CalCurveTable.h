
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
 * File CalCurveTable.h
 */
 
#ifndef CalCurveTable_CLASS
#define CalCurveTable_CLASS

#include <string>
#include <vector>
#include <map>
#include <set>
using std::string;
using std::vector;
using std::map;

#include <Angle.h>
#include <AngularRate.h>
#include <ArrayTime.h>
#include <ArrayTimeInterval.h>
#include <Complex.h>
#include <Entity.h>
#include <EntityId.h>
#include <EntityRef.h>
#include <Flux.h>
#include <Frequency.h>
#include <Humidity.h>
#include <Interval.h>
#include <Length.h>
#include <PartId.h>
#include <Pressure.h>
#include <Speed.h>
#include <Tag.h>
#include <Temperature.h>
#include <ConversionException.h>
#include <DuplicateKey.h>
#include <UniquenessViolationException.h>
#include <NoSuchRow.h>
#include <DuplicateKey.h>

/*
#include <Enumerations.h>
using namespace enumerations;
*/




	

	

	

	

	

	

	

	
#include "CReceiverBand.h"
using namespace ReceiverBandMod;
	

	
#include "CAtmPhaseCorrection.h"
using namespace AtmPhaseCorrectionMod;
	

	
#include "CPolarizationType.h"
using namespace PolarizationTypeMod;
	

	

	

	

	
#include "CCalCurveType.h"
using namespace CalCurveTypeMod;
	

	

	

	


#ifndef WITHOUT_ACS
#include <asdmIDLC.h>
using asdmIDL::CalCurveTableIDL;
#endif

using asdm::Angle;
using asdm::AngularRate;
using asdm::ArrayTime;
using asdm::Complex;
using asdm::Entity;
using asdm::EntityId;
using asdm::EntityRef;
using asdm::Flux;
using asdm::Frequency;
using asdm::Humidity;
using asdm::Interval;
using asdm::Length;
using asdm::PartId;
using asdm::Pressure;
using asdm::Speed;
using asdm::Tag;
using asdm::Temperature;

using asdm::DuplicateKey;
using asdm::ConversionException;
using asdm::NoSuchRow;
using asdm::DuplicateKey;

#include <Representable.h>

namespace asdm {

//class asdm::ASDM;
//class asdm::CalCurveRow;

class ASDM;
class CalCurveRow;
/**
 * The CalCurveTable class is an Alma table.
 * 
 * Generated from model's revision "1.46", branch "HEAD"
 *
 * <TABLE BORDER="1">
 * <CAPTION> Attributes of CalCurve </CAPTION>
 * <TR BGCOLOR="#AAAAAA"> <TH> Name </TH> <TH> Type </TH> <TH> Comment </TH></TR>
 
 * <TR> <TH BGCOLOR="#CCCCCC" colspan="3" align="center"> Key </TD></TR>
	
 		
 * <TR>
 * <TD> calDataId </TD> 
 * <TD> Tag </TD>
 * <TD> &nbsp; </TD>
 * </TR>
 		
	
 		
 * <TR>
 * <TD> calReductionId </TD> 
 * <TD> Tag </TD>
 * <TD> &nbsp; </TD>
 * </TR>
 		
	


 * <TR> <TH BGCOLOR="#CCCCCC"  colspan="3" valign="center"> Value <br> (Mandarory) </TH></TR>
	
 * <TR>
 * <TD> numAntenna </TD> 
 * <TD> int </TD>
 * <TD>  &nbsp;  </TD> 
 * </TR>
	
 * <TR>
 * <TD> numBaseline </TD> 
 * <TD> int </TD>
 * <TD>  &nbsp;  </TD> 
 * </TR>
	
 * <TR>
 * <TD> numAPC </TD> 
 * <TD> int </TD>
 * <TD>  &nbsp;  </TD> 
 * </TR>
	
 * <TR>
 * <TD> numReceptor </TD> 
 * <TD> int </TD>
 * <TD>  &nbsp;  </TD> 
 * </TR>
	
 * <TR>
 * <TD> numPoly </TD> 
 * <TD> int </TD>
 * <TD>  &nbsp;  </TD> 
 * </TR>
	
 * <TR>
 * <TD> antennaNames </TD> 
 * <TD> vector<string > </TD>
 * <TD>  numAntenna </TD> 
 * </TR>
	
 * <TR>
 * <TD> refAntennaName </TD> 
 * <TD> string </TD>
 * <TD>  &nbsp;  </TD> 
 * </TR>
	
 * <TR>
 * <TD> receiverBand </TD> 
 * <TD> ReceiverBandMod::ReceiverBand </TD>
 * <TD>  &nbsp;  </TD> 
 * </TR>
	
 * <TR>
 * <TD> atmPhaseCorrections </TD> 
 * <TD> vector<AtmPhaseCorrectionMod::AtmPhaseCorrection > </TD>
 * <TD>  numAPC </TD> 
 * </TR>
	
 * <TR>
 * <TD> polarizationTypes </TD> 
 * <TD> vector<PolarizationTypeMod::PolarizationType > </TD>
 * <TD>  numReceptor </TD> 
 * </TR>
	
 * <TR>
 * <TD> startValidTime </TD> 
 * <TD> ArrayTime </TD>
 * <TD>  &nbsp;  </TD> 
 * </TR>
	
 * <TR>
 * <TD> endValidTime </TD> 
 * <TD> ArrayTime </TD>
 * <TD>  &nbsp;  </TD> 
 * </TR>
	
 * <TR>
 * <TD> frequencyRange </TD> 
 * <TD> vector<Frequency > </TD>
 * <TD>  2 </TD> 
 * </TR>
	
 * <TR>
 * <TD> typeCurve </TD> 
 * <TD> CalCurveTypeMod::CalCurveType </TD>
 * <TD>  &nbsp;  </TD> 
 * </TR>
	
 * <TR>
 * <TD> timeOrigin </TD> 
 * <TD> ArrayTime </TD>
 * <TD>  &nbsp;  </TD> 
 * </TR>
	
 * <TR>
 * <TD> curve </TD> 
 * <TD> vector<vector<vector<vector<float > > > > </TD>
 * <TD>  numAntenna, numAPC, numReceptor, numPoly </TD> 
 * </TR>
	
 * <TR>
 * <TD> rms </TD> 
 * <TD> vector<vector<vector<float > > > </TD>
 * <TD>  numBaseline, numAPC, numReceptor </TD> 
 * </TR>
	


 * </TABLE>
 */
class CalCurveTable : public Representable {
	friend class asdm::ASDM;

public:


	/**
	 * Return the list of field names that make up key key
	 * as an array of strings.
	 * @return a vector of string.
	 */	
	static vector<string> getKeyName();


	virtual ~CalCurveTable();
	
	/**
	 * Return the container to which this table belongs.
	 *
	 * @return the ASDM containing this table.
	 */
	ASDM &getContainer() const;
	
	/**
	 * Return the number of rows in the table.
	 *
	 * @return the number of rows in an unsigned int.
	 */
	unsigned int size() ;
	
	/**
	 * Return the name of this table.
	 *
	 * @return the name of this table in a string.
	 */
	string getName() const;

	/**
	 * Return this table's Entity.
	 */
	Entity getEntity() const;

	/**
	 * Set this table's Entity.
	 * @param e An entity. 
	 */
	void setEntity(Entity e);

	//
	// ====> Row creation.
	//
	
	/**
	 * Create a new row with default values.
	 * @return a pointer on a CalCurveRow
	 */
	CalCurveRow *newRow();
	
	/**
	  * Has the same definition than the newRow method with the same signature.
	  * Provided to facilitate the call from Python, otherwise the newRow method will be preferred.
	  */
	CalCurveRow* newRowEmpty();

	
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
	CalCurveRow *newRow(Tag calDataId, Tag calReductionId, int numAntenna, int numBaseline, int numAPC, int numReceptor, int numPoly, vector<string > antennaNames, string refAntennaName, ReceiverBandMod::ReceiverBand receiverBand, vector<AtmPhaseCorrectionMod::AtmPhaseCorrection > atmPhaseCorrections, vector<PolarizationTypeMod::PolarizationType > polarizationTypes, ArrayTime startValidTime, ArrayTime endValidTime, vector<Frequency > frequencyRange, CalCurveTypeMod::CalCurveType typeCurve, ArrayTime timeOrigin, vector<vector<vector<vector<float > > > > curve, vector<vector<vector<float > > > rms);
	
	/**
	  * Has the same definition than the newRow method with the same signature.
	  * Provided to facilitate the call from Python, otherwise the newRow method will be preferred.
	  */
	CalCurveRow *newRowFull(Tag calDataId, Tag calReductionId, int numAntenna, int numBaseline, int numAPC, int numReceptor, int numPoly, vector<string > antennaNames, string refAntennaName, ReceiverBandMod::ReceiverBand receiverBand, vector<AtmPhaseCorrectionMod::AtmPhaseCorrection > atmPhaseCorrections, vector<PolarizationTypeMod::PolarizationType > polarizationTypes, ArrayTime startValidTime, ArrayTime endValidTime, vector<Frequency > frequencyRange, CalCurveTypeMod::CalCurveType typeCurve, ArrayTime timeOrigin, vector<vector<vector<vector<float > > > > curve, vector<vector<vector<float > > > rms);


	/**
	 * Create a new row using a copy constructor mechanism.
	 * 
	 * The method creates a new CalCurveRow owned by this. Each attribute of the created row 
	 * is a (deep) copy of the corresponding attribute of row. The method does not add 
	 * the created row to this, its simply parents it to this, a call to the add method
	 * has to be done in order to get the row added (very likely after having modified
	 * some of its attributes).
	 * If row is null then the method returns a new CalCurveRow with default values for its attributes. 
	 *
	 * @param row the row which is to be copied.
	 */
	 CalCurveRow *newRow(CalCurveRow *row); 

	/**
	  * Has the same definition than the newRow method with the same signature.
	  * Provided to facilitate the call from Python, otherwise the newRow method will be preferred.
	  */
	 CalCurveRow *newRowCopy(CalCurveRow *row); 

	//
	// ====> Append a row to its table.
	//
 
	
	/**
	 * Add a row.
	 * @param x a pointer to the CalCurveRow to be added.
	 *
	 * @return a pointer to a CalCurveRow. If the table contains a CalCurveRow whose attributes (key and mandatory values) are equal to x ones
	 * then returns a pointer on that CalCurveRow, otherwise returns x.
	 *
	 * @throw DuplicateKey { thrown when the table contains a CalCurveRow with a key equal to the x one but having
	 * and a value section different from x one }
	 *
	
	 */
	CalCurveRow* add(CalCurveRow* x) ; 

 



	//
	// ====> Methods returning rows.
	//
		
	/**
	 * Get all rows.
	 * @return Alls rows as a vector of pointers of CalCurveRow. The elements of this vector are stored in the order 
	 * in which they have been added to the CalCurveTable.
	 */
	vector<CalCurveRow *> get() ;
	


 
	
	/**
 	 * Returns a CalCurveRow* given a key.
 	 * @return a pointer to the row having the key whose values are passed as parameters, or 0 if
 	 * no row exists for that key.
	
	 * @param calDataId. 
	
	 * @param calReductionId. 
	
 	 *
	 */
 	CalCurveRow* getRowByKey(Tag calDataId, Tag calReductionId);

 	 	



	/**
 	 * Look up the table for a row whose all attributes 
 	 * are equal to the corresponding parameters of the method.
 	 * @return a pointer on this row if any, null otherwise.
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
	CalCurveRow* lookup(Tag calDataId, Tag calReductionId, int numAntenna, int numBaseline, int numAPC, int numReceptor, int numPoly, vector<string > antennaNames, string refAntennaName, ReceiverBandMod::ReceiverBand receiverBand, vector<AtmPhaseCorrectionMod::AtmPhaseCorrection > atmPhaseCorrections, vector<PolarizationTypeMod::PolarizationType > polarizationTypes, ArrayTime startValidTime, ArrayTime endValidTime, vector<Frequency > frequencyRange, CalCurveTypeMod::CalCurveType typeCurve, ArrayTime timeOrigin, vector<vector<vector<vector<float > > > > curve, vector<vector<vector<float > > > rms); 


#ifndef WITHOUT_ACS
	// Conversion Methods
	/**
	 * Convert this table into a CalCurveTableIDL CORBA structure.
	 *
	 * @return a pointer to a CalCurveTableIDL
	 */
	CalCurveTableIDL *toIDL() ;
#endif

#ifndef WITHOUT_ACS
	/**
	 * Populate this table from the content of a CalCurveTableIDL Corba structure.
	 *
	 * @throws DuplicateKey Thrown if the method tries to add a row having a key that is already in the table.
	 * @throws ConversionException
	 */	
	void fromIDL(CalCurveTableIDL x) throw(DuplicateKey,ConversionException);
#endif

	/**
	 * To be implemented
	 */
	char *toFITS() const throw(ConversionException);

	/**
	 * To be implemented
	 */
	void fromFITS(char *fits) throw(ConversionException);

	/**
	 * To be implemented
	 */
	string toVOTable() const throw(ConversionException);

	/**
	 * To be implemented
	 */
	void fromVOTable(string vo) throw(ConversionException);

	/**
	 * Translate this table to an XML representation conform
	 * to the schema defined for CalCurve (CalCurveTable.xsd).
	 *
	 * @returns a string containing the XML representation.
	 */
	string toXML()  throw(ConversionException);
	
	/**
	 * Populate this table from the content of a XML document that is required to
	 * be conform to the XML schema defined for a CalCurve (CalCurveTable.xsd).
	 * 
	 */
	void fromXML(string xmlDoc) throw(ConversionException);
	
   /**
	 * Serialize this into a stream of bytes and encapsulates that stream into a MIME message.
	 * @returns a string containing the MIME message.
	 * 
	 */
	string toMIME();
	
   /** 
     * Extracts the binary part of a MIME message and deserialize its content
	 * to fill this with the result of the deserialization. 
	 * @param mimeMsg the string containing the MIME message.
	 * @throws ConversionException
	 */
	 void setFromMIME(const string & mimeMsg);
	
	/**
	  * Stores a representation (binary or XML) of this table into a file.
	  *
	  * Depending on the boolean value of its private field fileAsBin a binary serialization  of this (fileAsBin==true)  
	  * will be saved in a file "CalCurve.bin" or an XML representation (fileAsBin==false) will be saved in a file "CalCurve.xml".
	  * The file is always written in a directory whose name is passed as a parameter.
	 * @param directory The name of directory  where the file containing the table's representation will be saved.
	  * 
	  */
	  void toFile(string directory);
	  
	/**
	 * Reads and parses a file containing a representation of a CalCurveTable as those produced  by the toFile method.
	 * This table is populated with the result of the parsing.
	 * @param directory The name of the directory containing the file te be read and parsed.
	 * @throws ConversionException If any error occurs while reading the 
	 * files in the directory or parsing them.
	 *
	 */
	 void setFromFile(const string& directory);	

private:

	/**
	 * Create a CalCurveTable.
	 * <p>
	 * This constructor is private because only the
	 * container can create tables.  All tables must know the container
	 * to which they belong.
	 * @param container The container to which this table belongs.
	 */ 
	CalCurveTable (ASDM & container);

	ASDM & container;
	
	bool archiveAsBin; // If true archive binary else archive XML
	bool fileAsBin ; // If true file binary else file XML	
	
	Entity entity;
	


	/**
	 * The name of this table.
	 */
	static string tableName;


	/**
	 * The list of field names that make up key key.
	 */
	static vector<string> key;


	/**
	 * If this table has an autoincrementable attribute then check if *x verifies the rule of uniqueness and throw exception if not.
	 * Check if *x verifies the key uniqueness rule and throw an exception if not.
	 * Append x to its table.
	 */
	CalCurveRow* checkAndAdd(CalCurveRow* x) throw (DuplicateKey);



// A data structure to store the pointers on the table's rows.

// In all cases we maintain a private ArrayList of CalCurveRow s.
   vector<CalCurveRow * > privateRows;
   

			
	vector<CalCurveRow *> row;


	void error() throw(ConversionException);

};

} // End namespace asdm

#endif /* CalCurveTable_CLASS */
