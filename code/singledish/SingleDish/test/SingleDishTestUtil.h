#ifndef TEST_SINGLEDISHTESTUTIL_H_
#define TEST_SINGLEDISHTESTUTIL_H_

#include <iostream>
#include <list>
#include <string>
#include <utility>

#include <casacore/casa/OS/EnvVar.h>

using namespace std;

namespace test_utility {

#define ELEMENTSOF(x) (sizeof(x) / sizeof((x)[0]))

  template <typename DataType>
  void PrintArray(string const name, size_t const num_data, DataType const* data, size_t max_size = 32) {
    if (num_data > max_size) {
      cout << name << " has " << num_data << " elements" << endl;
      return;
    }
    cout <<  name << " = ";
    ostringstream oss;
    cout << "[";
    if (num_data > 0)
      cout << data[0] ;
    for (size_t i=1; i<num_data; ++i) {
      cout << ", "  << data[i];
    }
    cout << "] (" << num_data << " elements)" << endl;
    return;
  }

  template <>
  void PrintArray(string const name, size_t const num_data, bool const* data, size_t max_size) {
    if (num_data > max_size) {
      cout << name << " has " << num_data << " elements" << endl;
      return;
    }
    cout <<  name << " = [" ;
    if (num_data > 0)
      cout << ( data[0] ? 'T' : 'F' ) ;
    for (size_t i=1; i<num_data; ++i) {
      cout << ", " << ( data[i] ? 'T' : 'F' );
    }
    cout << "] (" << num_data << " elements)" << endl;
    return;
  }

  void PrintLine(list<pair<size_t,size_t>> &line_list) {
    cout << "Number of Lines: " << line_list.size() << endl;
    for (list<pair<size_t,size_t>>::iterator iter = line_list.begin();
	 iter!=line_list.end(); ++iter) {
      cout << "- [ " << (*iter).first << ", " << (*iter).second << " ]" << endl;
    }
  }

  string GetCasaDataPath() {
    if (casacore::EnvironmentVariable::isDefined ("CASAPATH"))  {
      string casapath = casacore::EnvironmentVariable::get("CASAPATH");
      size_t endindex = casapath.find(" ");
      if (endindex!=string::npos) {
	string casaroot = casapath.substr(0, endindex);
	cout << "casaroot = " << casaroot << endl;
	return (casaroot + "/data/");
      }
      else {
	cout << "hit npos" << endl;
	return "/data/";
      }
    }
    else {
      cout  << "CASAPATH is not defined" << endl;
      return "";
    }
  }
}

#endif /* TEST_SINGLEDISHTESTUTIL_H_ */
