//# tSIIterBot.cc:  Test for SIIterbotClass
//# Copyright (C) 1996,1997,1998,1999,2000,2001
//# Associated Universities, Inc. Washington DC, USA.
//#
//# This library is free software; you can redistribute it and/or modify it
//# under the terms of the GNU Library General Public License as published by
//# the Free Software Foundation; either version 2 of the License, or (at your
//# option) any later version.
//#
//# This library is distributed in the hope that it will be useful, but WITHOUT
//# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
//# FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
//# License for more details.
//#
//# You should have received a copy of the GNU Library General Public License
//# along with this library; if not, write to the Free Software Foundation,
//# Inc., 675 Massachusetts Ave, Cambridge, MA 02139, USA.
//#
//# Correspondence concerning AIPS++ should be addressed as follows:
//#        Internet email: aips2-request@nrao.edu.
//#        Postal address: AIPS++ Project Office
//#                        National Radio Astronomy Observatory
//#                        520 Edgemont Road
//#                        Charlottesville, VA 22903-2475 USA
//#
//# $Id$


//#include <casa/namespace.h>
#include <casa/iostream.h>
#include <deque>

#include <casa/Containers/Record.h>

// Include file for the SIIterBot Class
#include<synthesis/MeasurementEquations/SIIterBot.h>

class SIIterBotTest;
typedef void (SIIterBotTest::* TestFunc)();

class SIIterBotTest {
public:
  SIIterBotTest();
  ~SIIterBotTest();

  int runTests();

  void testDefaultValues();
  void testSettingControlValues();
  void testInitializingFromRecord();
  void testSettingCycleParameters();
  void testIterationBehavior();
  void testThresholdBehavior();
  void testExportingToRecord();

protected:

  void addTestFunction(TestFunc testFunc);

  bool compareFloat(float, float, float thresh=1E-12);

  void failIf(bool condition, string Message);

private:
  std::deque<TestFunc> testFunctions;
  bool currentTestFailed;
  string currentTestExecution;

};

SIIterBotTest::SIIterBotTest(){
  addTestFunction(&SIIterBotTest::testDefaultValues);
  addTestFunction(&SIIterBotTest::testSettingControlValues);
  addTestFunction(&SIIterBotTest::testInitializingFromRecord);
  addTestFunction(&SIIterBotTest::testExportingToRecord);
  addTestFunction(&SIIterBotTest::testIterationBehavior);
  addTestFunction(&SIIterBotTest::testThresholdBehavior);
  addTestFunction(&SIIterBotTest::testSettingCycleParameters);
}

SIIterBotTest::~SIIterBotTest(){}

/* --------- Begin Test Fixture Code -----------------------*/
void SIIterBotTest::addTestFunction(TestFunc testFunc){
  testFunctions.push_back(testFunc);
}

void SIIterBotTest::failIf(bool condition, string message){
  if (condition) {
    // Test Failed
    std::cout << "Failure (" << currentTestExecution <<"): " 
              << message << std::endl;
    currentTestFailed = true;
  }
}

bool SIIterBotTest::compareFloat(float value1, float value2, float thresh){
  return fabs(value1 - value2) < thresh;
}

int SIIterBotTest::runTests() {
  unsigned int successfulTests = 0;

  std::deque<TestFunc>::iterator fptr = testFunctions.begin();
  for (; fptr != testFunctions.end(); fptr++) {
    currentTestFailed = false;
    (this->**fptr)();
    if (!currentTestFailed) successfulTests++;
  }
  
  if (successfulTests != testFunctions.size()) {
    std::cout << successfulTests << " out of " << testFunctions.size() 
              << " test(s) passed" << std::endl;
    std::cout << "Test FAILED." << std::endl;
    return -1;
  } 
  
  std::cout << "All " << successfulTests << " test(s) passed" << std::endl;
  std::cout << "Test PASSED." << std::endl;
  return 0;
}

/* --------------- Begin Actual Tests ---------------*/

void SIIterBotTest::testDefaultValues(){
  currentTestExecution = __FUNCTION__;
  /* This test checks to make sure that the getter functions all work
     and that the default values are as expected */
  casa::SIIterBot siBot;

  failIf(!compareFloat(siBot.getLoopGain(), 0.1),
         "Initial LoopGain incorrect");
  failIf(siBot.getRemainingNiter() != 0,
         "Initial RemainingNiter value incorrect");
  failIf(siBot.getCompletedNiter() != 0, 
         "Initial CompletedNiter value incorrect");
  failIf(siBot.getThreshold() != 0.0,
         "Initial Threshold value incorrect");
  failIf(siBot.getCycleThreshold() != 0.0,
         "Initial CycleThreshold value incorrect");
  failIf(siBot.getInteractiveThreshold() != 0.0,
         "Initial InteractiveThreshold value incorrect");
  failIf(siBot.getInteractiveMode(),
         "Initial Interactive Mode Flag incorrect");
  failIf(siBot.getMajorCycleCount() != 0,
         "Initial MajorCycleCount value incorrect.");
  failIf(!compareFloat(siBot.getLoopGain(),0.1),
         "Initial loop gain value incorrect");
  failIf(!compareFloat(siBot.getCycleFactor(),1.0),
         "Initial cycle factor value incorrect");
  failIf(!compareFloat(siBot.getMaxPsfSidelobe(),0.1),
         "Initial max psf sidelobe value incorrect");
  failIf(!compareFloat(siBot.getMaxPsfFraction(),0.8),
         "Initial max psf fraction value incorrect");
  failIf(!compareFloat(siBot.getMinPsfFraction(),0.05),
         "Initial min psf fraction value incorrect");
  failIf(siBot.getUpdatedModelFlag(),
         "Initial UpdatedModelFlag value incorrect");
}


/* Tests to Do */
void SIIterBotTest::testSettingControlValues(){
  currentTestExecution = __FUNCTION__;
  /* Test that the various setter methods we have are working properly */

  casa::SIIterBot siBot;

  for (int counter=0; counter < 5; ++counter) {
    failIf(siBot.getMajorCycleCount() != counter,
           "major cycle counter did not increment correctly");
    siBot.incrementMajorCycleCount();
  }

  siBot.changeNiter(20);
  failIf(siBot.getNiter() != 20,
         "Unable to properly set Niter");
  siBot.changeCycleNiter(10);
  failIf(siBot.getCycleNiter() != 10,
         "Unable to properly set CycleNiter");
  siBot.changeInteractiveNiter(15);
  failIf(siBot.getInteractiveNiter() != 15,
         "Unable to properly set InteractiveNiter");
  
  siBot.changeThreshold(.687);
  failIf(!compareFloat(siBot.getThreshold(), .687),
         "threshold not properly set");
  siBot.changeCycleThreshold(.987);
  failIf(!compareFloat(siBot.getCycleThreshold(), .987),
         "cycle threshold not properly set");
  siBot.changeInteractiveThreshold(.787);
  failIf(!compareFloat(siBot.getInteractiveThreshold(), .787),
         "interactive threshold not properly set");

  siBot.changeInteractiveMode(true);
  failIf(!siBot.getInteractiveMode(), 
         "Failed to set interactive mode");
  
  siBot.changeLoopGain(.654);
  failIf(!compareFloat(siBot.getLoopGain(), .654),
         "loop gain not properly set");

  siBot.changeCycleFactor(4.56);
  failIf(!compareFloat( siBot.getCycleFactor(),4.56),
         "cycle factor not properly set");

  siBot.changePauseFlag(true);
  failIf(!siBot.getPauseFlag(), 
         "Failed to set pause flag");

  siBot.changeStopFlag(true);
  failIf(!siBot.getStopFlag(), 
         "Failed to set stop flag");
}

void SIIterBotTest::testInitializingFromRecord(){
  currentTestExecution = __FUNCTION__;
  /* Check that we can set all variables from a record 
     do this in several parts to make sure we handle missing fields ok*/
   casa::Record iterRec;
   iterRec.define( casa::RecordFieldId("niter"), 123);
   iterRec.define( casa::RecordFieldId("cycleniter"), 456);
   iterRec.define( casa::RecordFieldId("interactiveniter"), 789);
                                        
   casa::Record thresholdRec;
   thresholdRec.define( casa::RecordFieldId("threshold"), 5.67);
   thresholdRec.define( casa::RecordFieldId("cyclethreshold"), 7.89);
   thresholdRec.define( casa::RecordFieldId("interactivethreshold"), 8.91);

   casa::Record controlRec;
   controlRec.define( casa::RecordFieldId("cyclefactor"), 4.56);
   controlRec.define( casa::RecordFieldId("loopgain"), 6.78);

   casa::SIIterBot siBot;
   /* Start with the basic test */
   failIf(siBot.getNiter() != 0,
          "Niter Threshold value incorrect");
   failIf(siBot.getCycleNiter() != 0,
          "CycleNiter Threshold value incorrect");
   failIf(siBot.getInteractiveNiter() != 0,
          "InteractiveNiter Threshold value incorrect");
   failIf(!compareFloat(siBot.getThreshold(), 0.0),
          "Threshold value incorrect");
   failIf(!compareFloat(siBot.getCycleThreshold(),0.0),
          "CycleThreshold value incorrect");
   failIf(!compareFloat(siBot.getInteractiveThreshold(),0.0),
           "InteractiveThreshold value incorrect");
   failIf(!compareFloat(siBot.getLoopGain(),0.1),
          "Loop gain value incorrect");
   failIf(!compareFloat(siBot.getCycleFactor(),1.0),
          "Cycle factor value incorrect");
   
   siBot.setControlsFromRecord(iterRec);
   failIf(siBot.getNiter() != 123,
          "Niter Threshold value incorrect");
   failIf(siBot.getCycleNiter() != 456,
          "CycleNiter Threshold value incorrect");
   failIf(siBot.getInteractiveNiter() != 789,
          "InteractiveNiter Threshold value incorrect");
   failIf(!compareFloat(siBot.getThreshold(), 0.0),
          "Threshold value incorrect");
   failIf(!compareFloat(siBot.getCycleThreshold(),0.0),
          "CycleThreshold value incorrect");
   failIf(!compareFloat(siBot.getInteractiveThreshold(),0.0),
           "InteractiveThreshold value incorrect");
   failIf(!compareFloat(siBot.getLoopGain(),0.1),
          "Loop gain value incorrect");
   failIf(!compareFloat(siBot.getCycleFactor(),1.0),
          "Cycle factor value incorrect");

   siBot.setControlsFromRecord(thresholdRec);
   failIf(siBot.getNiter() != 123,
          "Niter Threshold value incorrect");
   failIf(siBot.getCycleNiter() != 456,
          "CycleNiter Threshold value incorrect");
   failIf(siBot.getInteractiveNiter() != 789,
          "InteractiveNiter Threshold value incorrect");
   failIf(!compareFloat(siBot.getThreshold(), 5.67),
          "Threshold value incorrect");
   failIf(!compareFloat(siBot.getCycleThreshold(),7.89),
          "CycleThreshold value incorrect");
   failIf(!compareFloat(siBot.getInteractiveThreshold(),8.91),
           "InteractiveThreshold value incorrect");
   failIf(!compareFloat(siBot.getLoopGain(), 0.1),
          "Loop gain value incorrect");
   failIf(!compareFloat(siBot.getCycleFactor(), 1.0),
          "Cycle factor value incorrect");

   siBot.setControlsFromRecord(controlRec);
   failIf(siBot.getNiter() != 123,
          "Niter Threshold value incorrect");
   failIf(siBot.getCycleNiter() != 456,
          "CycleNiter Threshold value incorrect");
   failIf(siBot.getInteractiveNiter() != 789,
          "InteractiveNiter Threshold value incorrect");
   failIf(!compareFloat(siBot.getThreshold(), 5.67),
          "Threshold value incorrect");
   failIf(!compareFloat(siBot.getCycleThreshold(),7.89),
          "CycleThreshold value incorrect");
   failIf(!compareFloat(siBot.getInteractiveThreshold(),8.91),
           "InteractiveThreshold value incorrect");
   failIf(!compareFloat(siBot.getLoopGain(),6.78),
          "Loop gain value incorrect");
   failIf(!compareFloat(siBot.getCycleFactor(),4.56),
          "Cycle factor value incorrect");
}

void SIIterBotTest::testSettingCycleParameters() {
  currentTestExecution = __FUNCTION__;
  /* This tests the ability to modify the cycle dependent parameters and
     get the cycle threshold correctly */
    casa::SIIterBot siBot; 
    
    siBot.setMaxPsfSidelobe(.9);
    failIf(!compareFloat(siBot.getMaxPsfSidelobe(),0.9),
           "Unable to set PSF Sidelobe");

    siBot.setMaxPsfFraction(1.0);
    failIf(!compareFloat(siBot.getMaxPsfFraction(),1.0),
           "Unable to set Maximum PSF Fraction");

    siBot.setMinPsfFraction(0.0);
    failIf(!compareFloat(siBot.getMinPsfFraction(),0.0),
           "Unable to set Minimum PSF Fraction");

    siBot.changeCycleFactor(1.0);
    siBot.changeLoopGain(0.1);

    siBot.updateCycleThreshold(100);
    failIf(!compareFloat(siBot.getCycleThreshold(),90.0),
           "Incorrectly determined cyclefactor");

    /* Test the Max PSF Fraction Works */
    siBot.setMaxPsfFraction(0.7);
    siBot.updateCycleThreshold(100);
    failIf(!compareFloat(siBot.getCycleThreshold(), 70.0),
           "Max Psf Fraction did not correctly affect threshold");
    
    /* Test that Changeing MaxPSF Sidelobe works */
    siBot.setMaxPsfSidelobe(0.1);
    siBot.updateCycleThreshold(100);
    failIf(!compareFloat(siBot.getCycleThreshold(),10.0),
           "Max Psf Sidelobe did not correctly affect threshold");

    /* Test the Min PSF Fraction Works */
    siBot.setMinPsfFraction(0.25);
    siBot.updateCycleThreshold(100);
    failIf(!compareFloat(siBot.getCycleThreshold(), 25),
           "Min Psf Fraction did not correctly affect threshold");

    /* Test changing the cyclefactor works */
    siBot.changeCycleFactor(5.0);
    siBot.updateCycleThreshold(100);
    failIf(!compareFloat(siBot.getCycleThreshold(), 50.0),
           "Cycle Factor did not correctly affect threshold");

    /* Test changing the peak residual works */
    siBot.updateCycleThreshold(10);
    failIf(!compareFloat(siBot.getCycleThreshold(), 5.0),
           "Peak Residual did not correctly affect threshold");
}

void SIIterBotTest::testIterationBehavior(){
  currentTestExecution = __FUNCTION__;
  casa::SIIterBot siBot; 
  const int     MaxIter    = 15000; // Maximum
  const int     nIter      = 12345;
  const int     cycleNiter = 345;

  siBot.changeThreshold(0.0);
  siBot.changeCycleThreshold(0.0);
  siBot.changeNiter(nIter);
  siBot.changeCycleNiter(cycleNiter);

  int localCount = 0;
  while (!siBot.cleanComplete(MAXFLOAT)) {
    ++localCount;
    if (localCount >= MaxIter) {
      failIf(true," Exceeded Maximum Interation Threshold");
      break;
    }
    siBot.incrementMinorCycleCount();
    if (siBot.majorCycleRequired(MAXFLOAT)) {
      /* Make sure we really need one */
      failIf(localCount % cycleNiter && localCount!=nIter, 
             "Major cycle at incorrect time");
      siBot.incrementMajorCycleCount();
    } else {
      failIf(localCount % cycleNiter == 0 && localCount != 0,
             "Missed a Major cycle");
    }

    failIf(siBot.getCompletedNiter() != localCount,
           "Incorrect number of completed iterations");
    failIf(siBot.getRemainingNiter() != nIter-localCount,
           "Incorrect number of completed iterations");
  }
  failIf(localCount != nIter, "Incorrect number of cycles performed");
  /* Plus one below to account for the last one when we are done! */
  failIf((nIter/cycleNiter) + 1 != siBot.getMajorCycleCount(),
         "Incorrect number of Major Cycles");
}

void SIIterBotTest::testThresholdBehavior(){
  currentTestExecution = __FUNCTION__;
  /* This test checks the threshold dependent behavior assuming that
     the user is controlling it (e.g. never updating the cycle threshold)
  */
  casa::SIIterBot siBot; 
  const float cleanStep = 10.0;
  const float threshold = 35.0;
  float cycleThreshold  = 900;

  siBot.changeNiter(200000);
  siBot.changeCycleNiter(200000);
  siBot.changeThreshold(threshold);
  siBot.changeCycleThreshold(cycleThreshold);
  
  
  float localPeak = 1000;
  int   localCount= 0;
  
  while (!siBot.cleanComplete(localPeak)) {
    if (localPeak < 0) {
      failIf(true, " Cleaned beyond all reason!");
      break;
    }

    if (siBot.majorCycleRequired(localPeak)) {
      siBot.incrementMajorCycleCount();
      failIf((cycleThreshold < localPeak) || 
             (cycleThreshold - localPeak > cleanStep),
             "Major cycle at inappropriate time");
      cycleThreshold -= 100;
      siBot.changeCycleThreshold(cycleThreshold);
    } else {
      failIf(localPeak < cycleThreshold,
             "Missed a major cycle");
    }

    /* This is the equivelent of the minor cycle */
    siBot.incrementMinorCycleCount();
    ++localCount;
    localPeak -= cleanStep;

  }
  failIf((fabs(threshold - localPeak)>cleanStep) || (localPeak > threshold),
         "Failed to stop at appropriate level");
  failIf(siBot.getMajorCycleCount() != (int)((1000 - threshold) / 100),
         "Incorrect number of major cycles transpired");
  failIf(siBot.getCompletedNiter() != 
         (int)(((1000 - threshold) / cleanStep)+0.5),
         "Incorrect number of minor cycles transpired");    
}

void SIIterBotTest::testExportingToRecord(){
  currentTestExecution = __FUNCTION__;
  casa::SIIterBot siBot; 

  siBot.changeNiter(123);
  siBot.changeCycleNiter(456);
  siBot.changeInteractiveNiter(789);
  siBot.changeThreshold(1.23);
  siBot.changeCycleThreshold(4.56);
  siBot.changeInteractiveThreshold(7.89);
  siBot.changeLoopGain(0.2);
  siBot.changeCycleFactor(2.0);
  siBot.setMaxPsfSidelobe(0.75);
  siBot.setMaxPsfFraction(0.8);
  siBot.setMinPsfFraction(0.2);

  for (int idx = 0; idx < 22; idx++) siBot.incrementMinorCycleCount();
  for (int idx = 0; idx < 12; idx++) siBot.incrementMajorCycleCount();
  for (int idx = 0; idx < 22; idx++) siBot.incrementMinorCycleCount();

  casa::Record recOut = siBot.getDetailsRecord();

  failIf(!recOut.isDefined("niter"),
         "niter is not defined in the record");
  failIf(recOut.asInt(casa::RecordFieldId("niter")) != 123,
         "niter value incorrect");
  failIf(!recOut.isDefined("cycleniter"),
         "cycleniter is not defined in the record");
  failIf(recOut.asInt(casa::RecordFieldId("cycleniter")) != 456,
         "cycleniter value incorrect");
  failIf(!recOut.isDefined("interactiveniter"),
         "interactiveniter is not defined in the record");
  failIf(recOut.asInt(casa::RecordFieldId("interactiveniter")) != 789,
         "interactiveniter value incorrect");

  failIf(!recOut.isDefined("threshold"),
         "threshold is not defined in the record");
  failIf(!compareFloat(recOut.asFloat(casa::RecordFieldId("threshold")),1.23),
         "incorrect threshold");
  failIf(!recOut.isDefined("cyclethreshold"),
         "cyclethreshold is not defined in the record");
  failIf(!compareFloat(recOut.asFloat(casa::RecordFieldId("cyclethreshold")),
                       4.56), "incorrect threshold");
  failIf(!recOut.isDefined("interactivethreshold"),
         "interactivethreshold is not defined in the record");
  failIf(!compareFloat(recOut.asFloat
                       (casa::RecordFieldId("interactivethreshold")),7.89),
         "incorrect threshold");


  failIf(!recOut.isDefined("loopgain"),
         "loopgain is not defined in the record");
  failIf(!compareFloat(recOut.asFloat(casa::RecordFieldId("loopgain")), 0.2),
         "incorrect loopgain");
  failIf(!recOut.isDefined("cyclefactor"),
         "cyclefactor is not defined in the record");
  failIf(!compareFloat(recOut.asFloat(casa::RecordFieldId("cyclefactor")),
                       2.0), "incorrect cyclefactor");

  failIf(!recOut.isDefined("maxpsfsidelobe"),
         "maxpsfsidelobe is not defined in the record");
  failIf(!compareFloat(recOut.asFloat(casa::RecordFieldId("maxpsfsidelobe")),
                       0.75), "incorrect max psf sidelobe");
  failIf(!recOut.isDefined("maxpsffraction"),
         "maxpsffraction is not defined in the record");
  failIf(!compareFloat(recOut.asFloat(casa::RecordFieldId("maxpsffraction")),
                       0.8), "incorrect max psf fraction ");
  failIf(!recOut.isDefined("minpsffraction"),
         "minpsffraction is not defined in the record");
  failIf(!compareFloat(recOut.asFloat(casa::RecordFieldId("minpsffraction")),
                       0.2),"incorrect min psf fraction ");

  failIf(!recOut.isDefined("nmajordone"),
         "nmajordone is not defined in the record");
  failIf(recOut.asInt(casa::RecordFieldId("nmajordone")) != 12,
         "incorrect number of major cycles reported");
  failIf(!recOut.isDefined("iterdone"),
         "iterdone is not defined in the record");
  failIf(recOut.asInt(casa::RecordFieldId("iterdone")) != 44,
         "incorrect number of minor cycles reported");
  failIf(!recOut.isDefined("cycleiterdone"),
         "cycleiterdone is not defined in the record");
  failIf(recOut.asInt(casa::RecordFieldId("cycleiterdone")) != 22,
         "incorrect number of minor cycles reported for this cycle");
  failIf(!recOut.isDefined("interactiveiterdone"),
         "interactiveiterdone is not defined in the record");
  failIf(recOut.asInt(casa::RecordFieldId("interactiveiterdone")) != 44,
         "incorrect number of minor cycles reported for interactive cycle");
}

int main(int, char**){
  SIIterBotTest siBotTest;
  exit(siBotTest.runTests());
}
