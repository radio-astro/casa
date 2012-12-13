#ifndef CASAXMLUTILS_H
#define CASAXMLUTILS_H

//*
//The decoding was change from the original CasaXMLUtil.
//Here I am interested in parsing ASDM XML tables, which
//have different structure. All the work is done by asdmCasaSaXHandler class
//
//P. C. Cortes, July 2007
//
namespace casa {

class String;
class Record;
class Table;

class asdmCasaXMLUtil {
   public :
	asdmCasaXMLUtil();
	~asdmCasaXMLUtil();
	bool toCasaRecord(Record &outRec, String &xml);
	bool fromCasaRecord(String &outXML, const Record &theRecord);
	bool readXMLFile(Table &outTab, const String &xmlFile, const String &tableName);
	bool writeXMLFile(const String &xmlFile, const Record &inRec);
	void setRecord(Record &aRec);
};

}
#endif
