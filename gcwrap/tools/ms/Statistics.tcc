//# Copyright (C) 2000,2001,2002
//# Associated Universities, Inc. Washington DC, USA.
//#
//# This library is free software; you can redistribute it and/or modify it
//# under the terms of the GNU Library General Public License as published by
//# the Free Software Foundation; either version 2 of the License, or (at your
//# option) any later version.
//#
//# This library is distributed in the hope that it will be useful, but WITHOUT
//# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
//# FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
//# License for more details.
//#
//# You should have received a copy of the GNU Library General Public License
//# along with this library; if not, write to the Free Software Foundation,
//# Inc., 675 Massachusetts Ave, Cambridge, MA 02139, USA.
//#
//# Correspondence concerning AIPS++ should be addressed as follows:
//#        Internet email: aips2-request@nrao.edu.
//#        Postal address: AIPS++ Project Office
//#                        National Radio Astronomy Observatory
//#                        520 Edgemont Road
//#                        Charlottesville, VA 22903-2475 USA


#include <tools/ms/Statistics.h>

#include <lattices/LatticeMath/LatticeStatistics.h>
#include <lattices/Lattices/SubLattice.h>

#include <ms/MeasurementSets/MeasurementSet.h>

#include <tables/Tables/ScalarColumn.h>

#include <cassert>

using namespace casa;

namespace casac {

static void
get_statistics_1d(casacore::Record &result, 
                  const std::string keyname,
                  const casacore::Vector<casacore::Float> data_float)
{   
    unsigned long number_of_values = data_float.nelements();
           
    casacore::ArrayLattice<casacore::Float> al(data_float);
    
    casacore::SubLattice<casacore::Float> sl(al);
    casacore::LatticeStatistics<casacore::Float> ls(sl);
    
    struct {
        casacore::LatticeStatsBase::StatisticsTypes type;
        std::string name;
        std::string descr;
    }
    stats_types[] = {
        {casacore::LatticeStatsBase::MIN   , "min",   "minimum"},
        {casacore::LatticeStatsBase::MAX   , "max",   "maximum"},
        {casacore::LatticeStatsBase::SUM   , "sum",   "sum of values"},
        {casacore::LatticeStatsBase::SUMSQ , "sumsq", "sum of squared values"},
        
        {casacore::LatticeStatsBase::MEAN  , "mean"  , "mean value"},
        {casacore::LatticeStatsBase::VARIANCE, "var" , "variance"},
        {casacore::LatticeStatsBase::SIGMA , "stddev", "standard deviation wrt mean"},
        {casacore::LatticeStatsBase::RMS   , "rms"   , "root mean square"},
        {casacore::LatticeStatsBase::MEDIAN, "median", "median value"},
        {casacore::LatticeStatsBase::MEDABSDEVMED, "medabsdevmed", "median absolute deviation wrt median"},
        {casacore::LatticeStatsBase::QUARTILE, "quartile", "first quartile"}
    };
    
    casacore::Record rec;
    
    for (unsigned i = 0 ; i < sizeof(stats_types) / sizeof(*stats_types); i++) {
        casacore::Array<casacore::Double> the_stats;
        ls.getStatistic(the_stats, stats_types[i].type);
      
        if (0) cout << stats_types[i].descr << " [" 
                    << stats_types[i].name << "]: " << the_stats(casacore::IPosition(1, 0)) << endl;
        rec.define(stats_types[i].name,
                   the_stats(casacore::IPosition(1, 0)));
    }
    
    rec.define("npts", (casacore::Double) number_of_values);

    result.defineRecord(keyname, rec);

    return;
}

/*
  Linearize array

  rotc:        N-dimensional table column
  t:           table
  column:      column name
  supported:   set to true if column type is supported

  Return: Linearized array.

*/
template <class T>
casacore::Vector<T>
reform_array(casacore::ROTableColumn &rotc,
             const casacore::Table &t,
             const std::string &column,
             bool &supported)
{
    casacore::Array <T> v;
    
    if (rotc.columnDesc().isScalar()) {
        supported = true;
        casacore::ROScalarColumn<T> ro_col(t, column);
        v = ro_col.getColumn().reform(casacore::IPosition(1, ro_col.getColumn().shape().product()));
    }
    else if (rotc.columnDesc().isArray()) {
        supported = true;
        casacore::ROArrayColumn<T> ro_col(t, column);
        v = ro_col.getColumn().reform(casacore::IPosition(1, ro_col.getColumn().shape().product()));
    }

    return v;
}

static
void
apply_flags(casacore::Vector<casacore::Float> &v,
            const casacore::Vector<casacore::Bool> flags)
{
    casacore::IPosition unflagged(1);
    casacore::IPosition indx(1);
    unflagged(0) = 0;

    for (unsigned i = 0; i < v.nelements(); i++) {
        indx(0) = i;
        if (!flags(indx)) {
            
            v(unflagged) = v(indx);
            unflagged(0) += 1;
        }
    }

    bool copy_values = true;
    v.resize(unflagged, copy_values);
}

template <class T>
casacore::Record
Statistics<T>::get_stats(const casacore::Vector<T> v,
                         const casacore::Vector<casacore::Bool> flags,
                         const std::string &column,
                         bool &supported)
{
    if (v.shape() != flags.shape()) {
      stringstream ss;
      ss << "Incompatible array vs. flags shapes: " << v.shape() << " vs. " << flags.shape();
      throw casacore::AipsError(ss.str());
    }

    casacore::Record result;
    if (supported) {

        casacore::Vector<casacore::Float> data_float(v.shape());

        casacore::IPosition indx(1);

        for (unsigned i = 0; i < (unsigned)v.shape()(0); i++) {
            indx(0) = i;
            data_float(indx) = v(indx);
        }
        
        apply_flags(data_float, flags);
        if (data_float.shape() == 0) {
          stringstream ss;
          ss << "All selected rows are flagged.";
          throw casacore::AipsError(ss.str());
        }
        get_statistics_1d(result, column, data_float);
    }

    return result;
}

template <class T>
casacore::Record
Statistics<T>::get_stats(const casacore::Vector<T> v,
          const std::string &column,
          bool &supported)
{
  return get_stats(v, 
                   casacore::Vector<casacore::Bool>(v.nelements(), false),
                   column, 
                   supported);
}


template<class T>
casacore::Record
Statistics<T>::get_stats_complex(const casacore::Vector<casacore::Complex> v,
                                 const casacore::Vector<casacore::Bool> flags,
                                 const std::string &column,
                                 bool &supported,
                                 const std::string complex_value)
{
    if (complex_value != "amp" && complex_value != "amplitude" &&
        complex_value != "phase" && complex_value != "imag" &&
        complex_value != "real" && complex_value != "imaginary") {
      throw casacore::AipsError("complex_value must be amp, amplitude, phase, imag, imaginary or real" +
                      std::string(", is ") + complex_value);
    }

    if (v.shape() != flags.shape()) {
      stringstream ss;
      ss << "Incompatible array vs flags shapes: " << v.shape() << " vs " << flags.shape();
      throw casacore::AipsError(ss.str());
    }
    
    casacore::Record result;
    
    if (supported) {

      casacore::Vector<casacore::Float> data_float(v.shape());

      casacore::IPosition indx(1);

      switch (complex_value[0]) {
      case 'a':
        for (unsigned i = 0; i < (unsigned)v.shape()(0); i++) {
          indx(0) = i;
          data_float(indx) = abs(v(indx));
        }       
        break;
      case 'p':
        for (unsigned i = 0; i < (unsigned)v.shape()(0); i++) {
          indx(0) = i;
          data_float(indx) = arg(v(indx));
        }
        break;
      case 'i':
        for (unsigned i = 0; i < (unsigned)v.shape()(0); i++) {
          indx(0) = i;
          data_float(indx) = v(indx).imag();
        }
        break;
      case 'r':
        for (unsigned i = 0; i < (unsigned)v.shape()(0); i++) {
          indx(0) = i;
          data_float(indx) = v(indx).real();
        }
        break;
      default:
        assert(false);
      }
      
      apply_flags(data_float, flags);
      if (data_float.shape() == 0) {
        stringstream ss;
        ss << "All selected rows are flagged.";
        throw casacore::AipsError(ss.str());
      }
      get_statistics_1d(result, column, data_float);
    }
    return result;
    
}

template<class T>
casacore::Record
Statistics<T>::get_stats_complex(const casacore::Vector<casacore::Complex> v,
                                 const std::string &column,
                                 bool &supported,
                                 const std::string complex_value)
{
  return get_stats_complex(v, 
                           casacore::Vector<casacore::Bool>(v.nelements(), false),
                           column,
                           supported,
                           complex_value);
}     


template <class T>
static void
get_stats_array_table(const casacore::Table &t, 
                      casacore::Record &result,
                      const std::string &column,
                      bool &supported)
{
    supported = true;
    
    casacore::ROArrayColumn<T> ro_col(t, column);
    
    casacore::Matrix<T> v = ro_col.getColumn();

    result = Statistics<T>::get_stats_array(v,
                                            casacore::Vector<casacore::Bool>(v.shape()(1), false),
                                            column,
                                            supported);

    return;
}

template <class T>
casacore::Record
Statistics<T>::get_stats_array(const casacore::Matrix<T> v,
                               const casacore::Vector<casacore::Bool> flags,
                               const std::string &column,
                               bool &)
{
    casacore::Record result;

    if (v.shape()(1) != flags.shape()(0)) {
      stringstream ss;
      ss << "Incompatible number of values (" << v.shape()(1) <<
        ") and flags (" << flags.shape()(0) << ") given";
      throw casacore::AipsError(ss.str());
    }

    casacore::Vector<casacore::Float> data_float(casacore::IPosition(1, v.shape()(1)));

    casacore::IPosition indx(2);

    /* Compute statistics per column element */

    for (unsigned i = 0; i < (unsigned)v.shape()(0); i++) {
        indx(0) = i;
        for (unsigned j = 0; j < (unsigned)v.shape()(1); j++) {
            indx(1) = j;
            data_float[j] = v(indx);
        }

        std::stringstream s;
        s << column << "_" << i;
        
        apply_flags(data_float, flags);
        get_statistics_1d(result, s.str(), data_float);
    }

    return result;
}


/*
  Linearize array

  rotc:        N-dimensional table column
  sel_p:       measurement set
  column:      column name
  supported:   set to true if column type is supported

  Return: Linearized array.

*/
template <class T>
static casacore::Vector<T>
reform_array(casacore::ROTableColumn &rotc,
             casacore::MeasurementSet *sel_p,
             const std::string &column,
             bool &supported)
{
    casacore::Array <T> v;
    
    if (rotc.columnDesc().isScalar()) {
        supported = true;
        casacore::ROScalarColumn<T> ro_col(*sel_p, column);
        v = ro_col.getColumn().reform(casacore::IPosition(1, ro_col.getColumn().shape().product()));
    }
    else if (rotc.columnDesc().isArray()) {
        supported = true;
        casacore::ROArrayColumn<T> ro_col(*sel_p, column);
        v = ro_col.getColumn().reform(casacore::IPosition(1, ro_col.getColumn().shape().product()));
    }

    return v;
}


template <class T>
casacore::Record
Statistics<T>::get_statistics(const casacore::Table &table,
                              const std::string &column,
                              const std::string &complex_value,
                              casacore::LogIO *itsLog)
{
    casacore::ROTableColumn rotc(table, column);
    
    std::string type;
    
    if (rotc.columnDesc().ndim() > 0) {
        std::stringstream s;
        s << rotc.columnDesc().ndim();
        type = s.str() + "-dimensional ";
    }
            
    casacore::DataType dt1 = rotc.columnDesc().dataType();
    
    {
        ostringstream formatter;
        formatter << dt1;
        type += casacore::String(formatter);
    }
    
    if (rotc.columnDesc().isScalar()) {
        type += " scalar";
    }
    else if (rotc.columnDesc().isArray()) {
        type += " array";
    }
    else if (rotc.columnDesc().isTable()) {
        type += " table";
    }
    else {
        type += " unknown type";
    }
    
    *itsLog << "Compute statistics on " << type << " column " 
            << column;

    if (complex_value != "") {
        *itsLog << ", use " << complex_value;
    }
    *itsLog << "..." << casacore::LogIO::POST;

    casacore::Record result;

    /* Strategy depends on data type */
    
    bool supported = false; // Supported type?
    
    if (rotc.columnDesc().isArray() && rotc.columnDesc().ndim() == 1) {
      
        /* 1d is a special case: Loop over the array, 
           compute statistics for each index
        */
        if (dt1 == casacore::TpDouble) {
            get_stats_array_table<casacore::Double>(table, result, column, supported);
        }
        else if (dt1 == casacore::TpFloat) {
            get_stats_array_table<casacore::Float>(table, result, column, supported);
        }
    }
    else {
      /* Scalar or multi-dimensional array */

      if (dt1 == casacore::TpBool) {
        result = Statistics<casacore::Bool>::get_stats(reform_array<casacore::Bool>(rotc, table, column, supported),
                                 column, supported);
      }
      else if (dt1 == casacore::TpInt) {
        result = Statistics<casacore::Int>::get_stats(reform_array<casacore::Int>(rotc, table, column, supported),
                                column, supported);
      }
      else if (dt1 == casacore::TpFloat) {
        result = Statistics<casacore::Float>::get_stats(reform_array<casacore::Float>(rotc, table, column, supported),
                                  column, supported);
      }
      else if (dt1 == casacore::TpDouble) {
        result = Statistics<casacore::Double>::get_stats(reform_array<casacore::Double>(rotc, table, column, supported),
                                   column, supported);
      }
      else if (dt1 == casacore::TpComplex) {
        result = get_stats_complex(reform_array<casacore::Complex>(rotc, table, column, supported),
                                   column, supported, complex_value);
      }
    }

    if (supported) {
      return result;
    }
    else {
      std::string msg("Sorry, no support for " + type + " columns");
      throw casacore::AipsError(msg);
    }
}

} // namespace casac
