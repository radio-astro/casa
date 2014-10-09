/**
   Bojan Nikolic <b.nikolic@mrao.cam.ac.uk>, <bojan@bnikolic.co.uk>
   Initial version October 2011

   \file alma_datastruct.h

   Simple data structures for the ALMA application. This file should
   be "C"-langauge compatible
*/
#ifndef _LIBAIR_ALMA_DATASTRUCT_H__
#define _LIBAIR_ALMA_DATASTRUCT_H__

#ifdef __cplusplus
extern "C" {
#endif

  /** \brief Data inputs for coefficient retrieval from absolute
      temperatures
   */
  typedef struct {
    // Antenna number
    size_t antno;
    // Observed sky brightness
    //boost::array<double, 4> TObs;
    double TObs[4];
    // Elevation
    double el;
    // Time
    double time;
    // State ID
    size_t state;
    // Source ID of source observed during this measurement
    size_t source;
  } ALMAAbsInput;

  typedef struct {

    /** \brief The evidence value of the data
     */
    double ev;
    
    /** \brief The zenith water vapour column estimate in mm
     */
    double c;

    /** \brief The first-order error on the water vapour estimate
     */
    double c_err;

    /** \brief The phase correction coefficients
     */
    double dTdL[4];

    /** \brief The first-order error on phase correction coefficients
     */
    double dTdL_err[4];

  } ALMARes_Basic;



#ifdef __cplusplus
}
#endif


#endif
