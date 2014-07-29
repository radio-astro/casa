#include <xercesc/dom/DOM.hpp>
#include <xercesc/util/XMLString.hpp>
#include <xercesc/util/HexBin.hpp>
#include <xercesc/framework/LocalFileInputSource.hpp>
#include <xercesc/framework/MemBufInputSource.hpp>
#include <xercesc/framework/Wrapper4InputSource.hpp>
#include <iostream>
#include <fstream>
#include <stdcasa/record.h>
#include <stdcasa/variant.h>
#include <string.h>
#include <tools/xerces/stdcasaXMLUtil.h>
//#include <casa/Exceptions/Error.h>

//namespace XERCES_CPP_NAMESPACE {
//}

namespace casac {

using namespace std;
XERCES_CPP_NAMESPACE_USE;

stdcasaXMLUtil::stdcasaXMLUtil() :
    rangeRec(0),
    whenRec(0),
    equalsRec(0),
    constraintsRec(0),
    defaultsRec(0),
    paramSet(0),
    ttName(""),
    parmName(""),
    lastParm("")
    {
    try {
        XMLPlatformUtils::Initialize();
        name = XMLString::transcode("name");
        type = XMLString::transcode("type");
        //xsitype = XMLString::transcode("xsi:type");
        param = XMLString::transcode("param");
        value = XMLString::transcode("value");
        allowed = XMLString::transcode("allowed");
        range = XMLString::transcode("range");
        kind = XMLString::transcode("kind");
        description = XMLString::transcode("description");
        mustexist = XMLString::transcode("mustexist");
        allowblank = XMLString::transcode("allowblank");
        units = XMLString::transcode("units");
        any = XMLString::transcode("any");
        limittype = XMLString::transcode("limittype");
        limittypes = XMLString::transcode("limittypes");
        ienum = XMLString::transcode("enum"); //enum is a reservered work
        shortdesc = XMLString::transcode("shortdescription");
        example = XMLString::transcode("example");
        subparam = XMLString::transcode("subparam");
        ignorecase = XMLString::transcode("ignorecase");
        //
    }
    catch (const XMLException& toCatch) {
        char* message = XMLString::transcode(toCatch.getMessage());
        cout << "Error during initialization! :\n";
        cout << "Exception message is: \n"
        << message << "\n";
        XMLString::release(&message);
    }
    }

stdcasaXMLUtil::~stdcasaXMLUtil()
{
    XMLString::release(&name);
    XMLString::release(&type);
    //XMLString::release(&xsitype);
    XMLString::release(&param);
    XMLString::release(&value);
    XMLString::release(&allowed);
    XMLString::release(&range);
    XMLString::release(&description);
    XMLString::release(&mustexist);
    XMLString::release(&allowblank);
    XMLString::release(&ienum);
    XMLString::release(&kind);
    XMLString::release(&units);
    XMLString::release(&any);
    XMLString::release(&limittype);
    XMLString::release(&limittypes);
    XMLString::release(&shortdesc);
    XMLString::release(&example);
    XMLString::release(&subparam);
    XMLString::release(&ignorecase);
    XMLPlatformUtils::Terminate();
}

bool stdcasaXMLUtil::toCasaRecord(record &outRec, const string &inString)
{
    bool rstat(false);
    MemBufInputSource xmlSource((const XMLByte *)inString.c_str(), inString.size(), "casa::");
    Wrapper4InputSource theXML(&xmlSource);
    rstat = readXML(outRec, theXML);
    return rstat;
}
bool stdcasaXMLUtil::fromCasaRecord(string &outString, const record &inRec){
    ostringstream oss;
    bool rstat = fromCasaRecord(oss, inRec);
    outString = oss.str();
    return rstat;
}

bool stdcasaXMLUtil::fromCasaRecord(ostream &oss, const record &inRec)
{
    bool rstat(true);
    for(record::const_iterator iter = inRec.begin(); iter != inRec.end(); ++iter ){
        oss <<  "<param name=\"" << (*iter).first << "\" ";
        switch((*iter).second.type()){
        case variant::RECORD :
            oss << "type=\"record\">" << endl;
            fromCasaRecord(oss, iter->second.getRecord());
            break;
        case variant::BOOL :
            oss << "type=\"bool\">" << endl;
            oss <<  "<value>";
            if((*iter).second.getBool())
                oss << "True";
            else 
                oss << "False";
            oss << "</value>" << endl;
            break;
        case variant::INT :
            oss << "type=\"int\">" << endl;
            oss <<  "<value>" << (*iter).second.getInt() << "</value>" << endl;
            break;
        case variant::DOUBLE :
            oss << "type=\"double\">" << endl;
            oss <<  "<value>" << (*iter).second.getDouble() << "</value>" << endl;
            break;
        case variant::COMPLEX :
            oss << "type=\"complex\">" << endl;
            oss <<  "<value>" << (*iter).second.getComplex() << "</value>" << endl;
            break;
        case variant::STRING :
            oss << "type=\"string\">" << endl;
            oss <<  "<value>" << (*iter).second.getString() << "</value>" << endl;
            break;
        case variant::BOOLVEC :
            oss << "type=\"boolVec\">" << endl;
            oss << "<value>" << endl;
            {
                const vector<bool> &tVec =(*iter).second.getBoolVec();
                for(vector<bool>::const_iterator iter2 = tVec.begin();
                iter2 != tVec.end(); iter2++){
                    oss << "<value>";
                    if( *iter2) 
                        oss << "True";
                    else 
                        oss << "False";
                    oss << "</value>" <<endl;
                }
            }
            oss << "</value>" << endl;
            break;
        case variant::INTVEC :
            oss << "type=\"intVec\">" << endl;
            oss << "<value>" << endl;
            {
                const vector<int> &tVec =(*iter).second.getIntVec();
                for(vector<int>::const_iterator iter2 = tVec.begin();
                iter2 != tVec.end(); iter2++){
                    oss << "<value>" << *iter2 << "</value>" <<endl;
                }
            }
            oss << "</value>" << endl;
            break;
        case variant::DOUBLEVEC :
            oss << "type=\"doubleVec\">" << endl;
            oss << "<value>" << endl;
            {
                const vector<double> &tVec =(*iter).second.getDoubleVec();
                for(vector<double>::const_iterator iter2 = tVec.begin();
                iter2 != tVec.end(); iter2++){
                    oss << "<value>" << *iter2 << "</value>" <<endl;
                }
            }
            oss << "</value>" << endl;
            break;
        case variant::COMPLEXVEC :
            oss << "type=\"complexVec\">" << endl;
            oss << "<value>" << endl;
            {
                const vector<complex<double> > &tVec =(*iter).second.getComplexVec();
                for(vector<complex<double> >::const_iterator iter2 = tVec.begin();
                iter2 != tVec.end(); iter2++){
                    oss << "<value>" << *iter2 << "</value>" <<endl;
                }
            }
            oss << "</value>" << endl;
            break;
        case variant::STRINGVEC :
            oss << "type=\"stringVec\">" << endl;
            oss << "<value>" << endl;
            {
                const vector<string> &tVec =(*iter).second.getStringVec();
                for(vector<string>::const_iterator iter2 = tVec.begin();
                iter2 != tVec.end(); iter2++){
                    oss << "<value>" << *iter2 << "</value>" <<endl;
                }
            }
            oss << "</value>" << endl;
            break;
        default :
            break;
        }
        oss << "</param name>" << endl;
    }
    return rstat;
}
bool stdcasaXMLUtil::readXMLFile(record &outRec, const string &fileName)
{
    bool rstat(false);
    XMLCh *xmlFile = XMLString::transcode(fileName.c_str());
    XMLCh *xmlRPath = XMLString::transcode(".");
    LocalFileInputSource xmlSource(xmlRPath, xmlFile);
    Wrapper4InputSource theXML(&xmlSource, false);
    rstat = readXML(outRec, theXML);
    XMLString::release(&xmlFile);
    XMLString::release(&xmlRPath);
    return rstat;
}

bool stdcasaXMLUtil::readXML(record &itsRecord,  const Wrapper4InputSource &xmlSource)
{
    bool rstat(false);
    //const char* xmlFile = fileName.c_str();
    XMLCh tempStr[100];
    XMLString::transcode("LS", tempStr, 99);
    DOMImplementation *impl = DOMImplementationRegistry::getDOMImplementation(tempStr);
    DOMBuilder* parser = ((DOMImplementationLS*)impl)->createDOMBuilder(DOMImplementationLS::MODE_SYNCHRONOUS, 0);
    // optionally you can set some features on this builder
    if(parser->canSetFeature(XMLUni::fgDOMValidation, true))
        parser->setFeature(XMLUni::fgDOMValidation, true);
    if(parser->canSetFeature(XMLUni::fgDOMNamespaces, true))
        parser->setFeature(XMLUni::fgDOMNamespaces, true);
    if(parser->canSetFeature(XMLUni::fgDOMDatatypeNormalization, true))
        parser->setFeature(XMLUni::fgDOMDatatypeNormalization, true);

    DOMDocument *doc = 0;
    vector<string> paramOrder;
    try {
        doc = parser->parse(xmlSource);
        if(doc){
            DOMElement *theMethod = doc->getDocumentElement();
            XMLCh *theTask = XMLString::transcode("task");
            DOMNodeList *allNodes = theMethod->getElementsByTagName(theTask);
            if(!allNodes->getLength()){
                //cerr << "Not task" << endl;
                XMLCh *theTool = XMLString::transcode("tool");
                allNodes = theMethod->getElementsByTagName(theTool);
                if(!allNodes->getLength()){
                    //cerr << "Not tools" << endl;
                    XMLCh *theParms = XMLString::transcode("casaparams");
                    allNodes = theMethod->getElementsByTagName(theParms);
                }
                //XMLString::release(&theTool);
            }
            //XMLString::release(&theTask);
            //
            // Loop through all the tasks and tools
            //
            //cerr << "Number of nodes is: " << allNodes->getLength() << endl;
            for(XMLSize_t i = 0; i<allNodes->getLength();i++){
                DOMNode *theNode = allNodes->item(i);
                DOMNamedNodeMap *theAttributes = theNode->getAttributes();
                //cerr << "Attributes pointer is: " << theAttributes << endl;
                if(theAttributes && theAttributes->getLength() > 0){
                    DOMNode *nameNode = theAttributes->getNamedItem(name);
                    const XMLCh *myName = nameNode->getNodeValue();
                    if(myName){
                        char *aName = XMLString::transcode(myName);
                        DOMNode *typeNode = theAttributes->getNamedItem(type);
                        ttName = aName;
                        itsRecord.insert(ttName, *new record());
                        itsRecord[ttName].asRecord().insert("parameters", *new record());
                        if(typeNode){
                            //const XMLCh *myType = typeNode->getNodeValue();
                            //char *aType = XMLString::transcode(myType);
                        }
                    }
                    //XMLString::release(&aName);
                }
                if(theNode->getNodeType() == DOMNode::ELEMENT_NODE){
                    XMLCh *input = XMLString::transcode("input");
                    DOMNodeList *inputNodes = dynamic_cast<DOMElement *>(theNode)->getElementsByTagName(input);
                    //cerr << "Number of nodes is: " << inputNodes->getLength() << endl;
                    if(!inputNodes->getLength()){
                        //input = XMLString::transcode("casaparams");
                        //inputNodes = dynamic_cast<DOMElement *>(theNode)->getElementsByTagName(input);
                        inputNodes = allNodes;
                    }
                    //cerr << "Number of nodes is: " << inputNodes->getLength() << endl;
                    for(XMLSize_t j=0; j<inputNodes->getLength(); j++){
                        DOMNodeList *paramNodes = dynamic_cast<DOMElement *>(inputNodes->item(j))->getElementsByTagName(param);
                        //cerr << "Number of params is: " << paramNodes->getLength() << endl;
                        for(XMLSize_t k=0; k<paramNodes->getLength(); k++){
                            variant::TYPE paramType(variant::STRING);
                            bool typeIsAny = false;
                            DOMNode *theNode = paramNodes->item(k);
                            DOMNamedNodeMap *theAttributes = theNode->getAttributes();
                            if(theAttributes){
                                DOMNode *nameNode = theAttributes->getNamedItem(name);
                                const XMLCh *myName = nameNode->getNodeValue();
                                char *aName = XMLString::transcode(myName);
                                parmName = aName;
                                itsRecord[ttName].asRecord()["parameters"].asRecord().insert(parmName, *new record());
                                paramOrder.push_back(parmName);
                                DOMNode *ignoreNode = theAttributes->getNamedItem(ignorecase);
                                if(ignoreNode){
                                    const XMLCh *myType = ignoreNode->getNodeValue();
                                    char *aType = XMLString::transcode(myType);
                                    itsRecord[ttName].asRecord()["parameters"].asRecord()[parmName].asRecord().insert("ignorecase", aType);
				}
                                DOMNode *typeNode = theAttributes->getNamedItem(type);
                                if(typeNode){
                                    const XMLCh *myType = typeNode->getNodeValue();
                                    char *aType = XMLString::transcode(myType);
                                    itsRecord[ttName].asRecord()["parameters"].asRecord()[parmName].asRecord().insert("type", aType);
                                    paramType = setType(aType);
                                    typeIsAny = string(aType) == string("any");
                                    // XMLString::release(&myType);
                                    //XMLString::release(&aType);
                                }
                                // Need to handle limittypes here for param
                                DOMNode *unitsNode = theAttributes->getNamedItem(units);
                                if(unitsNode){
                                    const XMLCh *myUnits = unitsNode->getNodeValue();
                                    char *aUnit = XMLString::transcode(myUnits);
                                    itsRecord[ttName].asRecord()["parameters"].asRecord()[parmName].asRecord().insert("units", aUnit);
                                }
                                DOMNode *extantNode = theAttributes->getNamedItem(mustexist);
                                if(extantNode){
                                    const XMLCh *myVal = extantNode->getNodeValue();
                                    char *aVal = XMLString::transcode(myVal);
                                    if(XMLString::equals("true", aVal) || XMLString::equals("yes", aVal)){
                                        itsRecord[ttName].asRecord()["parameters"].asRecord()[parmName].asRecord().insert("mustexist", true);
                                    } else {
                                        itsRecord[ttName].asRecord()["parameters"].asRecord()[parmName].asRecord().insert("mustexist", false);
                                    }
                                    //XMLString::release(&aVal);
                                }
                                DOMNode *blankNode = theAttributes->getNamedItem(allowblank);
                                if(blankNode){
                                    const XMLCh *myVal = blankNode->getNodeValue();
                                    char *aVal = XMLString::transcode(myVal);
                                    if(XMLString::equals("true", aVal) || XMLString::equals("yes", aVal)){
                                        itsRecord[ttName].asRecord()["parameters"].asRecord()[parmName].asRecord().insert("allowblank", true);
                                    } else {
                                        itsRecord[ttName].asRecord()["parameters"].asRecord()[parmName].asRecord().insert("allowblank", false);
                                    }
                                    //XMLString::release(&aVal);
                                }

                                // check for kind
                                DOMNode *kindNode = theAttributes->getNamedItem(kind);
                                if(kindNode) {
                                    const XMLCh *myKind = kindNode->getNodeValue();
                                    char *aKind = XMLString::transcode(myKind);
                                    itsRecord[ttName].asRecord()["parameters"].asRecord()[parmName].asRecord().insert("kind", aKind);
                                }
                            }
                            //
                            // Now loop through the kids picking up useful bits like
                            //  the description and allowed values
                            //
                            DOMNodeList *childNodes = theNode->getChildNodes();
                            for(XMLSize_t l=0;l<childNodes->getLength();l++){
                                DOMNode *childNode = childNodes->item(l);
                                // if(childNode->getNodeType() == DOMNode::ELEMENT_NODE &&
                                //cerr << "The node is: " << XMLString::transcode(childNode->getNodeName()) << endl;
                                if(XMLString::equals(description, childNode->getNodeName())){
                                    const XMLCh *myDesc = childNode->getTextContent();
                                    char *aDesc = XMLString::transcode(myDesc);
                                    //cerr << "description value is: " << aDesc << endl;
                                    itsRecord[ttName].asRecord()["parameters"].asRecord()[parmName].asRecord().insert("description", string(aDesc));
                                    //XMLString::release(&aDesc);
                                }
                                // element allowed, attributes: enum, range (values chan have attributes min, max)
                                // element any, attributes: record, variant
                                if(XMLString::equals(allowed, childNode->getNodeName())){
                                    DOMNamedNodeMap *theAttributes = childNode->getAttributes();
                                    DOMNode *kindNode = theAttributes->getNamedItem(kind);
                                    if(kindNode){
                                        const XMLCh *kindType = kindNode->getNodeValue();
                                        if(XMLString::equals(kindType, ienum)){
                                            itsRecord[ttName].asRecord()["parameters"].asRecord()[parmName].asRecord().insert("allowed", *itsvalue(childNode, paramType));
                                        } else if(XMLString::equals(kindType, range)){
                                            // std::cerr << "Adding ranges" << std::endl;
                                            itsRecord[ttName].asRecord()["parameters"].asRecord()[parmName].asRecord().insert("range", *itsvalue(childNode, paramType));
                                        } else {
                                            std::cerr << "Unknown allowed attribute" << std::endl;
                                        }
                                    }
                                }
                                if(XMLString::equals(value, childNode->getNodeName())){
                                    if(typeIsAny) {
                                        // get the type from the <value type="type"> tag
                                        DOMNamedNodeMap *theAttributes = childNode->getAttributes();
                                        DOMNode *typeNode = theAttributes->getNamedItem(type);
                                        if(typeNode) {
                                            const XMLCh *aType = typeNode->getNodeValue();
                                            paramType = setType(XMLString::transcode(aType));
                                        }
                                    }
                                    itsRecord[ttName].asRecord()["parameters"].asRecord()[parmName].asRecord().insert("value", *itsvalue(childNode, paramType));
                                }
                                // examples
                                if(XMLString::equals(example, childNode->getNodeName())) {
                                    const XMLCh *myExample = childNode->getTextContent();
                                    char *aExample = XMLString::transcode(myExample);
                                    
                                    record& r = itsRecord[ttName].asRecord()["parameters"].asRecord()[parmName].asRecord();
                                    if(r.find("example") == r.end()) {
                                        r.insert("example", *new variant(vector<string>(1, aExample)));
                                    } else {
                                        r["example"].push(string(aExample));
                                    }
                                }

                                // get limit types and put them in a string array
                                if(XMLString::equals(any, childNode->getNodeName())) {
                                    DOMNamedNodeMap *theAttributes = childNode->getAttributes();
                                    DOMNode *ltNode1 = theAttributes->getNamedItem(limittype),
                                    *ltNode2 = theAttributes->getNamedItem(limittypes);

                                    if(ltNode1 || ltNode2) {
                                        const XMLCh *myTypes;
                                        if(ltNode1) myTypes = ltNode1->getNodeValue();
                                        else myTypes = ltNode2->getNodeValue();

                                        vector<string> v;
                                        stringstream ss(XMLString::transcode(myTypes));
                                        string str;
					/*
					while(ss >> str){
					       	v.push_back(str);
					*/
                                        while(ss >> str){
						// The xml uses Array and variant uses vec so we make the change
						// so we don't get false failures for limittype failures
						ssize_t hasArray = str.find("Array");
						if(hasArray != string::npos){
						   str.replace(hasArray, 5, string("vec"));
						}
					       	v.push_back(str);
					}

                                        itsRecord[ttName].asRecord()["parameters"].asRecord()[parmName].asRecord().insert(
                                                "limittypes", *(new variant(v)));
                                    }
                                }
                            }
                        }
                        // insert the paramater order
                        itsRecord[ttName].asRecord().insert("parameterorder", *new variant(paramOrder));
                    }
                    // 
                    // Following section deals with the constraints block
                    //
                    record *constraintsRec(0);
                    XMLCh *constraints = XMLString::transcode("constraints");
                    DOMNodeList *constraintsNodes = dynamic_cast<DOMElement *>(theNode)->getElementsByTagName(constraints);
                    for(XMLSize_t j=0; j<constraintsNodes->getLength(); j++){
                        if(!constraintsRec)
                            constraintsRec = new record;
                        XMLCh *when = XMLString::transcode("when");
                        DOMNodeList *whenNodes = dynamic_cast<DOMElement *>(constraintsNodes->item(j))->getElementsByTagName(when);
                        for(XMLSize_t k=0; k<whenNodes->getLength(); k++){
                            record whenRec;
                            char *whenName(0);
                            // cerr << endl << "when param ";
                            DOMNode *theNode = whenNodes->item(k);
                            DOMNamedNodeMap *theAttributes = theNode->getAttributes();
                            if(theAttributes){
                                DOMNode *nameNode = theAttributes->getNamedItem(param);
                                const XMLCh *myName = nameNode->getNodeValue();
                                char *aName = XMLString::transcode(myName);
                                // cerr << aName;
                                whenName = aName;
                            } else {
                                // cerr << endl;
                            }
                            XMLCh *equals = XMLString::transcode("equals");
                            DOMNodeList *equalsNodes = dynamic_cast<DOMElement *>(whenNodes->item(k))->getElementsByTagName(equals);
                            for(XMLSize_t l=0; l<equalsNodes->getLength(); l++){
                                record equalsRec;
                                char *equalsName(0);
                                DOMNode *theNode = equalsNodes->item(l);
                                DOMNamedNodeMap *theAttributes = theNode->getAttributes();
                                if(theAttributes){
                                    DOMNode *nameNode = theAttributes->getNamedItem(value);
                                    const XMLCh *myName = nameNode->getNodeValue();
                                    char *aName = XMLString::transcode(myName);
                                    // cerr << endl  << "\t is " << aName;
                                    equalsName = aName;
                                } else {
                                    // cerr << endl;
                                }
                                XMLCh *idefault = XMLString::transcode("default");
                                DOMNodeList *defaultNodes = dynamic_cast<DOMElement *>(equalsNodes->item(l))->getElementsByTagName(idefault);
                                if(defaultNodes->getLength() > 0){
                                    record defRecord;
                                    equalsRec.insert("defaults", defRecord);
                                }
                                for(XMLSize_t m=0; m<defaultNodes->getLength(); m++){
                                    record defRec;
                                    char *defName(0);
                                    DOMNode *theNode = defaultNodes->item(m);
                                    DOMNamedNodeMap *theAttributes = theNode->getAttributes();
                                    if(theAttributes){
                                        DOMNode *nameNode = theAttributes->getNamedItem(param);
                                        const XMLCh *myName = nameNode->getNodeValue();
                                        char *aName = XMLString::transcode(myName);
                                        defName = aName;
                                        // cerr << endl << "\t\tset " << aName;
                                    } else {
                                        // cerr << endl;
                                    }
                                    // Here fetch the type from the tool/task record for the name
                                    if(defName){                                        
                                        variant::TYPE eh(variant::STRING);
                                        // std::cerr << " default: " << defName << std::endl;
                                        DOMNodeList *descNode = dynamic_cast<DOMElement *>(theNode)->getElementsByTagName(description);
                                        if(descNode->getLength()){
                                            const XMLCh *xmlText = descNode->item(0)->getTextContent();
                                            char *theDescription = XMLString::transcode(xmlText);
                                            if(theDescription){
                                                // std::cerr << "the description: " << theDescription << std::endl;
                                                defRec.insert("description", theDescription);
                                            } else {
                                                defRec.insert("description", "");
                                            }
                                        }
                                        // check that defName is defined..
                                        record& tempRec = itsRecord[ttName].asRecord()["parameters"].asRecord();
                                        if(tempRec.find(defName) != tempRec.end()) {                                            
                                            DOMNodeList *valNodes = dynamic_cast<DOMElement *>(theNode)->getElementsByTagName(value);
                                            if(valNodes->getLength() > 0) {
                                                // if the <value> tag as a type, use that.
                                                theAttributes = valNodes->item(0)->getAttributes();
                                                if(theAttributes && theAttributes->getNamedItem(type)) {
                                                    DOMNode *typeNode = theAttributes->getNamedItem(type);
                                                    const XMLCh *myType = typeNode->getNodeValue();
                                                    char *aType = XMLString::transcode(myType);
                                                    if(string(aType) != "vector" || tempRec[defName].asRecord()["type"].asString() == "any")
                                                        eh = setType(aType);
                                                    else eh = setType(tempRec[defName].asRecord()["type"].asString());
                                                } else
                                                    eh = setType(tempRec[defName].asRecord()["type"].asString());
                                                
                                                defRec.insert("value", *itsvalue(valNodes->item(0), eh));
                                                equalsRec["defaults"].asRecord().insert(defName, defRec);
                                            }
                                            
                                            // check that whenName is defined..
                                            if(tempRec.find(whenName) != tempRec.end()) {
                                                // add to subparams list
                                                record& parentRec = tempRec[whenName].asRecord();
                                                if(parentRec.find("subparameters") == parentRec.end())
                                                    parentRec.insert("subparameters", vector<string>());
                                                // make sure it isn't already there
                                                bool found = false;
                                                for(unsigned int a = 0; a < parentRec["subparameters"].asStringVec().size(); a++) {
                                                    if(parentRec["subparameters"].asStringVec()[a] == defName) {
                                                        found = true;
                                                        break;
                                                    }
                                                }
                                                if(!found) {
                                                    parentRec["subparameters"].push(string(defName));
                                                }
                                            }
                                        }
                                    }
                                }
                                if(equalsName)
                                    whenRec.insert(equalsName, equalsRec);
                            }
                            if(whenName)
                                constraintsRec->insert(whenName, whenRec);
                        }
                    }
                    if(constraintsRec)
                        itsRecord[ttName].asRecord().insert("constraints", constraintsRec);
                    
                    // shortdescription, description, and example
                    DOMNodeList *descNodes = dynamic_cast<DOMElement*>(theNode)->getElementsByTagName(description);
                    if(descNodes->getLength() > 0) {
                        const XMLCh *desc = descNodes->item(0)->getTextContent();
                        char *aDesc = XMLString::transcode(desc);
                        itsRecord[ttName].asRecord().insert("description", aDesc);
                    }
                    descNodes = dynamic_cast<DOMElement*>(theNode)->getElementsByTagName(shortdesc);
                    if(descNodes->getLength() > 0) {
                        const XMLCh *sdesc = descNodes->item(0)->getTextContent();
                        char *aSdesc = XMLString::transcode(sdesc);
                        itsRecord[ttName].asRecord().insert("shortdescription", aSdesc);
                    }
                    descNodes = dynamic_cast<DOMElement*>(theNode)->getElementsByTagName(example);
                    if(descNodes->getLength() > 0) {
                        const XMLCh *ex = descNodes->item(0)->getTextContent();
                        char *aEx = XMLString::transcode(ex);
                        itsRecord[ttName].asRecord().insert("example", aEx);
                    }
                }
            }
            // cerr << endl;
            rstat = true;
        }
    }
    catch (const XMLException& toCatch) {
        char* message = XMLString::transcode(toCatch.getMessage());
        cout << "Exception message is: \n"
        << message << "\n";
        XMLString::release(&message);
    }
    catch (...) {
        cout << "Unexpected Exception \n" ;
    }
    delete parser;
    return rstat;
}

bool stdcasaXMLUtil::writeXMLFile(const string &fileName, const record &inRec)
{
    bool rstatus(true);
    ofstream ofs(fileName.c_str());
    ofs << "<?xml version=\"1.0\" encoding=\"UTF-8\"?> " << endl;
    ofs << "<?xml-stylesheet type=\"text/xsl\" ?> " << endl;
    ofs << "<casaxml xmlns=\"http://casa.nrao.edu/schema/psetTypes.html\" " << endl;
    ofs << "xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" " << endl;
    ofs << "xsi:schemaLocation=\"http://casa.nrao.edu/schema/casa.xsd " << endl;
    ofs << "file:///opt/casa/code/xmlcasa/xml/casa.xsd\"> " << endl;

    ofs << "<casaparams>" << endl;
    rstatus = fromCasaRecord(ofs, inRec);
    ofs << "</casaparams>" << endl;
    ofs << "</casaxml>" << endl;
    return rstatus;
}

variant *stdcasaXMLUtil::tovariant(variant::TYPE theType, string &theInput, bool isVector)
{
    variant *rstat(0);
    // Lump the VECs with the scalars as we only expect one 
    // value element, which has on
    switch(theType){
    case variant::BOOL :
    case variant::BOOLVEC :
    {
        bool val(false);
        if(theInput == "True" || theInput == "true"){
            val = true;
        } 
        if(isVector || theType == variant::BOOLVEC)
            rstat = new variant(vector<bool>(1,val));
        else
            rstat = new variant(val);
    }
    break;
    case variant::INT :
    case variant::INTVEC :
    {
        int val(0);
        if(theInput != "none"){
            istringstream iss(theInput.c_str());
            iss >> val;
        } else {
            val = INT_MAX;
        }
        if(isVector || theType == variant::INTVEC)
            rstat = new variant(vector<int>(1,val));
        else
            rstat = new variant(val);
    }
    break;
    case variant::DOUBLE :
    case variant::DOUBLEVEC :
    {
        double val(0);
        if(theInput != "none"){
            istringstream iss(theInput.c_str());
            iss >> val;
        } else {
            val = NAN;
        }
        if(isVector || theType == variant::DOUBLEVEC)
            rstat = new variant(vector<double>(1,val));
        else
            rstat = new variant(val);
    }
    break;
    case variant::COMPLEX :
    case variant::COMPLEXVEC :
    {
        std::complex<double> val;
        if(theInput != "none"){
            istringstream iss(theInput.c_str());
            iss >> val;
        } else {
            val = NAN;
        }
        if(isVector || theType == variant::COMPLEXVEC)
            rstat = new variant(vector<complex<double> >(1,val));
        else
            rstat = new variant(val);
    }
    break;
    case variant::STRING :
    case variant::STRINGVEC :
        if(isVector || theType == variant::STRINGVEC)
            rstat = new variant(vector<string>(1,theInput));
        else
            rstat = new variant(theInput);
        break;
    case variant::RECORD :
        // for now, don't bother parsing
        rstat = new variant(*new record());
        break;
    default :
        rstat = new variant;
        break;
    }
    if(!rstat)
        rstat = new variant;
    return rstat;
}
// If there are no valueNodes then we must assign the current value!
variant *stdcasaXMLUtil::itsvalue(DOMNode *theNode, variant::TYPE itsType){
    variant *rstatus(0);
    record *rangeRec(0);
    bool isVector(false);
    DOMNodeList *valueNodes = dynamic_cast<DOMElement *>(theNode)->getElementsByTagName(value);
    if(valueNodes->getLength() > 1){
        isVector = true;
    }
    if(!valueNodes->getLength()) {
        isVector = itsType == variant::STRINGVEC || itsType == variant::BOOLVEC ||
                   itsType == variant::DOUBLEVEC || itsType == variant::COMPLEXVEC ||
                   itsType == variant::INTVEC;
        const XMLCh *myValue = theNode->getTextContent();
        char *aValue = XMLString::transcode(myValue);        
        string sValue(aValue);
        
        bool empty = true;
        for(unsigned int i = 0; i < sValue.size(); i++) {
            if(sValue[i] != ' ' && sValue[i] != '\t' && sValue[i] != '\n' &&
               sValue[i] != '\t') {
                empty = false;
                break;
            }
        }
        
        if(isVector && empty) {
            if(itsType == variant::STRINGVEC) rstatus = new variant(vector<string>());
            else if(itsType == variant::BOOLVEC) rstatus = new variant(vector<bool>());
            else if(itsType == variant::DOUBLEVEC) rstatus = new variant(vector<double>());
            else if(itsType == variant::COMPLEXVEC) rstatus = new variant(vector<complex<double> >());
            else if(itsType == variant::INTVEC) rstatus = new variant(vector<int>());            
        } else if(aValue) {
            rstatus = tovariant(itsType, sValue, isVector);
        }
    }
    for(XMLSize_t n=0; n<valueNodes->getLength(); n++){
        DOMNode *theNode = valueNodes->item(n);
        DOMNamedNodeMap *theAttributes = theNode->getAttributes();
        if(theAttributes && theAttributes->getLength() > 0){
            DOMNode *typeNode = theAttributes->getNamedItem(type);
            DOMNode *rangeNode = theAttributes->getNamedItem(range);
            if(typeNode){
                const XMLCh *myType = typeNode->getNodeValue();
                char *aType = XMLString::transcode(myType);
                string sType = aType ? aType : "";
                if(aType == string("vector") || (sType.size() > 5 && sType.substr(sType.size() - 4) == "Array")) {
                    /*
                        aType == string("intArray") ||
                   aType == string("doubleArray") || aType == string("stringArray") ||
                   aType == string("complexArray") || aType == string("boolArray")) {
                   */
                    DOMNodeList *vectorNodes =
                        dynamic_cast<DOMElement *>(valueNodes->item(n))->getElementsByTagName(value);
                        //cerr << XMLString::transcode(valueNodes->item(n)->getNodeName()) << " " << vectorNodes->getLength() << endl;
                        for(XMLSize_t o=0; o<vectorNodes->getLength(); o++){
                            DOMNode *theNode = vectorNodes->item(o);
                            const XMLCh *myValue = theNode->getTextContent();
                            char *aValue = XMLString::transcode(myValue);
                            if(aValue) {
                                string tmp(aValue);
                                if(!o)
                                    rstatus = tovariant(itsType, tmp, isVector);
                                else
                                    addtovariant(rstatus, itsType, tmp);
                            }
                        }
                        break; // Now we've gotten all the vector
                } else {
                    const XMLCh *myValue = theNode->getTextContent();
                    char *aValue = XMLString::transcode(myValue);
                    if(aValue){
                        string tmp = aValue;
                        if(!n)
                            rstatus = tovariant(itsType, tmp, isVector);
                        else
                            addtovariant(rstatus, itsType, tmp);
                    }
                }
            } else if(rangeNode){
                char *rangeType = XMLString::transcode(rangeNode->getNodeValue());
                if(!rangeRec){
                    rangeRec = new record;
                }
                if(!rangeRec->count(rangeType)){
                    record rangeOps;
                    vector<double> avec;
                    vector<string> svec;
                    rangeOps.insert("value", avec);
                    rangeOps.insert("inclusive", svec);
                    rangeRec->insert(rangeType, rangeOps);
                }
                const XMLCh *inclusive = XMLString::transcode("inclusive");
                DOMNode *rangeNode = theAttributes->getNamedItem(inclusive);
                char *rangeInclude(0);
                if(rangeNode)
                    rangeInclude = XMLString::transcode(theAttributes->getNamedItem(inclusive)->getNodeValue());
                if(rangeNode && rangeInclude)
                    (*rangeRec)[rangeType].asRecord()["inclusive"].push(string(rangeInclude));
                else
                    (*rangeRec)[rangeType].asRecord()["inclusive"].push(string());
                const XMLCh *myValue = theNode->getTextContent();
                char *aValue = XMLString::transcode(myValue);
                if(aValue) {
                    istringstream iss(aValue);
                    double myval;
                    iss >> myval;
                    (*rangeRec)[rangeType].asRecord()["value"].asDoubleVec().push_back(myval);
                    (*rangeRec)[rangeType].asRecord()["value"].shape() = vector<int>(1,
                            (*rangeRec)[rangeType].asRecord()["value"].asDoubleVec().size());
                }
            } else {
                const XMLCh *myValue = theNode->getTextContent();
                char *aValue = XMLString::transcode(myValue);
                if(aValue) {
                    string tmp(aValue);
                    if(!n)
                        rstatus = tovariant(itsType, tmp, isVector);
                    else
                        addtovariant(rstatus, itsType, tmp);
                }
            }
        } else {
            const XMLCh *myValue = theNode->getTextContent();
            char *aValue = XMLString::transcode(myValue);
            if(aValue){
                string tmp(aValue);
                if(!n)
                    rstatus = tovariant(itsType, tmp, isVector);
                else 
                    addtovariant(rstatus, itsType, tmp);
            }
            // }
        }
    }
    if(rangeRec)
        rstatus = new variant(rangeRec);
    if(!rstatus)
        rstatus = new variant();
    return rstatus;
}


variant::TYPE stdcasaXMLUtil::setType(const string &xmlType){
    variant::TYPE theType(variant::BOOL);
    if(xmlType == "double"){
        theType = variant::DOUBLE;
    } else if(xmlType == "int"){
        theType = variant::INT;
    } else if(xmlType == "bool"){
        theType = variant::BOOL;
    } else if(xmlType == "complex"){
        theType = variant::COMPLEX;
    } else if(xmlType == "string"){
        theType = variant::STRING;
    } else if(xmlType == "doubleArray"){
        theType = variant::DOUBLEVEC;
    } else if(xmlType == "intArray"){
        theType = variant::INTVEC;
    } else if(xmlType == "boolArray"){
        theType = variant::BOOLVEC;
    } else if(xmlType == "complexArray"){
        theType = variant::COMPLEXVEC;
    } else if(xmlType == "stringArray"){
        theType = variant::STRINGVEC;
    } else if(xmlType == "record"){
        theType = variant::RECORD;
    }
    return theType;
}

void stdcasaXMLUtil::addtovariant(variant *theVariant, variant::TYPE theType, string &theInput){
    switch(theType) {
    case variant::BOOL :
    case variant::BOOLVEC :
    {
        bool val(false);
        if(theInput == "True" || theInput == "true"){
            val = true;
        } 
        //vector<bool> &members = theVariant->asBoolVec();
        theVariant->getBoolVecMod().push_back(val);
        theVariant->shape() = vector<int>(1, theVariant->asBoolVec().size());
    }
    break;
    case variant::INT:
    case variant::INTVEC :
    {
        int val(0);
        if(theInput != "none"){
            istringstream iss(theInput.c_str());
            iss >> val;
        } else {
            val = INT_MAX;
        }
        // vector<int> &members = theVariant->asIntVec();
        theVariant->getIntVecMod().push_back(val);
        theVariant->shape() = vector<int>(1, theVariant->asIntVec().size());
    }
    break;
    case variant::DOUBLE:
    case variant::DOUBLEVEC :
    {
        double val(0);
        if(theInput != "none"){
            istringstream iss(theInput.c_str());
            iss >> val;
        } else {
            val = NAN;
        }
        //vector<double> &members = theVariant->asDoubleVec();
        theVariant->getDoubleVecMod().push_back(val);
        theVariant->shape() = vector<int>(1, theVariant->asDoubleVec().size());
    }
    break;
    case variant::COMPLEX:
    case variant::COMPLEXVEC :
    {
        complex<double>  val(0);
        if(theInput != "none"){
            istringstream iss(theInput.c_str());
            iss >> val;
        } else {
            val = NAN;
        }
        //vector<complex<double> > &members = theVariant->asComplexVec();
        theVariant->getComplexVecMod().push_back(val);
        theVariant->shape() = vector<int>(1, theVariant->asComplexVec().size());
    }
    break;
    case variant::STRING :
    case variant::STRINGVEC :
        //vector<string> &members = theVariant->asStringVec();
        theVariant->push(theInput);
        break;
    case variant::RECORD :
    default :
        break;
    }
    return;
}


}  //End of namespace casac
