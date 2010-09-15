//
// C++ Interface: Scantable
//
// Description:
//
//
// Author: Malte Marquarding <asap@atnf.csiro.au>, (C) 2006
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef ASAPSCANTABLEWRAPPER_H
#define ASAPSCANTABLEWRAPPER_H

#include <vector>
#include <string>
#include <casa/Arrays/Vector.h>

#include "MathUtils.h"
#include "STFit.h"
#include "Scantable.h"
#include "STCoordinate.h"

namespace asap {
/**
  * This class contains and wraps a CountedPtr<Scantable>, as the CountedPtr
  * class does not provide the dor operator which is need for references
  * in boost::python
  * see Scantable for interfce description
  *
  * @brief The main ASAP data container wrapper
  * @author Malte Marquarding
  * @date 2006-02-23
  * @version 2.0a
*/
class ScantableWrapper {

public:
  explicit ScantableWrapper( const std::string& name,
                    int type=0)
  {
    casa::Table::TableType tp = casa::Table::Memory;
    if ( type == 1 ) tp = casa::Table::Plain;
    table_ = new Scantable(name, tp);
  }

  explicit ScantableWrapper(int type=0)
  {
    casa::Table::TableType tp = casa::Table::Memory;
    if ( type == 1) tp = casa::Table::Plain;
    table_= new Scantable(tp);
  }

  explicit ScantableWrapper(casa::CountedPtr<Scantable> cp) : table_(cp) {;}

  ScantableWrapper(const ScantableWrapper& mt) :
    table_(mt.getCP()) {;}

  void assign(const ScantableWrapper& mt)
    { table_= mt.getCP(); }

  ScantableWrapper copy() {
    return ScantableWrapper(new Scantable(*(this->getCP()), false));
  }

  std::vector<float> getSpectrum( int whichrow=0,
                                  const std::string& poltype="" ) const {
    return table_->getSpectrum(whichrow, poltype);
  }
  //  std::string getPolarizationLabel(bool linear, bool stokes, bool linPol, int polIdx) const {
  // Boost fails with 4 arguments.
  std::string getPolarizationLabel(int index, const std::string& ptype) const {
    return table_->getPolarizationLabel(index, ptype);
  }

  std::vector<double> getAbcissa(int whichrow=0) const
    { return table_->getAbcissa(whichrow); }

  std::string getAbcissaLabel(int whichrow=0) const
    { return table_->getAbcissaLabel(whichrow); }

  float getTsys(int whichrow=0) const
    { return table_->getTsys(whichrow); }

  std::string getTime(int whichrow=0) const
    { return table_->getTime(whichrow); }

  double getIntTime(int whichrow=0) const
    { return table_->getIntTime(whichrow); }

  std::string getDirectionString(int whichrow=0) const
    { return table_->getDirectionString(whichrow); }

  std::string getFluxUnit() const { return table_->getFluxUnit(); }

  void setFluxUnit(const std::string& unit) { table_->setFluxUnit(unit); }

  void setInstrument(const std::string& name) {table_->setInstrument(name);}
  void setFeedType(const std::string& ftype) {table_->setFeedType(ftype);}

  std::vector<bool> getMask(int whichrow=0) const
    { return table_->getMask(whichrow); }

  /**
  void flag(const std::vector<bool>& msk=std::vector<bool>())
    { table_->flag(msk); }
  **/
  void flag(const std::vector<bool>& msk=std::vector<bool>(), bool unflag=false)
    { table_->flag(msk, unflag); }

  void flagRow(const std::vector<casa::uInt>& rows=std::vector<casa::uInt>(), bool unflag=false)
    { table_->flagRow(rows, unflag); }

  bool getFlagRow(int whichrow=0) const
    { return table_->getFlagRow(whichrow); }

  void clip(const casa::Float uthres, const casa::Float dthres, bool clipoutside=true, bool unflag=false)
    { table_->clip(uthres, dthres, clipoutside, unflag); }

  std::vector<bool> getClipMask(int whichrow, const casa::Float uthres, const casa::Float dthres, bool clipoutside, bool unflag) const
    { return table_->getClipMask(whichrow, uthres, dthres, clipoutside, unflag); }

  std::string getSourceName(int whichrow=0) const
    { return table_->getSourceName(whichrow); }

  float getElevation(int whichrow=0) const
    { return table_->getElevation(whichrow); }

  float getAzimuth(int whichrow=0) const
    { return table_->getAzimuth(whichrow); }

  float getParAngle(int whichrow=0) const
    { return table_->getParAngle(whichrow); }


  void setSpectrum(std::vector<float> spectrum, int whichrow=0)
    { table_->setSpectrum(spectrum, whichrow); }

  std::vector<uint> getIFNos() { return table_->getIFNos(); }
  int getIF(int whichrow) const {return table_->getIF(whichrow);}
  std::vector<uint> getBeamNos() { return table_->getBeamNos(); }
  int getBeam(int whichrow) const {return table_->getBeam(whichrow);}
  std::vector<uint> getPolNos() { return table_->getPolNos(); }
  int getPol(int whichrow) const {return table_->getPol(whichrow);}
  int getCycle(int whichrow) const {return table_->getCycle(whichrow);}
  std::vector<uint> getScanNos() { return table_->getScanNos(); }
  int getScan(int whichrow) const {return table_->getScan(whichrow);}
  std::vector<uint> getMolNos() { return table_->getMolNos();}

  STSelector getSelection() const { return table_->getSelection(); }
  void setSelection(const STSelector& sts)
    { return table_->setSelection(sts);}

  std::string getPolType() const { return table_->getPolType(); }

  int nif(int scanno=-1) const {return table_->nif(scanno);}
  int nbeam(int scanno=-1) const {return table_->nbeam(scanno);}
  int npol(int scanno=-1) const {return table_->npol(scanno);}
  int nchan(int ifno=-1) const {return table_->nchan(ifno);}
  int nscan() const {return table_->nscan();}
  int nrow() const {return table_->nrow();}
  int ncycle(int scanno) const {return table_->ncycle(scanno);}

  void makePersistent(const std::string& fname)
    { table_->makePersistent(fname); }

  void setSourceType(int stype)
    { table_->setSourceType(stype); }

  void shift(int npix)
  { table_->shift(npix); }

/**
  commented out by TT
  void setRestFrequencies(double rf, const std::string& name,
                          const std::string& unit)
    { table_->setRestFrequencies(rf, name, unit); }
**/
  void setRestFrequencies(vector<double> rf, const vector<std::string>& name,
                          const std::string& unit)
    { table_->setRestFrequencies(rf, name, unit); }

/*
  void setRestFrequencies(const std::string& name) {
    table_->setRestFrequencies(name);
  }
*/

/*
  std::vector<double> getRestFrequencies() const
    { return table_->getRestFrequencies(); }
*/
  std::vector<double> getRestFrequency(int id) const
    { return table_->getRestFrequency(id); }

  void setCoordInfo(std::vector<string> theinfo) {
    table_->setCoordInfo(theinfo);
  }
  std::vector<string> getCoordInfo() const {
    return table_->getCoordInfo();
  }

  void setDirection(const std::string& refstr="")
    { table_->setDirectionRefString(refstr); }

  casa::CountedPtr<Scantable> getCP() const {return table_;}
  Scantable* getPtr() {return &(*table_);}

  std::string summary(bool verbose=false) const {
    return table_->summary(verbose);
  }

  std::vector<std::string> getHistory()const
    { return table_->getHistory(); }

  void addHistory(const std::string& hist)
    { table_->addHistory(hist); }

  void addFit(const STFitEntry& fit, int row)
    { table_->addFit(fit, row); }

  STFitEntry getFit(int whichrow) const
  { return table_->getFit(whichrow); }

  void calculateAZEL() { table_->calculateAZEL(); };

  std::vector<std::string> columnNames() const
    { return table_->columnNames(); }

  std::string getAntennaName() const
    { return table_->getAntennaName(); }

  int checkScanInfo(const vector<int>& scanlist) const
    { return table_->checkScanInfo(scanlist); }
 
  std::vector<double> getDirectionVector(int whichrow) const
    { return table_->getDirectionVector(whichrow); }

  void parallactify(bool flag)
    { table_->parallactify(flag); }

  STCoordinate getCoordinate(int row) {
    return STCoordinate(table_->getSpectralCoordinate(row));
  }

  std::vector<float> getWeather(int whichrow) const
    { return table_->getWeather(whichrow); }

  void reshapeSpectrum( int nmin, int nmax )
  { table_->reshapeSpectrum( nmin, nmax ); }

  void polyBaseline(const std::vector<bool>& mask, int order, int rowno, long pars_ptr, long pars_size, long errs_ptr, long errs_size, long fmask_ptr, long fmask_size)
  { table_->polyBaseline(mask, order, rowno, pars_ptr, pars_size, errs_ptr, errs_size, fmask_ptr, fmask_size); }

  void polyBaselineBatch(const std::vector<bool>& mask, int order, int rowno)
  { table_->polyBaselineBatch(mask, order, rowno); }

  bool getFlagtraFast(int whichrow=0) const
    { return table_->getFlagtraFast(whichrow); }


private:
  casa::CountedPtr<Scantable> table_;
};

} // namespace
#endif

