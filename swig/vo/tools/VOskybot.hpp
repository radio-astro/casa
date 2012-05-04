
/*
 *  VOCLIENT.HPP == C++ Wrapper functions for VOClient Interface.
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

#ifndef _VOSKYBOT_HPP
#define _VOSKYBOT_HPP

#define _VOCLIENT_LIB_ 1

#include <VOClient.h>

namespace vao {



/* Prototype Declarations.
 */

namespace skybot {

/*  SkyBoT interface procedures.
 */
Skybot      skybot (double ra, double dec, double rsz, double dsz, double epoch)
                   {return voc_skybot(ra, dec, rsz, dsz, epoch);}
int         skybotNObjs (Skybot sb){return voc_skybotNObjs(sb);}
char       *skybotStrAttr (Skybot sb, char *attr, int index){return voc_skybotStrAttr(sb, attr, index);}
double      skybotDblAttr (Skybot sb, char *attr, int index){return voc_skybotDblAttr(sb, attr, index);}

};


};

#endif /* _VOSKYBOT_HPP */
