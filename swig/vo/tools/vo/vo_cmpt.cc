
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
#include <stdcasa/record.h>
#include <tools/VOClient.hpp>

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
vo::sesame(const std::vector<std::string> &targets)
{
  std::vector<std::string> rstat(0);
  
  for(int i=0; i<targets.size(); i++){
    Sesame sr = vao::sesame::nameResolver(const_cast<char *>(targets[i].c_str()));
    rstat.push_back(string(vao::sesame::resolverPos(sr)));
    cerr << "Arg " << i << " " << rstat[i] << endl;
  }
  cerr << rstat.size() << endl;
  return (rstat);
}
//
//
std::vector<std::string>
vo::data(const std::vector<std::string> &resources,
	 const std::vector<std::string> &object,
	 const std::vector<std::string> &position,
	 const std::string &searchradius,
	 const std::vector<std::string> &serviceURL)
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
  return (rstat) ;
}

//
//
//

//casac::record
std::vector<casac::record>
vo::registry(const string &keywords,
	     const string &service){
	//casac::record rstat;
  std::vector<casac::record> rstat;
  stringstream sql;
  if(service == string("CONE")){
	  ////sql << "cone" << ends;
	  sql << "ResourceType like '%cone%'" << ends;
  }else if(service == string("SIAP")){
	  //sql << "siap" << ends;
	  sql << "ResourceType like '%siap%'" << ends;
  }else if(service == string("TABULAR")){
	  //sql << "tabular" << ends;
	  sql << "ResourceType like '%tabular%'" << ends;
  }
  cerr << sql.str() << endl;
  int orValues(0);
  RegResult res = vao::registry::regSearch(const_cast<char *>(sql.str().c_str()), const_cast<char *>(keywords.c_str()), orValues);
  int nresources = vao::registry::resGetCount(res);
  cerr << "Number of reources: " << nresources << endl;
  string serviceURL("ServiceUrl");
  for(int i=0;i<nresources;i++){
	  char *title = vao::registry::resGetStr(res, "Title", i);
	  //cerr << title << endl;
	  char *surl = vao::registry::resGetStr(res, "ServiceURL", i);
	  //cerr << surl << endl;
	  char *rtype = vao::registry::resGetStr(res, "ServiceType", i);
	  //cerr << rtype << endl;
	  char *sname = vao::registry::resGetStr(res, "ShortName", i);
	  //cerr << sname << endl;
	  char *clev = vao::registry::resGetStr(res, "ContentLevel", i);
	  record subrec;
	  if(title)subrec.insert("title", string(title));
	  if(surl)subrec.insert("surl", string(surl));
	  if(rtype)subrec.insert("rtype", string(rtype));
	  if(sname)subrec.insert("sname", string(sname));
	  if(clev)subrec.insert("clev", string(clev));
	  //ostringstream oss;
	  //oss << i << ends;
	  //cerr << i << " " << string(sname) << endl;
	  rstat.push_back(subrec);
	  if(title)delete title;
	  if(surl)delete surl;
	  if(rtype)delete rtype;
	  if(sname)delete sname;
	  if(clev)delete clev;
  }
  return rstat;
}

} // casac namespace

