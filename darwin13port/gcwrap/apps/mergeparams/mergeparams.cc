#include <string>
#include <iostream>
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

int loopit(Wrapper4InputSource &xmlParamSource, Wrapper4InputSource &xmlSource);

int main(int argc, char **argv){
   XMLPlatformUtils::Initialize();
   XMLCh *xmlParamFile = XMLString::transcode(argv[1]);
   XMLCh *xmlParamRPath = XMLString::transcode(".");
   LocalFileInputSource xmlParamSource(xmlParamRPath, xmlParamFile);
   XMLCh *xmlFile = XMLString::transcode(argv[2]);
   XMLCh *xmlRPath = XMLString::transcode(".");
   LocalFileInputSource xmlSource(xmlRPath, xmlFile);
   LocalFileInputSource xmlParams(xmlRPath, xmlParamFile);
   Wrapper4InputSource theParamXML(&xmlParamSource, false);
   Wrapper4InputSource theXML(&xmlSource, false);
   loopit(theParamXML, theXML);

   XMLString::release(&xmlFile);
   XMLString::release(&xmlRPath);
   XMLString::release(&xmlParamFile);
   XMLString::release(&xmlParamRPath);
   return 0;
}
int loopit(Wrapper4InputSource &theParams, Wrapper4InputSource &xmlSource){
    XMLCh tempStr1[3] = {chLatin_L, chLatin_S, chNull};
    DOMImplementation *impl1          = DOMImplementationRegistry::getDOMImplementation(tempStr1);
    DOMLSSerializer   *theSerializer = ((DOMImplementationLS*)impl1)->createLSSerializer();
    DOMLSOutput       *theOutputDesc = ((DOMImplementationLS*)impl1)->createLSOutput();
    XMLFormatTarget *myFormTarget;
//if (goutputfile)
//myFormTarget=new LocalFileFormatTarget(goutputfile);
//else
    myFormTarget=new StdOutFormatTarget();
    theOutputDesc->setByteStream(myFormTarget);

    XMLCh tempStr[100];
    XMLString::transcode("LS", tempStr, 99);
    DOMImplementationLS *impl = DOMImplementationRegistry::getDOMImplementation(tempStr);
    DOMLSParser* parser = impl->createLSParser(DOMImplementationLS::MODE_SYNCHRONOUS, 0);
    DOMDocument *doc(0), *doc2(0);
    try{
       doc2 = parser->parse(&theParams);
       doc = parser->parse(&xmlSource);
       if(doc){
	  DOMElement *theMethod = doc->getDocumentElement();
	  DOMElement *theParamsMethod = doc2->getDocumentElement();
	  XMLCh *theParam = XMLString::transcode("iparam");
	  DOMNodeList *allNodes = theMethod->getElementsByTagName(theParam);
	  XMLCh *theParams = XMLString::transcode("param");
	  DOMNodeList *paramNodes = theParamsMethod->getElementsByTagName(theParams);
	  XMLSize_t numParams = paramNodes->getLength();
	  for(XMLSize_t i=0; i<allNodes->getLength(); i++){
		  DOMNode *theNode=allNodes->item(i);
		  DOMNamedNodeMap *theAttributes = theNode->getAttributes();
		  DOMNode *nameNode = theAttributes->getNamedItem(XMLString::transcode("name"));
		  const XMLCh *myName = nameNode->getNodeValue();
		  char *aName = XMLString::transcode(myName);
		  for(XMLSize_t j=0; j<numParams; j++){
		     DOMNode *paramNode=paramNodes->item(j);
		     DOMNamedNodeMap *theParamAttributes = paramNode->getAttributes();
		     DOMNode *nameParam = theParamAttributes->getNamedItem(XMLString::transcode("name"));
		     const XMLCh *paramName = nameParam->getNodeValue();
		     if(!strcmp(aName, XMLString::transcode(paramName))){
			     //std::cerr << " " << theParamAttributes->getLength() << " ";
			     for(XMLSize_t k=0; k<theParamAttributes->getLength(); k++){
				     //std::cerr << " " << XMLString::transcode((theParamAttributes->item(k))->getNodeName());
				 if(!((DOMElement *)theNode)->hasAttribute(theParamAttributes->item(k)->getNodeName())){
					 DOMNode *thehit = theParamAttributes->item(k);
					 const XMLCh *attr = thehit->getNodeName();
					 const XMLCh *itsval = thehit->getNodeValue();
					 ((DOMElement *)theNode)->setAttribute(attr, itsval);
				 }
		             }
			     DOMNodeList *children = theNode->getChildNodes();
			     DOMNodeList *paramChildren = paramNode->getChildNodes();
			     //std::cerr << children->getLength() << std::endl;
			     for(XMLSize_t l=0; l<paramChildren->getLength(); l++){
				XMLSize_t k(0);
			        for(k=0; k<children->getLength(); k++){
				     //std::cerr << "*" << XMLString::transcode(children->item(k)->getNodeName()) << "* *"
				               //<< XMLString::transcode(paramChildren->item(l)->getNodeName()) << "* "<< std::endl;
				     if(!strcmp(XMLString::transcode(children->item(k)->getNodeName()), XMLString::transcode(paramChildren->item(l)->getNodeName())))
			                     // std::cerr << " A hit ";
					     break;
			        }
				if(!k || k >= children->getLength()){
					//if(strcmp("#text", XMLString::transcode(paramChildren->item(l)->getNodeName()))){
			                   //std::cerr << " It's A hit ";
					   DOMNode *deep = doc->importNode(paramChildren->item(l), true);
					   //std::cerr << XMLString::transcode(deep->getNodeName()) << std::endl;
					   ((DOMElement *)theNode)->appendChild(deep);
					//}
				}
			     }
		      }
		  }
		  //std::cerr << aName << std::endl;
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
