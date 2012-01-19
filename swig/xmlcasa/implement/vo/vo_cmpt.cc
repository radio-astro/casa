
/***
 * Framework independent implementation file for Vo...
 *
 * Implement the VO component here.
 * 
 * // TODO: WRITE YOUR DESCRIPTION HERE! 
 *
 * @author
 * @version 
 ***/

#include <iostream>
#include <casa/aips.h>
#include <vo_cmpt.h>
#include <xmlcasa/record.h>
#include <vo/VOClient/VOClient.hpp>

using namespace std;
using namespace casa;

namespace casac {

vo::vo()
{
  vao::dal::initVOClient("");
}

vo::~vo()
{
  vao::dal::closeVOClient(0);
}

std::vector<std::string>
vo::sesame(std::vector<std::string> targets)
{
  std::vector<std::string> rstat(0);
  for(int i=0; i<targets.size(); i++){
    Sesame sr = vao::sesame::nameResolver(const_cast<char *>(targets[i].c_str()));
    rstat.push_back(vao::sesame::resolverPos(sr));
    cerr << "Arg " << i << " " << rstat[i] << endl;
  }
  return rstat;
}
//
//
std::vector<std::string>
vo::data(std::vector<std::string> resources,
	 std::vector<std::string> object,
	 std::vector<std::string> position,
	 std::string searchradius,
	 std::vector<std::string> serviceURL)
{
  std::vector<std::string> rstat(0);
  char *opts(NULL);
  vao::dal::initVOClient(opts);
  if(object.size() > 0){
     for(int j=0;j<object.size();j++){
	Sesame srch = vao::sesame::nameResolver(const_cast<char *>(object[j].c_str()));
	double ra = vao::sesame::resolverRA(srch);
	double dec = vao::sesame::resolverDEC(srch);
	double sr = 0.5;
        for(int i=0;i<serviceURL.size(); i++){
           vao::dal::openConeConnection(const_cast<char *>(serviceURL[i].c_str()));
	   rstat.push_back(vao::dal::coneCaller(const_cast<char *>(serviceURL[i].c_str()), ra, dec, sr, VOC_VOTABLE));
        }
     }
  }
  cerr << "Your search result here" << endl;
  return rstat;
}

//
//
//

casac::record
vo::registry(string keywords,
	     string service){
	casac::record rstat;
  stringstream sql;
  if(service == string("CONE")){
	  sql << "cone" << ends;
	  // sql << "ResourceType like '%cone%'" << ends;
  }else if(service == string("SIAP")){
	  sql << "siap" << ends;
	  //sql << "ResourceType like '%siap%'" << ends;
  }else if(service == string("TABULAR")){
	  sql << "tabular" << ends;
	  //sql << "ResourceType like '%tabular%'" << ends;
  }
  cerr << sql.str() << endl;
  int orValues(0);
  RegResult res = vao::registry::regSearch(const_cast<char *>(keywords.c_str()), const_cast<char *>(sql.str().c_str()), orValues);
  int nresources = vao::registry::resGetCount(res);
  cerr << "Number of reources: " << nresources << endl;
  string serviceURL("ServiceUrl");
  for(int i=0;i<nresources;i++){
	  char *title = vao::registry::resGetStr(res, "Title", i);
	  char *surl = vao::registry::resGetStr(res, const_cast<char*>(serviceURL.c_str()), i);
	  char *rtype = vao::registry::resGetStr(res, "ServiceType", i);
	  char *sname = vao::registry::resGetStr(res, "ShortName", i);
	  char *clev = vao::registry::resGetStr(res, "ContentLevel", i);
	  /*
	  cerr << endl << "-------------------------------" <<endl;
	  cerr << "(" << i << " of " << nresources << ")";
	  if(title)
	     cerr << "\t" << title << endl;
	  if(surl)
	     cerr << "Service URL: " << surl << endl;
	  if(sname)
	     cerr << "Short Name: " << sname;
	  if(rtype)
	     cerr << "\tService Type: " << rtype << endl;
	  cerr << "ContentLevel: " <<  (clev ? clev : "none provided") << endl;
	  */
	  record subrec;
	  if(title)subrec.insert("title", string(title));
	  if(surl)subrec.insert("surl", string(surl));
	  if(rtype)subrec.insert("rtype", string(rtype));
	  if(sname)subrec.insert("sname", string(sname));
	  if(clev)subrec.insert("clev", string(clev));
	  ostringstream oss;
	  oss << i << ends;
	  rstat.insert(oss.str(), subrec);
	  if(title)delete title;
	  if(surl)delete surl;
	  if(rtype)delete rtype;
	  if(sname)delete sname;
	  if(clev)delete clev;
  }
  return rstat;
}

} // casac namespace

