#ifndef _NAME2TABLE_H_
#define _NAME2TABLE_H_
#include "ASDMTableBase.h"

#include <map>
#include <set>

using namespace std;

class Name2Table {
 private:
  static map<string, ASDM_TABLE_BASE*> name2Table_;
  static bool init_;
  static bool init();

  static set<ASDM_TABLE_BASE*> table_;

 public:
  static const set<ASDM_TABLE_BASE*>& find(const vector<string>& name,bool verbose=false);
};
#endif // _NAME2TABLE_H_
