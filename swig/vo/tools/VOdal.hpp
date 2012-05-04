
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

#ifndef _VODAL_HPP
#define _VODAL_HPP

#define _VOCLIENT_LIB_ 1

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


};

#endif /* _VODAL_HPP */
