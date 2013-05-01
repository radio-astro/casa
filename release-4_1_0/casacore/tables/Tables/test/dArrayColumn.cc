//# dArrayColumn.cc: Demo program comparing the performance of
//# ROArrayColumn<T>::getColumn(const Vector<Vector<Slice> >& slices,
//#                             Array<T>& arr, Bool resize) to
//# ROArrayColumn<T>::getColumnCells(const RefRows& rownrs, Array<T>& arr,
//#                                  Bool resize)
//# when rownrs and arr match the size of the column.
//# 
//# Copyright (C) 2011
//# Associated Universities, Inc. Washington DC, USA.
//#
//# This program is free software; you can redistribute it and/or modify it
//# under the terms of the GNU General Public License as published by the Free
//# Software Foundation; either version 2 of the License, or (at your option)
//# any later version.
//#
//# This program is distributed in the hope that it will be useful, but WITHOUT
//# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
//# FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
//# more details.
//#
//# You should have received a copy of the GNU General Public License along
//# with this program; if not, write to the Free Software Foundation, Inc.,
//# 675 Massachusetts Ave, Cambridge, MA 02139, USA.
//#
//# Correspondence concerning AIPS++ should be addressed as follows:
//#        Internet email: aips2-request@nrao.edu.
//#        Postal address: AIPS++ Project Office
//#                        National Radio Astronomy Observatory
//#                        520 Edgemont Road
//#                        Charlottesville, VA 22903-2475 USA
//#
#include <tables/Tables/TableDesc.h>
#include <tables/Tables/SetupNewTab.h>
#include <tables/Tables/Table.h>
#include <tables/Tables/TiledShapeStMan.h>
#include <tables/Tables/ArrColDesc.h>
#include <tables/Tables/ArrayColumn.h>
#include <casa/Arrays/Vector.h>
#include <casa/Arrays/Cube.h>
#include <casa/Arrays/ArrayMath.h>
#include <casa/Arrays/ArrayLogical.h>
#include <casa/Arrays/Slice.h>
#include <casa/Exceptions/Error.h>
#include <casa/OS/Timer.h>

#include <cstdlib>

using namespace casa;

// Set true if running on a filesystem which can report read and write
// statistics (e.g. lustre).
const Bool LUSTRE = false;
const String lcache_clearer("/home/casa-dev-07/clear-cache");

// This could be grepped from lcache_clearer.
const String extents_stats("/proc/fs/lustre/llite/lustre-ffff81021c14e400/extents_stats");


// dimensions
const uInt nx = 4;
const uInt ny = 256;
const uInt nrowspertile = 1000;
const uInt ntiles = 100;
const uInt nrows = ntiles * nrowspertile;

const uInt nreps = 1;
const uInt ncols = 5;

const IPosition arrshp(2, nx, ny);

void startLustreStats()
{
  if(LUSTRE)
    system(("sudo " + lcache_clearer).c_str());
  else
  //   there are other ways to clear the cache, if you have permission...
    cerr << "The disk cache was not cleared, so the following times may be misleading."
         << endl;
}

void writeLustreStats(const String& output_fn)
{
  if(LUSTRE)
    system(("cat " + extents_stats + " | tee " + output_fn).c_str());
}

// Create the table.
void createTab()
{
  system("rm -rf dArrayColumn_tmp.tab");
  startLustreStats();
  Timer timer; timer.mark();

  // Build the table description.
  TableDesc td("", "1", TableDesc::Scratch);
  Vector<String> colnames(ncols);
  for(uInt cn = 0; cn < ncols; ++cn){
    colnames[cn] = "arr" + String(cn + '0');
    //cerr << "colnames[cn] = '"  << colnames[cn] << "'" << endl;
    td.addColumn(ArrayColumnDesc<Complex>(colnames[cn], arrshp,
					  ColumnDesc::FixedShape));
  }

  // Now create a new table from the description.
  SetupNewTable newtab("dArrayColumn_tmp.tab", td, Table::New);
  IPosition tileShape(3, nx, ny, nrowspertile);
  TiledShapeStMan tsm("TiledArr1", tileShape);
  newtab.bindColumn("arr1", tsm);
  Table tab(newtab, nrows, False, Table::LocalEndian);
  ArrayColumn<Complex> col;
  Array<Complex> arrf(arrshp);
  indgen(arrf);
  cerr << "Filling dArrayColumn_tmp.tab" << endl;
  for(uInt cn = 0; cn < ncols; ++cn){
    col.attach(tab, colnames[cn]);
    for(uInt i = 0; i < nrows; ++i){
      col.put(i, arrf);
      arrf += Complex(0.0, 1.0);	// pun intended
    }
  }
  cerr << "createTab() took " << timer.real() << "s." << endl;
  writeLustreStats("extent_stats.createTab");
  system("du -sh dArrayColumn_tmp.tab");
}

void reftabReader(Cube<Complex>& data, Complex& honest, const Table& tab,
		  const Vector<Vector<Slice> >& slices,
		  ROArrayColumn<Complex>& arr1)
{
  honest = 0.0;
  Table reftab;
  uInt startrow = 0;
  Vector<uInt> rows(nrowspertile);
  for(uInt i = 0; i < ntiles; ++i){
    indgen(rows, startrow);
    reftab = tab(rows);
    arr1.attach(reftab, "arr1");
    arr1.getColumn(slices, data, True);
    honest += data(1, 2, 3);
    startrow += nrowspertile;
  }
}

void readSlices()
{
  Timer timer;

  Table tab("dArrayColumn_tmp.tab");
  ROArrayColumn<Complex> arr1(tab, "arr1");

  // Makes sure the compiler doesn't optimize out reading arr1.
  Complex honest(0.0, 0.0);

  {
    // No axes, thus all entire axes.
    startLustreStats();
    Vector<Vector<Slice> > slices;
    Cube<Complex> data;
    timer.mark();
    reftabReader(data, honest, tab, slices, arr1);
    cerr << "arr1.getColumn(empty_slices, data, True) took      "
	 << timer.real() << "s." << endl;    
    writeLustreStats("extent_stats.noAxes");
  }
  {
    // Empty axes, thus entire axes.
    Vector<Vector<Slice> > slices(2);
    Cube<Complex> data;
    honest = 0.0;
    startLustreStats();
    timer.mark();
    reftabReader(data, honest, tab, slices, arr1);
    cerr << nreps
	 << " reps of arr1.getColumn(slices_empty_axes, data, True) took "
	 << timer.real() << "s." << endl;
    writeLustreStats("extent_stats.emptyAxes");
  }
  {
    // Entire axes, but in parts.
    Vector<Vector<Slice> > slices(2);
    slices[0].resize(2);
    uInt xsplit = 5;
    xsplit = min(nx / 2, xsplit);
    cerr << "x slices: (0, " << xsplit << ") and ("
	 << xsplit << ", " << nx - xsplit << ")" << endl;
    slices[0][0] = Slice(0, xsplit);
    slices[0][1] = Slice(xsplit, nx - xsplit);
    slices[1].resize(3);
    uInt ysplit1 = 10;
    ysplit1 = min(ny / 4, ysplit1);
    uInt ysplit2 = ysplit1 + 4;
    ysplit2 = ysplit1 + min(ny / 2, ysplit2 - ysplit1);
    cerr << "y slices: (0, " << ysplit1 << "), ("
	 << ysplit1 << ", " << ysplit2 - ysplit1 << ") and ("
	 << ysplit2 << ", " << ny - ysplit2 << ")" << endl;
    slices[1][0] = Slice(0, ysplit1);
    slices[1][1] = Slice(ysplit1, ysplit2 - ysplit1);
    slices[1][2] = Slice(ysplit2, ny - ysplit2);
    Cube<Complex> data;
    honest = 0.0;
    startLustreStats();
    timer.mark();
    reftabReader(data, honest, tab, slices, arr1);
    cerr << nreps
	 << " reps of arr1.getColumn(multiple_slices, data, True) took   "
	 << timer.real() << "s." << endl;
    writeLustreStats("extent_stats.multipleSlices");
  }
  cerr << "honest = " << honest << endl;
}

void readColumnCells()
{
  startLustreStats();
  Timer timer;

  Table tab("dArrayColumn_tmp.tab");
  ROArrayColumn<Complex> arr1(tab, "arr1");

  // Makes sure the compiler doesn't optimize out reading arr1.
  Complex honest(0.0, 0.0);

  Cube<Complex> data;
  uInt startrow = 0;
  uInt endrow = nrowspertile - 1;
  RefRows rr(startrow, endrow);  // no default c'tor?
  timer.mark();
  for(uInt i = 0; i < ntiles; ++i){
    rr = RefRows(startrow, endrow);
    arr1.getColumnCells(rr, data, True);
    honest += data(1, 2, 3);
    startrow += nrowspertile;
    endrow += nrowspertile;
  }
  cerr << "arr1.getColumnCells(rr, data, True) took           "
       << timer.real() << "s." << endl;
  writeLustreStats("extent_stats.columnCells");
  cerr << "honest = " << honest << endl;
}

// void writeCellSlices()
// {
//   Table tab("dArrayColumn_tmp.tab", Table::Update);
//   ArrayColumn<Complex> arr1(tab, "arr1");
//   {
//     // Axes parts with strides.
//     Vector<Vector<Slice> > slices(2);
//     slices[0].resize(2);
//     slices[0][0] = Slice(2,5,2);
//     slices[0][1] = Slice(12,3,2);
//     slices[1].resize(3);
//     slices[1][0] = Slice(1,3,3);
//     slices[1][1] = Slice(10,2,3);
//     slices[1][2] = Slice(16,1);
//     for (uInt i=0; i<tab.nrow(); ++i) {
//       cout << "put row " << i << endl;
//       Array<Complex> arr = Complex(1) + arr1.getSlice(i, Slicer(IPosition(2,2,1),
//                                                             IPosition(2,8,6),
//                                                             IPosition(2,2,3)));
      
//       arr1.putSlice (i, slices, arr);
//       AlwaysAssertExit (allEQ (arr1.getSlice(i, Slicer(IPosition(2,2,1),
//                                                        IPosition(2,8,6),
//                                                        IPosition(2,2,3))),
//                                arr));
//     }
//   }
// }

// void writeColumnSlices()
// {
//   Table tab("dArrayColumn_tmp.tab", Table::Update);
//   ArrayColumn<Complex> arr1(tab, "arr1");
//   {
//     // Entire axes, but in parts.
//     Vector<Vector<Slice> > slices(2);
//     slices[0].resize(2);
//     slices[0][0] = Slice(0,5);
//     slices[0][1] = Slice(5,15);
//     slices[1].resize(3);
//     slices[1][0] = Slice(0,10);
//     slices[1][1] = Slice(10,4);
//     slices[1][2] = Slice(14,16);
//     Array<Complex> arr = Complex(12) + arr1.getColumn();
//     arr1.putColumn (slices, arr);
//     AlwaysAssertExit (allEQ (arr1.getColumn(), arr));
//   }
// }

int main()
{
  try {
    createTab();
    readSlices();
    readColumnCells();
    system("rm -rf dArrayColumn_tmp.tab");
  } catch (AipsError& x) {
    cout << "Caught an exception: " << x.getMesg() << endl;
    return 1;
  } 
  return 0;                           // exit with success status
}
