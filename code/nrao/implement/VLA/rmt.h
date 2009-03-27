/*
 * $Header$
 *
 * $Log$
 * Revision 19.1  2004/11/30 17:50:43  ddebonis
 * Adding namespace
 *
 * Revision 19.0  2003/07/16 03:39:58  aips2adm
 * exhale: Base release 19.000.00
 *
 * Revision 18.0  2002/06/07 19:44:34  aips2adm
 * exhale: Base release 18.000.00
 *
 * Revision 17.0  2001/11/12 18:30:58  aips2adm
 * exhale: Base release 17.000.00
 *
 * Revision 16.0  2001/05/02 22:55:40  aips2adm
 * exhale: Base release 16.000.00
 *
 * Revision 15.0  2000/10/26 14:06:14  aips2adm
 * exhale: Base release 15.000.00
 *
 * Revision 14.0  2000/03/23 14:32:29  aips2adm
 * exhale: Base release 14.000.00
 *
 * Revision 13.0  1999/08/10 17:02:04  aips2adm
 * exhale: Base release 13.000.00
 *
 * Revision 12.0  1999/07/14 21:45:36  aips2adm
 * exhale: Base release 12.000.00
 *
 * Revision 11.0  1998/10/03 04:33:40  aips2adm
 * exhale: Base release 11.000.00
 *
 * Revision 10.0  1998/07/20 15:12:32  aips2adm
 * exhale: Base release 10.000.00
 *
 * Revision 9.0  1997/08/25 18:25:43  aips2adm
 * exhale: Base release 09.000.00
 *
 * Revision 8.1  1997/07/23 20:03:47  wyoung
 * Initial check in
 *
 * Revision 1.1  86/10/09  16:17:20  root
 * Initial revision
 * 
 */

/*
 *	rmt.h
 *
 *	Added routines to replace open(), close(), lseek(), ioctl(), etc.
 *	The preprocessor can be used to remap these the rmtopen(), etc
 *	thus minimizing source changes.
 *
 *	This file must be included before <sys/stat.h>, since it redefines
 *	stat to be rmtstat, so that struct stat xyzzy; declarations work
 *	properly.
 *
 *	-- Fred Fish (w/some changes by Arnold Robbins)
 */
#include <casa/namespace.h>
#ifndef access		/* avoid multiple redefinition */
#ifndef lint		/* in this case what lint doesn't know won't hurt it */
#define access rmtaccess
#define close rmtclose
#define creat rmtcreat
#define dup rmtdup
#define fcntl rmtfcntl
#define fstat rmtfstat
#define ioctl rmtioctl
#define isatty rmtisatty
#define lseek rmtlseek
#define lstat rmtlstat
#define open rmtopen
#define read rmtread
#define stat rmtstat
#define write rmtwrite

extern long rmtlseek ();	/* all the rest are int's */
#endif
#endif






