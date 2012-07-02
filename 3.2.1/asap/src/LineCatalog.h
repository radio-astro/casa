//
// C++ Interface: LineCatalog
//
// Description:
//
//
// Author: Malte Marquarding <Malte.Marquarding@csiro.au>, (C) 2006
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef LINECATALOG_H
#define LINECATALOG_H

#include <string>

#include <casa/aips.h>
#include <tables/Tables/Table.h>

namespace asap {
/**
  * A represenation of a line catalog, which can be ASCII, or an aips++ table
  *
  * ASCII catalogs have to be formatted like JPL.
  * Name frequency error log(I)
  * Only  "name", "frequency" and log(I) are used at this stage
  *
  * @author Malte Marquarding
  * @date $Date:$
  */
class LineCatalog {
public:
  /**
    *
    * @param name the name of the ASCII file or aips++ table
    */
  explicit LineCatalog(const std::string& name = "jpl");

  virtual ~LineCatalog() {}

  /**
   * select a subset of the data by frequency range
   * @param fmin the lower frequency bound
   * @param fmax the upper frequency bound
   */
  void setFrequencyLimits(double fmin, double fmax);

  /**
    * select a subset of the table by line strength range
    * @param smin the lower strength bound
    * @param smin the upper strength bound
    */
  void setStrengthLimits(double smin, double smax);
  
  /**
    * select a subset of the data by name pattern match (unix-style)
    * @param name the string pattern e.g. "*CS*"
    * @param ptype pattern type e.g.
    * @li "pattern"
    * @li "regex"
    */
  void setPattern(const std::string& name, const std::string& ptype="pattern");
  
  /**
    * save the table  with current limits to disk (as an aips++ table)
    * @param name the filename
    */
  void save(const std::string& name);
  
  /**
    * Return a string representation of this table
    * @param row an integer describing the row number to show
    * default -1 is all rows
    * @return std::string
    */
  std::string summary(int row=-1) const;

  /**
   * Return the rest frequency value for a specific row
   * @param row the row number
   * @return a double rest frequency value
   */
  double getFrequency(uint row) const;

  /**
   * Return the line strength value for a specific row
   * @param row the row number
   * @return a double rest line strength value
   */
  double getStrength(uint row) const;

  /**
   *
   * @param row
   * @return
   */
  std::string getName(uint row) const;

  int nrow() const  { return table_.nrow(); }

  void reset() { table_ = baseTable_; }

private:
  /**
   * utility function to handle range limits
   * @param lmin the lower limit
   * @param lmax the upper limit
   * @param colname the columd to apply the limits to
   * @return a new casa::Table
   */
  casa::Table setLimits(double lmin, double lmax, const std::string& colname);

  double getDouble(const std::string& colname, uint row) const;

  // the table with seelection
  casa::Table table_;
  // the pristine table
  casa::Table baseTable_;
};

} // namespace

#endif //LINECATALOG_H
