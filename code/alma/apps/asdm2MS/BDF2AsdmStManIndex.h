#ifndef BDF2ASDMSTMANINDEX
#define BDF2ASDMSTMANINDEX
#include "CPrimitiveDataType.h"
#include <casa/Containers/Block.h>
#include <asdmstman/AsdmIndex.h>
#include <casa/Arrays/Array.h>
#include <casa/Arrays/ArrayIO.h>
#include <casa/Arrays/ArrayUtil.h>
#include <casa/Arrays/ArrayLogical.h>
#include <casa/IO/AipsIO.h>
#include <casa/Containers/BlockIO.h>
#include <map>

#include <boost/lambda/lambda.hpp>
#include <boost/lambda/bind.hpp>
#include <boost/lambda/casts.hpp>
#include <stdint.h>

using namespace boost::lambda;
/*
** A simplistic tracing toolbox.
*/
extern bool debug; 
extern vector<char> logIndent;
#define LOGENTER(name) if (debug) {for_each(logIndent.begin(), logIndent.end(), cout << _1); logIndent.push_back('\t'); cout << #name ": entering" << endl;}
#define LOGEXIT(name)  if (debug) {logIndent.pop_back(); for_each(logIndent.begin(), logIndent.end(), cout << _1); cout << #name ": exiting" << endl;}
#define LOG(msg) if (debug) {for_each(logIndent.begin(), logIndent.end(), cout << _1); cout << msg << endl;}

class BDF2AsdmStManIndex {
public:
  BDF2AsdmStManIndex(const std::vector<std::string>& bdfNames, bool isBigEndian, const string& fname);
  virtual ~BDF2AsdmStManIndex();
  void setNumberOfDataDescriptions(unsigned int numberOfDataDescriptions);
  void						done();
  void						clearIndexes();
  void						clearAutoIndexes();
  void						clearCrossIndexes();
  void appendAutoIndex(unsigned int             iDD,
		       const string&		bdfName,
		       unsigned int		nBl,
		       unsigned int		nSpw,
		       unsigned int		nChan,
		       unsigned int		nPol,
		       unsigned int		stepBl,
		       unsigned int		stepSpw,
		       const vector<double>&	scaleFactors,
		       uint64_t	         	fileOffset,
		       uint32_t                 spwOffset);

  void appendWVRIndex(unsigned int              iDD,
		      const string&		bdfName,
		      unsigned int		nBl,
		      unsigned int		nSpw,
		      unsigned int		nChan,
		      unsigned int		nPol,
		      unsigned int		stepBl,
		      unsigned int		stepSpw,
		      const vector<double>&	scaleFactors,
		      uint64_t	         	fileOffset,
		      uint32_t                  spwOffset);

  void appendCrossIndex(unsigned int            iDD,
			const string&		bdfName,
			unsigned int		nBl,
			unsigned int		nSpw,
			unsigned int		nChan,
			unsigned int		nPol,
			unsigned int		stepBl,
			unsigned int		stepSpw,
			const vector<double>&	scaleFactors,
			uint64_t		fileOffset,
			uint32_t                spwOffset,
			PrimitiveDataTypeMod::PrimitiveDataType       dataType);

  void	dumpAutoCross();
  void	dumpCrossAuto();

  static int version();

private:
  uint32_t                      numberOfDataDescriptions;
  casa::Block<casa::String>	bdfNames;
  casa::String			fname;
  std::map<std::string, int>	s2i_m;
  bool				asBigEndian;
  std::vector<std::vector<casa::AsdmIndex> >	autoIndexes_vv;
  std::vector<std::vector<casa::AsdmIndex> >	crossIndexes_vv;
  std::vector<casa::AsdmIndex>  allIndexes_v;
  casa::AipsIO			aio;
  uint64_t			MSMainRowNumber;
};
#endif
