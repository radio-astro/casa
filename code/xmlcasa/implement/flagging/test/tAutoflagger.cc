#include <xmlcasa/flagging/autoflag_cmpt.h>
#include <iostream>

using namespace std;

int main()
{
    casac::autoflag af;
    
    af.reset();
    
    cout << "Did not crash, test has passed." << endl;

    return 0;
}
