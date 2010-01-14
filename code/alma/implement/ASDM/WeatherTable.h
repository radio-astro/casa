
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
 * File WeatherTable.h
 */
 
#ifndef WeatherTable_CLASS
#define WeatherTable_CLASS

#include <string>
#include <vector>
#include <map>
#include <set>
using std::string;
using std::vector;
using std::map;



#include <Angle.h>
using  asdm::Angle;

#include <Speed.h>
using  asdm::Speed;

#include <Tag.h>
using  asdm::Tag;

#include <Temperature.h>
using  asdm::Temperature;

#include <Humidity.h>
using  asdm::Humidity;

#include <ArrayTimeInterval.h>
using  asdm::ArrayTimeInterval;

#include <Pressure.h>
using  asdm::Pressure;




	

	

	

	

	

	

	

	

	

	

	

	

	

	

	



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
using asdmIDL::WeatherTableIDL;
#endif

#include <Representable.h>

namespace asdm {

//class asdm::ASDM;
//class asdm::WeatherRow;

class ASDM;
class WeatherRow;
/**
 * The WeatherTable class is an Alma table.
 * <BR>
 * 
 * \par Role
 * Weather station information.
 * <BR>
 
 * Generated from model's revision "1.53", branch "HEAD"
 *
 * <TABLE BORDER="1">
 * <CAPTION> Attributes of Weather </CAPTION>
 * <TR BGCOLOR="#AAAAAA"> <TH> Name </TH> <TH> Type </TH> <TH> Expected shape  </TH> <TH> Comment </TH></TR>
 
 * <TR> <TH BGCOLOR="#CCCCCC" colspan="4" align="center"> Key </TD></TR>
	
 * <TR>
 		
 * <TD> stationId </TD>
 		 
 * <TD> Tag</TD>
 * <TD> &nbsp; </TD>
 * <TD> &nbsp;refers to a unique row in StationTable. </TD>
 * </TR>
	
 * <TR>
 		
 * <TD> timeInterval </TD>
 		 
 * <TD> ArrayTimeInterval</TD>
 * <TD> &nbsp; </TD>
 * <TD> &nbsp;the time interval for which the row's content is valid. </TD>
 * </TR>
	


 * <TR> <TH BGCOLOR="#CCCCCC"  colspan="4" valign="center"> Value <br> (Mandarory) </TH></TR>
	
 * <TR>
 * <TD> pressure </TD> 
 * <TD> Pressure </TD>
 * <TD>  &nbsp;  </TD> 
 * <TD> &nbsp;the ambient pressure. </TD>
 * </TR>
	
 * <TR>
 * <TD> pressureFlag </TD> 
 * <TD> bool </TD>
 * <TD>  &nbsp;  </TD> 
 * <TD> &nbsp;the ambient pressure's value  is invalid (true) or valid (false). </TD>
 * </TR>
	
 * <TR>
 * <TD> relHumidity </TD> 
 * <TD> Humidity </TD>
 * <TD>  &nbsp;  </TD> 
 * <TD> &nbsp;the relative humidity. </TD>
 * </TR>
	
 * <TR>
 * <TD> relHumidityFlag </TD> 
 * <TD> bool </TD>
 * <TD>  &nbsp;  </TD> 
 * <TD> &nbsp;the relative humidity's value  is invalid (true) or valid (false). </TD>
 * </TR>
	
 * <TR>
 * <TD> temperature </TD> 
 * <TD> Temperature </TD>
 * <TD>  &nbsp;  </TD> 
 * <TD> &nbsp;the ambient temperature. </TD>
 * </TR>
	
 * <TR>
 * <TD> temperatureFlag </TD> 
 * <TD> bool </TD>
 * <TD>  &nbsp;  </TD> 
 * <TD> &nbsp;the ambient temperature's value is invalid (true) or valid (false). </TD>
 * </TR>
	
 * <TR>
 * <TD> windDirection </TD> 
 * <TD> Angle </TD>
 * <TD>  &nbsp;  </TD> 
 * <TD> &nbsp;the wind direction. </TD>
 * </TR>
	
 * <TR>
 * <TD> windDirectionFlag </TD> 
 * <TD> bool </TD>
 * <TD>  &nbsp;  </TD> 
 * <TD> &nbsp;the wind direction's value is invalid (true) or valid (false). </TD>
 * </TR>
	
 * <TR>
 * <TD> windSpeed </TD> 
 * <TD> Speed </TD>
 * <TD>  &nbsp;  </TD> 
 * <TD> &nbsp;the wind speed. </TD>
 * </TR>
	
 * <TR>
 * <TD> windSpeedFlag </TD> 
 * <TD> bool </TD>
 * <TD>  &nbsp;  </TD> 
 * <TD> &nbsp;the  wind speed value is invalid (true) or valid (false). </TD>
 * </TR>
	
 * <TR>
 * <TD> windMax </TD> 
 * <TD> Speed </TD>
 * <TD>  &nbsp;  </TD> 
 * <TD> &nbsp;the maximum wind speed </TD>
 * </TR>
	
 * <TR>
 * <TD> windMaxFlag </TD> 
 * <TD> bool </TD>
 * <TD>  &nbsp;  </TD> 
 * <TD> &nbsp;the maximum wind speed value is invalid (true) or valid (false). </TD>
 * </TR>
	


 * <TR> <TH BGCOLOR="#CCCCCC"  colspan="4" valign="center"> Value <br> (Optional) </TH></TR>
	
 * <TR>
 * <TD> dewPoint </TD> 
 * <TD> Temperature </TD>
 * <TD>  &nbsp; </TD>
 * <TD>&nbsp; the dew point's value. </TD>
 * </TR>
	
 * <TR>
 * <TD> dewPointFlag </TD> 
 * <TD> bool </TD>
 * <TD>  &nbsp; </TD>
 * <TD>&nbsp; the dew point's value is invalid (true) or valid (false). </TD>
 * </TR>
	

 * </TABLE>
 */
class WeatherTable : public Representable {
	friend class ASDM;

public:


	/**
	 * Return the list of field names that make up key key
	 * as an array of strings.
	 * @return a vector of string.
	 */	
	static vector<string> getKeyName();


	virtual ~WeatherTable();
	
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


	//
	// ====> Row creation.
	//
	
	/**
	 * Create a new row with default values.
	 * @return a pointer on a WeatherRow
	 */
	WeatherRow *newRow();
	
	
	/**
	 * Create a new row initialized to the specified values.
	 * @return a pointer on the created and initialized row.
	
 	 * @param stationId
	
 	 * @param timeInterval
	
 	 * @param pressure
	
 	 * @param pressureFlag
	
 	 * @param relHumidity
	
 	 * @param relHumidityFlag
	
 	 * @param temperature
	
 	 * @param temperatureFlag
	
 	 * @param windDirection
	
 	 * @param windDirectionFlag
	
 	 * @param windSpeed
	
 	 * @param windSpeedFlag
	
 	 * @param windMax
	
 	 * @param windMaxFlag
	
     */
	WeatherRow *newRow(Tag stationId, ArrayTimeInterval timeInterval, Pressure pressure, bool pressureFlag, Humidity relHumidity, bool relHumidityFlag, Temperature temperature, bool temperatureFlag, Angle windDirection, bool windDirectionFlag, Speed windSpeed, bool windSpeedFlag, Speed windMax, bool windMaxFlag);
	


	/**
	 * Create a new row using a copy constructor mechanism.
	 * 
	 * The method creates a new WeatherRow owned by this. Each attribute of the created row 
	 * is a (deep) copy of the corresponding attribute of row. The method does not add 
	 * the created row to this, its simply parents it to this, a call to the add method
	 * has to be done in order to get the row added (very likely after having modified
	 * some of its attributes).
	 * If row is null then the method returns a new WeatherRow with default values for its attributes. 
	 *
	 * @param row the row which is to be copied.
	 */
	 WeatherRow *newRow(WeatherRow *row); 

	//
	// ====> Append a row to its table.
	//
 
	
	/**
	 * Add a row.
	 * @param x a pointer to the WeatherRow to be added.
	 *
	 * @return a pointer to a WeatherRow. If the table contains a WeatherRow whose attributes (key and mandatory values) are equal to x ones
	 * then returns a pointer on that WeatherRow, otherwise returns x.
	 *
	 * @throw DuplicateKey { thrown when the table contains a WeatherRow with a key equal to the x one but having
	 * and a value section different from x one }
	 *
	
	 * @note The row is inserted in the table in such a way that all the rows having the same value of
	 * ( stationId ) are stored by ascending time.
	 * @see method getByContext.
	
	 */
	WeatherRow* add(WeatherRow* x) ; 

 



	//
	// ====> Methods returning rows.
	//
		
	/**
	 * Get all rows.
	 * @return Alls rows as a vector of pointers of WeatherRow. The elements of this vector are stored in the order 
	 * in which they have been added to the WeatherTable.
	 */
	vector<WeatherRow *> get() ;
	

	/**
	 * Returns all the rows sorted by ascending startTime for a given context. 
	 * The context is defined by a value of ( stationId ).
	 *
	 * @return a pointer on a vector<WeatherRow *>. A null returned value means that the table contains
	 * no WeatherRow for the given ( stationId ).
	 */
	 vector <WeatherRow*> *getByContext(Tag stationId);
	 


 
	
	/**
 	 * Returns a WeatherRow* given a key.
 	 * @return a pointer to the row having the key whose values are passed as parameters, or 0 if
 	 * no row exists for that key.
	
	 * @param stationId
	
	 * @param timeInterval
	
 	 *
	 */
 	WeatherRow* getRowByKey(Tag stationId, ArrayTimeInterval timeInterval);

 	 	



	/**
 	 * Look up the table for a row whose all attributes 
 	 * are equal to the corresponding parameters of the method.
 	 * @return a pointer on this row if any, null otherwise.
 	 *
			
 	 * @param stationId
 	 		
 	 * @param timeInterval
 	 		
 	 * @param pressure
 	 		
 	 * @param pressureFlag
 	 		
 	 * @param relHumidity
 	 		
 	 * @param relHumidityFlag
 	 		
 	 * @param temperature
 	 		
 	 * @param temperatureFlag
 	 		
 	 * @param windDirection
 	 		
 	 * @param windDirectionFlag
 	 		
 	 * @param windSpeed
 	 		
 	 * @param windSpeedFlag
 	 		
 	 * @param windMax
 	 		
 	 * @param windMaxFlag
 	 		 
 	 */
	WeatherRow* lookup(Tag stationId, ArrayTimeInterval timeInterval, Pressure pressure, bool pressureFlag, Humidity relHumidity, bool relHumidityFlag, Temperature temperature, bool temperatureFlag, Angle windDirection, bool windDirectionFlag, Speed windSpeed, bool windSpeedFlag, Speed windMax, bool windMaxFlag); 


private:

	/**
	 * Create a WeatherTable.
	 * <p>
	 * This constructor is private because only the
	 * container can create tables.  All tables must know the container
	 * to which they belong.
	 * @param container The container to which this table belongs.
	 */ 
	WeatherTable (ASDM & container);

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
	 * @throws DuplicateKey
	 
	 */
	WeatherRow* checkAndAdd(WeatherRow* x) ;


	
	
	/**
	 * Insert a WeatherRow* in a vector of WeatherRow* so that it's ordered by ascending time.
	 *
	 * @param WeatherRow* x . The pointer to be inserted.
	 * @param vector <WeatherRow*>& row . A reference to the vector where to insert x.
	 *
	 */
	 WeatherRow * insertByStartTime(WeatherRow* x, vector<WeatherRow* >& row);
	  


// A data structure to store the pointers on the table's rows.

// In all cases we maintain a private vector of WeatherRow s.
   vector<WeatherRow * > privateRows;
   

	

	
	
		
				
	typedef vector <WeatherRow* > TIME_ROWS;
	map<string, TIME_ROWS > context;
		
	/** 
	 * Returns a string built by concatenating the ascii representation of the
	 * parameters values suffixed with a "_" character.
	 */
	 string Key(Tag stationId) ;
		 
		
	
	
	/**
	 * Fills the vector vout (passed by reference) with pointers on elements of vin 
	 * whose attributes are equal to the corresponding parameters of the method.
	 *
	 */
	void getByKeyNoAutoIncNoTime(vector <WeatherRow*>& vin, vector <WeatherRow*>& vout,  Tag stationId);
	


	/**
	 * Return this table's Entity.
	 */
	Entity getEntity() const;

	/**
	 * Set this table's Entity.
	 * @param e An entity. 
	 */
	void setEntity(Entity e);

#ifndef WITHOUT_ACS
	// Conversion Methods
	/**
	 * Convert this table into a WeatherTableIDL CORBA structure.
	 *
	 * @return a pointer to a WeatherTableIDL
	 */
	WeatherTableIDL *toIDL() ;
#endif

#ifndef WITHOUT_ACS
	/**
	 * Populate this table from the content of a WeatherTableIDL Corba structure.
	 *
	 * @throws DuplicateKey Thrown if the method tries to add a row having a key that is already in the table.
	 * @throws ConversionException
	 */	
	void fromIDL(WeatherTableIDL x) ;
#endif
	
	
	void error() ; //throw(ConversionException);

	/**
	 * Translate this table to an XML representation conform
	 * to the schema defined for Weather (WeatherTable.xsd).
	 *
	 * @returns a string containing the XML representation.
	 * @throws ConversionException
	 */
	string toXML()  ;
	
	/**
	 * Populate this table from the content of a XML document that is required to
	 * be conform to the XML schema defined for a Weather (WeatherTable.xsd).
	 * @throws ConversionException
	 * 
	 */
	void fromXML(string xmlDoc) ;
		
	/**
	  * Private methods involved during the build of this table out of the content
	  * of file(s) containing an external representation of a Weather table.
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
	  * will be saved in a file "Weather.bin" or an XML representation (fileAsBin==false) will be saved in a file "Weather.xml".
	  * The file is always written in a directory whose name is passed as a parameter.
	 * @param directory The name of directory  where the file containing the table's representation will be saved.
	  * 
	  */
	  void toFile(string directory);
	  
	/**
	 * Reads and parses a file containing a representation of a WeatherTable as those produced  by the toFile method.
	 * This table is populated with the result of the parsing.
	 * @param directory The name of the directory containing the file te be read and parsed.
	 * @throws ConversionException If any error occurs while reading the 
	 * files in the directory or parsing them.
	 *
	 */
	 void setFromFile(const string& directory);	
 
};

} // End namespace asdm

#endif /* WeatherTable_CLASS */
