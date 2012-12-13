#include <flagging/Flagging/RFCubeLattice.h>
#include <flagging/Flagging/RFCommon.h>
#include <iostream>

using namespace casa;
using namespace std;

unsigned nchan = 8;
unsigned nifr = 30;

unsigned ncorr = 2;
unsigned ntime = 7;
unsigned nagent = 3;
 
void dump(const RFCubeLattice<RFlagWord> &r)
{   
    for (unsigned ifr = 0; ifr < nifr; ifr++) {
        for (unsigned chan = 0; chan < nchan; chan++) {
            
            cout << "(" << ifr << ", " << chan << ") = " << r(chan, ifr) << " ";
        }
        cout << endl;
    }
    
    return;
}

void test()
{
    unsigned val = 3;
    RFCubeLattice<RFlagWord> r(nchan, nifr, ntime, ncorr, nagent, val);

    dump(r);

    // set(chan, ifr, val)
    r.set(0, 0, 1);
    r.set(7, 29, 42);

    assert( r(0, 0) == 1);
    assert( r(7, 29) == 42 % (1<<(ncorr+nagent)));  

    for(unsigned i = 0; i < ((unsigned)1)<<(ncorr+nagent); i++)
        {
            unsigned chan = i/4;
            unsigned ifr = i/2;
            r.set(chan, ifr, i);
            assert( r(chan, ifr) == i);
        }

    dump(r);

    // advance()
    assert( r.position() == 0 );

    r.advance(5);

    assert( r.position() == 5 );

    // reset()
    r.reset();

    assert( r.position() == 0 );


    // set(chan, ifr, corr, val)
    r.set(7, 7, 0);
    assert( r(7, 7) == 0 );
    unsigned corr = 1;
    r.set(7, 7, corr, 1);
    assert( r(7, 7) == 2 );
    corr = 0;
    r.set(7, 7, corr, 1);
    assert( r(7, 7) == 3 );

    // set_column
    r.set_column( 3, 0 );
    for (unsigned chan = 0; chan < nchan; chan++)
        {
            assert( r(chan, 3) == 0);
        }


    r.set_column( 4, 29 );
    for (unsigned chan = 0; chan < nchan; chan++)
        {
            assert( r(chan, 4) == 29 );
        }

    dump(r);

    return;
}

int main()
{
    test();

    

    return 0;
}
