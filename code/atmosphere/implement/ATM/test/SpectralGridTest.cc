#include <iostream>
#include <vector>
#include <string>
using namespace std;

#include <limits>
#include <math.h>


#include "ATMSpectralGrid.h"



#include <iostream>
using namespace atm;

int main()
{
  unsigned int     numChan         = 64;
  unsigned int     refChan         = 32;

  Frequency myRefFreq(90.0,"GHz");
  Frequency myChanSep(0.01,"GHz");

  SpectralGrid* sgPtr1;

  cout << " SpectralGridTest: Test 1:" <<endl;
  cout << " SpectralGridTest: Build using constructor SpectralGrid(usigned int numChan, unsigned int refChan, Frequency refFreq, Frequency chanSep):" << endl;
  sgPtr1 = new SpectralGrid(numChan, refChan, myRefFreq, myChanSep);
  cout << " SpectralGridTest: Number of channels retrieved:  " << sgPtr1->getNumChan() << " Input:" << numChan << endl;
  cout << " SpectralGridTest: Reference frequency retrieved: "
       << sgPtr1->getRefFreq().get()
       << " SpectralGridTest:  Input:" << myRefFreq.get("GHz") << "GHz" << endl;
  cout << " SpectralGridTest: Reference frequency retrieved: "
       << sgPtr1->getRefFreq().get("GHz")<< "GHz "  
       <<" SpectralGridTest: Input:" << myRefFreq.get("GHz") << "GHz" << endl;
  cout << " SpectralGridTest: Reference channel retrieved:   " << sgPtr1->getRefChan() << " Input:" << refChan << endl;
  cout << " SpectralGridTest: Channel separation retrieved:  "
       << sgPtr1->getChanSep().get() << "Hz "
       << " SpectralGridTest: Input:" << myChanSep.get("GHz") << "GHz" << endl;
  cout << " SpectralGridTest: Channel separation retrieved:  "
       << sgPtr1->getChanSep().get("kHz") << "kHz "
       << " SpectralGridTest: Input:" << myChanSep.get("GHz") << "GHz" << endl;
  cout << " SpectralGridTest: Number of spectral windows: "
       << sgPtr1->getNumSpectralWindow()
       << " SpectralGridTest:  Expected: 0" << endl;
  cout << " SpectralGridTest: Number of channels for spectral window identifier 1: "
       << sgPtr1->getNumChan(1)
       << " SpectralGridTest:  Expected: trigger an error message " << endl;
  if(sgPtr1->isRegular()){
    cout << " SpectralGridTest: the first spectral window is regularily sampled" << endl;
  }else{
    cout << " SpectralGridTest: the first spectral window is not regularily sampled" << endl;
  }
  if(sgPtr1->getAssocNature(0).size()==0){
    cout<<" SpectralGridTest:  There is a warning as expected" << endl;
  }else{
    cout<<" SpectralGridTest:  ERROR detected with this test: \n this spectral window has NO associated spectral window "<<endl;
  } 
  if(sgPtr1->getAssocSpwId(0).size()==0){
    cout<<" SpectralGridTest:  There is a warning as expected" << endl;
  }else{
    cout<<" SpectralGridTest:  ERROR detected with this test: \n this spectral window has NO associated spectral window "<<endl;
  }

  cout << " SpectralGridTest: New spectral window using add(unsigned int numChan, unsigned int refChan, Frequency refFreq, Frequency chanSep):" << endl;
  unsigned int numChan1         = 128;
  unsigned int refChan1         = 32;
  Frequency myNewRefFreq(215.0,"GHz");
  Frequency myNewChanSep(0.02,"GHz");
  unsigned int spwId = sgPtr1->add( numChan1, refChan1, myNewRefFreq, myNewChanSep);
  cout << " SpectralGridTest: A new spectral window has been appended and got the identifier " << spwId << endl;
  cout << endl;
  cout << " SpectralGridTest: Number of spectral windows: "
       << sgPtr1->getNumSpectralWindow()
       << " SpectralGridTest:  Expected: 2" << endl;
  cout << endl;

  cout << " SpectralGridTest: Channel frequency and number for the first spectral window: " << endl;
  double chFreq[sgPtr1->getNumChan()];           // one dynamic alloc
  vector<double> chanFreq;
  chanFreq.reserve(sgPtr1->getNumChan());        // a more versatil dynamic alloc (allowing eg resizing)
  for(unsigned int i=0; i<sgPtr1->getNumChan(); i++){
    chanFreq[i] = sgPtr1->getChanFreq(i).get();
    chFreq[i] = chanFreq[i];
    cout << "SpectralGridTest: " << i << "channel: " << i-refChan+1 << " freq: " << chanFreq[i] << endl;
  }
  cout << endl;

  spwId=0;
  cout << " SpectralGridTest: Number of channels retrieved for spwId "<<spwId<<": "
       << sgPtr1->getNumChan(spwId) << " Input:" << numChan << endl;
  cout << " SpectralGridTest: Reference frequency retrieved: "
       << sgPtr1->getRefFreq(spwId).get()
       << " SpectralGridTest:  Input:" << myRefFreq.get("GHz") << "GHz" << endl;
  cout << " SpectralGridTest: Reference frequency retrieved: "
       << sgPtr1->getRefFreq(spwId).get("GHz")<< "GHz "  
       <<" SpectralGridTest: Input:" << myRefFreq.get("GHz") << "GHz" << endl;
  cout << " SpectralGridTest: Reference channel retrieved:   " << sgPtr1->getRefChan() << " Input:" << refChan << endl;
  cout << " SpectralGridTest: Channel separation retrieved:  "
       << sgPtr1->getChanSep(spwId).get() << "Hz "
       << " SpectralGridTest: Input:" << myChanSep.get("GHz") << "GHz" << endl;
  cout << " SpectralGridTest: Channel separation retrieved:  "
       << sgPtr1->getChanSep(spwId).get("kHz") << "kHz "
       << " SpectralGridTest: Input:" << myChanSep.get("GHz") << "GHz" << endl;
  cout << endl;
  spwId=1;
  cout << " SpectralGridTest: Number of channels retrieved for spwId "<<spwId<<": "
       << sgPtr1->getNumChan(spwId) << " Input:" << numChan1 << endl;
  cout << " SpectralGridTest: Reference frequency retrieved: "
       << sgPtr1->getRefFreq(spwId).get()
       << " SpectralGridTest:  Input:" << myNewRefFreq.get("GHz") << "GHz" << endl;
  cout << " SpectralGridTest: Reference frequency retrieved: "
       << sgPtr1->getRefFreq(spwId).get("GHz")<< "GHz "  
       <<" SpectralGridTest: Input:" << myNewRefFreq.get("GHz") << "GHz" << endl;
  cout << " SpectralGridTest: Reference channel retrieved:   " << sgPtr1->getRefChan() << " Input:" << refChan1 << endl;
  cout << " SpectralGridTest: Channel separation retrieved:  "
       << sgPtr1->getChanSep(spwId).get() << "Hz "
       << " SpectralGridTest: Input:" << myNewChanSep.get("GHz") << "GHz" << endl;
  cout << " SpectralGridTest: Channel separation retrieved:  "
       << sgPtr1->getChanSep(spwId).get("kHz") << "kHz "
       << " SpectralGridTest: Input:" << myNewChanSep.get("GHz") << "GHz" << endl;
  if(sgPtr1->isRegular(spwId)){
    cout << " SpectralGridTest: the spectral window with id "<<spwId<<" is regularily sampled" << endl;
  }else{
    cout << " SpectralGridTest: the spectral window with id "<<spwId<<" is not regularily sampled" << endl;
  }

  cout << " SpectralGridTest: Number of spectral windows: "
       << sgPtr1->getNumSpectralWindow()
       << " SpectralGridTest:  Expected: 2" << endl;

  if(sgPtr1->getSideband(spwId).size()==0)
    cout << " SpectralGridTest: As expected this spectral window with spwid="<<spwId
	 << " SpectralGridTest:  has no sideband specification" << endl;
  unsigned int id=10;
  if(sgPtr1->getSideband(id).size()==0)
    cout << " SpectralGridTest: As expected error because spectral window with id="<<id
	 << " SpectralGridTest:  does not exist!" << endl;

  cout << endl;


  delete sgPtr1; sgPtr1=0;



  double  refFreq         = 90.0E9;

  SpectralGrid* sgPtr2;
  cout << " SpectralGridTest: Test 2:" <<endl;
  cout << " SpectralGridTest: Build using SpectralGrid( unsigned int numChan, unsigned int refChan, double* chFreq, string units):" << endl;
  sgPtr2 = new SpectralGrid::SpectralGrid( numChan, refChan, chFreq, "Hz");
  cout << " SpectralGridTest: Number of channels retrieved: " << sgPtr2->getNumChan()      << "    Input:  " << numChan << endl;
  cout << " SpectralGridTest: Reference frequency retrieved:" << sgPtr2->getRefFreq().get()      << "Hz  Initial: none" << endl;
  cout << " SpectralGridTest: Reference frequency retrieved:" << sgPtr2->getRefFreq().get("MHz") << "MHz Initial: none" << endl;
  cout << " SpectralGridTest: Reference channel retrieved:  " << sgPtr2->getRefChan()      << "    Input:  " << refChan << endl;
  cout << " SpectralGridTest: Channel separation retrieved: " << sgPtr2->getChanSep().get()      << "Hz  Initial: none" << endl;
  cout << " SpectralGridTest: Channel separation retrieved: " << sgPtr2->getChanSep().get("MHz") << "MHz Initial: none" << endl;
  if(sgPtr2->isRegular()){
    cout << " SpectralGridTest: the first spectral window with id 0 is regularily sampled" << endl;
  }else{
    cout << " SpectralGridTest: the first spectral window with id 0 is not regularily sampled" << endl;
  }
  chFreq[sgPtr2->getNumChan()/4]=chFreq[sgPtr2->getNumChan()/4]+1.;
  cout << " SpectralGridTest: Add a second irregular spectral window using add( unsigned int numChan, unsigned int refChan, double* chFreq, string units):" << endl;
  sgPtr2->add( numChan, refChan, chFreq, "Hz");
  if(sgPtr2->isRegular()){
    cout << " SpectralGridTest: the first spectral window with id 0 is regularily sampled as expected" << endl;
  }else{
    cout << " SpectralGridTest: the first spectral window with id 0 is not regularily sampled ==> ERROR in the code" << endl;
  }
  if(sgPtr2->isRegular(spwId)){
    cout << " SpectralGridTest: the spectral window with id "<<spwId<<" is regularily sampled ==> ERROR in the code" << endl;
  }else{
    cout << " SpectralGridTest: the spectral window with id "<<spwId<<" is not regularily sampled as expected" << endl;
  }
  delete sgPtr2; sgPtr2=0;

  cout << endl;
  cout << endl;

  SpectralGrid* sgPtr3;

  cout << " SpectralGridTest: Test 3:" << endl;
  cout << " SpectralGridTest: Build using SpectralGrid( unsigned int numChan, double refFreq, double* chFreq, string freqUnits):" << endl;
  sgPtr3 = new SpectralGrid::SpectralGrid( numChan, refFreq, chFreq, "Hz");
  cout << " SpectralGridTest: Number of channels retrieved: " << sgPtr3->getNumChan() << " Input: " << numChan << endl;
  cout << " SpectralGridTest: Reference frequency retrieved:" << sgPtr3->getRefFreq().get()      << "Hz  Initial:" << refFreq << "Hz" << endl;
  cout << " SpectralGridTest: Reference frequency retrieved:" << sgPtr3->getRefFreq().get("MHz") << "MHz Initial:" << refFreq << "Hz" << endl;
  cout << " SpectralGridTest: Reference channel retrieved:  " << sgPtr3->getRefChan() << " Initial: " << refChan << endl;
  cout << " SpectralGridTest: Channel separation retrieved: " << sgPtr3->getChanSep().get() << " Initial: none"<< endl;

  { double chan=16.123456;
    cout << " SpectralGridTest: Position (GU) retrieved: "
         << sgPtr3->getChanNum(refFreq+(sgPtr3->getChanSep().get())*chan)
         << " SpectralGridTest:  Exact: " << chan << endl;
  }
  cout << " SpectralGridTest: Total bandwidth retrieved: " << sgPtr3->getBandwidth().get()
       << " SpectralGridTest: Initial: "<< (sgPtr3->getChanSep().get())*(numChan-1) << endl;

  cout << " SpectralGridTest: Frequency range: from "<< sgPtr3->getMinFreq().get() <<" to "<< sgPtr3->getMaxFreq().get() <<"Hz"<< endl;
  cout << " SpectralGridTest: Frequency range: from "<< sgPtr3->getMinFreq().get("GHz") <<" to "<< sgPtr3->getMaxFreq().get("GHz") <<"GHz"<< endl;

  delete sgPtr3;

  cout << endl;
  cout << endl;

  numChan         = 128;
  refChan         = 64;
  Frequency refFreq2(215.0,"GHz");
  Frequency chanSep2(0.02,"GHz");
  Frequency intermediateFreq(2.0,"GHz");
  Frequency bandWidth(1.0,"GHz");
  //  SidebandSide sbSide=LSB;        
  //  SidebandType sbType=SSB;
  
  sgPtr1 = new SpectralGrid(numChan, refChan, refFreq2, chanSep2,
                            intermediateFreq, LSB, SSB);
    
  cout   << " SpectralGridTest: Number of spectral windows:            " << sgPtr1->getNumSpectralWindow() << " Expected: 2" << endl;

  for(unsigned int spwId=0; spwId<sgPtr1->getNumSpectralWindow(); spwId++){
    cout << " SpectralGridTest: Sideband:                              " << sgPtr1->getSideband(spwId) << endl;
    cout << " SpectralGridTest: LO frequency:                          " << sgPtr1->getLoFrequency(spwId) << "Hz " <<  endl;
    cout << " SpectralGridTest: Number of channels retrieved:          " << sgPtr1->getNumChan(spwId) << " for spwId " <<  spwId<<": "
         << " Input:" << numChan << endl;
    cout << " SpectralGridTest: Reference frequency retrieved:         " << sgPtr1->getRefFreq(spwId).get()
         << "  Input:" << refFreq2.get("GHz") << "GHz" << endl;
    cout << " SpectralGridTest: Reference frequency retrieved:         " << sgPtr1->getRefFreq(spwId).get("GHz")<< "GHz "  
         << " Input:" << refFreq2.get("GHz") << "GHz" << endl;
    cout << " SpectralGridTest: Reference channel retrieved:           " << sgPtr1->getRefChan()
         << " Input:" << refChan << endl;
    cout << " SpectralGridTest: Channel separation retrieved:          " << sgPtr1->getChanSep(spwId).get() << "Hz "
         << " Input: |" << chanSep2.get("GHz") << "| GHz" << endl;
    cout << " SpectralGridTest: Channel separation retrieved:          " << sgPtr1->getChanSep(spwId).get("kHz") << "kHz "
         << " Input: |" << chanSep2.get("GHz") << "| GHz" << endl;
    cout << " SpectralGridTest: minFreq:                               " << sgPtr1->getMinFreq(spwId).get("GHz") << " GHz" << endl;
    cout << " SpectralGridTest: maxFreq:                               " << sgPtr1->getMaxFreq(spwId).get("GHz") << " GHz" << endl;
    cout << " SpectralGridTest: Channel (grid units) for the min:      " << sgPtr1->getChanNum(spwId,sgPtr1->getMinFreq(spwId).get()) << endl;
    cout << " SpectralGridTest: Channel (grid units) for the max:      " << sgPtr1->getChanNum(spwId,sgPtr1->getMaxFreq(spwId).get()) << endl;

    if(sgPtr1->isRegular(spwId)){
      cout << " SpectralGridTest: the spectral window with id "<<spwId<<" is regularily sampled" << endl;
    }else{
      cout << " SpectralGridTest: the spectral window with id "<<spwId<<" is not regularily sampled" << endl;
    }

    if(sgPtr1->getAssocSpwId(spwId).size()==0){
      cout << " SpectralGridTest: the spectral window with id "<< spwId <<" has no associated spectral window" << endl;
    }else{
      for(unsigned int n=0; n<sgPtr1->getAssocSpwId(spwId).size(); n++){
	unsigned int assocSpwId = sgPtr1->getAssocSpwId(spwId)[n];
	cout << " SpectralGridTest: the spectral window with id "<< spwId 
	     << " has the associated spec. win. with id " <<  assocSpwId 
	     << " (" <<  sgPtr1->getAssocNature(spwId)[n] << ")" << endl;

	for(unsigned int i=0; i<sgPtr1->getNumChan(spwId); i++){
	  cout << " SpectralGridTest: chan index:" << i << " "
	       <<  sgPtr1->getSideband(spwId) <<" "<<sgPtr1->getChanFreq(spwId,i).get("GHz")<<"GHz  "
	       <<  sgPtr1->getAssocNature(spwId)[n] <<" "<<sgPtr1->getChanFreq(assocSpwId,i).get("GHz")<<"GHz"<<endl;
	}
      }
    }
    cout << endl;
    


  }

  cout << " SpectralGridTest: TESTBED done" << endl;
  return 0;
}

