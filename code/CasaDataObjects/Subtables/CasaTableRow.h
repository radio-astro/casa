/*
 * CasaTableRow.h
 *
 *  Created on: Jul 11, 2013
 *      Author: jjacobs
 */

#ifndef CASATABLEROW_H_
#define CASATABLEROW_H_

namespace casa {

class String;

namespace cdo {

class CasaTableRow {

public:

    enum { NoId = -1};

    CasaTableRow (int id);
    CasaTableRow (const CasaTableRow &);

    virtual ~CasaTableRow ();

    int getId () const;

    virtual String toString () const = 0;

protected:

    void setDirty (bool isDirty);

private:

    int id_p;
};

}
}


#endif /* CASATABLEROW_H_ */
