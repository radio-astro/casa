#include <tools/xerces/asdmCasaSaxHandler.h>
#include <tools/xerces/asdmCasaXMLUtil.h>
#include <xercesc/sax2/SAX2XMLReader.hpp>
#include <xercesc/sax2/XMLReaderFactory.hpp>
#include <xercesc/sax2/DefaultHandler.hpp>
#include <xercesc/util/XMLString.hpp>
#include <iostream>
#include <casa/Containers/Record.h>
#include <casa/BasicSL/String.h>
#include <casa/Exceptions/Error.h>

using namespace casa;
using namespace std;
using namespace xercesc;

namespace casa {

asdmCasaXMLUtil::asdmCasaXMLUtil()
{
   try {
      XMLPlatformUtils::Initialize();
   }
   catch (const XMLException& toCatch) {
      char* message = XMLString::transcode(toCatch.getMessage());
      cout << "Error during initialization! :\n";
      cout << "Exception message is: \n"
      << message << "\n";
      XMLString::release(&message);
   }
}

asdmCasaXMLUtil::~asdmCasaXMLUtil()
{
   XMLPlatformUtils::Terminate();
}

Bool asdmCasaXMLUtil::toCasaRecord(Record &outRec, String &inString)
{
   Bool rstat(false);
   return rstat;
}

Bool asdmCasaXMLUtil::fromCasaRecord(String &outString, const Record &inRec)
{
   Bool rstat(false);
   return rstat;
}

Bool asdmCasaXMLUtil::readXMLFile(Table &outTab, const String &fileName, const String &tableName)
{
   Bool rstat(false);
   const char* xmlFile = fileName.c_str();
   SAX2XMLReader* parser = XMLReaderFactory::createXMLReader();
   parser->setFeature(XMLUni::fgSAX2CoreValidation, true); // optional 
   parser->setFeature(XMLUni::fgSAX2CoreNameSpaces, true); // optional 
   asdmCasaSaxHandler* theHandler = new asdmCasaSaxHandler(outTab,tableName);
   parser->setContentHandler(theHandler);
   parser->setErrorHandler(theHandler);
   try {
      parser->parse(xmlFile);
      rstat = True;
   }
   catch (const XMLException& toCatch) {
      char* message = XMLString::transcode(toCatch.getMessage());
      cout << "Exception message is: \n"
      << message << "\n";
      XMLString::release(&message);
   }
   catch (const SAXParseException& toCatch) {
      char* message = XMLString::transcode(toCatch.getMessage());
      cout << "Exception message is: \n"
      << message << "\n";
      XMLString::release(&message);
   }
   catch (AipsError &x){
	   cerr << x.getMesg() << endl;
   }
   catch (...) {
      cout << "Unexpected Exception \n" ;
   }
   delete parser;
   delete theHandler;
   return rstat;
}

Bool asdmCasaXMLUtil::writeXMLFile(const String &fileName, const Record &inRec)
{
   Bool rstat(false);
   return rstat;
}

}  //End of namespace casa
