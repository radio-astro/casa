/*
 * Subtable.h
 *
 *  Created on: Jul 11, 2013
 *      Author: jjacobs
 */

#ifndef SUBTABLE_H_
#define SUBTABLE_H_

#include <vector>

namespace casa {

namespace cdo {

template <typename T>
class SubtableIterator {

public:

    typedef T Value;
    typedef T & Reference;
    typedef T * Pointer;
    typedef SubtableIterator<T> Iterator;

    SubtableIterator (Pointer begin, Pointer end)
    : begin_p (begin),
      current_p (begin),
      end_p (end)
    {}

    SubtableIterator (const Iterator & other)
    : begin_p (other.begin_p),
      current_p (other.current_p),
      end_p (other.end_p)
    {}

    Reference
    operator*() const
    {
        ThrowIf (current_p >= end_p,
                 "Attempt to dereference iterator when off end of data");

        return * current_p;
    }

    Pointer
    operator->() const
    {
        ThrowIf (current_p >= end_p,
                 "Attempt to dereference iterator when off end of data");

        return current_p;
    }

    Iterator &
    operator++()
    {
        ++ current_p;

        return *this;
    }

    Iterator
    operator++(int)
    {
        return Iterator(current_p ++);
    }


    const Pointer
    base() const
    { return current_p; }

private:

    const Pointer begin_p;
    Pointer current_p;
    const Pointer end_p;
};

template <typename T>
Bool
operator== (const SubtableIterator<T> & a, const SubtableIterator<T> & b)
{
    return a.base() == b.base();
}

template <typename T>
Bool
operator!= (const SubtableIterator<T> & a, const SubtableIterator<T> & b)
{
    return a.base() != b.base();
}

class SubtableBase {

public:

    virtual ~SubtableBase () {}

protected:

    SubtableBase () {}
};


template <typename T>
class Subtable : public SubtableBase {

public:

    typedef T RowType;

    typedef SubtableIterator<T> iterator;
    typedef SubtableIterator<const T> const_iterator;

    explicit Subtable (Int nRows = 0);
    virtual ~Subtable ();

    Int append (const CasaTableRow & newRow);

    virtual iterator begin ();
    virtual const_iterator begin () const;

    Bool empty () const;

    virtual iterator end ();
    virtual const_iterator end () const;

    String filename () const;
    const T & get (Int id) const;
    String name () const;

    void resize (Int newSize);

    template <typename P>
    const_iterator select (P predicate) const;
    template <typename P>
    iterator select (P predicate);

    void set (Int id, const T &);
    Int size () const;

protected:

    String identifyTable () const;

private:

    std::vector<T *> rows_p;
};

template <typename T>
Subtable::Subtable (Int nRows = 0)
: rows_p (nRows, 0)
{}

template <typename T>
Subtable::~Subtable (Int nRows = 0)
: rows_p (nRows, 0)
{
    // Delete all of the subtable row objects

    for (Int i = 0; i < size(); i++){
        delete rows_p [i];
    }
}


template <typename T>
const T &
Subtable::get(Int id) const
{
    ThrowIf (id < 0 || id >= size(),
             String::format ("Subtable::get index %d out of range [0, %d]\n%s",
                             id,
                             size()),
                             identifyTable().c_str());
    return rows_p.size();
}

template <typename T>
Int
Subtable::append(const T & subtable)
{
    rows_p.push_back (subtable->clone());

    return rows_p.size() - 1;
}


template <typename T>
const T &
Subtable::set(Int id, const T & subtable)
{
    ThrowIf (id < 0 || id >= size(),
             String::format ("Subtable::set index %d out of range [0, %d]\n%s",
                             id,
                             size()),
                             identifyTable().c_str());

    delete rows_p [id];

    rows_p = subtable->clone();
}



template <typename T>
Int
Subtable::size() const
{
    return rows_p.size();
}

template <typename T>
void
Subtable::resize (Int newSize) const
{
    if (newSize < size()){

        // Delete the rows that are being truncated

        for (Int i = newSize; i < size(); i++){
            delete rows_p [i];
        }

    }

    // Resize the vector to the appropriate size filling any added
    // elements to 0.

    rows_p.resize (newSize, 0);
}



}

}


#endif /* SUBTABLE_H_ */
