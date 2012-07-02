//
// C++ Interface: STSelector
//
// Description:
//
//
// Author: Malte Marquarding <asap@atnf.csiro.au>, (C) 2006
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef ASAPSTSELECTOR_H
#define ASAPSTSELECTOR_H

#include <string>
#include <vector>
#include <map>

#include <casa/Containers/Block.h>
#include <casa/BasicSL/String.h>
#include <tables/Tables/Table.h>

namespace asap {

/**
A class to set a subselection of a Scantable

@author Malte Marquarding
*/
class STSelector {

public:
  STSelector();
  STSelector(const STSelector& other);

  STSelector& operator=(const STSelector& other);

  virtual ~STSelector();

  void setScans(const std::vector<int>& scans);
  void setBeams(const std::vector<int>& beams);
  void setIFs(const std::vector<int>& ifs);
  void setPolarizations(const std::vector<int>& pols);
  void setPolFromStrings(const std::vector<std::string>& pols);
  void setCycles(const std::vector<int>& cycs);
  void setName(const std::string&);
  void setTypes(const std::vector<int>& types);
  virtual void setTaQL(const std::string& taql);

  void setSortOrder(const std::vector<std::string>& order);
  void setRows(const std::vector<int>& rows);

  std::vector<int> getScans() const;
  std::vector<int> getBeams() const;
  std::vector<int> getIFs() const;
  std::vector<int> getPols() const;
  std::vector<int> getCycles() const;
  std::vector<int> getTypes() const;
  std::vector<std::string> getPolTypes() const;
  std::string getTaQL() const { return taql_; }
  std::vector<std::string> getSortOrder() const;

  casa::Table apply(const casa::Table& tab);
  casa::Table operator()(const casa::Table& tab) { return apply(tab); };

  void reset() { intselections_.clear();stringselections_.clear(); taql_ = "";};

  bool empty() const;

  std::string print();

protected:
  std::vector< int > getint( const std::string& key) const;
  //std::vector< std::string > getstring( const std::string& key) const;

  void setint(const std::string& key, const std::vector< int >& val);
  void setstring(const std::string& key, const std::vector< std::string >& val);

private:

  casa::Table sort(const casa::Table& tab);

  typedef std::map<std::string, std::vector<int> > intidmap;
  typedef std::map<std::string, std::vector<std::string> > stringidmap;
  // has to be mutable, as to stl limitations
  mutable intidmap intselections_;
  mutable stringidmap stringselections_;
  std::vector<std::string> poltypes_;
  casa::Block<casa::String> order_;
  std::string taql_;
  std::vector<int> rowselection_;
};

}

#endif
