/* try_run.cc */
#include <cstdio>  /* Needed for readline/readline.h */
#include <RPFITS.h>
#include <iostream>
int main() {
    float v;
    v = 0; /* Suppress warning about unused variable (needed for CFITSIO) */
    std::cout << names_.rpfitsversion;
    return 0;
}
