
//
// C++ Implementation: LineCatalog
//
// Description: A class representing a line catalog
//
//
// Author: Malte Marquarding <asap@atnf.csiro.au>, (C) 2006
//
// Copyright: See COPYING file that comes with this distribution
//
//

// std includes

// casa includes
#include <casa/Exceptions/Error.h>
#include <casa/iostream.h>
#include <casa/iomanip.h>
#include <casa/OS/Path.h>
#include <casa/OS/File.h>
#include <casa/Arrays/Vector.h>
#include <tables/Tables/ReadAsciiTable.h>
#include <tables/Tables/TableParse.h>
#include <tables/Tables/ScalarColumn.h>

#include "STAttr.h"
#include "LineCatalog.h"

using namespace casa;

namespace asap
{

LineCatalog::LineCatalog(const std::string& name)
{
  Path path(name);
  std::string inname = path.expandedName();
  File f(inname);
  if (f.isDirectory()) { //assume it's a table
    table_ = Table(inname);
  } else {
    String formatString;
    // formatSring , TableType, ascii file name, TableDesc name, table name, autoheader
    table_ = readAsciiTable(formatString, Table::Plain, inname, "", "", True);
    // do not keep aips++ table
    table_.markForDelete();
  }
  baseTable_ = table_;
}

void LineCatalog::setStrengthLimits(double smin, double smax)
{
  table_ = setLimits(smin, smax, "Column4");
}

void LineCatalog::setFrequencyLimits(double fmin, double fmax)
{
  table_ = setLimits(fmin, fmax, "Column2");
}

void LineCatalog::setPattern(const std::string& name, const std::string& stype)
{
  std::string mode = stype+"('"+name+"')";
  std::string taql = "SELECT FROM $1 WHERE Column1 == " + mode;
  Table tmp = tableCommand(taql, table_);
  if (tmp.nrow() > 0) table_ = tmp.sort("Column2");
  else throw(AipsError("No match."));
}

Table LineCatalog::setLimits(double lmin, double lmax, const std::string& colname)
{
  Table tmp = table_(table_.col(colname) > lmin && table_.col(colname) < lmax);
  if (tmp.nrow() > 0) return tmp.sort("Column2");
  else throw(AipsError("No match."));
}

void LineCatalog::save(const std::string& name)
{
  Path path(name);
  std::string inname = path.expandedName();
  table_.deepCopy(inname, Table::New);
}

std::string LineCatalog::summary(int row) const
{
  std::string stlout;
  ostringstream oss;
  oss << asap::SEPERATOR << endl;
  oss << "Line Catalog summary" << endl;
  oss << asap::SEPERATOR << endl << endl;
  if (row == -1) {
    for (uint i=0; i<table_.nrow(); ++i) {
      oss << std::right << setw(7) << i << setw(2) << "";
      oss << std::left << setw(20) << getName(i);
      oss << setw(12) << setprecision(8) << std::left << getFrequency(i);
      oss << setw(12) << setprecision(8) << std::left << getStrength(i);
      oss << endl;
    }
  } else {
      if ( row < table_.nrow() ) {
        oss << std::right << setw(7) << row << setw(2) << "";
        oss << std::left << setw(20) << getName(row);
        oss << setw(12) << setprecision(8) << std::left << getFrequency(row);
        oss << setw(12) << setprecision(8) << std::left << getStrength(row);
        oss << endl;
      } else {
        throw(AipsError("Row doesn't exist"));
      }
  }
  return String(oss);
}


std::string LineCatalog::getName(uint row) const
{
  ROScalarColumn<String> col(table_, "Column1");
  return col(row);
}

double LineCatalog::getFrequency(uint row) const
{
  return getDouble("Column2", row);
}

double LineCatalog::getStrength(uint row) const
{
  return getDouble("Column4", row);
}

double LineCatalog::getDouble(const std::string& colname, uint row) const {
  DataType dtype = table_.tableDesc().columnDesc(colname).dataType();
  if (dtype == TpDouble) {
    ROScalarColumn<Double> col(table_, colname);
    return col(row);
  } else if (dtype == TpInt) {
    ROScalarColumn<Int> col(table_, colname);
    return Double(col(row));
  } else {
    throw AipsError("Column " + colname + "doesn't contain numerical values." );
  }
}

} // namespace


