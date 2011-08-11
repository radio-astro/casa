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

#include <libxml/xmlmemory.h>
#include <libxml/debugXML.h>
#include <libxml/HTMLtree.h>
#include <libxml/xmlIO.h>
#include <libxml/parser.h>
#include <libxml/xinclude.h>
#include <libxml/catalog.h>

#include <boost/filesystem/path.hpp>
#include <boost/filesystem/convenience.hpp>
#include <boost/algorithm/string/trim.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/algorithm/string/split.hpp>

#include <libxslt/xslt.h>
#include <libxslt/xsltInternals.h>
#include <libxslt/transform.h>
#include <libxslt/xsltutils.h>

extern int xmlLoadExtDtdDefaultValue;

using namespace boost;
using namespace boost::filesystem;

using namespace std;

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
    string toString() const ;

    /**
     * Convert a string to a const ByteOrder*.
     *
     * @param s the input string.
     * @return a const pointer to a ByteOrder for which the toString() method returns
     * a string == to the input string, or 0 if no such instance of ByteOrder exists.
     */
    static const ByteOrder* fromString(const string & s);

  private:
    string name_;
    int endianity_;

    ByteOrder(const string & name, int endianity);
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
  string uniqSlashes(const string& s);

  class ASDMUtils {
  public :
    enum Origin { UNKNOWN, ALMA, EVLA };
    static string		version (const string& asdmPath);
    static vector<string> telescopeNames(const string& asdmPath);
    static Origin		origin(const vector<string>& telescopeNames);
    static vector<string> xmlFilenames(const string& asdmPath);
    
    static string pathToV2V3ALMAxslTransform() ;
    static string pathToV2V3EVLAxslTransform() ;
    static string nameOfV2V3xslTransform(ASDMUtils::Origin origin);

    struct DotXMLFilter {
    public:
      DotXMLFilter(vector<string>& filenames);
      //void operator() (path& p);
      void operator() (directory_entry& p);
    private:
      vector<string>* filenames;    
    };
    
  private :
    static bool initialize();
    static bool initialized;
    static string parseRow(xmlDocPtr, xmlNodePtr node, const xmlChar* childName);
    static set<string> evlaValidNames;
    static set<string> almaValidNames;
    static map<ASDMUtils::Origin, string> filenameOfV2V3xslTransform;
    static map<string, string> rootSubdir ;

    static string pathToxslTransform( const string& xsltFilename);
  };

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
    XSLTransformerException(const string & m);
    
    /**
     * The destructor.
     */
    virtual ~XSLTransformerException();

    /**
     * Return the message associated to this exception.
     * @return a string.
     */
    string getMessage() const;

  protected:
    string message;

  };

  inline XSLTransformerException::XSLTransformerException() : message ("XSLTransformerException") {}
  inline XSLTransformerException::XSLTransformerException(const string& m) : message(m) {}
  inline XSLTransformerException::~XSLTransformerException() {}
  inline string XSLTransformerException::getMessage() const {
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
    XSLTransformer(const string& xsltPath);

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
    void setTransformation(const string& xsltPath);

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
    string operator()(const string& xmlPath);
    
  private:
    xsltStylesheetPtr cur;

    XSLTransformer& operator=(const XSLTransformer & rhs);
    XSLTransformer(const XSLTransformer & rsh);    
  }; // end class XSLTransformer.
} // end namespace asdm
#endif  // MISC_H

