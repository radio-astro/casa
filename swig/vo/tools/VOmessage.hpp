
/*
 *  VOMESSAGE.HPP == C++ Wrapper functions for VOClient Interface.
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

#ifndef _VOMESSAGE_HPP
#define _VOMESSAGE_HPP

#define _VOCLIENT_LIB_ 1

#include <VOClient.h>

namespace vao {



/* Prototype Declarations.
 */

/***************************************************************************
 *   VOCMSG.C Prototypes
 */

namespace message {

vocMsg_t *newCallMsg (ObjectID objid, char *method, int nparams){return msg_newCallMsg(objid, method, nparams);}
vocMsg_t *newResultMsg (int status, int type, int nitems){return msg_newResultMsg(status, type, nitems);}
vocMsg_t *newMsg (char *msgclass, char *str){return msg_newMsg(msgclass, str);}

vocMsg_t *shutdownMsg (){return msg_shutdownMsg();}
vocMsg_t *quitMsg (){return msg_quitMsg();}
vocMsg_t *ackMsg (){return msg_ackMsg();}

vocRes_t *sendMsg (int fd, vocMsg_t *msg){return msg_sendMsg(fd, msg);}
int       sendRawMsg (int fd, vocMsg_t *msg){return msg_sendRawMsg(fd, msg);}

vocRes_t *getResult (int fd){return msg_getResult(fd);}
vocRes_t *getResultToFile (int fd, char *fname, int overwrite){return msg_getResultToFile(fd, fname, overwrite);}

void      addIntParam (vocMsg_t *msg, int ival){return msg_addIntParam(msg, ival);}
void      addFloatParam (vocMsg_t *msg, double dval){return msg_addFloatParam(msg, dval);}
void      addStringParam (vocMsg_t *msg, char *str){return msg_addStringParam(msg, str);}
void      addIntResult (vocMsg_t *msg, int ival){return msg_addIntResult(msg, ival);}
void      addFloatResult (vocMsg_t *msg, double dval){return msg_addFloatResult(msg, dval);}
void      addStringResult (vocMsg_t *msg, char *str){return msg_addStringResult(msg, str);}

int       resultStatus (vocRes_t *res){return msg_resultStatus(res);}
int       resultType (vocRes_t *res){return msg_resultType(res);}
int       resultLength (vocRes_t *res){return msg_resultLength(res);}

int       getIntResult (vocRes_t *res, int index){return msg_getIntResult(res, index);}
double    getFloatResult (vocRes_t *res, int index){return msg_getFloatResult(res, index);}
char     *getStringResult (vocRes_t *res, int index){return msg_getStringResult(res, index);}
void     *getBuffer (vocRes_t *res){return msg_getBuffer(res);}
//char     *getFilename (vocRes_t *res){return msg_getFilename(res);}
};


};

#endif /* _VOMESSAGE_HPP */
