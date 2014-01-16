#include <string>
#include <iostream>
#include <map>
#include <xercesc/dom/DOM.hpp>
#include <xercesc/util/XMLChar.hpp>
#include <xercesc/dom/DOM.hpp>
#include <xercesc/framework/Wrapper4InputSource.hpp>
#include <xercesc/dom/DOM.hpp>
#include <xercesc/util/XMLString.hpp>
#include <xercesc/util/HexBin.hpp>
#include <xercesc/framework/LocalFileInputSource.hpp>
#include <xercesc/framework/MemBufInputSource.hpp>
#include <xercesc/framework/Wrapper4InputSource.hpp>
#include <xercesc/framework/StdOutFormatTarget.hpp>

XERCES_CPP_NAMESPACE_USE;

int loopit(Wrapper4InputSource &xmlParamSource);

int main(int argc, char **argv){
   XMLPlatformUtils::Initialize();
   XMLCh *xmlParamFile = XMLString::transcode(argv[1]);
   XMLCh *xmlParamRPath = XMLString::transcode(".");
   LocalFileInputSource xmlParamSource(xmlParamRPath, xmlParamFile);
   XMLCh *xmlRPath = XMLString::transcode(".");
   LocalFileInputSource xmlParams(xmlRPath, xmlParamFile);
   Wrapper4InputSource theParamXML(&xmlParamSource, false);
   loopit(theParamXML);

   XMLString::release(&xmlParamFile);
   XMLString::release(&xmlParamRPath);
   return 0;
}
int loopit(Wrapper4InputSource &theParams ){
    XMLCh tempStr1[3] = {chLatin_L, chLatin_S, chNull};
    DOMImplementation *impl1          = DOMImplementationRegistry::getDOMImplementation(tempStr1);
    DOMLSSerializer   *theSerializer = ((DOMImplementationLS*)impl1)->createLSSerializer();
    DOMLSOutput       *theOutputDesc = ((DOMImplementationLS*)impl1)->createLSOutput();
    XMLFormatTarget *myFormTarget;
    myFormTarget=new StdOutFormatTarget();
    theOutputDesc->setByteStream(myFormTarget);

    XMLCh tempStr[100];
    XMLString::transcode("LS", tempStr, 99);
    DOMImplementationLS *impl = DOMImplementationRegistry::getDOMImplementation(tempStr);
    DOMLSParser* parser = impl->createLSParser(DOMImplementationLS::MODE_SYNCHRONOUS, 0);
    DOMDocument *doc(0), *doc2(0);
    try{
       doc2 = parser->parse(&theParams);
       doc = impl1->createDocument(0, XMLString::transcode("casaxml"), 0);
       DOMElement *rootElem = doc->getDocumentElement();
       if(doc2){
	  std::map<std::string, int> hits;
	  DOMElement *theParamsMethod = doc2->getDocumentElement();
	  XMLCh *theParams = XMLString::transcode("param");
	  DOMNodeList *paramNodes = theParamsMethod->getElementsByTagName(theParams);
	  XMLSize_t numParams = paramNodes->getLength();
	  for(XMLSize_t i=0; i<paramNodes->getLength(); i++){
		  DOMNode *theNode=paramNodes->item(i);
		  DOMNamedNodeMap *theAttributes = theNode->getAttributes();
		  DOMNode *nameNode = theAttributes->getNamedItem(XMLString::transcode("name"));
		  const XMLCh *myName = nameNode->getNodeValue();
		  char *aName = XMLString::transcode(myName);
		  for(XMLSize_t j=0; j<numParams; j++){
		     DOMNode *paramNode=paramNodes->item(j);
		     DOMNamedNodeMap *theParamAttributes = paramNode->getAttributes();
		     DOMNode *nameParam = theParamAttributes->getNamedItem(XMLString::transcode("name"));
		     const XMLCh *paramName = nameParam->getNodeValue();
		     std::string theParam = std::string(XMLString::transcode(paramName));
		     if(!hits.count(theParam)){
			     hits[theParam] = 1;
			     DOMNodeList *paramChildren = paramNode->getChildNodes();
			     DOMElement *pNode = doc->createElement(XMLString::transcode("param"));
			     for(XMLSize_t k=0; k<theParamAttributes->getLength(); k++){
				     DOMNode *aNode = theParamAttributes->item(k);
				     pNode->setAttribute(aNode->getNodeName(), aNode->getNodeValue());
			     }
			     rootElem->appendChild(pNode);
			     for(XMLSize_t l=0; l<paramChildren->getLength(); l++){
					   DOMNode *deep = doc->importNode(paramChildren->item(l), true);
					   ((DOMElement *)pNode)->appendChild(deep);
			     }
			     // std::cerr << " It's A hit: " << theParam << " " << hits.count(theParam)<< std::endl ;
		      }
		  }
	  }
       }
    } catch (DOMException err){
	    std::cerr << XMLString::transcode(err.getMessage()) << std::endl;
    } catch (...){
	    std::cerr << "Some error" << std::endl;
	    return 1;
    }
    theSerializer->write(doc, theOutputDesc);
    return 0;
}
