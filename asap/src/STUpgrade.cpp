#include <tables/Tables/Table.h>
#include <tables/Tables/ScaColDesc.h>
#include <tables/Tables/ArrColDesc.h>
#include <tables/Tables/ArrayColumn.h>

#include "STUpgrade.h"


using namespace casa;

namespace asap {

std::string STUpgrade::upgrade(const std::string& name) {
  std::string inname = name;
  Table origtab(name);
  uInt version = origtab.keywordSet().asuInt("VERSION");
  if (version == version_) {
    return name;
  }
  if (version == 2) {
    inname = two2three(inname);
    version = 3;
  }
  if (version == 3) {
    return three2four(inname);
  }
  throw(AipsError("Unsupported version of ASAP file."));
}

std::string STUpgrade::two2three(const std::string& name) {
  
  std::string fname = name+".asap3";
  Table origtab(name);
  origtab.deepCopy(fname, Table::New);
  Table tab(fname, Table::Update);
  tab.removeColumn("PARANGLE");
  Table tfocus = tab.rwKeywordSet().asTable("FOCUS");
  ScalarColumnDesc<Float> pa("PARANGLE");
  pa.setDefault(Float(0.0));
  tfocus.addColumn(pa);
  //tfocus.rwKeywordSet().define("PARALLACTIFY", False)
  Int verid = tab.rwKeywordSet().fieldNumber("VERSION");
  tab.rwKeywordSet().define(verid, uInt(3));
  tab.tableInfo().setType("Scantable");
  return fname;
}

std::string STUpgrade::three2four(const std::string& name) {
  std::string fname = name;
  Table origtab(name);
  Vector<String> cnames(3);
  cnames[0] = "RESTFREQUENCY";
  cnames[1] = "NAME";
  cnames[2] = "FORMATTEDNAME";
  Table origmoltab = origtab.rwKeywordSet().asTable("MOLECULES");
  const ColumnDesc &desc = (origmoltab.tableDesc().columnDescSet())[cnames[0]];
  Bool isScalar = desc.isScalar() ;
  if ( isScalar ) {
    fname += ".asap4";
    origtab.deepCopy(fname, Table::New);
    Table tab(fname, Table::Update);
    Table moltable = tab.rwKeywordSet().asTable("MOLECULES");
    ROScalarColumn<Double> rfcol(moltable, cnames[0]);
    ROScalarColumn<String> nmecol(moltable,  cnames[1]);
    ROScalarColumn<String> fmtnmecol(moltable, cnames[2]);
    Vector<Double> rf = rfcol.getColumn();
    Vector<String> nme = nmecol.getColumn();
    Vector<String> fmtnme = fmtnmecol.getColumn();
    Array<Double> arf = rf.addDegenerate(1);
    Array<String> anme = nme.addDegenerate(1);
    Array<String> afmtnme = fmtnme.addDegenerate(1);
    moltable.removeColumn(cnames);
    moltable.addColumn(ArrayColumnDesc<Double>(cnames[0]));
    moltable.addColumn(ArrayColumnDesc<String>(cnames[1]));
    moltable.addColumn(ArrayColumnDesc<String>(cnames[2]));
    ArrayColumn<Double> arfcol(moltable, cnames[0]);
    ArrayColumn<String> anmecol(moltable, cnames[1]);
    ArrayColumn<String> afmtnmecol(moltable, cnames[2] );
    arfcol.putColumn(arf);
    anmecol.putColumn(anme);
    afmtnmecol.putColumn(afmtnme);
    Int verid = tab.rwKeywordSet().fieldNumber("VERSION");
    tab.rwKeywordSet().define(verid, uInt(4));
  }
  else {
    Int verid = origtab.rwKeywordSet().fieldNumber("VERSION");
    origtab.rwKeywordSet().define(verid, uInt(4));
  }
  return fname;
}

}
