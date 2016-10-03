#ifndef CASAXMLUTILS_H
#define CASAXMLUTILS_H

//*
//The decoding was change from the original CasaXMLUtil.
//Here I am interested in parsing ASDM XML tables, which
//have different structure. All the work is done by asdmCasaSaXHandler class
//
//P. C. Cortes, July 2007
//
namespace casacore{

class String;
class Record;
class Table;
}

namespace casa {


class asdmCasaXMLUtil {
   public :
	asdmCasaXMLUtil();
	~asdmCasaXMLUtil();
	bool toCasaRecord(casacore::Record &outRec, casacore::String &xml);
	bool fromCasaRecord(casacore::String &outXML, const casacore::Record &theRecord);
	bool readXMLFile(casacore::Table &outTab, const casacore::String &xmlFile, const casacore::String &tableName);
	bool writeXMLFile(const casacore::String &xmlFile, const casacore::Record &inRec);
	void setRecord(casacore::Record &aRec);
};

}
#endif
