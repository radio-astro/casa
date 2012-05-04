
/*
 *  VOREGISTRY.HPP == C++ Wrapper functions for VOClient Interface.
 *
 *  W. Young, NRAO 2012
 *
 * Note dal,,registry, sesame, skybot, message  functions are nested in vao namespace
 * i.e.
 * vao::dal::myfunction is the same as voc_myfunction
 * vao::message::myfunction is the same as msg_myfunction
 *
 * The use of namespaces is a more natural C++ way of doing things.
 */

#ifndef _VOREGISTRY_HPP
#define _VOREGISTRY_HPP

#define _VOCLIENT_LIB_ 1

#include <VOClient.h>

namespace vao {

/*  Registry Interface procedures.
 */

namespace registry {

RegResult   regSearch (char *term1,  char *term2, int orValues){return voc_regSearch(term1, term2, orValues);}
RegResult   regSearchByService (char *svc,  char *term, int orValues){return voc_regSearchByService(svc, term, orValues);}
RegQuery    regQuery (char *term, int orValues){return voc_regQuery(term, orValues);}
void	    regConstSvcType (RegQuery query, char *svc){voc_regConstSvcType(query, svc);}
void	    regConstWaveband (RegQuery query, char *bpass){voc_regConstWaveband(query, bpass);}
void	    regDALOnly (RegQuery query, int value){voc_regDALOnly(query, value);}
void	    regSortRes (RegQuery query, int value){voc_regSortRes(query, value);}
void	    regAddSearchTerm (RegQuery query, char *term, int orValue){voc_regAddSearchTerm(query, term, orValue);}
void	    regRemoveSearchTerm (RegQuery query, char *term){voc_regRemoveSearchTerm(query, term);}
int	    regGetSTCount (RegQuery query){return voc_regGetSTCount(query);}
char	   *regGetQueryString (RegQuery query){return voc_regGetQueryString(query);}
RegResult   regExecute (RegQuery query){return voc_regExecute(query);}
char	   *regExecuteRaw (RegQuery query){return voc_regExecuteRaw(query);}

int	    resGetCount (RegResult res){return voc_resGetCount(res);}
char	   *resGetStr (RegResult res, char *attribute, int index){return voc_resGetStr(res, attribute, index);}
double	    resGetFloat (RegResult res, char *attribute, int index){return voc_resGetFloat(res, attribute, index);}
int	    resGetInt (RegResult res, char *attribute, int index){return voc_resGetInt(res, attribute, index);}

};


};

#endif /* _VOREGISTRY_HPP */
