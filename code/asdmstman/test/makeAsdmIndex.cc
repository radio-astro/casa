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
  AipsIO aio(fname + "asdmindex", ByteIO::New);
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
  cout << "Performing consistency check ..." << endl;

  Table tab(msName);
  ROArrayColumn<Complex> data(tab, "DATA");
  ROArrayColumn<Complex> oldd(tab, "DATASAVE");
  for(uInt i=0; i<tab.nrow(); i++){
    if (!allNear(oldd(i), data(i), 1E-6)){
      cout << "disagreement in row " << i << " " << oldd(i) << endl;
      cout << data(i) << endl;
    }
  }
}

void createIndex (const String& msName, const string& fileName,
                  const string& dataPath, bool bigendian, bool docheck)
{
  map<string,int> fileMap;
  vector<string> fileNames;
  ifstream infile (fileName.c_str());
  AlwaysAssert (infile, AipsError);
  AsdmIndex ix;
  vector<AsdmIndex> index;
  // Read first line.
  string line;
  getline (infile, line);
  while (infile) {
    // A line looks like:
    // row fname nbl nspw nchan npol stepbl stepspw scalefactors fileoffset type
    //  0    1    2    3    4     5     6       7        8            9      10
    Vector<String> parts = stringToVector(line, '|');
    AlwaysAssert (parts.size() == 11, AipsError);
    ix.row = toInt(parts[0]);
    ix.nBl = toInt(parts[2]);
    ix.nSpw = toInt(parts[3]);
    ix.nChan = toInt(parts[4]);
    ix.nPol = toInt(parts[5]);
    ix.stepBl = toInt(parts[6]);
    ix.stepSpw = toInt(parts[7]);
    ix.fileOffset = toInt(parts[9]);
    ix.dataType = toInt(parts[10]);

    // Split scale factors (remove [] around it).
    String factStr (parts[8].substr(1, parts[8].size()-2));
    Vector<String> factors = stringToVector(factStr, ',');
    ix.scaleFactors.resize(0); //DP
    for (uInt i=0; i<factors.size(); ++i) {
      ix.scaleFactors.push_back (toDouble(factors[i]));
    }
    //for (uInt i=0; i<ix.scaleFactors.size(); ++i) {
    //  cout << "ix.scaleFactors[i] " << i << " " << ix.scaleFactors[i] << endl;
    //}

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

    Table tab(msName, Table::Update);
    if(docheck){ // Rename the DATA column to DATASAVE.
      tab.renameColumn ("DATASAVE", "DATA");
    }
    else{
      if(tab.canRemoveColumn("DATA")){ 
	tab.removeColumn ("DATA");
      }
      else{
	cout << "Cannot remove column DATA. Renaming it to DATASAVE" << endl;
	tab.renameColumn ("DATASAVE", "DATA");
      }
    }
    // Add new DATA column binding it to AsdmStMan.
    ArrayColumnDesc<Complex> dataCol ("DATA");
    AsdmStMan stman;
    tab.addColumn (dataCol, stman);
    tab.flush();
    // The name of the file is table.f<i>index, where <i> is the seqnr
    // of the new data manager.
    ostringstream oss;
    oss << RODataManAccessor(tab, "DATA", True).dataManagerSeqNr();
    // Now write the index file.
    Block<String> bdfNames(fileNames.size());
    for (uInt i=0; i<fileNames.size(); ++i) {
      bdfNames[i] = dataPath + '/' + fileNames[i];
    }
    writeIndex (tab.tableName() + "/table.f" + String(oss.str()),
                bigendian, bdfNames, index);
  }
}

int main(int argc, char* argv[])
{
  if (argc < 6  &&  argc != 2) {
    cerr << "Run as:   makeAsdmIndex ms [infile datapath isbigendian (0 or 1) check (0 or 1)]"
         << endl;
    cerr << "If 1 argument is given, only the DATA comparison will be done"
         << endl;
    return 1;
  }
  try {
    // Register AsdmStMan to be able to use it.
    AsdmStMan::registerClass();
    String msName(argv[1]);
    bool docheck = True;
    if (argc > 2) {
      string infile(argv[2]);
      string dataPath(argv[3]);
      bool bigendian = toInt(argv[4]);
      docheck = toInt(argv[5]);
      // Create the index file.
      createIndex (msName, infile, dataPath, bigendian, docheck);
    }
    // Compare the DATA and DATASAVE column.
    if(docheck){
      check (msName);
    }
  } catch (std::exception& x) {
    cerr << "Error: " << x.what() << endl;
    return 1;
  }
  return 0;
}


