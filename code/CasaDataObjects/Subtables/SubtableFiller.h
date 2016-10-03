/*
 * SubtableFiller.h
 *
 *  Created on: Jan 14, 2014
 *      Author: jjacobs
 */

#ifndef SUBTABLEFILLER_H_
#define SUBTABLEFILLER_H_

namespace casacore{

class Table;

}

namespace casa {


template <typename SubtableType>
class SubtableColumnPicklerBase {

public:

    virtual ~SubtableColumnPicklerBase () {}

    virtual void readFromTable (SubtableType &, casacore::Table &) = 0;
    virtual void writeToTable (SubtableType &, casacore::Table &) = 0;

};

template <typename SubtableType, typename ColumnType, typename ColumnId, typename Getter, typename Setter>
class SubtableColumnPickler : public SubtableColumnPicklerBase<SubtableType> {

public:

    SubtableColumnPickler (ColumnId columnId, Getter & getter, Setter & setter);

    void
    readFromTable (SubtableType & subtable, casacore::Table & table)
    {
        ColumnType column;
        column.attach (table, columnId_p);
        casacore::Int i;

        try {

            for (i = 0; i < subtable.size(); i ++){

                typename SubtableType::RowType row = subtable.get (i);
                ((& row) ->* setter_p) (column (i));

            }
        }
        catch (casacore::AipsError & e){

            Rethrow (e, casacore::String::format ("While filling from row %d of column %s in table %s",
                                        i, getColumnName (columnId), table.tableName().c_str()));
        }
    }
    void writeToTable (casacore::Table & table, SubtableType & subtable);

private:

    ColumnId columnId_p;
    Getter getter_p;
    Setter setter_p;

};


template <typename S>
class SubtableFiller {

public:

    SubtableFiller ();

    virtual ~SubtableFiller ();

    template <typename ColumnType, typename ColumnId, typename Getter, typename Setter>
    SubtableColumnPicklerBase<S> *
    addPickler (ColumnId columnId, Getter getter, Setter setter)
    {
        picklers_p.push_back (new SubtableColumnPickler <S, ColumnType, ColumnId, Getter, Setter>
                                                         (columnId, getter, setter));
    }

    void readFromTable (S & subtableObject, casacore::Table & table);
    void writeToTable (S & subtableObject, casacore::Table & table);


protected:

    template <typename ColumnType, typename CellType, typename ColumnId, typename Setter>
    void
    SubtableFiller::fillColumn (ColumnId columnId,  Setter & setter);

private:

    typedef std::vector<SubtableColumnPicklerBase<S> *> Picklers;

    Picklers picklers_p;
};

template <typename S>
SubtableFiller::SubtableFiller ()
{}


template <typename S>
void
SubtableFiller::readFromTable (S & subtableObject, casacore::Table & table)
{
    for (Picklers::iterator pickler = picklers_p.begin();
         pickler != picklers_p.end();
         ++ pickler){

        (* pickler) -> readFromTable (subtableObject, table);

    }
}

template <typename S>
void
SubtableFiller::writeToTable (S & subtableObject, casacore::Table & table)
{
    for (Picklers::iterator pickler = picklers_p.begin();
         pickler != picklers_p.end();
         ++ pickler){

        (* pickler) -> writeToTable (subtableObject, table);

    }
}


} // end namespace casa

#endif /* SUBTABLEFILLER_H_ */
