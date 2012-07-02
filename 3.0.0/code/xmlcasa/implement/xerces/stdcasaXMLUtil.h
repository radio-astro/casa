#ifndef STDCASAXMLUTILS_H
#define STDCASAXMLUTILS_H

//*
// What we are doing is encoding a parameter set
// The parameter set <pset> has a "name" attribute that corresponds to
// task, or tool.method, additional attributes specifies the scope, global, project, or local
// if project, then an addtional project attribute, no scope attribute assume local
// Then for each <param> we have the following attributes
// "name", "xsi:type", and optionally "units"
// Each parameter will then have the following elements
// <value> can have attribute type="vector"
// <description> short help text
// <any> only if type is any
// Description

#include <string>
#include <xercesc/dom/DOM.hpp>
#include <xmlcasa/variant.h>
#include <xercesc/util/XMLChar.hpp>
#include <xercesc/dom/DOM.hpp>
#include <xercesc/framework/Wrapper4InputSource.hpp>

namespace casac {

using std::ostream;
using std::string;

class record;

class stdcasaXMLUtil {
   public :
	stdcasaXMLUtil();
	~stdcasaXMLUtil();
	bool toCasaRecord(record &outRec, const string &xml);
	bool fromCasaRecord(string &outXML, const record &theRecord);
	bool fromCasaRecord(ostream &outXML, const record &theRecord);
	bool readXMLFile(record &outRec, const string &xmlFile);
	bool writeXMLFile(const string &xmlFile, const record &inRec);
	void setRecord(record &aRec);
   private :
	bool readXML(record &outRec, const XERCES_CPP_NAMESPACE::Wrapper4InputSource &xml);
	variant *itsvalue(XERCES_CPP_NAMESPACE::DOMNode *theNode, variant::TYPE itsType);

	XMLCh *name;
	XMLCh *type;
	//XMLCh *xsitype;
	XMLCh *param;
	XMLCh *value;
	XMLCh *allowed;
	XMLCh *range;
	XMLCh *description;
	XMLCh *mustexist;
	XMLCh *ienum;
	XMLCh *kind;
	XMLCh *any;
	XMLCh *limittype;
	XMLCh *limittypes;
	XMLCh *units;
	XMLCh *shortdesc;
	XMLCh *example;
	XMLCh *subparam;
	XMLCh *ignorecase;
	
	record *rangeRec;
	record *whenRec;
	record *equalsRec;
	record *constraintsRec;
	record *defaultsRec;
	record *paramSet;

	string ttName;  //Task or Tool Name
	string parmName; //Parameter Name
	string lastParm; // Last non-subparameter
	variant::TYPE setType(const string &xmlType);
	variant *tovariant(variant::TYPE theType, string &theInput, bool isVector);
	void addtovariant(variant *it, variant::TYPE theType, string &theInput);
};

}
#endif
