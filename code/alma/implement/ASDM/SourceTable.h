
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
 * File SourceTable.h
 */
 
#ifndef SourceTable_CLASS
#define SourceTable_CLASS

#include <string>
#include <vector>
#include <map>
#include <set>
using std::string;
using std::vector;
using std::map;



#include <ArrayTime.h>
using  asdm::ArrayTime;

#include <AngularRate.h>
using  asdm::AngularRate;

#include <Angle.h>
using  asdm::Angle;

#include <Speed.h>
using  asdm::Speed;

#include <Flux.h>
using  asdm::Flux;

#include <Tag.h>
using  asdm::Tag;

#include <Length.h>
using  asdm::Length;

#include <Frequency.h>
using  asdm::Frequency;

#include <ArrayTimeInterval.h>
using  asdm::ArrayTimeInterval;




	

	

	

	

	

	

	
#include "CDirectionReferenceCode.h"
using namespace DirectionReferenceCodeMod;
	

	

	

	

	

	

	

	

	

	

	

	
#include "CSourceModel.h"
using namespace SourceModelMod;
	

	
#include "CFrequencyReferenceCode.h"
using namespace FrequencyReferenceCodeMod;
	

	

	

	

	

	
#include "CStokesParameter.h"
using namespace StokesParameterMod;
	

	

	

	

	

	

	



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
using asdmIDL::SourceTableIDL;
#endif

#include <Representable.h>

namespace asdm {

//class asdm::ASDM;
//class asdm::SourceRow;

class ASDM;
class SourceRow;
/**
 * The SourceTable class is an Alma table.
 * <BR>
 * 
 * \par Role
 * Summary of astromomical source information.
 * <BR>
 
 * Generated from model's revision "1.53", branch "HEAD"
 *
 * <TABLE BORDER="1">
 * <CAPTION> Attributes of Source </CAPTION>
 * <TR BGCOLOR="#AAAAAA"> <TH> Name </TH> <TH> Type </TH> <TH> Expected shape  </TH> <TH> Comment </TH></TR>
 
 * <TR> <TH BGCOLOR="#CCCCCC" colspan="4" align="center"> Key </TD></TR>
	
 * <TR>
 		
 * <TD><I> sourceId </I></TD>
 		 
 * <TD> int</TD>
 * <TD> &nbsp; </TD>
 * <TD> &nbsp;identifies a collection of rows in the table. </TD>
 * </TR>
	
 * <TR>
 		
 * <TD> timeInterval </TD>
 		 
 * <TD> ArrayTimeInterval</TD>
 * <TD> &nbsp; </TD>
 * <TD> &nbsp; the time interval of validity of the row's content. </TD>
 * </TR>
	
 * <TR>
 		
 * <TD> spectralWindowId </TD>
 		 
 * <TD> Tag</TD>
 * <TD> &nbsp; </TD>
 * <TD> &nbsp;refers to a unique row in SpectralWindowTable. </TD>
 * </TR>
	


 * <TR> <TH BGCOLOR="#CCCCCC"  colspan="4" valign="center"> Value <br> (Mandarory) </TH></TR>
	
 * <TR>
 * <TD> code </TD> 
 * <TD> string </TD>
 * <TD>  &nbsp;  </TD> 
 * <TD> &nbsp;indicates the nature of the source. </TD>
 * </TR>
	
 * <TR>
 * <TD> direction </TD> 
 * <TD> vector<Angle > </TD>
 * <TD>  2 </TD> 
 * <TD> &nbsp;the direction of the source. </TD>
 * </TR>
	
 * <TR>
 * <TD> properMotion </TD> 
 * <TD> vector<AngularRate > </TD>
 * <TD>  2 </TD> 
 * <TD> &nbsp;the proper motion of the source. </TD>
 * </TR>
	
 * <TR>
 * <TD> sourceName </TD> 
 * <TD> string </TD>
 * <TD>  &nbsp;  </TD> 
 * <TD> &nbsp;the name of the source. </TD>
 * </TR>
	


 * <TR> <TH BGCOLOR="#CCCCCC"  colspan="4" valign="center"> Value <br> (Optional) </TH></TR>
	
 * <TR>
 * <TD> directionCode </TD> 
 * <TD> DirectionReferenceCodeMod::DirectionReferenceCode </TD>
 * <TD>  &nbsp; </TD>
 * <TD>&nbsp; identifies the direction reference frame associated to direction. </TD>
 * </TR>
	
 * <TR>
 * <TD> directionEquinox </TD> 
 * <TD> ArrayTime </TD>
 * <TD>  &nbsp; </TD>
 * <TD>&nbsp; the equinox associated to the direction reference frame (if required). </TD>
 * </TR>
	
 * <TR>
 * <TD> calibrationGroup </TD> 
 * <TD> int </TD>
 * <TD>  &nbsp; </TD>
 * <TD>&nbsp; the calibration group number. </TD>
 * </TR>
	
 * <TR>
 * <TD> catalog </TD> 
 * <TD> string </TD>
 * <TD>  &nbsp; </TD>
 * <TD>&nbsp; the name of the catalog. </TD>
 * </TR>
	
 * <TR>
 * <TD> deltaVel </TD> 
 * <TD> Speed </TD>
 * <TD>  &nbsp; </TD>
 * <TD>&nbsp; the velocity resolution. </TD>
 * </TR>
	
 * <TR>
 * <TD> position </TD> 
 * <TD> vector<Length > </TD>
 * <TD>  3  </TD>
 * <TD>&nbsp; the position of the source. </TD>
 * </TR>
	
 * <TR>
 * <TD> numLines </TD> 
 * <TD> int </TD>
 * <TD>  &nbsp; </TD>
 * <TD>&nbsp; the number of line transitions. </TD>
 * </TR>
	
 * <TR>
 * <TD> transition </TD> 
 * <TD> vector<string > </TD>
 * <TD>  numLines  </TD>
 * <TD>&nbsp; the names of the transitions. </TD>
 * </TR>
	
 * <TR>
 * <TD> restFrequency </TD> 
 * <TD> vector<Frequency > </TD>
 * <TD>  numLines  </TD>
 * <TD>&nbsp; the rest frequencies (one value per transition line). </TD>
 * </TR>
	
 * <TR>
 * <TD> sysVel </TD> 
 * <TD> vector<Speed > </TD>
 * <TD>  numLines  </TD>
 * <TD>&nbsp; the systemic velocity. </TD>
 * </TR>
	
 * <TR>
 * <TD> rangeVel </TD> 
 * <TD> vector<Speed > </TD>
 * <TD>  2  </TD>
 * <TD>&nbsp; the velocity range. </TD>
 * </TR>
	
 * <TR>
 * <TD> sourceModel </TD> 
 * <TD> SourceModelMod::SourceModel </TD>
 * <TD>  &nbsp; </TD>
 * <TD>&nbsp; identifies the source model. </TD>
 * </TR>
	
 * <TR>
 * <TD> frequencyRefCode </TD> 
 * <TD> FrequencyReferenceCodeMod::FrequencyReferenceCode </TD>
 * <TD>  &nbsp; </TD>
 * <TD>&nbsp; the frequency reference code. </TD>
 * </TR>
	
 * <TR>
 * <TD> numFreq </TD> 
 * <TD> int </TD>
 * <TD>  &nbsp; </TD>
 * <TD>&nbsp; the number of frequencies. </TD>
 * </TR>
	
 * <TR>
 * <TD> numStokes </TD> 
 * <TD> int </TD>
 * <TD>  &nbsp; </TD>
 * <TD>&nbsp; the number of Stokes parameters. </TD>
 * </TR>
	
 * <TR>
 * <TD> frequency </TD> 
 * <TD> vector<Frequency > </TD>
 * <TD>  numFreq  </TD>
 * <TD>&nbsp; the array of frequencies (one value per frequency). </TD>
 * </TR>
	
 * <TR>
 * <TD> frequencyInterval </TD> 
 * <TD> vector<Frequency > </TD>
 * <TD>  numFreq  </TD>
 * <TD>&nbsp; an array of frequency intervals (one value per interval). </TD>
 * </TR>
	
 * <TR>
 * <TD> stokesParameter </TD> 
 * <TD> vector<StokesParameterMod::StokesParameter > </TD>
 * <TD>  numStokes  </TD>
 * <TD>&nbsp; the array of Stokes parameters (one value per parameter). </TD>
 * </TR>
	
 * <TR>
 * <TD> flux </TD> 
 * <TD> vector<vector<Flux > > </TD>
 * <TD>  numFreq, numStokes  </TD>
 * <TD>&nbsp; the array of flux densities. </TD>
 * </TR>
	
 * <TR>
 * <TD> fluxErr </TD> 
 * <TD> vector<vector<Flux > > </TD>
 * <TD>  numFreq, numStokes  </TD>
 * <TD>&nbsp; the array of uncertainties on flux densities. </TD>
 * </TR>
	
 * <TR>
 * <TD> positionAngle </TD> 
 * <TD> vector<Angle > </TD>
 * <TD>  numFreq  </TD>
 * <TD>&nbsp; the major axis position angles (one value per frequency). </TD>
 * </TR>
	
 * <TR>
 * <TD> positionAngleErr </TD> 
 * <TD> vector<Angle > </TD>
 * <TD>  numFreq  </TD>
 * <TD>&nbsp; the uncertainties on major axis position angles. </TD>
 * </TR>
	
 * <TR>
 * <TD> size </TD> 
 * <TD> vector<vector<Angle > > </TD>
 * <TD>  numFreq, 2  </TD>
 * <TD>&nbsp; the sizes of source (one pair of values per frequency). </TD>
 * </TR>
	
 * <TR>
 * <TD> sizeErr </TD> 
 * <TD> vector<vector<Angle > > </TD>
 * <TD>  numFreq, 2  </TD>
 * <TD>&nbsp; the uncertainties on the source sizes (one pair of value per frequency). </TD>
 * </TR>
	

 * </TABLE>
 */
class SourceTable : public Representable {
	friend class ASDM;

public:


	/**
	 * Return the list of field names that make up key key
	 * as an array of strings.
	 * @return a vector of string.
	 */	
	static vector<string> getKeyName();


	virtual ~SourceTable();
	
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
	 * @return a pointer on a SourceRow
	 */
	SourceRow *newRow();
	
	
	/**
	 * Create a new row initialized to the specified values.
	 * @return a pointer on the created and initialized row.
	
 	 * @param timeInterval
	
 	 * @param spectralWindowId
	
 	 * @param code
	
 	 * @param direction
	
 	 * @param properMotion
	
 	 * @param sourceName
	
     */
	SourceRow *newRow(ArrayTimeInterval timeInterval, Tag spectralWindowId, string code, vector<Angle > direction, vector<AngularRate > properMotion, string sourceName);
	


	/**
	 * Create a new row using a copy constructor mechanism.
	 * 
	 * The method creates a new SourceRow owned by this. Each attribute of the created row 
	 * is a (deep) copy of the corresponding attribute of row. The method does not add 
	 * the created row to this, its simply parents it to this, a call to the add method
	 * has to be done in order to get the row added (very likely after having modified
	 * some of its attributes).
	 * If row is null then the method returns a new SourceRow with default values for its attributes. 
	 *
	 * @param row the row which is to be copied.
	 */
	 SourceRow *newRow(SourceRow *row); 

	//
	// ====> Append a row to its table.
	//

	
	 
	
	
	/** 
	 * Add a row.
	 * If there table contains a row whose key's fields except² sourceId are equal
	 * to x's ones then return a pointer on this row (i.e. no actual insertion is performed) 
	 * otherwise add x to the table and return x.
	 * @param x . A pointer on the row to be added.
 	 * @returns a pointer to a SourceRow.
	 */
	 
 	 SourceRow* add(SourceRow* x) ;



	//
	// ====> Methods returning rows.
	//
		
	/**
	 * Get all rows.
	 * @return Alls rows as a vector of pointers of SourceRow. The elements of this vector are stored in the order 
	 * in which they have been added to the SourceTable.
	 */
	vector<SourceRow *> get() ;
	


 
	
	/**
 	 * Returns a SourceRow* given a key.
 	 * @return a pointer to the row having the key whose values are passed as parameters, or 0 if
 	 * no row exists for that key.
	
	 * @param sourceId
	
	 * @param timeInterval
	
	 * @param spectralWindowId
	
 	 *
	 */
 	SourceRow* getRowByKey(int sourceId, ArrayTimeInterval timeInterval, Tag spectralWindowId);

 	 	
 	
	/**
 	 * Returns a vector of pointers on rows whose key element sourceId 
	 * is equal to the parameter sourceId.
	 * @return a vector of vector <SourceRow *>. A returned vector of size 0 means that no row has been found.
	 * @param sourceId int contains the value of
	 * the autoincrementable attribute that is looked up in the table.
	 */
 	vector <SourceRow *>  getRowBySourceId(int);



	/**
 	 * Look up the table for a row whose all attributes  except the autoincrementable one 
 	 * are equal to the corresponding parameters of the method.
 	 * @return a pointer on this row if any, null otherwise.
 	 *
			
 	 * @param timeInterval
 	 		
 	 * @param spectralWindowId
 	 		
 	 * @param code
 	 		
 	 * @param direction
 	 		
 	 * @param properMotion
 	 		
 	 * @param sourceName
 	 		 
 	 */
	SourceRow* lookup(ArrayTimeInterval timeInterval, Tag spectralWindowId, string code, vector<Angle > direction, vector<AngularRate > properMotion, string sourceName); 


private:

	/**
	 * Create a SourceTable.
	 * <p>
	 * This constructor is private because only the
	 * container can create tables.  All tables must know the container
	 * to which they belong.
	 * @param container The container to which this table belongs.
	 */ 
	SourceTable (ASDM & container);

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
	 
	 * @throws UniquenessViolationException
	 
	 */
	SourceRow* checkAndAdd(SourceRow* x) ;


	
	
	/**
	 * Insert a SourceRow* in a vector of SourceRow* so that it's ordered by ascending time.
	 *
	 * @param SourceRow* x . The pointer to be inserted.
	 * @param vector <SourceRow*>& row . A reference to the vector where to insert x.
	 *
	 */
	 SourceRow * insertByStartTime(SourceRow* x, vector<SourceRow* >& row);
	  


// A data structure to store the pointers on the table's rows.

// In all cases we maintain a private vector of SourceRow s.
   vector<SourceRow * > privateRows;
   

	

	
	
		
		
	typedef vector <vector <SourceRow* > > ID_TIME_ROWS;
	map<string, ID_TIME_ROWS > context;
	
	/** 
	 * Returns a string built by concatenating the ascii representation of the
	 * parameters values suffixed with a "_" character.
	 */
	 string Key(Tag spectralWindowId) ;
	 	
		
	
	
	/**
	 * Fills the vector vout (passed by reference) with pointers on elements of vin 
	 * whose attributes are equal to the corresponding parameters of the method.
	 *
	 */
	void getByKeyNoAutoIncNoTime(vector <SourceRow*>& vin, vector <SourceRow*>& vout,  Tag spectralWindowId);
	


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
	 * Convert this table into a SourceTableIDL CORBA structure.
	 *
	 * @return a pointer to a SourceTableIDL
	 */
	SourceTableIDL *toIDL() ;
#endif

#ifndef WITHOUT_ACS
	/**
	 * Populate this table from the content of a SourceTableIDL Corba structure.
	 *
	 * @throws DuplicateKey Thrown if the method tries to add a row having a key that is already in the table.
	 * @throws ConversionException
	 */	
	void fromIDL(SourceTableIDL x) ;
#endif
	
	
	void error() ; //throw(ConversionException);

	/**
	 * Translate this table to an XML representation conform
	 * to the schema defined for Source (SourceTable.xsd).
	 *
	 * @returns a string containing the XML representation.
	 * @throws ConversionException
	 */
	string toXML()  ;
	
	/**
	 * Populate this table from the content of a XML document that is required to
	 * be conform to the XML schema defined for a Source (SourceTable.xsd).
	 * @throws ConversionException
	 * 
	 */
	void fromXML(string xmlDoc) ;
		
	/**
	  * Private methods involved during the build of this table out of the content
	  * of file(s) containing an external representation of a Source table.
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
	  * will be saved in a file "Source.bin" or an XML representation (fileAsBin==false) will be saved in a file "Source.xml".
	  * The file is always written in a directory whose name is passed as a parameter.
	 * @param directory The name of directory  where the file containing the table's representation will be saved.
	  * 
	  */
	  void toFile(string directory);
	  
	/**
	 * Reads and parses a file containing a representation of a SourceTable as those produced  by the toFile method.
	 * This table is populated with the result of the parsing.
	 * @param directory The name of the directory containing the file te be read and parsed.
	 * @throws ConversionException If any error occurs while reading the 
	 * files in the directory or parsing them.
	 *
	 */
	 void setFromFile(const string& directory);	
 
};

} // End namespace asdm

#endif /* SourceTable_CLASS */
