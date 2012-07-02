
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
 * File CalWVRTable.h
 */
 
#ifndef CalWVRTable_CLASS
#define CalWVRTable_CLASS

#include <string>
#include <vector>
#include <map>
#include <set>
using std::string;
using std::vector;
using std::map;



#include <ArrayTime.h>
using  asdm::ArrayTime;

#include <Tag.h>
using  asdm::Tag;

#include <Length.h>
using  asdm::Length;

#include <Temperature.h>
using  asdm::Temperature;

#include <Frequency.h>
using  asdm::Frequency;




	

	

	
#include "CWVRMethod.h"
using namespace WVRMethodMod;
	

	

	

	

	

	

	

	

	

	

	

	

	

	



#include <ConversionException.h>
#include <DuplicateKey.h>
#include <UniquenessViolationException.h>
#include <NoSuchRow.h>
#include <DuplicateKey.h>
using asdm::DuplicateKey;
using asdm::ConversionException;
using asdm::NoSuchRow;
using asdm::DuplicateKey;

#ifndef WITHOUT_ACS
#include <asdmIDLC.h>
using asdmIDL::CalWVRTableIDL;
#endif

#include <Representable.h>

namespace asdm {

//class asdm::ASDM;
//class asdm::CalWVRRow;

class ASDM;
class CalWVRRow;
/**
 * The CalWVRTable class is an Alma table.
 * <BR>
 * 
 * \par Role
 * Result of the water vapour radiometric  calibration performed by TelCal. 
 * <BR>
 
 * Generated from model's revision "1.54", branch "HEAD"
 *
 * <TABLE BORDER="1">
 * <CAPTION> Attributes of CalWVR </CAPTION>
 * <TR BGCOLOR="#AAAAAA"> <TH> Name </TH> <TH> Type </TH> <TH> Expected shape  </TH> <TH> Comment </TH></TR>
 
 * <TR> <TH BGCOLOR="#CCCCCC" colspan="4" align="center"> Key </TD></TR>
	
 * <TR>
 		
 * <TD> antennaName </TD>
 		 
 * <TD> string</TD>
 * <TD> &nbsp; </TD>
 * <TD> &nbsp;the name of the antenna. </TD>
 * </TR>
	
 * <TR>
 		
 * <TD> calDataId </TD>
 		 
 * <TD> Tag</TD>
 * <TD> &nbsp; </TD>
 * <TD> &nbsp;refers to a unique row in CalData Table. </TD>
 * </TR>
	
 * <TR>
 		
 * <TD> calReductionId </TD>
 		 
 * <TD> Tag</TD>
 * <TD> &nbsp; </TD>
 * <TD> &nbsp;refers to unique row  in CalReductionTable. </TD>
 * </TR>
	


 * <TR> <TH BGCOLOR="#CCCCCC"  colspan="4" valign="center"> Value <br> (Mandarory) </TH></TR>
	
 * <TR>
 * <TD> startValidTime </TD> 
 * <TD> ArrayTime </TD>
 * <TD>  &nbsp;  </TD> 
 * <TD> &nbsp;the start time of result validity period. </TD>
 * </TR>
	
 * <TR>
 * <TD> endValidTime </TD> 
 * <TD> ArrayTime </TD>
 * <TD>  &nbsp;  </TD> 
 * <TD> &nbsp;the end time of result validity period. </TD>
 * </TR>
	
 * <TR>
 * <TD> wvrMethod </TD> 
 * <TD> WVRMethodMod::WVRMethod </TD>
 * <TD>  &nbsp;  </TD> 
 * <TD> &nbsp;identifies the method used for the calibration. </TD>
 * </TR>
	
 * <TR>
 * <TD> numInputAntennas </TD> 
 * <TD> int </TD>
 * <TD>  &nbsp;  </TD> 
 * <TD> &nbsp;the number of input antennas (i.e. equiped with functional WVRs). </TD>
 * </TR>
	
 * <TR>
 * <TD> inputAntennaNames </TD> 
 * <TD> vector<string > </TD>
 * <TD>  numInputAntennas </TD> 
 * <TD> &nbsp;the names of the input antennas (one string per antenna). </TD>
 * </TR>
	
 * <TR>
 * <TD> numChan </TD> 
 * <TD> int </TD>
 * <TD>  &nbsp;  </TD> 
 * <TD> &nbsp;the number of frequency channels in the WVR receiver. </TD>
 * </TR>
	
 * <TR>
 * <TD> chanFreq </TD> 
 * <TD> vector<Frequency > </TD>
 * <TD>  numChan </TD> 
 * <TD> &nbsp;the channel frequencies (one value per channel). </TD>
 * </TR>
	
 * <TR>
 * <TD> chanWidth </TD> 
 * <TD> vector<Frequency > </TD>
 * <TD>  numChan </TD> 
 * <TD> &nbsp;the widths of the channels (one value per channel). </TD>
 * </TR>
	
 * <TR>
 * <TD> refTemp </TD> 
 * <TD> vector<vector<Temperature > > </TD>
 * <TD>  numInputAntennas, numChan </TD> 
 * <TD> &nbsp;the reference temperatures (one value per input antenna per channel). </TD>
 * </TR>
	
 * <TR>
 * <TD> numPoly </TD> 
 * <TD> int </TD>
 * <TD>  &nbsp;  </TD> 
 * <TD> &nbsp;the number of polynomial coefficients. </TD>
 * </TR>
	
 * <TR>
 * <TD> pathCoeff </TD> 
 * <TD> vector<vector<vector<float > > > </TD>
 * <TD>  numInputAntennas, numChan, numPoly </TD> 
 * <TD> &nbsp;the path length coefficients (one value per input antenna per channel per polynomial coefficient). </TD>
 * </TR>
	
 * <TR>
 * <TD> polyFreqLimits </TD> 
 * <TD> vector<Frequency > </TD>
 * <TD>  2 </TD> 
 * <TD> &nbsp;the limits of the interval of frequencies for which the path length coefficients are computed. </TD>
 * </TR>
	
 * <TR>
 * <TD> wetPath </TD> 
 * <TD> vector<float > </TD>
 * <TD>  numPoly </TD> 
 * <TD> &nbsp;The wet path as a function frequency (expressed as a polynomial). </TD>
 * </TR>
	
 * <TR>
 * <TD> dryPath </TD> 
 * <TD> vector<float > </TD>
 * <TD>  numPoly </TD> 
 * <TD> &nbsp;The dry path as a function frequency (expressed as a polynomial). </TD>
 * </TR>
	
 * <TR>
 * <TD> water </TD> 
 * <TD> Length </TD>
 * <TD>  &nbsp;  </TD> 
 * <TD> &nbsp;The precipitable water vapor corresponding to the reference model. </TD>
 * </TR>
	


 * </TABLE>
 */
class CalWVRTable : public Representable {
	friend class ASDM;

public:


	/**
	 * Return the list of field names that make up key key
	 * as an array of strings.
	 * @return a vector of string.
	 */	
	static vector<string> getKeyName();


	virtual ~CalWVRTable();
	
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
	 * Return the names of the attributes of this table.
	 *
	 * @return a vector of string
	 */
	 static const vector<string>& getAttributesNames();

	/**
	 * Return this table's Entity.
	 */
	Entity getEntity() const;

	/**
	 * Set this table's Entity.
	 * @param e An entity. 
	 */
	void setEntity(Entity e);
		
	/**
	 * Produces an XML representation conform
	 * to the schema defined for CalWVR (CalWVRTable.xsd).
	 *
	 * @returns a string containing the XML representation.
	 * @throws ConversionException
	 */
	string toXML()  ;

#ifndef WITHOUT_ACS
	// Conversion Methods
	/**
	 * Convert this table into a CalWVRTableIDL CORBA structure.
	 *
	 * @return a pointer to a CalWVRTableIDL
	 */
	CalWVRTableIDL *toIDL() ;
#endif

#ifndef WITHOUT_ACS
	/**
	 * Populate this table from the content of a CalWVRTableIDL Corba structure.
	 *
	 * @throws DuplicateKey Thrown if the method tries to add a row having a key that is already in the table.
	 * @throws ConversionException
	 */	
	void fromIDL(CalWVRTableIDL x) ;
#endif
	
	//
	// ====> Row creation.
	//
	
	/**
	 * Create a new row with default values.
	 * @return a pointer on a CalWVRRow
	 */
	CalWVRRow *newRow();
	
	
	/**
	 * Create a new row initialized to the specified values.
	 * @return a pointer on the created and initialized row.
	
 	 * @param antennaName
	
 	 * @param calDataId
	
 	 * @param calReductionId
	
 	 * @param startValidTime
	
 	 * @param endValidTime
	
 	 * @param wvrMethod
	
 	 * @param numInputAntennas
	
 	 * @param inputAntennaNames
	
 	 * @param numChan
	
 	 * @param chanFreq
	
 	 * @param chanWidth
	
 	 * @param refTemp
	
 	 * @param numPoly
	
 	 * @param pathCoeff
	
 	 * @param polyFreqLimits
	
 	 * @param wetPath
	
 	 * @param dryPath
	
 	 * @param water
	
     */
	CalWVRRow *newRow(string antennaName, Tag calDataId, Tag calReductionId, ArrayTime startValidTime, ArrayTime endValidTime, WVRMethodMod::WVRMethod wvrMethod, int numInputAntennas, vector<string > inputAntennaNames, int numChan, vector<Frequency > chanFreq, vector<Frequency > chanWidth, vector<vector<Temperature > > refTemp, int numPoly, vector<vector<vector<float > > > pathCoeff, vector<Frequency > polyFreqLimits, vector<float > wetPath, vector<float > dryPath, Length water);
	


	/**
	 * Create a new row using a copy constructor mechanism.
	 * 
	 * The method creates a new CalWVRRow owned by this. Each attribute of the created row 
	 * is a (deep) copy of the corresponding attribute of row. The method does not add 
	 * the created row to this, its simply parents it to this, a call to the add method
	 * has to be done in order to get the row added (very likely after having modified
	 * some of its attributes).
	 * If row is null then the method returns a new CalWVRRow with default values for its attributes. 
	 *
	 * @param row the row which is to be copied.
	 */
	 CalWVRRow *newRow(CalWVRRow *row); 

	//
	// ====> Append a row to its table.
	//
 
	
	/**
	 * Add a row.
	 * @param x a pointer to the CalWVRRow to be added.
	 *
	 * @return a pointer to a CalWVRRow. If the table contains a CalWVRRow whose attributes (key and mandatory values) are equal to x ones
	 * then returns a pointer on that CalWVRRow, otherwise returns x.
	 *
	 * @throw DuplicateKey { thrown when the table contains a CalWVRRow with a key equal to the x one but having
	 * and a value section different from x one }
	 *
	
	 */
	CalWVRRow* add(CalWVRRow* x) ; 

 



	//
	// ====> Methods returning rows.
	//
		
	/**
	 * Get all rows.
	 * @return Alls rows as a vector of pointers of CalWVRRow. The elements of this vector are stored in the order 
	 * in which they have been added to the CalWVRTable.
	 */
	vector<CalWVRRow *> get() ;
	


 
	
	/**
 	 * Returns a CalWVRRow* given a key.
 	 * @return a pointer to the row having the key whose values are passed as parameters, or 0 if
 	 * no row exists for that key.
	
	 * @param antennaName
	
	 * @param calDataId
	
	 * @param calReductionId
	
 	 *
	 */
 	CalWVRRow* getRowByKey(string antennaName, Tag calDataId, Tag calReductionId);

 	 	



	/**
 	 * Look up the table for a row whose all attributes 
 	 * are equal to the corresponding parameters of the method.
 	 * @return a pointer on this row if any, null otherwise.
 	 *
			
 	 * @param antennaName
 	 		
 	 * @param calDataId
 	 		
 	 * @param calReductionId
 	 		
 	 * @param startValidTime
 	 		
 	 * @param endValidTime
 	 		
 	 * @param wvrMethod
 	 		
 	 * @param numInputAntennas
 	 		
 	 * @param inputAntennaNames
 	 		
 	 * @param numChan
 	 		
 	 * @param chanFreq
 	 		
 	 * @param chanWidth
 	 		
 	 * @param refTemp
 	 		
 	 * @param numPoly
 	 		
 	 * @param pathCoeff
 	 		
 	 * @param polyFreqLimits
 	 		
 	 * @param wetPath
 	 		
 	 * @param dryPath
 	 		
 	 * @param water
 	 		 
 	 */
	CalWVRRow* lookup(string antennaName, Tag calDataId, Tag calReductionId, ArrayTime startValidTime, ArrayTime endValidTime, WVRMethodMod::WVRMethod wvrMethod, int numInputAntennas, vector<string > inputAntennaNames, int numChan, vector<Frequency > chanFreq, vector<Frequency > chanWidth, vector<vector<Temperature > > refTemp, int numPoly, vector<vector<vector<float > > > pathCoeff, vector<Frequency > polyFreqLimits, vector<float > wetPath, vector<float > dryPath, Length water); 


private:

	/**
	 * Create a CalWVRTable.
	 * <p>
	 * This constructor is private because only the
	 * container can create tables.  All tables must know the container
	 * to which they belong.
	 * @param container The container to which this table belongs.
	 */ 
	CalWVRTable (ASDM & container);

	ASDM & container;
	
	bool archiveAsBin; // If true archive binary else archive XML
	bool fileAsBin ; // If true file binary else file XML	
	
	Entity entity;
	


	/**
	 * The name of this table.
	 */
	static string tableName;
	
	/**
	 * The attributes names.
	 */
	static const vector<string> attributesNames;
	
	/**
	 * A method to fill attributesNames;
	 */
	static vector<string> initAttributesNames();


	/**
	 * The list of field names that make up key key.
	 */
	static vector<string> key;


	/**
	 * If this table has an autoincrementable attribute then check if *x verifies the rule of uniqueness and throw exception if not.
	 * Check if *x verifies the key uniqueness rule and throw an exception if not.
	 * Append x to its table.
	 * @throws DuplicateKey
	 
	 */
	CalWVRRow* checkAndAdd(CalWVRRow* x) ;



// A data structure to store the pointers on the table's rows.

// In all cases we maintain a private vector of CalWVRRow s.
   vector<CalWVRRow * > privateRows;
   

			
	vector<CalWVRRow *> row;

	
	void error() ; //throw(ConversionException);

	
	/**
	 * Populate this table from the content of a XML document that is required to
	 * be conform to the XML schema defined for a CalWVR (CalWVRTable.xsd).
	 * @throws ConversionException
	 * 
	 */
	void fromXML(string xmlDoc) ;
		
	/**
	  * Private methods involved during the build of this table out of the content
	  * of file(s) containing an external representation of a CalWVR table.
	  */
	void setFromMIMEFile(const string& directory);
	void setFromXMLFile(const string& directory);
	
		 /**
	 * Serialize this into a stream of bytes and encapsulates that stream into a MIME message.
	 * @returns a string containing the MIME message.
	 *
	 * @param byteOrder a const pointer to a static instance of the class ByteOrder.
	 * 
	 */
	string toMIME(const asdm::ByteOrder* byteOrder=asdm::ByteOrder::Machine_Endianity);
  
	
   /** 
     * Extracts the binary part of a MIME message and deserialize its content
	 * to fill this with the result of the deserialization. 
	 * @param mimeMsg the string containing the MIME message.
	 * @throws ConversionException
	 */
	 void setFromMIME(const string & mimeMsg);
	
	/**
	  * Private methods involved during the export of this table into disk file(s).
	  */
	string MIMEXMLPart(const asdm::ByteOrder* byteOrder=asdm::ByteOrder::Machine_Endianity);
	
	/**
	  * Stores a representation (binary or XML) of this table into a file.
	  *
	  * Depending on the boolean value of its private field fileAsBin a binary serialization  of this (fileAsBin==true)  
	  * will be saved in a file "CalWVR.bin" or an XML representation (fileAsBin==false) will be saved in a file "CalWVR.xml".
	  * The file is always written in a directory whose name is passed as a parameter.
	 * @param directory The name of directory  where the file containing the table's representation will be saved.
	  * 
	  */
	  void toFile(string directory);
	  
	/**
	 * Reads and parses a file containing a representation of a CalWVRTable as those produced  by the toFile method.
	 * This table is populated with the result of the parsing.
	 * @param directory The name of the directory containing the file te be read and parsed.
	 * @throws ConversionException If any error occurs while reading the 
	 * files in the directory or parsing them.
	 *
	 */
	 void setFromFile(const string& directory);	
 
};

} // End namespace asdm

#endif /* CalWVRTable_CLASS */
