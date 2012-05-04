
/*
 *  VOSESAME.HPP == C++ Wrapper functions for VOClient Interface.
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

#ifndef _VOSESAME_HPP
#define _VOSESAME_HPP

#define _VOCLIENT_LIB_ 1

#include <VOClient.h>

namespace vao {



/* Prototype Declarations.
 */

namespace sesame {

/*  SESAME interface procedures.
 */
Sesame      nameResolver (char *target){return voc_nameResolver(target);}
char       *resolverPos (Sesame sr){return voc_resolverPos(sr);}
double      resolverRA (Sesame sr){return voc_resolverRA(sr);}
double      resolverDEC (Sesame sr){return voc_resolverDEC(sr);}
double      resolverRAErr (Sesame sr){return voc_resolverRAErr(sr);}
double      resolverDECErr (Sesame sr){return voc_resolverDECErr(sr);}
char       *resolverOtype (Sesame sr){return voc_resolverOtype(sr);}
};

};

#endif /* _VOSESAME_HPP */
