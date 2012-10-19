//  0|uid___A002_X2bf6bf_X1e|105|4|64|4|15|4|64|4|1024|4|1024|4|[412431.812546, 412431.812546, 412431.812546, 412431.812546]|3145|863403|1
//  row filename nbl nspw nchan npol nant nspw nchan npol stepbl stepspw stepant stepspw scalefactors offsetcross offsetauto type

#include <fstream>
#include <iostream>
#include <map>
#include <tables/Tables/Table.h>
#include <asdmstman/AsdmStMan.h>
#include <asdmstman/AsdmIndex.h>
#include <tables/Tables/ArrayColumn.h>
#include <tables/Tables/ArrColDesc.h>
#include <tables/Tables/DataManAccessor.h>
#include <casa/Arrays/Array.h>
#include <casa/Arrays/ArrayIO.h>
#include <casa/Arrays/ArrayUtil.h>
#include <casa/Arrays/ArrayLogical.h>
#include <casa/IO/AipsIO.h>
#include <casa/Containers/BlockIO.h>
#include <casa/Utilities/Assert.h>
#include <casa/Exceptions/Error.h>

using namespace std;
using namespace casa;

// Write the index file in the same way as read by AsdmStMan.
// The file name should asl obe that expected by AsdmStMan.
void writeIndex (const String& fname,
                 bool asBigEndian,
                 const Block<String>& bdfNames,
                 const vector<AsdmIndex>& index)
{
  AipsIO aio(fname + "index", ByteIO::New);
  aio.putstart ("AsdmStMan", 1);
  // Write the index info.
  aio << asBigEndian << bdfNames;
  aio.put (index);
  aio.putend();
}

int toInt (const String& str)
{
  istringstream istr(str);
  int v;
  istr >> v;
  return v;
}
double toDouble (const String& str)
{
  istringstream istr(str);
  double v;
  istr >> v;
  return v;
}

void check (const String& msName)
{
  Table tab(msName);
  ROArrayColumn<Complex> data(tab, "DATA");
  ROArrayColumn<Complex> oldd(tab, "DATASAV");
  for(uInt i=0; i<tab.nrow(); i++){
    if (!allEQ(oldd(i), data(i))){
      cout << "disagreement in row " << i << " " << oldd(i) << endl;
      cout << data(i) << endl;
    }
  }
}

int main(int argc, char* argv[])
{
  if (argc < 5) {
    cerr << "Run as:   makeAsdmIndex ms infile datapath isbigendian (0 or 1)"
         << endl;
    return 1;
  }
  String msName(argv[1]);
  string dataPath(argv[3]);
  bool bigendian = toInt(argv[4]);
  try {
    // Register AsdmStMan to be able to use it.
    AsdmStMan::registerClass();
    map<string,int> fileMap;
    vector<string> fileNames;
    ifstream infile (argv[2]);
    AlwaysAssert (infile, AipsError);
    AsdmIndex ix;
    vector<AsdmIndex> index;
    // Read first line.
    string line;
    cout << "start" << endl;
    getline (infile, line);
    while (infile) {
      //cout << "new line" << endl;
      Vector<String> parts = stringToVector(line, '|');
      AlwaysAssert (parts.size() == 18, AipsError);
      ix.row = toInt(parts[0]);
      ix.nBl = toInt(parts[2]);
      ix.crossNspw = toInt(parts[3]);
      ix.crossNchan = toInt(parts[4]);
      ix.crossNpol = toInt(parts[5]);
      ix.nAnt = toInt(parts[6]);
      ix.autoNspw = toInt(parts[7]);
      ix.autoNchan = toInt(parts[8]);
      ix.autoNpol = toInt(parts[9]);
      ix.crossStepBl = toInt(parts[10]);
      ix.crossStepSpw = toInt(parts[11]);
      ix.autoStepBl = toInt(parts[12]);
      ix.autoStepSpw = toInt(parts[13]);
      ix.crossOffset = toInt(parts[15]);
      ix.autoOffset = toInt(parts[16]);
      ix.dataType = toInt(parts[17]);
      // Split scale factors.
      String factStr (parts[14].substr(1, parts[14].size()-2));
      Vector<String> factors = stringToVector(factStr, ',');
      for (uInt i=0; i<factors.size(); ++i) {
        ix.scaleFactors.push_back (toDouble(factors[i]));
      }
      // Now turn file name into a filenr.
      map<string,int>::const_iterator iter = fileMap.find(parts[1]);
      if (iter == fileMap.end()) {
        // New filename.
        ix.fileNr = fileNames.size();
        fileNames.push_back (parts[1]);
        fileMap[parts[1]] = ix.fileNr;
      } else {
        ix.fileNr = iter->second;
      }
      // The index entry is complete; add it to the vector.
      index.push_back (ix);
      // Read next line.
      getline (infile, line);
    }

    // Handle the MS if given.
    if (msName.empty()) {
      for (uInt i=0; i<fileNames.size(); ++i) {
        cout << i << "  " << fileNames[i] << endl;
      }
    } else {
      {
	// Rename the DATA column to DATASAV.
	Table tab(msName, Table::Update);
	if(!tab.tableDesc().isColumn("DATASAV")){
	  tab.renameColumn ("DATASAV", "DATA");
	}
	// Add new DATA column binding it to AsdmStMan.
	ArrayColumnDesc<Complex> dataCol ("DATA");
	AsdmStMan stman;
	tab.addColumn (dataCol, stman);
	tab.flush();
	// The name of the file is table.f<i>index, where <i> is the seqnr
	// of the new data manager.
	//ostringstream oss;
	//oss << RODataManAccessor(tab, "DATA", True).dataManagerSeqNr();
	// Now write the index file.
	Block<String> bdfNames(fileNames.size());
	for (uInt i=0; i<fileNames.size(); ++i) {
	  bdfNames[i] = dataPath + '/' + fileNames[i];
	}
	//writeIndex (tab.tableName() + "/table.f" + String(oss.str()),
	//          bigendian, bdfNames, index);
	writeIndex (tab.tableName() + "/table.asdm",
		    bigendian, bdfNames, index);
      }

      check(msName);

    }
  } catch (std::exception& x) {
    cerr << "Error: " << x.what() << endl;
    return 1;
  }
  return 0;
}
