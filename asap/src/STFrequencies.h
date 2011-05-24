//
// C++ Interface: STFrequencies
//
// Description:
//
//
// Author: Malte Marquarding <asap@atnf.csiro.au>, (C) 2006
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef ASAPSTFREQUENCIES_H
#define ASAPSTFREQUENCIES_H

#include <casa/aips.h>
#include <casa/BasicSL/String.h>
#include <coordinates/Coordinates/SpectralCoordinate.h>
#include <tables/Tables/Table.h>
#include <tables/Tables/ScalarColumn.h>

#include "STSubTable.h"

namespace asap {

/**
The Frequencies subtable of the Scantable

@author Malte Marquarding
@brief The frequency subtable of the Scantable
*/
class STFrequencies : public STSubTable {
public:
  STFrequencies() {;}
  explicit STFrequencies(casa::Table tab);
  explicit STFrequencies(const Scantable& parent);

  virtual ~STFrequencies();

  STFrequencies& operator=(const STFrequencies& other);

  /**
   * Add a new Entry to the Frequency subtable. This checks for duplicates.
   * @param[in] refpix the reference pixel
   * @param[in] refval the reference value
   * @param[in] inc the increment
   * @return an index into the frequency table
   */
  casa::uInt addEntry( casa::Double refpix, casa::Double refval,
                       casa::Double inc );

  /**
   * Retrieve the frequency values for a specific id via references
   * @param refpix the reference pixel
   * @param refval the reference value
   * @param inc the increment
   * @param id the identifier
   */
  void getEntry( casa::Double& refpix, casa::Double& refval,
                 casa::Double& inc, casa::uInt id );

  /***
   * Set the frequency values for a specific id via references
   * @param refpix the reference pixel
   * @param refval the reference value
   * @param inc    the increment
   * @param id     the identifier
   *
   * 17/09/2008 Takeshi Nakazato
   ***/
  void setEntry( casa::Double refpix, casa::Double refval, 
		 casa::Double inc, casa::uInt id ) ;


  bool conformant(const STFrequencies& other) const;

  /**
   * Retrieve  the frequency values as a casa::SpectralCoordinate
   * @param freqID
   * @return casa::SpectralCoordinate
   */
  casa::SpectralCoordinate getSpectralCoordinate( casa::uInt freqID ) const;

  /**
  casa::SpectralCoordinate getSpectralCoordinate( const casa::MDirection& md,
                                                  const casa::MPosition& mp,
                                                  const casa::MEpoch& me,
                                                  casa::Double restfreq,
                                                  casa::uInt freqID
                                                  ) const;
  **/
  casa::SpectralCoordinate getSpectralCoordinate( const casa::MDirection& md,
                                                  const casa::MPosition& mp,
                                                  const casa::MEpoch& me,
                                                  casa::Vector<casa::Double> restfreq,
                                                  casa::uInt freqID
                                                  ) const;

  /**
   * Return the unit of the frequency values
   * @return casa::Unit
   */
  casa::Unit getUnit() const;
  std::string getUnitString() const;

  /**
   * Return the doppler type of the values
   * @return casa::MDoppler::Types
   */
  casa::MDoppler::Types getDoppler() const;
  std::string getDopplerString() const;


  /**
   * Return the frame type, e.g MFrequency::TOPO
   * @param base return the base frame or the user frame
   * @return casa::MFrequency::Types
   */
  casa::MFrequency::Types getFrame(bool base=false) const;

  /**
   * Return a string representation of the frame type, e.g TOPO
   * @param base return the base frame or the user frame
   * @return the string representation of the frame
   */
  std::string getFrameString(bool base=false) const;

  /**
   * set the frequency frame from a string value
   * @param frame a string identifier
   */
  void setFrame(const std::string& frame, bool base=false);
  /**
   * set the frequency frame from a casa::MFrequency::Types
   * @param frame casa::MFrequency::Types
   */
  void setFrame(casa::MFrequency::Types frame, bool base=false);
  void setUnit( const std::string & unit );
  void setDoppler( const std::string & doppler );
  /**
   * rescale the whole table by a given factor
   * @param factor the factor to bin or resample by
   * @param mode the rescaling mode
   * @li "BIN"
   * @li "RESAMPLE"
   */
  void rescale(casa::Float factor, const std::string& mode);

  /**
   * get the reference frequency at a given channel for a specidif identifier
   * @param id the identifier
   * @param channel the channel number
   * @return teh reference frequency
   */
  float getRefFreq(casa::uInt id, casa::uInt channel);

  /**
    * shift the reference pixel by an integer amount
    * @param npix the shift in pixels
    * @param id the coordinate id
    */
  void shiftRefPix(int npix, casa::uInt id);
  /**
   * Return this table or s specific row as a string representation
   * @param id the identifier. If id<0 all rows are returned
   * @return a string
   */
  std::string print(int id=-1, casa::Bool strip=casa::False) const;

  std::vector<std::string> getInfo() const;
  void setInfo( const std::vector<std::string>& theinfo );

  const casa::String& name() const { return name_; }

private:

  /**
   * setup the the column structure of the casa::table
   */
  void setup();
  /**
   * the actual binning of the SpectralCoordinate as called by rescale
   * @param sc
   * @param factor the bin factor
   * @return casa::SpectralCoordinate
   */
  casa::SpectralCoordinate binCsys(const casa::SpectralCoordinate& sc, casa::Int factor);
  /**
   * the actual resampling of the SpectralCoordinate as called by rescale
   * @param sc
   * @param width the resacle width. Can be decimal.
   * @return
   */
  casa::SpectralCoordinate resampleCsys(const casa::SpectralCoordinate& sc, casa::Float width);

  static const casa::String name_;
  casa::ScalarColumn<casa::Double> refvalCol_, refpixCol_, incrCol_;
};

}

#endif
