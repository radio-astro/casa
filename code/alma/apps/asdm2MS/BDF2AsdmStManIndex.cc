#include <boost/foreach.hpp>
#include <asdmstman/AsdmStMan.h>
#include <asdmstman/AsdmIndex.h>
#include <casa/IO/AipsIO.h>
#include <casa/Arrays/ArrayIO.h>
#include <casa/Containers/BlockIO.h>

#include <map>

using namespace std;
using namespace casa;

class BDF2AsdmStManIndex {
public:
  BDF2AsdmStManIndex(const String& fname, bool asBigEndian, const Block<String>& bdfNames);
  virtual ~BDF2AsdmStManIndex();
  void done();
  void clearIndexes();
  void clearAutoIndexes();
  void clearCrossIndexes();
  void appendAutoIndex(const AsdmIndex& index);
  void appendCrossIndex(const AsdmIndex& index);

  static int version();

  void writeIndexes();

private:
  String fname;
  bool asBigEndian;
  Block<String> bdfNames;
  map<String, int> s2i_m;
  vector<AsdmIndex> autoIndexes;
  vector<AsdmIndex> crossIndexes;
};

int BDF2AsdmStManIndex::version() { return 1; }

BDF2AsdmStManIndex::BDF2AsdmStManIndex(const String& fname, bool asBigEndian, const Block<String>& bdfNames):fname(fname), asBigEndian(asBigEndian), bdfNames(bdfNames) {
  BOOST_FOREACH(String bdfName, bdfNames) {
    if (s2i_m.find(bdfName) == s2i_m.end()) s2i_m[bdfName] = s2i_m.size();
  }
}

BDF2AsdmStManIndex::~BDF2AsdmStManIndex() {}

void BDF2AsdmStManIndex::done(){
  s2i_m.clear();
  autoIndexes.clear();
  crossIndexes.clear();
}

void BDF2AsdmStManIndex::clearIndexes() {
  autoIndexes.clear();
  crossIndexes.clear();
}

void BDF2AsdmStManIndex::clearAutoIndexes() {
  autoIndexes.clear();
}

void BDF2AsdmStManIndex::clearCrossIndexes() {
  crossIndexes.clear();
}

void BDF2AsdmStManIndex::appendAutoIndex(const AsdmIndex& asdmIndex) {
  autoIndexes.push_back(asdmIndex);
}

void BDF2AsdmStManIndex::appendCrossIndex(const AsdmIndex& asdmIndex) {
  crossIndexes.push_back(asdmIndex);
}

void BDF2AsdmStManIndex::writeIndexes() {
  AipsIO aio(fname +  + "asdmindex", ByteIO::New);
  aio.putstart ("AsdmStMan", version());
  // Write the index info.
  aio << asBigEndian << bdfNames;
  aio.put (autoIndexes);
  aio.put (crossIndexes);
  aio.putend();
}

