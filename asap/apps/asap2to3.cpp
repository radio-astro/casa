#include <casa/aips.h>
#include <casa/iostream.h>
#include <casa/Inputs.h>
#include <tables/Tables/Table.h>
#include <tables/Tables/ScaColDesc.h>

//#include <casa/namesapce.h>
using namespace casa;

int main(int argc, const char* argv[]) {
  // enable input in no-prompt mode
  Input inputs(1);
  // define the input structure
  inputs.version("20090915MM");
  inputs.create ("in", "", "Name of asap file", "string");
  // Fill the input structure from the command line.
  inputs.readArguments (argc, argv);
  String fname =  inputs.getString("in");
  Table origtab(fname);
  fname +=".asap3";
  origtab.deepCopy(fname, Table::New);
  Table tab(fname, Table::Update);
  tab.removeColumn("PARANGLE");
  Table tfocus = tab.rwKeywordSet().asTable("FOCUS");
  ScalarColumnDesc<Float> pa("PARANGLE");
  pa.setDefault(Float(0.0));
  tfocus.addColumn(pa);
  //tfocus.rwKeywordSet().define("PARALLACTIFY", False)
  Int verid=tab.rwKeywordSet().fieldNumber("VERSION");
  tab.rwKeywordSet().define(verid,uInt(3));
  tab.tableInfo().setType("Scantable");
  cout << "WARNING: This has invalidated the parallactic angle in the data. Reprocess the data in ASAP 3 "
       << "if you need to handle polarisation conversions"
       << endl;
  return 0;
  
}
