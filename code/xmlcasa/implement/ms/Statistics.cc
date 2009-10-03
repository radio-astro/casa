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


#include <xmlcasa/ms/Statistics.h>

#include <lattices/Lattices/LatticeStatistics.h>
#include <lattices/Lattices/SubLattice.h>

#include <ms/MeasurementSets/MeasurementSet.h>

#include <tables/Tables/ScalarColumn.h>

#include <cassert>

using namespace casa;

namespace casac {

static void
get_statistics_1d(Record &result, 
                  const std::string keyname,
                  const Vector<Float> data_float)
{   
    unsigned long number_of_values = data_float.nelements();
           
    ArrayLattice<Float> al(data_float);
    
    SubLattice<Float> sl(al);
    LatticeStatistics<Float> ls(sl);
    
    struct {
        LatticeStatsBase::StatisticsTypes type;
        std::string name;
        std::string descr;
    }
    stats_types[] = {
        {LatticeStatsBase::MIN   , "min",   "minimum"},
        {LatticeStatsBase::MAX   , "max",   "maximum"},
        {LatticeStatsBase::SUM   , "sum",   "sum of values"},
        {LatticeStatsBase::SUMSQ , "sumsq", "sum of squared values"},
        
        {LatticeStatsBase::MEAN  , "mean"  , "mean value"},
        {LatticeStatsBase::VARIANCE, "var" , "variance"},
        {LatticeStatsBase::SIGMA , "stddev", "standard deviation wrt mean"},
        {LatticeStatsBase::RMS   , "rms"   , "root mean square"},
        {LatticeStatsBase::MEDIAN, "median", "median value"},
        {LatticeStatsBase::MEDABSDEVMED, "medabsdevmed", "median absolute deviation wrt median"},
        {LatticeStatsBase::QUARTILE, "quartile", "first quartile"}
    };
    
    Record rec;
    
    for (unsigned i = 0 ; i < sizeof(stats_types) / sizeof(*stats_types); i++) {
        Array<Double> the_stats;
        ls.getStatistic(the_stats, stats_types[i].type);
      
        if (0) cout << stats_types[i].descr << " [" 
                    << stats_types[i].name << "]: " << the_stats(IPosition(1, 0)) << endl;
        rec.define(stats_types[i].name,
                   the_stats(IPosition(1, 0)));
    }
    
    rec.define("npts", (Double) number_of_values);

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
Vector<T>
reform_array(ROTableColumn &rotc,
                         const Table &t,
                         const std::string &column,
                         bool &supported)
{
    Array <T> v;
    
    if (rotc.columnDesc().isScalar()) {
        supported = true;
        ROScalarColumn<T> ro_col(t, column);
        v = ro_col.getColumn().reform(IPosition(1, ro_col.getColumn().shape().product()));
    }
    else if (rotc.columnDesc().isArray()) {
        supported = true;
        ROArrayColumn<T> ro_col(t, column);
        v = ro_col.getColumn().reform(IPosition(1, ro_col.getColumn().shape().product()));
    }

    return v;
}

/*
  Converts data to Float, computes statistics

  v:           data
  result:      output dictionary
  column:      column name
  supported:   set to true if column type is supported
*/
template <class T>
static void
get_stats(Vector<T> v,
          Record &result,
          const std::string &column,
          bool &supported)
{
    if (supported) {

        Vector<Float> data_float(v.shape());

        IPosition indx(1);
        for (unsigned i = 0; i < (unsigned)v.shape()(0); i++) {
            indx(0) = i;
            data_float(indx) = v(indx);
        }
        
        get_statistics_1d(result, column, data_float);
    }

    return;
}

/*
  As get_stats().

  The conversion from complex to Float depends on the parameter
  complex_value. Eg complex_value="imag" picks out the imaginary part.
 */
static void
get_stats_complex(Vector<Complex> v,
                  Record &result,
                  const std::string &column,
                  bool &supported,
                  const std::string complex_value)
{
    if (supported) {

      Vector<Float> data_float(v.shape());

      IPosition indx(1);

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
      
      get_statistics_1d(result, column, data_float);
    }

    return;

}

/*
  Converts data to Float, computes statistics

  t:           Table
  result:      output dictionary, which will contain
               N sets of statistical information, one for
               each index in the array
  column:      column whose values are N-dimensional arrays
  supported:   set to true if column type is supported
*/
template <class T>
static void
get_stats_array(const Table &t, 
                Record &result,
                const std::string &column,
                bool &supported)
{
    supported = true;
    
    ROArrayColumn<T> ro_col(t, column);
    
    Matrix<T> v = ro_col.getColumn();

    Vector<Float> data_float(IPosition(1, v.shape()(1)));

    IPosition indx(2);

    /* Compute statistics for each element in the array */

    for (unsigned i = 0; i < (unsigned)v.shape()(0); i++) {
        indx(0) = i;
        for (unsigned j = 0; j < (unsigned)v.shape()(1); j++) {
            indx(1) = j;
            data_float[j] = v(indx);
        }

        std::stringstream s;
        s << column << "_" << i;
        
        get_statistics_1d(result, s.str(), data_float);
    }

    return;
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
static Vector<T>
reform_array(ROTableColumn &rotc,
             MeasurementSet *sel_p,
             const std::string &column,
             bool &supported)
{
    Array <T> v;
    
    if (rotc.columnDesc().isScalar()) {
        supported = true;
        ROScalarColumn<T> ro_col(*sel_p, column);
        v = ro_col.getColumn().reform(IPosition(1, ro_col.getColumn().shape().product()));
    }
    else if (rotc.columnDesc().isArray()) {
        supported = true;
        ROArrayColumn<T> ro_col(*sel_p, column);
        v = ro_col.getColumn().reform(IPosition(1, ro_col.getColumn().shape().product()));
    }

    return v;
}




Record
Statistics::get_statistics(const Table &table,
                           const std::string &column,
                           const std::string &complex_value,
                           casa::LogIO *itsLog)
{
    ROTableColumn rotc(table, column);
    
    std::string type;
    
    if (rotc.columnDesc().ndim() > 0) {
        std::stringstream s;
        s << rotc.columnDesc().ndim();
        type = s.str() + "-dimensional ";
    }
            
    DataType dt1 = rotc.columnDesc().dataType();
    
    {
        ostringstream formatter;
        formatter << dt1;
        type += String(formatter);
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
    *itsLog << "..." << LogIO::POST;

    Record result;

    /* Strategy depends on data type */
    
    bool supported = false; // Supported type?
    
    if (rotc.columnDesc().isArray() && rotc.columnDesc().ndim() == 1) {
      
        /* 1d is a special case: Loop over the array, 
           compute statistics for each index
        */
        if (dt1 == TpDouble) {
            get_stats_array<Double>(table, result, column, supported);
        }
        else if (dt1 == TpFloat) {
                 get_stats_array<Float>(table, result, column, supported);
        }
    }
    else {
      /* Scalar or multi-dimensional array */

      if (dt1 == TpBool) {
        get_stats<Bool>(reform_array<Bool>(rotc, table, column, supported),
                        result, column, supported);
      }
      else if (dt1 == TpInt) {
        get_stats<Int>(reform_array<Int>(rotc, table, column, supported),
                               result, column, supported);
      }
      else if (dt1 == TpFloat) {
        get_stats<Float>(reform_array<Float>(rotc, table, column, supported),
                         result, column, supported);
      }
      else if (dt1 == TpDouble) {
       get_stats<Double>(reform_array<Double>(rotc, table, column, supported),
                         result, column, supported);
      }
      else if (dt1 == TpComplex) {
        if (complex_value != "amp" && complex_value != "amplitude" &&
            complex_value != "phase" && complex_value != "imag" &&
            complex_value != "real" && complex_value != "imaginary") {
          throw AipsError("complex_value must be amp, amplitude, phase, imag, imaginary or real" +
                          std::string(", is ") + complex_value);
        }

        get_stats_complex(reform_array<Complex>(rotc, table, column, supported),
                          result, column, supported, complex_value);
      }
    }

    if (supported) {
      return result;
    }
    else {
      std::string msg("Sorry, no support for " + type + " columns");
      throw AipsError(msg);
    }
}



} // namespace casac
