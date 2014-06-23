#include <iostream>
#include <vector>
using namespace std;

#include "ASDM.h"
#include "MainTable.h"
#include "MainRow.h"

#include "SDMBinData.h"
using namespace sdmbin;

#define CHECKROWUNIQUENESS  false
#define BLOCKSIZE 1024*1024

vector<unsigned int> sizeInMemory(unsigned int BDFsize, unsigned int approxSizeInMemory) {
  vector<unsigned int> result;
  unsigned int Q = BDFsize / approxSizeInMemory;
  if (Q == 0) { 
    result.push_back(BDFsize);
  }
  else {
    result.resize(Q, approxSizeInMemory);
    unsigned int R = BDFsize % approxSizeInMemory;
    if ( R > (Q * approxSizeInMemory / 5) )  {
      result.push_back(R); 
    }
    else {
      while (R > 0) 
	for (unsigned int i = 0; R >0 && i < result.size(); i++) {
	  result[i]++ ; R--;
	}
    }
  }
  return result;
} 

int main(int argC, char* argV[]) {
  if (argC < 2) {
    cout << "Usage : TestReadBigBDF <ASDM-path> [numberOfBlocks]" << endl;
    exit(1);
  }

  int numberOfBlocks = 10;
  if (argC == 3) numberOfBlocks = atoi(argV[2]);
  if (numberOfBlocks == 0) {
    cout << "blockNumber can't be null" << endl;
    exit(1);
  }

  cout << "I'll use numberOfBlocks = " << numberOfBlocks << endl;
  cout << "And the size of one block of BDF resident in memory should be around " << numberOfBlocks*BLOCKSIZE << endl;


  ASDM ds;
  try {
    cout << "Selected ASDM " << argV[1] << endl;
    ds.setFromFile(string(argV[1]), ASDMParseOptions()
		   .loadTablesOnDemand(true)
		   .checkRowUniqueness(CHECKROWUNIQUENESS));
  }
  catch (ConversionException e) {
    cout << e.getMessage() << endl;
    exit(1);
  }
  
  const vector<MainRow *>& mRs = ds.getMain().get();
  if (mRs.size() == 0) {
    cout << "Please give me an ASDM with visibilities." << endl;
  }
 
  // Let's look for an interesting candidate : a number of integrations greater than and a not too small size.
  unsigned int index = 0;
  while ( (index < mRs.size()) &&
	  ((mRs[index]->getNumIntegration() < 2) ||
	   (mRs[index]->getDataSize() < 300000000)) ) index++;
  
  if ( index == mRs.size() ) {
    cout << "I did not find any interesting row in that ASDM" << endl;
    exit(1);
  }

  cout << "Ok , let's work with Main row #" << index << ", it has " << mRs[index]->getNumIntegration() << " integrations and the size of the BDF is " << mRs[index]->getDataSize() << endl;
  vector<unsigned int> actualSizeInMemory(sizeInMemory(mRs[index]->getDataSize(), numberOfBlocks*BLOCKSIZE));
  int N = mRs[index]->getNumIntegration();

  
  SDMBinData sdmBinData(&ds, string(argV[1]));
  sdmBinData.openMainRow(mRs[index]);

  // A pointer to a VMSData structure as returned by getMSMainCols.
  const VMSData *vmsDataPtr = 0;

  int64_t numberOfMSMainRows = 0;
  int64_t numberOfIntegrations = 0;
  int64_t numberOfReadIntegrations = 0;
  cout << "We are going to process this row in " << actualSizeInMemory.size() << " steps at maximum" << endl;
  for (unsigned int i = 0; i < actualSizeInMemory.size(); i++) {
    cout << "We want to have around " << actualSizeInMemory[i] << " bytes at maximum in memory" << endl;
    numberOfIntegrations = actualSizeInMemory[i] / (mRs[index]->getDataSize() / N);
    cout << "That is approximately " << numberOfIntegrations << " integrations." << endl;
    vmsDataPtr = sdmBinData.getNextMSMainCols(numberOfIntegrations);
    numberOfReadIntegrations += numberOfIntegrations;
    numberOfMSMainRows += vmsDataPtr->v_antennaId1.size();
    cout << "Number of MS Main rows = " << numberOfMSMainRows << endl;
  }

  unsigned int numberOfRemainingIntegrations = N - numberOfReadIntegrations;
  cout << numberOfRemainingIntegrations << " integrations remaining." << endl;
  if (numberOfRemainingIntegrations) {
    vmsDataPtr = sdmBinData.getNextMSMainCols(numberOfRemainingIntegrations);
    numberOfMSMainRows += vmsDataPtr->v_antennaId1.size();
    cout << "Number of MS Main rows = " << numberOfMSMainRows << endl;
  }  
}

