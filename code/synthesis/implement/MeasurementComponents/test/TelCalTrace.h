#ifndef TELCAL_TRACE_H
#define TELCAL_TRACE_H
/*******************************************************************************
* ALMA - Atacama Large Millimiter Array
* (c) Institut de Radioastronomie Millimetrique, 2003 
*
*This library is free software; you can redistribute it and/or
*modify it under the terms of the GNU Lesser General Public
*License as published by the Free Software Foundation; either
*version 2.1 of the License, or (at your option) any later version.
*
*This library is distributed in the hope that it will be useful,
*but WITHOUT ANY WARRANTY; without even the implied warranty of
*MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
*Lesser General Public License for more details.
*
*You should have received a copy of the GNU Lesser General Public
*License along with this library; if not, write to the Free Software
*Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307  USA
*
* "@(#) $Id$"
*
* who       when      what
* --------  --------  ----------------------------------------------
* fanny   2003-05-14  created
*/

/************************************************************************
 * Header file for TelCalTrace
 *----------------------------------------------------------------------
 */

#ifndef __cplusplus
#error This is a C++ include file and cannot be used from plain C
#endif

/** @file TelCalTrace.h
 *  Header file for TelCalTrace.
 */

#define TELCAL_DEBUG 1

#if TELCAL_DEBUG==1
#define TELCAL_TRACE(str) \
            printf("TelCal trace : "); \
            printf(str);

#define TELCAL_TRACE1(str, param) \
            printf("TelCal trace : "); \
            printf(str, param);

#define TELCAL_TRACE2(str, param1, param2) \
            printf("TelCal trace : "); \
            printf(str, param1, param2);

#define TELCAL_TRACE3(str, param1, param2, param3) \
            printf("TelCal trace : "); \
            printf(str, param1, param2, param3);

#define TELCAL_TRACE4(str, param1, param2, param3, param4) \
            printf("TelCal trace : "); \
            printf(str, param1, param2, param3, param4);

#define TELCAL_TRACE5(str, param1, param2, param3, param4, param5) \
            printf("TelCal trace : "); \
            printf(str, param1, param2, param3, param4,param5);

#define TELCAL_COUT(str) \
            cout << "TelCal trace : "; \
            cout << str << endl;

#endif


#include <stdio.h>


#endif /*!TELCAL_TRACE_H*/
