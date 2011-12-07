//
// C++ Interface: TableTraverse
//
// Description:
//
// Copyright: See COPYING file that comes with this distribution
//

/*
 * $Id: TableTraverse.h 2343 2011-11-06 14:55:32Z TakeshiNakazato $
 */

#ifndef ASAP_TABLETRAVERSE_H
#define ASAP_TABLETRAVERSE_H

#include <casa/aips.h>
#include <tables/Tables/BaseTable.h>
#include <tables/Tables/Table.h>

namespace asap {

  class TableVisitor {
  public:
    virtual ~TableVisitor() {}

    // This method is called just before the first <src>visit()<src> call.
    virtual void start() {}

    // <p><src>isFirst</src> is True only when the first visit.</p>
    // <p><src>recordNo</src> indicates which record is currently visited.
    // <p><src>nCols</src> indicates a number of columns used to sort records.
    // <src>nCols</src> is equal to a number of elements of
    // <src>columnNames</src> argument of <src>traverseTable()</src>
    // excluding the trailing NULL element.</p>
    // <p><src>colValues</src> is an array of arrays which contains all
    // column values for each column.
    // For example, <src>colValues[0]</src> is an array which contains all
    // values of the first column.
    // The order of <src>colValues</src> is same as the one of
    // <src>columnNames</src> argument of <src>traverseTable()</src>.</p>
    // <p>Returning False causes stopping iteration,
    // otherwise iterate over all records.</p>
    // <p>If <src>visit()</src> throws an exception, the iteration stops and
    // <src>finish()</src> will not be called.</p>
    virtual casa::Bool visit(casa::Bool isFirst, casa::uInt recordNo,
			     casa::uInt nCols,
			     void const *const colValues[]) = 0;

    // This method is called immediately after all visits have done.
    virtual void finish() {}
  };

  // <src>TypeManager</src> describes a type to handle the type dynamically.
  class TypeManager {
  public:
    virtual ~TypeManager() {}
    virtual casa::BaseCompare *getComparator() const = 0;
    virtual size_t sizeOf() const = 0;
    virtual void *allocArray(size_t size) const = 0;
    virtual void freeArray(void *array) const = 0;
  };

  // This template is applicable to the type which can be applied to
  // <src>casa::ObjCompare&lt;T&gt;</src>.
  template<class T> class TypeManagerImpl: public TypeManager {
  public:
    TypeManagerImpl(){}
    virtual casa::BaseCompare *getComparator() const {
      static casa::ObjCompare<T> comparator;
      return &comparator;
    }
    virtual size_t sizeOf() const {
      return sizeof(T);
    }
    virtual void *allocArray(size_t size) const {
      return new T[size];
    }
    virtual void freeArray(void *array) const {
      delete[] (T*)array;
    }
  };


  // <p>This function iterates over <src>table</src> with calling
  // <src>visitor->visit()</src> for each record after calling
  // <src>visitor->start()</src> and, finally, it calls
  // <src>visitor->finish()</src>.</p>
  // <p><src>columnNames</src> should be a NULL terminated array of column
  // names which will be used to sort records to determine the order of visits.
  // If <src>doSort</src> is False, <src>columnNames</src> is used only to
  // specify <src>nCols</src> and <src>colValues</src> arguments for calling
  // <src>visitor->visit()</src>.</p>
  // <p><src>typeManagers</src> should be a NULL terminated array of
  // <src>TypeManager</src> too.
  // It's number of elements and order of elements should be same as
  // <src>columnNames</src>.
  // <src>TypeManagerImpl</src> is useful to create instances of
  // <src>TypeManager</src>.</p>
  // <p>If <src>doSort</src> is True, the order of visits is ascending order
  // for each column. <src>columnNames[0]</src> is a primary sort key and
  // <src>columnNames[1]</src> is a secondary sort key, ...</p>
  void traverseTable(const casa::Table &table,
		     const char *const columnNames[],
		     const TypeManager *const typeManagers[],
		     TableVisitor *visitor,
		     casa::Bool doSort = casa::True);

}

#endif
