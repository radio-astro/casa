
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
 * File CalFluxTable.h
 */
 
#ifndef CalFluxTable_CLASS
#define CalFluxTable_CLASS

#include <string>
#include <vector>
#include <map>
#include <set>
using std::string;
using std::vector;
using std::map;



#include <ArrayTime.h>
using  asdm::ArrayTime;

#include <Angle.h>
using  asdm::Angle;

#include <Tag.h>
using  asdm::Tag;

#include <Frequency.h>
using  asdm::Frequency;




	

	

	

	

	

	

	
#include "CFluxCalibrationMethod.h"
using namespace FluxCalibrationMethodMod;
	

	

	

	
#include "CStokesParameter.h"
using namespace StokesParameterMod;
	

	

	
#include "CDirectionReferenceCode.h"
using namespace DirectionReferenceCodeMod;
	

	

	

	

	

	

	
#include "CSourceModel.h"
using namespace SourceModelMod;
	



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
using asdmIDL::CalFluxTableIDL;
#endif

#include <Representable.h>

namespace asdm {

//class asdm::ASDM;
//class asdm::CalFluxRow;

class ASDM;
class CalFluxRow;
/**
 * The CalFluxTable class is an Alma table.
 * <BR>
 * 
 * \par Role
 * Result of flux calibration performed on-line by TelCal. Atmospheric absorption is corrected for. No ionosphere correction has been applied.
 * <BR>
 
 * Generated from model's revision "1.55", branch "HEAD"
 *
 * <TABLE BORDER="1">
 * <CAPTION> Attributes of CalFlux </CAPTION>
 * <TR BGCOLOR="#AAAAAA"> <TH> Name </TH> <TH> Type </TH> <TH> Expected shape  </TH> <TH> Comment </TH></TR>
 
 * <TR> <TH BGCOLOR="#CCCCCC" colspan="4" align="center"> Key </TD></TR>
	
 * <TR>
 		
 * <TD> sourceName </TD>
 		 
 * <TD> string</TD>
 * <TD> &nbsp; </TD>
 * <TD> &nbsp;the name of the source. </TD>
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
 * <TD> &nbsp;refers to a unique row in CalReduction Table. </TD>
 * </TR>
	


 * <TR> <TH BGCOLOR="#CCCCCC"  colspan="4" valign="center"> Value <br> (Mandatory) </TH></TR>
	
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
 * <TD> numFrequencyRanges </TD> 
 * <TD> int </TD>
 * <TD>  &nbsp;  </TD> 
 * <TD> &nbsp;the number of frequency ranges. </TD>
 * </TR>
	
 * <TR>
 * <TD> numStokes </TD> 
 * <TD> int </TD>
 * <TD>  &nbsp;  </TD> 
 * <TD> &nbsp;the number of Stokes parameters. </TD>
 * </TR>
	
 * <TR>
 * <TD> frequencyRanges </TD> 
 * <TD> vector<vector<Frequency > > </TD>
 * <TD>  numFrequencyRanges, 2 </TD> 
 * <TD> &nbsp;the frequency ranges (one pair of values per range). </TD>
 * </TR>
	
 * <TR>
 * <TD> fluxMethod </TD> 
 * <TD> FluxCalibrationMethodMod::FluxCalibrationMethod </TD>
 * <TD>  &nbsp;  </TD> 
 * <TD> &nbsp;identifies the flux determination method. </TD>
 * </TR>
	
 * <TR>
 * <TD> flux </TD> 
 * <TD> vector<vector<double > > </TD>
 * <TD>  numStokes, numFrequencyRanges </TD> 
 * <TD> &nbsp;the flux densities (one value par Stokes parameter per frequency range). </TD>
 * </TR>
	
 * <TR>
 * <TD> fluxError </TD> 
 * <TD> vector<vector<double > > </TD>
 * <TD>  numStokes, numFrequencyRanges </TD> 
 * <TD> &nbsp;the uncertainties on the flux densities (one value per Stokes parameter per frequency range). </TD>
 * </TR>
	
 * <TR>
 * <TD> stokes </TD> 
 * <TD> vector<StokesParameterMod::StokesParameter > </TD>
 * <TD>  numStokes </TD> 
 * <TD> &nbsp;the Stokes parameter. </TD>
 * </TR>
	


 * <TR> <TH BGCOLOR="#CCCCCC"  colspan="4" valign="center"> Value <br> (Optional) </TH></TR>
	
 * <TR>
 * <TD> direction </TD> 
 * <TD> vector<Angle > </TD>
 * <TD>  2  </TD>
 * <TD>&nbsp; the direction of the source. </TD>
 * </TR>
	
 * <TR>
 * <TD> directionCode </TD> 
 * <TD> DirectionReferenceCodeMod::DirectionReferenceCode </TD>
 * <TD>  &nbsp; </TD>
 * <TD>&nbsp; identifies the reference frame of the source's direction. </TD>
 * </TR>
	
 * <TR>
 * <TD> directionEquinox </TD> 
 * <TD> Angle </TD>
 * <TD>  &nbsp; </TD>
 * <TD>&nbsp; equinox associated with the reference frame of the source's direction. </TD>
 * </TR>
	
 * <TR>
 * <TD> PA </TD> 
 * <TD> vector<vector<Angle > > </TD>
 * <TD>  numStokes, numFrequencyRanges  </TD>
 * <TD>&nbsp; the position's angles for the source model (one value per Stokes parameter per frequency range). </TD>
 * </TR>
	
 * <TR>
 * <TD> PAError </TD> 
 * <TD> vector<vector<Angle > > </TD>
 * <TD>  numStokes, numFrequencyRanges  </TD>
 * <TD>&nbsp; the uncertainties on the position's angles (one value per Stokes parameter per frequency range). </TD>
 * </TR>
	
 * <TR>
 * <TD> size </TD> 
 * <TD> vector<vector<vector<Angle > > > </TD>
 * <TD>  numStokes, numFrequencyRanges, 2  </TD>
 * <TD>&nbsp; the sizes of the source (one pair of angles per Stokes parameter per frequency range). </TD>
 * </TR>
	
 * <TR>
 * <TD> sizeError </TD> 
 * <TD> vector<vector<vector<Angle > > > </TD>
 * <TD>  numStokes, numFrequencyRanges, 2  </TD>
 * <TD>&nbsp; the uncertainties of the sizes of the source (one pair of angles per Stokes parameter per frequency range). </TD>
 * </TR>
	
 * <TR>
 * <TD> sourceModel </TD> 
 * <TD> SourceModelMod::SourceModel </TD>
 * <TD>  &nbsp; </TD>
 * <TD>&nbsp; identifies the source model. </TD>
 * </TR>
	

 * </TABLE>
 */
class CalFluxTable : public Representable {
	friend class ASDM;

public:


	/**
	 * Return the list of field names that make up key key
	 * as an array of strings.
	 * @return a vector of string.
	 */	
	static vector<string> getKeyName();


	virtual ~CalFluxTable();
	
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
	 * to the schema defined for CalFlux (CalFluxTable.xsd).
	 *
	 * @returns a string containing the XML representation.
	 * @throws ConversionException
	 */
	string toXML()  ;

#ifndef WITHOUT_ACS
	// Conversion Methods
	/**
	 * Convert this table into a CalFluxTableIDL CORBA structure.
	 *
	 * @return a pointer to a CalFluxTableIDL
	 */
	CalFluxTableIDL *toIDL() ;
#endif

#ifndef WITHOUT_ACS
	/**
	 * Populate this table from the content of a CalFluxTableIDL Corba structure.
	 *
	 * @throws DuplicateKey Thrown if the method tries to add a row having a key that is already in the table.
	 * @throws ConversionException
	 */	
	void fromIDL(CalFluxTableIDL x) ;
#endif
	
	//
	// ====> Row creation.
	//
	
	/**
	 * Create a new row with default values.
	 * @return a pointer on a CalFluxRow
	 */
	CalFluxRow *newRow();
	
	
	/**
	 * Create a new row initialized to the specified values.
	 * @return a pointer on the created and initialized row.
	
 	 * @param sourceName
	
 	 * @param calDataId
	
 	 * @param calReductionId
	
 	 * @param startValidTime
	
 	 * @param endValidTime
	
 	 * @param numFrequencyRanges
	
 	 * @param numStokes
	
 	 * @param frequencyRanges
	
 	 * @param fluxMethod
	
 	 * @param flux
	
 	 * @param fluxError
	
 	 * @param stokes
	
     */
	CalFluxRow *newRow(string sourceName, Tag calDataId, Tag calReductionId, ArrayTime startValidTime, ArrayTime endValidTime, int numFrequencyRanges, int numStokes, vector<vector<Frequency > > frequencyRanges, FluxCalibrationMethodMod::FluxCalibrationMethod fluxMethod, vector<vector<double > > flux, vector<vector<double > > fluxError, vector<StokesParameterMod::StokesParameter > stokes);
	


	/**
	 * Create a new row using a copy constructor mechanism.
	 * 
	 * The method creates a new CalFluxRow owned by this. Each attribute of the created row 
	 * is a (deep) copy of the corresponding attribute of row. The method does not add 
	 * the created row to this, its simply parents it to this, a call to the add method
	 * has to be done in order to get the row added (very likely after having modified
	 * some of its attributes).
	 * If row is null then the method returns a new CalFluxRow with default values for its attributes. 
	 *
	 * @param row the row which is to be copied.
	 */
	 CalFluxRow *newRow(CalFluxRow *row); 

	//
	// ====> Append a row to its table.
	//
 
	
	/**
	 * Add a row.
	 * @param x a pointer to the CalFluxRow to be added.
	 *
	 * @return a pointer to a CalFluxRow. If the table contains a CalFluxRow whose attributes (key and mandatory values) are equal to x ones
	 * then returns a pointer on that CalFluxRow, otherwise returns x.
	 *
	 * @throw DuplicateKey { thrown when the table contains a CalFluxRow with a key equal to the x one but having
	 * and a value section different from x one }
	 *
	
	 */
	CalFluxRow* add(CalFluxRow* x) ; 

 



	//
	// ====> Methods returning rows.
	//
		
	/**
	 * Get all rows.
	 * @return Alls rows as a vector of pointers of CalFluxRow. The elements of this vector are stored in the order 
	 * in which they have been added to the CalFluxTable.
	 */
	vector<CalFluxRow *> get() ;
	


 
	
	/**
 	 * Returns a CalFluxRow* given a key.
 	 * @return a pointer to the row having the key whose values are passed as parameters, or 0 if
 	 * no row exists for that key.
	
	 * @param sourceName
	
	 * @param calDataId
	
	 * @param calReductionId
	
 	 *
	 */
 	CalFluxRow* getRowByKey(string sourceName, Tag calDataId, Tag calReductionId);

 	 	



	/**
 	 * Look up the table for a row whose all attributes 
 	 * are equal to the corresponding parameters of the method.
 	 * @return a pointer on this row if any, null otherwise.
 	 *
			
 	 * @param sourceName
 	 		
 	 * @param calDataId
 	 		
 	 * @param calReductionId
 	 		
 	 * @param startValidTime
 	 		
 	 * @param endValidTime
 	 		
 	 * @param numFrequencyRanges
 	 		
 	 * @param numStokes
 	 		
 	 * @param frequencyRanges
 	 		
 	 * @param fluxMethod
 	 		
 	 * @param flux
 	 		
 	 * @param fluxError
 	 		
 	 * @param stokes
 	 		 
 	 */
	CalFluxRow* lookup(string sourceName, Tag calDataId, Tag calReductionId, ArrayTime startValidTime, ArrayTime endValidTime, int numFrequencyRanges, int numStokes, vector<vector<Frequency > > frequencyRanges, FluxCalibrationMethodMod::FluxCalibrationMethod fluxMethod, vector<vector<double > > flux, vector<vector<double > > fluxError, vector<StokesParameterMod::StokesParameter > stokes); 


private:

	/**
	 * Create a CalFluxTable.
	 * <p>
	 * This constructor is private because only the
	 * container can create tables.  All tables must know the container
	 * to which they belong.
	 * @param container The container to which this table belongs.
	 */ 
	CalFluxTable (ASDM & container);

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
	CalFluxRow* checkAndAdd(CalFluxRow* x) ;



// A data structure to store the pointers on the table's rows.

// In all cases we maintain a private vector of CalFluxRow s.
   vector<CalFluxRow * > privateRows;
   

			
	vector<CalFluxRow *> row;

	
	void error() ; //throw(ConversionException);

	
	/**
	 * Populate this table from the content of a XML document that is required to
	 * be conform to the XML schema defined for a CalFlux (CalFluxTable.xsd).
	 * @throws ConversionException
	 * 
	 */
	void fromXML(string xmlDoc) ;
		
	/**
	  * Private methods involved during the build of this table out of the content
	  * of file(s) containing an external representation of a CalFlux table.
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
	  * will be saved in a file "CalFlux.bin" or an XML representation (fileAsBin==false) will be saved in a file "CalFlux.xml".
	  * The file is always written in a directory whose name is passed as a parameter.
	 * @param directory The name of directory  where the file containing the table's representation will be saved.
	  * 
	  */
	  void toFile(string directory);
	  
	/**
	 * Reads and parses a file containing a representation of a CalFluxTable as those produced  by the toFile method.
	 * This table is populated with the result of the parsing.
	 * @param directory The name of the directory containing the file te be read and parsed.
	 * @throws ConversionException If any error occurs while reading the 
	 * files in the directory or parsing them.
	 *
	 */
	 void setFromFile(const string& directory);	
 
};

} // End namespace asdm

#endif /* CalFluxTable_CLASS */
