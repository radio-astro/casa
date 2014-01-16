/*
 * CasaTableRow.h
 *
 *  Created on: Jul 11, 2013
 *      Author: jjacobs
 */

#ifndef CASATABLEROW_H_
#define CASATABLEROW_H_


namespace casa {

namespace cdo {

class CasaTableRow {

public:

    enum { NoId = -1};

    CasaTableRow ();

    virtual ~CasaTableRow ();


    Int getId () const;
    Bool isDirty () const;

protected:

    virtual CasaTableRow * clone (Int id = NoId) const;
    void setDirty (Bool isDirty);

private:

    Bool dirty_p;

};

}
}


#endif /* CASATABLEROW_H_ */
