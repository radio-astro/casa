// Compile command against local build environment:
//   g++ /Users/nakazato/work/eclipse/casadevel/casa/code/singledish/Filler/test/SampleReaderDemo.cc -o SampleReaderDemo -I../../darwin/casacode/ -I../../darwin/include -I../../darwin/include/casacore -L../../darwin/lib -lcasa_casa -lcasa_measures -lcasa_ms -lcasa_tables -lsingledish -std=c++11 -DUseCasacoreNamespace

#include <singledish/Filler/SingleDishMSFiller.h>
#include <casacore/casa/Logging/LogIO.h>
#include "SampleReader.h"

int main(int argc, char *argv[]) {
  casacore::LogIO os(LogOrigin("", "SampleReaderDemo", WHERE));
  os <<"This is a test program to demonstrate how SingleDishMSFiller<SampleReader>\n"
     << "works. Usage of the filler is only three steps below:\n"
     << "\n"
     << "    int main(int argc, char *argv[]) {\n"
     << "(1)   casa::SingleDishMSFiller<SampleReader> filler(\"mysampledata.nro\", false);\n"
     << "(2)   filler.fill();\n"
     << "(3)   filler.save(\"mysampledata.ms\");\n"
     << "      return 0;\n"
     << "    }\n"
     << "\n" << casacore::LogIO::POST;
  os << "Here I will show you how Filler/Reader works:\n"
     << "\n" << casacore::LogIO::POST;
  os << "=== Step 1. Create filler object with SampleReader\n"
     << "=== (1)   casa::SingleDishMSFiller<SampleReader> filler(\"mysampledata.nro\", false);\n"
     << "  (NB: input data name \"mysampledata.nro\" is dummy. SampleReader generates\n"
     << "       the data on-the-fly)\n"
     << "\n" << casacore::LogIO::POST;
  casa::SingleDishMSFiller<SampleReader> filler("mysampledata.nro", false);
  os << "=== Step 2. Fill MS\n"
     << "=== (2)   filler.fill();\n"
     << "\n" << casacore::LogIO::POST;
  filler.fill();
  os << "=== Step 3. Write MS as \"mysampledata.ms\"\n"
     << "=== (3)   filler.save(\"mysampledata.ms\");\n"
     << "\n" << casacore::LogIO::POST;
  filler.save("mysampledata.ms");
  os << "Now you should have \"mysampledata.ms\" on your current working directory.\n"
     << "Please open it with, e.g., casabrowser to see the contents!\n"
     << casacore::LogIO::POST;

  return 0;
}

