/*
 * Subtable.h
 *
 *  Created on: Jul 11, 2013
 *      Author: jjacobs
 */

#ifndef SUBTABLE_H_
#define SUBTABLE_H_

#include <vector>

#include <casa/BasicSL/String.h>

#include "Subtables.h"

namespace casa {

namespace cdo {

template <typename It>
class SubtableIterator {

public:

    typedef typename It::value_type T;
    typedef const T & ConstReference;
    typedef const T * ConstPointer;
    typedef SubtableIterator<T> Iterator;

    SubtableIterator (It begin, It end)
    : begin_p (begin),
      current_p (begin),
      end_p (end)
    {}

    SubtableIterator (const It & other)
    : begin_p (other.begin_p),
      current_p (other.current_p),
      end_p (other.end_p)
    {}

    ConstReference
    operator*() const
    {
        ThrowIf (current_p >= end_p,
                 "Attempt to dereference iterator when off end of data");

        return * current_p;
    }

    ConstPointer
    operator->() const
    {
        ThrowIf (current_p >= end_p,
                 "Attempt to dereference iterator when off end of data");

        return current_p;
    }

    Iterator &
    operator++()
    {
        ThrowIf (current_p >= end_p,
                 "Attempt to advance iterator pointing at end()");

        ++ current_p;

        return *this;
    }

    Iterator
    operator++(int)
    {
        ThrowIf (current_p >= end_p,
                 "Attempt to advance iterator pointing at end()");

        return Iterator(current_p ++);
    }

    bool
    operator== (const SubtableIterator<T> & other)
    {
        return current_p == other.current_p;
    }


    bool
    operator!= (const SubtableIterator<T> & other)
    {
        return current_p != other.current_p;
    }
private:

    const It begin_p;
    It current_p;
    const It end_p;
};






class Subtable {

public:

    virtual ~Subtable () {}

    SubtableType getType () const;
    String getName () const;
    bool isMemoryResident () const;
    bool isWritable () const;

    static Subtable * create (SubtableType type, const String & msPath);
    static SubtableType stFromString (const String & text);
    static String stToString (SubtableType type);

protected:

    Subtable (SubtableType type, const String & name, bool isMemoryResident = true,
              bool isWritable = false);

private:

    class Impl;

    bool isMemoryResident_p;
    bool isWritable_p;
    String name_p;
    SubtableType type_p;

    Subtable (const Subtable & other) = delete; // not implemented
    Subtable & operator= (const Subtable & other) = delete; // not implemented
};


template <typename T>
class SubtableImpl : public Subtable {

public:

    typedef T RowType;

    typedef SubtableIterator<const T> const_iterator;

//    SubtableImpl (bool isMutable, bool isWritable)
//    : isMutable_p (isMutable), isWritable_p (isWritable)
//    {}

    SubtableImpl (SubtableType type, const String & name,
                  bool isMemoryResident, bool isWritable)
    : Subtable (type, name, isMemoryResident, isWritable)
    {}

    virtual ~SubtableImpl () {}

    void
    append (const T & newRow)
    {
        rows_p.push_back (newRow);
    }

    virtual SubtableIterator<T> begin () const
    {
        return SubtableIterator<T> (rows_p.begin(), rows_p.end());
    }

    bool empty () const
    {
        return rows_p.empty();
    }

    virtual SubtableIterator<T> end () const
    {
        return SubtableIterator<T> (rows_p.end(), rows_p.end());
    }

    virtual String filename () const { return ""; }

    const T &
    get (Int id) const
    {
        ThrowIf (id < 0 || id >= rows_p.size(),
                 String::format ("SubtableImpl::get: index %d out of bounds [0, %d]",
                                 id, rows_p.size()-1));

        return rows_p [id];
    }

    unsigned int
    size () const
    {
        return rows_p.size();
    }

protected:

    String identifyTable () const;

private:

    std::vector<T> rows_p;
};

} // end namespace cdo

} // end namespace casa


#endif /* SUBTABLE_H_ */
