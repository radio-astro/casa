#include <boost/foreach.hpp>

#include <casa/IO/AipsIO.h>
#include <casa/Arrays/ArrayIO.h>
#include <casa/Containers/BlockIO.h>

#include "BDF2AsdmStManIndex.h"

using namespace casa;
using namespace std;
using namespace PrimitiveDataTypeMod;

ostringstream oss;

int BDF2AsdmStManIndex::version() {
  LOGENTER("BDF2AsdmStManIndex::version");
  LOGEXIT("BDF2AsdmStManIndex::version");
  return 1;
}

BDF2AsdmStManIndex::BDF2AsdmStManIndex(const vector<string>& bdfNames, bool isBigEndian, const string& fname) {
  LOGENTER("BDF2AsdmStManIndex::BDF2AsdmStManIndex");
  this->numberOfDataDescriptions = numberOfDataDescriptions;
  this->bdfNames.resize(bdfNames.size());

  int i = 0;
  if (getenv("SHOW_ASDMINDEX"))
    cout << "* isBigEndian = " << isBigEndian << endl;

  BOOST_FOREACH(string bdfName, bdfNames) {
    if (s2i_m.find(bdfName) == s2i_m.end()) {
      this->bdfNames[i] = bdfName;
      s2i_m[bdfName] = i;
      if (getenv("SHOW_ASDMINDEX"))
	cout << "* s2i_m[" << bdfName << "] = " << i << endl;
      i++;	    
    }
  }

  // Write the preamble in the AsdmStMan table.
  aio.open(fname + "asdmindex", ByteIO::New);
  aio.putstart("AsdmStMan", version());
  aio << isBigEndian << this->bdfNames;

  // We start with row # 0 in the MS Main table.
  MSMainRowNumber = 0;

  if (debug) {
    for (map<string, int>::const_iterator iter = s2i_m.begin(); iter != s2i_m.end(); iter++) {
      oss.str("");
      oss << iter->first << "->" << iter->second << endl;
      LOG(oss.str());
    }
  }
  LOGEXIT("BDF2AsdmStManIndex::BDF2AsdmStManIndex");
}

BDF2AsdmStManIndex::~BDF2AsdmStManIndex() {
  LOGENTER("BDF2AsdmStManIndex::~BDF2AsdmStManIndex");
  LOGEXIT("BDF2AsdmStManIndex::~BDF2AsdmStManIndex");
}

void BDF2AsdmStManIndex::setNumberOfDataDescriptions(unsigned int numberOfDataDescriptions) {
  LOGENTER("BDF2AsdmStManIndex::setNumberOfDataDescriptions");
  this->numberOfDataDescriptions = numberOfDataDescriptions;
  clearIndexes();
  autoIndexes_vv.resize(numberOfDataDescriptions);
  crossIndexes_vv.resize(numberOfDataDescriptions);  
  LOGEXIT("BDF2AsdmStManIndex::setNumberOfDataDescriptions");
}

void BDF2AsdmStManIndex::done(){
  LOGENTER("BDF2AsdmStManIndex::done");
  s2i_m.clear();
  autoIndexes_vv.clear();
  crossIndexes_vv.clear();
  aio.put(allIndexes_v);
  aio.putend();
  LOGEXIT("BDF2AsdmStManIndex::done");
}

void BDF2AsdmStManIndex::clearIndexes() {
  LOGENTER("BDF2AsdmStManIndex::clearIndexes");
  autoIndexes_vv.clear();
  crossIndexes_vv.clear();
  LOGEXIT("BDF2AsdmStManIndex::clearIndexes");
}

void BDF2AsdmStManIndex::clearAutoIndexes() {
  LOGENTER("BDF2AsdmStManIndex::clearAutoIndexes");
  autoIndexes_vv.clear();
  LOGEXIT("BDF2AsdmStManIndex::clearAutoIndexes");
}

void BDF2AsdmStManIndex::clearCrossIndexes() {
  LOGENTER("BDF2AsdmStManIndex::clearCrossIndexes");
  crossIndexes_vv.clear();
  LOGEXIT("BDF2AsdmStManIndex::clearCrossIndexes");
}

void BDF2AsdmStManIndex::appendAutoIndex(unsigned int           iDD,
					 const string&		bdfName,
					 unsigned int		nAnt,
					 unsigned int		nSpw,
					 unsigned int		nChan,
					 unsigned int		nPol,
					 unsigned int		stepBl,
					 unsigned int		iSpw,
					 const vector<double>&	scaleFactors,
					 uint64_t		fileOffset,
					 uint32_t               spwOffset) {
  LOGENTER("BDF2AsdmStManIndex::appendAutoIndex");
  AsdmIndex							asdmIndex;

  asdmIndex.fileNr	 = s2i_m[bdfName];
  asdmIndex.nBl		 = nAnt;
  asdmIndex.nSpw	 = nSpw;
  asdmIndex.nChan	 = nChan;
  asdmIndex.nPol	 = nPol;
  asdmIndex.stepBl	 = stepBl;
  asdmIndex.iSpw	 = iSpw;
  asdmIndex.row		 = 0;   // Temporarily because we do not know yet how the MS MAIN DATA is going to be ordered.
  asdmIndex.scaleFactors = scaleFactors;
  asdmIndex.fileOffset	 = fileOffset;
  asdmIndex.blockOffset  = spwOffset; // iDD * nChan * nPol;
  asdmIndex.dataType     = 10;

  autoIndexes_vv[iDD].push_back(asdmIndex);
  LOGEXIT("BDF2AsdmStManIndex::appendAutoIndex");
}

void BDF2AsdmStManIndex::appendWVRIndex(unsigned int            iDD,
					const string&		bdfName,
					unsigned int		nAnt,
					unsigned int		nSpw,
					unsigned int		nChan,
					unsigned int		nPol,
					unsigned int		stepBl,
					unsigned int		iSpw,
					const vector<double>&	scaleFactors,
					uint64_t		fileOffset,
					uint32_t                spwOffset) {
  LOGENTER("BDF2AsdmStManIndex::appendWVRIndex");
  AsdmIndex							asdmIndex;

  asdmIndex.fileNr	 = s2i_m[bdfName];
  asdmIndex.nBl		 = nAnt;
  asdmIndex.nSpw	 = nSpw;
  asdmIndex.nChan	 = nChan;
  asdmIndex.nPol	 = nPol;
  asdmIndex.stepBl	 = stepBl;
  asdmIndex.iSpw	 = iSpw;
  asdmIndex.row		 = 0;   // Temporarily because we do not know yet how the MS MAIN DATA is going to be ordered.
  asdmIndex.scaleFactors = scaleFactors;
  asdmIndex.fileOffset	 = fileOffset;
  asdmIndex.blockOffset  = spwOffset; //iDD * nChan * nPol;
  asdmIndex.dataType     = 10;  // was 11.

  autoIndexes_vv[iDD].push_back(asdmIndex);
  LOGEXIT("BDF2AsdmStManIndex::appendWVRIndex");
}


void BDF2AsdmStManIndex::appendCrossIndex(unsigned int          iDD,
					  const string&		bdfName,
					  unsigned int		nBl,
					  unsigned int		nSpw,
					  unsigned int		nChan,
					  unsigned int		nPol,
					  unsigned int		stepBl,
					  unsigned int		iSpw,
					  const vector<double>&	scaleFactors,
					  uint64_t		fileOffset,
					  uint32_t              spwOffset,
					  PrimitiveDataType     dataType) {
  LOGENTER("BDF2AsdmStManIndex::appendCrossIndex");
  AsdmIndex							asdmIndex;

  asdmIndex.fileNr	 = s2i_m[bdfName];
  asdmIndex.nBl		 = nBl;
  asdmIndex.nSpw	 = nSpw;
  asdmIndex.nChan	 = nChan;
  asdmIndex.nPol	 = nPol;
  asdmIndex.stepBl	 = stepBl;
  asdmIndex.iSpw	 = iSpw;
  asdmIndex.row		 = 0;   // Temporarily because we do not know yet how the MS MAIN DATA is going to be ordered.
  asdmIndex.scaleFactors = scaleFactors;
  asdmIndex.fileOffset	 = fileOffset;
  asdmIndex.blockOffset  = spwOffset; 
  switch (dataType) {
  case INT16_TYPE : asdmIndex.dataType = 0; break;
  case INT32_TYPE : asdmIndex.dataType = 1; break;
  case FLOAT32_TYPE : asdmIndex.dataType = 3; break;
  default: 
    cout << "BDF2AsdmStManIndex::appendCrossIndex: '" << dataType << "' unrecognized crossdata type." << endl;
  }
  asdmIndex.dataType     = dataType;

  crossIndexes_vv[iDD].push_back(asdmIndex);
  LOGEXIT("BDF2AsdmStManIndex::appendCrossIndex");
}

void BDF2AsdmStManIndex::dumpCrossAuto() {
  LOGENTER("BDF2AsdmStManIndex::dumpCrossAuto");
  //
  // Calculate the MS MAIN row number from :
  // * the current MS MAIN row number.
  // * the number of MS MAIN rows for each cross | auto index
  for (unsigned int iDD = 0; iDD < numberOfDataDescriptions; iDD++) {
    for (unsigned int i = 0; i < crossIndexes_vv[iDD].size(); i++) {
      crossIndexes_vv[iDD][i].row = MSMainRowNumber;
      MSMainRowNumber += crossIndexes_vv[iDD][i].nBl * crossIndexes_vv[iDD][i].nSpw;
    }
    for (unsigned int i = 0; i < autoIndexes_vv[iDD].size(); i++) {
      autoIndexes_vv[iDD][i].row = MSMainRowNumber;
      MSMainRowNumber += autoIndexes_vv[iDD][i].nBl * autoIndexes_vv[iDD][i].nSpw;
    }
  }

  if (getenv("SHOW_ASDMINDEX")) {
    for (unsigned int iDD = 0; iDD < numberOfDataDescriptions; iDD++) {
      vector<AsdmIndex>& crossIndexes_v = crossIndexes_vv[iDD];
      for (unsigned int i = 0; i < crossIndexes_v.size(); i++)
	cout << "c: " << crossIndexes_v[i] << endl;

      vector<AsdmIndex>& autoIndexes_v = autoIndexes_vv[iDD];
      for (unsigned int i = 0; i < autoIndexes_v.size(); i++)
	cout << "a: " << autoIndexes_v[i] << endl;
    }
  }

  for (unsigned int iDD = 0; iDD < numberOfDataDescriptions; iDD++) {
    vector<AsdmIndex>& crossIndexes_v = crossIndexes_vv[iDD];
    for (unsigned int i = 0; i < crossIndexes_v.size(); i++)
      allIndexes_v.push_back(crossIndexes_v[i]);
    
    vector<AsdmIndex>& autoIndexes_v = autoIndexes_vv[iDD];
    for (unsigned int i = 0; i < autoIndexes_v.size(); i++)
      allIndexes_v.push_back(autoIndexes_v[i]);
  }
  clearIndexes();
  LOGEXIT("BDF2AsdmStManIndex::dumpCrossAuto");
}

void BDF2AsdmStManIndex::dumpAutoCross() {
  LOGENTER("BDF2AsdmStManIndex::dumpAutoCross");
  //
  // Calculate the MS MAIN row number from :
  // * the current MS MAIN row number.
  // * the number of MS MAIN rows for each auto and then cross index
  for (unsigned int iDD = 0; iDD < numberOfDataDescriptions; iDD++) {
    for (unsigned int i = 0; i < autoIndexes_vv[iDD].size(); i++) {
      autoIndexes_vv[iDD][i].row = MSMainRowNumber;
      MSMainRowNumber += autoIndexes_vv[iDD][i].nBl ; // * autoIndexes_vv[iDD][i].nSpw;
    }
  
    for (unsigned int i = 0; i < crossIndexes_vv[iDD].size(); i++) {
      crossIndexes_vv[iDD][i].row = MSMainRowNumber;
      MSMainRowNumber += crossIndexes_vv[iDD][i].nBl ; //* crossIndexes_vv[iDD][i].nSpw;
    }
  }

  if (getenv("SHOW_ASDMINDEX")) {
    for (unsigned int iDD = 0; iDD < numberOfDataDescriptions; iDD++) {
      vector<AsdmIndex>& autoIndexes_v = autoIndexes_vv[iDD];
      for (unsigned int i = 0; i < autoIndexes_v.size(); i++)
	cout << "a: " << autoIndexes_v[i] << endl;
      
      vector<AsdmIndex>& crossIndexes_v = crossIndexes_vv[iDD];
      for (unsigned int i = 0; i < crossIndexes_v.size(); i++)
	cout << "c: " << crossIndexes_v[i] << endl;
    }
  }

  for (unsigned int iDD = 0; iDD < numberOfDataDescriptions; iDD++) {
    vector<AsdmIndex>& autoIndexes_v = autoIndexes_vv[iDD];
    for (unsigned int i = 0; i < autoIndexes_v.size(); i++)
      allIndexes_v.push_back(autoIndexes_v[i]);

    vector<AsdmIndex>& crossIndexes_v = crossIndexes_vv[iDD];
    for (unsigned int i = 0; i < crossIndexes_v.size(); i++)
      allIndexes_v.push_back(crossIndexes_v[i]);
  }


  clearIndexes();
  LOGEXIT("BDF2AsdmStManIndex::dumpAutoCross");
}
 
