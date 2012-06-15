//#---------------------------------------------------------------------------
//# SDMemTable.h: A MemoryTable container for single dish integrations
//#---------------------------------------------------------------------------
//# Copyright (C) 2004
//# ATNF
//#
//# This program is free software; you can redistribute it and/or modify it
//# under the terms of the GNU General Public License as published by the Free
//# Software Foundation; either version 2 of the License, or (at your option)
//# any later version.
//#
//# This program is distributed in the hope that it will be useful, but
//# WITHOUT ANY WARRANTY; without even the implied warranty of
//# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General
//# Public License for more details.
//#
//# You should have received a copy of the GNU General Public License along
//# with this program; if not, write to the Free Software Foundation, Inc.,
//# 675 Massachusetts Ave, Cambridge, MA 02139, USA.
//#
//# Correspondence concerning this software should be addressed as follows:
//#        Internet email: Malte.Marquarding@csiro.au
//#        Postal address: Malte Marquarding,
//#                        Australia Telescope National Facility,
//#                        P.O. Box 76,
//#                        Epping, NSW, 2121,
//#                        AUSTRALIA
//#
//# $Id:
//#---------------------------------------------------------------------------
#ifndef SDMEMTABLE_H
#define SDMEMTABLE_H

// STL
#include <string>
#include <vector>
// AIPS++
#include <casa/aips.h>
#include <casa/Arrays/MaskedArray.h>
#include <casa/BasicSL/String.h>
#include <coordinates/Coordinates/SpectralCoordinate.h>
#include <tables/Tables/Table.h>
#include <tables/Tables/ArrayColumn.h>
#include <tables/Tables/ScalarColumn.h>
//asap
#include <singledish/SDDefs.h>
#include <singledish/SDFitTable.h>
#include <singledish/SDLog.h>

namespace asap {

class SDContainer;
class SDHeader;
class SDFrequencyTable;
class SDFitTable;


class SDMemTable : private SDLog {
public:
  // create a new (empty) SDMemTable
  SDMemTable();
  // create a SDMemTable from an (aips++) table on disk
  SDMemTable(const std::string& name);

  // Copy Construct a SDMemTable, if clear==True only header and
  // skeleton are copied, otherwise the whole table is copied.
  SDMemTable(const SDMemTable& other, casa::Bool clear=casa::False);

  // Copy Construct (copy semantics) a SDMemTable, give a scanid constraint
  // see also getScan()
  SDMemTable(const casa::Table& tab, const std::string& expr);

  // Assignment operator (copy semantics)
  SDMemTable &operator=(const SDMemTable& other);

  virtual ~SDMemTable();

  // put data from meta conatiner into the table
  bool putSDContainer(const SDContainer& sdc);
  bool putSDHeader(const SDHeader& sdh);
  bool putSDFreqTable(const SDFrequencyTable& sdft);

  //get the data wrapped up in a meta container

  SDContainer getSDContainer(casa::uInt whichRow=0) const;
  SDHeader getSDHeader() const;

  // Get SD Frequency table.
  SDFrequencyTable getSDFreqTable() const;

  // get spectrum,mask and tsys for the given row, at the selected
  // cursor - all as stl vectors
  virtual std::vector<float> getSpectrum(casa::Int whichRow=0) const;
  virtual std::vector<bool> getMask(casa::Int whichRow=0) const;

  // Get length of STokes spectrum. XX & YY -> I
  // Anything else the length stays the same.
  int nStokes() const;

  // Get specific Stokes at cursor location. One of either I,Q,U,V or
  // I,P,PA,V (doPol=True) (determined by the polSel cursor location
  // 0->3) If the latter, you can add a PA offset (degrees)
  virtual std::vector<float> getStokesSpectrum(casa::Int whichRow=0,
                                               casa::Bool doPol=casa::False) const;

  // Returns Label for polarization selected by polSel_ (or
  // over-ridden with arg. polIdx).  If doStokes is False, returns
  // label for raw correlations (linear or circular). If True returns
  // label for Stokes.  If doPol=False, I,Q,U,V else I,P,PA,V
  std::string getPolarizationLabel(casa::Bool linear, casa::Bool stokes,
                                   casa::Bool linPol,
                                   casa::Int polIdx=-1) const;

  // Convert Stokes to linear or circular polarizations (specified by
  // polIdx=0:3).  Presently, only conversion to RR or LL is
  // supported.
  virtual std::vector<float> stokesToPolSpectrum(casa::Int whichRow,
                                                 casa::Bool toLinear,
                                                 casa::Int polIdx) const;

  // Get all Stokes at the specified Beam/IF cursor location (ignoring
  // the internal cursor).  -1 means all on that axis.  Really, this function
  // should not be here, and the SDContainer should hold the stokes spectrum
  // as well.  However, because the Stokes column is virtual, and its shape
  // is not the same as nPol(), I have put it here for direct access as needed
  virtual casa::Array<casa::Float> getStokesSpectrum(casa::Int whichRow=0,
                                                     casa::Int iBeam=-1,
                                                     casa::Int iIF=-1) const;

  // Get Tsys at cursor location
  virtual casa::Float getTsys(casa::Int whichRow=0) const;

  // get all as aips++ Vectors
  virtual void getSpectrum(casa::Vector<casa::Float>& spectrum,
                           casa::Int whichRow=0) const;

  // Get rest frequencies
  std::vector<double> getRestFreqs() const;

  // get info for current row
  // if whichRow == -1 the Header time is given
  std::string getTime(casa::Int whichRow=0,
                      casa::Bool showDate=casa::False) const ;
  casa::MEpoch getEpoch(casa::Int whichRow=0) const;
  casa::MDirection getDirection(casa::Int whichRow=0,
                                casa::Bool refBeam=casa::False) const;

  std::string getSourceName(casa::Int whichRow=0) const;

  float getElevation(casa::Int whichRow=0) const;
  float getAzimuth(casa::Int whichRow=0) const;
  float getParAngle(casa::Int whichRow=0) const;

  double getInterval(casa::Int whichRow=0) const;

  virtual void setSpectrum(std::vector<float> spectrum, int whichRow=0);
  virtual void setCoordInfo(std::vector<string> theinfo);

  // Set RestFreqID.  source="" and IF=-1 means select all
  virtual casa::Bool setRestFreqs(const casa::Vector<casa::Double>& restFreqs,
                                  const casa::String& unit,
                                  const std::vector<std::string>& lines,
                                  const casa::String& source,
                                  casa::Int whichIF=-1);

  // List lines
  std::string spectralLines() const;

  // Get/Set flux unit
  std::string getFluxUnit() const;
  void setFluxUnit (const std::string& unit);

  // Set Instrument
  void setInstrument (const std::string& instrument);

  // set the current value
  virtual bool setIF(casa::Int whichIF=0);
  virtual bool setBeam(casa::Int whichBeam=0);
  virtual bool setPol(casa::Int whichPol=0);

  // REset cursor to 0
  virtual void resetCursor();

  // Hard flags the current spectrum, not reversible
  virtual void flag(int whichRow);

  // return the currently selected values
  virtual casa::Int getIF() const { return IFSel_; }
  virtual casa::Int getBeam() const { return beamSel_; }
  virtual casa::Int getPol() const { return polSel_; }

  // returns unit, conversion frame, doppler, base-frame
  virtual std::vector<std::string> getCoordInfo() const;

  // number of scans in table
  virtual casa::Int nScan() const;

  // get a summary of the table
  virtual std::string summary(bool verbose=false) const;

  // get/set the history
  std::vector<std::string> getHistory() const;
  void addHistory(const std::string& hist);

  casa::Table getHistoryTable() const;
  void appendToHistoryTable(const casa::Table& otherHist);

  // write to disk as aips++ table
  void makePersistent(const std::string& filename);

  // get a new SDMemTable containing all rows with the same give SCANID
  SDMemTable getScan(casa::Int scanID) const;
  SDMemTable getSource(const std::string& source) const;

  const casa::TableRecord& getHeader() const {return table_.keywordSet();}
  // get a handle to the "raw" aips++ table
  const casa::Table& table() const { return table_; }

  // return the number of values
  casa::Int nBeam() const;
  casa::Int nIF() const;
  casa::Int nPol() const;
  casa::Int nChan() const;

  // return the number of rows (integrations) in the table
  casa::Int nRow() const { return table_.nrow(); }

  // return a row as a Masked array, internally converting uChar flags
  // to bool mask
  casa::MaskedArray<casa::Float> rowAsMaskedArray(casa::uInt whichRow,
                                                  casa::Bool toStokes=casa::False) const;

  // Return SC, setting only the basic construction state (i.e.
  // no conversion or velocity or rest frequency state).
  // Specify the index of the FreqID you want
  casa::SpectralCoordinate getSpectralCoordinate(casa::uInt whichIdx) const;

  // Return SC. Set velocity conversion state (unit,doppler), and
  // rest frequency.  If row number given (>=0), also set
  // frame conversion layer (needs direction & time which require row)
  casa::SpectralCoordinate getSpectralCoordinate(casa::uInt freqID,
                                                 casa::uInt restFreqID,
                                                 casa::uInt row) const;

  // Set just the reference value, pixel and increment into the table
  // No other state is extracted.
  casa::Bool setCoordinate(const casa::SpectralCoordinate& speccord,
                           casa::uInt whichIdx);

  casa::Int nCoordinates() const;

  std::vector<double> getAbcissa(int whichRow=0) const;
  std::string getAbcissaString(casa::Int whichRow=0) const;

// Get global reference  types
  casa::MDirection::Types getDirectionReference() const;
  casa::MEpoch::Types getTimeReference() const;

// Get global antenna position
  casa::MPosition getAntennaPosition() const;

  bool putSDFitTable(const SDFitTable& sdft);
  SDFitTable getSDFitTable() const;
  SDFitTable getSDFitTable(casa::uInt whichRow) const;

  void addFit(casa::uInt whichRow,
              const casa::Vector<casa::Double>& p,
              const casa::Vector<casa::Bool>& m,
              const casa::Vector<casa::String>& f,
              const casa::Vector<casa::Int>& c);


  void calculateAZEL();

private:
  // utility func for nice printout
  casa::String formatSec(casa::Double x) const;
  casa::String formatDirection(const casa::MDirection& md) const;
  std::vector<float> getFloatSpectrum(const casa::Array<casa::Float>& arr) const;
  void setup();
  void attach();
  void renumber();

  // Generate start and end for shape and current cursor selection
  void getCursorSlice(casa::IPosition& start, casa::IPosition& end,
                      const casa::IPosition& shape) const;

  // the current cursor into the array
  casa::Int IFSel_,beamSel_,polSel_;
  // the underlying memory table
  casa::Table table_;
  // The current table version
  static const casa::uInt version_ = 1;

  // Cached Columns to avoid reconstructing them for each row get/put
  casa::ScalarColumn<casa::Double> timeCol_, integrCol_;
  casa::ScalarColumn<casa::Float> azCol_, elCol_, paraCol_;
  casa::ScalarColumn<casa::String> srcnCol_, fldnCol_, tcaltCol_;
  casa::ScalarColumn<casa::Int> scanCol_, rbeamCol_;
  casa::ArrayColumn<casa::Float> specCol_, tsCol_, tcalCol_;
  casa::ArrayColumn<casa::Double> dirCol_;
  casa::ArrayColumn<casa::uChar> flagsCol_;
  casa::ArrayColumn<casa::uInt> freqidCol_, restfreqidCol_;
  casa::ArrayColumn<casa::String> histCol_;
  casa::ArrayColumn<casa::Int> fitCol_;
  casa::ROArrayColumn<casa::Float> stokesCol_;
};

}// namespace
#endif
