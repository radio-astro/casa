
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

#ifndef _VOCLIENT_HPP
#define _VOCLIENT_HPP
#define _VOCLIENT_LIB_

#include <VOClient.h>

namespace vao {



/* Prototype Declarations.
 */

/*  DAL Interface procedures.
 */
namespace dal {
char       *coneCaller (char *url, double ra, double dec, double sr, int otype)
                       {return voc_coneCaller(url, ra, dec, sr, otype);}
int         coneCallerToFile (char *url, double ra, double dec, double sr, int otype, char *file)
                             {return voc_coneCallerToFile(url, ra, dec, sr, otype, file);}
//
char       *siapCaller (char *url, double ra, double dec, double rsize, double dsize, char *fmt, int otype)
	               {return voc_siapCaller(url, ra, dec, rsize, dsize, fmt, otype);}
int         siapCallerToFile (char *url, double ra, double dec, double rsize, double dsize, char *fmt, int otype, char *file)
	               {return voc_siapCallerToFile(url, ra, dec, rsize, dsize, fmt, otype, file);}
//
char       *ssapCaller (char *url, double ra, double dec, double size, char *band, char *time, char *fmt, int otype)
	               {return voc_ssapCaller(url, ra, dec, size, band, time, fmt, otype);}
int         ssapCallerToFile (char *url, double ra, double dec, double size, char *band, char *time, char *fmt, int otype, char *file)
	               {return voc_ssapCallerToFile(url, ra, dec, size, band, time, fmt, otype, file);}
//
char       *getRawURL (char *url, int *nbytes){return voc_getRawURL(url, nbytes);}
int         validateObject (int hcode){return voc_validateObject(hcode);}
void        freePointer (char *ptr){voc_freePointer(ptr);}


int 	    initVOClient (char *opts){return voc_initVOClient(opts);}
void	    closeVOClient (int shutdown){voc_closeVOClient(shutdown);}
void	    abortVOClient (int code, char *msg){voc_abortVOClient(code, msg);}

DAL         openConnection (char *service_url, int type){return voc_openConnection(service_url, type);}
DAL         openConeConnection (char *service_url){return voc_openConeConnection(service_url );}
DAL         openSiapConnection (char *service_url){return voc_openSiapConnection(service_url );}
DAL         openSsapConnection (char *service_url){return voc_openSsapConnection(service_url );}
void	    closeConnection (DAL dal){voc_closeConnection(dal);}

int	    getServiceCount (DAL dal){return voc_getServiceCount(dal);}
void	    addServiceURL (DAL dal, char *service_url){voc_addServiceURL(dal, service_url);}
char 	   *getServiceURL (DAL dal, int index){return voc_getServiceURL(dal, index);}

Query	    getQuery (DAL dal, int type){return voc_getQuery(dal, type);}
Query	    getConeQuery (DAL dal, double ra, double dec, double sr){return voc_getConeQuery(dal, ra, dec, sr);}
Query	    getSiapQuery (DAL dal, double ra, double dec, 
			double ra_size, double dec_size, char *format){return voc_getSiapQuery(dal, ra, dec, ra_size, dec_size, format);}
Query	    getSsapQuery (DAL dal, double ra, double dec, 
			double size, char *band, char *time, char *format){return voc_getSsapQuery(dal, ra, dec, size, band, time, format);}

int         addIntParam (Query query, char *name, int value){return voc_addIntParam(query, name, value);}
int         addFloatParam (Query query, char *name, double value){return voc_addFloatParam(query, name, value);}
int         addStringParam (Query query, char *name, char *value){return voc_addStringParam(query, name, value);}

char	   *getQueryString (Query query, int type, int index){return voc_getQueryString(query, type, index);}

QResponse   executeQuery (Query query){return voc_executeQuery(query);}
QResponse   getQueryResponse (Query query){return voc_getQueryResponse(query);}
char       *executeCSV (Query query){return voc_executeCSV(query);}
char       *executeTSV (Query query){return voc_executeTSV(query);}
char       *executeASCII (Query query){return voc_executeASCII(query);}
char       *executeVOTable (Query query){return voc_executeVOTable(query);}
int	    executeQueryAs (Query query, char *fname, int type){return voc_executeQueryAs(query, fname, type);}
int	    getRecordCount (QResponse qr){return voc_getRecordCount(qr);}

QRecord     getRecord (QResponse qr, int recnum){return voc_getRecord(qr, recnum);}
char       *getFieldAttr (QResponse qr, int fieldnum, char *attr){return voc_getFieldAttr(qr, fieldnum, attr);}
               
QRAttribute getAttribute (QRecord rec, char *attrname){return voc_getAttribute(rec, attrname);}

int         intValue (QRAttribute v){return voc_intValue(v);}
double      floatValue (QRAttribute v){return voc_floatValue(v);}
char       *stringValue (QRAttribute v){return voc_stringValue(v);}

int         getIntAttr (QRecord rec, char *attr_name){return voc_getIntAttr(rec, attr_name);}
double      getFloatAttr (QRecord rec, char *attr_name){return voc_getFloatAttr(rec, attr_name);}
char       *getStringAttr (QRecord rec, char *attr_name){return voc_getStringAttr(rec, attr_name);}

char       *getAttrList (QRecord rec){return voc_getAttrList(rec);}
int 	    getAttrCount (QRecord rec){return voc_getAttrCount(rec);}

void	    setIntAttr (QRecord rec, char *attrname, int ival){voc_setIntAttr(rec, attrname, ival);}
void	    setFloatAttr (QRecord rec, char *attrname, double dval){voc_setFloatAttr(rec, attrname, dval);}
void	    setStringAttr (QRecord rec, char *attrname, char *str){voc_setStringAttr(rec, attrname, str);}

int 	    getDataset (QRecord rec, char *acref, char *fname){return voc_getDataset(rec, acref, fname);}

	};


namespace registry {

/*  Registry Interface procedures.
 */

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



namespace skybot {

/*  SkyBoT interface procedures.
 */
Skybot      skybot (double ra, double dec, double rsz, double dsz, double epoch)
                   {return voc_skybot(ra, dec, rsz, dsz, epoch);}
int         skybotNObjs (Skybot sb){return voc_skybotNObjs(sb);}
char       *skybotStrAttr (Skybot sb, char *attr, int index){return voc_skybotStrAttr(sb, attr, index);}
double      skybotDblAttr (Skybot sb, char *attr, int index){return voc_skybotDblAttr(sb, attr, index);}

};

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

#endif /* _VOCLIENT_HPP */
