#include <synthesis/Parallel/Applicator.h>
#include <synthesis/Parallel/Algorithm.h>
#include <synthesis/Parallel/PTransport.h>
#include <casa/Arrays/Vector.h>
#include <casa/BasicSL/Complex.h>
#include <casa/BasicSL/String.h>
#include <casa/iostream.h>

#include <casa/namespace.h>
extern casa::Applicator casa::applicator;

class TestAlgorithm : public Algorithm {
   public :
      TestAlgorithm() : myName(String("Test Algorithm")){}
     ~TestAlgorithm(){};

      void get();
      void put();
      String &name(){return myName;}
   private :
      Int      one;
      Float    two;
      Double   three;
      Complex  four;
      DComplex five;
      String   six;
      Bool     seven;
      
      Vector<Int>      aOne;
      Vector<Float>    aTwo;
      Vector<Double>   aThree;
      Vector<Complex>  aFour;
      Vector<DComplex> aFive;
      void             task();
      String           myName;
};

void TestAlgorithm::get(){

      cout << "In TestAlgorithm::get" << endl;
      casa::applicator.get(one);
      cout << "got one " << one << endl;
      casa::applicator.get(two);
      cout << "got two " << two << endl;
      casa::applicator.get(three);
      cout << "got three " << three << endl;
      casa::applicator.get(four);
      cout << "got four " << four << endl;
      casa::applicator.get(five);
      cout << "got five " << five << endl;
      casa::applicator.get(six);
      cout << "got six " << six << endl;
      casa::applicator.get(seven);
      cout << "got seven " << seven << endl;

      casa::applicator.get(aOne);
      cout << "got aOne " << endl;
      casa::applicator.get(aTwo);
      cout << "got aTwo " << endl;
      casa::applicator.get(aThree);
      cout << "got aThree " << endl;
      casa::applicator.get(aFour);
      cout << "got aFour " << endl;
      casa::applicator.get(aFive);
      cout << "got aFive " << endl;

      return;
}

void TestAlgorithm::put(){
      casa::applicator.put(True);
      return;
}

void TestAlgorithm::task(){
      cout << "Do work now!" << endl;
      return;
}

//  OK the test program

int main(Int argc, Char *argv[]){

   TestAlgorithm testMe;
   casa::applicator.defineAlgorithm(&testMe);
   casa::applicator.init(argc, argv);
   if(casa::applicator.isController()){
      Int rank(1);
      casa::applicator.nextAvailProcess(testMe, rank);

      Int      one(1);
      Float    two(2.0f);
      Double   three(3.0);
      Complex  four(4.0,4.0);
      DComplex five(5.0,5.0);
      String   six("Six");
      Bool     seven(True);
      cout << "one " << one << endl;
      cout << "two " << two << endl;
      cout << "three " << three << endl;
      cout << "four " << four << endl;
      cout << "five " << five << endl;
      cout << "six " << six << endl;
      cout << "seven " << seven << endl;

      casa::applicator.put(one);
      casa::applicator.put(two);
      casa::applicator.put(three);
      casa::applicator.put(four);
      casa::applicator.put(five);
      casa::applicator.put(six);
      casa::applicator.put(seven);
      
      Vector<Int>      aOne(3,one);
      Vector<Float>    aTwo(4,two);
      Vector<Double>   aThree(5,three);
      Vector<Complex>  aFour(6,four);
      Vector<DComplex> aFive(7,five);

      casa::applicator.put(aOne);
      casa::applicator.put(aTwo);
      casa::applicator.put(aThree);
      casa::applicator.put(aFour);
      casa::applicator.put(aFive);

      casa::applicator.apply(testMe);
      Bool status;
      casa::applicator.get(status);
/*
      Bool allDone;
      applicator.nextProcessDone(testMe, allDone);
*/

      return 0;

   }
   return 0;
}
