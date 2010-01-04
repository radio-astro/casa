//
// C++ Interface: RowAccumulator
//
// Description:
//
//
// Author: Malte Marquarding <Malte.Marquarding@csiro.au>, (C) 2005
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef ASAPROWACCUMULATOR_H
#define ASAPROWACCUMULATOR_H

#include <casa/aips.h>
#include <casa/Arrays/Vector.h>
#include <casa/Arrays/MaskedArray.h>
#include "STDefs.h"

namespace asap {
/**
  * This class accumulates spectra and weights and returns the averaged data
  * @brief Class for averaging of spectra
  * @author Malte Marquarding
  * @date $Date:$
  * @version
  */
class RowAccumulator {

public:

  /**
   * Constructor taking a weight type as defined in @ref STDefs
   */
  explicit RowAccumulator(WeightType wt = asap::NONE);

 ~RowAccumulator();

  /**
    * add a new "row" to the accumulator
    * @param v the spectrum
    * @param m the mask for the spectrum
    * @param tsys the Tsys corresponing to the spectrum
    * @param interval the intergration time
    * @param time the time of the observation
    */
  void add(const casa::Vector<casa::Float>& v,
           const casa::Vector<casa::Bool>& m,
           const casa::Vector<casa::Float>& tsys,
           casa::Double interval,
           casa::Double time);
  /**
    * Also set a user mask which get combined with the individual masks
    * from the spectra
    * @param m a boolean mask of teh same length as the spectrum
    */
  void setUserMask(const casa::Vector<casa::Bool>& m);
  /**
    * Get the spectrum. Applies the normalisation (averaging)
    * @return the spectrum vector
    */
  casa::Vector<casa::Float> getSpectrum() const;
  /**
    * Get the Tsys. Applies the normalisation (averaging)
    * @return the Tsys vector
    */
  casa::Vector<casa::Float> getTsys() const;
  /**
    * Get the spectrum's mask. Applies the normalisation (averaging)
    * @return the mask vector
    */
  casa::Vector<casa::Bool> getMask() const;
  /**
    * Get the total interval.
    * @return the integration time
    */
  casa::Double getInterval() const;
  /**
    * Get the time of the observation. Retrieves the "mean" time.
    * @return the integration time
    */
  casa::Double getTime() const;
  /**
    * Reset the acummulator to the state at construction.
    */
  void reset();
  /**
    * check the initialization state 
    */ 
  casa::Bool state() const;

private:
  void addSpectrum( const casa::Vector<casa::Float>& v,
                    const casa::Vector<casa::Bool>& m,
                    casa::Float weight);

  casa::Float addTsys(const casa::Vector<casa::Float>& v);
  casa::Float addInterval(casa::Double inter);
  void addTime(casa::Double t);

  WeightType weightType_;
  casa::Bool initialized_;
  //these are Vectors
  casa::MaskedArray<casa::Float> spectrum_;
  casa::MaskedArray<casa::Float> weightSum_;
  casa::MaskedArray<casa::uInt> n_;

  casa::Vector<casa::Bool> userMask_;

  casa::Vector<casa::Float> tsysSum_;
  casa::Double timeSum_;
  casa::Double intervalSum_;
};

}
#endif
