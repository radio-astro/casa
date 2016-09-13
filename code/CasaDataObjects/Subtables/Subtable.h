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

class Subtable {

public:

    enum class ST : uInt {

        Antenna,
        DataDescription,
        Feed,
        Field,
        Observation,
        Pointing,
        Polarization,
        Processor,
        SpectralWindow,
        State,
        Source,
        Syscal,

        N_Types
    };

    virtual ~Subtable () {}

    Subtable * clone () const = 0;
    ST getType () const;
    String getName () const;
    Bool isMemoryResident () const;
    Bool isWritable () const;

    static Subtable * create (ST type, const String & msPath);
    static ST stFromString (const String & text);
    static String stToString (ST type);

protected:

    Subtable (ST type, const String & name, bool isMemoryResident,
              bool isWritable);

private:

    class Impl;

    Bool isMemoryResident_p;
    Bool isWritable_p;
    String name_p;
    ST type_p;

    Subtable (const Subtable & other); // not implemented
    Subtable & operator= (const Subtable & other); // not implemented
};


template <typename T>
class SubtableImpl : public Subtable {

public:

    typedef T RowType;

    typedef SubtableIterator<T> iterator;
    typedef SubtableIterator<const T> const_iterator;

    explicit SubtableImpl (Int nRows = 0);
    virtual ~SubtableImpl ();

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
    uInt size () const;

protected:

    String identifyTable () const;

private:

    std::vector<T *> rows_p;
};

template <typename T>
SubtableImpl<T>::SubtableImpl (Int nRows)
: rows_p (nRows, 0)
{}

template <typename T>
SubtableImpl<T>::~SubtableImpl ()
{
    // Delete all of the subtable row objects

    for (Int i = 0; i < size(); i++){
        delete rows_p [i];
    }
}


template <typename T>
const T &
SubtableImpl<T>::get (Int id) const
{
    ThrowIf (id < 0 || id >= size(),
             String::format ("SubtableImpl::get index %d out of range [0, %d]\n%s",
                             id,
                             size()),
                             identifyTable().c_str());
    return rows_p.size();
}

template <typename T>
Int
SubtableImpl<T>::append (const T & subtable)
{
    rows_p.push_back (subtable->clone());

    return rows_p.size() - 1;
}


template <typename T>
const T &
SubtableImpl<T>::set (Int id, const T & row)
{
    ThrowIf (id < 0 || id >= size(),
             String::format ("SubtableImpl::set index %d out of range [0, %d]\n%s",
                             id,
                             size()),
                             identifyTable().c_str());

    delete rows_p [id];

    rows_p [id] = row->clone();
}



template <typename T>
uInt
SubtableImpl<T>::size () const
{
    return rows_p.size();
}

template <typename T>
void
SubtableImpl<T>::resize (Int newSize) const
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

std::ostream & operator<< (std::ostream & os, Subtable::ST type);


}

}


#endif /* SUBTABLE_H_ */
