/**
   Bojan Nikolic <b.nikolic@mrao.cam.ac.uk>, <bojan@bnikolic.co.uk>
   Initial version January 2010.
   Maintained by ESO since 2013. Incorporating code originally in
   apps/almaabs_i.hpp.

   This file is part of LibAIR and is licensed under GNU Public
   License Version 2

   \file dtdltools.hpp

   Tools for computing the coefficients dT/dL

*/
#ifndef _LIBAIR_DTDLTOOLS_HPP__
#define _LIBAIR_DTDLTOOLS_HPP__

#include <list>
#include <vector>

#include <bnmin1/src/minim.hxx>
#include <bnmin1/src/minimmodel.hxx>


namespace LibAIR2 {

  class WVRAtmoQuants;
  class WVRAtmoQuantModel;

  /** Calculate the mean of dTdL coefficients from a weighted
      likelihood point list
      
      Suitable for use with outputs of the nested sampling algorithm
   */
  void dTdLMom1(const std::list<Minim::WPPoint> &l,
		Minim::ModelDesc &md,
		const WVRAtmoQuants &model,
		double Z,
		double thresh,
		double *res
		);

  /** Calculate the variance of dTdL coefficients from a weighted
      likelihood point list
      

   */
  void dTdLMom2(const std::list<Minim::WPPoint> &l,
		Minim::ModelDesc &md,
		const WVRAtmoQuants &model,
		const double *m1,
		double Z,
		double thresh,
		double *res
		);

  /** Calculate 2nd derivative of dT by DL
   */
  void dTdL2_ND(WVRAtmoQuantModel &m,
		std::vector<double> &res);

  /** Calculate 3rd derivative of dT by DL
   */
  void dTdL3_ND(WVRAtmoQuantModel &m,
		std::vector<double> &res);

  /** Calculate derivative of sky brightness wrt the brightness of
      model temperature
  */
  void dTdTAtm(WVRAtmoQuantModel &m,
	       std::vector<double> &res);
  


}

#endif
