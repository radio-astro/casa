/*
 * ScantableIterator.h
 *
 *  Created on: Jan 28, 2016
 *      Author: nakazato
 */

#ifndef SINGLEDISH_FILLER_SCANTABLEITERATOR_H_
#define SINGLEDISH_FILLER_SCANTABLEITERATOR_H_

#include <singledish/Filler/FillerUtil.h>
#include <singledish/Filler/FieldRecord.h>
#include <singledish/Filler/SourceRecord.h>
#include <singledish/Filler/SpectralWindowRecord.h>
#include <singledish/Filler/SysCalRecord.h>

#include <casacore/casa/Arrays/Vector.h>
#include <casacore/casa/Utilities/Compare.h>
#include <casacore/casa/Utilities/Sort.h>

#include <casacore/measures/Measures/MFrequency.h>

#include <casacore/tables/Tables/Table.h>
#include <casacore/tables/Tables/ScalarColumn.h>
#include <casacore/tables/Tables/ArrayColumn.h>
#include <casacore/tables/TaQL/ExprNode.h>

using namespace casacore;

namespace {
template<class T>
inline void getDataRangePerId(Vector<uInt> const &index_list,
    Vector<T> const &id_list, size_t start,
    size_t end, std::map<T, Block<uInt> > &range_index) {
  uInt id_prev = id_list[index_list[start]];
  uInt id_min = index_list[start];
  uInt id_max = 0;
  Block < uInt > current_index(2);
  for (uInt i = start + 1; i < end; ++i) {
    uInt j = index_list[i];
    uInt current_id = id_list[j];
//      std::cout << "weather_id = " << weather_id << " id_prev = " << id_prev
//          << " time range (" << time_min << "," << time_max << ")" << std::endl;
    if (current_id != id_prev) {
      id_max = index_list[i - 1];
      current_index[0] = id_min;
      current_index[1] = id_max;
      range_index[id_prev] = current_index;

      id_prev = current_id;
      id_min = j;
    }
  }
  uInt last_id = id_list[index_list[end - 1]];
  if (range_index.find(last_id) == range_index.end()) {
    id_max = index_list[end - 1];
    current_index[0] = id_min;
    current_index[1] = id_max;
    range_index[last_id] = current_index;
  }
}

}

namespace casa { //# NAMESPACE CASA - BEGIN

class ScantableIteratorInterface {
public:
  ScantableIteratorInterface(Table const &table) :
      current_iter_(0), main_table_(table), num_iter_(0) {
  }
  virtual ~ScantableIteratorInterface() {
  }
  void initialize(size_t num_iter) {
    num_iter_ = num_iter;
    current_iter_ = 0;
  }
  bool moreData() const {
    return current_iter_ < num_iter_;
  }
  void next() {
    ++current_iter_;
  }

protected:
  size_t current_iter_;
  Table const main_table_;

private:
  size_t num_iter_;
};

class ScantableFrequenciesIterator: public ScantableIteratorInterface {
public:
  typedef std::map<Int, Int> Product;
  ScantableFrequenciesIterator(Table const &table) :
      ScantableIteratorInterface(table) {
    TableRecord const &header = main_table_.keywordSet();
    sub_table_ = header.asTable("FREQUENCIES");
    //size_t nrow = sub_table_.nrow();
    ROScalarColumn < uInt > ifno_column(main_table_, "IFNO");
    Vector < uInt > ifno_list = ifno_column.getColumn();
    Sort sorter;
    sorter.sortKey(ifno_list.data(), TpUInt);
    Vector < uInt > index_vector;
    uInt n = sorter.sort(index_vector, ifno_list.nelements(),
        Sort::HeapSort | Sort::NoDuplicates);

    initialize(n);
    ifno_list_.resize(n);
    for (uInt i = 0; i < n; ++i) {
      ifno_list_[i] = ifno_list[index_vector[i]];
    }

    ROScalarColumn < uInt > freq_id_column(main_table_, "FREQ_ID");

    // attach columns
    id_column_.attach(sub_table_, "ID");
    refpix_column_.attach(sub_table_, "REFPIX");
    refval_column_.attach(sub_table_, "REFVAL");
    increment_column_.attach(sub_table_, "INCREMENT");
    id_list_ = id_column_.getColumn();
  }
  virtual ~ScantableFrequenciesIterator() {
  }

  void getEntry(sdfiller::SpectralWindowRecord &record) {
    size_t const irow = current_iter_;
//    std::cout << "getEntry for row " << irow << std::endl;
    Int spw_id = ifno_list_[irow];
    Table subtable = main_table_(main_table_.col("IFNO") == (uInt) spw_id, 1);
    ROScalarColumn < uInt > freq_id_column(subtable, "FREQ_ID");
    uInt freq_id = freq_id_column(0);
    Int jrow = -1;
    for (uInt i = 0; i < id_list_.size(); ++i) {
      if (id_list_[i] == freq_id) {
        jrow = (Int) i;
        break;
      }
    }
    ROArrayColumn<uChar> flag_column(subtable, "FLAGTRA");
    Int num_chan = flag_column.shape(0)[0];
    String freq_frame = sub_table_.keywordSet().asString("BASEFRAME");
    MFrequency::Types frame_type;
    Bool status = MFrequency::getType(frame_type, freq_frame);
    if (!status) {
      frame_type = MFrequency::N_Types;
    }
    Double refpix = refpix_column_(jrow);
    Double refval = refval_column_(jrow);
    Double increment = increment_column_(jrow);
//    std::cout << "spw " << spw_id << " nchan " << num_chan << " mfr "
//        << (Int) frame_type << " (" << freq_frame << ") ref " << refpix << ", "
//        << refval << ", " << increment << std::endl;
    record.spw_id = spw_id;
    record.num_chan = num_chan;
    record.meas_freq_ref = frame_type;
    record.refpix = refpix;
    record.refval = refval;
    record.increment = increment;

    // update product
    product_[spw_id] = num_chan;
  }
  virtual void getProduct(Product *p) {
    if (p) {
      for (auto iter = product_.begin(); iter != product_.end(); ++iter) {
        (*p)[iter->first] = iter->second;
      }
    }
  }

private:
  Table sub_table_;
  ScalarColumn<uInt> id_column_;
  ScalarColumn<Double> refpix_column_;
  ScalarColumn<Double> refval_column_;
  ScalarColumn<Double> increment_column_;
  Vector<uInt> ifno_list_;
  Vector<uInt> id_list_;
  Product product_;
};

class ScantableFieldIterator: public ScantableIteratorInterface {
public:
  typedef std::map<String, Int> Product;
  ScantableFieldIterator(Table const &table) :
      ScantableIteratorInterface(table), row_list_(), is_reserved_(), field_column_(
          main_table_, "FIELDNAME"), source_column_(main_table_, "SRCNAME"), time_column_(
          main_table_, "TIME"), direction_column_(main_table_, "SRCDIRECTION"), scanrate_column_(
          main_table_, "SCANRATE"), direction_storage_(2, 2, 0.0) {
    Vector < String > field_name_list = field_column_.getColumn();
    Sort sorter;
    sorter.sortKey(field_name_list.data(), TpString);
    uInt n = sorter.sort(row_list_, field_name_list.size(),
        Sort::QuickSort | Sort::NoDuplicates);
    is_reserved_.resize(n);
    is_reserved_ = False;
    initialize(n);
  }

  virtual ~ScantableFieldIterator() {
  }

  void getEntry(sdfiller::FieldRecord &record) {
    uInt const irow = row_list_[current_iter_];
    String field_name_with_id = field_column_(irow);
    auto pos = field_name_with_id.find("__");
    auto defaultFieldId =
        [&]() {
          Int my_field_id = 0;
          while (is_reserved_[my_field_id] && (uInt)my_field_id < is_reserved_.size()) {
            my_field_id++;
          }
          if ((uInt)my_field_id >= is_reserved_.size()) {
            throw AipsError("Internal inconsistency in FIELD_ID numbering");
          }
          is_reserved_[my_field_id] = True;
          return my_field_id;
        };
    if (pos != String::npos) {
      record.name = field_name_with_id.substr(0, pos);
      Int field_id = String::toInt(field_name_with_id.substr(pos + 2));
      if (field_id < 0) {
        record.field_id = defaultFieldId();
      } else if ((uInt) field_id >= is_reserved_.size()
          || !is_reserved_[field_id]) {
        record.field_id = field_id;
        is_reserved_[field_id] = True;
      } else {
        record.field_id = defaultFieldId();
      }
    } else {
      record.name = field_name_with_id;
      record.field_id = defaultFieldId();
    }
    record.time = time_column_(irow) * 86400.0;
    record.source_name = source_column_(irow);
    record.frame = MDirection::J2000;
    Matrix < Double
        > direction(direction_storage_(IPosition(2, 0, 0), IPosition(2, 1, 0)));
//    std::cout << "direction = " << direction << " (shape " << direction.shape()
//        << ")" << std::endl;
    direction_storage_.column(0) = direction_column_(irow);
    if (scanrate_column_.isDefined(irow)) {
      Vector < Double > scan_rate = scanrate_column_(irow);
      if (anyNE(scan_rate, 0.0)) {
        direction_storage_.column(1) = scan_rate;
        direction.reference(direction_storage_);
      }
    }
//    std::cout << "direction = " << direction << " (shape " << direction.shape()
//        << ")" << std::endl;
    record.direction = direction;

    // update product
    product_[field_name_with_id] = record.field_id;
  }
  virtual void getProduct(Product *p) {
    if (p) {
      for (auto iter = product_.begin(); iter != product_.end(); ++iter) {
        (*p)[iter->first] = iter->second;
      }
    }
  }

private:
  Vector<uInt> row_list_;
  Vector<Bool> is_reserved_;
  ROScalarColumn<String> field_column_;
  ROScalarColumn<String> source_column_;
  ROScalarColumn<Double> time_column_;
  ArrayColumn<Double> direction_column_;
  ArrayColumn<Double> scanrate_column_;
  Matrix<Double> direction_storage_;
  Product product_;
};

class ScantableSourceIterator: public ScantableIteratorInterface {
public:
  typedef void * Product;
  ScantableSourceIterator(Table const &table) :
      ScantableIteratorInterface(table), name_column_(main_table_, "SRCNAME"), direction_column_(
          main_table_, "SRCDIRECTION"), proper_motion_column_(main_table_,
          "SRCPROPERMOTION"), sysvel_column_(main_table_, "SRCVELOCITY"), molecule_id_column_(
          main_table_, "MOLECULE_ID"), ifno_column_(main_table_, "IFNO"), molecules_table_(), row_list_(), source_id_map_() {
    TableRecord const &header = main_table_.keywordSet();
    molecules_table_ = header.asTable("MOLECULES");
    restfrequency_column_.attach(molecules_table_, "RESTFREQUENCY");
    molecule_name_column_.attach(molecules_table_, "NAME");
    Vector < String > source_name_list = name_column_.getColumn();
    Vector < uInt > ifno_list = ifno_column_.getColumn();
    Sort sorter;
    sorter.sortKey(source_name_list.data(), TpString);
    Vector < uInt > unique_vector;
    uInt num_unique = sorter.sort(unique_vector, source_name_list.size(),
        Sort::QuickSort | Sort::NoDuplicates);
    for (uInt i = 0; i < num_unique; ++i) {
      source_id_map_[name_column_(unique_vector[i])] = (Int) i;
    }
    Sort sorter2;
    sorter2.sortKey(source_name_list.data(), TpString);
    sorter2.sortKey(ifno_list.data(), TpUInt);
    unique_vector.resize();
    num_unique = sorter2.sort(row_list_, source_name_list.size(),
        Sort::QuickSort | Sort::NoDuplicates);
//    for (uInt i = 0; i < num_unique; ++i) {
//      std::cout << i << ": SRCNAME \"" << name_column_(row_list_[i])
//          << "\" IFNO " << ifno_column_(row_list_[i]) << std::endl;
//    }
    initialize(num_unique);

    // generate molecule_id_map_
    ROScalarColumn < uInt > id_column(molecules_table_, "ID");
    Vector < uInt > molecule_id_list = id_column.getColumn();
    for (uInt i = 0; i < id_column.nrow(); ++i) {
      molecule_id_map_[molecule_id_list[i]] = i;
    }

    // generate sorted_index_
    uInt nrow_main = main_table_.nrow();
    ROScalarColumn < String > srcname_column(main_table_, "SRCNAME");
    ROScalarColumn < uInt > ifno_column(main_table_, "IFNO");
    ROScalarColumn < Double > time_column(main_table_, "TIME");
    ROScalarColumn < Double > &interval_column = time_column;
    Sort sorter3;
    Vector < String > srcname_list = srcname_column.getColumn();
    Vector < Double > time_list = time_column.getColumn();
    sorter3.sortKey(srcname_list.data(), TpString, 0, Sort::Ascending);
    sorter3.sortKey(ifno_list.data(), TpUInt, 0, Sort::Ascending);
    sorter3.sortKey(time_list.data(), TpDouble, 0, Sort::Ascending);
    Vector < uInt > index_list;
    sorter3.sort(index_list, nrow_main, Sort::QuickSort);

    std::vector<size_t> srcname_boundary;
    srcname_boundary.push_back(0u);
    String current = srcname_list[index_list[0]];
    for (uInt i = 1; i < nrow_main; ++i) {
      String name = srcname_list[index_list[i]];
      if (current != name) {
        srcname_boundary.push_back(i);
        current = name;
      }
    }
    srcname_boundary.push_back(nrow_main);

    constexpr double kDay2Sec = 86400.0;
    interval_column.attach(main_table_, "INTERVAL");
    time_range_.clear();
    for (size_t i = 0; i < srcname_boundary.size() - 1; ++i) {
      String name = srcname_list[srcname_boundary[i]];
      size_t start = srcname_boundary[i];
      size_t end = srcname_boundary[i + 1];
      std::map<uInt, Block<Double> > range;
      std::map<uInt, Block<uInt> > range_index;
      getDataRangePerId(index_list, ifno_list, start, end,
          range_index);
      for (auto iter = range_index.begin(); iter != range_index.end(); ++iter) {
        Block<uInt> idx = iter->second;
        Block<Double> time_range(2);
        time_range[0] = time_list[idx[0]] * kDay2Sec
            - 0.5 * interval_column(idx[0]);
        time_range[1] = time_list[idx[1]] * kDay2Sec
            + 0.5 * interval_column(idx[1]);
        range[iter->first] = time_range;
      }
      time_range_[name] = range;
    }

//    for (auto i = time_range_.begin(); i != time_range_.end(); ++i) {
//      std::cout << "SRCNAME \"" << i->first << "\": " << std::endl;
//      for (auto j = i->second.begin(); j != i->second.end(); ++j) {
//        std::cout << "    " << j->first << ": " << j->second[0] << " "
//            << j->second[1] << std::endl;
//      }
//    }
  }

  virtual ~ScantableSourceIterator() {
  }

  void getEntry(sdfiller::SourceRecord &record) {
    uInt const irow = row_list_[current_iter_];
    uInt const ifno = ifno_column_(irow);
    record.name = name_column_(irow);
    record.source_id = source_id_map_[record.name];
    record.spw_id = ifno;    //ifno_column_(irow);
    record.direction = MDirection(
        Quantum<Vector<Double> >(direction_column_(irow), "rad"),
        MDirection::J2000);
    record.proper_motion = proper_motion_column_(irow);
    uInt molecule_id = molecule_id_column_(irow);
    auto iter = molecule_id_map_.find(molecule_id);
    if (iter != molecule_id_map_.end()) {
      uInt jrow = iter->second;
      if (restfrequency_column_.isDefined(jrow)) {
        record.rest_frequency = restfrequency_column_(jrow);
      }
      if (molecule_name_column_.isDefined(jrow)) {
        record.transition = molecule_name_column_(jrow);
      }
    }
    // 2016/02/04 TN
    // comment out the following else block since if no ID is found in
    // molecule_id_map_ it indicates that there is no corresponding
    // entry in MOLECULES table for given MOLECULE_ID. Serch result is
    // always empty table.
//    else {
//      Table t = molecules_table_(molecules_table_.col("ID") == molecule_id, 1);
//      if (t.nrow() == 1) {
//        ArrayColumn<Double> rest_freq_column(t, "RESTFREQUENCY");
//        ArrayColumn<String> molecule_name_column(t, "NAME");
//        if (rest_freq_column.isDefined(0)) {
//          record.rest_frequency = rest_freq_column(0);
//        }
//        if (molecule_name_column.isDefined(0)) {
//          record.transition = molecule_name_column(0);
//        }
//      }
//    }
    Double sysvel = sysvel_column_(irow);
    record.num_lines = record.rest_frequency.size();
    record.sysvel = Vector < Double > (record.num_lines, sysvel);

//    Table t = main_table_(
//        main_table_.col("SRCNAME") == record.name
//            && main_table_.col("IFNO") == record.spw_id);
//    time_column_.attach(t, "TIME");
//    Vector < Double > time_list = time_column_.getColumn();
//    Sort sorter;
//    sorter.sortKey(time_list.data(), TpDouble);
//    Vector < uInt > index_vector;
//    uInt n = sorter.sort(index_vector, time_list.size());
//    interval_column_.attach(t, "INTERVAL");
//    constexpr double kDay2Sec = 86400.0;
//    Double time_min = time_list[index_vector[0]] * kDay2Sec
//        - 0.5 * interval_column_(index_vector[0]);
//    Double time_max = time_list[index_vector[n - 1]] * kDay2Sec
//        + 0.5 * interval_column_(index_vector[n - 1]);
    Block<Double> const &time_range = time_range_[record.name][ifno];
    Double time_min = time_range[0];
    Double time_max = time_range[1];
    record.time = 0.5 * (time_min + time_max);
    record.interval = (time_max - time_min);
  }
  virtual void getProduct(Product */*p*/) {

  }

private:
  ROScalarColumn<String> name_column_;
  ArrayColumn<Double> direction_column_;
  ArrayColumn<Double> proper_motion_column_;
  ROScalarColumn<Double> sysvel_column_;
  ROScalarColumn<uInt> molecule_id_column_;
  ROScalarColumn<uInt> ifno_column_;
  ROScalarColumn<Double> time_column_;
  ROScalarColumn<Double> interval_column_;
  ArrayColumn<Double> restfrequency_column_;
  ArrayColumn<String> molecule_name_column_;
  Table molecules_table_;
  Vector<uInt> row_list_;
  std::map<String, Int> source_id_map_;
  std::map<uInt, uInt> molecule_id_map_;
  std::map<String, std::map<uInt, Block<Double> > > time_range_;
};

} //# NAMESPACE CASA - END

#endif /* SINGLEDISH_FILLER_SCANTABLEITERATOR_H_ */
