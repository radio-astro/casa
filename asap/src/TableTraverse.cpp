//
// C++ Interface: Scantable
//
// Description:
//
// Copyright: See COPYING file that comes with this distribution
//

#include <stdio.h>
#include <stdlib.h>

#include "TableTraverse.h"
#include <tables/Tables/TableColumn.h>
#include <tables/Tables/PlainColumn.h>
#include <tables/Tables/DataManager.h>

#define TIMING 0

#if TIMING
#include <sys/time.h>
#endif

static const char version[] = "$Id: TableTraverse.cpp 2316 2011-09-29 07:09:17Z MalteMarquarding $";

using namespace casa;

namespace asap {
  class Comparator {
  public:
    virtual ~Comparator() {}
    virtual int compare(const void *a, const void *b) = 0;
  };

  class CompContext: public Comparator {
    uInt nCols;
    void const *const *colValues;
    const TypeManager *const* typeManagers;
  public:
    CompContext(uInt nCols, void const *const colValues[],
		const TypeManager *const typeManagers[])
      : nCols(nCols), colValues(colValues), typeManagers(typeManagers) {
      DebugAssert(colValues != NULL, AipsError);
      DebugAssert(typeManagers != NULL, AipsError);
      for (uInt i = 0; i < nCols; i++) {
	DebugAssert(colValues[i] != NULL, AipsError);
	DebugAssert(typeManagers[i] != NULL, AipsError);
      }
    }

    virtual int compare(const void *a, const void *b) {
      uInt ai = *(uInt*)a; 
      uInt bi = *(uInt*)b; 
      for (size_t i = 0; i < nCols; i++) {
	const size_t valueSize = typeManagers[i]->sizeOf();
	const char *values = (const char *)colValues[i];
	int diff = typeManagers[i]->getComparator()->comp(&values[ai*valueSize], &values[bi*valueSize]);
	if (diff != 0) {
	  return diff;
	}
      }
      return 0;
    }
  };
}

#if TIMING
static double currenttime()
{
  struct timeval tv;
  int result = gettimeofday(&tv, NULL);
  return tv.tv_sec + ((double)tv.tv_usec) / 1000000.;
}
#endif

#ifdef HAVE_QSORT_R

static int compWithComparator(void *arg, const void *a, const void *b)
{
	return ((asap::Comparator *)arg)->compare(a, b);
}

#else

static inline void swap_(char *a, char *b, size_t size)
{
  char tmp[size];
  char *p = tmp;
  do {
    *p = *a;
    *a++ = *b;
    *b++ = *p++;
  } while (--size > 0);
}

static inline void cpy(char *dest, char *src, size_t size)
{
  for (size_t i = 0; i < size; i++) {
    dest[i] = src[i];
  }
}

// returning NULL means, already sorted.
static inline char *quickFindPivot(char *const left, char *const right,
				   const size_t size, asap::Comparator *comp)
{
  char *const m = &left[(((right - left) / size) / 2 ) * size];
  char *result = NULL;
  if (left == m) { // This means there are at most 2 elements.
    int diff = (left == right) ? 0 : comp->compare(left, right);
    if (diff > 0) {
      swap_(left, right, size);
    }
    if (diff != 0) {
      return right;
    }
    return result;
  }

  int diff = comp->compare(left, m);
  if (diff > 0) {
    swap_(left, m, size);
  }
  int diff2 = comp->compare(m, right);
  if (diff == 0 && diff2 == 0) {
    return result;
  }
  result = m;
  if (diff2 > 0) {
    swap_(right, m, size);
    int diff3 = comp->compare(left, m);
    if (diff3 > 0) {
      swap_(left, m, size);
    } else if (diff3 == 0) {
      result = right;
    }
  } else if (diff == 0) {
    result = right;
  }
  return result;
}

static inline char *findPivot(char *const start, char *const end,
			      size_t size, asap::Comparator *comp)
{
  char *result = quickFindPivot(start, end, size, comp);
  if (result || &start[2*size] >= end) {
    return result;
  }

  for (char *pivot = start + size; pivot <= end; pivot += size) {
    int diff = comp->compare(start, pivot);
    if (diff < 0) {
      return pivot;
    } else if (diff > 0) {
      swap_(start, pivot, size);
      return pivot;
    }
  }
  return NULL; // all values are same.
}

static void sort_(char *const left, char *const right, const size_t size,
		  asap::Comparator *comp, int level)
{
  char *const m = findPivot(left, right, size, comp);
  if (m == NULL) {
    return;
  }
  char *l = left;
  char *r = right;
  char pivot[size]  __attribute__ ((aligned (16)));
  cpy(pivot, m, size);
  while (true) {
    while (l < r && comp->compare(l, pivot) < 0) {
      l += size;
    }
    while (l < r && comp->compare(pivot, r) <= 0) {
      r -= size;
    }
    if (l >= r) {
      break;
    }
    swap_(l, r, size);
    l += size;
    r -= size;
  }
  if (l == r && comp->compare(l, pivot) < 0) {
    l += size;
  }
  sort_(left, l - size, size, comp, level + 1);
  sort_(l, right, size, comp, level + 1);
}

#endif

static inline void modernQSort(void *base, size_t elements, size_t size,
			       asap::Comparator *comp)
{
  if (elements > 1) {
#ifdef HAVE_QSORT_R
    qsort_r(base, elements, size, comp, compWithComparator);
#else
    sort_((char *)base, &((char *)base)[(elements - 1) * size], size, comp, 0);
#endif
  }
}

namespace asap {
  class ROTableColumnBackDoor: public ROTableColumn {
  public:
    static BaseColumn *baseColumnPtr(const ROTableColumn *col)
    {
      return ((ROTableColumnBackDoor*)col)->baseColPtr();
    }
  };

  static void copyColumnData(void *colValues, size_t elementSize, uInt nRows,
			     BaseColumn *refCol)
  {
    char *base = (char *)colValues;
    for (uInt i = 0; i < nRows; i++) {
      refCol->get(i, &base[elementSize * i]);
    }
  }

  void traverseTable(const Table &table,
		     const char *const columnNames[],
		     const TypeManager *const typeManagers[],
		     TableVisitor *visitor,
		     Bool doSort)
  {
#if TIMING
    double s = currenttime();
#endif
    uInt colCount = 0;
    for (; columnNames[colCount]; colCount++) {
      AlwaysAssert(typeManagers[colCount], AipsError);
    }

    ROTableColumn *cols[colCount];
    void *colValues[colCount];
    for (uInt i = 0; i < colCount; i++) {
      cols[i] = NULL;
      colValues[i] = NULL;
    }
    size_t sizes[colCount];
    const uInt nRows = table.nrow();
    for (uInt i = 0; i < colCount; i++) {
      cols[i] = new ROTableColumn(table, columnNames[i]);
      colValues[i] = typeManagers[i]->allocArray(nRows);
      sizes[i] = typeManagers[i]->sizeOf();
      BaseColumn *baseCol = ROTableColumnBackDoor::baseColumnPtr(cols[i]);
      copyColumnData(colValues[i], typeManagers[i]->sizeOf(), nRows, baseCol);
//       PlainColumn *col = dynamic_cast <PlainColumn *>(baseCol);
//       if (col) {
// 	const uInt gotRows = col->dataManagerColumn()->
// 	  getBlockV(0, nRows, colValues[i]);
// 	DebugAssert(gotRows == nRows, AipsError);
//       } else {
// 	copyColumnData(colValues[i], typeManagers[i]->sizeOf(), nRows, baseCol);
//       }
    }

    uInt *idx = new uInt[nRows];
    for (uInt i = 0; i < nRows; i++) {
      idx[i] = i;
    }

    try {
      if (doSort) {
	CompContext compCtx(colCount, colValues, typeManagers);
	modernQSort(idx, nRows, sizeof(idx[0]), &compCtx);
      }

      visitor->start();
      Bool first = True;
      for (uInt i = 0; i < nRows; i++) {
	if (visitor->visit(first, idx[i], colCount, colValues) == False) {
	  break;
	}
	first = False;
      }
      visitor->finish();
    } catch (...) {
      delete[] idx;
      for (uInt i = 0; i < colCount; i++) {
	typeManagers[i]->freeArray(colValues[i]);
	delete cols[i];
      }
      throw;
    }

    delete[] idx;
    for (uInt i = 0; i < colCount; i++) {
      typeManagers[i]->freeArray(colValues[i]);
      delete cols[i];
    }

#if TIMING
    double e = currenttime();
    printf("%s took %.3f sec\n", __PRETTY_FUNCTION__, e - s);
#endif
  }
}

#if 0 && TIMING
#include <assert.h>

#define elementsof(x) (sizeof(x)/sizeof(*x))

using namespace asap ;

class IntComp: public Comparator {
public:
  virtual int compare(const void *a, const void *b) {
    return (*(int*)a) - (*(int*)b);
  }
};

static int compare(const void *a, const void *b) {
  return (*(int*)a) - (*(int*)b);
}

int main()
{
  IntComp myComp;
  srand((int)currenttime());
  const size_t N = 1024*1024*100;
  int *x = new int[N];
  int xx[] = {
    5, 3,0,  1, 2,4, 4, 4
  };
#if 0
  {
    int x[] = {1};
    char *p = findPivot((char *)x, (char *)&x[0], sizeof(x[0]), &myComp);
    assert(p == NULL);
  }
  {
    int x[] = {1, 1};
    char *p = findPivot((char *)x, (char *)&x[1], sizeof(x[0]), &myComp);
    assert(p == NULL);
  }
  {
    int x[] = {2, 1};
    char *p = findPivot((char *)x, (char *)&x[1], sizeof(x[0]), &myComp);
    assert((int*)p == &x[1] && x[0] == 1 && x[1] == 2);
  }
  {
    int x[] = {1, 2};
    char *p = findPivot((char *)x, (char *)&x[1], sizeof(x[0]), &myComp);
    assert((int*)p == &x[1] && x[0] == 1 && x[1] == 2);
  }
  {
    int x[] = {1, 1, 1};
    char *p = findPivot((char *)x, (char *)&x[2], sizeof(x[0]), &myComp);
    assert(p == NULL);
  }
  {
    int x[] = {2, 1, 1};
    char *p = findPivot((char *)x, (char *)&x[2], sizeof(x[0]), &myComp);
    assert((int*)p == &x[2]);
    assert(x[0] == 1 && x[1] == 1 && x[2] == 2);
  }
  {
    int x[] = {1, 2, 1};
    char *p = findPivot((char *)x, (char *)&x[2], sizeof(x[0]), &myComp);
    assert((int*)p == &x[2]);
    assert(x[0] == 1 && x[1] == 1 && x[2] == 2);
  }
  {
    int x[] = {1, 1, 2};
    char *p = findPivot((char *)x, (char *)&x[2], sizeof(x[0]), &myComp);
    assert((int*)p == &x[2]);
    assert(x[0] == 1 && x[1] == 1 && x[2] == 2);
  }
  {
    int x[] = {2, 2, 1};
    char *p = findPivot((char *)x, (char *)&x[2], sizeof(x[0]), &myComp);
    assert((int*)p == &x[1]);
    assert(x[0] == 1 && x[1] == 2 && x[2] == 2);
  }
  {
    int x[] = {2, 1, 2};
    char *p = findPivot((char *)x, (char *)&x[2], sizeof(x[0]), &myComp);
    assert((int*)p == &x[1]);
    assert(x[0] == 1 && x[1] == 2 && x[2] == 2);
  }
  {
    int x[] = {1, 2, 2};
    char *p = findPivot((char *)x, (char *)&x[2], sizeof(x[0]), &myComp);
    assert((int*)p == &x[1]);
    assert(x[0] == 1 && x[1] == 2 && x[2] == 2);
  }
  {
    int x[] = {1, 2, 3};
    char *p = findPivot((char *)x, (char *)&x[2], sizeof(x[0]), &myComp);
    assert((int*)p == &x[1]);
    assert(x[0] == 1 && x[1] == 2 && x[2] == 3);
  }
  {
    int x[] = {2, 1, 3};
    char *p = findPivot((char *)x, (char *)&x[2], sizeof(x[0]), &myComp);
    assert((int*)p == &x[1]);
    assert(x[0] == 1 && x[1] == 2 && x[2] == 3);
  }
  {
    int x[] = {1, 3, 2};
    char *p = findPivot((char *)x, (char *)&x[2], sizeof(x[0]), &myComp);
    assert((int*)p == &x[1]);
    assert(x[0] == 1 && x[1] == 2 && x[2] == 3);
  }
  {
    int x[] = {3, 1, 2};
    char *p = findPivot((char *)x, (char *)&x[2], sizeof(x[0]), &myComp);
    assert((int*)p == &x[1]);
    assert(x[0] == 1 && x[1] == 2 && x[2] == 3);
  }
  {
    int x[] = {3, 2, 1};
    char *p = findPivot((char *)x, (char *)&x[2], sizeof(x[0]), &myComp);
    assert((int*)p == &x[1]);
    assert(x[0] == 1 && x[1] == 2 && x[2] == 3);
  }
  {
    int x[] = {2, 3, 1};
    char *p = findPivot((char *)x, (char *)&x[2], sizeof(x[0]), &myComp);
    assert((int*)p == &x[1]);
    assert(x[0] == 1 && x[1] == 2 && x[2] == 3);
  }
  {
    int x[] = {36, 39, 42, 41, 36};
    char *p = findPivot((char *)x, (char *)&x[4], sizeof(x[0]), &myComp);
    assert((int*)p == &x[4]);
    assert(x[4] == 42);
  }

  //assert(("assert enabled: OK", false));
  modernQSort(x, 8, sizeof(x[0]), &myComp);
  for (int i = 0; i < 8; i++) {
    fprintf(stderr, "%d\n", x[i]);
  }
#endif
  fprintf(stderr, "init\n");
  for (int i = 0; i < N; i++) {
    x[i] = rand();
  }
  fprintf(stderr, "sorting\n");
  modernQSort(x, 1, sizeof(x[0]), &myComp);
  modernQSort(x, 2, sizeof(x[0]), &myComp);
  modernQSort(x, 3, sizeof(x[0]), &myComp);
  if (! getenv("QSORT")) {
    double _s = currenttime();
    modernQSort(x, N, sizeof(x[0]), &myComp);
    double _e = currenttime();
    fprintf(stderr, "modernSorted %.3f\n", _e - _s);
  } else {
    double _s = currenttime();
    qsort(x, N, sizeof(x[0]), compare);
    double _e = currenttime();
    fprintf(stderr, "qsorted %.3f\n", _e - _s);
  }
  fprintf(stderr, "outputting\n");
  for (int i = 0; i < N; i++) {
    printf("%d\n", x[i]);
  }
  delete[] x;
}
#endif
