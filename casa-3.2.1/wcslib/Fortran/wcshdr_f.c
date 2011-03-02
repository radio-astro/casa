/*============================================================================

    WCSLIB 4.3 - an implementation of the FITS WCS standard.
    Copyright (C) 1995-2007, Mark Calabretta

    This file is part of WCSLIB.

    WCSLIB is free software: you can redistribute it and/or modify it under
    the terms of the GNU Lesser General Public License as published by the
    Free Software Foundation, either version 3 of the License, or (at your
    option) any later version.

    WCSLIB is distributed in the hope that it will be useful, but WITHOUT ANY
    WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
    FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for
    more details.

    You should have received a copy of the GNU Lesser General Public License
    along with WCSLIB.  If not, see <http://www.gnu.org/licenses/>.

    Correspondence concerning WCSLIB may be directed to:
       Internet email: mcalabre@atnf.csiro.au
       Postal address: Dr. Mark Calabretta
                       Australia Telescope National Facility, CSIRO
                       PO Box 76
                       Epping NSW 1710
                       AUSTRALIA

    Author: Mark Calabretta, Australia Telescope National Facility
    http://www.atnf.csiro.au/~mcalabre/index.html
    $Id: wcshdr_f.c,v 4.3 2007/12/27 05:48:07 cal103 Exp $
*===========================================================================*/

#include <wcshdr.h>
#include <wcs.h>

/* Fortran name mangling. */
#include <wcsconfig_f77.h>
#define wcspih_   F77_FUNC(wcspih,   WCSPIH)
#define wcstab_   F77_FUNC(wcstab,   WCSTAB)
#define wcsidx_   F77_FUNC(wcsidx,   WCSIDX)
#define wcsvcopy_ F77_FUNC(wcsvcopy, WCSVCOPY)
#define wcsvfree_ F77_FUNC(wcsvfree, WCSVFREE)

/*--------------------------------------------------------------------------*/

int wcspih_(
   char header[],
   const int *nkeys,
   const int *relax,
   const int *errlvl,
   int *nreject,
   int *nwcs,
   int *wcsp)

{
   return wcspih(header, *nkeys, *relax, *errlvl, nreject, nwcs,
      (struct wcsprm **)wcsp);
}

/*--------------------------------------------------------------------------*/

int wcstab_(int *wcs)

{
   return wcstab((struct wcsprm *)wcs);
}

/*--------------------------------------------------------------------------*/

int wcsidx_(int *nwcs, int *wcsp, int alts[27])

{
   return wcsidx(*nwcs, (struct wcsprm **)wcsp, alts);
}

/*--------------------------------------------------------------------------*/

int wcsvcopy_(const int *wcspp, const int *i, int *wcs)

{
   struct wcsprm *wcsdst, *wcssrc;

   /* Do a shallow copy. */
   wcssrc = *((struct wcsprm **)wcspp) + *i;
   wcsdst = (struct wcsprm *)wcs;
   *wcsdst = *wcssrc;

   /* Don't take memory. */
   wcsdst->m_flag   = 0;
   wcsdst->m_naxis  = 0;
   wcsdst->m_crpix  = 0x0;
   wcsdst->m_pc     = 0x0;
   wcsdst->m_cdelt  = 0x0;
   wcsdst->m_cunit  = 0x0;
   wcsdst->m_ctype  = 0x0;
   wcsdst->m_crval  = 0x0;
   wcsdst->m_pv     = 0x0;
   wcsdst->m_ps     = 0x0;
   wcsdst->m_cd     = 0x0;
   wcsdst->m_crota  = 0x0;
   wcsdst->m_cname  = 0x0;
   wcsdst->m_crder  = 0x0;
   wcsdst->m_csyer  = 0x0;
   wcsdst->m_wtb    = 0x0;
   wcsdst->m_tab    = 0x0;

   return 0;
}

/*--------------------------------------------------------------------------*/

int wcsvfree_(int *nwcs, int *wcspp)

{
   return wcsvfree(nwcs, (struct wcsprm **)wcspp);
}
