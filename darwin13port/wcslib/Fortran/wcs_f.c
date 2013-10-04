/*============================================================================

  WCSLIB 4.7 - an implementation of the FITS WCS standard.
  Copyright (C) 1995-2011, Mark Calabretta

  This file is part of WCSLIB.

  WCSLIB is free software: you can redistribute it and/or modify it under the
  terms of the GNU Lesser General Public License as published by the Free
  Software Foundation, either version 3 of the License, or (at your option)
  any later version.

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
  $Id: wcs_f.c,v 4.7 2011/02/07 07:03:42 cal103 Exp $
*===========================================================================*/

#include <string.h>

#include <wcsutil.h>
#include <wcs.h>

/* Fortran name mangling. */
#include <wcsconfig_f77.h>
#define wcsnpv_  F77_FUNC(wcsnpv,  WCSNPV)
#define wcsnps_  F77_FUNC(wcsnps,  WCSNPS)
#define wcsini_  F77_FUNC(wcsini,  WCSINI)
#define wcssub_  F77_FUNC(wcssub,  WCSSUB)
#define wcscopy_ F77_FUNC(wcscopy, WCSCOPY)
#define wcsput_  F77_FUNC(wcsput,  WCSPUT)
#define wcsget_  F77_FUNC(wcsget,  WCSGET)
#define wcsfree_ F77_FUNC(wcsfree, WCSFREE)
#define wcsprt_  F77_FUNC(wcsprt,  WCSPRT)
#define wcsset_  F77_FUNC(wcsset,  WCSSET)
#define wcsp2s_  F77_FUNC(wcsp2s,  WCSP2S)
#define wcss2p_  F77_FUNC(wcss2p,  WCSS2P)
#define wcsmix_  F77_FUNC(wcsmix,  WCSMIX)
#define wcssptr_ F77_FUNC(wcssptr, WCSSPTR)

#define wcsptc_  F77_FUNC(wcsptc,  WCSPTC)
#define wcsptd_  F77_FUNC(wcsptd,  WCSPTD)
#define wcspti_  F77_FUNC(wcspti,  WCSPTI)
#define wcsgtc_  F77_FUNC(wcsgtc,  WCSGTC)
#define wcsgtd_  F77_FUNC(wcsgtd,  WCSGTD)
#define wcsgti_  F77_FUNC(wcsgti,  WCSGTI)

#define WCS_FLAG     100
#define WCS_NAXIS    101
#define WCS_CRPIX    102
#define WCS_PC       103
#define WCS_CDELT    104
#define WCS_CRVAL    105
#define WCS_CUNIT    106
#define WCS_CTYPE    107
#define WCS_LONPOLE  108
#define WCS_LATPOLE  109
#define WCS_RESTFRQ  110
#define WCS_RESTWAV  111
#define WCS_NPV      112
#define WCS_NPVMAX   113
#define WCS_PV       114
#define WCS_NPS      115
#define WCS_NPSMAX   116
#define WCS_PS       117
#define WCS_CD       118
#define WCS_CROTA    119
#define WCS_ALTLIN   120
#define WCS_VELREF   121

#define WCS_ALT      122
#define WCS_COLNUM   123
#define WCS_COLAX    124

#define WCS_CNAME    125
#define WCS_CRDER    126
#define WCS_CSYER    127
#define WCS_DATEAVG  128
#define WCS_DATEOBS  129
#define WCS_EQUINOX  130
#define WCS_MJDAVG   131
#define WCS_MJDOBS   132
#define WCS_OBSGEO   133
#define WCS_RADESYS  134
#define WCS_SPECSYS  135
#define WCS_SSYSOBS  136
#define WCS_VELOSYS  137
#define WCS_ZSOURCE  138
#define WCS_SSYSSRC  139
#define WCS_VELANGL  140
#define WCS_WCSNAME  141

#define WCS_NTAB     200
#define WCS_NWTB     201
#define WCS_TAB      202
#define WCS_WTB      203
#define WCS_TYPES    204
#define WCS_LNGTYP   205
#define WCS_LATTYP   206
#define WCS_LNG      207
#define WCS_LAT      208
#define WCS_SPEC     209
#define WCS_CUBEFACE 210
#define WCS_LIN      211
#define WCS_CEL      212
#define WCS_SPC      213

/*--------------------------------------------------------------------------*/

int wcsnpv_(int *npvmax) { return wcsnpv(*npvmax); }
int wcsnps_(int *npsmax) { return wcsnps(*npsmax); }

/*--------------------------------------------------------------------------*/

int wcsini_(const int *naxis, int *wcs)

{
  return wcsini(1, *naxis, (struct wcsprm *)wcs);
}

/*--------------------------------------------------------------------------*/

int wcssub_(const int *wcssrc, int *nsub, int axes[], int *wcsdst)

{
  return wcssub(1, (const struct wcsprm *)wcssrc, nsub, axes,
                (struct wcsprm *)wcsdst);
}

/*--------------------------------------------------------------------------*/

int wcscopy_(const int *wcssrc, int *wcsdst)

{
  return wcscopy(1, (const struct wcsprm *)wcssrc, (struct wcsprm *)wcsdst);
}

/*--------------------------------------------------------------------------*/

int wcsput_(
  int *wcs,
  const int *what,
  const void *value,
  const int *i,
  const int *j)

{
  int i0, j0, k;
  const char *cvalp;
  const int  *ivalp;
  const double *dvalp;
  struct wcsprm *wcsp;

  /* Cast pointers. */
  wcsp  = (struct wcsprm *)wcs;
  cvalp = (const char *)value;
  ivalp = (const int *)value;
  dvalp = (const double *)value;

  /* Convert 1-relative FITS axis numbers to 0-relative C array indices. */
  i0 = *i - 1;
  j0 = *j - 1;

  wcsp->flag = 0;

  switch (*what) {
  case WCS_FLAG:
    wcsp->flag = *ivalp;
    break;
  case WCS_NAXIS:
    wcsp->naxis = *ivalp;
    break;
  case WCS_CRPIX:
    wcsp->crpix[i0] = *dvalp;
    break;
  case WCS_PC:
    k = (i0)*(wcsp->naxis) + (j0);
    *(wcsp->pc+k) = *dvalp;
    break;
  case WCS_CDELT:
    wcsp->cdelt[i0] = *dvalp;
    break;
  case WCS_CUNIT:
    strncpy(wcsp->cunit[i0], cvalp, 72);
    wcsutil_null_fill(72, wcsp->cunit[i0]);
    break;
  case WCS_CTYPE:
    strncpy(wcsp->ctype[i0], cvalp, 72);
    wcsutil_null_fill(72, wcsp->ctype[i0]);
    break;
  case WCS_CRVAL:
    wcsp->crval[i0] = *dvalp;
    break;
  case WCS_LONPOLE:
    wcsp->lonpole = *dvalp;
    break;
  case WCS_LATPOLE:
    wcsp->latpole = *dvalp;
    break;
  case WCS_RESTFRQ:
    wcsp->restfrq = *dvalp;
    break;
  case WCS_RESTWAV:
    wcsp->restwav = *dvalp;
    break;
  case WCS_NPV:
  case WCS_NPVMAX:
    return 1;
    break;
  case WCS_PV:
    (wcsp->pv + wcsp->npv)->i = *i;
    (wcsp->pv + wcsp->npv)->m = *j;
    (wcsp->pv + wcsp->npv)->value = *dvalp;
    (wcsp->npv)++;
    break;
  case WCS_NPS:
  case WCS_NPSMAX:
    return 1;
    break;
  case WCS_PS:
    (wcsp->ps + wcsp->nps)->i = *i;
    (wcsp->ps + wcsp->nps)->m = *j;
    strncpy((wcsp->ps + wcsp->nps)->value, cvalp, 72);
    wcsutil_null_fill(72, (wcsp->ps + wcsp->nps)->value);
    (wcsp->nps)++;
    break;
  case WCS_ALTLIN:
    wcsp->altlin = *ivalp;
    break;
  case WCS_CD:
    k = (i0)*(wcsp->naxis) + (j0);
    *(wcsp->cd+k) = *dvalp;
    break;
  case WCS_CROTA:
    wcsp->crota[i0] = *dvalp;
    break;

  case WCS_ALT:
    wcsp->alt[0] = cvalp[0];
    wcsutil_null_fill(4, wcsp->alt);
    break;
  case WCS_COLNUM:
    wcsp->colnum = *ivalp;
    break;
  case WCS_COLAX:
    wcsp->colax[i0] = *ivalp;
    break;

  case WCS_CNAME:
    strncpy(wcsp->cname[i0], cvalp, 72);
    wcsutil_null_fill(72, wcsp->cname[i0]);
    break;
  case WCS_CRDER:
    wcsp->crder[i0] = *dvalp;
    break;
  case WCS_CSYER:
    wcsp->csyer[i0] = *dvalp;
    break;
  case WCS_DATEAVG:
    strncpy(wcsp->dateavg, cvalp, 72);
    wcsutil_null_fill(72, wcsp->dateavg);
    break;
  case WCS_DATEOBS:
    strncpy(wcsp->dateobs, cvalp, 72);
    wcsutil_null_fill(72, wcsp->dateobs);
    break;
  case WCS_EQUINOX:
    wcsp->equinox = *dvalp;
    break;
  case WCS_MJDAVG:
    wcsp->mjdavg = *dvalp;
    break;
  case WCS_MJDOBS:
    wcsp->mjdobs = *dvalp;
    break;
  case WCS_OBSGEO:
    wcsp->obsgeo[i0] = *dvalp;
    break;
  case WCS_RADESYS:
    strncpy(wcsp->radesys, cvalp, 72);
    wcsutil_null_fill(72, wcsp->radesys);
    break;
  case WCS_SPECSYS:
    strncpy(wcsp->specsys, cvalp, 72);
    wcsutil_null_fill(72, wcsp->specsys);
    break;
  case WCS_SSYSOBS:
    strncpy(wcsp->ssysobs, cvalp, 72);
    wcsutil_null_fill(72, wcsp->ssysobs);
    break;
  case WCS_SSYSSRC:
    strncpy(wcsp->ssyssrc, cvalp, 72);
    wcsutil_null_fill(72, wcsp->ssyssrc);
    break;
  case WCS_VELANGL:
    wcsp->velangl = *dvalp;
    break;
  case WCS_VELOSYS:
    wcsp->velosys = *dvalp;
    break;
  case WCS_WCSNAME:
    strncpy(wcsp->wcsname, cvalp, 72);
    wcsutil_null_fill(72, wcsp->wcsname);
    break;
  case WCS_ZSOURCE:
    wcsp->zsource = *dvalp;
    break;
  default:
    return 1;
  }

  return 0;
}

int wcsptc_(int *wcs, const int *what, const char *value, const int *i,
  const int *j)
{
  return wcsput_(wcs, what, value, i, j);
}

int wcsptd_(int *wcs, const int *what, const double *value, const int *i,
  const int *j)
{
  return wcsput_(wcs, what, value, i, j);
}

int wcspti_(int *wcs, const int *what, const int *value, const int *i,
  const int *j)
{
  return wcsput_(wcs, what, value, i, j);
}

/*--------------------------------------------------------------------------*/

int wcsget_(const int *wcs, const int *what, void *value)

{
  int i, j, k, naxis;
  char   *cvalp;
  int    *ivalp;
  double *dvalp;
  const int    *iwcsp;
  const double *dwcsp;
  const struct wcsprm *wcsp;

  /* Cast pointers. */
  wcsp  = (const struct wcsprm *)wcs;
  cvalp = (char *)value;
  ivalp = (int *)value;
  dvalp = (double *)value;

  naxis = wcsp->naxis;

  switch (*what) {
  case WCS_FLAG:
    *ivalp = wcsp->flag;
    break;
  case WCS_NAXIS:
    *ivalp = naxis;
    break;
  case WCS_CRPIX:
    for (i = 0; i < naxis; i++) {
      *(dvalp++) = wcsp->crpix[i];
    }
    break;
  case WCS_PC:
    /* C row-major to FORTRAN column-major. */
    for (j = 0; j < naxis; j++) {
      dwcsp = wcsp->pc + j;
      for (i = 0; i < naxis; i++) {
        *(dvalp++) = *dwcsp;
        dwcsp += naxis;
      }
    }
    break;
  case WCS_CDELT:
    for (i = 0; i < naxis; i++) {
      *(dvalp++) = wcsp->cdelt[i];
    }
    break;
  case WCS_CUNIT:
    for (i = 0; i < naxis; i++) {
      strncpy(cvalp, wcsp->cunit[i], 72);
      wcsutil_blank_fill(72, cvalp);
      cvalp += 72;
    }
    break;
  case WCS_CTYPE:
    for (i = 0; i < naxis; i++) {
      strncpy(cvalp, wcsp->ctype[i], 72);
      wcsutil_blank_fill(72, cvalp);
      cvalp += 72;
    }
    break;
  case WCS_CRVAL:
    for (i = 0; i < naxis; i++) {
      *(dvalp++) = wcsp->crval[i];
    }
    break;
  case WCS_LONPOLE:
    *dvalp = wcsp->lonpole;
    break;
  case WCS_LATPOLE:
    *dvalp = wcsp->latpole;
    break;
  case WCS_RESTFRQ:
    *dvalp = wcsp->restfrq;
    break;
  case WCS_RESTWAV:
    *dvalp = wcsp->restwav;
    break;
  case WCS_NPV:
    *ivalp = wcsp->npv;
    break;
  case WCS_NPVMAX:
    *ivalp = wcsp->npvmax;
    break;
  case WCS_PV:
    for (k = 0; k < wcsp->npv; k++) {
      *(dvalp++) = (wcsp->pv + k)->i;
      *(dvalp++) = (wcsp->pv + k)->m;
      *(dvalp++) = (wcsp->pv + k)->value;
    }
    break;
  case WCS_NPS:
    *ivalp = wcsp->nps;
    break;
  case WCS_NPSMAX:
    *ivalp = wcsp->npsmax;
    break;
  case WCS_PS:
    for (k = 0; k < wcsp->nps; k++) {
      *(dvalp++) = (wcsp->ps + k)->i;
      *(dvalp++) = (wcsp->ps + k)->m;
      cvalp += 2*sizeof(double);
      strncpy(cvalp, (wcsp->ps + k)->value, 72);
      wcsutil_blank_fill(72, cvalp);
      cvalp += 72;
    }
    break;
  case WCS_ALTLIN:
    *ivalp = wcsp->altlin;
    break;
  case WCS_CD:
    /* C row-major to FORTRAN column-major. */
    for (j = 0; j < naxis; j++) {
      dwcsp = wcsp->cd + j;
      for (i = 0; i < naxis; i++) {
        *(dvalp++) = *dwcsp;
        dwcsp += naxis;
      }
    }
    break;
  case WCS_CROTA:
    for (i = 0; i < naxis; i++) {
      *(dvalp++) = wcsp->crota[i];
    }
    break;

  case WCS_ALT:
    strncpy(cvalp, wcsp->alt, 4);
    wcsutil_blank_fill(4, cvalp);
    break;
  case WCS_COLNUM:
    *ivalp = wcsp->colnum;
    break;
  case WCS_COLAX:
    for (i = 0; i < naxis; i++) {
      *(ivalp++) = wcsp->colax[i];
    }
    break;

  case WCS_CNAME:
    for (i = 0; i < naxis; i++) {
      strncpy(cvalp, wcsp->cname[i], 72);
      wcsutil_blank_fill(72, cvalp);
      cvalp += 72;
    }
    break;
  case WCS_CRDER:
    for (i = 0; i < naxis; i++) {
      *(dvalp++) = wcsp->crder[i];
    }
    break;
  case WCS_CSYER:
    for (i = 0; i < naxis; i++) {
      *(dvalp++) = wcsp->csyer[i];
    }
    break;
  case WCS_DATEAVG:
    strncpy(cvalp, wcsp->dateavg, 72);
    wcsutil_blank_fill(72, cvalp);
    break;
  case WCS_DATEOBS:
    strncpy(cvalp, wcsp->dateobs, 72);
    wcsutil_blank_fill(72, cvalp);
    break;
  case WCS_EQUINOX:
    *dvalp = wcsp->equinox;
    break;
  case WCS_MJDAVG:
    *dvalp = wcsp->mjdavg;
    break;
  case WCS_MJDOBS:
    *dvalp = wcsp->mjdobs;
    break;
  case WCS_OBSGEO:
    for (i = 0; i < 3; i++) {
      *(dvalp++) = wcsp->obsgeo[i];
    }
    break;
  case WCS_RADESYS:
    strncpy(cvalp, wcsp->radesys, 72);
    wcsutil_blank_fill(72, cvalp);
    break;
  case WCS_SPECSYS:
    strncpy(cvalp, wcsp->specsys, 72);
    wcsutil_blank_fill(72, cvalp);
    break;
  case WCS_SSYSOBS:
    strncpy(cvalp, wcsp->ssysobs, 72);
    wcsutil_blank_fill(72, cvalp);
    break;
  case WCS_SSYSSRC:
    strncpy(cvalp, wcsp->ssyssrc, 72);
    wcsutil_blank_fill(72, cvalp);
    break;
  case WCS_VELANGL:
    *dvalp = wcsp->velangl;
    break;
  case WCS_VELOSYS:
    *dvalp = wcsp->velosys;
    break;
  case WCS_WCSNAME:
    strncpy(cvalp, wcsp->wcsname, 72);
    wcsutil_blank_fill(72, cvalp);
    break;
  case WCS_ZSOURCE:
    *dvalp = wcsp->zsource;
    break;

  case WCS_NWTB:
    *ivalp = wcsp->nwtb;
    break;
  case WCS_WTB:
    *(void **)value = wcsp->wtb;
    break;
  case WCS_NTAB:
    *ivalp = wcsp->ntab;
    break;
  case WCS_TAB:
    *(void **)value = wcsp->tab;
    break;
  case WCS_TYPES:
    for (i = 0; i < naxis; i++) {
      *(ivalp++) = wcsp->types[i];
    }
    break;
  case WCS_LNGTYP:
    strncpy(cvalp, wcsp->lngtyp, 4);
    wcsutil_blank_fill(4, cvalp);
    break;
  case WCS_LATTYP:
    strncpy(cvalp, wcsp->lattyp, 4);
    wcsutil_blank_fill(4, cvalp);
    break;
  case WCS_LNG:
    *ivalp = wcsp->lng + 1;
    break;
  case WCS_LAT:
    *ivalp = wcsp->lat + 1;
    break;
  case WCS_SPEC:
    *ivalp = wcsp->spec + 1;
    break;
  case WCS_CUBEFACE:
    *ivalp = wcsp->cubeface;
    break;
  case WCS_LIN:
    k = (int *)(&(wcsp->lin)) - (int *)wcsp;
    iwcsp = wcs + k;
    for (k = 0; k < LINLEN; k++) {
      *(ivalp++) = *(iwcsp++);
    }
    break;
  case WCS_CEL:
    k = (int *)(&(wcsp->cel)) - (int *)wcsp;
    iwcsp = wcs + k;
    for (k = 0; k < CELLEN; k++) {
      *(ivalp++) = *(iwcsp++);
    }
    break;
  case WCS_SPC:
    k = (int *)(&(wcsp->spc)) - (int *)wcsp;
    iwcsp = wcs + k;
    for (k = 0; k < SPCLEN; k++) {
      *(ivalp++) = *(iwcsp++);
    }
    break;
  default:
    return 1;
  }

  return 0;
}

int wcsgtc_(const int *wcs, const int *what, char *value)
{
  return wcsget_(wcs, what, value);
}

int wcsgtd_(const int *wcs, const int *what, double *value)
{
  return wcsget_(wcs, what, value);
}

int wcsgti_(const int *wcs, const int *what, int *value)
{
  return wcsget_(wcs, what, value);
}

/*--------------------------------------------------------------------------*/

int wcsfree_(int *wcs)

{
  return wcsfree((struct wcsprm *)wcs);
}

/*--------------------------------------------------------------------------*/

int wcsprt_(int *wcs)

{
  return wcsprt((struct wcsprm *)wcs);
}

/*--------------------------------------------------------------------------*/

int wcsset_(int *wcs)

{
  return wcsset((struct wcsprm *)wcs);
}

/*--------------------------------------------------------------------------*/

int wcsp2s_(
  int *wcs,
  const int *ncoord,
  const int *nelem,
  const double pixcrd[],
  double imgcrd[],
  double phi[],
  double theta[],
  double world[],
  int stat[])

{
  return wcsp2s((struct wcsprm *)wcs, *ncoord, *nelem, pixcrd, imgcrd, phi,
                theta, world, stat);
}

/*--------------------------------------------------------------------------*/

int wcss2p_(
  int* wcs,
  const int *ncoord,
  const int *nelem,
  const double world[],
  double phi[],
  double theta[],
  double imgcrd[],
  double pixcrd[],
  int stat[])

{
  return wcss2p((struct wcsprm *)wcs, *ncoord, *nelem, world, phi, theta,
                imgcrd, pixcrd, stat);
}

/*--------------------------------------------------------------------------*/

int wcsmix_(
  int *wcs,
  const int *mixpix,
  const int *mixcel,
  const double vspan[2],
  const double *vstep,
  int *viter,
  double world[],
  double phi[],
  double theta[],
  double imgcrd[],
  double pixcrd[])

{
  return wcsmix((struct wcsprm *)wcs, *mixpix-1, *mixcel, vspan, *vstep,
                *viter, world, phi, theta, imgcrd, pixcrd);
}

/*--------------------------------------------------------------------------*/

int wcssptr_(struct wcsprm *wcs, int *i, char ctype[9])

{
  int status = wcssptr(wcs, i, ctype);
  wcsutil_blank_fill(9, ctype);

  return status;
}
