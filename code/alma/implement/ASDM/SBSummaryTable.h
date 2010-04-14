
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
 * File SBSummaryTable.h
 */
 
#ifndef SBSummaryTable_CLASS
#define SBSummaryTable_CLASS

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

#include <Interval.h>
using  asdm::Interval;

#include <Tag.h>
using  asdm::Tag;

#include <EntityRef.h>
using  asdm::EntityRef;




	

	

	

	

	

	
#include "CReceiverBand.h"
using namespace ReceiverBandMod;
	

	
#include "CSBType.h"
using namespace SBTypeMod;
	

	

	

	

	

	

	

	

	

	

	
#include "CDirectionReferenceCode.h"
using namespace DirectionReferenceCodeMod;
	

	



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
using asdmIDL::SBSummaryTableIDL;
#endif

#include <Representable.h>

namespace asdm {

//class asdm::ASDM;
//class asdm::SBSummaryRow;

class ASDM;
class SBSummaryRow;
/**
 * The SBSummaryTable class is an Alma table.
 * <BR>
 * 
 * \par Role
 * Characteristics of the Scheduling Block that has been executed. Much of the  data here is reproduced from the Scheduling block itself.
 * <BR>
 
 * Generated from model's revision "1.53", branch "HEAD"
 *
 * <TABLE BORDER="1">
 * <CAPTION> Attributes of SBSummary </CAPTION>
 * <TR BGCOLOR="#AAAAAA"> <TH> Name </TH> <TH> Type </TH> <TH> Expected shape  </TH> <TH> Comment </TH></TR>
 
 * <TR> <TH BGCOLOR="#CCCCCC" colspan="4" align="center"> Key </TD></TR>
	
 * <TR>
 		
 * <TD><I> sBSummaryId </I></TD>
 		 
 * <TD> Tag</TD>
 * <TD> &nbsp; </TD>
 * <TD> &nbsp;refers to a unique row in the table. </TD>
 * </TR>
	


 * <TR> <TH BGCOLOR="#CCCCCC"  colspan="4" valign="center"> Value <br> (Mandarory) </TH></TR>
	
 * <TR>
 * <TD> sbSummaryUID </TD> 
 * <TD> EntityRef </TD>
 * <TD>  &nbsp;  </TD> 
 * <TD> &nbsp;the scheduling block archive's UID. </TD>
 * </TR>
	
 * <TR>
 * <TD> projectUID </TD> 
 * <TD> EntityRef </TD>
 * <TD>  &nbsp;  </TD> 
 * <TD> &nbsp;the projet archive's UID. </TD>
 * </TR>
	
 * <TR>
 * <TD> obsUnitSetId </TD> 
 * <TD> EntityRef </TD>
 * <TD>  &nbsp;  </TD> 
 * <TD> &nbsp;the observing unit set archive's UID. </TD>
 * </TR>
	
 * <TR>
 * <TD> frequency </TD> 
 * <TD> double </TD>
 * <TD>  &nbsp;  </TD> 
 * <TD> &nbsp;a representative frequency. </TD>
 * </TR>
	
 * <TR>
 * <TD> frequencyBand </TD> 
 * <TD> ReceiverBandMod::ReceiverBand </TD>
 * <TD>  &nbsp;  </TD> 
 * <TD> &nbsp;the frequency band. </TD>
 * </TR>
	
 * <TR>
 * <TD> sbType </TD> 
 * <TD> SBTypeMod::SBType </TD>
 * <TD>  &nbsp;  </TD> 
 * <TD> &nbsp;the type of scheduling block. </TD>
 * </TR>
	
 * <TR>
 * <TD> sbDuration </TD> 
 * <TD> Interval </TD>
 * <TD>  &nbsp;  </TD> 
 * <TD> &nbsp;the duration of the scheduling block. </TD>
 * </TR>
	
 * <TR>
 * <TD> centerDirection </TD> 
 * <TD> vector<Angle > </TD>
 * <TD>  2 </TD> 
 * <TD> &nbsp;the representative target direction. </TD>
 * </TR>
	
 * <TR>
 * <TD> numObservingMode </TD> 
 * <TD> int </TD>
 * <TD>  &nbsp;  </TD> 
 * <TD> &nbsp;the number of observing modes. </TD>
 * </TR>
	
 * <TR>
 * <TD> observingMode </TD> 
 * <TD> vector<string > </TD>
 * <TD>  numObservingMode </TD> 
 * <TD> &nbsp;the observing modes. </TD>
 * </TR>
	
 * <TR>
 * <TD> numberRepeats </TD> 
 * <TD> int </TD>
 * <TD>  &nbsp;  </TD> 
 * <TD> &nbsp;the number of repeats. </TD>
 * </TR>
	
 * <TR>
 * <TD> numScienceGoal </TD> 
 * <TD> int </TD>
 * <TD>  &nbsp;  </TD> 
 * <TD> &nbsp;the number of scientific goals. </TD>
 * </TR>
	
 * <TR>
 * <TD> scienceGoal </TD> 
 * <TD> vector<string > </TD>
 * <TD>  numScienceGoal </TD> 
 * <TD> &nbsp;the scientific goals. </TD>
 * </TR>
	
 * <TR>
 * <TD> numWeatherConstraint </TD> 
 * <TD> int </TD>
 * <TD>  &nbsp;  </TD> 
 * <TD> &nbsp;the number of weather constraints. </TD>
 * </TR>
	
 * <TR>
 * <TD> weatherConstraint </TD> 
 * <TD> vector<string > </TD>
 * <TD>  numWeatherConstraint </TD> 
 * <TD> &nbsp;the weather constraints. </TD>
 * </TR>
	


 * <TR> <TH BGCOLOR="#CCCCCC"  colspan="4" valign="center"> Value <br> (Optional) </TH></TR>
	
 * <TR>
 * <TD> centerDirectionCode </TD> 
 * <TD> DirectionReferenceCodeMod::DirectionReferenceCode </TD>
 * <TD>  &nbsp; </TD>
 * <TD>&nbsp; identifies the direction reference frame associated with centerDirection. </TD>
 * </TR>
	
 * <TR>
 * <TD> centerDirectionEquinox </TD> 
 * <TD> ArrayTime </TD>
 * <TD>  &nbsp; </TD>
 * <TD>&nbsp; the equinox associated to centerDirectionReferenceCode (if needed). </TD>
 * </TR>
	

 * </TABLE>
 */
class SBSummaryTable : public Representable {
	friend class ASDM;

public:


	/**
	 * Return the list of field names that make up key key
	 * as an array of strings.
	 * @return a vector of string.
	 */	
	static vector<string> getKeyName();


	virtual ~SBSummaryTable();
	
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
		
	/**
	 * Produces an XML representation conform
	 * to the schema defined for SBSummary (SBSummaryTable.xsd).
	 *
	 * @returns a string containing the XML representation.
	 * @throws ConversionException
	 */
	string toXML()  ;

#ifndef WITHOUT_ACS
	// Conversion Methods
	/**
	 * Convert this table into a SBSummaryTableIDL CORBA structure.
	 *
	 * @return a pointer to a SBSummaryTableIDL
	 */
	SBSummaryTableIDL *toIDL() ;
#endif

#ifndef WITHOUT_ACS
	/**
	 * Populate this table from the content of a SBSummaryTableIDL Corba structure.
	 *
	 * @throws DuplicateKey Thrown if the method tries to add a row having a key that is already in the table.
	 * @throws ConversionException
	 */	
	void fromIDL(SBSummaryTableIDL x) ;
#endif
	
	//
	// ====> Row creation.
	//
	
	/**
	 * Create a new row with default values.
	 * @return a pointer on a SBSummaryRow
	 */
	SBSummaryRow *newRow();
	
	
	/**
	 * Create a new row initialized to the specified values.
	 * @return a pointer on the created and initialized row.
	
 	 * @param sbSummaryUID
	
 	 * @param projectUID
	
 	 * @param obsUnitSetId
	
 	 * @param frequency
	
 	 * @param frequencyBand
	
 	 * @param sbType
	
 	 * @param sbDuration
	
 	 * @param centerDirection
	
 	 * @param numObservingMode
	
 	 * @param observingMode
	
 	 * @param numberRepeats
	
 	 * @param numScienceGoal
	
 	 * @param scienceGoal
	
 	 * @param numWeatherConstraint
	
 	 * @param weatherConstraint
	
     */
	SBSummaryRow *newRow(EntityRef sbSummaryUID, EntityRef projectUID, EntityRef obsUnitSetId, double frequency, ReceiverBandMod::ReceiverBand frequencyBand, SBTypeMod::SBType sbType, Interval sbDuration, vector<Angle > centerDirection, int numObservingMode, vector<string > observingMode, int numberRepeats, int numScienceGoal, vector<string > scienceGoal, int numWeatherConstraint, vector<string > weatherConstraint);
	


	/**
	 * Create a new row using a copy constructor mechanism.
	 * 
	 * The method creates a new SBSummaryRow owned by this. Each attribute of the created row 
	 * is a (deep) copy of the corresponding attribute of row. The method does not add 
	 * the created row to this, its simply parents it to this, a call to the add method
	 * has to be done in order to get the row added (very likely after having modified
	 * some of its attributes).
	 * If row is null then the method returns a new SBSummaryRow with default values for its attributes. 
	 *
	 * @param row the row which is to be copied.
	 */
	 SBSummaryRow *newRow(SBSummaryRow *row); 

	//
	// ====> Append a row to its table.
	//

	
	
	
	/** 
	 * Add a row.
	 * If there table contains a row whose key's fields are equal
	 * to x's ones then return a pointer on this row (i.e. no actual insertion is performed) 
	 * otherwise add x to the table and return x.
	 * @param x . A pointer on the row to be added.
 	 * @returns a pointer to a SBSummaryRow.	 
	 */	 
	 
 	 SBSummaryRow* add(SBSummaryRow* x) ;



	//
	// ====> Methods returning rows.
	//
		
	/**
	 * Get all rows.
	 * @return Alls rows as a vector of pointers of SBSummaryRow. The elements of this vector are stored in the order 
	 * in which they have been added to the SBSummaryTable.
	 */
	vector<SBSummaryRow *> get() ;
	


 
	
	/**
 	 * Returns a SBSummaryRow* given a key.
 	 * @return a pointer to the row having the key whose values are passed as parameters, or 0 if
 	 * no row exists for that key.
	
	 * @param sBSummaryId
	
 	 *
	 */
 	SBSummaryRow* getRowByKey(Tag sBSummaryId);

 	 	



	/**
 	 * Look up the table for a row whose all attributes  except the autoincrementable one 
 	 * are equal to the corresponding parameters of the method.
 	 * @return a pointer on this row if any, null otherwise.
 	 *
			
 	 * @param sbSummaryUID
 	 		
 	 * @param projectUID
 	 		
 	 * @param obsUnitSetId
 	 		
 	 * @param frequency
 	 		
 	 * @param frequencyBand
 	 		
 	 * @param sbType
 	 		
 	 * @param sbDuration
 	 		
 	 * @param centerDirection
 	 		
 	 * @param numObservingMode
 	 		
 	 * @param observingMode
 	 		
 	 * @param numberRepeats
 	 		
 	 * @param numScienceGoal
 	 		
 	 * @param scienceGoal
 	 		
 	 * @param numWeatherConstraint
 	 		
 	 * @param weatherConstraint
 	 		 
 	 */
	SBSummaryRow* lookup(EntityRef sbSummaryUID, EntityRef projectUID, EntityRef obsUnitSetId, double frequency, ReceiverBandMod::ReceiverBand frequencyBand, SBTypeMod::SBType sbType, Interval sbDuration, vector<Angle > centerDirection, int numObservingMode, vector<string > observingMode, int numberRepeats, int numScienceGoal, vector<string > scienceGoal, int numWeatherConstraint, vector<string > weatherConstraint); 


private:

	/**
	 * Create a SBSummaryTable.
	 * <p>
	 * This constructor is private because only the
	 * container can create tables.  All tables must know the container
	 * to which they belong.
	 * @param container The container to which this table belongs.
	 */ 
	SBSummaryTable (ASDM & container);

	ASDM & container;
	
	bool archiveAsBin; // If true archive binary else archive XML
	bool fileAsBin ; // If true file binary else file XML	
	
	Entity entity;
	

	// A map for the autoincrementation algorithm
	map<string,int>  noAutoIncIds;
	void autoIncrement(string key, SBSummaryRow* x);


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
	SBSummaryRow* checkAndAdd(SBSummaryRow* x) ;



// A data structure to store the pointers on the table's rows.

// In all cases we maintain a private vector of SBSummaryRow s.
   vector<SBSummaryRow * > privateRows;
   

			
	vector<SBSummaryRow *> row;

	
	void error() ; //throw(ConversionException);

	
	/**
	 * Populate this table from the content of a XML document that is required to
	 * be conform to the XML schema defined for a SBSummary (SBSummaryTable.xsd).
	 * @throws ConversionException
	 * 
	 */
	void fromXML(string xmlDoc) ;
		
	/**
	  * Private methods involved during the build of this table out of the content
	  * of file(s) containing an external representation of a SBSummary table.
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
	  * will be saved in a file "SBSummary.bin" or an XML representation (fileAsBin==false) will be saved in a file "SBSummary.xml".
	  * The file is always written in a directory whose name is passed as a parameter.
	 * @param directory The name of directory  where the file containing the table's representation will be saved.
	  * 
	  */
	  void toFile(string directory);
	  
	/**
	 * Reads and parses a file containing a representation of a SBSummaryTable as those produced  by the toFile method.
	 * This table is populated with the result of the parsing.
	 * @param directory The name of the directory containing the file te be read and parsed.
	 * @throws ConversionException If any error occurs while reading the 
	 * files in the directory or parsing them.
	 *
	 */
	 void setFromFile(const string& directory);	
 
};

} // End namespace asdm

#endif /* SBSummaryTable_CLASS */
