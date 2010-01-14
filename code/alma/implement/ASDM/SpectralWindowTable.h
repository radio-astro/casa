
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
 * File SpectralWindowTable.h
 */
 
#ifndef SpectralWindowTable_CLASS
#define SpectralWindowTable_CLASS

#include <string>
#include <vector>
#include <map>
#include <set>
using std::string;
using std::vector;
using std::map;



#include <Tag.h>
using  asdm::Tag;

#include <Frequency.h>
using  asdm::Frequency;




	

	
#include "CBasebandName.h"
using namespace BasebandNameMod;
	

	
#include "CNetSideband.h"
using namespace NetSidebandMod;
	

	

	

	
#include "CSidebandProcessingMode.h"
using namespace SidebandProcessingModeMod;
	

	

	
#include "CWindowFunction.h"
using namespace WindowFunctionMod;
	

	

	

	

	

	

	
#include "CCorrelationBit.h"
using namespace CorrelationBitMod;
	

	

	

	

	

	

	
#include "CFrequencyReferenceCode.h"
using namespace FrequencyReferenceCodeMod;
	

	

	

	

	

	

	

	

	
#include "CSpectralResolutionType.h"
using namespace SpectralResolutionTypeMod;
	



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
using asdmIDL::SpectralWindowTableIDL;
#endif

#include <Representable.h>

namespace asdm {

//class asdm::ASDM;
//class asdm::SpectralWindowRow;

class ASDM;
class SpectralWindowRow;
/**
 * The SpectralWindowTable class is an Alma table.
 * <BR>
 * 
 * \par Role
 * Spectral window description. The convention in ALMA is to describe the  frequency axis in the topocentric reference frame. If this is not  the case (for instance if active Doppler tracking is implemented) then  \texttt{measFreqRef} should be set accordingly.
 * <BR>
 
 * Generated from model's revision "1.53", branch "HEAD"
 *
 * <TABLE BORDER="1">
 * <CAPTION> Attributes of SpectralWindow </CAPTION>
 * <TR BGCOLOR="#AAAAAA"> <TH> Name </TH> <TH> Type </TH> <TH> Expected shape  </TH> <TH> Comment </TH></TR>
 
 * <TR> <TH BGCOLOR="#CCCCCC" colspan="4" align="center"> Key </TD></TR>
	
 * <TR>
 		
 * <TD><I> spectralWindowId </I></TD>
 		 
 * <TD> Tag</TD>
 * <TD> &nbsp; </TD>
 * <TD> &nbsp;identifies a unique row in the table. </TD>
 * </TR>
	


 * <TR> <TH BGCOLOR="#CCCCCC"  colspan="4" valign="center"> Value <br> (Mandarory) </TH></TR>
	
 * <TR>
 * <TD> basebandName </TD> 
 * <TD> BasebandNameMod::BasebandName </TD>
 * <TD>  &nbsp;  </TD> 
 * <TD> &nbsp;identifies the baseband. </TD>
 * </TR>
	
 * <TR>
 * <TD> netSideband </TD> 
 * <TD> NetSidebandMod::NetSideband </TD>
 * <TD>  &nbsp;  </TD> 
 * <TD> &nbsp;identifies the net sideband. </TD>
 * </TR>
	
 * <TR>
 * <TD> numChan </TD> 
 * <TD> int </TD>
 * <TD>  &nbsp;  </TD> 
 * <TD> &nbsp;the number of frequency channels. </TD>
 * </TR>
	
 * <TR>
 * <TD> refFreq </TD> 
 * <TD> Frequency </TD>
 * <TD>  &nbsp;  </TD> 
 * <TD> &nbsp;the reference frequency. </TD>
 * </TR>
	
 * <TR>
 * <TD> sidebandProcessingMode </TD> 
 * <TD> SidebandProcessingModeMod::SidebandProcessingMode </TD>
 * <TD>  &nbsp;  </TD> 
 * <TD> &nbsp;identifies the sideband processing mode. </TD>
 * </TR>
	
 * <TR>
 * <TD> totBandwidth </TD> 
 * <TD> Frequency </TD>
 * <TD>  &nbsp;  </TD> 
 * <TD> &nbsp;the total bandwidth. </TD>
 * </TR>
	
 * <TR>
 * <TD> windowFunction </TD> 
 * <TD> WindowFunctionMod::WindowFunction </TD>
 * <TD>  &nbsp;  </TD> 
 * <TD> &nbsp;identifies the window function. </TD>
 * </TR>
	


 * <TR> <TH BGCOLOR="#CCCCCC"  colspan="4" valign="center"> Value <br> (Optional) </TH></TR>
	
 * <TR>
 * <TD> chanFreqStart </TD> 
 * <TD> Frequency </TD>
 * <TD>  &nbsp; </TD>
 * <TD>&nbsp; the frequency of the first channel. </TD>
 * </TR>
	
 * <TR>
 * <TD> chanFreqStep </TD> 
 * <TD> Frequency </TD>
 * <TD>  &nbsp; </TD>
 * <TD>&nbsp; the increment between two successive frequencies. </TD>
 * </TR>
	
 * <TR>
 * <TD> chanFreqArray </TD> 
 * <TD> vector<Frequency > </TD>
 * <TD>  numChan  </TD>
 * <TD>&nbsp; the frequencies defined as an array (\texttt{numChan} values). </TD>
 * </TR>
	
 * <TR>
 * <TD> chanWidth </TD> 
 * <TD> Frequency </TD>
 * <TD>  &nbsp; </TD>
 * <TD>&nbsp; the width of the frequency channel (supposedly constant). </TD>
 * </TR>
	
 * <TR>
 * <TD> chanWidthArray </TD> 
 * <TD> vector<Frequency > </TD>
 * <TD>  numChan  </TD>
 * <TD>&nbsp; Array of channel widths </TD>
 * </TR>
	
 * <TR>
 * <TD> correlationBit </TD> 
 * <TD> CorrelationBitMod::CorrelationBit </TD>
 * <TD>  &nbsp; </TD>
 * <TD>&nbsp; identifies the number of bits used in the signal representation. </TD>
 * </TR>
	
 * <TR>
 * <TD> effectiveBw </TD> 
 * <TD> Frequency </TD>
 * <TD>  &nbsp; </TD>
 * <TD>&nbsp; the effective noise bandwidth. </TD>
 * </TR>
	
 * <TR>
 * <TD> effectiveBwArray </TD> 
 * <TD> vector<Frequency > </TD>
 * <TD>  numChan  </TD>
 * <TD>&nbsp; array of effective bandwidths (one value per channel). </TD>
 * </TR>
	
 * <TR>
 * <TD> freqGroup </TD> 
 * <TD> int </TD>
 * <TD>  &nbsp; </TD>
 * <TD>&nbsp; the frequency group number. </TD>
 * </TR>
	
 * <TR>
 * <TD> freqGroupName </TD> 
 * <TD> string </TD>
 * <TD>  &nbsp; </TD>
 * <TD>&nbsp; the frequency group name. </TD>
 * </TR>
	
 * <TR>
 * <TD> lineArray </TD> 
 * <TD> vector<bool > </TD>
 * <TD>  numChan  </TD>
 * <TD>&nbsp; indicates lines (true) versus baselines (false). </TD>
 * </TR>
	
 * <TR>
 * <TD> measFreqRef </TD> 
 * <TD> FrequencyReferenceCodeMod::FrequencyReferenceCode </TD>
 * <TD>  &nbsp; </TD>
 * <TD>&nbsp; the reference frame of the frequencies. </TD>
 * </TR>
	
 * <TR>
 * <TD> name </TD> 
 * <TD> string </TD>
 * <TD>  &nbsp; </TD>
 * <TD>&nbsp; a name for this spectral window. </TD>
 * </TR>
	
 * <TR>
 * <TD> oversampling </TD> 
 * <TD> bool </TD>
 * <TD>  &nbsp; </TD>
 * <TD>&nbsp; data are "oversampled" (true) or not (false). </TD>
 * </TR>
	
 * <TR>
 * <TD> quantization </TD> 
 * <TD> bool </TD>
 * <TD>  &nbsp; </TD>
 * <TD>&nbsp; a quantization correction has been applied (true) or not applied (false). </TD>
 * </TR>
	
 * <TR>
 * <TD> refChan </TD> 
 * <TD> double </TD>
 * <TD>  &nbsp; </TD>
 * <TD>&nbsp; the reference channel "number". </TD>
 * </TR>
	
 * <TR>
 * <TD> resolution </TD> 
 * <TD> Frequency </TD>
 * <TD>  &nbsp; </TD>
 * <TD>&nbsp; the half power frequency resolution (supposedly constant for all the channels). </TD>
 * </TR>
	
 * <TR>
 * <TD> resolutionArray </TD> 
 * <TD> vector<Frequency > </TD>
 * <TD>  numChan  </TD>
 * <TD>&nbsp; the frequency resolutions (possibly variable )(one value per channel). </TD>
 * </TR>
	
 * <TR>
 * <TD> numAssocValues </TD> 
 * <TD> int </TD>
 * <TD>  &nbsp; </TD>
 * <TD>&nbsp; the number of associated values. </TD>
 * </TR>
	
 * <TR>
 * <TD> assocNature </TD> 
 * <TD> vector<SpectralResolutionTypeMod::SpectralResolutionType > </TD>
 * <TD>  numAssocValues  </TD>
 * <TD>&nbsp; the natures of the associations with the rows refered to by assocSpectralWindowId. </TD>
 * </TR>
	
 * <TR>
 * <TD> assocSpectralWindowId </TD> 
 * <TD> vector<Tag>  </TD>
 * <TD>  numAssocValues  </TD>
 * <TD>&nbsp; refers to a collection of associated rows in the table. </TD>
 * </TR>
	
 * <TR>
 * <TD> imageSpectralWindowId </TD> 
 * <TD> Tag </TD>
 * <TD>  &nbsp; </TD>
 * <TD>&nbsp; refers to a unique row in the table (image sideband description). </TD>
 * </TR>
	
 * <TR>
 * <TD> dopplerId </TD> 
 * <TD> int </TD>
 * <TD>  &nbsp; </TD>
 * <TD>&nbsp; refers to a collection of rows in DopplerTable. </TD>
 * </TR>
	

 * </TABLE>
 */
class SpectralWindowTable : public Representable {
	friend class ASDM;

public:


	/**
	 * Return the list of field names that make up key key
	 * as an array of strings.
	 * @return a vector of string.
	 */	
	static vector<string> getKeyName();


	virtual ~SpectralWindowTable();
	
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
	 * @return a pointer on a SpectralWindowRow
	 */
	SpectralWindowRow *newRow();
	
	
	/**
	 * Create a new row initialized to the specified values.
	 * @return a pointer on the created and initialized row.
	
 	 * @param basebandName
	
 	 * @param netSideband
	
 	 * @param numChan
	
 	 * @param refFreq
	
 	 * @param sidebandProcessingMode
	
 	 * @param totBandwidth
	
 	 * @param windowFunction
	
     */
	SpectralWindowRow *newRow(BasebandNameMod::BasebandName basebandName, NetSidebandMod::NetSideband netSideband, int numChan, Frequency refFreq, SidebandProcessingModeMod::SidebandProcessingMode sidebandProcessingMode, Frequency totBandwidth, WindowFunctionMod::WindowFunction windowFunction);
	


	/**
	 * Create a new row using a copy constructor mechanism.
	 * 
	 * The method creates a new SpectralWindowRow owned by this. Each attribute of the created row 
	 * is a (deep) copy of the corresponding attribute of row. The method does not add 
	 * the created row to this, its simply parents it to this, a call to the add method
	 * has to be done in order to get the row added (very likely after having modified
	 * some of its attributes).
	 * If row is null then the method returns a new SpectralWindowRow with default values for its attributes. 
	 *
	 * @param row the row which is to be copied.
	 */
	 SpectralWindowRow *newRow(SpectralWindowRow *row); 

	//
	// ====> Append a row to its table.
	//

	
	
	
	/** 
	 * Add a row.
	 * If there table contains a row whose key's fields are equal
	 * to x's ones then return a pointer on this row (i.e. no actual insertion is performed) 
	 * otherwise add x to the table and return x.
	 * @param x . A pointer on the row to be added.
 	 * @returns a pointer to a SpectralWindowRow.	 
	 */	 
	 
 	 SpectralWindowRow* add(SpectralWindowRow* x) ;



	//
	// ====> Methods returning rows.
	//
		
	/**
	 * Get all rows.
	 * @return Alls rows as a vector of pointers of SpectralWindowRow. The elements of this vector are stored in the order 
	 * in which they have been added to the SpectralWindowTable.
	 */
	vector<SpectralWindowRow *> get() ;
	


 
	
	/**
 	 * Returns a SpectralWindowRow* given a key.
 	 * @return a pointer to the row having the key whose values are passed as parameters, or 0 if
 	 * no row exists for that key.
	
	 * @param spectralWindowId
	
 	 *
	 */
 	SpectralWindowRow* getRowByKey(Tag spectralWindowId);

 	 	



	/**
 	 * Look up the table for a row whose all attributes  except the autoincrementable one 
 	 * are equal to the corresponding parameters of the method.
 	 * @return a pointer on this row if any, null otherwise.
 	 *
			
 	 * @param basebandName
 	 		
 	 * @param netSideband
 	 		
 	 * @param numChan
 	 		
 	 * @param refFreq
 	 		
 	 * @param sidebandProcessingMode
 	 		
 	 * @param totBandwidth
 	 		
 	 * @param windowFunction
 	 		 
 	 */
	SpectralWindowRow* lookup(BasebandNameMod::BasebandName basebandName, NetSidebandMod::NetSideband netSideband, int numChan, Frequency refFreq, SidebandProcessingModeMod::SidebandProcessingMode sidebandProcessingMode, Frequency totBandwidth, WindowFunctionMod::WindowFunction windowFunction); 


//private:

	/**
	 * Create a SpectralWindowTable.
	 * <p>
	 * This constructor is private because only the
	 * container can create tables.  All tables must know the container
	 * to which they belong.
	 * @param container The container to which this table belongs.
	 */ 
	SpectralWindowTable (ASDM & container);

	ASDM & container;
	
	bool archiveAsBin; // If true archive binary else archive XML
	bool fileAsBin ; // If true file binary else file XML	
	
	Entity entity;
	

	// A map for the autoincrementation algorithm
	map<string,int>  noAutoIncIds;
	void autoIncrement(string key, SpectralWindowRow* x);


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
	SpectralWindowRow* checkAndAdd(SpectralWindowRow* x) ;



// A data structure to store the pointers on the table's rows.

// In all cases we maintain a private vector of SpectralWindowRow s.
   vector<SpectralWindowRow * > privateRows;
   

			
	vector<SpectralWindowRow *> row;


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
	 * Convert this table into a SpectralWindowTableIDL CORBA structure.
	 *
	 * @return a pointer to a SpectralWindowTableIDL
	 */
	SpectralWindowTableIDL *toIDL() ;
#endif

#ifndef WITHOUT_ACS
	/**
	 * Populate this table from the content of a SpectralWindowTableIDL Corba structure.
	 *
	 * @throws DuplicateKey Thrown if the method tries to add a row having a key that is already in the table.
	 * @throws ConversionException
	 */	
	void fromIDL(SpectralWindowTableIDL x) ;
#endif
	
	
	void error() ; //throw(ConversionException);

	/**
	 * Translate this table to an XML representation conform
	 * to the schema defined for SpectralWindow (SpectralWindowTable.xsd).
	 *
	 * @returns a string containing the XML representation.
	 * @throws ConversionException
	 */
	string toXML()  ;
	
	/**
	 * Populate this table from the content of a XML document that is required to
	 * be conform to the XML schema defined for a SpectralWindow (SpectralWindowTable.xsd).
	 * @throws ConversionException
	 * 
	 */
	void fromXML(string xmlDoc) ;
		
	/**
	  * Private methods involved during the build of this table out of the content
	  * of file(s) containing an external representation of a SpectralWindow table.
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
	  * will be saved in a file "SpectralWindow.bin" or an XML representation (fileAsBin==false) will be saved in a file "SpectralWindow.xml".
	  * The file is always written in a directory whose name is passed as a parameter.
	 * @param directory The name of directory  where the file containing the table's representation will be saved.
	  * 
	  */
	  void toFile(string directory);
	  
	/**
	 * Reads and parses a file containing a representation of a SpectralWindowTable as those produced  by the toFile method.
	 * This table is populated with the result of the parsing.
	 * @param directory The name of the directory containing the file te be read and parsed.
	 * @throws ConversionException If any error occurs while reading the 
	 * files in the directory or parsing them.
	 *
	 */
	 void setFromFile(const string& directory);	
 
};

} // End namespace asdm

#endif /* SpectralWindowTable_CLASS */
