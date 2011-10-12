#ifndef MISC_H
#define MISC_H
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
 *
 * File Misc.h
 */

#include <string> 
#include <vector>
#include <set>
#include <map>

#include <boost/filesystem/path.hpp>
#include <boost/filesystem/convenience.hpp>
#include <boost/algorithm/string/trim.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/algorithm/string/split.hpp>

typedef unsigned char xmlChar;

struct _xmlNode;
typedef struct _xmlNode xmlNode;
typedef xmlNode *xmlNodePtr;

struct _xmlDoc;
typedef struct _xmlDoc xmlDoc;
typedef xmlDoc *xmlDocPtr;

struct _xsltStylesheet;
typedef struct _xsltStylesheet xsltStylesheet;
typedef xsltStylesheet *xsltStylesheetPtr;

extern "C" int xmlLoadExtDtdDefaultValue;

namespace asdm {
  /**
   * Some utility methods to manipulate directories.
   */
	 
  /**
   * Returns whether the specified directory exists.
   *
   * Example:
   * --------
   *          if (!directoryExists("output"))
   *          {
   *              createDirectory("output");
   *          }
   */ 
  bool directoryExists(const char* dir);
	
  /**
   * Creates the specified directory. Fails if the path leading to
   * this directory does not exist.
   *
   * Example:
   * --------
   *          createDirectory("output");	
   */
  bool createDirectory(const char* dir);
	
  /**
   * Creates a complete path.
   *
   * Example:
   * --------
   *           ("output/sample1/temperature0/");
   */
  bool createPath(const char* path);
	
	
  /**
   * Swap bytes 
   */
#define ByteSwap5(x) ByteSwap((unsigned char *) &x,sizeof(x))	 
  void ByteSwap(unsigned char * b, int n);

  /**
   * A class to represent byte order information.
   *
   */
  class ByteOrder {
  public:
    static const ByteOrder* Little_Endian; /*< A unique object to represent a little endian byte order. */
    static const ByteOrder* Big_Endian;    /*< A unique object to represent a big endian byte order. */
    static const ByteOrder* Machine_Endianity; /*< A unique object storing the endianity of the machine. */

    /**
     * Returns a string representation of this.
     *
     * <ul>
     * <li> Little_Endian is returned as "Little_Endian", </li>
     * <li> Big_Endian is returned as "Big_Endian", </li>
     * </ul>
     */
    std::string toString() const ;

    /**
     * Convert a string to a const ByteOrder*.
     *
     * @param s the input string.
     * @return a const pointer to a ByteOrder for which the toString() method returns
     * a string == to the input string, or 0 if no such instance of ByteOrder exists.
     */
    static const ByteOrder* fromString(const std::string & s);

  private:
    std::string name_;
    int endianity_;

    ByteOrder(const std::string & name, int endianity);
    virtual ~ByteOrder();
    static const ByteOrder* machineEndianity(); 
  };

  /**
   * Return a string whose content is equal to the content of s
   * but with all the repetitions of '/' characters replaced by
   * a unique '/'.
   *
   * @return a string.
   */
  std::string uniqSlashes(const std::string& s);

  class ASDMUtilsException {
  public:
    ASDMUtilsException();
    ASDMUtilsException(const std::string& message);
    
    const std::string& getMessage();
    
  private:
    std::string message;
  };
  
  class ASDMUtils {
  public :
    enum Origin { UNKNOWN, ALMA, EVLA };
    static std::string		version (const std::string& asdmPath);
    static std::vector<std::string> telescopeNames(const std::string& asdmPath);
    static Origin		origin(const std::vector<std::string>& telescopeNames);
    static std::vector<std::string> xmlFilenames(const std::string& asdmPath);
    
    static std::string pathToV2V3ALMAxslTransform() ;
    static std::string pathToV2V3EVLAxslTransform() ;
    static std::string nameOfV2V3xslTransform(ASDMUtils::Origin origin);

    struct DotXMLFilter {
    public:
      DotXMLFilter(std::vector<std::string>& filenames);
      //void operator() (path& p);
      void operator() ( boost::filesystem::directory_entry& p);
    private:
      std::vector<std::string>* filenames;    
    };
    
  private :
    static bool initialize();
    static bool initialized;
    static bool hasChild(xmlDocPtr, xmlNodePtr node, const xmlChar* childName);
    static std::string parseRow(xmlDocPtr, xmlNodePtr node, const xmlChar* childName);
    static std::set<std::string> evlaValidNames;
    static std::set<std::string> almaValidNames;
    static std::map<ASDMUtils::Origin, std::string> filenameOfV2V3xslTransform;
    static std::map<std::string, std::string> rootSubdir ;

    static std::string pathToxslTransform( const std::string& xsltFilename);
  }; // end class ASDMUtil.


  /**
   * A class to define a collection of options regarding the way to consider an ASDM dataset
   * especially when it's about to be read on disk, parsed and transformed into its representation in memory.
   *
   *
   * The options that can be set with instances of this class are :
   * <ul>
   * <li> The origin of the dataset , i.e. the location where it's been produced. Currently three locations are known : ALMA, EVLA and IRAM at
   * its Plateau de Bure.</li>
   * <li> The version of the dataset. At present time version 2 or version 3 are known.</li>
   * <li> The fact that all the tables of a dataset are expected to be present in memory or only those which are referenced by the code
   * which needs to acces them </li>
   * </ul>
   * The method setFromFile defined in the class ASDM is the principal (not to say only) user of instances of this class.
   *
   * It should be noticed that all the methods defined to set the options return a reference onto the instance of ASDMParseOptions they 
   * are applied. This technique facilitates the definition of a collection of options in one single statement, e.g. :
   *
   * <code>
   * ASDMParseOptions parse;
   * parse.asALMA().asV2().loadTablesOnDemand(true);
   * </code>
   *
   */ 

  class ASDMParseOptions {
    friend class ASDM;

  public:
    /**
     * A null constructor.
     *
     * The instance built by this constructor defines the instances as follows :
     * <ul>
     * <li> The origin is UNKNOWN. </li>
     * <li> The version is UNKNOWN. </li>
     * <li> All the tables are to be loaded in memory. </li>
     * </ul>
     *
     */
    ASDMParseOptions();

    /**
     * The destructor.
     * Does nothing special.
     */
    virtual ~ASDMParseOptions();
    
    /**
     * This is an ALMA dataset.
     */
    ASDMParseOptions& asALMA();

    /**
     * This is an IRAM / Plateau de Bure dataset.
     */
    ASDMParseOptions& asIRAM_PDB();

    /**
     * This is an EVLA dataset.
     */
    ASDMParseOptions& asEVLA();

    /**
     * This is a V2 dataset.
     */
    ASDMParseOptions& asV2();

    /**
     * This is a V3 dataset.
     */
    ASDMParseOptions& asV3();

    /**
     * Load all the tables in memory or only those which are required by the code.
     *
     * @param b a boolean value. <code>true</code> <-> load only the table which are required by the code when they are referred, <code>false</code> <-> load
     * all the code.
     */
    ASDMParseOptions& loadTablesOnDemand(bool b);

  private:
    ASDMUtils::Origin	origin_;
    bool		detectOrigin_;
    std::string		version_;
    bool		detectVersion_;
    bool		loadTablesOnDemand_;
  };				// end class ASDMParseOptions.

  /**
   * A class to represent an exception thrown while an XSL transformation is applied by an instance of XSLTransformer.
   */

  class XSLTransformerException {

  public:
    /**
     * An empty constructor.
     */
    XSLTransformerException();

    /**
     * A constructor with a message associated with the exception.
     * @param m a string containing the message.
     */
    XSLTransformerException(const std::string & m);
    
    /**
     * The destructor.
     */
    virtual ~XSLTransformerException();

    /**
     * Return the message associated to this exception.
     * @return a string.
     */
    std::string getMessage() const;

  protected:
    std::string message;

  };

  inline XSLTransformerException::XSLTransformerException() : message ("XSLTransformerException") {}
  inline XSLTransformerException::XSLTransformerException(const std::string& m) : message(m) {}
  inline XSLTransformerException::~XSLTransformerException() {}
  inline std::string XSLTransformerException::getMessage() const {
    return "XSLTransformerException : " + message;
  }

  /**
   * A class to apply XSL transformations on an XML document.
   * The XSL transformation to be applied is defined when an instance is constructed. The class has its operator operator() overloaded 
   * so that the transformation can be applied by using the instance as a functor.
   *
   * An instance built with an empty constructor will be interpreted as a "neutral" transformation, i.e. a transformation which leaves 
   * the XML document unchanged.
   *
   */
  class XSLTransformer {
  public:
    /**
     * The empty constructor.
     * 
     * The instance will leave the XML document unchanged.
     */
    XSLTransformer();

    /**
     * The destructor.
     */
    virtual ~XSLTransformer();

    /**
     * A constructor with an XSL transformation.
     *
     * The XSL transformation is read in the file whose path is passed as a parameter, then it's parsed
     * and stored in memory. 
     *
     * @param xslPath the path ot the file containing the XSL transformation.
     * @throws XSLTransformerException;
     */
    XSLTransformer(const std::string& xsltPath);

    /**
     * A setter to (re) define the XSL transformation associated with the instance of XSLTransformer.
     *
     * The XSL transformation is read in the file whose path is passed as a parameter, then it's parsed
     * and stored in memory. 
     * 
     * @param xslPath the path ot the file containing the XSL transformation.
     *
     * @throws XSLTransformerException.
     */
    void setTransformation(const std::string& xsltPath);

    /**
     * Overloads operator() so that the instance can be used as a functor to apply the transformation on a given XML document.
     *
     * The XML document to be transformed is expected to be found in a file whose path is passed as a parameter. 
     * It is read and parsed and stored into memory, then the XSL transformation associated with the instance is applied to the XML document 
     * stored in memory and finally the result of the transformation is converted to a string which is returned to the caller.
     *
     * @param xsltPath a string whose value is the path to the file containing the XML document to be transformed.
     * @return a string containing the result of the transformation applied to the XML document.
     *
     * @throws XSLTransformerException.
     *
     */
    std::string operator()(const std::string& xmlPath);
    
  private:
    xsltStylesheetPtr cur;

    XSLTransformer& operator=(const XSLTransformer & rhs);
    XSLTransformer(const XSLTransformer & rsh);    
  }; // end class XSLTransformer.
} // end namespace asdm
#endif  // MISC_H

