/**
   Bojan Nikolic <bn204@mrao.cam.ac.uk>, <bojan@bnikolic.co.uk>
   Initial version February 2008
   Maintained by ESO since 2013.

   This file is part of LibAIR and is licensed under GNU Public
   License Version 2

   \file libair_main.hpp
   
   Main include file for libair
*/

/**
   \mainpage

   \section Preamble
   
   LibAIR is written by Bojan Nikolic and is licenced under the GPL V3
   or latter. The full licence terms are available in the file
   COPYING. 

   For help with installation see files INSTALL and README. 

   This library is designed for use either directly from C++, or from
   the Python interpreted language (see "pybind" directory).
   
   \section Structure 
   
   The library consists of following conceptual components:
   
   1) Model of sky brightness due to the 183.3 GHz water vapour line

   2) Model of what the ALMA water vapour radiometers measure, i.e.,
   how the sky brightness is translated to the WVR outputs

   3) A noise model for the WVRs allowing calculation of likelyhood
   that any given measurement is made given some model 

   \subsection sky_tb_mod Sky brightness modelling

   There are two sources of opacity in the models, Gross line and
   pseudo-continuum. They are the subclasses of the LibAIR2::Column
   class -- see documentation of those.

   A slice (LibAIR2::Slice) is a slab of the atmosphere that is
   considered to be at a single pressure and temperature.

   A slice result (LibAIR2::SliceResult) contains the brightness as
   function of frequency for a slice. The frequency points at which
   the brightness is computed are supplied by the user.

   The LibAIR2::SliceResult class supports radiative transfer by
   supplying the background slice to the UpdateI function.

   \subsection wvr_mod Modelling the WVRs

   \subsection likl Likelihood computation
*/

#ifndef _LIBAIR_MAIN_HPP__
#define _LIBAIR_MAIN_HPP__

/** The main LibAIR namespace */
namespace LibAIR2 {

  
  const char * version(void);


}

#endif   

