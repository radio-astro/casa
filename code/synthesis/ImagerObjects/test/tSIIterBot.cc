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
#include <casadbus/interfaces/SynthImager.proxy.h>

// Include file for the SIIterBot Class
#include<synthesis/ImagerObjects/SIIterBot.h>
#include <sys/wait.h>

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
  void testInteractiveMode();

  /* DBUS Interface Tests */
  void testInstanciation();
  void testServerSideCleanup();
  void testInteractiveCycle();
  void testGettingDetailsLocal();
  void testSettingDetails();

protected:

  void addTestFunction(TestFunc testFunc);

  bool compareFloat(float, float, float thresh=1E-12);

  void failIf(bool condition, string Message);

  int spawnController(bool, bool, bool);
  int checkChildState(int);

private:
  std::deque<TestFunc> testFunctions;
  bool currentTestFailed;
  string currentTestExecution;

};

SIIterBotTest::SIIterBotTest(){
//   addTestFunction(&SIIterBotTest::testDefaultValues);
//   addTestFunction(&SIIterBotTest::testSettingControlValues);
//   addTestFunction(&SIIterBotTest::testInitializingFromRecord);
//   addTestFunction(&SIIterBotTest::testExportingToRecord);
//   addTestFunction(&SIIterBotTest::testIterationBehavior);
//   addTestFunction(&SIIterBotTest::testThresholdBehavior);
//   addTestFunction(&SIIterBotTest::testSettingCycleParameters);
//   addTestFunction(&SIIterBotTest::testInteractiveMode);

  addTestFunction(&SIIterBotTest::testInstanciation);
//   addTestFunction(&SIIterBotTest::testServerSideCleanup);
//   addTestFunction(&SIIterBotTest::testInteractiveCycle);
//   addTestFunction(&SIIterBotTest::testGettingDetailsLocal);
//   addTestFunction(&SIIterBotTest::testSettingDetails);
}

SIIterBotTest::~SIIterBotTest(){}

/* --------- Begin Test Fixture Code -----------------------*/
void SIIterBotTest::addTestFunction(TestFunc testFunc){
  testFunctions.push_back(testFunc);
}

void SIIterBotTest::failIf(bool condition, string message){
  if (condition) {
    // Test Failed
    std::cout << "\tFailure (" << currentTestExecution <<"): " 
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
    if (!currentTestFailed) {
      std::cout << "Test "<< currentTestExecution << ": PASSED" << std::endl;
      successfulTests++;
    }
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
  casa::SIIterBot siBot("TestSIService");

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

  casa::SIIterBot siBot("TestSIService");

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

   casa::SIIterBot siBot("TestSIService");
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
    casa::SIIterBot siBot("TestSIService"); 
    
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
  casa::SIIterBot siBot("TestSIService"); 
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
  casa::SIIterBot siBot("TestSIService"); 
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
  casa::SIIterBot siBot("TestSIService"); 

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

void SIIterBotTest::testInteractiveMode() {
  currentTestExecution = __FUNCTION__;
  casa::SIIterBot siBot("TestSIService"); 

  /* Iter Based Test */
  siBot.changeInteractiveMode(true);
  siBot.changeInteractiveNiter(15);
  siBot.changeCycleNiter(10);
  siBot.changeNiter(100);

  siBot.changeInteractiveThreshold(0.0); 
  siBot.changeCycleThreshold(0.0); 
  siBot.changeThreshold(0.0);
  
  /* Assume we have 3 Fields */
  int FieldCount = 0;
  while (!siBot.cleanComplete(1.0)) {
    siBot.incrementMinorCycleCount();
    if (siBot.majorCycleRequired(1.0)) {
      FieldCount++;
      if (FieldCount % 3 == 0) {
        siBot.incrementMajorCycleCount();
        if (siBot.interactiveInputRequired(1.0)) {
          siBot.interactionComplete();
          failIf(siBot.getCompletedNiter() != 45 &&
                 siBot.getCompletedNiter() != 90,
                 "Interactive Break at Incorrect Time");
        }
      } else {
        siBot.resetCycleIter();
      }
      
      if (FieldCount < 4) 
        failIf(10 * FieldCount != siBot.getCompletedNiter(),
               "Major Cycle Break at incorrect time");
      else if (FieldCount < 7)
        failIf((5 * (FieldCount -3) + 30) != siBot.getCompletedNiter(),
               "Major Cycle Break at incorrect time");
      else if (FieldCount <  10) 
        failIf((10 * (FieldCount - 6) + 45) != siBot.getCompletedNiter(),
               "Major Cycle Break at incorrect time");
      else if (FieldCount <  13) 
        failIf((5 * (FieldCount - 9) + 75) != siBot.getCompletedNiter(),
               "Major Cycle Break at incorrect time");
    }
  }
}

void SIIterBotTest::testInstanciation() {
  currentTestExecution = __FUNCTION__;
  casa::SIIterBot     iterBot("SITestService");
  /* This could fail because we can't start the service, or because we
     never clean up.
  */
}

void SIIterBotTest::testServerSideCleanup() {
  currentTestExecution = __FUNCTION__;
  /* Check that when the service ends, the clients get notified */

  int childPID = 0;
  
  {
    casa::SIIterBot     iterBot("SITestService");

    childPID = spawnController(true, true, false);
    failIf(childPID < 0, "Failed to spawn child process");

    /* Make sure we're connected */
    while (iterBot.getNumberOfControllers() == 0) {
      sleep(1);
    }

  } // iterbot now out of scope

  failIf(checkChildState(childPID),
         "Controller did not exit cleanly");
} 

void SIIterBotTest::testInteractiveCycle() {
  currentTestExecution = __FUNCTION__;
  casa::SIIterBot     iterBot("SITestService");
  
  int childPID = spawnController(true, true, false);
  failIf(childPID < 0, "Failed to spawn child process");

  int sleepCount = 10;
  while (iterBot.getNumberOfControllers() == 0) {
    sleep(1);
    if (--sleepCount <= 0) {
      failIf(true, "Remote Controller failed to connect");
      return;
    }
  }
  iterBot.waitForInteractiveInput();

  /* Waiting for child process to exit */
  failIf(checkChildState(childPID),
         "Controller did not exit cleanly");
}

void SIIterBotTest::testGettingDetailsLocal() {
  currentTestExecution = __FUNCTION__;
  casa::SIIterBot     iterBot("SITestService");
  
  std::map<std::string,DBus::Variant> myMap = iterBot.getDetails();
  failIf(myMap.size() != 15, "Incorrect number of fields in map");

  /* Translate back to a CASA record for simplicity (and a bit of testing) */
  casa::Record record = casa::DBusThreadedBase::toRecord(myMap);

  /* Just make sure the size is correct, we'll check values when we
     send in new ones
  */
  failIf(myMap.size() != record.nfields(),
         "Map and Record sizes do not match");
}
  
void SIIterBotTest::testSettingDetails() {
  currentTestExecution = __FUNCTION__;
  
  int childPID; 

  {
    casa::SIIterBot     iterBot("SITestService");
    childPID = spawnController(false, true, true);
    failIf(childPID < 0, "Failed to spawn child process");

    int sleepCount = 10;
    while (iterBot.getNumberOfControllers() == 0) {
      sleep(1);
      if (--sleepCount <= 0) {
        failIf(true, "Remote Controller failed to connect");
        return;
      }
    }

    iterBot.waitForInteractiveInput();
    
    /* Now check that iterBot has the correct values */
    failIf(iterBot.getNiter() != 123,
           "Niter Threshold value incorrect");
    failIf(iterBot.getCycleNiter() != 456,
           "CycleNiter Threshold value incorrect");
    failIf(iterBot.getInteractiveNiter() != 789,
           "InteractiveNiter Threshold value incorrect");
    failIf(!compareFloat(iterBot.getThreshold(), 5.67),
           "Threshold value incorrect");
    failIf(!compareFloat(iterBot.getCycleThreshold(),7.89),
           "CycleThreshold value incorrect");
    failIf(!compareFloat(iterBot.getInteractiveThreshold(),8.91),
           "InteractiveThreshold value incorrect");
    failIf(!compareFloat(iterBot.getLoopGain(),6.78),
           "Loop gain value incorrect");
    failIf(!compareFloat(iterBot.getCycleFactor(),4.56),
           "Cycle factor value incorrect");
  }
  /* Check that the client exited cleanly */
  failIf(checkChildState(childPID),
         "Controller did not exit cleanly");

}



int SIIterBotTest::spawnController(bool interactiveExit,
                                       bool interactiveResponse,
                                       bool checkDetails){

  int pid;
  if ((pid = fork()) == 0) {
    std::string interactiveExitStr(interactiveExit ? "true":"false");
    std::string interactiveResponseStr(interactiveResponse ? "true":"false");
    std::string checkDetailsStr(checkDetails ? "true":"false");

    /* Child Process */
    execl("/lustre/jkern/code/active/code/build/synthesis/tSIIterBot",
          "/lustre/jkern/code/active/code/build/synthesis/tSIIterBot", 
          interactiveExitStr.c_str(),
          interactiveResponseStr.c_str(),
          checkDetailsStr.c_str(),
          NULL);
    exit(0);
  }

  return pid;
}

int SIIterBotTest::checkChildState(int pid) {
  int pidStatus;
  int counter = 10;
  
  while (waitpid(pid, &pidStatus, WNOHANG) == 0) {
    counter--;
    if (counter <= 0)
      return -1;
    sleep(1);
  }
  return pidStatus;

}



/* This is a very simple implemenetaion of a Controller used
   for the testing*/
class SITestController: public edu::nrao::casa::SynthImager_proxy,
                        public casa::ServiceProxy
{
public:
  SITestController(const std::string& serviceName,
                   const bool&        exitOnInteractive,
                   const bool&        serviceInteractiveRequest,
                   const bool&        checkDetails):
    ServiceProxy(serviceName),
    serviceInteractiveFlag(serviceInteractiveRequest),
    exitOnInteractiveFlag(exitOnInteractive),
    checkDetailsFlag(checkDetails),
    interactiveIRQ(false),
    checkDetails(false),
    doneFlag(false),
    exitCondition(0)
  {
      }

  ~SITestController(){
    decrementController();
  }

  void interactionRequired(const bool& required){
    interactiveIRQ = required;
  }
  
  void detailUpdate(const std::map<std::string, DBus::Variant>& updatedParams){
    if (checkDetails && checkDetailsFlag) {
      casa::Record recOut = toRecord(updatedParams);

      
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
      failIf(!compareFloat(recOut.asFloat(casa::RecordFieldId("threshold")),
                           5.67), "incorrect threshold");
      failIf(!recOut.isDefined("cyclethreshold"),
             "cyclethreshold is not defined in the record");
      failIf(!compareFloat(recOut.asFloat(casa::RecordFieldId("cyclethreshold")),
                           7.89), "incorrect cyclethreshold");
      failIf(!recOut.isDefined("interactivethreshold"),
             "interactivethreshold is not defined in the record");
      failIf(!compareFloat(recOut.asFloat
                           (casa::RecordFieldId("interactivethreshold")),8.91),
             "incorrect interactive threshold");
      
      failIf(!recOut.isDefined("loopgain"),
             "loopgain is not defined in the record");
      failIf(!compareFloat(recOut.asFloat(casa::RecordFieldId("loopgain")),
                           6.78),
             "incorrect loopgain");
      failIf(!recOut.isDefined("cyclefactor"),
             "cyclefactor is not defined in the record");
      failIf(!compareFloat(recOut.asFloat(casa::RecordFieldId("cyclefactor")),
                           4.56), "incorrect cyclefactor");

    }    
  }

  void failIf(bool test, std::string message) {
    if (test) {
      std::cout << "Child Failure: " << message << std::endl;
      exitCondition = -1;
    }
  }

  bool compareFloat(float value1, float value2, float thresh = 1E-9){
    return fabs(value1 - value2) < thresh;
  }

  void sendInteractionComplete() {
    checkDetails = true;

    casa::Record record;
    record.define( casa::RecordFieldId("niter"), 123);
    record.define( casa::RecordFieldId("cycleniter"), 456);
    record.define( casa::RecordFieldId("interactiveniter"), 789);
    
    record.define( casa::RecordFieldId("threshold"), 5.67);
    record.define( casa::RecordFieldId("cyclethreshold"), 7.89);
    record.define( casa::RecordFieldId("interactivethreshold"), 8.91);
    
    record.define( casa::RecordFieldId("cyclefactor"), 4.56);
    record.define( casa::RecordFieldId("loopgain"), 6.78);

    std::map<std::string, DBus::Variant> map= fromRecord(record);
    controlUpdate(map);
    interactionComplete();
  }

  void summaryUpdate(const DBus::Variant& summary){
    std::cout << "Update Summary Signal Recieved" << std::endl;
  }

  void disconnect(){
    doneFlag = true;
  }

  int serviceLoop() {
    incrementController();
    while (!doneFlag) {
      if (interactiveIRQ) {
        interactiveIRQ = false;
        if (serviceInteractiveFlag) {
          sendInteractionComplete();
        }
        if (exitOnInteractiveFlag) {
          doneFlag = true;
        }
      }
      usleep(10000);
    }
    return exitCondition;
  }

protected:
  /* Control Flags */
  const bool serviceInteractiveFlag;
  const bool exitOnInteractiveFlag;
  const bool checkDetailsFlag;

  /* State Flags */
  bool interactiveIRQ;

  bool checkDetails;
  bool doneFlag;
  int  exitCondition;
};

/* If called without any arguments the main test program executes
   if arguments are included then we spawn a Controller process for testing
*/
int main(int argc, char** argv){
  using namespace casa;


  if (argc == 1) {
    /* Main Program */
    SIIterBotTest siIterBotTest;
    exit(siIterBotTest.runTests());
  } else {
    /* Controller Process:
       Behavior is set by the incoming arguments:
       arg[1]: If true, exit after an interacive cycle
       arg[2]: If true, respond to interactive requests
       arg[3]: If true, check the details after the interactive request
    */
    std::cout << "Not running SITestController test" << std::endl;
    /*    
    SITestController controller("SITestService",
                                !strcmp(argv[1], "true"),
                                !strcmp(argv[2], "true"),
                                !strcmp(argv[3], "true"));

    int exitCondition = controller.serviceLoop();
    exit(exitCondition);
    */
  }
}
