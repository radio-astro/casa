
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
 * File SysCalTable.h
 */
 
#ifndef SysCalTable_CLASS
#define SysCalTable_CLASS

#include <string>
#include <vector>
#include <map>
#include <set>
using std::string;
using std::vector;
using std::map;



#include <Tag.h>
using  asdm::Tag;

#include <Temperature.h>
using  asdm::Temperature;

#include <ArrayTimeInterval.h>
using  asdm::ArrayTimeInterval;




	

	

	

	

	

	

	

	

	

	

	

	

	

	

	

	

	



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
using asdmIDL::SysCalTableIDL;
#endif

#include <Representable.h>

namespace asdm {

//class asdm::ASDM;
//class asdm::SysCalRow;

class ASDM;
class SysCalRow;
/**
 * The SysCalTable class is an Alma table.
 * <BR>
 * 
 * \par Role
 * System calibration. Gives information on the conversion  of data to temperature scale. This table is reduced to follow  the contents of the Measurement Set SysCal table. Use only spectral  values (use a single channel spectral window for single numbers).   \texttt{numChan} can be found in the SpectralWindow Table.  The contents of this table are used to scale the data in the filler.
 * <BR>
 
 * Generated from model's revision "1.55", branch "HEAD"
 *
 * <TABLE BORDER="1">
 * <CAPTION> Attributes of SysCal </CAPTION>
 * <TR BGCOLOR="#AAAAAA"> <TH> Name </TH> <TH> Type </TH> <TH> Expected shape  </TH> <TH> Comment </TH></TR>
 
 * <TR> <TH BGCOLOR="#CCCCCC" colspan="4" align="center"> Key </TD></TR>
	
 * <TR>
 		
 * <TD> antennaId </TD>
 		 
 * <TD> Tag</TD>
 * <TD> &nbsp; </TD>
 * <TD> &nbsp;refers to a unique row  in AntennaTable. </TD>
 * </TR>
	
 * <TR>
 		
 * <TD> spectralWindowId </TD>
 		 
 * <TD> Tag</TD>
 * <TD> &nbsp; </TD>
 * <TD> &nbsp;refers to a unique row in SpectralWindowTable. </TD>
 * </TR>
	
 * <TR>
 		
 * <TD> timeInterval </TD>
 		 
 * <TD> ArrayTimeInterval</TD>
 * <TD> &nbsp; </TD>
 * <TD> &nbsp;time interval for which the row's content is valid. </TD>
 * </TR>
	
 * <TR>
 		
 * <TD> feedId </TD>
 		 
 * <TD> int</TD>
 * <TD> &nbsp; </TD>
 * <TD> &nbsp;refers to a collection of rows in FeedTable. </TD>
 * </TR>
	


 * <TR> <TH BGCOLOR="#CCCCCC"  colspan="4" valign="center"> Value <br> (Mandatory) </TH></TR>
	
 * <TR>
 * <TD> numReceptor </TD> 
 * <TD> int </TD>
 * <TD>  &nbsp;  </TD> 
 * <TD> &nbsp;the number of receptors. </TD>
 * </TR>
	
 * <TR>
 * <TD> numChan </TD> 
 * <TD> int </TD>
 * <TD>  &nbsp;  </TD> 
 * <TD> &nbsp;the number of frequency channels. </TD>
 * </TR>
	


 * <TR> <TH BGCOLOR="#CCCCCC"  colspan="4" valign="center"> Value <br> (Optional) </TH></TR>
	
 * <TR>
 * <TD> tcalFlag </TD> 
 * <TD> bool </TD>
 * <TD>  &nbsp; </TD>
 * <TD>&nbsp; the calibration temperature flag. </TD>
 * </TR>
	
 * <TR>
 * <TD> tcalSpectrum </TD> 
 * <TD> vector<vector<Temperature > > </TD>
 * <TD>  numReceptor, numChan  </TD>
 * <TD>&nbsp; the calibration temperatures (one value per receptor per channel). </TD>
 * </TR>
	
 * <TR>
 * <TD> trxFlag </TD> 
 * <TD> bool </TD>
 * <TD>  &nbsp; </TD>
 * <TD>&nbsp; the receiver temperature flag. </TD>
 * </TR>
	
 * <TR>
 * <TD> trxSpectrum </TD> 
 * <TD> vector<vector<Temperature > > </TD>
 * <TD>  numReceptor, numChan  </TD>
 * <TD>&nbsp; the receiver temperatures (one value per receptor per channel). </TD>
 * </TR>
	
 * <TR>
 * <TD> tskyFlag </TD> 
 * <TD> bool </TD>
 * <TD>  &nbsp; </TD>
 * <TD>&nbsp; the sky temperature flag. </TD>
 * </TR>
	
 * <TR>
 * <TD> tskySpectrum </TD> 
 * <TD> vector<vector<Temperature > > </TD>
 * <TD>  numReceptor, numChan  </TD>
 * <TD>&nbsp; the sky temperatures (one value per receptor per channel). </TD>
 * </TR>
	
 * <TR>
 * <TD> tsysFlag </TD> 
 * <TD> bool </TD>
 * <TD>  &nbsp; </TD>
 * <TD>&nbsp; the system temperature flag. </TD>
 * </TR>
	
 * <TR>
 * <TD> tsysSpectrum </TD> 
 * <TD> vector<vector<Temperature > > </TD>
 * <TD>  numReceptor, numChan  </TD>
 * <TD>&nbsp; the system temperatures (one value per receptor per channel). </TD>
 * </TR>
	
 * <TR>
 * <TD> tantFlag </TD> 
 * <TD> bool </TD>
 * <TD>  &nbsp; </TD>
 * <TD>&nbsp; the tant flag. </TD>
 * </TR>
	
 * <TR>
 * <TD> tantSpectrum </TD> 
 * <TD> vector<vector<float > > </TD>
 * <TD>  numReceptor, numChan  </TD>
 * <TD>&nbsp; the Tant spectrum (one value per receptor per channel). </TD>
 * </TR>
	
 * <TR>
 * <TD> tantTsysFlag </TD> 
 * <TD> bool </TD>
 * <TD>  &nbsp; </TD>
 * <TD>&nbsp; the Tant/Tsys flag. </TD>
 * </TR>
	
 * <TR>
 * <TD> tantTsysSpectrum </TD> 
 * <TD> vector<vector<float > > </TD>
 * <TD>  numReceptor, numChan  </TD>
 * <TD>&nbsp; the Tant/Tsys spectrum(one value per receptor per channel) . </TD>
 * </TR>
	
 * <TR>
 * <TD> phaseDiffFlag </TD> 
 * <TD> bool </TD>
 * <TD>  &nbsp; </TD>
 * <TD>&nbsp; the phase difference flag. </TD>
 * </TR>
	
 * <TR>
 * <TD> phaseDiffSpectrum </TD> 
 * <TD> vector<vector<float > > </TD>
 * <TD>  numReceptor, numChan  </TD>
 * <TD>&nbsp; the phase difference spectrum (one value per receptor per channel). </TD>
 * </TR>
	

 * </TABLE>
 */
class SysCalTable : public Representable {
	friend class ASDM;

public:


	/**
	 * Return the list of field names that make up key key
	 * as an array of strings.
	 * @return a vector of string.
	 */	
	static vector<string> getKeyName();


	virtual ~SysCalTable();
	
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
	 * to the schema defined for SysCal (SysCalTable.xsd).
	 *
	 * @returns a string containing the XML representation.
	 * @throws ConversionException
	 */
	string toXML()  ;

#ifndef WITHOUT_ACS
	// Conversion Methods
	/**
	 * Convert this table into a SysCalTableIDL CORBA structure.
	 *
	 * @return a pointer to a SysCalTableIDL
	 */
	SysCalTableIDL *toIDL() ;
#endif

#ifndef WITHOUT_ACS
	/**
	 * Populate this table from the content of a SysCalTableIDL Corba structure.
	 *
	 * @throws DuplicateKey Thrown if the method tries to add a row having a key that is already in the table.
	 * @throws ConversionException
	 */	
	void fromIDL(SysCalTableIDL x) ;
#endif
	
	//
	// ====> Row creation.
	//
	
	/**
	 * Create a new row with default values.
	 * @return a pointer on a SysCalRow
	 */
	SysCalRow *newRow();
	
	
	/**
	 * Create a new row initialized to the specified values.
	 * @return a pointer on the created and initialized row.
	
 	 * @param antennaId
	
 	 * @param spectralWindowId
	
 	 * @param timeInterval
	
 	 * @param feedId
	
 	 * @param numReceptor
	
 	 * @param numChan
	
     */
	SysCalRow *newRow(Tag antennaId, Tag spectralWindowId, ArrayTimeInterval timeInterval, int feedId, int numReceptor, int numChan);
	


	/**
	 * Create a new row using a copy constructor mechanism.
	 * 
	 * The method creates a new SysCalRow owned by this. Each attribute of the created row 
	 * is a (deep) copy of the corresponding attribute of row. The method does not add 
	 * the created row to this, its simply parents it to this, a call to the add method
	 * has to be done in order to get the row added (very likely after having modified
	 * some of its attributes).
	 * If row is null then the method returns a new SysCalRow with default values for its attributes. 
	 *
	 * @param row the row which is to be copied.
	 */
	 SysCalRow *newRow(SysCalRow *row); 

	//
	// ====> Append a row to its table.
	//
 
	
	/**
	 * Add a row.
	 * @param x a pointer to the SysCalRow to be added.
	 *
	 * @return a pointer to a SysCalRow. If the table contains a SysCalRow whose attributes (key and mandatory values) are equal to x ones
	 * then returns a pointer on that SysCalRow, otherwise returns x.
	 *
	 * @throw DuplicateKey { thrown when the table contains a SysCalRow with a key equal to the x one but having
	 * and a value section different from x one }
	 *
	
	 * @note The row is inserted in the table in such a way that all the rows having the same value of
	 * ( antennaId, spectralWindowId, feedId ) are stored by ascending time.
	 * @see method getByContext.
	
	 */
	SysCalRow* add(SysCalRow* x) ; 

 



	//
	// ====> Methods returning rows.
	//
		
	/**
	 * Get all rows.
	 * @return Alls rows as a vector of pointers of SysCalRow. The elements of this vector are stored in the order 
	 * in which they have been added to the SysCalTable.
	 */
	vector<SysCalRow *> get() ;
	

	/**
	 * Returns all the rows sorted by ascending startTime for a given context. 
	 * The context is defined by a value of ( antennaId, spectralWindowId, feedId ).
	 *
	 * @return a pointer on a vector<SysCalRow *>. A null returned value means that the table contains
	 * no SysCalRow for the given ( antennaId, spectralWindowId, feedId ).
	 */
	 vector <SysCalRow*> *getByContext(Tag antennaId, Tag spectralWindowId, int feedId);
	 


 
	
	/**
 	 * Returns a SysCalRow* given a key.
 	 * @return a pointer to the row having the key whose values are passed as parameters, or 0 if
 	 * no row exists for that key.
	
	 * @param antennaId
	
	 * @param spectralWindowId
	
	 * @param timeInterval
	
	 * @param feedId
	
 	 *
	 */
 	SysCalRow* getRowByKey(Tag antennaId, Tag spectralWindowId, ArrayTimeInterval timeInterval, int feedId);

 	 	



	/**
 	 * Look up the table for a row whose all attributes 
 	 * are equal to the corresponding parameters of the method.
 	 * @return a pointer on this row if any, null otherwise.
 	 *
			
 	 * @param antennaId
 	 		
 	 * @param spectralWindowId
 	 		
 	 * @param timeInterval
 	 		
 	 * @param feedId
 	 		
 	 * @param numReceptor
 	 		
 	 * @param numChan
 	 		 
 	 */
	SysCalRow* lookup(Tag antennaId, Tag spectralWindowId, ArrayTimeInterval timeInterval, int feedId, int numReceptor, int numChan); 


private:

	/**
	 * Create a SysCalTable.
	 * <p>
	 * This constructor is private because only the
	 * container can create tables.  All tables must know the container
	 * to which they belong.
	 * @param container The container to which this table belongs.
	 */ 
	SysCalTable (ASDM & container);

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
	SysCalRow* checkAndAdd(SysCalRow* x) ;


	
	
	/**
	 * Insert a SysCalRow* in a vector of SysCalRow* so that it's ordered by ascending time.
	 *
	 * @param SysCalRow* x . The pointer to be inserted.
	 * @param vector <SysCalRow*>& row . A reference to the vector where to insert x.
	 *
	 */
	 SysCalRow * insertByStartTime(SysCalRow* x, vector<SysCalRow* >& row);
	  


// A data structure to store the pointers on the table's rows.

// In all cases we maintain a private vector of SysCalRow s.
   vector<SysCalRow * > privateRows;
   

	

	
	
		
				
	typedef vector <SysCalRow* > TIME_ROWS;
	map<string, TIME_ROWS > context;
		
	/** 
	 * Returns a string built by concatenating the ascii representation of the
	 * parameters values suffixed with a "_" character.
	 */
	 string Key(Tag antennaId, Tag spectralWindowId, int feedId) ;
		 
		
	
	
	/**
	 * Fills the vector vout (passed by reference) with pointers on elements of vin 
	 * whose attributes are equal to the corresponding parameters of the method.
	 *
	 */
	void getByKeyNoAutoIncNoTime(vector <SysCalRow*>& vin, vector <SysCalRow*>& vout,  Tag antennaId, Tag spectralWindowId, int feedId);
	

	
	void error() ; //throw(ConversionException);

	
	/**
	 * Populate this table from the content of a XML document that is required to
	 * be conform to the XML schema defined for a SysCal (SysCalTable.xsd).
	 * @throws ConversionException
	 * 
	 */
	void fromXML(string xmlDoc) ;
		
	/**
	  * Private methods involved during the build of this table out of the content
	  * of file(s) containing an external representation of a SysCal table.
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
	  * will be saved in a file "SysCal.bin" or an XML representation (fileAsBin==false) will be saved in a file "SysCal.xml".
	  * The file is always written in a directory whose name is passed as a parameter.
	 * @param directory The name of directory  where the file containing the table's representation will be saved.
	  * 
	  */
	  void toFile(string directory);
	  
	/**
	 * Reads and parses a file containing a representation of a SysCalTable as those produced  by the toFile method.
	 * This table is populated with the result of the parsing.
	 * @param directory The name of the directory containing the file te be read and parsed.
	 * @throws ConversionException If any error occurs while reading the 
	 * files in the directory or parsing them.
	 *
	 */
	 void setFromFile(const string& directory);	
 
};

} // End namespace asdm

#endif /* SysCalTable_CLASS */
