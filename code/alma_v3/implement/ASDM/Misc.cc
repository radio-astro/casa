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
 * File Misc.cpp
 */

#include <Misc.h>
 
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>

#include <algorithm> //required for std::swap
#include <iostream>

using namespace std;

using namespace boost::filesystem;
using namespace boost::algorithm;

extern int xmlLoadExtDtdDefaultValue;

namespace asdm {
  bool directoryExists(const char* dir) {
    DIR* dhandle = opendir(dir);

    if (dhandle != NULL) {
      closedir(dhandle);
      return true;
    }
    else {
      return false;
    }
  }

  bool createDirectory(const char* dir) { 
    return mkdir(dir, S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH) == 0;
  }
	
  bool createPath(const char* path) {
    char localpath[256];
    strcpy(localpath, path);

    char directory[256];
    if (path[0] == '/') {
      strcpy(directory, "/");
    }
    else {
      strcpy(directory, "");
    }
    	
    char* pch = strtok(localpath, "/");
    while (pch != NULL) {
      strcat(directory, pch);
      strcat(directory, "/");
      if (!directoryExists(directory) && !createDirectory(directory)) {
	return false;
      }
      pch = strtok(NULL, "/");
    }
    return true;
  }
    
  void ByteSwap(unsigned char * b, int n) {
    register int i = 0;
    register int j = n-1;
    while (i<j) {
      std::swap(b[i], b[j]);
      i++, j--;
    }
  }

#if defined(__APPLE__)
  const ByteOrder* ByteOrder::Little_Endian = new ByteOrder("Little_Endian", __DARWIN_LITTLE_ENDIAN);
  const ByteOrder* ByteOrder::Big_Endian = new ByteOrder("Big_Endian", __DARWIN_BIG_ENDIAN);
#else 
  const ByteOrder* ByteOrder::Little_Endian = new ByteOrder("Little_Endian", __LITTLE_ENDIAN);
  const ByteOrder* ByteOrder::Big_Endian = new ByteOrder("Big_Endian", __BIG_ENDIAN);
#endif
  const ByteOrder* ByteOrder::Machine_Endianity = ByteOrder::machineEndianity();

  ByteOrder::ByteOrder(const string& name, int endianity):
    name_(name), endianity_(endianity){;}

  ByteOrder::~ByteOrder() {;}

  const ByteOrder* ByteOrder::machineEndianity() {
#if defined(__APPLE__)
    if (__DARWIN_BYTE_ORDER == __DARWIN_LITTLE_ENDIAN)
#else 
      if (__BYTE_ORDER == __LITTLE_ENDIAN)
#endif
	return Little_Endian;
      else
	return Big_Endian;
  }
  
  string ByteOrder::toString() const {
    return name_;
  }

  const ByteOrder* ByteOrder::fromString(const string &s) {
    if (s == "Little_Endian") return Little_Endian;
    if (s == "Big_Endian") return Big_Endian;
    return 0;
  }

  string uniqSlashes(const string & s) {
    string result;
    char c;
    bool inslash = false;
    size_t indexi=0;

    while (indexi < s.size()) {
      if ((c = s.at(indexi)) != '/') {
	inslash = false;
	result.push_back(c);
      }
      else
	if (inslash == false) {
	  result.push_back(c);
	  inslash = true;
	}
      indexi++;
    }
    return result;
  }

  ASDMUtils::DotXMLFilter::DotXMLFilter(vector<string>& filenames) {this->filenames = &filenames;}
  void ASDMUtils::DotXMLFilter::operator()(directory_entry& p) {
    cout << "Entering () with path = " << p.string() << endl;
    if (!extension(p).compare(".xml")) {
      filenames->push_back(p.string());
    }
  }
 
  set<string>				ASDMUtils::evlaValidNames;
  set<string>				ASDMUtils::almaValidNames;
  map<ASDMUtils::Origin, string>	ASDMUtils::filenameOfV2V3xslTransform;
  map<string, string>			ASDMUtils::rootSubdir ;
  bool					ASDMUtils::initialized = ASDMUtils::initialize();

  bool ASDMUtils::initialize() {
    string evlaValidNames_a[] = {"EVLA"};
    evlaValidNames = set<string>(evlaValidNames_a, evlaValidNames_a + 1);

    string almaValidNames_a[] = {"ALMA", "AOS", "OSF"};
    almaValidNames = set<string>(almaValidNames_a, almaValidNames_a + 3);

    filenameOfV2V3xslTransform[ASDMUtils::ALMA]    = "sdm-v2v3-alma.xsl";
    filenameOfV2V3xslTransform[ASDMUtils::EVLA]    = "sdm-v2v3-evla.xsl";
    filenameOfV2V3xslTransform[ASDMUtils::UNKNOWN] = "";

    rootSubdir["INTROOT"]  = "config/";
    rootSubdir["ACSROOT"]  = "config/";
    rootSubdir["CASAPATH"] = "data/alma/asdm/";

    return true;
  }

  string ASDMUtils::version(const string& asdmPath) {
    string ASDMPath = trim_copy(asdmPath);
    if (!ends_with(ASDMPath, "/")) ASDMPath+="/";
    ASDMPath += "ASDM.xml";

    // Does ASDMPath exist ?
    if (!exists(path(ASDMPath))) {
      cout << "Could not find a file 'ASDM.xml' in " << asdmPath << "'." << endl;
      exit(1);
    }

    // Read and parse ASDM.xml
    xmlDocPtr ASDMdoc = xmlParseFile(ASDMPath.c_str());
  
    if (ASDMdoc == NULL ) {
      fprintf(stderr,"Document not parsed successfully. \n");
      exit(1);
    }
  
    xmlNodePtr cur = xmlDocGetRootElement(ASDMdoc);
    xmlChar* version_ = xmlGetProp(cur, (const xmlChar *) "schemaVersion");
    if (version_ == NULL) {
      return "UNKNOWN";
    }

    string result((char *) version_);
    xmlFree(version_);
    return result;
  }

  string ASDMUtils::parseRow(xmlDocPtr doc, xmlNodePtr node, const xmlChar* childName) {
    //
    // Consider the children of node (i.e. expectedly of "<row>...</row>")
    node = node->xmlChildrenNode;

    //
    // And look for childName.
    while (node != NULL) {
      if (!xmlStrcmp(node->name , childName)) {
	xmlChar* content = xmlNodeListGetString(doc, node->xmlChildrenNode, 1);
	string result((char *) content);
	xmlFree(content);
	return result;
      }
      node = node->next;
    }

    // If we are here , it's because we have not found telescopeName.
    cout << "Could not find a 'telescopeName' element." << endl;
    exit(1);
  }
 
  vector<string> ASDMUtils::telescopeNames(const string& asdmPath) {
    vector<string> result;

    string execBlockPath = trim_copy(asdmPath);
    if (!ends_with(execBlockPath, "/")) execBlockPath+="/";
    execBlockPath += "ExecBlock.xml";

    // Does execBlockPath exist ?
    if (!exists(path(execBlockPath))) {
      cout << "Could not find a file 'ExecBlock.xml' in '" << asdmPath << "'." << endl;
      exit(1);
    }

    // Read and parse ExecBlock.xml
    xmlDocPtr execBlockDoc;
    xmlNodePtr cur;

    execBlockDoc = xmlParseFile(execBlockPath.c_str());
    if (execBlockDoc == NULL) {
      cout << "'" << execBlockPath << "' was not parsed successfully." << endl;
      exit (0);
    }

    cur = xmlDocGetRootElement(execBlockDoc)->xmlChildrenNode;
    while (cur != NULL) {
      if (!xmlStrcmp(cur->name, (const xmlChar*) "row")) {
	result.push_back(parseRow(execBlockDoc, cur, (const xmlChar *) "telescopeName"));
      }
      cur = cur -> next;
    }

    return result;
  }

  ASDMUtils::Origin ASDMUtils::origin(const vector<string>& telescopeNames) {
    unsigned int numEVLA = 0;
    unsigned int numALMA = 0;
    for ( vector<string>::const_iterator iter = telescopeNames.begin(); iter != telescopeNames.end(); iter++ ) {
      if (evlaValidNames.find(*iter) != evlaValidNames.end())
	numEVLA++;
      else if (almaValidNames.find(*iter) != almaValidNames.end())
	numALMA++;
    }

    Origin autoOrigin = UNKNOWN;
    if (numEVLA == telescopeNames.size())
      autoOrigin = EVLA;
    else if (numALMA == telescopeNames.size())
      autoOrigin = ALMA;

    return autoOrigin;
  }

  vector<string> ASDMUtils::xmlFilenames ( const string & asdmPath  ) {
    vector<string> result;

    path p(asdmPath);
    DotXMLFilter dotXMLFilter(result);
    std::for_each(directory_iterator(p), directory_iterator(), dotXMLFilter);
    //result = dotXMLFilter.xmlFilenames();
    cout << "size of result " << result.size() << endl;
    return result;
  }

  string ASDMUtils::pathToxslTransform( const string& xsltFilename) {

    char * envVars[] = {"INTROOT", "ACSROOT", "CASAPATH"};
    char * rootDir_p;
    for (int i = 0; i < sizeof(envVars) ; i++) 
      if ((rootDir_p = getenv(envVars[i])) != 0) {
	string rootPath(rootDir_p);
	vector<string> rootPathElements;
	split(rootPathElements, rootPath, is_any_of(" "));
	for ( vector<string>::iterator iter = rootPathElements.begin(); iter != rootPathElements.end(); iter++) {
	  string xsltPath = *iter;
	  if (!ends_with(xsltPath, "/")) xsltPath+="/";
	  xsltPath+=rootSubdir[string(envVars[i])]+ xsltFilename;
	  if (exists(path(xsltPath)))
	    return xsltPath;
	}
      }
    
    // Here rootDir_p == NULL , let's return an empty string.
    return "" ;  // An empty string will be interpreted as no file found.
  }
  
  string ASDMUtils::pathToV2V3ALMAxslTransform() {return pathToxslTransform(filenameOfV2V3xslTransform[ASDMUtils::ALMA]);}
  string ASDMUtils::pathToV2V3EVLAxslTransform() {return pathToxslTransform(filenameOfV2V3xslTransform[ASDMUtils::EVLA]);}
  string ASDMUtils::nameOfV2V3xslTransform(ASDMUtils::Origin origin) {
    return filenameOfV2V3xslTransform[origin];
  }

  XSLTransformer::XSLTransformer() : cur(NULL) {
    xmlSubstituteEntitiesDefault(1);
    xmlLoadExtDtdDefaultValue = 1;
    // cout << "XSLTransformer::XSLTransformer() called " << endl;
  }

  XSLTransformer::XSLTransformer(const string& xsltPath) {
    // cout << "XSLTransformer::XSLTransformer(const string& xsltPath) called " << endl;
    // cout << "About parse the style sheet contained in " << xsltPath << endl;
    xmlSubstituteEntitiesDefault(1);
    xmlLoadExtDtdDefaultValue = 1;
    
    cur = xsltParseStylesheetFile((const xmlChar*) xsltPath.c_str());
    if (cur == NULL)
      cout << "Could not parse the XSL stylecheet contained in '" << xsltPath << "'." << endl;
    // cout << "XSLTransformer::XSLTransformer(const string& xsltPath) exiting " << endl;
  }

  void XSLTransformer::setTransformation(const string& xsltPath) {
    // cout << "XSLTransformer::setTransformation(const string& xsltPath) called " << endl;
    
    if (cur) {
      xsltFreeStylesheet(cur);
      cur = NULL;
    }

    // cout << "About parse the style sheet contained in " << xsltPath << endl;
      
    cur = xsltParseStylesheetFile((const xmlChar*) xsltPath.c_str());
    if (cur == NULL)
      cout << "Could not parse the XSL stylecheet contained in '" << xsltPath << "'." << endl;
    // cout << "XSLTransformer::setTransformation(const string& xsltPath) exiting " << endl;
  }


  XSLTransformer::~XSLTransformer() {
    // cout << "XSLTransformer::~XSLTransformer() called" << endl;
    if (cur) {
      xsltFreeStylesheet(cur);
      cur = NULL;
    }
  } 

  string XSLTransformer::operator()(const string& xmlPath){
    xmlDocPtr doc = NULL, res = NULL;
    //xsltStylesheetPtr cur;

    xmlChar* docTxtPtr = NULL;
    int docTxtLen = 0;
    
    // cout << "About to read and parse " << xmlPath << endl;
    doc = xmlParseFile(xmlPath.c_str());
    if (doc == NULL) {
      cout << "Could not parse the XML file '" << xmlPath << "'." << endl;
    }

    if (!cur) {
      xmlDocDumpFormatMemory(doc, &docTxtPtr, &docTxtLen, 1);
    }
    else {
      res = xsltApplyStylesheet(cur, doc, NULL);
      if ( res == NULL ) {
	cout << "Could not apply the XSLT tranformation to the XML document contained in '" << xmlPath << "'." << endl;
      }
      int status = xsltSaveResultToString(&docTxtPtr,
					  &docTxtLen,
					  res,
					  cur);
      if (status == -1) 
	cout << "Could not dump the result of the XSL transformation into memory." << endl;
    }

    // cout << "Making a string out of the result of applying the XSL transformation" << endl;
    string docXML((char *) docTxtPtr, docTxtLen);
    // cout << "docXML = " << docXML << endl;

    xmlFree(docTxtPtr);
    if (res) xmlFreeDoc(res);
    xmlFreeDoc(doc);
    
    // cout << "All resources unneeded freed" << endl;
    return docXML;
  } 

} // end namespace asdm
 
 
